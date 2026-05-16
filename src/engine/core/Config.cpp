#include "engine/core/Config.h"

#include <charconv>
#include <sstream>
#include <string_view>
#include <system_error>

namespace engine {
namespace {

bool ReadValue(int argc, const char* const* argv, int& index, std::string_view argument, std::string_view name, std::string& value)
{
    const std::string prefix = std::string(name) + "=";
    const std::string_view prefixView(prefix);
    if (argument.rfind(prefixView, 0) == 0) {
        value = std::string(argument.substr(prefix.size()));
        return true;
    }

    if (argument == name && index + 1 < argc) {
        ++index;
        value = argv[index];
        return true;
    }

    return false;
}

bool ParsePositiveInt(std::string_view text, int& value)
{
    int parsed = 0;
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    const auto result = std::from_chars(begin, end, parsed);
    if (result.ec != std::errc() || result.ptr != end || parsed <= 0) {
        return false;
    }
    value = parsed;
    return true;
}

bool ParseUiMode(std::string_view text, UiMode& value)
{
    if (text == "playtest") {
        value = UiMode::Playtest;
        return true;
    }
    if (text == "debug") {
        value = UiMode::Debug;
        return true;
    }
    if (text == "minimal") {
        value = UiMode::Minimal;
        return true;
    }
    return false;
}

} // namespace

std::string_view UiModeName(UiMode mode)
{
    switch (mode) {
    case UiMode::Playtest:
        return "playtest";
    case UiMode::Debug:
        return "debug";
    case UiMode::Minimal:
        return "minimal";
    }

    return "playtest";
}

bool AppConfig::captureRequested() const
{
    return !captureFramePath.empty() || !captureDir.empty();
}

bool AppConfig::qaPlaythroughRequested() const
{
    return !qaPlaythrough.empty();
}

bool AppConfig::qaPhysicsParityRequested() const
{
    return !qaPhysicsParity.empty();
}

ConfigParseResult ParseArguments(int argc, const char* const* argv)
{
    ConfigParseResult result;

    for (int index = 1; index < argc; ++index) {
        const std::string_view argument = argv[index];
        std::string value;

        if (argument == "--help" || argument == "-h") {
            result.helpRequested = true;
            continue;
        }

        if (argument == "--smoke-test") {
            result.config.smokeTest = true;
            result.config.headless = true;
            if (result.config.maxFrames == 0) {
                result.config.maxFrames = 3;
            }
            continue;
        }

        if (argument == "--headless") {
            result.config.headless = true;
            continue;
        }

        if (argument == "--capture-cursor") {
            result.config.captureCursor = true;
            continue;
        }

        if (argument == "--free-cursor" || argument == "--show-cursor") {
            result.config.captureCursor = false;
            continue;
        }

        if (argument == "--playtest-ui") {
            result.config.uiMode = UiMode::Playtest;
            continue;
        }

        if (argument == "--debug-ui") {
            result.config.uiMode = UiMode::Debug;
            continue;
        }

        if (ReadValue(argc, argv, index, argument, "--frames", value)) {
            if (!ParsePositiveInt(value, result.config.maxFrames)) {
                result.errors.push_back("--frames must be a positive integer.");
            }
            continue;
        }

        if (ReadValue(argc, argv, index, argument, "--width", value)) {
            if (!ParsePositiveInt(value, result.config.windowWidth)) {
                result.errors.push_back("--width must be a positive integer.");
            }
            continue;
        }

        if (ReadValue(argc, argv, index, argument, "--height", value)) {
            if (!ParsePositiveInt(value, result.config.windowHeight)) {
                result.errors.push_back("--height must be a positive integer.");
            }
            continue;
        }

        if (ReadValue(argc, argv, index, argument, "--renderer", value)) {
            if (value == "auto" || value == "null" || value == "gdi" || value == "dx11") {
                result.config.rendererBackend = value;
            } else {
                result.errors.push_back("--renderer must be one of: auto, null, gdi, dx11.");
            }
            continue;
        }

        if (ReadValue(argc, argv, index, argument, "--ui-mode", value)) {
            if (!ParseUiMode(value, result.config.uiMode)) {
                result.errors.push_back("--ui-mode must be one of: playtest, debug, minimal.");
            }
            continue;
        }

        if (ReadValue(argc, argv, index, argument, "--assets", value)) {
            result.config.assetRoot = value;
            continue;
        }

        if (ReadValue(argc, argv, index, argument, "--scene", value)) {
            result.config.scenePath = value;
            continue;
        }

        if (ReadValue(argc, argv, index, argument, "--capture-frame", value)) {
            if (value.empty()) {
                result.errors.push_back("--capture-frame requires a non-empty output path.");
            } else {
                result.config.captureFramePath = value;
            }
            continue;
        }

        if (ReadValue(argc, argv, index, argument, "--capture-dir", value)) {
            if (value.empty()) {
                result.errors.push_back("--capture-dir requires a non-empty output directory.");
            } else {
                result.config.captureDir = value;
            }
            continue;
        }

        if (ReadValue(argc, argv, index, argument, "--qa-playthrough", value)) {
            if (value == "ferry-office-service-call") {
                result.config.qaPlaythrough = value;
            } else {
                result.errors.push_back("--qa-playthrough must be: ferry-office-service-call.");
            }
            continue;
        }

        if (ReadValue(argc, argv, index, argument, "--qa-playthrough-report", value)) {
            if (value.empty()) {
                result.errors.push_back("--qa-playthrough-report requires a non-empty output path.");
            } else {
                result.config.qaPlaythroughReportPath = value;
            }
            continue;
        }

        if (ReadValue(argc, argv, index, argument, "--qa-physics-parity", value)) {
            if (value == "ferry-office-collision") {
                result.config.qaPhysicsParity = value;
            } else {
                result.errors.push_back("--qa-physics-parity must be: ferry-office-collision.");
            }
            continue;
        }

        if (ReadValue(argc, argv, index, argument, "--qa-physics-report", value)) {
            if (value.empty()) {
                result.errors.push_back("--qa-physics-report requires a non-empty output path.");
            } else {
                result.config.qaPhysicsReportPath = value;
            }
            continue;
        }

        result.errors.push_back("Unknown argument: " + std::string(argument));
    }

    if (!result.config.captureFramePath.empty() && !result.config.captureDir.empty()) {
        result.errors.push_back("--capture-frame and --capture-dir cannot be combined.");
    }
    if (!result.config.qaPlaythroughReportPath.empty() && result.config.qaPlaythrough.empty()) {
        result.errors.push_back("--qa-playthrough-report requires --qa-playthrough.");
    }
    if (!result.config.qaPhysicsReportPath.empty() && result.config.qaPhysicsParity.empty()) {
        result.errors.push_back("--qa-physics-report requires --qa-physics-parity.");
    }

    return result;
}

std::string BuildHelpText()
{
    std::ostringstream output;
    output
        << "EngineApp options:\n"
        << "  --smoke-test          Run a bounded headless startup/render loop and exit.\n"
        << "  --frames <count>      Exit after count frames.\n"
        << "  --headless            Run without opening a platform window.\n"
        << "  --capture-cursor      Hide and confine the cursor during windowed play (default).\n"
        << "  --free-cursor         Keep the cursor visible and uncaptured for desktop debugging.\n"
        << "  --show-cursor         Alias for --free-cursor.\n"
        << "  --ui-mode <mode>      Select playtest, debug, or minimal overlay text (default: playtest).\n"
        << "  --playtest-ui         Alias for --ui-mode playtest.\n"
        << "  --debug-ui            Alias for --ui-mode debug.\n"
        << "  --renderer <backend>  Select auto, null, gdi, or dx11.\n"
        << "  --width <pixels>      Window width for windowed runs.\n"
        << "  --height <pixels>     Window height for windowed runs.\n"
        << "  --assets <path>       Asset root path.\n"
        << "  --scene <path>        Runtime scene JSON path.\n"
        << "  --capture-frame <path> Write one renderer-owned BMP capture to the exact path.\n"
        << "  --capture-dir <path>  Write one renderer-owned BMP capture into the directory.\n"
        << "  --qa-playthrough <name> Run a QA-only automated playthrough. Supported: ferry-office-service-call.\n"
        << "  --qa-playthrough-report <path> Write the QA playthrough JSON report.\n"
        << "  --qa-physics-parity <name> Run QA-only physics parity. Supported: ferry-office-collision.\n"
        << "  --qa-physics-report <path> Write the QA physics parity JSON report.\n"
        << "  --help                Show this help.\n";
    return output.str();
}

} // namespace engine
