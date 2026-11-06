#pragma once
#include "UnrealTypes.hpp"
#include <algorithm>

struct MonitorConfig
{
    double WidthCm{59.7};           // 27" 16:9 screen = ~59.7 cm wide
    double HeightCm{33.6};          // 27" 16:9 screen = ~33.6 cm high
    double ViewingDistanceCm{65.0};  // Distance from eyes to monitor surface (cm)
    double FocalDistanceCm{120.0};   // Convergence focal depth (cm) - objects at this depth remain anchored to screen
    double ParallaxIntensity{1.0};   // Parallax depth intensity multiplier
    bool DynamicFOV{false};          // Keep false for stable, comfortable perspective
    double MaxOffsetFrustum{0.30};   // Safety clamp on off-axis frustum shear
    bool InvertParallaxX{false};     // Invert horizontal frustum shear
    bool InvertParallaxY{false};     // Invert vertical frustum shear
};

struct ParallaxResult
{
    FVector2d ProjectionOffset;   // Frustum shear (X, Y)
    float AdjustedFOV;            // Dynamic FOV in degrees
    FVector3d CameraShiftLocal;   // Camera physical shift in local space (cm)
    double CurrentFocalDepth{120.0};
    bool IsActive{true};
};

class Parallax3DEngine
{
public:
    Parallax3DEngine() = default;

    void SetConfig(const MonitorConfig& config);
    const MonitorConfig& GetConfig() const;

    void SetFocalDistance(double focalDistCm);
    double GetFocalDistance() const { return m_config.FocalDistanceCm; }

    // Calculates true Johnny Lee-style off-axis perspective projection anchored at FocalDistanceCm
    ParallaxResult CalculateParallax(double headLateral_cm, double headVertical_cm, double headDepth_cm,
                                     float baseGameFOV);

private:
    MonitorConfig m_config;
    double m_smoothedShearX{0.0};
    double m_smoothedShearY{0.0};
};
