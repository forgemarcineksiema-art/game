#include "game/TargetSliceObjectiveQa.h"

#include "engine/input/Input.h"
#include "game/InteractionSystem.h"
#include "game/PlayerController.h"
#include "game/PrototypeScene.h"
#include "game/SceneDefinition.h"
#include "game/SceneLoader.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string_view>
#include <vector>

namespace {

constexpr std::string_view ScenarioName = "veyra-target-objective-acquisition";
constexpr std::string_view InputScriptName = "recorded-veyra-target-objective-v1";
constexpr std::string_view SchemaName = "v0.99-target-slice-objective-acquisition-qa";
constexpr float FixedStepSeconds = 1.0f / 60.0f;
constexpr int ContactBudgetFrames = 140;
constexpr int FocusBudgetFrames = 240;
constexpr float WaypointArrivalDistance = 0.22f;
constexpr float RecoveryDistance = 0.35f;
constexpr std::string_view ExpectedSceneId = "veyra-reach-pilot";
constexpr std::string_view ContactColliderName = "pilot-road-edge-collider";

const Interactable* FindInteractableByName(const PrototypeScene& scene, std::string_view name)
{
    for (const Interactable& interactable : scene.interactions().interactables()) {
        if (interactable.name == name) {
            return &interactable;
        }
    }
    return nullptr;
}

std::vector<engine::Vec3> BuildRecordedRoute(const SceneDefinition& scene, const Interactable& target)
{
    if (!scene.routeMarkers.empty() && !scene.routeMarkers.front().points.empty()) {
        return scene.routeMarkers.front().points;
    }

    return {scene.playerStart.position, target.position};
}

nlohmann::json Vec3Json(engine::Vec3 value)
{
    return {
        {"x", value.x},
        {"y", value.y},
        {"z", value.z},
    };
}

void MovePlayerToward(PlayerController& player, engine::Vec3 destination)
{
    const engine::Vec3 playerPosition = player.state().position;
    const engine::Vec3 delta = {destination.x - playerPosition.x, 0.0f, destination.z - playerPosition.z};
    const engine::Vec3 direction = engine::Normalize(delta);
    engine::InputState input;
    input.moveForward = engine::Length(direction) > 0.0f ? 1.0f : 0.0f;
    const float cameraYawRadians = input.moveForward > 0.0f
        ? engine::YawFromDirection(direction)
        : player.state().facingYawRadians;
    player.update(FixedStepSeconds, input, cameraYawRadians, nullptr);
}

bool WriteReport(const TargetSliceObjectiveQaResult& result)
{
    if (result.reportPath.empty()) {
        return true;
    }

    const std::filesystem::path parent = result.reportPath.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    const nlohmann::json report = {
        {"schema", std::string(SchemaName)},
        {"scenario", result.scenario},
        {"passed", result.passed},
        {"scene", {{"id", result.sceneId}, {"path", result.scenePath.generic_string()}}},
        {"input",
            {
                {"scriptName", result.inputScriptName},
                {"framesToFocus", result.framesToFocus},
                {"framesToInteract", result.framesToInteract},
            }},
        {"contact",
            {
                {"attempted", result.contactAttempted},
                {"hit", result.contactHit},
                {"recoveredControl", result.contactRecoveredControl},
                {"colliderName", result.contactColliderName},
                {"framesToContact", result.framesToContact},
                {"framesToRecovery", result.framesToRecovery},
                {"hitCount", result.contactHitCount},
                {"position", Vec3Json(result.contactPosition)},
                {"push", Vec3Json(result.contactPush)},
                {"normal", Vec3Json(result.contactNormal)},
            }},
        {"focus",
            {
                {"acquired", result.focusAcquired},
                {"name", result.focusName},
                {"prompt", result.focusPrompt},
                {"distance", result.focusDistance},
            }},
        {"interaction",
            {
                {"triggered", result.interactionTriggered},
                {"message", result.interactionMessage},
            }},
        {"final",
            {
                {"objectiveId", result.objectiveId},
                {"objectiveComplete", result.objectiveComplete},
                {"completionSummary", result.completionSummary},
                {"completionEventText", result.completionEventText},
                {"playerPosition", Vec3Json(result.finalPlayerPosition)},
                {"playerYawDegrees", engine::Degrees(result.finalPlayerYawRadians)},
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

void Fail(TargetSliceObjectiveQaResult& result, std::string error)
{
    result.passed = false;
    result.error = std::move(error);
}

} // namespace

std::filesystem::path DefaultTargetSliceObjectiveQaReportPath()
{
    return std::filesystem::path("build") / "playthroughs" / "veyra-target-objective-acquisition-report.json";
}

TargetSliceObjectiveQaResult RunTargetSliceObjectiveAcquisitionQa(
    const std::filesystem::path& scenePath,
    const std::filesystem::path& requestedReportPath)
{
    TargetSliceObjectiveQaResult result;
    result.scenario = std::string(ScenarioName);
    result.inputScriptName = std::string(InputScriptName);
    result.scenePath = scenePath;
    result.reportPath = requestedReportPath.empty() ? DefaultTargetSliceObjectiveQaReportPath() : requestedReportPath;

    const SceneLoadResult loadedScene = LoadSceneDefinition(scenePath);
    if (!loadedScene.ok()) {
        Fail(result, loadedScene.error);
        WriteReport(result);
        return result;
    }

    result.sceneId = loadedScene.scene.id;
    result.objectiveId = loadedScene.scene.targetObjective.id;
    if (loadedScene.scene.id != ExpectedSceneId || !IsTargetSliceScaffoldScene(loadedScene.scene)) {
        Fail(result, "Target-slice objective QA requires the Veyra target-slice scaffold scene.");
        WriteReport(result);
        return result;
    }
    if (loadedScene.scene.targetObjective.completionInteractableName.empty()) {
        Fail(result, "Target-slice objective scene is missing completionInteractableName.");
        WriteReport(result);
        return result;
    }

    PrototypeScene scene(loadedScene.scene);
    const Interactable* target = FindInteractableByName(scene, loadedScene.scene.targetObjective.completionInteractableName);
    if (!target) {
        Fail(result, "Target-slice objective completion interactable was not found.");
        WriteReport(result);
        return result;
    }
    const StaticCollider* contactCollider = scene.world().colliderByName(ContactColliderName);
    if (!contactCollider) {
        Fail(result, "Target-slice objective contact collider was not found.");
        WriteReport(result);
        return result;
    }

    PlayerController player;
    player.setWorld(&scene.world());
    player.setPosition(loadedScene.scene.playerStart.position);
    player.setFacingYawRadians(loadedScene.scene.playerStart.yawRadians);

    result.contactAttempted = true;
    const engine::Vec3 contactProbeDestination = {
        contactCollider->bounds.center.x + contactCollider->bounds.halfExtents.x + player.settings().radius + 0.25f,
        0.0f,
        -0.80f,
    };
    for (int frame = 0; frame < ContactBudgetFrames; ++frame) {
        MovePlayerToward(player, contactProbeDestination);
        const PlayerState& state = player.state();
        if (state.lastCollisionHitCount > 0 && state.lastCollisionColliderName == ContactColliderName) {
            result.contactHit = true;
            result.framesToContact = frame + 1;
            result.contactHitCount = state.lastCollisionHitCount;
            result.contactColliderName = state.lastCollisionColliderName;
            result.contactPosition = state.position;
            result.contactPush = state.lastCollisionPush;
            result.contactNormal = state.lastCollisionNormal;
            break;
        }
    }
    if (!result.contactHit) {
        Fail(result, "Recorded live input did not hit the authored target-slice road-edge collider.");
        result.finalPlayerPosition = player.state().position;
        result.finalPlayerYawRadians = player.state().facingYawRadians;
        WriteReport(result);
        return result;
    }

    std::vector<engine::Vec3> waypoints = BuildRecordedRoute(loadedScene.scene, *target);
    std::size_t waypointIndex = waypoints.size() > 1 ? 1 : 0;
    const int framesBeforeObjectiveRoute = result.framesToContact;

    for (int frame = 0; frame < FocusBudgetFrames; ++frame) {
        const engine::Vec3 playerPosition = player.state().position;
        const InteractionFocus focus =
            scene.interactions().updateFocus(playerPosition, engine::ForwardFromYaw(player.state().facingYawRadians));
        if (focus.hasFocus && focus.name == target->name) {
            result.focusAcquired = true;
            result.framesToFocus = framesBeforeObjectiveRoute + frame;
            result.focusName = focus.name;
            result.focusPrompt = focus.prompt;
            result.focusDistance = focus.distance;
            break;
        }

        const engine::Vec3 destination =
            waypointIndex < waypoints.size() ? waypoints[waypointIndex] : target->position;
        const engine::Vec3 delta = {destination.x - playerPosition.x, 0.0f, destination.z - playerPosition.z};
        if (engine::Length(delta) <= WaypointArrivalDistance && waypointIndex + 1 < waypoints.size()) {
            ++waypointIndex;
        }

        MovePlayerToward(player, destination);
        if (!result.contactRecoveredControl
            && engine::Length(player.state().position - result.contactPosition) > RecoveryDistance
            && player.state().lastCollisionHitCount == 0) {
            result.contactRecoveredControl = true;
            result.framesToRecovery = framesBeforeObjectiveRoute + frame + 1;
        }
    }

    if (!result.focusAcquired) {
        Fail(result, "Recorded live input did not acquire focus on the target objective interactable.");
        result.completionSummary = scene.completionSummary();
        result.completionEventText = scene.lastRuntimeEventText();
        result.finalPlayerPosition = player.state().position;
        result.finalPlayerYawRadians = player.state().facingYawRadians;
        WriteReport(result);
        return result;
    }

    scene.interactions().updateFocus(player.state().position, engine::ForwardFromYaw(player.state().facingYawRadians));
    engine::InputState interactInput;
    interactInput.interactPressed = true;
    const InteractionResult interaction = scene.interactions().interact(interactInput);
    result.interactionTriggered = interaction.triggered && interaction.name == target->name;
    result.framesToInteract = result.framesToFocus + 1;
    result.interactionMessage = interaction.message;
    if (result.interactionTriggered) {
        scene.applyInteractionResult(interaction);
    }

    result.objectiveComplete = scene.isSliceComplete();
    result.completionSummary = scene.completionSummary();
    result.completionEventText = scene.lastRuntimeEventText();
    result.finalPlayerPosition = player.state().position;
    result.finalPlayerYawRadians = player.state().facingYawRadians;
    result.passed = result.focusAcquired
        && result.contactHit
        && result.contactRecoveredControl
        && result.interactionTriggered
        && result.objectiveComplete
        && result.completionSummary.find("targetObjective=" + result.objectiveId) != std::string::npos;
    if (!result.passed && result.error.empty()) {
        Fail(result, "Target-slice objective QA did not complete through focus and interact input.");
    }
    if (!WriteReport(result)) {
        Fail(result, "Failed to write target-slice objective QA report: " + result.reportPath.string());
    }
    return result;
}
