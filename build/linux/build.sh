#!/usr/bin/env bash
# Rebuilds the release version of KUK360 from scratch on this system: deletes the build tree
# and the distribution, then configures, builds and installs into build/linux/output/release
# (KUK360.AppDir and the AppImage packed from it). The AppImage needs the glibc of this system
# or newer, fine for development; build-dist.sh builds the AppImage for publishing in a
# container, so it runs on older distributions too.
# build-dist.sh sets these in the environment: KUK360_MAX_GLIBC and KUK360_MAX_GLIBCXX stop
# the build if the AppImage would require newer glibc or libstdc++ symbols.
set -euo pipefail
trap 'echo; echo "Build failed." >&2' ERR

# Qt installation with CMake and Ninja; keep in line with CMakePresets.json.
QT_ROOT="$HOME/Qt/Qt6.12.0"
export PATH="$QT_ROOT/Tools/CMake/bin:$PATH"

MAX_GLIBC="${KUK360_MAX_GLIBC:-}"
MAX_GLIBCXX="${KUK360_MAX_GLIBCXX:-}"

# Repository root (two levels above this script)
cd "$(dirname "$0")/../.."

# A running KUK360 does not block the rebuild: deleting its files only unlinks them, the
# running process (or the mounted AppImage) keeps its copy.
rm -rf build/linux/output/release build/linux/buildroot/release

cmake --preset linux-release "-DKUK360_MAX_GLIBC=$MAX_GLIBC" "-DKUK360_MAX_GLIBCXX=$MAX_GLIBCXX"
cmake --build build/linux/buildroot/release
cmake --install build/linux/buildroot/release

echo
echo "Done: $(ls build/linux/output/release/*.AppImage)"
