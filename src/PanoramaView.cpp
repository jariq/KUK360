#include "PanoramaView.h"

#include "MessageBoxes.h"

#include <QKeyEvent>
#include <QInputDevice>
#include <QMouseEvent>
#include <QNativeGestureEvent>
#include <QOpenGLContext>
#include <QOpenGLTexture>
#include <QTouchEvent>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace {

constexpr double kPi = 3.14159265358979323846;

double degToRad(double deg) { return deg * kPi / 180.0; }
double radToDeg(double rad) { return rad * 180.0 / kPi; }

bool isPowerOfTwo(int value) { return value > 0 && (value & (value - 1)) == 0; }

} // namespace

PanoramaView::PanoramaView(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_quad(QOpenGLBuffer::VertexBuffer)
{
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::OpenHandCursor);
    setAttribute(Qt::WA_AcceptTouchEvents);  // touch panning and pinch zoom, see touchEvent()

    m_animationTimer.setTimerType(Qt::PreciseTimer);
    m_animationTimer.setInterval(kAnimationIntervalMs);
    connect(&m_animationTimer, &QTimer::timeout, this, &PanoramaView::animate);
    m_clock.start();
}

PanoramaView::~PanoramaView()
{
    // ~QOpenGLWidget destroys the context and emits aboutToBeDestroyed after the members of
    // this class are gone; the connection is removed only later by ~QObject, so cleanup()
    // would run on destroyed members and crash on quit (macOS, Linux).
    if (context())
        disconnect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &PanoramaView::cleanup);
    cleanup();
}

void PanoramaView::setImage(const QImage &image)
{
    m_image = image;

    // The image covers 360° horizontally (kHaov); the vertical extent follows from the
    // aspect ratio (2:1 -> 180°) and is centered on the horizon.
    m_vaov = m_image.isNull() ? 180.0 : std::min(180.0, kHaov * m_image.height() / m_image.width());

    // Start every panorama from the default view
    stopAnimation();
    m_speed = Speed();
    m_autoRotateLevel = 0;
    m_dragging = false;
    m_yaw = 0.0;
    m_pitch = 0.0;
    m_portrait = width() < height();
    m_hfov = hfovRange().initial;

    // Free the textures of the previous image now: a hidden view (flat projection,
    // welcome page) does not paint, so uploadTextures() would keep them in GPU memory
    // until it is shown again.
    if (m_glReady && (m_texture0 || m_texture1)) {
        makeCurrent();
        m_texture0.reset();
        m_texture1.reset();
        doneCurrent();
    }
    m_textureDirty = true;
    update();
}

void PanoramaView::initializeGL()
{
    initializeOpenGLFunctions();
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &PanoramaView::cleanup,
            Qt::UniqueConnection);

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_maxTextureSize);
    if (m_maxTextureSize <= 0)
        m_maxTextureSize = 2048;

    // A new program for every context: the program of a destroyed context (cleanup()) is
    // not valid in a new one.
    m_program = std::make_unique<QOpenGLShaderProgram>();
    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, QStringLiteral(":/src/shaders/panorama.vert"))
        || !m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, QStringLiteral(":/src/shaders/panorama.frag"))
        || !m_program->link()) {
        reportGlError(QStringLiteral("Unable to build the panorama shader:\n%1").arg(m_program->log()));
        return;
    }

    // Two triangles covering the whole viewport
    static const GLfloat quad[] = {
        -1.0f, 1.0f,   1.0f, 1.0f,   1.0f, -1.0f,
        -1.0f, 1.0f,   1.0f, -1.0f, -1.0f, -1.0f,
    };
    m_quad.create();
    m_quad.bind();
    m_quad.allocate(quad, sizeof(quad));
    m_quad.release();

    m_glReady = true;
    m_textureDirty = !m_image.isNull();
}

void PanoramaView::cleanup()
{
    if (!context())
        return;

    makeCurrent();
    m_texture0.reset();
    m_texture1.reset();
    m_quad.destroy();
    m_program.reset();
    doneCurrent();

    m_glReady = false;
    m_textureDirty = !m_image.isNull();
}

