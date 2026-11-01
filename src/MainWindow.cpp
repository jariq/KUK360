#include "MainWindow.h"

#include "AboutDialog.h"
#include "ButtonPanel.h"
#include "FileLocation.h"
#include "FontAwesome.h"
#include "MainActions.h"
#include "MessageBoxes.h"
#include "MessageOverlay.h"
#include "PanoramaDetector.h"
#include "Settings.h"
#include "UpdateChecker.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileOpenEvent>
#include <QGuiApplication>
#include <QImageReader>
#include <QMenu>
#include <QMenuBar>
#include <QMimeData>
#include <QScreen>
#include <QStandardPaths>
#include <QStyleHints>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

using FontAwesome::Icon;

// Reads the image file, turned as its EXIF orientation says. On failure returns a null
// image and fills errorString.
static QImage loadImage(const QString &filePath, QString *errorString)
{
    QImageReader reader(filePath);
    reader.setAutoTransform(true);
    const QImage image = reader.read();
    if (image.isNull())
        *errorString = reader.errorString();
    return image;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setAcceptDrops(true);  // dropped image files open like File > Open
    setMinimumSize(750, 450);  // as the original KUK360
#if defined(Q_OS_MACOS)
    // The green title bar button maximizes the window (zoom) instead of the system full
    // screen, which would keep the menu and the button panel; F12 has its own full screen
    // mode. Qt enables the system full screen unless the flags are customized without
    // Qt::WindowFullscreenButtonHint.
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint
                   | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
#endif

    m_viewer = new ImageViewer;
    connect(m_viewer, &ImageViewer::openRequested, this, &MainWindow::openImageDialog);
    // Right click on the image (the views ignore context menu events, so they reach m_viewer)
    m_viewer->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_viewer, &QWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);
    // macOS delivers "Open with" / Finder file opens as QFileOpenEvent to the application.
    qApp->installEventFilter(this);

    m_actions = new MainActions(this);
    m_actions->addToMenuBar(menuBar());
    m_actions->addShortcutsTo(this);
    connectActions();

    auto *central = new QWidget;
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_viewer, 1);
    m_buttonPanel = new ButtonPanel;
    connectButtonPanel();
    layout->addWidget(m_buttonPanel);
    setCentralWidget(central);

    // Actions, panel buttons and the window title follow the image, its projection and the
    // projection lock.
    connect(m_viewer, &ImageViewer::imageChanged, this, &MainWindow::updateForImage);
    connect(m_viewer, &ImageViewer::projectionChanged, this, &MainWindow::updateProjectionButton);
    updateLockProjectionButton();
    updateForImage();

    // Colors of the own UI elements follow the light / dark color scheme of the system,
    // also when it changes while KUK360 runs.
    applyTheme();
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &MainWindow::applyTheme);
}

// What the actions do; the panel buttons trigger their actions (connectButtonPanel()).
void MainWindow::connectActions()
{
    connect(m_actions->openAction, &QAction::triggered, this, &MainWindow::openImageDialog);
    connect(m_actions->reloadAction, &QAction::triggered, this, &MainWindow::reloadImage);
    connect(m_actions->previousAction, &QAction::triggered, this, &MainWindow::openPreviousImage);
    connect(m_actions->nextAction, &QAction::triggered, this, &MainWindow::openNextImage);
    connect(m_actions->closeAction, &QAction::triggered, this, &MainWindow::closeImage);
    connect(m_actions->openFileLocationAction, &QAction::triggered, this, &MainWindow::openFileLocation);
    connect(m_actions->exitAction, &QAction::triggered, this, &MainWindow::close);
    connect(m_actions->copyAction, &QAction::triggered, this, &MainWindow::copyFileToClipboard);
    connect(m_actions->deleteAction, &QAction::triggered, this, &MainWindow::deleteImage);

    connect(m_actions->projectionAction, &QAction::triggered, this, &MainWindow::toggleProjection);
    connect(m_actions->lockProjectionAction, &QAction::triggered, this, &MainWindow::toggleProjectionLock);
    connect(m_actions->zoomInAction, &QAction::triggered, m_viewer, &ImageViewer::zoomIn);
    connect(m_actions->zoomOutAction, &QAction::triggered, m_viewer, &ImageViewer::zoomOut);
    connect(m_actions->zoomResetAction, &QAction::triggered, m_viewer, &ImageViewer::resetZoom);
    connect(m_actions->rotateLeftAction, &QAction::triggered, this, [this] { m_viewer->rotate(-1); });
    connect(m_actions->rotateRightAction, &QAction::triggered, this, [this] { m_viewer->rotate(+1); });
    connect(m_actions->gridAction, &QAction::triggered, this,
            [this] { m_viewer->setGridVisible(!m_viewer->isGridVisible()); });
    connect(m_actions->fullScreenAction, &QAction::triggered, this, [this] { setFullScreen(!isFullScreen()); });
    connect(m_actions->exitFullScreenAction, &QAction::triggered, this, [this] { setFullScreen(false); });

    connect(m_actions->visitWebsiteAction, &QAction::triggered, this,
            [] { QDesktopServices::openUrl(QUrl(QStringLiteral("https://www.kuk360.com/"))); });
    connect(m_actions->checkForUpdatesAction, &QAction::triggered, this, [this] { checkForUpdates(this); });
    connect(m_actions->aboutAction, &QAction::triggered, this, [this] {
        AboutDialog dialog(this);
        dialog.exec();
    });
}

