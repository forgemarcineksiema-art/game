#include "game/ScenePresentation.h"

#include <algorithm>
#include <array>

namespace {

engine::Color ScaleColor(engine::Color color, float scale)
{
    return {color.r * scale, color.g * scale, color.b * scale, color.a};
}

engine::Color ToRendererColor(SceneColorDefinition color)
{
    return {color.r, color.g, color.b, color.a};
}

SceneMaterial DefaultMaterial(engine::Color color)
{
    SceneMaterial material;
    material.baseColor = color;
    return material;
}

SceneMaterial WetMaterial(engine::Color color)
{
    SceneMaterial material = DefaultMaterial(color);
    material.ambientShade = 0.56f;
    material.directionalShade = 0.36f;
    material.topShade = 0.16f;
    material.undersideShade = 0.10f;
    material.minShade = 0.44f;
    material.maxShade = 1.16f;
    return material;
}

SceneMaterial MatteMaterial(engine::Color color)
{
    SceneMaterial material = DefaultMaterial(color);
    material.ambientShade = 0.64f;
    material.directionalShade = 0.22f;
    material.topShade = 0.08f;
    material.undersideShade = 0.07f;
    material.minShade = 0.54f;
    material.maxShade = 1.02f;
    return material;
}

SceneMaterial PaintedMaterial(engine::Color color)
{
    SceneMaterial material = DefaultMaterial(color);
    material.ambientShade = 0.60f;
    material.directionalShade = 0.30f;
    material.topShade = 0.12f;
    material.undersideShade = 0.08f;
    material.minShade = 0.48f;
    material.maxShade = 1.12f;
    return material;
}

SceneMaterial MaterialForResponse(std::string_view response, engine::Color color)
{
    if (response == "wet") {
        return WetMaterial(color);
    }
    if (response == "painted") {
        return PaintedMaterial(color);
    }
    return MatteMaterial(color);
}

engine::Color DynamicSceneColor(std::string_view key, engine::Color authoredBaseColor, ScenePresentationState state)
{
    if (key == "oxidized-service-green" && state.powerRestored) {
        return {0.18f, 0.74f, 0.62f, 1.0f};
    }
    if (key == "service-gate-state" && state.routeOpened) {
        return {0.12f, 0.36f, 0.20f, 1.0f};
    }
    if (key == "service-vehicle-placeholder" && state.vehicleOccupied) {
        return {0.18f, 0.58f, 0.95f, 1.0f};
    }
    if (key == "dock-road-relay-state") {
        return state.dockRoadRelayReset
            ? engine::Color {0.18f, 0.78f, 0.42f, 1.0f}
            : engine::Color {0.82f, 0.46f, 0.12f, 1.0f};
    }
    if (key == "dock-road-clearance-state") {
        return state.dockRoadClearanceTagged
            ? engine::Color {0.16f, 0.72f, 0.82f, 1.0f}
            : engine::Color {0.44f, 0.50f, 0.48f, 1.0f};
    }
    if (key == "service-vehicle-cabin-placeholder") {
        const engine::Color body = state.vehicleOccupied
            ? engine::Color {0.18f, 0.58f, 0.95f, 1.0f}
            : engine::Color {0.62f, 0.66f, 0.48f, 1.0f};
        return ScaleColor(body, 0.78f);
    }
    return authoredBaseColor;
}

void DrawShadedTriangle(engine::IRenderer& renderer, engine::Vec3 a, engine::Vec3 b, engine::Vec3 c, SceneMaterial material)
{
    const std::array<engine::Vec3, 3> triangle {a, b, c};
    renderer.drawDebugFlatTriangles(triangle, SceneShadedColor(material, a, b, c));
}

} // namespace

bool IsKnownSceneColorKey(std::string_view key)
{
    return key == "dock-weathered-wood"
        || key == "office-muted-concrete"
        || key == "damp-service-concrete"
        || key == "deep-harbor-blue"
        || key == "misty-island-ground"
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
        || key == "service-vehicle-cabin-placeholder"
        || key == "dock-road-relay-state"
        || key == "dock-road-clearance-state";
}

