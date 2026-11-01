#include "MessageBoxes.h"

#include <QCoreApplication>
#include <QMessageBox>

namespace MessageBoxes {

void showInformation(QWidget *parent, const QString &text)
{
    QMessageBox::information(parent, QCoreApplication::applicationName(), text);
}

void showWarning(QWidget *parent, const QString &text)
{
    QMessageBox::warning(parent, QCoreApplication::applicationName(), text);
}

void showCritical(QWidget *parent, const QString &text)
{
    QMessageBox::critical(parent, QCoreApplication::applicationName(), text);
}

bool askYesNo(QWidget *parent, const QString &text)
{
    return QMessageBox::question(parent, QCoreApplication::applicationName(), text,
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
           == QMessageBox::Yes;
}

} // namespace MessageBoxes
