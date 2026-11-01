#include "UpdateChecker.h"

#include "MessageBoxes.h"
#include "Settings.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDialog>
#include <QElapsedTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressBar>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QVersionNumber>
#include <QXmlStreamReader>

#include <algorithm>

namespace {

const char kLatestVersionInfoUrl[] = "https://www.kuk360.com/LatestVersionInfo.xml";
constexpr int kMinWaitMs = 1000;        // the wait dialog stays at least this long, so it does not flash
constexpr int kTransferTimeoutMs = 30000;
constexpr int kReminderIntervalDays = 14;

struct LatestVersionInfo
{
    QVersionNumber version;
    QUrl downloadUrl;
};

// Parses <LatestVersionInfo><Version>2.1.0</Version><DownloadUrl>...</DownloadUrl></LatestVersionInfo>.
bool parseLatestVersionInfo(const QByteArray &xml, LatestVersionInfo *info)
{
    QXmlStreamReader reader(xml);
    if (!reader.readNextStartElement() || reader.name() != QLatin1String("LatestVersionInfo"))
        return false;
    while (reader.readNextStartElement()) {
        if (reader.name() == QLatin1String("Version"))
            info->version = QVersionNumber::fromString(reader.readElementText().trimmed());
        else if (reader.name() == QLatin1String("DownloadUrl"))
            info->downloadUrl = QUrl(reader.readElementText().trimmed(), QUrl::StrictMode);
        else
            reader.skipCurrentElement();
    }
    return !reader.hasError() && !info->version.isNull();
}

// Modal "Please wait..." window with an indeterminate (busy) progress bar.
class WaitDialog : public QDialog
{
public:
    explicit WaitDialog(QWidget *parent)
        : QDialog(parent)
    {
        setWindowTitle(QCoreApplication::translate("UpdateChecker", "Please wait..."));
        auto *progress = new QProgressBar;
        progress->setRange(0, 0);  // busy indicator
        progress->setTextVisible(false);
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(12, 12, 12, 12);
        layout->addWidget(progress);
        setFixedSize(420, sizeHint().height());
    }
};

QString tr(const char *text)
{
    return QCoreApplication::translate("UpdateChecker", text);
}

} // namespace

void checkForUpdates(QWidget *parent)
{
    Settings::setLastUpdateCheck(QDateTime::currentDateTimeUtc());

    QNetworkAccessManager network;
    QNetworkRequest request(QUrl(QString::fromLatin1(kLatestVersionInfoUrl)));
    request.setTransferTimeout(kTransferTimeoutMs);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("%1/%2").arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion()));
    QNetworkReply *reply = network.get(request);  // owned (and deleted) by network

    // The dialog closes when the download is finished, but not sooner than kMinWaitMs.
    WaitDialog dialog(parent);
    QElapsedTimer elapsed;
    elapsed.start();
    QObject::connect(reply, &QNetworkReply::finished, &dialog, [&dialog, &elapsed] {
        const int remaining = std::max(0, kMinWaitMs - int(elapsed.elapsed()));
        QTimer::singleShot(remaining, &dialog, &QDialog::accept);
    });
    if (dialog.exec() != QDialog::Accepted) {  // closed by the user: cancel the check
        reply->abort();
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        MessageBoxes::showWarning(parent, tr("Unable to check for updates:\n%1").arg(reply->errorString()));
        return;
    }
    LatestVersionInfo info;
    if (!parseLatestVersionInfo(reply->readAll(), &info)) {
        MessageBoxes::showWarning(parent, tr("Unable to check for updates:\nInvalid version information."));
        return;
    }

    const QVersionNumber current = QVersionNumber::fromString(QCoreApplication::applicationVersion());
    if (info.version <= current) {
        MessageBoxes::showInformation(parent, tr("Application is up to date."));
        return;
    }

    if (!MessageBoxes::askYesNo(parent, tr("Application update is available.\nDo you want to open the downloads page?")))
        return;
    // Only an absolute http(s) URL may be opened, never e.g. a local file or a program.
    const QString scheme = info.downloadUrl.scheme().toLower();
    if (!info.downloadUrl.isValid() || info.downloadUrl.isRelative()
        || (scheme != QLatin1String("http") && scheme != QLatin1String("https"))) {
        MessageBoxes::showWarning(parent, tr("Unable to open the downloads page:\nInvalid update URL."));
        return;
    }
    QDesktopServices::openUrl(info.downloadUrl);
}

void remindToCheckForUpdates(QWidget *parent)
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    const QDateTime last = Settings::lastUpdateCheck();
    // A time in the future (the clock was wrong) counts as no check at all, otherwise the
    // reminder could stay away for a very long time.
    if (!last.isValid() || last > now) {
        Settings::setLastUpdateCheck(now);
        return;
    }
    if (last.addDays(kReminderIntervalDays) > now)
        return;

    Settings::setLastUpdateCheck(now);
    if (MessageBoxes::askYesNo(parent, tr("It has been a while since %1 last checked for updates.\n"
                                          "Do you want to check now?").arg(QCoreApplication::applicationName())))
        checkForUpdates(parent);
}
