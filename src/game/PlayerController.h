#pragma once

#include "engine/input/Input.h"
#include "engine/math/Math.h"
#include "game/TraversalSystem.h"

class PrototypeWorld;

struct PlayerControllerSettings {
    float walkSpeed = 4.0f;
    float sprintSpeed = 7.0f;
    float jumpImpulse = 5.5f;
    float gravity = 18.0f;
    float radius = 0.35f;
    float height = 1.8f;
};

enum class PlayerTraversalMode {
    Normal,
    Traversing,
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
    PlayerTraversalMode traversalMode = PlayerTraversalMode::Normal;
    float traversalProgress = 0.0f;
    int activeTraversalId = 0;
    engine::Vec3 traversalStartPosition;
    engine::Vec3 traversalTargetPosition;
    bool traversalUsesCurrentPlayerPositionStart = false;
    bool traversalLandedThisFrame = false;
};

class PlayerController {
public:
    void setSettings(const PlayerControllerSettings& settings);
    void setPosition(engine::Vec3 position);
    void setWorld(const PrototypeWorld* world);
    void update(float deltaSeconds, const engine::InputState& input, float cameraYawRadians, const TraversalActivation* traversalActivation = nullptr);

    const PlayerState& state() const;
    const PlayerControllerSettings& settings() const;

private:
    PlayerControllerSettings m_settings;
    PlayerState m_state;
    const PrototypeWorld* m_world = nullptr;
    TraversalActivation m_activeTraversal;
    float m_traversalElapsedSeconds = 0.0f;
};
