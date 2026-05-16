#include "engine/physics/VehicleProbe.h"

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

#include <algorithm>
#include <cmath>
#include <cfloat>
#include <memory>
#include <mutex>

namespace engine::physics {
namespace {

void JoltVehicleTrace(const char*, ...)
{
}

#ifdef JPH_ENABLE_ASSERTS
bool JoltVehicleAssertFailed(const char*, const char*, const char*, JPH::uint)
{
    return true;
}
#endif

class JoltVehicleRuntime {
public:
    static void retain()
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_refCount == 0) {
            JPH::RegisterDefaultAllocator();
            JPH::Trace = JoltVehicleTrace;
            JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = JoltVehicleAssertFailed;)
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
        if (object1 == Layers::NonMoving) {
            return object2 == Layers::Moving;
        }
        if (object1 == Layers::Moving) {
            return true;
        }
        return false;
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
        return layer < Layers::NumLayers ? m_objectToBroadPhase[layer] : BroadPhaseLayers::NonMoving;
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override
    {
        return static_cast<JPH::BroadPhaseLayer::Type>(layer) == static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::Moving)
            ? "Moving"
            : "NonMoving";
    }
#endif

private:
    JPH::BroadPhaseLayer m_objectToBroadPhase[Layers::NumLayers];
};

class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const override
    {
        if (layer1 == Layers::NonMoving) {
            return layer2 == BroadPhaseLayers::Moving;
        }
        if (layer1 == Layers::Moving) {
            return true;
        }
        return false;
    }
};

JPH::Vec3 ToJoltVec3(Vec3 value)
{
    return {value.x, value.y, value.z};
}

JPH::RVec3 ToJoltRVec3(Vec3 value)
{
    return {value.x, value.y, value.z};
}

Vec3 FromJoltRVec3(JPH::RVec3 value)
{
    return {
        static_cast<float>(value.GetX()),
        static_cast<float>(value.GetY()),
        static_cast<float>(value.GetZ()),
    };
}

float YawFromVelocityOrFallback(JPH::Vec3 velocity, float fallbackYawRadians)
{
    const Vec3 horizontalVelocity {
        velocity.GetX(),
        0.0f,
        velocity.GetZ(),
    };
    if (Length(horizontalVelocity) <= 0.01f) {
        return fallbackYawRadians;
    }
    return YawFromDirection(horizontalVelocity);
}

bool OutOfBounds(Vec3 position, const VehicleProbeConfig& config)
{
    return position.x < config.boundsMin.x || position.x > config.boundsMax.x
        || position.z < config.boundsMin.y || position.z > config.boundsMax.y;
}

int CountWheelContacts(const JPH::VehicleConstraint& constraint)
{
    int count = 0;
    for (const JPH::Wheel* wheel : constraint.GetWheels()) {
        if (!wheel->GetContactBodyID().IsInvalid()) {
            ++count;
        }
    }
    return count;
}

} // namespace

