#include "engine/application/Application.h"

#include "engine/application/Engine.h"
#include "engine/core/Logger.h"
#include "engine/platform/Window.h"
#include "engine/renderer/RendererFactory.h"

#include <chrono>
#include <memory>
#include <thread>

namespace engine {

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

    bool running = true;
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
        if (window && engine.clock().frameIndex() % 10 == 0) {
            const std::string debugText = layer->debugText();
            if (!debugText.empty()) {
                window->setTitle(config.appName + " | " + debugText);
            }
        }
        renderer->beginFrame(engine.clock().frameIndex());
        layer->onRender(*renderer);
        renderer->endFrame();

        if (config.maxFrames > 0 && engine.clock().frameIndex() >= static_cast<unsigned long long>(config.maxFrames)) {
            running = false;
        }

        if (config.smokeTest) {
            continue;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    layer->onDetach();
    renderer->shutdown();
    if (window) {
        window->shutdown();
    }
    engine.shutdown();

    return 0;
}

} // namespace engine
