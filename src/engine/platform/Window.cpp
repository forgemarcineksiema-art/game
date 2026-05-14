#include "engine/platform/Window.h"

#if !defined(_WIN32)

namespace engine {

std::unique_ptr<IWindow> CreatePlatformWindow()
{
    return nullptr;
}

} // namespace engine

#endif
