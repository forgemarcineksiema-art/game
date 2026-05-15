#include "engine/application/Application.h"

#include "engine/application/Engine.h"
#include "engine/core/Logger.h"
#include "engine/platform/Window.h"
#include "engine/renderer/RendererFactory.h"

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>

namespace engine {
namespace {

std::string FirstDebugLine(std::string_view text)
{
    const std::size_t lineEnd = text.find('\n');
    if (lineEnd == std::string_view::npos) {
        return std::string(text);
    }
    return std::string(text.substr(0, lineEnd));
}

bool IsCaptureFileNameCharacter(char value)
{
    return (value >= 'a' && value <= 'z')
        || (value >= 'A' && value <= 'Z')
        || (value >= '0' && value <= '9')
        || value == '-'
        || value == '_';
}

std::string CaptureFileNameForRenderer(std::string_view rendererName)
{
    std::string safeName;
    for (char value : rendererName) {
        safeName += IsCaptureFileNameCharacter(value) ? value : '-';
    }
    if (safeName.empty()) {
        safeName = "renderer";
    }

    return "capture-" + safeName + ".bmp";
}

std::filesystem::path ResolveCapturePath(const AppConfig& config, std::string_view rendererName)
{
    if (!config.captureFramePath.empty()) {
        return config.captureFramePath;
    }

    return config.captureDir / CaptureFileNameForRenderer(rendererName);
}

} // namespace

std::string BuildDebugWindowTitle(std::string_view appName, std::string_view debugText)
{
    std::string title(appName);
    if (debugText.empty()) {
        return title;
    }

    std::string summary = FirstDebugLine(debugText);
    constexpr std::size_t MaxSummaryLength = 112;
    if (summary.size() > MaxSummaryLength) {
        summary.resize(MaxSummaryLength - 3);
        summary += "...";
    }

    title += " | ";
    title += summary;
    return title;
}

int Application::run(AppConfig config, std::unique_ptr<IGameLayer> layer)
{
    if (!layer) {
        Logger::error("Application cannot run without a game layer.");
        return 2;
    }

    Engine engine;
    if (!engine.initialize(config)) {
        return 3;
    }

    std::unique_ptr<IWindow> window;
    if (!config.headless) {
        window = CreatePlatformWindow();
        if (window && window->create(config.windowWidth, config.windowHeight, config.appName)) {
            window->show();
            window->setCursorCaptured(config.captureCursor);
        } else {
            Logger::warning("Window creation unavailable; falling back to headless renderer.");
            window.reset();
            config.headless = true;
        }
    }

    RendererConfig rendererConfig;
    rendererConfig.appName = config.appName;
    rendererConfig.width = config.windowWidth;
    rendererConfig.height = config.windowHeight;
    rendererConfig.clearColor = config.clearColor;
    rendererConfig.headless = config.headless;
    rendererConfig.nativeWindow = window ? window->nativeHandle() : nullptr;

    auto renderer = CreateRenderer(config.rendererBackend, rendererConfig);
    if (!renderer || !renderer->initialize(rendererConfig)) {
        if (!config.headless && (config.rendererBackend == "auto" || config.rendererBackend == "dx11")) {
            Logger::warning("Primary renderer failed; trying GDI fallback renderer.");
            renderer = CreateRenderer("gdi", rendererConfig);
        }

        if (!renderer || !renderer->initialize(rendererConfig)) {
            Logger::error("Renderer initialization failed.");
            engine.shutdown();
            return 4;
        }
    }

    Logger::info("Runtime started with renderer: " + renderer->name());
    layer->onAttach();

    constexpr unsigned long long StableCaptureFrame = 3;
    bool captureComplete = false;
    bool captureFailed = false;
    bool running = true;
    std::string lastWindowTitle;
    while (running) {
        if (window) {
            window->processEvents();
            if (window->shouldClose()) {
                running = false;
            }
        }

        const double deltaSeconds = engine.clock().tick();
        InputState input;
        if (window) {
            input = window->inputState();
            if (input.quitRequested) {
                running = false;
            }
        }
        input.quitRequested = input.quitRequested || !running;

        layer->onUpdate(deltaSeconds, input);
        if (window && engine.clock().frameIndex() % 30 == 0) {
            const std::string debugText = layer->debugText();
            if (!debugText.empty()) {
                const std::string nextTitle = BuildDebugWindowTitle(config.appName, debugText);
                if (nextTitle != lastWindowTitle) {
                    window->setTitle(nextTitle);
                    lastWindowTitle = nextTitle;
                }
            }
        }
        renderer->beginFrame(engine.clock().frameIndex());
        layer->onRender(*renderer);
        const bool lastBoundedFrame = config.maxFrames > 0
            && engine.clock().frameIndex() >= static_cast<unsigned long long>(config.maxFrames);
        if (config.captureRequested()
            && !captureComplete
            && (engine.clock().frameIndex() >= StableCaptureFrame || lastBoundedFrame)) {
            const std::filesystem::path capturePath = ResolveCapturePath(config, renderer->name());
            if (renderer->captureFrame(capturePath)) {
                Logger::info("Captured renderer frame to: " + capturePath.string());
            } else {
                Logger::error("Renderer frame capture failed for: " + capturePath.string());
                captureFailed = true;
                running = false;
            }
            captureComplete = true;
        }
        renderer->endFrame();

        if (config.maxFrames > 0 && engine.clock().frameIndex() >= static_cast<unsigned long long>(config.maxFrames)) {
            running = false;
        }

        if (config.headless) {
            std::this_thread::yield();
        } else if (!renderer->isFramePaced()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

    if (config.captureRequested() && !captureComplete) {
        Logger::error("Frame capture was requested, but the application exited before a frame could be captured.");
        captureFailed = true;
    }

    layer->onDetach();
    renderer->shutdown();
    if (window) {
        window->shutdown();
    }
    engine.shutdown();

    return captureFailed ? 5 : 0;
}

} // namespace engine
