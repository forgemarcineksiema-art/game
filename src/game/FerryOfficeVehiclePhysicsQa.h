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
    struct RouteCheck {
        std::string backendName;
        bool passed = false;
        bool checkpointReached = false;
        int framesToCheckpoint = 0;
        float minDistanceToCheckpoint = 0.0f;
        engine::Vec3 finalPosition;
        float finalYawRadians = 0.0f;
        bool hitBounds = false;
        std::string message;
    };
    struct ObstacleCheck {
        std::string backendName;
        bool passed = false;
        bool clearedObstacleProxy = false;
        int frameCount = 0;
        float maxLateralOffset = 0.0f;
        float minDistanceToObstacle = 0.0f;
        bool collisionBacked = false;
        bool obstacleCollisionClear = false;
        int obstacleOverlapFrames = 0;
        float minCollisionClearance = 0.0f;
        float maxCameraYawDeltaDegrees = 0.0f;
        float finalCameraYawRadians = 0.0f;
        engine::Vec3 finalPosition;
        float finalYawRadians = 0.0f;
        bool hitBounds = false;
        std::string message;
    };
    struct DrivingFeelCheck {
        std::string backendName;
        std::string name;
        bool passed = false;
        float value = 0.0f;
        float minValue = 0.0f;
        float maxValue = 0.0f;
        std::string units;
        std::string message;
    };
    struct RoutePaceProbe {
        std::string backendName;
        std::string name;
        float throttle = 0.0f;
        bool passed = false;
        bool checkpointReached = false;
        bool stable = false;
        int framesToCheckpoint = 0;
        float minDistanceToCheckpoint = 0.0f;
        engine::Vec3 finalPosition;
        float finalYawRadians = 0.0f;
        float finalSpeed = 0.0f;
        bool hitBounds = false;
        std::string message;
    };
    struct RoadEdgeCheck {
        std::string backendName;
        bool passed = false;
        bool collisionBacked = false;
        bool roadEdgeClear = false;
        int edgeOverlapFrames = 0;
        int frameCount = 0;
        float minCollisionClearance = 0.0f;
        float maxCameraYawDeltaDegrees = 0.0f;
        float finalCameraYawRadians = 0.0f;
        std::vector<std::string> authoredEdgeIds;
        engine::Vec3 finalPosition;
        float finalYawRadians = 0.0f;
        bool hitBounds = false;
        std::string message;
    };
    std::vector<ControlCheck> controlChecks;
    std::vector<RouteCheck> routeChecks;
    std::vector<ObstacleCheck> obstacleChecks;
    std::vector<DrivingFeelCheck> drivingFeelChecks;
    std::vector<RoutePaceProbe> routePaceProbes;
    std::vector<RoadEdgeCheck> roadEdgeChecks;
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
