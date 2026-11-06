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
    FVector3d Location;                   // 0x000 - 0x018
    FRotator3d Rotation;                  // 0x018 - 0x030
    float FOV;                            // 0x030 - 0x034
    float DesiredFOV;                     // 0x034 - 0x038
    float OrthoWidth;                     // 0x038 - 0x03C
    uint8_t Pad_3C[0x760 - 0x3C];         // 0x03C - 0x760 (Clip planes, AspectRatio, PostProcessSettings)
    FVector2d OffCenterProjectionOffset;  // 0x760 - 0x770 (X, Y in normalized frustum units)
    uint8_t Pad_770[0x800 - 0x770];       // 0x770 - 0x800 (PreviousViewTransform, etc.)
};
#pragma pack(pop)

static_assert(sizeof(FVector3d) == 24, "FVector3d must be 24 bytes");
static_assert(sizeof(FRotator3d) == 24, "FRotator3d must be 24 bytes");
static_assert(sizeof(FVector2d) == 16, "FVector2d must be 16 bytes");
static_assert(offsetof(FMinimalViewInfo, Location) == 0x00, "Location offset mismatch");
static_assert(offsetof(FMinimalViewInfo, Rotation) == 0x18, "Rotation offset mismatch");
static_assert(offsetof(FMinimalViewInfo, FOV) == 0x30, "FOV offset mismatch");
static_assert(offsetof(FMinimalViewInfo, OffCenterProjectionOffset) == 0x760, "OffCenterProjectionOffset offset mismatch");
static_assert(sizeof(FMinimalViewInfo) == 0x800, "FMinimalViewInfo total size mismatch");
