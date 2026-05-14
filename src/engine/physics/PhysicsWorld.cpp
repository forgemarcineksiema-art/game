#include "engine/physics/PhysicsWorld.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace engine::physics {

namespace {

struct SimpleBody {
    BodyHandle handle;
    std::string name;
    Vec3 center;
    Vec3 halfExtents;
    Vec3 linearVelocity;
    bool dynamic = false;
    bool trigger = false;
};

Vec3 minPoint(const SimpleBody& body)
{
    return body.center - body.halfExtents;
}

Vec3 maxPoint(const SimpleBody& body)
{
    return body.center + body.halfExtents;
}

class SimplePhysicsWorld final : public IPhysicsWorld {
public:
    bool initialize(const PhysicsConfig& config) override
    {
        m_config = config;
        m_initialized = true;
        m_nextHandle = 1;
        m_bodies.clear();
        return true;
    }

    void shutdown() override
    {
        m_bodies.clear();
        m_initialized = false;
    }

    bool isInitialized() const override
    {
        return m_initialized;
    }

    std::string_view backendName() const override
    {
        return "simple";
    }

    BodyHandle addStaticBox(const BoxColliderDesc& desc) override
    {
        return addBox(desc.name, desc.center, desc.halfExtents, {}, false, desc.isTrigger);
    }

    BodyHandle addTriggerBox(const BoxColliderDesc& desc) override
    {
        return addBox(desc.name, desc.center, desc.halfExtents, {}, false, true);
    }

    BodyHandle addDynamicBox(const DynamicBoxDesc& desc) override
    {
        const float safeMass = std::max(desc.mass, 0.001f);
        (void)safeMass;
        return addBox(desc.name, desc.center, desc.halfExtents, desc.linearVelocity, true, false);
    }

    BodyHandle addFloor(std::string name, float y, float halfSize, float thickness) override
    {
        const float safeHalfSize = std::max(halfSize, 0.1f);
        const float safeThickness = std::max(thickness, 0.01f);
        BoxColliderDesc floor;
        floor.name = std::move(name);
        floor.center = {0.0f, y - safeThickness, 0.0f};
        floor.halfExtents = {safeHalfSize, safeThickness, safeHalfSize};
        return addStaticBox(floor);
    }

    void step(float deltaSeconds) override
    {
        if (!m_initialized) {
            return;
        }

        const float dt = deltaSeconds > 0.0f ? deltaSeconds : m_config.fixedStepSeconds;
        for (SimpleBody& body : m_bodies) {
            if (!body.dynamic) {
                continue;
            }

            body.linearVelocity += m_config.gravity * dt;
            body.center += body.linearVelocity * dt;
        }
    }

    RaycastResult raycast(Vec3 origin, Vec3 direction, float maxDistance) const override
    {
        RaycastResult best;
        if (!m_initialized || maxDistance <= 0.0f) {
            return best;
        }

        const Vec3 rayDirection = Normalize(direction);
        if (Length(rayDirection) <= 0.00001f) {
            return best;
        }

        float bestDistance = maxDistance;
        for (const SimpleBody& body : m_bodies) {
            const RaycastResult hit = raycastBody(body, origin, rayDirection, maxDistance);
            if (hit.hit && hit.distance < bestDistance) {
                bestDistance = hit.distance;
                best = hit;
            }
        }

        return best;
    }

    std::vector<PhysicsDebugLine> debugLines() const override
    {
        std::vector<PhysicsDebugLine> lines;
        for (const SimpleBody& body : m_bodies) {
            appendBoxLines(body, lines);
        }
        return lines;
    }

private:
    BodyHandle addBox(std::string name, Vec3 center, Vec3 halfExtents, Vec3 linearVelocity, bool dynamic, bool trigger)
    {
        if (!m_initialized) {
            return {};
        }

        SimpleBody body;
        body.handle.value = m_nextHandle++;
        body.name = std::move(name);
        body.center = center;
        body.halfExtents = {
            std::max(halfExtents.x, 0.001f),
            std::max(halfExtents.y, 0.001f),
            std::max(halfExtents.z, 0.001f),
        };
        body.linearVelocity = linearVelocity;
        body.dynamic = dynamic;
        body.trigger = trigger;
        m_bodies.push_back(std::move(body));
        return m_bodies.back().handle;
    }

