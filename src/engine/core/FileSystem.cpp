#include "engine/core/FileSystem.h"

namespace engine {

std::filesystem::path NormalizePath(const std::filesystem::path& base, const std::filesystem::path& input)
{
    if (input.is_absolute()) {
        return input.lexically_normal();
    }

    return (base / input).lexically_normal();
}

bool PathExists(const std::filesystem::path& path)
{
    return std::filesystem::exists(path);
}

bool EnsureDirectory(const std::filesystem::path& path)
{
    if (std::filesystem::exists(path)) {
        return std::filesystem::is_directory(path);
    }

    return std::filesystem::create_directories(path);
}

} // namespace engine

