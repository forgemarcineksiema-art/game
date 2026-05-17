#pragma once

#include "game/SceneRuntimePolicy.h"

struct SceneRenderSubmissionPlan {
    bool drawsFallbackFerryOfficeMoodBase = true;
    bool drawsSceneAuthoredVisualPlaceholders = false;
    bool drawsSceneAuthoredMeshes = false;
    bool drawsDebugColliders = true;
    bool drawsFerryOfficeWorldStateCues = true;
};

SceneRenderSubmissionPlan BuildSceneRenderSubmissionPlan(
    bool sceneDefinitionLoaded,
    const SceneRuntimePolicy& runtimePolicy);
