#!/usr/bin/env bash
# Generates the notices for the third-party code inside the Qt frameworks and plugins of the
# macOS bundle: one file per component in build/macos/licenses/Qt with its copyright notices
# and license texts.
#
# Unlike the Windows script, the list is not maintained by hand. The Qt frameworks and
# plugins are read from the installed KUK360.app, the SBOM of the Qt kit
# (<Qt>/<version>/macos/sbom/*.spdx.json) maps them to its packages (CONTAINS), and their
# DEPENDS_ON relationships lead to the third-party packages ("*_Attribution_*", "Bundled*").
# The texts come from the attribution pages of the local Qt documentation.
#
# Run it after "cmake --install" whenever the Qt version or the shipped plugins change, and
# review the diff. It fails if a shipped binary is missing in the SBOM or a third-party
# package has no text.
#
# Usage: build/macos/tools/qt-third-party-notices.sh [Qt directory] [KUK360.app]
#   Qt directory  installation root of the Qt installer, default ~/Qt/Qt6.12.0
#   KUK360.app    installed bundle, default build/macos/output/release/KUK360.app
# Needs jq and perl (both part of macOS).
set -euo pipefail
shopt -s nullglob

here="$(cd "$(dirname "$0")" && pwd)"
qt="${1:-$HOME/Qt/Qt6.12.0}"
app="${2:-$here/../output/release/KUK360.app}"
version="$(basename "$qt")"
version="${version#Qt}"
sbom="$qt/$version/macos/sbom"
docs="$qt/Docs/Qt-$version"
out="$here/../licenses/Qt"

for dir in "$sbom" "$docs" "$app/Contents/Frameworks"; do
    [ -d "$dir" ] || { echo "Not found: $dir" >&2; exit 1; }
done

# Shipped binaries, named as in the SBOM (relative to the Qt kit)
shipped="$(
    { for framework in "$app"/Contents/Frameworks/Qt*.framework; do
          name="$(basename "$framework" .framework)"
          echo "./lib/$name.framework/Versions/A/$name"
      done
      for plugin in "$app"/Contents/PlugIns/*/*.dylib; do
          echo "./plugins/${plugin#"$app"/Contents/PlugIns/}"
      done
    } | jq -R . | jq -s .
)"

# Third-party packages reachable from the shipped binaries, one JSON object per line
packages="$(jq -s -c --argjson shipped "$shipped" '
    def field($k): [.comment // "" | capture("\n *" + $k + ": (?<v>[^\n]*)") | .v] | first // "";
    [ .[] as $doc
      | ($doc.packages | map({key: .SPDXID, value: .}) | from_entries) as $pkg
      | ($doc.files // [] | map({key: .SPDXID, value: .fileName}) | from_entries) as $file
      | ($doc.relationships | map(select(.relationshipType == "DEPENDS_ON"))
         | group_by(.spdxElementId)
         | map({key: .[0].spdxElementId, value: map(.relatedSpdxElement)}) | from_entries) as $deps
      | [ $doc.relationships[] | select(.relationshipType == "CONTAINS")
          | select(($file[.relatedSpdxElement] // "") as $f | $shipped | index($f))
          | {pkg: .spdxElementId, file: $file[.relatedSpdxElement]} ] as $contained
      | ($contained | map(.pkg) | unique) as $roots
      | {found: ($contained | map(.file) | unique)}
        + ({seen: $roots, todo: $roots}
           | until(.todo | length == 0;
                 (.todo | map($deps[.] // []) | add // [] | unique) as $next
                 | ($next - .seen) as $new
                 | {seen: (.seen + $new), todo: $new})
           | {third: [.seen[] | $pkg[.] // empty
                      | select(.name | test("_Attribution_|^Bundled"))
                      | {id: field("Id"), title: field("Name"),
                         version: (.versionInfo // ""), license: (.licenseConcluded // ""),
                         source: ([.comment // "" | capture("extracted from:\n *(?<v>[^\n]*)") | .v]
                                  | first // "")}]})
    ]
    | (map(.found) | add | unique) as $found
    | ($shipped - $found) as $missing
    | if ($missing | length) > 0 then error("Not in the SBOM: \($missing | join(", "))") else . end
    # CMake scripts of the build system are not part of the binaries
    | map(.third) | add | map(select(.source | test("/cmake/") | not)) | unique_by(.id) | .[]
    ' "$sbom"/*.spdx.json)"

# Attribution page of a package id (the pages use "-" where the ids may use "_")
page_of() {
    local pages=("$docs"/*/*-attribution-"${1//_/-}".html "$docs"/*/*-attribution-"$1".html)
    [ ${#pages[@]} -gt 0 ] && echo "${pages[0]}"
}

# HTML of one attribution page -> plain text (title, description, copyrights, license texts)
page_text() {
    perl -0777 -ne '
        my ($title) = /<h1 class="title">(.*?)<\/h1>/s;
        my ($descr) = /<div class="descr"[^>]*>(.*?)\n<\/div>/s;
        $_ = "$title\n$descr";
        s/<p>/\n/g; s/<\/pre>/\n/g; s/<[^>]*>//g;
        s/&quot;/"/g; s/&#39;/\x27/g; s/&lt;/</g; s/&gt;/>/g;
        s/&copy;/(C)/g; s/&nbsp;/ /g; s/&amp;/&/g;
        s/\n{3,}/\n\n/g;
        print;
    ' "$1"
    echo
}

# XSVG (arc handling in Qt GUI) has no page in the Qt 6.12 documentation; the text is taken
# from the Qt 6.11 page, the license file in Qt is unchanged.
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

# Sort by attribution page (module, then id) like the Windows notices; xsvg after Qt GUI.
entries=""
while IFS= read -r package; do
    id="$(jq -r .id <<<"$package")"
    page="$(page_of "$id" || true)"
    if [ -z "$page" ]; then
        [ "$id" = xsvg ] || { echo "No attribution page for '$id' in $docs" >&2; exit 1; }
        page="$docs/qtgui/qtgui-attribution-xsvg.html"  # sort key only
    fi
    entries+="${page#"$docs"/}"$'\t'"$package"$'\n'
done <<<"$packages"
entries="$(printf "%s" "$entries" | sort)"

# One file per component, named after its attribution page (qtcore-attribution-zlib.html ->
# zlib.txt). The previous files go first (LICENSE, the license of Qt itself, has no
# extension and stays).
rm -f "$out"/*.txt
while IFS=$'\t' read -r page package; do
    name="$(basename "$page" .html | sed 's/^[a-z0-9]*-attribution-//').txt"
    [ ! -e "$out/$name" ] || { echo "Duplicate file name: $name" >&2; exit 1; }
    if [ "$(jq -r .id <<<"$package")" = xsvg ] && [ ! -f "$docs/$page" ]; then
        xsvg_text
    else
        page_text "$docs/$page"
    fi | cat -s > "$out/$name"
done <<<"$entries"

echo "Written $(wc -l <<<"$entries" | tr -d " ") files to $out"
echo "Shipped Qt binaries:"
jq -r '.[]' <<<"$shipped" | sed 's/^/  /'
