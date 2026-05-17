#include "SandboxLayer.h"

#include "engine/core/Logger.h"
#include "engine/math/Math.h"
#include "game/FerryOfficeData.h"
#include "game/ScenePresentation.h"
#include "game/SceneLoader.h"

#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

const engine::Vec3 ServiceYardVehicleSpawnPosition {6.2f, 0.0f, -2.2f};
const float ServiceYardVehicleSpawnYawRadians = engine::Radians(88.0f);
constexpr float VehicleRuntimeRoadEdgeObstacleCenterY = 0.58f;
constexpr float VehicleRuntimeRoadEdgeObstacleMinHalfY = 0.72f;
constexpr float VehicleRuntimeRoadEdgeObstacleMinHalfZ = 0.38f;
const float ServiceYardBoundsMinX = 3.35f;
const float ServiceYardBoundsMaxX = 19.45f;
const float ServiceYardBoundsMinZ = -5.05f;
const float ServiceYardBoundsMaxZ = 0.95f;
const engine::Vec3 ServiceYardPadCenter {6.30f, -0.045f, -2.20f};
const engine::Vec3 ServiceYardPadHalfExtents {3.05f, 0.045f, 2.95f};
const engine::Vec3 ServiceYardWestRailCenter {3.25f, 0.35f, -2.20f};
const engine::Vec3 ServiceYardRailHalfExtents {0.12f, 0.35f, 2.85f};
const engine::Vec3 ServiceYardEastEntryPostNorthCenter {9.35f, 0.35f, 0.15f};
const engine::Vec3 ServiceYardEastEntryPostSouthCenter {9.35f, 0.35f, -4.55f};
const engine::Vec3 ServiceYardEastEntryPostHalfExtents {0.12f, 0.35f, 0.55f};
const engine::Vec3 ServiceYardBackStopCenter {6.30f, 0.18f, -5.15f};
const engine::Vec3 ServiceYardBackStopHalfExtents {2.55f, 0.18f, 0.12f};
const engine::Vec3 ServiceYardCrateCenter {8.15f, 0.45f, 0.45f};
const engine::Vec3 ServiceYardCrateHalfExtents {0.55f, 0.45f, 0.45f};
const engine::Vec3 DockRoadCenter {13.35f, -0.050f, -2.20f};
const engine::Vec3 DockRoadHalfExtents {4.25f, 0.050f, 1.30f};
const engine::Vec3 DockRoadTurnaroundCenter {17.80f, -0.045f, -2.20f};
const engine::Vec3 DockRoadTurnaroundHalfExtents {1.80f, 0.045f, 1.80f};
const engine::Vec3 DockRoadWaterEdgeCenter {13.70f, -0.070f, -4.15f};
const engine::Vec3 DockRoadWaterEdgeHalfExtents {5.70f, 0.050f, 0.45f};
const engine::Vec3 DockRoadSouthRailCenter {13.55f, 0.28f, -3.65f};
const engine::Vec3 DockRoadNorthCurbCenter {13.55f, 0.18f, -0.70f};
const engine::Vec3 DockRoadRailHalfExtents {4.45f, 0.28f, 0.10f};
const engine::Vec3 DockRoadEndMarkerCenter {19.35f, 0.38f, -2.20f};
const engine::Vec3 VehicleBodyHalfExtents {0.58f, 0.34f, 0.92f};
const engine::Vec3 VehicleCabinHalfExtents {0.42f, 0.20f, 0.46f};

engine::Color ScaleColor(engine::Color color, float scale)
{
    return {color.r * scale, color.g * scale, color.b * scale, color.a};
}

std::string_view PresentationJobStepText(FerryOfficeJobPhase phase)
{
    switch (phase) {
    case FerryOfficeJobPhase::CollectManifest:
        return "Collect manifest";
    case FerryOfficeJobPhase::UseServiceRoute:
        return "Use service route";
    case FerryOfficeJobPhase::RestorePower:
        return "Restore power";
    case FerryOfficeJobPhase::OpenServiceGate:
        return "Open service gate";
    case FerryOfficeJobPhase::UseServiceVehicle:
        return "Enter service vehicle";
    case FerryOfficeJobPhase::ReachDockRoad:
        return "Reach dock-road checkpoint";
    case FerryOfficeJobPhase::ConfirmServiceRun:
        return "Confirm service run";
    case FerryOfficeJobPhase::Complete:
        return "Complete";
    default:
        return "Unknown";
    }
}

std::filesystem::path ResolveProjectPath(const std::filesystem::path& path)
{
    if (path.empty() || path.is_absolute() || std::filesystem::exists(path)) {
        return path;
    }

#ifdef ENGINE_SOURCE_ROOT
    const std::filesystem::path sourcePath = std::filesystem::path(ENGINE_SOURCE_ROOT) / path;
    if (std::filesystem::exists(sourcePath)) {
        return sourcePath;
    }
#endif

    return path;
}

std::vector<engine::physics::VehicleRuntimeStaticObstacle> BuildAuthoredRoadEdgeRuntimeObstacles(
    const SceneDefinition& scene)
{
    std::vector<engine::physics::VehicleRuntimeStaticObstacle> obstacles;
    for (const SceneVisualPlaceholderDefinition& placeholder : scene.visualPlaceholders) {
        if (placeholder.id != "dock-road-south-rail" && placeholder.id != "dock-road-north-curb") {
            continue;
        }
        engine::Vec3 center = placeholder.center;
        engine::Vec3 halfExtents = placeholder.halfExtents;
        center.y = VehicleRuntimeRoadEdgeObstacleCenterY;
        halfExtents.y = std::max(halfExtents.y, VehicleRuntimeRoadEdgeObstacleMinHalfY);
        halfExtents.z = std::max(halfExtents.z, VehicleRuntimeRoadEdgeObstacleMinHalfZ);
        obstacles.push_back({placeholder.id, center, halfExtents});
    }
    return obstacles;
}

bool NameContains(const StaticCollider& collider, const char* token)
{
    return collider.name.find(token) != std::string::npos;
}

engine::Vec3 RotateYawOffset(engine::Vec3 offset, float yawRadians)
{
    const float c = std::cos(yawRadians);
    const float s = std::sin(yawRadians);
    return {
        offset.x * c + offset.z * s,
        offset.y,
        offset.z * c - offset.x * s,
    };
}

engine::Color ColliderSolidColor(const StaticCollider& collider, bool routeOpened)
{
    if (collider.name == FerryOffice::Names::ServiceGateCollider) {
        return routeOpened
            ? engine::Color {0.12f, 0.30f, 0.18f, 1.0f}
            : engine::Color {0.36f, 0.11f, 0.08f, 1.0f};
    }
    if (NameContains(collider, "ferry-office")) {
        return {0.34f, 0.30f, 0.21f, 1.0f};
    }
    if (NameContains(collider, "dock-rail")) {
        return {0.48f, 0.38f, 0.14f, 1.0f};
    }
    if (NameContains(collider, "service-barrier")) {
        return {0.46f, 0.25f, 0.12f, 1.0f};
    }
    if (NameContains(collider, "office-counter")) {
        return {0.25f, 0.19f, 0.12f, 1.0f};
    }
    if (NameContains(collider, "maintenance")) {
        return {0.13f, 0.32f, 0.29f, 1.0f};
    }

    return collider.blocksPlayer
        ? engine::Color {0.30f, 0.27f, 0.20f, 1.0f}
        : engine::Color {0.13f, 0.27f, 0.17f, 1.0f};
}

engine::Color ColliderWireColor(const StaticCollider& collider, bool routeOpened)
{
    if (collider.name == FerryOffice::Names::ServiceGateCollider) {
        return routeOpened
            ? engine::Color {0.25f, 1.0f, 0.35f, 1.0f}
            : engine::Color {1.0f, 0.25f, 0.2f, 1.0f};
    }

    return collider.blocksPlayer
        ? engine::Color {0.9f, 0.72f, 0.28f, 1.0f}
        : engine::Color {0.25f, 1.0f, 0.35f, 1.0f};
}

void DrawFerryOfficeMoodBase(engine::IRenderer& renderer, float floor)
{
    renderer.drawDebugSolidBox({0.0f, floor - 0.055f, -1.25f}, {4.35f, 0.055f, 2.05f}, {0.33f, 0.30f, 0.22f, 1.0f});
    renderer.drawDebugSolidBox({0.0f, floor - 0.06f, 3.25f}, {3.15f, 0.06f, 2.35f}, {0.20f, 0.24f, 0.20f, 1.0f});
    renderer.drawDebugSolidBox({2.85f, floor - 0.05f, 1.35f}, {1.45f, 0.05f, 2.15f}, {0.18f, 0.23f, 0.22f, 1.0f});
    renderer.drawDebugSolidBox({-4.85f, floor - 0.07f, -0.7f}, {0.35f, 0.07f, 3.25f}, {0.07f, 0.18f, 0.24f, 1.0f});
    renderer.drawDebugSolidBox({4.85f, floor - 0.07f, -0.7f}, {0.35f, 0.07f, 3.25f}, {0.07f, 0.18f, 0.24f, 1.0f});
    renderer.drawDebugSolidBox({0.0f, floor + 1.68f, 5.18f}, {2.95f, 0.12f, 0.55f}, {0.40f, 0.22f, 0.13f, 1.0f});
    renderer.drawDebugSolidBox({-0.55f, floor + 0.32f, -1.35f}, {0.07f, 0.32f, 0.07f}, {0.43f, 0.34f, 0.16f, 1.0f});
    renderer.drawDebugSolidBox({0.55f, floor + 0.32f, -1.35f}, {0.07f, 0.32f, 0.07f}, {0.43f, 0.34f, 0.16f, 1.0f});
    renderer.drawDebugSolidBox({2.85f, floor + 0.65f, 1.85f}, {0.10f, 0.65f, 0.10f}, {0.10f, 0.42f, 0.38f, 1.0f});
}

void DrawVehicleServiceYardBase(engine::IRenderer& renderer, float floor)
{
    const engine::Vec3 padCenter {ServiceYardPadCenter.x, floor + ServiceYardPadCenter.y, ServiceYardPadCenter.z};
    const engine::Vec3 padOutlineCenter {ServiceYardPadCenter.x, floor + 0.02f, ServiceYardPadCenter.z};
    renderer.drawDebugSolidBox(padCenter, ServiceYardPadHalfExtents, {0.16f, 0.18f, 0.17f, 1.0f});
    renderer.drawDebugBox(padOutlineCenter, {ServiceYardPadHalfExtents.x, 0.02f, ServiceYardPadHalfExtents.z}, {0.50f, 0.72f, 0.62f, 1.0f});
    renderer.drawDebugLine(
        {ServiceYardPadCenter.x - ServiceYardPadHalfExtents.x + 0.65f, floor + 0.04f, ServiceYardPadCenter.z},
        {ServiceYardPadCenter.x + ServiceYardPadHalfExtents.x - 0.65f, floor + 0.04f, ServiceYardPadCenter.z},
        {0.85f, 0.78f, 0.45f, 1.0f});
    renderer.drawDebugLine(
        {ServiceYardPadCenter.x, floor + 0.04f, ServiceYardPadCenter.z - ServiceYardPadHalfExtents.z + 0.65f},
        {ServiceYardPadCenter.x, floor + 0.04f, ServiceYardPadCenter.z + ServiceYardPadHalfExtents.z - 0.65f},
        {0.85f, 0.78f, 0.45f, 1.0f});
    renderer.drawDebugSolidBox(
        {ServiceYardWestRailCenter.x, floor + ServiceYardWestRailCenter.y, ServiceYardWestRailCenter.z},
        ServiceYardRailHalfExtents,
        {0.42f, 0.30f, 0.13f, 1.0f});
    renderer.drawDebugSolidBox(
        {ServiceYardEastEntryPostNorthCenter.x, floor + ServiceYardEastEntryPostNorthCenter.y, ServiceYardEastEntryPostNorthCenter.z},
        ServiceYardEastEntryPostHalfExtents,
        {0.42f, 0.30f, 0.13f, 1.0f});
    renderer.drawDebugSolidBox(
        {ServiceYardEastEntryPostSouthCenter.x, floor + ServiceYardEastEntryPostSouthCenter.y, ServiceYardEastEntryPostSouthCenter.z},
        ServiceYardEastEntryPostHalfExtents,
        {0.42f, 0.30f, 0.13f, 1.0f});
    renderer.drawDebugSolidBox(
        {ServiceYardBackStopCenter.x, floor + ServiceYardBackStopCenter.y, ServiceYardBackStopCenter.z},
        ServiceYardBackStopHalfExtents,
        {0.42f, 0.30f, 0.13f, 1.0f});
    renderer.drawDebugSolidBox(
        {ServiceYardCrateCenter.x, floor + ServiceYardCrateCenter.y, ServiceYardCrateCenter.z},
        ServiceYardCrateHalfExtents,
        {0.27f, 0.30f, 0.22f, 1.0f});
}

