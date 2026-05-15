#include "game/ThirdPersonCamera.h"

#include <cmath>

void ThirdPersonCamera::setSettings(const ThirdPersonCameraSettings& settings)
{
    m_settings = settings;
    m_state.distance = settings.distance;
}

void ThirdPersonCamera::update(float deltaSeconds, const engine::InputState& input, const CameraTarget& target)
{
    const float dt = engine::Clamp(deltaSeconds, 0.0f, 0.1f);
    m_state.yawRadians += input.cameraYawDelta * m_settings.yawSensitivity;
    if (m_settings.targetYawFollowStrength > 0.0f) {
        const float yawDelta = std::atan2(std::sin(target.yawRadians - m_state.yawRadians),
            std::cos(target.yawRadians - m_state.yawRadians));
        const float yawFollow = engine::ExponentialSmoothingFactor(m_settings.targetYawFollowStrength, dt);
        m_state.yawRadians += yawDelta * yawFollow;
    }
    m_state.pitchRadians += input.cameraPitchDelta * m_settings.pitchSensitivity;
    m_state.pitchRadians = engine::Clamp(m_state.pitchRadians, m_settings.minPitchRadians, m_settings.maxPitchRadians);
    m_state.distance = m_settings.distance;
    m_state.target = target.position + engine::Vec3 {0.0f, m_settings.heightOffset, 0.0f};

    const engine::Vec3 desiredPosition = desiredPositionForTarget(target);
    if (!m_initialized) {
        m_state.position = desiredPosition;
        m_initialized = true;
    } else {
        const float smoothing = engine::ExponentialSmoothingFactor(m_settings.smoothing, dt);
        m_state.position = engine::Lerp(m_state.position, desiredPosition, smoothing);
    }
}

const ThirdPersonCameraState& ThirdPersonCamera::state() const
{
    return m_state;
}

const ThirdPersonCameraSettings& ThirdPersonCamera::settings() const
{
    return m_settings;
}

engine::DebugCamera ThirdPersonCamera::debugCamera() const
{
    engine::DebugCamera camera;
    camera.position = m_state.position;
    camera.target = m_state.target;
    camera.fovYRadians = engine::Radians(60.0f);
    camera.nearPlane = 0.05f;
    return camera;
}

engine::Vec3 ThirdPersonCamera::desiredPositionForTarget(const CameraTarget& target) const
{
    const engine::Vec3 targetPoint = target.position + engine::Vec3 {0.0f, m_settings.heightOffset, 0.0f};
    const float cosPitch = std::cos(m_state.pitchRadians);
    const engine::Vec3 offset {
        -std::sin(m_state.yawRadians) * cosPitch * m_settings.distance,
        std::sin(m_state.pitchRadians) * m_settings.distance,
        -std::cos(m_state.yawRadians) * cosPitch * m_settings.distance,
    };

    return targetPoint + offset;
}
