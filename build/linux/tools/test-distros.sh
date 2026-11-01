#!/usr/bin/env bash
# Tests the AppImage from build/linux/build-dist.sh on the oldest supported distributions in
# containers (podman): Ubuntu 22.04 and Debian 12, with only the libraries of a GNOME desktop
# (build/linux/container/Containerfile.test). On each of them it
#   1. extracts the AppImage (containers have no FUSE) and checks with ldd that every
#      shipped library and plugin finds its dependencies (LD_LIBRARY_PATH: the shipped X11
#      libraries have no RPATH, at run time they find each other as already loaded by Qt),
#   2. starts KUK360 with the sample panorama in a virtual X server (xcb platform, software
#      OpenGL), focuses its window like a window manager would (KUK360 opens the file from
#      the command line only once it is active) and checks that it is still running,
#   3. saves a screenshot and the output of KUK360 into build/linux/output/release/tests and
#      checks in the screenshot that the panorama is shown (needs ImageMagick, magick).
# Before that it validates the AppStream metadata in the AppDir (appstreamcli, package
# appstream).
# Fails if a library is missing, KUK360 does not keep running or does not show the panorama.
#
# Usage: build/linux/tools/test-distros.sh [image...]   default: ubuntu:22.04 debian:12
set -euo pipefail

cd "$(dirname "$0")/../../.."
appimage="$(ls build/linux/output/release/KUK360-*-x86_64.AppImage)"
tests="build/linux/output/release/tests"
images=("$@")
[ ${#images[@]} -gt 0 ] || images=(docker.io/library/ubuntu:22.04 docker.io/library/debian:12)

rm -rf "$tests"
mkdir -p "$tests"
failed=0

if command -v appstreamcli > /dev/null; then
    appstreamcli validate --no-net \
        build/linux/output/release/KUK360.AppDir/usr/share/metainfo/*.metainfo.xml \
        || failed=1
else
    echo "appstreamcli not found, AppStream metadata not validated."
fi

for base in "${images[@]}"; do
    name="$(basename "$base" | tr ':' '-')"
    image="localhost/kuk360-test-linux:$name"
    echo "== $base"
    podman build --quiet --build-arg BASE="$base" --tag "$image" \
        --file build/linux/container/Containerfile.test build/linux/container > /dev/null

    # label=disable: SELinux would block the mounts (see build-dist.sh).
    if ! podman run --rm --security-opt label=disable \
        --volume "$PWD/$appimage:/test/KUK360.AppImage:ro" \
        --volume "$PWD/img/KUK360-Photo-Sample.jpg:/test/sample.jpg:ro" \
        --volume "$PWD/$tests:/out" \
        --env NAME="$name" \
        "$image" bash -euc '
            cd /tmp
            /test/KUK360.AppImage --appimage-extract > /dev/null

            missing="$(LD_LIBRARY_PATH=squashfs-root/usr/lib find squashfs-root/usr -type f \( -name "*.so*" -o -path "*/bin/KUK360" \) \
                -exec ldd {} \; 2>/dev/null | awk "/not found/ {print \$1}" | sort -u)"
            if [ -n "$missing" ]; then
                echo "Missing libraries:" $missing
                exit 1
            fi
            echo "Libraries: all found"

            export LANG=C.UTF-8 DISPLAY=:99  # the containers have no locale
            Xvfb :99 -screen 0 1280x800x24 -nolisten tcp > /dev/null 2>&1 &
            sleep 2
            QT_QPA_PLATFORM=xcb QT_FORCE_STDERR_LOGGING=1 \
                squashfs-root/AppRun /test/sample.jpg > "/out/$NAME.log" 2>&1 &
            app=$!
            sleep 3
            xdotool windowfocus "$(xdotool search --onlyvisible --name KUK360 | head -1)"
            sleep 4
            if ! kill -0 $app 2> /dev/null; then
                echo "KUK360 is not running:"
                cat "/out/$NAME.log"
                exit 1
            fi
            echo "KUK360: running"
            xwd -root -silent -out "/out/$NAME.xwd"
            kill $app'; then
        failed=1
    fi
    if [ -f "$tests/$name.xwd" ] && command -v magick > /dev/null; then
        magick "$tests/$name.xwd" "$tests/$name.png" && rm "$tests/$name.xwd"
        # The window is at the top left (no window manager). Above the panorama is sky; the
        # gray of the start page (viewBackground #aaaaaa) means the image was not opened.
        pixel="$(magick "$tests/$name.png" -format "%[hex:p{512,100}]" info:)"
        if [ "$pixel" = AAAAAA ]; then
            echo "The panorama was not opened (start page in the screenshot)."
            failed=1
        else
            echo "Screenshot: image shown"
        fi
    fi
done

echo
ls "$tests"
if [ $failed -ne 0 ]; then
    echo "Test failed." >&2
    exit 1
fi
echo "All tests passed."
