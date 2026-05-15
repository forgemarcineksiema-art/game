#include "engine/renderer/DebugBitmapText.h"

#include <algorithm>
#include <array>
#include <cctype>

namespace engine {
namespace {

constexpr int GlyphWidth = 5;
constexpr int GlyphHeight = 7;

using GlyphRows = std::array<unsigned char, GlyphHeight>;

constexpr unsigned char Row(const char* bits)
{
    return static_cast<unsigned char>(
        ((bits[0] == '1') ? 0b10000 : 0)
        | ((bits[1] == '1') ? 0b01000 : 0)
        | ((bits[2] == '1') ? 0b00100 : 0)
        | ((bits[3] == '1') ? 0b00010 : 0)
        | ((bits[4] == '1') ? 0b00001 : 0));
}

constexpr GlyphRows Glyph(
    const char* r0,
    const char* r1,
    const char* r2,
    const char* r3,
    const char* r4,
    const char* r5,
    const char* r6)
{
    return {Row(r0), Row(r1), Row(r2), Row(r3), Row(r4), Row(r5), Row(r6)};
}

GlyphRows GlyphFor(char value)
{
    const char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(value)));
    switch (upper) {
    case 'A': return Glyph("01110", "10001", "10001", "11111", "10001", "10001", "10001");
    case 'B': return Glyph("11110", "10001", "10001", "11110", "10001", "10001", "11110");
    case 'C': return Glyph("01111", "10000", "10000", "10000", "10000", "10000", "01111");
    case 'D': return Glyph("11110", "10001", "10001", "10001", "10001", "10001", "11110");
    case 'E': return Glyph("11111", "10000", "10000", "11110", "10000", "10000", "11111");
    case 'F': return Glyph("11111", "10000", "10000", "11110", "10000", "10000", "10000");
    case 'G': return Glyph("01111", "10000", "10000", "10111", "10001", "10001", "01111");
    case 'H': return Glyph("10001", "10001", "10001", "11111", "10001", "10001", "10001");
    case 'I': return Glyph("11111", "00100", "00100", "00100", "00100", "00100", "11111");
    case 'J': return Glyph("00111", "00010", "00010", "00010", "10010", "10010", "01100");
    case 'K': return Glyph("10001", "10010", "10100", "11000", "10100", "10010", "10001");
    case 'L': return Glyph("10000", "10000", "10000", "10000", "10000", "10000", "11111");
    case 'M': return Glyph("10001", "11011", "10101", "10101", "10001", "10001", "10001");
    case 'N': return Glyph("10001", "11001", "10101", "10011", "10001", "10001", "10001");
    case 'O': return Glyph("01110", "10001", "10001", "10001", "10001", "10001", "01110");
    case 'P': return Glyph("11110", "10001", "10001", "11110", "10000", "10000", "10000");
    case 'Q': return Glyph("01110", "10001", "10001", "10001", "10101", "10010", "01101");
    case 'R': return Glyph("11110", "10001", "10001", "11110", "10100", "10010", "10001");
    case 'S': return Glyph("01111", "10000", "10000", "01110", "00001", "00001", "11110");
    case 'T': return Glyph("11111", "00100", "00100", "00100", "00100", "00100", "00100");
    case 'U': return Glyph("10001", "10001", "10001", "10001", "10001", "10001", "01110");
    case 'V': return Glyph("10001", "10001", "10001", "10001", "01010", "01010", "00100");
    case 'W': return Glyph("10001", "10001", "10001", "10101", "10101", "11011", "10001");
    case 'X': return Glyph("10001", "01010", "00100", "00100", "00100", "01010", "10001");
    case 'Y': return Glyph("10001", "01010", "00100", "00100", "00100", "00100", "00100");
    case 'Z': return Glyph("11111", "00001", "00010", "00100", "01000", "10000", "11111");
    case '0': return Glyph("01110", "10001", "10011", "10101", "11001", "10001", "01110");
    case '1': return Glyph("00100", "01100", "00100", "00100", "00100", "00100", "01110");
    case '2': return Glyph("01110", "10001", "00001", "00010", "00100", "01000", "11111");
    case '3': return Glyph("11110", "00001", "00001", "01110", "00001", "00001", "11110");
    case '4': return Glyph("00010", "00110", "01010", "10010", "11111", "00010", "00010");
    case '5': return Glyph("11111", "10000", "10000", "11110", "00001", "00001", "11110");
    case '6': return Glyph("01110", "10000", "10000", "11110", "10001", "10001", "01110");
    case '7': return Glyph("11111", "00001", "00010", "00100", "01000", "01000", "01000");
    case '8': return Glyph("01110", "10001", "10001", "01110", "10001", "10001", "01110");
    case '9': return Glyph("01110", "10001", "10001", "01111", "00001", "00001", "01110");
    case ':': return Glyph("00000", "00100", "00100", "00000", "00100", "00100", "00000");
    case '.': return Glyph("00000", "00000", "00000", "00000", "00000", "01100", "01100");
    case ',': return Glyph("00000", "00000", "00000", "00000", "01100", "00100", "01000");
    case '-': return Glyph("00000", "00000", "00000", "11111", "00000", "00000", "00000");
    case '+': return Glyph("00000", "00100", "00100", "11111", "00100", "00100", "00000");
    case '/': return Glyph("00001", "00010", "00010", "00100", "01000", "01000", "10000");
    case '\\': return Glyph("10000", "01000", "01000", "00100", "00010", "00010", "00001");
    case '|': return Glyph("00100", "00100", "00100", "00100", "00100", "00100", "00100");
    case '=': return Glyph("00000", "11111", "00000", "11111", "00000", "00000", "00000");
    case '_': return Glyph("00000", "00000", "00000", "00000", "00000", "00000", "11111");
    case '!': return Glyph("00100", "00100", "00100", "00100", "00100", "00000", "00100");
    case '?': return Glyph("01110", "10001", "00001", "00010", "00100", "00000", "00100");
    case '\'': return Glyph("00100", "00100", "01000", "00000", "00000", "00000", "00000");
    case '"': return Glyph("01010", "01010", "01010", "00000", "00000", "00000", "00000");
    case '(': return Glyph("00010", "00100", "01000", "01000", "01000", "00100", "00010");
    case ')': return Glyph("01000", "00100", "00010", "00010", "00010", "00100", "01000");
    case '[': return Glyph("01110", "01000", "01000", "01000", "01000", "01000", "01110");
    case ']': return Glyph("01110", "00010", "00010", "00010", "00010", "00010", "01110");
    case '{': return Glyph("00010", "00100", "00100", "01000", "00100", "00100", "00010");
    case '}': return Glyph("01000", "00100", "00100", "00010", "00100", "00100", "01000");
    case '<': return Glyph("00010", "00100", "01000", "10000", "01000", "00100", "00010");
    case '>': return Glyph("01000", "00100", "00010", "00001", "00010", "00100", "01000");
    case '&': return Glyph("01100", "10010", "10100", "01000", "10101", "10010", "01101");
    case '#': return Glyph("01010", "01010", "11111", "01010", "11111", "01010", "01010");
    case '%': return Glyph("11001", "11010", "00010", "00100", "01000", "01011", "10011");
    case '*': return Glyph("00000", "10101", "01110", "11111", "01110", "10101", "00000");
    case ' ': return Glyph("00000", "00000", "00000", "00000", "00000", "00000", "00000");
    default: return Glyph("11111", "10001", "00010", "00100", "00100", "00000", "00100");
    }
}

} // namespace

