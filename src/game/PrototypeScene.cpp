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
    m_ferryOfficeBehaviorEnabled = true;
    m_sceneDefinition = {};
    m_targetSliceObjective = {};
    m_completedTargetSliceInteractableName.clear();
    m_lastTargetSliceEventText = "none";
    m_world.buildFerryOfficePrototypeLayout();
    m_interactableActionBindings.clear();
    m_traversalActionBindings.clear();

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

    Interactable relayServiceLog;
    relayServiceLog.name = FerryOffice::Names::RelayServiceLog;
    relayServiceLog.prompt = FerryOffice::Prompts::RelayServiceLog;
    relayServiceLog.position = FerryOffice::Positions::RelayServiceLog;
    relayServiceLog.radius = FerryOffice::Radii::RelayServiceLog;
    relayServiceLog.type = InteractableType::Info;
    relayServiceLog.message = FerryOffice::Messages::RelayServiceLog;
    m_interactions.addInteractable(relayServiceLog);

    Interactable dockRoadClearanceTag;
    dockRoadClearanceTag.name = FerryOffice::Names::DockRoadClearanceTag;
    dockRoadClearanceTag.prompt = FerryOffice::Prompts::DockRoadClearanceTag;
    dockRoadClearanceTag.position = FerryOffice::Positions::DockRoadClearanceTag;
    dockRoadClearanceTag.radius = FerryOffice::Radii::DockRoadClearanceTag;
    dockRoadClearanceTag.type = InteractableType::Info;
    dockRoadClearanceTag.message = FerryOffice::Messages::DockRoadClearanceTag;
    m_interactions.addInteractable(dockRoadClearanceTag);

    Interactable harborPartsCrate;
    harborPartsCrate.name = FerryOffice::Names::HarborPartsCrate;
    harborPartsCrate.prompt = FerryOffice::Prompts::HarborPartsCrate;
    harborPartsCrate.position = FerryOffice::Positions::HarborPartsCrate;
    harborPartsCrate.radius = FerryOffice::Radii::HarborPartsCrate;
    harborPartsCrate.type = InteractableType::Info;
    harborPartsCrate.message = FerryOffice::Messages::HarborPartsCrate;
    m_interactions.addInteractable(harborPartsCrate);

    Interactable ferryOfficePartsShelf;
    ferryOfficePartsShelf.name = FerryOffice::Names::FerryOfficePartsShelf;
    ferryOfficePartsShelf.prompt = FerryOffice::Prompts::FerryOfficePartsShelf;
    ferryOfficePartsShelf.position = FerryOffice::Positions::FerryOfficePartsShelf;
    ferryOfficePartsShelf.radius = FerryOffice::Radii::FerryOfficePartsShelf;
    ferryOfficePartsShelf.type = InteractableType::Info;
    ferryOfficePartsShelf.message = FerryOffice::Messages::FerryOfficePartsShelf;
    m_interactions.addInteractable(ferryOfficePartsShelf);

    Interactable ferryOfficeWorkBoard;
    ferryOfficeWorkBoard.name = FerryOffice::Names::FerryOfficeWorkBoard;
    ferryOfficeWorkBoard.prompt = FerryOffice::Prompts::FerryOfficeWorkBoard;
    ferryOfficeWorkBoard.position = FerryOffice::Positions::FerryOfficeWorkBoard;
    ferryOfficeWorkBoard.radius = FerryOffice::Radii::FerryOfficeWorkBoard;
    ferryOfficeWorkBoard.type = InteractableType::Info;
    ferryOfficeWorkBoard.message = FerryOffice::Messages::FerryOfficeWorkBoard;
    m_interactions.addInteractable(ferryOfficeWorkBoard);

    Interactable ferryOfficeHandoffNote;
    ferryOfficeHandoffNote.name = FerryOffice::Names::FerryOfficeHandoffNote;
    ferryOfficeHandoffNote.prompt = FerryOffice::Prompts::FerryOfficeHandoffNote;
    ferryOfficeHandoffNote.position = FerryOffice::Positions::FerryOfficeHandoffNote;
    ferryOfficeHandoffNote.radius = FerryOffice::Radii::FerryOfficeHandoffNote;
    ferryOfficeHandoffNote.type = InteractableType::Info;
    ferryOfficeHandoffNote.message = FerryOffice::Messages::FerryOfficeHandoffNote;
    m_interactions.addInteractable(ferryOfficeHandoffNote);

    Interactable stormPumpSwitch;
    stormPumpSwitch.name = FerryOffice::Names::StormPumpSwitch;
    stormPumpSwitch.prompt = FerryOffice::Prompts::StormPumpSwitch;
    stormPumpSwitch.position = FerryOffice::Positions::StormPumpSwitch;
    stormPumpSwitch.radius = FerryOffice::Radii::StormPumpSwitch;
    stormPumpSwitch.type = InteractableType::Info;
    stormPumpSwitch.message = FerryOffice::Messages::StormPumpSwitch;
    m_interactions.addInteractable(stormPumpSwitch);

    Interactable stormPumpTicket;
    stormPumpTicket.name = FerryOffice::Names::StormPumpTicket;
    stormPumpTicket.prompt = FerryOffice::Prompts::StormPumpTicket;
    stormPumpTicket.position = FerryOffice::Positions::StormPumpTicket;
    stormPumpTicket.radius = FerryOffice::Radii::StormPumpTicket;
    stormPumpTicket.type = InteractableType::Info;
    stormPumpTicket.message = FerryOffice::Messages::StormPumpTicket;
    m_interactions.addInteractable(stormPumpTicket);

    Interactable lowDockDrainMarker;
    lowDockDrainMarker.name = FerryOffice::Names::LowDockDrainMarker;
    lowDockDrainMarker.prompt = FerryOffice::Prompts::LowDockDrainMarker;
    lowDockDrainMarker.position = FerryOffice::Positions::LowDockDrainMarker;
    lowDockDrainMarker.radius = FerryOffice::Radii::LowDockDrainMarker;
    lowDockDrainMarker.type = InteractableType::Info;
    lowDockDrainMarker.message = FerryOffice::Messages::LowDockDrainMarker;
    m_interactions.addInteractable(lowDockDrainMarker);

    Interactable ferryOfficeDrainLog;
    ferryOfficeDrainLog.name = FerryOffice::Names::FerryOfficeDrainLog;
    ferryOfficeDrainLog.prompt = FerryOffice::Prompts::FerryOfficeDrainLog;
    ferryOfficeDrainLog.position = FerryOffice::Positions::FerryOfficeDrainLog;
    ferryOfficeDrainLog.radius = FerryOffice::Radii::FerryOfficeDrainLog;
    ferryOfficeDrainLog.type = InteractableType::Info;
    ferryOfficeDrainLog.message = FerryOffice::Messages::FerryOfficeDrainLog;
    m_interactions.addInteractable(ferryOfficeDrainLog);

    addInteractableActionBinding(std::string(FerryOffice::Names::FerryManifest),
        {WorldFlag::ManifestCollected},
        {},
        {});
    addInteractableActionBinding(std::string(FerryOffice::Names::MaintenanceBox),
        {WorldFlag::MaintenanceBoxInspected, WorldFlag::PowerRestored},
        {},
        {});
    addInteractableActionBinding(std::string(FerryOffice::Names::WallButton),
        {WorldFlag::RouteOpened},
        {},
        {});
    addInteractableActionBinding(std::string(FerryOffice::Names::ExitMarker),
        {},
        {WorldFlag::ManifestCollected,
            WorldFlag::ServiceRouteUsed,
            WorldFlag::MaintenanceBoxInspected,
            WorldFlag::PowerRestored,
            WorldFlag::RouteOpened},
        {WorldFlag::ExitReached});
    addInteractableActionBinding(std::string(FerryOffice::Names::ServiceRunMarker),
        {},
        {WorldFlag::FerryOfficeJobStarted,
            WorldFlag::ManifestCollected,
            WorldFlag::ServiceRouteUsed,
            WorldFlag::MaintenanceBoxInspected,
            WorldFlag::PowerRestored,
            WorldFlag::RouteOpened,
            WorldFlag::ServiceVehicleUsed,
            WorldFlag::DockRoadReached},
        {WorldFlag::ServiceRunConfirmed, WorldFlag::FerryOfficeJobComplete});
    addInteractableActionBinding(std::string(FerryOffice::Names::DockRoadRelay),
        {},
        {WorldFlag::FerryOfficeJobComplete},
        {WorldFlag::DockRoadRelayReset});
    addInteractableActionBinding(std::string(FerryOffice::Names::RelayServiceLog),
        {},
        {WorldFlag::DockRoadRelayReset},
        {WorldFlag::DockRoadRelayLogged});
    addInteractableActionBinding(std::string(FerryOffice::Names::DockRoadClearanceTag),
        {},
        {WorldFlag::DockRoadRelayLogged},
        {WorldFlag::DockRoadClearanceTagged});
    addInteractableActionBinding(std::string(FerryOffice::Names::HarborPartsCrate),
        {},
        {WorldFlag::DockRoadClearanceTagged},
        {WorldFlag::HarborPartsPickedUp});
    addInteractableActionBinding(std::string(FerryOffice::Names::FerryOfficePartsShelf),
        {},
        {WorldFlag::HarborPartsPickedUp},
        {WorldFlag::HarborPartsDelivered});
    addInteractableActionBinding(std::string(FerryOffice::Names::FerryOfficeWorkBoard),
        {},
        {WorldFlag::HarborPartsDelivered},
        {WorldFlag::FerryOfficeBoardUpdated});
    addInteractableActionBinding(std::string(FerryOffice::Names::FerryOfficeHandoffNote),
        {},
        {WorldFlag::FerryOfficeBoardUpdated},
        {WorldFlag::FerryOfficeHandoffFiled});
    addInteractableActionBinding(std::string(FerryOffice::Names::StormPumpSwitch),
        {},
        {WorldFlag::FerryOfficeHandoffFiled},
        {WorldFlag::StormPumpReset});
    addInteractableActionBinding(std::string(FerryOffice::Names::StormPumpTicket),
        {},
        {WorldFlag::StormPumpReset},
        {WorldFlag::StormPumpTicketClosed});
    addInteractableActionBinding(std::string(FerryOffice::Names::LowDockDrainMarker),
        {},
        {WorldFlag::StormPumpTicketClosed},
        {WorldFlag::LowDockDrainCleared});
    addInteractableActionBinding(std::string(FerryOffice::Names::FerryOfficeDrainLog),
        {},
        {WorldFlag::LowDockDrainCleared},
        {WorldFlag::LowDockDrainLogged});

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
    const int serviceVaultId = m_traversal.addAffordance(serviceVault);
    addTraversalActionBinding(
        serviceVaultId,
        std::string(FerryOffice::Messages::ServiceVault),
        {WorldFlag::ServiceRouteUsed});

    FerryOfficeJobConfig jobConfig;
    jobConfig.vehicleCheckpointPosition = FerryOffice::Positions::ServiceRunCheckpoint;
    jobConfig.vehicleCheckpointRadius = FerryOffice::Radii::ServiceRunCheckpoint;
    m_job.configure(std::move(jobConfig));

    syncWorldStateColliders();
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
    m_sceneDefinition = sceneDefinition;
    m_ferryOfficeBehaviorEnabled = IsFerryOfficeRegressionScene(sceneDefinition);
    m_completedTargetSliceInteractableName.clear();
    m_targetSliceObjective = BuildTargetSliceObjectiveState(sceneDefinition);
    m_lastTargetSliceEventText = "none";
    m_worldState.clear();
    m_world.buildFromSceneDefinition(sceneDefinition);
    m_interactions.clear();
    m_traversal.clear();
    m_interactableActionBindings.clear();
    m_traversalActionBindings.clear();

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
        addInteractableActionBinding(authored);
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
        const int affordanceId = m_traversal.addAffordance(std::move(affordance));
        addTraversalActionBinding(affordanceId, authored);
    }

    configureJobFromDefinition(sceneDefinition);
    syncWorldStateColliders();
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
    if (m_ferryOfficeBehaviorEnabled && (result.name == FerryOffice::Names::FerryManifest || result.name == "Test Pickup")) {
        changed |= m_job.recordJobStarted(m_worldState, result.name);
        if (result.name == "Test Pickup") {
            changed |= m_worldState.setFlag(WorldFlag::ManifestCollected, true, result.name);
        }
    } else if (m_ferryOfficeBehaviorEnabled && result.name == FerryOffice::Names::FerryOfficeNotice) {
        changed |= m_job.recordJobStarted(m_worldState, result.name);
    } else if (m_ferryOfficeBehaviorEnabled && result.name == FerryOffice::Names::ExitMarker) {
        changed |= recordExitReached();
    } else if (m_ferryOfficeBehaviorEnabled && result.name == FerryOffice::Names::ServiceRunMarker) {
        changed |= m_job.confirmServiceRun(m_worldState, result.name);
    }

    changed |= applyAuthoredInteractionBinding(result.name, result.name);
    changed |= recordTargetSliceInteraction(result.name);
    if (changed) {
        syncWorldStateColliders();
    }

    return changed;
}

