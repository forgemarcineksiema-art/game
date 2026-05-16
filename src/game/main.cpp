#include "SandboxLayer.h"

#include "engine/application/Application.h"
#include "engine/core/Config.h"
#include "engine/core/Logger.h"
#include "game/FerryOfficeCharacterContactQa.h"
#include "game/FerryOfficePhysicsParity.h"
#include "game/FerryOfficePlaythroughQa.h"
#include "game/FerryOfficeVehiclePhysicsQa.h"

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

    if (parseResult.config.qaPlaythroughRequested()) {
        const auto result = RunFerryOfficeServiceCallPlaythroughQa(
            parseResult.config.scenePath,
            parseResult.config.qaPlaythroughReportPath);
        if (result.passed) {
            engine::Logger::info("QA playthrough passed: " + result.reportPath.string());
            return 0;
        }
        engine::Logger::error("QA playthrough failed: " + result.error);
        return 6;
    }

    if (parseResult.config.qaPhysicsParityRequested()) {
        if (parseResult.config.qaPhysicsParity == "ferry-office-vehicle-runtime-comparison") {
            const auto result = RunFerryOfficeVehicleRuntimeComparisonQa(
                parseResult.config.scenePath,
                parseResult.config.qaPhysicsReportPath,
                engine::physics::OptInPhysicsBackend());
            if (result.passed) {
                engine::Logger::info("QA vehicle runtime comparison passed: " + result.reportPath.string());
                return 0;
            }
            engine::Logger::error("QA vehicle runtime comparison failed: " + result.error);
            return 10;
        }

        if (parseResult.config.qaPhysicsParity == "ferry-office-vehicle-feasibility") {
            const auto result = RunFerryOfficeVehiclePhysicsQa(
                parseResult.config.scenePath,
                parseResult.config.qaPhysicsReportPath,
                engine::physics::OptInPhysicsBackend());
            if (result.passed) {
                engine::Logger::info("QA vehicle physics passed: " + result.reportPath.string());
                return 0;
            }
            engine::Logger::error("QA vehicle physics failed: " + result.error);
            return 9;
        }

        if (parseResult.config.qaPhysicsParity == "ferry-office-character-contact") {
            const auto result = RunFerryOfficeCharacterContactQa(
                parseResult.config.scenePath,
                parseResult.config.qaPhysicsReportPath,
                engine::physics::OptInPhysicsBackend());
            if (result.passed) {
                engine::Logger::info("QA character contact passed: " + result.reportPath.string());
                return 0;
            }
            engine::Logger::error("QA character contact failed: " + result.error);
            return 8;
        }

        const auto result = RunFerryOfficePhysicsParityQa(
            parseResult.config.scenePath,
            parseResult.config.qaPhysicsReportPath,
            engine::physics::OptInPhysicsBackend());
        if (result.passed) {
            engine::Logger::info("QA physics parity passed: " + result.reportPath.string());
            return 0;
        }
        engine::Logger::error("QA physics parity failed: " + result.error);
        return 7;
    }

    engine::Application app;
    return app.run(parseResult.config, std::make_unique<SandboxLayer>(parseResult.config.scenePath, parseResult.config.uiMode));
}
