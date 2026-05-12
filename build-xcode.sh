#!/bin/bash
# Generate Xcode project for debugging

set -e
cd "$(dirname "$0")"

if ! command -v cmake &> /dev/null; then
    echo "CMake not found. Install with: brew install cmake"
    exit 1
fi

if [ ! -f "libs/JUCE/CMakeLists.txt" ]; then
    echo "Initializing submodules..."
    git submodule update --init --recursive --depth 1
fi

mkdir -p build-xcode
cd build-xcode

echo "Generating Xcode project..."
cmake -G Xcode ..

echo ""
echo "Done! Opening Xcode..."
open vitOTTx.xcodeproj
