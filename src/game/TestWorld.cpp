#include "game/TestWorld.h"

#include <algorithm>
#include <cmath>
#include <limits>

engine::Vec3 Aabb::min() const
{
    return center - halfExtents;
}

engine::Vec3 Aabb::max() const
{
    return center + halfExtents;
}

bool Aabb::overlaps(engine::Vec3 otherCenter, engine::Vec3 otherHalfExtents) const
{
    const engine::Vec3 aMin = min();
    const engine::Vec3 aMax = max();
    const engine::Vec3 bMin = otherCenter - otherHalfExtents;
    const engine::Vec3 bMax = otherCenter + otherHalfExtents;

    return aMin.x <= bMax.x && aMax.x >= bMin.x
        && aMin.y <= bMax.y && aMax.y >= bMin.y
        && aMin.z <= bMax.z && aMax.z >= bMin.z;
}

engine::Vec3 Aabb::closestPoint(engine::Vec3 point) const
{
    const engine::Vec3 aMin = min();
    const engine::Vec3 aMax = max();
    return {
        engine::Clamp(point.x, aMin.x, aMax.x),
        engine::Clamp(point.y, aMin.y, aMax.y),
        engine::Clamp(point.z, aMin.z, aMax.z),
    };
}

void TestWorld::clear()
{
    m_colliders.clear();
    m_nextColliderId = 1;
}

void TestWorld::setFloorHeight(float floorHeight)
{
    m_floorHeight = floorHeight;
}

int TestWorld::addBox(std::string name, engine::Vec3 center, engine::Vec3 halfExtents)
{
    StaticCollider collider;
    collider.id = m_nextColliderId++;
    collider.name = std::move(name);
    collider.bounds.center = center;
    collider.bounds.halfExtents = halfExtents;
    m_colliders.push_back(std::move(collider));
    return m_colliders.back().id;
}

void TestWorld::buildDefaultCollisionTestLayout()
{
    clear();
    setFloorHeight(0.0f);
    addBox("wide-wall", {0.0f, 0.75f, 6.0f}, {3.0f, 0.75f, 0.35f});
    addBox("left-passage-post", {-1.15f, 0.7f, 2.8f}, {0.35f, 0.7f, 1.0f});
    addBox("right-passage-post", {1.15f, 0.7f, 2.8f}, {0.35f, 0.7f, 1.0f});
    addBox("corner-a", {-3.0f, 0.65f, -1.5f}, {1.0f, 0.65f, 0.4f});
    addBox("corner-b", {-4.0f, 0.65f, -0.5f}, {0.4f, 0.65f, 1.0f});
    addBox("low-step-blocker", {2.8f, 0.25f, -1.5f}, {1.0f, 0.25f, 0.5f});
    addBox("test-crate", {3.5f, 0.5f, 2.0f}, {0.6f, 0.5f, 0.6f});
}

void TestWorld::buildFerryOfficePrototypeLayout()
{
    clear();
    setFloorHeight(0.0f);

    addBox("ferry-office-back-wall", {0.0f, 0.75f, 5.4f}, {2.8f, 0.75f, 0.25f});
    addBox("ferry-office-left-wall", {-2.8f, 0.75f, 3.75f}, {0.25f, 0.75f, 1.9f});
    addBox("ferry-office-right-wall", {2.8f, 0.75f, 3.75f}, {0.25f, 0.75f, 1.9f});
    addBox("service-gate", {0.0f, 0.75f, 2.35f}, {2.45f, 0.75f, 0.16f});
    addBox("service-barrier", {2.8f, 0.25f, -1.5f}, {1.0f, 0.25f, 0.5f});
    addBox("dock-rail-left", {-4.2f, 0.45f, -0.5f}, {0.2f, 0.45f, 3.0f});
    addBox("dock-rail-right", {4.2f, 0.45f, -0.5f}, {0.2f, 0.45f, 3.0f});
    addBox("office-counter", {-1.35f, 0.45f, 1.45f}, {0.7f, 0.45f, 0.3f});
    addBox("maintenance-crate", {3.6f, 0.45f, 0.55f}, {0.45f, 0.45f, 0.45f});
}

