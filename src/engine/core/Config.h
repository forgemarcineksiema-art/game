#pragma once

#include "engine/renderer/RendererTypes.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace engine {

enum class UiMode {
    Playtest,
    Debug,
    Minimal,
};

std::string_view UiModeName(UiMode mode);

struct AppConfig {
    std::string appName = "Tidebreak Prototype";
    int windowWidth = 1280;
    int windowHeight = 720;
    bool smokeTest = false;
    bool headless = false;
    bool captureCursor = true;
    int maxFrames = 0;
    std::string rendererBackend = "auto";
    UiMode uiMode = UiMode::Playtest;
    std::filesystem::path assetRoot = "assets";
    std::filesystem::path scenePath = "data/scenes/ferry_office.scene.json";
    std::filesystem::path captureFramePath;
    std::filesystem::path captureDir;
    Color clearColor = {0.08f, 0.11f, 0.16f, 1.0f};

    bool captureRequested() const;
};

struct ConfigParseResult {
    AppConfig config;
    std::vector<std::string> errors;
    bool helpRequested = false;
};

ConfigParseResult ParseArguments(int argc, const char* const* argv);
std::string BuildHelpText();

} // namespace engine
