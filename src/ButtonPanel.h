#pragma once

#include <QList>
#include <QWidget>

class QAction;
class QToolButton;

// Panel with the view buttons at the bottom of the main window. It only lays out and
// styles the buttons; what they do is connected by the main window (button()).
class ButtonPanel : public QWidget
{
    Q_OBJECT

public:
    // The buttons from left to right
    enum class Button {
        Projection,  // its icon is set by the main window (the current projection)
        LockProjection,  // its icon is set by the main window (closed lock when locked)
        Grid,
        Previous,
        FullScreen,
        Next,
        RotateLeft,
        RotateRight,
        Delete,
    };

    explicit ButtonPanel(QWidget *parent = nullptr);

    QToolButton *button(Button button) const { return m_buttons.at(int(button)); }
    // A click on the button triggers the action; the button is enabled exactly when the
    // action is.
    void setAction(Button button, QAction *action);

    // The panel takes less of a low window (small screen or high display scaling).
    void adjustToWindowHeight(int windowHeight);

    // Applies the Theme colors (again, after a change of the system color scheme).
    void applyTheme();

private:
    QList<QToolButton *> m_buttons;  // in the order of Button
};
