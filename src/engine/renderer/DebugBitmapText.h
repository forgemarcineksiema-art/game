#pragma once

#include <string_view>
#include <vector>

namespace engine {

struct DebugBitmapTextLayout {
    int viewportWidth = 1280;
    int viewportHeight = 720;
    int originX = 16;
    int originY = 16;
    int rightPadding = 16;
    int bottomPadding = 16;
    int glyphScale = 2;
    int glyphSpacing = 2;
    int lineSpacing = 4;
};

struct DebugBitmapTextQuad {
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
};

std::vector<DebugBitmapTextQuad> BuildDebugBitmapTextQuads(std::string_view text, const DebugBitmapTextLayout& layout);

} // namespace engine
