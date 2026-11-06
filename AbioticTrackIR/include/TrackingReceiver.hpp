#pragma once
#include <winsock2.h>
#include <windows.h>
#include <cstdint>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>

enum class TrackingSource
{
    None = 0,
    FreeTrackSharedMem,
    OpenTrackUDP,
    NPClient64
};

struct RawTrackingPose
{
    // Rotations in degrees
    double Yaw{0.0};
    double Pitch{0.0};
    double Roll{0.0};

    // Translations in centimeters
    double X{0.0};
    double Y{0.0};
    double Z{0.0};

    uint32_t FrameId{0};
    double TimestampSec{0.0};
    bool IsValid{false};
};

#pragma pack(push, 1)
struct FTData
{
    int32_t DataID;
    int32_t CamWidth;
    int32_t CamHeight;
    float Yaw;    // Radians
    float Pitch;  // Radians
    float Roll;   // Radians
    float X;      // Millimeters
    float Y;      // Millimeters
    float Z;      // Millimeters
    float RawYaw;
    float RawPitch;
    float RawRoll;
    float RawX;
    float RawY;
    float RawZ;
};
#pragma pack(pop)

class TrackingReceiver
{
public:
    TrackingReceiver();
    ~TrackingReceiver();

    bool Start(int udpPort = 4242, bool preferSharedMem = true);
    void Stop();

    RawTrackingPose GetLatestPose();
    TrackingSource GetActiveSource() const;
    std::string GetActiveSourceName() const;
    uint32_t GetPacketsPerSecond();

private:
    void ReceiverLoop();
    bool PollSharedMemory(RawTrackingPose& outPose);
    bool InitUDP(int port);
    void CloseUDP();
    bool PollUDP(RawTrackingPose& outPose);

    std::atomic<bool> m_running{false};
    std::thread m_thread;
    mutable std::mutex m_mutex;

    int m_udpPort{4242};
    bool m_preferSharedMem{true};

    // Shared Memory handle
    HANDLE m_hSharedMem{NULL};
    FTData* m_pFTData{nullptr};
    int32_t m_lastDataId{-1};
    int m_staleCount{0};

    // UDP socket
    uintptr_t m_udpSocket{static_cast<uintptr_t>(~0)}; // INVALID_SOCKET

    // Telemetry
    std::atomic<TrackingSource> m_activeSource{TrackingSource::None};
    RawTrackingPose m_latestPose;
    std::atomic<uint32_t> m_fpsCounter{0};
    std::atomic<uint32_t> m_currentFPS{0};
    double m_lastFpsTime{0.0};
};
