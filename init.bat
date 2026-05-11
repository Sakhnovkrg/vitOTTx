@echo off
setlocal

cd /d "%~dp0"

echo Initializing project...

if not exist .git (
    echo Creating git repository...
    git init
)

if not exist libs\JUCE\CMakeLists.txt (
    if exist libs\JUCE rmdir /S /Q libs\JUCE
    echo Adding JUCE submodule...
    git submodule add https://github.com/juce-framework/JUCE.git libs/JUCE
)

git submodule update --init --recursive

if not exist config.bat (
    copy /Y config.bat.example config.bat
    echo Created config.bat - edit paths to your build tools.
)

echo.
echo Done! Now:
echo   1. Edit config.bat (paths to VS Build Tools / cmake / ninja)
echo   2. Run: build.bat            (Debug)
echo   3. Or:  build.bat Release    (Release)
