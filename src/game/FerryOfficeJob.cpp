#include "game/FerryOfficeJob.h"

#include <sstream>
#include <utility>

namespace {

bool HasCoreAccessFlags(const WorldState& state)
{
    return state.isFlagSet(WorldFlag::FerryOfficeJobStarted)
        && state.isFlagSet(WorldFlag::ManifestCollected)
        && state.isFlagSet(WorldFlag::ServiceRouteUsed)
        && state.isFlagSet(WorldFlag::MaintenanceBoxInspected)
        && state.isFlagSet(WorldFlag::PowerRestored)
        && state.isFlagSet(WorldFlag::RouteOpened);
}

bool IsWithinHorizontalRadius(engine::Vec3 a, engine::Vec3 b, float radius)
{
    const engine::Vec3 delta {a.x - b.x, 0.0f, a.z - b.z};
    return engine::Length(delta) <= radius;
}

} // namespace

std::string_view FerryOfficeJobPhaseName(FerryOfficeJobPhase phase)
{
    switch (phase) {
    case FerryOfficeJobPhase::CollectManifest:
        return "collectManifest";
    case FerryOfficeJobPhase::UseServiceRoute:
        return "useServiceRoute";
    case FerryOfficeJobPhase::RestorePower:
        return "restorePower";
    case FerryOfficeJobPhase::OpenServiceGate:
        return "openServiceGate";
    case FerryOfficeJobPhase::UseServiceVehicle:
        return "useServiceVehicle";
    case FerryOfficeJobPhase::ReachDockRoad:
        return "reachDockRoad";
    case FerryOfficeJobPhase::ConfirmServiceRun:
        return "confirmServiceRun";
    case FerryOfficeJobPhase::Complete:
        return "complete";
    default:
        return "unknown";
    }
}

std::string_view FerryOfficeActiveRouteMarkerId(const WorldState& state, FerryOfficeJobPhase phase)
{
    if (!state.isFlagSet(WorldFlag::FerryOfficeJobComplete)) {
        switch (phase) {
        case FerryOfficeJobPhase::UseServiceRoute:
            return "route-manifest-to-vault";
        case FerryOfficeJobPhase::RestorePower:
            return "route-vault-to-maintenance";
        case FerryOfficeJobPhase::OpenServiceGate:
            return "route-maintenance-to-button";
        case FerryOfficeJobPhase::ReachDockRoad:
            return "route-service-yard-to-dock-road";
        case FerryOfficeJobPhase::ConfirmServiceRun:
            return "route-dock-road-to-service-confirm";
        default:
            return {};
        }
    }

    if (!state.isFlagSet(WorldFlag::DockRoadRelayReset)) {
        return "route-service-confirm-to-relay";
    }
    if (!state.isFlagSet(WorldFlag::DockRoadRelayLogged)) {
        return "route-relay-to-service-log";
    }
    if (!state.isFlagSet(WorldFlag::DockRoadClearanceTagged)) {
        return "route-service-log-to-clearance-tag";
    }
    if (!state.isFlagSet(WorldFlag::HarborPartsPickedUp)) {
        return "route-clearance-tag-to-harbor-parts";
    }
    if (!state.isFlagSet(WorldFlag::HarborPartsDelivered)) {
        return "route-harbor-parts-to-office-shelf";
    }
    if (!state.isFlagSet(WorldFlag::FerryOfficeBoardUpdated)) {
        return "route-parts-shelf-to-work-board";
    }
    if (!state.isFlagSet(WorldFlag::FerryOfficeHandoffFiled)) {
        return "route-work-board-to-handoff-note";
    }
    if (!state.isFlagSet(WorldFlag::StormPumpReset)) {
        return "route-handoff-note-to-storm-pump";
    }
    if (!state.isFlagSet(WorldFlag::StormPumpTicketClosed)) {
        return "route-storm-pump-to-ticket";
    }
    if (!state.isFlagSet(WorldFlag::LowDockDrainCleared)) {
        return "route-storm-ticket-to-low-dock-drain";
    }
    if (!state.isFlagSet(WorldFlag::LowDockDrainLogged)) {
        return "route-low-dock-drain-to-office-log";
    }
    return {};
}