// Applies the Theme colors to the own UI elements; called again when the system color
// scheme changes. Icons take their colors from Theme when painted, so a repaint suffices.
void MainWindow::applyTheme()
{
    m_buttonPanel->applyTheme();
    m_viewer->applyTheme();
    update();  // repaints the children too, including the icons
}

// The panel buttons trigger the actions of the menu and are enabled with them.
void MainWindow::connectButtonPanel()
{
    using Button = ButtonPanel::Button;
    m_buttonPanel->setAction(Button::Projection, m_actions->projectionAction);
    m_buttonPanel->setAction(Button::LockProjection, m_actions->lockProjectionAction);
    m_buttonPanel->setAction(Button::Grid, m_actions->gridAction);
    m_buttonPanel->setAction(Button::Previous, m_actions->previousAction);
    m_buttonPanel->setAction(Button::FullScreen, m_actions->fullScreenAction);
    m_buttonPanel->setAction(Button::Next, m_actions->nextAction);
    m_buttonPanel->setAction(Button::RotateLeft, m_actions->rotateLeftAction);
    m_buttonPanel->setAction(Button::RotateRight, m_actions->rotateRightAction);
    m_buttonPanel->setAction(Button::Delete, m_actions->deleteAction);
}

// Files dropped onto the window open like File > Open. Any file drag is accepted, so that
// an unsupported file or several files get an error message instead of a "no entry" cursor.
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        // Always copy: with a "move" action the source (Explorer) would delete the file.
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;
    event->setDropAction(Qt::CopyAction);
    event->accept();
    // The drag source (e.g. Explorer) waits until the drop returns, so message boxes and
    // loading run afterwards and do not block it.
    QTimer::singleShot(0, this, [this, urls] { openDroppedFiles(urls); });
}

void MainWindow::openDroppedFiles(const QList<QUrl> &urls)
{
    activateWindow();  // the drag came from another application
    if (urls.size() > 1) {
        MessageBoxes::showWarning(this, tr("Please drop only one image file."));
        return;
    }
    const QString filePath = urls.first().toLocalFile();
    if (!ImageFolder::isSupportedFile(filePath)) {
        const QString name = filePath.isEmpty() ? urls.first().toDisplayString() : QFileInfo(filePath).fileName();
        MessageBoxes::showWarning(this, tr("Unsupported file type:\n%1\n\nSupported file types: %2")
                                            .arg(name, ImageFolder::nameFilters().join(QStringLiteral(", "))));
        return;
    }
    openImage(filePath);
}

// Context menu of the image, with items of the actions (MainActions::addToContextMenu()).
void MainWindow::showContextMenu(const QPoint &pos)
{
    if (m_imageFolder.currentFile().isEmpty())
        return;

    QMenu menu(this);
    m_actions->addToContextMenu(&menu);
    menu.exec(m_viewer->mapToGlobal(pos));
}

