#include "game/SceneLoader.h"

#include "engine/math/Math.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

using json = nlohmann::json;

std::filesystem::path ResolveReadablePath(const std::filesystem::path& path)
{
    if (path.empty() || path.is_absolute() || std::filesystem::exists(path)) {
        return path;
    }

#ifdef ENGINE_SOURCE_ROOT
    const std::filesystem::path sourcePath = std::filesystem::path(ENGINE_SOURCE_ROOT) / path;
    if (std::filesystem::exists(sourcePath)) {
        return sourcePath;
    }
#endif

    return path;
}

const json& Required(const json& object, std::string_view key, std::string_view path)
{
    const auto found = object.find(std::string(key));
    if (found == object.end()) {
        throw std::runtime_error(std::string(path) + "." + std::string(key) + " is required.");
    }

    return *found;
}

std::string ReadString(const json& object, std::string_view key, std::string_view path)
{
    const json& value = Required(object, key, path);
    if (!value.is_string()) {
        throw std::runtime_error(std::string(path) + "." + std::string(key) + " must be a string.");
    }

    return value.get<std::string>();
}

std::string ReadOptionalString(const json& object, std::string_view key, std::string_view path)
{
    const auto found = object.find(std::string(key));
    if (found == object.end() || found->is_null()) {
        return {};
    }
    if (!found->is_string()) {
        throw std::runtime_error(std::string(path) + "." + std::string(key) + " must be a string.");
    }

    return found->get<std::string>();
}

float ReadFloat(const json& object, std::string_view key, std::string_view path)
{
    const json& value = Required(object, key, path);
    if (!value.is_number()) {
        throw std::runtime_error(std::string(path) + "." + std::string(key) + " must be a number.");
    }

    return value.get<float>();
}

float ReadOptionalFloat(const json& object, std::string_view key, float fallback, std::string_view path)
{
    const auto found = object.find(std::string(key));
    if (found == object.end() || found->is_null()) {
        return fallback;
    }
    if (!found->is_number()) {
        throw std::runtime_error(std::string(path) + "." + std::string(key) + " must be a number.");
    }

    return found->get<float>();
}

bool ReadOptionalBool(const json& object, std::string_view key, bool fallback, std::string_view path)
{
    const auto found = object.find(std::string(key));
    if (found == object.end() || found->is_null()) {
        return fallback;
    }
    if (!found->is_boolean()) {
        throw std::runtime_error(std::string(path) + "." + std::string(key) + " must be a boolean.");
    }

    return found->get<bool>();
}

engine::Vec2 ReadVec2Value(const json& value, std::string_view path)
{
    if (!value.is_array() || value.size() != 2 || !value[0].is_number() || !value[1].is_number()) {
        throw std::runtime_error(std::string(path) + " must be a numeric [x, z] array.");
    }

    return {value[0].get<float>(), value[1].get<float>()};
}

engine::Vec3 ReadVec3Value(const json& value, std::string_view path)
{
    if (!value.is_array() || value.size() != 3 || !value[0].is_number() || !value[1].is_number() || !value[2].is_number()) {
        throw std::runtime_error(std::string(path) + " must be a numeric [x, y, z] array.");
    }

    return {value[0].get<float>(), value[1].get<float>(), value[2].get<float>()};
}

SceneColorDefinition ReadColorValue(const json& value, std::string_view path)
{
    if (!value.is_array() || (value.size() != 3 && value.size() != 4)) {
        throw std::runtime_error(std::string(path) + " must be a numeric [r, g, b] or [r, g, b, a] array.");
    }
    for (std::size_t index = 0; index < value.size(); ++index) {
        if (!value[index].is_number()) {
            throw std::runtime_error(std::string(path) + " must contain numeric color components.");
        }
    }

    const float alpha = value.size() == 4 ? value[3].get<float>() : 1.0f;
    return {value[0].get<float>(), value[1].get<float>(), value[2].get<float>(), alpha};
}

engine::Vec3 ReadVec3(const json& object, std::string_view key, std::string_view path)
{
    return ReadVec3Value(Required(object, key, path), std::string(path) + "." + std::string(key));
}

std::vector<std::string> ReadOptionalStringArray(const json& object, std::string_view key, std::string_view path)
{
    const auto found = object.find(std::string(key));
    if (found == object.end() || found->is_null()) {
        return {};
    }
    if (!found->is_array()) {
        throw std::runtime_error(std::string(path) + "." + std::string(key) + " must be an array.");
    }

    std::vector<std::string> values;
    for (std::size_t index = 0; index < found->size(); ++index) {
        if (!(*found)[index].is_string()) {
            throw std::runtime_error(std::string(path) + "." + std::string(key) + "[] must contain strings.");
        }
        values.push_back((*found)[index].get<std::string>());
    }
    return values;
}

