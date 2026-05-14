#include "engine/core/Clock.h"
#include "engine/core/Config.h"
#include "engine/core/FileSystem.h"
#include "engine/math/Math.h"
#include "game/PlayerController.h"
#include "game/TestWorld.h"
#include "game/ThirdPersonCamera.h"
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
