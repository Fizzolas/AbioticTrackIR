#pragma once
#include "UnrealTypes.hpp"
#include "TrackingReceiver.hpp"
#include "Parallax3D.hpp"
#include <mutex>

struct TrackingConfig
{
    // Enable toggles
    bool Enabled{true};
    bool Parallax3DEnabled{false}; // Disabled by default to prevent nausea and projection warping
    bool DecoupledFreelookActive{false};
    bool AimCoupled{false}; // If true, head tracking also steers character crosshair/aim
    bool PauseInMenus{true}; // Pause head-look while inventory/crafting mouse cursor is visible

    // Sensitivities
    double SensYaw{1.0};
    double SensPitch{1.0};
    double SensRoll{0.8};
    double SensX{1.0};
    double SensY{1.0};
    double SensZ{1.0};
    bool ScaleSensWithFOV{true}; // Dynamically damp sensitivity when aiming down sights (ADS)

    // Translation limits (cm) to prevent camera clipping through walls
    double MaxTranslationX_Cm{25.0};
    double MaxTranslationY_Cm{30.0};
    double MaxTranslationZ_Cm{25.0};

    // Axis Inversions
    bool InvertYaw{false};
    bool InvertPitch{false};
    bool InvertRoll{false};
    bool InvertX{false};
    bool InvertY{false};
    bool InvertZ{false};

    // Smoothing & Filtering (0.0 = raw/no smoothing, 0.9 = heavy smoothing)
    double SmoothingFactor{0.35};
    double DeadzoneDegrees{0.1};
    double DeadzoneTransCm{0.05};

    // Neck model (cm) - Disabled by default to prevent camera clipping inside player mesh
    bool UseNeckModel{false};
    double NeckPivotForwardCm{-1.5};
    double NeckPivotUpCm{-2.0};

    // Monitor & Parallax setup
    MonitorConfig Monitor{};

    // Hotkeys (Virtual Key codes)
    int HotkeyCenter{VK_F12};         // F12: Recenter
    int HotkeyToggleTracking{VK_F11}; // F11: Toggle On/Off
    int HotkeyToggleParallax{VK_F10}; // F10: Toggle Parallax 3D
    int HotkeyFreelook{VK_LMENU};     // Left Alt: Free-look mode
};

struct ProcessedTrackingState
{
    FRotator3d Rotation;       // Pitch, Yaw, Roll in degrees
    FVector3d LocalTranslation;// Local translation in cm
    FVector3d NeckDisplacement;// Eye offset caused by head rotation around neck (cm)
    ParallaxResult Parallax;   // Parallax 3D results (frustum offset, FOV)

    bool IsTrackingActive{false};
    bool IsParallaxActive{false};
    bool IsFreelookActive{false};
};

class TrackingEngine
{
public:
    TrackingEngine();

    void SetConfig(const TrackingConfig& config);
    const TrackingConfig& GetConfig() const;

    void Recenter();
    void ToggleTracking();
    void ToggleParallax();
    void SetFreelook(bool active);

    // Updates filter with raw input and computes final camera transformation (framerate-independent using deltaTime)
    ProcessedTrackingState Process(const RawTrackingPose& rawPose, float baseGameFOV, float deltaTime = 0.016667f);

    // Applies processed tracking directly to Unreal Engine FMinimalViewInfo
    void ApplyToViewInfo(FMinimalViewInfo* pov, const ProcessedTrackingState& state, double actorBaseYawDeg, bool applyRotation = true);

private:
    FVector3d ComputeNeckDisplacement(double pitchDeg, double yawDeg, double rollDeg);

    TrackingConfig m_config;
    mutable std::mutex m_mutex;

    // Calibration offsets
    bool m_hasCenter{false};
    bool m_streamWasLost{true};
    RawTrackingPose m_centerOffset{};

    // Smoothing states
    RawTrackingPose m_filteredPose{};

    Parallax3DEngine m_parallaxEngine;
};
