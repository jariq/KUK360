#pragma once

#include <QLabel>
#include <QTimer>

class QGraphicsOpacityEffect;
class QPropertyAnimation;

// Short text message floating over the bottom center of its parent (the image area),
// used to inform the user about events, e.g. "360° projection". It hides itself after
// the given time with a short fade-out, a new message replaces the current one, and it
// is transparent for the mouse so it never gets in the way of the image view.
class MessageOverlay : public QLabel
{
    Q_OBJECT

public:
    explicit MessageOverlay(QWidget *parent);

    // A durationMs <= 0 keeps the message shown until the next one or hideMessage().
    void showMessage(const QString &text, int durationMs = 1000);
    void hideMessage();
    // Applies the Theme colors (again, after a change of the system color scheme).
    void applyTheme();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void updatePosition();

    QTimer m_hideTimer;
    QGraphicsOpacityEffect *m_opacity = nullptr;
    QPropertyAnimation *m_fadeOut = nullptr;
};
