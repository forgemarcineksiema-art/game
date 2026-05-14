#pragma once

#include "engine/renderer/RendererTypes.h"

#include <string>

namespace engine {

class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual bool initialize(const RendererConfig& config) = 0;
    virtual void beginFrame(unsigned long long frameIndex) = 0;
    virtual void drawDebugGridAndAxes() = 0;
    virtual void endFrame() = 0;
    virtual void shutdown() = 0;
    virtual std::string name() const = 0;
};

} // namespace engine

