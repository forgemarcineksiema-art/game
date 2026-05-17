#pragma once

#include "engine/application/Application.h"
#include "engine/assets/StaticMesh.h"
#include "engine/physics/VehicleRuntime.h"
#include "engine/physics/PhysicsWorld.h"
#include "game/PlayerController.h"
#include "game/PrototypeScene.h"
#include "game/SceneDefinition.h"
#include "game/SceneRuntimePolicy.h"
#include "game/ThirdPersonCamera.h"
#include "game/VehicleController.h"

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

class SandboxLayer final : public engine::IGameLayer {
public:
    explicit SandboxLayer(
        std::filesystem::path scenePath = "data/scenes/ferry_office.scene.json",
        engine::UiMode uiMode = engine::UiMode::Debug,
        engine::physics::PhysicsBackend vehicleRuntimeBackend = engine::physics::PhysicsBackend::Simple,
        bool vehicleRuntimeAdapterEnabled = false,
        std::string qaCaptureState = {});

    void onAttach() override;
    void onUpdate(double deltaSeconds, const engine::InputState& input) override;
    void onRender(engine::IRenderer& renderer) override;
    std::string debugText() const override;
    void onDetach() override;

private:
    void updateDebugText();
    std::string buildFullDebugText() const;
    std::string buildNeutralSceneDebugText() const;
    std::string buildPresentationText(bool minimal) const;
    std::string buildNeutralScenePresentationText(bool minimal) const;
    void toggleDebugUiMode();
    void drawInteractionDebug(engine::IRenderer& renderer);
    void drawTraversalDebug(engine::IRenderer& renderer);
    void drawVehicleDebug(engine::IRenderer& renderer);
    void drawStaticMeshDebug(engine::IRenderer& renderer);
    void drawWorldStateDebug(engine::IRenderer& renderer);
    void drawSliceDebug(engine::IRenderer& renderer);
    void drawPlaytestGuidance(engine::IRenderer& renderer);
    void drawSceneVisualPlaceholders(engine::IRenderer& renderer);
    void drawPlayerPresentation(engine::IRenderer& renderer);
    void drawPlayerDebug(engine::IRenderer& renderer);
    void recordWorldStateChange(bool changed);
    void loadSceneDefinition();
    void configureRuntimeFromScene();
    void applyQaCaptureState();
    void setupVehiclePhysicsWorld();
    void setupVehicleRuntimeAdapter();
    void updateVehicleDriving(float deltaSeconds, const engine::InputState& input);
    bool isVehicleExitPositionClear(engine::Vec3 position) const;
    void applyCameraSettingsForMode(bool vehicleMode);
    void loadStaticMeshAssets();
    bool shouldDrawFullGuidance() const;
    bool shouldDrawRouteMarker(std::string_view routeId) const;
    bool shouldDrawObjectiveMarker(std::string_view markerId) const;
    bool shouldDrawInteractableMarker(const Interactable& interactable) const;
    bool shouldDrawTraversalMarker(const TraversalAffordance& affordance) const;
    bool shouldDrawVehicleGuidance() const;

    PrototypeScene m_scene;
    SceneDefinition m_sceneDefinition;
    SceneRuntimePolicy m_runtimePolicy;
    std::filesystem::path m_scenePath;
    PlayerController m_player;
    ThirdPersonCamera m_camera;
    VehicleController m_vehicle;
    std::unordered_map<std::string, engine::StaticMeshAsset> m_staticMeshAssets;
    engine::Vec3 m_vehicleProxyHalfExtents {0.58f, 0.34f, 0.92f};
    engine::Vec3 m_vehicleCabinHalfExtents {0.42f, 0.20f, 0.46f};
    ThirdPersonCameraSettings m_onFootCameraSettings;
    ThirdPersonCameraSettings m_vehicleCameraSettings;
    std::unique_ptr<engine::physics::IPhysicsWorld> m_vehiclePhysicsWorld;
    std::unique_ptr<engine::physics::IVehicleRuntimeAdapter> m_vehicleRuntimeAdapter;
    std::string m_debugText;
    std::string m_lastInteractionText = "none";
    std::string m_lastWorldEventText = "none";
    std::string m_lastVehicleText = "none";
    std::string m_vehiclePhysicsBackendText = "none";
    std::string m_vehicleRuntimeText = "deterministic";
    bool m_interactPressedThisFrame = false;
    bool m_traversalPressedThisFrame = false;
    bool m_worldStateChangedThisFrame = false;
    bool m_cameraInVehicleMode = false;
    bool m_sceneDefinitionLoaded = false;
    bool m_vehicleAvailable = true;
    bool m_vehicleRuntimeAdapterEnabled = false;
    std::string m_qaCaptureState;
    engine::physics::PhysicsBackend m_vehicleRuntimeBackend = engine::physics::PhysicsBackend::Simple;
    engine::UiMode m_uiMode = engine::UiMode::Debug;
    engine::UiMode m_nonDebugUiMode = engine::UiMode::Playtest;
    unsigned long long m_frameIndex = 0;
};
