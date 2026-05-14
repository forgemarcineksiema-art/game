#include "engine/core/Clock.h"
#include "engine/core/Config.h"
#include "engine/core/FileSystem.h"
#include "engine/math/Math.h"
#include "game/InteractionSystem.h"
#include "game/PlayerController.h"
#include "game/TestWorld.h"
#include "game/ThirdPersonCamera.h"
#include "game/TraversalSystem.h"
#include "engine/renderer/NullRenderer.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct TestFailure {
    std::string name;
    std::string message;
};

std::vector<TestFailure> failures;

void Expect(bool condition, const std::string& name, const std::string& message)
{
    if (!condition) {
        failures.push_back({name, message});
    }
}

void ExpectNear(float actual, float expected, float tolerance, const std::string& name, const std::string& message)
{
    if (std::abs(actual - expected) > tolerance) {
        failures.push_back({name, message});
    }
}

void TestSmokeArgumentsEnableBoundedHeadlessRun()
{
    const char* argv[] = {"EngineApp", "--smoke-test"};
    const auto result = engine::ParseArguments(2, argv);

    Expect(result.errors.empty(), "TestSmokeArgumentsEnableBoundedHeadlessRun", "Smoke arguments should not emit errors.");
    Expect(result.config.smokeTest, "TestSmokeArgumentsEnableBoundedHeadlessRun", "Smoke test flag should be true.");
    Expect(result.config.headless, "TestSmokeArgumentsEnableBoundedHeadlessRun", "Smoke test should default to headless.");
    Expect(result.config.maxFrames == 3, "TestSmokeArgumentsEnableBoundedHeadlessRun", "Smoke test should default to 3 frames.");
}

void TestFramesArgumentOverridesSmokeDefault()
{
    const char* argv[] = {"EngineApp", "--smoke-test", "--frames", "8"};
    const auto result = engine::ParseArguments(4, argv);

    Expect(result.errors.empty(), "TestFramesArgumentOverridesSmokeDefault", "Frames override should parse cleanly.");
    Expect(result.config.maxFrames == 8, "TestFramesArgumentOverridesSmokeDefault", "Frames should be overridden to 8.");
}

void TestInvalidRendererIsRejected()
{
    const char* argv[] = {"EngineApp", "--renderer", "pbr"};
    const auto result = engine::ParseArguments(3, argv);

    Expect(!result.errors.empty(), "TestInvalidRendererIsRejected", "Invalid renderer should emit an error.");
}

void TestNormalizePathKeepsAssetPathsInsideBase()
{
    const std::filesystem::path base = "C:/project";
    const auto normalized = engine::NormalizePath(base, "assets/../assets/grid.txt");

    Expect(normalized.generic_string() == "C:/project/assets/grid.txt",
        "TestNormalizePathKeepsAssetPathsInsideBase",
        "Normalized path should collapse parent directory segments.");
}

void TestClockStartsAtFrameZeroAndTicksForward()
{
    engine::Clock clock;
    clock.reset();
    Expect(clock.frameIndex() == 0, "TestClockStartsAtFrameZeroAndTicksForward", "Reset clock should start at frame zero.");
    const double delta = clock.tick();
    Expect(clock.frameIndex() == 1, "TestClockStartsAtFrameZeroAndTicksForward", "Tick should advance frame index.");
    Expect(delta >= 0.0, "TestClockStartsAtFrameZeroAndTicksForward", "Delta time should not be negative.");
}

void TestNullRendererRecordsFrameAndDebugDraw()
{
    engine::NullRenderer renderer;
    engine::RendererConfig config;
    config.headless = true;

    Expect(renderer.initialize(config), "TestNullRendererRecordsFrameAndDebugDraw", "Null renderer should initialize.");
    renderer.beginFrame(7);
    renderer.drawDebugGridAndAxes();
    renderer.endFrame();

    Expect(renderer.frameCount() == 7, "TestNullRendererRecordsFrameAndDebugDraw", "Null renderer should record frame index.");
    Expect(renderer.debugDrawCount() == 1, "TestNullRendererRecordsFrameAndDebugDraw", "Null renderer should count debug draw calls.");
    renderer.shutdown();
}