bool PrototypeScene::recordServiceRouteUsed()
{
    if (!m_ferryOfficeBehaviorEnabled) {
        return false;
    }
    const bool changed =
        m_worldState.setFlag(WorldFlag::ServiceRouteUsed, true, std::string(FerryOffice::Messages::ServiceVault));
    if (changed) {
        syncWorldStateColliders();
    }
    return changed;
}

bool PrototypeScene::recordTraversalCompleted(int affordanceId)
{
    bool changed = false;
    for (const TraversalActionBinding& binding : m_traversalActionBindings) {
        if (binding.affordanceId != affordanceId) {
            continue;
        }
        for (WorldFlag flag : binding.worldFlagsSetOnComplete) {
            changed |= m_worldState.setFlag(flag, true, binding.source);
        }
    }
    if (changed) {
        syncWorldStateColliders();
    }
    return changed;
}

bool PrototypeScene::recordServiceVehicleUsed()
{
    if (!m_ferryOfficeBehaviorEnabled) {
        return false;
    }
    const bool changed = m_job.recordServiceVehicleUsed(m_worldState);
    if (changed) {
        syncWorldStateColliders();
    }
    return changed;
}

bool PrototypeScene::updateJobVehicleCheckpoint(engine::Vec3 vehiclePosition, bool vehicleOccupied)
{
    if (!m_ferryOfficeBehaviorEnabled) {
        return false;
    }
    const bool changed = m_job.updateVehicleCheckpoint(m_worldState, vehiclePosition, vehicleOccupied);
    if (changed) {
        syncWorldStateColliders();
    }
    return changed;
}