void PanoramaView::uploadTextures()
{
    m_textureDirty = false;
    m_texture0.reset();
    m_texture1.reset();
    m_splitImage = false;

    if (m_image.isNull())
        return;

    // Like Pannellum, an image wider than the texture size limit is split into two
    // halves. Pannellum gives up if even that does not fit; we downscale instead.
    QImage image = m_image;
    const QSize limit(2 * m_maxTextureSize, m_maxTextureSize);
    if (image.width() > limit.width() || image.height() > limit.height()) {
        qWarning("Panorama %dx%d exceeds the GPU texture limit (%d), downscaling.",
                 image.width(), image.height(), m_maxTextureSize);
        image = image.scaled(limit, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    m_splitImage = image.width() > m_maxTextureSize;

    // Repeat avoids a seam at yaw ±180°; GLES 2 only supports it for power-of-two sizes.
    const bool repeat = !m_splitImage
                        && (!context()->isOpenGLES() || (isPowerOfTwo(image.width()) && isPowerOfTwo(image.height())));

    auto makeTexture = [](const QImage &part, QOpenGLTexture::WrapMode wrapS) {
        auto texture = std::make_unique<QOpenGLTexture>(part, QOpenGLTexture::DontGenerateMipMaps);
        texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
        texture->setWrapMode(QOpenGLTexture::DirectionS, wrapS);
        texture->setWrapMode(QOpenGLTexture::DirectionT, QOpenGLTexture::ClampToEdge);
        return texture;
    };

    if (m_splitImage) {
        const int half = image.width() / 2;
        m_texture0 = makeTexture(image.copy(0, 0, half, image.height()), QOpenGLTexture::ClampToEdge);
        m_texture1 = makeTexture(image.copy(half, 0, half, image.height()), QOpenGLTexture::ClampToEdge);
    } else {
        m_texture0 = makeTexture(image, repeat ? QOpenGLTexture::Repeat : QOpenGLTexture::ClampToEdge);
    }
}

void PanoramaView::paintGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!m_glReady)
        return;
    if (m_textureDirty)
        uploadTextures();
    if (!m_texture0)
        return;

    // Focal length from the vertical field of view (libpannellum.js render())
    const double aspect = aspectRatio();
    const double focal = 1.0 / std::tan(degToRad(verticalFov()) * 0.5);

    m_program->bind();
    m_program->setUniformValue("u_aspectRatio", GLfloat(aspect));
    m_program->setUniformValue("u_psi", GLfloat(degToRad(m_yaw)));
    m_program->setUniformValue("u_theta", GLfloat(degToRad(m_pitch)));
    m_program->setUniformValue("u_rot", 0.0f);
    m_program->setUniformValue("u_f", GLfloat(focal));
    m_program->setUniformValue("u_h", GLfloat(kHaov / 360.0));
    m_program->setUniformValue("u_v", GLfloat(m_vaov / 180.0));
    m_program->setUniformValue("u_vo", 0.0f);
    m_program->setUniformValue("u_backgroundColor", 0.0f, 0.0f, 0.0f, 1.0f);
    m_program->setUniformValue("u_splitImage", GLint(m_splitImage));
    m_program->setUniformValue("u_image0", 0);
    // Without a second half, u_image1 shares unit 0: macOS warns about samplers bound to
    // an empty texture unit.
    m_program->setUniformValue("u_image1", m_texture1 ? 1 : 0);
    m_program->setUniformValue("u_grid", GLint(m_gridVisible));
    m_program->setUniformValue("u_pixelSize", GLfloat(2.0 / (height() * devicePixelRatioF())));

    m_texture0->bind(0);
    if (m_texture1)
        m_texture1->bind(1);

    const int texCoord = m_program->attributeLocation("a_texCoord");
    m_quad.bind();
    m_program->enableAttributeArray(texCoord);
    m_program->setAttributeBuffer(texCoord, GL_FLOAT, 0, 2);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_program->disableAttributeArray(texCoord);
    m_quad.release();
    if (m_texture1)
        m_texture1->release(1);
    m_texture0->release(0);
    m_program->release();
}

