#pragma once

#include "engine/renderer/RendererTypes.h"

namespace engine {

struct ProjectedPoint {
    float x = 0.0f;
    float y = 0.0f;
    bool visible = false;
};

inline bool ProjectWorldPoint(const DebugCamera& camera, float aspectRatio, Vec3 point, ProjectedPoint& projected)
{
    const Vec3 forward = Normalize(camera.target - camera.position);
    if (Length(forward) <= 0.00001f) {
        return false;
    }

    Vec3 right = Normalize(Cross({0.0f, 1.0f, 0.0f}, forward));
    if (Length(right) <= 0.00001f) {
        right = {1.0f, 0.0f, 0.0f};
    }
    const Vec3 up = Normalize(Cross(forward, right));

    const Vec3 delta = point - camera.position;
    const float z = Dot(delta, forward);
    if (z <= camera.nearPlane) {
        return false;
    }

    const float tanHalfFov = std::tan(camera.fovYRadians * 0.5f);
    if (tanHalfFov <= 0.00001f) {
        return false;
    }

    projected.x = Dot(delta, right) / (z * tanHalfFov * aspectRatio);
    projected.y = Dot(delta, up) / (z * tanHalfFov);
    projected.visible = projected.x >= -1.5f && projected.x <= 1.5f && projected.y >= -1.5f && projected.y <= 1.5f;
    return projected.visible;
}

} // namespace engine
