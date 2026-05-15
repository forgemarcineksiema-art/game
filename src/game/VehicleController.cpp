#include "game/VehicleController.h"

#include <algorithm>
#include <cmath>

void VehicleController::setSettings(const VehicleControllerSettings& settings)
{
    m_settings = settings;
}

void VehicleController::setPosition(engine::Vec3 position)
{
    m_state.position = position;
    m_state.position.y = 0.0f;
}

void VehicleController::setYawRadians(float yawRadians)
{
    m_state.yawRadians = yawRadians;
}

void VehicleController::setOccupiedForTesting(bool occupied)
{
    m_state.occupied = occupied;
}

void VehicleController::beginFrame()
{
    m_state.enteredThisFrame = false;
    m_state.exitedThisFrame = false;
    m_state.exitBlockedThisFrame = false;
    m_state.hitBoundsThisFrame = false;
}

VehicleFocus VehicleController::updateFocus(engine::Vec3 playerPosition, engine::Vec3 playerFacing)
{
    m_focus = {};
    if (m_state.occupied) {
        return m_focus;
    }

    const engine::Vec3 toVehicle = m_state.position - playerPosition;
    const engine::Vec3 horizontalToVehicle {toVehicle.x, 0.0f, toVehicle.z};
    const float distance = engine::Length(horizontalToVehicle);
    m_focus.distance = distance;

    if (distance > m_settings.enterRadius) {
        return m_focus;
    }

    const engine::Vec3 facing = engine::Normalize({playerFacing.x, 0.0f, playerFacing.z});
    const engine::Vec3 directionToVehicle = engine::Normalize(horizontalToVehicle);
    const float facingScore = engine::Dot(facing, directionToVehicle);
    const bool closeEnoughFallback = distance <= m_settings.closeEnterRadius;
    if (closeEnoughFallback || facingScore >= m_settings.requiredFacingDot) {
        m_focus.canEnter = true;
        m_focus.prompt = "Press E: Enter Service Yard Vehicle";
    }

    return m_focus;
}

bool VehicleController::tryEnter(const engine::InputState& input)
{
    if (m_state.occupied || !m_focus.canEnter || !input.interactPressed) {
        return false;
    }

    m_state.occupied = true;
    m_state.enteredThisFrame = true;
    m_state.throttle = 0.0f;
    m_state.brake = 0.0f;
    m_state.steer = 0.0f;
    m_focus = {};
    return true;
}

bool VehicleController::tryExit(const engine::InputState& input, bool exitPositionClear)
{
    if (!m_state.occupied || !input.interactPressed) {
        return false;
    }

    if (!exitPositionClear) {
        m_state.exitBlockedThisFrame = true;
        return false;
    }

    m_state.occupied = false;
    m_state.exitedThisFrame = true;
    m_state.speed = 0.0f;
    m_state.velocity = {};
    m_state.throttle = 0.0f;
    m_state.brake = 0.0f;
    m_state.steer = 0.0f;
    return true;
}

void VehicleController::updateDriving(float deltaSeconds, const engine::InputState& input)
{
    const float dt = engine::Clamp(deltaSeconds, 0.0f, 0.1f);
    m_state.throttle = 0.0f;
    m_state.brake = 0.0f;
    m_state.steer = m_state.occupied ? engine::Clamp(input.moveRight, -1.0f, 1.0f) : 0.0f;

    if (!m_state.occupied) {
        applyDrag(dt);
        m_state.velocity = forward() * m_state.speed;
        return;
    }

    const float driveInput = engine::Clamp(input.moveForward, -1.0f, 1.0f);
    if (driveInput > 0.0f) {
        m_state.throttle = driveInput;
        m_state.speed += m_settings.acceleration * driveInput * dt;
    } else if (driveInput < 0.0f) {
        if (m_state.speed > 0.05f) {
            m_state.brake = -driveInput;
            m_state.speed -= m_settings.braking * (-driveInput) * dt;
            if (m_state.speed < 0.0f) {
                m_state.speed = 0.0f;
            }
        } else {
            m_state.throttle = driveInput;
            m_state.speed += m_settings.acceleration * 0.65f * driveInput * dt;
        }
    } else {
        applyDrag(dt);
    }

    m_state.speed = engine::Clamp(m_state.speed, -m_settings.maxReverseSpeed, m_settings.maxForwardSpeed);

    const float absoluteSpeed = std::abs(m_state.speed);
    if (absoluteSpeed > 0.05f && std::abs(m_state.steer) > 0.001f) {
        const float speedFactor = engine::Clamp(absoluteSpeed / std::max(0.001f, m_settings.maxForwardSpeed),
            m_settings.minSteeringSpeedFactor,
            1.0f);
        const float directionSign = m_state.speed >= 0.0f ? 1.0f : -1.0f;
        m_state.yawRadians += m_state.steer * m_settings.steeringRate * speedFactor * dt * directionSign;
    }

    m_state.position += forward() * (m_state.speed * dt);
    m_state.position.y = 0.0f;
    if (m_settings.clampToBounds) {
        clampToYardBounds();
    }
    m_state.velocity = forward() * m_state.speed;
}

engine::Vec3 VehicleController::forward() const
{
    return engine::ForwardFromYaw(m_state.yawRadians);
}

engine::Vec3 VehicleController::right() const
{
    return engine::RightFromYaw(m_state.yawRadians);
}

engine::Vec3 VehicleController::exitPosition() const
{
    return m_state.position - right() * m_settings.exitSideOffset - forward() * m_settings.exitBackOffset;
}

CameraTarget VehicleController::cameraTarget() const
{
    CameraTarget target;
    target.position = m_state.position + forward() * m_settings.cameraLookAhead;
    target.yawRadians = m_state.yawRadians;
    return target;
}

const VehicleState& VehicleController::state() const
{
    return m_state;
}

const VehicleControllerSettings& VehicleController::settings() const
{
    return m_settings;
}

const VehicleFocus& VehicleController::focus() const
{
    return m_focus;
}

void VehicleController::applyDrag(float deltaSeconds)
{
    const float dragAmount = m_settings.drag * deltaSeconds;
    if (m_state.speed > 0.0f) {
        m_state.speed = std::max(0.0f, m_state.speed - dragAmount);
    } else if (m_state.speed < 0.0f) {
        m_state.speed = std::min(0.0f, m_state.speed + dragAmount);
    }
}

void VehicleController::clampToYardBounds()
{
    const engine::Vec3 before = m_state.position;
    m_state.position.x = engine::Clamp(m_state.position.x, m_settings.boundsMinX, m_settings.boundsMaxX);
    m_state.position.z = engine::Clamp(m_state.position.z, m_settings.boundsMinZ, m_settings.boundsMaxZ);
    if (before.x != m_state.position.x || before.z != m_state.position.z) {
        m_state.hitBoundsThisFrame = true;
        m_state.speed = 0.0f;
    }
}
