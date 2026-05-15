#include "SandboxLayer.h"

#include "engine/core/Logger.h"
#include "engine/math/Math.h"
#include "game/FerryOfficeData.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace {

const engine::Vec3 VehicleSpawnPosition {6.2f, 0.0f, -2.2f};
const float VehicleSpawnYawRadians = engine::Radians(88.0f);

engine::Color ScaleColor(engine::Color color, float scale)
{
    return {color.r * scale, color.g * scale, color.b * scale, color.a};
}

bool NameContains(const StaticCollider& collider, const char* token)
{
    return collider.name.find(token) != std::string::npos;
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
    renderer.drawDebugSolidBox({6.25f, floor - 0.045f, -2.1f}, {2.85f, 0.045f, 2.65f}, {0.16f, 0.18f, 0.17f, 1.0f});
    renderer.drawDebugBox({6.25f, floor + 0.02f, -2.1f}, {2.85f, 0.02f, 2.65f}, {0.50f, 0.72f, 0.62f, 1.0f});
    renderer.drawDebugLine({3.9f, floor + 0.04f, -2.1f}, {8.6f, floor + 0.04f, -2.1f}, {0.85f, 0.78f, 0.45f, 1.0f});
    renderer.drawDebugLine({6.25f, floor + 0.04f, -4.35f}, {6.25f, floor + 0.04f, 0.15f}, {0.85f, 0.78f, 0.45f, 1.0f});
    renderer.drawDebugSolidBox({3.45f, floor + 0.35f, -2.1f}, {0.12f, 0.35f, 2.55f}, {0.42f, 0.30f, 0.13f, 1.0f});
    renderer.drawDebugSolidBox({9.05f, floor + 0.35f, -2.1f}, {0.12f, 0.35f, 2.55f}, {0.42f, 0.30f, 0.13f, 1.0f});
    renderer.drawDebugSolidBox({6.25f, floor + 0.18f, -4.7f}, {2.25f, 0.18f, 0.12f}, {0.42f, 0.30f, 0.13f, 1.0f});
    renderer.drawDebugSolidBox({8.15f, floor + 0.45f, 0.45f}, {0.55f, 0.45f, 0.45f}, {0.27f, 0.30f, 0.22f, 1.0f});
}

void DrawMeshInstance(engine::IRenderer& renderer, const engine::StaticMeshAsset& mesh, const engine::StaticMeshInstance& instance)
{
    const std::vector<engine::Vec3> triangles = engine::BuildFlatTriangleList(mesh, instance);
    renderer.drawDebugFlatTriangles(triangles, instance.tint);
}

} // namespace

void SandboxLayer::onAttach()
{
    engine::Logger::info("Sandbox layer attached.");
    m_player.setWorld(&m_scene.world());
    m_onFootCameraSettings = m_camera.settings();
    m_vehicleCameraSettings = m_onFootCameraSettings;
    m_vehicleCameraSettings.distance = 8.25f;
    m_vehicleCameraSettings.heightOffset = 2.35f;
    m_vehicleCameraSettings.smoothing = 9.5f;

    VehicleControllerSettings vehicleSettings;
    vehicleSettings.boundsMinX = 3.55f;
    vehicleSettings.boundsMaxX = 8.95f;
    vehicleSettings.boundsMinZ = -4.55f;
    vehicleSettings.boundsMaxZ = 0.35f;
    m_vehicle.setSettings(vehicleSettings);
    m_vehicle.setPosition(VehicleSpawnPosition);
    m_vehicle.setYawRadians(VehicleSpawnYawRadians);
    setupVehiclePhysicsWorld();
    loadStaticMeshAssets();
    updateDebugText();
}