VehicleProbeResult RunJoltVehicleFeasibilityProbeIfAvailable(
    const VehicleProbeConfig& config,
    const std::vector<VehicleProbeInputFrame>& inputScript)
{
    VehicleProbeResult result;
    result.backendName = "jolt";
    result.vehicleId = config.vehicleId;
    result.spawnPosition = config.spawnPosition;
    result.halfExtents = config.halfExtents;
    result.finalPosition = config.spawnPosition;
    result.finalYawRadians = config.spawnYawRadians;

    JoltVehicleRuntime::retain();

    BroadPhaseLayerInterfaceImpl broadPhaseLayerInterface;
    ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseLayerFilter;
    ObjectLayerPairFilterImpl objectLayerPairFilter;
    JPH::PhysicsSystem physicsSystem;
    physicsSystem.Init(1024, 0, 1024, 1024, broadPhaseLayerInterface, objectVsBroadPhaseLayerFilter, objectLayerPairFilter);
    auto tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(4 * 1024 * 1024);
    auto jobSystem = std::make_unique<JPH::JobSystemSingleThreaded>(JPH::cMaxPhysicsJobs);
    JPH::BodyInterface& bodyInterface = physicsSystem.GetBodyInterface();

    JPH::BodyID floorId;
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
            result.error = "Failed to create Jolt vehicle probe floor.";
            JoltVehicleRuntime::release();
            return result;
        }
        floorId = floorBody->GetID();
        bodyInterface.AddBody(floorId, JPH::EActivation::DontActivate);
    }

    const float halfWidth = std::max(config.halfExtents.x, 0.25f);
    const float halfHeight = std::max(config.halfExtents.y * 0.45f, 0.18f);
    const float halfLength = std::max(config.halfExtents.z, 0.5f);
    JPH::RefConst<JPH::Shape> carShape =
        JPH::OffsetCenterOfMassShapeSettings(
            JPH::Vec3(0.0f, -halfHeight, 0.0f),
            new JPH::BoxShape(JPH::Vec3(halfWidth, halfHeight, halfLength)))
            .Create()
            .Get();
    JPH::BodyCreationSettings carBodySettings(
        carShape,
        ToJoltRVec3(config.spawnPosition + Vec3 {0.0f, 1.1f, 0.0f}),
        JPH::Quat::sRotation(JPH::Vec3::sAxisY(), config.spawnYawRadians),
        JPH::EMotionType::Dynamic,
        Layers::Moving);
    carBodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
    carBodySettings.mMassPropertiesOverride.mMass = 900.0f;
    JPH::Body* carBody = bodyInterface.CreateBody(carBodySettings);
    if (carBody == nullptr) {
        bodyInterface.RemoveBody(floorId);
        bodyInterface.DestroyBody(floorId);
        result.error = "Failed to create Jolt vehicle body.";
        JoltVehicleRuntime::release();
        return result;
    }
    bodyInterface.AddBody(carBody->GetID(), JPH::EActivation::Activate);

    JPH::VehicleConstraintSettings vehicle;
    vehicle.mDrawConstraintSize = 0.1f;
    vehicle.mMaxPitchRollAngle = JPH::DegreesToRadians(60.0f);
    constexpr int frontLeft = 0;
    constexpr int frontRight = 1;
    constexpr int backLeft = 2;
    constexpr int backRight = 3;
    const float wheelRadius = 0.24f;
    const float wheelWidth = 0.12f;
    const float wheelZ = std::max(halfLength * 0.72f, 0.45f);
    const float wheelX = std::max(halfWidth * 0.82f, 0.35f);
    const float wheelY = std::max(halfHeight * 0.5f, 0.12f);
    vehicle.mWheels.resize(4);
    vehicle.mWheels[frontLeft] = new JPH::WheelSettingsWV();
    vehicle.mWheels[frontRight] = new JPH::WheelSettingsWV();
    vehicle.mWheels[backLeft] = new JPH::WheelSettingsWV();
    vehicle.mWheels[backRight] = new JPH::WheelSettingsWV();
    vehicle.mWheels[frontLeft]->mPosition = JPH::Vec3(wheelX, -wheelY, wheelZ);
    vehicle.mWheels[frontRight]->mPosition = JPH::Vec3(-wheelX, -wheelY, wheelZ);
    vehicle.mWheels[backLeft]->mPosition = JPH::Vec3(wheelX, -wheelY, -wheelZ);
    vehicle.mWheels[backRight]->mPosition = JPH::Vec3(-wheelX, -wheelY, -wheelZ);
    for (std::size_t index = 0; index < vehicle.mWheels.size(); ++index) {
        JPH::WheelSettings* wheel = vehicle.mWheels[index];
        wheel->mRadius = wheelRadius;
        wheel->mWidth = wheelWidth;
        wheel->mSuspensionMinLength = 0.22f;
        wheel->mSuspensionMaxLength = 0.45f;
        if (index == frontLeft || index == frontRight) {
            static_cast<JPH::WheelSettingsWV*>(wheel)->mMaxSteerAngle = JPH::DegreesToRadians(30.0f);
            static_cast<JPH::WheelSettingsWV*>(wheel)->mMaxHandBrakeTorque = 0.0f;
        } else {
            static_cast<JPH::WheelSettingsWV*>(wheel)->mMaxSteerAngle = 0.0f;
        }
    }

    auto* controllerSettings = new JPH::WheeledVehicleControllerSettings();
    controllerSettings->mDifferentials.resize(1);
    controllerSettings->mDifferentials[0].mLeftWheel = frontLeft;
    controllerSettings->mDifferentials[0].mRightWheel = frontRight;
    controllerSettings->mDifferentials[0].mLimitedSlipRatio = 1.4f;
    controllerSettings->mDifferentialLimitedSlipRatio = 1.4f;
    vehicle.mController = controllerSettings;

    JPH::VehicleConstraint* constraint = new JPH::VehicleConstraint(*carBody, vehicle);
    constraint->SetVehicleCollisionTester(new JPH::VehicleCollisionTesterRay(Layers::Moving));
    physicsSystem.AddConstraint(constraint);
    physicsSystem.AddStepListener(constraint);
    auto* controller = static_cast<JPH::WheeledVehicleController*>(constraint->GetController());

    bool everMoved = false;
    bool anyOutOfBounds = false;
    bool samplesStable = true;
    for (const VehicleProbeInputFrame& input : inputScript) {
        for (int frame = 0; frame < input.frameCount; ++frame) {
            const float forward = Clamp(input.throttle, -1.0f, 1.0f);
            const float right = Clamp(input.steer, -1.0f, 1.0f);
            const float brake = Clamp(input.brake, 0.0f, 1.0f);
            controller->SetDriverInput(forward, right, brake, 0.0f);
            if (std::abs(forward) > 0.001f || std::abs(right) > 0.001f || brake > 0.001f) {
                bodyInterface.ActivateBody(carBody->GetID());
            }
            physicsSystem.Update(config.fixedStepSeconds, 1, tempAllocator.get(), jobSystem.get());
        }

        const Vec3 position = FromJoltRVec3(bodyInterface.GetPosition(carBody->GetID()));
        const JPH::Vec3 linearVelocity = bodyInterface.GetLinearVelocity(carBody->GetID());
        const JPH::Vec3 rotationEuler = bodyInterface.GetRotation(carBody->GetID()).GetEulerAngles();
        const float speed = linearVelocity.Length();
        const int wheelContacts = CountWheelContacts(*constraint);
        VehicleProbeSample sample;
        sample.name = input.name;
        sample.position = position;
        sample.yawRadians = YawFromVelocityOrFallback(linearVelocity, config.spawnYawRadians);
        sample.speed = speed;
        sample.wheelContactCount = wheelContacts;
        sample.maxPitchDegrees = std::abs(Degrees(rotationEuler.GetX()));
        sample.maxRollDegrees = std::abs(Degrees(rotationEuler.GetZ()));
        sample.outOfBounds = OutOfBounds(position, config);
        sample.passed = wheelContacts >= 2
            && !sample.outOfBounds
            && sample.maxPitchDegrees <= 35.0f
            && sample.maxRollDegrees <= 35.0f;
        sample.message = sample.passed ? "Jolt wheeled vehicle sample stayed stable." : "Jolt wheeled vehicle sample did not meet stability thresholds.";
        samplesStable = samplesStable && sample.passed;
        everMoved = everMoved || speed > 0.25f;
        anyOutOfBounds = anyOutOfBounds || sample.outOfBounds;
        result.samples.push_back(std::move(sample));
        result.inputFrameCount += input.frameCount;
    }

    result.finalPosition = FromJoltRVec3(bodyInterface.GetPosition(carBody->GetID()));
    const JPH::Vec3 finalLinearVelocity = bodyInterface.GetLinearVelocity(carBody->GetID());
    result.finalSpeed = finalLinearVelocity.Length();
    result.finalYawRadians = YawFromVelocityOrFallback(finalLinearVelocity, config.spawnYawRadians);
    result.passed = !inputScript.empty() && everMoved && samplesStable && !anyOutOfBounds;
    result.recommendation = result.passed ? "promote" : "defer";
    result.recommendationReason = result.passed
        ? "The opt-in Jolt wheeled-vehicle constraint stayed stable enough to promote to a narrower runtime adapter spike."
        : "The opt-in Jolt wheeled-vehicle constraint did not yet meet stability thresholds for runtime promotion.";
    if (!result.passed) {
        result.error = "Jolt vehicle feasibility probe did not meet stability thresholds.";
    }

    physicsSystem.RemoveStepListener(constraint);
    physicsSystem.RemoveConstraint(constraint);
    bodyInterface.RemoveBody(carBody->GetID());
    bodyInterface.DestroyBody(carBody->GetID());
    bodyInterface.RemoveBody(floorId);
    bodyInterface.DestroyBody(floorId);
    JoltVehicleRuntime::release();
    return result;
}

} // namespace engine::physics

#endif // ENGINE_WITH_JOLT
