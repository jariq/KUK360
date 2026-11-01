#include "ButtonPanel.h"

#include "FontAwesome.h"
#include "Theme.h"

#include <QAction>
#include <QHBoxLayout>
#include <QToolButton>

using FontAwesome::Icon;

namespace {

// The buttons are larger than their icons, so they are easier to hit.
constexpr int kButtonSize = 56;
constexpr int kIconSize = 30;
// Gaps of up to 41 px between the icons, i.e. 15 px between the buttons (half of the
// original 82 px, which looked too sparse)
constexpr int kIconGap = 41;
constexpr int kSpacing = 8;

// Panel height: 96 px from a window height of 900 px, 72 px at the minimum height of
// 450 px and linearly in between (all in logical pixels).
constexpr int kLowWindow = 450;
constexpr int kHighWindow = 900;
constexpr int kLowPanel = 72;
constexpr int kHighPanel = 96;

} // namespace

ButtonPanel::ButtonPanel(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("buttonPanel"));
    setAttribute(Qt::WA_StyledBackground);
    setFixedHeight(kHighPanel);  // until adjustToWindowHeight()

    // In the order of Button. Tool tips name the keyboard shortcuts of the actions.
    const QList<QPair<Icon, QString>> buttons = {
        {Icon::Globe, tr("Switch projection (F2)")},
        {Icon::Unlock, tr("Lock/unlock projection (F3)")},
        {Icon::Th, tr("Show/hide grid (F4)")},
        {Icon::ChevronLeft, tr("Previous image in the folder (Left arrow)")},
        {Icon::ArrowsAlt, tr("Enter/exit full screen (F12)")},
        {Icon::ChevronRight, tr("Next image in the folder (Right arrow)")},
        {Icon::Undo, tr("Rotate left (L)")},
        {Icon::Repeat, tr("Rotate right (R)")},
        {Icon::TrashO, tr("Delete image (DEL)")},
    };

    auto *layout = new QHBoxLayout(this);
    // No vertical margins: the buttons are centered and must fit the lowest panel.
    const QMargins margins = layout->contentsMargins();
    layout->setContentsMargins(margins.left(), 0, margins.right(), 0);
    // The gaps shrink in a narrow window (down to the spacing, which the layout adds once
    // between a button and the following spacer); the outer stretches take any extra width.
    layout->setSpacing(kSpacing);
    layout->addStretch();
    for (const auto &[glyph, toolTip] : buttons) {
        if (!m_buttons.isEmpty())
            layout->addSpacerItem(new QSpacerItem(kIconGap - (kButtonSize - kIconSize) - kSpacing, 0,
                                                  QSizePolicy::Maximum));
        auto *button = new QToolButton;
        button->setIcon(FontAwesome::icon(glyph));
        button->setIconSize(QSize(kIconSize, kIconSize));
        button->setFixedSize(kButtonSize, kButtonSize);
        button->setToolTip(toolTip);
        button->setAutoRaise(true);
        button->setEnabled(false);
        layout->addWidget(button);
        m_buttons.append(button);
    }
    layout->addStretch();

    applyTheme();
}

void ButtonPanel::setAction(Button button, QAction *action)
{
    QToolButton *toolButton = this->button(button);
    toolButton->setEnabled(action->isEnabled());
    connect(action, &QAction::enabledChanged, toolButton, &QToolButton::setEnabled);
    connect(toolButton, &QToolButton::clicked, action, &QAction::trigger);
}

void ButtonPanel::adjustToWindowHeight(int windowHeight)
{
    windowHeight = qBound(kLowWindow, windowHeight, kHighWindow);
    setFixedHeight(kLowPanel + (kHighPanel - kLowPanel) * (windowHeight - kLowWindow) / (kHighWindow - kLowWindow));
}

void ButtonPanel::applyTheme()
{
    const Theme::Colors &colors = Theme::colors();
    QString style = QStringLiteral("#buttonPanel { background-color: %1; }").arg(Theme::css(colors.panelBackground));
#if defined(Q_OS_MACOS)
    // The macOS style draws a square bezel around large auto-raise tool buttons; the
    // hover feedback comes from the icon colors, the pressed one from the background.
    style += QStringLiteral("#buttonPanel QToolButton { border: none; border-radius: 6px;"
                            " background: transparent; padding: 6px; }"
                            "#buttonPanel QToolButton:pressed { background-color: %1; }")
                 .arg(Theme::css(colors.iconNormal, 50));
#endif
    setStyleSheet(style);
}
