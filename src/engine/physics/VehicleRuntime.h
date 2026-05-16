#pragma once

#include "engine/math/Math.h"
#include "engine/physics/PhysicsWorld.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace engine::physics {

struct VehicleRuntimeStaticObstacle {
    std::string name;
    Vec3 center;
    Vec3 halfExtents;
};

struct VehicleRuntimeConfig {
    std::string vehicleId;
    Vec3 spawnPosition;
    float spawnYawRadians = 0.0f;
    Vec3 halfExtents {0.58f, 0.53f, 0.92f};
    Vec2 boundsMin;
    Vec2 boundsMax;
    float fixedStepSeconds = 1.0f / 60.0f;
    std::vector<VehicleRuntimeStaticObstacle> staticObstacles;
};

struct VehicleRuntimeInput {
    float throttle = 0.0f;
    float steer = 0.0f;
    float brake = 0.0f;
};

struct VehicleRuntimeState {
    Vec3 position;
    float yawRadians = 0.0f;
    float speed = 0.0f;
    int wheelContactCount = 0;
    float maxPitchDegrees = 0.0f;
    float maxRollDegrees = 0.0f;
    bool outOfBounds = false;
    int frameIndex = 0;
};

class IVehicleRuntimeAdapter {
public:
    virtual ~IVehicleRuntimeAdapter() = default;

    virtual bool initialize(const VehicleRuntimeConfig& config) = 0;
    virtual void shutdown() = 0;
    virtual bool step(const VehicleRuntimeInput& input, float deltaSeconds) = 0;
    virtual VehicleRuntimeState state() const = 0;
    virtual std::string_view backendName() const = 0;
    virtual std::string_view error() const = 0;
};

std::unique_ptr<IVehicleRuntimeAdapter> CreateVehicleRuntimeAdapter(PhysicsBackend backend);
std::string_view VehicleRuntimeRequestName(PhysicsBackend backend, bool adapterEnabled);

} // namespace engine::physics
