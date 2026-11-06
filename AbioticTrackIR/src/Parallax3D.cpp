#include "Parallax3D.hpp"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void Parallax3DEngine::SetConfig(const MonitorConfig& config)
{
    m_config = config;
    if (m_config.FocalDistanceCm < 30.0) m_config.FocalDistanceCm = 30.0;
    if (m_config.FocalDistanceCm > 500.0) m_config.FocalDistanceCm = 500.0;
}

const MonitorConfig& Parallax3DEngine::GetConfig() const
{
    return m_config;
}

void Parallax3DEngine::SetFocalDistance(double focalDistCm)
{
    m_config.FocalDistanceCm = std::clamp(focalDistCm, 30.0, 500.0);
}

ParallaxResult Parallax3DEngine::CalculateParallax(double headLateral_cm, double headVertical_cm, double headDepth_cm,
                                                 float baseGameFOV)
{
    ParallaxResult result{};
    result.AdjustedFOV = baseGameFOV;
    result.CurrentFocalDepth = m_config.FocalDistanceCm;

    double focalDist = std::clamp(m_config.FocalDistanceCm, 30.0, 500.0);
    double fovDeg = std::clamp((double)baseGameFOV, 20.0, 140.0);
    double halfFovRad = (fovDeg * 0.5) * (M_PI / 180.0);
    double tanHalfFov = std::tan(halfFovRad);
    if (tanHalfFov < 0.05) tanHalfFov = 0.05;

    // Aspect ratio derived from monitor dimensions (fallback 16:9)
    double aspect = (m_config.WidthCm > 5.0 && m_config.HeightCm > 5.0) 
                  ? (m_config.WidthCm / m_config.HeightCm) 
                  : (16.0 / 9.0);
    if (aspect < 0.5) aspect = 16.0 / 9.0;

    // View frustum half-extents at the focal plane
    // At this distance D_focal, physical camera translation and projection shear cancel out exactly:
    // Objects at D_focal remain locked to the monitor surface, while objects behind gain deep inward
    // parallax and objects in front pop outward toward the player.
    double halfW_focal = focalDist * tanHalfFov;
    double halfH_focal = halfW_focal / aspect;

    // Johnny Chung Lee off-axis projection shear:
    // Moving head right (headLateral_cm > 0) -> camera moves right -> projection offset positive
    // cancels translation at focal plane Z = focalDist
    double targetShearX = (headLateral_cm / halfW_focal) * m_config.ParallaxIntensity;
    double targetShearY = (headVertical_cm / halfH_focal) * m_config.ParallaxIntensity;

    if (m_config.InvertParallaxX) targetShearX = -targetShearX;
    if (m_config.InvertParallaxY) targetShearY = -targetShearY;

    // Clamping to safety bounds to avoid nausea
    targetShearX = std::clamp(targetShearX, -m_config.MaxOffsetFrustum, m_config.MaxOffsetFrustum);
    targetShearY = std::clamp(targetShearY, -m_config.MaxOffsetFrustum, m_config.MaxOffsetFrustum);

    // Exponential smoothing (alpha = 0.35) to eliminate high-frequency webcam face tracking jitter
    constexpr double alpha = 0.35;
    m_smoothedShearX += alpha * (targetShearX - m_smoothedShearX);
    m_smoothedShearY += alpha * (targetShearY - m_smoothedShearY);

    result.ProjectionOffset.X = m_smoothedShearX;
    result.ProjectionOffset.Y = m_smoothedShearY;

    // Dynamic depth FOV: leaning in increases natural visual angle (disabled by default)
    if (m_config.DynamicFOV && baseGameFOV > 3.0f)
    {
        double currentDist = m_config.ViewingDistanceCm - headDepth_cm;
        if (currentDist < 15.0) currentDist = 15.0;

        double halfW_screen = std::max(10.0, m_config.WidthCm * 0.5);
        double nominalPhysicalFOV = 2.0 * std::atan(halfW_screen / m_config.ViewingDistanceCm) * (180.0 / M_PI);
        double currentPhysicalFOV = 2.0 * std::atan(halfW_screen / currentDist) * (180.0 / M_PI);
        double fovRatio = currentPhysicalFOV / nominalPhysicalFOV;

        if (baseGameFOV < 65.0f)
        {
            double damp = std::clamp(((double)baseGameFOV - 5.0) / 60.0, 0.0, 1.0);
            fovRatio = 1.0 + (fovRatio - 1.0) * damp * 0.4;
        }

        double targetFOV = (double)baseGameFOV * fovRatio;
        double minAllowedFOV = std::max(3.0, (double)baseGameFOV * 0.70);
        double maxAllowedFOV = std::min(150.0, (double)baseGameFOV * 1.30);
        result.AdjustedFOV = (float)std::clamp(targetFOV, minAllowedFOV, maxAllowedFOV);
    }

    // Physical Camera Shift in local coordinates (cm)
    result.CameraShiftLocal.X = headDepth_cm;
    result.CameraShiftLocal.Y = headLateral_cm;
    result.CameraShiftLocal.Z = headVertical_cm;

    return result;
}
