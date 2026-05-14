#pragma once

#include <memory>
#include <string_view>

namespace engine {

class IWindow {
public:
    virtual ~IWindow() = default;

    virtual bool create(int width, int height, std::string_view title) = 0;
    virtual void show() = 0;
    virtual void processEvents() = 0;
    virtual bool shouldClose() const = 0;
    virtual void* nativeHandle() const = 0;
    virtual void shutdown() = 0;
};

std::unique_ptr<IWindow> CreatePlatformWindow();

} // namespace engine

