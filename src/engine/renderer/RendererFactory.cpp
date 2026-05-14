#include "engine/renderer/RendererFactory.h"

#include "engine/core/Logger.h"
#include "engine/renderer/NullRenderer.h"

#if defined(_WIN32)
#include "engine/renderer/Dx11Renderer.h"
#include "engine/renderer/GdiRenderer.h"
#endif

namespace engine {

std::unique_ptr<IRenderer> CreateRenderer(const std::string& requestedBackend, const RendererConfig& config)
{
    if (!config.headless && (requestedBackend == "auto" || requestedBackend == "dx11")) {
#if defined(_WIN32)
        if (config.nativeWindow != nullptr) {
            return std::make_unique<Dx11Renderer>();
        }
#else
        Logger::warning("DirectX 11 renderer is only available on Windows.");
#endif
    }

    if (!config.headless && requestedBackend == "gdi") {
#if defined(_WIN32)
        if (config.nativeWindow != nullptr) {
            return std::make_unique<GdiRenderer>();
        }
#else
        Logger::warning("No platform window renderer is available on this platform.");
#endif
    }

    return std::make_unique<NullRenderer>();
}

} // namespace engine