void MainWindow::openImageDialog()
{
    const QString filter = tr("Images (%1);;All files (*)").arg(ImageFolder::nameFilters().join(QLatin1Char(' ')));
    // The dialog opens in the directory of the last opened image, also after a restart. Before
    // the first image, or when that directory is gone (e.g. a removed drive), it opens in the
    // Pictures folder.
    QString directory = Settings::lastDirectory();
    if (directory.isEmpty() || !QFileInfo(directory).isDir())
        directory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (!QFileInfo(directory).isDir())
        directory = QDir::homePath();
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Open image"), directory, filter);
    if (filePath.isEmpty())
        return;
    openImage(filePath);
}

void MainWindow::openPreviousImage()
{
    const QString filePath = m_imageFolder.previousFile();
    if (!filePath.isEmpty())
        openImage(filePath);
}

void MainWindow::openNextImage()
{
    const QString filePath = m_imageFolder.nextFile();
    if (!filePath.isEmpty())
        openImage(filePath);
}

// The file becomes the current one even if it cannot be loaded (except a directory): the
// error page then stands in for the image, so Previous, Next, Reload and Delete work as
// with an image.
void MainWindow::openImage(const QString &filePath)
{
    const QFileInfo fileInfo(filePath);
    const QString absolutePath = fileInfo.absoluteFilePath();
    Settings::setLastDirectory(fileInfo.absolutePath());

    // Zero width spaces let a long path on the error page wrap after its separators.
    const QString separator = QDir::toNativeSeparators(QStringLiteral("/"));
    const QString wrappablePath =
        QDir::toNativeSeparators(absolutePath).replace(separator, separator + QChar(0x200B));

    // A directory (command line, macOS QFileOpenEvent) does not become the current file, so
    // no file action applies to it (Delete would move the whole directory to the trash).
    if (fileInfo.isDir()) {
        m_imageFolder.setCurrentFile(QString());  // before updateForImage()
        setFullScreen(false);                     // F12 is disabled without a file
        m_viewer->showError(tr("Unable to load image\n%1\n\n%2")
                                .arg(wrappablePath, tr("It is a directory, not an image file.")));
        return;
    }

    showLoadingMessage();
    QString error;
    const QImage image = loadImage(absolutePath, &error);
    m_imageFolder.setCurrentFile(absolutePath);  // before updateForImage()
    if (image.isNull()) {
        m_viewer->messageOverlay()->hideMessage();
        m_viewer->showError(tr("Unable to load image\n%1\n\n%2").arg(wrappablePath, error));
        return;
    }
    m_viewer->setImage(image, projectionFor(absolutePath));
}

// Loading blocks the event loop (huge panoramas take seconds), so the message is painted
// now. It stays until setProjection() replaces it once the image is shown.
// processEvents() repaints the window (e.g. the area under a just closed file dialog),
// repaint() is still needed for the message itself to appear right after the dialog.
void MainWindow::showLoadingMessage()
{
    MessageOverlay *messageOverlay = m_viewer->messageOverlay();
    messageOverlay->showMessage(tr("Loading..."), 0);
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    messageOverlay->repaint();
}

// With an unlocked projection, 360° panoramas open in the 360 view and everything else in
// the flat view; a locked projection stays. The user can still switch
// with the projection button.
ImageViewer::Projection MainWindow::projectionFor(const QString &filePath) const
{
    using Projection = ImageViewer::Projection;
    if (m_projectionLocked)
        return m_viewer->projection();
    return isEquirectangularPanorama(filePath) ? Projection::Panorama360 : Projection::Flat;
}

// Runs the function once the event loop runs and the application is active. The window is
// then already visible, e.g. with the "Loading..." message while a large image loads. On
// macOS the application finishes launching only when it becomes active; processing events
// earlier (openImage() does it for the message) would hold back all keyboard and mouse
// input until another event.
void MainWindow::runWhenActive(const std::function<void()> &function)
{
    QTimer::singleShot(0, this, [this, function] {
        if (QGuiApplication::applicationState() == Qt::ApplicationActive) {
            function();
            return;
        }
        auto *waiting = new QObject(this);  // deleting it ends the connection
        connect(qApp, &QGuiApplication::applicationStateChanged, waiting,
                [function, waiting](Qt::ApplicationState state) {
                    if (state != Qt::ApplicationActive)
                        return;
                    delete waiting;
                    function();
                });
    });
}

