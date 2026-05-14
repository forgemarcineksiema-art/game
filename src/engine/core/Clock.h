#pragma once

#include <chrono>

namespace engine {

class Clock {
public:
    void reset();
    double tick();

    double deltaSeconds() const;
    double totalSeconds() const;
    unsigned long long frameIndex() const;

private:
    using SteadyClock = std::chrono::steady_clock;

    SteadyClock::time_point m_start = SteadyClock::now();
    SteadyClock::time_point m_previous = m_start;
    double m_deltaSeconds = 0.0;
    double m_totalSeconds = 0.0;
    unsigned long long m_frameIndex = 0;
};

} // namespace engine

