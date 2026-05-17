#pragma once

#include "game/SceneDefinition.h"

enum class SceneGuidancePolicy {
    FerryOfficeActive,
    AllAuthored,
};

struct SceneRuntimePolicy {
    bool usesFerryOfficeBehavior = true;
    bool usesNeutralPresentation = false;
    bool allowsFallbackVehicle = true;
    bool drawsWorldStateDebug = true;
    SceneGuidancePolicy guidancePolicy = SceneGuidancePolicy::FerryOfficeActive;
};

SceneRuntimePolicy BuildSceneRuntimePolicy(bool sceneDefinitionLoaded, const SceneDefinition& scene);
