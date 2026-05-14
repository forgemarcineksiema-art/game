#include "game/PlayerController.h"

#include "game/TestWorld.h"

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

void PlayerController::setWorld(const TestWorld* world)
{
    m_world = world;
}

void PlayerController::update(float deltaSeconds, const engine::InputState& input, float cameraYawRadians)
{
    const float dt = engine::Clamp(deltaSeconds, 0.0f, 0.1f);
    const engine::Vec3 previousPosition = m_state.position;
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

    if (m_world) {
        PlayerCollisionProxy proxy;
        proxy.previousPosition = previousPosition;
        proxy.position = m_state.position;
        proxy.velocity = m_state.velocity;
        proxy.radius = m_settings.radius;
        proxy.height = m_settings.height;

        const CollisionResult collision = m_world->resolvePlayer(proxy);
        m_state.position = collision.position;
        m_state.velocity = collision.velocity;
        m_state.grounded = collision.grounded;
        m_state.lastCollisionPush = collision.lastPush;
        m_state.lastCollisionNormal = collision.lastNormal;
        m_state.lastCollisionHitCount = collision.hitCount;
    } else if (m_state.position.y <= 0.0f) {
        m_state.position.y = 0.0f;
        m_state.velocity.y = 0.0f;
        m_state.grounded = true;
        m_state.lastCollisionPush = {};
        m_state.lastCollisionNormal = {};
        m_state.lastCollisionHitCount = 0;
    }
}

const PlayerState& PlayerController::state() const
{
    return m_state;
}

const PlayerControllerSettings& PlayerController::settings() const
{
    return m_settings;
}
