#pragma once

#include "game/SceneDefinition.h"
#include "game/SceneRenderSurface.h"
#include "game/SceneRuntimePolicy.h"

struct SceneRuntimePackage {
    SceneRuntimePolicy policy;
    SceneRenderSubmissionPlan renderSubmission;
};

SceneRuntimePackage BuildSceneRuntimePackage(bool sceneDefinitionLoaded, const SceneDefinition& scene);