bool TestWorld::setColliderBlocksPlayer(std::string_view name, bool blocksPlayer)
{
    for (StaticCollider& collider : m_colliders) {
        if (collider.name == name) {
            collider.blocksPlayer = blocksPlayer;
            return true;
        }
    }

    return false;
}

CollisionResult TestWorld::resolvePlayer(const PlayerCollisionProxy& proxy) const
{
    CollisionResult result;
    result.position = proxy.position;
    result.velocity = proxy.velocity;

    if (result.position.y <= m_floorHeight) {
        result.position.y = m_floorHeight;
        result.grounded = true;
        if (result.velocity.y < 0.0f) {
            result.velocity.y = 0.0f;
        }
    } else {
        result.grounded = isGrounded(result.position);
    }

    PlayerCollisionProxy resolvedProxy = proxy;
    resolvedProxy.position = result.position;
    resolvedProxy.velocity = result.velocity;
    for (const StaticCollider& collider : m_colliders) {
        if (!collider.blocksPlayer) {
            continue;
        }

        resolvedProxy.position = resolveCollider(resolvedProxy, collider, result);
        resolvedProxy.velocity = result.velocity;
    }

    if (result.position.y <= m_floorHeight) {
        result.position.y = m_floorHeight;
        result.grounded = true;
    }

    return result;
}

bool TestWorld::playerOverlapsCollider(engine::Vec3 position, float radius, float height, const StaticCollider& collider) const
{
    const engine::Vec3 halfExtents {radius, std::max(height * 0.5f, 0.01f), radius};
    const engine::Vec3 center = position + engine::Vec3 {0.0f, halfExtents.y, 0.0f};
    return collider.bounds.overlaps(center, halfExtents);
}

bool TestWorld::isGrounded(engine::Vec3 position, float probeDistance) const
{
    return position.y <= m_floorHeight + probeDistance;
}

RaycastHit TestWorld::raycast(engine::Vec3 origin, engine::Vec3 direction, float maxDistance) const
{
    RaycastHit bestHit;
    float bestDistance = maxDistance;
    const engine::Vec3 rayDirection = engine::Normalize(direction);
    if (engine::Length(rayDirection) <= 0.00001f || maxDistance <= 0.0f) {
        return bestHit;
    }

    for (const StaticCollider& collider : m_colliders) {
        const engine::Vec3 boundsMin = collider.bounds.min();
        const engine::Vec3 boundsMax = collider.bounds.max();
        float tMin = 0.0f;
        float tMax = maxDistance;
        engine::Vec3 normal {};

        const auto testAxis = [&](float originValue, float directionValue, float minValue, float maxValue, engine::Vec3 axisNormal) {
            if (std::abs(directionValue) <= 0.00001f) {
                return originValue >= minValue && originValue <= maxValue;
            }

            float t1 = (minValue - originValue) / directionValue;
            float t2 = (maxValue - originValue) / directionValue;
            engine::Vec3 entryNormal = directionValue > 0.0f ? axisNormal * -1.0f : axisNormal;
            if (t1 > t2) {
                std::swap(t1, t2);
                entryNormal = entryNormal * -1.0f;
            }

            if (t1 > tMin) {
                tMin = t1;
                normal = entryNormal;
            }
            tMax = std::min(tMax, t2);
            return tMin <= tMax;
        };

        if (!testAxis(origin.x, rayDirection.x, boundsMin.x, boundsMax.x, {1.0f, 0.0f, 0.0f})
            || !testAxis(origin.y, rayDirection.y, boundsMin.y, boundsMax.y, {0.0f, 1.0f, 0.0f})
            || !testAxis(origin.z, rayDirection.z, boundsMin.z, boundsMax.z, {0.0f, 0.0f, 1.0f})) {
            continue;
        }

        if (tMin >= 0.0f && tMin < bestDistance) {
            bestDistance = tMin;
            bestHit.hit = true;
            bestHit.distance = tMin;
            bestHit.point = origin + rayDirection * tMin;
            bestHit.normal = normal;
            bestHit.colliderId = collider.id;
            bestHit.colliderName = collider.name;
        }
    }

    return bestHit;
}

