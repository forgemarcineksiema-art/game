#include "engine/core/Logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

namespace engine {
namespace {

std::mutex& LogMutex()
{
    static std::mutex mutex;
    return mutex;
}

std::string Timestamp()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime {};

#if defined(_WIN32)
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif

    std::ostringstream output;
    output << std::put_time(&localTime, "%H:%M:%S");
    return output.str();
}

} // namespace

bool& Logger::quietFlag()
{
    static bool quiet = false;
    return quiet;
}

void Logger::setQuiet(bool quiet)
{
    quietFlag() = quiet;
}

void Logger::write(LogLevel level, std::string_view message)
{
    if (quietFlag() && level == LogLevel::Info) {
        return;
    }

    std::lock_guard<std::mutex> lock(LogMutex());
    std::ostream& stream = level == LogLevel::Error ? std::cerr : std::cout;
    stream << "[" << Timestamp() << "] [" << ToString(level) << "] " << message << '\n';
}

void Logger::info(std::string_view message)
{
    write(LogLevel::Info, message);
}

void Logger::warning(std::string_view message)
{
    write(LogLevel::Warning, message);
}

void Logger::error(std::string_view message)
{
    write(LogLevel::Error, message);
}

std::string ToString(LogLevel level)
{
    switch (level) {
    case LogLevel::Info:
        return "info";
    case LogLevel::Warning:
        return "warn";
    case LogLevel::Error:
        return "error";
    }

    return "unknown";
}

} // namespace engine

