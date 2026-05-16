#include "game/PrototypeScene.h"

#include "game/SceneDefinition.h"

#include <algorithm>
#include <sstream>
#include <utility>

PrototypeScene::PrototypeScene()
{
    buildFromFerryOfficeData();
}

PrototypeScene::PrototypeScene(const SceneDefinition& sceneDefinition)
{
    loadFromDefinition(sceneDefinition);
}

void PrototypeScene::buildFromFerryOfficeData()
{
    m_world.buildFerryOfficePrototypeLayout();

    Interactable pickup;
    pickup.name = FerryOffice::Names::FerryManifest;
    pickup.prompt = FerryOffice::Prompts::FerryManifest;
    pickup.position = FerryOffice::Positions::FerryManifest;
    pickup.radius = FerryOffice::Radii::FerryManifest;
    pickup.type = InteractableType::Pickup;
    pickup.oneShot = true;
    pickup.message = FerryOffice::Messages::FerryManifest;
    m_interactions.addInteractable(pickup);

    Interactable toggle;
    toggle.name = FerryOffice::Names::WallButton;
    toggle.prompt = FerryOffice::Prompts::WallButton;
    toggle.position = FerryOffice::Positions::WallButton;
    toggle.radius = FerryOffice::Radii::WallButton;
    toggle.type = InteractableType::Info;
    toggle.message = FerryOffice::Messages::WallButton;
    m_interactions.addInteractable(toggle);

    Interactable info;
    info.name = FerryOffice::Names::FerryOfficeNotice;
    info.prompt = FerryOffice::Prompts::FerryOfficeNotice;
    info.position = FerryOffice::Positions::FerryOfficeNotice;
    info.radius = FerryOffice::Radii::FerryOfficeNotice;
    info.type = InteractableType::Info;
    info.message = FerryOffice::Messages::FerryOfficeNotice;
    m_interactions.addInteractable(info);

    Interactable maintenanceBox;
    maintenanceBox.name = FerryOffice::Names::MaintenanceBox;
    maintenanceBox.prompt = FerryOffice::Prompts::MaintenanceBox;
    maintenanceBox.position = FerryOffice::Positions::MaintenanceBox;
    maintenanceBox.radius = FerryOffice::Radii::MaintenanceBox;
    maintenanceBox.type = InteractableType::Info;
    maintenanceBox.message = FerryOffice::Messages::MaintenanceBox;
    m_interactions.addInteractable(maintenanceBox);

    Interactable exitMarker;
    exitMarker.name = FerryOffice::Names::ExitMarker;
    exitMarker.prompt = FerryOffice::Prompts::ExitMarker;
    exitMarker.position = FerryOffice::Positions::ExitMarker;
    exitMarker.radius = FerryOffice::Radii::ExitMarker;
    exitMarker.type = InteractableType::Info;
    exitMarker.message = FerryOffice::Messages::ExitMarker;
    m_interactions.addInteractable(exitMarker);

    Interactable serviceRunMarker;
    serviceRunMarker.name = FerryOffice::Names::ServiceRunMarker;
    serviceRunMarker.prompt = FerryOffice::Prompts::ServiceRunMarker;
    serviceRunMarker.position = FerryOffice::Positions::ServiceRunMarker;
    serviceRunMarker.radius = FerryOffice::Radii::ServiceRunMarker;
    serviceRunMarker.type = InteractableType::Info;
    serviceRunMarker.message = FerryOffice::Messages::ServiceRunMarker;
    m_interactions.addInteractable(serviceRunMarker);

    Interactable dockRoadRelay;
    dockRoadRelay.name = FerryOffice::Names::DockRoadRelay;
    dockRoadRelay.prompt = FerryOffice::Prompts::DockRoadRelay;
    dockRoadRelay.position = FerryOffice::Positions::DockRoadRelay;
    dockRoadRelay.radius = FerryOffice::Radii::DockRoadRelay;
    dockRoadRelay.type = InteractableType::Info;
    dockRoadRelay.message = FerryOffice::Messages::DockRoadRelay;
    m_interactions.addInteractable(dockRoadRelay);

    TraversalAffordance serviceVault;
    serviceVault.name = FerryOffice::Names::ServiceVault;
    serviceVault.prompt = FerryOffice::Prompts::ServiceVault;
    serviceVault.type = TraversalType::Vault;
    serviceVault.startPosition = FerryOffice::Positions::ServiceVaultStart;
    serviceVault.endPosition = FerryOffice::Positions::ServiceVaultEnd;
    serviceVault.focusRadius = FerryOffice::Traversal::ServiceVaultFocusRadius;
    serviceVault.requiredFacingDirection = FerryOffice::Positions::ServiceVaultFacing;
    serviceVault.requiredFacingDot = FerryOffice::Traversal::ServiceVaultRequiredFacingDot;
    serviceVault.durationSeconds = FerryOffice::Traversal::ServiceVaultDurationSeconds;
    m_traversal.addAffordance(serviceVault);

    FerryOfficeJobConfig jobConfig;
    jobConfig.vehicleCheckpointPosition = FerryOffice::Positions::ServiceRunCheckpoint;
    jobConfig.vehicleCheckpointRadius = FerryOffice::Radii::ServiceRunCheckpoint;
    m_job.configure(std::move(jobConfig));

    syncRouteGateCollider();
}

