cmake_minimum_required(VERSION 3.21)

project(KUK360 VERSION 3.0.0
    DESCRIPTION "Simple 360 photo viewer"
    LANGUAGES CXX)
set(KUK360_COPYRIGHT "Copyright (c) 2019-2026 Jaroslav Imrich")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform specific files (icons, resources, licenses, tools) are in build/<platform>, next
# to the build tree (buildroot) and the distribution (output) of that platform.
if(WIN32)
    set(KUK360_PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/windows")
elseif(APPLE)
    set(KUK360_PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/macos")
else()
    set(KUK360_PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/linux")
endif()

# Install layout. It must be set before find_package(Qt6), which stores the destinations
# for the Qt deploy script.
if(WIN32)
    # Flat layout usual for Windows: KUK360.exe, the libraries, qt.conf and the license
    # files side by side, Qt plugins in the "plugins" subdirectory.
    set(CMAKE_INSTALL_BINDIR .)
    set(KUK360_INSTALL_BINDIR .)
    set(KUK360_INSTALL_DOCDIR .)
elseif(APPLE)
    # Everything inside the self-contained KUK360.app bundle.
    set(KUK360_INSTALL_BINDIR bin)
    set(KUK360_INSTALL_DOCDIR KUK360.app/Contents/Resources)
else()
    # Everything inside KUK360.AppDir, which is packed into the AppImage: the usual FHS
    # layout under usr (bin, lib, plugins, share). The Qt deploy script runs with the install
    # prefix switched to usr (see below), so qt.conf and the RPATH ($ORIGIN/../lib) stay
    # valid inside the mounted AppImage; lib instead of lib64 as usual in AppImages.
    set(KUK360_APPDIR KUK360.AppDir)
    set(CMAKE_INSTALL_LIBDIR lib)
    # Link libGL.so.1 (package libgl1 etc., present everywhere, also used by the Qt libraries)
    # instead of the GLVND libOpenGL.so.0, which some distributions do not install by default.
    set(OpenGL_GL_PREFERENCE LEGACY)
    set(KUK360_INSTALL_BINDIR ${KUK360_APPDIR}/usr/bin)
    set(KUK360_INSTALL_DOCDIR ${KUK360_APPDIR}/usr/share/doc/KUK360)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets Network)
qt_standard_project_setup()

qt_add_executable(KUK360 WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/AboutDialog.h
    src/AboutDialog.cpp
    src/ButtonPanel.h
    src/ButtonPanel.cpp
    src/CompactMenuBarStyle.h
    src/CompactMenuBarStyle.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/MessageOverlay.h
    src/MessageOverlay.cpp
    src/MessageBoxes.h
    src/MessageBoxes.cpp
    src/FileLocation.h
    src/FileLocation.cpp
    src/FontAwesome.h
    src/FontAwesome.cpp
    src/ImageFolder.h
    src/ImageFolder.cpp
    src/ImageViewer.h
    src/ImageViewer.cpp
    src/MainActions.h
    src/MainActions.cpp
    src/PanoramaDetector.h
    src/PanoramaDetector.cpp
    src/PanoramaView.h
    src/PanoramaView.cpp
    src/Settings.h
    src/Settings.cpp
    src/FlatView.h
    src/FlatView.cpp
    src/Theme.h
    src/Theme.cpp
    src/UpdateChecker.h
    src/UpdateChecker.cpp
    src/ZoomableView.h
)

qt_add_resources(KUK360 "resources"
    PREFIX "/"
    FILES
        src/shaders/panorama.vert
        src/shaders/panorama.frag
        src/fontawesome-webfont.ttf
        src/KUK360.svg
)

target_compile_definitions(KUK360 PRIVATE KUK360_VERSION="${PROJECT_VERSION}")
target_link_libraries(KUK360 PRIVATE Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Network)

# Warnings of our code are errors (GCC, MinGW, Clang). Qt headers come from imported targets
# as system includes, so their warnings do not count. If a newer compiler warns about
# something new, "cmake --preset ... --compile-no-warning-as-error" builds anyway until the
# code is fixed (COMPILE_WARNING_AS_ERROR needs CMake 3.24, older versions ignore it).
target_compile_options(KUK360 PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>)
set_target_properties(KUK360 PROPERTIES COMPILE_WARNING_AS_ERROR ON)

if(WIN32)
    # Executable icon, version information and application manifest. Qt adds its own
    # generated manifest only as a source file, which CMake embeds with MSVC but not with
    # MinGW, so the manifest is embedded through the resource file there. Listing our
    # manifest as a source also stops Qt from generating its own one.
    set(KUK360_RC_ICON "${KUK360_PLATFORM_DIR}/icons/KUK360.ico")
    set(KUK360_MANIFEST "${KUK360_PLATFORM_DIR}/resources/KUK360.exe.manifest")
    if(MINGW)
        set(KUK360_RC_MANIFEST "1 24 \"${KUK360_MANIFEST}\"  // CREATEPROCESS_MANIFEST_RESOURCE_ID, RT_MANIFEST")
    else()
        set(KUK360_RC_MANIFEST "")
    endif()
    configure_file("${KUK360_PLATFORM_DIR}/resources/KUK360.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/KUK360.rc" @ONLY)
    target_sources(KUK360 PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/KUK360.rc" "${KUK360_MANIFEST}")
    # Private Qt API for the full screen border of OpenGL windows (see main.cpp); only
    # headers, no additional library is deployed.
    set(QT_NO_PRIVATE_MODULE_WARNING ON)  # used on purpose, review when updating Qt
    find_package(Qt6 REQUIRED COMPONENTS GuiPrivate)
    target_link_libraries(KUK360 PRIVATE Qt6::GuiPrivate)

    # Distribution packages: the installer (see build/windows/tools/msi.cmake), built by the
    # WiX Toolset, a local .NET tool (needs the .NET SDK), and the portable ZIP. On in
    # build/windows/build-dist.bat.
    option(KUK360_BUILD_PACKAGES "Build the Windows installer (MSI) and the portable ZIP on install" OFF)
    if(KUK360_BUILD_PACKAGES)
        find_program(KUK360_DOTNET dotnet REQUIRED)
    endif()

    # Code signing of the distribution and the installer (see build/windows/tools/sign.cmake),
    # so Windows (SmartScreen, Smart App Control) shows the publisher instead of a warning
    # about an unknown application. Off without a certificate thumbprint; build-dist.bat sets it.
    set(KUK360_SIGN_CERT_SHA1 "" CACHE STRING
        "SHA-1 thumbprint of the code signing certificate in the user's store (empty = not signed)")
    set(KUK360_SIGN_TIMESTAMP_URL "http://time.certum.pl/" CACHE STRING
        "RFC 3161 timestamp server of the code signature")
    if(KUK360_SIGN_CERT_SHA1)
        # signtool of the newest Windows SDK
        file(GLOB signtool_dirs "$ENV{ProgramFiles\(x86\)}/Windows Kits/10/bin/*/x64")
        list(SORT signtool_dirs COMPARE NATURAL ORDER DESCENDING)
        find_program(KUK360_SIGNTOOL signtool HINTS ${signtool_dirs} REQUIRED)
    endif()
endif()

if(UNIX AND NOT APPLE)
    # D-Bus: "Show in file manager" selects the file through the file manager interface
    # (FileLocation.cpp) and the color scheme comes from the desktop portal (Theme.cpp). QtGui
    # already depends on QtDBus on Linux, so no library is added.
    find_package(Qt6 REQUIRED COMPONENTS DBus)
    target_link_libraries(KUK360 PRIVATE Qt6::DBus)

    # AppImage tools (see build/linux/tools/appimage.cmake). The AppImage runtime is a
    # fixed release, downloaded once next to the build trees and checked by its SHA-256
    # (published with the release on GitHub). mksquashfs is from squashfs-tools.
    if(NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        message(FATAL_ERROR "The AppImage is built only for x86_64.")
    endif()
    set(KUK360_APPIMAGE_RUNTIME_VERSION 20251108)
    set(KUK360_APPIMAGE_RUNTIME_SHA256 2fca8b443c92510f1483a883f60061ad09b46b978b2631c807cd873a47ec260d)
    set(KUK360_APPIMAGE_RUNTIME
        "${KUK360_PLATFORM_DIR}/buildroot/appimage-runtime-${KUK360_APPIMAGE_RUNTIME_VERSION}-x86_64")
    if(EXISTS "${KUK360_APPIMAGE_RUNTIME}")
        file(SHA256 "${KUK360_APPIMAGE_RUNTIME}" runtime_hash)
    endif()
    if(NOT runtime_hash STREQUAL KUK360_APPIMAGE_RUNTIME_SHA256)
        message(STATUS "Downloading AppImage runtime ${KUK360_APPIMAGE_RUNTIME_VERSION}")
        file(DOWNLOAD
            "https://github.com/AppImage/type2-runtime/releases/download/${KUK360_APPIMAGE_RUNTIME_VERSION}/runtime-x86_64"
            "${KUK360_APPIMAGE_RUNTIME}"
            EXPECTED_HASH SHA256=${KUK360_APPIMAGE_RUNTIME_SHA256}
            STATUS download_status)
        list(GET download_status 0 download_error)
        if(download_error)
            file(REMOVE "${KUK360_APPIMAGE_RUNTIME}")
            message(FATAL_ERROR "Cannot download the AppImage runtime: ${download_status}")
        endif()
    endif()
    # Newest glibc and libstdc++ symbol versions the AppImage may require (empty = no check;
    # build/linux/build-dist.sh sets them for the release built in the container).
    set(KUK360_MAX_GLIBC "" CACHE STRING "Newest GLIBC symbol version allowed in the AppImage")
    set(KUK360_MAX_GLIBCXX "" CACHE STRING "Newest GLIBCXX symbol version allowed in the AppImage")
    find_program(KUK360_MKSQUASHFS mksquashfs PATHS /usr/sbin /sbin)
    if(NOT KUK360_MKSQUASHFS)
        message(FATAL_ERROR "mksquashfs not found, install squashfs-tools.")
    endif()
endif()

if(APPLE)
    # Bundle icon and Info.plist (version, copyright, "Open With" document types).
    set(KUK360_ICNS "${KUK360_PLATFORM_DIR}/icons/KUK360.icns")
    set_source_files_properties("${KUK360_ICNS}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    target_sources(KUK360 PRIVATE "${KUK360_ICNS}")
    set_target_properties(KUK360 PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${KUK360_PLATFORM_DIR}/resources/Info.plist.in")

    # Distribution package: the disk image (see build/macos/tools/dmg.cmake). On in
    # build/macos/build-dist.sh.
    option(KUK360_BUILD_PACKAGES "Build the macOS disk image (DMG) on install" OFF)

    # Code signing and notarization of the bundle and the DMG (see build/macos/tools/sign.cmake
    # and notarize.cmake), so Gatekeeper opens the downloaded application. Without an identity
    # the bundle is signed ad hoc; build-dist.sh sets both.
    set(KUK360_SIGN_IDENTITY "" CACHE STRING
        "Developer ID Application certificate in the keychain, SHA-1 hash or name (empty = ad hoc)")
    set(KUK360_NOTARY_PROFILE "" CACHE STRING
        "notarytool keychain profile for the notarization (empty = not notarized)")
    if(KUK360_NOTARY_PROFILE AND NOT KUK360_SIGN_IDENTITY)
        message(FATAL_ERROR "Notarization (KUK360_NOTARY_PROFILE) needs KUK360_SIGN_IDENTITY.")
    endif()
endif()

# Distribution: "cmake --install <build dir>" puts the application with all its runtime
# dependencies (Qt libraries, plugins, compiler runtime) and the license files into a
# separate directory (installDir of the preset), keeping the build directory clean.
# The install layout is set at the top of this file.
install(TARGETS KUK360
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${KUK360_INSTALL_BINDIR}
)
if(WIN32)
    # Leave out what KUK360 does not use (windeployqt options): the Direct3D shader
    # compilers (rendering uses OpenGL), touch input and network information plugins, the
    # certificate-only TLS backend (Schannel is used) and image formats other than the
    # supported JPEG, PNG, BMP (built in), GIF, TIFF and WebP. The SVG window icon needs
    # the qsvgicon icon engine, not the qsvg image format. The software OpenGL renderer
    # (opengl32sw.dll) stays as a fallback for systems without OpenGL drivers.
    # Keep the Qt licenses in build/windows/licenses/Qt and
    # build/windows/tools/qt-third-party-notices.sh in line with this.
    set(KUK360_DEPLOY_TOOL_OPTIONS
        --no-system-d3d-compiler
        --no-system-dxc-compiler
        --skip-plugin-types generic,networkinformation
        --exclude-plugins qcertonlybackend,qico,qicns,qtga,qwbmp,qsvg
    )
elseif(UNIX AND NOT APPLE)
    # The generic deploy script takes only the default platform (xcb), so Wayland is added.
    # Left out: the same as on Windows and macOS (touch input, network information, the
    # certificate-only TLS backend, unsupported image formats, the virtual keyboard, which
    # pulls in Qt Quick and QML) and what a desktop application does not use: eglfs, the QML
    # debugger, Lottie (Qt Quick again), Wayland shells other than xdg-shell and the server
    # buffer integrations. The wayland-egl client integration is needed for OpenGL.
    # Keep the Qt licenses in build/linux/licenses/Qt in line with this.
    set(KUK360_DEPLOY_OPTIONS
        INCLUDE_PLUGINS qwayland
        EXCLUDE_PLUGIN_TYPES egldeviceintegrations generic networkinformation qmltooling
            vectorimageformats
        EXCLUDE_PLUGINS qtvirtualkeyboardplugin qcertonlybackend qico qicns qtga qwbmp qsvg
            fullscreen-shell-v1 ivi-shell qt-shell wl-shell-plugin
            dmabuf-server drm-egl-server shm-emulation-server vulkan-server
    )
endif()
if(UNIX AND NOT APPLE)
    # The Qt deploy script copies the Qt libraries and plugins into the AppDir; libraries of
    # the system (glibc, OpenGL, X11, Wayland, fontconfig...) stay outside, as usual in
    # AppImages. KUK360_DEPLOY_OPTIONS (above) selects the plugins and libraries.
    # The script installs relative to CMAKE_INSTALL_PREFIX, which therefore points to the
    # usr directory of the AppDir while it runs.
    install(CODE "
        set(KUK360_INSTALL_PREFIX \"\${CMAKE_INSTALL_PREFIX}\")
        set(CMAKE_INSTALL_PREFIX \"\${CMAKE_INSTALL_PREFIX}/${KUK360_APPDIR}/usr\")")
endif()
qt_generate_deploy_app_script(
    TARGET KUK360
    OUTPUT_SCRIPT deploy_script
    NO_TRANSLATIONS
    DEPLOY_TOOL_OPTIONS ${KUK360_DEPLOY_TOOL_OPTIONS}
    ${KUK360_DEPLOY_OPTIONS}
)
install(SCRIPT ${deploy_script})
if(UNIX AND NOT APPLE)
    install(CODE [[set(CMAKE_INSTALL_PREFIX "${KUK360_INSTALL_PREFIX}")]])
    # Libraries of the system are not copied, except the xcb-util helper libraries of the xcb
    # platform plugin: common distributions do not install all of them by default (e.g.
    # libxcb-cursor0 on Ubuntu 22.04 and Debian 12, see build/linux/tools/test-distros.sh),
    # and without them KUK360 does not start in X11 sessions. They use only the stable API of
    # libxcb, come from the build container (AlmaLinux 9) and are found by the Qt libraries
    # and plugins through their RPATH. libxkbcommon-x11 must not be shipped: it uses internals
    # of libxkbcommon of exactly the same version, and with the libxkbcommon of the system
    # KUK360 crashed at start (Ubuntu 22.04). X11 desktops have it anyway (mutter, kwin...).
    # Keep build/linux/licenses/XCB in line with this.
    foreach(library xcb-cursor.so.0 xcb-icccm.so.4 xcb-image.so.0 xcb-keysyms.so.1
            xcb-render-util.so.0 xcb-shape.so.0 xcb-util.so.1 xcb-xkb.so.1)
        unset(library_path)
        find_file(library_path lib${library} PATHS /usr/lib64 /usr/lib/x86_64-linux-gnu
            NO_DEFAULT_PATH NO_CACHE REQUIRED)
        # The file itself under its soname, not the symlink
        file(REAL_PATH "${library_path}" library_file)
        install(FILES "${library_file}" DESTINATION ${KUK360_APPDIR}/usr/lib RENAME lib${library})
    endforeach()
    # Application menu entry, "Open with" in the file manager and the window icon on
    # Wayland (see setDesktopFileName() in main.cpp); also the root of the AppDir.
    install(FILES "${KUK360_PLATFORM_DIR}/resources/com.kuk360.KUK360.desktop"
        DESTINATION ${KUK360_APPDIR}/usr/share/applications)
    install(FILES src/KUK360.svg
        DESTINATION ${KUK360_APPDIR}/usr/share/icons/hicolor/scalable/apps
        RENAME com.kuk360.KUK360.svg)
    # AppStream metadata (name, summary, description, website) for software centers and
    # AppImage catalogs.
    install(FILES "${KUK360_PLATFORM_DIR}/resources/com.kuk360.KUK360.metainfo.xml"
        DESTINATION ${KUK360_APPDIR}/usr/share/metainfo)
endif()
if(APPLE)
    # macdeployqt has no option to leave plugins out, so the same set as on Windows is
    # removed afterwards: the virtual keyboard input context (which pulls in Qt Quick and
    # QML), network information, the OpenSSL and certificate-only TLS backends (Secure
    # Transport is used) and image formats other than the supported ones.
    # Keep the Qt licenses in build/macos/licenses/Qt in line with this.
    install(CODE [[
        set(contents "${CMAKE_INSTALL_PREFIX}/KUK360.app/Contents")
        file(REMOVE_RECURSE
            "${contents}/PlugIns/platforminputcontexts"
            "${contents}/PlugIns/networkinformation"
            "${contents}/PlugIns/tls/libqopensslbackend.dylib"
            "${contents}/PlugIns/tls/libqcertonlybackend.dylib")
        foreach(plugin qico qicns qtga qwbmp qsvg qmacheif qmacjp2)
            file(REMOVE "${contents}/PlugIns/imageformats/lib${plugin}.dylib")
        endforeach()
        foreach(framework Qml QmlMeta QmlModels QmlWorkerScript Quick VirtualKeyboard VirtualKeyboardQml)
            file(REMOVE_RECURSE "${contents}/Frameworks/Qt${framework}.framework")
        endforeach()
    ]])
endif()
# The license files: LICENSE.txt (LICENSE with an extension, so it opens by double-click) and
# THIRD_PARTY_NOTICES.txt from the repository root, which also says where the sources of the
# shipped Qt libraries are (LGPL), and the license texts of each component of the platform in
# its own subdirectory of "licenses".
install(FILES LICENSE DESTINATION ${KUK360_INSTALL_DOCDIR} RENAME LICENSE.txt)
install(FILES THIRD_PARTY_NOTICES.txt DESTINATION ${KUK360_INSTALL_DOCDIR})
install(DIRECTORY "${KUK360_PLATFORM_DIR}/licenses/" DESTINATION ${KUK360_INSTALL_DOCDIR}/licenses)
if(APPLE)
    # The finished bundle is signed (Developer ID or ad hoc) and notarized before it is packed
    # into the DMG, so the application keeps its stapled ticket also after it is dragged out
    # of the image. The DMG is then signed and notarized on its own. The variables stay set
    # for the following scripts.
    if(KUK360_SIGN_IDENTITY)
        set(identity "${KUK360_SIGN_IDENTITY}")
    else()
        set(identity -)
    endif()
    install(CODE "
        set(APP \"\${CMAKE_INSTALL_PREFIX}/KUK360.app\")
        set(IDENTITY \"${identity}\")
        set(PROFILE \"${KUK360_NOTARY_PROFILE}\")")
    install(SCRIPT "${KUK360_PLATFORM_DIR}/tools/sign.cmake")
    if(KUK360_NOTARY_PROFILE)
        install(CODE "
            set(FILE \"\${APP}\")
            set(WORK_DIR \"${CMAKE_CURRENT_BINARY_DIR}/notarize\")")
        install(SCRIPT "${KUK360_PLATFORM_DIR}/tools/notarize.cmake")
    endif()
    if(KUK360_BUILD_PACKAGES)
        install(CODE "
            set(DMG \"\${CMAKE_INSTALL_PREFIX}/KUK360-${PROJECT_VERSION}.dmg\")
            set(VOLUME_NAME \"KUK360 ${PROJECT_VERSION}\")
            set(WORK_DIR \"${CMAKE_CURRENT_BINARY_DIR}/dmg\")")
        install(SCRIPT "${KUK360_PLATFORM_DIR}/tools/dmg.cmake")
        if(KUK360_NOTARY_PROFILE)
            install(CODE "
                set(FILE \"\${DMG}\")
                set(WORK_DIR \"${CMAKE_CURRENT_BINARY_DIR}/notarize\")")
            install(SCRIPT "${KUK360_PLATFORM_DIR}/tools/notarize.cmake")
        endif()
    endif()
endif()
if(UNIX AND NOT APPLE)
    # The finished AppDir is packed into the AppImage next to it.
    install(CODE "
        set(APPDIR \"\${CMAKE_INSTALL_PREFIX}/${KUK360_APPDIR}\")
        set(APPIMAGE \"\${CMAKE_INSTALL_PREFIX}/KUK360-${PROJECT_VERSION}-x86_64.AppImage\")
        set(RUNTIME \"${KUK360_APPIMAGE_RUNTIME}\")
        set(MKSQUASHFS \"${KUK360_MKSQUASHFS}\")
        set(DESKTOP_ID com.kuk360.KUK360)
        set(MAX_GLIBC \"${KUK360_MAX_GLIBC}\")
        set(MAX_GLIBCXX \"${KUK360_MAX_GLIBCXX}\")")
    install(SCRIPT "${KUK360_PLATFORM_DIR}/tools/appimage.cmake")
endif()
if(WIN32 AND KUK360_SIGN_CERT_SHA1)
    # KUK360.exe and the unsigned libraries (MinGW runtime) of the installed application are
    # signed before they are packed into the MSI and the ZIP; the MSI is signed after it is
    # built (below). The variables stay set for that.
    install(CODE "
        set(SIGNTOOL \"${KUK360_SIGNTOOL}\")
        set(CERT_SHA1 \"${KUK360_SIGN_CERT_SHA1}\")
        set(TIMESTAMP_URL \"${KUK360_SIGN_TIMESTAMP_URL}\")
        set(DESCRIPTION KUK360)
        set(DESCRIPTION_URL https://www.kuk360.com/)
        set(FILES \"\${CMAKE_INSTALL_PREFIX}\")")
    install(SCRIPT "${KUK360_PLATFORM_DIR}/tools/sign.cmake")
endif()
if(WIN32 AND KUK360_BUILD_PACKAGES)
    # The installed application (the KUK360 directory, installDir of the preset) is packed
    # into the MSI and the ZIP next to it.
    install(CODE "
        set(APPDIR \"\${CMAKE_INSTALL_PREFIX}\")
        get_filename_component(MSI \"\${CMAKE_INSTALL_PREFIX}/../KUK360-${PROJECT_VERSION}-x64.msi\" ABSOLUTE)
        set(WXS \"${KUK360_PLATFORM_DIR}/resources/KUK360.wxs\")
        set(VERSION \"${PROJECT_VERSION}\")
        set(ICON \"${KUK360_RC_ICON}\")
        set(LICENSE_FILE \"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE\")
        set(WORK_DIR \"${CMAKE_CURRENT_BINARY_DIR}/msi\")
        set(DOTNET \"${KUK360_DOTNET}\")
        set(WIX_MANIFEST_DIR \"${KUK360_PLATFORM_DIR}\")")
    install(SCRIPT "${KUK360_PLATFORM_DIR}/tools/msi.cmake")
    if(KUK360_SIGN_CERT_SHA1)
        install(CODE [[set(FILES "${MSI}")]])
        install(SCRIPT "${KUK360_PLATFORM_DIR}/tools/sign.cmake")
    endif()
    # Portable version: the KUK360 directory as it is, unpacked anywhere and started from
    # there (no Start menu shortcut, no Open with).
    install(CODE "
        get_filename_component(ZIP \"\${CMAKE_INSTALL_PREFIX}/../KUK360-${PROJECT_VERSION}-x64-portable.zip\" ABSOLUTE)
        get_filename_component(zip_dir \"\${CMAKE_INSTALL_PREFIX}\" DIRECTORY)
        get_filename_component(app_dir_name \"\${CMAKE_INSTALL_PREFIX}\" NAME)
        message(STATUS \"Building \${ZIP}\")
        file(REMOVE \"\${ZIP}\")
        execute_process(COMMAND \"\${CMAKE_COMMAND}\" -E tar cf \"\${ZIP}\" --format=zip \"\${app_dir_name}\"
            WORKING_DIRECTORY \"\${zip_dir}\" COMMAND_ERROR_IS_FATAL ANY)")
endif()