SceneColliderDefinition ParseCollider(const json& value, std::size_t index)
{
    const std::string path = "colliders[" + std::to_string(index) + "]";
    SceneColliderDefinition collider;
    collider.id = ReadString(value, "id", path);
    collider.kind = ReadString(value, "kind", path);
    collider.center = ReadVec3(value, "center", path);
    collider.halfExtents = ReadVec3(value, "halfExtents", path);
    collider.blocksPlayer = ReadOptionalBool(value, "blocksPlayer", true, path);
    collider.stateFlag = ReadOptionalString(value, "stateFlag", path);
    collider.blocksWhenFlagFalse = ReadOptionalBool(value, "blocksWhenFlagFalse", false, path);
    return collider;
}

SceneMaterialDefinition ParseSceneMaterial(const json& value, std::size_t index)
{
    const std::string path = "sceneMaterials[" + std::to_string(index) + "]";
    SceneMaterialDefinition material;
    material.key = ReadString(value, "key", path);
    material.response = ReadString(value, "response", path);
    material.baseColor = ReadColorValue(Required(value, "baseColor", path), path + ".baseColor");
    return material;
}

SceneVisualPlaceholderDefinition ParseVisualPlaceholder(const json& value, std::size_t index)
{
    const std::string path = "visualPlaceholders[" + std::to_string(index) + "]";
    SceneVisualPlaceholderDefinition placeholder;
    placeholder.id = ReadString(value, "id", path);
    placeholder.role = ReadString(value, "role", path);
    placeholder.center = ReadVec3(value, "center", path);
    placeholder.halfExtents = ReadVec3(value, "halfExtents", path);
    placeholder.colorKey = ReadString(value, "colorKey", path);
    return placeholder;
}

SceneMeshAssetDefinition ParseMeshAsset(const json& value, std::size_t index)
{
    const std::string path = "meshAssets[" + std::to_string(index) + "]";
    SceneMeshAssetDefinition asset;
    asset.id = ReadString(value, "id", path);
    asset.path = ReadString(value, "path", path);
    asset.format = ReadOptionalString(value, "format", path);
    asset.license = ReadOptionalString(value, "license", path);
    asset.provenance = ReadOptionalString(value, "provenance", path);
    return asset;
}

SceneMeshInstanceDefinition ParseMeshInstance(const json& value, std::size_t index)
{
    const std::string path = "meshInstances[" + std::to_string(index) + "]";
    SceneMeshInstanceDefinition instance;
    instance.id = ReadString(value, "id", path);
    instance.assetId = ReadString(value, "assetId", path);
    instance.position = ReadVec3(value, "position", path);
    instance.scale = ReadVec3(value, "scale", path);
    instance.yawRadians = engine::Radians(ReadOptionalFloat(value, "yawDegrees", 0.0f, path));
    instance.colorKey = ReadString(value, "colorKey", path);
    instance.replacesVisualPlaceholderId = ReadOptionalString(value, "replacesVisualPlaceholderId", path);
    instance.linkedColliderId = ReadOptionalString(value, "linkedColliderId", path);
    return instance;
}

SceneInteractableDefinition ParseInteractable(const json& value, std::size_t index)
{
    const std::string path = "interactables[" + std::to_string(index) + "]";
    SceneInteractableDefinition interactable;
    interactable.id = ReadString(value, "id", path);
    interactable.name = ReadString(value, "name", path);
    interactable.prompt = ReadString(value, "prompt", path);
    interactable.type = ReadString(value, "type", path);
    interactable.position = ReadVec3(value, "position", path);
    interactable.radius = ReadFloat(value, "radius", path);
    interactable.oneShot = ReadOptionalBool(value, "oneShot", false, path);
    interactable.message = ReadOptionalString(value, "message", path);
    interactable.worldFlagsSet = ReadOptionalStringArray(value, "worldFlagsSet", path);
    interactable.worldFlagsSetWhenReady = ReadOptionalStringArray(value, "worldFlagsSetWhenReady", path);
    return interactable;
}

SceneTraversalAffordanceDefinition ParseTraversalAffordance(const json& value, std::size_t index)
{
    const std::string path = "traversalAffordances[" + std::to_string(index) + "]";
    SceneTraversalAffordanceDefinition affordance;
    affordance.id = ReadString(value, "id", path);
    affordance.name = ReadString(value, "name", path);
    affordance.prompt = ReadString(value, "prompt", path);
    affordance.type = ReadString(value, "type", path);
    affordance.startPosition = ReadVec3(value, "startPosition", path);
    affordance.endPosition = ReadVec3(value, "endPosition", path);
    affordance.focusRadius = ReadFloat(value, "focusRadius", path);
    affordance.requiredFacingDirection = ReadVec3(value, "requiredFacingDirection", path);
    affordance.requiredFacingDot = ReadFloat(value, "requiredFacingDot", path);
    affordance.durationSeconds = ReadFloat(value, "durationSeconds", path);
    affordance.worldFlagsSetOnComplete = ReadOptionalStringArray(value, "worldFlagsSetOnComplete", path);
    return affordance;
}