std::vector<DebugBitmapTextQuad> BuildDebugBitmapTextQuads(std::string_view text, const DebugBitmapTextLayout& layout)
{
    const int scale = std::max(layout.glyphScale, 1);
    const int glyphSpacing = std::max(layout.glyphSpacing, 0);
    const int lineSpacing = std::max(layout.lineSpacing, 0);
    const int glyphWidth = GlyphWidth * scale;
    const int glyphHeight = GlyphHeight * scale;
    const int advance = glyphWidth + glyphSpacing;
    const int lineAdvance = glyphHeight + lineSpacing;
    const int maxRight = std::max(layout.originX, layout.viewportWidth - std::max(layout.rightPadding, 0));
    const int maxBottom = std::max(layout.originY, layout.viewportHeight - std::max(layout.bottomPadding, 0));

    std::vector<DebugBitmapTextQuad> quads;
    int x = layout.originX;
    int y = layout.originY;

    auto newLine = [&]() {
        x = layout.originX;
        y += lineAdvance;
    };

    for (char value : text) {
        if (value == '\r') {
            continue;
        }
        if (value == '\n') {
            newLine();
            continue;
        }
        if (value == '\t') {
            for (int tab = 0; tab < 4; ++tab) {
                if (x + glyphWidth > maxRight) {
                    newLine();
                }
                x += advance;
            }
            continue;
        }

        if (x + glyphWidth > maxRight) {
            newLine();
        }
        if (y + glyphHeight > maxBottom) {
            break;
        }

        const GlyphRows glyph = GlyphFor(value);
        for (int row = 0; row < GlyphHeight; ++row) {
            for (int column = 0; column < GlyphWidth; ++column) {
                const unsigned char bit = static_cast<unsigned char>(1U << (GlyphWidth - 1 - column));
                if ((glyph[row] & bit) == 0) {
                    continue;
                }

                const int left = x + column * scale;
                const int top = y + row * scale;
                quads.push_back({left, top, left + scale, top + scale});
            }
        }

        x += advance;
    }

    return quads;
}

} // namespace engine