void DrawDockRoadBase(engine::IRenderer& renderer, float floor)
{
    renderer.drawDebugSolidBox(
        {DockRoadCenter.x, floor + DockRoadCenter.y, DockRoadCenter.z},
        DockRoadHalfExtents,
        {0.13f, 0.16f, 0.15f, 1.0f});
    renderer.drawDebugBox(
        {DockRoadCenter.x, floor + 0.025f, DockRoadCenter.z},
        {DockRoadHalfExtents.x, 0.025f, DockRoadHalfExtents.z},
        {0.52f, 0.72f, 0.62f, 1.0f});
    renderer.drawDebugSolidBox(
        {DockRoadTurnaroundCenter.x, floor + DockRoadTurnaroundCenter.y, DockRoadTurnaroundCenter.z},
        DockRoadTurnaroundHalfExtents,
        {0.14f, 0.17f, 0.16f, 1.0f});
    renderer.drawDebugBox(
        {DockRoadTurnaroundCenter.x, floor + 0.025f, DockRoadTurnaroundCenter.z},
        {DockRoadTurnaroundHalfExtents.x, 0.025f, DockRoadTurnaroundHalfExtents.z},
        {0.84f, 0.76f, 0.42f, 1.0f});
    renderer.drawDebugSolidBox(
        {DockRoadWaterEdgeCenter.x, floor + DockRoadWaterEdgeCenter.y, DockRoadWaterEdgeCenter.z},
        DockRoadWaterEdgeHalfExtents,
        {0.05f, 0.13f, 0.22f, 1.0f});
    renderer.drawDebugSolidBox(
        {DockRoadSouthRailCenter.x, floor + DockRoadSouthRailCenter.y, DockRoadSouthRailCenter.z},
        DockRoadRailHalfExtents,
        {0.42f, 0.30f, 0.13f, 1.0f});
    renderer.drawDebugSolidBox(
        {DockRoadNorthCurbCenter.x, floor + DockRoadNorthCurbCenter.y, DockRoadNorthCurbCenter.z},
        {DockRoadRailHalfExtents.x, 0.18f, 0.08f},
        {0.31f, 0.32f, 0.26f, 1.0f});
    renderer.drawDebugLine(
        {ServiceYardPadCenter.x, floor + 0.06f, ServiceYardPadCenter.z},
        {DockRoadTurnaroundCenter.x, floor + 0.06f, DockRoadTurnaroundCenter.z},
        {0.88f, 0.80f, 0.36f, 1.0f});
    renderer.drawDebugLine(
        {DockRoadTurnaroundCenter.x - 0.75f, floor + 0.06f, DockRoadTurnaroundCenter.z - 0.75f},
        {DockRoadTurnaroundCenter.x + 0.75f, floor + 0.06f, DockRoadTurnaroundCenter.z + 0.75f},
        {0.88f, 0.80f, 0.36f, 1.0f});
    renderer.drawDebugLine(
        {DockRoadTurnaroundCenter.x - 0.75f, floor + 0.06f, DockRoadTurnaroundCenter.z + 0.75f},
        {DockRoadTurnaroundCenter.x + 0.75f, floor + 0.06f, DockRoadTurnaroundCenter.z - 0.75f},
        {0.88f, 0.80f, 0.36f, 1.0f});
    renderer.drawDebugSolidBox(
        {DockRoadEndMarkerCenter.x, floor + DockRoadEndMarkerCenter.y, DockRoadEndMarkerCenter.z},
        {0.25f, 0.38f, 0.25f},
        {0.78f, 0.65f, 0.22f, 1.0f});
    renderer.drawDebugBox(
        {DockRoadEndMarkerCenter.x, floor + DockRoadEndMarkerCenter.y, DockRoadEndMarkerCenter.z},
        {0.25f, 0.38f, 0.25f},
        {1.0f, 0.86f, 0.28f, 1.0f});
}

void DrawMeshInstance(
    engine::IRenderer& renderer,
    const engine::StaticMeshAsset& mesh,
    const engine::StaticMeshInstance& instance,
    SceneMaterial material)
{
    const std::vector<engine::Vec3> triangles = engine::BuildFlatTriangleList(mesh, instance);
    DrawSceneShadedTriangleList(renderer, triangles, material);
}

const engine::StaticMeshAsset& PlayerRaincoatMesh()
{
    static const engine::StaticMeshAsset mesh = [] {
        engine::StaticMeshAsset asset;
        asset.id = "runtime-player-raincoat-proxy";
        asset.sourcePath = "runtime/generated/player-raincoat-proxy";
        asset.vertices = {
            {{-0.28f, 0.54f, -0.18f}},
            {{0.28f, 0.54f, -0.18f}},
            {{0.28f, 0.54f, 0.18f}},
            {{-0.28f, 0.54f, 0.18f}},
            {{-0.19f, 1.28f, -0.14f}},
            {{0.19f, 1.28f, -0.14f}},
            {{0.19f, 1.28f, 0.14f}},
            {{-0.19f, 1.28f, 0.14f}},
        };
        asset.indices = {
            0, 1, 2, 0, 2, 3,
            4, 7, 6, 4, 6, 5,
            0, 4, 5, 0, 5, 1,
            1, 5, 6, 1, 6, 2,
            2, 6, 7, 2, 7, 3,
            3, 7, 4, 3, 4, 0,
        };
        asset.bounds = engine::ComputeBounds(asset.vertices);
        return asset;
    }();
    return mesh;
}

} // namespace

SandboxLayer::SandboxLayer(
    std::filesystem::path scenePath,
    engine::UiMode uiMode,
    engine::physics::PhysicsBackend vehicleRuntimeBackend,
    bool vehicleRuntimeAdapterEnabled,
    std::string qaCaptureState)
    : m_scenePath(std::move(scenePath))
    , m_uiMode(uiMode)
    , m_nonDebugUiMode(uiMode == engine::UiMode::Debug ? engine::UiMode::Playtest : uiMode)
    , m_vehicleRuntimeAdapterEnabled(vehicleRuntimeAdapterEnabled)
    , m_qaCaptureState(std::move(qaCaptureState))
    , m_vehicleRuntimeBackend(vehicleRuntimeBackend)
{
}

void SandboxLayer::onAttach()
{
    engine::Logger::info("Sandbox layer attached.");
    loadSceneDefinition();
    configureRuntimeFromScene();
    m_onFootCameraSettings = m_camera.settings();
    m_vehicleCameraSettings = m_onFootCameraSettings;
    m_vehicleCameraSettings.distance = 6.75f;
    m_vehicleCameraSettings.heightOffset = 1.95f;
    m_vehicleCameraSettings.smoothing = 9.0f;
    m_vehicleCameraSettings.targetYawFollowStrength = 5.0f;
    applyQaCaptureState();
    m_player.setWorld(&m_scene.world());
    setupVehiclePhysicsWorld();
    setupVehicleRuntimeAdapter();
    loadStaticMeshAssets();
    updateDebugText();
}

void SandboxLayer::loadSceneDefinition()
{
    const std::filesystem::path resolvedScenePath = ResolveProjectPath(m_scenePath);
    const SceneLoadResult loadedScene = LoadSceneDefinition(resolvedScenePath);
    if (!loadedScene.ok()) {
        m_sceneDefinitionLoaded = false;
        engine::Logger::warning("Runtime scene load failed; using built-in Ferry Office fallback. " + loadedScene.error);
        return;
    }

    m_sceneDefinition = loadedScene.scene;
    m_sceneDefinitionLoaded = true;
    m_scene.loadFromDefinition(m_sceneDefinition);
    engine::Logger::info("Loaded runtime scene data: " + m_sceneDefinition.id + " from " + resolvedScenePath.string());
}

void SandboxLayer::configureRuntimeFromScene()
{
    if (m_sceneDefinitionLoaded) {
        m_player.setPosition(m_sceneDefinition.playerStart.position);
        m_player.setFacingYawRadians(m_sceneDefinition.playerStart.yawRadians);
        m_camera.setYawRadians(m_sceneDefinition.playerStart.yawRadians);
    }

    VehicleControllerSettings vehicleSettings;
    const SceneVehicleDefinition* vehicle = m_sceneDefinitionLoaded && !m_sceneDefinition.vehicles.empty()
        ? &m_sceneDefinition.vehicles.front()
        : nullptr;

    if (vehicle) {
        m_vehicleAvailable = true;
        vehicleSettings.enterRadius = vehicle->enterRadius;
        vehicleSettings.boundsMinX = vehicle->boundsMin.x;
        vehicleSettings.boundsMaxX = vehicle->boundsMax.x;
        vehicleSettings.boundsMinZ = vehicle->boundsMin.y;
        vehicleSettings.boundsMaxZ = vehicle->boundsMax.y;
        m_vehicleProxyHalfExtents = vehicle->proxyHalfExtents;
        m_vehicle.setSettings(vehicleSettings);
        m_vehicle.setPosition(vehicle->spawnPosition);
        m_vehicle.setYawRadians(vehicle->spawnYawRadians);
        return;
    }

    if (m_sceneDefinitionLoaded && IsTargetSliceScaffoldScene(m_sceneDefinition)) {
        m_vehicleAvailable = false;
        m_vehicleRuntimeText = "none";
        m_vehiclePhysicsBackendText = "none";
        return;
    }

    m_vehicleAvailable = true;
    vehicleSettings.boundsMinX = ServiceYardBoundsMinX;
    vehicleSettings.boundsMaxX = ServiceYardBoundsMaxX;
    vehicleSettings.boundsMinZ = ServiceYardBoundsMinZ;
    vehicleSettings.boundsMaxZ = ServiceYardBoundsMaxZ;
    m_vehicleProxyHalfExtents = VehicleBodyHalfExtents;
    m_vehicle.setSettings(vehicleSettings);
    m_vehicle.setPosition(ServiceYardVehicleSpawnPosition);
    m_vehicle.setYawRadians(ServiceYardVehicleSpawnYawRadians);
}

