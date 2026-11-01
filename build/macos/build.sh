#!/usr/bin/env bash
# Rebuilds the release version of KUK360 from scratch: deletes the build tree and the
# distribution, then configures, builds and installs into build/macos/output/release
# (self-contained KUK360.app). The application is signed ad hoc and not notarized, fine for
# development on this Mac; no disk image is built. build-dist.sh builds the signed and
# notarized disk image for publishing.
# build-dist.sh sets these in the environment: KUK360_BUILD_PACKAGES=ON builds the disk image
# (DMG), KUK360_SIGN_IDENTITY (Developer ID Application certificate in the keychain, SHA-1
# hash or name) signs and KUK360_NOTARY_PROFILE (notarytool keychain profile) notarizes;
# "none" is the same as unset. Notarization needs signing.
set -euo pipefail
trap 'echo; echo "Build failed." >&2' ERR

# Qt installation with CMake and Ninja; keep in line with CMakePresets.json.
QT_ROOT="$HOME/Qt/Qt6.12.0"
export PATH="$QT_ROOT/Tools/CMake/CMake.app/Contents/bin:$PATH"

BUILD_PACKAGES="${KUK360_BUILD_PACKAGES:-OFF}"
SIGN_IDENTITY="${KUK360_SIGN_IDENTITY:-}"
NOTARY_PROFILE="${KUK360_NOTARY_PROFILE:-}"
[[ "$SIGN_IDENTITY" == none ]] && SIGN_IDENTITY=""
[[ "$NOTARY_PROFILE" == none || -z "$SIGN_IDENTITY" ]] && NOTARY_PROFILE=""

# Repository root (two levels above this script)
cd "$(dirname "$0")/../.."

# macOS does not lock the files of a running application, but replacing its bundle
# leaves the running copy with a broken signature.
if pgrep -f "build/macos/output/release/KUK360.app/Contents/MacOS/KUK360" > /dev/null; then
    echo "KUK360 from build/macos/output/release is running. Quit it first." >&2
    exit 1
fi

rm -rf build/macos/output/release build/macos/buildroot/release

cmake --preset macos-release "-DKUK360_BUILD_PACKAGES=$BUILD_PACKAGES" \
    "-DKUK360_SIGN_IDENTITY=$SIGN_IDENTITY" "-DKUK360_NOTARY_PROFILE=$NOTARY_PROFILE"
cmake --build build/macos/buildroot/release
cmake --install build/macos/buildroot/release

echo
echo "Done: build/macos/output/release"
