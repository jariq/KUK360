#pragma once

// Zoom of the two image views (PanoramaView, FlatView), so that View > Zoom of
// ImageViewer acts on whichever view is active. Each view also has the signal
// zoomChanged(double percent), emitted when the user zooms, which an interface without
// QObject cannot declare.
class ZoomableView
{
public:
    virtual ~ZoomableView() = default;

    // View > Zoom: one step in / out, back to the initial zoom
    virtual void zoomIn() = 0;
    virtual void zoomOut() = 0;
    virtual void resetZoom() = 0;
};
