#pragma once

#include "ZoomableView.h"

#include <QGraphicsView>
#include <QImage>

class QGraphicsPixmapItem;

// Classic flat image view. The image is initially fitted into the widget (never
// upscaled) and centered; it can be zoomed up to kMaxScale (mouse wheel towards the
// cursor, pinch, menu) and panned by dragging when zoomed in.
class FlatView : public QGraphicsView, public ZoomableView
{
    Q_OBJECT

public:
    explicit FlatView(QWidget *parent = nullptr);

    void setImage(const QImage &image);
    void clear();

    // Rotates the displayed image by a multiple of 90° (negative = left); the file on
    // disk is not changed. A new image starts unrotated. (QGraphicsView::rotate() would turn
    // the whole view instead.)
    void rotateImage(int degrees);

    // Zoom by kZoomStep towards / away from the viewport center; resetZoom() zooms to
    // 100 % (one image pixel per device pixel, also with display scaling), within the
    // fit .. kMaxScale limits.
    void zoomIn() override;
    void zoomOut() override;
    void resetZoom() override;

    // Alignment grid over the image (e.g. to check the horizon): lines every kGridSpacing
    // screen pixels independent of zoom, alternating two colors. Stays on for new images.
    bool isGridVisible() const { return m_gridVisible; }
    void setGridVisible(bool visible);

    // Applies the Theme background color (again, after a change of the system color scheme).
    void applyTheme();

signals:
    // The user zoomed (wheel, keys, pinch, menu), also when a limit kept the scale. 100 %
    // is one image pixel per device pixel.
    void zoomChanged(double percent);

protected:
    void drawForeground(QPainter *painter, const QRectF &rect) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    bool viewportEvent(QEvent *event) override;
    bool event(QEvent *event) override;

private:
    static constexpr double kMaxScale = 4.0;   // 400 % (of the device pixels)
    static constexpr double kZoomStep = 1.25;  // per wheel notch / key press
    static constexpr int kGridSpacing = 40;    // pixels

    void fitImage();
    double minScale() const;
    double maxScale() const;
    // View scale at which one image pixel covers one device pixel (100 %)
    double actualSizeScale() const;
    double zoomPercent() const { return 100.0 * m_scale / actualSizeScale(); }
    // Applies the scale after a size or device pixel ratio change: fitted again or zoomedScale
    void updateScale(double zoomedScale);
    // Sets the scale so that the scene point under viewport position "anchor" stays
    // there (the transformation anchors of QGraphicsView fail while the image grows
    // beyond the viewport).
    void setScale(double scale, const QPointF &anchor);
    void setScale(double scale) { setScale(scale, QRectF(viewport()->rect()).center()); }
    void zoomBy(double factor, const QPointF &anchor);

    QGraphicsScene m_scene;
    QGraphicsPixmapItem *m_item = nullptr;
    double m_scale = 1.0;  // of the view, in logical pixels
    bool m_fitted = true;  // zoomed out completely; follows window size changes
    double m_devicePixelRatio = 1.0;  // of the last applied scale (updateScale())
    bool m_gridVisible = false;
};
