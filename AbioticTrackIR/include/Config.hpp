#pragma once
#include "TrackingEngine.hpp"
#include <string>

class ModConfig
{
public:
    static bool Load(const std::string& iniPath, TrackingConfig& outConfig, int& outUdpPort);
    static bool Save(const std::string& iniPath, const TrackingConfig& config, int udpPort);
    static std::string GetDefaultIniPath();

private:
    static int ParseKeyString(const std::string& keyStr, int defaultKey);
    static std::string KeyToString(int vkCode);
};