void TestVec3NormalizationKeepsDiagonalMovementAtUnitLength()
{
    const engine::Vec3 diagonal = engine::Normalize(engine::Vec3 {3.0f, 0.0f, 4.0f});
    ExpectNear(engine::Length(diagonal), 1.0f, 0.001f,
        "TestVec3NormalizationKeepsDiagonalMovementAtUnitLength",
        "Normalized diagonal vector should have unit length.");

    const engine::Vec3 zero = engine::Normalize(engine::Vec3 {});
    ExpectNear(engine::Length(zero), 0.0f, 0.001f,
        "TestVec3NormalizationKeepsDiagonalMovementAtUnitLength",
        "Zero vector should remain zero after normalization.");
}

void TestPlayerMovementIsCameraRelativeAndNormalized()
{
    PlayerController player;
    PlayerControllerSettings settings;
    settings.walkSpeed = 4.0f;
    settings.sprintSpeed = 8.0f;
    player.setSettings(settings);

    engine::InputState input;
    input.moveForward = 1.0f;
    input.moveRight = 1.0f;

    player.update(1.0f, input, 0.0f);

    ExpectNear(player.state().horizontalSpeed, settings.walkSpeed, 0.01f,
        "TestPlayerMovementIsCameraRelativeAndNormalized",
        "Diagonal movement should be normalized to walk speed.");
    Expect(player.state().facingYawRadians > 0.0f,
        "TestPlayerMovementIsCameraRelativeAndNormalized",
        "Facing yaw should follow the camera-relative movement direction.");
}

void TestPlayerSprintAndJumpRemainGroundedDeterministically()
{
    PlayerController player;
    PlayerControllerSettings settings;
    settings.walkSpeed = 4.0f;
    settings.sprintSpeed = 8.0f;
    settings.jumpImpulse = 5.0f;
    settings.gravity = 20.0f;
    player.setSettings(settings);

    engine::InputState input;
    input.moveForward = 1.0f;
    input.sprintHeld = true;
    input.jumpPressed = true;

    player.update(0.1f, input, 0.0f);
    Expect(player.state().position.y > 0.0f,
        "TestPlayerSprintAndJumpRemainGroundedDeterministically",
        "Jump should lift the player off the floor.");
    Expect(!player.state().grounded,
        "TestPlayerSprintAndJumpRemainGroundedDeterministically",
        "Player should not be grounded immediately after jump impulse.");

    input.jumpPressed = false;
    for (int i = 0; i < 60; ++i) {
        player.update(0.016f, input, 0.0f);
    }

    ExpectNear(player.state().position.y, 0.0f, 0.001f,
        "TestPlayerSprintAndJumpRemainGroundedDeterministically",
        "Player should land back on the floor.");
    Expect(player.state().grounded,
        "TestPlayerSprintAndJumpRemainGroundedDeterministically",
        "Player should be grounded after landing.");
}

void TestThirdPersonCameraClampsPitchAndSmooths()
{
    ThirdPersonCamera camera;
    ThirdPersonCameraSettings settings;
    settings.distance = 6.0f;
    settings.heightOffset = 1.6f;
    settings.minPitchRadians = -0.5f;
    settings.maxPitchRadians = 0.75f;
    settings.smoothing = 8.0f;
    camera.setSettings(settings);

    CameraTarget target;
    target.position = {0.0f, 0.0f, 0.0f};
    target.yawRadians = 0.0f;

    engine::InputState input;
    input.cameraPitchDelta = 100.0f;
    input.cameraYawDelta = 2.0f;

    camera.update(0.016f, input, target);
    Expect(camera.state().pitchRadians <= settings.maxPitchRadians,
        "TestThirdPersonCameraClampsPitchAndSmooths",
        "Camera pitch should clamp to the configured maximum.");
    Expect(camera.state().yawRadians > 0.0f,
        "TestThirdPersonCameraClampsPitchAndSmooths",
        "Camera yaw should respond to yaw input.");
    Expect(engine::Length(camera.state().position - target.position) > 0.1f,
        "TestThirdPersonCameraClampsPitchAndSmooths",
        "Camera should have a follow position offset from the target.");
}

