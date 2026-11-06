#include "Config.hpp"
#include <windows.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

static std::string Trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

int ModConfig::ParseKeyString(const std::string& keyStr, int defaultKey)
{
    std::string s = Trim(keyStr);
    if (s.empty()) return defaultKey;

    if (s == "VK_F1" || s == "F1") return VK_F1;
    if (s == "VK_F2" || s == "F2") return VK_F2;
    if (s == "VK_F3" || s == "F3") return VK_F3;
    if (s == "VK_F4" || s == "F4") return VK_F4;
    if (s == "VK_F5" || s == "F5") return VK_F5;
    if (s == "VK_F6" || s == "F6") return VK_F6;
    if (s == "VK_F7" || s == "F7") return VK_F7;
    if (s == "VK_F8" || s == "F8") return VK_F8;
    if (s == "VK_F9" || s == "F9") return VK_F9;
    if (s == "VK_F10" || s == "F10") return VK_F10;
    if (s == "VK_F11" || s == "F11") return VK_F11;
    if (s == "VK_F12" || s == "F12") return VK_F12;
    if (s == "VK_LMENU" || s == "LeftAlt" || s == "Alt") return VK_LMENU;
    if (s == "VK_LCONTROL" || s == "LeftCtrl" || s == "Ctrl") return VK_LCONTROL;
    if (s == "VK_LSHIFT" || s == "LeftShift" || s == "Shift") return VK_LSHIFT;
    if (s == "VK_MBUTTON" || s == "MiddleMouse") return VK_MBUTTON;
    if (s == "VK_XBUTTON1" || s == "Mouse4") return VK_XBUTTON1;
    if (s == "VK_XBUTTON2" || s == "Mouse5") return VK_XBUTTON2;

    try
    {
        return std::stoi(s, nullptr, 0);
    }
    catch (...)
    {
        return defaultKey;
    }
}

std::string ModConfig::KeyToString(int vkCode)
{
    switch (vkCode)
    {
    case VK_F1: return "VK_F1";
    case VK_F2: return "VK_F2";
    case VK_F3: return "VK_F3";
    case VK_F4: return "VK_F4";
    case VK_F5: return "VK_F5";
    case VK_F6: return "VK_F6";
    case VK_F7: return "VK_F7";
    case VK_F8: return "VK_F8";
    case VK_F9: return "VK_F9";
    case VK_F10: return "VK_F10";
    case VK_F11: return "VK_F11";
    case VK_F12: return "VK_F12";
    case VK_LMENU: return "VK_LMENU";
    case VK_LCONTROL: return "VK_LCONTROL";
    case VK_LSHIFT: return "VK_LSHIFT";
    case VK_MBUTTON: return "VK_MBUTTON";
    case VK_XBUTTON1: return "VK_XBUTTON1";
    case VK_XBUTTON2: return "VK_XBUTTON2";
    default: return std::to_string(vkCode);
    }
}

std::string ModConfig::GetDefaultIniPath()
{
    // Try current executable directory first
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::filesystem::path p(exePath);
    std::filesystem::path dir = p.parent_path();

    std::filesystem::path localIni = dir / "AbioticTrackIR.ini";
    if (std::filesystem::exists(localIni))
    {
        return localIni.string();
    }

    // Default to alongside game binary or in working directory
    return (dir / "AbioticTrackIR.ini").string();
}

