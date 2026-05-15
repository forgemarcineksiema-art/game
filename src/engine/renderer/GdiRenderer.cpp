#include "engine/renderer/GdiRenderer.h"

#if defined(_WIN32)

#include "engine/core/Logger.h"
#include "engine/math/BoxEdges.h"
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
    m_windowDeviceContext = GetDC(m_window);
    if (!m_windowDeviceContext) {
        return;
    }

    RECT rect {};
    GetClientRect(m_window, &rect);
    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;
    if (width <= 0 || height <= 0 || !ensureBackBuffer(width, height)) {
        ReleaseDC(m_window, m_windowDeviceContext);
        m_windowDeviceContext = nullptr;
        return;
    }

    m_deviceContext = m_backBufferContext;
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

    HPEN pen = acquirePen(ToColorRef(color), 2);
    HPEN oldPen = static_cast<HPEN>(SelectObject(m_deviceContext, pen));
    MoveToEx(m_deviceContext, screenA.x, screenA.y, nullptr);
    LineTo(m_deviceContext, screenB.x, screenB.y);
    SelectObject(m_deviceContext, oldPen);
}

void GdiRenderer::drawDebugSolidBox(Vec3 center, Vec3 halfExtents, Color color)
{
    if (!m_deviceContext) {
        return;
    }

    RECT rect {};
    GetClientRect(m_window, &rect);
    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;
    if (width <= 0 || height <= 0) {
        return;
    }

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

    POINT screenCorners[8] {};
    for (int index = 0; index < 8; ++index) {
        ProjectedPoint projected;
        if (!ProjectWorldPoint(m_debugCamera, AspectRatio(m_config), corners[index], projected)) {
            return;
        }
        screenCorners[index] = ToScreen(projected, width, height);
    }

    const int faces[][4] = {
        {0, 1, 2, 3},
        {4, 5, 6, 7},
        {0, 1, 5, 4},
        {1, 2, 6, 5},
        {2, 3, 7, 6},
        {3, 0, 4, 7},
    };

    HBRUSH brush = acquireBrush(ToColorRef(color));
    HPEN pen = acquirePen(ToColorRef(color), 1);
    HGDIOBJ oldBrush = SelectObject(m_deviceContext, brush);
    HGDIOBJ oldPen = SelectObject(m_deviceContext, pen);
    for (const auto& face : faces) {
        POINT points[] = {
            screenCorners[face[0]],
            screenCorners[face[1]],
            screenCorners[face[2]],
            screenCorners[face[3]],
        };
        Polygon(m_deviceContext, points, 4);
    }
    SelectObject(m_deviceContext, oldPen);
    SelectObject(m_deviceContext, oldBrush);
}

void GdiRenderer::drawDebugFlatTriangles(std::span<const Vec3> triangleVertices, Color color)
{
    if (!m_deviceContext || triangleVertices.size() < 3) {
        return;
    }

    RECT rect {};
    GetClientRect(m_window, &rect);
    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;
    if (width <= 0 || height <= 0) {
        return;
    }

    HBRUSH brush = acquireBrush(ToColorRef(color));
    HPEN pen = acquirePen(ToColorRef(color), 1);
    HGDIOBJ oldBrush = SelectObject(m_deviceContext, brush);
    HGDIOBJ oldPen = SelectObject(m_deviceContext, pen);

    for (std::size_t index = 0; index + 2 < triangleVertices.size(); index += 3) {
        POINT points[3] {};
        bool projectedAll = true;
        for (std::size_t corner = 0; corner < 3; ++corner) {
            ProjectedPoint projected;
            if (!ProjectWorldPoint(m_debugCamera, AspectRatio(m_config), triangleVertices[index + corner], projected)) {
                projectedAll = false;
                break;
            }
            points[corner] = ToScreen(projected, width, height);
        }
        if (projectedAll) {
            Polygon(m_deviceContext, points, 3);
        }
    }

    SelectObject(m_deviceContext, oldPen);
    SelectObject(m_deviceContext, oldBrush);
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

    for (const auto& edge : BoxEdgeIndices) {
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
    if (m_windowDeviceContext && m_backBufferContext) {
        BitBlt(m_windowDeviceContext, 0, 0, m_backBufferWidth, m_backBufferHeight, m_backBufferContext, 0, 0, SRCCOPY);
    }

    if (m_windowDeviceContext) {
        ReleaseDC(m_window, m_windowDeviceContext);
        m_windowDeviceContext = nullptr;
    }
    m_deviceContext = nullptr;
}

void GdiRenderer::shutdown()
{
    releaseBackBuffer();
    releaseGdiObjects();
    Logger::info("GDI fallback renderer shutdown.");
}

std::string GdiRenderer::name() const
{
    return "gdi-fallback";
}

bool GdiRenderer::ensureBackBuffer(int width, int height)
{
    if (m_backBufferContext && m_backBufferBitmap && m_backBufferWidth == width && m_backBufferHeight == height) {
        return true;
    }

    releaseBackBuffer();

    m_backBufferContext = CreateCompatibleDC(m_windowDeviceContext);
    if (!m_backBufferContext) {
        return false;
    }

    m_backBufferBitmap = CreateCompatibleBitmap(m_windowDeviceContext, width, height);
    if (!m_backBufferBitmap) {
        DeleteDC(m_backBufferContext);
        m_backBufferContext = nullptr;
        return false;
    }

    m_previousBackBufferBitmap = static_cast<HBITMAP>(SelectObject(m_backBufferContext, m_backBufferBitmap));
    m_backBufferWidth = width;
    m_backBufferHeight = height;
    return true;
}

void GdiRenderer::releaseBackBuffer()
{
    if (m_backBufferContext && m_previousBackBufferBitmap) {
        SelectObject(m_backBufferContext, m_previousBackBufferBitmap);
    }
    if (m_backBufferBitmap) {
        DeleteObject(m_backBufferBitmap);
    }
    if (m_backBufferContext) {
        DeleteDC(m_backBufferContext);
    }

    m_backBufferContext = nullptr;
    m_backBufferBitmap = nullptr;
    m_previousBackBufferBitmap = nullptr;
    m_backBufferWidth = 0;
    m_backBufferHeight = 0;
}

void GdiRenderer::releaseGdiObjects()
{
    for (auto& entry : m_penCache) {
        DeleteObject(entry.second);
    }
    m_penCache.clear();
    for (auto& entry : m_brushCache) {
        DeleteObject(entry.second);
    }
    m_brushCache.clear();
}

HPEN GdiRenderer::acquirePen(COLORREF color, int width)
{
    const int safeWidth = std::max(width, 1);
    const std::uint64_t key = (static_cast<std::uint64_t>(safeWidth) << 32) | static_cast<std::uint64_t>(color);
    const auto it = m_penCache.find(key);
    if (it != m_penCache.end()) {
        return it->second;
    }

    HPEN pen = CreatePen(PS_SOLID, safeWidth, color);
    m_penCache[key] = pen;
    return pen;
}

HBRUSH GdiRenderer::acquireBrush(COLORREF color)
{
    const auto it = m_brushCache.find(color);
    if (it != m_brushCache.end()) {
        return it->second;
    }

    HBRUSH brush = CreateSolidBrush(color);
    m_brushCache[color] = brush;
    return brush;
}

} // namespace engine

#endif
