#include "TrackingReceiver.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <chrono>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define RAD_TO_DEG(x) ((x) * (180.0 / M_PI))

TrackingReceiver::TrackingReceiver()
{
}

TrackingReceiver::~TrackingReceiver()
{
    Stop();
}

bool TrackingReceiver::Start(int udpPort, bool preferSharedMem)
{
    if (m_running.load()) return true;

    m_udpPort = udpPort;
    m_preferSharedMem = preferSharedMem;

    // Initialize Winsock for UDP receiver
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    InitUDP(m_udpPort);

    m_running.store(true);
    m_thread = std::thread(&TrackingReceiver::ReceiverLoop, this);

    return true;
}

void TrackingReceiver::Stop()
{
    if (!m_running.load()) return;

    m_running.store(false);
    if (m_thread.joinable())
    {
        m_thread.join();
    }

    CloseUDP();

    if (m_pFTData)
    {
        UnmapViewOfFile(m_pFTData);
        m_pFTData = nullptr;
    }
    if (m_hSharedMem)
    {
        CloseHandle(m_hSharedMem);
        m_hSharedMem = NULL;
    }

    WSACleanup();
}

bool TrackingReceiver::InitUDP(int port)
{
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET) return false;

    // Set non-blocking mode
    u_long mode = 1;
    ioctlsocket(s, FIONBIO, &mode);

    // Allow reuse
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons((u_short)port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        closesocket(s);
        return false;
    }

    m_udpSocket = static_cast<uintptr_t>(s);
    return true;
}

void TrackingReceiver::CloseUDP()
{
    if (m_udpSocket != static_cast<uintptr_t>(~0))
    {
        closesocket(static_cast<SOCKET>(m_udpSocket));
        m_udpSocket = static_cast<uintptr_t>(~0);
    }
}

bool TrackingReceiver::PollSharedMemory(RawTrackingPose& outPose)
{
    // Try to open shared memory if not already open
    if (!m_pFTData)
    {
        if (!m_hSharedMem)
        {
            m_hSharedMem = OpenFileMappingA(FILE_MAP_READ, FALSE, "FT_SharedMem");
        }
        if (m_hSharedMem)
        {
            m_pFTData = (FTData*)MapViewOfFile(m_hSharedMem, FILE_MAP_READ, 0, 0, sizeof(FTData));
        }
    }

    if (!m_pFTData) return false;

    int32_t currentId = m_pFTData->DataID;
    
    // Check if DataID has actually incremented
    if (currentId == m_lastDataId)
    {
        m_staleCount++;
        // If data hasn't updated for > 150 polls (~300ms), OpenTrack is paused or stopped
        if (m_staleCount > 150)
        {
            // Close view so we can reconnect if OpenTrack re-creates mapping
            UnmapViewOfFile(m_pFTData);
            m_pFTData = nullptr;
            if (m_hSharedMem)
            {
                CloseHandle(m_hSharedMem);
                m_hSharedMem = NULL;
            }
            m_staleCount = 0;
            return false;
        }
        // Frame is not new
        return false;
    }

    // Double-checked seqlock copy for torn-frame protection against concurrent OpenTrack writes
    FTData snapshot;
    int32_t idBefore = m_pFTData->DataID;
    memcpy(&snapshot, (const void*)m_pFTData, sizeof(FTData));
    int32_t idAfter = m_pFTData->DataID;

    if (idBefore != idAfter)
    {
        // OpenTrack updated memory during copy; ignore torn frame and poll next cycle
        return false;
    }

    // New valid frame received
    m_staleCount = 0;
    outPose.FrameId = (uint32_t)idBefore;
    
    // FreeTrack rotations are in radians; convert to degrees
    outPose.Yaw   = RAD_TO_DEG(snapshot.Yaw);
    outPose.Pitch = RAD_TO_DEG(snapshot.Pitch);
    outPose.Roll  = RAD_TO_DEG(snapshot.Roll);

    // FreeTrack translations are in millimeters; convert to centimeters
    outPose.X = snapshot.X * 0.1;
    outPose.Y = snapshot.Y * 0.1;
    outPose.Z = snapshot.Z * 0.1;

    outPose.IsValid = true;
    m_lastDataId = idBefore;
    return true;
}

