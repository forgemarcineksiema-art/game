#include "engine/core/Clock.h"

namespace engine {

void Clock::reset()
{
    m_start = SteadyClock::now();
    m_previous = m_start;
    m_deltaSeconds = 0.0;
    m_totalSeconds = 0.0;
    m_frameIndex = 0;
}

double Clock::tick()
{
    const auto now = SteadyClock::now();
    m_deltaSeconds = std::chrono::duration<double>(now - m_previous).count();
    m_totalSeconds = std::chrono::duration<double>(now - m_start).count();
    m_previous = now;
    ++m_frameIndex;
    return m_deltaSeconds;
}

double Clock::deltaSeconds() const
{
    return m_deltaSeconds;
}

double Clock::totalSeconds() const
{
    return m_totalSeconds;
}

unsigned long long Clock::frameIndex() const
{
    return m_frameIndex;
}

} // namespace engine

