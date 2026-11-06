@echo off
setlocal
echo ======================================================================
echo   Uninstalling AbioticTrackIR from Abiotic Factor
echo ======================================================================

set "GAME_BIN=C:\Program Files (x86)\Steam\steamapps\common\AbioticFactor\AbioticFactor\Binaries\Win64"

if exist "%GAME_BIN%\version.dll" (
    del /F /Q "%GAME_BIN%\version.dll"
    echo [INFO] Removed version.dll.
)

if exist "%GAME_BIN%\AbioticTrackIR-Bridge.exe" (
    del /F /Q "%GAME_BIN%\AbioticTrackIR-Bridge.exe"
    echo [INFO] Removed AbioticTrackIR-Bridge.exe.
)

echo [INFO] Note: AbioticTrackIR.ini was kept so your settings are saved.
echo [SUCCESS] Mod uninstalled.
pause