// When the view turns from landscape to portrait or back (rotated display, window resized
// across a square), the hfov range of the new orientation applies and the zoom returns to
// its initial hfov (resetHfov() of the author's Pannellum fork). Unlike there, a resize
// within the same orientation keeps the zoom.
void PanoramaView::resizeEvent(QResizeEvent *event)
{
    // Before the base class, which already paints the resized view
    const bool portrait = width() < height();
    if (portrait != m_portrait) {
        m_portrait = portrait;
        m_speed.hfov = 0.0;
        m_hfov = hfovRange().initial;
    }

    QOpenGLWidget::resizeEvent(event);
}

void PanoramaView::reportGlError(const QString &message)
{
    qWarning("%s", qPrintable(message));
    QTimer::singleShot(0, this, [this, message] {
        MessageBoxes::showCritical(this, message);
        window()->close();
    });
}

// --- Interaction (pannellum.js) ---------------------------------------------

// Mouse events that Qt synthesizes from touch input are ignored: touch is handled by
// touchEvent() (drag, two-finger pan and pinch zoom), the two would fight otherwise.
static bool isFromTouch(const QMouseEvent *event)
{
    return event->pointingDevice()->type() == QInputDevice::DeviceType::TouchScreen;
}

void PanoramaView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || isFromTouch(event))
        return;

    beginDrag(event->position());
    setCursor(Qt::ClosedHandCursor);
}

// Port of onDocumentMouseDown() / onDocumentTouchStart(): the view follows the pointer from
// pos on, so the view direction and the pointer position at this moment are remembered.
void PanoramaView::beginDrag(const QPointF &pos)
{
    stopAutoRotation();
    stopAnimation();
    m_speed.hfov = 0.0;

    m_dragging = true;
    markInteraction();
    m_pointerDownPos = pos;
    m_pointerDownYaw = m_yaw;
    m_pointerDownPitch = m_pitch;
}

void PanoramaView::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging || isFromTouch(event))
        return;
    dragTo(event->position());
}

// Port of onDocumentMouseMove(): the view follows the pointer dragged from m_pointerDownPos.
// Used for the mouse and for dragging with one finger.
void PanoramaView::dragTo(const QPointF &pos)
{
    markInteraction();
    const double w = width();
    const double h = height();

    const double yaw = radToDeg(std::atan(m_pointerDownPos.x() / w * 2.0 - 1.0) - std::atan(pos.x() / w * 2.0 - 1.0))
                       * m_hfov / 90.0 + m_pointerDownYaw;
    m_speed.yaw = std::remainder(yaw - m_yaw, 360.0) * kDragSpeedFactor;
    m_yaw = yaw;

    const double pitch = radToDeg(std::atan(pos.y() / h * 2.0 - 1.0) - std::atan(m_pointerDownPos.y() / h * 2.0 - 1.0))
                         * verticalFov() / 90.0 + m_pointerDownPitch;
    m_speed.pitch = (pitch - m_pitch) * kDragSpeedFactor;
    m_pitch = pitch;

    constrainView();
    update();
}

void PanoramaView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || !m_dragging || isFromTouch(event))
        return;

    endDrag(kMouseRestMs);
    setCursor(Qt::OpenHandCursor);
}

// Port of onDocumentMouseUp() / onDocumentTouchEnd(): the view glides on with the speed of
// the drag (inertia), unless the pointer rested longer than restLimitMs before release;
// this prevents a jump when it is moved quickly, stopped and then released.
void PanoramaView::endDrag(qint64 restLimitMs)
{
    m_dragging = false;
    if (m_clock.elapsed() - m_latestInteraction > restLimitMs)
        m_speed.pitch = m_speed.yaw = 0.0;
    markInteraction();
    startAnimation();
}

bool PanoramaView::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
        // Only touch screens: macOS also reports the fingers on a trackpad as touch events,
        // which would pan the view while scrolling; its pinch arrives as a native gesture.
        if (static_cast<QTouchEvent *>(event)->device()->type() != QInputDevice::DeviceType::TouchScreen)
            return QOpenGLWidget::event(event);
        // Accepted, so Qt does not synthesize mouse events from the touch input (the
        // accepted flag counts, not the return value).
        touchEvent(static_cast<QTouchEvent *>(event));
        event->accept();
        return true;
    case QEvent::NativeGesture: {
        // Trackpad pinch (macOS): value() is the change of the magnification
        auto *gesture = static_cast<QNativeGestureEvent *>(event);
        if (gesture->gestureType() != Qt::ZoomNativeGesture)
            return QOpenGLWidget::event(event);
        stopAutoRotation();
        m_speed.hfov = 0.0;
        markInteraction();
        setHfov(m_hfov / (1.0 + gesture->value()));
        update();
        event->accept();
        return true;
    }
    default:
        return QOpenGLWidget::event(event);
    }
}

