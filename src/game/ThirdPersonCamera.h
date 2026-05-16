#pragma once

#include "engine/input/Input.h"
#include "engine/math/Math.h"
#include "engine/renderer/RendererTypes.h"

struct ThirdPersonCameraSettings {
    float distance = 6.0f;
    float heightOffset = 1.6f;
    float minPitchRadians = engine::Radians(-35.0f);
    float maxPitchRadians = engine::Radians(55.0f);
    float yawSensitivity = 0.004f;
    float pitchSensitivity = 0.004f;
    float smoothing = 12.0f;
    float targetYawFollowStrength = 0.0f;
};

struct CameraTarget {
    engine::Vec3 position;
    float yawRadians = 0.0f;
};

struct ThirdPersonCameraState {
    engine::Vec3 position = {0.0f, 3.0f, -6.0f};
    engine::Vec3 target = {0.0f, 1.6f, 0.0f};
    float yawRadians = 0.0f;
    float pitchRadians = engine::Radians(22.0f);
    float distance = 6.0f;
};

class ThirdPersonCamera {
public:
    void setSettings(const ThirdPersonCameraSettings& settings);
    void setYawRadians(float yawRadians);
    void update(float deltaSeconds, const engine::InputState& input, const CameraTarget& target);

    const ThirdPersonCameraState& state() const;
    const ThirdPersonCameraSettings& settings() const;
    engine::DebugCamera debugCamera() const;

private:
    engine::Vec3 desiredPositionForTarget(const CameraTarget& target) const;

    ThirdPersonCameraSettings m_settings;
    ThirdPersonCameraState m_state;
    bool m_initialized = false;
};
