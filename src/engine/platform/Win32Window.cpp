#include "engine/platform/Window.h"

#if defined(_WIN32)

#include "engine/core/Logger.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>

namespace engine {
namespace {

constexpr wchar_t WindowClassName[] = L"AINativeEngineFoundationWindow";

std::wstring ToWide(std::string_view text)
{
    return std::wstring(text.begin(), text.end());
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
        MSG message {};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
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
            default:
                break;
            }
        }

        return DefWindowProcW(window, message, wParam, lParam);
    }

    HWND m_window = nullptr;
    bool m_shouldClose = false;
};

std::unique_ptr<IWindow> CreatePlatformWindow()
{
    return std::make_unique<Win32Window>();
}

} // namespace engine

#endif

