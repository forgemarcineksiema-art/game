#include "engine/physics/VehicleRuntime.h"

#if ENGINE_WITH_JOLT

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemSingleThreaded.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Constraints/Constraint.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Vehicle/VehicleCollisionTester.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>
#include <Jolt/Physics/Vehicle/WheeledVehicleController.h>

#include <atomic>
#include <cmath>
#include <memory>
#include <string>

namespace engine::physics {
namespace {

void JoltVehicleTrace(const char*, ...)
{
}

bool JoltVehicleAssertFailed(const char*, const char*, const char*, JPH::uint)
{
    return true;
}

class JoltVehicleRuntimeGlobals {
public:
    static void retain()
    {
        if (s_refCount.fetch_add(1) == 0) {
            JPH::RegisterDefaultAllocator();
            JPH::Trace = JoltVehicleTrace;
            JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = JoltVehicleAssertFailed;)
            JPH::Factory::sInstance = new JPH::Factory();
            JPH::RegisterTypes();
        }
    }

    static void release()
    {
        if (s_refCount.fetch_sub(1) == 1) {
            JPH::UnregisterTypes();
            delete JPH::Factory::sInstance;
            JPH::Factory::sInstance = nullptr;
        }
    }

private:
    static std::atomic<int> s_refCount;
};

std::atomic<int> JoltVehicleRuntimeGlobals::s_refCount {0};

namespace Layers {
    constexpr JPH::ObjectLayer NonMoving = 0;
    constexpr JPH::ObjectLayer Moving = 1;
    constexpr JPH::ObjectLayer Count = 2;
}

namespace BroadPhaseLayers {
    constexpr JPH::BroadPhaseLayer NonMoving(0);
    constexpr JPH::BroadPhaseLayer Moving(1);
    constexpr JPH::uint Count = 2;
}

class BroadPhaseLayerInterface final : public JPH::BroadPhaseLayerInterface {
public:
    BroadPhaseLayerInterface()
    {
        m_objectToBroadPhase[Layers::NonMoving] = BroadPhaseLayers::NonMoving;
        m_objectToBroadPhase[Layers::Moving] = BroadPhaseLayers::Moving;
    }

    JPH::uint GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::Count; }

    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override
    {
        return m_objectToBroadPhase[layer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override
    {
        switch (static_cast<JPH::BroadPhaseLayer::Type>(layer)) {
        case static_cast<JPH::BroadPhaseLayer::Type>(0):
            return "NonMoving";
        case static_cast<JPH::BroadPhaseLayer::Type>(1):
            return "Moving";
        default:
            return "Unknown";
        }
    }
#endif

private:
    JPH::BroadPhaseLayer m_objectToBroadPhase[Layers::Count];
};

class ObjectVsBroadPhaseLayerFilter final : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer broadPhaseLayer) const override
    {
        switch (layer) {
        case Layers::NonMoving:
            return broadPhaseLayer == BroadPhaseLayers::Moving;
        case Layers::Moving:
            return true;
        default:
            return false;
        }
    }
};

class ObjectLayerPairFilter final : public JPH::ObjectLayerPairFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer first, JPH::ObjectLayer second) const override
    {
        if (first == Layers::NonMoving) {
            return second == Layers::Moving;
        }
        if (first == Layers::Moving) {
            return true;
        }
        return false;
    }
};

JPH::Vec3 ToJoltVec3(Vec3 value)
{
    return JPH::Vec3(value.x, value.y, value.z);
}

JPH::RVec3 ToJoltRVec3(Vec3 value)
{
    return JPH::RVec3(value.x, value.y, value.z);
}

Vec3 FromJoltRVec3(JPH::RVec3 value)
{
    return {
        static_cast<float>(value.GetX()),
        static_cast<float>(value.GetY()),
        static_cast<float>(value.GetZ()),
    };
}

bool OutOfBounds(Vec3 position, const VehicleRuntimeConfig& config)
{
    return position.x < config.boundsMin.x || position.x > config.boundsMax.x
        || position.z < config.boundsMin.y || position.z > config.boundsMax.y;
}

int CountWheelContacts(const JPH::VehicleConstraint& constraint)
{
    int contacts = 0;
    for (JPH::uint wheel = 0; wheel < constraint.GetWheels().size(); ++wheel) {
        if (constraint.GetWheel(wheel)->HasContact()) {
            contacts += 1;
        }
    }
    return contacts;
}

class JoltVehicleRuntimeAdapter final : public IVehicleRuntimeAdapter {
public:
    ~JoltVehicleRuntimeAdapter() override
    {
        shutdown();
    }