bool TrackingReceiver::PollUDP(RawTrackingPose& outPose)
{
    if (m_udpSocket == static_cast<uintptr_t>(~0)) return false;

    // OpenTrack UDP protocol: 6 doubles (48 bytes)
    // [x, y, z, yaw, pitch, roll]
    double buffer[6]{0.0};
    sockaddr_in clientAddr{};
    int clientLen = sizeof(clientAddr);

    int bytesReceived = 0;
    int latestBytes = 0;

    // Drain socket buffer to get the absolute newest packet
    while (true)
    {
        int res = recvfrom(static_cast<SOCKET>(m_udpSocket), (char*)buffer, sizeof(buffer), 0,
                           (sockaddr*)&clientAddr, &clientLen);
        if (res == 48)
        {
            latestBytes = res;
        }
        else
        {
            break;
        }
    }

    if (latestBytes == 48)
    {
        // buffer[0]=X (cm), buffer[1]=Y (cm), buffer[2]=Z (cm)
        // buffer[3]=Yaw (deg), buffer[4]=Pitch (deg), buffer[5]=Roll (deg)
        outPose.X     = buffer[0];
        outPose.Y     = buffer[1];
        outPose.Z     = buffer[2];
        outPose.Yaw   = buffer[3];
        outPose.Pitch = buffer[4];
        outPose.Roll  = buffer[5];
        outPose.IsValid = true;
        outPose.FrameId++;
        return true;
    }

    return false;
}

void TrackingReceiver::ReceiverLoop()
{
    using clock = std::chrono::steady_clock;
    auto lastFpsUpdate = clock::now();
    uint32_t packetCount = 0;

    while (m_running.load())
    {
        RawTrackingPose pose{};
        bool gotData = false;
        TrackingSource source = TrackingSource::None;

        if (m_preferSharedMem)
        {
            if (PollSharedMemory(pose))
            {
                gotData = true;
                source = TrackingSource::FreeTrackSharedMem;
            }
            else if (PollUDP(pose))
            {
                gotData = true;
                source = TrackingSource::OpenTrackUDP;
            }
        }
        else
        {
            if (PollUDP(pose))
            {
                gotData = true;
                source = TrackingSource::OpenTrackUDP;
            }
            else if (PollSharedMemory(pose))
            {
                gotData = true;
                source = TrackingSource::FreeTrackSharedMem;
            }
        }

        auto now = clock::now();
        double elapsedSec = std::chrono::duration<double>(now - lastFpsUpdate).count();

        if (gotData)
        {
            pose.TimestampSec = std::chrono::duration<double>(now.time_since_epoch()).count();
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_latestPose = pose;
            }
            m_activeSource.store(source);
            packetCount++;
        }

        if (elapsedSec >= 1.0)
        {
            uint32_t fps = (uint32_t)(packetCount / elapsedSec);
            m_currentFPS.store(fps);
            if (fps == 0)
            {
                m_activeSource.store(TrackingSource::None);
                std::lock_guard<std::mutex> lock(m_mutex);
                m_latestPose.IsValid = false;
            }
            packetCount = 0;
            lastFpsUpdate = now;
        }

        // High frequency poll: ~2ms sleep gives ~500Hz poll rate with near-zero CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

RawTrackingPose TrackingReceiver::GetLatestPose()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_latestPose;
}

TrackingSource TrackingReceiver::GetActiveSource() const
{
    return m_activeSource.load();
}

std::string TrackingReceiver::GetActiveSourceName() const
{
    switch (m_activeSource.load())
    {
    case TrackingSource::FreeTrackSharedMem: return "OpenTrack (FreeTrack SharedMem)";
    case TrackingSource::OpenTrackUDP:        return "OpenTrack (UDP 4242)";
    case TrackingSource::NPClient64:         return "TrackIR (NPClient64)";
    default:                                 return "Disconnected / Searching...";
    }
}

uint32_t TrackingReceiver::GetPacketsPerSecond()
{
    return m_currentFPS.load();
}
