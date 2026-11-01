#include "FileLocation.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QUrl>

#if defined(Q_OS_LINUX)
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#endif

// Platform specific file locations; this is the only place with such code.

namespace {

void openDirectory(const QFileInfo &fileInfo)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absolutePath()));
}

} // namespace

void showInFileManager(const QString &filePath)
{
    const QFileInfo fileInfo(filePath);

#if defined(Q_OS_WIN)
    // explorer.exe needs exactly  /select,"<path>"  which QProcess's own argument
    // quoting cannot produce, hence the native command line.
    QProcess explorer;
    explorer.setProgram(QStringLiteral("explorer.exe"));
    explorer.setNativeArguments(
        QStringLiteral("/select,\"%1\"").arg(QDir::toNativeSeparators(fileInfo.absoluteFilePath())));
    if (explorer.startDetached())
        return;
#elif defined(Q_OS_MACOS)
    if (QProcess::startDetached(QStringLiteral("open"), {QStringLiteral("-R"), fileInfo.absoluteFilePath()}))
        return;
#elif defined(Q_OS_LINUX)
    // The freedesktop.org file manager interface (Nautilus, Dolphin, Nemo, Caja, Thunar...)
    // opens the directory and selects the file. The call is asynchronous, as D-Bus may first
    // have to start the file manager; without such a file manager only the directory opens.
    QDBusMessage call = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.FileManager1"), QStringLiteral("/org/freedesktop/FileManager1"),
        QStringLiteral("org.freedesktop.FileManager1"), QStringLiteral("ShowItems"));
    const QString uri = QString::fromUtf8(QUrl::fromLocalFile(fileInfo.absoluteFilePath()).toEncoded());
    call << QStringList{uri} << QString();
    auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(call), qApp);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, [fileInfo](QDBusPendingCallWatcher *w) {
        if (w->isError())
            openDirectory(fileInfo);
        w->deleteLater();
    });
    return;
#endif

    // Fallback: open the directory.
    openDirectory(fileInfo);
}
