#pragma once

#include "engine/renderer/Renderer.h"

#if defined(_WIN32)

#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace engine {

class GdiRenderer final : public IRenderer {
public:
    bool initialize(const RendererConfig& config) override;
    void beginFrame(unsigned long long frameIndex) override;
    void setDebugCamera(const DebugCamera& camera) override;
    void drawDebugGridAndAxes() override;
    void drawDebugLine(Vec3 from, Vec3 to, Color color) override;
    void drawDebugSolidBox(Vec3 center, Vec3 halfExtents, Color color) override;
    void drawDebugFlatTriangles(std::span<const Vec3> triangleVertices, Color color) override;
    void drawDebugBox(Vec3 center, Vec3 halfExtents, Color color) override;
    void drawDebugText(std::string_view text) override;
    void endFrame() override;
    void shutdown() override;
    std::string name() const override;

private:
    RendererConfig m_config;
    DebugCamera m_debugCamera;
    HWND m_window = nullptr;
    HDC m_deviceContext = nullptr;
};

} // namespace engine

#endif
