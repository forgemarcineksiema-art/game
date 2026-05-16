#pragma once

#include "engine/physics/VehicleProbe.h"
#include "engine/physics/VehicleRuntime.h"

#include <filesystem>
#include <string>
#include <vector>

struct FerryOfficeVehiclePhysicsQaResult {
    bool passed = false;
    std::string scenario = "ferry-office-vehicle-feasibility";
    std::string backendName;
    std::string sceneId;
    std::string vehicleId;
    std::filesystem::path scenePath;
    std::filesystem::path reportPath;
    engine::Vec3 spawnPosition;
    engine::Vec3 proxyHalfExtents;
    int inputFrameCount = 0;
    std::vector<engine::physics::VehicleProbeSample> samples;
    engine::Vec3 finalPosition;
    float finalYawRadians = 0.0f;
    float finalSpeed = 0.0f;
    std::string recommendation;
    std::string recommendationReason;
    std::string error;
};

struct FerryOfficeVehicleRuntimeComparisonResult {
    bool passed = false;
    std::string scenario = "ferry-office-vehicle-runtime-comparison";
    std::string adapterBackendName;
    std::string sceneId;
    std::string vehicleId;
    std::filesystem::path scenePath;
    std::filesystem::path reportPath;
    int inputFrameCount = 0;
    std::vector<engine::physics::VehicleProbeSample> deterministicSamples;
    std::vector<engine::physics::VehicleProbeSample> adapterSamples;
    struct ControlCheck {
        std::string name;
        bool passed = false;
        int frameIndex = 0;
        float speed = 0.0f;
        float distance = 0.0f;
        std::string message;
    };
    std::vector<ControlCheck> controlChecks;
    float maxPositionDelta = 0.0f;
    float maxYawDeltaDegrees = 0.0f;
    float maxSpeedDelta = 0.0f;
    std::string recommendation = "defer";
    std::string recommendationReason;
    std::string error;
};

std::filesystem::path DefaultFerryOfficeVehiclePhysicsReportPath();
std::filesystem::path DefaultFerryOfficeVehicleRuntimeComparisonReportPath();

FerryOfficeVehiclePhysicsQaResult RunFerryOfficeVehiclePhysicsQa(
    const std::filesystem::path& scenePath,
    const std::filesystem::path& reportPath = {},
    engine::physics::PhysicsBackend backend = engine::physics::OptInPhysicsBackend());

FerryOfficeVehicleRuntimeComparisonResult RunFerryOfficeVehicleRuntimeComparisonQa(
    const std::filesystem::path& scenePath,
    const std::filesystem::path& reportPath = {},
    engine::physics::PhysicsBackend backend = engine::physics::OptInPhysicsBackend());
