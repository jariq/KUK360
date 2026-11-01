@echo off
rem Rebuilds the release version of KUK360 for publishing (see build.bat): next to the
rem application in build\windows\output\release\KUK360 the installer KUK360-<version>-x64.msi
rem and the portable KUK360-<version>-x64-portable.zip. The installer needs the .NET SDK
rem (WiX Toolset is a local .NET tool, see build\windows\dotnet-tools.json).
rem The application and the installer are signed with the code signing certificate of the
rem author (Certum, SimplySign Desktop must be logged in). KUK360_SIGN_CERT_SHA1 set in the
rem environment selects another certificate by its thumbprint, "none" builds unsigned packages.

setlocal
set "KUK360_BUILD_PACKAGES=ON"
if not defined KUK360_SIGN_CERT_SHA1 set "KUK360_SIGN_CERT_SHA1=5A640AD3AD0DE8608BB38C7686258A2440D3C3C2"
call "%~dp0build.bat"
exit /b %ERRORLEVEL%
