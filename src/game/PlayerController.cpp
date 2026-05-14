#include "game/PlayerController.h"

#include <algorithm>
#include <cmath>

void PlayerController::setSettings(const PlayerControllerSettings& settings)
{
    m_settings = settings;
}

void PlayerController::setPosition(engine::Vec3 position)
{
    m_state.position = position;
    if (m_state.position.y <= 0.0f) {
        m_state.position.y = 0.0f;
        m_state.grounded = true;
        m_state.velocity.y = 0.0f;
    }
}

void PlayerController::setObstacles(std::vector<PlayerObstacle> obstacles)
{
    m_obstacles = std::move(obstacles);
}

void PlayerController::update(float deltaSeconds, const engine::InputState& input, float cameraYawRadians)
{
    const float dt = engine::Clamp(deltaSeconds, 0.0f, 0.1f);
    const engine::Vec2 inputAxis = engine::Normalize(engine::Vec2 {input.moveRight, input.moveForward});
    const engine::Vec3 forward = engine::ForwardFromYaw(cameraYawRadians);
    const engine::Vec3 right = engine::RightFromYaw(cameraYawRadians);
    const engine::Vec3 moveDirection = engine::Normalize(right * inputAxis.x + forward * inputAxis.y);

    m_state.sprinting = input.sprintHeld && engine::Length(inputAxis) > 0.0f;
    const float targetSpeed = m_state.sprinting ? m_settings.sprintSpeed : m_settings.walkSpeed;
    const engine::Vec3 horizontalVelocity = moveDirection * targetSpeed;

    m_state.velocity.x = horizontalVelocity.x;
    m_state.velocity.z = horizontalVelocity.z;
    m_state.horizontalSpeed = engine::Length(engine::Vec2 {m_state.velocity.x, m_state.velocity.z});

    if (engine::Length(moveDirection) > 0.0f) {
        m_state.facingYawRadians = engine::YawFromDirection(moveDirection);
    }

    if (m_state.grounded && input.jumpPressed) {
        m_state.velocity.y = m_settings.jumpImpulse;
        m_state.grounded = false;
    }

    if (!m_state.grounded) {
        m_state.velocity.y -= m_settings.gravity * dt;
    }

    m_state.position += m_state.velocity * dt;

    if (m_state.position.y <= 0.0f) {
        m_state.position.y = 0.0f;
        m_state.velocity.y = 0.0f;
        m_state.grounded = true;
    }

    resolveObstacleCollisions();
}

const PlayerState& PlayerController::state() const
{
    return m_state;
}

const PlayerControllerSettings& PlayerController::settings() const
{
    return m_settings;
}

void PlayerController::resolveObstacleCollisions()
{
    for (const PlayerObstacle& obstacle : m_obstacles) {
        const float minX = obstacle.center.x - obstacle.halfExtents.x - m_settings.radius;
        const float maxX = obstacle.center.x + obstacle.halfExtents.x + m_settings.radius;
        const float minZ = obstacle.center.z - obstacle.halfExtents.z - m_settings.radius;
        const float maxZ = obstacle.center.z + obstacle.halfExtents.z + m_settings.radius;
        const float top = obstacle.center.y + obstacle.halfExtents.y;

        if (m_state.position.y > top + 0.1f) {
            continue;
        }

        if (m_state.position.x < minX || m_state.position.x > maxX || m_state.position.z < minZ || m_state.position.z > maxZ) {
            continue;
        }

        const float pushLeft = std::abs(m_state.position.x - minX);
        const float pushRight = std::abs(maxX - m_state.position.x);
        const float pushBack = std::abs(m_state.position.z - minZ);
        const float pushForward = std::abs(maxZ - m_state.position.z);
        const float minPush = std::min({pushLeft, pushRight, pushBack, pushForward});

        if (minPush == pushLeft) {
            m_state.position.x = minX;
        } else if (minPush == pushRight) {
            m_state.position.x = maxX;
        } else if (minPush == pushBack) {
            m_state.position.z = minZ;
        } else {
            m_state.position.z = maxZ;
        }
    }
}
