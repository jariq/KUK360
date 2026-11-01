#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QString>

// Persistent user settings (QSettings). Every setting has its key only in Settings.cpp and is
// read and written only through the functions here.
//
// QSettings stores them by the organization and application name set in main.cpp: in the
// registry key HKEY_CURRENT_USER\Software\KUK360\KUK360 on Windows, in ~/Library/Preferences/
// com.kuk360.KUK360.plist on macOS and in ~/.config/KUK360/KUK360.conf on Linux.
namespace Settings {

// Directory of the last opened image, where the file dialog opens. Empty before the first
// image is opened. The directory may no longer exist.
QString lastDirectory();
void setLastDirectory(const QString &directory);

// Time of the last check for updates, or of the last reminder of it (see UpdateChecker.h).
// Invalid before the first start. Stored in UTC.
QDateTime lastUpdateCheck();
void setLastUpdateCheck(const QDateTime &time);

// Size, position and maximized state of the main window (QWidget::saveGeometry()), saved
// when it closes. Empty before the first close.
QByteArray windowGeometry();
void setWindowGeometry(const QByteArray &geometry);

} // namespace Settings
