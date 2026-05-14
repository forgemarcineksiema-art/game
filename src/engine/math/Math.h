#pragma once

#include <algorithm>
#include <cmath>

namespace engine {

constexpr float Pi = 3.14159265358979323846f;
constexpr float TwoPi = Pi * 2.0f;

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

inline Vec2 operator+(Vec2 a, Vec2 b)
{
    return {a.x + b.x, a.y + b.y};
}

inline Vec2 operator-(Vec2 a, Vec2 b)
{
    return {a.x - b.x, a.y - b.y};
}

inline Vec2 operator*(Vec2 value, float scalar)
{
    return {value.x * scalar, value.y * scalar};
}

inline Vec3 operator+(Vec3 a, Vec3 b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline Vec3 operator-(Vec3 a, Vec3 b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

inline Vec3 operator*(Vec3 value, float scalar)
{
    return {value.x * scalar, value.y * scalar, value.z * scalar};
}

inline Vec3 operator/(Vec3 value, float scalar)
{
    return {value.x / scalar, value.y / scalar, value.z / scalar};
}

inline Vec3& operator+=(Vec3& a, Vec3 b)
{
    a = a + b;
    return a;
}

inline float Clamp(float value, float minValue, float maxValue)
{
    return std::clamp(value, minValue, maxValue);
}

inline float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

inline Vec3 Lerp(Vec3 a, Vec3 b, float t)
{
    return {
        Lerp(a.x, b.x, t),
        Lerp(a.y, b.y, t),
        Lerp(a.z, b.z, t),
    };
}

inline float Dot(Vec2 a, Vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

inline float Dot(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 Cross(Vec3 a, Vec3 b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

inline float Length(Vec2 value)
{
    return std::sqrt(Dot(value, value));
}

inline float Length(Vec3 value)
{
    return std::sqrt(Dot(value, value));
}

inline Vec2 Normalize(Vec2 value)
{
    const float length = Length(value);
    if (length <= 0.00001f) {
        return {};
    }

    return value * (1.0f / length);
}

inline Vec3 Normalize(Vec3 value)
{
    const float length = Length(value);
    if (length <= 0.00001f) {
        return {};
    }

    return value / length;
}

inline float Radians(float degrees)
{
    return degrees * Pi / 180.0f;
}

inline float Degrees(float radians)
{
    return radians * 180.0f / Pi;
}

inline Vec3 ForwardFromYaw(float yawRadians)
{
    return {std::sin(yawRadians), 0.0f, std::cos(yawRadians)};
}

inline Vec3 RightFromYaw(float yawRadians)
{
    return {std::cos(yawRadians), 0.0f, -std::sin(yawRadians)};
}

inline float YawFromDirection(Vec3 direction)
{
    return std::atan2(direction.x, direction.z);
}

inline float ExponentialSmoothingFactor(float smoothing, float deltaSeconds)
{
    if (smoothing <= 0.0f) {
        return 1.0f;
    }

    return 1.0f - std::exp(-smoothing * deltaSeconds);
}

} // namespace engine
