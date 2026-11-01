# KUK360

Simple 360° photo viewer in C++17 / Qt 6 / OpenGL for Windows, macOS and Linux. The version is in `CMakeLists.txt` (`project(KUK360 VERSION ...)`) and reaches the code as `KUK360_VERSION`.

It opens JPEG, PNG, BMP, GIF (first frame), TIFF and WebP from the file dialog, the command line (`KUK360 [file]`, used by "Open with"), drag & drop and, on macOS, `QFileOpenEvent`. A JPEG with GPano XMP `ProjectionType` `equirectangular` opens in the 360° view, everything else in the flat view.

## Working rules
- Code, comments, commit messages and everything else is in English.
- Development alternates between Windows, macOS and Linux. Run `git pull` before starting work. Commit only after jariq approves it, push only when asked.
- With every new feature or decision, update this file, update `README.md` when features, shortcuts or panel buttons change, and add the checks of a new feature to `TESTING.md`. This file holds what further development needs: how things work, why, and the traps. History belongs in git and `CHANGELOG.md`.
- The behavior in "Intentional behavior" was decided by jariq. Don't change it without asking.
- Only Qt and standard C++, no new dependencies without agreement. Platform-specific code only where unavoidable, behind `#if defined(Q_OS_...)`.
- Qt style (`m_` members, `QStringLiteral`). `-Wall -Wextra` with warnings as errors; Qt headers are `-isystem`. `--compile-no-warning-as-error` on the configure command line gets around a new compiler warning until the code is fixed.

## Repository layout
- `src/`: all sources, GLSL shaders, the Font Awesome 4.7 font and the window icon `KUK360.svg`.
- `build/<windows|macos|linux>/`: everything that belongs to one platform (`KUK360_PLATFORM_DIR` in `CMakeLists.txt`): build scripts, `tools/`, `resources/`, `licenses/`, on Windows and macOS `icons/` (Linux installs `src/KUK360.svg` as `com.kuk360.KUK360.svg`), on Windows `dotnet-tools.json`, on Linux `container/`.
  - `buildroot/<release|debug>` (CMake binary dir) and `output/<release|debug>` (install dir) are generated and ignored by git.
- `img/`: `KUK360-Photo-Sample.jpg` (real panorama with GPano XMP, the test image) and README images.
- `LICENSE`: GPL-3.0-only, the unmodified plain license text so GitHub recognizes and shows it. Installed as `LICENSE.txt`.
- `THIRD_PARTY_NOTICES.txt` (plain text): the GPL notice of KUK360 and all third-party components KUK360 uses (Qt LGPL-3.0, Pannellum MIT, Font Awesome OFL 1.1) and where the Qt sources are. Installed next to `LICENSE.txt` on every platform, the license texts of each distribution are in `build/<platform>/licenses`.

