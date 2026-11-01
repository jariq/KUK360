#include "CompactMenuBarStyle.h"

#include <QApplication>
#include <QStyleOption>

namespace {

const QString kBaseStyle = QStringLiteral("windows11");

// The Windows 11 style (Qt 6.12, qwindows11style.cpp) draws the hover background 5 px
// inside the item rect and the text 11 px inside it (4 px from the top, 6 px from the
// bottom), in a 10 pt font, while the width comes from the 9 pt font of the menu bar.
constexpr int kStyleHoverMargin = 5;
// Our item: the text with kTextMargin on both sides (enough for the larger font) and
// the hover background kHoverMargin inside the item.
constexpr int kTextMargin = 12;
constexpr int kHoverMargin = 2;
constexpr int kItemHeight = 26;

} // namespace

CompactMenuBarStyle::CompactMenuBarStyle()
    : QProxyStyle(kBaseStyle)
{
}

void CompactMenuBarStyle::install()
{
    if (QApplication::style()->name() == kBaseStyle)
        QApplication::setStyle(new CompactMenuBarStyle);
}

QSize CompactMenuBarStyle::sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size,
                                            const QWidget *widget) const
{
    if (type == CT_MenuBarItem && !size.isEmpty())
        return QSize(size.width() + 2 * kTextMargin, kItemHeight);
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

void CompactMenuBarStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
                                      const QWidget *widget) const
{
    if (element == CE_MenuBarItem) {
        // Widen the rect for the style, so its hover background lands kHoverMargin inside
        // our item. The option is changed in place and restored: QMenuBar passes a private
        // subclass (QStyleOptionMenuItemV2 with the pressed state) that a copy would lose.
        auto *itemOption = const_cast<QStyleOption *>(option);
        const QRect rect = itemOption->rect;
        const int widen = kStyleHoverMargin - kHoverMargin;
        itemOption->rect.adjust(-widen, 0, widen, 0);
        QProxyStyle::drawControl(element, itemOption, painter, widget);
        itemOption->rect = rect;
        return;
    }
    QProxyStyle::drawControl(element, option, painter, widget);
}
