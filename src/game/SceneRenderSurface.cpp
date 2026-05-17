#include "game/SceneRenderSurface.h"

SceneRenderSubmissionPlan BuildSceneRenderSubmissionPlan(
    bool sceneDefinitionLoaded,
    const SceneRuntimePolicy& runtimePolicy)
{
    SceneRenderSubmissionPlan plan;
    plan.drawsFallbackFerryOfficeMoodBase = !sceneDefinitionLoaded;
    plan.drawsSceneAuthoredVisualPlaceholders = sceneDefinitionLoaded;
    plan.drawsSceneAuthoredMeshes = sceneDefinitionLoaded;
    plan.drawsDebugColliders = true;
    plan.drawsFerryOfficeWorldStateCues = runtimePolicy.drawsWorldStateDebug;
    return plan;
}
