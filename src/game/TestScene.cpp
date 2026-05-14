#include "game/TestScene.h"

TestScene::TestScene()
{
    m_world.buildDefaultCollisionTestLayout();
}

const TestWorld& TestScene::world() const
{
    return m_world;
}