void SandboxLayer::applyQaCaptureState()
{
    if (m_qaCaptureState.empty()) {
        return;
    }
    if (!isFerryOfficeRuntimeScene()) {
        engine::Logger::warning("QA capture state ignored for non-Ferry runtime scene: " + m_qaCaptureState);
        return;
    }

    WorldState& state = m_scene.worldState();
    const auto setFlag = [&state](WorldFlag flag) {
        state.setFlag(flag, true, "QA capture state");
    };
    const auto setServiceRouteOpen = [&setFlag]() {
        setFlag(WorldFlag::FerryOfficeJobStarted);
        setFlag(WorldFlag::ManifestCollected);
        setFlag(WorldFlag::ServiceRouteUsed);
        setFlag(WorldFlag::MaintenanceBoxInspected);
        setFlag(WorldFlag::PowerRestored);
        setFlag(WorldFlag::RouteOpened);
    };
    const auto setServiceRunComplete = [&setFlag, &setServiceRouteOpen]() {
        setServiceRouteOpen();
        setFlag(WorldFlag::ServiceVehicleUsed);
        setFlag(WorldFlag::DockRoadReached);
        setFlag(WorldFlag::ServiceRunConfirmed);
        setFlag(WorldFlag::FerryOfficeJobComplete);
    };
    const auto setRelayReady = [&setFlag, &setServiceRunComplete]() {
        setServiceRunComplete();
        setFlag(WorldFlag::DockRoadRelayReset);
    };
    const auto setLowDockReady = [&setFlag, &setRelayReady]() {
        setRelayReady();
        setFlag(WorldFlag::DockRoadRelayLogged);
        setFlag(WorldFlag::DockRoadClearanceTagged);
        setFlag(WorldFlag::HarborPartsPickedUp);
        setFlag(WorldFlag::HarborPartsDelivered);
        setFlag(WorldFlag::FerryOfficeBoardUpdated);
        setFlag(WorldFlag::FerryOfficeHandoffFiled);
        setFlag(WorldFlag::StormPumpReset);
        setFlag(WorldFlag::StormPumpTicketClosed);
    };
    const auto applyVehicleCapturePose = [this](engine::Vec3 position, float yawRadians, float speed, float throttle, float brake, float steer) {
        m_vehicle.applyRuntimeState(position, yawRadians, speed, throttle, brake, steer, false);
        m_vehicle.setOccupiedForTesting(true);
        m_player.setPosition(position - m_vehicle.right() * 0.20f);
        m_player.setFacingYawRadians(yawRadians);
        applyCameraSettingsForMode(true);
        m_camera.setYawRadians(yawRadians);
    };

    if (m_qaCaptureState == "relay-to-service-log") {
        setRelayReady();
        m_player.setPosition({17.45f, 0.0f, 0.05f});
        m_player.setFacingYawRadians(0.0f);
        m_camera.setYawRadians(0.0f);
        engine::Logger::info("QA capture state applied: relay-to-service-log.");
    } else if (m_qaCaptureState == "low-dock-drain-access") {
        setLowDockReady();
        m_player.setPosition({18.75f, 0.0f, -2.05f});
        m_player.setFacingYawRadians(0.0f);
        m_camera.setYawRadians(0.0f);
        engine::Logger::info("QA capture state applied: low-dock-drain-access.");
    } else if (m_qaCaptureState == "office-front-oblique") {
        m_player.setPosition({1.15f, 0.0f, -1.35f});
        m_player.setFacingYawRadians(engine::Radians(28.0f));
        m_camera.setYawRadians(engine::Radians(38.0f));
        engine::Logger::info("QA capture state applied: office-front-oblique.");
    } else if (m_qaCaptureState == "service-yard-vehicle-side") {
        setServiceRouteOpen();
        m_player.setPosition({5.10f, 0.0f, -2.35f});
        m_player.setFacingYawRadians(engine::Radians(92.0f));
        m_camera.setYawRadians(engine::Radians(105.0f));
        engine::Logger::info("QA capture state applied: service-yard-vehicle-side.");
    } else if (m_qaCaptureState == "dock-road-wide") {
        setServiceRunComplete();
        m_player.setPosition({13.60f, 0.0f, -2.30f});
        m_player.setFacingYawRadians(engine::Radians(86.0f));
        m_camera.setYawRadians(engine::Radians(125.0f));
        engine::Logger::info("QA capture state applied: dock-road-wide.");
    } else if (m_qaCaptureState == "vehicle-dock-road-forward") {
        setServiceRouteOpen();
        setFlag(WorldFlag::ServiceVehicleUsed);
        applyVehicleCapturePose({13.20f, 0.0f, -2.30f}, engine::Radians(88.0f), 2.25f, 0.65f, 0.0f, 0.18f);
        engine::Logger::info("QA capture state applied: vehicle-dock-road-forward.");
    } else if (m_qaCaptureState == "vehicle-dock-road-reverse") {
        setServiceRouteOpen();
        setFlag(WorldFlag::ServiceVehicleUsed);
        applyVehicleCapturePose({17.45f, 0.0f, -2.20f}, engine::Radians(-92.0f), -0.85f, -0.45f, 0.0f, -0.40f);
        engine::Logger::info("QA capture state applied: vehicle-dock-road-reverse.");
    }
    m_scene.syncWorldStateColliders();
}

void SandboxLayer::onUpdate(double deltaSeconds, const engine::InputState& input)
{
    ++m_frameIndex;
    m_worldStateChangedThisFrame = false;
    if (input.debugOverlayTogglePressed) {
        toggleDebugUiMode();
    }
    const float dt = static_cast<float>(deltaSeconds);
    m_interactPressedThisFrame = input.interactPressed;
    m_traversalPressedThisFrame = false;
    m_vehicle.beginFrame();

    if (m_vehicleAvailable && m_vehicle.state().occupied) {
        const engine::Vec3 exitPosition = m_vehicle.exitPosition();
        const bool exitClear = isVehicleExitPositionClear(exitPosition);
        if (m_vehicle.tryExit(input, exitClear)) {
            m_player.setPosition(exitPosition);
            m_lastVehicleText = "Exited service yard vehicle.";
            engine::Logger::info("Vehicle: " + m_lastVehicleText);
        }
    }

    if (m_vehicleAvailable && m_vehicle.state().occupied) {
        m_scene.traversal().updateFocus({999.0f, 0.0f, 999.0f}, {0.0f, 0.0f, 1.0f});
        m_scene.interactions().updateFocus({999.0f, 0.0f, 999.0f}, {0.0f, 0.0f, 1.0f});
        updateVehicleDriving(dt, input);
    } else {
        const engine::Vec3 traversalFacing = engine::ForwardFromYaw(m_player.state().facingYawRadians);
        m_scene.traversal().updateFocus(m_player.state().position, traversalFacing);
        const TraversalActivation traversalActivation = m_scene.traversal().activationFromInput(input);
        m_traversalPressedThisFrame = input.jumpPressed && m_scene.traversal().focus().hasFocus;

        m_player.update(dt, input, m_camera.state().yawRadians, &traversalActivation);
        if (m_player.state().traversalLandedThisFrame) {
            recordWorldStateChange(m_scene.recordTraversalCompleted(m_player.state().landedTraversalId));
        }

        const engine::Vec3 playerFacing = engine::ForwardFromYaw(m_player.state().facingYawRadians);
        m_scene.interactions().updateFocus(m_player.state().position, playerFacing);
        if (m_vehicleAvailable) {
            m_vehicle.updateFocus(m_player.state().position, playerFacing);
        }
        const InteractionFocus& focus = m_scene.interactions().focus();
        const bool vehicleEntered = m_vehicleAvailable
            && m_player.state().traversalMode == PlayerTraversalMode::Normal
            && !focus.hasFocus
            && m_vehicle.tryEnter(input);

        if (vehicleEntered) {
            m_lastVehicleText = "Entered service yard vehicle.";
            engine::Logger::info("Vehicle: " + m_lastVehicleText);
            recordWorldStateChange(m_scene.recordServiceVehicleUsed());
            updateVehicleDriving(dt, input);
        } else {
            const InteractionResult interaction = m_scene.interactions().interact(input);
            if (interaction.triggered) {
                m_lastInteractionText = interaction.message;
                engine::Logger::info("Interaction: " + interaction.message);
                recordWorldStateChange(m_scene.applyInteractionResult(interaction));
            }
        }
    }

    if (m_vehicleAvailable && m_vehicle.state().occupied) {
        recordWorldStateChange(m_scene.updateJobVehicleCheckpoint(m_vehicle.state().position, true));
    }

    if (m_vehicleAvailable && m_vehiclePhysicsWorld) {
        m_vehiclePhysicsWorld->step(dt);
    }

    const bool vehicleMode = m_vehicleAvailable && m_vehicle.state().occupied;
    applyCameraSettingsForMode(vehicleMode);
    CameraTarget target;
    if (vehicleMode) {
        target = m_vehicle.cameraTarget();
    } else {
        target.position = m_player.state().position;
        target.yawRadians = m_player.state().facingYawRadians;
    }
    m_camera.update(dt, input, target);

    updateDebugText();
    if (m_frameIndex == 1 || m_frameIndex % 120 == 0) {
        engine::Logger::info(m_debugText);
    }
}

void SandboxLayer::onRender(engine::IRenderer& renderer)
{
    const bool fullDebug = m_uiMode == engine::UiMode::Debug;
    renderer.setDebugCamera(m_camera.debugCamera());
    if (fullDebug) {
        renderer.drawDebugGridAndAxes();
    }
    if (m_sceneDefinitionLoaded) {
        drawSceneVisualPlaceholders(renderer);
    } else {
        DrawFerryOfficeMoodBase(renderer, m_scene.world().floorHeight());
        DrawVehicleServiceYardBase(renderer, m_scene.world().floorHeight());
        DrawDockRoadBase(renderer, m_scene.world().floorHeight());
    }
    drawStaticMeshDebug(renderer);
    if (fullDebug) {
        renderer.drawDebugLine({-12.0f, m_scene.world().floorHeight(), -12.0f}, {12.0f, m_scene.world().floorHeight(), -12.0f}, {0.35f, 0.9f, 0.55f, 1.0f});
        renderer.drawDebugLine({12.0f, m_scene.world().floorHeight(), -12.0f}, {12.0f, m_scene.world().floorHeight(), 12.0f}, {0.35f, 0.9f, 0.55f, 1.0f});
        renderer.drawDebugLine({12.0f, m_scene.world().floorHeight(), 12.0f}, {-12.0f, m_scene.world().floorHeight(), 12.0f}, {0.35f, 0.9f, 0.55f, 1.0f});
        renderer.drawDebugLine({-12.0f, m_scene.world().floorHeight(), 12.0f}, {-12.0f, m_scene.world().floorHeight(), -12.0f}, {0.35f, 0.9f, 0.55f, 1.0f});
        const bool routeOpened = m_scene.worldState().isFlagSet(WorldFlag::RouteOpened);
        for (const StaticCollider& collider : m_scene.world().colliders()) {
            renderer.drawDebugSolidBox(collider.bounds.center, collider.bounds.halfExtents, ColliderSolidColor(collider, routeOpened));
            renderer.drawDebugBox(collider.bounds.center, collider.bounds.halfExtents, ColliderWireColor(collider, routeOpened));
        }
    }
    if (!m_vehicleAvailable || !m_vehicle.state().occupied) {
        drawPlayerPresentation(renderer);
    }
    if (fullDebug) {
        if (isFerryOfficeRuntimeScene()) {
            drawWorldStateDebug(renderer);
        }
        drawSliceDebug(renderer);
        drawPlayerDebug(renderer);
        drawTraversalDebug(renderer);
        drawInteractionDebug(renderer);
        if (m_vehicleAvailable) {
            drawVehicleDebug(renderer);
        }
        renderer.drawDebugBox(m_camera.state().target, {0.08f, 0.08f, 0.08f}, {1.0f, 0.25f, 0.7f, 1.0f});
    } else {
        drawPlaytestGuidance(renderer);
    }
    renderer.drawDebugText(m_debugText);
}

