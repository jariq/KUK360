#pragma once

#include <QString>

class QWidget;

// Modal message boxes titled with the application name. The window title is not used, it
// contains the path of the open file (and is empty for a child widget).
namespace MessageBoxes {

void showInformation(QWidget *parent, const QString &text);
void showWarning(QWidget *parent, const QString &text);
void showCritical(QWidget *parent, const QString &text);
// Yes / No question, Yes is the default. Returns true for Yes.
bool askYesNo(QWidget *parent, const QString &text);

} // namespace MessageBoxes
