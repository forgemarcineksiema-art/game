#include "game/FerryOfficePhysicsParity.h"

#include "game/PrototypeWorld.h"
#include "game/SceneDefinition.h"
#include "game/SceneLoader.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include <set>
#include <string_view>
#include <utility>

namespace {

constexpr std::string_view ScenarioName = "ferry-office-collision";
constexpr float DistanceTolerance = 0.025f;
constexpr float FloorRayHeight = 1.25f;
constexpr float FloorProbeDistance = 0.05f;

struct RaycastProbe {
    std::string name;
    engine::Vec3 origin;
    engine::Vec3 direction;
    float maxDistance = 0.0f;
    bool expectedHit = false;
    std::string expectedName;
};

struct OverlapProbe {
    std::string name;
    engine::Vec3 position;
    float radius = 0.35f;
    float height = 1.8f;
};

std::vector<engine::Vec3> FloorProbePositions()
{
    return {
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, 3.25f},
        {6.3f, 0.0f, -2.2f},
        {17.8f, 0.0f, -2.2f},
    };
}

std::vector<std::string> FloorProbeNames()
{
    return {
        "dock-start",
        "ferry-office-floor",
        "service-yard-pad",
        "dock-road-turnaround",
    };
}

std::vector<RaycastProbe> RaycastProbes()
{
    return {
        {"service-gate-front", {0.0f, 0.75f, 1.2f}, {0.0f, 0.0f, 1.0f}, 3.0f, true, "service-gate"},
        {"office-back-wall", {0.0f, 0.75f, 3.0f}, {0.0f, 0.0f, 1.0f}, 4.0f, true, "ferry-office-back-wall"},
        {"dock-left-rail", {-3.4f, 0.45f, -0.5f}, {-1.0f, 0.0f, 0.0f}, 2.0f, true, "dock-rail-left"},
        {"dock-lane-clear", {0.0f, 0.5f, -1.0f}, {1.0f, 0.0f, 0.0f}, 3.0f, false, ""},
    };
}

std::vector<OverlapProbe> OverlapProbes()
{
    return {
        {"service-gate-blocked", {0.0f, 0.0f, 2.35f}},
        {"office-counter-blocked", {-1.35f, 0.0f, 1.45f}},
        {"dock-lane-clear", {0.0f, 0.0f, -1.0f}},
        {"dock-right-rail-blocked", {4.2f, 0.0f, -0.5f}},
    };
}

bool MirrorSceneCollidersIntoPhysicsWorld(const SceneDefinition& scene, engine::physics::IPhysicsWorld& physicsWorld)
{
    physicsWorld.addFloor("scene-floor", scene.floorHeight, 32.0f, 0.05f);
    bool allAdded = true;
    for (const SceneColliderDefinition& collider : scene.colliders) {
        if (collider.kind != "box" || !collider.blocksPlayer) {
            continue;
        }

        engine::physics::BoxColliderDesc desc;
        desc.name = collider.id;
        desc.center = collider.center;
        desc.halfExtents = collider.halfExtents;
        const engine::physics::BodyHandle handle = physicsWorld.addStaticBox(desc);
        allAdded = allAdded && handle.isValid();
    }
    return allAdded;
}

std::vector<std::string> PrototypeOverlapsForProbe(const PrototypeWorld& world, const OverlapProbe& probe)
{
    std::vector<std::string> names;
    for (const StaticCollider& collider : world.colliders()) {
        if (!collider.blocksPlayer) {
            continue;
        }
        if (world.playerOverlapsCollider(probe.position, probe.radius, probe.height, collider)) {
            names.push_back(collider.name);
        }
    }
    std::sort(names.begin(), names.end());
    return names;
}

std::vector<std::string> PhysicsOverlapsForProbe(const engine::physics::IPhysicsWorld& physicsWorld, const OverlapProbe& probe)
{
    const engine::Vec3 halfExtents {probe.radius, std::max(probe.height * 0.5f, 0.01f), probe.radius};
    const engine::Vec3 center = probe.position + engine::Vec3 {0.0f, halfExtents.y, 0.0f};
    std::vector<std::string> names;
    for (const engine::physics::OverlapResult& overlap : physicsWorld.overlapBox(center, halfExtents)) {
        if (overlap.bodyName != "scene-floor") {
            names.push_back(overlap.bodyName);
        }
    }
    std::sort(names.begin(), names.end());
    return names;
}

bool SameNames(const std::vector<std::string>& lhs, const std::vector<std::string>& rhs)
{
    return lhs == rhs;
}

