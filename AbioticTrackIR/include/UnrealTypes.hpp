#pragma once
#include <cstdint>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Unreal Engine 5 Large World Coordinates (LWC) vector / rotator structures (double-precision)
struct FVector3d
{
    double X{0.0};
    double Y{0.0};
    double Z{0.0};

    FVector3d() = default;
    FVector3d(double inX, double inY, double inZ) : X(inX), Y(inY), Z(inZ) {}

    FVector3d operator+(const FVector3d& other) const { return {X + other.X, Y + other.Y, Z + other.Z}; }
    FVector3d operator-(const FVector3d& other) const { return {X - other.X, Y - other.Y, Z - other.Z}; }
    FVector3d operator*(double scalar) const { return {X * scalar, Y * scalar, Z * scalar}; }
    FVector3d& operator+=(const FVector3d& other) { X += other.X; Y += other.Y; Z += other.Z; return *this; }
};

struct FRotator3d
{
    double Pitch{0.0}; // Rotation around Right axis (Y in UE)
    double Yaw{0.0};   // Rotation around Up axis (Z in UE)
    double Roll{0.0};  // Rotation around Forward axis (X in UE)

    FRotator3d() = default;
    FRotator3d(double inP, double inY, double inR) : Pitch(inP), Yaw(inY), Roll(inR) {}

    static double NormalizeAxis(double angle)
    {
        while (angle > 180.0)  angle -= 360.0;
        while (angle < -180.0) angle += 360.0;
        return angle;
    }

    FRotator3d GetNormalized() const
    {
        return {NormalizeAxis(Pitch), NormalizeAxis(Yaw), NormalizeAxis(Roll)};
    }
};

struct FVector2d
{
    double X{0.0};
    double Y{0.0};

    FVector2d() = default;
    FVector2d(double inX, double inY) : X(inX), Y(inY) {}
};

// Unreal Engine 5.4 FMinimalViewInfo
// Verified against AbioticFactor-Win64-Shipping.pdb symbol layouts
#pragma pack(push, 1)
struct FMinimalViewInfo
{
    FVector3d Location;                   // 0x00 - 0x18
    FRotator3d Rotation;                  // 0x18 - 0x30
    float FOV;                            // 0x30 - 0x34
    float DesiredFOV;                     // 0x34 - 0x38
    float OrthoWidth;                     // 0x38 - 0x3C
    float OrthoNearClipPlane;             // 0x3C - 0x40
    float OrthoFarClipPlane;              // 0x40 - 0x44
    float PerspectiveNearClipPlane;       // 0x44 - 0x48
    float AspectRatio;                    // 0x48 - 0x4C
    uint8_t Flags;                        // 0x4C (bit 0: bConstrainAspectRatio, bit 1: bUseFieldOfViewForLOD)
    uint8_t ProjectionMode;               // 0x4D (0: Perspective, 1: Orthographic)
    uint8_t Pad[2];                       // 0x4E - 0x50 alignment
    FVector2d OffCenterProjectionOffset;  // 0x50 - 0x60 (X, Y in normalized frustum units)
    float PostProcessBlendWeight;         // 0x60 - 0x64
};
#pragma pack(pop)

static_assert(sizeof(FVector3d) == 24, "FVector3d must be 24 bytes");
static_assert(sizeof(FRotator3d) == 24, "FRotator3d must be 24 bytes");
static_assert(sizeof(FVector2d) == 16, "FVector2d must be 16 bytes");
static_assert(offsetof(FMinimalViewInfo, Location) == 0x00, "Location offset mismatch");
static_assert(offsetof(FMinimalViewInfo, Rotation) == 0x18, "Rotation offset mismatch");
static_assert(offsetof(FMinimalViewInfo, FOV) == 0x30, "FOV offset mismatch");
static_assert(offsetof(FMinimalViewInfo, OffCenterProjectionOffset) == 0x50, "OffCenterProjectionOffset offset mismatch");
