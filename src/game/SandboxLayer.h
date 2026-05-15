#pragma once

#include "engine/application/Application.h"
#include "engine/assets/StaticMesh.h"
#include "engine/physics/PhysicsWorld.h"
#include "game/PlayerController.h"
#include "game/PrototypeScene.h"
#include "game/SceneDefinition.h"
#include "game/ThirdPersonCamera.h"
#include "game/VehicleController.h"

#include <filesystem>
#include <memory>
#include <string>

class SandboxLayer final : public engine::IGameLayer {
public:
    explicit SandboxLayer(std::filesystem::path scenePath = "data/scenes/ferry_office.scene.json");

    void onAttach() override;
    void onUpdate(double deltaSeconds, const engine::InputState& input) override;
    void onRender(engine::IRenderer& renderer) override;
    std::string debugText() const override;
    void onDetach() override;

private:
    void updateDebugText();
    void drawInteractionDebug(engine::IRenderer& renderer);
    void drawTraversalDebug(engine::IRenderer& renderer);
    void drawVehicleDebug(engine::IRenderer& renderer);
    void drawStaticMeshDebug(engine::IRenderer& renderer);
    void drawWorldStateDebug(engine::IRenderer& renderer);
    void drawSliceDebug(engine::IRenderer& renderer);
    void drawSceneVisualPlaceholders(engine::IRenderer& renderer);
    void recordWorldStateChange(bool changed);
    void loadSceneDefinition();
    void configureRuntimeFromScene();
    void setupVehiclePhysicsWorld();
    bool isVehicleExitPositionClear(engine::Vec3 position) const;
    void applyCameraSettingsForMode(bool vehicleMode);
    void loadStaticMeshAssets();

    PrototypeScene m_scene;
    SceneDefinition m_sceneDefinition;
    std::filesystem::path m_scenePath;
    PlayerController m_player;
    ThirdPersonCamera m_camera;
    VehicleController m_vehicle;
    engine::StaticMeshAsset m_unitBoxMesh;
    engine::Vec3 m_vehicleProxyHalfExtents {0.58f, 0.34f, 0.92f};
    engine::Vec3 m_vehicleCabinHalfExtents {0.42f, 0.20f, 0.46f};
    ThirdPersonCameraSettings m_onFootCameraSettings;
    ThirdPersonCameraSettings m_vehicleCameraSettings;
    std::unique_ptr<engine::physics::IPhysicsWorld> m_vehiclePhysicsWorld;
    std::string m_debugText;
    std::string m_lastInteractionText = "none";
    std::string m_lastWorldEventText = "none";
    std::string m_lastVehicleText = "none";
    std::string m_vehiclePhysicsBackendText = "none";
    bool m_interactPressedThisFrame = false;
    bool m_traversalPressedThisFrame = false;
    bool m_worldStateChangedThisFrame = false;
    bool m_cameraInVehicleMode = false;
    bool m_unitBoxMeshLoaded = false;
    bool m_sceneDefinitionLoaded = false;
    unsigned long long m_frameIndex = 0;
};