// Both steps run in one function: as separate queued functions, the processEvents() in
// openImage() would run the reminder nested in it, before the image is loaded. The question
// comes after the image, so it does not hold it back, and not in full screen, where it would
// interrupt viewing; the next start asks then.
void MainWindow::startWhenActive(const QString &filePath)
{
    runWhenActive([this, filePath] {
        if (!filePath.isEmpty())
            openImage(filePath);
        if (!isFullScreen())
            remindToCheckForUpdates(this);
    });
}

// Returns to the state right after start: welcome page, no image, disabled buttons.
// Only the directory for the next file dialog is kept.
void MainWindow::closeImage()
{
    m_imageFolder.setCurrentFile(QString());
    setFullScreen(false);
    m_viewer->clear();
}

void MainWindow::openFileLocation()
{
    if (!m_imageFolder.currentFile().isEmpty())
        showInFileManager(m_imageFolder.currentFile());
}

// Puts the file itself (not the pixels) on the clipboard, so pasting in a file manager
// creates a copy of it.
void MainWindow::copyFileToClipboard()
{
    const QString filePath = m_imageFolder.currentFile();
    if (filePath.isEmpty())
        return;

    const QUrl url = QUrl::fromLocalFile(filePath);
    auto *mimeData = new QMimeData;
    mimeData->setUrls({url});  // Windows Explorer (CF_HDROP), macOS Finder, KDE Dolphin
    // GNOME Files and similar file managers paste only from this format.
    mimeData->setData(QStringLiteral("x-special/gnome-copied-files"), "copy\n" + url.toEncoded());
    QGuiApplication::clipboard()->setMimeData(mimeData);

    showMessage(tr("File %1 was copied to the clipboard").arg(QFileInfo(filePath).fileName()));
}

// After an image was opened or closed, or a file could not be loaded (ImageViewer::imageChanged).
void MainWindow::updateForImage()
{
    updateImageActions();
    updateProjectionButton();

    const QString appName = QCoreApplication::applicationName();
    const QString filePath = m_imageFolder.currentFile();
    if (!filePath.isEmpty())
        setWindowTitle(QStringLiteral("%1 - %2").arg(appName, QDir::toNativeSeparators(filePath)));
    else
        setWindowTitle(appName);
}

// Actions on the file are enabled with a current file (also one that could not be
// loaded), actions on the view only with a shown image. The panel buttons follow their
// actions (ButtonPanel::setAction()).
void MainWindow::updateImageActions()
{
    m_actions->setImageActionsEnabled(!m_imageFolder.currentFile().isEmpty(), m_viewer->hasImage());
    updateNavigationActions();
}

// Previous/Next are enabled only when the folder of the current file contains another
// supported file. Also checked when the window gets activated, as files may have been
// added or removed meanwhile.
void MainWindow::updateNavigationActions()
{
    if (!m_actions)  // not created yet
        return;
    const bool canNavigate = m_imageFolder.hasOtherFiles();
    m_actions->previousAction->setEnabled(canNavigate);
    m_actions->nextAction->setEnabled(canNavigate);
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::ActivationChange && isActiveWindow()) {
        m_imageFolder.refresh();
        updateNavigationActions();
    }
}

void MainWindow::toggleProjection()
{
    using Projection = ImageViewer::Projection;
    m_viewer->setProjection(m_viewer->projection() == Projection::Panorama360 ? Projection::Flat
                                                                              : Projection::Panorama360);
}

void MainWindow::toggleProjectionLock()
{
    m_projectionLocked = !m_projectionLocked;
    updateLockProjectionButton();
    showMessage(m_projectionLocked ? tr("Projection locked") : tr("Projection unlocked"));
}

void MainWindow::updateProjectionButton()
{
    // The button shows the current projection: the globe in the 360 view, the (flat) map in
    // the flat view. Without an image the disabled button shows the remembered projection,
    // in which the next file opens when the projection is locked.
    const bool is360 = m_viewer->projection() == ImageViewer::Projection::Panorama360;
    m_buttonPanel->button(ButtonPanel::Button::Projection)->setIcon(FontAwesome::icon(is360 ? Icon::Globe : Icon::Map));
}

