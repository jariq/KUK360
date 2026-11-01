#pragma once

#include <QProxyStyle>

// The Qt Windows 11 style ("windows11") with smaller menu bar items. The style sizes
// them like WinUI (32 px high with wide margins), which makes the menu bar look too
// large next to the compact menus. Everything else, including the drawing of the items,
// stays with the style.
class CompactMenuBarStyle : public QProxyStyle
{
    Q_OBJECT

public:
    // Uses the compact style if the application has the Windows 11 style.
    static void install();

    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size,
                           const QWidget *widget) const override;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
                     const QWidget *widget) const override;

private:
    CompactMenuBarStyle();
};