// Touch input, port of onDocumentTouchStart/Move/End (pannellum.js): one finger drags like
// the mouse, two fingers pan with their center and zoom with their distance (pinch).
void PanoramaView::touchEvent(QTouchEvent *event)
{
    // The system took the touch over (e.g. a swipe from the screen edge): the drag stops
    // where it is, without the inertia of lifted fingers.
    if (event->type() == QEvent::TouchCancel) {
        m_dragging = false;
        m_touchPointCount = 0;
        m_pointerDownDist = -1.0;
        m_speed.yaw = m_speed.pitch = 0.0;
        return;
    }

    QList<QPointF> points;
    for (const QEventPoint &point : event->points()) {
        if (point.state() != QEventPoint::Released)
            points.append(point.position());
    }

    // onDocumentTouchEnd
    if (points.isEmpty()) {
        if (!m_dragging)
            return;
        // One finger rests like the mouse, two like in Pannellum touch
        const qint64 restLimit = m_touchPointCount == 1 ? kMouseRestMs : kTouchRestMs;
        m_touchPointCount = 0;
        m_pointerDownDist = -1.0;
        endDrag(restLimit);
        return;
    }

    QPointF pos = points.at(0);
    double dist = -1.0;
    if (points.size() >= 2) {
        // The pointer is the center of the two fingers
        pos += (points.at(1) - points.at(0)) * 0.5;
        dist = QLineF(points.at(0), points.at(1)).length();
    }

    // onDocumentTouchStart, also when a finger is added or lifted (then the view continues
    // from the new pointer position instead of jumping)
    if (!m_dragging || points.size() != m_touchPointCount) {
        beginDrag(pos);
        m_touchPointCount = points.size();
        m_pointerDownDist = dist;
        return;
    }

    // One finger drags like the mouse (onDocumentMouseMove); Pannellum's touch pan formula
    // felt worse on a real touch screen.
    if (points.size() == 1) {
        dragTo(pos);
        return;
    }

    // onDocumentTouchMove with two fingers
    markInteraction();
    if (dist >= 0.0 && m_pointerDownDist >= 0.0) {
        setHfov(m_hfov + (m_pointerDownDist - dist) * kPinchZoomFactor);
        m_pointerDownDist = dist;
    }

    // Pan speed depending on the zoom keeps the panorama roughly under the finger
    // (touchPanSpeedCoeffFactor = 1).
    const double panSpeed = m_hfov / 360.0;
    const double yaw = (m_pointerDownPos.x() - pos.x()) * panSpeed + m_pointerDownYaw;
    m_speed.yaw = std::remainder(yaw - m_yaw, 360.0) * kDragSpeedFactor;
    m_yaw = yaw;
    const double pitch = (pos.y() - m_pointerDownPos.y()) * panSpeed + m_pointerDownPitch;
    m_speed.pitch = (pitch - m_pitch) * kDragSpeedFactor;
    m_pitch = pitch;

    constrainView();
    update();
}

void PanoramaView::wheelEvent(QWheelEvent *event)
{
    const int delta = event->angleDelta().y();
    if (delta == 0)
        return;

    event->accept();
    stopAutoRotation();
    markInteraction();
    setHfov(m_hfov - delta * kWheelZoomFactor);
    m_speed.hfov = delta < 0 ? 1.0 : -1.0;
    update();
    startAnimation();
}

void PanoramaView::keyPressEvent(QKeyEvent *event)
{
    if (!setKey(event->key(), true)) {
        QOpenGLWidget::keyPressEvent(event);
        return;
    }
    stopAutoRotation();
    markInteraction();
    startAnimation();
}

void PanoramaView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat() || !setKey(event->key(), false))
        QOpenGLWidget::keyReleaseEvent(event);
}

