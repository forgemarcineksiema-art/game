#pragma once

#include "engine/math/Math.h"
#include "engine/renderer/Renderer.h"

#include <span>
#include <string_view>

struct ScenePresentationState {
    bool routeOpened = false;
    bool powerRestored = false;
    bool vehicleOccupied = false;
};

bool IsKnownSceneColorKey(std::string_view key);
engine::Color SceneColorForKey(std::string_view key, ScenePresentationState state = {});
engine::Color SceneShadedColor(engine::Color color, engine::Vec3 a, engine::Vec3 b, engine::Vec3 c);
void DrawSceneShadedBox(engine::IRenderer& renderer, engine::Vec3 center, engine::Vec3 halfExtents, engine::Color color);
void DrawSceneShadedTriangleList(engine::IRenderer& renderer, std::span<const engine::Vec3> triangles, engine::Color color);
