#pragma once

#include "engine/math/Math.h"

#include <filesystem>
#include <string>

struct TargetSliceObjectiveQaResult {
    bool passed = false;
    std::string scenario = "veyra-target-objective-acquisition";
    std::string sceneId;
    std::filesystem::path scenePath;
    std::filesystem::path reportPath;
    std::string inputScriptName = "recorded-veyra-target-objective-v1";
    bool focusAcquired = false;
    bool interactionTriggered = false;
    bool objectiveComplete = false;
    int framesToFocus = -1;
    int framesToInteract = -1;
    std::string focusName;
    std::string focusPrompt;
    float focusDistance = 0.0f;
    std::string interactionMessage;
    std::string objectiveId;
    std::string completionSummary;
    std::string completionEventText;
    engine::Vec3 finalPlayerPosition;
    float finalPlayerYawRadians = 0.0f;
    std::string error;
};

std::filesystem::path DefaultTargetSliceObjectiveQaReportPath();

TargetSliceObjectiveQaResult RunTargetSliceObjectiveAcquisitionQa(
    const std::filesystem::path& scenePath,
    const std::filesystem::path& reportPath = {});
