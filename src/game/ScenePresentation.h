#pragma once

#include "engine/math/Math.h"
#include "engine/renderer/Renderer.h"
#include "game/SceneDefinition.h"

#include <span>
#include <string_view>

struct ScenePresentationState {
    bool routeOpened = false;
    bool powerRestored = false;
    bool vehicleOccupied = false;
    bool dockRoadRelayReset = false;
};

struct SceneMaterial {
    engine::Color baseColor {};
    float ambientShade = 0.62f;
    float directionalShade = 0.28f;
    float topShade = 0.10f;
    float undersideShade = 0.08f;
    float minShade = 0.50f;
    float maxShade = 1.10f;
};

bool IsKnownSceneColorKey(std::string_view key);
SceneMaterial SceneMaterialForKey(std::string_view key, ScenePresentationState state = {});
SceneMaterial SceneMaterialForKey(std::string_view key, std::span<const SceneMaterialDefinition> authoredMaterials, ScenePresentationState state = {});
engine::Color SceneColorForKey(std::string_view key, ScenePresentationState state = {});
engine::Color SceneShadedColor(engine::Color color, engine::Vec3 a, engine::Vec3 b, engine::Vec3 c);
engine::Color SceneShadedColor(SceneMaterial material, engine::Vec3 a, engine::Vec3 b, engine::Vec3 c);
void DrawSceneShadedBox(engine::IRenderer& renderer, engine::Vec3 center, engine::Vec3 halfExtents, engine::Color color);
void DrawSceneShadedBox(engine::IRenderer& renderer, engine::Vec3 center, engine::Vec3 halfExtents, SceneMaterial material);
void DrawSceneShadedTriangleList(engine::IRenderer& renderer, std::span<const engine::Vec3> triangles, engine::Color color);
void DrawSceneShadedTriangleList(engine::IRenderer& renderer, std::span<const engine::Vec3> triangles, SceneMaterial material);
