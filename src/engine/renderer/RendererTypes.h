#pragma once

#include "engine/math/Math.h"

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
    Color clearColor = {0.14f, 0.18f, 0.20f, 1.0f};
    bool headless = false;
    void* nativeWindow = nullptr;
};

struct DebugCamera {
    Vec3 position = {0.0f, 6.0f, -8.0f};
    Vec3 target = {0.0f, 1.0f, 0.0f};
    float fovYRadians = Radians(60.0f);
    float nearPlane = 0.05f;
};

} // namespace engine