bool PrototypeScene::recordExitReached()
{
    if (!m_ferryOfficeBehaviorEnabled) {
        return false;
    }
    if (!isSliceReadyForExit()) {
        return false;
    }

    const bool changed = m_worldState.setFlag(WorldFlag::ExitReached, true, std::string(FerryOffice::Names::ExitMarker));
    if (changed) {
        syncWorldStateColliders();
    }
    return changed;
}

bool PrototypeScene::isSliceReadyForExit() const
{
    if (!m_ferryOfficeBehaviorEnabled) {
        return m_targetSliceObjective.active && m_targetSliceObjective.complete;
    }
    return m_worldState.isFlagSet(WorldFlag::ManifestCollected)
        && m_worldState.isFlagSet(WorldFlag::ServiceRouteUsed)
        && m_worldState.isFlagSet(WorldFlag::MaintenanceBoxInspected)
        && m_worldState.isFlagSet(WorldFlag::PowerRestored)
        && m_worldState.isFlagSet(WorldFlag::RouteOpened);
}

bool PrototypeScene::isSliceComplete() const
{
    if (!m_ferryOfficeBehaviorEnabled) {
        return m_targetSliceObjective.active && m_targetSliceObjective.complete;
    }
    return isSliceReadyForExit() && m_worldState.isFlagSet(WorldFlag::ExitReached);
}

