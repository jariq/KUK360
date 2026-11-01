#include "MainActions.h"

#include "FontAwesome.h"

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QWidget>

using FontAwesome::Icon;

namespace {

QAction *newAction(Icon icon, const QString &text, const QKeySequence &shortcut, QObject *parent)
{
    auto *action = new QAction(FontAwesome::icon(icon), text, parent);
    action->setShortcut(shortcut);
    return action;
}

} // namespace

MainActions::MainActions(QObject *parent)
    : QObject(parent)
{
    // File menu
    openAction = newAction(Icon::FolderOpen, tr("&Open..."), QKeySequence(Qt::CTRL | Qt::Key_O), this);
    reloadAction = newAction(Icon::Refresh, tr("&Reload"), QKeySequence(Qt::Key_F5), this);
    // Qt would display the arrow shortcuts as "Left"/"Right"; the text after the tab
    // replaces the displayed shortcut while the real shortcut stays Left/Right.
    // Panning in the 360 view uses W/A/S/D, so the arrows are free for navigation.
    previousAction = newAction(Icon::ChevronLeft, tr("&Previous\tLeft Arrow"), QKeySequence(Qt::Key_Left), this);
    nextAction = newAction(Icon::ChevronRight, tr("&Next\tRight Arrow"), QKeySequence(Qt::Key_Right), this);
    closeAction = newAction(Icon::WindowClose, tr("&Close"), QKeySequence(Qt::CTRL | Qt::Key_W), this);
    openFileLocationAction =
        newAction(Icon::Folder, tr("Show in &file manager..."), QKeySequence(Qt::CTRL | Qt::Key_L), this);
    // The platform's quit shortcut (Cmd+Q on macOS, Ctrl+Q on Linux), Alt+F4 on Windows.
    // On macOS the action moves to the application menu as "Quit KUK360".
    exitAction = newAction(Icon::PowerOff, tr("E&xit"), QKeySequence(), this);
    const QList<QKeySequence> quitKeys = QKeySequence::keyBindings(QKeySequence::Quit);
    exitAction->setShortcuts(quitKeys.isEmpty() ? QList{QKeySequence(Qt::ALT | Qt::Key_F4)} : quitKeys);

    // Edit menu
    copyAction = newAction(Icon::FilesO, tr("&Copy"), QKeySequence(Qt::CTRL | Qt::Key_C), this);
    deleteAction = newAction(Icon::TrashO, tr("&Delete"), QKeySequence(Qt::Key_Delete), this);
#if defined(Q_OS_MACOS)
    // Mac keyboards have no forward delete key; Cmd+Backspace moves to the trash in Finder.
    deleteAction->setShortcuts({QKeySequence(Qt::Key_Delete), QKeySequence(Qt::CTRL | Qt::Key_Backspace)});
#endif

    // View > Projection
    projectionAction = newAction(Icon::Globe, tr("&Switch projection"), QKeySequence(Qt::Key_F2), this);
    // Global setting: when locked, files open in the currently selected projection; when
    // unlocked (default), opening a file picks the projection by panorama detection.
    // The menus show no check mark, the state is shown by the icon of the panel button.
    lockProjectionAction = newAction(Icon::Lock, tr("&Lock/unlock projection"), QKeySequence(Qt::Key_F3), this);

    // View > Zoom, acting on the active view. Ctrl+= is an alternative to Ctrl++ for
    // keyboard layouts where "+" needs Shift (e.g. US).
    zoomInAction = newAction(Icon::SearchPlus, tr("Zoom &in"), QKeySequence(), this);
    zoomInAction->setShortcuts({QKeySequence(Qt::CTRL | Qt::Key_Plus), QKeySequence(Qt::CTRL | Qt::Key_Equal)});
    zoomOutAction = newAction(Icon::SearchMinus, tr("Zoom &out"), QKeySequence(Qt::CTRL | Qt::Key_Minus), this);
    // 100 % in the flat view, the default field of view in the 360 view.
    zoomResetAction = newAction(Icon::Arrows, tr("Zoom to &100%"), QKeySequence(Qt::CTRL | Qt::Key_0), this);

    // View > Rotation. Plain letters, as [ and ] need AltGr / Option on some layouts.
    rotateLeftAction = newAction(Icon::Undo, tr("Rotate &left"), QKeySequence(Qt::Key_L), this);
    rotateRightAction = newAction(Icon::Repeat, tr("Rotate &right"), QKeySequence(Qt::Key_R), this);

    // Alignment grid (e.g. to check the horizon), shown or hidden in both views at once.
    gridAction = newAction(Icon::Th, tr("Show/hide &grid"), QKeySequence(Qt::Key_F4), this);

    // F12 toggles full screen, Escape (not in a menu) leaves it.
    fullScreenAction =
        newAction(Icon::ArrowsAlt, tr("Enter/exit &full screen"), QKeySequence(Qt::Key_F12), this);
    exitFullScreenAction = new QAction(tr("Exit full screen"), this);
    exitFullScreenAction->setShortcut(QKeySequence(Qt::Key_Escape));
    exitFullScreenAction->setEnabled(false);

    // Help menu
    visitWebsiteAction = newAction(Icon::ExternalLink, tr("&Visit website..."), QKeySequence(), this);
    checkForUpdatesAction = newAction(Icon::Heartbeat, tr("&Check for updates..."), QKeySequence(), this);
    aboutAction = newAction(Icon::Copyright, tr("&About..."), QKeySequence(Qt::Key_F1), this);

    // A held key triggers the actions of the panel buttons and Reload only once (e.g. no fast
    // toggling of the projection or the grid, no series of loads of large panoramas).
    for (QAction *action : {projectionAction, lockProjectionAction, gridAction, reloadAction, previousAction,
                            fullScreenAction, nextAction, rotateLeftAction, rotateRightAction, deleteAction})
        action->setAutoRepeat(false);
}

