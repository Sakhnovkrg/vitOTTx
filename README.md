# vitOTTx

Multiband compressor

## Requirements
SSE2-compliant CPU.

## Building (Windows)

1. `init.bat` — initializes the JUCE submodule and copies `config.bat.example` → `config.bat`.
2. Edit `config.bat` — point the variables at your local Visual Studio Build Tools / CMake / Ninja.
3. `build.bat` (Debug) or `build.bat Release`.

Artifacts land in `build\vitOTTx_artefacts\<Config>\`.

## Building (macOS / Linux)

```sh
git submodule update --init --recursive
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## License

The entire source is licensed under the GPLv3 (see `LICENSE`). If you distribute the source or built binaries, you must comply with that license.

## Links
- Original DSP: https://github.com/mtytel/vital
- Upstream fork: https://github.com/edgjj/vitOTT
- JUCE: https://github.com/juce-framework/JUCE
