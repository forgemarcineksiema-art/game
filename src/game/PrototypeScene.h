#pragma once

#include "game/FerryOfficeData.h"
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
    void loadFromDefinition(const SceneDefinition& sceneDefinition);

    bool applyInteractionResult(const InteractionResult& result);
    bool recordServiceRouteUsed();
    bool recordExitReached();

    bool isSliceReadyForExit() const;
    bool isSliceComplete() const;
    bool isServiceGateBlocking() const;
    std::string currentObjectiveText() const;
    std::string completionSummary() const;

private:
    void buildFromFerryOfficeData();
    void syncRouteGateCollider();

    PrototypeWorld m_world;
    InteractionSystem m_interactions;
    TraversalSystem m_traversal;
    WorldState m_worldState;
};
