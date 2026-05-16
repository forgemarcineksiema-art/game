#include "engine/physics/VehicleRuntime.h"

#include <algorithm>
#include <cmath>

namespace engine::physics {
namespace {

bool OutOfBounds(Vec3 position, const VehicleRuntimeConfig& config)
{
    return position.x < config.boundsMin.x || position.x > config.boundsMax.x
        || position.z < config.boundsMin.y || position.z > config.boundsMax.y;
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

        m_state.position += ForwardFromYaw(m_state.yawRadians) * (m_state.speed * dt);
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

#if !ENGINE_WITH_JOLT
std::unique_ptr<IVehicleRuntimeAdapter> CreateJoltVehicleRuntimeAdapterIfAvailable()
{
    return nullptr;
}
#endif

} // namespace engine::physics
