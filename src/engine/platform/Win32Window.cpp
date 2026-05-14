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
        m_hasFocus = GetForegroundWindow() == m_window;
        updateCursorState();
    }

    void setCursorCaptured(bool captured) override
    {
        m_cursorCaptureRequested = captured;
        updateCursorState();
    }

    void processEvents() override
    {
        m_input.mouseDeltaX = 0.0f;
        m_input.mouseDeltaY = 0.0f;
        m_input.cameraYawDelta = 0.0f;
        m_input.cameraPitchDelta = 0.0f;
        m_input.jumpPressed = false;
        m_input.interactPressed = false;

        MSG message {};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        refreshCursorClip();

        m_input.moveForward = AxisFromKeys('W', 'S');
        m_input.moveRight = AxisFromKeys('D', 'A');
        m_input.sprintHeld = KeyDown(VK_SHIFT);
        const bool jumpDown = KeyDown(VK_SPACE);
        const bool interactDown = KeyDown('E');
        m_input.jumpPressed = jumpDown && !m_previousJumpDown;
        m_input.interactHeld = interactDown;
        m_input.interactPressed = interactDown && !m_previousInteractDown;
        m_input.quitRequested = m_shouldClose || KeyDown(VK_ESCAPE);
        m_previousJumpDown = jumpDown;
        m_previousInteractDown = interactDown;

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
        releaseCursor();
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
                self->releaseCursor();
                DestroyWindow(window);
                return 0;
            case WM_DESTROY:
                self->m_shouldClose = true;
                self->releaseCursor();
                return 0;
            case WM_ACTIVATE:
                self->m_hasFocus = LOWORD(wParam) != WA_INACTIVE;
                self->updateCursorState();
                break;
            case WM_MOUSEACTIVATE:
                self->m_hasFocus = true;
                self->updateCursorState();
                return MA_ACTIVATE;
            case WM_SETFOCUS:
                self->m_hasFocus = true;
                self->updateCursorState();
                return 0;
            case WM_KILLFOCUS:
                self->m_hasFocus = false;
                self->releaseCursor();
                return 0;
            case WM_MOUSEMOVE: {
                const int x = GET_X_LPARAM(lParam);
                const int y = GET_Y_LPARAM(lParam);
                self->handleMouseMove(x, y);
                return 0;
            }
            default:
                break;
            }
        }

        return DefWindowProcW(window, message, wParam, lParam);
    }

    void handleMouseMove(int x, int y)
    {
        if (m_cursorCapturedActive) {
            const POINT center = clientCenter();
            const int deltaX = x - center.x;
            const int deltaY = y - center.y;
            if (deltaX != 0 || deltaY != 0) {
                m_input.mouseDeltaX += static_cast<float>(deltaX);
                m_input.mouseDeltaY += static_cast<float>(deltaY);
                centerCursor();
            }
            m_lastMouseX = center.x;
            m_lastMouseY = center.y;
            m_hasMousePosition = true;
            return;
        }

        if (m_hasMousePosition) {
            m_input.mouseDeltaX += static_cast<float>(x - m_lastMouseX);
            m_input.mouseDeltaY += static_cast<float>(y - m_lastMouseY);
        }
        m_lastMouseX = x;
        m_lastMouseY = y;
        m_hasMousePosition = true;
    }

    void updateCursorState()
    {
        const bool shouldCapture = m_cursorCaptureRequested && m_hasFocus && m_window != nullptr;
        if (shouldCapture == m_cursorCapturedActive) {
            return;
        }

        if (shouldCapture) {
            captureCursor();
        } else {
            releaseCursor();
        }
    }

    void captureCursor()
    {
        if (!m_window) {
            return;
        }

        m_cursorCapturedActive = true;
        setCursorVisible(false);
        refreshCursorClip();
        centerCursor();
        Logger::info("Cursor captured for windowed play.");
    }

    void releaseCursor()
    {
        if (!m_cursorCapturedActive && !m_cursorHidden) {
            return;
        }

        ClipCursor(nullptr);
        setCursorVisible(true);
        m_cursorCapturedActive = false;
        m_hasMousePosition = false;
        Logger::info("Cursor released.");
    }

    void refreshCursorClip()
    {
        if (!m_cursorCapturedActive || !m_window) {
            return;
        }

        RECT clientRect {};
        if (!GetClientRect(m_window, &clientRect)) {
            return;
        }

        POINT topLeft {clientRect.left, clientRect.top};
        POINT bottomRight {clientRect.right, clientRect.bottom};
        ClientToScreen(m_window, &topLeft);
        ClientToScreen(m_window, &bottomRight);

        RECT clipRect {topLeft.x, topLeft.y, bottomRight.x, bottomRight.y};
        ClipCursor(&clipRect);
    }

    POINT clientCenter() const
    {
        RECT clientRect {};
        GetClientRect(m_window, &clientRect);
        POINT center {
            (clientRect.right - clientRect.left) / 2,
            (clientRect.bottom - clientRect.top) / 2,
        };
        return center;
    }

    void centerCursor()
    {
        if (!m_window) {
            return;
        }

        POINT center = clientCenter();
        POINT screenCenter = center;
        ClientToScreen(m_window, &screenCenter);
        SetCursorPos(screenCenter.x, screenCenter.y);
        m_lastMouseX = center.x;
        m_lastMouseY = center.y;
        m_hasMousePosition = true;
    }

    void setCursorVisible(bool visible)
    {
        if (visible && m_cursorHidden) {
            while (ShowCursor(TRUE) < 0) {
            }
            m_cursorHidden = false;
        } else if (!visible && !m_cursorHidden) {
            while (ShowCursor(FALSE) >= 0) {
            }
            m_cursorHidden = true;
        }
    }

    HWND m_window = nullptr;
    bool m_shouldClose = false;
    InputState m_input;
    bool m_hasFocus = false;
    bool m_cursorCaptureRequested = false;
    bool m_cursorCapturedActive = false;
    bool m_cursorHidden = false;
    bool m_hasMousePosition = false;
    int m_lastMouseX = 0;
    int m_lastMouseY = 0;
    bool m_previousJumpDown = false;
    bool m_previousInteractDown = false;
};

std::unique_ptr<IWindow> CreatePlatformWindow()
{
    return std::make_unique<Win32Window>();
}

} // namespace engine

#endif
