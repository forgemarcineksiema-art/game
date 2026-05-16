#include "game/FerryOfficeCharacterContactQa.h"

#include "game/FerryOfficeData.h"
#include "game/PrototypeWorld.h"
#include "game/SceneDefinition.h"
#include "game/SceneLoader.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <string_view>
#include <utility>

namespace {

constexpr std::string_view ScenarioName = "ferry-office-character-contact";
constexpr float PositionTolerance = 0.035f;
constexpr float PlayerRadius = 0.35f;
constexpr float PlayerHeight = 1.8f;

struct ContactProbe {
    std::string name;
    engine::Vec3 previousPosition;
    engine::Vec3 requestedPosition;
    engine::Vec3 velocity;
    bool gateOpen = false;
};

struct ContactCollider {
    std::string name;
    engine::Vec3 center;
    engine::Vec3 halfExtents;
};

struct PhysicsResolveResult {
    engine::Vec3 position;
    engine::Vec3 velocity;
    bool grounded = false;
    int hitCount = 0;
    std::vector<std::string> contacts;
};

std::vector<ContactProbe> ContactProbes()
{
    return {
        {"dock-floor-grounding", {0.0f, 0.2f, -1.0f}, {0.0f, -0.2f, -1.0f}, {0.0f, -1.0f, 0.0f}, false},
        {"office-back-wall-blocked", {0.0f, 0.0f, 4.35f}, {0.0f, 0.0f, 5.2f}, {0.0f, 0.0f, 1.0f}, false},
        {"service-gate-blocked", {0.0f, 0.0f, 1.45f}, {0.0f, 0.0f, 2.35f}, {0.0f, 0.0f, 1.0f}, false},
        {"service-barrier-blocked", {2.8f, 0.0f, -0.85f}, {2.8f, 0.0f, 0.25f}, {0.0f, 0.0f, 1.0f}, false},
        {"dock-lane-clear", {0.0f, 0.0f, -1.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, false},
        {"office-corner-pushout", {-2.2f, 0.0f, 4.35f}, {-2.8f, 0.0f, 5.2f}, {-1.0f, 0.0f, 1.0f}, false},
        {"opened-gate-clear", {0.0f, 0.0f, 1.45f}, {0.0f, 0.0f, 2.35f}, {0.0f, 0.0f, 1.0f}, true},
    };
}

std::vector<ContactCollider> BlockingSceneColliders(const SceneDefinition& scene, bool gateOpen)
{
    std::vector<ContactCollider> colliders;
    for (const SceneColliderDefinition& collider : scene.colliders) {
        if (collider.kind != "box" || !collider.blocksPlayer) {
            continue;
        }
        if (gateOpen && collider.id == FerryOffice::Names::ServiceGateCollider) {
            continue;
        }
        colliders.push_back({collider.id, collider.center, collider.halfExtents});
    }
    return colliders;
}

bool MirrorColliders(
    const std::vector<ContactCollider>& colliders,
    float floorHeight,
    engine::physics::IPhysicsWorld& physicsWorld)
{
    bool allAdded = physicsWorld.addFloor("scene-floor", floorHeight, 32.0f, 0.05f).isValid();
    for (const ContactCollider& collider : colliders) {
        engine::physics::BoxColliderDesc desc;
        desc.name = collider.name;
        desc.center = collider.center;
        desc.halfExtents = collider.halfExtents;
        allAdded = allAdded && physicsWorld.addStaticBox(desc).isValid();
    }
    return allAdded;
}

PrototypeWorld BuildPrototypeWorld(const SceneDefinition& scene, bool gateOpen)
{
    PrototypeWorld world;
    world.buildFromSceneDefinition(scene);
    if (gateOpen) {
        world.setColliderBlocksPlayer(FerryOffice::Names::ServiceGateCollider, false);
    }
    return world;
}

PlayerCollisionProxy MakeProxy(const ContactProbe& probe)
{
    PlayerCollisionProxy proxy;
    proxy.previousPosition = probe.previousPosition;
    proxy.position = probe.requestedPosition;
    proxy.velocity = probe.velocity;
    proxy.radius = PlayerRadius;
    proxy.height = PlayerHeight;
    return proxy;
}

bool OverlapContains(const engine::physics::IPhysicsWorld& physicsWorld, engine::Vec3 position, const std::string& colliderName)
{
    const engine::Vec3 halfExtents {PlayerRadius, PlayerHeight * 0.5f, PlayerRadius};
    const engine::Vec3 center = position + engine::Vec3 {0.0f, halfExtents.y, 0.0f};
    for (const engine::physics::OverlapResult& overlap : physicsWorld.overlapBox(center, halfExtents)) {
        if (overlap.bodyName == colliderName) {
            return true;
        }
    }
    return false;
}

engine::Vec3 ResolveContactCollider(
    const PlayerCollisionProxy& originalProxy,
    const ContactCollider& collider,
    engine::Vec3 position,
    PhysicsResolveResult& result)
{
    const float top = collider.center.y + collider.halfExtents.y;
    const float playerBottom = position.y;
    const float playerTop = position.y + originalProxy.height;
    if (playerTop < collider.center.y - collider.halfExtents.y || playerBottom > top + 0.05f) {
        return position;
    }

    const engine::Vec3 expandedHalfExtents = collider.halfExtents + engine::Vec3 {originalProxy.radius, 0.0f, originalProxy.radius};
    const float minX = collider.center.x - expandedHalfExtents.x;
    const float maxX = collider.center.x + expandedHalfExtents.x;
    const float minZ = collider.center.z - expandedHalfExtents.z;
    const float maxZ = collider.center.z + expandedHalfExtents.z;

    const int maxPasses = 3;
    bool contacted = false;
    for (int pass = 0; pass < maxPasses; ++pass) {
        if (position.x < minX || position.x > maxX || position.z < minZ || position.z > maxZ) {
            break;
        }

        const float pushLeft = maxX - position.x;
        const float pushRight = position.x - minX;
        const float pushForward = maxZ - position.z;
        const float pushBack = position.z - minZ;

        if (pass == 0) {
            const bool cameFromLeft = originalProxy.previousPosition.x <= minX && pushLeft <= pushRight;
            const bool cameFromRight = originalProxy.previousPosition.x >= maxX && pushRight <= pushLeft;
            const bool cameFromBack = originalProxy.previousPosition.z <= minZ && pushBack <= pushForward;
            const bool cameFromForward = originalProxy.previousPosition.z >= maxZ && pushForward <= pushBack;

            if (cameFromLeft) {
                position.x = minX;
                result.velocity.x = std::min(result.velocity.x, 0.0f);
            } else if (cameFromRight) {
                position.x = maxX;
                result.velocity.x = std::max(result.velocity.x, 0.0f);
            } else if (cameFromBack) {
                position.z = minZ;
                result.velocity.z = std::min(result.velocity.z, 0.0f);
            } else if (cameFromForward) {
                position.z = maxZ;
                result.velocity.z = std::max(result.velocity.z, 0.0f);
            } else {
                const float minPush = std::min({pushLeft, pushRight, pushBack, pushForward});
                if (minPush == pushLeft) {
                    position.x = maxX;
                } else if (minPush == pushRight) {
                    position.x = minX;
                } else if (minPush == pushBack) {
                    position.z = maxZ;
                } else {
                    position.z = minZ;
                }
            }

            result.hitCount += 1;
            contacted = true;
        } else {
            const float minPush = std::min({pushLeft, pushRight, pushBack, pushForward});
            if (minPush == pushLeft) {
                position.x = maxX;
                result.velocity.x = std::max(result.velocity.x, 0.0f);
            } else if (minPush == pushRight) {
                position.x = minX;
                result.velocity.x = std::min(result.velocity.x, 0.0f);
            } else if (minPush == pushBack) {
                position.z = maxZ;
                result.velocity.z = std::max(result.velocity.z, 0.0f);
            } else {
                position.z = minZ;
                result.velocity.z = std::min(result.velocity.z, 0.0f);
            }
        }
    }

    if (contacted) {
        result.contacts.push_back(collider.name);
    }
    return position;
}

PhysicsResolveResult ResolveWithPhysicsContacts(
    const ContactProbe& probe,
    const std::vector<ContactCollider>& colliders,
    const engine::physics::IPhysicsWorld& physicsWorld,
    float floorHeight)
{
    const PlayerCollisionProxy proxy = MakeProxy(probe);
    PhysicsResolveResult result;
    result.position = proxy.position;
    result.velocity = proxy.velocity;

    if (result.position.y <= floorHeight) {
        result.position.y = floorHeight;
        result.grounded = true;
        if (result.velocity.y < 0.0f) {
            result.velocity.y = 0.0f;
        }
    } else {
        result.grounded = result.position.y <= floorHeight + 0.05f;
    }

    for (const ContactCollider& collider : colliders) {
        if (!OverlapContains(physicsWorld, result.position, collider.name)) {
            continue;
        }
        result.position = ResolveContactCollider(proxy, collider, result.position, result);
    }

    if (result.position.y <= floorHeight) {
        result.position.y = floorHeight;
        result.grounded = true;
    }

    return result;
}

std::vector<std::string> PrototypeContacts(const PrototypeWorld& world, engine::Vec3 position)
{
    std::vector<std::string> contacts;
    for (const StaticCollider& collider : world.colliders()) {
        if (!collider.blocksPlayer) {
            continue;
        }
        if (world.playerOverlapsCollider(position, PlayerRadius, PlayerHeight, collider)) {
            contacts.push_back(collider.name);
        }
    }
    std::sort(contacts.begin(), contacts.end());
    return contacts;
}

std::vector<std::string> SortedContacts(std::vector<std::string> contacts)
{
    std::sort(contacts.begin(), contacts.end());
    return contacts;
}

bool NearVec3(engine::Vec3 lhs, engine::Vec3 rhs)
{
    return std::abs(lhs.x - rhs.x) <= PositionTolerance
        && std::abs(lhs.y - rhs.y) <= PositionTolerance
        && std::abs(lhs.z - rhs.z) <= PositionTolerance;
}

nlohmann::json Vec3Json(engine::Vec3 value)
{
    return nlohmann::json::array({value.x, value.y, value.z});
}

nlohmann::json ProbeJson(const FerryOfficeCharacterContactProbeResult& probe)
{
    return {
        {"name", probe.name},
        {"passed", probe.passed},
        {"gateOpen", probe.gateOpen},
        {"previousPosition", Vec3Json(probe.previousPosition)},
        {"requestedPosition", Vec3Json(probe.requestedPosition)},
        {"prototypePosition", Vec3Json(probe.prototypePosition)},
        {"physicsPosition", Vec3Json(probe.physicsPosition)},
        {"prototypeGrounded", probe.prototypeGrounded},
        {"physicsGrounded", probe.physicsGrounded},
        {"prototypeHitCount", probe.prototypeHitCount},
        {"physicsHitCount", probe.physicsHitCount},
        {"prototypeContacts", probe.prototypeContacts},
        {"physicsContacts", probe.physicsContacts},
        {"message", probe.message},
    };
}

bool WriteReport(const FerryOfficeCharacterContactQaResult& result)
{
    if (result.reportPath.empty()) {
        return true;
    }

    const std::filesystem::path parent = result.reportPath.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    nlohmann::json probes = nlohmann::json::array();
    for (const FerryOfficeCharacterContactProbeResult& probe : result.probes) {
        probes.push_back(ProbeJson(probe));
    }

    const nlohmann::json report = {
        {"schema", "v0.34-ferry-office-character-contact"},
        {"scenario", result.scenario},
        {"passed", result.passed},
        {"backend", result.backendName},
        {"scene", {{"id", result.sceneId}, {"path", result.scenePath.generic_string()}}},
        {"staticColliderCount", result.staticColliderCount},
        {"probes", probes},
        {"error", result.error},
    };

    std::ofstream output(result.reportPath);
    if (!output) {
        return false;
    }
    output << report.dump(2) << '\n';
    return true;
}

} // namespace

std::filesystem::path DefaultFerryOfficeCharacterContactReportPath()
{
    return std::filesystem::path("build") / "physics" / "ferry-office-character-contact-report.json";
}

FerryOfficeCharacterContactQaResult RunFerryOfficeCharacterContactQa(
    const std::filesystem::path& scenePath,
    const std::filesystem::path& requestedReportPath,
    engine::physics::PhysicsBackend backend)
{
    FerryOfficeCharacterContactQaResult result;
    result.scenario = std::string(ScenarioName);
    result.backendName = "unavailable";
    result.scenePath = scenePath;
    result.reportPath = requestedReportPath.empty() ? DefaultFerryOfficeCharacterContactReportPath() : requestedReportPath;

    const SceneLoadResult loadedScene = LoadSceneDefinition(scenePath);
    if (!loadedScene.ok()) {
        result.error = loadedScene.error;
        WriteReport(result);
        return result;
    }
    result.sceneId = loadedScene.scene.id;
    result.staticColliderCount = loadedScene.scene.colliders.size();

    engine::physics::PhysicsConfig config;
    config.backend = backend;
    result.passed = true;
    for (const ContactProbe& contactProbe : ContactProbes()) {
        const PrototypeWorld prototypeWorld = BuildPrototypeWorld(loadedScene.scene, contactProbe.gateOpen);
        const std::vector<ContactCollider> colliders = BlockingSceneColliders(loadedScene.scene, contactProbe.gateOpen);

        auto physicsWorld = engine::physics::CreatePhysicsWorld(backend);
        if (!physicsWorld) {
            result.error = "Requested physics backend is unavailable. Use the opt-in physics configure/build preset for character contact QA.";
            result.passed = false;
            break;
        }
        if (!physicsWorld->initialize(config)) {
            result.error = "Failed to initialize physics world for character contact probe.";
            result.passed = false;
            break;
        }
        result.backendName = std::string(physicsWorld->backendName());
        if (!MirrorColliders(colliders, loadedScene.scene.floorHeight, *physicsWorld)) {
            result.error = "Failed to mirror Ferry Office character contact colliders.";
            result.passed = false;
            physicsWorld->shutdown();
            break;
        }

        const PlayerCollisionProxy proxy = MakeProxy(contactProbe);
        const CollisionResult prototype = prototypeWorld.resolvePlayer(proxy);
        const PhysicsResolveResult physics =
            ResolveWithPhysicsContacts(contactProbe, colliders, *physicsWorld, prototypeWorld.floorHeight());

        FerryOfficeCharacterContactProbeResult probe;
        probe.name = contactProbe.name;
        probe.gateOpen = contactProbe.gateOpen;
        probe.previousPosition = contactProbe.previousPosition;
        probe.requestedPosition = contactProbe.requestedPosition;
        probe.prototypePosition = prototype.position;
        probe.physicsPosition = physics.position;
        probe.prototypeGrounded = prototype.grounded;
        probe.physicsGrounded = physics.grounded;
        probe.prototypeHitCount = prototype.hitCount;
        probe.physicsHitCount = physics.hitCount;
        probe.prototypeContacts = PrototypeContacts(prototypeWorld, contactProbe.requestedPosition);
        probe.physicsContacts = SortedContacts(physics.contacts);
        probe.passed = NearVec3(probe.prototypePosition, probe.physicsPosition)
            && probe.prototypeGrounded == probe.physicsGrounded
            && probe.prototypeHitCount == probe.physicsHitCount;
        probe.message = probe.passed ? "Character contact probe matched PrototypeWorld resolve." : "Character contact probe mismatch.";
        result.passed = result.passed && probe.passed;
        result.probes.push_back(std::move(probe));
        physicsWorld->shutdown();
    }

    if (!result.passed && result.error.empty()) {
        result.error = "Ferry Office character contact probes did not all match PrototypeWorld.";
    }
    if (!WriteReport(result)) {
        result.passed = false;
        result.error = "Failed to write character contact report: " + result.reportPath.string();
    }

    return result;
}