void TestAabbOverlapAndClosestPoint()
{
    StaticCollider collider;
    collider.bounds.center = {0.0f, 1.0f, 0.0f};
    collider.bounds.halfExtents = {1.0f, 1.0f, 1.0f};

    Expect(collider.bounds.overlaps({0.5f, 1.0f, 0.5f}, {0.25f, 0.25f, 0.25f}),
        "TestAabbOverlapAndClosestPoint",
        "AABB should overlap a small box inside it.");
    Expect(!collider.bounds.overlaps({3.0f, 1.0f, 0.0f}, {0.25f, 0.25f, 0.25f}),
        "TestAabbOverlapAndClosestPoint",
        "AABB should not overlap a distant box.");

    const engine::Vec3 closest = collider.bounds.closestPoint({3.0f, 5.0f, -3.0f});
    ExpectNear(closest.x, 1.0f, 0.001f, "TestAabbOverlapAndClosestPoint", "Closest point should clamp X.");
    ExpectNear(closest.y, 2.0f, 0.001f, "TestAabbOverlapAndClosestPoint", "Closest point should clamp Y.");
    ExpectNear(closest.z, -1.0f, 0.001f, "TestAabbOverlapAndClosestPoint", "Closest point should clamp Z.");
}

void TestWorldGroundClampAndGroundedState()
{
    TestWorld world;
    world.setFloorHeight(0.0f);

    PlayerCollisionProxy proxy;
    proxy.position = {0.0f, -0.5f, 0.0f};
    proxy.velocity = {0.0f, -2.0f, 0.0f};
    proxy.radius = 0.35f;
    proxy.height = 1.8f;

    const CollisionResult result = world.resolvePlayer(proxy);
    ExpectNear(result.position.y, 0.0f, 0.001f,
        "TestWorldGroundClampAndGroundedState",
        "World collision should clamp player to the floor height.");
    Expect(result.grounded,
        "TestWorldGroundClampAndGroundedState",
        "Player should be grounded after floor clamp.");
    ExpectNear(result.velocity.y, 0.0f, 0.001f,
        "TestWorldGroundClampAndGroundedState",
        "Downward velocity should be removed by floor collision.");
}

void TestWorldPushesPlayerOutOfBox()
{
    TestWorld world;
    world.addBox("center-box", {0.0f, 0.5f, 0.0f}, {1.0f, 0.5f, 1.0f});

    PlayerCollisionProxy proxy;
    proxy.previousPosition = {-2.0f, 0.0f, 0.0f};
    proxy.position = {0.0f, 0.0f, 0.0f};
    proxy.velocity = {5.0f, 0.0f, 0.0f};
    proxy.radius = 0.35f;
    proxy.height = 1.8f;

    const CollisionResult result = world.resolvePlayer(proxy);
    Expect(result.hitCount > 0,
        "TestWorldPushesPlayerOutOfBox",
        "Player should report a collider hit.");
    Expect(result.position.x <= -1.34f || result.position.x >= 1.34f,
        "TestWorldPushesPlayerOutOfBox",
        "Player should be pushed outside the expanded box.");
    Expect(engine::Length(result.lastPush) > 0.0f,
        "TestWorldPushesPlayerOutOfBox",
        "Collision result should expose the push vector for debug drawing.");
}

void TestPlayerMovementUsesWorldCollisionForWall()
{
    TestWorld world;
    world.addBox("wall", {0.0f, 0.5f, 1.2f}, {3.0f, 0.5f, 0.2f});

    PlayerController player;
    player.setWorld(&world);
    player.setPosition({0.0f, 0.0f, 0.0f});

    engine::InputState input;
    input.moveForward = 1.0f;
    for (int i = 0; i < 20; ++i) {
        player.update(0.05f, input, 0.0f);
    }

    Expect(player.state().position.z <= 0.66f,
        "TestPlayerMovementUsesWorldCollisionForWall",
        "Player should not move through a simple wall collider.");
    Expect(player.state().lastCollisionHitCount > 0,
        "TestPlayerMovementUsesWorldCollisionForWall",
        "Player should record collision hits from the world.");
}

