#pragma once

#include <cstdint>
#include <filesystem>
#include <span>

namespace engine {

bool WriteBgraBmp(const std::filesystem::path& outputPath, int width, int height, std::span<const std::uint8_t> bgraPixels);

} // namespace engine