namespace {

InteractableType InteractableTypeFromSceneString(const std::string& type)
{
    if (type == "pickup") {
        return InteractableType::Pickup;
    }
    if (type == "toggle") {
        return InteractableType::Toggle;
    }
    return InteractableType::Info;
}

TraversalType TraversalTypeFromSceneString(const std::string&)
{
    return TraversalType::Vault;
}

} // namespace

void PrototypeScene::loadFromDefinition(const SceneDefinition& sceneDefinition)
{
    m_worldState.clear();
    m_world.buildFromSceneDefinition(sceneDefinition);
    m_interactions.clear();
    m_traversal.clear();

    for (const SceneInteractableDefinition& authored : sceneDefinition.interactables) {
        Interactable interactable;
        interactable.name = authored.name;
        interactable.prompt = authored.prompt;
        interactable.position = authored.position;
        interactable.radius = authored.radius;
        interactable.type = InteractableTypeFromSceneString(authored.type);
        interactable.oneShot = authored.oneShot;
        interactable.message = authored.message;
        m_interactions.addInteractable(std::move(interactable));
    }

    for (const SceneTraversalAffordanceDefinition& authored : sceneDefinition.traversalAffordances) {
        TraversalAffordance affordance;
        affordance.name = authored.name;
        affordance.prompt = authored.prompt;
        affordance.type = TraversalTypeFromSceneString(authored.type);
        affordance.startPosition = authored.startPosition;
        affordance.endPosition = authored.endPosition;
        affordance.focusRadius = authored.focusRadius;
        affordance.requiredFacingDirection = authored.requiredFacingDirection;
        affordance.requiredFacingDot = authored.requiredFacingDot;
        affordance.durationSeconds = authored.durationSeconds;
        m_traversal.addAffordance(std::move(affordance));
    }

    configureJobFromDefinition(sceneDefinition);
    syncRouteGateCollider();
}

PrototypeWorld& PrototypeScene::world()
{
    return m_world;
}

const PrototypeWorld& PrototypeScene::world() const
{
    return m_world;
}

InteractionSystem& PrototypeScene::interactions()
{
    return m_interactions;
}

const InteractionSystem& PrototypeScene::interactions() const
{
    return m_interactions;
}

TraversalSystem& PrototypeScene::traversal()
{
    return m_traversal;
}

const TraversalSystem& PrototypeScene::traversal() const
{
    return m_traversal;
}

WorldState& PrototypeScene::worldState()
{
    return m_worldState;
}

const WorldState& PrototypeScene::worldState() const
{
    return m_worldState;
}

FerryOfficeJob& PrototypeScene::job()
{
    return m_job;
}

const FerryOfficeJob& PrototypeScene::job() const
{
    return m_job;
}

bool PrototypeScene::applyInteractionResult(const InteractionResult& result)
{
    if (!result.triggered) {
        return false;
    }

    bool changed = false;
    if (result.name == FerryOffice::Names::FerryManifest || result.name == "Test Pickup") {
        changed |= m_job.recordJobStarted(m_worldState, result.name);
        changed |= m_worldState.setFlag(WorldFlag::ManifestCollected, true, result.name);
    } else if (result.name == FerryOffice::Names::FerryOfficeNotice) {
        changed |= m_job.recordJobStarted(m_worldState, result.name);
    } else if (result.name == FerryOffice::Names::MaintenanceBox) {
        changed |= m_worldState.setFlag(WorldFlag::MaintenanceBoxInspected, true, result.name);
        changed |= m_worldState.setFlag(WorldFlag::PowerRestored, true, result.name);
    } else if (result.name == FerryOffice::Names::WallButton) {
        changed |= m_worldState.setFlag(WorldFlag::RouteOpened, true, result.name);
        syncRouteGateCollider();
    } else if (result.name == FerryOffice::Names::ExitMarker) {
        changed |= recordExitReached();
    } else if (result.name == FerryOffice::Names::ServiceRunMarker) {
        changed |= m_job.confirmServiceRun(m_worldState, result.name);
    } else if (result.name == FerryOffice::Names::DockRoadRelay) {
        if (m_job.isComplete(m_worldState)) {
            changed |= m_worldState.setFlag(WorldFlag::DockRoadRelayReset, true, result.name);
        }
    } else if (result.name == FerryOffice::Names::RelayServiceLog) {
        if (m_worldState.isFlagSet(WorldFlag::DockRoadRelayReset)) {
            changed |= m_worldState.setFlag(WorldFlag::DockRoadRelayLogged, true, result.name);
        }
    }

    return changed;
}

