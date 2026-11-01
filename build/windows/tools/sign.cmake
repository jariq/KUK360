# Signs files with the code signing certificate (Authenticode, SHA-256, RFC 3161 timestamp);
# run by "cmake --install" when KUK360_SIGN_CERT_SHA1 is set (see CMakeLists.txt). Expects
# SIGNTOOL, CERT_SHA1, TIMESTAMP_URL, DESCRIPTION, DESCRIPTION_URL and FILES: files or
# directories, in which every .exe and .dll is signed. Files that already carry a valid
# signature (Qt libraries and plugins signed by The Qt Company) are left as they are.
#
# The certificate is selected by its SHA-1 thumbprint in the certificate store of the user;
# its private key may be on a smart card, which then asks for the PIN.

set(to_sign "")
foreach(path IN LISTS FILES)
    if(IS_DIRECTORY "${path}")
        file(GLOB_RECURSE binaries "${path}/*.exe" "${path}/*.dll")
    else()
        set(binaries "${path}")
    endif()
    foreach(binary IN LISTS binaries)
        # /pa: the default Authenticode policy; fails for unsigned files.
        execute_process(COMMAND "${SIGNTOOL}" verify /pa /q "${binary}"
            RESULT_VARIABLE unsigned OUTPUT_QUIET ERROR_QUIET)
        if(unsigned)
            list(APPEND to_sign "${binary}")
        endif()
    endforeach()
endforeach()

if(NOT to_sign)
    return()
endif()
foreach(file IN LISTS to_sign)
    message(STATUS "Signing ${file}")
endforeach()
# One call for all files: a smart card asks for the PIN only once.
execute_process(
    COMMAND "${SIGNTOOL}" sign /sha1 "${CERT_SHA1}" /fd sha256 /tr "${TIMESTAMP_URL}" /td sha256
        /d "${DESCRIPTION}" /du "${DESCRIPTION_URL}" ${to_sign}
    COMMAND_ERROR_IS_FATAL ANY)
