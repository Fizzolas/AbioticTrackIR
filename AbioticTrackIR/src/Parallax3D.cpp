#include "Parallax3D.hpp"

void Parallax3DEngine::SetConfig(const MonitorConfig& config)
{
    m_config = config;
}

const MonitorConfig& Parallax3DEngine::GetConfig() const
{
    return m_config;
}

ParallaxResult Parallax3DEngine::CalculateParallax(double headLateral_cm, double headVertical_cm, double headDepth_cm,
                                                 float baseGameFOV)
{
    ParallaxResult result{};
    result.AdjustedFOV = baseGameFOV;

    double halfW = std::max(10.0, m_config.WidthCm * 0.5);
    double halfH = std::max(5.0,  m_config.HeightCm * 0.5);

    // Current distance from head to display
    // headDepth_cm is positive when leaning towards screen
    double currentDist = m_config.ViewingDistanceCm - headDepth_cm;
    if (currentDist < 15.0) currentDist = 15.0; // Prevent clipping through display plane

    // 1. Off-axis projection frustum shear
    // When head moves right (headLateral > 0), frustum center shifts left (offset.X < 0)
    // When head moves up    (headVertical > 0), frustum center shifts down (offset.Y < 0)
    double shearX = -(headLateral_cm / halfW) * m_config.ParallaxIntensity * (m_config.InvertParallaxX ? -1.0 : 1.0);
    double shearY = -(headVertical_cm / halfH) * m_config.ParallaxIntensity * (m_config.InvertParallaxY ? -1.0 : 1.0);

    // Apply safety clamping
    shearX = std::clamp(shearX, -m_config.MaxOffsetFrustum, m_config.MaxOffsetFrustum);
    shearY = std::clamp(shearY, -m_config.MaxOffsetFrustum, m_config.MaxOffsetFrustum);

    result.ProjectionOffset.X = shearX;
    result.ProjectionOffset.Y = shearY;

    // 2. Dynamic depth FOV: leaning in increases natural visual angle
    if (m_config.DynamicFOV && baseGameFOV > 3.0f)
    {
        double nominalPhysicalFOV = 2.0 * std::atan(halfW / m_config.ViewingDistanceCm) * (180.0 / M_PI);
        double currentPhysicalFOV = 2.0 * std::atan(halfW / currentDist) * (180.0 / M_PI);
        double fovRatio = currentPhysicalFOV / nominalPhysicalFOV;

        // If the player is in ADS / Scoped zoom (baseGameFOV < 65 deg), dampen FOV scaling
        // to prevent jarring optical shifts when aiming through precision optics
        if (baseGameFOV < 65.0f)
        {
            double damp = std::clamp(((double)baseGameFOV - 5.0) / 60.0, 0.0, 1.0);
            fovRatio = 1.0 + (fovRatio - 1.0) * damp * 0.4;
        }

        double targetFOV = (double)baseGameFOV * fovRatio;

        // Dynamic relative clamping based on baseGameFOV (allows sniper zoom down to 5 deg)
        double minAllowedFOV = std::max(3.0, (double)baseGameFOV * 0.70);
        double maxAllowedFOV = std::min(150.0, (double)baseGameFOV * 1.30);
        targetFOV = std::clamp(targetFOV, minAllowedFOV, maxAllowedFOV);
        result.AdjustedFOV = (float)targetFOV;
    }

    // 3. Physical Camera Shift in local coordinates (cm)
    // In UE coordinate system:
    // Local X = Forward (towards screen / depth)
    // Local Y = Right   (lateral)
    // Local Z = Up      (vertical)
    result.CameraShiftLocal.X = headDepth_cm;
    result.CameraShiftLocal.Y = headLateral_cm;
    result.CameraShiftLocal.Z = headVertical_cm;

    return result;
}
