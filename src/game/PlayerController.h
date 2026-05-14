#pragma once

#include "engine/input/Input.h"
#include "engine/math/Math.h"

class TestWorld;

struct PlayerControllerSettings {
    float walkSpeed = 4.0f;
    float sprintSpeed = 7.0f;
    float jumpImpulse = 5.5f;
    float gravity = 18.0f;
    float radius = 0.35f;
    float height = 1.8f;
};

struct PlayerState {
    engine::Vec3 position;
    engine::Vec3 velocity;
    engine::Vec3 lastCollisionPush;
    engine::Vec3 lastCollisionNormal;
    float facingYawRadians = 0.0f;
    bool grounded = true;
    bool sprinting = false;
    float horizontalSpeed = 0.0f;
    int lastCollisionHitCount = 0;
};

class PlayerController {
public:
    void setSettings(const PlayerControllerSettings& settings);
    void setPosition(engine::Vec3 position);
    void setWorld(const TestWorld* world);
    void update(float deltaSeconds, const engine::InputState& input, float cameraYawRadians);

    const PlayerState& state() const;
    const PlayerControllerSettings& settings() const;

private:
    PlayerControllerSettings m_settings;
    PlayerState m_state;
    const TestWorld* m_world = nullptr;
};
