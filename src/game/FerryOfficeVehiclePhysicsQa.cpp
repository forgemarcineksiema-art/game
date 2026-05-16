#include "game/FerryOfficeVehiclePhysicsQa.h"

#include "game/SceneDefinition.h"
#include "game/SceneLoader.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string_view>

namespace {

constexpr std::string_view ScenarioName = "ferry-office-vehicle-feasibility";
constexpr std::string_view ServiceVehicleId = "service-yard-vehicle";

nlohmann::json Vec3Json(engine::Vec3 value)
{
    return nlohmann::json::array({value.x, value.y, value.z});
}

nlohmann::json SampleJson(const engine::physics::VehicleProbeSample& sample)
{
    return {
        {"name", sample.name},
        {"passed", sample.passed},
        {"position", Vec3Json(sample.position)},
        {"yawDegrees", engine::Degrees(sample.yawRadians)},
        {"speed", sample.speed},
        {"wheelContactCount", sample.wheelContactCount},
        {"maxPitchDegrees", sample.maxPitchDegrees},
        {"maxRollDegrees", sample.maxRollDegrees},
        {"outOfBounds", sample.outOfBounds},
        {"message", sample.message},
    };
}

bool WriteReport(const FerryOfficeVehiclePhysicsQaResult& result)
{
    if (result.reportPath.empty()) {
        return true;
    }

    const std::filesystem::path parent = result.reportPath.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    nlohmann::json samples = nlohmann::json::array();
    for (const engine::physics::VehicleProbeSample& sample : result.samples) {
        samples.push_back(SampleJson(sample));
    }

    const nlohmann::json report = {
        {"schema", "v0.35-ferry-office-vehicle-feasibility"},
        {"scenario", result.scenario},
        {"passed", result.passed},
        {"backend", result.backendName},
        {"scene", {{"id", result.sceneId}, {"path", result.scenePath.generic_string()}}},
        {"vehicle", {
            {"id", result.vehicleId},
            {"spawnPosition", Vec3Json(result.spawnPosition)},
            {"proxyHalfExtents", Vec3Json(result.proxyHalfExtents)},
        }},
        {"inputFrameCount", result.inputFrameCount},
        {"samples", samples},
        {"final", {
            {"position", Vec3Json(result.finalPosition)},
            {"yawDegrees", engine::Degrees(result.finalYawRadians)},
            {"speed", result.finalSpeed},
        }},
        {"recommendation", result.recommendation},
        {"recommendationReason", result.recommendationReason},
        {"error", result.error},
    };

    std::ofstream output(result.reportPath);
    if (!output) {
        return false;
    }
    output << report.dump(2) << '\n';
    return true;
}

} // namespace

std::filesystem::path DefaultFerryOfficeVehiclePhysicsReportPath()
{
    return std::filesystem::path("build") / "physics" / "ferry-office-vehicle-feasibility-report.json";
}

FerryOfficeVehiclePhysicsQaResult RunFerryOfficeVehiclePhysicsQa(
    const std::filesystem::path& scenePath,
    const std::filesystem::path& requestedReportPath,
    engine::physics::PhysicsBackend backend)
{
    FerryOfficeVehiclePhysicsQaResult result;
    result.scenario = std::string(ScenarioName);
    result.backendName = "unavailable";
    result.scenePath = scenePath;
    result.reportPath = requestedReportPath.empty() ? DefaultFerryOfficeVehiclePhysicsReportPath() : requestedReportPath;

    const SceneLoadResult loadedScene = LoadSceneDefinition(scenePath);
    if (!loadedScene.ok()) {
        result.error = loadedScene.error;
        WriteReport(result);
        return result;
    }
    result.sceneId = loadedScene.scene.id;

    const SceneVehicleDefinition* vehicle = FindSceneVehicleById(loadedScene.scene, std::string(ServiceVehicleId));
    if (vehicle == nullptr) {
        result.error = "Ferry Office scene is missing service-yard vehicle data.";
        WriteReport(result);
        return result;
    }

    engine::physics::VehicleProbeConfig config;
    config.vehicleId = vehicle->id;
    config.spawnPosition = vehicle->spawnPosition;
    config.spawnYawRadians = vehicle->spawnYawRadians;
    config.halfExtents = vehicle->proxyHalfExtents;
    config.boundsMin = vehicle->boundsMin;
    config.boundsMax = vehicle->boundsMax;

    const engine::physics::VehicleProbeResult probe =
        engine::physics::RunVehicleFeasibilityProbe(
            config,
            engine::physics::DefaultVehicleFeasibilityInputScript(),
            backend);

    result.passed = probe.passed;
    result.backendName = probe.backendName;
    result.vehicleId = vehicle->id;
    result.spawnPosition = vehicle->spawnPosition;
    result.proxyHalfExtents = vehicle->proxyHalfExtents;
    result.inputFrameCount = probe.inputFrameCount;
    result.samples = probe.samples;
    result.finalPosition = probe.finalPosition;
    result.finalYawRadians = probe.finalYawRadians;
    result.finalSpeed = probe.finalSpeed;
    result.recommendation = probe.recommendation;
    result.recommendationReason = probe.recommendationReason;
    result.error = probe.error;

    if (!WriteReport(result)) {
        result.passed = false;
        result.error = "Failed to write vehicle physics report: " + result.reportPath.string();
    }
    return result;
}
