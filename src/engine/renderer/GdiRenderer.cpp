#include "engine/renderer/GdiRenderer.h"

#if defined(_WIN32)

#include "engine/core/Logger.h"

#include <algorithm>
#include <cmath>

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

    RECT rect {};
    GetClientRect(m_window, &rect);
    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;
    const int centerX = width / 2;
    const int centerY = height / 2;

    HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(53, 65, 82));
    HPEN oldPen = static_cast<HPEN>(SelectObject(m_deviceContext, gridPen));
    for (int x = centerX % 40; x < width; x += 40) {
        MoveToEx(m_deviceContext, x, 0, nullptr);
        LineTo(m_deviceContext, x, height);
    }
    for (int y = centerY % 40; y < height; y += 40) {
        MoveToEx(m_deviceContext, 0, y, nullptr);
        LineTo(m_deviceContext, width, y);
    }
    SelectObject(m_deviceContext, oldPen);
    DeleteObject(gridPen);

    HPEN xAxis = CreatePen(PS_SOLID, 3, RGB(224, 80, 80));
    oldPen = static_cast<HPEN>(SelectObject(m_deviceContext, xAxis));
    MoveToEx(m_deviceContext, centerX - 240, centerY, nullptr);
    LineTo(m_deviceContext, centerX + 240, centerY);
    SelectObject(m_deviceContext, oldPen);
    DeleteObject(xAxis);

    HPEN yAxis = CreatePen(PS_SOLID, 3, RGB(80, 200, 120));
    oldPen = static_cast<HPEN>(SelectObject(m_deviceContext, yAxis));
    MoveToEx(m_deviceContext, centerX, centerY + 180, nullptr);
    LineTo(m_deviceContext, centerX, centerY - 180);
    SelectObject(m_deviceContext, oldPen);
    DeleteObject(yAxis);

    HPEN primitivePen = CreatePen(PS_SOLID, 2, RGB(96, 160, 255));
    HBRUSH primitiveBrush = CreateSolidBrush(RGB(96, 160, 255));
    oldPen = static_cast<HPEN>(SelectObject(m_deviceContext, primitivePen));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(m_deviceContext, primitiveBrush));
    POINT triangle[] = {
        {centerX, centerY - 110},
        {centerX - 90, centerY + 70},
        {centerX + 90, centerY + 70}
    };
    Polygon(m_deviceContext, triangle, 3);
    SelectObject(m_deviceContext, oldBrush);
    SelectObject(m_deviceContext, oldPen);
    DeleteObject(primitiveBrush);
    DeleteObject(primitivePen);
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

