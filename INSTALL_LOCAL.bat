@echo off
setlocal
echo ======================================================================
echo   Installing AbioticTrackIR to Abiotic Factor
echo ======================================================================

set "GAME_BIN=C:\Program Files (x86)\Steam\steamapps\common\AbioticFactor\AbioticFactor\Binaries\Win64"

if not exist "%GAME_BIN%\AbioticFactor-Win64-Shipping.exe" (
    echo [ERROR] Could not find AbioticFactor-Win64-Shipping.exe at:
    echo "%GAME_BIN%"
    echo Please verify your Abiotic Factor installation.
    pause
    exit /b 1
)

echo [INFO] Copying version.dll (Proxy Hook)...
copy /Y "d:\OpenTrack\TrackOpen\ModIO_Package\Binaries\Win64\version.dll" "%GAME_BIN%\version.dll"

echo [INFO] Copying AbioticTrackIR.ini (Config)...
copy /Y "d:\OpenTrack\TrackOpen\ModIO_Package\Binaries\Win64\AbioticTrackIR.ini" "%GAME_BIN%\AbioticTrackIR.ini"

echo [INFO] Copying AbioticTrackIR-Bridge.exe (Control Center)...
copy /Y "d:\OpenTrack\TrackOpen\ModIO_Package\Binaries\Win64\AbioticTrackIR-Bridge.exe" "%GAME_BIN%\AbioticTrackIR-Bridge.exe"

echo.
echo ======================================================================
echo   INSTALLATION COMPLETE!
echo ======================================================================
echo 1. Start OpenTrack (Output: freetrack 2.0 Enhanced or UDP 4242).
echo 2. Launch Abiotic Factor.
echo 3. In-Game Controls:
echo    - Home / F12: Recenter View (Auto-centers on face lock)
echo    - F11: Toggle Head Tracking
echo    - F10: Toggle Parallax 3D Window Mode
echo    - F9:  Increase Focal Depth (+15 cm deeper into screen)
echo    - F8:  Decrease Focal Depth (-15 cm closer to viewer)
echo    - Left Alt: Hold for Free-Look
echo ======================================================================
pause