void SandboxLayer::onDetach()
{
    if (m_vehicleRuntimeAdapter) {
        m_vehicleRuntimeAdapter->shutdown();
        m_vehicleRuntimeAdapter.reset();
    }
    if (m_vehiclePhysicsWorld) {
        m_vehiclePhysicsWorld->shutdown();
        m_vehiclePhysicsWorld.reset();
    }
    engine::Logger::info("Sandbox layer detached.");
}

std::string SandboxLayer::debugText() const
{
    return m_debugText;
}

void SandboxLayer::updateDebugText()
{
    if (m_uiMode == engine::UiMode::Debug) {
        m_debugText = buildFullDebugText();
    } else {
        m_debugText = buildPresentationText(m_uiMode == engine::UiMode::Minimal);
    }
}

std::string SandboxLayer::buildPresentationText(bool minimal) const
{
    if (isTargetSliceRuntimeScene()) {
        return buildNeutralScenePresentationText(minimal);
    }

    const PlayerState& player = m_player.state();
    const VehicleState& vehicle = m_vehicle.state();
    const VehicleFocus& vehicleFocus = m_vehicle.focus();
    const InteractionFocus& focus = m_scene.interactions().focus();
    const TraversalFocus& traversalFocus = m_scene.traversal().focus();
    const FerryOfficeJobPhase phase = m_scene.job().phase(m_scene.worldState());

    std::ostringstream output;
    output << std::fixed << std::setprecision(1)
           << "Objective: " << m_scene.currentJobObjectiveText() << "\n";

    bool hasPrompt = false;
    if (focus.hasFocus) {
        output << "Prompt: Press E: " << focus.prompt << "\n";
        hasPrompt = true;
    } else if (traversalFocus.hasFocus && player.traversalMode == PlayerTraversalMode::Normal) {
        output << "Prompt: Press Space: " << traversalFocus.prompt << "\n";
        hasPrompt = true;
    } else if (vehicle.occupied) {
        output << "Prompt: Press E: Exit Service Yard Vehicle "
               << (isVehicleExitPositionClear(m_vehicle.exitPosition()) ? "(clear)" : "(blocked)") << "\n";
        hasPrompt = true;
    } else if (vehicleFocus.canEnter) {
        output << "Prompt: " << vehicleFocus.prompt << "\n";
        hasPrompt = true;
    }

    if (!hasPrompt && !minimal) {
        output << "Prompt: Move near highlighted markers, then press E or Space.\n";
    }

    output << "Job: " << PresentationJobStepText(phase)
           << " | " << (m_scene.isJobComplete() ? "complete" : "in progress");
    if (!minimal) {
        output << " | vehicle=" << (m_scene.worldState().isFlagSet(WorldFlag::ServiceVehicleUsed) ? "used" : "later")
               << " | checkpoint=" << (m_scene.worldState().isFlagSet(WorldFlag::DockRoadReached) ? "reached" : "later")
               << " | confirm=" << (m_scene.worldState().isFlagSet(WorldFlag::ServiceRunConfirmed) ? "done" : "later");
    }
    output << "\n";

    if (!minimal) {
        output << "Status: service gate=" << (m_scene.worldState().isFlagSet(WorldFlag::RouteOpened) ? "open" : "closed")
               << " | power=" << (m_scene.worldState().isFlagSet(WorldFlag::PowerRestored) ? "restored" : "offline")
               << " | mode=" << (vehicle.occupied ? "driving" : "on foot") << "\n";
        const bool showFollowupStatus = m_scene.isJobComplete()
            || m_scene.worldState().isFlagSet(WorldFlag::DockRoadRelayReset)
            || m_scene.worldState().isFlagSet(WorldFlag::DockRoadRelayLogged)
            || m_scene.worldState().isFlagSet(WorldFlag::DockRoadClearanceTagged);
        if (showFollowupStatus) {
            output << FerryOfficeFollowupStatusText(m_scene.worldState()) << "\n";
            const std::string nextStep = FerryOfficeFollowupNextStepText(m_scene.worldState());
            if (!nextStep.empty()) {
                output << "Next: " << nextStep << "\n";
            }
        }
        if (vehicle.occupied) {
            output << "Drive: speed=" << vehicle.speed
                   << " | checkpoint="
                   << (m_scene.worldState().isFlagSet(WorldFlag::DockRoadReached) ? "reached" : "drive east")
                   << " | exit=" << (isVehicleExitPositionClear(m_vehicle.exitPosition()) ? "clear" : "blocked") << "\n";
        }
        output << "F1: debug | Esc: quit";
    }

    return output.str();
}

std::string SandboxLayer::buildNeutralScenePresentationText(bool minimal) const
{
    const InteractionFocus& focus = m_scene.interactions().focus();
    const TraversalFocus& traversalFocus = m_scene.traversal().focus();
    const PlayerState& player = m_player.state();
    const std::string sceneName = m_sceneDefinitionLoaded ? m_sceneDefinition.name : "Built-in fallback";
    const std::string kind = m_sceneDefinitionLoaded ? m_sceneDefinition.sliceMetadata.kind : "fallback";
    const std::string vehicleText = m_vehicleAvailable ? "authored" : "none";

    std::ostringstream output;
    output << "Scene: " << sceneName << " | role=" << kind << "\n"
           << "Objective: " << m_scene.currentJobObjectiveText() << "\n";

    bool hasPrompt = false;
    if (focus.hasFocus) {
        output << "Prompt: Press E: " << focus.prompt << "\n";
        hasPrompt = true;
    } else if (traversalFocus.hasFocus && player.traversalMode == PlayerTraversalMode::Normal) {
        output << "Prompt: Press Space: " << traversalFocus.prompt << "\n";
        hasPrompt = true;
    }
    if (!hasPrompt && !minimal) {
        output << "Prompt: Inspect authored neutral markers; no regression job chain is active.\n";
    }

    if (!minimal) {
        output << "Status: colliders=" << m_scene.world().colliders().size()
               << " | interactables=" << m_scene.interactions().interactableCount()
               << " | routes=" << (m_sceneDefinitionLoaded ? m_sceneDefinition.routeMarkers.size() : 0)
               << " | markers=" << (m_sceneDefinitionLoaded ? m_sceneDefinition.objectiveMarkers.size() : 0)
               << " | vehicle=" << vehicleText << "\n"
               << "F1: debug | Esc: quit";
    }

    return output.str();
}

std::string SandboxLayer::buildFullDebugText() const
{
    if (isTargetSliceRuntimeScene()) {
        return buildNeutralSceneDebugText();
    }

    const PlayerState& player = m_player.state();
    const VehicleState& vehicle = m_vehicle.state();
    const VehicleFocus& vehicleFocus = m_vehicle.focus();
    const ThirdPersonCameraState& camera = m_camera.state();
    const InteractionFocus& focus = m_scene.interactions().focus();
    const TraversalFocus& traversalFocus = m_scene.traversal().focus();
    const VehicleControllerSettings& vehicleSettings = m_vehicle.settings();
    const std::string sceneId = m_sceneDefinitionLoaded ? m_sceneDefinition.id : "built-in-fallback";

    std::ostringstream output;
    output << std::fixed << std::setprecision(2)
           << "objective=\"" << m_scene.currentObjectiveText() << "\" "
           << "readyForExit=" << (m_scene.isSliceReadyForExit() ? "yes" : "no") << " "
           << "sliceComplete=" << (m_scene.isSliceComplete() ? "yes" : "no") << "\n"
           << "jobObjective=\"" << m_scene.currentJobObjectiveText() << "\" "
           << m_scene.jobDebugSummary() << "\n"
           << "focus=" << (focus.hasFocus ? focus.name : "none") << " ";
    if (focus.hasFocus) {
        output << "prompt=\"Press E: " << focus.prompt << "\" ";
    }
    if (traversalFocus.hasFocus && player.traversalMode == PlayerTraversalMode::Normal) {
        output << "travPrompt=\"" << traversalFocus.prompt << "\" ";
    }
    if (vehicle.occupied) {
        output << "vehiclePrompt=\"Press E: Exit Service Yard Vehicle\" ";
    } else if (!focus.hasFocus && vehicleFocus.canEnter) {
        output << "vehiclePrompt=\"" << vehicleFocus.prompt << "\" ";
    }
    output << "travFocus=" << (traversalFocus.hasFocus ? traversalFocus.name : "none") << "\n"
           << "player=(" << player.position.x << "," << player.position.y << "," << player.position.z << ") "
           << "playerYaw=" << engine::Degrees(player.facingYawRadians) << " "
           << "speed=" << player.horizontalSpeed << " "
           << (player.sprinting ? "sprint" : "walk") << " "
           << (player.grounded ? "grounded" : "air") << " "
           << "hits=" << player.lastCollisionHitCount << " "
           << "colliders=" << m_scene.world().colliders().size() << " "
           << "interactables=" << m_scene.interactions().interactableCount() << " "
           << "cameraMode=" << (vehicle.occupied ? "vehicle" : "on-foot") << " "
           << "camera yaw=" << engine::Degrees(camera.yawRadians)
           << " pitch=" << engine::Degrees(camera.pitchRadians)
           << " dist=" << camera.distance << "\n"
           << "vehicleAvailable=" << (m_vehicleAvailable ? "yes" : "no") << " "
           << "vehicle=(" << vehicle.position.x << "," << vehicle.position.y << "," << vehicle.position.z << ") "
           << (vehicle.occupied ? "occupied" : "empty") << " "
           << "speed=" << vehicle.speed << " "
           << "throttle=" << vehicle.throttle << " "
           << "brake=" << vehicle.brake << " "
           << "steer=" << vehicle.steer << " "
           << "focus=" << (vehicleFocus.canEnter ? "yes" : "no") << " "
           << "exitClear=" << (m_vehicleAvailable && isVehicleExitPositionClear(m_vehicle.exitPosition()) ? "yes" : "no") << " "
           << "exitBlocked=" << (vehicle.exitBlockedThisFrame ? "yes" : "no") << " "
           << "boundsHit=" << (vehicle.hitBoundsThisFrame ? "yes" : "no") << " "
           << "scene=" << sceneId << " "
           << "loaded=" << (m_sceneDefinitionLoaded ? "yes" : "no") << " "
           << "roadSegment=dock-road "
           << "roadBounds=(" << vehicleSettings.boundsMinX << "," << vehicleSettings.boundsMinZ << ")-("
           << vehicleSettings.boundsMaxX << "," << vehicleSettings.boundsMaxZ << ") "
           << "physics=" << m_vehiclePhysicsBackendText << " "
           << "vehicleRuntime=" << m_vehicleRuntimeText << "\n"
           << "interactPressed=" << (m_interactPressedThisFrame ? "yes" : "no") << " "
           << "worldChanged=" << (m_worldStateChangedThisFrame ? "yes" : "no") << " "
           << "traversal=" << (player.traversalMode == PlayerTraversalMode::Traversing ? "active" : "normal") << " "
           << "travProgress=" << player.traversalProgress << " "
           << "travStart=" << (player.traversalUsesCurrentPlayerPositionStart ? "current" : "fixed") << " "
           << "travLanded=" << (player.traversalLandedThisFrame ? "yes" : "no") << " "
           << "travPressed=" << (m_traversalPressedThisFrame ? "yes" : "no") << " ";
    if (player.traversalMode == PlayerTraversalMode::Traversing) {
        output << "travFrom=(" << player.traversalStartPosition.x << "," << player.traversalStartPosition.z << ") "
               << "travTo=(" << player.traversalTargetPosition.x << "," << player.traversalTargetPosition.z << ") ";
    }
    output << "\nlastInteraction=\"" << m_lastInteractionText << "\" "
           << "lastVehicle=\"" << m_lastVehicleText << "\" "
           << "lastWorldEvent=\"" << m_lastWorldEventText << "\"\n"
           << "worldState={" << m_scene.worldState().debugSummary() << "}\n"
           << "slice={" << m_scene.completionSummary() << "}";
    return output.str();
}