void TestDiagonalMovementIntoObstacleDoesNotTunnel()
{
    TestWorld world;
    world.addBox("corner", {1.0f, 0.5f, 1.0f}, {0.5f, 0.5f, 0.5f});

    PlayerController player;
    player.setWorld(&world);
    player.setPosition({0.0f, 0.0f, 0.0f});

    engine::InputState input;
    input.moveForward = 1.0f;
    input.moveRight = 1.0f;
    for (int i = 0; i < 30; ++i) {
        player.update(0.033f, input, 0.0f);
    }

    const StaticCollider* collider = world.colliderByName("corner");
    Expect(collider != nullptr,
        "TestDiagonalMovementIntoObstacleDoesNotTunnel",
        "Corner collider should exist.");
    if (collider) {
        Expect(!world.playerOverlapsCollider(player.state().position, player.settings().radius, player.settings().height, *collider),
            "TestDiagonalMovementIntoObstacleDoesNotTunnel",
            "Player proxy should not remain inside the corner collider after resolution.");
    }
}

void TestWorldRaycastFindsNearestCollider()
{
    TestWorld world;
    world.addBox("near", {0.0f, 0.5f, 2.0f}, {0.5f, 0.5f, 0.5f});
    world.addBox("far", {0.0f, 0.5f, 5.0f}, {0.5f, 0.5f, 0.5f});

    const RaycastHit hit = world.raycast({0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, 10.0f);
    Expect(hit.hit,
        "TestWorldRaycastFindsNearestCollider",
        "Raycast should hit a collider.");
    Expect(hit.colliderName == "near",
        "TestWorldRaycastFindsNearestCollider",
        "Raycast should return the nearest collider.");
    ExpectNear(hit.distance, 1.5f, 0.01f,
        "TestWorldRaycastFindsNearestCollider",
        "Raycast distance should land on the near box front face.");
}

void TestInteractionFocusSelectsNearestFacingCandidate()
{
    InteractionSystem interactions;

    Interactable nearPickup;
    nearPickup.name = "Near Pickup";
    nearPickup.prompt = "Pick up Near Pickup";
    nearPickup.position = {0.0f, 0.5f, 1.25f};
    nearPickup.radius = 2.0f;
    nearPickup.type = InteractableType::Pickup;
    const int nearId = interactions.addInteractable(nearPickup);

    Interactable farInfo;
    farInfo.name = "Far Info";
    farInfo.prompt = "Read Far Info";
    farInfo.position = {0.0f, 0.5f, 3.0f};
    farInfo.radius = 4.0f;
    farInfo.type = InteractableType::Info;
    interactions.addInteractable(farInfo);

    const InteractionFocus focus = interactions.updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
    Expect(focus.hasFocus,
        "TestInteractionFocusSelectsNearestFacingCandidate",
        "A nearby interactable in front of the player should receive focus.");
    Expect(focus.interactableId == nearId,
        "TestInteractionFocusSelectsNearestFacingCandidate",
        "Nearest facing interactable should win focus.");
    Expect(focus.prompt == nearPickup.prompt,
        "TestInteractionFocusSelectsNearestFacingCandidate",
        "Focus should expose prompt text for debug display.");
}

void TestInteractionFocusIgnoresDisabledAndConsumedInteractables()
{
    InteractionSystem interactions;

    Interactable disabled;
    disabled.name = "Disabled Pickup";
    disabled.prompt = "Pick up Disabled Pickup";
    disabled.position = {0.0f, 0.5f, 0.75f};
    disabled.radius = 2.0f;
    disabled.type = InteractableType::Pickup;
    disabled.enabled = false;
    interactions.addInteractable(disabled);

    Interactable consumed;
    consumed.name = "Consumed Pickup";
    consumed.prompt = "Pick up Consumed Pickup";
    consumed.position = {0.0f, 0.5f, 1.0f};
    consumed.radius = 2.0f;
    consumed.type = InteractableType::Pickup;
    consumed.oneShot = true;
    consumed.consumed = true;
    interactions.addInteractable(consumed);

    Interactable active;
    active.name = "Active Info";
    active.prompt = "Read Active Info";
    active.position = {0.0f, 0.5f, 1.25f};
    active.radius = 2.0f;
    active.type = InteractableType::Info;
    const int activeId = interactions.addInteractable(active);

    const InteractionFocus focus = interactions.updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
    Expect(focus.hasFocus,
        "TestInteractionFocusIgnoresDisabledAndConsumedInteractables",
        "An active interactable should still receive focus.");
    Expect(focus.interactableId == activeId,
        "TestInteractionFocusIgnoresDisabledAndConsumedInteractables",
        "Disabled and consumed interactables should not win focus.");
}

void TestInteractPressedTriggersExactlyOnceWhenHeld()
{
    InteractionSystem interactions;

    Interactable toggle;
    toggle.name = "Test Button";
    toggle.prompt = "Toggle Test Button";
    toggle.position = {0.0f, 0.5f, 1.0f};
    toggle.radius = 2.0f;
    toggle.type = InteractableType::Toggle;
    const int toggleId = interactions.addInteractable(toggle);
    interactions.updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});

    engine::InputState input;
    input.interactHeld = true;
    input.interactPressed = true;
    const InteractionResult first = interactions.interact(input);
    Expect(first.triggered,
        "TestInteractPressedTriggersExactlyOnceWhenHeld",
        "Pressed edge should trigger the focused interactable.");

    input.interactPressed = false;
    const InteractionResult held = interactions.interact(input);
    Expect(!held.triggered,
        "TestInteractPressedTriggersExactlyOnceWhenHeld",
        "Holding interact without a new pressed edge should not trigger again.");

    const Interactable* state = interactions.interactableById(toggleId);
    Expect(state != nullptr && state->toggled,
        "TestInteractPressedTriggersExactlyOnceWhenHeld",
        "The toggle should remain in the first triggered state.");
}

