#include "game/TargetSliceObjectiveRuntime.h"

#include <sstream>

TargetSliceObjectiveState BuildTargetSliceObjectiveState(
    const SceneDefinition& scene,
    std::string_view completedInteractableName,
    std::string_view completedRiskyInteractableName,
    std::string_view completedExitInteractableName)
{
    TargetSliceObjectiveState state;
    if (!IsTargetSliceScaffoldScene(scene) || scene.targetObjective.id.empty()) {
        return state;
    }

    state.active = true;
    state.id = scene.targetObjective.id;
    state.objectiveText = scene.targetObjective.objectiveText;
    state.debugObjectiveText = scene.targetObjective.debugObjectiveText;
    state.completionInteractableName = scene.targetObjective.completionInteractableName;
    state.complete = !state.completionInteractableName.empty()
        && state.completionInteractableName == completedInteractableName;

    if (!scene.targetActionResponse.id.empty()) {
        state.actionResponseActive = true;
        state.actionResponseId = scene.targetActionResponse.id;
        state.riskyInteractableName = scene.targetActionResponse.riskyInteractableName;
        state.responseStateId = scene.targetActionResponse.responseStateId;
        state.responseSummary = scene.targetActionResponse.responseSummary;
        state.responseEventText = scene.targetActionResponse.responseEventText;
        state.exitInteractableName = scene.targetActionResponse.exitInteractableName;
        state.exitRecoveryStateId = scene.targetActionResponse.exitRecoveryStateId;
        state.exitSummary = scene.targetActionResponse.exitSummary;
        state.exitEventText = scene.targetActionResponse.exitEventText;
        state.riskyActionComplete = !state.riskyInteractableName.empty()
            && state.riskyInteractableName == completedRiskyInteractableName;
        state.localResponseActive = state.riskyActionComplete;
        state.exitRecovered = state.riskyActionComplete
            && !state.exitInteractableName.empty()
            && state.exitInteractableName == completedExitInteractableName;
    }

    std::ostringstream summary;
    summary << "complete=" << (state.complete ? "true" : "false")
            << " role=target-slice-authored-objective"
            << " targetObjective=" << state.id;
    if (!scene.targetObjective.completionSummary.empty()) {
        summary << " " << scene.targetObjective.completionSummary;
    }
    if (state.actionResponseActive) {
        summary << " riskyAction=" << state.actionResponseId
                << " complete=" << (state.riskyActionComplete ? "true" : "false")
                << " responseState=" << state.responseStateId
                << " active=" << (state.localResponseActive ? "true" : "false")
                << " exitRecovery=" << state.exitRecoveryStateId
                << " complete=" << (state.exitRecovered ? "true" : "false");
        if (!state.responseSummary.empty()) {
            summary << " " << state.responseSummary;
        }
        if (!state.exitSummary.empty()) {
            summary << " " << state.exitSummary;
        }
    }
    state.completionSummary = summary.str();

    if (state.complete) {
        state.completionEventText = scene.targetObjective.completionEventText.empty()
            ? "Target-slice objective complete: " + state.id
            : scene.targetObjective.completionEventText;
    } else {
        state.completionEventText = "none";
    }

    return state;
}
