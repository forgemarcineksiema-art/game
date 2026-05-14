#pragma once

#include "game/InteractionSystem.h"
#include "game/TestWorld.h"
#include "game/TraversalSystem.h"
#include "game/WorldState.h"

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

private:
    TestWorld m_world;
    InteractionSystem m_interactions;
    TraversalSystem m_traversal;
    WorldState m_worldState;
};
