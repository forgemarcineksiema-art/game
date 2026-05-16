#pragma once

#include "engine/physics/VehicleProbe.h"

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

std::filesystem::path DefaultFerryOfficeVehiclePhysicsReportPath();

FerryOfficeVehiclePhysicsQaResult RunFerryOfficeVehiclePhysicsQa(
    const std::filesystem::path& scenePath,
    const std::filesystem::path& reportPath = {},
    engine::physics::PhysicsBackend backend = engine::physics::OptInPhysicsBackend());
