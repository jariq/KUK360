#pragma once

#include <QDateTime>
#include <QString>
#include <QStringList>

// The current image within its directory: knows which files are supported and finds
// the previous / next one in natural order (case-insensitive, pano2 before pano10),
// wrapping around at both ends. Does not load images; MainWindow opens the returned paths.
class ImageFolder
{
public:
    // Name filters of the supported image files, e.g. "*.jpg"; the only list of them.
    static const QStringList &nameFilters();
    // True for an existing file matching nameFilters() (case-insensitive).
    static bool isSupportedFile(const QString &filePath);

    QString currentFile() const { return m_currentFile; }
    void setCurrentFile(const QString &filePath);

    // Absolute path of the previous / next supported file in the directory of the
    // current file, or an empty string if there is no other file.
    QString previousFile() const { return adjacentFile(-1); }
    QString nextFile() const { return adjacentFile(+1); }
    // True if the directory contains another supported file besides the current one.
    bool hasOtherFiles() const { return !nextFile().isEmpty(); }

    // The directory is listed again on the next query (e.g. when the window gets activated,
    // as other applications may have changed it meanwhile).
    void refresh() { m_listedDir.clear(); }

    // Moves the current file to the trash / recycle bin (QFile::moveToTrash, works on
    // all platforms). The current file stays set as the position, so nextFile() then
    // returns the file that followed it. On failure returns false and fills errorString.
    bool moveCurrentFileToTrash(QString *errorString = nullptr);

private:
    const QStringList &files() const;
    QString adjacentFile(int step) const;
    bool adjacentFileInList(int step, QString *file) const;

    QString m_currentFile;  // absolute path

    // Cached sorted list of the supported files in m_listedDir (files())
    mutable QString m_listedDir;
    mutable QDateTime m_listedDirModified;
    mutable QStringList m_files;
};