std::string SandboxLayer::buildNeutralSceneDebugText() const
{
    const PlayerState& player = m_player.state();
    const ThirdPersonCameraState& camera = m_camera.state();
    const InteractionFocus& focus = m_scene.interactions().focus();
    const TraversalFocus& traversalFocus = m_scene.traversal().focus();
    const std::string sceneId = m_sceneDefinitionLoaded ? m_sceneDefinition.id : "built-in-fallback";
    const std::string kind = m_sceneDefinitionLoaded ? m_sceneDefinition.sliceMetadata.kind : "fallback";
    const std::string worldId = m_sceneDefinitionLoaded ? m_sceneDefinition.sliceMetadata.worldId : "fallback";
    const std::string sliceId = m_sceneDefinitionLoaded ? m_sceneDefinition.sliceMetadata.sliceId : "fallback";
    const std::string vehicleText = m_vehicleAvailable ? "authored" : "none";

    std::ostringstream output;
    output << std::fixed << std::setprecision(2)
           << "scene=" << sceneId << " loaded=" << (m_sceneDefinitionLoaded ? "yes" : "no")
           << " role=" << kind << " world=" << worldId << " sliceId=" << sliceId << "\n"
           << "objective=\"" << m_scene.currentObjectiveText() << "\" "
           << "sliceComplete=" << (m_scene.isSliceComplete() ? "yes" : "no") << "\n"
           << "focus=" << (focus.hasFocus ? focus.name : "none") << " ";
    if (focus.hasFocus) {
        output << "prompt=\"Press E: " << focus.prompt << "\" ";
    }
    if (traversalFocus.hasFocus && player.traversalMode == PlayerTraversalMode::Normal) {
        output << "travPrompt=\"" << traversalFocus.prompt << "\" ";
    }
    output << "travFocus=" << (traversalFocus.hasFocus ? traversalFocus.name : "none") << "\n"
           << "player=(" << player.position.x << "," << player.position.y << "," << player.position.z << ") "
           << "playerYaw=" << engine::Degrees(player.facingYawRadians) << " "
           << "speed=" << player.horizontalSpeed << " "
           << (player.sprinting ? "sprint" : "walk") << " "
           << (player.grounded ? "grounded" : "air") << " "
           << "hits=" << player.lastCollisionHitCount << " "
           << "cameraMode=on-foot "
           << "camera yaw=" << engine::Degrees(camera.yawRadians)
           << " pitch=" << engine::Degrees(camera.pitchRadians)
           << " dist=" << camera.distance << "\n"
           << "sceneCounts=colliders:" << m_scene.world().colliders().size()
           << " interactables:" << m_scene.interactions().interactableCount()
           << " routes:" << (m_sceneDefinitionLoaded ? m_sceneDefinition.routeMarkers.size() : 0)
           << " markers:" << (m_sceneDefinitionLoaded ? m_sceneDefinition.objectiveMarkers.size() : 0)
           << " vehicle:" << vehicleText << " "
           << "physics=" << m_vehiclePhysicsBackendText << " "
           << "vehicleRuntime=" << m_vehicleRuntimeText << "\n"
           << "input=interact:" << (m_interactPressedThisFrame ? "yes" : "no")
           << " traversal:" << (m_traversalPressedThisFrame ? "yes" : "no") << " "
           << "lastInteraction=\"" << m_lastInteractionText << "\" "
           << "lastWorldEvent=\"" << m_lastWorldEventText << "\"\n"
           << "slice={" << m_scene.completionSummary() << "}";
    return output.str();
}

void SandboxLayer::toggleDebugUiMode()
{
    if (m_uiMode == engine::UiMode::Debug) {
        m_uiMode = m_nonDebugUiMode;
        engine::Logger::info("UI mode: " + std::string(engine::UiModeName(m_uiMode)));
        return;
    }

    m_nonDebugUiMode = m_uiMode;
    m_uiMode = engine::UiMode::Debug;
    engine::Logger::info("UI mode: debug");
}

bool SandboxLayer::shouldDrawFullGuidance() const
{
    return m_uiMode == engine::UiMode::Debug;
}

bool SandboxLayer::shouldDrawRouteMarker(std::string_view routeId) const
{
    if (shouldDrawFullGuidance()) {
        return true;
    }
    if (!isFerryOfficeRuntimeScene()) {
        return true;
    }

    return routeId == FerryOfficeActiveRouteMarkerId(m_scene.worldState(), m_scene.job().phase(m_scene.worldState()));
}

bool SandboxLayer::shouldDrawObjectiveMarker(std::string_view markerId) const
{
    if (shouldDrawFullGuidance()) {
        return true;
    }
    if (!isFerryOfficeRuntimeScene()) {
        return true;
    }

    const FerryOfficeJobPhase phase = m_scene.job().phase(m_scene.worldState());
    if (markerId == "dock-start-marker" || markerId == "office-marker") {
        return phase == FerryOfficeJobPhase::CollectManifest
            || phase == FerryOfficeJobPhase::UseServiceRoute
            || phase == FerryOfficeJobPhase::RestorePower
            || phase == FerryOfficeJobPhase::OpenServiceGate;
    }
    if (markerId == "service-yard-marker") {
        return phase == FerryOfficeJobPhase::UseServiceVehicle
            || phase == FerryOfficeJobPhase::ReachDockRoad;
    }
    if (markerId == "dock-road-marker") {
        return phase == FerryOfficeJobPhase::ReachDockRoad;
    }

    const std::string_view activeMarker = FerryOfficeActiveObjectiveMarkerId(m_scene.worldState(), phase);
    if (!activeMarker.empty()) {
        return markerId == activeMarker;
    }

    return false;
}

bool SandboxLayer::shouldDrawInteractableMarker(const Interactable& interactable) const
{
    if (shouldDrawFullGuidance()) {
        return true;
    }
    if (!isFerryOfficeRuntimeScene()) {
        return true;
    }

    const InteractionFocus& focus = m_scene.interactions().focus();
    if (focus.hasFocus && focus.interactableId == interactable.id) {
        return true;
    }
    if (!interactable.enabled || interactable.consumed) {
        return false;
    }
    if (interactable.name == FerryOffice::Names::FerryManifest || interactable.name == FerryOffice::Names::FerryOfficeNotice) {
        return true;
    }
    if (interactable.name == FerryOffice::Names::MaintenanceBox) {
        return m_scene.worldState().isFlagSet(WorldFlag::ServiceRouteUsed)
            || m_scene.worldState().isFlagSet(WorldFlag::PowerRestored);
    }
    if (interactable.name == FerryOffice::Names::WallButton) {
        return m_scene.worldState().isFlagSet(WorldFlag::PowerRestored)
            || m_scene.worldState().isFlagSet(WorldFlag::RouteOpened);
    }
    if (interactable.name == FerryOffice::Names::ExitMarker) {
        return m_scene.isSliceReadyForExit() || m_scene.isSliceComplete();
    }
    if (interactable.name == FerryOffice::Names::ServiceRunMarker) {
        const FerryOfficeJobPhase phase = m_scene.job().phase(m_scene.worldState());
        return phase == FerryOfficeJobPhase::ReachDockRoad
            || phase == FerryOfficeJobPhase::ConfirmServiceRun
            || phase == FerryOfficeJobPhase::Complete;
    }

    return true;
}

bool SandboxLayer::shouldDrawTraversalMarker(const TraversalAffordance& affordance) const
{
    if (shouldDrawFullGuidance()) {
        return true;
    }
    if (!isFerryOfficeRuntimeScene()) {
        return true;
    }

    const TraversalFocus& focus = m_scene.traversal().focus();
    const PlayerState& player = m_player.state();
    if ((focus.hasFocus && focus.affordanceId == affordance.id) || player.activeTraversalId == affordance.id) {
        return true;
    }

    return m_scene.job().phase(m_scene.worldState()) == FerryOfficeJobPhase::UseServiceRoute;
}

bool SandboxLayer::shouldDrawVehicleGuidance() const
{
    if (!m_vehicleAvailable) {
        return false;
    }
    if (shouldDrawFullGuidance() || m_vehicle.state().occupied) {
        return true;
    }

    const FerryOfficeJobPhase phase = m_scene.job().phase(m_scene.worldState());
    return phase == FerryOfficeJobPhase::UseServiceVehicle
        || phase == FerryOfficeJobPhase::ReachDockRoad
        || phase == FerryOfficeJobPhase::ConfirmServiceRun
        || phase == FerryOfficeJobPhase::Complete;
}

bool SandboxLayer::isFerryOfficeRuntimeScene() const
{
    return !m_sceneDefinitionLoaded || IsFerryOfficeRegressionScene(m_sceneDefinition);
}

bool SandboxLayer::isTargetSliceRuntimeScene() const
{
    return m_sceneDefinitionLoaded && IsTargetSliceScaffoldScene(m_sceneDefinition);
}

void SandboxLayer::drawInteractionDebug(engine::IRenderer& renderer)
{
    const InteractionFocus& focus = m_scene.interactions().focus();
    for (const Interactable& interactable : m_scene.interactions().interactables()) {
        if (!shouldDrawInteractableMarker(interactable)) {
            continue;
        }

        const bool isFocused = focus.hasFocus && focus.interactableId == interactable.id;
        engine::Color color {0.35f, 0.75f, 1.0f, 1.0f};
        float markerSize = 0.16f;
        float beaconHeight = 0.75f;
        if (interactable.type == InteractableType::Pickup) {
            color = {0.35f, 1.0f, 0.45f, 1.0f};
            markerSize = 0.20f;
        } else if (interactable.type == InteractableType::Toggle) {
            color = interactable.toggled
                ? engine::Color {1.0f, 0.82f, 0.25f, 1.0f}
                : engine::Color {1.0f, 0.55f, 0.25f, 1.0f};
        }
        if (interactable.name == FerryOffice::Names::FerryOfficeNotice) {
            color = {0.45f, 0.70f, 1.0f, 1.0f};
        }
        if (interactable.name == FerryOffice::Names::WallButton && m_scene.worldState().isFlagSet(WorldFlag::RouteOpened)) {
            color = {0.25f, 1.0f, 0.35f, 1.0f};
        }
        if (interactable.name == FerryOffice::Names::MaintenanceBox && m_scene.worldState().isFlagSet(WorldFlag::PowerRestored)) {
            color = {0.25f, 1.0f, 0.85f, 1.0f};
        }
        if (interactable.name == FerryOffice::Names::MaintenanceBox) {
            markerSize = 0.22f;
            beaconHeight = 1.15f;
        }
        if (interactable.name == FerryOffice::Names::WallButton) {
            markerSize = 0.18f;
            beaconHeight = 1.0f;
        }
        if (interactable.name == FerryOffice::Names::ExitMarker) {
            if (m_scene.isSliceComplete()) {
                color = {0.35f, 1.0f, 0.35f, 1.0f};
            } else if (m_scene.isSliceReadyForExit()) {
                color = {1.0f, 0.9f, 0.25f, 1.0f};
            } else {
                color = {0.45f, 0.45f, 0.55f, 1.0f};
            }
            markerSize = m_scene.isSliceReadyForExit() ? 0.34f : 0.24f;
            beaconHeight = m_scene.isSliceReadyForExit() ? 1.65f : 0.9f;
        }
        if (interactable.name == FerryOffice::Names::ServiceRunMarker) {
            if (m_scene.isJobComplete()) {
                color = {0.35f, 1.0f, 0.35f, 1.0f};
            } else if (m_scene.job().isReadyForConfirmation(m_scene.worldState())) {
                color = {1.0f, 0.82f, 0.25f, 1.0f};
            } else {
                color = {0.60f, 0.45f, 0.95f, 1.0f};
            }
            markerSize = 0.26f;
            beaconHeight = 1.35f;
        }

        if (!interactable.enabled || interactable.consumed) {
            color = {0.35f, 0.35f, 0.35f, 1.0f};
            markerSize = 0.14f;
        }
        if (isFocused) {
            color = {1.0f, 1.0f, 0.25f, 1.0f};
            markerSize += 0.06f;
            beaconHeight += 0.45f;
        }

        renderer.drawDebugSolidBox(interactable.position, {markerSize, markerSize, markerSize}, ScaleColor(color, isFocused ? 0.65f : 0.45f));
        renderer.drawDebugBox(interactable.position, {markerSize, markerSize, markerSize}, color);
        renderer.drawDebugBox(
            {interactable.position.x, m_scene.world().floorHeight() + 0.03f, interactable.position.z},
            {interactable.radius, 0.03f, interactable.radius},
            color);
        renderer.drawDebugLine(interactable.position,
            interactable.position + engine::Vec3 {0.0f, beaconHeight, 0.0f},
            color);
    }
}

