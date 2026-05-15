#include "SandboxLayer.h"

#include "engine/application/Application.h"
#include "engine/core/Config.h"
#include "engine/core/Logger.h"

#include <iostream>
#include <memory>

int main(int argc, const char* const* argv)
{
    auto parseResult = engine::ParseArguments(argc, argv);
    if (parseResult.helpRequested) {
        std::cout << engine::BuildHelpText();
        return 0;
    }

    if (!parseResult.errors.empty()) {
        for (const auto& error : parseResult.errors) {
            engine::Logger::error(error);
        }
        std::cerr << engine::BuildHelpText();
        return 1;
    }

    engine::Application app;
    return app.run(parseResult.config, std::make_unique<SandboxLayer>(parseResult.config.scenePath));
}
