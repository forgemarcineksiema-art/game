#pragma once

#include "game/FerryOfficeData.h"
#include "game/FerryOfficeJob.h"
#include "game/InteractionSystem.h"
#include "game/PrototypeWorld.h"
#include "game/TraversalSystem.h"
#include "game/WorldState.h"

#include <string>

struct SceneDefinition;

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
    bool recordServiceVehicleUsed();
    bool updateJobVehicleCheckpoint(engine::Vec3 vehiclePosition, bool vehicleOccupied);
    bool recordExitReached();

    bool isJobComplete() const;
    bool isSliceReadyForExit() const;
    bool isSliceComplete() const;
    bool isServiceGateBlocking() const;
    std::string currentObjectiveText() const;
    std::string currentJobObjectiveText() const;
    std::string completionSummary() const;
    std::string jobDebugSummary() const;

private:
    void buildFromFerryOfficeData();
    void configureJobFromDefinition(const SceneDefinition& sceneDefinition);
    void syncRouteGateCollider();

    PrototypeWorld m_world;
    InteractionSystem m_interactions;
    TraversalSystem m_traversal;
    WorldState m_worldState;
    FerryOfficeJob m_job;
};