void PanoramaView::focusOutEvent(QFocusEvent *event)
{
    std::fill(std::begin(m_keysDown), std::end(m_keysDown), false);
    QOpenGLWidget::focusOutEvent(event);
}

bool PanoramaView::setKey(int qtKey, bool down)
{
    Key key;
    switch (qtKey) {
    // Q / E zoom next to WASD, so the view is controlled with the left hand only
    case Qt::Key_Q: key = ZoomOut; break;
    case Qt::Key_E: key = ZoomIn; break;
    case Qt::Key_W: key = Up; break;
    case Qt::Key_S: key = Down; break;
    case Qt::Key_A: key = Left; break;
    case Qt::Key_D: key = Right; break;
    default: return false;
    }
    m_keysDown[key] = down;
    return true;
}

bool PanoramaView::anyKeyDown() const
{
    return std::any_of(std::begin(m_keysDown), std::end(m_keysDown), [](bool down) { return down; });
}

// --- Animation --------------------------------------------------------------

void PanoramaView::startAnimation()
{
    if (!m_animationTimer.isActive())
        m_animationTimer.start();
}

void PanoramaView::stopAnimation()
{
    m_animationTimer.stop();
    m_prevTime = -1.0;
}

// Port of animate(): keeps running while keys are held or the view still moves.
void PanoramaView::animate()
{
    // A hidden view (flat projection shown) does not rotate in the background;
    // showEvent() restarts the animation.
    const bool autoRotating = m_autoRotateLevel != 0 && isVisible();
    if (!m_dragging
        && (anyKeyDown() || autoRotating || std::abs(m_speed.yaw) > kMinSpeed
            || std::abs(m_speed.pitch) > kMinSpeed || std::abs(m_speed.hfov) > kMinSpeed)) {
        keyRepeat();
        update();
        return;
    }

    stopAnimation();
}

// Port of keyRepeat(): keyboard panning/zooming and "inertia".
void PanoramaView::keyRepeat()
{
    const double prevPitch = m_pitch;
    const double prevYaw = m_yaw;
    const double prevZoom = m_hfov;

    const double newTime = m_clock.nsecsElapsed() / 1.0e6;
    if (m_prevTime < 0.0)
        m_prevTime = newTime;
    // Step of this frame, proportional to the elapsed time and the hfov (slower when zoomed
    // in). The limits avoid a jump if something goes wrong with the time difference.
    constexpr double kTimeScale = 1200.0;  // ms * degrees of hfov per unit step
    constexpr double kMaxStep = 10.0;
    constexpr double kMaxFrameMs = 100.0;  // automatic rotation
    const double diff = std::min((newTime - m_prevTime) * m_hfov / kTimeScale, kMaxStep);
    const double elapsedSeconds = std::min(newTime - m_prevTime, kMaxFrameMs) / 1000.0;
    const bool autoRotating = m_autoRotateLevel != 0;

    if (m_keysDown[ZoomOut])
        setHfov(m_hfov + (m_speed.hfov * 0.8 + 0.4) * diff);
    if (m_keysDown[ZoomIn])
        setHfov(m_hfov + (m_speed.hfov * 0.8 - 0.2) * diff);
    if (m_keysDown[Up])
        m_pitch += (m_speed.pitch * 0.8 + 0.2) * diff;
    if (m_keysDown[Down])
        m_pitch += (m_speed.pitch * 0.8 - 0.2) * diff;
    if (m_keysDown[Left])
        m_yaw += (m_speed.yaw * 0.8 - 0.2) * diff;
    if (m_keysDown[Right])
        m_yaw += (m_speed.yaw * 0.8 + 0.2) * diff;

    // Automatic rotation at a constant angular speed, independent of the zoom
    if (autoRotating && !m_keysDown[Left] && !m_keysDown[Right])
        m_yaw += m_autoRotateLevel * kAutoRotateStep * elapsedSeconds;

    // "Inertia" (not for yaw while rotating automatically, as in Pannellum)
    if (diff > 0.0) {
        double slowDownFactor = 1.0 - kFriction;
        if (!m_keysDown[Left] && !m_keysDown[Right] && !autoRotating)
            m_yaw += m_speed.yaw * diff * slowDownFactor;
        if (!m_keysDown[Up] && !m_keysDown[Down])
            m_pitch += m_speed.pitch * diff * slowDownFactor;
        if (!m_keysDown[ZoomOut] && !m_keysDown[ZoomIn]) {
            // Slow down faster for wider HFOV
            if (m_hfov > 90.0)
                slowDownFactor *= 1.0 - (m_hfov - 90.0) / 90.0;
            setHfov(m_hfov + m_speed.hfov * diff * slowDownFactor);
        }
    }

    m_prevTime = newTime;
    if (diff > 0.0) {
        m_speed.yaw = std::clamp(m_speed.yaw * 0.8 + (m_yaw - prevYaw) / diff * 0.2, -kMaxSpeed, kMaxSpeed);
        m_speed.pitch = std::clamp(m_speed.pitch * 0.8 + (m_pitch - prevPitch) / diff * 0.2, -kMaxSpeed, kMaxSpeed);
        m_speed.hfov = std::clamp(m_speed.hfov * 0.8 + (m_hfov - prevZoom) / diff * 0.2, -kMaxSpeed, kMaxSpeed);
    }

    // Stop movement if opposite controls are pressed
    if (m_keysDown[ZoomOut] && m_keysDown[ZoomIn])
        m_speed.hfov = 0.0;
    if (m_keysDown[Up] && m_keysDown[Down])
        m_speed.pitch = 0.0;
    if (m_keysDown[Left] && m_keysDown[Right])
        m_speed.yaw = 0.0;

    constrainView();
}

