#pragma once

#include "engine/application/Application.h"

class SandboxLayer final : public engine::IGameLayer {
public:
    void onAttach() override;
    void onUpdate(double deltaSeconds, const engine::InputState& input) override;
    void onRender(engine::IRenderer& renderer) override;
    void onDetach() override;
};