SceneMaterial SceneMaterialForKey(std::string_view key, ScenePresentationState state)
{
    if (key == "dock-weathered-wood") {
        return MatteMaterial({0.33f, 0.30f, 0.22f, 1.0f});
    }
    if (key == "office-muted-concrete") {
        return MatteMaterial({0.25f, 0.28f, 0.24f, 1.0f});
    }
    if (key == "damp-service-concrete") {
        return WetMaterial({0.18f, 0.23f, 0.22f, 1.0f});
    }
    if (key == "deep-harbor-blue") {
        return WetMaterial({0.05f, 0.13f, 0.22f, 1.0f});
    }
    if (key == "misty-island-ground") {
        return MatteMaterial({0.20f, 0.28f, 0.24f, 1.0f});
    }
    if (key == "rusted-roof-trim") {
        return MatteMaterial({0.44f, 0.24f, 0.15f, 1.0f});
    }
    if (key == "wet-timber") {
        return WetMaterial({0.38f, 0.30f, 0.14f, 1.0f});
    }
    if (key == "oxidized-service-green") {
        return PaintedMaterial(state.powerRestored
            ? engine::Color {0.18f, 0.74f, 0.62f, 1.0f}
            : engine::Color {0.11f, 0.40f, 0.36f, 1.0f});
    }
    if (key == "dark-service-asphalt") {
        return WetMaterial({0.15f, 0.18f, 0.17f, 1.0f});
    }
    if (key == "weathered-yard-rail") {
        return MatteMaterial({0.42f, 0.30f, 0.13f, 1.0f});
    }
    if (key == "mossy-service-crate") {
        return MatteMaterial({0.24f, 0.32f, 0.22f, 1.0f});
    }
    if (key == "dock-muted-sign-yellow") {
        return PaintedMaterial({0.78f, 0.65f, 0.22f, 1.0f});
    }
    if (key == "ferry-route-sign-blue") {
        return PaintedMaterial({0.10f, 0.32f, 0.42f, 1.0f});
    }
    if (key == "salt-white-road-post") {
        return PaintedMaterial({0.74f, 0.76f, 0.68f, 1.0f});
    }
    if (key == "warning-service-orange") {
        return PaintedMaterial({0.82f, 0.35f, 0.12f, 1.0f});
    }
    if (key == "service-gate-state") {
        return PaintedMaterial(state.routeOpened
            ? engine::Color {0.12f, 0.36f, 0.20f, 1.0f}
            : engine::Color {0.42f, 0.12f, 0.08f, 1.0f});
    }
    if (key == "service-vehicle-placeholder") {
        return PaintedMaterial(state.vehicleOccupied
            ? engine::Color {0.18f, 0.58f, 0.95f, 1.0f}
            : engine::Color {0.62f, 0.66f, 0.48f, 1.0f});
    }
    if (key == "service-vehicle-cabin-placeholder") {
        const engine::Color body = SceneMaterialForKey("service-vehicle-placeholder", state).baseColor;
        return PaintedMaterial(ScaleColor(body, 0.78f));
    }
    if (key == "dock-road-relay-state") {
        return PaintedMaterial(state.dockRoadRelayReset
            ? engine::Color {0.18f, 0.78f, 0.42f, 1.0f}
            : engine::Color {0.82f, 0.46f, 0.12f, 1.0f});
    }
    if (key == "dock-road-clearance-state") {
        return PaintedMaterial(state.dockRoadClearanceTagged
            ? engine::Color {0.16f, 0.72f, 0.82f, 1.0f}
            : engine::Color {0.44f, 0.50f, 0.48f, 1.0f});
    }

    return DefaultMaterial({0.35f, 0.42f, 0.40f, 1.0f});
}

SceneMaterial SceneMaterialForKey(std::string_view key, std::span<const SceneMaterialDefinition> authoredMaterials, ScenePresentationState state)
{
    for (const SceneMaterialDefinition& authored : authoredMaterials) {
        if (authored.key == key) {
            return MaterialForResponse(authored.response, DynamicSceneColor(key, ToRendererColor(authored.baseColor), state));
        }
    }
    return SceneMaterialForKey(key, state);
}

engine::Color SceneColorForKey(std::string_view key, ScenePresentationState state)
{
    return SceneMaterialForKey(key, state).baseColor;
}

engine::Color SceneShadedColor(engine::Color color, engine::Vec3 a, engine::Vec3 b, engine::Vec3 c)
{
    return SceneShadedColor(DefaultMaterial(color), a, b, c);
}

engine::Color SceneShadedColor(SceneMaterial material, engine::Vec3 a, engine::Vec3 b, engine::Vec3 c)
{
    const engine::Vec3 normal = engine::Normalize(engine::Cross(b - a, c - a));
    const engine::Vec3 overcastLight = engine::Normalize(engine::Vec3 {-0.35f, 0.82f, -0.45f});
    const float directional = std::max(0.0f, engine::Dot(normal, overcastLight));
    const float topLight = std::max(0.0f, normal.y);
    const float underside = std::max(0.0f, -normal.y);
    const float shade = engine::Clamp(
        material.ambientShade + directional * material.directionalShade + topLight * material.topShade - underside * material.undersideShade,
        material.minShade,
        material.maxShade);
    return ScaleColor(material.baseColor, shade);
}

void DrawSceneShadedBox(engine::IRenderer& renderer, engine::Vec3 center, engine::Vec3 halfExtents, engine::Color color)
{
    DrawSceneShadedBox(renderer, center, halfExtents, DefaultMaterial(color));
}

void DrawSceneShadedBox(engine::IRenderer& renderer, engine::Vec3 center, engine::Vec3 halfExtents, SceneMaterial material)
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
        DrawShadedTriangle(renderer, corners[triangle[0]], corners[triangle[1]], corners[triangle[2]], material);
    }
}

void DrawSceneShadedTriangleList(engine::IRenderer& renderer, std::span<const engine::Vec3> triangles, engine::Color color)
{
    DrawSceneShadedTriangleList(renderer, triangles, DefaultMaterial(color));
}

void DrawSceneShadedTriangleList(engine::IRenderer& renderer, std::span<const engine::Vec3> triangles, SceneMaterial material)
{
    for (std::size_t index = 0; index + 2 < triangles.size(); index += 3) {
        DrawShadedTriangle(renderer, triangles[index], triangles[index + 1], triangles[index + 2], material);
    }
}
