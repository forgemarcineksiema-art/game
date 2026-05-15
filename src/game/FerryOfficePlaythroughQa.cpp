#include "game/FerryOfficePlaythroughQa.h"

#include "engine/input/Input.h"
#include "game/FerryOfficeData.h"
#include "game/InteractionSystem.h"
#include "game/PlayerController.h"
#include "game/PrototypeScene.h"
#include "game/SceneLoader.h"
#include "game/TraversalSystem.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <nlohmann/json.hpp>
#include <string_view>
#include <utility>

namespace {

constexpr std::string_view ScenarioName = "ferry-office-service-call";

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
    const std::filesystem::path& requestedReportPath)
{
    FerryOfficePlaythroughQaResult result;
    result.scenario = std::string(ScenarioName);
    result.scenePath = scenePath;
    result.reportPath = requestedReportPath.empty() ? DefaultFerryOfficePlaythroughQaReportPath() : requestedReportPath;

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

    actionOk = scene.recordServiceVehicleUsed();
    RecordActionStep(result,
        scene,
        "serviceVehicle",
        {WorldFlag::ServiceVehicleUsed},
        actionOk,
        actionOk ? "Service vehicle use recorded." : "Service vehicle use was not recorded.");

    actionOk = scene.updateJobVehicleCheckpoint(scene.job().config().vehicleCheckpointPosition, true);
    RecordActionStep(result,
        scene,
        "dockRoadCheckpoint",
        {WorldFlag::DockRoadReached},
        actionOk,
        actionOk ? "Dock-road checkpoint reached." : "Dock-road checkpoint was not recorded.");

    actionOk = TriggerInteraction(scene, FerryOffice::Names::ServiceRunMarker, message);
    RecordActionStep(result,
        scene,
        "confirmServiceRun",
        {WorldFlag::ServiceRunConfirmed, WorldFlag::FerryOfficeJobComplete},
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
