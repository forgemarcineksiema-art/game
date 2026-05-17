#pragma once

#include "game/FerryOfficeJob.h"
#include "game/InteractionSystem.h"
#include "game/SceneRuntimePolicy.h"
#include "game/TraversalSystem.h"

#include <string>
#include <string_view>

struct SceneGuidanceContext {
    bool fullDebug = false;
    SceneGuidancePolicy guidancePolicy = SceneGuidancePolicy::FerryOfficeActive;
    FerryOfficeJobPhase ferryOfficePhase = FerryOfficeJobPhase::CollectManifest;
    std::string activeRouteMarkerId;
    std::string activeObjectiveMarkerId;
    bool hasInteractionFocus = false;
    int focusedInteractableId = 0;
    bool hasTraversalFocus = false;
    int focusedTraversalId = 0;
    int activeTraversalId = 0;
    bool serviceRouteUsed = false;
    bool powerRestored = false;
    bool routeOpened = false;
    bool sliceReadyForExit = false;
    bool sliceComplete = false;
    bool jobComplete = false;
    bool serviceRunReadyForConfirmation = false;
    bool vehicleAvailable = false;
    bool vehicleOccupied = false;
};

bool ShouldDrawRouteGuidanceMarker(const SceneGuidanceContext& context, std::string_view routeId);
bool ShouldDrawObjectiveGuidanceMarker(const SceneGuidanceContext& context, std::string_view markerId);
bool ShouldDrawInteractableGuidanceMarker(const SceneGuidanceContext& context, const Interactable& interactable);
bool ShouldDrawTraversalGuidanceMarker(const SceneGuidanceContext& context, const TraversalAffordance& affordance);
bool ShouldDrawVehicleGuidance(const SceneGuidanceContext& context);
