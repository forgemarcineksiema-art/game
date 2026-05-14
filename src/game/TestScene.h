#pragma once

#include "game/InteractionSystem.h"
#include "game/TestWorld.h"

class TestScene {
public:
    TestScene();

    TestWorld& world();
    const TestWorld& world() const;
    InteractionSystem& interactions();
    const InteractionSystem& interactions() const;

private:
    TestWorld m_world;
    InteractionSystem m_interactions;
};
