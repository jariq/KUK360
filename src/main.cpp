#include "CompactMenuBarStyle.h"
#include "MainWindow.h"
#include "Theme.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QImageReader>
#include <QSurfaceFormat>

#if defined(Q_OS_WIN)
#include <QtGui/private/qguiapplication_p.h>  // QNativeInterface::Private::QWindowsApplication
#elif defined(Q_OS_MACOS)
#include <QSettings>
#endif

int main(int argc, char *argv[])
{
    QSurfaceFormat format;
    format.setDepthBufferSize(0);
    format.setStencilBufferSize(0);
    format.setSwapInterval(1);
    QSurfaceFormat::setDefaultFormat(format);

    // Name of the installed .desktop file. On Wayland the window icon below is ignored and the
    // compositor takes the icon from the .desktop file that matches this name. It must be set
    // before QApplication, which registers it with the XDG desktop portal; setting it later
    // registers the connection a second time and the portal rejects it.
    QGuiApplication::setDesktopFileName(QStringLiteral("com.kuk360.KUK360"));

    // Organization and application name identify where QSettings stores the settings (see
    // Settings.h; on macOS the domain gives the bundle identifier). Set before QApplication,
    // as the macOS defaults below already use them.
    QCoreApplication::setOrganizationName(QStringLiteral("KUK360"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kuk360.com"));
    QCoreApplication::setApplicationName(QStringLiteral("KUK360"));

#if defined(Q_OS_MACOS)
    // macOS adds AutoFill, Start Dictation and Emoji & Symbols to every menu titled "Edit" and
    // "Enter Full Screen" to the "View" menu (F12 has its own full screen mode) unless these
    // user defaults of the application are set. QSettings writes them to the preferences of
    // the application; AppKit reads them when it builds the menus after QApplication.
    QSettings appDefaults;
    appDefaults.setValue(QStringLiteral("NSAutoFillSystemInsertMenuEnabled"), false);
    appDefaults.setValue(QStringLiteral("NSDisabledDictationMenuItem"), true);
    appDefaults.setValue(QStringLiteral("NSDisabledCharacterPaletteMenuItem"), true);
    appDefaults.setValue(QStringLiteral("NSFullScreenMenuItemEverywhere"), false);
    appDefaults.sync();
#endif

    QApplication app(argc, argv);
    QApplication::setApplicationVersion(QStringLiteral(KUK360_VERSION));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/src/KUK360.svg")));
    Theme::followSystemColorScheme();
    CompactMenuBarStyle::install();

#if defined(Q_OS_WIN)
    // A full screen window containing a QOpenGLWidget bypasses the DWM composition, so menus
    // and dialogs (e.g. the context menu) would stay hidden behind it. A 1 px border in full
    // screen avoids that ("Fullscreen OpenGL Based Windows" in the Qt for Windows docs).
    if (auto *windowsApp = app.nativeInterface<QNativeInterface::Private::QWindowsApplication>())
        windowsApp->setHasBorderInFullScreenDefault(true);
#endif

    // Large panoramas can exceed Qt's default 256 MB decode limit (e.g. 24000 x 12000 px
    // needs about 1.1 GB, as JPEGs are decoded with 4 bytes per pixel).
    QImageReader::setAllocationLimit(4096);

    // Optional image file, e.g. from "Open with" in the file manager (Windows, Linux; macOS
    // sends a QFileOpenEvent instead, handled by MainWindow).
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Simple 360 photo viewer"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("file"), QStringLiteral("Image file to open."),
                                 QStringLiteral("[file]"));
    parser.process(app);

    MainWindow window;
    window.restoreWindowGeometry();
    window.show();

    // Opened once the event loop runs and the application is active (see MainWindow).
    const QStringList files = parser.positionalArguments();
    window.startWhenActive(files.isEmpty() ? QString() : files.first());

    return app.exec();
}
