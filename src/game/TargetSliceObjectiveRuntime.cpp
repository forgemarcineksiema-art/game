#include "game/TargetSliceObjectiveRuntime.h"

#include <sstream>

TargetSliceObjectiveState BuildTargetSliceObjectiveState(
    const SceneDefinition& scene,
    std::string_view completedInteractableName)
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

    std::ostringstream summary;
    summary << "complete=" << (state.complete ? "true" : "false")
            << " role=target-slice-authored-objective"
            << " targetObjective=" << state.id;
    if (!scene.targetObjective.completionSummary.empty()) {
        summary << " " << scene.targetObjective.completionSummary;
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