bool ModConfig::Load(const std::string& iniPath, TrackingConfig& outConfig, int& outUdpPort)
{
    std::ifstream file(iniPath);
    if (!file.is_open())
    {
        // Save defaults if file doesn't exist
        Save(iniPath, outConfig, 4242);
        return false;
    }

    std::string line;
    std::string currentSection;

    auto parseBool = [](const std::string& s, bool def) -> bool {
        std::string lower = s;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower == "true" || lower == "1" || lower == "yes") return true;
        if (lower == "false" || lower == "0" || lower == "no") return false;
        return def;
    };

    auto parseDouble = [](const std::string& s, double def) -> double {
        try { return std::stod(s); } catch (...) { return def; }
    };

    auto parseInt = [](const std::string& s, int def) -> int {
        try { return std::stoi(s); } catch (...) { return def; }
    };

    while (std::getline(file, line))
    {
        // Strip comments
        size_t commentIdx = line.find(';');
        if (commentIdx != std::string::npos) line = line.substr(0, commentIdx);
        commentIdx = line.find('#');
        if (commentIdx != std::string::npos) line = line.substr(0, commentIdx);

        std::string trimmed = Trim(line);
        if (trimmed.empty()) continue;

        if (trimmed.front() == '[' && trimmed.back() == ']')
        {
            currentSection = trimmed.substr(1, trimmed.size() - 2);
            continue;
        }

        size_t eqIdx = trimmed.find('=');
        if (eqIdx == std::string::npos) continue;

        std::string key = Trim(trimmed.substr(0, eqIdx));
        std::string val = Trim(trimmed.substr(eqIdx + 1));

        if (currentSection == "General")
        {
            if (key == "Enabled") outConfig.Enabled = parseBool(val, outConfig.Enabled);
            else if (key == "AimCoupled") outConfig.AimCoupled = parseBool(val, outConfig.AimCoupled);
            else if (key == "PauseInMenus") outConfig.PauseInMenus = parseBool(val, outConfig.PauseInMenus);
            else if (key == "UdpPort") outUdpPort = parseInt(val, 4242);
        }
        else if (currentSection == "Sensitivity")
        {
            if (key == "Yaw")   outConfig.SensYaw   = parseDouble(val, outConfig.SensYaw);
            else if (key == "Pitch") outConfig.SensPitch = parseDouble(val, outConfig.SensPitch);
            else if (key == "Roll")  outConfig.SensRoll  = parseDouble(val, outConfig.SensRoll);
            else if (key == "X")     outConfig.SensX     = parseDouble(val, outConfig.SensX);
            else if (key == "Y")     outConfig.SensY     = parseDouble(val, outConfig.SensY);
            else if (key == "Z")     outConfig.SensZ     = parseDouble(val, outConfig.SensZ);
            else if (key == "ScaleSensWithFOV") outConfig.ScaleSensWithFOV = parseBool(val, outConfig.ScaleSensWithFOV);
        }
        else if (currentSection == "Limits")
        {
            if (key == "MaxTranslationX_Cm") outConfig.MaxTranslationX_Cm = parseDouble(val, outConfig.MaxTranslationX_Cm);
            else if (key == "MaxTranslationY_Cm") outConfig.MaxTranslationY_Cm = parseDouble(val, outConfig.MaxTranslationY_Cm);
            else if (key == "MaxTranslationZ_Cm") outConfig.MaxTranslationZ_Cm = parseDouble(val, outConfig.MaxTranslationZ_Cm);
        }
        else if (currentSection == "Inversion")
        {
            if (key == "InvertYaw")   outConfig.InvertYaw   = parseBool(val, outConfig.InvertYaw);
            else if (key == "InvertPitch") outConfig.InvertPitch = parseBool(val, outConfig.InvertPitch);
            else if (key == "InvertRoll")  outConfig.InvertRoll  = parseBool(val, outConfig.InvertRoll);
            else if (key == "InvertX")     outConfig.InvertX     = parseBool(val, outConfig.InvertX);
            else if (key == "InvertY")     outConfig.InvertY     = parseBool(val, outConfig.InvertY);
            else if (key == "InvertZ")     outConfig.InvertZ     = parseBool(val, outConfig.InvertZ);
        }
        else if (currentSection == "Smoothing")
        {
            if (key == "Factor") outConfig.SmoothingFactor = parseDouble(val, outConfig.SmoothingFactor);
            else if (key == "DeadzoneDegrees") outConfig.DeadzoneDegrees = parseDouble(val, outConfig.DeadzoneDegrees);
            else if (key == "DeadzoneTransCm") outConfig.DeadzoneTransCm = parseDouble(val, outConfig.DeadzoneTransCm);
        }
        else if (currentSection == "NeckModel")
        {
            if (key == "Enabled") outConfig.UseNeckModel = parseBool(val, outConfig.UseNeckModel);
            else if (key == "PivotForwardCm") outConfig.NeckPivotForwardCm = parseDouble(val, outConfig.NeckPivotForwardCm);
            else if (key == "PivotUpCm")      outConfig.NeckPivotUpCm      = parseDouble(val, outConfig.NeckPivotUpCm);
        }
        else if (currentSection == "Parallax3D")
        {
            if (key == "Enabled") outConfig.Parallax3DEnabled = parseBool(val, outConfig.Parallax3DEnabled);
            else if (key == "MonitorWidthCm") outConfig.Monitor.WidthCm = parseDouble(val, outConfig.Monitor.WidthCm);
            else if (key == "MonitorHeightCm") outConfig.Monitor.HeightCm = parseDouble(val, outConfig.Monitor.HeightCm);
            else if (key == "ViewingDistanceCm") outConfig.Monitor.ViewingDistanceCm = parseDouble(val, outConfig.Monitor.ViewingDistanceCm);
            else if (key == "Intensity") outConfig.Monitor.ParallaxIntensity = parseDouble(val, outConfig.Monitor.ParallaxIntensity);
            else if (key == "DynamicFOV") outConfig.Monitor.DynamicFOV = parseBool(val, outConfig.Monitor.DynamicFOV);
            else if (key == "MaxOffsetFrustum") outConfig.Monitor.MaxOffsetFrustum = parseDouble(val, outConfig.Monitor.MaxOffsetFrustum);
            else if (key == "InvertParallaxX") outConfig.Monitor.InvertParallaxX = parseBool(val, outConfig.Monitor.InvertParallaxX);
            else if (key == "InvertParallaxY") outConfig.Monitor.InvertParallaxY = parseBool(val, outConfig.Monitor.InvertParallaxY);
        }
        else if (currentSection == "Hotkeys")
        {
            if (key == "Center") outConfig.HotkeyCenter = ParseKeyString(val, VK_F12);
            else if (key == "ToggleTracking") outConfig.HotkeyToggleTracking = ParseKeyString(val, VK_F11);
            else if (key == "ToggleParallax") outConfig.HotkeyToggleParallax = ParseKeyString(val, VK_F10);
            else if (key == "Freelook") outConfig.HotkeyFreelook = ParseKeyString(val, VK_LMENU);
        }
    }

    return true;
}

