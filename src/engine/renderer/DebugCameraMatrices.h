#pragma once

#include "engine/renderer/DebugProjection.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace engine {

inline constexpr float DefaultDebugFarPlane = 1000.0f;

struct DebugWorldToClipMatrix {
    std::array<float, 16> values {};

    float& at(std::size_t row, std::size_t column)
    {
        return values[row * 4 + column];
    }

    float at(std::size_t row, std::size_t column) const
    {
        return values[row * 4 + column];
    }
};

struct DebugClipPoint {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};

inline bool BuildDebugWorldToClipMatrix(
    const DebugCamera& camera,
    float aspectRatio,
    DebugWorldToClipMatrix& matrix,
    float farPlane = DefaultDebugFarPlane)
{
    matrix = {};

    detail::DebugProjectionBasis basis;
    if (!detail::BuildDebugProjectionBasis(camera, basis)) {
        return false;
    }

    const float tanHalfFov = std::tan(camera.fovYRadians * 0.5f);
    if (tanHalfFov <= detail::ProjectionEpsilon) {
        return false;
    }

    const float safeAspectRatio = std::max(aspectRatio, detail::ProjectionEpsilon);
    const float nearPlane = detail::EffectiveNearPlane(camera);
    const float safeFarPlane = std::max(farPlane, nearPlane + detail::ProjectionEpsilon);
    const float xScale = 1.0f / (tanHalfFov * safeAspectRatio);
    const float yScale = 1.0f / tanHalfFov;
    const float depthScale = safeFarPlane / (safeFarPlane - nearPlane);
    const float depthBias = (-nearPlane * safeFarPlane) / (safeFarPlane - nearPlane);
    const float rightBias = -Dot(camera.position, basis.right);
    const float upBias = -Dot(camera.position, basis.up);
    const float forwardBias = -Dot(camera.position, basis.forward);

    matrix.at(0, 0) = basis.right.x * xScale;
    matrix.at(1, 0) = basis.right.y * xScale;
    matrix.at(2, 0) = basis.right.z * xScale;
    matrix.at(3, 0) = rightBias * xScale;

    matrix.at(0, 1) = basis.up.x * yScale;
    matrix.at(1, 1) = basis.up.y * yScale;
    matrix.at(2, 1) = basis.up.z * yScale;
    matrix.at(3, 1) = upBias * yScale;

    matrix.at(0, 2) = basis.forward.x * depthScale;
    matrix.at(1, 2) = basis.forward.y * depthScale;
    matrix.at(2, 2) = basis.forward.z * depthScale;
    matrix.at(3, 2) = forwardBias * depthScale + depthBias;

    matrix.at(0, 3) = basis.forward.x;
    matrix.at(1, 3) = basis.forward.y;
    matrix.at(2, 3) = basis.forward.z;
    matrix.at(3, 3) = forwardBias;

    return true;
}

inline DebugClipPoint TransformWorldPoint(const DebugWorldToClipMatrix& matrix, Vec3 point)
{
    return {
        point.x * matrix.at(0, 0) + point.y * matrix.at(1, 0) + point.z * matrix.at(2, 0) + matrix.at(3, 0),
        point.x * matrix.at(0, 1) + point.y * matrix.at(1, 1) + point.z * matrix.at(2, 1) + matrix.at(3, 1),
        point.x * matrix.at(0, 2) + point.y * matrix.at(1, 2) + point.z * matrix.at(2, 2) + matrix.at(3, 2),
        point.x * matrix.at(0, 3) + point.y * matrix.at(1, 3) + point.z * matrix.at(2, 3) + matrix.at(3, 3),
    };
}

} // namespace engine