const std::vector<StaticCollider>& TestWorld::colliders() const
{
    return m_colliders;
}

const StaticCollider* TestWorld::colliderByName(std::string_view name) const
{
    for (const StaticCollider& collider : m_colliders) {
        if (collider.name == name) {
            return &collider;
        }
    }

    return nullptr;
}

float TestWorld::floorHeight() const
{
    return m_floorHeight;
}

engine::Vec3 TestWorld::resolveCollider(const PlayerCollisionProxy& proxy, const StaticCollider& collider, CollisionResult& result) const
{
    engine::Vec3 position = proxy.position;
    const float top = collider.bounds.center.y + collider.bounds.halfExtents.y;
    const float playerBottom = position.y;
    const float playerTop = position.y + proxy.height;
    if (playerTop < collider.bounds.center.y - collider.bounds.halfExtents.y || playerBottom > top + 0.05f) {
        result.position = position;
        return position;
    }

    const engine::Vec3 expandedHalfExtents = collider.bounds.halfExtents + engine::Vec3 {proxy.radius, 0.0f, proxy.radius};
    const float minX = collider.bounds.center.x - expandedHalfExtents.x;
    const float maxX = collider.bounds.center.x + expandedHalfExtents.x;
    const float minZ = collider.bounds.center.z - expandedHalfExtents.z;
    const float maxZ = collider.bounds.center.z + expandedHalfExtents.z;

    if (position.x < minX || position.x > maxX || position.z < minZ || position.z > maxZ) {
        result.position = position;
        return position;
    }

    const float pushLeft = std::abs(position.x - minX);
    const float pushRight = std::abs(maxX - position.x);
    const float pushBack = std::abs(position.z - minZ);
    const float pushForward = std::abs(maxZ - position.z);

    const bool cameFromLeft = proxy.previousPosition.x <= minX && pushLeft <= pushRight;
    const bool cameFromRight = proxy.previousPosition.x >= maxX && pushRight <= pushLeft;
    const bool cameFromBack = proxy.previousPosition.z <= minZ && pushBack <= pushForward;
    const bool cameFromForward = proxy.previousPosition.z >= maxZ && pushForward <= pushBack;

    engine::Vec3 corrected = position;
    engine::Vec3 normal {};
    if (cameFromLeft) {
        corrected.x = minX;
        normal = {-1.0f, 0.0f, 0.0f};
        result.velocity.x = std::min(result.velocity.x, 0.0f);
    } else if (cameFromRight) {
        corrected.x = maxX;
        normal = {1.0f, 0.0f, 0.0f};
        result.velocity.x = std::max(result.velocity.x, 0.0f);
    } else if (cameFromBack) {
        corrected.z = minZ;
        normal = {0.0f, 0.0f, -1.0f};
        result.velocity.z = std::min(result.velocity.z, 0.0f);
    } else if (cameFromForward) {
        corrected.z = maxZ;
        normal = {0.0f, 0.0f, 1.0f};
        result.velocity.z = std::max(result.velocity.z, 0.0f);
    } else {
        const float minPush = std::min({pushLeft, pushRight, pushBack, pushForward});
        if (minPush == pushLeft) {
            corrected.x = minX;
            normal = {-1.0f, 0.0f, 0.0f};
        } else if (minPush == pushRight) {
            corrected.x = maxX;
            normal = {1.0f, 0.0f, 0.0f};
        } else if (minPush == pushBack) {
            corrected.z = minZ;
            normal = {0.0f, 0.0f, -1.0f};
        } else {
            corrected.z = maxZ;
            normal = {0.0f, 0.0f, 1.0f};
        }
    }

    result.lastPush = corrected - position;
    result.lastNormal = normal;
    result.hitCount += 1;
    result.position = corrected;
    return corrected;
}
