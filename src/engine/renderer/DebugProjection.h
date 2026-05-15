#pragma once

#include "engine/renderer/RendererTypes.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cmath>
#include <vector>

namespace engine {

struct ProjectedPoint {
    float x = 0.0f;
    float y = 0.0f;
    bool visible = false;
};

struct ProjectedPolygon {
    std::array<ProjectedPoint, 4> points {};
    std::size_t pointCount = 0;
};

struct ProjectedTriangle {
    ProjectedPolygon polygon {};
    float cameraDepth = 0.0f;
};

namespace detail {

inline constexpr float ProjectionEpsilon = 0.0001f;
inline constexpr float ProjectionCoordinateLimit = 32.0f;

struct DebugProjectionBasis {
    Vec3 forward {};
    Vec3 right {};
    Vec3 up {};
};

inline float EffectiveNearPlane(const DebugCamera& camera)
{
    return std::max(camera.nearPlane, ProjectionEpsilon);
}

inline bool BuildDebugProjectionBasis(const DebugCamera& camera, DebugProjectionBasis& basis)
{
    basis.forward = Normalize(camera.target - camera.position);
    if (Length(basis.forward) <= 0.00001f) {
        return false;
    }

    basis.right = Normalize(Cross({0.0f, 1.0f, 0.0f}, basis.forward));
    if (Length(basis.right) <= 0.00001f) {
        basis.right = {1.0f, 0.0f, 0.0f};
    }
    basis.up = Normalize(Cross(basis.forward, basis.right));
    return true;
}

inline Vec3 ToCameraSpace(const DebugCamera& camera, const DebugProjectionBasis& basis, Vec3 point)
{
    const Vec3 delta = point - camera.position;
    return {
        Dot(delta, basis.right),
        Dot(delta, basis.up),
        Dot(delta, basis.forward),
    };
}

inline bool ProjectCameraPoint(const DebugCamera& camera, float aspectRatio, Vec3 point, ProjectedPoint& projected)
{
    if (point.z <= EffectiveNearPlane(camera)) {
        return false;
    }

    const float tanHalfFov = std::tan(camera.fovYRadians * 0.5f);
    if (tanHalfFov <= 0.00001f) {
        return false;
    }

    const float safeAspectRatio = std::max(aspectRatio, ProjectionEpsilon);
    const float rawX = point.x / (point.z * tanHalfFov * safeAspectRatio);
    const float rawY = point.y / (point.z * tanHalfFov);
    if (!std::isfinite(rawX) || !std::isfinite(rawY)) {
        return false;
    }

    projected.x = Clamp(rawX, -ProjectionCoordinateLimit, ProjectionCoordinateLimit);
    projected.y = Clamp(rawY, -ProjectionCoordinateLimit, ProjectionCoordinateLimit);
    projected.visible = rawX >= -1.5f && rawX <= 1.5f && rawY >= -1.5f && rawY <= 1.5f;
    return true;
}

inline Vec3 IntersectNearPlane(Vec3 from, Vec3 to, float nearPlane)
{
    const float denominator = to.z - from.z;
    if (std::abs(denominator) <= ProjectionEpsilon) {
        return from;
    }

    const float t = Clamp((nearPlane - from.z) / denominator, 0.0f, 1.0f);
    return Lerp(from, to, t);
}

inline bool ClipLineToNearPlane(const DebugCamera& camera, Vec3& from, Vec3& to)
{
    const float nearPlane = EffectiveNearPlane(camera);
    const bool fromInside = from.z > nearPlane;
    const bool toInside = to.z > nearPlane;
    if (!fromInside && !toInside) {
        return false;
    }

    const float clipPlane = nearPlane + ProjectionEpsilon;
    if (!fromInside) {
        from = IntersectNearPlane(from, to, clipPlane);
    } else if (!toInside) {
        to = IntersectNearPlane(from, to, clipPlane);
    }
    return true;
}

inline void AppendPolygonPoint(std::array<Vec3, 4>& points, std::size_t& count, Vec3 point)
{
    if (count < points.size()) {
        points[count++] = point;
    }
}

inline std::size_t ClipTriangleToNearPlane(const DebugCamera& camera, const std::array<Vec3, 3>& input, std::array<Vec3, 4>& output)
{
    const float nearPlane = EffectiveNearPlane(camera);
    const float clipPlane = nearPlane + ProjectionEpsilon;
    std::size_t outputCount = 0;

    for (std::size_t index = 0; index < input.size(); ++index) {
        const Vec3 current = input[index];
        const Vec3 next = input[(index + 1) % input.size()];
        const bool currentInside = current.z > nearPlane;
        const bool nextInside = next.z > nearPlane;

        if (currentInside && nextInside) {
            AppendPolygonPoint(output, outputCount, next);
        } else if (currentInside && !nextInside) {
            AppendPolygonPoint(output, outputCount, IntersectNearPlane(current, next, clipPlane));
        } else if (!currentInside && nextInside) {
            AppendPolygonPoint(output, outputCount, IntersectNearPlane(current, next, clipPlane));
            AppendPolygonPoint(output, outputCount, next);
        }
    }

    return outputCount;
}

} // namespace detail

inline bool ProjectWorldPoint(const DebugCamera& camera, float aspectRatio, Vec3 point, ProjectedPoint& projected)
{
    detail::DebugProjectionBasis basis;
    if (!detail::BuildDebugProjectionBasis(camera, basis)) {
        return false;
    }

    return detail::ProjectCameraPoint(camera, aspectRatio, detail::ToCameraSpace(camera, basis, point), projected);
}

inline bool ProjectWorldLine(
    const DebugCamera& camera,
    float aspectRatio,
    Vec3 from,
    Vec3 to,
    ProjectedPoint& projectedFrom,
    ProjectedPoint& projectedTo)
{
    detail::DebugProjectionBasis basis;
    if (!detail::BuildDebugProjectionBasis(camera, basis)) {
        return false;
    }

    Vec3 cameraFrom = detail::ToCameraSpace(camera, basis, from);
    Vec3 cameraTo = detail::ToCameraSpace(camera, basis, to);
    if (!detail::ClipLineToNearPlane(camera, cameraFrom, cameraTo)) {
        return false;
    }

    return detail::ProjectCameraPoint(camera, aspectRatio, cameraFrom, projectedFrom)
        && detail::ProjectCameraPoint(camera, aspectRatio, cameraTo, projectedTo);
}

inline bool ProjectWorldTriangleWithDepth(
    const DebugCamera& camera,
    float aspectRatio,
    Vec3 a,
    Vec3 b,
    Vec3 c,
    ProjectedTriangle& projected)
{
    detail::DebugProjectionBasis basis;
    if (!detail::BuildDebugProjectionBasis(camera, basis)) {
        return false;
    }

    const std::array<Vec3, 3> cameraPoints {{
        detail::ToCameraSpace(camera, basis, a),
        detail::ToCameraSpace(camera, basis, b),
        detail::ToCameraSpace(camera, basis, c),
    }};

    std::array<Vec3, 4> clippedPoints {};
    const std::size_t clippedPointCount = detail::ClipTriangleToNearPlane(camera, cameraPoints, clippedPoints);
    if (clippedPointCount < 3) {
        projected.polygon.pointCount = 0;
        projected.cameraDepth = 0.0f;
        return false;
    }

    projected.polygon.pointCount = 0;
    float cameraDepthSum = 0.0f;
    for (std::size_t index = 0; index < clippedPointCount; ++index) {
        ProjectedPoint point;
        if (!detail::ProjectCameraPoint(camera, aspectRatio, clippedPoints[index], point)) {
            projected.polygon.pointCount = 0;
            projected.cameraDepth = 0.0f;
            return false;
        }
        projected.polygon.points[projected.polygon.pointCount++] = point;
        cameraDepthSum += clippedPoints[index].z;
    }
    projected.cameraDepth = cameraDepthSum / static_cast<float>(clippedPointCount);
    return true;
}

inline bool ProjectWorldTriangle(
    const DebugCamera& camera,
    float aspectRatio,
    Vec3 a,
    Vec3 b,
    Vec3 c,
    ProjectedPolygon& projected)
{
    ProjectedTriangle triangle;
    const bool result = ProjectWorldTriangleWithDepth(camera, aspectRatio, a, b, c, triangle);
    projected = triangle.polygon;
    return result;
}

inline void SortProjectedTrianglesBackToFront(std::vector<ProjectedTriangle>& triangles)
{
    std::stable_sort(triangles.begin(),
        triangles.end(),
        [](const ProjectedTriangle& lhs, const ProjectedTriangle& rhs) {
            return lhs.cameraDepth > rhs.cameraDepth;
        });
}

} // namespace engine
