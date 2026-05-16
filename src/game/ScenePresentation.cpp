#include "game/ScenePresentation.h"

#include <algorithm>
#include <array>

namespace {

engine::Color ScaleColor(engine::Color color, float scale)
{
    return {color.r * scale, color.g * scale, color.b * scale, color.a};
}

void DrawShadedTriangle(engine::IRenderer& renderer, engine::Vec3 a, engine::Vec3 b, engine::Vec3 c, engine::Color color)
{
    const std::array<engine::Vec3, 3> triangle {a, b, c};
    renderer.drawDebugFlatTriangles(triangle, SceneShadedColor(color, a, b, c));
}

} // namespace

bool IsKnownSceneColorKey(std::string_view key)
{
    return key == "dock-weathered-wood"
        || key == "office-muted-concrete"
        || key == "damp-service-concrete"
        || key == "deep-harbor-blue"
        || key == "rusted-roof-trim"
        || key == "wet-timber"
        || key == "oxidized-service-green"
        || key == "dark-service-asphalt"
        || key == "weathered-yard-rail"
        || key == "mossy-service-crate"
        || key == "dock-muted-sign-yellow"
        || key == "ferry-route-sign-blue"
        || key == "salt-white-road-post"
        || key == "warning-service-orange"
        || key == "service-gate-state"
        || key == "service-vehicle-placeholder"
        || key == "service-vehicle-cabin-placeholder";
}

engine::Color SceneColorForKey(std::string_view key, ScenePresentationState state)
{
    if (key == "dock-weathered-wood") {
        return {0.33f, 0.30f, 0.22f, 1.0f};
    }
    if (key == "office-muted-concrete") {
        return {0.25f, 0.28f, 0.24f, 1.0f};
    }
    if (key == "damp-service-concrete") {
        return {0.18f, 0.23f, 0.22f, 1.0f};
    }
    if (key == "deep-harbor-blue") {
        return {0.05f, 0.13f, 0.22f, 1.0f};
    }
    if (key == "rusted-roof-trim") {
        return {0.44f, 0.24f, 0.15f, 1.0f};
    }
    if (key == "wet-timber") {
        return {0.38f, 0.30f, 0.14f, 1.0f};
    }
    if (key == "oxidized-service-green") {
        return state.powerRestored
            ? engine::Color {0.18f, 0.74f, 0.62f, 1.0f}
            : engine::Color {0.11f, 0.40f, 0.36f, 1.0f};
    }
    if (key == "dark-service-asphalt") {
        return {0.15f, 0.18f, 0.17f, 1.0f};
    }
    if (key == "weathered-yard-rail") {
        return {0.42f, 0.30f, 0.13f, 1.0f};
    }
    if (key == "mossy-service-crate") {
        return {0.24f, 0.32f, 0.22f, 1.0f};
    }
    if (key == "dock-muted-sign-yellow") {
        return {0.78f, 0.65f, 0.22f, 1.0f};
    }
    if (key == "ferry-route-sign-blue") {
        return {0.10f, 0.32f, 0.42f, 1.0f};
    }
    if (key == "salt-white-road-post") {
        return {0.74f, 0.76f, 0.68f, 1.0f};
    }
    if (key == "warning-service-orange") {
        return {0.82f, 0.35f, 0.12f, 1.0f};
    }
    if (key == "service-gate-state") {
        return state.routeOpened
            ? engine::Color {0.12f, 0.36f, 0.20f, 1.0f}
            : engine::Color {0.42f, 0.12f, 0.08f, 1.0f};
    }
    if (key == "service-vehicle-placeholder") {
        return state.vehicleOccupied
            ? engine::Color {0.18f, 0.58f, 0.95f, 1.0f}
            : engine::Color {0.62f, 0.66f, 0.48f, 1.0f};
    }
    if (key == "service-vehicle-cabin-placeholder") {
        const engine::Color body = SceneColorForKey("service-vehicle-placeholder", state);
        return ScaleColor(body, 0.78f);
    }

    return {0.35f, 0.42f, 0.40f, 1.0f};
}

engine::Color SceneShadedColor(engine::Color color, engine::Vec3 a, engine::Vec3 b, engine::Vec3 c)
{
    const engine::Vec3 normal = engine::Normalize(engine::Cross(b - a, c - a));
    const engine::Vec3 overcastLight = engine::Normalize(engine::Vec3 {-0.35f, 0.82f, -0.45f});
    const float directional = std::max(0.0f, engine::Dot(normal, overcastLight));
    const float topLight = std::max(0.0f, normal.y);
    const float underside = std::max(0.0f, -normal.y);
    const float shade = engine::Clamp(0.62f + directional * 0.28f + topLight * 0.10f - underside * 0.08f, 0.50f, 1.10f);
    return ScaleColor(color, shade);
}

void DrawSceneShadedBox(engine::IRenderer& renderer, engine::Vec3 center, engine::Vec3 halfExtents, engine::Color color)
{
    const std::array<engine::Vec3, 8> corners {{
        center + engine::Vec3 {-halfExtents.x, -halfExtents.y, -halfExtents.z},
        center + engine::Vec3 { halfExtents.x, -halfExtents.y, -halfExtents.z},
        center + engine::Vec3 { halfExtents.x, -halfExtents.y,  halfExtents.z},
        center + engine::Vec3 {-halfExtents.x, -halfExtents.y,  halfExtents.z},
        center + engine::Vec3 {-halfExtents.x,  halfExtents.y, -halfExtents.z},
        center + engine::Vec3 { halfExtents.x,  halfExtents.y, -halfExtents.z},
        center + engine::Vec3 { halfExtents.x,  halfExtents.y,  halfExtents.z},
        center + engine::Vec3 {-halfExtents.x,  halfExtents.y,  halfExtents.z},
    }};

    const int triangles[][3] = {
        {0, 1, 2}, {0, 2, 3},
        {4, 5, 6}, {4, 6, 7},
        {0, 1, 5}, {0, 5, 4},
        {1, 2, 6}, {1, 6, 5},
        {2, 3, 7}, {2, 7, 6},
        {3, 0, 4}, {3, 4, 7},
    };

    for (const auto& triangle : triangles) {
        DrawShadedTriangle(renderer, corners[triangle[0]], corners[triangle[1]], corners[triangle[2]], color);
    }
}

void DrawSceneShadedTriangleList(engine::IRenderer& renderer, std::span<const engine::Vec3> triangles, engine::Color color)
{
    for (std::size_t index = 0; index + 2 < triangles.size(); index += 3) {
        DrawShadedTriangle(renderer, triangles[index], triangles[index + 1], triangles[index + 2], color);
    }
}
