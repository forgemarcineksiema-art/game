#pragma once

#include "engine/math/Math.h"

#include <filesystem>
#include <string>
#include <vector>

struct ScenePlayerStartDefinition {
    std::string id;
    engine::Vec3 position;
    float yawRadians = 0.0f;
};

struct SceneColliderDefinition {
    std::string id;
    std::string kind;
    engine::Vec3 center;
    engine::Vec3 halfExtents;
    bool blocksPlayer = true;
    std::string stateFlag;
    bool blocksWhenFlagFalse = false;
};

struct SceneVisualPlaceholderDefinition {
    std::string id;
    std::string role;
    engine::Vec3 center;
    engine::Vec3 halfExtents;
    std::string colorKey;
};

struct SceneColorDefinition {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

struct SceneMaterialDefinition {
    std::string key;
    std::string response;
    SceneColorDefinition baseColor;
};

struct SceneMeshAssetDefinition {
    std::string id;
    std::filesystem::path path;
    std::string format;
    std::string license;
    std::string provenance;
};

struct SceneMeshInstanceDefinition {
    std::string id;
    std::string assetId;
    engine::Vec3 position;
    engine::Vec3 scale {1.0f, 1.0f, 1.0f};
    float yawRadians = 0.0f;
    std::string colorKey;
    std::string replacesVisualPlaceholderId;
    std::string linkedColliderId;
};

struct SceneInteractableDefinition {
    std::string id;
    std::string name;
    std::string prompt;
    std::string type;
    engine::Vec3 position;
    float radius = 1.5f;
    bool oneShot = false;
    std::string message;
    std::vector<std::string> worldFlagsSet;
    std::vector<std::string> worldFlagsSetWhenReady;
};

struct SceneTraversalAffordanceDefinition {
    std::string id;
    std::string name;
    std::string prompt;
    std::string type;
    engine::Vec3 startPosition;
    engine::Vec3 endPosition;
    float focusRadius = 1.5f;
    engine::Vec3 requiredFacingDirection {0.0f, 0.0f, 1.0f};
    float requiredFacingDot = 0.25f;
    float durationSeconds = 0.45f;
    std::vector<std::string> worldFlagsSetOnComplete;
};

struct SceneVehicleDefinition {
    std::string id;
    std::string name;
    engine::Vec3 spawnPosition;
    float spawnYawRadians = 0.0f;
    engine::Vec3 proxyHalfExtents {0.58f, 0.53f, 0.92f};
    float enterRadius = 1.8f;
    engine::Vec2 boundsMin;
    engine::Vec2 boundsMax;
};

struct SceneRouteMarkerDefinition {
    std::string id;
    std::string from;
    std::string to;
    std::vector<engine::Vec3> points;
};

struct SceneObjectiveMarkerDefinition {
    std::string id;
    std::string label;
    engine::Vec3 position;
};

struct SceneDefinition {
    int schemaVersion = 0;
    std::string id;
    std::string name;
    std::string linearUnits;
    float floorHeight = 0.0f;
    ScenePlayerStartDefinition playerStart;
    std::vector<SceneMaterialDefinition> sceneMaterials;
    std::vector<SceneColliderDefinition> colliders;
    std::vector<SceneVisualPlaceholderDefinition> visualPlaceholders;
    std::vector<SceneMeshAssetDefinition> meshAssets;
    std::vector<SceneMeshInstanceDefinition> meshInstances;
    std::vector<SceneInteractableDefinition> interactables;
    std::vector<SceneTraversalAffordanceDefinition> traversalAffordances;
    std::vector<SceneVehicleDefinition> vehicles;
    std::vector<SceneRouteMarkerDefinition> routeMarkers;
    std::vector<SceneObjectiveMarkerDefinition> objectiveMarkers;
};

const SceneVehicleDefinition* FindSceneVehicleById(const SceneDefinition& scene, const std::string& id);
const SceneMeshAssetDefinition* FindSceneMeshAssetById(const SceneDefinition& scene, const std::string& id);
