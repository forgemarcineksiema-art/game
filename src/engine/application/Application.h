#pragma once

#include "engine/core/Config.h"
#include "engine/input/Input.h"
#include "engine/renderer/Renderer.h"

#include <memory>
#include <string>
#include <string_view>

namespace engine {

class IGameLayer {
public:
    virtual ~IGameLayer() = default;

    virtual void onAttach() {}
    virtual void onUpdate(double deltaSeconds, const InputState& input) = 0;
    virtual void onRender(IRenderer& renderer) = 0;
    virtual std::string debugText() const { return {}; }
    virtual void onDetach() {}
};

class Application {
public:
    int run(AppConfig config, std::unique_ptr<IGameLayer> layer);
};

std::string BuildDebugWindowTitle(std::string_view appName, std::string_view debugText);

} // namespace engine
