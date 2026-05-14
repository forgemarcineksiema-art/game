#pragma once

#include "engine/core/Clock.h"
#include "engine/core/Config.h"

namespace engine {

class Engine {
public:
    bool initialize(const AppConfig& config);
    void shutdown();

    Clock& clock();
    const Clock& clock() const;

private:
    Clock m_clock;
    bool m_initialized = false;
};

} // namespace engine

