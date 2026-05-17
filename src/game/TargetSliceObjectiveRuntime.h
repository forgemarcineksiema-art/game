#pragma once

#include "game/SceneDefinition.h"

#include <string>
#include <string_view>

struct TargetSliceObjectiveState {
    bool active = false;
    bool complete = false;
    std::string id;
    std::string objectiveText;
    std::string debugObjectiveText;
    std::string completionInteractableName;
    std::string completionSummary;
    std::string completionEventText;
};

TargetSliceObjectiveState BuildTargetSliceObjectiveState(
    const SceneDefinition& scene,
    std::string_view completedInteractableName = {});
