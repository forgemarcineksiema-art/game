#pragma once

#include <string>

namespace engine {

struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

struct RendererConfig {
    std::string appName;
    int width = 1280;
    int height = 720;
    Color clearColor = {0.08f, 0.11f, 0.16f, 1.0f};
    bool headless = false;
    void* nativeWindow = nullptr;
};

} // namespace engine

