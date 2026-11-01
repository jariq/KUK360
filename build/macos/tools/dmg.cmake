# Builds the macOS disk image (DMG) from the installed, signed KUK360.app; run by
# "cmake --install" at the end when KUK360_BUILD_PACKAGES is on (see CMakeLists.txt).
# Expects APP, DMG, VOLUME_NAME, WORK_DIR and IDENTITY (see sign.cmake).
#
# The image contains KUK360.app and a link to /Applications, so the user drags the
# application there (the usual way on macOS). It is compressed and read-only (UDZO).

set(staging "${WORK_DIR}/staging")
file(REMOVE_RECURSE "${staging}")
file(MAKE_DIRECTORY "${staging}")

# ditto keeps the signature, the symbolic links inside the frameworks and extended attributes
execute_process(COMMAND ditto "${APP}" "${staging}/KUK360.app" COMMAND_ERROR_IS_FATAL ANY)
file(CREATE_LINK /Applications "${staging}/Applications" SYMBOLIC)

message(STATUS "Building ${DMG}")
file(REMOVE "${DMG}")
execute_process(
    COMMAND hdiutil create -quiet -volname "${VOLUME_NAME}" -srcfolder "${staging}"
        -fs HFS+ -format UDZO -imagekey zlib-level=9 "${DMG}"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND hdiutil verify -quiet "${DMG}" COMMAND_ERROR_IS_FATAL ANY)

# The image itself carries a Developer ID signature too (needed for its notarization); an ad
# hoc signature means nothing to Gatekeeper, so the image stays unsigned then.
if(NOT IDENTITY STREQUAL "-")
    execute_process(COMMAND codesign --sign "${IDENTITY}" --timestamp "${DMG}"
        COMMAND_ERROR_IS_FATAL ANY)
endif()

file(REMOVE_RECURSE "${staging}")
