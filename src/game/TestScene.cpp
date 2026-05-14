#include "game/TestScene.h"

#include <sstream>

namespace {

constexpr const char* ServiceGateColliderName = "service-gate";
constexpr const char* FerryManifestName = "Ferry Manifest";
constexpr const char* WallButtonName = "Wall Button";
constexpr const char* MaintenanceBoxName = "Maintenance Box";
constexpr const char* ExitMarkerName = "Exit Summary Marker";
constexpr const char* ServiceVaultName = "Service Barrier Vault";

} // namespace

TestScene::TestScene()
{
    m_world.buildFerryOfficePrototypeLayout();

    Interactable pickup;
    pickup.name = FerryManifestName;
    pickup.prompt = "Collect Ferry Manifest";
    pickup.position = {0.0f, 0.45f, 1.35f};
    pickup.radius = 1.8f;
    pickup.type = InteractableType::Pickup;
    pickup.oneShot = true;
    pickup.message = "Collected Ferry Manifest.";
    m_interactions.addInteractable(pickup);

    Interactable toggle;
    toggle.name = WallButtonName;
    toggle.prompt = "Open Service Gate";
    toggle.position = {-0.9f, 0.7f, 2.15f};
    toggle.radius = 1.7f;
    toggle.type = InteractableType::Info;
    toggle.message = "Service gate opened.";
    m_interactions.addInteractable(toggle);

    Interactable info;
    info.name = "Ferry Office Notice";
    info.prompt = "Read Ferry Office Notice";
    info.position = {-1.8f, 0.55f, 3.2f};
    info.radius = 1.7f;
    info.type = InteractableType::Info;
    info.message = "Notice: restore local power before opening the service route.";
    m_interactions.addInteractable(info);

    Interactable maintenanceBox;
    maintenanceBox.name = MaintenanceBoxName;
    maintenanceBox.prompt = "Inspect Maintenance Box";
    maintenanceBox.position = {2.8f, 0.65f, 1.9f};
    maintenanceBox.radius = 1.4f;
    maintenanceBox.type = InteractableType::Info;
    maintenanceBox.message = "Maintenance box inspected: local power restored.";
    m_interactions.addInteractable(maintenanceBox);

    Interactable exitMarker;
    exitMarker.name = ExitMarkerName;
    exitMarker.prompt = "Review Ferry Office Summary";
    exitMarker.position = {0.0f, 0.55f, 4.55f};
    exitMarker.radius = 1.8f;
    exitMarker.type = InteractableType::Info;
    exitMarker.message = "Ferry Office summary marker reached.";
    m_interactions.addInteractable(exitMarker);

    TraversalAffordance serviceVault;
    serviceVault.name = ServiceVaultName;
    serviceVault.prompt = "Press Space: Vault Service Barrier";
    serviceVault.type = TraversalType::Vault;
    serviceVault.startPosition = {2.8f, 0.0f, -0.35f};
    serviceVault.endPosition = {2.8f, 0.0f, 1.45f};
    serviceVault.focusRadius = 1.25f;
    serviceVault.requiredFacingDirection = {0.0f, 0.0f, 1.0f};
    serviceVault.requiredFacingDot = 0.15f;
    serviceVault.durationSeconds = 0.55f;
    m_traversal.addAffordance(serviceVault);

    syncRouteGateCollider();
}

TestWorld& TestScene::world()
{
    return m_world;
}

const TestWorld& TestScene::world() const
{
    return m_world;
}

InteractionSystem& TestScene::interactions()
{
    return m_interactions;
}

const InteractionSystem& TestScene::interactions() const
{
    return m_interactions;
}

TraversalSystem& TestScene::traversal()
{
    return m_traversal;
}

const TraversalSystem& TestScene::traversal() const
{
    return m_traversal;
}

WorldState& TestScene::worldState()
{
    return m_worldState;
}

const WorldState& TestScene::worldState() const
{
    return m_worldState;
}

