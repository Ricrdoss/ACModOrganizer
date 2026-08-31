@echo off
setlocal enabledelayedexpansion

echo ======================================================================
echo   ACBO (Assetto Corsa Mod Organizer) - Single-File Standalone Builder
echo ======================================================================
echo.

set "CMAKE_BIN=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
set "QT_BIN=C:\Qt\6.8.0\msvc2022_64\bin"
set "PATH=%QT_BIN%;%CMAKE_BIN%;%PATH%"

taskkill /F /IM ACModOrganize.exe /IM ACBO.exe /IM acbo_tests.exe >nul 2>&1

echo [1/4] Building Release targets (ACModOrganize, ACBO, acbo_tests)...
cmake --build build --config Release --target ACModOrganize ACBO acbo_tests
if errorlevel 1 (
    echo [ERROR] Build failed!
    exit /b 1
)

echo.
echo [2/4] Running test suite...
build\Release\acbo_tests.exe
if errorlevel 1 (
    echo [ERROR] Tests failed!
    exit /b 1
)

echo.
echo [3/4] Deploying Qt runtimes via windeployqt...
windeployqt.exe --qmldir ui build\Release\ACModOrganize.exe --release --compiler-runtime >nul 2>&1

echo.
echo [4/4] Bundling standalone single-file dist\ACBO.exe...
python tools\package_single_exe.py
if errorlevel 1 (
    echo [ERROR] Packaging failed!
    exit /b 1
)

echo.
echo ======================================================================
echo   SUCCESS! Standalone single executable ready at: dist\ACBO.exe
echo   Upload dist\ACBO.exe directly to your GitHub Release!
echo ======================================================================
echo.
