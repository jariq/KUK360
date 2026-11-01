#include "MessageOverlay.h"

#include "Theme.h"

#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

namespace {

constexpr int kBottomMargin = 12;   // distance from the bottom edge of the parent
constexpr int kFadeOutMs = 250;

} // namespace

MessageOverlay::MessageOverlay(QWidget *parent)
    : QLabel(parent)
{
    setObjectName(QStringLiteral("messageOverlay"));
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_StyledBackground);
    applyTheme();
    hide();

    m_opacity = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(m_opacity);
    m_fadeOut = new QPropertyAnimation(m_opacity, "opacity", this);
    m_fadeOut->setDuration(kFadeOutMs);
    m_fadeOut->setStartValue(1.0);
    m_fadeOut->setEndValue(0.0);
    connect(m_fadeOut, &QPropertyAnimation::finished, this, &QWidget::hide);

    m_hideTimer.setSingleShot(true);
    connect(&m_hideTimer, &QTimer::timeout, m_fadeOut, [this] { m_fadeOut->start(); });

    parent->installEventFilter(this);  // follow the parent's size
}

void MessageOverlay::showMessage(const QString &text, int durationMs)
{
    m_fadeOut->stop();
    m_opacity->setOpacity(1.0);
    setText(text);
    adjustSize();
    updatePosition();
    raise();
    show();
    if (durationMs > 0)
        m_hideTimer.start(durationMs);
    else
        m_hideTimer.stop();
}

void MessageOverlay::hideMessage()
{
    m_hideTimer.stop();
    m_fadeOut->stop();
    hide();
}

bool MessageOverlay::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget() && event->type() == QEvent::Resize)
        updatePosition();
    return QLabel::eventFilter(watched, event);
}

void MessageOverlay::updatePosition()
{
    const QWidget *parent = parentWidget();
    move((parent->width() - width()) / 2, parent->height() - height() - kBottomMargin);
}

void MessageOverlay::applyTheme()
{
    const Theme::Colors &colors = Theme::colors();
    setStyleSheet(QStringLiteral("#messageOverlay { background-color: %1; color: %2;"
                                 " font-size: 14pt; border-radius: 4px; padding: 8px 16px; }")
                      .arg(Theme::css(colors.overlayBackground, 200), Theme::css(colors.text)));
}
