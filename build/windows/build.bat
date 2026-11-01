@echo off
rem Rebuilds the release version of KUK360 from scratch: deletes the build tree and the
rem distribution, then configures, builds and installs into build\windows\output\release\KUK360.
rem KUK360.exe there runs by double-click, it has the Qt libraries and plugins next to it.
rem Nothing is signed and no packages are built; build-dist.bat builds the signed installer
rem and portable ZIP for publishing.
rem build-dist.bat sets these in the environment: KUK360_BUILD_PACKAGES=ON builds the
rem installer (MSI, needs the .NET SDK) and the portable ZIP, KUK360_SIGN_CERT_SHA1 (thumbprint
rem of a code signing certificate in the user's store) signs; "none" is the same as unset.

setlocal
rem Qt installation with MinGW, CMake and Ninja; keep in line with CMakePresets.json.
set "QT_ROOT=C:\Qt\Qt6.12.0"
set "PATH=%QT_ROOT%\Tools\mingw1310_64\bin;%QT_ROOT%\6.12.0\mingw_64\bin;%QT_ROOT%\Tools\CMake_64\bin;%PATH%"
if not defined KUK360_BUILD_PACKAGES set "KUK360_BUILD_PACKAGES=OFF"
if /i "%KUK360_SIGN_CERT_SHA1%"=="none" set "KUK360_SIGN_CERT_SHA1="

rem Repository root (two levels above this script)
pushd "%~dp0..\.." || exit /b 1

rem The distribution first: it fails while KUK360.exe is running, the build tree then stays.
if exist build\windows\output\release rmdir /s /q build\windows\output\release
if exist build\windows\output\release (
    echo Cannot delete build\windows\output\release. Is KUK360.exe running?
    goto :error
)
if exist build\windows\buildroot\release rmdir /s /q build\windows\buildroot\release

cmake --preset mingw-release "-DKUK360_BUILD_PACKAGES=%KUK360_BUILD_PACKAGES%" "-DKUK360_SIGN_CERT_SHA1=%KUK360_SIGN_CERT_SHA1%" || goto :error
cmake --build build\windows\buildroot\release || goto :error
cmake --install build\windows\buildroot\release || goto :error

echo.
echo Done: build\windows\output\release
popd
exit /b 0

:error
echo.
echo Build failed.
popd
exit /b 1
