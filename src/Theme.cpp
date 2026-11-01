#include "Theme.h"

#include <QGuiApplication>
#include <QStyleHints>

#if defined(Q_OS_LINUX)
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusVariant>
#endif

namespace {

// True when the system uses the dark color scheme.
bool isDark()
{
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
}

} // namespace

namespace Theme {

const Colors &colors()
{
    // Light windows of Windows 11 (Fluent: window background #f3f3f3, primary text #1b1b1b,
    // secondary #616161, disabled #a0a0a0); the view background is a neutral gray under
    // the photo, Windows 11 defines none.
    static const Colors light = {
        QColor(0xf3, 0xf3, 0xf3),  // panelBackground
        QColor(0xaa, 0xaa, 0xaa),  // viewBackground
        QColor(0x1b, 0x1b, 0x1b),  // text
        QColor(0x61, 0x61, 0x61),  // secondaryText
        QColor(0x61, 0x61, 0x61),  // iconNormal
        QColor(0x1b, 0x1b, 0x1b),  // iconActive
        QColor(0xa0, 0xa0, 0xa0),  // iconDisabled
        QColor(0xf3, 0xf3, 0xf3),  // overlayBackground
    };
    // Dark windows of Windows 11; a dark gray (not black) view background distorts the
    // brightness of photos less.
    static const Colors dark = {
        QColor(0x20, 0x20, 0x20),  // panelBackground
        QColor(0x3a, 0x3a, 0x3a),  // viewBackground
        QColor(0xc8, 0xc8, 0xc8),  // text
        QColor(0xa0, 0xa0, 0xa0),  // secondaryText
        QColor(0xc8, 0xc8, 0xc8),  // iconNormal
        QColor(0xff, 0xff, 0xff),  // iconActive
        QColor(0x5a, 0x5a, 0x5a),  // iconDisabled
        QColor(0x20, 0x20, 0x20),  // overlayBackground
    };
    return isDark() ? dark : light;
}

QString css(const QColor &color, int alpha)
{
    return QStringLiteral("rgba(%1, %2, %3, %4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(alpha);
}

} // namespace Theme

#if defined(Q_OS_LINUX)
namespace {

// The color scheme of the desktop from the XDG desktop portal (GNOME, KDE and others):
// org.freedesktop.appearance color-scheme, 0 = no preference, 1 = dark, 2 = light.
//
// Qt does not follow it reliably with the GTK 3 platform theme used on GNOME (Qt 6.12): at
// start it takes the scheme from the GTK theme, which stays "Adwaita" (light) in the dark
// mode of GNOME, and when the portal setting changes it reports the new scheme but keeps
// the light palette. Requesting the scheme with QStyleHints::setColorScheme() switches the
// GTK theme to its dark variant, so the palette of the standard widgets follows as well.
class PortalColorScheme : public QObject
{
    Q_OBJECT

public:
    explicit PortalColorScheme(QObject *parent) : QObject(parent)
    {
        QDBusConnection::sessionBus().connect(kService, kPath, kInterface,
            QStringLiteral("SettingChanged"), this, SLOT(settingChanged(QString,QString,QDBusVariant)));
        apply(read());
    }

private slots:
    void settingChanged(const QString &nameSpace, const QString &key, const QDBusVariant &value)
    {
        if (nameSpace == kNamespace && key == kKey)
            apply(value.variant());
    }

private:
    static constexpr QLatin1StringView kService{"org.freedesktop.portal.Desktop"};
    static constexpr QLatin1StringView kPath{"/org/freedesktop/portal/desktop"};
    static constexpr QLatin1StringView kInterface{"org.freedesktop.portal.Settings"};
    static constexpr QLatin1StringView kNamespace{"org.freedesktop.appearance"};
    static constexpr QLatin1StringView kKey{"color-scheme"};

    // ReadOne (portal version 2) returns the value, the older Read (e.g. Ubuntu 22.04) the
    // value wrapped in one more variant.
    static QVariant read()
    {
        for (const auto &method : {QStringLiteral("ReadOne"), QStringLiteral("Read")}) {
            QDBusMessage call = QDBusMessage::createMethodCall(kService, kPath, kInterface, method);
            call << QString(kNamespace) << QString(kKey);
            const QDBusMessage reply = QDBusConnection::sessionBus().call(call, QDBus::Block, 1000);
            if (reply.type() != QDBusMessage::ReplyMessage || reply.arguments().isEmpty())
                continue;
            QVariant value = reply.arguments().constFirst();
            while (value.canConvert<QDBusVariant>())
                value = qvariant_cast<QDBusVariant>(value).variant();
            return value;
        }
        return {};
    }

    static void apply(const QVariant &value)
    {
        bool ok = false;
        const uint scheme = value.toUInt(&ok);
        if (!ok)
            return;  // no portal: leave the scheme to Qt
        QStyleHints *hints = QGuiApplication::styleHints();
        if (scheme == 1)
            hints->setColorScheme(Qt::ColorScheme::Dark);
        else if (scheme == 2)
            hints->setColorScheme(Qt::ColorScheme::Light);
        else
            hints->unsetColorScheme();
    }
};

} // namespace
#endif

namespace Theme {

void followSystemColorScheme()
{
#if defined(Q_OS_LINUX)
    new PortalColorScheme(qApp);
#endif
}

} // namespace Theme

#if defined(Q_OS_LINUX)
#include "Theme.moc"
#endif