// --- View -------------------------------------------------------------------

void PanoramaView::setHfov(double hfov)
{
    const HfovRange &range = hfovRange();
    const double clamped = std::clamp(hfov, range.min, range.max);
    if (clamped == m_hfov)
        return;
    m_hfov = clamped;
    emit zoomChanged(zoomPercent());
}

double PanoramaView::zoomPercent() const
{
    return 100.0 * std::tan(degToRad(hfovRange().initial) * 0.5) / std::tan(degToRad(m_hfov) * 0.5);
}

void PanoramaView::setGridVisible(bool visible)
{
    m_gridVisible = visible;
    update();
}

void PanoramaView::zoomIn()
{
    zoomToHfov(m_hfov - kZoomButtonStep);
}

void PanoramaView::zoomOut()
{
    zoomToHfov(m_hfov + kZoomButtonStep);
}

void PanoramaView::resetZoom()
{
    zoomToHfov(hfovRange().initial);
}

// Zoom requested by the user (menu): counts as interaction like the wheel.
void PanoramaView::zoomToHfov(double hfov)
{
    stopAutoRotation();
    m_speed.hfov = 0.0;  // stop zoom inertia
    setHfov(hfov);
    update();
}

double PanoramaView::aspectRatio() const
{
    return height() > 0 ? double(width()) / height() : 1.0;
}

double PanoramaView::verticalFov() const
{
    return radToDeg(2.0 * std::atan(std::tan(degToRad(m_hfov) * 0.5) / aspectRatio()));
}

// Same limits as Pannellum's render() with default (unset) min/max yaw and pitch.
void PanoramaView::constrainView()
{
    m_yaw = std::remainder(m_yaw, 360.0);
    m_pitch = std::clamp(m_pitch, -90.0, 90.0);
}

void PanoramaView::setAutoRotateLevel(int level)
{
    m_autoRotateLevel = std::clamp(level, -kMaxAutoRotateLevel, kMaxAutoRotateLevel);
    if (m_autoRotateLevel == 0)
        m_speed.yaw = 0.0;  // stop right away instead of gliding on
    else
        startAnimation();
    emit autoRotateLevelChanged(m_autoRotateLevel);
}

// Any user interaction with the view (dragging, keys, zooming) stops the rotation.
void PanoramaView::stopAutoRotation()
{
    if (m_autoRotateLevel != 0)
        setAutoRotateLevel(0);
}

void PanoramaView::showEvent(QShowEvent *event)
{
    QOpenGLWidget::showEvent(event);
    if (m_autoRotateLevel != 0)
        startAnimation();
}