    bool initialize(const VehicleRuntimeConfig& config) override
    {
        shutdown();
        m_config = config;
        m_state = {};
        m_state.position = config.spawnPosition;
        m_state.yawRadians = config.spawnYawRadians;
        m_state.wheelContactCount = 0;
        m_error.clear();

        JoltVehicleRuntimeGlobals::retain();
        m_runtimeRetained = true;
        m_tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(8 * 1024 * 1024);
        m_jobSystem = std::make_unique<JPH::JobSystemSingleThreaded>(2048);
        m_physicsSystem = std::make_unique<JPH::PhysicsSystem>();
        m_physicsSystem->Init(
            1024,
            0,
            1024,
            1024,
            m_broadPhaseLayerInterface,
            m_objectVsBroadPhaseLayerFilter,
            m_objectLayerPairFilter);

        JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
        {
            JPH::BoxShapeSettings floorShape(ToJoltVec3({40.0f, 0.05f, 40.0f}));
            floorShape.SetEmbedded();
            JPH::BodyCreationSettings floorSettings(
                floorShape.Create().Get(),
                JPH::RVec3(0.0f, -0.05f, 0.0f),
                JPH::Quat::sIdentity(),
                JPH::EMotionType::Static,
                Layers::NonMoving);
            JPH::Body* floorBody = bodyInterface.CreateBody(floorSettings);
            if (floorBody == nullptr) {
                m_error = "Failed to create Jolt vehicle runtime floor.";
                shutdown();
                return false;
            }
            m_floorId = floorBody->GetID();
            bodyInterface.AddBody(m_floorId, JPH::EActivation::DontActivate);
        }

        JPH::BoxShapeSettings bodyBox(ToJoltVec3(config.halfExtents));
        bodyBox.SetEmbedded();
        JPH::OffsetCenterOfMassShapeSettings bodyShape(
            JPH::Vec3(0.0f, -config.halfExtents.y * 0.55f, 0.0f),
            bodyBox.Create().Get());
        JPH::BodyCreationSettings bodySettings(
            bodyShape.Create().Get(),
            ToJoltRVec3(config.spawnPosition + Vec3 {0.0f, config.halfExtents.y + 0.08f, 0.0f}),
            JPH::Quat::sRotation(JPH::Vec3::sAxisY(), config.spawnYawRadians),
            JPH::EMotionType::Dynamic,
            Layers::Moving);
        bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
        bodySettings.mMassPropertiesOverride.mMass = 950.0f;

        JPH::Body* body = bodyInterface.CreateBody(bodySettings);
        if (body == nullptr) {
            m_error = "Failed to create Jolt vehicle runtime body.";
            shutdown();
            return false;
        }
        m_bodyId = body->GetID();
        bodyInterface.AddBody(m_bodyId, JPH::EActivation::Activate);

        JPH::VehicleConstraintSettings vehicleSettings;
        vehicleSettings.mDrawConstraintSize = 0.1f;
        vehicleSettings.mMaxPitchRollAngle = JPH::DegreesToRadians(60.0f);
        vehicleSettings.mWheels.resize(4);

        const float wheelX = std::max(config.halfExtents.x * 0.82f, 0.35f);
        const float frontZ = std::max(config.halfExtents.z * 0.72f, 0.55f);
        const float rearZ = -frontZ;
        const float wheelRadius = 0.28f;
        const float wheelWidth = 0.18f;
        const JPH::Vec3 wheelPositions[4] = {
            JPH::Vec3(-wheelX, -config.halfExtents.y * 0.65f, frontZ),
            JPH::Vec3(wheelX, -config.halfExtents.y * 0.65f, frontZ),
            JPH::Vec3(-wheelX, -config.halfExtents.y * 0.65f, rearZ),
            JPH::Vec3(wheelX, -config.halfExtents.y * 0.65f, rearZ),
        };

        for (int index = 0; index < 4; ++index) {
            JPH::WheelSettingsWV* wheel = new JPH::WheelSettingsWV();
            wheel->mPosition = wheelPositions[index];
            wheel->mRadius = wheelRadius;
            wheel->mWidth = wheelWidth;
            wheel->mSuspensionMinLength = 0.20f;
            wheel->mSuspensionMaxLength = 0.56f;
            wheel->mSuspensionSpring.mFrequency = 1.9f;
            wheel->mSuspensionSpring.mDamping = 0.62f;
            wheel->mMaxSteerAngle = index < 2 ? JPH::DegreesToRadians(32.0f) : 0.0f;
            vehicleSettings.mWheels[index] = wheel;
        }

        auto* controllerSettings = new JPH::WheeledVehicleControllerSettings();
        controllerSettings->mDifferentials.resize(1);
        controllerSettings->mDifferentials[0].mLeftWheel = 0;
        controllerSettings->mDifferentials[0].mRightWheel = 1;
        controllerSettings->mDifferentials[0].mEngineTorqueRatio = 1.0f;
        controllerSettings->mEngine.mMaxTorque = 520.0f;
        controllerSettings->mEngine.mMinRPM = 800.0f;
        controllerSettings->mEngine.mMaxRPM = 5400.0f;
        controllerSettings->mTransmission.mMode = JPH::ETransmissionMode::Auto;
        controllerSettings->mTransmission.mClutchStrength = 8.0f;
        vehicleSettings.mController = controllerSettings;

        m_constraint = new JPH::VehicleConstraint(*body, vehicleSettings);
        m_constraint->SetVehicleCollisionTester(new JPH::VehicleCollisionTesterRay(Layers::Moving));
        m_physicsSystem->AddConstraint(m_constraint);
        m_physicsSystem->AddStepListener(m_constraint);
        m_controller = static_cast<JPH::WheeledVehicleController*>(m_constraint->GetController());
        return true;
    }

