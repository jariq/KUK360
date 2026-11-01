# Signs the installed KUK360.app; run by "cmake --install" after the bundle is complete (see
# CMakeLists.txt). Expects APP and IDENTITY: a Developer ID Application certificate in the
# keychain (its SHA-1 hash or name), or "-" for an ad hoc signature.
#
# Changing the bundle (macdeployqt, removed plugins, license files) invalidates the signature
# from linking, and Apple Silicon refuses unsigned code, so the bundle is always signed again.
# Developer ID signatures get the hardened runtime and a secure timestamp, both required for
# notarization. The code is signed from the inside out (plugins, frameworks, then the bundle),
# as Apple recommends instead of --deep.

if(IDENTITY STREQUAL "-")
    set(options "")
    message(STATUS "Signing ${APP} (ad hoc)")
else()
    set(options --timestamp --options runtime)
    message(STATUS "Signing ${APP} (${IDENTITY})")
endif()

file(GLOB_RECURSE plugins "${APP}/Contents/PlugIns/*.dylib")
file(GLOB libraries "${APP}/Contents/Frameworks/*.dylib")
file(GLOB frameworks LIST_DIRECTORIES true "${APP}/Contents/Frameworks/*.framework")
execute_process(
    COMMAND codesign --force --sign "${IDENTITY}" ${options} ${plugins} ${libraries} ${frameworks}
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(
    COMMAND codesign --force --sign "${IDENTITY}" ${options} "${APP}"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND codesign --verify --deep --strict "${APP}" COMMAND_ERROR_IS_FATAL ANY)