void SandboxLayer::onUpdate(double deltaSeconds, const engine::InputState& input)
{
    ++m_frameIndex;
    m_worldStateChangedThisFrame = false;
    const float dt = static_cast<float>(deltaSeconds);
    m_interactPressedThisFrame = input.interactPressed;
    m_traversalPressedThisFrame = false;
    m_vehicle.beginFrame();

    if (m_vehicle.state().occupied) {
        const engine::Vec3 exitPosition = m_vehicle.exitPosition();
        const bool exitClear = isVehicleExitPositionClear(exitPosition);
        if (m_vehicle.tryExit(input, exitClear)) {
            m_player.setPosition(exitPosition);
            m_lastVehicleText = "Exited service yard vehicle.";
            engine::Logger::info("Vehicle: " + m_lastVehicleText);
        }
    }

    if (m_vehicle.state().occupied) {
        m_scene.traversal().updateFocus({999.0f, 0.0f, 999.0f}, {0.0f, 0.0f, 1.0f});
        m_scene.interactions().updateFocus({999.0f, 0.0f, 999.0f}, {0.0f, 0.0f, 1.0f});
        m_vehicle.updateDriving(dt, input);
    } else {
        const engine::Vec3 traversalFacing = engine::ForwardFromYaw(m_player.state().facingYawRadians);
        m_scene.traversal().updateFocus(m_player.state().position, traversalFacing);
        const TraversalActivation traversalActivation = m_scene.traversal().activationFromInput(input);
        m_traversalPressedThisFrame = input.jumpPressed && m_scene.traversal().focus().hasFocus;

        m_player.update(dt, input, m_camera.state().yawRadians, &traversalActivation);
        if (m_player.state().traversalLandedThisFrame) {
            recordWorldStateChange(m_scene.recordServiceRouteUsed());
        }

        const engine::Vec3 playerFacing = engine::ForwardFromYaw(m_player.state().facingYawRadians);
        m_scene.interactions().updateFocus(m_player.state().position, playerFacing);
        m_vehicle.updateFocus(m_player.state().position, playerFacing);
        const InteractionFocus& focus = m_scene.interactions().focus();
        const bool vehicleEntered = m_player.state().traversalMode == PlayerTraversalMode::Normal
            && !focus.hasFocus
            && m_vehicle.tryEnter(input);

        if (vehicleEntered) {
            m_lastVehicleText = "Entered service yard vehicle.";
            engine::Logger::info("Vehicle: " + m_lastVehicleText);
            m_vehicle.updateDriving(dt, input);
        } else {
            const InteractionResult interaction = m_scene.interactions().interact(input);
            if (interaction.triggered) {
                m_lastInteractionText = interaction.message;
                engine::Logger::info("Interaction: " + interaction.message);
                recordWorldStateChange(m_scene.applyInteractionResult(interaction));
            }
        }
    }

    if (m_vehiclePhysicsWorld) {
        m_vehiclePhysicsWorld->step(dt);
    }

    const bool vehicleMode = m_vehicle.state().occupied;
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
    renderer.setDebugCamera(m_camera.debugCamera());
    renderer.drawDebugGridAndAxes();
    DrawFerryOfficeMoodBase(renderer, m_scene.world().floorHeight());
    DrawVehicleServiceYardBase(renderer, m_scene.world().floorHeight());
    drawStaticMeshDebug(renderer);
    renderer.drawDebugLine({-12.0f, m_scene.world().floorHeight(), -12.0f}, {12.0f, m_scene.world().floorHeight(), -12.0f}, {0.35f, 0.9f, 0.55f, 1.0f});
    renderer.drawDebugLine({12.0f, m_scene.world().floorHeight(), -12.0f}, {12.0f, m_scene.world().floorHeight(), 12.0f}, {0.35f, 0.9f, 0.55f, 1.0f});
    renderer.drawDebugLine({12.0f, m_scene.world().floorHeight(), 12.0f}, {-12.0f, m_scene.world().floorHeight(), 12.0f}, {0.35f, 0.9f, 0.55f, 1.0f});
    renderer.drawDebugLine({-12.0f, m_scene.world().floorHeight(), 12.0f}, {-12.0f, m_scene.world().floorHeight(), -12.0f}, {0.35f, 0.9f, 0.55f, 1.0f});
    const bool routeOpened = m_scene.worldState().isFlagSet(WorldFlag::RouteOpened);
    for (const StaticCollider& collider : m_scene.world().colliders()) {
        renderer.drawDebugSolidBox(collider.bounds.center, collider.bounds.halfExtents, ColliderSolidColor(collider, routeOpened));
        renderer.drawDebugBox(collider.bounds.center, collider.bounds.halfExtents, ColliderWireColor(collider, routeOpened));
    }
    drawWorldStateDebug(renderer);
    drawSliceDebug(renderer);

    const PlayerState& player = m_player.state();
    renderer.drawDebugSolidBox(player.position + engine::Vec3 {0.0f, m_player.settings().height * 0.5f, 0.0f},
        {m_player.settings().radius, m_player.settings().height * 0.5f, m_player.settings().radius},
        {0.10f, 0.28f, 0.65f, 1.0f});
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
    drawTraversalDebug(renderer);
    drawInteractionDebug(renderer);
    drawVehicleDebug(renderer);
    renderer.drawDebugBox(m_camera.state().target, {0.08f, 0.08f, 0.08f}, {1.0f, 0.25f, 0.7f, 1.0f});
    renderer.drawDebugText(m_debugText);
}