void SandboxLayer::drawWorldStateDebug(engine::IRenderer& renderer)
{
    const bool routeOpened = m_scene.worldState().isFlagSet(WorldFlag::RouteOpened);
    const engine::Color gateColor = routeOpened
        ? engine::Color {0.25f, 1.0f, 0.35f, 1.0f}
        : engine::Color {1.0f, 0.25f, 0.2f, 1.0f};
    if (const StaticCollider* gate = m_scene.world().colliderByName(FerryOffice::Names::ServiceGateCollider)) {
        renderer.drawDebugSolidBox(gate->bounds.center, gate->bounds.halfExtents + engine::Vec3 {0.03f, 0.03f, 0.03f}, ScaleColor(gateColor, routeOpened ? 0.45f : 0.30f));
        renderer.drawDebugBox(gate->bounds.center, gate->bounds.halfExtents + engine::Vec3 {0.04f, 0.04f, 0.04f}, gateColor);
    }

    const bool powerRestored = m_scene.worldState().isFlagSet(WorldFlag::PowerRestored);
    const engine::Color powerColor = powerRestored
        ? engine::Color {0.25f, 1.0f, 0.85f, 1.0f}
        : engine::Color {0.45f, 0.45f, 0.55f, 1.0f};
    renderer.drawDebugSolidBox({FerryOffice::Positions::MaintenanceBox.x, m_scene.world().floorHeight() + 0.75f, FerryOffice::Positions::MaintenanceBox.z},
        {0.24f, 0.24f, 0.24f},
        ScaleColor(powerColor, 0.5f));
    renderer.drawDebugBox({FerryOffice::Positions::MaintenanceBox.x, m_scene.world().floorHeight() + 0.75f, FerryOffice::Positions::MaintenanceBox.z},
        {0.24f, 0.24f, 0.24f},
        powerColor);
}

void SandboxLayer::drawSceneVisualPlaceholders(engine::IRenderer& renderer)
{
    const ScenePresentationState presentationState {
        m_scene.worldState().isFlagSet(WorldFlag::RouteOpened),
        m_scene.worldState().isFlagSet(WorldFlag::PowerRestored),
        m_vehicle.state().occupied,
        m_scene.worldState().isFlagSet(WorldFlag::DockRoadRelayReset),
        m_scene.worldState().isFlagSet(WorldFlag::DockRoadClearanceTagged),
        m_scene.worldState().isFlagSet(WorldFlag::FerryOfficeHandoffFiled),
        m_scene.worldState().isFlagSet(WorldFlag::StormPumpReset),
        m_scene.worldState().isFlagSet(WorldFlag::StormPumpTicketClosed),
        m_scene.worldState().isFlagSet(WorldFlag::LowDockDrainCleared),
        m_scene.worldState().isFlagSet(WorldFlag::LowDockDrainLogged),
    };
    const bool fullDebug = m_uiMode == engine::UiMode::Debug;

    for (const SceneVisualPlaceholderDefinition& placeholder : m_sceneDefinition.visualPlaceholders) {
        const SceneMaterial material = SceneMaterialForKey(placeholder.colorKey, m_sceneDefinition.sceneMaterials, presentationState);
        DrawSceneShadedBox(renderer, placeholder.center, placeholder.halfExtents, material);

        if (fullDebug
            && (placeholder.role.find("pad") != std::string::npos
            || placeholder.role.find("marker") != std::string::npos
            || placeholder.role.find("bound") != std::string::npos
            || placeholder.role.find("curb") != std::string::npos)) {
            renderer.drawDebugBox(
                {placeholder.center.x, placeholder.center.y + 0.02f, placeholder.center.z},
                {placeholder.halfExtents.x, std::max(placeholder.halfExtents.y, 0.02f), placeholder.halfExtents.z},
                ScaleColor(material.baseColor, 1.65f));
        }
    }
}

void SandboxLayer::drawPlayerPresentation(engine::IRenderer& renderer)
{
    const PlayerState& player = m_player.state();
    const engine::Color bootColor {0.05f, 0.10f, 0.11f, 1.0f};
    const engine::Color sleeveColor {0.05f, 0.12f, 0.13f, 1.0f};
    renderer.drawDebugSolidBox(player.position + engine::Vec3 {-0.10f, 0.38f, 0.0f},
        {0.07f, 0.34f, 0.08f},
        bootColor);
    renderer.drawDebugSolidBox(player.position + engine::Vec3 {0.10f, 0.38f, 0.0f},
        {0.07f, 0.34f, 0.08f},
        bootColor);
    engine::StaticMeshInstance raincoat;
    raincoat.assetId = "runtime-player-raincoat-proxy";
    raincoat.position = player.position;
    raincoat.yawRadians = player.facingYawRadians;
    SceneMaterial raincoatMaterial;
    raincoatMaterial.baseColor = {0.06f, 0.17f, 0.19f, 1.0f};
    raincoatMaterial.ambientShade = 0.66f;
    raincoatMaterial.directionalShade = 0.30f;
    raincoatMaterial.topShade = 0.12f;
    DrawMeshInstance(renderer, PlayerRaincoatMesh(), raincoat, raincoatMaterial);
    renderer.drawDebugSolidBox(player.position + engine::Vec3 {-0.33f, 0.94f, 0.0f},
        {0.055f, 0.25f, 0.065f},
        sleeveColor);
    renderer.drawDebugSolidBox(player.position + engine::Vec3 {0.33f, 0.94f, 0.0f},
        {0.055f, 0.25f, 0.065f},
        sleeveColor);
    renderer.drawDebugSolidBox(player.position + engine::Vec3 {0.0f, 1.32f, -0.01f},
        {0.18f, 0.08f, 0.14f},
        {0.07f, 0.15f, 0.17f, 1.0f});
    renderer.drawDebugSolidBox(player.position + engine::Vec3 {0.0f, 1.52f, 0.0f},
        {0.15f, 0.15f, 0.14f},
        {0.38f, 0.30f, 0.22f, 1.0f});
}

void SandboxLayer::drawPlayerDebug(engine::IRenderer& renderer)
{
    const PlayerState& player = m_player.state();
    renderer.drawDebugBox(player.position + engine::Vec3 {0.0f, m_player.settings().height * 0.5f, 0.0f},
        {m_player.settings().radius, m_player.settings().height * 0.5f, m_player.settings().radius},
        {0.25f, 0.55f, 1.0f, 1.0f});

    const engine::Vec3 facing = engine::ForwardFromYaw(player.facingYawRadians);
    renderer.drawDebugLine(player.position + engine::Vec3 {0.0f, 1.0f, 0.0f},
        player.position + engine::Vec3 {0.0f, 1.0f, 0.0f} + facing * 1.25f,
        {1.0f, 1.0f, 1.0f, 1.0f});
    if (engine::Length(player.lastCollisionPush) > 0.0f) {
        renderer.drawDebugLine(player.position + engine::Vec3 {0.0f, 0.25f, 0.0f},
            player.position + engine::Vec3 {0.0f, 0.25f, 0.0f} + player.lastCollisionNormal,
            {1.0f, 0.2f, 0.2f, 1.0f});
    }
}

void SandboxLayer::drawSliceDebug(engine::IRenderer& renderer)
{
    const float floor = m_scene.world().floorHeight();
    const engine::Color dockColor {0.25f, 0.65f, 1.0f, 1.0f};
    const engine::Color officeColor {0.95f, 0.95f, 0.8f, 1.0f};
    const engine::Color exitColor = m_scene.isSliceComplete()
        ? engine::Color {0.35f, 1.0f, 0.35f, 1.0f}
        : (m_scene.isSliceReadyForExit()
                  ? engine::Color {1.0f, 0.9f, 0.25f, 1.0f}
                  : engine::Color {0.45f, 0.45f, 0.55f, 1.0f});
    const engine::Color routeColor {0.70f, 0.92f, 1.0f, 1.0f};

    if (m_sceneDefinitionLoaded) {
        for (const SceneObjectiveMarkerDefinition& marker : m_sceneDefinition.objectiveMarkers) {
            if (!shouldDrawObjectiveMarker(marker.id)) {
                continue;
            }

            engine::Color color = dockColor;
            engine::Vec3 halfExtents {0.20f, 0.08f, 0.20f};
            if (marker.id.find("office") != std::string::npos) {
                color = officeColor;
                halfExtents = {0.35f, 0.35f, 0.35f};
            } else if (marker.id.find("service-run") != std::string::npos) {
                color = {1.0f, 0.82f, 0.25f, 1.0f};
                halfExtents = {0.30f, 0.22f, 0.30f};
            } else if (marker.id.find("road") != std::string::npos) {
                color = {1.0f, 0.82f, 0.25f, 1.0f};
                halfExtents = {0.26f, 0.18f, 0.26f};
            }

            renderer.drawDebugSolidBox(marker.position, halfExtents, ScaleColor(color, 0.45f));
            renderer.drawDebugBox(marker.position, halfExtents, color);
            renderer.drawDebugLine(marker.position, marker.position + engine::Vec3 {0.0f, 1.0f, 0.0f}, color);
        }

        for (const SceneRouteMarkerDefinition& marker : m_sceneDefinition.routeMarkers) {
            if (!shouldDrawRouteMarker(marker.id)) {
                continue;
            }

            for (std::size_t index = 1; index < marker.points.size(); ++index) {
                renderer.drawDebugLine(marker.points[index - 1], marker.points[index], routeColor);
            }
        }
        if (!isFerryOfficeRuntimeScene()) {
            return;
        }
    } else {
        const engine::Vec3 dockStart {
            FerryOffice::Positions::DockStart.x,
            floor + FerryOffice::Positions::DockStart.y,
            FerryOffice::Positions::DockStart.z,
        };
        const engine::Vec3 officeMarker {
            FerryOffice::Positions::OfficeMarker.x,
            floor + FerryOffice::Positions::OfficeMarker.y,
            FerryOffice::Positions::OfficeMarker.z,
        };
        renderer.drawDebugSolidBox(dockStart, {0.22f, 0.08f, 0.22f}, ScaleColor(dockColor, 0.45f));
        renderer.drawDebugBox(dockStart, {0.22f, 0.08f, 0.22f}, dockColor);
        renderer.drawDebugLine(dockStart, dockStart + engine::Vec3 {0.0f, 1.0f, 0.0f}, dockColor);
        renderer.drawDebugSolidBox(officeMarker, {0.35f, 0.35f, 0.35f}, ScaleColor(officeColor, 0.45f));
        renderer.drawDebugBox(officeMarker, {0.35f, 0.35f, 0.35f}, officeColor);
    }

    const engine::Vec3 exitMarker {
        FerryOffice::Positions::ExitMarker.x,
        floor + FerryOffice::Positions::ExitMarker.y,
        FerryOffice::Positions::ExitMarker.z,
    };
    renderer.drawDebugSolidBox(exitMarker, {0.28f, 0.28f, 0.28f}, ScaleColor(exitColor, 0.45f));
    renderer.drawDebugBox(exitMarker, {0.28f, 0.28f, 0.28f}, exitColor);
    renderer.drawDebugBox({exitMarker.x, floor + 0.03f, exitMarker.z}, {FerryOffice::Radii::ExitMarker, 0.03f, FerryOffice::Radii::ExitMarker}, exitColor);
}

