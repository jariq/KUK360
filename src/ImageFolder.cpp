#include "ImageFolder.h"

#include <QCollator>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <algorithm>

namespace {

// File names in the order of the file managers (Explorer, Finder, Nautilus): case-insensitive
// and natural, numbers compared by value (pano2 before pano10).
// Total Commander sorts this way only with its natural sorting option turned on.
QCollator fileNameCollator()
{
    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    collator.setNumericMode(true);
    return collator;
}

} // namespace

const QStringList &ImageFolder::nameFilters()
{
    // Photo formats Qt reads without extra dependencies (JPEG, GIF: qjpeg, qgif plugins of
    // Qt GUI; PNG, BMP: built in; TIFF, WebP: Qt Image Formats plugins). GIF shows the first
    // frame. Panorama detection (XMP) is done for JPEG only.
    static const QStringList filters = {
        QStringLiteral("*.jpg"), QStringLiteral("*.jpeg"), QStringLiteral("*.png"), QStringLiteral("*.bmp"),
        QStringLiteral("*.gif"), QStringLiteral("*.tif"), QStringLiteral("*.tiff"), QStringLiteral("*.webp"),
    };
    return filters;
}

bool ImageFolder::isSupportedFile(const QString &filePath)
{
    const QFileInfo info(filePath);
    return info.isFile() && QDir::match(nameFilters(), info.fileName());
}

void ImageFolder::setCurrentFile(const QString &filePath)
{
    m_currentFile = filePath.isEmpty() ? QString() : QFileInfo(filePath).absoluteFilePath();
}

// Sorted supported files of the directory of the current file. Listing a large directory
// (or one on a network drive) is slow, so the list is kept until the directory or its
// modification time changes, or refresh() is called.
const QStringList &ImageFolder::files() const
{
    const QString dir = QFileInfo(m_currentFile).absolutePath();
    const QDateTime modified = QFileInfo(dir).lastModified();
    if (dir == m_listedDir && modified == m_listedDirModified)
        return m_files;

    m_files = QDir(dir).entryList(nameFilters(), QDir::Files | QDir::Readable);
    const QCollator collator = fileNameCollator();
    std::sort(m_files.begin(), m_files.end(),
              [&collator](const QString &a, const QString &b) { return collator.compare(a, b) < 0; });
    m_listedDir = dir;
    // A change within the resolution of the modification time (2 s on FAT) after the last one
    // would not change it ("racy" timestamps, as in git), so a directory modified just now is
    // listed again next time.
    constexpr qint64 kTimestampResolutionSecs = 2;
    m_listedDirModified =
        modified.secsTo(QDateTime::currentDateTime()) > kTimestampResolutionSecs ? modified : QDateTime();
    return m_files;
}

QString ImageFolder::adjacentFile(int step) const
{
    if (m_currentFile.isEmpty())
        return QString();

    // The modification time of a directory does not change on every file system (FAT and
    // exFAT of memory cards); a list that is evidently out of date is read again.
    QString file;
    if (!adjacentFileInList(step, &file)) {
        m_listedDir.clear();
        adjacentFileInList(step, &file);
    }
    return file;
}

// Finds the adjacent file in the cached list. Returns false when the list is out of date:
// the current supported file exists but is not in it, or the found file no longer exists.
bool ImageFolder::adjacentFileInList(int step, QString *file) const
{
    file->clear();
    const QFileInfo current(m_currentFile);
    // A supported current file that exists must be in the list.
    const bool mustBeListed = QDir::match(nameFilters(), current.fileName()) && current.exists();
    const QStringList &files = this->files();
    if (files.isEmpty())
        return !mustBeListed;

    bool upToDate = true;
    const QCollator collator = fileNameCollator();
    const auto lessThan = [&collator](const QString &a, const QString &b) { return collator.compare(a, b) < 0; };

    // On a case-insensitive file system (Windows, macOS) the path may differ in case from
    // the name in the directory (e.g. typed on the command line); an existing file then
    // matches its listed name regardless of case.
    int index = files.indexOf(current.fileName());
    if (index < 0 && current.exists()) {
        const auto it = std::find_if(files.cbegin(), files.cend(), [&](const QString &name) {
            return name.compare(current.fileName(), Qt::CaseInsensitive) == 0;
        });
        if (it != files.cend())
            index = int(it - files.cbegin());
        else if (mustBeListed)
            upToDate = false;  // e.g. created after the directory was listed
    }
    if (index >= 0 && files.size() == 1)
        return true;  // the current file is the only one

    // The current file may have been renamed or deleted meanwhile; then continue from
    // the position where it would be.
    if (index < 0) {
        const auto it = std::lower_bound(files.cbegin(), files.cend(), current.fileName(), lessThan);
        index = int(it - files.cbegin());
        if (step > 0)
            --index;  // the file at the insertion point is the next one
    }

    const int count = int(files.size());
    *file = QDir(m_listedDir).absoluteFilePath(files.at(((index + step) % count + count) % count));
    return upToDate && QFileInfo::exists(*file);
}

bool ImageFolder::moveCurrentFileToTrash(QString *errorString)
{
    // MainWindow::openImage() never makes a directory the current file; this is a safety net,
    // as moveToTrash() would move the whole directory.
    if (QFileInfo(m_currentFile).isDir()) {
        if (errorString)
            *errorString = QCoreApplication::translate("ImageFolder", "It is a directory, not an image file.");
        return false;
    }

    QFile file(m_currentFile);
    if (m_currentFile.isEmpty() || !file.moveToTrash()) {
        if (errorString)
            *errorString = file.errorString();
        return false;
    }
    refresh();
    return true;
}
