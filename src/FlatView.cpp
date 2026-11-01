#include "FlatView.h"

#include "Theme.h"

#include <QGestureEvent>
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QPixmap>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

FlatView::FlatView(QWidget *parent)
    : QGraphicsView(parent)
{
    setScene(&m_scene);
    // QGraphicsView accepts drops for its scene and swallows them; dropped files belong to
    // MainWindow, which opens them.
    setAcceptDrops(false);
    // Pinch zoom on touch screens (viewportEvent()); one finger pans through the mouse events
    // Qt synthesizes from the touch input, which the scene does not accept.
    viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    viewport()->grabGesture(Qt::PinchGesture);
    setFrameShape(QFrame::NoFrame);
    applyTheme();
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::ScrollHandDrag);  // panning when zoomed in
    // Repaint everything when panning instead of scrolling the pixels, which would move
    // the alignment grid along with the image.
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    m_item = m_scene.addPixmap(QPixmap());
    m_item->setTransformationMode(Qt::SmoothTransformation);
}

void FlatView::setImage(const QImage &image)
{
    m_item->setPixmap(QPixmap::fromImage(image));
    m_item->setTransformOriginPoint(m_item->boundingRect().center());
    m_item->setRotation(0.0);
    fitImage();
}

void FlatView::clear()
{
    m_item->setPixmap(QPixmap());
    m_item->setRotation(0.0);
    m_scene.setSceneRect(QRectF());
}

// Rotates only the displayed image (the file is not touched) and fits it into the
// window again, as its proportions may have changed.
void FlatView::rotateImage(int degrees)
{
    if (m_item->pixmap().isNull())
        return;
    m_item->setRotation(std::fmod(m_item->rotation() + degrees + 360.0, 360.0));
    fitImage();
}

void FlatView::setGridVisible(bool visible)
{
    m_gridVisible = visible;
    viewport()->update();
}

void FlatView::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawForeground(painter, rect);
    if (!m_gridVisible || m_item->pixmap().isNull())
        return;

    // Draw in viewport pixels, limited to the visible part of the image.
    const QRect area = mapFromScene(m_item->sceneBoundingRect()).boundingRect() & viewport()->rect();
    if (area.isEmpty())
        return;
    static const QColor colors[] = {QColor(0x80, 0xff, 0x00), QColor(0x53, 0xed, 0xf3)};

    painter->save();
    painter->resetTransform();
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setClipRect(area);
    // Lines at multiples of kGridSpacing from the viewport origin, colors alternating.
    for (int i = area.left() / kGridSpacing; i * kGridSpacing <= area.right(); ++i) {
        painter->setPen(colors[i % 2]);
        painter->drawLine(i * kGridSpacing, area.top(), i * kGridSpacing, area.bottom());
    }
    for (int i = area.top() / kGridSpacing; i * kGridSpacing <= area.bottom(); ++i) {
        painter->setPen(colors[i % 2]);
        painter->drawLine(area.left(), i * kGridSpacing, area.right(), i * kGridSpacing);
    }
    painter->restore();
}

void FlatView::fitImage()
{
    m_scene.setSceneRect(m_item->sceneBoundingRect());
    m_fitted = true;
    updateScale(m_scale);
}

// The view scale works in logical pixels; with display scaling (e.g. 150 %) one image pixel
// covers one device pixel at 1 / devicePixelRatio. 100 % and the limits refer to that.
double FlatView::actualSizeScale() const
{
    return 1.0 / devicePixelRatioF();
}

// Fit into the viewport, but do not enlarge small images beyond 100 %.
double FlatView::minScale() const
{
    const QRectF bounds = m_item->sceneBoundingRect();  // includes the rotation
    if (bounds.isEmpty())
        return actualSizeScale();
    const QSizeF viewport = this->viewport()->size();
    return std::min({viewport.width() / bounds.width(), viewport.height() / bounds.height(), actualSizeScale()});
}

double FlatView::maxScale() const
{
    return std::max(kMaxScale * actualSizeScale(), minScale());
}

void FlatView::zoomIn()
{
    zoomBy(kZoomStep, QRectF(viewport()->rect()).center());
}

void FlatView::zoomOut()
{
    zoomBy(1.0 / kZoomStep, QRectF(viewport()->rect()).center());
}

void FlatView::resetZoom()
{
    zoomBy(actualSizeScale() / m_scale, QRectF(viewport()->rect()).center());
}

void FlatView::setScale(double scale, const QPointF &anchor)
{
    const QPointF scenePoint = mapToScene(anchor.toPoint());
    m_scale = std::clamp(scale, minScale(), maxScale());
    setTransform(QTransform::fromScale(m_scale, m_scale));

    // Put scenePoint back under the anchor (centerOn clamps to the image).
    const QPointF viewportCenter = QRectF(viewport()->rect()).center();
    centerOn(scenePoint - (anchor - viewportCenter) / m_scale);
}

void FlatView::zoomBy(double factor, const QPointF &anchor)
{
    if (m_item->pixmap().isNull())
        return;
    setScale(m_scale * factor, anchor);
    m_fitted = m_scale <= minScale();
    emit zoomChanged(zoomPercent());
}

void FlatView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    // A fitted image stays fitted; a zoomed one keeps its scale (within the new limits).
    updateScale(m_scale);
}

// The window moved to a screen with another display scaling: a zoomed image keeps its size
// in device pixels (100 % stays 100 %), a fitted one is fitted again.
bool FlatView::event(QEvent *event)
{
    if (event->type() == QEvent::DevicePixelRatioChange) {
        const double ratio = devicePixelRatioF();
        if (ratio != m_devicePixelRatio)
            updateScale(m_scale * m_devicePixelRatio / ratio);
    }
    return QGraphicsView::event(event);
}

void FlatView::updateScale(double zoomedScale)
{
    m_devicePixelRatio = devicePixelRatioF();
    setScale(m_fitted ? minScale() : zoomedScale);
    if (m_fitted)
        centerOn(m_item);
}

void FlatView::wheelEvent(QWheelEvent *event)
{
    const int delta = event->angleDelta().y();
    if (delta == 0)
        return;
    event->accept();

    // Zoom towards the point under the cursor
    zoomBy(std::pow(kZoomStep, delta / 120.0), event->position());
}

bool FlatView::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::Gesture) {
        auto *gestureEvent = static_cast<QGestureEvent *>(event);
        if (auto *pinch = static_cast<QPinchGesture *>(gestureEvent->gesture(Qt::PinchGesture))) {
            // Zoom by the change of the finger distance towards the center of the fingers
            if (pinch->changeFlags() & QPinchGesture::ScaleFactorChanged)
                zoomBy(pinch->scaleFactor(), viewport()->mapFromGlobal(pinch->centerPoint()));
            gestureEvent->accept(pinch);
            return true;
        }
    }
    return QGraphicsView::viewportEvent(event);
}

void FlatView::applyTheme()
{
    setBackgroundBrush(Theme::colors().viewBackground);
}
