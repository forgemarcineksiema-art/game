#include "engine/platform/Window.h"

#if defined(_WIN32)

#include "engine/core/Logger.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <algorithm>
#include <string>

namespace engine {
namespace {

constexpr wchar_t WindowClassName[] = L"AINativeEngineFoundationWindow";

std::wstring ToWide(std::string_view text)
{
    return std::wstring(text.begin(), text.end());
}

bool KeyDown(int virtualKey)
{
    return (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
}

float AxisFromKeys(int positiveKey, int negativeKey)
{
    return (KeyDown(positiveKey) ? 1.0f : 0.0f) - (KeyDown(negativeKey) ? 1.0f : 0.0f);
}

} // namespace

class Win32Window final : public IWindow {
public:
    bool create(int width, int height, std::string_view title) override
    {
        HINSTANCE instance = GetModuleHandleW(nullptr);

        WNDCLASSEXW windowClass {};
        windowClass.cbSize = sizeof(WNDCLASSEXW);
        windowClass.lpfnWndProc = &Win32Window::WindowProc;
        windowClass.hInstance = instance;
        windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        windowClass.lpszClassName = WindowClassName;

        RegisterClassExW(&windowClass);

        RECT desiredRect {0, 0, width, height};
        AdjustWindowRect(&desiredRect, WS_OVERLAPPEDWINDOW, FALSE);

        const auto wideTitle = ToWide(title);
        m_window = CreateWindowExW(
            0,
            WindowClassName,
            wideTitle.c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            desiredRect.right - desiredRect.left,
            desiredRect.bottom - desiredRect.top,
            nullptr,
            nullptr,
            instance,
            this);

        if (!m_window) {
            Logger::error("Win32 window creation failed.");
            return false;
        }

        Logger::info("Win32 window created.");
        return true;
    }

    void show() override
    {
        ShowWindow(m_window, SW_SHOW);
        UpdateWindow(m_window);
    }

    void processEvents() override
    {
        m_input.mouseDeltaX = 0.0f;
        m_input.mouseDeltaY = 0.0f;
        m_input.cameraYawDelta = 0.0f;
        m_input.cameraPitchDelta = 0.0f;
        m_input.jumpPressed = false;

        MSG message {};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        m_input.moveForward = AxisFromKeys('W', 'S');
        m_input.moveRight = AxisFromKeys('D', 'A');
        m_input.sprintHeld = KeyDown(VK_SHIFT);
        m_input.jumpPressed = KeyDown(VK_SPACE);
        m_input.quitRequested = m_shouldClose || KeyDown(VK_ESCAPE);

        const float arrowYaw = AxisFromKeys(VK_RIGHT, VK_LEFT) * 12.0f;
        const float arrowPitch = AxisFromKeys(VK_UP, VK_DOWN) * 12.0f;
        m_input.cameraYawDelta += m_input.mouseDeltaX + arrowYaw;
        m_input.cameraPitchDelta += m_input.mouseDeltaY + arrowPitch;

        if (m_input.quitRequested) {
            m_shouldClose = true;
        }
    }

    InputState inputState() const override
    {
        return m_input;
    }

    void setTitle(std::string_view title) override
    {
        if (m_window) {
            const auto wideTitle = ToWide(title);
            SetWindowTextW(m_window, wideTitle.c_str());
        }
    }

    bool shouldClose() const override
    {
        return m_shouldClose;
    }

    void* nativeHandle() const override
    {
        return m_window;
    }

    void shutdown() override
    {
        if (m_window) {
            DestroyWindow(m_window);
            m_window = nullptr;
        }
    }

private:
    static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
    {
        Win32Window* self = nullptr;

        if (message == WM_NCCREATE) {
            auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
            self = static_cast<Win32Window*>(createStruct->lpCreateParams);
            SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        } else {
            self = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(window, GWLP_USERDATA));
        }

        if (self) {
            switch (message) {
            case WM_CLOSE:
                self->m_shouldClose = true;
                DestroyWindow(window);
                return 0;
            case WM_DESTROY:
                self->m_shouldClose = true;
                return 0;
            case WM_MOUSEMOVE: {
                const int x = GET_X_LPARAM(lParam);
                const int y = GET_Y_LPARAM(lParam);
                if (self->m_hasMousePosition) {
                    self->m_input.mouseDeltaX += static_cast<float>(x - self->m_lastMouseX);
                    self->m_input.mouseDeltaY += static_cast<float>(y - self->m_lastMouseY);
                }
                self->m_lastMouseX = x;
                self->m_lastMouseY = y;
                self->m_hasMousePosition = true;
                return 0;
            }
            default:
                break;
            }
        }

        return DefWindowProcW(window, message, wParam, lParam);
    }

    HWND m_window = nullptr;
    bool m_shouldClose = false;
    InputState m_input;
    bool m_hasMousePosition = false;
    int m_lastMouseX = 0;
    int m_lastMouseY = 0;
};

std::unique_ptr<IWindow> CreatePlatformWindow()
{
    return std::make_unique<Win32Window>();
}

} // namespace engine

#endif