bool TestScene::applyInteractionResult(const InteractionResult& result)
{
    if (!result.triggered) {
        return false;
    }

    bool changed = false;
    if (result.name == FerryManifestName || result.name == "Test Pickup") {
        changed |= m_worldState.setFlag(WorldFlag::ManifestCollected, true, result.name);
    } else if (result.name == MaintenanceBoxName) {
        changed |= m_worldState.setFlag(WorldFlag::MaintenanceBoxInspected, true, result.name);
        changed |= m_worldState.setFlag(WorldFlag::PowerRestored, true, result.name);
    } else if (result.name == WallButtonName) {
        changed |= m_worldState.setFlag(WorldFlag::RouteOpened, true, result.name);
        syncRouteGateCollider();
    } else if (result.name == ExitMarkerName) {
        changed |= recordExitReached();
    }

    return changed;
}

bool TestScene::recordServiceRouteUsed()
{
    return m_worldState.setFlag(WorldFlag::ServiceRouteUsed, true, "Service Barrier Vault");
}

bool TestScene::recordExitReached()
{
    if (!isSliceReadyForExit()) {
        return false;
    }

    return m_worldState.setFlag(WorldFlag::ExitReached, true, ExitMarkerName);
}

bool TestScene::isSliceReadyForExit() const
{
    return m_worldState.isFlagSet(WorldFlag::ManifestCollected)
        && m_worldState.isFlagSet(WorldFlag::ServiceRouteUsed)
        && m_worldState.isFlagSet(WorldFlag::MaintenanceBoxInspected)
        && m_worldState.isFlagSet(WorldFlag::PowerRestored)
        && m_worldState.isFlagSet(WorldFlag::RouteOpened);
}

bool TestScene::isSliceComplete() const
{
    return isSliceReadyForExit() && m_worldState.isFlagSet(WorldFlag::ExitReached);
}

bool TestScene::isServiceGateBlocking() const
{
    const StaticCollider* gate = m_world.colliderByName(ServiceGateColliderName);
    return gate != nullptr && gate->blocksPlayer;
}

std::string TestScene::currentObjectiveText() const
{
    if (!m_worldState.isFlagSet(WorldFlag::ManifestCollected)) {
        return "Collect the Ferry Manifest near the dock office.";
    }
    if (!m_worldState.isFlagSet(WorldFlag::ServiceRouteUsed)) {
        return "Use the Service Barrier Vault to reach the maintenance side.";
    }
    if (!m_worldState.isFlagSet(WorldFlag::MaintenanceBoxInspected) || !m_worldState.isFlagSet(WorldFlag::PowerRestored)) {
        return "Inspect the Maintenance Box to restore local power.";
    }
    if (!m_worldState.isFlagSet(WorldFlag::RouteOpened)) {
        return "Use the Wall Button to open the service gate.";
    }
    if (!m_worldState.isFlagSet(WorldFlag::ExitReached)) {
        return "Reach the Exit Summary Marker.";
    }

    return "Ferry Office micro-slice complete.";
}

std::string TestScene::completionSummary() const
{
    std::ostringstream output;
    output << "complete=" << (isSliceComplete() ? "true" : "false")
           << " readyForExit=" << (isSliceReadyForExit() ? "true" : "false")
           << " manifest=" << (m_worldState.isFlagSet(WorldFlag::ManifestCollected) ? "true" : "false")
           << " serviceRoute=" << (m_worldState.isFlagSet(WorldFlag::ServiceRouteUsed) ? "true" : "false")
           << " maintenance=" << (m_worldState.isFlagSet(WorldFlag::MaintenanceBoxInspected) ? "true" : "false")
           << " power=" << (m_worldState.isFlagSet(WorldFlag::PowerRestored) ? "true" : "false")
           << " routeOpen=" << (m_worldState.isFlagSet(WorldFlag::RouteOpened) ? "true" : "false")
           << " exit=" << (m_worldState.isFlagSet(WorldFlag::ExitReached) ? "true" : "false");
    return output.str();
}

void TestScene::syncRouteGateCollider()
{
    m_world.setColliderBlocksPlayer(ServiceGateColliderName, !m_worldState.isFlagSet(WorldFlag::RouteOpened));
}
