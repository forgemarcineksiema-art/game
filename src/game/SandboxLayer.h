#pragma once

#include "engine/application/Application.h"
#include "game/PlayerController.h"
#include "game/TestScene.h"
#include "game/ThirdPersonCamera.h"

#include <string>

class SandboxLayer final : public engine::IGameLayer {
public:
    void onAttach() override;
    void onUpdate(double deltaSeconds, const engine::InputState& input) override;
    void onRender(engine::IRenderer& renderer) override;
    std::string debugText() const override;
    void onDetach() override;

private:
    void updateDebugText();

    TestScene m_scene;
    PlayerController m_player;
    ThirdPersonCamera m_camera;
    std::string m_debugText;
    unsigned long long m_frameIndex = 0;
};