bool PrototypeScene::isServiceGateBlocking() const
{
    if (!m_ferryOfficeBehaviorEnabled) {
        return false;
    }
    const StaticCollider* gate = m_world.colliderByName(FerryOffice::Names::ServiceGateCollider);
    return gate != nullptr && gate->blocksPlayer;
}

bool PrototypeScene::isJobComplete() const
{
    if (!m_ferryOfficeBehaviorEnabled) {
        return false;
    }
    return m_job.isComplete(m_worldState);
}

std::string PrototypeScene::currentObjectiveText() const
{
    if (!m_ferryOfficeBehaviorEnabled) {
        if (m_targetSliceObjective.active) {
            return m_targetSliceObjective.complete
                ? m_targetSliceObjective.completionEventText
                : m_targetSliceObjective.debugObjectiveText;
        }
        return "Neutral target-slice scaffold loaded.";
    }
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
    if (!m_ferryOfficeBehaviorEnabled) {
        if (m_targetSliceObjective.active) {
            return m_targetSliceObjective.complete
                ? m_targetSliceObjective.completionEventText
                : m_targetSliceObjective.objectiveText;
        }
        return "Inspect neutral slice markers; no authored job is active.";
    }
    if (m_job.isComplete(m_worldState) && !m_worldState.isFlagSet(WorldFlag::DockRoadRelayReset)) {
        return "Reset the Dock Road Relay beside the service-run marker.";
    }
    if (m_worldState.isFlagSet(WorldFlag::DockRoadRelayReset) && !m_worldState.isFlagSet(WorldFlag::DockRoadRelayLogged)) {
        return "Log the relay reset on the service-run board.";
    }
    if (m_worldState.isFlagSet(WorldFlag::DockRoadRelayLogged)
        && !m_worldState.isFlagSet(WorldFlag::DockRoadClearanceTagged)) {
        return "Tag Dock Road clear beside the relay service board.";
    }
    if (m_worldState.isFlagSet(WorldFlag::DockRoadClearanceTagged)
        && !m_worldState.isFlagSet(WorldFlag::HarborPartsPickedUp)) {
        return "Collect the Harbor Parts Crate near the dock-road cabinet.";
    }
    if (m_worldState.isFlagSet(WorldFlag::HarborPartsPickedUp)
        && !m_worldState.isFlagSet(WorldFlag::HarborPartsDelivered)) {
        return "Deliver the harbor parts to the Ferry Office parts shelf.";
    }
    if (m_worldState.isFlagSet(WorldFlag::HarborPartsDelivered)
        && !m_worldState.isFlagSet(WorldFlag::FerryOfficeBoardUpdated)) {
        return "Update the Ferry Office Work Board with the parts return.";
    }
    if (m_worldState.isFlagSet(WorldFlag::FerryOfficeBoardUpdated)
        && !m_worldState.isFlagSet(WorldFlag::FerryOfficeHandoffFiled)) {
        return "File the Ferry Office handoff note for the next ferry crew.";
    }
    if (m_worldState.isFlagSet(WorldFlag::FerryOfficeHandoffFiled)
        && !m_worldState.isFlagSet(WorldFlag::StormPumpReset)) {
        return "Reset the storm pump by the service-yard cable reel.";
    }
    if (m_worldState.isFlagSet(WorldFlag::StormPumpReset)
        && !m_worldState.isFlagSet(WorldFlag::StormPumpTicketClosed)) {
        return "Close the storm pump ticket at the Ferry Office board.";
    }
    if (m_worldState.isFlagSet(WorldFlag::StormPumpTicketClosed)
        && !m_worldState.isFlagSet(WorldFlag::LowDockDrainCleared)) {
        return "Tag the low dock drain clear beside the service-run marker.";
    }
    if (m_worldState.isFlagSet(WorldFlag::LowDockDrainCleared)
        && !m_worldState.isFlagSet(WorldFlag::LowDockDrainLogged)) {
        return "Return to the Ferry Office Drain Log.";
    }
    if (m_worldState.isFlagSet(WorldFlag::LowDockDrainLogged)) {
        return "Low dock drain logged. Ferry Office follow-up complete.";
    }
    return m_job.currentObjectiveText(m_worldState);
}

