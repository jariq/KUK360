#pragma once

#include <QObject>

class QAction;
class QMenu;
class QMenuBar;
class QWidget;

// The actions of the main window with their icons, texts and key shortcuts, and the
// menus built from them. What the actions do is connected by the main window.
class MainActions : public QObject
{
    Q_OBJECT

public:
    explicit MainActions(QObject *parent = nullptr);

    // Fills the menu bar: File, Edit, View, Help.
    void addToMenuBar(QMenuBar *menuBar) const;
    // Fills the context menu of the image. Its items trigger the actions and mirror their
    // state; they have their own texts and show no shortcuts.
    void addToContextMenu(QMenu *menu) const;
    // Actions added to the window itself keep their shortcuts working in full screen,
    // where the menu bar is hidden.
    void addShortcutsTo(QWidget *window) const;

    // Actions on the opened file (Reload, Close, Copy, Delete...) are enabled with a file,
    // even one that could not be loaded; actions on the view only with a shown image.
    void setImageActionsEnabled(bool hasFile, bool hasImage);

    // File menu
    QAction *openAction = nullptr;
    QAction *reloadAction = nullptr;
    QAction *previousAction = nullptr;
    QAction *nextAction = nullptr;
    QAction *closeAction = nullptr;
    QAction *openFileLocationAction = nullptr;
    QAction *exitAction = nullptr;

    // Edit menu
    QAction *copyAction = nullptr;
    QAction *deleteAction = nullptr;

    // View menu
    QAction *projectionAction = nullptr;
    QAction *lockProjectionAction = nullptr;  // F3, global setting (MainWindow)
    QAction *zoomInAction = nullptr;
    QAction *zoomOutAction = nullptr;
    QAction *zoomResetAction = nullptr;
    QAction *rotateLeftAction = nullptr;
    QAction *rotateRightAction = nullptr;
    QAction *gridAction = nullptr;            // F4
    QAction *fullScreenAction = nullptr;      // F12
    QAction *exitFullScreenAction = nullptr;  // Escape, not in a menu, enabled only in full screen

    // Help menu
    QAction *visitWebsiteAction = nullptr;
    QAction *checkForUpdatesAction = nullptr;
    QAction *aboutAction = nullptr;  // F1
};
