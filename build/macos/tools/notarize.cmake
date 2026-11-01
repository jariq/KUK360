# Notarizes a signed KUK360.app or DMG with Apple and staples the ticket to it, so Gatekeeper
# accepts it after download even offline; run by "cmake --install" when KUK360_NOTARY_PROFILE
# is set (see CMakeLists.txt). Expects FILE, PROFILE (a notarytool keychain profile, created
# with "xcrun notarytool store-credentials") and WORK_DIR.

get_filename_component(name "${FILE}" NAME)
if(FILE MATCHES "\\.app$")
    # notarytool takes a bundle only packed; ditto keeps the symbolic links in the frameworks.
    file(MAKE_DIRECTORY "${WORK_DIR}")
    set(upload "${WORK_DIR}/${name}.zip")
    file(REMOVE "${upload}")
    execute_process(COMMAND ditto -c -k --keepParent "${FILE}" "${upload}"
        COMMAND_ERROR_IS_FATAL ANY)
else()
    set(upload "${FILE}")
endif()

message(STATUS "Notarizing ${FILE} (takes a few minutes)")
execute_process(
    COMMAND xcrun notarytool submit "${upload}" --keychain-profile "${PROFILE}" --wait
        --output-format json
    OUTPUT_VARIABLE result)
string(JSON status ERROR_VARIABLE json_error GET "${result}" status)
if(NOT status STREQUAL "Accepted")
    # The log lists the rejected files and the reasons.
    string(JSON id ERROR_VARIABLE json_error GET "${result}" id)
    if(id)
        execute_process(COMMAND xcrun notarytool log "${id}" --keychain-profile "${PROFILE}")
    endif()
    message(FATAL_ERROR "Notarization of ${FILE} failed: ${result}")
endif()
if(NOT upload STREQUAL FILE)
    file(REMOVE "${upload}")
endif()

execute_process(COMMAND xcrun stapler staple -q "${FILE}" COMMAND_ERROR_IS_FATAL ANY)
