#pragma once
#include <winsock2.h>
#include <windows.h>
#include <cstdint>
#include "UnrealTypes.hpp"
#include "TrackingReceiver.hpp"
#include "TrackingEngine.hpp"

class HookManager
{
public:
    static HookManager& Instance();

    bool Initialize(TrackingReceiver* receiver, TrackingEngine* engine);
    void Shutdown();

    bool IsHooked() const { return m_hooked; }
    uintptr_t GetTargetAddress() const { return m_targetAddress; }

private:
    HookManager() = default;
    ~HookManager() = default;

    uintptr_t FindTargetAddress();
    static void __fastcall HookedDoUpdateCamera(void* thisCameraManager, float deltaTime);
    void HandlePostUpdate(void* thisCameraManager, float deltaTime);

    TrackingReceiver* m_receiver{nullptr};
    TrackingEngine* m_engine{nullptr};

    uintptr_t m_targetAddress{0};
    uint8_t* m_trampoline{nullptr};
    uint8_t m_originalBytes[10]{0};
    bool m_hooked{false};

    // Hotkey edge triggers
    bool m_centerKeyWasDown{false};
    bool m_trackingKeyWasDown{false};
    bool m_parallaxKeyWasDown{false};
    bool m_depthIncKeyWasDown{false};
    bool m_depthDecKeyWasDown{false};

    // Frame-to-frame delta state for Aim Coupling (prevents infinite angular spin)
    double m_lastCoupledPitch{0.0};
    double m_lastCoupledYaw{0.0};
    bool m_aimCoupledInitialized{false};
};
