#pragma once

#include "engine/input/Input.h"
#include "engine/math/Math.h"

#include <vector>

struct PlayerObstacle {
    engine::Vec3 center;
    engine::Vec3 halfExtents;
};

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
    float facingYawRadians = 0.0f;
    bool grounded = true;
    bool sprinting = false;
    float horizontalSpeed = 0.0f;
};

class PlayerController {
public:
    void setSettings(const PlayerControllerSettings& settings);
    void setPosition(engine::Vec3 position);
    void setObstacles(std::vector<PlayerObstacle> obstacles);
    void update(float deltaSeconds, const engine::InputState& input, float cameraYawRadians);

    const PlayerState& state() const;
    const PlayerControllerSettings& settings() const;

private:
    void resolveObstacleCollisions();

    PlayerControllerSettings m_settings;
    PlayerState m_state;
    std::vector<PlayerObstacle> m_obstacles;
};
