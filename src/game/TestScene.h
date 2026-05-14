#pragma once

#include "game/PlayerController.h"

#include <vector>

class TestScene {
public:
    TestScene();

    const std::vector<PlayerObstacle>& obstacles() const;

private:
    std::vector<PlayerObstacle> m_obstacles;
};