nlohmann::json Vec3Json(engine::Vec3 value)
{
    return nlohmann::json::array({value.x, value.y, value.z});
}

nlohmann::json FloorProbesJson(const std::vector<FerryOfficeFloorProbeResult>& probes)
{
    nlohmann::json output = nlohmann::json::array();
    for (const FerryOfficeFloorProbeResult& probe : probes) {
        output.push_back({
            {"name", probe.name},
            {"passed", probe.passed},
            {"position", Vec3Json(probe.position)},
            {"prototypeGrounded", probe.prototypeGrounded},
            {"physicsGrounded", probe.physicsGrounded},
            {"physicsDistance", probe.physicsDistance},
            {"message", probe.message},
        });
    }
    return output;
}

nlohmann::json RaycastProbesJson(const std::vector<FerryOfficeRaycastProbeResult>& probes)
{
    nlohmann::json output = nlohmann::json::array();
    for (const FerryOfficeRaycastProbeResult& probe : probes) {
        output.push_back({
            {"name", probe.name},
            {"passed", probe.passed},
            {"origin", Vec3Json(probe.origin)},
            {"direction", Vec3Json(probe.direction)},
            {"maxDistance", probe.maxDistance},
            {"prototypeHit", probe.prototypeHit},
            {"physicsHit", probe.physicsHit},
            {"prototypeCollider", probe.prototypeCollider},
            {"physicsBody", probe.physicsBody},
            {"prototypeDistance", probe.prototypeDistance},
            {"physicsDistance", probe.physicsDistance},
            {"message", probe.message},
        });
    }
    return output;
}

nlohmann::json OverlapProbesJson(const std::vector<FerryOfficeOverlapProbeResult>& probes)
{
    nlohmann::json output = nlohmann::json::array();
    for (const FerryOfficeOverlapProbeResult& probe : probes) {
        output.push_back({
            {"name", probe.name},
            {"passed", probe.passed},
            {"position", Vec3Json(probe.position)},
            {"prototypeColliders", probe.prototypeColliders},
            {"physicsBodies", probe.physicsBodies},
            {"message", probe.message},
        });
    }
    return output;
}

