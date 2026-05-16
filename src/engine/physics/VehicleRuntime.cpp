#include "engine/physics/VehicleRuntime.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace engine::physics {
namespace {

bool OutOfBounds(Vec3 position, const VehicleRuntimeConfig& config)
{
    return position.x < config.boundsMin.x || position.x > config.boundsMax.x
        || position.z < config.boundsMin.y || position.z > config.boundsMax.y;
}

bool OverlapsObstacle(Vec3 position, Vec3 halfExtents, const VehicleRuntimeStaticObstacle& obstacle)
{
    return std::abs(position.x - obstacle.center.x) < (halfExtents.x + obstacle.halfExtents.x)
        && std::abs(position.z - obstacle.center.z) < (halfExtents.z + obstacle.halfExtents.z);
}

Vec3 ResolveStaticObstacleOverlap(Vec3 position, Vec3 halfExtents, const VehicleRuntimeStaticObstacle& obstacle)
{
    if (!OverlapsObstacle(position, halfExtents, obstacle)) {
        return position;
    }

    const float overlapX = (halfExtents.x + obstacle.halfExtents.x) - std::abs(position.x - obstacle.center.x);
    const float overlapZ = (halfExtents.z + obstacle.halfExtents.z) - std::abs(position.z - obstacle.center.z);
    if (overlapX < overlapZ) {
        const float direction = position.x < obstacle.center.x ? -1.0f : 1.0f;
        position.x += direction * (overlapX + 0.001f);
    } else {
        const float direction = position.z < obstacle.center.z ? -1.0f : 1.0f;
        position.z += direction * (overlapZ + 0.001f);
    }
    return position;
}

class SimpleVehicleRuntimeAdapter final : public IVehicleRuntimeAdapter {
public:
    bool initialize(const VehicleRuntimeConfig& config) override
    {
        m_config = config;
        m_state = {};
        m_state.position = config.spawnPosition;
        m_state.yawRadians = config.spawnYawRadians;
        m_state.wheelContactCount = 4;
        m_initialized = true;
        return true;
    }

    void shutdown() override
    {
        m_initialized = false;
    }

    bool step(const VehicleRuntimeInput& input, float deltaSeconds) override
    {
        if (!m_initialized) {
            m_error = "Simple vehicle runtime adapter was not initialized.";
            return false;
        }

        const float dt = Clamp(deltaSeconds, 0.0f, 0.1f);
        const float throttle = Clamp(input.throttle, -1.0f, 1.0f);
        const float brake = Clamp(input.brake, 0.0f, 1.0f);
        const float steer = Clamp(input.steer, -1.0f, 1.0f);

        const float driveInput = brake > 0.0f ? -brake : throttle;
        if (driveInput > 0.0f) {
            m_state.speed += 6.5f * driveInput * dt;
        } else if (driveInput < 0.0f) {
            if (m_state.speed > 0.05f) {
                m_state.speed -= 10.0f * (-driveInput) * dt;
                if (m_state.speed < 0.0f) {
                    m_state.speed = 0.0f;
                }
            } else {
                m_state.speed += 6.5f * 0.65f * driveInput * dt;
            }
        } else {
            const float drag = 2.4f * dt;
            m_state.speed = m_state.speed > 0.0f
                ? std::max(0.0f, m_state.speed - drag)
                : std::min(0.0f, m_state.speed + drag);
        }
        m_state.speed = Clamp(m_state.speed, -2.75f, 7.5f);

        if (std::abs(m_state.speed) > 0.05f && std::abs(steer) > 0.001f) {
            const float speedFactor = Clamp(std::abs(m_state.speed) / 7.5f, 0.38f, 1.0f);
            const float directionSign = m_state.speed >= 0.0f ? 1.0f : -1.0f;
            m_state.yawRadians += steer * 2.05f * speedFactor * dt * directionSign;
        }

        const Vec3 previousPosition = m_state.position;
        m_state.position += ForwardFromYaw(m_state.yawRadians) * (m_state.speed * dt);
        bool hitObstacle = false;
        for (const VehicleRuntimeStaticObstacle& obstacle : m_config.staticObstacles) {
            const Vec3 resolved = ResolveStaticObstacleOverlap(m_state.position, m_config.halfExtents, obstacle);
            hitObstacle = hitObstacle || resolved.x != m_state.position.x || resolved.z != m_state.position.z;
            m_state.position = resolved;
        }
        if (hitObstacle) {
            const Vec3 movement = m_state.position - previousPosition;
            const float travel = Length(Vec2 {movement.x, movement.z});
            m_state.speed = std::min(std::abs(m_state.speed), travel / std::max(dt, 0.001f)) * (m_state.speed < 0.0f ? -0.15f : 0.15f);
        }
        m_state.position.y = m_config.spawnPosition.y;
        m_state.outOfBounds = OutOfBounds(m_state.position, m_config);
        m_state.wheelContactCount = 4;
        m_state.maxPitchDegrees = 0.0f;
        m_state.maxRollDegrees = 0.0f;
        m_state.frameIndex += 1;
        return true;
    }

    VehicleRuntimeState state() const override
    {
        return m_state;
    }

    std::string_view backendName() const override
    {
        return "simple";
    }

    std::string_view error() const override
    {
        return m_error;
    }

private:
    VehicleRuntimeConfig m_config;
    VehicleRuntimeState m_state;
    std::string m_error;
    bool m_initialized = false;
};

} // namespace

std::unique_ptr<IVehicleRuntimeAdapter> CreateJoltVehicleRuntimeAdapterIfAvailable();

std::unique_ptr<IVehicleRuntimeAdapter> CreateVehicleRuntimeAdapter(PhysicsBackend backend)
{
    if (backend == PhysicsBackend::Simple) {
        return std::make_unique<SimpleVehicleRuntimeAdapter>();
    }
    return CreateJoltVehicleRuntimeAdapterIfAvailable();
}

std::string_view VehicleRuntimeRequestName(PhysicsBackend backend, bool adapterEnabled)
{
    if (!adapterEnabled || backend == PhysicsBackend::Simple) {
        return "deterministic";
    }
    if (backend == PhysicsBackend::Jolt) {
        return "jolt";
    }
    return "unknown";
}

#if !ENGINE_WITH_JOLT
std::unique_ptr<IVehicleRuntimeAdapter> CreateJoltVehicleRuntimeAdapterIfAvailable()
{
    return nullptr;
}
#endif

} // namespace engine::physics