## Building
Common to all platforms:
- Qt 6.12.0 from the Qt online installer, with its CMake and Ninja: `C:\Qt\Qt6.12.0` (MinGW kit, no MSVC), `~/Qt/Qt6.12.0` on macOS and Linux. The presets in `CMakePresets.json` (`mingw-*`, `macos-*`, `linux-*`) point there.
- Qt modules: Widgets, OpenGL, OpenGLWidgets, Network. On Linux also DBus, on Windows also `GuiPrivate` (headers only). The Qt installer component Qt Image Formats is required too (`qtiff`, `qwebp` plugins).
- `cmake --install` creates a self-contained distribution through `qt_generate_deploy_app_script`. With `KUK360_BUILD_PACKAGES=ON` (Windows, macOS) it also builds the packages. Signing doesn't depend on it: Windows signs whenever `KUK360_SIGN_CERT_SHA1` is set, macOS always signs (ad hoc without an identity) and notarizes whenever `KUK360_NOTARY_PROFILE` is set. The deploy script needs an absolute prefix, so `installDir` is set in the presets.
- After changing the generator or the kit, delete `buildroot/<type>`, the CMake cache keeps the old one.
- Release scripts rebuild from scratch (delete `output/release` and `buildroot/release`) and run from any directory:
  - `build/windows/build.bat` and `build/macos/build.sh` build only the application (unsigned, resp. ad hoc signed) into `output/release`, where it runs by double-click. For development and contributors: they need nothing but Qt.
  - `build/windows/build-dist.bat` and `build/macos/build-dist.sh` build the distribution for publishing: they set `KUK360_BUILD_PACKAGES=ON` and the signing variables in the environment and call the scripts above. With `KUK360_SIGN_CERT_SHA1=none`, resp. `KUK360_SIGN_IDENTITY=none`, they build unsigned (ad hoc) packages, e.g. to test the installer.
  - On Linux the AppImage is both the application and the distribution and is always built. `build/linux/build.sh` builds it on the host (it then needs the host's glibc or newer), for development and contributors. `build/linux/build-dist.sh` runs `build.sh` in the AlmaLinux 9 container with the glibc limits for publishing.
- **Qt upgrade checklist:** presets, `QT_ROOT` in the build scripts (also `build/linux/build-dist.sh`) and the default Qt paths in the three `qt-third-party-notices.sh`; the "Qt 6.12 / glibc 2.34" comments in the Linux `Containerfile` and `build-dist.sh`; private `QWindowsApplication::setHasBorderInFullScreenDefault()` still exists; menu bar margins in `CompactMenuBarStyle`; plugin lists and system libraries in the Linux `Containerfile`; `Qt/LICENSE` of every platform; rerun the `qt-third-party-notices.sh` of every platform.

### Windows (MinGW)
```powershell
$env:PATH = "C:\Qt\Qt6.12.0\Tools\mingw1310_64\bin;C:\Qt\Qt6.12.0\6.12.0\mingw_64\bin;C:\Qt\Qt6.12.0\Tools\CMake_64\bin;" + $env:PATH
cmake --preset mingw-release          # or mingw-debug
cmake --build build/windows/buildroot/release
cmake --install build/windows/buildroot/release   # -> build/windows/output/release/KUK360
```
- The exe in `buildroot` needs Qt `bin` in `PATH`. `output/<type>/KUK360` runs without it.
- A running `KUK360.exe` makes linking and installing fail ("Permission denied") and `build.bat` stops.
- Flat layout: `CMAKE_INSTALL_BINDIR` is `.`. It must be set **before** `find_package(Qt6)`, because Qt generates `QtDeploySupport.cmake` with the target directories during it.
- The distribution is trimmed by windeployqt options (`DEPLOY_TOOL_OPTIONS`): no D3D/DXC shader compilers, touch `generic`, `networkinformation`, `qcertonlybackend` and image formats we don't support. The software OpenGL renderer `opengl32sw.dll` stays on purpose (fallback in virtual machines and over remote desktop). A feature that needs another plugin must change the options and the license notices. Installing is incremental, delete `output/<type>/KUK360` after removing Qt modules.
- `.bat` files must have CRLF, `.sh` files LF (`.gitattributes`), `cmd.exe` misreads labels with LF.
- `resources/KUK360.rc.in`: exe icon and `VERSIONINFO`. The manifest is `resources/KUK360.exe.manifest` (asInvoker, Windows 10/11, longPathAware; DPI awareness is left to Qt). Qt embeds a manifest only with MSVC, so with MinGW ours goes into the `.rc` file through `@KUK360_RC_MANIFEST@` (RT_MANIFEST 1). Listing it as a target source also stops Qt from generating its own.
- **MSI** (`resources/KUK360.wxs`, `tools/msi.cmake`, WiX 7), built with the zip only by `build-dist.bat` (`KUK360_BUILD_PACKAGES`):
  - WiX is a local .NET tool (`build/windows/dotnet-tools.json`) and needs the .NET SDK. Each user accepts its EULA once: `dotnet tool run wix eula accept wix7` in `build/windows`.
  - Per-machine, x64, by default `C:\Program Files\KUK360` (the user can change it, `WixUI_InstallDir`). It upgrades the old 32-bit 2.x installs: **`UpgradeCode` `EDE8C2FD-D43A-4F9A-8BE6-39B675CE0A8B` must never change.**
  - Files are included by a wildcard. `KUK360.exe` is its own component, the Start menu shortcut and the registry refer to it.
  - Registers "Open with" (ProgIDs, `OpenWithProgids`, `Applications\KUK360.exe`, `Capabilities`) for all supported formats, but never makes KUK360 the default application.
  - The license page is RTF generated from `LICENSE`.
  - User settings in HKCU stay after uninstall, like with other applications.
- **Portable zip**: the same `KUK360` folder packed by `cmake -E tar --format=zip`. Settings still go to the registry.
- **Signing** (Authenticode, SHA-256, RFC 3161 timestamp):
  - Enabled by the cache variable `KUK360_SIGN_CERT_SHA1` (empty = unsigned). `build-dist.bat` sets the author's code signing certificate. The timestamp server is `KUK360_SIGN_TIMESTAMP_URL` (default `http://time.certum.pl/`).
  - `tools/sign.cmake` signs in one `signtool` call every `.exe` and `.dll` without a valid signature (`KUK360.exe` and the MinGW runtime; Qt files are signed by The Qt Company), before the MSI and the zip are built, and the MSI afterwards.
  - Check: `Get-AuthenticodeSignature <file>`.

### macOS
```sh
export PATH="$HOME/Qt/Qt6.12.0/Tools/CMake/CMake.app/Contents/bin:$PATH"
cmake --preset macos-release          # or macos-debug
cmake --build build/macos/buildroot/release
cmake --install build/macos/buildroot/release   # -> build/macos/output/release/KUK360.app
```
- Universal (arm64 + x86_64), deployment target 14.4 (minimum of Qt 6.12). Compiler is Apple clang from Xcode, no Homebrew.
- `macdeployqt` cannot exclude plugins, so `install(CODE)` deletes the unneeded ones afterwards (virtual keyboard input context together with the QML, Quick and VirtualKeyboard frameworks it pulls in, `networkinformation`, unused TLS backends and image formats, including `qmacheif` and `qmacjp2`).
- `build.sh` fails if KUK360 runs from `output/release`. macOS doesn't lock the files, and the running copy would end up with an invalid signature.
- `resources/Info.plist.in`: identifier `com.kuk360.KUK360`, `CFBundleDocumentTypes` for the supported formats with role Viewer/Alternate ("Open With" without becoming the default).
- **Signing and notarization:**
  - Cache variables `KUK360_SIGN_IDENTITY` (Developer ID Application certificate in the keychain; empty = ad hoc) and `KUK360_NOTARY_PROFILE` (`notarytool` keychain profile, created by `xcrun notarytool store-credentials`; empty = not notarized). `build-dist.sh` sets the author's certificate and the profile `KUK360`. `KUK360_NOTARY_PROFILE=none` signs without notarization. `build.sh` drops the profile without an identity, and CMake stops if a profile is set without one.
  - `tools/sign.cmake` signs from the inside out (plugins, frameworks, bundle) instead of `--deep`. A Developer ID signature has the hardened runtime and a timestamp. No entitlements are needed.
  - `tools/notarize.cmake` submits with `notarytool submit --wait` and staples. On rejection it prints the notarization log and stops the build.
  - The app is notarized before it goes into the DMG, then the DMG is signed and notarized on its own. A ticket stapled only to the DMG doesn't reach the app copied out of it.
  - Check: `spctl --assess --type execute -vv KUK360.app`, `xcrun stapler validate <file>`, `codesign -dvv`.
- **DMG** (`tools/dmg.cmake`, only by `build-dist.sh`, `KUK360_BUILD_PACKAGES`): `ditto` into a staging folder with an `Applications` link, `hdiutil create` (UDZO). With an ad hoc identity the DMG stays unsigned.

### Linux (AppImage)
```bash
build/linux/build.sh                  # AppImage built on the host -> build/linux/output/release
build/linux/build-dist.sh             # AppImage for publishing, built in an AlmaLinux 9 container
build/linux/tools/test-distros.sh     # tests the AppImage on Ubuntu 22.04 and Debian 12
```
Development build on the host (the result needs the host's glibc):
```bash
export PATH="$HOME/Qt/Qt6.12.0/Tools/CMake/bin:$PATH"
cmake --preset linux-debug
cmake --build build/linux/buildroot/debug
build/linux/buildroot/debug/KUK360
```
- Host packages (Fedora): for `build.sh` `gcc-c++`, `libglvnd-devel` (OpenGL and EGL headers, required by the CMake package of Qt GUI), `squashfs-tools` and the bundled XCB libraries (`xcb-util-*`, a configure error without them; Fedora 44 has them only through its own Qt). The README lists them for Debian/Ubuntu too, checked in the GNOME-like test images. `podman` for `build-dist.sh`; `ImageMagick` and `appstream` for `test-distros.sh`.
- **Distribution build in a container** (`build-dist.sh`, `container/Containerfile`, podman): the container (`almalinux:9.8`, with `mesa-libGL-devel`, `mesa-libEGL-devel` and `binutils` for the glibc check) runs `build.sh` with `KUK360_MAX_GLIBC=2.34` and `KUK360_MAX_GLIBCXX=3.4.29` in the environment. AlmaLinux 9 has glibc 2.34 and GCC 11, the same minimum as the Qt 6.12 binaries. The AppImage then runs on RHEL 9+, Ubuntu 22.04+, Debian 12+ and similar.
  - Qt and the repository are mounted at the same paths as on the host, so the presets and the CMake cache work in both. `--userns=keep-id` keeps file ownership, `--security-opt label=disable` avoids relabeling for SELinux.
  - The image contains the system libraries needed by the bundled Qt libraries and plugins (`NEEDED` from `objdump -p`). Check the list when plugins change. The image tag follows the base version (`IMAGE` in `build-dist.sh`).
  - `appimage.cmake` stops the build if any binary needs a newer glibc or libstdc++ than `KUK360_MAX_GLIBC` / `KUK360_MAX_GLIBCXX`. Building on Fedora 44 would require glibc 2.43.
- `OpenGL_GL_PREFERENCE` is `LEGACY`: link `libGL.so.1`, because Ubuntu 22.04 and Debian 12 don't have GLVND `libOpenGL.so.0` by default.
- **Deploy:** the generic Qt deploy script, which doesn't copy system libraries.
  - `install(CODE)` switches `CMAKE_INSTALL_PREFIX` to `KUK360.AppDir/usr` around the script, so `qt.conf` and RPATH are relative to `usr`. `CMAKE_INSTALL_LIBDIR` is `lib` and is set before `find_package(Qt6)`, as on Windows.
  - Wayland is added by `INCLUDE_PLUGINS qwayland`. `KUK360_DEPLOY_OPTIONS` excludes what Windows and macOS exclude, plus eglfs, the QML debugger, Lottie (`vectorimageformats`), the Wayland shells other than `xdg-shell` and the Wayland server buffer integrations.
  - Never use `POST_INCLUDE_REGEXES` or similar options of the deploy script: Qt breaks the regular expressions when forwarding them and copies all system libraries, glibc included.
- **Bundled XCB libraries** (`libxcb-cursor`, `-icccm`, `-image`, `-keysyms`, `-render-util`, `-shape`, `-util`, `-xkb`): `qxcb` needs them and Ubuntu 22.04 and Debian 12 lack them, so KUK360 wouldn't start in an X11 session.
  - The real file is copied under its soname (`file(REAL_PATH)` + `RENAME`). `find_file(... NO_CACHE)` in a loop needs `unset()` before each search.
  - **Never bundle `libxkbcommon-x11`.** It must match the system `libxkbcommon` exactly, a mismatch crashed at startup.
- **AppImage** is built by every install, without `appimagetool` (`tools/appimage.cmake`): root symlinks (`AppRun` → `usr/bin/KUK360`, desktop file, icon, `.DirIcon`), `mksquashfs` with zstd, prepended runtime. `KUK360_BUILD_PACKAGES` doesn't exist on Linux.
  - The runtime is the pinned `type2-runtime` release (`KUK360_APPIMAGE_RUNTIME_VERSION`), downloaded at configure time to `build/linux/buildroot/appimage-runtime-<version>-x86_64` (shared by debug and release, downloaded again only when the hash differs) and checked by SHA-256 (`KUK360_APPIMAGE_RUNTIME_SHA256`). Change the version and the hash together.
  - x86_64 only. The running process is called `AppRun`.
- **Desktop integration:** `resources/com.kuk360.KUK360.desktop` and `QGuiApplication::setDesktopFileName("com.kuk360.KUK360")`.
  - `setDesktopFileName()` must be called **before** the `QApplication` constructor. A later call registers the app ID with the portal a second time and the portal rejects it.
  - Without an installed desktop file, Qt prints "Failed to register with host portal". That's expected for the AppImage and `buildroot`. On Wayland the window icon comes from the desktop file, so a non-integrated AppImage has a generic icon. Integration is left to tools like Gear Lever.
- **AppStream metadata** (`resources/com.kuk360.KUK360.metainfo.xml`) is a deliberate minimum. Validate with `appstreamcli validate --no-net` (`test-distros.sh` does it too). The pedantic notes `cid-contains-uppercase-letter` and `releases-info-missing` are expected. Flathub would need screenshots and releases.
- **`test-distros.sh`** runs the AppImage in containers that only have what a GNOME desktop has (`container/Containerfile.test`): `ldd` on all bundled binaries, start with the sample panorama in Xvfb, a screenshot and a check that the start page isn't shown. Screenshots and logs go to `output/release/tests`.

### Third-party licenses
- `cmake --install` puts `LICENSE.txt` and the root `THIRD_PARTY_NOTICES.txt` next to the application and `build/<platform>/licenses` into `licenses/`. The root notices are the only overview in the distribution; they name the components in the sources and Qt, and say that Qt is linked dynamically and unmodified and where its sources are, which the LGPL / GPL §6 requires next to the shipped Qt libraries.
- Each platform has a complete, standalone set in `build/<platform>/licenses`, with no files shared between platforms (copies are fine). There is no platform notices file, the license texts are complete and the directories name the components.
- Every component has its own directory (`licenses/Qt`, `Pannellum`, `Font-Awesome`, on Windows `GCC`, `MinGW-w64`, `winpthreads`, on Linux `ICU`, `XCB`) with the license files of its authors, copied unchanged under their original names (`COPYING`, `LICENSE`, ...), never renamed or merged.
  - Sources: `Qt/LICENSE` is `<Qt>/Licenses/LICENSE` of the Qt installer, `Pannellum/COPYING` is from the Pannellum release, the Windows `GCC` (all five files) and `winpthreads` are `<Qt>/Tools/mingw1310_64/licenses/<name>`. Of `mingw-w64` only `COPYING.MinGW-w64-runtime.txt` is shipped: its authors say in `COPYING.MinGW-w64.txt` that this file fulfills the license requirements of the runtime in binaries built with MinGW-w64, the other files cover MinGW-w64 itself. Font Awesome 4.7.0 has no license file, `Font-Awesome/OFL-1.1.txt` is its notice with the OFL text.
- The texts of the third-party code in Qt are one file per component in `licenses/Qt`, named after its attribution page in the Qt documentation (`qtcore-attribution-zlib.html` → `zlib.txt`), with the text of that page (Qt doesn't install the original license files of that code). They are generated by `build/<platform>/tools/qt-third-party-notices.sh` (bash; on macOS and Linux it needs `jq` and `perl`) from the attribution pages of the local Qt documentation. The script deletes all `*.txt` in `licenses/Qt` first; `LICENSE` stays.
  - macOS and Linux find the pages through the Qt SBOM (`<kit>/sbom/*.spdx.json`) from the binaries of the finished bundle or AppDir, run them after `cmake --install`. On Windows the list of pages is in the script and must be checked against the SBOM.
  - The XSVG text is in the scripts (its page is missing in the Qt 6.12 docs). On Linux the texts of the Wayland protocols are composed from the SBOM and the license files of the kit.
  - The macOS and Linux scripts fail if a binary is not in the SBOM, a component has no text or two files would get the same name. The Windows script fails on a missing page or a duplicate name.
- Linux `ICU/LICENSE`: the Qt installer ships ICU 73.2 without its license, the file is `icu4c/LICENSE` of the ICU tag `release-73-2`.
- Linux `XCB/<source package>/COPYING` (`libxcb`, `xcb-util`, `xcb-util-cursor`, `-image`, `-renderutil`, `-wm`) is from the AlmaLinux 9 source RPMs of the bundled libraries (`dnf download --source` in the build container, `rpm2cpio` on the host, the container has no `cpio`). `xcb-util-keysyms` has no license file, `keysyms.c-header.txt` is the unchanged license header of `keysyms/keysyms.c`. Check them when the AlmaLinux version changes.
- A new library or borrowed code needs a notice in `AboutDialog`, in the root `THIRD_PARTY_NOTICES.txt` (components of the sources and Qt) and its license files in `build/<platform>/licenses` of every affected distribution.

## Architecture
- `main.cpp`:
  - Organization `KUK360`, domain `kuk360.com` and application `KUK360` determine where `QSettings` stores data (Windows `HKCU\Software\KUK360\KUK360`, macOS `com.kuk360.KUK360.plist`, Linux `~/.config/KUK360/KUK360.conf`).
  - On macOS it writes `NSUserDefaults` keys before `QApplication` that remove AutoFill, Dictation and Emoji & Symbols from Edit and "Enter Full Screen" from View. The AutoFill key is undocumented.
  - `QImageReader` allocation limit is 4 GB, a 24000 × 12000 panorama needs about 1.1 GB.
  - `QCommandLineParser`. On Windows `--help` and `--version` show a message box, the application has no console.
  - `MainWindow::startWhenActive()` opens the file from the command line and then shows the update reminder, both in one function and only after the event loop runs and the application is active (`runWhenActive()`). On macOS, loading earlier blocked all input until another event arrived. Two separately queued functions would not keep the order: `processEvents()` in `openImage()` (the "Loading..." message) would run the second one nested, before the image is loaded.
- `MainWindow`: composes `MainActions`, `ImageViewer` and `ButtonPanel` and handles opening, Previous/Next, copy, delete, drag & drop and full screen. `ImageViewer::imageChanged()` goes to `updateForImage()` (actions, projection button icon, window title), `projectionChanged()` to `updateProjectionButton()`; the lock button has `updateLockProjectionButton()`. Images are read with `QImageReader::setAutoTransform(true)` (EXIF orientation). The "Loading..." message (`showLoadingMessage()`) needs both `processEvents(ExcludeUserInputEvents)` and `repaint()` to appear right after the file dialog closes. Long paths on the error page wrap because zero-width spaces follow the separators.
- `MainActions`: all `QAction`s with icons, texts and shortcuts. Fills the menu bar and the context menu, adds the actions to the window (shortcuts then work in full screen without the menu) and enables them (`setImageActionsEnabled(hasFile, hasImage)`). Context menu items are copies: they take the enabled state of their action when the menu opens and forward `triggered` to it. `exitFullScreenAction` (Escape) is in no menu and enabled only in full screen, so Escape stays free otherwise.
- `ButtonPanel`: the 9 buttons at the bottom, addressed by `enum class Button`. `setAction()` binds a button to an action: a click triggers it and the button is enabled with it.
- `ImageViewer`: a `QWidget` with a `QStackedWidget` (start page, error page `showError()`, `PanoramaView`, `FlatView`) and the `MessageOverlay` floating over it. It owns the image, the projection and the grid state. `currentView()` returns the active viewer as `ZoomableView` (or `nullptr`). The image goes to a viewer lazily, when it becomes active (`FlatView` converts it to a `QPixmap`, a copy). `setImage()` emits `projectionChanged()` before `imageChanged()`.
- `PanoramaView` (`QOpenGLWidget`): the 360° view, a port of Pannellum (see below). Shaders in `src/shaders`, GLSL 1.10 / ES 1.00.
- `FlatView` (`QGraphicsView` with `QGraphicsPixmapItem`): the flat view.
- `ZoomableView`: zoom interface of both viewers (`zoomIn()`, `zoomOut()`, `resetZoom()`). Each viewer has its own `zoomChanged(double)` signal, the interface is not a `QObject`.
- `PanoramaDetector` (`isEquirectangularPanorama()`): walks the JPEG markers up to SOS and matches `ProjectionType` = `equirectangular` (case-insensitive, any namespace prefix) as an attribute or an element in each APP1 segment, so in standard and extended XMP. Extended XMP chunks are not joined, a value split between two chunks is missed.
- `ImageFolder`: the current file in its folder, Previous/Next, move to trash. `nameFilters()` is the only list of supported extensions.
- `MessageOverlay`: short messages above the image (fade-out, transparent for the mouse).
- `MessageBoxes`, `Settings`, `Theme`, `FontAwesome`, `CompactMenuBarStyle`, `FileLocation` (`showInFileManager()`), `UpdateChecker` (`checkForUpdates()`, `remindToCheckForUpdates()`), `AboutDialog`: see the rules below. `FileLocation`, `PanoramaDetector` and `UpdateChecker` are free functions, not classes.

## Code rules and traps
- **Message boxes** only through `MessageBoxes` (`showInformation()`, `showWarning()`, `showCritical()`, `askYesNo()`), never `QMessageBox` directly. The title is `QCoreApplication::applicationName()`, not `windowTitle()`.
- **Messages for the user** go through `MessageOverlay`, not message boxes, where possible.
- **Colors** of custom widgets are in `Theme::colors()` (light and dark set after Windows 11 Fluent) and are applied in `applyTheme()`, which runs again on `QStyleHints::colorSchemeChanged`. Never hard-code a color. Exceptions on purpose: the grid colors (`FlatView.cpp`, `panorama.frag`) and the black background of the 360° view (`PanoramaView.cpp`, `u_backgroundColor`), which don't follow the theme.
- **Settings** only through the `Settings` namespace (a getter/setter pair per setting, keys only in `Settings.cpp`).
- **Supported formats:** `ImageFolder::nameFilters()`. On a change also update `KUK360.wxs`, `Info.plist.in` and the `.desktop` file. Formats are read by Qt plugins (`qjpeg`, `qgif`, `qtiff`, `qwebp`), which must stay in all distributions.
- **Actions:** every function is an action, buttons only trigger actions (`clicked` → `QAction::trigger`; never `animateClick()`, it delays every shortcut by 100 ms). Never put two actions or a `QShortcut` on the same key, the shortcut becomes ambiguous and nothing fires. Panel actions and Reload have `setAutoRepeat(false)`. A tab in the action text replaces the displayed shortcut ("Left Arrow" instead of Qt's "Left").
- **Icons** are Font Awesome 4.7 glyphs drawn by a custom `QIconEngine` at any size and in the current `Theme` colors. A new icon goes alphabetically into `enum class Icon`.
- **Shared image:** viewers get the `QImage` by implicit sharing and must not modify it, it would be copied.
- **`PanoramaView` and OpenGL:**
  - The shader program is created in `initializeGL()` for each context. A program from another context fails and `reportGlError()` closes the app.
  - `cleanup()` is connected to `QOpenGLContext::aboutToBeDestroyed` and disconnected in the destructor, otherwise it runs twice (crash on exit on macOS and Linux).
  - `setImage()` and `clear()` release the textures immediately (`makeCurrent()`), a hidden view doesn't paint.
  - Without a second texture half, `u_image1` points to unit 0 (macOS complains about an empty unit).
  - Orientation (portrait/landscape) switches **before** `QOpenGLWidget::resizeEvent()`, which paints synchronously.
  - Control constants are named in the Constants section of `PanoramaView.h`.
  - `m_portrait` is set in `setImage()` too, not only in `resizeEvent()`.
  - `animate()` doesn't auto-rotate while the view is hidden, `showEvent()` starts the animation again.
  - If `GL_MAX_TEXTURE_SIZE` can't be queried (≤ 0), 2048 is used. A panorama bigger than 2 × max by max is scaled down with a `qWarning`.
- **`FlatView`:**
  - 100 % is one image pixel per device pixel: `actualSizeScale()` = 1 / `devicePixelRatioF()`. The view scale is in logical pixels. The fit-to-window limit and 400 % use it too, and `DevicePixelRatioChange` updates the scale.
  - Zoom anchoring is manual (`mapToScene` + `centerOn`). `AnchorUnderMouse` jumps when the image first outgrows the window.
  - `minScale()` and the scene rect use `sceneBoundingRect()` because of rotation.
  - `setAcceptDrops(false)`, otherwise `QGraphicsView` swallows drops meant for `MainWindow`.
  - `FullViewportUpdate` because of the grid, which is fixed to the window.
- **Touch** (`PanoramaView`): only `TouchScreen` devices are handled (macOS sends trackpad fingers too). Mouse events synthesized from touch are ignored (`isFromTouch()`), otherwise the first finger of a pinch also rotates. Pinch on the macOS trackpad comes as `QNativeGestureEvent`. `FlatView` uses `QPinchGesture`, which Qt produces from both, and, unlike `PanoramaView`, pans with one finger through the mouse events Qt synthesizes from touch (`ScrollHandDrag`).
- **`ImageFolder`:**
  - Natural, case-insensitive order (`QCollator` with numeric mode), like Explorer, Finder and Nautilus. Wraps around at the ends.
  - The current file is found case-insensitively, a path from the command line may differ in case from the directory entry.
  - The sorted list is cached (20 000 files: 1.6 s → 17 ms per Next). It is reloaded when the directory or its modification time changes, after `refresh()` (window activation, trash) and when the list is obviously wrong (FAT cards don't always update the directory time).
  - For a directory modified more recently than 2 s ago the list is stored with an invalid time, so the next query reads the directory again ("racy" times, as in git).
  - After a move to trash, the current file stays as the position, so `nextFile()` returns the following one.
- **Drag & drop:** always `Qt::CopyAction` (with Move, Explorer deletes the source). Processed by `QTimer::singleShot(0)` after `dropEvent()` returns, because the drag source waits.
- **Delete:** if another file was opened during the confirmation dialog (macOS delivers `QFileOpenEvent` even then), nothing is deleted. A directory is never the current file, and `moveCurrentFileToTrash()` refuses one anyway.
- **Copy:** `QMimeData::setUrls()` plus `x-special/gnome-copied-files` for GNOME. On Windows Qt provides the data lazily, a killed process loses the clipboard.
- **`FileLocation`:** Windows `explorer.exe /select,` through `setNativeArguments`, macOS `open -R`, Linux asynchronous D-Bus `org.freedesktop.FileManager1.ShowItems` with a fully encoded URI (`QUrl::toEncoded()`), falling back to opening the folder.
- **`Theme` on Linux:** Qt 6.12 with the `gtk3` theme doesn't follow the GNOME dark mode. `PortalColorScheme` reads `org.freedesktop.appearance color-scheme` from the settings portal (`ReadOne`, old portals `Read`), follows `SettingChanged` and calls `QStyleHints::setColorScheme()`. The GTK theme then switches to dark as well. Value 0 calls `unsetColorScheme()`, no or a non-numeric value leaves the scheme to Qt. The reads are blocking D-Bus calls with a 1 s timeout each, so without a portal the start can wait about 2 s.
- **`CompactMenuBarStyle`:** a `QProxyStyle` over `windows11` (only if that style is active) that makes the menu bar items smaller than the WinUI defaults. Drawing stays with `windows11`, `drawControl()` only widens the rect by 3 px per side and changes the option in place (`QMenuBar` passes a private subclass). After a Qt upgrade check that the text isn't clipped.
- **Full screen on Windows:** an OpenGL window in full screen bypasses DWM, and menus and dialogs would be hidden behind it. `setHasBorderInFullScreenDefault(true)` gives it a 1 px `WS_BORDER`. A screenshot doesn't show the problem, check the window style instead.
- **`UpdateChecker`:** downloads `https://www.kuk360.com/LatestVersionInfo.xml` (`Version`, `DownloadUrl`), compares with `QVersionNumber`, opens only absolute http/https URLs. The wait dialog stays at least 1 s, timeout 30 s. This file must be updated after each public release.
  - Every manual check and every reminder stores the time in `Settings::lastUpdateCheck()` (UTC, ISO 8601 string).
  - To test the reminder, set `lastUpdateCheck` in the settings to an older time, e.g. `2026-01-01T00:00:00Z` (Windows: `HKCU\Software\KUK360\KUK360`; macOS: `defaults write com.kuk360.KUK360 lastUpdateCheck -string ...`; Linux: `~/.config/KUK360/KUK360.conf`). On macOS never edit the plist file directly (Xcode, text editor): `cfprefsd` caches the settings and doesn't see the change. An invalid or future value only stores the current time without a question.

## Intentional behavior
Keyboard shortcuts and panel buttons are listed in `README.md`. Beyond that:
- **Window:** size, position and maximized state are restored from the last session (`saveGeometry()` on close), the first time 80 % of the screen. On Wayland only the size and the maximized state come back, the compositor places the window and doesn't let applications position it (checked on GNOME). Full screen is never restored: entering it saves the geometry before, closing in it keeps that. Minimum 750 × 450. Menu File/Edit/View/Help, viewer in the middle, panel at the bottom.
  - Panel height follows the window height: 96 px from a 900 px window, 72 px at 450 px, linear in between.
  - Buttons are 56 × 56 with 30 px icons, gaps between icons at most 41 px.
- **Open dialog** starts in the folder of the last opened file (saved), otherwise in Pictures, otherwise in home.
- **Saved settings** are only the last folder (on every open, also of an unreadable file or a directory), the window geometry and the time of the last update check. Nothing else is saved. On macOS the same preferences domain also holds the `NS*` menu keys written by `main.cpp`.
- **Shortcut choices:**
  - Rotate is L/R, not Ctrl+[ / Ctrl+] (AltGr on many layouts).
  - In 360, WASD look and E/Q zoom, for one-handed use. They do nothing in flat.
  - Left/Right arrows are reserved for Previous/Next.
  - Exit uses the platform's `QKeySequence::Quit` (Cmd+Q, Ctrl+Q on Linux), falling back to Alt+F4 (Windows).
  - Zoom in also has Ctrl+= (Plus without Shift on many layouts). Delete also has Cmd+Backspace on macOS (no forward delete key).
- **Projection:**
  - A file opens in 360 if detected as a panorama, otherwise flat. Detection is JPEG only, other formats are switched with F2.
  - Lock (F3) keeps the current projection for the next files, including Previous/Next and Reload. It is off by default, not saved, and its button is enabled even without a file.
  - The projection and lock buttons show the **current** state (globe/map, closed/open lock), not the state after clicking. The menu items have fixed icons and texts ("Switch projection", "Lock/unlock projection") and are not checkable.
  - UI names are "360°" and "flat". Every projection change and every opened file (also Reload and Previous/Next) shows "360° projection" or "Flat projection". The lock shows "Projection locked" / "Projection unlocked".
- **Zoom:** wheel, pinch, View > Zoom, in 360 also E/Q. "Zoom: N%" is shown on user zoom only.
  - 360: View > Zoom and Ctrl+Plus/Minus step by 5° hfov, the wheel (with inertia), pinch and E/Q zoom continuously. The percentage is `tan(initial / 2) / tan(hfov / 2)`, so Ctrl+0 is 100 % in both viewers.
  - Flat: steps of 1.25×, from fit-to-window (never above 100 %) to 400 %. A fitted image stays fitted on resize.
- **Rotate:**
  - 360: auto-rotation (Pannellum `autoRotate`), levels −10…+10 at 3°/s per level, with messages ("Rotation left: N", "Rotation right: N", "Rotation stopped"). Any interaction stops it. A new image resets it.
  - Flat: rotates the displayed image by 90° and fits it again, without a message. The file isn't changed.
- **Grid** (F4): shared by both viewers and kept for the next images. The button icon doesn't change.
  - Flat: lines every 40 logical px of the window, alternating #80ff00 / #53edf3, only over the image.
  - 360: lines every 10° fixed to the sphere, 1 px, in the same alternating colors, red equator, meridians end at ±80°.
- **Unreadable file:** becomes the current file and shows an error page ("Unable to load image", path, reader error) instead of a message box. File actions work, view actions don't. A **directory** shows an error page and leaves full screen, but never becomes the current file, so file actions stay disabled.
- **Enabling:** file actions (Reload, Close, Show in file manager, Copy, Delete, full screen) need an open file, view actions (projection, zoom, rotate, grid) a displayed image. Previous/Next need another supported file in the folder, checked also on window activation. Lock and Help are always enabled.
- **Context menu** (right click, only with an open file): all actions of the main menu except Help (Exit full screen isn't in any menu), with own texts and without shortcuts.
- **Delete:** confirmation (Yes/No, default Yes), then move to trash and open the next file, or go back to the start page. Texts show only the file name and the platform's trash name (`trashName()`: Recycle Bin / Trash).
- **Full screen** (F12, Escape): only the viewer, menu and panel are hidden, and the window returns to normal or maximized. On macOS the green title button zooms the window instead of the system full screen, which wouldn't hide the menu and panel.
- **Update reminder:** at start, when the last check is older than 14 days, a question (Yes/No, default Yes) asks whether to check for updates. The time is stored whatever the answer, so the next reminder comes after another 14 days. Nothing goes to the network without consent. The first start only stores the time, a stored time in the future counts as none. The question comes after the image from the command line is shown, never in full screen (then the next start asks).
- **Linux packaging:** the AppImage stays unsigned and without zsync, and the AppStream metadata stays minimal. Updates go through Help > Check for updates. Jariq signs released files of all platforms with GPG by hand.
- **macOS packaging:** DMG only (no zip, no pkg), without a custom background.

## 360° algorithm (Pannellum port, keep to it)
Reference: `src/js/libpannellum.js` and `src/js/pannellum.js` at https://github.com/mpetroff/pannellum.
- **Rendering:** a full-screen quad. The fragment shader ray-casts yaw and pitch for each pixel (`fragEquiCubeBase` + `fragEquirectangular`, our `sphereCoord()`). Uniforms: `u_psi` (yaw), `u_theta` (pitch), `u_f` (focal length from vfov), `u_h` / `u_v` / `u_vo` (image extent), `u_rot` (roll).
- **Textures:** no mipmaps (they create a seam at ±180°). Wrap S `Repeat` unless split or, on OpenGL ES, the size is not a power of two (GLES 2 limitation), then `ClampToEdge`. An image wider than `GL_MAX_TEXTURE_SIZE` is split into two halves (`u_splitImage`), and scaled down if that's not enough.
- **Coverage:** haov is always 360°, vaov = `min(180, 360·h/w)`, centered on the horizon.
- **Controls** (`pannellum.js`):
  - Mouse drag `onDocumentMouseMove`; inertia and keys `keyRepeat` with friction 0.15 and max speed 5.
  - Wheel `hfov -= delta * 0.05` (6° per notch) plus inertia, range 50–120, start 100.
  - Pitch is limited to ±90, yaw wraps at ±180.
- **Portrait** (our addition, from jariq's Pannellum fork, `resetHfov()`): when the view is taller than wide, hfov range is 25–75, start 50. The range switches only when the orientation changes (a square counts as landscape), so resizing keeps the zoom.
- **Touch:**
  - One finger drags like the mouse, not with Pannellum's touch formula, which handled worse on a real touchscreen.
  - Two fingers pan by their center and pinch-zoom `hfov += (old − new distance) * 0.1`.
  - Inertia is dropped after 15 ms (one finger) or 150 ms (two) without movement.
  - "Touch start" repeats when a finger is added or lifted, so the view doesn't jump. `TouchCancel` ends without inertia.
- **Trackpad pinch** (macOS, our addition): `hfov /= 1 + value`. Two-finger scrolling is the wheel, as in Pannellum.
- **Animation:** a 16 ms `QTimer` runs only while something moves.
- **Grid** (our addition): `u_grid`. Line width in pixels comes from coordinate differences to neighbor pixels (`sphereCoord()` 3×, `fwidth` is not in GLSL ES 1.00). The signed distance is shifted by a quarter pixel (`gridDistance()`), otherwise a line exactly between two pixel centers disappears. `u_pixelSize` = 2 / height in device pixels.

## Testing
No automated tests yet. `TESTING.md` is the manual checklist before a release. Verify by running the installed build (`output/<type>`) and by scripts and screenshots.
- Test data: `img/KUK360-Photo-Sample.jpg`.
  - Tests that delete files or need big images use copies in the scratchpad, never the user's files.
  - Tests run with the user's real settings (`lastDirectory`, `windowGeometry`, `lastUpdateCheck`; Windows `HKCU\Software\KUK360\KUK360`). Back them up before and restore them after.
  - Other formats can be written by a one-off Qt program (`QImage::save()`). Qt can't write GIF: on Windows use `System.Drawing`.
- **Windows:**
  - Screenshots by `CopyFromScreen` after `SetProcessDPIAware()`.
  - Input:
    - `SetCursorPos` + `mouse_event`, and `SendKeys` (`%v` = Alt+V, `{F4}`, `^{+}`). In the file dialog type the path and `{ENTER}`. `~` means Enter in SendKeys, so use the long path form.
    - A click into a window that isn't in the foreground misses, so repeat the test.
  - Touch: `InjectTouchInput` simulates fingers without a touchscreen.
  - Drag & drop: a WinForms window with `DoDragDrop` (call `Application.DoEvents()` after pressing the mouse) and a second process moving the mouse.
  - PowerShell variables are case-insensitive (`$R` overwrites `$r`).
  - Debug output: `$env:QT_FORCE_STDERR_LOGGING = "1"` and `Start-Process ... -RedirectStandardError`.
  - Dark mode: `AppsUseLightTheme` in `HKCU:\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize` and a `WM_SETTINGCHANGE` broadcast with "ImmersiveColorSet". Restore the original value afterwards.
  - If company policy blocks exes run from the temp/scratchpad folder ("Access is denied"), extract test copies under `build\windows\output`.
  - `Copy-Item` keeps the modification time, so set `LastWriteTime` after restoring a source file, or it won't be recompiled.
  - "Update available" can be tested by temporarily lowering the version in `CMakeLists.txt`.
- **macOS:**
  - Terminal needs Screen Recording and Accessibility permissions. Accessibility takes effect after restarting Terminal.
  - Start with `open -n KUK360.app --args <file>`. `open -a` sends `QFileOpenEvent` like Finder does.
  - Screenshots: `screencapture -x -R<x>,<y>,<w>,<h>` with the window position from System Events. The first shot after launch can be black.
  - Input: a small Swift program with `CGEvent`. Menu items can be clicked and read through System Events.
  - Trackpad pinch: `CGEvent` with undocumented gesture fields (type 29, field 110 = 8, field 113 = magnification, field 132 = phase).
  - Dark mode: System Events `appearance preferences`.
  - Crash reports: `~/Library/Logs/DiagnosticReports/KUK360-*.ips`.
- **Linux:**
  - GNOME on Wayland allows no programmatic screenshots or input on the host, so the user checks visuals.
  - Automation runs in the test container: Xvfb, `xdotool` (focus the window with `xdotool windowfocus`, there is no window manager and the file from the command line opens only after activation), `xwd -root`, then `magick` on the host and Read on the PNG.
  - Containers have no locale, set `LANG=C.UTF-8`.
  - Without a terminal Qt logs to journald, use `QT_FORCE_STDERR_LOGGING=1`.
  - `pgrep -f` with text from your own command also matches your own shell.
  - Avoid commands that wait for stdin, the Bash tool hangs until the timeout.

## Known limitations and possible next steps
- **Loading** blocks the UI (big panoramas take seconds). Loading in a thread would also allow prefetching the neighbor files.
- **Aliasing when scaled down** (both viewers sample bilinearly): moiré and shimmering when the image has more than about 1 texel per screen pixel. In flat this happens with every photo bigger than the window, in 360 with panoramas wider than about 10 000 px. Postponed until jariq notices it on real photos. Possible fixes:
  - flat: a pyramid of downscaled copies,
  - 360: a second downscaled texture, mipmaps with a manual LOD (needs a newer GLSL), or several samples per pixel.
- **XMP GPano** initial yaw, pitch and hfov are not read (`pannellum.js` around line 600).
- **Formats:** HEIC and AVIF would need new dependencies.
- **Linux:** tested manually by jariq on Fedora 43 with Wayland and an NVIDIA card, without findings. Other distributions are tested automatically only for startup with a panorama in X11 (Xvfb, `test-distros.sh`).
- **macOS:** tested only on Apple Silicon with macOS 26. Intel and macOS 14.4 won't be tested: the only Intel Mac available runs macOS 12.7.6 and can't be upgraded to 14.4.
- **OpenGL** is deprecated on macOS. Qt RHI (`QRhiWidget`) would be the long-term path, and would also lift the GLSL 1.10 / ES 1.00 limit.
