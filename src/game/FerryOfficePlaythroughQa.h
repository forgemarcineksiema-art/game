#pragma once

#include "engine/physics/VehicleRuntime.h"
#include "game/FerryOfficeJob.h"
#include "game/WorldState.h"

#include <filesystem>
#include <string>
#include <vector>

struct FerryOfficePlaythroughQaStep {
    std::string name;
    bool passed = false;
    std::string phase;
    std::string message;
};

struct FerryOfficePlaythroughQaResult {
    bool passed = false;
    std::string scenario = "ferry-office-service-call";
    std::string sceneId;
    std::filesystem::path scenePath;
    std::filesystem::path reportPath;
    std::vector<FerryOfficePlaythroughQaStep> steps;
    WorldState finalWorldState;
    FerryOfficeJobPhase finalPhase = FerryOfficeJobPhase::CollectManifest;
    std::string requestedVehicleRuntime = "deterministic";
    std::string vehicleRuntimeBackend = "deterministic";
    bool vehicleRuntimeFallbackUsed = false;
    bool vehicleRuntimeHitBounds = false;
    int vehicleRuntimeFramesToCheckpoint = -1;
    engine::Vec3 vehicleRuntimeFinalPosition;
    float vehicleRuntimeFinalYawRadians = 0.0f;
    std::string error;
};

std::filesystem::path DefaultFerryOfficePlaythroughQaReportPath();

FerryOfficePlaythroughQaResult RunFerryOfficeServiceCallPlaythroughQa(
    const std::filesystem::path& scenePath,
    const std::filesystem::path& reportPath = {},
    engine::physics::PhysicsBackend vehicleRuntimeBackend = engine::physics::PhysicsBackend::Simple,
    bool vehicleRuntimeAdapterEnabled = false);
