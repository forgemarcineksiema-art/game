#pragma once

#include "engine/math/Math.h"
#include "engine/physics/PhysicsWorld.h"

#include <filesystem>
#include <string>
#include <vector>

struct FerryOfficeCharacterContactProbeResult {
    std::string name;
    bool passed = false;
    bool gateOpen = false;
    engine::Vec3 previousPosition;
    engine::Vec3 requestedPosition;
    engine::Vec3 prototypePosition;
    engine::Vec3 physicsPosition;
    bool prototypeGrounded = false;
    bool physicsGrounded = false;
    int prototypeHitCount = 0;
    int physicsHitCount = 0;
    std::vector<std::string> prototypeContacts;
    std::vector<std::string> physicsContacts;
    std::string message;
};

struct FerryOfficeCharacterContactQaResult {
    bool passed = false;
    std::string scenario = "ferry-office-character-contact";
    std::string backendName;
    std::string sceneId;
    std::filesystem::path scenePath;
    std::filesystem::path reportPath;
    std::size_t staticColliderCount = 0;
    std::vector<FerryOfficeCharacterContactProbeResult> probes;
    std::string error;
};

std::filesystem::path DefaultFerryOfficeCharacterContactReportPath();

FerryOfficeCharacterContactQaResult RunFerryOfficeCharacterContactQa(
    const std::filesystem::path& scenePath,
    const std::filesystem::path& reportPath = {},
    engine::physics::PhysicsBackend backend = engine::physics::OptInPhysicsBackend());