std::string_view FerryOfficeActiveObjectiveMarkerId(const WorldState& state, FerryOfficeJobPhase phase)
{
    if (!state.isFlagSet(WorldFlag::FerryOfficeJobComplete)) {
        switch (phase) {
        case FerryOfficeJobPhase::UseServiceVehicle:
        case FerryOfficeJobPhase::ReachDockRoad:
            return "service-yard-marker";
        case FerryOfficeJobPhase::ConfirmServiceRun:
            return "service-run-checkpoint-marker";
        default:
            return {};
        }
    }

    if (!state.isFlagSet(WorldFlag::DockRoadRelayReset)) {
        return "dock-road-relay-marker";
    }
    if (!state.isFlagSet(WorldFlag::DockRoadRelayLogged)) {
        return "relay-service-log-marker";
    }
    if (!state.isFlagSet(WorldFlag::DockRoadClearanceTagged)) {
        return "dock-road-clearance-marker";
    }
    if (!state.isFlagSet(WorldFlag::HarborPartsPickedUp)) {
        return "harbor-parts-marker";
    }
    if (!state.isFlagSet(WorldFlag::HarborPartsDelivered)) {
        return "ferry-office-parts-shelf-marker";
    }
    if (!state.isFlagSet(WorldFlag::FerryOfficeBoardUpdated)) {
        return "ferry-office-work-board-marker";
    }
    if (!state.isFlagSet(WorldFlag::FerryOfficeHandoffFiled)) {
        return "ferry-office-handoff-marker";
    }
    if (!state.isFlagSet(WorldFlag::StormPumpReset)) {
        return "storm-pump-marker";
    }
    if (!state.isFlagSet(WorldFlag::StormPumpTicketClosed)) {
        return "storm-pump-ticket-marker";
    }
    if (!state.isFlagSet(WorldFlag::LowDockDrainCleared)) {
        return "low-dock-drain-objective-marker";
    }
    if (!state.isFlagSet(WorldFlag::LowDockDrainLogged)) {
        return "ferry-office-drain-log-marker";
    }
    return {};
}

std::string FerryOfficeFollowupStatusText(const WorldState& state)
{
    std::ostringstream output;
    output << "Follow-up: relay="
           << (state.isFlagSet(WorldFlag::DockRoadRelayReset) ? "reset" : "later")
           << " | log="
           << (state.isFlagSet(WorldFlag::DockRoadRelayLogged) ? "signed" : "later")
           << " | road="
           << (state.isFlagSet(WorldFlag::DockRoadClearanceTagged) ? "clear" : "later")
           << " | parts="
           << (state.isFlagSet(WorldFlag::HarborPartsDelivered)
                   ? "delivered"
                   : (state.isFlagSet(WorldFlag::HarborPartsPickedUp) ? "picked" : "later"))
           << " | board="
           << (state.isFlagSet(WorldFlag::FerryOfficeBoardUpdated) ? "updated" : "later")
           << " | handoff="
           << (state.isFlagSet(WorldFlag::FerryOfficeHandoffFiled) ? "filed" : "later")
           << " | pump="
           << (state.isFlagSet(WorldFlag::StormPumpTicketClosed)
                   ? "closed"
                   : (state.isFlagSet(WorldFlag::StormPumpReset) ? "reset" : "later"))
           << " | drain="
           << (state.isFlagSet(WorldFlag::LowDockDrainCleared) ? "clear" : "later")
           << " | logbook="
           << (state.isFlagSet(WorldFlag::LowDockDrainLogged) ? "signed" : "later");
    return output.str();
}

std::string FerryOfficeFollowupNextStepText(const WorldState& state)
{
    if (!state.isFlagSet(WorldFlag::FerryOfficeJobComplete)) {
        return {};
    }
    if (!state.isFlagSet(WorldFlag::DockRoadRelayReset)) {
        return "Reset the Dock Road Relay.";
    }
    if (!state.isFlagSet(WorldFlag::DockRoadRelayLogged)) {
        return "Sign the Relay Service Log.";
    }
    if (!state.isFlagSet(WorldFlag::DockRoadClearanceTagged)) {
        return "Tag Dock Road clear.";
    }
    if (!state.isFlagSet(WorldFlag::HarborPartsPickedUp)) {
        return "Collect Harbor Parts at the dock-road cabinet.";
    }
    if (!state.isFlagSet(WorldFlag::HarborPartsDelivered)) {
        return "Deliver Harbor Parts to the Ferry Office shelf.";
    }
    if (!state.isFlagSet(WorldFlag::FerryOfficeBoardUpdated)) {
        return "Update the Ferry Office Work Board.";
    }
    if (!state.isFlagSet(WorldFlag::FerryOfficeHandoffFiled)) {
        return "File the Ferry Office handoff note.";
    }
    if (!state.isFlagSet(WorldFlag::StormPumpReset)) {
        return "Reset the storm pump by the service-yard cable reel.";
    }
    if (!state.isFlagSet(WorldFlag::StormPumpTicketClosed)) {
        return "Close the storm pump ticket at the Ferry Office board.";
    }
    if (!state.isFlagSet(WorldFlag::LowDockDrainCleared)) {
        return "Tag the low dock drain clear beside the service-run marker.";
    }
    if (!state.isFlagSet(WorldFlag::LowDockDrainLogged)) {
        return "Return to the Ferry Office Drain Log.";
    }
    return "Low dock drain logged. Ferry Office follow-up complete.";
}

void FerryOfficeJob::configure(FerryOfficeJobConfig config)
{
    m_config = std::move(config);
}

bool FerryOfficeJob::recordJobStarted(WorldState& state, std::string source) const
{
    return state.setFlag(WorldFlag::FerryOfficeJobStarted, true, std::move(source));
}

bool FerryOfficeJob::recordServiceVehicleUsed(WorldState& state) const
{
    return state.setFlag(WorldFlag::ServiceVehicleUsed, true, "Service Yard Vehicle");
}

