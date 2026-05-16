#include "game/FerryOfficeVehiclePhysicsQa.h"

#include "game/SceneDefinition.h"
#include "game/SceneLoader.h"
#include "game/ThirdPersonCamera.h"
#include "game/VehicleController.h"
#include "engine/physics/PhysicsWorld.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <nlohmann/json.hpp>
#include <string_view>

namespace {

constexpr std::string_view ScenarioName = "ferry-office-vehicle-feasibility";
constexpr std::string_view RuntimeComparisonScenarioName = "ferry-office-vehicle-runtime-comparison";
constexpr std::string_view ServiceVehicleId = "service-yard-vehicle";
constexpr std::string_view ServiceRunCheckpointMarkerId = "service-run-checkpoint-marker";
constexpr std::string_view ServiceRunConfirmMarkerId = "service-run-confirm-marker";
constexpr float RuntimePositionDeltaLimit = 4.0f;
constexpr float RuntimeYawDeltaLimitDegrees = 130.0f;
constexpr float RuntimeSpeedDeltaLimit = 5.0f;
constexpr int RouteCheckMaxFrames = 240;
constexpr float RouteCheckThrottle = 0.72f;
constexpr int ObstacleCheckMaxFrames = 150;
constexpr float ObstacleCheckThrottle = 0.72f;
constexpr float ObstacleLaneCenterZ = -2.2f;
constexpr float ObstacleProxyX = 12.8f;
constexpr float ObstacleProxyZ = -2.2f;
constexpr float ObstacleRequiredLateralOffset = 0.05f;
constexpr float ObstacleRequiredXProgress = 4.5f;
constexpr engine::Vec3 ObstacleCollisionHalfExtents {0.30f, 0.45f, 0.12f};
constexpr engine::Vec3 ObstacleCollisionProbeHalfExtents {0.18f, 0.35f, 0.12f};

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

nlohmann::json ControlCheckJson(const FerryOfficeVehicleRuntimeComparisonResult::ControlCheck& check)
{
    return {
        {"name", check.name},
        {"passed", check.passed},
        {"frameIndex", check.frameIndex},
        {"speed", check.speed},
        {"distance", check.distance},
        {"message", check.message},
    };
}

nlohmann::json RouteCheckJson(const FerryOfficeVehicleRuntimeComparisonResult::RouteCheck& check)
{
    return {
        {"backend", check.backendName},
        {"passed", check.passed},
        {"checkpointReached", check.checkpointReached},
        {"framesToCheckpoint", check.framesToCheckpoint},
        {"minDistanceToCheckpoint", check.minDistanceToCheckpoint},
        {"finalPosition", Vec3Json(check.finalPosition)},
        {"finalYawDegrees", engine::Degrees(check.finalYawRadians)},
        {"hitBounds", check.hitBounds},
        {"message", check.message},
    };
}

nlohmann::json ObstacleCheckJson(const FerryOfficeVehicleRuntimeComparisonResult::ObstacleCheck& check)
{
    return {
        {"backend", check.backendName},
        {"passed", check.passed},
        {"clearedObstacleProxy", check.clearedObstacleProxy},
        {"frameCount", check.frameCount},
        {"maxLateralOffset", check.maxLateralOffset},
        {"minDistanceToObstacle", check.minDistanceToObstacle},
        {"collisionBacked", check.collisionBacked},
        {"obstacleCollisionClear", check.obstacleCollisionClear},
        {"obstacleOverlapFrames", check.obstacleOverlapFrames},
        {"minCollisionClearance", check.minCollisionClearance},
        {"maxCameraYawDeltaDegrees", check.maxCameraYawDeltaDegrees},
        {"finalCameraYawDegrees", engine::Degrees(check.finalCameraYawRadians)},
        {"finalPosition", Vec3Json(check.finalPosition)},
        {"finalYawDegrees", engine::Degrees(check.finalYawRadians)},
        {"hitBounds", check.hitBounds},
        {"message", check.message},
    };
}

nlohmann::json DrivingFeelCheckJson(const FerryOfficeVehicleRuntimeComparisonResult::DrivingFeelCheck& check)
{
    return {
        {"backend", check.backendName},
        {"name", check.name},
        {"passed", check.passed},
        {"value", check.value},
        {"minValue", check.minValue},
        {"maxValue", check.maxValue},
        {"units", check.units},
        {"message", check.message},
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

bool RuntimeStateLooksStable(const engine::physics::VehicleRuntimeState& state)
{
    return !state.outOfBounds && state.wheelContactCount >= 2;
}

bool StepRuntimeAdapter(
    engine::physics::IVehicleRuntimeAdapter& adapter,
    const engine::physics::VehicleRuntimeInput& input,
    int frames,
    float fixedStepSeconds)
{
    for (int frame = 0; frame < frames; ++frame) {
        if (!adapter.step(input, fixedStepSeconds)) {
            return false;
        }
    }
    return true;
}

VehicleController BuildDeterministicVehicle(const SceneVehicleDefinition& vehicle);

FerryOfficeVehicleRuntimeComparisonResult::ControlCheck MakeControlCheck(
    std::string name,
    const engine::physics::VehicleRuntimeState& state,
    bool passed,
    float distance,
    std::string message)
{
    FerryOfficeVehicleRuntimeComparisonResult::ControlCheck check;
    check.name = std::move(name);
    check.passed = passed;
    check.frameIndex = state.frameIndex;
    check.speed = state.speed;
    check.distance = distance;
    check.message = std::move(message);
    return check;
}

std::vector<FerryOfficeVehicleRuntimeComparisonResult::ControlCheck> RunVehicleRuntimeControlChecks(
    const engine::physics::VehicleRuntimeConfig& config,
    engine::physics::PhysicsBackend backend)
{
    std::vector<FerryOfficeVehicleRuntimeComparisonResult::ControlCheck> checks;

    {
        std::unique_ptr<engine::physics::IVehicleRuntimeAdapter> adapter =
            engine::physics::CreateVehicleRuntimeAdapter(backend);
        if (!adapter || !adapter->initialize(config)) {
            engine::physics::VehicleRuntimeState state;
            state.position = config.spawnPosition;
            checks.push_back(MakeControlCheck(
                "tapThrottleCoast",
                state,
                false,
                0.0f,
                "Runtime adapter was unavailable for tap/coast control QA."));
            return checks;
        }

        const bool stepped = StepRuntimeAdapter(*adapter, {1.0f, 0.0f, 0.0f}, 1, config.fixedStepSeconds)
            && StepRuntimeAdapter(*adapter, {}, 90, config.fixedStepSeconds);
        const engine::physics::VehicleRuntimeState settled = adapter->state();
        adapter->shutdown();

        const float distance = HorizontalDistance(config.spawnPosition, settled.position);
        const bool passed = stepped
            && RuntimeStateLooksStable(settled)
            && std::abs(settled.speed) <= 0.25f
            && distance <= 1.5f;
        checks.push_back(MakeControlCheck(
            "tapThrottleCoast",
            settled,
            passed,
            distance,
            passed
                ? "Short throttle tap settled without service-yard creep."
                : "Short throttle tap did not settle quickly enough for compact-yard control."));
    }

    std::unique_ptr<engine::physics::IVehicleRuntimeAdapter> adapter =
        engine::physics::CreateVehicleRuntimeAdapter(backend);
    if (!adapter || !adapter->initialize(config)) {
        engine::physics::VehicleRuntimeState state;
        state.position = config.spawnPosition;
        checks.push_back(MakeControlCheck(
            "brakeStopsForwardMotion",
            state,
            false,
            0.0f,
            "Runtime adapter was unavailable for brake/reverse control QA."));
        return checks;
    }

    const bool forwardStepped = StepRuntimeAdapter(*adapter, {0.65f, 0.0f, 0.0f}, 30, config.fixedStepSeconds);
    const engine::physics::VehicleRuntimeState forward = adapter->state();
    const bool brakeStepped = StepRuntimeAdapter(*adapter, {0.0f, 0.0f, 1.0f}, 16, config.fixedStepSeconds);
    const engine::physics::VehicleRuntimeState stopped = adapter->state();
    const float brakeDistance = HorizontalDistance(forward.position, stopped.position);
    const bool brakePassed = forwardStepped
        && brakeStepped
        && RuntimeStateLooksStable(stopped)
        && std::abs(stopped.speed) <= 0.65f
        && stopped.speed >= -0.25f;
    checks.push_back(MakeControlCheck(
        "brakeStopsForwardMotion",
        stopped,
        brakePassed,
        brakeDistance,
        brakePassed
            ? "Brake input stopped forward motion before reverse was requested."
            : "Brake input did not stop forward motion inside the compact control budget."));

    adapter->shutdown();
    adapter = engine::physics::CreateVehicleRuntimeAdapter(backend);
    if (!adapter || !adapter->initialize(config)) {
        engine::physics::VehicleRuntimeState state;
        state.position = config.spawnPosition;
        checks.push_back(MakeControlCheck(
            "reverseMovesBackward",
            state,
            false,
            0.0f,
            "Runtime adapter was unavailable for reverse control QA."));
        return checks;
    }

    const bool reverseStepped = StepRuntimeAdapter(*adapter, {-1.0f, 0.0f, 0.0f}, 80, config.fixedStepSeconds);
    const engine::physics::VehicleRuntimeState reverse = adapter->state();
    const float signedReverseDistance = engine::Dot(reverse.position - config.spawnPosition, engine::ForwardFromYaw(config.spawnYawRadians));
    const bool reversePassed = reverseStepped
        && RuntimeStateLooksStable(reverse)
        && reverse.speed < -0.10f
        && signedReverseDistance < -0.15f;
    checks.push_back(MakeControlCheck(
        "reverseMovesBackward",
        reverse,
        reversePassed,
        -signedReverseDistance,
        reversePassed
            ? "Reverse input produced negative speed and backward motion."
            : "Reverse input did not produce clear backward motion."));

    adapter->shutdown();
    adapter = engine::physics::CreateVehicleRuntimeAdapter(backend);
    if (!adapter || !adapter->initialize(config)) {
        engine::physics::VehicleRuntimeState state;
        state.position = config.spawnPosition;
        checks.push_back(MakeControlCheck(
            "reverseCoastSettles",
            state,
            false,
            0.0f,
            "Runtime adapter was unavailable for reverse coast control QA."));
        return checks;
    }

    StepRuntimeAdapter(*adapter, {-0.65f, 0.0f, 0.0f}, 30, config.fixedStepSeconds);
    const engine::physics::VehicleRuntimeState reverseBeforeCoast = adapter->state();
    const bool coastStepped = StepRuntimeAdapter(*adapter, {}, 90, config.fixedStepSeconds);
    const engine::physics::VehicleRuntimeState coasted = adapter->state();
    const float coastDistance = HorizontalDistance(reverseBeforeCoast.position, coasted.position);
    const bool coastPassed = coastStepped
        && RuntimeStateLooksStable(coasted)
        && std::abs(coasted.speed) <= 0.35f;
    checks.push_back(MakeControlCheck(
        "reverseCoastSettles",
        coasted,
        coastPassed,
        coastDistance,
        coastPassed
            ? "Neutral input settled reverse motion after backing up."
            : "Neutral input did not settle reverse motion quickly enough."));

    adapter->shutdown();
    return checks;
}

FerryOfficeVehicleRuntimeComparisonResult::RouteCheck RunDeterministicRouteCheck(
    const SceneVehicleDefinition& vehicle,
    engine::Vec3 checkpointPosition,
    float checkpointRadius)
{
    FerryOfficeVehicleRuntimeComparisonResult::RouteCheck check;
    check.backendName = "deterministic";
    check.framesToCheckpoint = -1;
    check.minDistanceToCheckpoint = std::numeric_limits<float>::max();

    VehicleController controller = BuildDeterministicVehicle(vehicle);
    engine::InputState input;
    input.moveForward = RouteCheckThrottle;

    for (int frame = 0; frame < RouteCheckMaxFrames; ++frame) {
        controller.beginFrame();
        controller.updateDriving(1.0f / 60.0f, input);
        const VehicleState& state = controller.state();
        const float distance = HorizontalDistance(state.position, checkpointPosition);
        check.minDistanceToCheckpoint = std::min(check.minDistanceToCheckpoint, distance);
        check.finalPosition = state.position;
        check.finalYawRadians = state.yawRadians;
        check.hitBounds = check.hitBounds || state.hitBoundsThisFrame;
        if (distance <= checkpointRadius) {
            check.checkpointReached = true;
            check.framesToCheckpoint = frame + 1;
            break;
        }
    }

    check.passed = check.checkpointReached && !check.hitBounds;
    check.message = check.passed
        ? "Deterministic route reached the authored service-run checkpoint."
        : "Deterministic route did not reach the authored service-run checkpoint cleanly.";
    return check;
}

FerryOfficeVehicleRuntimeComparisonResult::RouteCheck RunAdapterRouteCheck(
    const engine::physics::VehicleRuntimeConfig& config,
    engine::physics::PhysicsBackend backend,
    engine::Vec3 checkpointPosition,
    float checkpointRadius)
{
    FerryOfficeVehicleRuntimeComparisonResult::RouteCheck check;
    check.backendName = "unavailable";
    check.framesToCheckpoint = -1;
    check.minDistanceToCheckpoint = std::numeric_limits<float>::max();
    check.finalPosition = config.spawnPosition;
    check.finalYawRadians = config.spawnYawRadians;

    std::unique_ptr<engine::physics::IVehicleRuntimeAdapter> adapter =
        engine::physics::CreateVehicleRuntimeAdapter(backend);
    if (!adapter || !adapter->initialize(config)) {
        check.message = "Runtime adapter was unavailable for service-run route QA.";
        return check;
    }

    check.backendName = std::string(adapter->backendName());
    for (int frame = 0; frame < RouteCheckMaxFrames; ++frame) {
        const bool stepped = adapter->step({RouteCheckThrottle, 0.0f, 0.0f}, config.fixedStepSeconds);
        const engine::physics::VehicleRuntimeState state = adapter->state();
        const float distance = HorizontalDistance(state.position, checkpointPosition);
        check.minDistanceToCheckpoint = std::min(check.minDistanceToCheckpoint, distance);
        check.finalPosition = state.position;
        check.finalYawRadians = state.yawRadians;
        check.hitBounds = check.hitBounds || state.outOfBounds;
        if (!stepped || state.wheelContactCount < 2) {
            break;
        }
        if (distance <= checkpointRadius) {
            check.checkpointReached = true;
            check.framesToCheckpoint = frame + 1;
            break;
        }
    }
    adapter->shutdown();

    check.passed = check.checkpointReached && !check.hitBounds;
    check.message = check.passed
        ? "Runtime adapter route reached the authored service-run checkpoint."
        : "Runtime adapter route did not reach the authored service-run checkpoint cleanly.";
    return check;
}

float ObstacleSteerForFrame(int frame)
{
    if (frame < 36) {
        return 0.0f;
    }
    if (frame < 96) {
        return -0.18f;
    }
    if (frame < 130) {
        return 0.16f;
    }
    return 0.0f;
}

ThirdPersonCamera BuildObstacleCamera(float yawRadians)
{
    ThirdPersonCamera camera;
    ThirdPersonCameraSettings settings;
    settings.distance = 6.75f;
    settings.heightOffset = 1.95f;
    settings.smoothing = 9.0f;
    settings.targetYawFollowStrength = 3.5f;
    camera.setSettings(settings);
    camera.setYawRadians(yawRadians);
    return camera;
}

void UpdateObstacleCameraTelemetry(
    FerryOfficeVehicleRuntimeComparisonResult::ObstacleCheck& check,
    ThirdPersonCamera& camera,
    engine::Vec3 vehiclePosition,
    float vehicleYawRadians,
    float dt)
{
    CameraTarget target;
    target.position = vehiclePosition;
    target.yawRadians = vehicleYawRadians;
    engine::InputState cameraInput;
    camera.update(dt, cameraInput, target);
    check.finalCameraYawRadians = camera.state().yawRadians;
    check.maxCameraYawDeltaDegrees = std::max(
        check.maxCameraYawDeltaDegrees,
        AbsYawDeltaDegrees(vehicleYawRadians, camera.state().yawRadians));
}

std::unique_ptr<engine::physics::IPhysicsWorld> BuildObstaclePhysicsWorld(engine::physics::PhysicsBackend backend)
{
    std::unique_ptr<engine::physics::IPhysicsWorld> world = engine::physics::CreatePhysicsWorld(backend);
    if (!world) {
        return nullptr;
    }

    engine::physics::PhysicsConfig config;
    config.backend = backend;
    if (!world->initialize(config)) {
        return nullptr;
    }

    engine::physics::BoxColliderDesc obstacle;
    obstacle.name = "qa-service-road-obstacle";
    obstacle.center = {ObstacleProxyX, ObstacleCollisionHalfExtents.y, ObstacleProxyZ};
    obstacle.halfExtents = ObstacleCollisionHalfExtents;
    obstacle.isTrigger = true;
    if (!world->addTriggerBox(obstacle).isValid()) {
        world->shutdown();
        return nullptr;
    }
    return world;
}

float CollisionProbeClearance(engine::Vec3 position)
{
    const float separatedX = std::abs(position.x - ObstacleProxyX)
        - (ObstacleCollisionHalfExtents.x + ObstacleCollisionProbeHalfExtents.x);
    const float separatedZ = std::abs(position.z - ObstacleProxyZ)
        - (ObstacleCollisionHalfExtents.z + ObstacleCollisionProbeHalfExtents.z);
    return std::max(separatedX, separatedZ);
}

void UpdateObstacleCollisionTelemetry(
    FerryOfficeVehicleRuntimeComparisonResult::ObstacleCheck& check,
    const std::unique_ptr<engine::physics::IPhysicsWorld>& world,
    engine::Vec3 vehiclePosition)
{
    if (!world) {
        return;
    }

    check.collisionBacked = true;
    const engine::Vec3 probeCenter {
        vehiclePosition.x,
        ObstacleCollisionProbeHalfExtents.y,
        vehiclePosition.z,
    };
    const std::vector<engine::physics::OverlapResult> overlaps =
        world->overlapBox(probeCenter, ObstacleCollisionProbeHalfExtents);
    if (!overlaps.empty()) {
        check.obstacleOverlapFrames += 1;
    }
    check.minCollisionClearance = std::min(check.minCollisionClearance, CollisionProbeClearance(vehiclePosition));
}

FerryOfficeVehicleRuntimeComparisonResult::ObstacleCheck RunDeterministicObstacleCheck(const SceneVehicleDefinition& vehicle)
{
    FerryOfficeVehicleRuntimeComparisonResult::ObstacleCheck check;
    check.backendName = "deterministic";
    check.minDistanceToObstacle = std::numeric_limits<float>::max();
    check.minCollisionClearance = std::numeric_limits<float>::max();
    check.finalPosition = vehicle.spawnPosition;
    check.finalYawRadians = vehicle.spawnYawRadians;

    VehicleController controller = BuildDeterministicVehicle(vehicle);
    ThirdPersonCamera camera = BuildObstacleCamera(vehicle.spawnYawRadians);
    std::unique_ptr<engine::physics::IPhysicsWorld> obstacleWorld =
        BuildObstaclePhysicsWorld(engine::physics::PhysicsBackend::Simple);
    for (int frame = 0; frame < ObstacleCheckMaxFrames; ++frame) {
        engine::InputState input;
        input.moveForward = ObstacleCheckThrottle;
        input.moveRight = ObstacleSteerForFrame(frame);
        controller.beginFrame();
        controller.updateDriving(1.0f / 60.0f, input);

        const VehicleState& state = controller.state();
        check.frameCount = frame + 1;
        check.finalPosition = state.position;
        check.finalYawRadians = state.yawRadians;
        check.hitBounds = check.hitBounds || state.hitBoundsThisFrame;
        check.maxLateralOffset = std::max(check.maxLateralOffset, std::abs(state.position.z - ObstacleLaneCenterZ));
        check.minDistanceToObstacle = std::min(check.minDistanceToObstacle, HorizontalDistance(state.position, {ObstacleProxyX, 0.0f, ObstacleProxyZ}));
        UpdateObstacleCollisionTelemetry(check, obstacleWorld, state.position);
        UpdateObstacleCameraTelemetry(check, camera, state.position, state.yawRadians, 1.0f / 60.0f);
    }
    if (obstacleWorld) {
        obstacleWorld->shutdown();
    }

    check.clearedObstacleProxy =
        (check.finalPosition.x - vehicle.spawnPosition.x) >= ObstacleRequiredXProgress
        && check.maxLateralOffset >= ObstacleRequiredLateralOffset;
    check.obstacleCollisionClear = check.collisionBacked && check.obstacleOverlapFrames == 0;
    check.passed = check.clearedObstacleProxy && check.obstacleCollisionClear && !check.hitBounds;
    check.message = check.passed
        ? "Deterministic steering cleared the overlap-backed obstacle probe."
        : "Deterministic steering did not cleanly clear the overlap-backed obstacle probe.";
    return check;
}

FerryOfficeVehicleRuntimeComparisonResult::ObstacleCheck RunAdapterObstacleCheck(
    const engine::physics::VehicleRuntimeConfig& config,
    engine::physics::PhysicsBackend backend)
{
    FerryOfficeVehicleRuntimeComparisonResult::ObstacleCheck check;
    check.backendName = "unavailable";
    check.minDistanceToObstacle = std::numeric_limits<float>::max();
    check.minCollisionClearance = std::numeric_limits<float>::max();
    check.finalPosition = config.spawnPosition;
    check.finalYawRadians = config.spawnYawRadians;
    ThirdPersonCamera camera = BuildObstacleCamera(config.spawnYawRadians);
    std::unique_ptr<engine::physics::IPhysicsWorld> obstacleWorld = BuildObstaclePhysicsWorld(backend);

    std::unique_ptr<engine::physics::IVehicleRuntimeAdapter> adapter =
        engine::physics::CreateVehicleRuntimeAdapter(backend);
    if (!adapter || !adapter->initialize(config)) {
        check.message = "Runtime adapter was unavailable for obstacle-proxy steering QA.";
        return check;
    }

    check.backendName = std::string(adapter->backendName());
    for (int frame = 0; frame < ObstacleCheckMaxFrames; ++frame) {
        const bool stepped = adapter->step({ObstacleCheckThrottle, ObstacleSteerForFrame(frame), 0.0f}, config.fixedStepSeconds);
        const engine::physics::VehicleRuntimeState state = adapter->state();
        check.frameCount = frame + 1;
        check.finalPosition = state.position;
        check.finalYawRadians = state.yawRadians;
        check.hitBounds = check.hitBounds || state.outOfBounds;
        check.maxLateralOffset = std::max(check.maxLateralOffset, std::abs(state.position.z - ObstacleLaneCenterZ));
        check.minDistanceToObstacle = std::min(check.minDistanceToObstacle, HorizontalDistance(state.position, {ObstacleProxyX, 0.0f, ObstacleProxyZ}));
        UpdateObstacleCollisionTelemetry(check, obstacleWorld, state.position);
        UpdateObstacleCameraTelemetry(check, camera, state.position, state.yawRadians, config.fixedStepSeconds);
        if (!stepped || state.wheelContactCount < 2) {
            break;
        }
    }
    adapter->shutdown();
    if (obstacleWorld) {
        obstacleWorld->shutdown();
    }

    check.clearedObstacleProxy =
        (check.finalPosition.x - config.spawnPosition.x) >= ObstacleRequiredXProgress
        && check.maxLateralOffset >= ObstacleRequiredLateralOffset;
    check.obstacleCollisionClear = check.collisionBacked && check.obstacleOverlapFrames == 0;
    check.passed = check.clearedObstacleProxy && check.obstacleCollisionClear && !check.hitBounds;
    check.message = check.passed
        ? "Runtime adapter steering cleared the overlap-backed obstacle probe."
        : "Runtime adapter steering did not cleanly clear the overlap-backed obstacle probe.";
    return check;
}

FerryOfficeVehicleRuntimeComparisonResult::DrivingFeelCheck MakeDrivingFeelCheck(
    std::string backendName,
    std::string name,
    bool passed,
    float value,
    float minValue,
    float maxValue,
    std::string units,
    std::string message)
{
    FerryOfficeVehicleRuntimeComparisonResult::DrivingFeelCheck check;
    check.backendName = std::move(backendName);
    check.name = std::move(name);
    check.passed = passed;
    check.value = value;
    check.minValue = minValue;
    check.maxValue = maxValue;
    check.units = std::move(units);
    check.message = std::move(message);
    return check;
}

std::vector<FerryOfficeVehicleRuntimeComparisonResult::DrivingFeelCheck> RunDeterministicDrivingFeelChecks(
    const SceneVehicleDefinition& vehicle,
    engine::Vec3 checkpointPosition,
    float checkpointRadius)
{
    std::vector<FerryOfficeVehicleRuntimeComparisonResult::DrivingFeelCheck> checks;

    {
        VehicleController controller = BuildDeterministicVehicle(vehicle);
        engine::InputState input;
        input.moveForward = RouteCheckThrottle;
        int framesToCheckpoint = -1;
        bool hitBounds = false;
        float maxRouteDeviation = 0.0f;
        for (int frame = 0; frame < RouteCheckMaxFrames; ++frame) {
            controller.beginFrame();
            controller.updateDriving(1.0f / 60.0f, input);
            const VehicleState& state = controller.state();
            hitBounds = hitBounds || state.hitBoundsThisFrame;
            maxRouteDeviation = std::max(maxRouteDeviation, std::abs(state.position.z - vehicle.spawnPosition.z));
            if (HorizontalDistance(state.position, checkpointPosition) <= checkpointRadius) {
                framesToCheckpoint = frame + 1;
                break;
            }
        }

        checks.push_back(MakeDrivingFeelCheck(
            "deterministic",
            "routeFramesToCheckpoint",
            framesToCheckpoint > 0 && framesToCheckpoint <= 170 && !hitBounds,
            static_cast<float>(framesToCheckpoint),
            1.0f,
            170.0f,
            "frames",
            "Deterministic vehicle should reach the service-run checkpoint without feeling sluggish."));
        checks.push_back(MakeDrivingFeelCheck(
            "deterministic",
            "routeMaxLateralDeviation",
            maxRouteDeviation <= 0.75f && !hitBounds,
            maxRouteDeviation,
            0.0f,
            0.75f,
            "meters",
            "Straight service-road throttle should stay visually centered in the authored lane."));
    }

    {
        VehicleController controller = BuildDeterministicVehicle(vehicle);
        engine::InputState throttle;
        throttle.moveForward = 0.72f;
        for (int frame = 0; frame < 60; ++frame) {
            controller.beginFrame();
            controller.updateDriving(1.0f / 60.0f, throttle);
        }
        const engine::Vec3 brakeStart = controller.state().position;
        engine::InputState brake;
        brake.moveForward = -1.0f;
        bool hitBounds = false;
        for (int frame = 0; frame < 90; ++frame) {
            controller.beginFrame();
            controller.updateDriving(1.0f / 60.0f, brake);
            hitBounds = hitBounds || controller.state().hitBoundsThisFrame;
            if (std::abs(controller.state().speed) <= 0.05f) {
                break;
            }
        }
        const float stopDistance = HorizontalDistance(brakeStart, controller.state().position);
        checks.push_back(MakeDrivingFeelCheck(
            "deterministic",
            "brakeStopDistance",
            stopDistance <= 2.65f && std::abs(controller.state().speed) <= 0.10f && !hitBounds,
            stopDistance,
            0.0f,
            2.65f,
            "meters",
            "Brake input should stop the cart within the compact service-yard road budget."));
    }

    {
        VehicleController controller = BuildDeterministicVehicle(vehicle);
        engine::InputState reverse;
        reverse.moveForward = -1.0f;
        bool hitBounds = false;
        for (int frame = 0; frame < 80; ++frame) {
            controller.beginFrame();
            controller.updateDriving(1.0f / 60.0f, reverse);
            hitBounds = hitBounds || controller.state().hitBoundsThisFrame;
        }
        const float signedDistance =
            engine::Dot(controller.state().position - vehicle.spawnPosition, engine::ForwardFromYaw(vehicle.spawnYawRadians));
        checks.push_back(MakeDrivingFeelCheck(
            "deterministic",
            "reverseDistance",
            signedDistance <= -0.75f && controller.state().speed < -0.10f && !hitBounds,
            -signedDistance,
            0.75f,
            5.0f,
            "meters",
            "Reverse should produce clear backward motion without a manual-feel dead zone."));
    }

    {
        VehicleController controller = BuildDeterministicVehicle(vehicle);
        ThirdPersonCamera camera = BuildObstacleCamera(vehicle.spawnYawRadians);
        bool hitBounds = false;
        float maxCameraYawDelta = 0.0f;
        engine::InputState input;
        input.moveForward = 0.65f;
        input.moveRight = 0.36f;
        for (int frame = 0; frame < 70; ++frame) {
            controller.beginFrame();
            controller.updateDriving(1.0f / 60.0f, input);
            const VehicleState& state = controller.state();
            hitBounds = hitBounds || state.hitBoundsThisFrame;
            CameraTarget target = controller.cameraTarget();
            engine::InputState cameraInput;
            camera.update(1.0f / 60.0f, cameraInput, target);
            maxCameraYawDelta =
                std::max(maxCameraYawDelta, AbsYawDeltaDegrees(state.yawRadians, camera.state().yawRadians));
        }
        const float yawDelta = AbsYawDeltaDegrees(vehicle.spawnYawRadians, controller.state().yawRadians);
        checks.push_back(MakeDrivingFeelCheck(
            "deterministic",
            "steeringYawResponse",
            yawDelta >= 12.0f && yawDelta <= 70.0f && !hitBounds,
            yawDelta,
            12.0f,
            70.0f,
            "degrees",
            "A moderate steering hold should visibly turn the vehicle without spinning it around."));
        checks.push_back(MakeDrivingFeelCheck(
            "deterministic",
            "cameraYawLag",
            maxCameraYawDelta <= 55.0f && !hitBounds,
            maxCameraYawDelta,
            0.0f,
            55.0f,
            "degrees",
            "Vehicle camera target should stay close enough to the turning vehicle for readable control."));
    }

    return checks;
}

std::vector<FerryOfficeVehicleRuntimeComparisonResult::DrivingFeelCheck> RunAdapterDrivingFeelChecks(
    const engine::physics::VehicleRuntimeConfig& config,
    engine::physics::PhysicsBackend backend,
    engine::Vec3 checkpointPosition,
    float checkpointRadius)
{
    std::vector<FerryOfficeVehicleRuntimeComparisonResult::DrivingFeelCheck> checks;
    std::unique_ptr<engine::physics::IVehicleRuntimeAdapter> adapter =
        engine::physics::CreateVehicleRuntimeAdapter(backend);
    if (!adapter || !adapter->initialize(config)) {
        checks.push_back(MakeDrivingFeelCheck(
            "unavailable",
            "adapterAvailable",
            false,
            0.0f,
            1.0f,
            1.0f,
            "bool",
            "Requested vehicle runtime adapter was unavailable for driving-feel QA."));
        return checks;
    }
    const std::string backendName(adapter->backendName());
    adapter->shutdown();

    {
        adapter = engine::physics::CreateVehicleRuntimeAdapter(backend);
        if (!adapter || !adapter->initialize(config)) {
            checks.push_back(MakeDrivingFeelCheck(backendName, "routeFramesToCheckpoint", false, -1.0f, 1.0f, 240.0f, "frames", "Adapter route check could not initialize."));
            return checks;
        }
        int framesToCheckpoint = -1;
        bool stable = true;
        bool hitBounds = false;
        float maxRouteDeviation = 0.0f;
        for (int frame = 0; frame < RouteCheckMaxFrames; ++frame) {
            stable = stable && adapter->step({RouteCheckThrottle, 0.0f, 0.0f}, config.fixedStepSeconds);
            const engine::physics::VehicleRuntimeState state = adapter->state();
            hitBounds = hitBounds || state.outOfBounds;
            maxRouteDeviation = std::max(maxRouteDeviation, std::abs(state.position.z - config.spawnPosition.z));
            if (HorizontalDistance(state.position, checkpointPosition) <= checkpointRadius) {
                framesToCheckpoint = frame + 1;
                break;
            }
            if (!stable || state.wheelContactCount < 2) {
                break;
            }
        }
        adapter->shutdown();
        checks.push_back(MakeDrivingFeelCheck(
            backendName,
            "routeFramesToCheckpoint",
            framesToCheckpoint > 0 && framesToCheckpoint <= RouteCheckMaxFrames && stable && !hitBounds,
            static_cast<float>(framesToCheckpoint),
            1.0f,
            static_cast<float>(RouteCheckMaxFrames),
            "frames",
            "Runtime adapter candidate should reach the service-run checkpoint inside the authored route budget."));
        checks.push_back(MakeDrivingFeelCheck(
            backendName,
            "routeMaxLateralDeviation",
            maxRouteDeviation <= 0.85f && stable && !hitBounds,
            maxRouteDeviation,
            0.0f,
            0.85f,
            "meters",
            "Runtime adapter candidate should hold the straight dock-road lane without drifting into bounds."));
    }

    {
        adapter = engine::physics::CreateVehicleRuntimeAdapter(backend);
        if (!adapter || !adapter->initialize(config)) {
            checks.push_back(MakeDrivingFeelCheck(backendName, "brakeStopDistance", false, 999.0f, 0.0f, 4.5f, "meters", "Adapter braking check could not initialize."));
            return checks;
        }
        bool stable = StepRuntimeAdapter(*adapter, {0.72f, 0.0f, 0.0f}, 60, config.fixedStepSeconds);
        const engine::Vec3 brakeStart = adapter->state().position;
        int stopFrame = -1;
        for (int frame = 0; frame < 90; ++frame) {
            stable = stable && adapter->step({0.0f, 0.0f, 1.0f}, config.fixedStepSeconds);
            const engine::physics::VehicleRuntimeState state = adapter->state();
            if (std::abs(state.speed) <= 0.10f) {
                stopFrame = frame + 1;
                break;
            }
            if (!stable || state.outOfBounds || state.wheelContactCount < 2) {
                break;
            }
        }
        const engine::physics::VehicleRuntimeState stopped = adapter->state();
        adapter->shutdown();
        const float stopDistance = HorizontalDistance(brakeStart, stopped.position);
        checks.push_back(MakeDrivingFeelCheck(
            backendName,
            "brakeStopDistance",
            stopFrame > 0 && stopDistance <= 4.5f && !stopped.outOfBounds && stable,
            stopDistance,
            0.0f,
            4.5f,
            "meters",
            "Runtime adapter candidate should brake within the compact service-yard road budget."));
    }

    {
        adapter = engine::physics::CreateVehicleRuntimeAdapter(backend);
        if (!adapter || !adapter->initialize(config)) {
            checks.push_back(MakeDrivingFeelCheck(backendName, "reverseDistance", false, 0.0f, 0.35f, 6.0f, "meters", "Adapter reverse check could not initialize."));
            return checks;
        }
        bool stable = StepRuntimeAdapter(*adapter, {-1.0f, 0.0f, 0.0f}, 80, config.fixedStepSeconds);
        const engine::physics::VehicleRuntimeState reverse = adapter->state();
        adapter->shutdown();
        const float signedDistance =
            engine::Dot(reverse.position - config.spawnPosition, engine::ForwardFromYaw(config.spawnYawRadians));
        checks.push_back(MakeDrivingFeelCheck(
            backendName,
            "reverseDistance",
            signedDistance <= -0.35f && reverse.speed < -0.05f && !reverse.outOfBounds && stable,
            -signedDistance,
            0.35f,
            6.0f,
            "meters",
            "Runtime adapter candidate should produce clear backward motion from reverse input."));
    }

    {
        adapter = engine::physics::CreateVehicleRuntimeAdapter(backend);
        if (!adapter || !adapter->initialize(config)) {
            checks.push_back(MakeDrivingFeelCheck(backendName, "steeringYawResponse", false, 0.0f, 4.0f, 95.0f, "degrees", "Adapter steering check could not initialize."));
            return checks;
        }
        ThirdPersonCamera camera = BuildObstacleCamera(config.spawnYawRadians);
        bool stable = true;
        bool hitBounds = false;
        float maxCameraYawDelta = 0.0f;
        for (int frame = 0; frame < 70; ++frame) {
            stable = stable && adapter->step({0.65f, 0.36f, 0.0f}, config.fixedStepSeconds);
            const engine::physics::VehicleRuntimeState state = adapter->state();
            hitBounds = hitBounds || state.outOfBounds;
            CameraTarget target;
            target.position = state.position + engine::ForwardFromYaw(state.yawRadians) * 0.85f;
            target.yawRadians = state.yawRadians;
            engine::InputState cameraInput;
            camera.update(config.fixedStepSeconds, cameraInput, target);
            maxCameraYawDelta =
                std::max(maxCameraYawDelta, AbsYawDeltaDegrees(state.yawRadians, camera.state().yawRadians));
            if (!stable || state.wheelContactCount < 2) {
                break;
            }
        }
        const engine::physics::VehicleRuntimeState turned = adapter->state();
        adapter->shutdown();
        const float yawDelta = AbsYawDeltaDegrees(config.spawnYawRadians, turned.yawRadians);
        checks.push_back(MakeDrivingFeelCheck(
            backendName,
            "steeringYawResponse",
            yawDelta >= 4.0f && yawDelta <= 95.0f && stable && !hitBounds,
            yawDelta,
            4.0f,
            95.0f,
            "degrees",
            "Runtime adapter candidate should respond visibly to steering without over-rotating."));
        checks.push_back(MakeDrivingFeelCheck(
            backendName,
            "cameraYawLag",
            maxCameraYawDelta <= 65.0f && stable && !hitBounds,
            maxCameraYawDelta,
            0.0f,
            65.0f,
            "degrees",
            "Camera target should stay readable while following the runtime adapter vehicle candidate."));
    }

    return checks;
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

engine::Vec3 ServiceRunCheckpointFromScene(const SceneDefinition& scene)
{
    for (const SceneObjectiveMarkerDefinition& marker : scene.objectiveMarkers) {
        if (marker.id == ServiceRunCheckpointMarkerId) {
            return marker.position;
        }
    }
    return {19.35f, 0.08f, -2.2f};
}

float ServiceRunCheckpointRadiusFromScene(const SceneDefinition& scene)
{
    for (const SceneInteractableDefinition& interactable : scene.interactables) {
        if (interactable.id == ServiceRunConfirmMarkerId) {
            return std::max(1.5f, interactable.radius);
        }
    }
    return 1.5f;
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

    nlohmann::json controlChecks = nlohmann::json::array();
    for (const FerryOfficeVehicleRuntimeComparisonResult::ControlCheck& check : result.controlChecks) {
        controlChecks.push_back(ControlCheckJson(check));
    }

    nlohmann::json routeChecks = nlohmann::json::array();
    for (const FerryOfficeVehicleRuntimeComparisonResult::RouteCheck& check : result.routeChecks) {
        routeChecks.push_back(RouteCheckJson(check));
    }

    nlohmann::json obstacleChecks = nlohmann::json::array();
    for (const FerryOfficeVehicleRuntimeComparisonResult::ObstacleCheck& check : result.obstacleChecks) {
        obstacleChecks.push_back(ObstacleCheckJson(check));
    }

    nlohmann::json drivingFeelChecks = nlohmann::json::array();
    for (const FerryOfficeVehicleRuntimeComparisonResult::DrivingFeelCheck& check : result.drivingFeelChecks) {
        drivingFeelChecks.push_back(DrivingFeelCheckJson(check));
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
        {"routeChecks", routeChecks},
        {"obstacleChecks", obstacleChecks},
        {"controlChecks", controlChecks},
        {"drivingFeelChecks", drivingFeelChecks},
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
    result.controlChecks = RunVehicleRuntimeControlChecks(config, backend);
    const engine::Vec3 checkpointPosition = ServiceRunCheckpointFromScene(loadedScene.scene);
    const float checkpointRadius = ServiceRunCheckpointRadiusFromScene(loadedScene.scene);
    result.routeChecks.push_back(RunDeterministicRouteCheck(*vehicle, checkpointPosition, checkpointRadius));
    result.routeChecks.push_back(RunAdapterRouteCheck(config, backend, checkpointPosition, checkpointRadius));
    result.obstacleChecks.push_back(RunDeterministicObstacleCheck(*vehicle));
    result.obstacleChecks.push_back(RunAdapterObstacleCheck(config, backend));
    result.drivingFeelChecks = RunDeterministicDrivingFeelChecks(*vehicle, checkpointPosition, checkpointRadius);
    std::vector<FerryOfficeVehicleRuntimeComparisonResult::DrivingFeelCheck> adapterDrivingFeelChecks =
        RunAdapterDrivingFeelChecks(config, backend, checkpointPosition, checkpointRadius);
    result.drivingFeelChecks.insert(
        result.drivingFeelChecks.end(),
        adapterDrivingFeelChecks.begin(),
        adapterDrivingFeelChecks.end());

    const bool closeEnough = result.maxPositionDelta <= RuntimePositionDeltaLimit
        && result.maxYawDeltaDegrees <= RuntimeYawDeltaLimitDegrees
        && result.maxSpeedDelta <= RuntimeSpeedDeltaLimit;
    const bool controlsStable = !result.controlChecks.empty()
        && std::all_of(result.controlChecks.begin(), result.controlChecks.end(), [](const auto& check) {
               return check.passed;
           });
    const bool routeStable = result.routeChecks.size() == 2
        && std::all_of(result.routeChecks.begin(), result.routeChecks.end(), [](const auto& check) {
               return check.passed;
           });
    const bool obstacleStable = result.obstacleChecks.size() == 2
        && std::all_of(result.obstacleChecks.begin(), result.obstacleChecks.end(), [](const auto& check) {
               return check.passed;
           });
    const bool drivingFeelStable = !result.drivingFeelChecks.empty()
        && std::all_of(result.drivingFeelChecks.begin(), result.drivingFeelChecks.end(), [](const auto& check) {
               return check.passed;
           });
    float obstacleProgressDelta = 0.0f;
    if (result.obstacleChecks.size() == 2) {
        obstacleProgressDelta =
            std::abs(result.obstacleChecks[0].finalPosition.x - result.obstacleChecks[1].finalPosition.x);
    }
    result.passed = deterministicStable
        && adapterStable
        && closeEnough
        && controlsStable
        && routeStable
        && obstacleStable
        && drivingFeelStable
        && !result.deterministicSamples.empty();
    const bool obstacleProgressAligned = obstacleProgressDelta <= 4.0f;
    result.recommendation = result.passed && obstacleProgressAligned ? "promote" : "defer";
    if (result.passed && obstacleProgressAligned) {
        result.recommendationReason =
            "The selected vehicle runtime adapter stayed stable, close enough to compare, passed compact control checks, reached the authored service-run checkpoint, and cleared the overlap-backed obstacle replay with aligned progress.";
    } else if (result.passed) {
        result.recommendationReason =
            "The selected vehicle runtime adapter stayed stable and camera-readable, but obstacle-proxy progress still diverges enough to keep it opt-in.";
    } else {
        result.recommendationReason =
            drivingFeelStable
            ? "The selected vehicle runtime adapter needs more work before live switch promotion."
            : "The selected vehicle runtime candidate needs a focused steering, braking, reverse, route, or camera-feel tuning pass before broader promotion.";
    }
    if (!result.passed) {
        result.error = "Vehicle runtime comparison did not meet stability or comparison thresholds.";
    }

    if (!WriteReport(result)) {
        result.passed = false;
        result.error = "Failed to write vehicle runtime comparison report: " + result.reportPath.string();
    }
    return result;
}
