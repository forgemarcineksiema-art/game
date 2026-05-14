#pragma once

#include "game/InteractionSystem.h"
#include "game/TestWorld.h"
#include "game/TraversalSystem.h"

class TestScene {
public:
    TestScene();

    TestWorld& world();
    const TestWorld& world() const;
    InteractionSystem& interactions();
    const InteractionSystem& interactions() const;
    TraversalSystem& traversal();
    const TraversalSystem& traversal() const;

private:
    TestWorld m_world;
    InteractionSystem m_interactions;
    TraversalSystem m_traversal;
};