std::string PrototypeScene::completionSummary() const
{
    if (!m_ferryOfficeBehaviorEnabled) {
        if (m_targetSliceObjective.active) {
            return m_targetSliceObjective.completionSummary;
        }
        return "complete=false role=neutral-target-slice targetObjective=none";
    }
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
    if (!m_ferryOfficeBehaviorEnabled) {
        if (m_targetSliceObjective.active) {
            return "job=none phase=target-slice-authored-objective targetObjective=" + m_targetSliceObjective.id;
        }
        return "job=none phase=neutral-target-slice targetObjective=none";
    }
    return m_job.debugSummary(m_worldState);
}

std::string PrototypeScene::lastRuntimeEventText() const
{
    const std::string worldEvent = m_worldState.lastEventText();
    if (worldEvent != "none") {
        return worldEvent;
    }
    return m_lastTargetSliceEventText;
}

void PrototypeScene::addInteractableActionBinding(
    std::string name,
    std::vector<WorldFlag> worldFlagsSet,
    std::vector<WorldFlag> requiredWorldFlags,
    std::vector<WorldFlag> worldFlagsSetWhenReady)
{
    if (worldFlagsSet.empty() && worldFlagsSetWhenReady.empty()) {
        return;
    }

    InteractableActionBinding binding;
    binding.name = std::move(name);
    binding.worldFlagsSet = std::move(worldFlagsSet);
    binding.requiredWorldFlags = std::move(requiredWorldFlags);
    binding.worldFlagsSetWhenReady = std::move(worldFlagsSetWhenReady);
    m_interactableActionBindings.push_back(std::move(binding));
}