void SandboxLayer::drawPlaytestGuidance(engine::IRenderer& renderer)
{
    if (!m_sceneDefinitionLoaded) {
        return;
    }

    const engine::Color routeColor {0.70f, 0.92f, 1.0f, 1.0f};
    const engine::Color objectiveColor {0.95f, 0.86f, 0.42f, 1.0f};

    for (const SceneRouteMarkerDefinition& marker : m_sceneDefinition.routeMarkers) {
        if (!shouldDrawRouteMarker(marker.id)) {
            continue;
        }

        for (std::size_t index = 1; index < marker.points.size(); ++index) {
            renderer.drawDebugLine(marker.points[index - 1], marker.points[index], routeColor);
        }
    }

    for (const SceneObjectiveMarkerDefinition& marker : m_sceneDefinition.objectiveMarkers) {
        if (!shouldDrawObjectiveMarker(marker.id)) {
            continue;
        }

        const engine::Vec3 center {marker.position.x, marker.position.y + 0.18f, marker.position.z};
        renderer.drawDebugSolidBox(center, {0.20f, 0.16f, 0.20f}, objectiveColor);
        renderer.drawDebugLine(center, center + engine::Vec3 {0.0f, 0.55f, 0.0f}, objectiveColor);
    }
}

void SandboxLayer::drawTraversalDebug(engine::IRenderer& renderer)
{
    const TraversalFocus& focus = m_scene.traversal().focus();
    const PlayerState& player = m_player.state();
    for (const TraversalAffordance& affordance : m_scene.traversal().affordances()) {
        if (!shouldDrawTraversalMarker(affordance)) {
            continue;
        }

        const bool isFocused = focus.hasFocus && focus.affordanceId == affordance.id;
        const bool isActive = player.activeTraversalId == affordance.id;
        engine::Color color {0.55f, 0.85f, 1.0f, 1.0f};
        if (!affordance.enabled) {
            color = {0.35f, 0.35f, 0.35f, 1.0f};
        } else if (isActive) {
            color = {1.0f, 0.35f, 1.0f, 1.0f};
        } else if (isFocused) {
            color = {1.0f, 1.0f, 0.25f, 1.0f};
        }

        renderer.drawDebugSolidBox(affordance.startPosition + engine::Vec3 {0.0f, 0.2f, 0.0f}, {0.18f, 0.18f, 0.18f}, ScaleColor(color, 0.55f));
        renderer.drawDebugBox(affordance.startPosition + engine::Vec3 {0.0f, 0.2f, 0.0f}, {0.18f, 0.18f, 0.18f}, color);
        renderer.drawDebugSolidBox(affordance.endPosition + engine::Vec3 {0.0f, 0.2f, 0.0f}, {0.18f, 0.18f, 0.18f}, {0.12f, 0.50f, 0.48f, 1.0f});
        renderer.drawDebugBox(affordance.endPosition + engine::Vec3 {0.0f, 0.2f, 0.0f}, {0.18f, 0.18f, 0.18f}, {0.25f, 1.0f, 0.95f, 1.0f});
        renderer.drawDebugLine(affordance.startPosition + engine::Vec3 {0.0f, 0.35f, 0.0f},
            affordance.endPosition + engine::Vec3 {0.0f, 0.35f, 0.0f},
            color);
        renderer.drawDebugBox(
            {affordance.startPosition.x, m_scene.world().floorHeight() + 0.04f, affordance.startPosition.z},
            {affordance.focusRadius, 0.03f, affordance.focusRadius},
            color);
    }

    if (player.traversalMode == PlayerTraversalMode::Traversing) {
        const engine::Vec3 activeStart = player.traversalStartPosition + engine::Vec3 {0.0f, 0.55f, 0.0f};
        const engine::Vec3 activeEnd = player.traversalTargetPosition + engine::Vec3 {0.0f, 0.55f, 0.0f};
        renderer.drawDebugSolidBox(activeStart, {0.14f, 0.14f, 0.14f}, {0.55f, 0.10f, 0.55f, 1.0f});
        renderer.drawDebugBox(activeStart, {0.14f, 0.14f, 0.14f}, {1.0f, 0.2f, 1.0f, 1.0f});
        renderer.drawDebugSolidBox(activeEnd, {0.14f, 0.14f, 0.14f}, {0.10f, 0.50f, 0.50f, 1.0f});
        renderer.drawDebugBox(activeEnd, {0.14f, 0.14f, 0.14f}, {0.2f, 1.0f, 1.0f, 1.0f});
        renderer.drawDebugLine(activeStart, player.position + engine::Vec3 {0.0f, 0.55f, 0.0f}, {1.0f, 0.2f, 1.0f, 1.0f});
        renderer.drawDebugLine(player.position + engine::Vec3 {0.0f, 0.55f, 0.0f}, activeEnd, {0.2f, 1.0f, 1.0f, 1.0f});
    }
}

void SandboxLayer::drawVehicleDebug(engine::IRenderer& renderer)
{
    const VehicleState& vehicle = m_vehicle.state();
    const VehicleFocus& focus = m_vehicle.focus();
    const VehicleControllerSettings& settings = m_vehicle.settings();
    const float floor = m_scene.world().floorHeight();

    const engine::Color vehicleColor = vehicle.occupied
        ? engine::Color {0.25f, 0.72f, 1.0f, 1.0f}
        : engine::Color {0.75f, 0.78f, 0.58f, 1.0f};
    const engine::Vec3 bodyCenter = vehicle.position + engine::Vec3 {0.0f, m_vehicleProxyHalfExtents.y, 0.0f};
    renderer.drawDebugSolidBox(bodyCenter, m_vehicleProxyHalfExtents, ScaleColor(vehicleColor, 0.50f));
    renderer.drawDebugBox(bodyCenter, m_vehicleProxyHalfExtents, vehicleColor);
    renderer.drawDebugSolidBox(vehicle.position + engine::Vec3 {0.0f, 0.86f, 0.0f}, m_vehicleCabinHalfExtents, ScaleColor(vehicleColor, 0.38f));
    renderer.drawDebugBox(vehicle.position + engine::Vec3 {0.0f, 0.86f, 0.0f}, m_vehicleCabinHalfExtents, vehicleColor);

    if (!shouldDrawVehicleGuidance()) {
        return;
    }

    const engine::Vec3 forward = m_vehicle.forward();
    renderer.drawDebugLine(vehicle.position + engine::Vec3 {0.0f, 0.75f, 0.0f},
        vehicle.position + engine::Vec3 {0.0f, 0.75f, 0.0f} + forward * 1.65f,
        {1.0f, 0.95f, 0.35f, 1.0f});

    const engine::Color focusColor = focus.canEnter
        ? engine::Color {1.0f, 1.0f, 0.25f, 1.0f}
        : engine::Color {0.35f, 0.55f, 0.65f, 1.0f};
    renderer.drawDebugBox({vehicle.position.x, floor + 0.04f, vehicle.position.z}, {settings.enterRadius, 0.03f, settings.enterRadius}, focusColor);

    const engine::Vec3 exitPosition = m_vehicle.exitPosition();
    const bool exitClear = isVehicleExitPositionClear(exitPosition);
    const engine::Color exitColor = exitClear
        ? engine::Color {0.30f, 1.0f, 0.65f, 1.0f}
        : engine::Color {1.0f, 0.25f, 0.20f, 1.0f};
    const engine::Color exitCoreColor = vehicle.exitBlockedThisFrame
        ? engine::Color {1.0f, 0.25f, 0.20f, 1.0f}
        : exitColor;
    renderer.drawDebugLine(vehicle.position + engine::Vec3 {0.0f, 0.20f, 0.0f}, exitPosition + engine::Vec3 {0.0f, 0.20f, 0.0f}, exitColor);
    renderer.drawDebugLine(exitPosition + engine::Vec3 {0.0f, 0.05f, 0.0f}, exitPosition + engine::Vec3 {0.0f, 1.15f, 0.0f}, exitColor);
    renderer.drawDebugBox({exitPosition.x, floor + 0.04f, exitPosition.z}, {m_player.settings().radius * 1.25f, 0.03f, m_player.settings().radius * 1.25f}, exitColor);
    renderer.drawDebugSolidBox(exitPosition + engine::Vec3 {0.0f, 0.25f, 0.0f}, {0.20f, 0.20f, 0.20f}, ScaleColor(exitCoreColor, 0.45f));
    renderer.drawDebugBox(exitPosition + engine::Vec3 {0.0f, 0.25f, 0.0f}, {0.20f, 0.20f, 0.20f}, exitCoreColor);

    const engine::Vec3 boundsCenter {
        (settings.boundsMinX + settings.boundsMaxX) * 0.5f,
        floor + 0.05f,
        (settings.boundsMinZ + settings.boundsMaxZ) * 0.5f,
    };
    const engine::Vec3 boundsHalf {
        (settings.boundsMaxX - settings.boundsMinX) * 0.5f,
        0.04f,
        (settings.boundsMaxZ - settings.boundsMinZ) * 0.5f,
    };
    renderer.drawDebugBox(boundsCenter, boundsHalf, vehicle.hitBoundsThisFrame ? engine::Color {1.0f, 0.3f, 0.2f, 1.0f} : engine::Color {0.40f, 0.90f, 0.70f, 1.0f});

    if (m_uiMode == engine::UiMode::Debug && m_vehiclePhysicsWorld) {
        for (const engine::physics::PhysicsDebugLine& line : m_vehiclePhysicsWorld->debugLines()) {
            renderer.drawDebugLine(line.from, line.to, {line.color.x, line.color.y, line.color.z, 1.0f});
        }
    }
}

