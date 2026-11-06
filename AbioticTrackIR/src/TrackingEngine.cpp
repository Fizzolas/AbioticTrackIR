#include "TrackingEngine.hpp"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEG_TO_RAD(x) ((x) * (M_PI / 180.0))
#define RAD_TO_DEG(x) ((x) * (180.0 / M_PI))

TrackingEngine::TrackingEngine()
{
    m_parallaxEngine.SetConfig(m_config.Monitor);
}

void TrackingEngine::SetConfig(const TrackingConfig& config)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config = config;
    m_parallaxEngine.SetConfig(config.Monitor);
}

const TrackingConfig& TrackingEngine::GetConfig() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_config;
}

void TrackingEngine::Recenter()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_hasCenter = false; // Next process cycle captures center
}

void TrackingEngine::ToggleTracking()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config.Enabled = !m_config.Enabled;
}

void TrackingEngine::ToggleParallax()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config.Parallax3DEnabled = !m_config.Parallax3DEnabled;
}

void TrackingEngine::AdjustFocalDepth(double deltaCm)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    double cur = m_parallaxEngine.GetFocalDistance();
    double updated = std::clamp(cur + deltaCm, 30.0, 500.0);
    m_parallaxEngine.SetFocalDistance(updated);
    m_config.Monitor.FocalDistanceCm = updated;
}

double TrackingEngine::GetFocalDepth() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_parallaxEngine.GetFocalDistance();
}

void TrackingEngine::SetFreelook(bool active)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config.DecoupledFreelookActive = active;
}

FVector3d TrackingEngine::ComputeNeckDisplacement(double pitchDeg, double yawDeg, double rollDeg)
{
    // Anthropomorphic cervical spine neck model
    // Rigid body kinematic displacement of eyes relative to neck rotation joint:
    // dx = distance from cervical joint to eyes along forward axis (typically ~8 cm)
    // dz = distance from cervical joint to eyes along vertical axis (typically ~12 cm)
    double p = DEG_TO_RAD(pitchDeg);
    double y = DEG_TO_RAD(yawDeg);

    double dx = std::abs(m_config.NeckPivotForwardCm);
    double dz = std::abs(m_config.NeckPivotUpCm);

    // When looking UP (p > 0 in UE): eyes rotate backward (-X) and upward (+Z)
    // When looking DOWN (p < 0 in UE): eyes rotate forward (+X) and downward (-Z)
    // When turning RIGHT (y > 0 in UE): eyes swing to the right (+Y) and slightly back (-X)
    double dispX = dx * (std::cos(p) * std::cos(y) - 1.0) - dz * std::sin(p);
    double dispY = dx * std::sin(y);
    double dispZ = dx * std::sin(p) + dz * (std::cos(p) - 1.0);

    // Safety bounds to prevent camera from ever penetrating player character mesh
    dispX = std::clamp(dispX, -1.5, 1.5);
    dispY = std::clamp(dispY, -1.5, 1.5);
    dispZ = std::clamp(dispZ, -1.5, 1.5);

    return {dispX, dispY, dispZ};
}