void PrototypeScene::addInteractableActionBinding(const SceneInteractableDefinition& interactable)
{
    std::vector<WorldFlag> worldFlagsSet;
    std::vector<WorldFlag> requiredWorldFlags;
    std::vector<WorldFlag> worldFlagsSetWhenReady;

    const auto appendKnownFlags = [](const std::vector<std::string>& names, std::vector<WorldFlag>& flags) {
        for (const std::string& name : names) {
            WorldFlag flag = WorldFlag::PowerRestored;
            if (TryWorldFlagFromName(name, flag)) {
                flags.push_back(flag);
            }
        }
    };

    appendKnownFlags(interactable.worldFlagsSet, worldFlagsSet);
    appendKnownFlags(interactable.requiredWorldFlags, requiredWorldFlags);
    appendKnownFlags(interactable.worldFlagsSetWhenReady, worldFlagsSetWhenReady);
    addInteractableActionBinding(
        interactable.name,
        std::move(worldFlagsSet),
        std::move(requiredWorldFlags),
        std::move(worldFlagsSetWhenReady));
}

void PrototypeScene::addTraversalActionBinding(
    int affordanceId,
    std::string source,
    std::vector<WorldFlag> worldFlagsSetOnComplete)
{
    if (affordanceId <= 0 || worldFlagsSetOnComplete.empty()) {
        return;
    }

    TraversalActionBinding binding;
    binding.affordanceId = affordanceId;
    binding.source = std::move(source);
    binding.worldFlagsSetOnComplete = std::move(worldFlagsSetOnComplete);
    m_traversalActionBindings.push_back(std::move(binding));
}

