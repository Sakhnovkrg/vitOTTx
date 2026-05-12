#!/bin/bash
# Build with Ninja (fast command-line build)

set -e
cd "$(dirname "$0")"

BUILD_TYPE="${1:-Debug}"

if ! command -v cmake &> /dev/null; then
    echo "CMake not found. Install with: brew install cmake  (macOS) / apt install cmake (Debian)"
    exit 1
fi

if ! command -v ninja &> /dev/null; then
    echo "Ninja not found. Install with: brew install ninja  (macOS) / apt install ninja-build (Debian)"
    exit 1
fi

if [ ! -f "libs/JUCE/CMakeLists.txt" ]; then
    echo "Initializing submodules..."
    git submodule update --init --recursive --depth 1
fi

mkdir -p build
cd build

if [ ! -f "build.ninja" ]; then
    echo "Configuring..."
    cmake -G Ninja -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ..
fi

echo "Building $BUILD_TYPE..."
ninja

echo ""
echo "Done! Artifacts:"
ls -la vitOTTx_artefacts/"$BUILD_TYPE"/ 2>/dev/null || ls -la vitOTTx_artefacts/