bool PrototypeScene::recordServiceRouteUsed()
{
    return m_worldState.setFlag(WorldFlag::ServiceRouteUsed, true, std::string(FerryOffice::Messages::ServiceVault));
}

bool PrototypeScene::recordServiceVehicleUsed()
{
    return m_job.recordServiceVehicleUsed(m_worldState);
}

bool PrototypeScene::updateJobVehicleCheckpoint(engine::Vec3 vehiclePosition, bool vehicleOccupied)
{
    return m_job.updateVehicleCheckpoint(m_worldState, vehiclePosition, vehicleOccupied);
}

bool PrototypeScene::recordExitReached()
{
    if (!isSliceReadyForExit()) {
        return false;
    }

    return m_worldState.setFlag(WorldFlag::ExitReached, true, std::string(FerryOffice::Names::ExitMarker));
}

bool PrototypeScene::isSliceReadyForExit() const
{
    return m_worldState.isFlagSet(WorldFlag::ManifestCollected)
        && m_worldState.isFlagSet(WorldFlag::ServiceRouteUsed)
        && m_worldState.isFlagSet(WorldFlag::MaintenanceBoxInspected)
        && m_worldState.isFlagSet(WorldFlag::PowerRestored)
        && m_worldState.isFlagSet(WorldFlag::RouteOpened);
}

bool PrototypeScene::isSliceComplete() const
{
    return isSliceReadyForExit() && m_worldState.isFlagSet(WorldFlag::ExitReached);
}

bool PrototypeScene::isServiceGateBlocking() const
{
    const StaticCollider* gate = m_world.colliderByName(FerryOffice::Names::ServiceGateCollider);
    return gate != nullptr && gate->blocksPlayer;
}

bool PrototypeScene::isJobComplete() const
{
    return m_job.isComplete(m_worldState);
}

std::string PrototypeScene::currentObjectiveText() const
{
    if (!m_worldState.isFlagSet(WorldFlag::ManifestCollected)) {
        return "Collect the Ferry Manifest at the dock-side office approach.";
    }
    if (!m_worldState.isFlagSet(WorldFlag::ServiceRouteUsed)) {
        return "Vault the right-side Service Barrier to reach the Maintenance Box.";
    }
    if (!m_worldState.isFlagSet(WorldFlag::MaintenanceBoxInspected) || !m_worldState.isFlagSet(WorldFlag::PowerRestored)) {
        return "Inspect the Maintenance Box on the service side to restore power.";
    }
    if (!m_worldState.isFlagSet(WorldFlag::RouteOpened)) {
        return "Return to the office Wall Button and open the service gate.";
    }
    if (!m_worldState.isFlagSet(WorldFlag::ExitReached)) {
        return "Pass the open service gate to the Exit Summary Marker.";
    }

    return "Ferry Office micro-slice complete.";
}

std::string PrototypeScene::currentJobObjectiveText() const
{
    if (m_job.isComplete(m_worldState) && !m_worldState.isFlagSet(WorldFlag::DockRoadRelayReset)) {
        return "Reset the Dock Road Relay beside the service-run marker.";
    }
    if (m_worldState.isFlagSet(WorldFlag::DockRoadRelayReset) && !m_worldState.isFlagSet(WorldFlag::DockRoadRelayLogged)) {
        return "Log the relay reset on the service-run board.";
    }
    if (m_worldState.isFlagSet(WorldFlag::DockRoadRelayLogged)) {
        return "Relay reset logged. Ferry Office follow-up complete.";
    }
    return m_job.currentObjectiveText(m_worldState);
}

std::string PrototypeScene::completionSummary() const
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

std::string PrototypeScene::jobDebugSummary() const
{
    return m_job.debugSummary(m_worldState);
}

void PrototypeScene::configureJobFromDefinition(const SceneDefinition& sceneDefinition)
{
    FerryOfficeJobConfig config;

    for (const SceneObjectiveMarkerDefinition& marker : sceneDefinition.objectiveMarkers) {
        if (marker.id == "service-run-checkpoint-marker") {
            config.vehicleCheckpointPosition = marker.position;
            break;
        }
    }

    for (const SceneInteractableDefinition& interactable : sceneDefinition.interactables) {
        if (interactable.id == "service-run-confirm-marker") {
            config.vehicleCheckpointRadius = std::max(interactable.radius, config.vehicleCheckpointRadius);
            break;
        }
    }

    m_job.configure(std::move(config));
}

void PrototypeScene::syncRouteGateCollider()
{
    m_world.setColliderBlocksPlayer(FerryOffice::Names::ServiceGateCollider, !m_worldState.isFlagSet(WorldFlag::RouteOpened));
}
