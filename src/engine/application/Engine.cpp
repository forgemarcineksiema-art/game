#include "engine/application/Engine.h"

#include "engine/core/Logger.h"

namespace engine {

bool Engine::initialize(const AppConfig& config)
{
    if (m_initialized) {
        return true;
    }

    Logger::info("Initializing engine v" ENGINE_VERSION_STRING);
    Logger::info("App: " + config.appName);
    m_clock.reset();
    m_initialized = true;
    return true;
}

void Engine::shutdown()
{
    if (!m_initialized) {
        return;
    }

    Logger::info("Shutting down engine.");
    m_initialized = false;
}

Clock& Engine::clock()
{
    return m_clock;
}

const Clock& Engine::clock() const
{
    return m_clock;
}

} // namespace engine

