#include "engine/physics/PhysicsWorld.h"

#if ENGINE_WITH_JOLT

#include "engine/math/BoxEdges.h"

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemSingleThreaded.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include <algorithm>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>

namespace engine::physics {

namespace {

void JoltTrace(const char*, ...)
{
}

#ifdef JPH_ENABLE_ASSERTS
bool JoltAssertFailed(const char*, const char*, const char*, JPH::uint)
{
    return true;
}
#endif

class JoltRuntime {
public:
    static void retain()
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_refCount == 0) {
            JPH::RegisterDefaultAllocator();
            JPH::Trace = JoltTrace;
            JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = JoltAssertFailed;)
            JPH::Factory::sInstance = new JPH::Factory();
            JPH::RegisterTypes();
        }
        ++s_refCount;
    }

    static void release()
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_refCount == 0) {
            return;
        }

        --s_refCount;
        if (s_refCount == 0) {
            JPH::UnregisterTypes();
            delete JPH::Factory::sInstance;
            JPH::Factory::sInstance = nullptr;
        }
    }

private:
    static inline std::mutex s_mutex;
    static inline int s_refCount = 0;
};

namespace Layers {
static constexpr JPH::ObjectLayer NonMoving = 0;
static constexpr JPH::ObjectLayer Moving = 1;
static constexpr JPH::ObjectLayer NumLayers = 2;
}

namespace BroadPhaseLayers {
static constexpr JPH::BroadPhaseLayer NonMoving(0);
static constexpr JPH::BroadPhaseLayer Moving(1);
static constexpr JPH::uint NumLayers = 2;
}

class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer object1, JPH::ObjectLayer object2) const override
    {
        switch (object1) {
        case Layers::NonMoving:
            return object2 == Layers::Moving;
        case Layers::Moving:
            return true;
        default:
            return false;
        }
    }
};

class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
public:
    BroadPhaseLayerInterfaceImpl()
    {
        m_objectToBroadPhase[Layers::NonMoving] = BroadPhaseLayers::NonMoving;
        m_objectToBroadPhase[Layers::Moving] = BroadPhaseLayers::Moving;
    }

    JPH::uint GetNumBroadPhaseLayers() const override
    {
        return BroadPhaseLayers::NumLayers;
    }

    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override
    {
        if (layer >= Layers::NumLayers) {
            return BroadPhaseLayers::NonMoving;
        }

        return m_objectToBroadPhase[layer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override
    {
        switch (static_cast<JPH::BroadPhaseLayer::Type>(layer)) {
        case static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::NonMoving):
            return "NonMoving";
        case static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::Moving):
            return "Moving";
        default:
            return "Invalid";
        }
    }
#endif

private:
    JPH::BroadPhaseLayer m_objectToBroadPhase[Layers::NumLayers];
};

class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const override
    {
        switch (layer1) {
        case Layers::NonMoving:
            return layer2 == BroadPhaseLayers::Moving;
        case Layers::Moving:
            return true;
        default:
            return false;
        }
    }
};

struct JoltBodyRecord {
    BodyHandle handle;
    JPH::BodyID bodyId;
    std::string name;
    Vec3 center;
    Vec3 halfExtents;
    bool trigger = false;
};

JPH::Vec3 ToJoltVec3(Vec3 value)
{
    return {value.x, value.y, value.z};
}

JPH::RVec3 ToJoltRVec3(Vec3 value)
{
    return {value.x, value.y, value.z};
}

Vec3 FromJoltVec3(JPH::Vec3 value)
{
    return {value.GetX(), value.GetY(), value.GetZ()};
}

Vec3 FromJoltRVec3(JPH::RVec3 value)
{
    return {
        static_cast<float>(value.GetX()),
        static_cast<float>(value.GetY()),
        static_cast<float>(value.GetZ()),
    };
}

bool OverlapsBox(Vec3 aCenter, Vec3 aHalfExtents, Vec3 bCenter, Vec3 bHalfExtents)
{
    const Vec3 aMin = aCenter - aHalfExtents;
    const Vec3 aMax = aCenter + aHalfExtents;
    const Vec3 bMin = bCenter - bHalfExtents;
    const Vec3 bMax = bCenter + bHalfExtents;
    return aMin.x <= bMax.x && aMax.x >= bMin.x
        && aMin.y <= bMax.y && aMax.y >= bMin.y
        && aMin.z <= bMax.z && aMax.z >= bMin.z;
}