void TestOneShotPickupConsumesAndDoesNotTriggerAgain()
{
    InteractionSystem interactions;

    Interactable pickup;
    pickup.name = "One Shot Pickup";
    pickup.prompt = "Pick up One Shot Pickup";
    pickup.position = {0.0f, 0.5f, 1.0f};
    pickup.radius = 2.0f;
    pickup.type = InteractableType::Pickup;
    pickup.oneShot = true;
    const int pickupId = interactions.addInteractable(pickup);
    interactions.updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});

    engine::InputState input;
    input.interactPressed = true;
    const InteractionResult first = interactions.interact(input);
    const Interactable* state = interactions.interactableById(pickupId);
    Expect(first.triggered && state != nullptr && state->consumed && !state->enabled,
        "TestOneShotPickupConsumesAndDoesNotTriggerAgain",
        "A one-shot pickup should be consumed and disabled after the first interaction.");

    const InteractionFocus focusAfterPickup = interactions.updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
    const InteractionResult second = interactions.interact(input);
    Expect(!focusAfterPickup.hasFocus && !second.triggered,
        "TestOneShotPickupConsumesAndDoesNotTriggerAgain",
        "A consumed pickup should not regain focus or trigger again.");
}

void TestToggleInteractableChangesStateOnEachPress()
{
    InteractionSystem interactions;

    Interactable toggle;
    toggle.name = "Door Button";
    toggle.prompt = "Toggle Door Button";
    toggle.position = {0.0f, 0.5f, 1.0f};
    toggle.radius = 2.0f;
    toggle.type = InteractableType::Toggle;
    const int toggleId = interactions.addInteractable(toggle);
    interactions.updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});

    engine::InputState input;
    input.interactPressed = true;
    const InteractionResult opened = interactions.interact(input);
    const Interactable* state = interactions.interactableById(toggleId);
    Expect(opened.triggered && state != nullptr && state->toggled,
        "TestToggleInteractableChangesStateOnEachPress",
        "First press should toggle the interactable on.");

    const InteractionResult closed = interactions.interact(input);
    state = interactions.interactableById(toggleId);
    Expect(closed.triggered && state != nullptr && !state->toggled,
        "TestToggleInteractableChangesStateOnEachPress",
        "A later pressed edge should toggle the interactable off.");
}

void TestNoFocusedInteractableMeansNoAction()
{
    InteractionSystem interactions;

    Interactable info;
    info.name = "Distant Info";
    info.prompt = "Read Distant Info";
    info.position = {0.0f, 0.5f, 10.0f};
    info.radius = 1.0f;
    info.type = InteractableType::Info;
    interactions.addInteractable(info);
    interactions.updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});

    engine::InputState input;
    input.interactPressed = true;
    const InteractionResult result = interactions.interact(input);
    Expect(!result.triggered,
        "TestNoFocusedInteractableMeansNoAction",
        "Interact should no-op when there is no focused candidate.");
}

