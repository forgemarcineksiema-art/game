#include "SandboxLayer.h"

#include "engine/core/Logger.h"

void SandboxLayer::onAttach()
{
    engine::Logger::info("Sandbox layer attached.");
}

void SandboxLayer::onUpdate(double, const engine::InputState&)
{
}

void SandboxLayer::onRender(engine::IRenderer& renderer)
{
    renderer.drawDebugGridAndAxes();
}

void SandboxLayer::onDetach()
{
    engine::Logger::info("Sandbox layer detached.");
}

