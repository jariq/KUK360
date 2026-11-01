#!/usr/bin/env bash
# Generates the license texts of the third-party code that is built into the Qt libraries and
# plugins shipped with KUK360 for Windows: one file per component in build/windows/licenses/Qt,
# named after its attribution page in the local Qt documentation (qtcore-attribution-zlib.html
# -> zlib.txt), with the text of that page.
#
# The list of pages follows the SBOM of the Qt build (<Qt>/<kit>/sbom/*.spdx.json, packages
# "*_Attribution_*" and "Bundled*" of the shipped modules and plugins). Review it when the
# Qt version or the set of shipped Qt modules / plugins changes.
#
# Usage: build/windows/tools/qt-third-party-notices.sh [Qt docs directory]
set -euo pipefail

docs="${1:-/c/Qt/Qt6.12.0/Docs/Qt-6.12.0}"
out="$(dirname "$0")/../licenses/Qt"

pages=(
    # Qt Core
    qtcore/qtcore-attribution-blake2
    qtcore/qtcore-attribution-doubleconversion
    qtcore/qtcore-attribution-easing
    qtcore/qtcore-attribution-md4
    qtcore/qtcore-attribution-md5
    qtcore/qtcore-attribution-pcre2
    qtcore/qtcore-attribution-pcre2-sljit
    qtcore/qtcore-attribution-rfc6234
    qtcore/qtcore-attribution-sha1
    qtcore/qtcore-attribution-sha3-endian
    qtcore/qtcore-attribution-sha3-keccak
    qtcore/qtcore-attribution-siphash
    qtcore/qtcore-attribution-tika-mimetypes
    qtcore/qtcore-attribution-tinycbor
    qtcore/qtcore-attribution-tlexpected
    qtcore/qtcore-attribution-unicode-character-database
    qtcore/qtcore-attribution-unicode-cldr
    qtcore/qtcore-attribution-zlib
    # Qt GUI (including the Windows platform plugin and the bundled image libraries)
    qtgui/qtgui-attribution-aglfn
    qtgui/qtgui-attribution-d3d12memoryallocator
    qtgui/qtgui-attribution-emoji-segmenter
    qtgui/qtgui-attribution-freetype
    qtgui/qtgui-attribution-freetype-bdf
    qtgui/qtgui-attribution-freetype-pcf
    qtgui/qtgui-attribution-freetype-zlib
    qtgui/qtgui-attribution-grayraster
    qtgui/qtgui-attribution-harfbuzz-ng
    qtgui/qtgui-attribution-icc-srgb-color-profile
    qtgui/qtgui-attribution-libjpeg
    qtgui/qtgui-attribution-libpng
    qtgui/qtgui-attribution-md4c
    qtgui/qtgui-attribution-opengl-es2-headers
    qtgui/qtgui-attribution-opengl-headers
    qtgui/qtgui-attribution-rhi-miniengine-d3d12-mipmap
    qtgui/qtgui-attribution-smooth-scaling-algorithm
    qtgui/qtgui-attribution-vulkan-xml-spec
    qtgui/qtgui-attribution-vulkanmemoryallocator
    qtgui/qtgui-attribution-webgradients
    qtgui/qtgui-attribution-wintab
    qtgui/qtgui-attribution-xserverhelper
    # Qt Network
    qtnetwork/qtnetwork-attribution-libpsl
    qtnetwork/qtnetwork-attribution-psl-data
    # XSVG (arc handling in Qt GUI, in Qt SVG before Qt 6.12): the Qt 6.12 documentation has
    # no page for it, so the text is written by xsvg_text below.
    xsvg
    # Qt Image Formats plugins
    qtimageformats/qtimageformats-attribution-libtiff
    qtimageformats/qtimageformats-attribution-libwebp
    # Software OpenGL renderer of the Qt binary packages for Windows (opengl32sw.dll)
    qtdoc/qt-attribution-llvmpipe
)

# HTML of one attribution page -> plain text (title, description, copyrights, license texts)
page_text() {
    local file="$1"
    sed -n 's/.*<h1 class="title">\(.*\)<\/h1>.*/\1/p' "$file"
    sed -n '/<div class="descr"/,/^<\/div>/p' "$file" \
        | sed -e 's/<p>/\n/g' -e 's/<\/pre>/\n/g' -e 's/<[^>]*>//g' \
              -e 's/&quot;/"/g' -e "s/&#39;/'/g" -e 's/&lt;/</g' -e 's/&gt;/>/g' \
              -e 's/&copy;/(C)/g' -e 's/&nbsp;/ /g' -e 's/&amp;/\&/g' \
        | cat -s
}

# XSVG notice in the format of page_text (taken from the Qt 6.11 documentation page, the
# license file in Qt is unchanged)
xsvg_text() {
    cat <<'EOF'
XSVG

Some code for arc handling is derived from code from the XSVG project.

Used in the Qt GUI library.

 Copyright 2002 USC/Information Sciences Institute

Historical Permission Notice and Disclaimer - sell variant.

 Copyright  2002 USC/Information Sciences Institute

 Permission to use, copy, modify, distribute, and sell this software
 and its documentation for any purpose is hereby granted without
 fee, provided that the above copyright notice appear in all copies
 and that both that copyright notice and this permission notice
 appear in supporting documentation, and that the name of
 Information Sciences Institute not be used in advertising or
 publicity pertaining to distribution of the software without
 specific, written prior permission.  Information Sciences Institute
 makes no representations about the suitability of this software for
 any purpose.  It is provided "as is" without express or implied
 warranty.

 INFORMATION SCIENCES INSTITUTE DISCLAIMS ALL WARRANTIES WITH REGARD
 TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL INFORMATION SCIENCES
 INSTITUTE BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 PERFORMANCE OF THIS SOFTWARE.

EOF
}

# The previous files go first (LICENSE, the license of Qt itself, has no extension and stays).
rm -f "$out"/*.txt
for page in "${pages[@]}"; do
    name="$(basename "$page" | sed 's/^[a-z0-9]*-attribution-//').txt"
    [ ! -e "$out/$name" ] || { echo "Duplicate file name: $name" >&2; exit 1; }
    if [ "$page" = xsvg ]; then
        xsvg_text
    else
        page_text "$docs/$page.html"
    fi > "$out/$name"
done

echo "Written ${#pages[@]} files to $out"