void SandboxLayer::onDetach()
{
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
    const PlayerState& player = m_player.state();
    const VehicleState& vehicle = m_vehicle.state();
    const VehicleFocus& vehicleFocus = m_vehicle.focus();
    const ThirdPersonCameraState& camera = m_camera.state();
    const InteractionFocus& focus = m_scene.interactions().focus();
    const TraversalFocus& traversalFocus = m_scene.traversal().focus();

    std::ostringstream output;
    output << std::fixed << std::setprecision(2)
           << "objective=\"" << m_scene.currentObjectiveText() << "\" "
           << "readyForExit=" << (m_scene.isSliceReadyForExit() ? "yes" : "no") << " "
           << "sliceComplete=" << (m_scene.isSliceComplete() ? "yes" : "no") << "\n"
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
           << "vehicle=(" << vehicle.position.x << "," << vehicle.position.y << "," << vehicle.position.z << ") "
           << (vehicle.occupied ? "occupied" : "empty") << " "
           << "speed=" << vehicle.speed << " "
           << "throttle=" << vehicle.throttle << " "
           << "brake=" << vehicle.brake << " "
           << "steer=" << vehicle.steer << " "
           << "focus=" << (vehicleFocus.canEnter ? "yes" : "no") << " "
           << "exitBlocked=" << (vehicle.exitBlockedThisFrame ? "yes" : "no") << " "
           << "boundsHit=" << (vehicle.hitBoundsThisFrame ? "yes" : "no") << " "
           << "physics=" << m_vehiclePhysicsBackendText << "\n"
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
    m_debugText = output.str();
}

