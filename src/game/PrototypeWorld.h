#pragma once

#include "engine/math/Math.h"

#include <string>
#include <string_view>
#include <vector>

struct Aabb {
    engine::Vec3 center;
    engine::Vec3 halfExtents;

    engine::Vec3 min() const;
    engine::Vec3 max() const;
    bool overlaps(engine::Vec3 otherCenter, engine::Vec3 otherHalfExtents) const;
    engine::Vec3 closestPoint(engine::Vec3 point) const;
};

struct StaticCollider {
    int id = 0;
    std::string name;
    Aabb bounds;
    bool blocksPlayer = true;
};

struct PlayerCollisionProxy {
    engine::Vec3 previousPosition;
    engine::Vec3 position;
    engine::Vec3 velocity;
    float radius = 0.35f;
    float height = 1.8f;
};

struct CollisionResult {
    engine::Vec3 position;
    engine::Vec3 velocity;
    engine::Vec3 lastPush;
    engine::Vec3 lastNormal;
    bool grounded = false;
    int hitCount = 0;
};

struct RaycastHit {
    bool hit = false;
    float distance = 0.0f;
    engine::Vec3 point;
    engine::Vec3 normal;
    int colliderId = 0;
    std::string colliderName;
};

class PrototypeWorld {
public:
    void clear();
    void setFloorHeight(float floorHeight);
    int addBox(std::string name, engine::Vec3 center, engine::Vec3 halfExtents);
    void buildDefaultCollisionTestLayout();
    void buildFerryOfficePrototypeLayout();
    bool setColliderBlocksPlayer(std::string_view name, bool blocksPlayer);

    CollisionResult resolvePlayer(const PlayerCollisionProxy& proxy) const;
    bool playerOverlapsCollider(engine::Vec3 position, float radius, float height, const StaticCollider& collider) const;
    bool isGrounded(engine::Vec3 position, float probeDistance = 0.05f) const;
    RaycastHit raycast(engine::Vec3 origin, engine::Vec3 direction, float maxDistance) const;

    const std::vector<StaticCollider>& colliders() const;
    const StaticCollider* colliderByName(std::string_view name) const;
    float floorHeight() const;

private:
    engine::Vec3 resolveCollider(const PlayerCollisionProxy& proxy, const StaticCollider& collider, CollisionResult& result) const;

    float m_floorHeight = 0.0f;
    int m_nextColliderId = 1;
    std::vector<StaticCollider> m_colliders;
};
