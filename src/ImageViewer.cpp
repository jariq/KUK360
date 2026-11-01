#include "ImageViewer.h"

#include "FlatView.h"
#include "FontAwesome.h"
#include "MessageOverlay.h"
#include "PanoramaView.h"
#include "Theme.h"

#include <QLabel>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>

ImageViewer::ImageViewer(QWidget *parent)
    : QWidget(parent)
{
    m_welcomePage = createWelcomePage();
    m_errorPage = createErrorPage();
    m_panoramaView = new PanoramaView;
    m_flatView = new FlatView;

    m_stack = new QStackedWidget;
    m_stack->addWidget(m_welcomePage);
    m_stack->addWidget(m_errorPage);
    m_stack->addWidget(m_panoramaView);
    m_stack->addWidget(m_flatView);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_stack);

    // Floating over the stack: the messages (positioned by MessageOverlay itself)
    m_messageOverlay = new MessageOverlay(this);

    // The message is shown also when an interaction with the view (drag, keys, zoom)
    // stops the rotation.
    connect(m_panoramaView, &PanoramaView::autoRotateLevelChanged, this, &ImageViewer::showRotationMessage);
    // Zooming in the views (wheel, keys, pinch, menu) tells the zoom level.
    connect(m_panoramaView, &PanoramaView::zoomChanged, this, &ImageViewer::showZoomMessage);
    connect(m_flatView, &FlatView::zoomChanged, this, &ImageViewer::showZoomMessage);

    applyTheme();
}

QWidget *ImageViewer::createWelcomePage()
{
    auto *page = new QWidget;
    page->setObjectName(QStringLiteral("welcomePage"));
    page->setAttribute(Qt::WA_StyledBackground);

    auto *openButton = new QToolButton;
    openButton->setObjectName(QStringLiteral("openImageButton"));
    openButton->setIcon(FontAwesome::icon(FontAwesome::Icon::FolderOpen));
    openButton->setIconSize(QSize(120, 120));
    openButton->setText(tr("Open image..."));
    openButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    openButton->setAutoRaise(true);
    openButton->setCursor(Qt::PointingHandCursor);
    connect(openButton, &QToolButton::clicked, this, &ImageViewer::openRequested);

    auto *layout = new QVBoxLayout(page);
    layout->addWidget(openButton, 0, Qt::AlignCenter);
    return page;
}

QWidget *ImageViewer::createErrorPage()
{
    auto *page = new QWidget;
    page->setObjectName(QStringLiteral("errorPage"));
    page->setAttribute(Qt::WA_StyledBackground);

    m_errorLabel = new QLabel;
    m_errorLabel->setObjectName(QStringLiteral("errorLabel"));
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setWordWrap(true);
    // A long path does not wrap; it must not widen the window.
    m_errorLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 32, 32, 32);
    layout->addWidget(m_errorLabel);
    return page;
}

// The view shown in the stack, nullptr on the welcome page.
ZoomableView *ImageViewer::currentView() const
{
    if (m_stack->currentWidget() == m_panoramaView)
        return m_panoramaView;
    if (m_stack->currentWidget() == m_flatView)
        return m_flatView;
    return nullptr;
}

void ImageViewer::setImage(const QImage &image, Projection projection)
{
    // Drop the previous image from both views; the active one gets the new image in
    // setProjection(), the other one only when the user switches to it.
    m_image = image;
    clearViews();
    m_panoramaViewDirty = true;
    m_flatViewDirty = true;
    setProjection(projection);
    emit imageChanged();
}

void ImageViewer::clear()
{
    showPageWithoutImage(m_welcomePage);
}

void ImageViewer::showError(const QString &text)
{
    m_errorLabel->setText(text);
    showPageWithoutImage(m_errorPage);
}

void ImageViewer::showPageWithoutImage(QWidget *page)
{
    m_image = QImage();
    clearViews();
    m_panoramaViewDirty = false;
    m_flatViewDirty = false;
    m_stack->setCurrentWidget(page);
    emit imageChanged();
}

