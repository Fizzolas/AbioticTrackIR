#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <conio.h>
#include "TrackingReceiver.hpp"
#include "TrackingEngine.hpp"
#include "Config.hpp"

static void PrintProgressBar(double val, double minVal, double maxVal, int width = 20)
{
    double norm = (val - minVal) / (maxVal - minVal);
    norm = std::clamp(norm, 0.0, 1.0);
    int pos = (int)(norm * width);

    std::cout << "[";
    for (int i = 0; i < width; ++i)
    {
        if (i == width / 2) std::cout << "|";
        else if (i < pos)   std::cout << "=";
        else if (i == pos)  std::cout << ">";
        else                std::cout << " ";
    }
    std::cout << "]";
}

int main(int argc, char** argv)
{
    SetConsoleTitleA("Abiotic Factor - TrackIR & OpenTrack Control Center");

    // Enable virtual terminal processing for clean ANSI console output
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode))
    {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }

    std::cout << "======================================================================\n";
    std::cout << "  ABIOTIC FACTOR: TrackIR & OpenTrack 6-DOF + Parallax 3D Bridge\n";
    std::cout << "======================================================================\n\n";

    TrackingConfig config{};
    int udpPort = 4242;
    std::string iniPath = ModConfig::GetDefaultIniPath();

    ModConfig::Load(iniPath, config, udpPort);
    std::cout << "[INFO] Configuration loaded from: " << iniPath << "\n";

    TrackingReceiver receiver;
    TrackingEngine engine;
    engine.SetConfig(config);

    std::cout << "[INFO] Connecting to OpenTrack (FreeTrack SharedMem / UDP " << udpPort << ")...\n";
    if (!receiver.Start(udpPort, true))
    {
        std::cerr << "[ERROR] Failed to initialize TrackingReceiver.\n";
        return 1;
    }

    std::cout << "[INFO] Tracking receiver started. Press 'C' to center, 'P' to toggle Parallax, '+/-' for focal depth, 'Q' to quit.\n\n";

    auto lastRefresh = std::chrono::steady_clock::now();

    while (true)
    {
        if (_kbhit())
        {
            int ch = _getch();
            if (ch == 'q' || ch == 'Q') break;
            if (ch == 'c' || ch == 'C')
            {
                engine.Recenter();
                std::cout << "\n>>> CALIBRATED: Head tracking centered at current pose! <<<\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
            if (ch == 'p' || ch == 'P')
            {
                engine.ToggleParallax();
                std::cout << "\n>>> Parallax 3D: " << (engine.GetConfig().Parallax3DEnabled ? "ENABLED" : "DISABLED") << " <<<\n";
            }
            if (ch == '+' || ch == '=')
            {
                engine.AdjustFocalDepth(+15.0);
                std::cout << "\n>>> Parallax Focal Depth: " << engine.GetFocalDepth() << " cm <<<\n";
            }
            if (ch == '-' || ch == '_')
            {
                engine.AdjustFocalDepth(-15.0);
                std::cout << "\n>>> Parallax Focal Depth: " << engine.GetFocalDepth() << " cm <<<\n";
            }
            if (ch == 't' || ch == 'T')
            {
                engine.ToggleTracking();
                std::cout << "\n>>> Head Tracking: " << (engine.GetConfig().Enabled ? "ENABLED" : "DISABLED") << " <<<\n";
            }
            if (ch == 's' || ch == 'S')
            {
                ModConfig::Save(iniPath, engine.GetConfig(), udpPort);
                std::cout << "\n>>> Config saved to " << iniPath << " <<<\n";
            }
        }

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration<double>(now - lastRefresh).count() >= 0.05) // 20 FPS terminal refresh
        {
            lastRefresh = now;

            RawTrackingPose raw = receiver.GetLatestPose();
            ProcessedTrackingState proc = engine.Process(raw, 90.0f); // Default 90 FOV

            // Reset cursor position to start of telemetry block (using console API with ANSI fallback)
            COORD coord = {0, 6};
            SetConsoleCursorPosition(hOut, coord);
            std::cout << "\033[6;1H";
            std::cout << "=== LIVE TELEMETRY === [" << receiver.GetActiveSourceName() << "] "
                      << receiver.GetPacketsPerSecond() << " Hz\n";
            std::cout << "Tracking Status : " << (proc.IsTrackingActive ? "ACTIVE  " : "PAUSED  ")
                      << " | Parallax 3D : " << (proc.IsParallaxActive ? "ENABLED " : "DISABLED")
                      << " | Freelook : " << (proc.IsFreelookActive ? "ON " : "OFF") << "\n\n";

            std::cout << std::fixed << std::setprecision(2);

            std::cout << "ROTATION (deg): \n";
            std::cout << "  Yaw   : " << std::setw(7) << proc.Rotation.Yaw   << " deg "; PrintProgressBar(proc.Rotation.Yaw, -45.0, 45.0); std::cout << "\n";
            std::cout << "  Pitch : " << std::setw(7) << proc.Rotation.Pitch << " deg "; PrintProgressBar(proc.Rotation.Pitch, -30.0, 30.0); std::cout << "\n";
            std::cout << "  Roll  : " << std::setw(7) << proc.Rotation.Roll  << " deg "; PrintProgressBar(proc.Rotation.Roll, -20.0, 20.0); std::cout << "\n\n";

            std::cout << "TRANSLATION (cm): \n";
            std::cout << "  Lateral (L/R) : " << std::setw(6) << proc.LocalTranslation.Y << " cm "; PrintProgressBar(proc.LocalTranslation.Y, -15.0, 15.0); std::cout << "\n";
            std::cout << "  Forward/Back  : " << std::setw(6) << proc.LocalTranslation.X << " cm "; PrintProgressBar(proc.LocalTranslation.X, -15.0, 15.0); std::cout << "\n";
            std::cout << "  Vertical(U/D) : " << std::setw(6) << proc.LocalTranslation.Z << " cm "; PrintProgressBar(proc.LocalTranslation.Z, -10.0, 10.0); std::cout << "\n\n";

            std::cout << "PARALLAX 3D ENGINE: \n";
            std::cout << "  Focal Depth     : " << std::setw(6) << proc.Parallax.CurrentFocalDepth << " cm [Convergence Distance]\n";
            std::cout << "  Frustum Shear X : " << std::setw(6) << proc.Parallax.ProjectionOffset.X << " [L/R Projection Plane Offset]\n";
            std::cout << "  Frustum Shear Y : " << std::setw(6) << proc.Parallax.ProjectionOffset.Y << " [U/D Projection Plane Offset]\n";
            std::cout << "  Dynamic Depth FOV: " << std::setw(6) << proc.Parallax.AdjustedFOV << " deg (Base 90.0)\n\n";

            std::cout << "CONTROLS: [C] Center | [T] Track | [P] Parallax | [+] Depth+ | [-] Depth- | [S] Save | [Q] Quit\n";
            std::cout << std::flush;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    receiver.Stop();
    std::cout << "\n[INFO] Bridge stopped cleanly.\n";
    return 0;
}
