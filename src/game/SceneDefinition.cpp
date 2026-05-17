#include "game/SceneDefinition.h"

const SceneVehicleDefinition* FindSceneVehicleById(const SceneDefinition& scene, const std::string& id)
{
    for (const SceneVehicleDefinition& vehicle : scene.vehicles) {
        if (vehicle.id == id) {
            return &vehicle;
        }
    }

    return nullptr;
}

const SceneMeshAssetDefinition* FindSceneMeshAssetById(const SceneDefinition& scene, const std::string& id)
{
    for (const SceneMeshAssetDefinition& asset : scene.meshAssets) {
        if (asset.id == id) {
            return &asset;
        }
    }

    return nullptr;
}

bool IsFerryOfficeRegressionScene(const SceneDefinition& scene)
{
    return scene.id == "ferry-office" || scene.sliceMetadata.kind == "regression-testbed";
}

bool IsTargetSliceScaffoldScene(const SceneDefinition& scene)
{
    return scene.sliceMetadata.kind == "target-slice-scaffold";
}
