#include "FontAwesome.h"

#include "Theme.h"

#include <QFont>
#include <QFontDatabase>
#include <QIconEngine>
#include <QPainter>
#include <QPixmap>

namespace FontAwesome {

namespace {

QString fontFamily()
{
    static const QString family = [] {
        const int id = QFontDatabase::addApplicationFont(QStringLiteral(":/src/fontawesome-webfont.ttf"));
        const QStringList families = QFontDatabase::applicationFontFamilies(id);
        if (families.isEmpty()) {
            qWarning("Unable to load Font Awesome.");
            return QString();
        }
        return families.first();
    }();
    return family;
}

class GlyphIconEngine : public QIconEngine
{
public:
    explicit GlyphIconEngine(Icon glyph)
        : m_glyph(glyph)
    {
    }

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State) override
    {
        QFont font(fontFamily());
        font.setPixelSize(qMax(1, rect.height()));
        font.setStyleStrategy(QFont::PreferAntialias);

        painter->save();
        painter->setRenderHint(QPainter::TextAntialiasing);
        painter->setFont(font);
        painter->setPen(color(mode));
        painter->drawText(rect, Qt::AlignCenter, QString(QChar(char16_t(m_glyph))));
        painter->restore();
    }

    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override
    {
        return scaledPixmap(size, mode, state, 1.0);
    }

    QPixmap scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale) override
    {
        QPixmap pixmap(size * scale);
        pixmap.fill(Qt::transparent);
        {
            QPainter painter(&pixmap);
            paint(&painter, QRect(QPoint(0, 0), pixmap.size()), mode, state);
        }
        pixmap.setDevicePixelRatio(scale);
        return pixmap;
    }

    QIconEngine *clone() const override { return new GlyphIconEngine(m_glyph); }

private:
    static QColor color(QIcon::Mode mode)
    {
        const Theme::Colors &colors = Theme::colors();
        switch (mode) {
        case QIcon::Disabled: return colors.iconDisabled;
        case QIcon::Active: return colors.iconActive;
        default: return colors.iconNormal;
        }
    }

    Icon m_glyph;
};

} // namespace

QIcon icon(Icon glyph)
{
    return QIcon(new GlyphIconEngine(glyph));
}

} // namespace FontAwesome
