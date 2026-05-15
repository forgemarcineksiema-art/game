#pragma once

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
    std::string error;
};

std::filesystem::path DefaultFerryOfficePlaythroughQaReportPath();

FerryOfficePlaythroughQaResult RunFerryOfficeServiceCallPlaythroughQa(
    const std::filesystem::path& scenePath,
    const std::filesystem::path& reportPath = {});