class JoltPhysicsWorld final : public IPhysicsWorld {
public:
    bool initialize(const PhysicsConfig& config) override
    {
        if (m_initialized) {
            shutdown();
        }

        m_config = config;
        JoltRuntime::retain();
        m_runtimeRetained = true;

        constexpr JPH::uint maxBodies = 1024;
        constexpr JPH::uint numBodyMutexes = 0;
        constexpr JPH::uint maxBodyPairs = 1024;
        constexpr JPH::uint maxContactConstraints = 1024;

        m_tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(4 * 1024 * 1024);
        m_jobSystem = std::make_unique<JPH::JobSystemSingleThreaded>(JPH::cMaxPhysicsJobs);
        m_physicsSystem.Init(
            maxBodies,
            numBodyMutexes,
            maxBodyPairs,
            maxContactConstraints,
            m_broadPhaseLayerInterface,
            m_objectVsBroadPhaseLayerFilter,
            m_objectLayerPairFilter);

        m_initialized = true;
        m_nextHandle = 1;
        m_records.clear();
        m_recordByBodyId.clear();
        return true;
    }

    void shutdown() override
    {
        if (m_initialized) {
            JPH::BodyInterface& bodyInterface = m_physicsSystem.GetBodyInterface();
            for (const JoltBodyRecord& record : m_records) {
                bodyInterface.RemoveBody(record.bodyId);
                bodyInterface.DestroyBody(record.bodyId);
            }
        }

        m_records.clear();
        m_recordByBodyId.clear();
        m_jobSystem.reset();
        m_tempAllocator.reset();
        m_initialized = false;

        if (m_runtimeRetained) {
            JoltRuntime::release();
            m_runtimeRetained = false;
        }
    }

    bool isInitialized() const override
    {
        return m_initialized;
    }

    std::string_view backendName() const override
    {
        return "jolt";
    }

    BodyHandle addStaticBox(const BoxColliderDesc& desc) override
    {
        return addBox(desc.name, desc.center, desc.halfExtents, {}, JPH::EMotionType::Static, Layers::NonMoving, desc.isTrigger);
    }

    BodyHandle addTriggerBox(const BoxColliderDesc& desc) override
    {
        return addBox(desc.name, desc.center, desc.halfExtents, {}, JPH::EMotionType::Static, Layers::NonMoving, true);
    }

    BodyHandle addDynamicBox(const DynamicBoxDesc& desc) override
    {
        const BodyHandle handle = addBox(desc.name, desc.center, desc.halfExtents, desc.linearVelocity, JPH::EMotionType::Dynamic, Layers::Moving, false);
        return handle;
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
        if (!m_initialized || !m_tempAllocator || !m_jobSystem) {
            return;
        }

        const float dt = deltaSeconds > 0.0f ? deltaSeconds : m_config.fixedStepSeconds;
        m_physicsSystem.Update(dt, 1, m_tempAllocator.get(), m_jobSystem.get());
    }

    RaycastResult raycast(Vec3 origin, Vec3 direction, float maxDistance) const override
    {
        RaycastResult result;
        if (!m_initialized || maxDistance <= 0.0f) {
            return result;
        }

        const Vec3 normalized = Normalize(direction);
        if (Length(normalized) <= 0.00001f) {
            return result;
        }

        JPH::RRayCast ray(ToJoltRVec3(origin), ToJoltVec3(normalized * maxDistance));
        JPH::RayCastResult hit;
        if (!m_physicsSystem.GetNarrowPhaseQuery().CastRay(ray, hit)) {
            return result;
        }

        result.hit = true;
        result.distance = hit.mFraction * maxDistance;
        result.point = FromJoltRVec3(ray.GetPointOnRay(hit.mFraction));

        const std::uint32_t joltId = hit.mBodyID.GetIndexAndSequenceNumber();
        const auto recordIt = m_recordByBodyId.find(joltId);
        if (recordIt != m_recordByBodyId.end()) {
            const JoltBodyRecord& record = m_records[recordIt->second];
            result.body = record.handle;
            result.bodyName = record.name;
        }

        return result;
    }

