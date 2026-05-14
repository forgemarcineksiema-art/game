#include "game/TestScene.h"

TestScene::TestScene()
{
    m_obstacles = {
        {{3.0f, 0.5f, 2.0f}, {0.75f, 0.5f, 0.75f}},
        {{-2.5f, 0.75f, 3.5f}, {1.0f, 0.75f, 0.5f}},
        {{0.0f, 0.6f, 6.0f}, {2.0f, 0.6f, 0.35f}},
    };
}

const std::vector<PlayerObstacle>& TestScene::obstacles() const
{
    return m_obstacles;
}
