#include "game/SceneRuntimePackage.h"

SceneRuntimePackage BuildSceneRuntimePackage(bool sceneDefinitionLoaded, const SceneDefinition& scene)
{
    SceneRuntimePackage package;
    package.policy = BuildSceneRuntimePolicy(sceneDefinitionLoaded, scene);
    package.renderSubmission = BuildSceneRenderSubmissionPlan(sceneDefinitionLoaded, package.policy);
    return package;
}
