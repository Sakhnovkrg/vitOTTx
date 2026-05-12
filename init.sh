#!/bin/bash
# Initialize development environment on macOS

set -e
cd "$(dirname "$0")"

echo "=== vitOTTx macOS Setup ==="
echo ""

if ! command -v brew &> /dev/null; then
    echo "Homebrew not found. Install from: https://brew.sh"
    echo "  /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
    exit 1
fi
echo "[OK] Homebrew"

if ! command -v cmake &> /dev/null; then
    echo "Installing CMake..."
    brew install cmake
else
    echo "[OK] CMake $(cmake --version | head -1 | cut -d' ' -f3)"
fi

if ! command -v ninja &> /dev/null; then
    echo "Installing Ninja..."
    brew install ninja
else
    echo "[OK] Ninja $(ninja --version)"
fi

echo ""
echo "Initializing git submodules..."
git submodule update --init --recursive --depth 1

echo ""
echo "=== Setup complete! ==="
echo ""
echo "Build commands:"
echo "  ./build.sh           # Debug build (Ninja)"
echo "  ./build.sh Release   # Release build (Ninja)"
echo "  ./build-xcode.sh     # Generate & open Xcode project"