ProcessedTrackingState TrackingEngine::Process(const RawTrackingPose& rawPose, float baseGameFOV, float deltaTime)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    ProcessedTrackingState state{};
    state.IsTrackingActive = m_config.Enabled;
    state.IsParallaxActive = m_config.Parallax3DEnabled;
    state.IsFreelookActive = m_config.DecoupledFreelookActive;

    if (!rawPose.IsValid || !m_config.Enabled)
    {
        m_streamWasLost = true;
        return state;
    }

    // 1. Recenter baseline capture (instant zero-calibration or stream recovery)
    // Detect whether m_centerOffset was captured from an uninitialized zero-frame
    bool centerIsZero = (std::abs(m_centerOffset.X) < 0.0001 &&
                         std::abs(m_centerOffset.Y) < 0.0001 &&
                         std::abs(m_centerOffset.Z) < 0.0001 &&
                         std::abs(m_centerOffset.Yaw) < 0.0001 &&
                         std::abs(m_centerOffset.Pitch) < 0.0001 &&
                         std::abs(m_centerOffset.Roll) < 0.0001);

    bool hasMeaningfulPose = (std::abs(rawPose.X) > 0.001 ||
                              std::abs(rawPose.Y) > 0.001 ||
                              std::abs(rawPose.Z) > 0.001 ||
                              std::abs(rawPose.Yaw) > 0.001 ||
                              std::abs(rawPose.Pitch) > 0.001 ||
                              std::abs(rawPose.Roll) > 0.001);

    if (!m_hasCenter || (centerIsZero && hasMeaningfulPose))
    {
        m_centerOffset = rawPose;
        m_hasCenter = true;
        m_filteredPose = RawTrackingPose{}; // Cleanly reset smoothed pose to zero
        m_filteredPose.IsValid = true;
        m_streamWasLost = false;
    }

    // 2. Subtract center offset and normalize angular deltas
    double rawYaw   = FRotator3d::NormalizeAxis(rawPose.Yaw   - m_centerOffset.Yaw);
    double rawPitch = FRotator3d::NormalizeAxis(rawPose.Pitch - m_centerOffset.Pitch);
    double rawRoll  = FRotator3d::NormalizeAxis(rawPose.Roll  - m_centerOffset.Roll);
    double rawX     = rawPose.X     - m_centerOffset.X;
    double rawY     = rawPose.Y     - m_centerOffset.Y;
    double rawZ     = rawPose.Z     - m_centerOffset.Z;

    // 3. Apply Deadzones
    auto applyDeadzone = [](double val, double dz) -> double {
        if (std::abs(val) <= dz) return 0.0;
        return (val > 0.0) ? (val - dz) : (val + dz);
    };

    rawYaw   = applyDeadzone(rawYaw,   m_config.DeadzoneDegrees);
    rawPitch = applyDeadzone(rawPitch, m_config.DeadzoneDegrees);
    rawRoll  = applyDeadzone(rawRoll,  m_config.DeadzoneDegrees);
    rawX     = applyDeadzone(rawX,     m_config.DeadzoneTransCm);
    rawY     = applyDeadzone(rawY,     m_config.DeadzoneTransCm);
    rawZ     = applyDeadzone(rawZ,     m_config.DeadzoneTransCm);

    // 4. Inversions & Sensitivities
    double fovDamp = 1.0;
    if (m_config.ScaleSensWithFOV && baseGameFOV < 85.0f && baseGameFOV > 3.0f)
    {
        fovDamp = std::clamp((double)baseGameFOV / 85.0, 0.20, 1.0);
    }

    double targetYaw   = rawYaw   * m_config.SensYaw   * fovDamp * (m_config.InvertYaw   ? -1.0 : 1.0);
    double targetPitch = rawPitch * m_config.SensPitch * fovDamp * (m_config.InvertPitch ? -1.0 : 1.0);
    double targetRoll  = rawRoll  * m_config.SensRoll  * fovDamp * (m_config.InvertRoll  ? -1.0 : 1.0);
    double targetX     = rawX     * m_config.SensX     * (m_config.InvertX     ? -1.0 : 1.0);
    double targetY     = rawY     * m_config.SensY     * (m_config.InvertY     ? -1.0 : 1.0);
    double targetZ     = rawZ     * m_config.SensZ     * (m_config.InvertZ     ? -1.0 : 1.0);

    // Clamp translation targets within safety bounds to prevent wall clipping
    targetX = std::clamp(targetX, -m_config.MaxTranslationX_Cm, m_config.MaxTranslationX_Cm);
    targetY = std::clamp(targetY, -m_config.MaxTranslationY_Cm, m_config.MaxTranslationY_Cm);
    targetZ = std::clamp(targetZ, -m_config.MaxTranslationZ_Cm, m_config.MaxTranslationZ_Cm);

    // 5. Dynamic Smoothing (Framerate-independent exponential moving average with angular normalization)
    double dtClamped = std::clamp((double)deltaTime, 0.001, 0.1);
    double decayBase = std::clamp(m_config.SmoothingFactor, 0.0, 0.95);
    double alpha = 1.0 - std::pow(decayBase, dtClamped * 60.0);

    double deltaYaw   = FRotator3d::NormalizeAxis(targetYaw   - m_filteredPose.Yaw);
    double deltaPitch = FRotator3d::NormalizeAxis(targetPitch - m_filteredPose.Pitch);
    double deltaRoll  = FRotator3d::NormalizeAxis(targetRoll  - m_filteredPose.Roll);

    m_filteredPose.Yaw   = FRotator3d::NormalizeAxis(m_filteredPose.Yaw   + alpha * deltaYaw);
    m_filteredPose.Pitch = FRotator3d::NormalizeAxis(m_filteredPose.Pitch + alpha * deltaPitch);
    m_filteredPose.Roll  = FRotator3d::NormalizeAxis(m_filteredPose.Roll  + alpha * deltaRoll);
    m_filteredPose.X     += alpha * (targetX - m_filteredPose.X);
    m_filteredPose.Y     += alpha * (targetY - m_filteredPose.Y);
    m_filteredPose.Z     += alpha * (targetZ - m_filteredPose.Z);

    m_filteredPose.X = std::clamp(m_filteredPose.X, -m_config.MaxTranslationX_Cm, m_config.MaxTranslationX_Cm);
    m_filteredPose.Y = std::clamp(m_filteredPose.Y, -m_config.MaxTranslationY_Cm, m_config.MaxTranslationY_Cm);
    m_filteredPose.Z = std::clamp(m_filteredPose.Z, -m_config.MaxTranslationZ_Cm, m_config.MaxTranslationZ_Cm);

    state.Rotation.Yaw   = m_filteredPose.Yaw;
    state.Rotation.Pitch = m_filteredPose.Pitch;
    state.Rotation.Roll  = std::clamp(m_filteredPose.Roll, -45.0, 45.0); // Bounded to natural cervical lateral flexion

    // True OpenTrack / FreeTrack to Unreal Engine 5.4 coordinate mapping:
    // OpenTrack X = Lateral (Left/Right)    -> Unreal Y (Right)
    // OpenTrack Y = Vertical (Up/Down)      -> Unreal Z (Up)
    // OpenTrack Z = Depth (Forward/Back)    -> Unreal X (Forward)
    if (m_config.EnableTranslation)
    {
        state.LocalTranslation.X = m_filteredPose.Z; // Local Forward in UE
        state.LocalTranslation.Y = m_filteredPose.X; // Local Right in UE
        state.LocalTranslation.Z = m_filteredPose.Y; // Local Up in UE
    }
    else
    {
        state.LocalTranslation = FVector3d{0.0, 0.0, 0.0};
    }

    // 6. Neck Pivot Displacement
    if (m_config.UseNeckModel)
    {
        state.NeckDisplacement = ComputeNeckDisplacement(state.Rotation.Pitch, state.Rotation.Yaw, state.Rotation.Roll);
    }

    // 7. Parallax 3D Calculation (Frustum shear + dynamic depth FOV)
    if (m_config.Parallax3DEnabled)
    {
        state.Parallax = m_parallaxEngine.CalculateParallax(m_filteredPose.X, m_filteredPose.Y, m_filteredPose.Z, baseGameFOV);
    }

    return state;
}

