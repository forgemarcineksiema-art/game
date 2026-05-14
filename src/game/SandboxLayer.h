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
    void drawInteractionDebug(engine::IRenderer& renderer);
    void drawTraversalDebug(engine::IRenderer& renderer);

    TestScene m_scene;
    PlayerController m_player;
    ThirdPersonCamera m_camera;
    std::string m_debugText;
    std::string m_lastInteractionText = "none";
    bool m_interactPressedThisFrame = false;
    bool m_traversalPressedThisFrame = false;
    unsigned long long m_frameIndex = 0;
};
