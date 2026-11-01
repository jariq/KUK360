#pragma once

#include <QString>

// Returns true if the JPEG file declares an equirectangular 360° panorama in its XMP
// metadata (Google Photo Sphere tag GPano:ProjectionType="equirectangular").
// Only the metadata segments before the image data are read.
bool isEquirectangularPanorama(const QString &filePath);
