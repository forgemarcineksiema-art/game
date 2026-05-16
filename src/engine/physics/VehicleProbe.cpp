#include "engine/physics/VehicleProbe.h"

#include <algorithm>
#include <cmath>

namespace engine::physics {
namespace {

bool OutOfBounds(Vec3 position, const VehicleProbeConfig& config)
{
    return position.x < config.boundsMin.x || position.x > config.boundsMax.x
        || position.z < config.boundsMin.y || position.z > config.boundsMax.y;
}

VehicleProbeResult RunSimpleVehicleFeasibilityProbe(
    const VehicleProbeConfig& config,
    const std::vector<VehicleProbeInputFrame>& inputScript)
{
    VehicleProbeResult result;
    result.backendName = "simple";
    result.vehicleId = config.vehicleId;
    result.spawnPosition = config.spawnPosition;
    result.halfExtents = config.halfExtents;
    result.finalPosition = config.spawnPosition;
    result.finalYawRadians = config.spawnYawRadians;

    Vec3 position = config.spawnPosition;
    float yaw = config.spawnYawRadians;
    float speed = 0.0f;
    bool everMoved = false;
    bool anyOutOfBounds = false;

    for (const VehicleProbeInputFrame& input : inputScript) {
        for (int frame = 0; frame < input.frameCount; ++frame) {
            const float throttle = Clamp(input.throttle, -1.0f, 1.0f);
            const float brake = Clamp(input.brake, 0.0f, 1.0f);
            const float steer = Clamp(input.steer, -1.0f, 1.0f);
            const float dt = config.fixedStepSeconds;

            speed += throttle * 6.5f * dt;
            if (brake > 0.0f) {
                const float braking = 10.0f * brake * dt;
                speed = speed > 0.0f ? std::max(0.0f, speed - braking) : std::min(0.0f, speed + braking);
            }
            if (std::abs(throttle) < 0.001f && brake <= 0.0f) {
                const float drag = 2.4f * dt;
                speed = speed > 0.0f ? std::max(0.0f, speed - drag) : std::min(0.0f, speed + drag);
            }
            speed = Clamp(speed, -2.75f, 7.5f);

            if (std::abs(speed) > 0.05f && std::abs(steer) > 0.001f) {
                const float speedFactor = Clamp(std::abs(speed) / 7.5f, 0.38f, 1.0f);
                const float directionSign = speed >= 0.0f ? 1.0f : -1.0f;
                yaw += steer * 2.05f * speedFactor * dt * directionSign;
            }

            position += ForwardFromYaw(yaw) * (speed * dt);
            position.y = config.spawnPosition.y;
            everMoved = everMoved || std::abs(speed) > 0.1f;
            anyOutOfBounds = anyOutOfBounds || OutOfBounds(position, config);
        }

        VehicleProbeSample sample;
        sample.name = input.name;
        sample.position = position;
        sample.yawRadians = yaw;
        sample.speed = speed;
        sample.wheelContactCount = 4;
        sample.maxPitchDegrees = 0.0f;
        sample.maxRollDegrees = 0.0f;
        sample.outOfBounds = OutOfBounds(position, config);
        sample.passed = !sample.outOfBounds;
        sample.message = sample.passed ? "Simple baseline stayed within authored vehicle bounds." : "Simple baseline left authored vehicle bounds.";
        result.samples.push_back(std::move(sample));
        result.inputFrameCount += input.frameCount;
    }

    result.finalPosition = position;
    result.finalYawRadians = yaw;
    result.finalSpeed = speed;
    result.passed = !inputScript.empty() && everMoved && !anyOutOfBounds;
    result.recommendation = result.passed ? "promote" : "defer";
    result.recommendationReason = result.passed
        ? "The dependency-free baseline stayed stable for the scripted vehicle probe."
        : "The dependency-free baseline did not produce a stable vehicle probe.";
    if (!result.passed) {
        result.error = "Vehicle feasibility probe failed in the dependency-free baseline.";
    }
    return result;
}

} // namespace

std::vector<VehicleProbeInputFrame> DefaultVehicleFeasibilityInputScript()
{
    return {
        {"settle", 60, 0.0f, 0.0f, 0.0f},
        {"accelerate", 50, 0.65f, 0.0f, 0.0f},
        {"steer", 40, 0.25f, 0.25f, 0.0f},
        {"brake", 60, 0.0f, 0.0f, 1.0f},
        {"reverse", 40, -0.35f, -0.20f, 0.0f},
    };
}

VehicleProbeResult RunJoltVehicleFeasibilityProbeIfAvailable(
    const VehicleProbeConfig& config,
    const std::vector<VehicleProbeInputFrame>& inputScript);

VehicleProbeResult RunVehicleFeasibilityProbe(
    const VehicleProbeConfig& config,
    const std::vector<VehicleProbeInputFrame>& inputScript,
    PhysicsBackend backend)
{
    if (backend == PhysicsBackend::Simple) {
        return RunSimpleVehicleFeasibilityProbe(config, inputScript);
    }
    return RunJoltVehicleFeasibilityProbeIfAvailable(config, inputScript);
}

#if !ENGINE_WITH_JOLT
VehicleProbeResult RunJoltVehicleFeasibilityProbeIfAvailable(
    const VehicleProbeConfig& config,
    const std::vector<VehicleProbeInputFrame>&)
{
    VehicleProbeResult result;
    result.backendName = "unavailable";
    result.vehicleId = config.vehicleId;
    result.spawnPosition = config.spawnPosition;
    result.halfExtents = config.halfExtents;
    result.recommendation = "defer";
    result.recommendationReason = "The opt-in Jolt backend is not compiled in this build.";
    result.error = "Requested vehicle physics backend is unavailable. Use the opt-in Jolt configure/build preset for vehicle feasibility QA.";
    return result;
}
#endif

} // namespace engine::physics