bool ModConfig::Save(const std::string& iniPath, const TrackingConfig& config, int udpPort)
{
    std::ofstream file(iniPath);
    if (!file.is_open()) return false;

    file << "; =====================================================================\n"
         << "; Abiotic Factor - TrackIR & OpenTrack 6-DOF + Parallax 3D Mod Config\n"
         << "; =====================================================================\n\n"
         << "[General]\n"
         << "Enabled = " << (config.Enabled ? "true" : "false") << "\n"
         << "AimCoupled = " << (config.AimCoupled ? "true" : "false") << "\n"
         << "PauseInMenus = " << (config.PauseInMenus ? "true" : "false") << "\n"
         << "UdpPort = " << udpPort << "\n\n"
         << "[Sensitivity]\n"
         << "Yaw = " << config.SensYaw << "\n"
         << "Pitch = " << config.SensPitch << "\n"
         << "Roll = " << config.SensRoll << "\n"
         << "X = " << config.SensX << "\n"
         << "Y = " << config.SensY << "\n"
         << "Z = " << config.SensZ << "\n"
         << "ScaleSensWithFOV = " << (config.ScaleSensWithFOV ? "true" : "false") << "\n\n"
         << "[Limits]\n"
         << "; Safety translation clamps in centimeters to prevent clipping through geometry\n"
         << "MaxTranslationX_Cm = " << config.MaxTranslationX_Cm << "\n"
         << "MaxTranslationY_Cm = " << config.MaxTranslationY_Cm << "\n"
         << "MaxTranslationZ_Cm = " << config.MaxTranslationZ_Cm << "\n\n"
         << "[Inversion]\n"
         << "InvertYaw = " << (config.InvertYaw ? "true" : "false") << "\n"
         << "InvertPitch = " << (config.InvertPitch ? "true" : "false") << "\n"
         << "InvertRoll = " << (config.InvertRoll ? "true" : "false") << "\n"
         << "InvertX = " << (config.InvertX ? "true" : "false") << "\n"
         << "InvertY = " << (config.InvertY ? "true" : "false") << "\n"
         << "InvertZ = " << (config.InvertZ ? "true" : "false") << "\n\n"
         << "[Smoothing]\n"
         << "; Factor: 0.0 = completely raw (immediate), 0.7 = heavy filter\n"
         << "Factor = " << config.SmoothingFactor << "\n"
         << "DeadzoneDegrees = " << config.DeadzoneDegrees << "\n"
         << "DeadzoneTransCm = " << config.DeadzoneTransCm << "\n\n"
         << "[NeckModel]\n"
         << "Enabled = " << (config.UseNeckModel ? "true" : "false") << "\n"
         << "PivotForwardCm = " << config.NeckPivotForwardCm << "\n"
         << "PivotUpCm = " << config.NeckPivotUpCm << "\n\n"
         << "[Parallax3D]\n"
         << "; Off-axis perspective projection for authentic depth illusion\n"
         << "Enabled = " << (config.Parallax3DEnabled ? "true" : "false") << "\n"
         << "; 24\": 53.1x29.9 | 27\": 59.7x33.6 | 32\": 69.8x39.3 | 34\" UW: 79.8x33.4\n"
         << "MonitorWidthCm = " << config.Monitor.WidthCm << "\n"
         << "MonitorHeightCm = " << config.Monitor.HeightCm << "\n"
         << "ViewingDistanceCm = " << config.Monitor.ViewingDistanceCm << "\n"
         << "Intensity = " << config.Monitor.ParallaxIntensity << "\n"
         << "DynamicFOV = " << (config.Monitor.DynamicFOV ? "true" : "false") << "\n"
         << "MaxOffsetFrustum = " << config.Monitor.MaxOffsetFrustum << "\n\n"
         << "[Hotkeys]\n"
         << "; Available: VK_F1..VK_F12, LeftAlt, LeftCtrl, LeftShift, MiddleMouse, Mouse4, Mouse5\n"
         << "Center = " << KeyToString(config.HotkeyCenter) << "\n"
         << "ToggleTracking = " << KeyToString(config.HotkeyToggleTracking) << "\n"
         << "ToggleParallax = " << KeyToString(config.HotkeyToggleParallax) << "\n"
         << "Freelook = " << KeyToString(config.HotkeyFreelook) << "\n";

    return true;
}
