@echo off
setlocal

call "%~dp0config.bat"

if not defined VSCMD_VER (
    call "%VCVARS%" %VS_ARCH% >nul 2>&1
)

cd /d "%~dp0"

set "BUILD_TYPE=%1"
if "%BUILD_TYPE%"=="" set "BUILD_TYPE=Debug"

if not exist build mkdir build
cd build

if not exist build.ninja (
    echo Configuring...
    "%CMAKE%" -G Ninja -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_MAKE_PROGRAM="%NINJA%" ..
)

echo Building %BUILD_TYPE%...
"%NINJA%"

echo Done! Artifacts: %~dp0build\%PLUGIN_NAME%_artefacts\%BUILD_TYPE%\
