#include "SandboxLayer.h"

#include "engine/core/Logger.h"
#include "engine/math/Math.h"
#include "game/FerryOfficeData.h"

#include <iomanip>
#include <sstream>

void SandboxLayer::onAttach()
{
    engine::Logger::info("Sandbox layer attached.");
    m_player.setWorld(&m_scene.world());
    updateDebugText();
}

void SandboxLayer::onUpdate(double deltaSeconds, const engine::InputState& input)
{
    ++m_frameIndex;
    m_worldStateChangedThisFrame = false;
    const float dt = static_cast<float>(deltaSeconds);
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
    m_interactPressedThisFrame = input.interactPressed;
    const InteractionResult interaction = m_scene.interactions().interact(input);
    if (interaction.triggered) {
        m_lastInteractionText = interaction.message;
        engine::Logger::info("Interaction: " + interaction.message);
        recordWorldStateChange(m_scene.applyInteractionResult(interaction));
    }

    CameraTarget target;
    target.position = m_player.state().position;
    target.yawRadians = m_player.state().facingYawRadians;
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
    renderer.drawDebugLine({-12.0f, m_scene.world().floorHeight(), -12.0f}, {12.0f, m_scene.world().floorHeight(), -12.0f}, {0.35f, 0.9f, 0.55f, 1.0f});
    renderer.drawDebugLine({12.0f, m_scene.world().floorHeight(), -12.0f}, {12.0f, m_scene.world().floorHeight(), 12.0f}, {0.35f, 0.9f, 0.55f, 1.0f});
    renderer.drawDebugLine({12.0f, m_scene.world().floorHeight(), 12.0f}, {-12.0f, m_scene.world().floorHeight(), 12.0f}, {0.35f, 0.9f, 0.55f, 1.0f});
    renderer.drawDebugLine({-12.0f, m_scene.world().floorHeight(), 12.0f}, {-12.0f, m_scene.world().floorHeight(), -12.0f}, {0.35f, 0.9f, 0.55f, 1.0f});
    for (const StaticCollider& collider : m_scene.world().colliders()) {
        engine::Color colliderColor = collider.blocksPlayer
            ? engine::Color {0.9f, 0.72f, 0.28f, 1.0f}
            : engine::Color {0.25f, 1.0f, 0.35f, 1.0f};
        if (collider.name == FerryOffice::Names::ServiceGateCollider) {
            colliderColor = m_scene.worldState().isFlagSet(WorldFlag::RouteOpened)
                ? engine::Color {0.25f, 1.0f, 0.35f, 1.0f}
                : engine::Color {1.0f, 0.25f, 0.2f, 1.0f};
        }
        renderer.drawDebugBox(collider.bounds.center, collider.bounds.halfExtents, colliderColor);
    }
    drawWorldStateDebug(renderer);
    drawSliceDebug(renderer);

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
    drawTraversalDebug(renderer);
    drawInteractionDebug(renderer);
    renderer.drawDebugBox(m_camera.state().target, {0.08f, 0.08f, 0.08f}, {1.0f, 0.25f, 0.7f, 1.0f});
    renderer.drawDebugText(m_debugText);
}

void SandboxLayer::onDetach()
{
    engine::Logger::info("Sandbox layer detached.");
}

std::string SandboxLayer::debugText() const
{
    return m_debugText;
}

