#pragma once

class QWidget;

// Help > Check for updates: downloads kLatestVersionInfoUrl (an XML file with the latest
// <Version> and its <DownloadUrl>) while a modal "Please wait..." dialog is shown for at
// least one second, compares the version with the running application and tells the user
// the result. When a newer version exists, the user may open the downloads page in the
// browser; downloading and installing is up to the user. Closing the wait dialog cancels
// the check. Every check, also a failed or cancelled one, is remembered in Settings.
void checkForUpdates(QWidget *parent);

// Called at start: when the last check (or the last reminder) is older than two weeks, asks
// the user whether to check for updates now. Nothing goes to the network without the user's
// consent. The time is remembered whatever the answer, so the user is reminded again only
// after another two weeks. The first start only remembers the time, the user has just
// downloaded the application.
void remindToCheckForUpdates(QWidget *parent);
