#include "HookManager.hpp"
#include <vector>
#include <cstring>
#include <cstdio>

HookManager& HookManager::Instance()
{
    static HookManager s_instance;
    return s_instance;
}

uintptr_t HookManager::FindTargetAddress()
{
    uintptr_t base = (uintptr_t)GetModuleHandleA(NULL);
    if (!base) return 0;

    // Signature of APlayerCameraManager::DoUpdateCamera:
    // 4C 8B DC 48 81 EC 98 08 00 00
    const uint8_t sig[] = { 0x4C, 0x8B, 0xDC, 0x48, 0x81, 0xEC, 0x98, 0x08, 0x00, 0x00 };
    const size_t sigLen = sizeof(sig);

    // Primary: Check known RVA 0x3ac19a0 for Abiotic Factor Win64 Shipping
    uintptr_t primaryAddr = base + 0x3ac19a0;
    if (memcmp((const void*)primaryAddr, sig, sigLen) == 0)
    {
        return primaryAddr;
    }

    // Secondary: Scan .text section if patched or relocated
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);
    PIMAGE_SECTION_HEADER sec = IMAGE_FIRST_SECTION(nt);

    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++)
    {
        if (strncmp((const char*)sec[i].Name, ".text", 5) == 0)
        {
            uint8_t* scanStart = (uint8_t*)(base + sec[i].VirtualAddress);
            size_t scanSize = sec[i].Misc.VirtualSize;

            for (size_t offset = 0; offset + sigLen <= scanSize; offset++)
            {
                if (scanStart[offset] == sig[0] &&
                    memcmp(scanStart + offset, sig, sigLen) == 0)
                {
                    return (uintptr_t)(scanStart + offset);
                }
            }
        }
    }

    return 0;
}

