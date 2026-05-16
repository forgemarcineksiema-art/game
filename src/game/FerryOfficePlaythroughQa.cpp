#include "game/FerryOfficePlaythroughQa.h"

#include "engine/input/Input.h"
#include "game/FerryOfficeData.h"
#include "game/InteractionSystem.h"
#include "game/PlayerController.h"
#include "game/PrototypeScene.h"
#include "game/SceneLoader.h"
#include "game/VehicleController.h"
#include "game/TraversalSystem.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <memory>
#include <nlohmann/json.hpp>
#include <string_view>
#include <utility>

namespace {

constexpr std::string_view ScenarioName = "ferry-office-service-call";
constexpr std::string_view ServiceVehicleId = "service-yard-vehicle";
constexpr int RuntimeRouteBudgetFrames = 240;
constexpr float RuntimeRouteThrottle = 0.72f;
constexpr float RuntimeFixedStepSeconds = 1.0f / 60.0f;

constexpr std::array RequiredFlags = {
    WorldFlag::ManifestCollected,
    WorldFlag::ServiceRouteUsed,
    WorldFlag::MaintenanceBoxInspected,
    WorldFlag::PowerRestored,
    WorldFlag::RouteOpened,
    WorldFlag::ServiceVehicleUsed,
    WorldFlag::DockRoadReached,
    WorldFlag::ServiceRunConfirmed,
    WorldFlag::FerryOfficeJobComplete,
    WorldFlag::DockRoadRelayReset,
    WorldFlag::DockRoadRelayLogged,
    WorldFlag::DockRoadClearanceTagged,
};

const Interactable* FindInteractableByName(const PrototypeScene& scene, std::string_view name)
{
    for (const Interactable& interactable : scene.interactions().interactables()) {
        if (interactable.name == name) {
            return &interactable;
        }
    }
    return nullptr;
}

const TraversalAffordance* FindTraversalByName(const PrototypeScene& scene, std::string_view name)
{
    for (const TraversalAffordance& affordance : scene.traversal().affordances()) {
        if (affordance.name == name) {
            return &affordance;
        }
    }
    return nullptr;
}

bool TriggerInteraction(PrototypeScene& scene, std::string_view name, std::string& message)
{
    const Interactable* target = FindInteractableByName(scene, name);
    if (!target) {
        message = "Missing interactable: " + std::string(name);
        return false;
    }

    scene.interactions().updateFocus(target->position, {0.0f, 0.0f, 1.0f});
    engine::InputState input;
    input.interactPressed = true;
    const InteractionResult interaction = scene.interactions().interact(input);
    if (!interaction.triggered || interaction.name != name) {
        message = "Interaction did not trigger expected target: " + std::string(name);
        return false;
    }

    scene.applyInteractionResult(interaction);
    message = interaction.message;
    return true;
}

bool TriggerServiceTraversal(PrototypeScene& scene, std::string& message)
{
    const TraversalAffordance* affordance = FindTraversalByName(scene, FerryOffice::Names::ServiceVault);
    if (!affordance) {
        message = "Missing traversal affordance: " + std::string(FerryOffice::Names::ServiceVault);
        return false;
    }

    PlayerController player;
    player.setWorld(&scene.world());
    player.setPosition(affordance->startPosition);
    scene.traversal().updateFocus(affordance->startPosition, affordance->requiredFacingDirection);

    engine::InputState input;
    input.jumpPressed = true;
    TraversalActivation activation = scene.traversal().activationFromInput(input);
    if (!activation.started || activation.name != FerryOffice::Names::ServiceVault) {
        message = "Traversal input did not activate the service vault.";
        return false;
    }

    player.update(0.0f, input, 0.0f, &activation);
    input.jumpPressed = false;
    activation = {};
    for (int index = 0; index < 20 && player.state().traversalMode != PlayerTraversalMode::Normal; ++index) {
        player.update(0.05f, input, 0.0f, &activation);
    }

    if (player.state().traversalMode != PlayerTraversalMode::Normal) {
        message = "Service vault traversal did not complete within the QA budget.";
        return false;
    }

    scene.recordServiceRouteUsed();
    message = "Service Barrier Vault completed.";
    return true;
}

bool AllFlagsSet(const WorldState& state, std::initializer_list<WorldFlag> flags)
{
    for (WorldFlag flag : flags) {
        if (!state.isFlagSet(flag)) {
            return false;
        }
    }
    return true;
}

VehicleController BuildRuntimeVehicle(const SceneVehicleDefinition& vehicle)
{
    VehicleController controller;
    VehicleControllerSettings settings;
    settings.enterRadius = vehicle.enterRadius;
    settings.boundsMinX = vehicle.boundsMin.x;
    settings.boundsMinZ = vehicle.boundsMin.y;
    settings.boundsMaxX = vehicle.boundsMax.x;
    settings.boundsMaxZ = vehicle.boundsMax.y;
    controller.setSettings(settings);
    controller.setPosition(vehicle.spawnPosition);
    controller.setYawRadians(vehicle.spawnYawRadians);
    return controller;
}

bool IsRuntimeExitPositionClear(const PrototypeScene& scene, const PlayerController& player, const VehicleController& vehicle)
{
    const engine::Vec3 exitPosition = vehicle.exitPosition();
    const VehicleControllerSettings& settings = vehicle.settings();
    if (exitPosition.x < settings.boundsMinX || exitPosition.x > settings.boundsMaxX
        || exitPosition.z < settings.boundsMinZ || exitPosition.z > settings.boundsMaxZ) {
        return false;
    }

    for (const StaticCollider& collider : scene.world().colliders()) {
        if (!collider.blocksPlayer) {
            continue;
        }
        if (scene.world().playerOverlapsCollider(
                exitPosition,
                player.settings().radius,
                player.settings().height,
                collider)) {
            return false;
        }
    }

    return true;
}

struct RuntimeVehicleLoopResult {
    bool vehicleEntered = false;
    bool checkpointReached = false;
    bool exitClear = false;
    bool vehicleExited = false;
    bool hitBounds = false;
    bool adapterStepFailed = false;
    bool adapterUnavailable = false;
    int framesToCheckpoint = -1;
    engine::Vec3 finalVehiclePosition;
    float finalVehicleYawRadians = 0.0f;
    std::string backend = "deterministic";
    std::string error;
};

void AddStep(FerryOfficePlaythroughQaResult& result, PrototypeScene& scene, FerryOfficePlaythroughQaStep step)
{
    step.phase = std::string(FerryOfficeJobPhaseName(scene.job().phase(scene.worldState())));
    result.steps.push_back(std::move(step));
}

void RecordActionStep(
    FerryOfficePlaythroughQaResult& result,
    PrototypeScene& scene,
    std::string name,
    std::initializer_list<WorldFlag> requiredFlags,
    bool actionOk,
    std::string message)
{
    const bool flagsOk = AllFlagsSet(scene.worldState(), requiredFlags);
    FerryOfficePlaythroughQaStep step;
    step.name = std::move(name);
    step.passed = actionOk && flagsOk;
    step.message = std::move(message);
    if (actionOk && !flagsOk) {
        step.message = "Action ran, but required state flags were not all set.";
    }
    AddStep(result, scene, std::move(step));
}

nlohmann::json FlagsJson(const WorldState& state)
{
    nlohmann::json flags = nlohmann::json::object();
    for (WorldFlag flag : RequiredFlags) {
        flags[std::string(WorldFlagName(flag))] = state.isFlagSet(flag);
    }
    flags[std::string(WorldFlagName(WorldFlag::FerryOfficeJobStarted))] =
        state.isFlagSet(WorldFlag::FerryOfficeJobStarted);
    return flags;
}

nlohmann::json EventsJson(const WorldState& state)
{
    nlohmann::json events = nlohmann::json::array();
    for (const WorldEvent& event : state.events()) {
        events.push_back({
            {"id", event.id},
            {"name", event.name},
            {"flag", std::string(WorldFlagName(event.flag))},
            {"value", event.value},
            {"source", event.source},
        });
    }
    return events;
}

engine::physics::VehicleRuntimeConfig BuildRuntimeAdapterConfig(
    const SceneVehicleDefinition& vehicle,
    const VehicleController& controller)
{
    const VehicleControllerSettings& settings = controller.settings();
    engine::physics::VehicleRuntimeConfig config;
    config.vehicleId = vehicle.id;
    config.spawnPosition = controller.state().position;
    config.spawnYawRadians = controller.state().yawRadians;
    config.halfExtents = vehicle.proxyHalfExtents;
    config.boundsMin = {settings.boundsMinX, settings.boundsMinZ};
    config.boundsMax = {settings.boundsMaxX, settings.boundsMaxZ};
    return config;
}

bool WriteReport(const FerryOfficePlaythroughQaResult& result)
{
    if (result.reportPath.empty()) {
        return true;
    }

    const std::filesystem::path parent = result.reportPath.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    nlohmann::json steps = nlohmann::json::array();
    for (const FerryOfficePlaythroughQaStep& step : result.steps) {
        steps.push_back({
            {"name", step.name},
            {"passed", step.passed},
            {"phase", step.phase},
            {"message", step.message},
        });
    }

    const WorldState& state = result.finalWorldState;
    const nlohmann::json report = {
        {"schema", "v0.32-ferry-office-playthrough-qa"},
        {"scenario", result.scenario},
        {"passed", result.passed},
        {"scene", {{"id", result.sceneId}, {"path", result.scenePath.generic_string()}}},
        {"vehicleRuntime",
            {
                {"requested", result.requestedVehicleRuntime},
                {"backend", result.vehicleRuntimeBackend},
                {"fallbackUsed", result.vehicleRuntimeFallbackUsed},
                {"framesToCheckpoint", result.vehicleRuntimeFramesToCheckpoint},
                {"hitBounds", result.vehicleRuntimeHitBounds},
                {"finalPosition",
                    {
                        {"x", result.vehicleRuntimeFinalPosition.x},
                        {"y", result.vehicleRuntimeFinalPosition.y},
                        {"z", result.vehicleRuntimeFinalPosition.z},
                    }},
                {"finalYawDegrees", engine::Degrees(result.vehicleRuntimeFinalYawRadians)},
            }},
        {"steps", steps},
        {"final",
            {
                {"phase", std::string(FerryOfficeJobPhaseName(result.finalPhase))},
                {"eventCount", state.eventCount()},
                {"flags", FlagsJson(state)},
                {"worldSummary", state.debugSummary()},
                {"events", EventsJson(state)},
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

std::filesystem::path DefaultFerryOfficePlaythroughQaReportPath()
{
    return std::filesystem::path("build") / "playthroughs" / "ferry-office-service-call-report.json";
}

FerryOfficePlaythroughQaResult RunFerryOfficeServiceCallPlaythroughQa(
    const std::filesystem::path& scenePath,
    const std::filesystem::path& requestedReportPath,
    engine::physics::PhysicsBackend vehicleRuntimeBackend,
    bool vehicleRuntimeAdapterEnabled)
{
    FerryOfficePlaythroughQaResult result;
    result.scenario = std::string(ScenarioName);
    result.scenePath = scenePath;
    result.reportPath = requestedReportPath.empty() ? DefaultFerryOfficePlaythroughQaReportPath() : requestedReportPath;
    result.requestedVehicleRuntime =
        std::string(engine::physics::VehicleRuntimeRequestName(vehicleRuntimeBackend, vehicleRuntimeAdapterEnabled));

    const SceneLoadResult loadedScene = LoadSceneDefinition(scenePath);
    if (!loadedScene.ok()) {
        result.error = loadedScene.error;
        WriteReport(result);
        return result;
    }

    result.sceneId = loadedScene.scene.id;
    PrototypeScene scene(loadedScene.scene);

    std::string message;
    bool actionOk = TriggerInteraction(scene, FerryOffice::Names::FerryManifest, message);
    RecordActionStep(result,
        scene,
        "collectManifest",
        {WorldFlag::FerryOfficeJobStarted, WorldFlag::ManifestCollected},
        actionOk,
        message);

    actionOk = TriggerServiceTraversal(scene, message);
    RecordActionStep(result, scene, "serviceRoute", {WorldFlag::ServiceRouteUsed}, actionOk, message);

    actionOk = TriggerInteraction(scene, FerryOffice::Names::MaintenanceBox, message);
    RecordActionStep(result,
        scene,
        "maintenancePower",
        {WorldFlag::MaintenanceBoxInspected, WorldFlag::PowerRestored},
        actionOk,
        message);

    actionOk = TriggerInteraction(scene, FerryOffice::Names::WallButton, message);
    RecordActionStep(result, scene, "openServiceGate", {WorldFlag::RouteOpened}, actionOk, message);

    const SceneVehicleDefinition* serviceVehicle = FindSceneVehicleById(loadedScene.scene, std::string(ServiceVehicleId));
    RuntimeVehicleLoopResult runtimeVehicle;
    PlayerController runtimePlayer;
    runtimePlayer.setWorld(&scene.world());
    VehicleController runtimeVehicleController;
    std::unique_ptr<engine::physics::IVehicleRuntimeAdapter> runtimeAdapter;
    if (serviceVehicle) {
        runtimeVehicleController = BuildRuntimeVehicle(*serviceVehicle);
        runtimeVehicle.finalVehiclePosition = runtimeVehicleController.state().position;
        runtimeVehicle.finalVehicleYawRadians = runtimeVehicleController.state().yawRadians;

        const engine::Vec3 entryPosition = runtimeVehicleController.state().position - runtimeVehicleController.forward();
        const engine::Vec3 entryFacing = engine::Normalize(runtimeVehicleController.state().position - entryPosition);
        runtimeVehicleController.beginFrame();
        runtimeVehicleController.updateFocus(entryPosition, entryFacing);
        engine::InputState enterInput;
        enterInput.interactPressed = true;
        runtimeVehicle.vehicleEntered = runtimeVehicleController.tryEnter(enterInput);
        if (runtimeVehicle.vehicleEntered) {
            scene.recordServiceVehicleUsed();
        }

        if (runtimeVehicle.vehicleEntered && vehicleRuntimeAdapterEnabled) {
            runtimeAdapter = engine::physics::CreateVehicleRuntimeAdapter(vehicleRuntimeBackend);
            if (!runtimeAdapter) {
                runtimeVehicle.adapterUnavailable = true;
                runtimeVehicle.backend = "unavailable";
                runtimeVehicle.error = "Requested vehicle runtime adapter is unavailable.";
            } else {
                if (runtimeAdapter->initialize(BuildRuntimeAdapterConfig(*serviceVehicle, runtimeVehicleController))) {
                    runtimeVehicle.backend = std::string(runtimeAdapter->backendName());
                } else {
                    runtimeVehicle.adapterUnavailable = true;
                    runtimeVehicle.backend = std::string(runtimeAdapter->backendName());
                    runtimeVehicle.error = std::string(runtimeAdapter->error());
                    if (runtimeVehicle.error.empty()) {
                        runtimeVehicle.error = "Requested vehicle runtime adapter failed to initialize.";
                    }
                    runtimeAdapter.reset();
                }
            }
        }
    }

    actionOk = serviceVehicle != nullptr && runtimeVehicle.vehicleEntered;
    RecordActionStep(result,
        scene,
        "serviceVehicleRuntime",
        {WorldFlag::ServiceVehicleUsed},
        actionOk,
        actionOk ? "Runtime input entered the service vehicle." : "Runtime input did not enter the service vehicle.");

    if (serviceVehicle && runtimeVehicle.vehicleEntered && !runtimeVehicle.adapterUnavailable) {
        for (int frame = 0; frame < RuntimeRouteBudgetFrames; ++frame) {
            runtimeVehicleController.beginFrame();
            engine::InputState driveInput;
            driveInput.moveForward = RuntimeRouteThrottle;

            if (runtimeAdapter) {
                engine::physics::VehicleRuntimeInput adapterInput;
                adapterInput.throttle = RuntimeRouteThrottle;
                if (!runtimeAdapter->step(adapterInput, RuntimeFixedStepSeconds)) {
                    runtimeVehicle.adapterStepFailed = true;
                    runtimeVehicle.error = std::string(runtimeAdapter->error());
                    if (runtimeVehicle.error.empty()) {
                        runtimeVehicle.error = "Requested vehicle runtime adapter failed during the route step.";
                    }
                    break;
                }
                const engine::physics::VehicleRuntimeState adapterState = runtimeAdapter->state();
                runtimeVehicleController.applyRuntimeState(
                    adapterState.position,
                    adapterState.yawRadians,
                    adapterState.speed,
                    adapterInput.throttle,
                    adapterInput.brake,
                    adapterInput.steer,
                    adapterState.outOfBounds);
            } else {
                runtimeVehicleController.updateDriving(RuntimeFixedStepSeconds, driveInput);
            }
            runtimeVehicle.hitBounds = runtimeVehicle.hitBounds || runtimeVehicleController.state().hitBoundsThisFrame;
            runtimeVehicle.finalVehiclePosition = runtimeVehicleController.state().position;
            runtimeVehicle.finalVehicleYawRadians = runtimeVehicleController.state().yawRadians;
            if (scene.updateJobVehicleCheckpoint(
                    runtimeVehicleController.state().position,
                    runtimeVehicleController.state().occupied)) {
                runtimeVehicle.checkpointReached = true;
                runtimeVehicle.framesToCheckpoint = frame + 1;
                break;
            }
        }
    }

    result.vehicleRuntimeBackend = runtimeVehicle.backend;
    result.vehicleRuntimeFallbackUsed = vehicleRuntimeAdapterEnabled && runtimeVehicle.backend != result.requestedVehicleRuntime;
    result.vehicleRuntimeHitBounds = runtimeVehicle.hitBounds;
    result.vehicleRuntimeFramesToCheckpoint = runtimeVehicle.framesToCheckpoint;
    result.vehicleRuntimeFinalPosition = runtimeVehicle.finalVehiclePosition;
    result.vehicleRuntimeFinalYawRadians = runtimeVehicle.finalVehicleYawRadians;

    actionOk = serviceVehicle != nullptr
        && runtimeVehicle.checkpointReached
        && !runtimeVehicle.hitBounds
        && !runtimeVehicle.adapterUnavailable
        && !runtimeVehicle.adapterStepFailed;
    RecordActionStep(result,
        scene,
        "dockRoadRuntimeCheckpoint",
        {WorldFlag::DockRoadReached},
        actionOk,
        actionOk
            ? "Runtime " + runtimeVehicle.backend + " vehicle reached the dock-road checkpoint in " + std::to_string(runtimeVehicle.framesToCheckpoint) + " frames."
            : "Runtime vehicle did not reach the dock-road checkpoint cleanly. " + runtimeVehicle.error);

    if (serviceVehicle && runtimeVehicle.vehicleEntered) {
        runtimeVehicleController.beginFrame();
        runtimeVehicle.exitClear = IsRuntimeExitPositionClear(scene, runtimePlayer, runtimeVehicleController);
        engine::InputState exitInput;
        exitInput.interactPressed = true;
        runtimeVehicle.vehicleExited = runtimeVehicleController.tryExit(exitInput, runtimeVehicle.exitClear);
        if (runtimeVehicle.vehicleExited) {
            runtimePlayer.setPosition(runtimeVehicleController.exitPosition());
        }
    }

    actionOk = serviceVehicle != nullptr && runtimeVehicle.exitClear && runtimeVehicle.vehicleExited;
    RecordActionStep(result,
        scene,
        "serviceVehicleRuntimeExit",
        {},
        actionOk,
        actionOk ? "Runtime input exited the service vehicle at a clear position." : "Runtime vehicle exit was blocked or did not trigger.");

    actionOk = TriggerInteraction(scene, FerryOffice::Names::ServiceRunMarker, message);
    RecordActionStep(result,
        scene,
        "confirmServiceRun",
        {WorldFlag::ServiceRunConfirmed, WorldFlag::FerryOfficeJobComplete},
        actionOk,
        message);

    actionOk = TriggerInteraction(scene, FerryOffice::Names::DockRoadRelay, message);
    RecordActionStep(result,
        scene,
        "dockRoadRelayReset",
        {WorldFlag::DockRoadRelayReset},
        actionOk,
        message);

    actionOk = TriggerInteraction(scene, FerryOffice::Names::RelayServiceLog, message);
    RecordActionStep(result,
        scene,
        "dockRoadRelayLogged",
        {WorldFlag::DockRoadRelayLogged},
        actionOk,
        message);

    actionOk = TriggerInteraction(scene, FerryOffice::Names::DockRoadClearanceTag, message);
    RecordActionStep(result,
        scene,
        "dockRoadClearanceTagged",
        {WorldFlag::DockRoadClearanceTagged},
        actionOk,
        message);

    result.finalWorldState = scene.worldState();
    result.finalPhase = scene.job().phase(scene.worldState());
    result.passed = result.finalPhase == FerryOfficeJobPhase::Complete;
    for (const FerryOfficePlaythroughQaStep& step : result.steps) {
        result.passed = result.passed && step.passed;
    }
    for (WorldFlag flag : RequiredFlags) {
        result.passed = result.passed && result.finalWorldState.isFlagSet(flag);
    }
    if (!result.passed && result.error.empty()) {
        result.error = "Ferry Office playthrough QA did not reach the complete state.";
    }
    if (!WriteReport(result)) {
        result.passed = false;
        result.error = "Failed to write playthrough QA report: " + result.reportPath.string();
    }

    return result;
}
