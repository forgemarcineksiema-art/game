#pragma once

#include "engine/input/Input.h"
#include "engine/math/Math.h"
#include "game/ThirdPersonCamera.h"

#include <string>

struct VehicleControllerSettings {
    float maxForwardSpeed = 12.0f;
    float maxReverseSpeed = 4.0f;
    float acceleration = 8.0f;
    float braking = 12.0f;
    float drag = 1.8f;
    float steeringRate = 1.65f;
    float enterRadius = 1.8f;
    float closeEnterRadius = 0.8f;
    float requiredFacingDot = 0.15f;
    float exitSideOffset = 1.65f;
    float exitBackOffset = 0.35f;
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
