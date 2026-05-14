#pragma once

#include "engine/math/Math.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace engine::physics {

enum class PhysicsBackend {
    Simple,
    Jolt,
};

struct PhysicsConfig {
    PhysicsBackend backend = PhysicsBackend::Simple;
    float fixedStepSeconds = 1.0f / 60.0f;
    Vec3 gravity {0.0f, -9.81f, 0.0f};
};

struct BodyHandle {
    std::uint32_t value = 0;

    bool isValid() const
    {
        return value != 0;
    }
};

struct BoxColliderDesc {
    std::string name;
    Vec3 center;
    Vec3 halfExtents {0.5f, 0.5f, 0.5f};
    bool isTrigger = false;
};

struct DynamicBoxDesc {
    std::string name;
    Vec3 center;
    Vec3 halfExtents {0.5f, 0.5f, 0.5f};
    Vec3 linearVelocity;
    float mass = 1.0f;
};

struct RaycastResult {
    bool hit = false;
    float distance = 0.0f;
    Vec3 point;
    Vec3 normal;
    BodyHandle body;
    std::string bodyName;
};

struct PhysicsDebugLine {
    Vec3 from;
    Vec3 to;
    Vec3 color {0.3f, 0.8f, 1.0f};
};

class IPhysicsWorld {
public:
    virtual ~IPhysicsWorld() = default;

    virtual bool initialize(const PhysicsConfig& config) = 0;
    virtual void shutdown() = 0;
    virtual bool isInitialized() const = 0;
    virtual std::string_view backendName() const = 0;

    virtual BodyHandle addStaticBox(const BoxColliderDesc& desc) = 0;
    virtual BodyHandle addTriggerBox(const BoxColliderDesc& desc) = 0;
    virtual BodyHandle addDynamicBox(const DynamicBoxDesc& desc) = 0;
    virtual BodyHandle addFloor(std::string name, float y, float halfSize, float thickness) = 0;

    virtual void step(float deltaSeconds) = 0;
    virtual RaycastResult raycast(Vec3 origin, Vec3 direction, float maxDistance) const = 0;
    virtual std::vector<PhysicsDebugLine> debugLines() const = 0;
};

std::unique_ptr<IPhysicsWorld> CreatePhysicsWorld(PhysicsBackend backend);
bool IsJoltPhysicsAvailable();

} // namespace engine::physics