bool HookManager::Initialize(TrackingReceiver* receiver, TrackingEngine* engine)
{
    if (m_hooked) return true;

    m_receiver = receiver;
    m_engine = engine;

    m_targetAddress = FindTargetAddress();
    if (!m_targetAddress)
    {
        return false;
    }

    // Allocate trampoline near target address (within +/- 2GB for rel32 jumps)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    uintptr_t minAddr = (m_targetAddress > 0x70000000) ? (m_targetAddress - 0x70000000) : 0x10000;
    uintptr_t maxAddr = m_targetAddress + 0x70000000;

    // Search for free page near target
    for (uintptr_t addr = (m_targetAddress & ~0xFFFF) - 0x10000; addr >= minAddr; addr -= 0x10000)
    {
        m_trampoline = (uint8_t*)VirtualAlloc((LPVOID)addr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (m_trampoline) break;
    }
    if (!m_trampoline)
    {
        for (uintptr_t addr = (m_targetAddress & ~0xFFFF) + 0x10000; addr <= maxAddr; addr += 0x10000)
        {
            m_trampoline = (uint8_t*)VirtualAlloc((LPVOID)addr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (m_trampoline) break;
        }
    }
    if (!m_trampoline)
    {
        // Fallback: any address with 64-bit jump
        m_trampoline = (uint8_t*)VirtualAlloc(NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    }
    if (!m_trampoline) return false;

    // Save original 10 bytes:
    // 4C 8B DC (mov r11, rsp - 3 bytes)
    // 48 81 EC 98 08 00 00 (sub rsp, 0x898 - 7 bytes)
    memcpy(m_originalBytes, (const void*)m_targetAddress, 10);

    // Build Trampoline:
    // 1. Original 10 instructions
    memcpy(m_trampoline, m_originalBytes, 10);
    // 2. Jump back to Target + 10:
    // FF 25 00 00 00 00 [Target + 10 (8 bytes)]
    m_trampoline[10] = 0xFF;
    m_trampoline[11] = 0x25;
    m_trampoline[12] = 0x00;
    m_trampoline[13] = 0x00;
    m_trampoline[14] = 0x00;
    m_trampoline[15] = 0x00;
    uintptr_t returnAddr = m_targetAddress + 10;
    memcpy(m_trampoline + 16, &returnAddr, sizeof(returnAddr));

    // Relay stub in trampoline page for HookedDoUpdateCamera (at offset 0x40):
    // FF 25 00 00 00 00 [HookedDoUpdateCamera (8 bytes)]
    uint8_t* relayStub = m_trampoline + 0x40;
    relayStub[0] = 0xFF;
    relayStub[1] = 0x25;
    relayStub[2] = 0x00;
    relayStub[3] = 0x00;
    relayStub[4] = 0x00;
    relayStub[5] = 0x00;
    uintptr_t hookAddr = (uintptr_t)&HookManager::HookedDoUpdateCamera;
    memcpy(relayStub + 6, &hookAddr, sizeof(hookAddr));

    // Hook TargetAddress:
    // Write 5-byte relative call/jump to relayStub: E9 [rel32] + 5 NOPs
    DWORD oldProtect;
    VirtualProtect((LPVOID)m_targetAddress, 10, PAGE_EXECUTE_READWRITE, &oldProtect);

    int64_t diff = (int64_t)relayStub - (int64_t)(m_targetAddress + 5);
    if (diff < INT32_MIN || diff > INT32_MAX)
    {
        VirtualFree(m_trampoline, 0, MEM_RELEASE);
        m_trampoline = nullptr;
        return false;
    }

    int32_t rel32 = (int32_t)diff;
    uint8_t patch[10];
    patch[0] = 0xE9;
    memcpy(patch + 1, &rel32, 4);
    memset(patch + 5, 0x90, 5); // 5 NOPs

    memcpy((LPVOID)m_targetAddress, patch, 10);

    VirtualProtect((LPVOID)m_targetAddress, 10, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), (LPCVOID)m_targetAddress, 10);

    m_hooked = true;
    return true;
}

void HookManager::Shutdown()
{
    if (!m_hooked || !m_targetAddress) return;

    DWORD oldProtect;
    VirtualProtect((LPVOID)m_targetAddress, 10, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy((LPVOID)m_targetAddress, m_originalBytes, 10);
    VirtualProtect((LPVOID)m_targetAddress, 10, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), (LPCVOID)m_targetAddress, 10);

    if (m_trampoline)
    {
        VirtualFree(m_trampoline, 0, MEM_RELEASE);
        m_trampoline = nullptr;
    }

    m_hooked = false;
}

void __fastcall HookManager::HookedDoUpdateCamera(void* thisCameraManager, float deltaTime)
{
    HookManager& mgr = HookManager::Instance();

    // 1. Call original function via trampoline
    typedef void (__fastcall* DoUpdateCameraFn)(void*, float);
    DoUpdateCameraFn originalFn = (DoUpdateCameraFn)mgr.m_trampoline;
    originalFn(thisCameraManager, deltaTime);

    // 2. Perform post-update TrackIR / OpenTrack & Parallax 3D injection
    mgr.HandlePostUpdate(thisCameraManager, deltaTime);
}

void HookManager::HandlePostUpdate(void* thisCameraManager, float deltaTime)
{
    if (!thisCameraManager || !m_receiver || !m_engine) return;

    uintptr_t baseAddr = (uintptr_t)thisCameraManager;
    if (baseAddr < 0x10000 || baseAddr > 0x00007FFFFFFFFFFFULL) return;

    // In Unreal Engine 5.4 APlayerCameraManager:
    // CameraCachePrivate is an FCameraCacheEntry at offset 0x13a0.
    // Inside FCameraCacheEntry, TimeStamp is at +0x00, and POV is at +0x10 -> 0x13a0 + 0x10 = 0x13b0!
    // ViewTarget is an FTViewTarget at offset 0x330.
    // Inside FTViewTarget, Target is at +0x00, and POV is at +0x10 -> 0x330 + 0x10 = 0x340!
    FMinimalViewInfo* povCache = (FMinimalViewInfo*)(baseAddr + 0x13b0);
    FMinimalViewInfo* povVT    = (FMinimalViewInfo*)(baseAddr + 0x340);

    // Sanity checks on pointers (FOV must be within realistic bounds, supporting sniper zoom):
    bool cacheValid = (povCache->FOV >= 3.0f && povCache->FOV <= 170.0f);
    bool vtValid    = (povVT->FOV >= 3.0f && povVT->FOV <= 170.0f);

    if (!cacheValid && !vtValid) return;

    const auto& config = m_engine->GetConfig();

    // Hotkey edge detections (only when the game window is currently focused)
    HWND fgWnd = GetForegroundWindow();
    DWORD fgPid = 0;
    if (fgWnd)
    {
        GetWindowThreadProcessId(fgWnd, &fgPid);
    }
    bool isGameFocused = (fgPid == GetCurrentProcessId());

    if (isGameFocused)
    {
        auto isKeyDown = [](int vk) -> bool {
            return (GetAsyncKeyState(vk) & 0x8000) != 0;
        };

        if (config.HotkeyCenter != 0)
        {
            bool down = isKeyDown(config.HotkeyCenter);
            if (down && !m_centerKeyWasDown)
            {
                m_engine->Recenter();
                m_aimCoupledInitialized = false;
            }
            m_centerKeyWasDown = down;
        }

        if (config.HotkeyToggleTracking != 0)
        {
            bool down = isKeyDown(config.HotkeyToggleTracking);
            if (down && !m_trackingKeyWasDown)
            {
                m_engine->ToggleTracking();
                m_aimCoupledInitialized = false;
            }
            m_trackingKeyWasDown = down;
        }

        if (config.HotkeyToggleParallax != 0)
        {
            bool down = isKeyDown(config.HotkeyToggleParallax);
            if (down && !m_parallaxKeyWasDown) m_engine->ToggleParallax();
            m_parallaxKeyWasDown = down;
        }

        if (config.HotkeyFreelook != 0)
        {
            bool down = isKeyDown(config.HotkeyFreelook);
            m_engine->SetFreelook(down);
        }
    }
    else
    {
        m_centerKeyWasDown = false;
        m_trackingKeyWasDown = false;
        m_parallaxKeyWasDown = false;
    }

    // Check if player is interacting with inventory, crafting, or terminal menus
    uintptr_t pcPtr = *(uintptr_t*)(baseAddr + 0x298); // PCOwner (APlayerController*)
    if (pcPtr < 0x10000 || pcPtr > 0x00007FFFFFFFFFFFULL) pcPtr = 0;

    if (pcPtr && config.PauseInMenus)
    {
        // bShowMouseCursor in APlayerController is at offset +0x54c (bit 0)
        uint8_t cursorFlags = *(uint8_t*)(pcPtr + 0x54c);
        bool cursorVisible = (cursorFlags & 0x01) != 0;
        if (cursorVisible)
        {
            // Do not alter view or control rotation while menu cursor is active; align frustum straight
            if (cacheValid) { povCache->OffCenterProjectionOffset = {0.0, 0.0}; }
            if (vtValid)    { povVT->OffCenterProjectionOffset = {0.0, 0.0}; }
            m_aimCoupledInitialized = false;
            return;
        }
    }

    // Query raw tracking from OpenTrack / FreeTrack
    RawTrackingPose raw = m_receiver->GetLatestPose();

    float baseFOV = cacheValid ? povCache->FOV : povVT->FOV;
    ProcessedTrackingState state = m_engine->Process(raw, baseFOV, deltaTime);

    if (!state.IsTrackingActive)
    {
        m_aimCoupledInitialized = false;
        return;
    }

    double baseYaw = cacheValid ? povCache->Rotation.Yaw : povVT->Rotation.Yaw;

    // Aim coupling vs Decoupled View:
    // If AimCoupled is active and not in free-look, head tracking steers ControlRotation (crosshair)
    // via framerate-independent deltas, and camera Pitch/Yaw is driven through the character pawn.
    bool applyCamRotation = true;
    if (config.AimCoupled && !state.IsFreelookActive && pcPtr)
    {
        applyCamRotation = false; // Pawn control rotation directly drives camera pitch & yaw

        FRotator3d* pControlRot = (FRotator3d*)(pcPtr + 0x310);
        if (pControlRot && std::abs(pControlRot->Pitch) <= 90.0)
        {
            if (!m_aimCoupledInitialized)
            {
                m_lastCoupledPitch = state.Rotation.Pitch;
                m_lastCoupledYaw   = state.Rotation.Yaw;
                m_aimCoupledInitialized = true;
            }
            else
            {
                double deltaPitch = FRotator3d::NormalizeAxis(state.Rotation.Pitch - m_lastCoupledPitch);
                double deltaYaw   = FRotator3d::NormalizeAxis(state.Rotation.Yaw   - m_lastCoupledYaw);

                m_lastCoupledPitch = state.Rotation.Pitch;
                m_lastCoupledYaw   = state.Rotation.Yaw;

                pControlRot->Pitch = std::clamp(FRotator3d::NormalizeAxis(pControlRot->Pitch + deltaPitch), -89.0, 89.0);
                pControlRot->Yaw   = FRotator3d::NormalizeAxis(pControlRot->Yaw + deltaYaw);
            }
        }
    }
    else
    {
        m_aimCoupledInitialized = false;
    }

    // Apply to both CameraCachePrivate (rendered by Viewport) and ViewTarget
    if (cacheValid)
    {
        m_engine->ApplyToViewInfo(povCache, state, baseYaw, applyCamRotation);
    }
    if (vtValid)
    {
        m_engine->ApplyToViewInfo(povVT, state, baseYaw, applyCamRotation);
    }
}
