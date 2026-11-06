#pragma once
#include "UnrealTypes.hpp"
#include <algorithm>

struct MonitorConfig
{
    double WidthCm{59.7};          // 27" 16:9 screen = ~59.7 cm wide
    double HeightCm{33.6};         // 27" 16:9 screen = ~33.6 cm high
    double ViewingDistanceCm{65.0}; // Typical desk distance = 65 cm
    double ParallaxIntensity{1.0};  // Frustum shear multiplier
    bool DynamicFOV{true};          // Expand FOV when leaning closer
    double MaxOffsetFrustum{0.85};  // Clamping to prevent edge distortion
    bool InvertParallaxX{false};    // Invert horizontal frustum shear
    bool InvertParallaxY{false};    // Invert vertical frustum shear
};

struct ParallaxResult
{
    FVector2d ProjectionOffset;   // Frustum shear (X, Y)
    float AdjustedFOV;            // Dynamic FOV in degrees
    FVector3d CameraShiftLocal;   // Camera physical shift in local space (cm)
    bool IsActive{true};
};

class Parallax3DEngine
{
public:
    Parallax3DEngine() = default;

    void SetConfig(const MonitorConfig& config);
    const MonitorConfig& GetConfig() const;

    // Calculates off-axis frustum projection and dynamic depth perspective
    // headLateral_cm: Left (-)/Right (+)
    // headVertical_cm: Down (-)/Up (+)
    // headDepth_cm: Away (-)/Towards screen (+)
    ParallaxResult CalculateParallax(double headLateral_cm, double headVertical_cm, double headDepth_cm,
                                     float baseGameFOV);

private:
    MonitorConfig m_config;
};
