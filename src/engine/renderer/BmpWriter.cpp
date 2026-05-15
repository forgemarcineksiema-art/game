#include "engine/renderer/BmpWriter.h"

#include <fstream>
#include <limits>
#include <system_error>

namespace engine {
namespace {

void WriteU16(std::ofstream& output, std::uint16_t value)
{
    output.put(static_cast<char>(value & 0xFF));
    output.put(static_cast<char>((value >> 8) & 0xFF));
}

void WriteU32(std::ofstream& output, std::uint32_t value)
{
    output.put(static_cast<char>(value & 0xFF));
    output.put(static_cast<char>((value >> 8) & 0xFF));
    output.put(static_cast<char>((value >> 16) & 0xFF));
    output.put(static_cast<char>((value >> 24) & 0xFF));
}

void WriteI32(std::ofstream& output, std::int32_t value)
{
    WriteU32(output, static_cast<std::uint32_t>(value));
}

} // namespace

bool WriteBgraBmp(const std::filesystem::path& outputPath, int width, int height, std::span<const std::uint8_t> bgraPixels)
{
    if (width <= 0 || height <= 0 || outputPath.empty()) {
        return false;
    }

    const std::size_t safeWidth = static_cast<std::size_t>(width);
    const std::size_t safeHeight = static_cast<std::size_t>(height);
    if (safeWidth > (std::numeric_limits<std::size_t>::max() / safeHeight) / 4U) {
        return false;
    }

    const std::size_t pixelByteCount = safeWidth * safeHeight * 4U;
    if (bgraPixels.size() != pixelByteCount) {
        return false;
    }

    constexpr std::uint32_t FileHeaderBytes = 14;
    constexpr std::uint32_t InfoHeaderBytes = 40;
    constexpr std::uint32_t PixelDataOffset = FileHeaderBytes + InfoHeaderBytes;
    if (pixelByteCount > std::numeric_limits<std::uint32_t>::max() - PixelDataOffset) {
        return false;
    }

    const std::filesystem::path parentPath = outputPath.parent_path();
    if (!parentPath.empty()) {
        std::error_code error;
        std::filesystem::create_directories(parentPath, error);
        if (error) {
            return false;
        }
    }

    std::ofstream output(outputPath, std::ios::binary);
    if (!output) {
        return false;
    }

    const auto pixelBytes32 = static_cast<std::uint32_t>(pixelByteCount);
    const std::uint32_t fileSize = PixelDataOffset + pixelBytes32;

    output.put('B');
    output.put('M');
    WriteU32(output, fileSize);
    WriteU16(output, 0);
    WriteU16(output, 0);
    WriteU32(output, PixelDataOffset);

    WriteU32(output, InfoHeaderBytes);
    WriteI32(output, width);
    WriteI32(output, -height);
    WriteU16(output, 1);
    WriteU16(output, 32);
    WriteU32(output, 0);
    WriteU32(output, pixelBytes32);
    WriteI32(output, 0);
    WriteI32(output, 0);
    WriteU32(output, 0);
    WriteU32(output, 0);

    output.write(reinterpret_cast<const char*>(bgraPixels.data()), static_cast<std::streamsize>(bgraPixels.size()));
    return output.good();
}

} // namespace engine
