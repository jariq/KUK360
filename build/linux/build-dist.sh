#!/usr/bin/env bash
# Rebuilds the release version of KUK360 for publishing: runs build.sh in an AlmaLinux 9
# container (podman), which builds KUK360.AppDir and the AppImage
# KUK360-<version>-x86_64.AppImage in build/linux/output/release.
#
# The container has glibc 2.34 and GCC 11 like the Qt 6.12 binaries, so the AppImage runs on
# the same distributions as Qt (see build/linux/container/Containerfile); the build fails if
# it would require newer glibc or libstdc++ symbols. Qt, CMake and Ninja come from the Qt
# installation of the host, mounted at the same path as the repository.
set -euo pipefail

# Qt installation with CMake and Ninja; keep in line with CMakePresets.json and build.sh.
QT_ROOT="$HOME/Qt/Qt6.12.0"
# Build image; the tag follows the base image in the Containerfile.
IMAGE="localhost/kuk360-build-linux:almalinux9.8"

# Repository root (two levels above this script)
cd "$(dirname "$0")/../.."
REPO="$PWD"

# Builds the image on the first run or after a change of the Containerfile, else uses the
# cache.
podman build --quiet --tag "$IMAGE" build/linux/container > /dev/null

# --userns=keep-id: files created in the container belong to the user.
# label=disable: SELinux would block the mounts; relabeling them (":z") would change the
# labels of the Qt installation and the repository on the host for good.
podman run --rm --userns=keep-id --security-opt label=disable \
    --volume "$QT_ROOT:$QT_ROOT:ro" --volume "$REPO:$REPO" --workdir "$REPO" \
    --env HOME="$HOME" --env PATH="/usr/bin:/bin" \
    --env KUK360_MAX_GLIBC=2.34 --env KUK360_MAX_GLIBCXX=3.4.29 \
    "$IMAGE" build/linux/build.sh
