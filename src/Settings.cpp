#include "Settings.h"

#include <QSettings>

namespace {

const auto kLastDirectoryKey = QStringLiteral("lastDirectory");
const auto kLastUpdateCheckKey = QStringLiteral("lastUpdateCheck");
const auto kWindowGeometryKey = QStringLiteral("windowGeometry");

} // namespace

namespace Settings {

QString lastDirectory()
{
    return QSettings().value(kLastDirectoryKey).toString();
}

void setLastDirectory(const QString &directory)
{
    QSettings().setValue(kLastDirectoryKey, directory);
}

QDateTime lastUpdateCheck()
{
    // An ISO 8601 string instead of a QDateTime value, which the native formats (registry,
    // plist) do not store the same way.
    return QDateTime::fromString(QSettings().value(kLastUpdateCheckKey).toString(), Qt::ISODate);
}

void setLastUpdateCheck(const QDateTime &time)
{
    QSettings().setValue(kLastUpdateCheckKey, time.toUTC().toString(Qt::ISODate));
}

QByteArray windowGeometry()
{
    return QSettings().value(kWindowGeometryKey).toByteArray();
}

void setWindowGeometry(const QByteArray &geometry)
{
    QSettings().setValue(kWindowGeometryKey, geometry);
}

} // namespace Settings
