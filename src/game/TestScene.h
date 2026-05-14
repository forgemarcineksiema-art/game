#pragma once

#include "game/InteractionSystem.h"
#include "game/TestWorld.h"
#include "game/TraversalSystem.h"
#include "game/WorldState.h"

#include <string>

class TestScene {
public:
    TestScene();

    TestWorld& world();
    const TestWorld& world() const;
    InteractionSystem& interactions();
    const InteractionSystem& interactions() const;
    TraversalSystem& traversal();
    const TraversalSystem& traversal() const;
    WorldState& worldState();
    const WorldState& worldState() const;

    bool applyInteractionResult(const InteractionResult& result);
    bool recordServiceRouteUsed();
    bool recordExitReached();

    bool isSliceReadyForExit() const;
    bool isSliceComplete() const;
    bool isServiceGateBlocking() const;
    std::string currentObjectiveText() const;
    std::string completionSummary() const;

private:
    void syncRouteGateCollider();

    TestWorld m_world;
    InteractionSystem m_interactions;
    TraversalSystem m_traversal;
    WorldState m_worldState;
};
