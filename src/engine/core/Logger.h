#pragma once

#include <string>
#include <string_view>

namespace engine {

enum class LogLevel {
    Info,
    Warning,
    Error
};

class Logger {
public:
    static void setQuiet(bool quiet);
    static void write(LogLevel level, std::string_view message);
    static void info(std::string_view message);
    static void warning(std::string_view message);
    static void error(std::string_view message);

private:
    static bool& quietFlag();
};

std::string ToString(LogLevel level);

} // namespace engine

