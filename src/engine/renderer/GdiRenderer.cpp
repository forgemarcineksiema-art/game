#include "engine/renderer/GdiRenderer.h"

#if defined(_WIN32)

#include "engine/core/Logger.h"
#include "engine/renderer/DebugProjection.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace engine {
namespace {

int ToByte(float value)
{
    return static_cast<int>(std::clamp(value, 0.0f, 1.0f) * 255.0f);
}

COLORREF ToColorRef(const Color& color)
{
    return RGB(ToByte(color.r), ToByte(color.g), ToByte(color.b));
}

float AspectRatio(const RendererConfig& config)
{
    return static_cast<float>(std::max(config.width, 1)) / static_cast<float>(std::max(config.height, 1));
}

POINT ToScreen(const ProjectedPoint& point, int width, int height)
{
    return {
        static_cast<LONG>((point.x * 0.5f + 0.5f) * static_cast<float>(width)),
        static_cast<LONG>((0.5f - point.y * 0.5f) * static_cast<float>(height)),
    };
}

} // namespace

bool GdiRenderer::initialize(const RendererConfig& config)
{
    m_config = config;
    m_window = static_cast<HWND>(config.nativeWindow);
    if (!m_window) {
        Logger::error("GDI renderer requires a Win32 window handle.");
        return false;
    }

    Logger::info("GDI fallback renderer initialized.");
    return true;
}

void GdiRenderer::setDebugCamera(const DebugCamera& camera)
{
    m_debugCamera = camera;
}

void GdiRenderer::beginFrame(unsigned long long)
{
    m_deviceContext = GetDC(m_window);
    if (!m_deviceContext) {
        return;
    }

    RECT rect {};
    GetClientRect(m_window, &rect);
    HBRUSH brush = CreateSolidBrush(ToColorRef(m_config.clearColor));
    FillRect(m_deviceContext, &rect, brush);
    DeleteObject(brush);
    SetBkMode(m_deviceContext, TRANSPARENT);
}

void GdiRenderer::drawDebugGridAndAxes()
{
    if (!m_deviceContext) {
        return;
    }

    for (int line = -10; line <= 10; ++line) {
        const float p = static_cast<float>(line);
        drawDebugLine({p, 0.0f, -10.0f}, {p, 0.0f, 10.0f}, {0.22f, 0.28f, 0.36f, 1.0f});
        drawDebugLine({-10.0f, 0.0f, p}, {10.0f, 0.0f, p}, {0.22f, 0.28f, 0.36f, 1.0f});
    }
    drawDebugLine({-10.0f, 0.02f, 0.0f}, {10.0f, 0.02f, 0.0f}, {0.95f, 0.24f, 0.24f, 1.0f});
    drawDebugLine({0.0f, 0.02f, -10.0f}, {0.0f, 0.02f, 10.0f}, {0.24f, 0.85f, 0.38f, 1.0f});
}

void GdiRenderer::drawDebugLine(Vec3 from, Vec3 to, Color color)
{
    if (!m_deviceContext) {
        return;
    }

    RECT rect {};
    GetClientRect(m_window, &rect);
    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;

    ProjectedPoint a;
    ProjectedPoint b;
    if (!ProjectWorldPoint(m_debugCamera, AspectRatio(m_config), from, a)
        || !ProjectWorldPoint(m_debugCamera, AspectRatio(m_config), to, b)) {
        return;
    }

    const POINT screenA = ToScreen(a, width, height);
    const POINT screenB = ToScreen(b, width, height);

    HPEN pen = CreatePen(PS_SOLID, 2, ToColorRef(color));
    HPEN oldPen = static_cast<HPEN>(SelectObject(m_deviceContext, pen));
    MoveToEx(m_deviceContext, screenA.x, screenA.y, nullptr);
    LineTo(m_deviceContext, screenB.x, screenB.y);
    SelectObject(m_deviceContext, oldPen);
    DeleteObject(pen);
}

void GdiRenderer::drawDebugBox(Vec3 center, Vec3 halfExtents, Color color)
{
    const Vec3 corners[] = {
        center + Vec3 {-halfExtents.x, -halfExtents.y, -halfExtents.z},
        center + Vec3 { halfExtents.x, -halfExtents.y, -halfExtents.z},
        center + Vec3 { halfExtents.x, -halfExtents.y,  halfExtents.z},
        center + Vec3 {-halfExtents.x, -halfExtents.y,  halfExtents.z},
        center + Vec3 {-halfExtents.x,  halfExtents.y, -halfExtents.z},
        center + Vec3 { halfExtents.x,  halfExtents.y, -halfExtents.z},
        center + Vec3 { halfExtents.x,  halfExtents.y,  halfExtents.z},
        center + Vec3 {-halfExtents.x,  halfExtents.y,  halfExtents.z},
    };

    const int edges[][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7},
    };

    for (const auto& edge : edges) {
        drawDebugLine(corners[edge[0]], corners[edge[1]], color);
    }
}

void GdiRenderer::drawDebugText(std::string_view text)
{
    if (!m_deviceContext || text.empty()) {
        return;
    }

    RECT rect {};
    GetClientRect(m_window, &rect);
    SetTextColor(m_deviceContext, RGB(230, 235, 245));
    std::string copy(text);
    RECT textRect {16, 16, rect.right - 16, rect.bottom - 16};
    DrawTextA(m_deviceContext, copy.c_str(), static_cast<int>(copy.size()), &textRect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOCLIP);
}

void GdiRenderer::endFrame()
{
    if (m_deviceContext) {
        ReleaseDC(m_window, m_deviceContext);
        m_deviceContext = nullptr;
    }
}

void GdiRenderer::shutdown()
{
    Logger::info("GDI fallback renderer shutdown.");
}

std::string GdiRenderer::name() const
{
    return "gdi-fallback";
}

} // namespace engine

#endif