TraversalAffordance MakeTestTraversalAffordance()
{
    TraversalAffordance affordance;
    affordance.name = "Test Vault";
    affordance.prompt = "Press Space: Vault Test Blocker";
    affordance.type = TraversalType::Vault;
    affordance.startPosition = {0.0f, 0.0f, 1.0f};
    affordance.endPosition = {0.0f, 0.0f, 2.25f};
    affordance.focusRadius = 1.5f;
    affordance.requiredFacingDirection = {0.0f, 0.0f, 1.0f};
    affordance.requiredFacingDot = 0.25f;
    affordance.durationSeconds = 0.25f;
    return affordance;
}

void TestTraversalFocusSelectsAvailableAffordance()
{
    TraversalSystem traversal;
    const int id = traversal.addAffordance(MakeTestTraversalAffordance());

    const TraversalFocus focus = traversal.updateFocus({0.0f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f});
    Expect(focus.hasFocus,
        "TestTraversalFocusSelectsAvailableAffordance",
        "A nearby traversal affordance in front of the player should receive focus.");
    Expect(focus.affordanceId == id,
        "TestTraversalFocusSelectsAvailableAffordance",
        "Traversal focus should expose the selected affordance id.");
    Expect(focus.prompt == "Press Space: Vault Test Blocker",
        "TestTraversalFocusSelectsAvailableAffordance",
        "Traversal focus should expose the prompt text.");
}

void TestTraversalFocusIgnoresDisabledAffordance()
{
    TraversalSystem traversal;
    TraversalAffordance disabled = MakeTestTraversalAffordance();
    disabled.enabled = false;
    traversal.addAffordance(disabled);

    const TraversalFocus focus = traversal.updateFocus({0.0f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f});
    Expect(!focus.hasFocus,
        "TestTraversalFocusIgnoresDisabledAffordance",
        "Disabled traversal affordances should not receive focus.");
}

void TestTraversalTriggerTakesPriorityOverJumpWhenFocused()
{
    TraversalSystem traversal;
    traversal.addAffordance(MakeTestTraversalAffordance());
    traversal.updateFocus({0.0f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f});

    engine::InputState input;
    input.jumpPressed = true;
    const TraversalActivation activation = traversal.activationFromInput(input);

    PlayerController player;
    player.setPosition({0.0f, 0.0f, 0.1f});
    player.update(0.1f, input, 0.0f, &activation);

    Expect(player.state().traversalMode == PlayerTraversalMode::Traversing,
        "TestTraversalTriggerTakesPriorityOverJumpWhenFocused",
        "Focused traversal should put the player into traversal state.");
    ExpectNear(player.state().velocity.y, 0.0f, 0.001f,
        "TestTraversalTriggerTakesPriorityOverJumpWhenFocused",
        "Traversal should suppress the normal jump impulse for that press.");
}

void TestTraversalCompletesAtTargetPosition()
{
    TraversalSystem traversal;
    traversal.addAffordance(MakeTestTraversalAffordance());
    traversal.updateFocus({0.0f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f});

    engine::InputState input;
    input.jumpPressed = true;
    TraversalActivation activation = traversal.activationFromInput(input);

    PlayerController player;
    player.setPosition({0.0f, 0.0f, 0.1f});
    player.update(0.1f, input, 0.0f, &activation);

    input.jumpPressed = false;
    activation = {};
    for (int i = 0; i < 10; ++i) {
        player.update(0.05f, input, 0.0f, &activation);
    }

    Expect(player.state().traversalMode == PlayerTraversalMode::Normal,
        "TestTraversalCompletesAtTargetPosition",
        "Traversal should return the player to normal state after the duration.");
    ExpectNear(player.state().position.x, 0.0f, 0.001f,
        "TestTraversalCompletesAtTargetPosition",
        "Traversal should end at the target X position.");
    ExpectNear(player.state().position.y, 0.0f, 0.001f,
        "TestTraversalCompletesAtTargetPosition",
        "Traversal should end grounded at the target Y position.");
    ExpectNear(player.state().position.z, 2.25f, 0.001f,
        "TestTraversalCompletesAtTargetPosition",
        "Traversal should end at the target Z position.");
}

