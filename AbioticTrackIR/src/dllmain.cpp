#include <winsock2.h>
#include <windows.h>
#include <string>
#include <fstream>
#include <chrono>
#include <algorithm>
#include "VersionProxy.hpp"
#include "TrackingReceiver.hpp"
#include "TrackingEngine.hpp"
#include "HookManager.hpp"
#include "Config.hpp"

static TrackingReceiver g_receiver;
static TrackingEngine   g_engine;
static HANDLE           g_initThread = NULL;

static void LogMessage(const std::string& msg)
{
    std::ofstream log("AbioticTrackIR.log", std::ios::app);
    if (log.is_open())
    {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        char timeBuf[32];
        ctime_s(timeBuf, sizeof(timeBuf), &t);
        timeBuf[24] = '\0'; // Remove newline
        log << "[" << timeBuf << "] " << msg << std::endl;
    }
}

static DWORD WINAPI InitThread(LPVOID lpParam)
{
    // Wait briefly for UE engine initialization
    Sleep(2000);

    LogMessage("==================================================================");
    LogMessage("AbioticTrackIR Mod v1.0.0 initializing...");
    LogMessage("6-DOF Head Tracking (TrackIR / OpenTrack) + Parallax 3D Engine");

    // Load configuration
    std::string iniPath = ModConfig::GetDefaultIniPath();
    TrackingConfig config{};
    int udpPort = 4242;

    if (ModConfig::Load(iniPath, config, udpPort))
    {
        LogMessage("Loaded configuration from: " + iniPath);
    }
    else
    {
        LogMessage("Created default configuration at: " + iniPath);
    }

    g_engine.SetConfig(config);

    // Start OpenTrack / TrackIR receiver
    if (g_receiver.Start(udpPort, true))
    {
        LogMessage("TrackingReceiver started successfully.");
        LogMessage("Active Protocol: " + g_receiver.GetActiveSourceName());
    }
    else
    {
        LogMessage("Warning: TrackingReceiver failed to start UDP listener.");
    }

    // Install camera detour hook with retry loop to accommodate engine initialization
    LogMessage("Searching for APlayerCameraManager::DoUpdateCamera...");
    bool hooked = false;
    for (int attempt = 1; attempt <= 15; ++attempt)
    {
        if (HookManager::Instance().Initialize(&g_receiver, &g_engine))
        {
            hooked = true;
            char addrBuf[64];
            snprintf(addrBuf, sizeof(addrBuf), "0x%llX", (unsigned long long)HookManager::Instance().GetTargetAddress());
            LogMessage(std::string("SUCCESS: Hook installed at ") + addrBuf + " (Attempt " + std::to_string(attempt) + ")");
            LogMessage("TrackIR / OpenTrack 6-DOF & Parallax 3D active!");
            LogMessage("Hotkeys: F12 = Recenter | F11 = Toggle Tracking | F10 = Toggle Parallax 3D | Alt = Free-look");
            break;
        }
        Sleep(1000);
    }

    if (!hooked)
    {
        LogMessage("ERROR: Could not locate APlayerCameraManager::DoUpdateCamera in memory after 15 attempts.");
    }

    LogMessage("==================================================================");
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        VersionProxy::Initialize();

        // Check if loaded by game executable (case-insensitive)
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        {
            std::string exeStr(exePath);
            std::transform(exeStr.begin(), exeStr.end(), exeStr.begin(), ::tolower);
            if (exeStr.find("abioticfactor") != std::string::npos)
            {
                g_initThread = CreateThread(NULL, 0, InitThread, NULL, 0, NULL);
            }
        }
        break;

    case DLL_PROCESS_DETACH:
        if (lpvReserved == NULL) // Dynamic unload
        {
            HookManager::Instance().Shutdown();
            g_receiver.Stop();
            VersionProxy::Shutdown();
        }
        break;
    }
    return TRUE;
}
