#pragma once

#include <filesystem>

namespace engine {

std::filesystem::path NormalizePath(const std::filesystem::path& base, const std::filesystem::path& input);
bool PathExists(const std::filesystem::path& path);
bool EnsureDirectory(const std::filesystem::path& path);

} // namespace engine

