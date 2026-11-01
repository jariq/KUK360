#pragma once

#include <QString>

// Opens the system file manager at the file's directory and selects the file where the
// platform supports it (Windows Explorer, macOS Finder, Linux file managers with the
// freedesktop.org D-Bus interface). Otherwise only the directory is opened.
void showInFileManager(const QString &filePath);