// Like the projection button, the button shows the current state: a closed lock while the
// projection is locked, an open lock while the projection of each opened file is detected.
void MainWindow::updateLockProjectionButton()
{
    m_buttonPanel->button(ButtonPanel::Button::LockProjection)->setIcon(FontAwesome::icon(m_projectionLocked ? Icon::Lock : Icon::Unlock));
}

void MainWindow::showMessage(const QString &text, int durationMs)
{
    m_viewer->messageOverlay()->showMessage(text, durationMs);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FileOpen && watched == qApp) {
        const QString file = static_cast<QFileOpenEvent *>(event)->file();
        if (!file.isEmpty()) {
            openImage(file);
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

// Restores the size, position and maximized state saved when the window last closed. The
// first time, or when the saved geometry is not usable, the window gets 80 % of the screen.
// A saved position on a screen that is no longer connected is moved onto an available one
// by restoreGeometry(). On Wayland the position is up to the compositor.
void MainWindow::restoreWindowGeometry()
{
    if (restoreGeometry(Settings::windowGeometry()))
        return;
    if (const QScreen *screen = QGuiApplication::primaryScreen())
        resize(screen->availableSize() * 0.8);
    else
        resize(1280, 720);
}

void MainWindow::saveWindowGeometry()
{
    Settings::setWindowGeometry(saveGeometry());
}

// Full screen is never saved, the next start would have no menu and panel. The geometry
// from before full screen was saved by setFullScreen().
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!isFullScreen())
        saveWindowGeometry();
    QMainWindow::closeEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    m_buttonPanel->adjustToWindowHeight(height());
    QMainWindow::resizeEvent(event);
}

// Loads the current file again from disk (picking up external changes) and resets
// everything that depends on the image (projection, view direction, zoom) to the state
// right after opening it. If loading fails now, the error page replaces the image.
void MainWindow::reloadImage()
{
    if (!m_imageFolder.currentFile().isEmpty())
        openImage(m_imageFolder.currentFile());
}

// Full screen shows only the image view: the menu bar and the button panel are hidden and
// restored afterwards, as is a maximized window.
void MainWindow::setFullScreen(bool fullScreen)
{
    if (fullScreen == isFullScreen())
        return;

    if (fullScreen) {
        saveWindowGeometry();  // closing in full screen keeps this one
        m_wasMaximized = isMaximized();
        menuBar()->hide();
        m_buttonPanel->hide();
        showFullScreen();
    } else {
        menuBar()->show();
        m_buttonPanel->show();
        if (m_wasMaximized)
            showMaximized();
        else
            showNormal();
    }
    m_actions->exitFullScreenAction->setEnabled(fullScreen);
}

// Name of the system trash in messages: the Recycle Bin on Windows, the Trash on macOS and
// on Linux (freedesktop.org).
static QString trashName()
{
#if defined(Q_OS_WIN)
    return MainWindow::tr("Recycle Bin");
#else
    return MainWindow::tr("Trash");
#endif
}

// Delete: after confirmation moves the current file to the trash and opens the next image
// in the directory (or returns to the welcome page if there is none).
void MainWindow::deleteImage()
{
    const QString filePath = m_imageFolder.currentFile();
    if (filePath.isEmpty())
        return;

    const QString fileName = QFileInfo(filePath).fileName();
    const QString question = tr("Do you really want to move file %1 to the %2?").arg(fileName, trashName());
    if (!MessageBoxes::askYesNo(this, question))
        return;
    // Another file may have been opened while the question was shown (macOS delivers
    // QFileOpenEvent also to a modal dialog); never delete a file the user did not confirm.
    if (m_imageFolder.currentFile() != filePath)
        return;

    QString error;
    if (!m_imageFolder.moveCurrentFileToTrash(&error)) {
        MessageBoxes::showWarning(this,
                                  tr("Unable to move file %1 to the %2:\n%3").arg(fileName, trashName(), error));
        return;
    }

    const QString nextFile = m_imageFolder.nextFile();
    if (nextFile.isEmpty())
        closeImage();
    else
        openImage(nextFile);
    showMessage(tr("File %1 was moved to the %2").arg(fileName, trashName()), 2000);
}
