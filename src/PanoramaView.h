#pragma once

#include "ZoomableView.h"

#include <QElapsedTimer>
#include <QImage>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QPointF>
#include <QTimer>

#include <memory>

class QOpenGLTexture;
class QTouchEvent;

// Interactive 360° viewer for equirectangular panoramas.
// Rendering and interaction are ported from Pannellum (libpannellum.js / pannellum.js):
// a full-screen quad is drawn and the fragment shader maps every pixel to the sphere.
class PanoramaView : public QOpenGLWidget, public ZoomableView, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit PanoramaView(QWidget *parent = nullptr);
    ~PanoramaView() override;

    // The image is shared (QImage implicit sharing), not copied; the caller owns it.
    void setImage(const QImage &image);
    void clear() { setImage(QImage()); }

    // Zoom steps of the Pannellum zoom buttons (hfov -/+ 5°); resetZoom() returns to
    // the initial hfov of a newly opened panorama (depends on the orientation of the view).
    void zoomIn() override;
    void zoomOut() override;
    void resetZoom() override;

    // Automatic rotation (like Pannellum's autoRotate): level -kMaxAutoRotateLevel..
    // kMaxAutoRotateLevel, negative = the view turns left, 0 = no rotation (default).
    static constexpr int kMaxAutoRotateLevel = 10;
    int autoRotateLevel() const { return m_autoRotateLevel; }
    void setAutoRotateLevel(int level);

    // Alignment grid fixed to the sphere: parallels and meridians every 10° (the
    // equator red), 1 px lines in the colors of the FlatView grid.
    bool isGridVisible() const { return m_gridVisible; }
    void setGridVisible(bool visible);

signals:
    // The user zoomed (wheel, keys, pinch, menu, also the inertia). The percentage is the
    // magnification in the middle of the view relative to the initial hfov (100 %), like
    // a ratio of focal lengths.
    void zoomChanged(double percent);
    // Emitted by setAutoRotateLevel(), also when user interaction stops the rotation.
    void autoRotateLevelChanged(int level);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeEvent(QResizeEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool event(QEvent *event) override;

private:
    enum Key { ZoomOut, ZoomIn, Up, Down, Left, Right, KeyCount };

    struct Speed
    {
        double yaw = 0.0;
        double pitch = 0.0;
        double hfov = 0.0;
    };

    void cleanup();
    void uploadTextures();
    void reportGlError(const QString &message);

    // Range and default of the hfov. A view taller than wide (portrait) gets a narrower hfov,
    // otherwise its vertical field of view would be huge and distorted (our addition, from
    // the author's Pannellum fork). The range switches when the orientation changes.
    struct HfovRange
    {
        double min;
        double max;
        double initial;
    };

    // --- Constants (degrees, ms), from pannellum.js unless noted ---
    static constexpr double kHaov = 360.0;  // horizontal extent of the image, always all around
    static constexpr HfovRange kLandscapeHfov{50.0, 120.0, 100.0};
    static constexpr HfovRange kPortraitHfov{25.0, 75.0, 50.0};
    static constexpr double kZoomButtonStep = 5.0;    // zoomIn()/zoomOut()
    static constexpr double kWheelZoomFactor = 0.05;  // hfov per wheel angle delta unit
    static constexpr double kPinchZoomFactor = 0.1;   // hfov per pixel of finger distance change
    // Part of the pointer movement that becomes the inertia speed when dragging
    static constexpr double kDragSpeedFactor = 0.2;
    // The inertia is dropped when the pointer rested this long before it was released
    static constexpr qint64 kMouseRestMs = 15;   // mouse and one finger
    static constexpr qint64 kTouchRestMs = 150;  // two fingers
    static constexpr double kFriction = 0.15;
    static constexpr double kMinSpeed = 0.01;  // the animation stops below this speed
    static constexpr double kMaxSpeed = 5.0;
    static constexpr double kAutoRotateStep = 3.0;  // degrees per second per level (KUK360)
    static constexpr int kAnimationIntervalMs = 16;

    void startAnimation();
    void stopAnimation();
    void animate();
    void keyRepeat();
    void markInteraction() { m_latestInteraction = m_clock.elapsed(); }
    void beginDrag(const QPointF &pos);
    void dragTo(const QPointF &pos);
    void endDrag(qint64 restLimitMs);
    void touchEvent(QTouchEvent *event);
    bool setKey(int qtKey, bool down);
    bool anyKeyDown() const;

    void setHfov(double hfov);
    double zoomPercent() const;
    void zoomToHfov(double hfov);
    void stopAutoRotation();
    const HfovRange &hfovRange() const { return m_portrait ? kPortraitHfov : kLandscapeHfov; }
    double aspectRatio() const;
    double verticalFov() const;
    void constrainView();

    // Panorama
    QImage m_image;  // shared with the owner, kept for (re)uploading the textures
    bool m_textureDirty = false;
    double m_vaov = 180.0;  // degrees, from the aspect ratio of the image

    // GL resources
    std::unique_ptr<QOpenGLShaderProgram> m_program;  // created per context in initializeGL()
    QOpenGLBuffer m_quad;
    std::unique_ptr<QOpenGLTexture> m_texture0;
    std::unique_ptr<QOpenGLTexture> m_texture1;
    bool m_splitImage = false;
    int m_maxTextureSize = 0;
    bool m_glReady = false;

    // View (degrees), same defaults as Pannellum
    double m_yaw = 0.0;
    double m_pitch = 0.0;
    bool m_portrait = false;
    double m_hfov = kLandscapeHfov.initial;
    int m_autoRotateLevel = 0;
    bool m_gridVisible = false;

    // Interaction
    bool m_dragging = false;
    QPointF m_pointerDownPos;
    double m_pointerDownDist = -1.0;  // distance of two touch points, -1 with one
    qsizetype m_touchPointCount = 0;   // touch points of the current touch drag, 0 = none
    double m_pointerDownYaw = 0.0;
    double m_pointerDownPitch = 0.0;
    qint64 m_latestInteraction = 0;  // ms on m_clock
    bool m_keysDown[KeyCount] = {};
    Speed m_speed;

    // Animation
    QTimer m_animationTimer;
    QElapsedTimer m_clock;
    double m_prevTime = -1.0;  // ms on m_clock, < 0 when not animating
};