    void shutdown() override
    {
        if (m_physicsSystem) {
            if (m_constraint) {
                m_physicsSystem->RemoveStepListener(m_constraint);
                m_physicsSystem->RemoveConstraint(m_constraint);
                m_constraint = nullptr;
                m_controller = nullptr;
            }

            JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
            if (!m_bodyId.IsInvalid()) {
                bodyInterface.RemoveBody(m_bodyId);
                bodyInterface.DestroyBody(m_bodyId);
                m_bodyId = {};
            }
            if (!m_floorId.IsInvalid()) {
                bodyInterface.RemoveBody(m_floorId);
                bodyInterface.DestroyBody(m_floorId);
                m_floorId = {};
            }
        }

        m_physicsSystem.reset();
        m_jobSystem.reset();
        m_tempAllocator.reset();
        if (m_runtimeRetained) {
            JoltVehicleRuntimeGlobals::release();
            m_runtimeRetained = false;
        }
    }

    bool step(const VehicleRuntimeInput& input, float deltaSeconds) override
    {
        if (!m_physicsSystem || m_bodyId.IsInvalid() || m_controller == nullptr || !m_constraint) {
            m_error = "Jolt vehicle runtime adapter was not initialized.";
            return false;
        }

        const float forward = Clamp(input.throttle, -1.0f, 1.0f);
        const float right = Clamp(input.steer, -1.0f, 1.0f);
        const float brake = Clamp(input.brake, 0.0f, 1.0f);
        m_controller->SetDriverInput(forward, right, brake, 0.0f);
        if (std::abs(forward) > 0.001f || std::abs(right) > 0.001f || brake > 0.001f) {
            m_physicsSystem->GetBodyInterface().ActivateBody(m_bodyId);
        }
        m_physicsSystem->Update(Clamp(deltaSeconds, 0.0f, 0.1f), 1, m_tempAllocator.get(), m_jobSystem.get());
        readState();
        m_state.frameIndex += 1;
        return true;
    }

    VehicleRuntimeState state() const override
    {
        return m_state;
    }

    std::string_view backendName() const override
    {
        return "jolt";
    }

    std::string_view error() const override
    {
        return m_error;
    }

private:
    void readState()
    {
        JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
        const JPH::Vec3 velocity = bodyInterface.GetLinearVelocity(m_bodyId);
        const JPH::Vec3 euler = bodyInterface.GetRotation(m_bodyId).GetEulerAngles();
        m_state.position = FromJoltRVec3(bodyInterface.GetPosition(m_bodyId));
        m_state.yawRadians = euler.GetY();
        m_state.speed = velocity.Length();
        m_state.wheelContactCount = CountWheelContacts(*m_constraint);
        m_state.maxPitchDegrees = std::abs(Degrees(euler.GetX()));
        m_state.maxRollDegrees = std::abs(Degrees(euler.GetZ()));
        m_state.outOfBounds = OutOfBounds(m_state.position, m_config);
    }

    VehicleRuntimeConfig m_config;
    VehicleRuntimeState m_state;
    std::string m_error;
    bool m_runtimeRetained = false;
    std::unique_ptr<JPH::TempAllocatorImpl> m_tempAllocator;
    std::unique_ptr<JPH::JobSystemSingleThreaded> m_jobSystem;
    std::unique_ptr<JPH::PhysicsSystem> m_physicsSystem;
    BroadPhaseLayerInterface m_broadPhaseLayerInterface;
    ObjectVsBroadPhaseLayerFilter m_objectVsBroadPhaseLayerFilter;
    ObjectLayerPairFilter m_objectLayerPairFilter;
    JPH::BodyID m_floorId;
    JPH::BodyID m_bodyId;
    JPH::Ref<JPH::VehicleConstraint> m_constraint;
    JPH::WheeledVehicleController* m_controller = nullptr;
};

} // namespace

std::unique_ptr<IVehicleRuntimeAdapter> CreateJoltVehicleRuntimeAdapterIfAvailable()
{
    return std::make_unique<JoltVehicleRuntimeAdapter>();
}

} // namespace engine::physics

#endif