bool WriteReport(const FerryOfficePhysicsParityResult& result)
{
    if (result.reportPath.empty()) {
        return true;
    }

    const std::filesystem::path parent = result.reportPath.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    const nlohmann::json report = {
        {"schema", "v0.33-ferry-office-physics-parity"},
        {"scenario", result.scenario},
        {"passed", result.passed},
        {"backend", result.backendName},
        {"scene", {{"id", result.sceneId}, {"path", result.scenePath.generic_string()}}},
        {"staticColliderCount", result.staticColliderCount},
        {"floorProbes", FloorProbesJson(result.floorProbes)},
        {"raycastProbes", RaycastProbesJson(result.raycastProbes)},
        {"overlapProbes", OverlapProbesJson(result.overlapProbes)},
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

std::filesystem::path DefaultFerryOfficePhysicsParityReportPath()
{
    return std::filesystem::path("build") / "physics" / "ferry-office-collision-parity-report.json";
}

FerryOfficePhysicsParityResult RunFerryOfficePhysicsParityQa(
    const std::filesystem::path& scenePath,
    const std::filesystem::path& requestedReportPath,
    engine::physics::PhysicsBackend backend)
{
    FerryOfficePhysicsParityResult result;
    result.scenario = std::string(ScenarioName);
    result.backendName = "unavailable";
    result.scenePath = scenePath;
    result.reportPath = requestedReportPath.empty() ? DefaultFerryOfficePhysicsParityReportPath() : requestedReportPath;

    const SceneLoadResult loadedScene = LoadSceneDefinition(scenePath);
    if (!loadedScene.ok()) {
        result.error = loadedScene.error;
        WriteReport(result);
        return result;
    }
    result.sceneId = loadedScene.scene.id;

    auto physicsWorld = engine::physics::CreatePhysicsWorld(backend);
    if (!physicsWorld) {
        result.error = "Requested physics backend is unavailable. Use the opt-in physics configure/build preset for parity.";
        WriteReport(result);
        return result;
    }

    engine::physics::PhysicsConfig config;
    config.backend = backend;
    if (!physicsWorld->initialize(config)) {
        result.error = "Failed to initialize " + result.backendName + " physics world.";
        WriteReport(result);
        return result;
    }
    result.backendName = std::string(physicsWorld->backendName());

    PrototypeWorld prototypeWorld;
    prototypeWorld.buildFromSceneDefinition(loadedScene.scene);
    result.staticColliderCount = prototypeWorld.colliders().size();

    if (!MirrorSceneCollidersIntoPhysicsWorld(loadedScene.scene, *physicsWorld)) {
        result.error = "Failed to mirror all Ferry Office colliders into the physics world.";
    }

    const std::vector<engine::Vec3> floorPositions = FloorProbePositions();
    const std::vector<std::string> floorNames = FloorProbeNames();
    for (std::size_t index = 0; index < floorPositions.size(); ++index) {
        FerryOfficeFloorProbeResult probe;
        probe.name = floorNames[index];
        probe.position = floorPositions[index];
        probe.prototypeGrounded = prototypeWorld.isGrounded(probe.position, FloorProbeDistance);
        const engine::Vec3 rayOrigin = probe.position + engine::Vec3 {0.0f, FloorRayHeight, 0.0f};
        const engine::physics::RaycastResult physicsHit =
            physicsWorld->raycast(rayOrigin, {0.0f, -1.0f, 0.0f}, FloorRayHeight + 0.5f);
        probe.physicsGrounded = physicsHit.hit && physicsHit.bodyName == "scene-floor";
        probe.physicsDistance = physicsHit.distance;
        probe.passed = probe.prototypeGrounded && probe.physicsGrounded
            && std::abs(probe.physicsDistance - FloorRayHeight) <= 0.075f;
        probe.message = probe.passed ? "Floor probe matched PrototypeWorld floor height." : "Floor probe mismatch.";
        result.floorProbes.push_back(std::move(probe));
    }

    for (const RaycastProbe& expected : RaycastProbes()) {
        FerryOfficeRaycastProbeResult probe;
        probe.name = expected.name;
        probe.origin = expected.origin;
        probe.direction = expected.direction;
        probe.maxDistance = expected.maxDistance;
        const RaycastHit prototypeHit = prototypeWorld.raycast(expected.origin, expected.direction, expected.maxDistance);
        const engine::physics::RaycastResult physicsHit = physicsWorld->raycast(expected.origin, expected.direction, expected.maxDistance);
        probe.prototypeHit = prototypeHit.hit;
        probe.physicsHit = physicsHit.hit;
        probe.prototypeCollider = prototypeHit.colliderName;
        probe.physicsBody = physicsHit.bodyName;
        probe.prototypeDistance = prototypeHit.distance;
        probe.physicsDistance = physicsHit.distance;
        probe.passed = probe.prototypeHit == expected.expectedHit
            && probe.physicsHit == expected.expectedHit
            && (!expected.expectedHit
                || (probe.prototypeCollider == expected.expectedName
                    && probe.physicsBody == expected.expectedName
                    && std::abs(probe.prototypeDistance - probe.physicsDistance) <= DistanceTolerance));
        probe.message = probe.passed ? "Raycast probe matched PrototypeWorld." : "Raycast probe mismatch.";
        result.raycastProbes.push_back(std::move(probe));
    }

    for (const OverlapProbe& expected : OverlapProbes()) {
        FerryOfficeOverlapProbeResult probe;
        probe.name = expected.name;
        probe.position = expected.position;
        probe.prototypeColliders = PrototypeOverlapsForProbe(prototypeWorld, expected);
        probe.physicsBodies = PhysicsOverlapsForProbe(*physicsWorld, expected);
        probe.passed = SameNames(probe.prototypeColliders, probe.physicsBodies);
        probe.message = probe.passed ? "Overlap probe matched PrototypeWorld." : "Overlap probe mismatch.";
        result.overlapProbes.push_back(std::move(probe));
    }

    result.passed = result.error.empty();
    for (const FerryOfficeFloorProbeResult& probe : result.floorProbes) {
        result.passed = result.passed && probe.passed;
    }
    for (const FerryOfficeRaycastProbeResult& probe : result.raycastProbes) {
        result.passed = result.passed && probe.passed;
    }
    for (const FerryOfficeOverlapProbeResult& probe : result.overlapProbes) {
        result.passed = result.passed && probe.passed;
    }
    if (!result.passed && result.error.empty()) {
        result.error = "Ferry Office physics parity probes did not all match PrototypeWorld.";
    }
    if (!WriteReport(result)) {
        result.passed = false;
        result.error = "Failed to write physics parity report: " + result.reportPath.string();
    }

    physicsWorld->shutdown();
    return result;
}