void SandboxLayer::drawStaticMeshDebug(engine::IRenderer& renderer)
{
    if (m_staticMeshAssets.empty()) {
        return;
    }

    const ScenePresentationState presentationState {
        m_scene.worldState().isFlagSet(WorldFlag::RouteOpened),
        m_scene.worldState().isFlagSet(WorldFlag::PowerRestored),
        m_vehicle.state().occupied,
        m_scene.worldState().isFlagSet(WorldFlag::DockRoadRelayReset),
        m_scene.worldState().isFlagSet(WorldFlag::DockRoadClearanceTagged),
        m_scene.worldState().isFlagSet(WorldFlag::FerryOfficeHandoffFiled),
        m_scene.worldState().isFlagSet(WorldFlag::StormPumpReset),
        m_scene.worldState().isFlagSet(WorldFlag::StormPumpTicketClosed),
        m_scene.worldState().isFlagSet(WorldFlag::LowDockDrainCleared),
        m_scene.worldState().isFlagSet(WorldFlag::LowDockDrainLogged),
    };

    for (const SceneMeshInstanceDefinition& authored : m_sceneDefinition.meshInstances) {
        const auto meshIt = m_staticMeshAssets.find(authored.assetId);
        if (meshIt == m_staticMeshAssets.end()) {
            continue;
        }

        engine::StaticMeshInstance instance;
        instance.assetId = authored.assetId;
        instance.position = authored.position;
        instance.scale = authored.scale;
        const SceneMaterial material = SceneMaterialForKey(authored.colorKey, m_sceneDefinition.sceneMaterials, presentationState);
        instance.tint = material.baseColor;
        instance.yawRadians = authored.yawRadians;

        engine::Vec3 position = authored.position;
        float yawRadians = authored.yawRadians;
        if (authored.linkedColliderId == "service-yard-vehicle") {
            const SceneVehicleDefinition* vehicleDefinition = !m_sceneDefinition.vehicles.empty()
                ? &m_sceneDefinition.vehicles.front()
                : nullptr;
            const engine::Vec3 authoredVehiclePosition = vehicleDefinition ? vehicleDefinition->spawnPosition : ServiceYardVehicleSpawnPosition;
            const float authoredVehicleYaw = vehicleDefinition ? vehicleDefinition->spawnYawRadians : ServiceYardVehicleSpawnYawRadians;
            const float yawDelta = m_vehicle.state().yawRadians - authoredVehicleYaw;
            const engine::Vec3 authoredOffset = authored.position - authoredVehiclePosition;
            position = m_vehicle.state().position + RotateYawOffset(authoredOffset, yawDelta);
            yawRadians = m_vehicle.state().yawRadians + (authored.yawRadians - authoredVehicleYaw);
        }
        instance.position = position;
        instance.yawRadians = yawRadians;

        DrawMeshInstance(renderer, meshIt->second, instance, material);
    }
}

void SandboxLayer::recordWorldStateChange(bool changed)
{
    if (!changed) {
        return;
    }

    m_worldStateChangedThisFrame = true;
    m_lastWorldEventText = m_scene.worldState().lastEventText();
    engine::Logger::info("World event: " + m_lastWorldEventText);
}

void SandboxLayer::setupVehicleRuntimeAdapter()
{
    m_vehicleRuntimeText = "deterministic";
    m_vehicleRuntimeAdapter.reset();
    if (!m_vehicleAvailable) {
        m_vehicleRuntimeText = "none";
        return;
    }
    if (!m_vehicleRuntimeAdapterEnabled) {
        return;
    }

    m_vehicleRuntimeAdapter = engine::physics::CreateVehicleRuntimeAdapter(m_vehicleRuntimeBackend);
    if (!m_vehicleRuntimeAdapter) {
        m_vehicleRuntimeText = "adapter-unavailable";
        engine::Logger::warning("Selected live vehicle runtime backend is unavailable; using deterministic vehicle fallback.");
        return;
    }

    const VehicleState& vehicle = m_vehicle.state();
    const VehicleControllerSettings& settings = m_vehicle.settings();
    engine::physics::VehicleRuntimeConfig config;
    config.vehicleId = "service-yard-vehicle";
    config.spawnPosition = vehicle.position;
    config.spawnYawRadians = vehicle.yawRadians;
    config.halfExtents = m_vehicleProxyHalfExtents;
    config.boundsMin = {settings.boundsMinX, settings.boundsMinZ};
    config.boundsMax = {settings.boundsMaxX, settings.boundsMaxZ};
    if (m_sceneDefinitionLoaded) {
        config.staticObstacles = BuildAuthoredRoadEdgeRuntimeObstacles(m_sceneDefinition);
    }

    if (!m_vehicleRuntimeAdapter->initialize(config)) {
        std::string error = std::string(m_vehicleRuntimeAdapter->error());
        if (error.empty()) {
            error = "unknown initialization failure";
        }
        m_vehicleRuntimeText = std::string(m_vehicleRuntimeAdapter->backendName()) + "-init-failed";
        m_vehicleRuntimeAdapter.reset();
        engine::Logger::warning("Live vehicle runtime adapter failed to initialize; using deterministic vehicle fallback. " + error);
        return;
    }

    m_vehicleRuntimeText = std::string(m_vehicleRuntimeAdapter->backendName()) + "-live";
    engine::Logger::info("Live vehicle runtime adapter enabled: " + m_vehicleRuntimeText);
}

void SandboxLayer::updateVehicleDriving(float deltaSeconds, const engine::InputState& input)
{
    if (!m_vehicleRuntimeAdapter) {
        m_vehicle.updateDriving(deltaSeconds, input);
        return;
    }

    const float driveInput = engine::Clamp(input.moveForward, -1.0f, 1.0f);
    engine::physics::VehicleRuntimeInput runtimeInput;
    runtimeInput.steer = engine::Clamp(input.moveRight, -1.0f, 1.0f);
    if (driveInput < 0.0f && m_vehicle.state().speed > 0.05f) {
        runtimeInput.brake = -driveInput;
    } else {
        runtimeInput.throttle = driveInput;
    }

    if (!m_vehicleRuntimeAdapter->step(runtimeInput, deltaSeconds)) {
        m_lastVehicleText = "Live vehicle runtime step failed; deterministic fallback remains available next run.";
        engine::Logger::warning("Vehicle: " + m_lastVehicleText + " " + std::string(m_vehicleRuntimeAdapter->error()));
        return;
    }

    const engine::physics::VehicleRuntimeState runtimeState = m_vehicleRuntimeAdapter->state();
    m_vehicle.applyRuntimeState(
        runtimeState.position,
        runtimeState.yawRadians,
        runtimeState.speed,
        runtimeInput.throttle,
        runtimeInput.brake,
        runtimeInput.steer,
        runtimeState.outOfBounds);
}

void SandboxLayer::setupVehiclePhysicsWorld()
{
    if (!m_vehicleAvailable) {
        m_vehiclePhysicsBackendText = "none";
        m_vehiclePhysicsWorld.reset();
        return;
    }

    m_vehiclePhysicsWorld = engine::physics::CreatePhysicsWorld(engine::physics::PhysicsBackend::Simple);
    if (!m_vehiclePhysicsWorld) {
        m_vehiclePhysicsBackendText = "none";
        return;
    }

    engine::physics::PhysicsConfig config;
    config.backend = engine::physics::PhysicsBackend::Simple;
    if (!m_vehiclePhysicsWorld->initialize(config)) {
        m_vehiclePhysicsBackendText = "init-failed";
        m_vehiclePhysicsWorld.reset();
        return;
    }

    m_vehiclePhysicsBackendText = std::string(m_vehiclePhysicsWorld->backendName());
    m_vehiclePhysicsWorld->addFloor("vehicle-yard-physics-floor", m_scene.world().floorHeight(), 12.0f, 0.05f);

    if (m_sceneDefinitionLoaded) {
        for (const SceneVisualPlaceholderDefinition& placeholder : m_sceneDefinition.visualPlaceholders) {
            const bool isVehicleBoundary = placeholder.role == "vehicle-yard-bound"
                || placeholder.role == "vehicle-yard-road-opening-post"
                || placeholder.role == "dock-road-bound"
                || placeholder.role == "dock-road-curb";
            if (!isVehicleBoundary) {
                continue;
            }

            engine::physics::BoxColliderDesc boundary;
            boundary.name = placeholder.id;
            boundary.center = placeholder.center;
            boundary.halfExtents = placeholder.halfExtents;
            m_vehiclePhysicsWorld->addStaticBox(boundary);
        }
    } else {
        engine::physics::BoxColliderDesc westRail;
        westRail.name = "vehicle-yard-west-rail";
        westRail.center = ServiceYardWestRailCenter;
        westRail.halfExtents = ServiceYardRailHalfExtents;
        m_vehiclePhysicsWorld->addStaticBox(westRail);

        engine::physics::BoxColliderDesc eastNorthPost = westRail;
        eastNorthPost.name = "vehicle-yard-east-entry-north-post";
        eastNorthPost.center = ServiceYardEastEntryPostNorthCenter;
        eastNorthPost.halfExtents = ServiceYardEastEntryPostHalfExtents;
        m_vehiclePhysicsWorld->addStaticBox(eastNorthPost);

        engine::physics::BoxColliderDesc eastSouthPost = eastNorthPost;
        eastSouthPost.name = "vehicle-yard-east-entry-south-post";
        eastSouthPost.center = ServiceYardEastEntryPostSouthCenter;
        m_vehiclePhysicsWorld->addStaticBox(eastSouthPost);

        engine::physics::BoxColliderDesc dockRoadSouthRail;
        dockRoadSouthRail.name = "dock-road-south-rail";
        dockRoadSouthRail.center = DockRoadSouthRailCenter;
        dockRoadSouthRail.halfExtents = DockRoadRailHalfExtents;
        m_vehiclePhysicsWorld->addStaticBox(dockRoadSouthRail);

        engine::physics::BoxColliderDesc dockRoadNorthCurb = dockRoadSouthRail;
        dockRoadNorthCurb.name = "dock-road-north-curb";
        dockRoadNorthCurb.center = DockRoadNorthCurbCenter;
        dockRoadNorthCurb.halfExtents = {DockRoadRailHalfExtents.x, 0.18f, 0.08f};
        m_vehiclePhysicsWorld->addStaticBox(dockRoadNorthCurb);

        engine::physics::BoxColliderDesc backStop;
        backStop.name = "vehicle-yard-back-stop";
        backStop.center = ServiceYardBackStopCenter;
        backStop.halfExtents = ServiceYardBackStopHalfExtents;
        m_vehiclePhysicsWorld->addStaticBox(backStop);
    }
}

bool SandboxLayer::isVehicleExitPositionClear(engine::Vec3 position) const
{
    if (!m_vehicleAvailable) {
        return false;
    }

    const VehicleControllerSettings& vehicleSettings = m_vehicle.settings();
    if (position.x < vehicleSettings.boundsMinX || position.x > vehicleSettings.boundsMaxX
        || position.z < vehicleSettings.boundsMinZ || position.z > vehicleSettings.boundsMaxZ) {
        return false;
    }

    for (const StaticCollider& collider : m_scene.world().colliders()) {
        if (!collider.blocksPlayer) {
            continue;
        }
        if (m_scene.world().playerOverlapsCollider(position, m_player.settings().radius, m_player.settings().height, collider)) {
            return false;
        }
    }

    return true;
}

void SandboxLayer::applyCameraSettingsForMode(bool vehicleMode)
{
    if (m_cameraInVehicleMode == vehicleMode) {
        return;
    }

    m_camera.setSettings(vehicleMode ? m_vehicleCameraSettings : m_onFootCameraSettings);
    m_cameraInVehicleMode = vehicleMode;
}

void SandboxLayer::loadStaticMeshAssets()
{
    m_staticMeshAssets.clear();

    std::vector<SceneMeshAssetDefinition> assetsToLoad;
    if (m_sceneDefinitionLoaded) {
        assetsToLoad = m_sceneDefinition.meshAssets;
    }
    if (assetsToLoad.empty()) {
        SceneMeshAssetDefinition fallback;
        fallback.id = "unit-box-mesh";
        fallback.path = "assets/models/unit_box.gltf";
        assetsToLoad.push_back(fallback);
    }

    for (const SceneMeshAssetDefinition& asset : assetsToLoad) {
        if (asset.id.empty()) {
            continue;
        }

        const std::filesystem::path meshPath = ResolveProjectPath(asset.path);
        const engine::StaticMeshLoadResult loadedMesh = engine::LoadStaticMeshFromGltf(meshPath);
        if (!loadedMesh.ok()) {
            engine::Logger::warning("Static mesh load failed for " + asset.id + ": " + loadedMesh.error);
            continue;
        }

        engine::StaticMeshAsset mesh = loadedMesh.mesh;
        mesh.id = asset.id;
        m_staticMeshAssets[asset.id] = std::move(mesh);
        engine::Logger::info("Loaded static mesh asset: " + asset.id + " from " + meshPath.string());
    }
}