void MainActions::addToMenuBar(QMenuBar *menuBar) const
{
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    fileMenu->addAction(openAction);
    fileMenu->addAction(reloadAction);
    fileMenu->addAction(previousAction);
    fileMenu->addAction(nextAction);
    fileMenu->addAction(closeAction);
    fileMenu->addSeparator();
    fileMenu->addAction(openFileLocationAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    QMenu *editMenu = menuBar->addMenu(tr("&Edit"));
    editMenu->addAction(copyAction);
    editMenu->addAction(deleteAction);

    QMenu *viewMenu = menuBar->addMenu(tr("&View"));
    QMenu *projectionMenu = viewMenu->addMenu(tr("&Projection"));
    projectionMenu->addAction(projectionAction);
    projectionMenu->addAction(lockProjectionAction);
    QMenu *zoomMenu = viewMenu->addMenu(tr("&Zoom"));
    zoomMenu->addAction(zoomInAction);
    zoomMenu->addAction(zoomOutAction);
    zoomMenu->addAction(zoomResetAction);
    QMenu *rotationMenu = viewMenu->addMenu(tr("&Rotation"));
    rotationMenu->addAction(rotateLeftAction);
    rotationMenu->addAction(rotateRightAction);
    viewMenu->addAction(gridAction);
    viewMenu->addAction(fullScreenAction);

    QMenu *helpMenu = menuBar->addMenu(tr("&Help"));
    helpMenu->addAction(visitWebsiteAction);
    helpMenu->addAction(checkForUpdatesAction);
    helpMenu->addAction(aboutAction);
}

void MainActions::addToContextMenu(QMenu *menu) const
{
    const auto addItem = [menu](QAction *action, const QString &text) {
        QAction *item = menu->addAction(action->icon(), text);
        item->setEnabled(action->isEnabled());
        connect(item, &QAction::triggered, action, &QAction::trigger);
    };
    addItem(openAction, tr("Open image..."));
    addItem(openFileLocationAction, tr("Show in file manager..."));
    addItem(reloadAction, tr("Reload image"));
    addItem(closeAction, tr("Close image"));
    menu->addSeparator();
    addItem(previousAction, tr("Previous image"));
    addItem(nextAction, tr("Next image"));
    menu->addSeparator();
    addItem(copyAction, tr("Copy"));
    addItem(deleteAction, tr("Delete"));
    menu->addSeparator();
    addItem(projectionAction, tr("Switch projection"));
    addItem(lockProjectionAction, tr("Lock/unlock projection"));
    menu->addSeparator();
    addItem(zoomInAction, tr("Zoom in"));
    addItem(zoomOutAction, tr("Zoom out"));
    addItem(zoomResetAction, tr("Zoom to 100%"));
    menu->addSeparator();
    addItem(rotateLeftAction, tr("Rotate left"));
    addItem(rotateRightAction, tr("Rotate right"));
    menu->addSeparator();
    addItem(gridAction, tr("Show/hide grid"));
    addItem(fullScreenAction, tr("Enter/exit full screen"));
    menu->addSeparator();
    addItem(exitAction, tr("Exit"));
}

void MainActions::addShortcutsTo(QWidget *window) const
{
    window->addActions({openAction, previousAction, nextAction, reloadAction, closeAction,
                        openFileLocationAction, exitAction, copyAction, deleteAction,
                        projectionAction, lockProjectionAction,
                        zoomInAction, zoomOutAction, zoomResetAction,
                        rotateLeftAction, rotateRightAction, gridAction, fullScreenAction,
                        exitFullScreenAction, aboutAction});
}

// Previous/Next depend on the folder as well and are enabled by the main window.
void MainActions::setImageActionsEnabled(bool hasFile, bool hasImage)
{
    // Also for a file that could not be loaded: its error page stands in for the image.
    for (QAction *action : {reloadAction, closeAction, openFileLocationAction, copyAction, deleteAction,
                            fullScreenAction})
        action->setEnabled(hasFile);
    for (QAction *action : {projectionAction, zoomInAction, zoomOutAction, zoomResetAction,
                            rotateLeftAction, rotateRightAction, gridAction})
        action->setEnabled(hasImage);
}
