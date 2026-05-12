#!/usr/bin/env bash
# Build a universal macOS .pkg installer that lets the user pick AU and/or VST3
# and choose between a system-wide (/Library) or per-user (~/Library) install.
#
# Reads PLUGIN_NAME / BUNDLE_ID from config.cmake, but they can be overridden
# via env vars.
#
# Usage:
#   build_pkg.sh <version> <vst3-bundle> <au-bundle> <out-dir> [installer-identity]
#
# vst3-bundle / au-bundle may be empty strings to skip that format.

set -euo pipefail

VERSION="${1:?version required}"
VST3_BUNDLE="${2:-}"
AU_BUNDLE="${3:-}"
OUT_DIR="${4:?out dir required}"
INSTALLER_IDENTITY="${5:-}"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
REPO_ROOT="$( cd "$SCRIPT_DIR/../.." && pwd )"

# Read plugin metadata from config.cmake unless caller already set them.
read_cmake_var() {
    local var="$1"
    grep -E "^set\($var " "$REPO_ROOT/config.cmake" | sed -E 's/.*"([^"]+)".*/\1/' | head -1
}

PLUGIN_NAME="${PLUGIN_NAME:-$(read_cmake_var PLUGIN_NAME)}"
BUNDLE_ID="${BUNDLE_ID:-$(read_cmake_var BUNDLE_ID)}"

if [ -z "$PLUGIN_NAME" ] || [ -z "$BUNDLE_ID" ]; then
    echo "Error: could not resolve PLUGIN_NAME or BUNDLE_ID from config.cmake" >&2
    exit 1
fi

mkdir -p "$OUT_DIR"

build_component_pkg() {
    local bundle="$1"
    local identifier="$2"
    local install_location="$3"
    local out_pkg="$4"

    if [ -z "$bundle" ] || [ ! -d "$bundle" ]; then
        echo "Skipping $out_pkg (bundle missing: '$bundle')"
        return 1
    fi

    pkgbuild \
        --component "$bundle" \
        --identifier "$identifier" \
        --version "$VERSION" \
        --install-location "$install_location" \
        "$out_pkg"
    return 0
}

HAS_VST3=0
HAS_AU=0

if build_component_pkg "$VST3_BUNDLE" "$BUNDLE_ID.vst3" "/Library/Audio/Plug-Ins/VST3" \
        "$OUT_DIR/$PLUGIN_NAME-VST3.pkg"; then
    HAS_VST3=1
fi

if build_component_pkg "$AU_BUNDLE" "$BUNDLE_ID.au" "/Library/Audio/Plug-Ins/Components" \
        "$OUT_DIR/$PLUGIN_NAME-AU.pkg"; then
    HAS_AU=1
fi

if [ "$HAS_VST3" = "0" ] && [ "$HAS_AU" = "0" ]; then
    echo "Error: no plugin bundles supplied" >&2
    exit 1
fi

# Render distribution.xml from template, dropping pkg-refs/choices for any
# missing formats so productbuild does not fail on a dangling reference.
DIST_FILE="$OUT_DIR/distribution.xml"
sed \
    -e "s|__PLUGIN_NAME__|$PLUGIN_NAME|g" \
    -e "s|__BUNDLE_ID__|$BUNDLE_ID|g" \
    -e "s|__VERSION__|$VERSION|g" \
    "$SCRIPT_DIR/distribution.xml" > "$DIST_FILE"

# Strip choices/pkg-refs for formats we did not build. Use a small awk filter
# that drops <choice id="X" .../> blocks (single- or multi-line) and the
# matching <pkg-ref id="...X"...>...</pkg-ref> lines and the corresponding
# <line choice="X"/> entries.
strip_choice() {
    local choice="$1"
    python3 - "$DIST_FILE" "$choice" <<'PY'
import re, sys
path, choice = sys.argv[1], sys.argv[2]
with open(path) as f:
    text = f.read()
# Remove <line choice="X"/>
text = re.sub(rf'\s*<line choice="{re.escape(choice)}"/>', '', text)
# Remove <choice id="X" ...>...</choice> (multiline)
text = re.sub(
    rf'\s*<choice\s+id="{re.escape(choice)}"[\s\S]*?</choice>',
    '', text, flags=re.MULTILINE)
# Remove <pkg-ref id="...X" .../> (the one with .X suffix)
text = re.sub(
    rf'\s*<pkg-ref\s+id="[^"]*\.{re.escape(choice)}"[^/]*/>',
    '', text)
with open(path, 'w') as f:
    f.write(text)
PY
}

[ "$HAS_VST3" = "0" ] && strip_choice vst3
[ "$HAS_AU" = "0" ] && strip_choice au

PRODUCT_PKG="$OUT_DIR/$PLUGIN_NAME-$VERSION.pkg"

# Always build unsigned here. Signing (if any) is the workflow's job — it uses
# rcodesign with the .p12 directly, bypassing the keychain entirely, because
# productbuild --sign / productsign hang on macos GitHub runners.
echo "[build_pkg] invoking productbuild..."
productbuild \
    --distribution "$DIST_FILE" \
    --package-path "$OUT_DIR" \
    "$PRODUCT_PKG"
echo "[build_pkg] productbuild done"

echo "Built (unsigned) installer: $PRODUCT_PKG"