SceneVehicleDefinition ParseVehicle(const json& value, std::size_t index)
{
    const std::string path = "vehicles[" + std::to_string(index) + "]";
    const json& spawn = Required(value, "spawn", path);
    const json& bounds = Required(value, "bounds", path);

    SceneVehicleDefinition vehicle;
    vehicle.id = ReadString(value, "id", path);
    vehicle.name = ReadString(value, "name", path);
    vehicle.spawnPosition = ReadVec3(spawn, "position", path + ".spawn");
    vehicle.spawnYawRadians = engine::Radians(ReadOptionalFloat(spawn, "yawDegrees", 0.0f, path + ".spawn"));
    vehicle.proxyHalfExtents = ReadVec3(value, "proxyHalfExtents", path);
    vehicle.enterRadius = ReadFloat(value, "enterRadius", path);
    vehicle.boundsMin = ReadVec2Value(Required(bounds, "min", path + ".bounds"), path + ".bounds.min");
    vehicle.boundsMax = ReadVec2Value(Required(bounds, "max", path + ".bounds"), path + ".bounds.max");
    return vehicle;
}

SceneRouteMarkerDefinition ParseRouteMarker(const json& value, std::size_t index)
{
    const std::string path = "routeMarkers[" + std::to_string(index) + "]";
    SceneRouteMarkerDefinition marker;
    marker.id = ReadString(value, "id", path);
    marker.from = ReadString(value, "from", path);
    marker.to = ReadString(value, "to", path);
    const json& points = Required(value, "points", path);
    if (!points.is_array()) {
        throw std::runtime_error(path + ".points must be an array.");
    }
    for (std::size_t pointIndex = 0; pointIndex < points.size(); ++pointIndex) {
        marker.points.push_back(ReadVec3Value(points[pointIndex], path + ".points[" + std::to_string(pointIndex) + "]"));
    }
    return marker;
}

SceneObjectiveMarkerDefinition ParseObjectiveMarker(const json& value, std::size_t index)
{
    const std::string path = "objectiveMarkers[" + std::to_string(index) + "]";
    SceneObjectiveMarkerDefinition marker;
    marker.id = ReadString(value, "id", path);
    marker.label = ReadString(value, "label", path);
    marker.position = ReadVec3(value, "position", path);
    return marker;
}

template <typename Parser, typename Output>
void ReadArray(const json& scene, std::string_view key, std::vector<Output>& output, Parser parser)
{
    const auto found = scene.find(std::string(key));
    if (found == scene.end() || found->is_null()) {
        return;
    }
    if (!found->is_array()) {
        throw std::runtime_error(std::string(key) + " must be an array.");
    }

    for (std::size_t index = 0; index < found->size(); ++index) {
        output.push_back(parser((*found)[index], index));
    }
}

SceneDefinition ParseScene(const json& root)
{
    SceneDefinition scene;
    scene.schemaVersion = Required(root, "schemaVersion", "scene").get<int>();
    scene.id = ReadString(root, "id", "scene");
    scene.name = ReadString(root, "name", "scene");
    scene.floorHeight = ReadFloat(root, "floorHeight", "scene");

    if (const auto units = root.find("units"); units != root.end() && units->is_object()) {
        scene.linearUnits = ReadOptionalString(*units, "linear", "scene.units");
    }

    const json& playerStart = Required(root, "playerStart", "scene");
    scene.playerStart.id = ReadString(playerStart, "id", "scene.playerStart");
    scene.playerStart.position = ReadVec3(playerStart, "position", "scene.playerStart");
    scene.playerStart.yawRadians = engine::Radians(ReadOptionalFloat(playerStart, "yawDegrees", 0.0f, "scene.playerStart"));

    ReadArray(root, "colliders", scene.colliders, ParseCollider);
    ReadArray(root, "sceneMaterials", scene.sceneMaterials, ParseSceneMaterial);
    ReadArray(root, "visualPlaceholders", scene.visualPlaceholders, ParseVisualPlaceholder);
    ReadArray(root, "meshAssets", scene.meshAssets, ParseMeshAsset);
    ReadArray(root, "meshInstances", scene.meshInstances, ParseMeshInstance);
    ReadArray(root, "interactables", scene.interactables, ParseInteractable);
    ReadArray(root, "traversalAffordances", scene.traversalAffordances, ParseTraversalAffordance);
    ReadArray(root, "vehicles", scene.vehicles, ParseVehicle);
    ReadArray(root, "routeMarkers", scene.routeMarkers, ParseRouteMarker);
    ReadArray(root, "objectiveMarkers", scene.objectiveMarkers, ParseObjectiveMarker);
    return scene;
}

} // namespace

bool SceneLoadResult::ok() const
{
    return error.empty();
}

SceneLoadResult LoadSceneDefinition(const std::filesystem::path& path)
{
    SceneLoadResult result;
    const std::filesystem::path resolvedPath = ResolveReadablePath(path);

    std::ifstream file(resolvedPath);
    if (!file) {
        result.error = "Scene file not found or could not be read: " + path.string();
        return result;
    }

    try {
        const json root = json::parse(file);
        result.scene = ParseScene(root);
    } catch (const std::exception& exception) {
        result.error = "Scene load failed for " + resolvedPath.string() + ": " + exception.what();
    }

    return result;
}