    std::vector<OverlapResult> overlapBox(Vec3 center, Vec3 halfExtents) const override
    {
        std::vector<OverlapResult> overlaps;
        if (!m_initialized) {
            return overlaps;
        }

        const Vec3 safeHalfExtents {
            std::max(halfExtents.x, 0.001f),
            std::max(halfExtents.y, 0.001f),
            std::max(halfExtents.z, 0.001f),
        };
        for (const JoltBodyRecord& record : m_records) {
            if (OverlapsBox(center, safeHalfExtents, record.center, record.halfExtents)) {
                overlaps.push_back({record.handle, record.name});
            }
        }
        return overlaps;
    }

    std::vector<PhysicsDebugLine> debugLines() const override
    {
        std::vector<PhysicsDebugLine> lines;
        for (const JoltBodyRecord& record : m_records) {
            appendBoxLines(record, lines);
        }
        return lines;
    }

private:
    BodyHandle addBox(
        std::string name,
        Vec3 center,
        Vec3 halfExtents,
        Vec3 linearVelocity,
        JPH::EMotionType motionType,
        JPH::ObjectLayer layer,
        bool trigger)
    {
        if (!m_initialized) {
            return {};
        }

        const Vec3 safeHalfExtents {
            std::max(halfExtents.x, 0.001f),
            std::max(halfExtents.y, 0.001f),
            std::max(halfExtents.z, 0.001f),
        };

        JPH::BoxShapeSettings shapeSettings(ToJoltVec3(safeHalfExtents));
        shapeSettings.SetEmbedded();
        JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
        if (shapeResult.HasError()) {
            return {};
        }

        JPH::BodyCreationSettings bodySettings(
            shapeResult.Get(),
            ToJoltRVec3(center),
            JPH::Quat::sIdentity(),
            motionType,
            layer);

        JPH::BodyInterface& bodyInterface = m_physicsSystem.GetBodyInterface();
        JPH::Body* body = bodyInterface.CreateBody(bodySettings);
        if (body == nullptr) {
            return {};
        }

        if (trigger) {
            body->SetIsSensor(true);
        }

        bodyInterface.AddBody(body->GetID(), motionType == JPH::EMotionType::Dynamic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
        if (motionType == JPH::EMotionType::Dynamic) {
            bodyInterface.SetLinearVelocity(body->GetID(), ToJoltVec3(linearVelocity));
        }

        JoltBodyRecord record;
        record.handle.value = m_nextHandle++;
        record.bodyId = body->GetID();
        record.name = std::move(name);
        record.center = center;
        record.halfExtents = safeHalfExtents;
        record.trigger = trigger;
        m_recordByBodyId[record.bodyId.GetIndexAndSequenceNumber()] = m_records.size();
        m_records.push_back(std::move(record));
        return m_records.back().handle;
    }

    static void appendBoxLines(const JoltBodyRecord& record, std::vector<PhysicsDebugLine>& lines)
    {
        const Vec3 min = record.center - record.halfExtents;
        const Vec3 max = record.center + record.halfExtents;
        const Vec3 color = record.trigger ? Vec3 {0.9f, 0.6f, 0.1f} : Vec3 {0.2f, 0.9f, 0.75f};
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

        for (const auto& edge : BoxEdgeIndices) {
            lines.push_back({c[edge[0]], c[edge[1]], color});
        }
    }

    PhysicsConfig m_config;
    bool m_initialized = false;
    bool m_runtimeRetained = false;
    std::uint32_t m_nextHandle = 1;
    BroadPhaseLayerInterfaceImpl m_broadPhaseLayerInterface;
    ObjectVsBroadPhaseLayerFilterImpl m_objectVsBroadPhaseLayerFilter;
    ObjectLayerPairFilterImpl m_objectLayerPairFilter;
    JPH::PhysicsSystem m_physicsSystem;
    std::unique_ptr<JPH::TempAllocatorImpl> m_tempAllocator;
    std::unique_ptr<JPH::JobSystemSingleThreaded> m_jobSystem;
    std::vector<JoltBodyRecord> m_records;
    std::unordered_map<std::uint32_t, std::size_t> m_recordByBodyId;
};

} // namespace

bool IsJoltPhysicsAvailable()
{
    return true;
}

std::unique_ptr<IPhysicsWorld> CreateJoltPhysicsWorldIfAvailable()
{
    return std::make_unique<JoltPhysicsWorld>();
}

} // namespace engine::physics

#endif // ENGINE_WITH_JOLT
