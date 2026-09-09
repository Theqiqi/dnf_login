@echo off
setlocal enabledelayedexpansion
title dnf Launcher Packaging Engine - ZIP Archive Generation

:: 1. Inject the CMake bin directory into the path to prevent 'cpack' command discovery errors
set "PATH=%PATH%;C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"

set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."

echo ============================================================
echo 📦 Reading build cache and generating pure ZIP archive...
echo ============================================================

if not exist "%PROJECT_ROOT%\out\build\release\CPackConfig.cmake" (
    echo ❌ ERROR: Build cache not found! Please run build-release.bat first before packaging.
    pause
    exit /b 1
)

:: 2. Ensure the isolated output distribution directory exists securely
set "PACKAGE_DIR=%PROJECT_ROOT%\out\package"
if not exist "%PACKAGE_DIR%" mkdir "%PACKAGE_DIR%"

:: 3. Change directory context to project root and execute packaging
cd /d "%PROJECT_ROOT%"

:: Force CPack to stream the final generated .zip bundle directly into out/package via -B
cpack.exe -G ZIP --config out/build/release/CPackConfig.cmake -B "%PACKAGE_DIR%"

if %ERRORLEVEL% equ 0 (
    echo.
    echo 🎉 Success! The ZIP archive has been perfectly compiled into: out\package\
    dir /b "%PACKAGE_DIR%\*.zip"
) else (
    echo ❌ ERROR: CPack packaging process failed. Check configuration steps above.
)

echo ============================================================
pause