    static RaycastResult raycastBody(const SimpleBody& body, Vec3 origin, Vec3 direction, float maxDistance)
    {
        RaycastResult result;
        float tMin = 0.0f;
        float tMax = maxDistance;
        Vec3 normal {};

        const Vec3 boundsMin = minPoint(body);
        const Vec3 boundsMax = maxPoint(body);
        const auto testAxis = [&](float originValue, float directionValue, float minValue, float maxValue, Vec3 axisNormal) {
            if (std::abs(directionValue) <= 0.00001f) {
                return originValue >= minValue && originValue <= maxValue;
            }

            float t1 = (minValue - originValue) / directionValue;
            float t2 = (maxValue - originValue) / directionValue;
            Vec3 entryNormal = directionValue > 0.0f ? axisNormal * -1.0f : axisNormal;
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

        if (!testAxis(origin.x, direction.x, boundsMin.x, boundsMax.x, {1.0f, 0.0f, 0.0f})
            || !testAxis(origin.y, direction.y, boundsMin.y, boundsMax.y, {0.0f, 1.0f, 0.0f})
            || !testAxis(origin.z, direction.z, boundsMin.z, boundsMax.z, {0.0f, 0.0f, 1.0f})) {
            return result;
        }

        if (tMin < 0.0f || tMin > maxDistance) {
            return result;
        }

        result.hit = true;
        result.distance = tMin;
        result.point = origin + direction * tMin;
        result.normal = normal;
        result.body = body.handle;
        result.bodyName = body.name;
        return result;
    }

    static void appendBoxLines(const SimpleBody& body, std::vector<PhysicsDebugLine>& lines)
    {
        const Vec3 min = minPoint(body);
        const Vec3 max = maxPoint(body);
        const Vec3 color = body.trigger ? Vec3 {0.9f, 0.6f, 0.1f} : Vec3 {0.2f, 0.75f, 1.0f};
        const Vec3 c[8] = {
            {min.x, min.y, min.z},
            {max.x, min.y, min.z},
            {max.x, min.y, max.z},
            {min.x, min.y, max.z},
            {min.x, max.y, min.z},
            {max.x, max.y, min.z},
            {max.x, max.y, max.z},
            {min.x, max.y, max.z},
        };

        const int edges[12][2] = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0},
            {4, 5}, {5, 6}, {6, 7}, {7, 4},
            {0, 4}, {1, 5}, {2, 6}, {3, 7},
        };

        for (const auto& edge : edges) {
            lines.push_back({c[edge[0]], c[edge[1]], color});
        }
    }

    PhysicsConfig m_config;
    bool m_initialized = false;
    std::uint32_t m_nextHandle = 1;
    std::vector<SimpleBody> m_bodies;
};

} // namespace

std::unique_ptr<IPhysicsWorld> CreateJoltPhysicsWorldIfAvailable();

std::unique_ptr<IPhysicsWorld> CreatePhysicsWorld(PhysicsBackend backend)
{
    switch (backend) {
    case PhysicsBackend::Simple:
        return std::make_unique<SimplePhysicsWorld>();
    case PhysicsBackend::Jolt:
        return CreateJoltPhysicsWorldIfAvailable();
    }

    return nullptr;
}

#if !ENGINE_WITH_JOLT
bool IsJoltPhysicsAvailable()
{
    return false;
}

std::unique_ptr<IPhysicsWorld> CreateJoltPhysicsWorldIfAvailable()
{
    return nullptr;
}
#endif

} // namespace engine::physics
