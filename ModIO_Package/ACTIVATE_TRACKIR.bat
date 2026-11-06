@echo off
setlocal enabledelayedexpansion
title AbioticTrackIR - One-Click Mod Activator
echo ======================================================================
echo   ABIOTIC FACTOR: TrackIR & OpenTrack 6-DOF + Parallax 3D
echo   Mod.io One-Click Activator & Installer
echo ======================================================================
echo.

set "SCRIPT_DIR=%~dp0"
set "TARGET_DIR="

:: 1. Check relative location (if extracted into game folder or Mods folder)
if exist "%SCRIPT_DIR%..\..\Binaries\Win64\AbioticFactor-Win64-Shipping.exe" (
    set "TARGET_DIR=%SCRIPT_DIR%..\..\Binaries\Win64"
)
if exist "%SCRIPT_DIR%..\..\..\Binaries\Win64\AbioticFactor-Win64-Shipping.exe" (
    set "TARGET_DIR=%SCRIPT_DIR%..\..\..\Binaries\Win64"
)

:: 2. Check default Steam installation directory
if "%TARGET_DIR%"=="" (
    if exist "C:\Program Files (x86)\Steam\steamapps\common\AbioticFactor\AbioticFactor\Binaries\Win64\AbioticFactor-Win64-Shipping.exe" (
        set "TARGET_DIR=C:\Program Files (x86)\Steam\steamapps\common\AbioticFactor\AbioticFactor\Binaries\Win64"
    )
)

:: 3. Check current folder
if "%TARGET_DIR%"=="" (
    if exist "%SCRIPT_DIR%AbioticFactor-Win64-Shipping.exe" (
        set "TARGET_DIR=%SCRIPT_DIR%"
    )
)

if "%TARGET_DIR%"=="" (
    echo [PROMPT] Could not auto-detect AbioticFactor-Win64-Shipping.exe.
    echo Please enter the full path to your AbioticFactor\Binaries\Win64 folder:
    set /p "TARGET_DIR=Path: "
)

if not exist "%TARGET_DIR%\AbioticFactor-Win64-Shipping.exe" (
    echo.
    echo [ERROR] AbioticFactor-Win64-Shipping.exe was not found in:
    echo "%TARGET_DIR%"
    echo Installation aborted.
    pause
    exit /b 1
)

echo [INFO] Found target directory:
echo "%TARGET_DIR%"
echo.

echo [INFO] Installing proxy hook (version.dll)...
copy /Y "%SCRIPT_DIR%Binaries\Win64\version.dll" "%TARGET_DIR%\version.dll"

if not exist "%TARGET_DIR%\AbioticTrackIR.ini" (
    echo [INFO] Installing default configuration (AbioticTrackIR.ini)...
    copy /Y "%SCRIPT_DIR%Binaries\Win64\AbioticTrackIR.ini" "%TARGET_DIR%\AbioticTrackIR.ini"
) else (
    echo [INFO] Preserved existing AbioticTrackIR.ini settings.
)

echo [INFO] Installing Control Center bridge (AbioticTrackIR-Bridge.exe)...
copy /Y "%SCRIPT_DIR%Binaries\Win64\AbioticTrackIR-Bridge.exe" "%TARGET_DIR%\AbioticTrackIR-Bridge.exe"

echo.
echo ======================================================================
echo   SUCCESSFULLY ACTIVATED!
echo ======================================================================
echo 1. Launch OpenTrack and ensure tracking is running (output: freetrack 2.0 or UDP 4242).
echo 2. Launch Abiotic Factor.
echo 3. Controls:
echo    - F12: Recenter View
echo    - F11: Toggle Head Tracking
echo    - F10: Toggle Parallax 3D Window Mode
echo    - F9:  Increase Focal Depth (+15 cm deeper into screen)
echo    - F8:  Decrease Focal Depth (-15 cm closer to viewer)
echo    - Left Alt: Hold for Free-Look
echo ======================================================================
echo.
pause
