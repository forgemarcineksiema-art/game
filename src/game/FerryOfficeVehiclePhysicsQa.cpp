#include "game/FerryOfficeVehiclePhysicsQa.h"

#include "game/SceneDefinition.h"
#include "game/SceneLoader.h"
#include "game/VehicleController.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string_view>

namespace {

constexpr std::string_view ScenarioName = "ferry-office-vehicle-feasibility";
constexpr std::string_view RuntimeComparisonScenarioName = "ferry-office-vehicle-runtime-comparison";
constexpr std::string_view ServiceVehicleId = "service-yard-vehicle";
constexpr float RuntimePositionDeltaLimit = 4.0f;
constexpr float RuntimeYawDeltaLimitDegrees = 130.0f;
constexpr float RuntimeSpeedDeltaLimit = 5.0f;

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

float HorizontalDistance(engine::Vec3 lhs, engine::Vec3 rhs)
{
    return engine::Length(engine::Vec2 {lhs.x - rhs.x, lhs.z - rhs.z});
}

float AbsYawDeltaDegrees(float lhsRadians, float rhsRadians)
{
    float delta = std::fmod(std::abs(engine::Degrees(lhsRadians - rhsRadians)), 360.0f);
    if (delta > 180.0f) {
        delta = 360.0f - delta;
    }
    return delta;
}

engine::physics::VehicleProbeSample SampleFromRuntimeState(
    const std::string& name,
    const engine::physics::VehicleRuntimeState& state,
    std::string_view message)
{
    engine::physics::VehicleProbeSample sample;
    sample.name = name;
    sample.position = state.position;
    sample.yawRadians = state.yawRadians;
    sample.speed = state.speed;
    sample.wheelContactCount = state.wheelContactCount;
    sample.maxPitchDegrees = state.maxPitchDegrees;
    sample.maxRollDegrees = state.maxRollDegrees;
    sample.outOfBounds = state.outOfBounds;
    sample.passed = !state.outOfBounds && state.wheelContactCount >= 2;
    sample.message = std::string(message);
    return sample;
}

engine::InputState RuntimeInputForDeterministicController(const engine::physics::VehicleProbeInputFrame& input)
{
    engine::InputState output;
    output.moveRight = input.steer;
    if (input.brake > 0.0f) {
        output.moveForward = -input.brake;
    } else {
        output.moveForward = input.throttle;
    }
    return output;
}

engine::physics::VehicleRuntimeInput RuntimeInputForAdapter(const engine::physics::VehicleProbeInputFrame& input)
{
    return {input.throttle, input.steer, input.brake};
}

VehicleController BuildDeterministicVehicle(const SceneVehicleDefinition& vehicle)
{
    VehicleController controller;
    VehicleControllerSettings settings;
    settings.enterRadius = vehicle.enterRadius;
    settings.boundsMinX = vehicle.boundsMin.x;
    settings.boundsMaxX = vehicle.boundsMax.x;
    settings.boundsMinZ = vehicle.boundsMin.y;
    settings.boundsMaxZ = vehicle.boundsMax.y;
    controller.setSettings(settings);
    controller.setPosition(vehicle.spawnPosition);
    controller.setYawRadians(vehicle.spawnYawRadians);
    controller.setOccupiedForTesting(true);
    return controller;
}

engine::physics::VehicleRuntimeConfig RuntimeConfigFromSceneVehicle(const SceneVehicleDefinition& vehicle)
{
    engine::physics::VehicleRuntimeConfig config;
    config.vehicleId = vehicle.id;
    config.spawnPosition = vehicle.spawnPosition;
    config.spawnYawRadians = vehicle.spawnYawRadians;
    config.halfExtents = vehicle.proxyHalfExtents;
    config.boundsMin = vehicle.boundsMin;
    config.boundsMax = vehicle.boundsMax;
    return config;
}

engine::physics::VehicleProbeConfig ProbeConfigFromSceneVehicle(const SceneVehicleDefinition& vehicle)
{
    engine::physics::VehicleProbeConfig config;
    config.vehicleId = vehicle.id;
    config.spawnPosition = vehicle.spawnPosition;
    config.spawnYawRadians = vehicle.spawnYawRadians;
    config.halfExtents = vehicle.proxyHalfExtents;
    config.boundsMin = vehicle.boundsMin;
    config.boundsMax = vehicle.boundsMax;
    return config;
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

bool WriteReport(const FerryOfficeVehicleRuntimeComparisonResult& result)
{
    if (result.reportPath.empty()) {
        return true;
    }

    const std::filesystem::path parent = result.reportPath.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    nlohmann::json deterministicSamples = nlohmann::json::array();
    for (const engine::physics::VehicleProbeSample& sample : result.deterministicSamples) {
        deterministicSamples.push_back(SampleJson(sample));
    }

    nlohmann::json adapterSamples = nlohmann::json::array();
    for (const engine::physics::VehicleProbeSample& sample : result.adapterSamples) {
        adapterSamples.push_back(SampleJson(sample));
    }

    const nlohmann::json report = {
        {"schema", "v0.36-ferry-office-vehicle-runtime-comparison"},
        {"scenario", result.scenario},
        {"passed", result.passed},
        {"scene", {{"id", result.sceneId}, {"path", result.scenePath.generic_string()}}},
        {"vehicle", {{"id", result.vehicleId}}},
        {"inputFrameCount", result.inputFrameCount},
        {"deterministic", {
            {"backend", "deterministic"},
            {"samples", deterministicSamples},
        }},
        {"adapter", {
            {"backend", result.adapterBackendName},
            {"samples", adapterSamples},
        }},
        {"comparison", {
            {"maxPositionDelta", result.maxPositionDelta},
            {"maxYawDeltaDegrees", result.maxYawDeltaDegrees},
            {"maxSpeedDelta", result.maxSpeedDelta},
            {"recommendation", result.recommendation},
            {"recommendationReason", result.recommendationReason},
        }},
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

std::filesystem::path DefaultFerryOfficeVehicleRuntimeComparisonReportPath()
{
    return std::filesystem::path("build") / "physics" / "ferry-office-vehicle-runtime-comparison-report.json";
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

    const engine::physics::VehicleProbeResult probe =
        engine::physics::RunVehicleFeasibilityProbe(
            ProbeConfigFromSceneVehicle(*vehicle),
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

FerryOfficeVehicleRuntimeComparisonResult RunFerryOfficeVehicleRuntimeComparisonQa(
    const std::filesystem::path& scenePath,
    const std::filesystem::path& requestedReportPath,
    engine::physics::PhysicsBackend backend)
{
    FerryOfficeVehicleRuntimeComparisonResult result;
    result.scenario = std::string(RuntimeComparisonScenarioName);
    result.adapterBackendName = "unavailable";
    result.scenePath = scenePath;
    result.reportPath = requestedReportPath.empty() ? DefaultFerryOfficeVehicleRuntimeComparisonReportPath() : requestedReportPath;

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
    result.vehicleId = vehicle->id;

    std::unique_ptr<engine::physics::IVehicleRuntimeAdapter> adapter =
        engine::physics::CreateVehicleRuntimeAdapter(backend);
    if (!adapter) {
        result.error = "Requested vehicle runtime backend is unavailable. Use the opt-in physics configure/build preset for runtime comparison QA.";
        WriteReport(result);
        return result;
    }

    const engine::physics::VehicleRuntimeConfig config = RuntimeConfigFromSceneVehicle(*vehicle);
    if (!adapter->initialize(config)) {
        result.adapterBackendName = std::string(adapter->backendName());
        result.error = std::string(adapter->error());
        if (result.error.empty()) {
            result.error = "Failed to initialize vehicle runtime adapter.";
        }
        WriteReport(result);
        return result;
    }
    result.adapterBackendName = std::string(adapter->backendName());

    VehicleController deterministic = BuildDeterministicVehicle(*vehicle);
    bool adapterStable = true;
    bool deterministicStable = true;
    const std::vector<engine::physics::VehicleProbeInputFrame> script =
        engine::physics::DefaultVehicleFeasibilityInputScript();

    for (const engine::physics::VehicleProbeInputFrame& segment : script) {
        for (int frame = 0; frame < segment.frameCount; ++frame) {
            deterministic.beginFrame();
            deterministic.updateDriving(config.fixedStepSeconds, RuntimeInputForDeterministicController(segment));
            adapterStable = adapterStable && adapter->step(RuntimeInputForAdapter(segment), config.fixedStepSeconds);
            result.inputFrameCount += 1;
        }

        const VehicleState& deterministicState = deterministic.state();
        engine::physics::VehicleRuntimeState deterministicRuntimeState;
        deterministicRuntimeState.position = deterministicState.position;
        deterministicRuntimeState.yawRadians = deterministicState.yawRadians;
        deterministicRuntimeState.speed = std::abs(deterministicState.speed);
        deterministicRuntimeState.wheelContactCount = 4;
        deterministicRuntimeState.outOfBounds = deterministicState.hitBoundsThisFrame;
        deterministicRuntimeState.frameIndex = result.inputFrameCount;

        const engine::physics::VehicleRuntimeState adapterState = adapter->state();
        result.deterministicSamples.push_back(SampleFromRuntimeState(
            segment.name,
            deterministicRuntimeState,
            "Deterministic live VehicleController fallback sample."));
        result.adapterSamples.push_back(SampleFromRuntimeState(
            segment.name,
            adapterState,
            "Runtime adapter sample from selected physics backend."));

        result.maxPositionDelta = std::max(result.maxPositionDelta, HorizontalDistance(deterministicRuntimeState.position, adapterState.position));
        result.maxYawDeltaDegrees = std::max(result.maxYawDeltaDegrees, AbsYawDeltaDegrees(deterministicRuntimeState.yawRadians, adapterState.yawRadians));
        result.maxSpeedDelta = std::max(result.maxSpeedDelta, std::abs(std::abs(deterministicRuntimeState.speed) - std::abs(adapterState.speed)));
        deterministicStable = deterministicStable && !deterministicRuntimeState.outOfBounds;
        adapterStable = adapterStable && !adapterState.outOfBounds && adapterState.wheelContactCount >= 2;
    }

    adapter->shutdown();

    const bool closeEnough = result.maxPositionDelta <= RuntimePositionDeltaLimit
        && result.maxYawDeltaDegrees <= RuntimeYawDeltaLimitDegrees
        && result.maxSpeedDelta <= RuntimeSpeedDeltaLimit;
    result.passed = deterministicStable && adapterStable && closeEnough && !result.deterministicSamples.empty();
    result.recommendation = result.passed ? "promote" : "defer";
    result.recommendationReason = result.passed
        ? "The selected vehicle runtime adapter stayed stable and close enough to compare behind a live switch."
        : "The selected vehicle runtime adapter needs more work before live switch promotion.";
    if (!result.passed) {
        result.error = "Vehicle runtime comparison did not meet stability or comparison thresholds.";
    }

    if (!WriteReport(result)) {
        result.passed = false;
        result.error = "Failed to write vehicle runtime comparison report: " + result.reportPath.string();
    }
    return result;
}
