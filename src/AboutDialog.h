#pragma once

#include <QDialog>

// Modal "About KUK360" window: application name, version and subtitle on top, the GPL
// license of KUK360 and the notices of third-party components in a scrollable text below.
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);

private:
    void applyTheme();
};