void SandboxLayer::drawInteractionDebug(engine::IRenderer& renderer)
{
    const InteractionFocus& focus = m_scene.interactions().focus();
    for (const Interactable& interactable : m_scene.interactions().interactables()) {
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

void SandboxLayer::drawSliceDebug(engine::IRenderer& renderer)
{
    const float floor = m_scene.world().floorHeight();
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
    const engine::Vec3 exitMarker {
        FerryOffice::Positions::ExitMarker.x,
        floor + FerryOffice::Positions::ExitMarker.y,
        FerryOffice::Positions::ExitMarker.z,
    };
    const engine::Color dockColor {0.25f, 0.65f, 1.0f, 1.0f};
    const engine::Color officeColor {0.95f, 0.95f, 0.8f, 1.0f};
    const engine::Color exitColor = m_scene.isSliceComplete()
        ? engine::Color {0.35f, 1.0f, 0.35f, 1.0f}
        : (m_scene.isSliceReadyForExit()
                  ? engine::Color {1.0f, 0.9f, 0.25f, 1.0f}
                  : engine::Color {0.45f, 0.45f, 0.55f, 1.0f});

    renderer.drawDebugSolidBox(dockStart, {0.22f, 0.08f, 0.22f}, ScaleColor(dockColor, 0.45f));
    renderer.drawDebugBox(dockStart, {0.22f, 0.08f, 0.22f}, dockColor);
    renderer.drawDebugLine(dockStart, dockStart + engine::Vec3 {0.0f, 1.0f, 0.0f}, dockColor);
    renderer.drawDebugSolidBox(officeMarker, {0.35f, 0.35f, 0.35f}, ScaleColor(officeColor, 0.45f));
    renderer.drawDebugBox(officeMarker, {0.35f, 0.35f, 0.35f}, officeColor);
    const engine::Color routeColor {0.70f, 0.92f, 1.0f, 1.0f};
    renderer.drawDebugLine({FerryOffice::Positions::FerryManifest.x, floor + 0.08f, FerryOffice::Positions::FerryManifest.z},
        {FerryOffice::Positions::ServiceVaultStart.x, floor + 0.08f, FerryOffice::Positions::ServiceVaultStart.z},
        routeColor);
    renderer.drawDebugLine({FerryOffice::Positions::ServiceVaultEnd.x, floor + 0.08f, FerryOffice::Positions::ServiceVaultEnd.z},
        {FerryOffice::Positions::MaintenanceBox.x, floor + 0.08f, FerryOffice::Positions::MaintenanceBox.z},
        routeColor);
    renderer.drawDebugLine({FerryOffice::Positions::MaintenanceBox.x, floor + 0.08f, FerryOffice::Positions::MaintenanceBox.z},
        {FerryOffice::Positions::WallButton.x, floor + 0.08f, FerryOffice::Positions::WallButton.z},
        routeColor);
    renderer.drawDebugLine({FerryOffice::Positions::WallButton.x, floor + 0.08f, FerryOffice::Positions::WallButton.z},
        {FerryOffice::Positions::ExitMarker.x, floor + 0.08f, FerryOffice::Positions::ExitMarker.z},
        routeColor);
    renderer.drawDebugSolidBox(exitMarker, {0.28f, 0.28f, 0.28f}, ScaleColor(exitColor, 0.45f));
    renderer.drawDebugBox(exitMarker, {0.28f, 0.28f, 0.28f}, exitColor);
    renderer.drawDebugBox({exitMarker.x, floor + 0.03f, exitMarker.z}, {FerryOffice::Radii::ExitMarker, 0.03f, FerryOffice::Radii::ExitMarker}, exitColor);
}

void SandboxLayer::drawTraversalDebug(engine::IRenderer& renderer)
{
    const TraversalFocus& focus = m_scene.traversal().focus();
    const PlayerState& player = m_player.state();
    for (const TraversalAffordance& affordance : m_scene.traversal().affordances()) {
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
    const engine::Vec3 bodyCenter = vehicle.position + engine::Vec3 {0.0f, 0.42f, 0.0f};
    renderer.drawDebugSolidBox(bodyCenter, {0.58f, 0.34f, 0.92f}, ScaleColor(vehicleColor, 0.50f));
    renderer.drawDebugBox(bodyCenter, {0.58f, 0.34f, 0.92f}, vehicleColor);
    renderer.drawDebugSolidBox(vehicle.position + engine::Vec3 {0.0f, 0.86f, 0.0f}, {0.42f, 0.20f, 0.46f}, ScaleColor(vehicleColor, 0.38f));
    renderer.drawDebugBox(vehicle.position + engine::Vec3 {0.0f, 0.86f, 0.0f}, {0.42f, 0.20f, 0.46f}, vehicleColor);

    const engine::Vec3 forward = m_vehicle.forward();
    renderer.drawDebugLine(vehicle.position + engine::Vec3 {0.0f, 0.75f, 0.0f},
        vehicle.position + engine::Vec3 {0.0f, 0.75f, 0.0f} + forward * 1.65f,
        {1.0f, 0.95f, 0.35f, 1.0f});

    const engine::Color focusColor = focus.canEnter
        ? engine::Color {1.0f, 1.0f, 0.25f, 1.0f}
        : engine::Color {0.35f, 0.55f, 0.65f, 1.0f};
    renderer.drawDebugBox({vehicle.position.x, floor + 0.04f, vehicle.position.z}, {settings.enterRadius, 0.03f, settings.enterRadius}, focusColor);

    const engine::Vec3 exitPosition = m_vehicle.exitPosition();
    const engine::Color exitColor = vehicle.exitBlockedThisFrame
        ? engine::Color {1.0f, 0.25f, 0.20f, 1.0f}
        : engine::Color {0.30f, 1.0f, 0.65f, 1.0f};
    renderer.drawDebugSolidBox(exitPosition + engine::Vec3 {0.0f, 0.18f, 0.0f}, {0.16f, 0.16f, 0.16f}, ScaleColor(exitColor, 0.45f));
    renderer.drawDebugBox(exitPosition + engine::Vec3 {0.0f, 0.18f, 0.0f}, {0.16f, 0.16f, 0.16f}, exitColor);

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

    if (m_vehiclePhysicsWorld) {
        for (const engine::physics::PhysicsDebugLine& line : m_vehiclePhysicsWorld->debugLines()) {
            renderer.drawDebugLine(line.from, line.to, {line.color.x, line.color.y, line.color.z, 1.0f});
        }
    }
}

void SandboxLayer::drawStaticMeshDebug(engine::IRenderer& renderer)
{
    if (!m_unitBoxMeshLoaded) {
        return;
    }

    const bool routeOpened = m_scene.worldState().isFlagSet(WorldFlag::RouteOpened);
    engine::StaticMeshInstance roof;
    roof.assetId = "unit-box-mesh";
    roof.position = {0.0f, 1.68f, 5.18f};
    roof.scale = {5.9f, 0.24f, 1.1f};
    roof.tint = {0.44f, 0.24f, 0.15f, 1.0f};
    DrawMeshInstance(renderer, m_unitBoxMesh, roof);

    engine::StaticMeshInstance serviceGate;
    serviceGate.assetId = "unit-box-mesh";
    serviceGate.position = {0.0f, 0.75f, 2.35f};
    serviceGate.scale = {4.9f, 1.5f, 0.32f};
    serviceGate.tint = routeOpened
        ? engine::Color {0.12f, 0.36f, 0.20f, 1.0f}
        : engine::Color {0.42f, 0.12f, 0.08f, 1.0f};
    DrawMeshInstance(renderer, m_unitBoxMesh, serviceGate);

    engine::StaticMeshInstance maintenanceBox;
    maintenanceBox.assetId = "unit-box-mesh";
    maintenanceBox.position = {2.8f, 0.65f, 1.9f};
    maintenanceBox.scale = {0.48f, 0.48f, 0.48f};
    maintenanceBox.tint = m_scene.worldState().isFlagSet(WorldFlag::PowerRestored)
        ? engine::Color {0.18f, 0.74f, 0.62f, 1.0f}
        : engine::Color {0.11f, 0.40f, 0.36f, 1.0f};
    DrawMeshInstance(renderer, m_unitBoxMesh, maintenanceBox);

    engine::StaticMeshInstance vehicleBody;
    vehicleBody.assetId = "unit-box-mesh";
    vehicleBody.position = m_vehicle.state().position + engine::Vec3 {0.0f, 0.42f, 0.0f};
    vehicleBody.scale = {1.16f, 0.68f, 1.84f};
    vehicleBody.yawRadians = m_vehicle.state().yawRadians;
    vehicleBody.tint = m_vehicle.state().occupied
        ? engine::Color {0.18f, 0.58f, 0.95f, 1.0f}
        : engine::Color {0.62f, 0.66f, 0.48f, 1.0f};
    DrawMeshInstance(renderer, m_unitBoxMesh, vehicleBody);
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

void SandboxLayer::setupVehiclePhysicsWorld()
{
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

    engine::physics::BoxColliderDesc westRail;
    westRail.name = "vehicle-yard-west-rail";
    westRail.center = {3.45f, 0.35f, -2.1f};
    westRail.halfExtents = {0.12f, 0.35f, 2.55f};
    m_vehiclePhysicsWorld->addStaticBox(westRail);

    engine::physics::BoxColliderDesc eastRail = westRail;
    eastRail.name = "vehicle-yard-east-rail";
    eastRail.center.x = 9.05f;
    m_vehiclePhysicsWorld->addStaticBox(eastRail);

    engine::physics::DynamicBoxDesc vehicleProxy;
    vehicleProxy.name = "service-yard-vehicle-proxy";
    vehicleProxy.center = VehicleSpawnPosition + engine::Vec3 {0.0f, 0.42f, 0.0f};
    vehicleProxy.halfExtents = {0.58f, 0.34f, 0.92f};
    vehicleProxy.mass = 900.0f;
    m_vehiclePhysicsWorld->addDynamicBox(vehicleProxy);
}

bool SandboxLayer::isVehicleExitPositionClear(engine::Vec3 position) const
{
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
    const engine::StaticMeshLoadResult unitBox = engine::LoadStaticMeshFromGltf("assets/models/unit_box.gltf");
    if (!unitBox.ok()) {
        engine::Logger::warning("Static mesh load failed: " + unitBox.error);
        m_unitBoxMeshLoaded = false;
        return;
    }

    m_unitBoxMesh = unitBox.mesh;
    m_unitBoxMesh.id = "unit-box-mesh";
    m_unitBoxMeshLoaded = true;
    engine::Logger::info("Loaded static mesh asset: unit-box-mesh from assets/models/unit_box.gltf");
}
