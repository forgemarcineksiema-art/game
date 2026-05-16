#pragma once

#include "engine/math/Math.h"
#include "engine/physics/PhysicsWorld.h"

#include <filesystem>
#include <string>
#include <vector>

namespace engine::physics {

struct VehicleProbeConfig {
    std::string vehicleId;
    Vec3 spawnPosition;
    float spawnYawRadians = 0.0f;
    Vec3 halfExtents {0.58f, 0.53f, 0.92f};
    Vec2 boundsMin;
    Vec2 boundsMax;
    float fixedStepSeconds = 1.0f / 60.0f;
};

struct VehicleProbeInputFrame {
    std::string name;
    int frameCount = 0;
    float throttle = 0.0f;
    float steer = 0.0f;
    float brake = 0.0f;
};

struct VehicleProbeSample {
    std::string name;
    bool passed = false;
    Vec3 position;
    float yawRadians = 0.0f;
    float speed = 0.0f;
    int wheelContactCount = 0;
    float maxPitchDegrees = 0.0f;
    float maxRollDegrees = 0.0f;
    bool outOfBounds = false;
    std::string message;
};

struct VehicleProbeResult {
    bool passed = false;
    std::string backendName = "unavailable";
    std::string vehicleId;
    Vec3 spawnPosition;
    Vec3 halfExtents;
    int inputFrameCount = 0;
    std::vector<VehicleProbeSample> samples;
    Vec3 finalPosition;
    float finalYawRadians = 0.0f;
    float finalSpeed = 0.0f;
    std::string recommendation = "defer";
    std::string recommendationReason;
    std::string error;
};

std::vector<VehicleProbeInputFrame> DefaultVehicleFeasibilityInputScript();

VehicleProbeResult RunVehicleFeasibilityProbe(
    const VehicleProbeConfig& config,
    const std::vector<VehicleProbeInputFrame>& inputScript,
    PhysicsBackend backend = OptInPhysicsBackend());

} // namespace engine::physics
