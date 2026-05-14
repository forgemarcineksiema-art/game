#pragma once

#include "engine/renderer/RendererTypes.h"

#include <string>
#include <string_view>

namespace engine {

class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual bool initialize(const RendererConfig& config) = 0;
    virtual void beginFrame(unsigned long long frameIndex) = 0;
    virtual void setDebugCamera(const DebugCamera& camera) { (void)camera; }
    virtual void drawDebugGridAndAxes() = 0;
    virtual void drawDebugLine(Vec3 from, Vec3 to, Color color) { (void)from; (void)to; (void)color; }
    virtual void drawDebugBox(Vec3 center, Vec3 halfExtents, Color color) { (void)center; (void)halfExtents; (void)color; }
    virtual void drawDebugText(std::string_view text) { (void)text; }
    virtual void endFrame() = 0;
    virtual void shutdown() = 0;
    virtual std::string name() const = 0;
};

} // namespace engine
