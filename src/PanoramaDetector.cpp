#include "PanoramaDetector.h"

#include <QByteArray>
#include <QFile>
#include <QRegularExpression>

namespace {

// APP1 payload prefixes of standard and extended XMP
const QByteArray kXmpSignature("http://ns.adobe.com/xap/1.0/\0", 29);
const QByteArray kExtendedXmpSignature("http://ns.adobe.com/xmp/extension/\0", 35);

// Matches both the attribute form  GPano:ProjectionType="equirectangular"
// and the element form             <GPano:ProjectionType>equirectangular</...>
bool xmpDeclaresEquirectangular(const QByteArray &xmp)
{
    static const QRegularExpression pattern(QStringLiteral(R"(ProjectionType(?:\s*=\s*["']|>)\s*equirectangular)"),
                                            QRegularExpression::CaseInsensitiveOption);
    return pattern.match(QString::fromUtf8(xmp)).hasMatch();
}

bool readByte(QFile &file, quint8 &byte)
{
    char c;
    if (!file.getChar(&c))
        return false;
    byte = quint8(c);
    return true;
}

} // namespace

bool isEquirectangularPanorama(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    // JPEG starts with SOI (FF D8)
    if (file.read(2) != QByteArray("\xFF\xD8", 2))
        return false;

    // Walk the marker segments until the image data starts (SOS) or the file ends (EOI).
    for (;;) {
        quint8 byte = 0;
        if (!readByte(file, byte) || byte != 0xFF)
            return false;
        quint8 marker = 0xFF;
        while (marker == 0xFF) {  // skip fill bytes
            if (!readByte(file, marker))
                return false;
        }

        if (marker == 0xDA || marker == 0xD9)  // SOS, EOI: no metadata follows
            return false;
        if (marker == 0x01 || (marker >= 0xD0 && marker <= 0xD7))  // TEM, RSTn: no length
            continue;

        const QByteArray lengthBytes = file.read(2);
        if (lengthBytes.size() != 2)
            return false;
        const int length = (quint8(lengthBytes[0]) << 8) | quint8(lengthBytes[1]);
        if (length < 2)
            return false;
        const int payloadSize = length - 2;

        if (marker == 0xE1) {  // APP1: Exif or XMP
            const QByteArray payload = file.read(payloadSize);
            if (payload.size() != payloadSize)
                return false;
            if ((payload.startsWith(kXmpSignature) || payload.startsWith(kExtendedXmpSignature))
                && xmpDeclaresEquirectangular(payload)) {
                return true;
            }
        } else if (!file.seek(file.pos() + payloadSize)) {
            return false;
        }
    }
}