void TestTraversalDoesNotRetriggerWhileActive()
{
    TraversalSystem traversal;
    TraversalAffordance first = MakeTestTraversalAffordance();
    first.name = "First Vault";
    first.endPosition = {0.0f, 0.0f, 2.25f};
    const int firstId = traversal.addAffordance(first);

    TraversalAffordance second = MakeTestTraversalAffordance();
    second.name = "Second Vault";
    second.startPosition = {0.0f, 0.0f, 1.1f};
    second.endPosition = {5.0f, 0.0f, 5.0f};
    const int secondId = traversal.addAffordance(second);

    engine::InputState input;
    input.jumpPressed = true;
    PlayerController player;
    player.setPosition({0.0f, 0.0f, 0.1f});

    traversal.updateFocus({0.0f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f});
    TraversalActivation activation = traversal.activationFromInput(input);
    Expect(activation.affordanceId == firstId,
        "TestTraversalDoesNotRetriggerWhileActive",
        "The first traversal should be selected initially.");
    player.update(0.05f, input, 0.0f, &activation);

    TraversalActivation secondActivation;
    secondActivation.started = true;
    secondActivation.affordanceId = secondId;
    secondActivation.name = "Second Vault";
    secondActivation.type = TraversalType::Vault;
    secondActivation.startPosition = {0.0f, 0.0f, 1.1f};
    secondActivation.endPosition = {5.0f, 0.0f, 5.0f};
    secondActivation.durationSeconds = 0.25f;

    player.update(0.05f, input, 0.0f, &secondActivation);
    Expect(player.state().activeTraversalId == firstId,
        "TestTraversalDoesNotRetriggerWhileActive",
        "A new traversal request should be ignored while traversal is already active.");
}

void TestNormalJumpStillWorksWithoutTraversalFocus()
{
    PlayerController player;
    player.setPosition({0.0f, 0.0f, 0.0f});

    engine::InputState input;
    input.jumpPressed = true;
    player.update(0.1f, input, 0.0f);

    Expect(player.state().traversalMode == PlayerTraversalMode::Normal,
        "TestNormalJumpStillWorksWithoutTraversalFocus",
        "Normal jump should not enter traversal state without a traversal activation.");
    Expect(player.state().position.y > 0.0f,
        "TestNormalJumpStillWorksWithoutTraversalFocus",
        "Normal jump should still lift the player when no traversal affordance is focused.");
    Expect(!player.state().grounded,
        "TestNormalJumpStillWorksWithoutTraversalFocus",
        "Player should be airborne after a normal jump.");
}

} // namespace

int main()
{
    TestSmokeArgumentsEnableBoundedHeadlessRun();
    TestFramesArgumentOverridesSmokeDefault();
    TestInvalidRendererIsRejected();
    TestNormalizePathKeepsAssetPathsInsideBase();
    TestClockStartsAtFrameZeroAndTicksForward();
    TestNullRendererRecordsFrameAndDebugDraw();
    TestVec3NormalizationKeepsDiagonalMovementAtUnitLength();
    TestPlayerMovementIsCameraRelativeAndNormalized();
    TestPlayerSprintAndJumpRemainGroundedDeterministically();
    TestThirdPersonCameraClampsPitchAndSmooths();
    TestAabbOverlapAndClosestPoint();
    TestWorldGroundClampAndGroundedState();
    TestWorldPushesPlayerOutOfBox();
    TestPlayerMovementUsesWorldCollisionForWall();
    TestDiagonalMovementIntoObstacleDoesNotTunnel();
    TestWorldRaycastFindsNearestCollider();
    TestInteractionFocusSelectsNearestFacingCandidate();
    TestInteractionFocusIgnoresDisabledAndConsumedInteractables();
    TestInteractPressedTriggersExactlyOnceWhenHeld();
    TestOneShotPickupConsumesAndDoesNotTriggerAgain();
    TestToggleInteractableChangesStateOnEachPress();
    TestNoFocusedInteractableMeansNoAction();
    TestTraversalFocusSelectsAvailableAffordance();
    TestTraversalFocusIgnoresDisabledAffordance();
    TestTraversalTriggerTakesPriorityOverJumpWhenFocused();
    TestTraversalCompletesAtTargetPosition();
    TestTraversalDoesNotRetriggerWhileActive();
    TestNormalJumpStillWorksWithoutTraversalFocus();

    if (!failures.empty()) {
        std::cerr << failures.size() << " test failure(s):\n";
        for (const auto& failure : failures) {
            std::cerr << "  " << failure.name << ": " << failure.message << '\n';
        }
        return 1;
    }

    std::cout << "EngineCoreTests passed.\n";
    return 0;
}