void TrackingEngine::ApplyToViewInfo(FMinimalViewInfo* pov, const ProcessedTrackingState& state, double actorBaseYawDeg, bool applyRotation)
{
    if (!pov) return;

    if (!state.IsTrackingActive)
    {
        pov->OffCenterProjectionOffset.X = 0.0;
        pov->OffCenterProjectionOffset.Y = 0.0;
        return;
    }

    // 1. Add Rotations to camera rotation
    if (applyRotation)
    {
        // Clamp pitch to [-89, 89] to prevent gimbal lock at UE's vertical singularity poles
        pov->Rotation.Pitch = std::clamp(FRotator3d::NormalizeAxis(pov->Rotation.Pitch + state.Rotation.Pitch), -89.0, 89.0);
        pov->Rotation.Yaw   = FRotator3d::NormalizeAxis(pov->Rotation.Yaw   + state.Rotation.Yaw);
        pov->Rotation.Roll  = FRotator3d::NormalizeAxis(pov->Rotation.Roll  + state.Rotation.Roll);
    }
    else
    {
        // When aim is coupled, ControlRotation directly steers Pitch & Yaw; apply Roll to camera
        pov->Rotation.Roll = FRotator3d::NormalizeAxis(pov->Rotation.Roll + state.Rotation.Roll);
    }

    // 2. Transform translation (neck + head translation + parallax shift) into World Space using Actor Yaw
    double totalLocalForward = state.LocalTranslation.X + state.NeckDisplacement.X;
    double totalLocalRight   = state.LocalTranslation.Y + state.NeckDisplacement.Y;
    double totalLocalUp      = state.LocalTranslation.Z + state.NeckDisplacement.Z;

    if (state.IsParallaxActive)
    {
        totalLocalForward += state.Parallax.CameraShiftLocal.X;
        totalLocalRight   += state.Parallax.CameraShiftLocal.Y;
        totalLocalUp      += state.Parallax.CameraShiftLocal.Z;
    }

    double radYaw = DEG_TO_RAD(actorBaseYawDeg);
    double cosY = std::cos(radYaw);
    double sinY = std::sin(radYaw);

    double worldDX = totalLocalForward * cosY - totalLocalRight * sinY;
    double worldDY = totalLocalForward * sinY + totalLocalRight * cosY;
    double worldDZ = totalLocalUp;

    pov->Location.X += worldDX;
    pov->Location.Y += worldDY;
    pov->Location.Z += worldDZ;

    // 3. Apply Parallax 3D (Frustum shear + Dynamic FOV)
    if (state.IsParallaxActive)
    {
        pov->OffCenterProjectionOffset.X = state.Parallax.ProjectionOffset.X;
        pov->OffCenterProjectionOffset.Y = state.Parallax.ProjectionOffset.Y;

        if (state.Parallax.AdjustedFOV > 3.0f)
        {
            pov->FOV = state.Parallax.AdjustedFOV;
        }
    }
    else
    {
        pov->OffCenterProjectionOffset.X = 0.0;
        pov->OffCenterProjectionOffset.Y = 0.0;
    }
}
