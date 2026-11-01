# Builds the Windows installer (MSI) from the installed application; run by "cmake --install"
# at the end when KUK360_BUILD_PACKAGES is on (see CMakeLists.txt). Expects APPDIR, MSI, WXS,
# VERSION, ICON, LICENSE_FILE, WORK_DIR, DOTNET and WIX_MANIFEST_DIR.
#
# The WiX Toolset is a local .NET tool: its version is in dotnet-tools.json in
# WIX_MANIFEST_DIR, "dotnet tool restore" fetches it and the UI extension of the same version
# goes to the extension cache in WORK_DIR. The WiX EULA must be accepted once per user
# ("dotnet tool run wix eula accept wix7" in build/windows).

file(MAKE_DIRECTORY "${WORK_DIR}")

# License page of the installer: GPL-3.0 (LICENSE, plain text) as RTF. The lines of each
# paragraph are joined, so the text wraps in the dialog.
file(READ "${LICENSE_FILE}" license)
string(REPLACE "\r\n" "\n" license "${license}")
string(STRIP "${license}" license)
string(REPLACE "\\" "\\\\" license "${license}")
string(REPLACE "{" "\\{" license "${license}")
string(REPLACE "}" "\\}" license "${license}")
string(REGEX REPLACE "\n[ \t]*\n[ \t\n]*" "@PARAGRAPH@" license "${license}")
string(REGEX REPLACE "[ \t]*\n[ \t]*" " " license "${license}")
string(REGEX REPLACE "^[ \t]+" "" license "${license}")
string(REPLACE "@PARAGRAPH@" "\\par\n\\par\n" license "${license}")
file(WRITE "${WORK_DIR}/LICENSE.rtf"
    "{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0\\fswiss Segoe UI;}}\\f0\\fs16\n${license}\\par\n}\n")

# Version of WiX from the tool manifest, the UI extension must be the same.
file(READ "${WIX_MANIFEST_DIR}/dotnet-tools.json" manifest)
string(JSON wix_version GET "${manifest}" tools wix version)

execute_process(COMMAND "${DOTNET}" tool restore
    WORKING_DIRECTORY "${WIX_MANIFEST_DIR}" OUTPUT_QUIET COMMAND_ERROR_IS_FATAL ANY)
set(wix "${DOTNET}" tool run --allow-roll-forward wix --)
execute_process(COMMAND ${wix} extension add --nologo WixToolset.UI.wixext/${wix_version}
    WORKING_DIRECTORY "${WORK_DIR}" COMMAND_ERROR_IS_FATAL ANY)

message(STATUS "Building ${MSI}")
execute_process(
    COMMAND ${wix} build --nologo "${WXS}" -arch x64 -ext WixToolset.UI.wixext
        -d "Version=${VERSION}" -d "AppDir=${APPDIR}" -d "IconFile=${ICON}"
        -d "LicenseRtf=${WORK_DIR}/LICENSE.rtf"
        -intermediatefolder "${WORK_DIR}/obj" -o "${MSI}"
    WORKING_DIRECTORY "${WORK_DIR}" COMMAND_ERROR_IS_FATAL ANY)
# wix also writes the debug symbols next to the MSI
get_filename_component(msi_dir "${MSI}" DIRECTORY)
get_filename_component(msi_name "${MSI}" NAME_WLE)
file(REMOVE "${msi_dir}/${msi_name}.wixpdb")
