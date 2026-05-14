#pragma once

#include "engine/renderer/Renderer.h"

#include <memory>
#include <string>

namespace engine {

std::unique_ptr<IRenderer> CreateRenderer(const std::string& requestedBackend, const RendererConfig& config);

} // namespace engine