// Both views let go of the previous image.
void ImageViewer::clearViews()
{
    m_panoramaView->clear();
    m_flatView->clear();
}

void ImageViewer::setProjection(Projection projection)
{
    m_projection = projection;
    if (!m_image.isNull()) {
        showImageInCurrentView();
        // Always tell the user which projection is active (after opening a file too).
        m_messageOverlay->showMessage(m_projection == Projection::Panorama360 ? tr("360° projection")
                                                                              : tr("Flat projection"));
    }
    emit projectionChanged();
}

void ImageViewer::showImageInCurrentView()
{
    // Views get the image lazily: FlatView converts it to a pixmap (a copy), so it
    // is only done when that view is actually shown.
    QWidget *view = nullptr;
    if (m_projection == Projection::Flat) {
        if (m_flatViewDirty)
            m_flatView->setImage(m_image);
        m_flatViewDirty = false;
        view = m_flatView;
    } else {
        if (m_panoramaViewDirty)
            m_panoramaView->setImage(m_image);
        m_panoramaViewDirty = false;
        view = m_panoramaView;
    }
    m_stack->setCurrentWidget(view);
    view->setFocus();
}

void ImageViewer::zoomIn()
{
    if (ZoomableView *view = currentView())
        view->zoomIn();
}

void ImageViewer::zoomOut()
{
    if (ZoomableView *view = currentView())
        view->zoomOut();
}

// 100 % in the flat view, the default field of view in the 360 view.
void ImageViewer::resetZoom()
{
    if (ZoomableView *view = currentView())
        view->resetZoom();
}

void ImageViewer::setGridVisible(bool visible)
{
    m_gridVisible = visible;
    m_flatView->setGridVisible(visible);
    m_panoramaView->setGridVisible(visible);
}

// In the 360 view each step changes the automatic rotation speed by one level towards
// the left / right, so the opposite button slows down, stops and then reverses the
// rotation. In the flat view each step turns the displayed image by 90° (the file is
// not changed).
void ImageViewer::rotate(int step)
{
    if (m_image.isNull())
        return;

    if (m_projection == Projection::Flat) {
        m_flatView->rotateImage(step * 90);
        return;
    }

    // The message is shown by showRotationMessage() via autoRotateLevelChanged.
    m_panoramaView->setAutoRotateLevel(m_panoramaView->autoRotateLevel() + step);
}

void ImageViewer::showRotationMessage(int level)
{
    if (level < 0)
        m_messageOverlay->showMessage(tr("Rotation left: %1").arg(-level));
    else if (level > 0)
        m_messageOverlay->showMessage(tr("Rotation right: %1").arg(level));
    else
        m_messageOverlay->showMessage(tr("Rotation stopped"));
}

// Flat view: 100 % = one image pixel per device pixel. 360 view: magnification relative
// to the initial field of view (100 %).
void ImageViewer::showZoomMessage(double percent)
{
    m_messageOverlay->showMessage(tr("Zoom: %1%").arg(qRound(percent)));
}

void ImageViewer::applyTheme()
{
    const Theme::Colors &colors = Theme::colors();
    m_welcomePage->setStyleSheet(
        QStringLiteral("#welcomePage { background-color: %1; }"
                       "#openImageButton { color: %2; font-size: 24pt; font-weight: bold;"
                       " border: none; background: transparent; padding: 12px; }"
                       "#openImageButton:hover { color: %3; }")
            .arg(Theme::css(colors.viewBackground), Theme::css(colors.text), Theme::css(colors.iconActive)));
    m_errorPage->setStyleSheet(
        QStringLiteral("#errorPage { background-color: %1; }"
                       "#errorLabel { color: %2; font-size: 14pt; }")
            .arg(Theme::css(colors.viewBackground), Theme::css(colors.text)));
    m_messageOverlay->applyTheme();
    m_flatView->applyTheme();
}
