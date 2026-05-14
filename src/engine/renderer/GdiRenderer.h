#pragma once

#include "engine/renderer/Renderer.h"

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace engine {

class GdiRenderer final : public IRenderer {
public:
    bool initialize(const RendererConfig& config) override;
    void beginFrame(unsigned long long frameIndex) override;
    void drawDebugGridAndAxes() override;
    void endFrame() override;
    void shutdown() override;
    std::string name() const override;

private:
    RendererConfig m_config;
    HWND m_window = nullptr;
    HDC m_deviceContext = nullptr;
};

} // namespace engine

#endif

