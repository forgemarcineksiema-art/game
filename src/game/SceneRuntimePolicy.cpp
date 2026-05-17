#include "game/SceneRuntimePolicy.h"

SceneRuntimePolicy BuildSceneRuntimePolicy(bool sceneDefinitionLoaded, const SceneDefinition& scene)
{
    if (!sceneDefinitionLoaded || IsFerryOfficeRegressionScene(scene)) {
        return {};
    }

    SceneRuntimePolicy policy;
    policy.usesFerryOfficeBehavior = false;
    policy.usesNeutralPresentation = true;
    policy.allowsFallbackVehicle = false;
    policy.drawsWorldStateDebug = false;
    policy.guidancePolicy = SceneGuidancePolicy::AllAuthored;
    return policy;
}
