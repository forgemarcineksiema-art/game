#include "engine/renderer/NullRenderer.h"

#include "engine/core/Logger.h"

namespace engine {

bool NullRenderer::initialize(const RendererConfig& config)
{
    m_config = config;
    Logger::info("Null renderer initialized.");
    return true;
}

void NullRenderer::beginFrame(unsigned long long frameIndex)
{
    m_frameCount = frameIndex;
}

void NullRenderer::drawDebugGridAndAxes()
{
    ++m_debugDrawCount;
}

void NullRenderer::drawDebugLine(Vec3, Vec3, Color)
{
    ++m_debugDrawCount;
}

void NullRenderer::drawDebugSolidBox(Vec3, Vec3, Color)
{
    ++m_debugDrawCount;
}

void NullRenderer::drawDebugBox(Vec3, Vec3, Color)
{
    ++m_debugDrawCount;
}

void NullRenderer::drawDebugText(std::string_view)
{
    ++m_debugDrawCount;
}

void NullRenderer::endFrame()
{
}

void NullRenderer::shutdown()
{
    Logger::info("Null renderer shutdown.");
}

std::string NullRenderer::name() const
{
    return "null";
}

unsigned long long NullRenderer::frameCount() const
{
    return m_frameCount;
}

unsigned int NullRenderer::debugDrawCount() const
{
    return m_debugDrawCount;
}

} // namespace engine
