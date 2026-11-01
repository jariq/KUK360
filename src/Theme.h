#pragma once

#include <QColor>
#include <QString>

// Colors of KUK360's own UI elements (button panel, icons, overlays, backgrounds) for the
// light and the dark color scheme of the system. Standard Qt widgets (menus, dialogs,
// sliders) follow the scheme by themselves. The light colors are those of the original
// design. Widgets apply the colors again on QStyleHints::colorSchemeChanged.
namespace Theme {

struct Colors
{
    QColor panelBackground;    // button panel, About dialog
    QColor viewBackground;     // welcome page, flat view
    QColor text;               // "Open image...", About title, message overlay
    QColor secondaryText;      // About subtitle
    QColor iconNormal;
    QColor iconActive;         // hover on auto-raise buttons
    QColor iconDisabled;
    QColor overlayBackground;  // message overlay, drawn translucent
};

// Makes Qt follow the light or dark color scheme of the system where it does not do so by
// itself (Linux, see Theme.cpp). Call once after the QApplication is created.
void followSystemColorScheme();

// Colors of the current color scheme.
const Colors &colors();

// "rgba(r, g, b, a)" for style sheets.
QString css(const QColor &color, int alpha = 255);

} // namespace Theme
