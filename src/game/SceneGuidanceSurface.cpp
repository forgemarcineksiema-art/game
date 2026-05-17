#include "game/SceneGuidanceSurface.h"

#include "game/FerryOfficeData.h"

namespace {

bool ShowsAllAuthoredGuidance(const SceneGuidanceContext& context)
{
    return context.fullDebug || context.guidancePolicy == SceneGuidancePolicy::AllAuthored;
}

} // namespace

bool ShouldDrawRouteGuidanceMarker(const SceneGuidanceContext& context, std::string_view routeId)
{
    if (ShowsAllAuthoredGuidance(context)) {
        return true;
    }

    return routeId == context.activeRouteMarkerId;
}

bool ShouldDrawObjectiveGuidanceMarker(const SceneGuidanceContext& context, std::string_view markerId)
{
    if (ShowsAllAuthoredGuidance(context)) {
        return true;
    }

    if (markerId == "dock-start-marker" || markerId == "office-marker") {
        return context.ferryOfficePhase == FerryOfficeJobPhase::CollectManifest
            || context.ferryOfficePhase == FerryOfficeJobPhase::UseServiceRoute
            || context.ferryOfficePhase == FerryOfficeJobPhase::RestorePower
            || context.ferryOfficePhase == FerryOfficeJobPhase::OpenServiceGate;
    }
    if (markerId == "service-yard-marker") {
        return context.ferryOfficePhase == FerryOfficeJobPhase::UseServiceVehicle
            || context.ferryOfficePhase == FerryOfficeJobPhase::ReachDockRoad;
    }
    if (markerId == "dock-road-marker") {
        return context.ferryOfficePhase == FerryOfficeJobPhase::ReachDockRoad;
    }

    if (!context.activeObjectiveMarkerId.empty()) {
        return markerId == context.activeObjectiveMarkerId;
    }

    return false;
}

bool ShouldDrawInteractableGuidanceMarker(const SceneGuidanceContext& context, const Interactable& interactable)
{
    if (ShowsAllAuthoredGuidance(context)) {
        return true;
    }

    if (context.hasInteractionFocus && context.focusedInteractableId == interactable.id) {
        return true;
    }
    if (!interactable.enabled || interactable.consumed) {
        return false;
    }
    if (interactable.name == FerryOffice::Names::FerryManifest || interactable.name == FerryOffice::Names::FerryOfficeNotice) {
        return true;
    }
    if (interactable.name == FerryOffice::Names::MaintenanceBox) {
        return context.serviceRouteUsed || context.powerRestored;
    }
    if (interactable.name == FerryOffice::Names::WallButton) {
        return context.powerRestored || context.routeOpened;
    }
    if (interactable.name == FerryOffice::Names::ExitMarker) {
        return context.sliceReadyForExit || context.sliceComplete;
    }
    if (interactable.name == FerryOffice::Names::ServiceRunMarker) {
        return context.ferryOfficePhase == FerryOfficeJobPhase::ReachDockRoad
            || context.ferryOfficePhase == FerryOfficeJobPhase::ConfirmServiceRun
            || context.ferryOfficePhase == FerryOfficeJobPhase::Complete;
    }

    return true;
}

bool ShouldDrawTraversalGuidanceMarker(const SceneGuidanceContext& context, const TraversalAffordance& affordance)
{
    if (ShowsAllAuthoredGuidance(context)) {
        return true;
    }

    if ((context.hasTraversalFocus && context.focusedTraversalId == affordance.id)
        || context.activeTraversalId == affordance.id) {
        return true;
    }

    return context.ferryOfficePhase == FerryOfficeJobPhase::UseServiceRoute;
}

bool ShouldDrawVehicleGuidance(const SceneGuidanceContext& context)
{
    if (!context.vehicleAvailable) {
        return false;
    }
    if (context.fullDebug || context.vehicleOccupied) {
        return true;
    }
    if (context.guidancePolicy == SceneGuidancePolicy::AllAuthored) {
        return true;
    }

    return context.ferryOfficePhase == FerryOfficeJobPhase::UseServiceVehicle
        || context.ferryOfficePhase == FerryOfficeJobPhase::ReachDockRoad
        || context.ferryOfficePhase == FerryOfficeJobPhase::ConfirmServiceRun
        || context.ferryOfficePhase == FerryOfficeJobPhase::Complete;
}
