#pragma once

#include <QImage>
#include <QWidget>

class MessageOverlay;
class QLabel;
class PanoramaView;
class QStackedWidget;
class FlatView;
class ZoomableView;

// The image area of the main window: the welcome page or the opened image in one of two
// projections (PanoramaView, FlatView), with the messages floating over it. It owns
// the opened image and hands it to the views; the actions of the main window act on the
// active view through it.
class ImageViewer : public QWidget
{
    Q_OBJECT

public:
    enum class Projection {
        Panorama360,  // "360°": interactive OpenGL view of an equirectangular panorama
        Flat,         // "flat": classic flat image view
    };

    explicit ImageViewer(QWidget *parent = nullptr);

    // Shows the image in the given projection, with the default view (direction, zoom,
    // rotation) as right after opening a file.
    void setImage(const QImage &image, Projection projection);
    // Back to the welcome page without an image.
    void clear();
    // Instead of an image that could not be loaded: the text in the middle of an otherwise
    // empty page, without an image like the welcome page.
    void showError(const QString &text);
    bool hasImage() const { return !m_image.isNull(); }

    // Without an image only remembered for the next one.
    Projection projection() const { return m_projection; }
    void setProjection(Projection projection);

    // Zoom of the active view (View > Zoom)
    void zoomIn();
    void zoomOut();
    void resetZoom();

    // Alignment grid, shown or hidden in both views at once.
    bool isGridVisible() const { return m_gridVisible; }
    void setGridVisible(bool visible);

    // Rotate left (step -1) / right (step +1): one level of automatic rotation in the 360
    // view, 90° of the displayed image in the flat view.
    void rotate(int step);

    // Short texts over the bottom of the image, e.g. "360° projection".
    MessageOverlay *messageOverlay() const { return m_messageOverlay; }

    // Applies the Theme colors (again, after a change of the system color scheme).
    void applyTheme();

signals:
    // "Open image..." on the welcome page
    void openRequested();
    // setImage(), clear() or showError(); setImage() emits projectionChanged() before.
    void imageChanged();
    // setProjection(), also when only remembered without an image
    void projectionChanged();

private:
    QWidget *createWelcomePage();
    QWidget *createErrorPage();
    void showPageWithoutImage(QWidget *page);
    void clearViews();
    void showImageInCurrentView();
    void showZoomMessage(double percent);
    void showRotationMessage(int level);
    ZoomableView *currentView() const;

    QStackedWidget *m_stack = nullptr;
    QWidget *m_welcomePage = nullptr;
    QWidget *m_errorPage = nullptr;
    QLabel *m_errorLabel = nullptr;
    PanoramaView *m_panoramaView = nullptr;
    FlatView *m_flatView = nullptr;
    MessageOverlay *m_messageOverlay = nullptr;

    // The opened image. The views get a shared (implicitly shared, not copied) QImage
    // only when they become active.
    QImage m_image;
    bool m_panoramaViewDirty = false;  // PanoramaView does not show m_image yet
    bool m_flatViewDirty = false;  // FlatView does not show m_image yet
    Projection m_projection = Projection::Panorama360;
    bool m_gridVisible = false;
};
