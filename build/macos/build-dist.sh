#!/usr/bin/env bash
# Rebuilds the release version of KUK360 for publishing (see build.sh): KUK360.app and next to
# it the disk image KUK360-<version>.dmg in build/macos/output/release, both signed with the
# Developer ID Application certificate of the author and notarized with the notarytool
# keychain profile "KUK360". KUK360_SIGN_IDENTITY and KUK360_NOTARY_PROFILE set in the
# environment select others, KUK360_NOTARY_PROFILE=none signs without notarization and
# KUK360_SIGN_IDENTITY=none builds an ad hoc signed disk image.
set -euo pipefail

export KUK360_BUILD_PACKAGES=ON
# Developer ID Application: Jaroslav Imrich (2PFS6H3V82)
export KUK360_SIGN_IDENTITY="${KUK360_SIGN_IDENTITY:-1240E31E306098350E847ED43887E2EDB4301F26}"
export KUK360_NOTARY_PROFILE="${KUK360_NOTARY_PROFILE:-KUK360}"

exec "$(dirname "$0")/build.sh"