void SandboxLayer::updateDebugText()
{
    const PlayerState& player = m_player.state();
    const ThirdPersonCameraState& camera = m_camera.state();
    const InteractionFocus& focus = m_scene.interactions().focus();
    const TraversalFocus& traversalFocus = m_scene.traversal().focus();

    std::ostringstream output;
    output << std::fixed << std::setprecision(2)
           << "player=(" << player.position.x << "," << player.position.y << "," << player.position.z << ") "
           << "speed=" << player.horizontalSpeed << " "
           << (player.sprinting ? "sprint" : "walk") << " "
           << (player.grounded ? "grounded" : "air") << " "
           << "hits=" << player.lastCollisionHitCount << " "
           << "colliders=" << m_scene.world().colliders().size() << " "
           << "interactables=" << m_scene.interactions().interactableCount() << " "
           << "camera yaw=" << engine::Degrees(camera.yawRadians)
           << " pitch=" << engine::Degrees(camera.pitchRadians)
           << " dist=" << camera.distance << "\n"
           << "objective=\"" << m_scene.currentObjectiveText() << "\" "
           << "readyForExit=" << (m_scene.isSliceReadyForExit() ? "yes" : "no") << " "
           << "sliceComplete=" << (m_scene.isSliceComplete() ? "yes" : "no") << "\n"
           << "interactPressed=" << (m_interactPressedThisFrame ? "yes" : "no") << " "
           << "worldChanged=" << (m_worldStateChangedThisFrame ? "yes" : "no") << " "
           << "traversal=" << (player.traversalMode == PlayerTraversalMode::Traversing ? "active" : "normal") << " "
           << "travProgress=" << player.traversalProgress << " "
           << "travStart=" << (player.traversalUsesCurrentPlayerPositionStart ? "current" : "fixed") << " "
           << "travLanded=" << (player.traversalLandedThisFrame ? "yes" : "no") << " "
           << "travFocus=" << (traversalFocus.hasFocus ? traversalFocus.name : "none") << " "
           << "travPressed=" << (m_traversalPressedThisFrame ? "yes" : "no") << " "
           << "focus=" << (focus.hasFocus ? focus.name : "none") << " ";
    if (player.traversalMode == PlayerTraversalMode::Traversing) {
        output << "travFrom=(" << player.traversalStartPosition.x << "," << player.traversalStartPosition.z << ") "
               << "travTo=(" << player.traversalTargetPosition.x << "," << player.traversalTargetPosition.z << ") ";
    }
    if (traversalFocus.hasFocus && player.traversalMode == PlayerTraversalMode::Normal) {
        output << "travPrompt=\"" << traversalFocus.prompt << "\" ";
    }
    if (focus.hasFocus) {
        output << "prompt=\"Press E: " << focus.prompt << "\" ";
    }
    output << "\nlastInteraction=\"" << m_lastInteractionText << "\" "
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
        if (interactable.type == InteractableType::Pickup) {
            color = {0.35f, 1.0f, 0.45f, 1.0f};
        } else if (interactable.type == InteractableType::Toggle) {
            color = interactable.toggled
                ? engine::Color {1.0f, 0.82f, 0.25f, 1.0f}
                : engine::Color {1.0f, 0.55f, 0.25f, 1.0f};
        }
        if (interactable.name == FerryOffice::Names::WallButton && m_scene.worldState().isFlagSet(WorldFlag::RouteOpened)) {
            color = {0.25f, 1.0f, 0.35f, 1.0f};
        }
        if (interactable.name == FerryOffice::Names::MaintenanceBox && m_scene.worldState().isFlagSet(WorldFlag::PowerRestored)) {
            color = {0.25f, 1.0f, 0.85f, 1.0f};
        }

        if (!interactable.enabled || interactable.consumed) {
            color = {0.35f, 0.35f, 0.35f, 1.0f};
        }
        if (isFocused) {
            color = {1.0f, 1.0f, 0.25f, 1.0f};
        }

        const float markerSize = isFocused ? 0.22f : 0.16f;
        renderer.drawDebugBox(interactable.position, {markerSize, markerSize, markerSize}, color);
        renderer.drawDebugBox(
            {interactable.position.x, m_scene.world().floorHeight() + 0.03f, interactable.position.z},
            {interactable.radius, 0.03f, interactable.radius},
            color);
        renderer.drawDebugLine(interactable.position,
            interactable.position + engine::Vec3 {0.0f, isFocused ? 1.2f : 0.75f, 0.0f},
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
        renderer.drawDebugBox(gate->bounds.center, gate->bounds.halfExtents + engine::Vec3 {0.04f, 0.04f, 0.04f}, gateColor);
    }

    const bool powerRestored = m_scene.worldState().isFlagSet(WorldFlag::PowerRestored);
    const engine::Color powerColor = powerRestored
        ? engine::Color {0.25f, 1.0f, 0.85f, 1.0f}
        : engine::Color {0.45f, 0.45f, 0.55f, 1.0f};
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

    renderer.drawDebugBox(dockStart, {0.22f, 0.08f, 0.22f}, dockColor);
    renderer.drawDebugLine(dockStart, dockStart + engine::Vec3 {0.0f, 1.0f, 0.0f}, dockColor);
    renderer.drawDebugBox(officeMarker, {0.35f, 0.35f, 0.35f}, officeColor);
    renderer.drawDebugLine({FerryOffice::Positions::FerryManifest.x, floor + 0.06f, FerryOffice::Positions::FerryManifest.z},
        {FerryOffice::Positions::ExitMarker.x, floor + 0.06f, FerryOffice::Positions::ExitMarker.z},
        officeColor);
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

        renderer.drawDebugBox(affordance.startPosition + engine::Vec3 {0.0f, 0.2f, 0.0f}, {0.18f, 0.18f, 0.18f}, color);
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
        renderer.drawDebugBox(activeStart, {0.14f, 0.14f, 0.14f}, {1.0f, 0.2f, 1.0f, 1.0f});
        renderer.drawDebugBox(activeEnd, {0.14f, 0.14f, 0.14f}, {0.2f, 1.0f, 1.0f, 1.0f});
        renderer.drawDebugLine(activeStart, player.position + engine::Vec3 {0.0f, 0.55f, 0.0f}, {1.0f, 0.2f, 1.0f, 1.0f});
        renderer.drawDebugLine(player.position + engine::Vec3 {0.0f, 0.55f, 0.0f}, activeEnd, {0.2f, 1.0f, 1.0f, 1.0f});
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
