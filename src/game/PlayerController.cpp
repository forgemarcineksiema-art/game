#include "game/PlayerController.h"

#include "game/PrototypeWorld.h"

#include <cmath>

namespace {

constexpr float TraversalArcHeight = 0.55f;

float SmoothStep(float value)
{
    return value * value * (3.0f - 2.0f * value);
}

} // namespace

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

void PlayerController::setFacingYawRadians(float yawRadians)
{
    m_state.facingYawRadians = yawRadians;
}

void PlayerController::setWorld(const PrototypeWorld* world)
{
    m_world = world;
}

void PlayerController::update(float deltaSeconds, const engine::InputState& input, float cameraYawRadians, const TraversalActivation* traversalActivation)
{
    const float dt = engine::Clamp(deltaSeconds, 0.0f, 0.1f);
    m_state.traversalLandedThisFrame = false;
    m_state.landedTraversalId = 0;
    if (m_state.traversalMode == PlayerTraversalMode::Normal && traversalActivation && traversalActivation->started) {
        m_activeTraversal = *traversalActivation;
        if (m_activeTraversal.useCurrentPlayerPositionAsStart) {
            m_activeTraversal.startPosition = m_state.position;
        }
        if (m_activeTraversal.durationSeconds <= 0.0001f) {
            m_activeTraversal.durationSeconds = 0.0001f;
        }
        m_traversalElapsedSeconds = 0.0f;
        m_state.traversalMode = PlayerTraversalMode::Traversing;
        m_state.activeTraversalId = m_activeTraversal.affordanceId;
        m_state.traversalProgress = 0.0f;
        m_state.traversalStartPosition = m_activeTraversal.startPosition;
        m_state.traversalTargetPosition = m_activeTraversal.endPosition;
        m_state.traversalUsesCurrentPlayerPositionStart = m_activeTraversal.useCurrentPlayerPositionAsStart;
        m_state.velocity = {};
        m_state.horizontalSpeed = 0.0f;
        m_state.sprinting = false;
        m_state.grounded = false;
        m_state.lastCollisionPush = {};
        m_state.lastCollisionNormal = {};
        m_state.lastCollisionHitCount = 0;
    }

    if (m_state.traversalMode == PlayerTraversalMode::Traversing) {
        m_traversalElapsedSeconds += dt;
        const float rawProgress = engine::Clamp(m_traversalElapsedSeconds / m_activeTraversal.durationSeconds, 0.0f, 1.0f);
        const float smoothedProgress = SmoothStep(rawProgress);
        engine::Vec3 position = engine::Lerp(m_activeTraversal.startPosition, m_activeTraversal.endPosition, smoothedProgress);
        position.y += std::sin(engine::Pi * rawProgress) * TraversalArcHeight;

        m_state.position = position;
        m_state.velocity = {};
        m_state.horizontalSpeed = 0.0f;
        m_state.sprinting = false;
        m_state.traversalProgress = rawProgress;

        if (rawProgress >= 1.0f) {
            const engine::Vec3 landingPosition = m_activeTraversal.endPosition;
            m_state.position = landingPosition;
            m_state.velocity = {};
            if (m_world) {
                PlayerCollisionProxy proxy;
                proxy.previousPosition = m_activeTraversal.startPosition;
                proxy.position = landingPosition;
                proxy.velocity = {};
                proxy.radius = m_settings.radius;
                proxy.height = m_settings.height;

                const CollisionResult collision = m_world->resolvePlayer(proxy);
                m_state.position = collision.position;
                m_state.velocity = collision.velocity;
                m_state.grounded = collision.grounded;
                m_state.lastCollisionPush = collision.lastPush;
                m_state.lastCollisionNormal = collision.lastNormal;
                m_state.lastCollisionHitCount = collision.hitCount;
            } else {
                if (m_state.position.y <= 0.0f) {
                    m_state.position.y = 0.0f;
                }
                m_state.grounded = true;
                m_state.lastCollisionPush = {};
                m_state.lastCollisionNormal = {};
                m_state.lastCollisionHitCount = 0;
            }
            m_state.traversalMode = PlayerTraversalMode::Normal;
            m_state.traversalProgress = 1.0f;
            m_state.traversalLandedThisFrame = true;
            m_state.landedTraversalId = m_activeTraversal.affordanceId;
            m_state.activeTraversalId = 0;
            m_activeTraversal = {};
            m_traversalElapsedSeconds = 0.0f;
        }
        return;
    }

    m_state.traversalProgress = 0.0f;
    m_state.activeTraversalId = 0;
    m_state.traversalUsesCurrentPlayerPositionStart = false;
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
