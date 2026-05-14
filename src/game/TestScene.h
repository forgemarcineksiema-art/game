#pragma once

#include "game/TestWorld.h"

class TestScene {
public:
    TestScene();

    const TestWorld& world() const;

private:
    TestWorld m_world;
};
