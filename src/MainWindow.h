#pragma once

#include "ImageFolder.h"
#include "ImageViewer.h"  // ImageViewer::Projection

#include <QList>
#include <QMainWindow>
#include <QUrl>

#include <functional>

class ButtonPanel;
class MainActions;

// Main window: menu bar on top, welcome page / image view (ImageViewer) in the middle,
// panel with view buttons at the bottom.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    void openImage(const QString &filePath);
    // Once the event loop runs and the application is active, opens the file from the command
    // line (if not empty) and then asks whether to check for updates when the last check is
    // old (see UpdateChecker.h).
    void startWhenActive(const QString &filePath);
    // Size, position and maximized state from the last session (Settings), or 80 % of the
    // screen. Called before show().
    void restoreWindowGeometry();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void openImageDialog();
    void toggleProjection();
    void openPreviousImage();
    void openNextImage();
    void closeImage();
    void openFileLocation();
    void copyFileToClipboard();
    void reloadImage();
    void setFullScreen(bool fullScreen);
    void deleteImage();

private:
    void runWhenActive(const std::function<void()> &function);
    void saveWindowGeometry();
    void connectActions();
    void connectButtonPanel();
    void updateForImage();
    void toggleProjectionLock();
    void updateProjectionButton();
    void updateLockProjectionButton();
    void updateImageActions();
    void updateNavigationActions();
    void showLoadingMessage();
    ImageViewer::Projection projectionFor(const QString &filePath) const;
    void openDroppedFiles(const QList<QUrl> &urls);
    void showContextMenu(const QPoint &pos);
    void showMessage(const QString &text, int durationMs = 1000);
    void applyTheme();

    MainActions *m_actions = nullptr;
    ImageViewer *m_viewer = nullptr;
    ButtonPanel *m_buttonPanel = nullptr;

    bool m_wasMaximized = false;         // window state to restore after full screen
    bool m_projectionLocked = false;     // opened files keep the current projection (F3)

    ImageFolder m_imageFolder;  // current file and Previous/Next navigation
};
