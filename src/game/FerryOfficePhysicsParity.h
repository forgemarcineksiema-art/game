#pragma once

#include "engine/math/Math.h"
#include "engine/physics/PhysicsWorld.h"

#include <filesystem>
#include <string>
#include <vector>

struct FerryOfficeFloorProbeResult {
    std::string name;
    bool passed = false;
    engine::Vec3 position;
    bool prototypeGrounded = false;
    bool physicsGrounded = false;
    float physicsDistance = 0.0f;
    std::string message;
};

struct FerryOfficeRaycastProbeResult {
    std::string name;
    bool passed = false;
    engine::Vec3 origin;
    engine::Vec3 direction;
    float maxDistance = 0.0f;
    bool prototypeHit = false;
    bool physicsHit = false;
    std::string prototypeCollider;
    std::string physicsBody;
    float prototypeDistance = 0.0f;
    float physicsDistance = 0.0f;
    std::string message;
};

struct FerryOfficeOverlapProbeResult {
    std::string name;
    bool passed = false;
    engine::Vec3 position;
    std::vector<std::string> prototypeColliders;
    std::vector<std::string> physicsBodies;
    std::string message;
};

struct FerryOfficePhysicsParityResult {
    bool passed = false;
    std::string scenario = "ferry-office-collision";
    std::string backendName;
    std::string sceneId;
    std::filesystem::path scenePath;
    std::filesystem::path reportPath;
    std::size_t staticColliderCount = 0;
    std::vector<FerryOfficeFloorProbeResult> floorProbes;
    std::vector<FerryOfficeRaycastProbeResult> raycastProbes;
    std::vector<FerryOfficeOverlapProbeResult> overlapProbes;
    std::string error;
};

std::filesystem::path DefaultFerryOfficePhysicsParityReportPath();

FerryOfficePhysicsParityResult RunFerryOfficePhysicsParityQa(
    const std::filesystem::path& scenePath,
    const std::filesystem::path& reportPath = {},
    engine::physics::PhysicsBackend backend = engine::physics::OptInPhysicsBackend());
