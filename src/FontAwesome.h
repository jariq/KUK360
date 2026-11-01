#pragma once

#include <QIcon>

// Icons rendered from the bundled Font Awesome 4.7 font (src/fontawesome-webfont.ttf).
// Glyphs are drawn on demand, so icons stay sharp at any size and device pixel ratio.
namespace FontAwesome {

// Code points from https://fontawesome.com/v4/cheatsheet/
enum class Icon : char16_t {
    Arrows = 0xf047,
    ArrowsAlt = 0xf0b2,
    ChevronLeft = 0xf053,
    ChevronRight = 0xf054,
    Copyright = 0xf1f9,
    ExternalLink = 0xf08e,
    FilesO = 0xf0c5,
    Folder = 0xf07b,
    FolderOpen = 0xf07c,
    Globe = 0xf0ac,
    Heartbeat = 0xf21e,
    Lock = 0xf023,
    Map = 0xf279,
    PowerOff = 0xf011,
    Refresh = 0xf021,
    Repeat = 0xf01e,
    SearchMinus = 0xf010,
    SearchPlus = 0xf00e,
    Th = 0xf00a,
    TrashO = 0xf014,
    Undo = 0xf0e2,
    Unlock = 0xf09c,
    WindowClose = 0xf2d3,
};

// The icon takes its colors (normal, hover, disabled) from Theme each time it is painted,
// so it follows a change of the system color scheme.
QIcon icon(Icon glyph);

} // namespace FontAwesome
