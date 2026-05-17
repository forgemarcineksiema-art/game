#pragma once

#include "game/FerryOfficeData.h"
#include "game/FerryOfficeJob.h"
#include "game/InteractionSystem.h"
#include "game/PrototypeWorld.h"
#include "game/TargetSliceObjectiveRuntime.h"
#include "game/TraversalSystem.h"
#include "game/WorldState.h"

#include <string>
#include <string_view>
#include <vector>

struct SceneDefinition;
struct SceneInteractableDefinition;
struct SceneTraversalAffordanceDefinition;

struct InteractableActionBinding {
    std::string name;
    std::vector<WorldFlag> worldFlagsSet;
    std::vector<WorldFlag> requiredWorldFlags;
    std::vector<WorldFlag> worldFlagsSetWhenReady;
};

struct TraversalActionBinding {
    int affordanceId = 0;
    std::string source;
    std::vector<WorldFlag> worldFlagsSetOnComplete;
};

class PrototypeScene {
public:
    PrototypeScene();
    explicit PrototypeScene(const SceneDefinition& sceneDefinition);

    PrototypeWorld& world();
    const PrototypeWorld& world() const;
    InteractionSystem& interactions();
    const InteractionSystem& interactions() const;
    TraversalSystem& traversal();
    const TraversalSystem& traversal() const;
    WorldState& worldState();
    const WorldState& worldState() const;
    FerryOfficeJob& job();
    const FerryOfficeJob& job() const;
    void loadFromDefinition(const SceneDefinition& sceneDefinition);

    bool applyInteractionResult(const InteractionResult& result);
    bool recordServiceRouteUsed();
    bool recordTraversalCompleted(int affordanceId);
    bool recordServiceVehicleUsed();
    bool updateJobVehicleCheckpoint(engine::Vec3 vehiclePosition, bool vehicleOccupied);
    bool recordExitReached();
    void syncWorldStateColliders();

    bool isJobComplete() const;
    bool isSliceReadyForExit() const;
    bool isSliceComplete() const;
    bool isServiceGateBlocking() const;
    std::string currentObjectiveText() const;
    std::string currentJobObjectiveText() const;
    std::string completionSummary() const;
    std::string jobDebugSummary() const;
    std::string lastRuntimeEventText() const;

private:
    void buildFromFerryOfficeData();
    void addInteractableActionBinding(
        std::string name,
        std::vector<WorldFlag> worldFlagsSet,
        std::vector<WorldFlag> requiredWorldFlags,
        std::vector<WorldFlag> worldFlagsSetWhenReady);
    void addInteractableActionBinding(const SceneInteractableDefinition& interactable);
    void addTraversalActionBinding(int affordanceId, std::string source, std::vector<WorldFlag> worldFlagsSetOnComplete);
    void addTraversalActionBinding(int affordanceId, const SceneTraversalAffordanceDefinition& affordance);
    bool applyAuthoredInteractionBinding(std::string_view name, const std::string& source);
    bool hasRequiredWorldFlags(const InteractableActionBinding& binding) const;
    void configureJobFromDefinition(const SceneDefinition& sceneDefinition);
    bool recordTargetSliceInteraction(std::string_view interactableName);

    PrototypeWorld m_world;
    InteractionSystem m_interactions;
    TraversalSystem m_traversal;
    WorldState m_worldState;
    FerryOfficeJob m_job;
    std::vector<InteractableActionBinding> m_interactableActionBindings;
    std::vector<TraversalActionBinding> m_traversalActionBindings;
    SceneDefinition m_sceneDefinition;
    TargetSliceObjectiveState m_targetSliceObjective;
    std::string m_completedTargetSliceInteractableName;
    std::string m_completedTargetSliceRiskyInteractableName;
    std::string m_completedTargetSliceExitInteractableName;
    std::string m_lastTargetSliceEventText = "none";
    bool m_ferryOfficeBehaviorEnabled = true;
};
