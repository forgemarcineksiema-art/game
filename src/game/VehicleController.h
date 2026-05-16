#pragma once

#include "engine/input/Input.h"
#include "engine/math/Math.h"
#include "game/ThirdPersonCamera.h"

#include <string>

struct VehicleControllerSettings {
    float maxForwardSpeed = 7.5f;
    float maxReverseSpeed = 2.75f;
    float acceleration = 6.5f;
    float braking = 10.0f;
    float drag = 2.4f;
    float steeringRate = 2.05f;
    float minSteeringSpeedFactor = 0.38f;
    float enterRadius = 1.8f;
    float closeEnterRadius = 0.8f;
    float requiredFacingDot = 0.15f;
    float exitSideOffset = 1.65f;
    float exitBackOffset = 0.35f;
    float cameraLookAhead = 0.85f;
    bool clampToBounds = true;
    float boundsMinX = -9.5f;
    float boundsMaxX = 9.5f;
    float boundsMinZ = -7.5f;
    float boundsMaxZ = 8.5f;
};

struct VehicleFocus {
    bool canEnter = false;
    float distance = 0.0f;
    std::string prompt;
};

struct VehicleState {
    engine::Vec3 position;
    engine::Vec3 velocity;
    float yawRadians = 0.0f;
    float speed = 0.0f;
    float throttle = 0.0f;
    float brake = 0.0f;
    float steer = 0.0f;
    bool occupied = false;
    bool enteredThisFrame = false;
    bool exitedThisFrame = false;
    bool exitBlockedThisFrame = false;
    bool hitBoundsThisFrame = false;
};

class VehicleController {
public:
    void setSettings(const VehicleControllerSettings& settings);
    void setPosition(engine::Vec3 position);
    void setYawRadians(float yawRadians);
    void setOccupiedForTesting(bool occupied);
    void applyRuntimeState(
        engine::Vec3 position,
        float yawRadians,
        float speed,
        float throttle,
        float brake,
        float steer,
        bool hitBounds);

    void beginFrame();
    VehicleFocus updateFocus(engine::Vec3 playerPosition, engine::Vec3 playerFacing);
    bool tryEnter(const engine::InputState& input);
    bool tryExit(const engine::InputState& input, bool exitPositionClear);
    void updateDriving(float deltaSeconds, const engine::InputState& input);

    engine::Vec3 forward() const;
    engine::Vec3 right() const;
    engine::Vec3 exitPosition() const;
    CameraTarget cameraTarget() const;

    const VehicleState& state() const;
    const VehicleControllerSettings& settings() const;
    const VehicleFocus& focus() const;

private:
    void applyDrag(float deltaSeconds);
    void clampToYardBounds();

    VehicleControllerSettings m_settings;
    VehicleState m_state;
    VehicleFocus m_focus;
};