void PrototypeScene::addTraversalActionBinding(int affordanceId, const SceneTraversalAffordanceDefinition& affordance)
{
    std::vector<WorldFlag> flags;
    for (const std::string& name : affordance.worldFlagsSetOnComplete) {
        WorldFlag flag = WorldFlag::PowerRestored;
        if (TryWorldFlagFromName(name, flag)) {
            flags.push_back(flag);
        }
    }

    addTraversalActionBinding(affordanceId, affordance.name, std::move(flags));
}

bool PrototypeScene::applyAuthoredInteractionBinding(std::string_view name, const std::string& source)
{
    bool changed = false;
    for (const InteractableActionBinding& binding : m_interactableActionBindings) {
        if (binding.name != name) {
            continue;
        }

        for (WorldFlag flag : binding.worldFlagsSet) {
            changed |= m_worldState.setFlag(flag, true, source);
        }
        if (hasRequiredWorldFlags(binding)) {
            for (WorldFlag flag : binding.worldFlagsSetWhenReady) {
                changed |= m_worldState.setFlag(flag, true, source);
            }
        }
    }
    return changed;
}

bool PrototypeScene::hasRequiredWorldFlags(const InteractableActionBinding& binding) const
{
    for (WorldFlag flag : binding.requiredWorldFlags) {
        if (!m_worldState.isFlagSet(flag)) {
            return false;
        }
    }
    return true;
}

bool PrototypeScene::recordTargetSliceInteraction(std::string_view interactableName)
{
    if (m_ferryOfficeBehaviorEnabled || !m_targetSliceObjective.active || m_targetSliceObjective.complete) {
        return false;
    }
    if (interactableName != m_targetSliceObjective.completionInteractableName) {
        return false;
    }

    m_completedTargetSliceInteractableName = std::string(interactableName);
    m_targetSliceObjective = BuildTargetSliceObjectiveState(m_sceneDefinition, m_completedTargetSliceInteractableName);
    m_lastTargetSliceEventText = m_targetSliceObjective.completionEventText;
    return m_targetSliceObjective.complete;
}

void PrototypeScene::configureJobFromDefinition(const SceneDefinition& sceneDefinition)
{
    if (!IsFerryOfficeRegressionScene(sceneDefinition)) {
        m_job.configure(FerryOfficeJobConfig {});
        return;
    }

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

void PrototypeScene::syncWorldStateColliders()
{
    std::vector<std::pair<std::string, bool>> updates;
    for (const StaticCollider& collider : m_world.colliders()) {
        if (collider.stateFlag.empty()) {
            continue;
        }

        WorldFlag flag = WorldFlag::PowerRestored;
        if (!TryWorldFlagFromName(collider.stateFlag, flag)) {
            continue;
        }

        const bool isSet = m_worldState.isFlagSet(flag);
        updates.emplace_back(collider.name, collider.blocksWhenFlagFalse ? !isSet : isSet);
    }

    for (const auto& [name, blocksPlayer] : updates) {
        m_world.setColliderBlocksPlayer(name, blocksPlayer);
    }

    const StaticCollider* serviceGate = m_world.colliderByName(FerryOffice::Names::ServiceGateCollider);
    if (serviceGate != nullptr && serviceGate->stateFlag.empty()) {
        m_world.setColliderBlocksPlayer(
            FerryOffice::Names::ServiceGateCollider,
            !m_worldState.isFlagSet(WorldFlag::RouteOpened));
    }
}
