#pragma once

#include "engine/renderer/RendererTypes.h"

#include <filesystem>
#include <string>
#include <vector>

namespace engine {

struct AppConfig {
    std::string appName = "AI-Native Engine Foundation";
    int windowWidth = 1280;
    int windowHeight = 720;
    bool smokeTest = false;
    bool headless = false;
    int maxFrames = 0;
    std::string rendererBackend = "auto";
    std::filesystem::path assetRoot = "assets";
    Color clearColor = {0.08f, 0.11f, 0.16f, 1.0f};
};

struct ConfigParseResult {
    AppConfig config;
    std::vector<std::string> errors;
    bool helpRequested = false;
};

ConfigParseResult ParseArguments(int argc, const char* const* argv);
std::string BuildHelpText();

} // namespace engine