bool FerryOfficeJob::updateVehicleCheckpoint(WorldState& state, engine::Vec3 vehiclePosition, bool vehicleOccupied) const
{
    if (!vehicleOccupied || state.isFlagSet(WorldFlag::DockRoadReached)) {
        return false;
    }

    if (!IsWithinHorizontalRadius(vehiclePosition, m_config.vehicleCheckpointPosition, m_config.vehicleCheckpointRadius)) {
        return false;
    }

    return state.setFlag(WorldFlag::DockRoadReached, true, "Service Run Checkpoint");
}

bool FerryOfficeJob::confirmServiceRun(WorldState& state, std::string source) const
{
    if (!isReadyForConfirmation(state) || isComplete(state)) {
        return false;
    }

    bool changed = false;
    changed |= state.setFlag(WorldFlag::ServiceRunConfirmed, true, source);
    changed |= state.setFlag(WorldFlag::FerryOfficeJobComplete, true, m_config.jobName);
    return changed;
}

FerryOfficeJobPhase FerryOfficeJob::phase(const WorldState& state) const
{
    if (isComplete(state)) {
        return FerryOfficeJobPhase::Complete;
    }
    if (!state.isFlagSet(WorldFlag::FerryOfficeJobStarted) || !state.isFlagSet(WorldFlag::ManifestCollected)) {
        return FerryOfficeJobPhase::CollectManifest;
    }
    if (!state.isFlagSet(WorldFlag::ServiceRouteUsed)) {
        return FerryOfficeJobPhase::UseServiceRoute;
    }
    if (!state.isFlagSet(WorldFlag::MaintenanceBoxInspected) || !state.isFlagSet(WorldFlag::PowerRestored)) {
        return FerryOfficeJobPhase::RestorePower;
    }
    if (!state.isFlagSet(WorldFlag::RouteOpened)) {
        return FerryOfficeJobPhase::OpenServiceGate;
    }
    if (!state.isFlagSet(WorldFlag::ServiceVehicleUsed)) {
        return FerryOfficeJobPhase::UseServiceVehicle;
    }
    if (!state.isFlagSet(WorldFlag::DockRoadReached)) {
        return FerryOfficeJobPhase::ReachDockRoad;
    }
    if (!state.isFlagSet(WorldFlag::ServiceRunConfirmed)) {
        return FerryOfficeJobPhase::ConfirmServiceRun;
    }

    return FerryOfficeJobPhase::Complete;
}

bool FerryOfficeJob::isReadyForConfirmation(const WorldState& state) const
{
    return HasCoreAccessFlags(state)
        && state.isFlagSet(WorldFlag::ServiceVehicleUsed)
        && state.isFlagSet(WorldFlag::DockRoadReached);
}

bool FerryOfficeJob::isComplete(const WorldState& state) const
{
    return state.isFlagSet(WorldFlag::FerryOfficeJobComplete);
}

std::string FerryOfficeJob::currentObjectiveText(const WorldState& state) const
{
    switch (phase(state)) {
    case FerryOfficeJobPhase::CollectManifest:
        return "Check the Ferry Manifest to start the Ferry Office service call.";
    case FerryOfficeJobPhase::UseServiceRoute:
        return "Use the Service Barrier route to reach the Maintenance Box.";
    case FerryOfficeJobPhase::RestorePower:
        return "Inspect the Maintenance Box to restore local power.";
    case FerryOfficeJobPhase::OpenServiceGate:
        return "Open the service gate with the Wall Button.";
    case FerryOfficeJobPhase::UseServiceVehicle:
        return "Enter the service vehicle in the yard.";
    case FerryOfficeJobPhase::ReachDockRoad:
        return "Drive the service vehicle to the dock-road checkpoint.";
    case FerryOfficeJobPhase::ConfirmServiceRun:
        return "Exit near the road marker and confirm the service run.";
    case FerryOfficeJobPhase::Complete:
        return "Ferry Office Service Call complete.";
    default:
        return "Ferry Office Service Call status unknown.";
    }
}

std::string FerryOfficeJob::debugSummary(const WorldState& state) const
{
    std::ostringstream output;
    output << "jobPhase=" << FerryOfficeJobPhaseName(phase(state))
           << " jobStarted=" << (state.isFlagSet(WorldFlag::FerryOfficeJobStarted) ? "yes" : "no")
           << " serviceVehicleUsed=" << (state.isFlagSet(WorldFlag::ServiceVehicleUsed) ? "yes" : "no")
           << " dockRoadReached=" << (state.isFlagSet(WorldFlag::DockRoadReached) ? "yes" : "no")
           << " serviceRunConfirmed=" << (state.isFlagSet(WorldFlag::ServiceRunConfirmed) ? "yes" : "no")
           << " jobComplete=" << (isComplete(state) ? "yes" : "no");
    return output.str();
}

const FerryOfficeJobConfig& FerryOfficeJob::config() const
{
    return m_config;
}
