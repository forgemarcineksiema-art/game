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
    bool actionResponseActive = false;
    bool riskyActionComplete = false;
    bool localResponseActive = false;
    bool exitRecovered = false;
    std::string actionResponseId;
    std::string riskyInteractableName;
    std::string responseStateId;
    std::string responseSummary;
    std::string responseEventText;
    std::string exitInteractableName;
    std::string exitRecoveryStateId;
    std::string exitSummary;
    std::string exitEventText;
};

TargetSliceObjectiveState BuildTargetSliceObjectiveState(
    const SceneDefinition& scene,
    std::string_view completedInteractableName = {},
    std::string_view completedRiskyInteractableName = {},
    std::string_view completedExitInteractableName = {});
