#pragma once

#include "engine/renderer/Renderer.h"

namespace engine {

class NullRenderer final : public IRenderer {
public:
    bool initialize(const RendererConfig& config) override;
    void beginFrame(unsigned long long frameIndex) override;
    void drawDebugGridAndAxes() override;
    void endFrame() override;
    void shutdown() override;
    std::string name() const override;

    unsigned long long frameCount() const;
    unsigned int debugDrawCount() const;

private:
    RendererConfig m_config;
    unsigned long long m_frameCount = 0;
    unsigned int m_debugDrawCount = 0;
};

} // namespace engine

