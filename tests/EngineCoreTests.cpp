#include "engine/application/Application.h"
#include "engine/core/Clock.h"
#include "engine/core/Config.h"
#include "engine/core/FileSystem.h"
#include "engine/math/Math.h"
#include "engine/physics/PhysicsWorld.h"
#include "engine/physics/VehicleRuntime.h"
#include "engine/assets/StaticMesh.h"
#include "engine/renderer/BmpWriter.h"
#include "engine/renderer/DebugBitmapText.h"
#include "engine/renderer/DebugCameraMatrices.h"
#include "engine/renderer/DebugProjection.h"
#include "engine/renderer/Renderer.h"
#include "engine/renderer/NullRenderer.h"
#include "game/InteractionSystem.h"
#include "game/PlayerController.h"
#include "game/FerryOfficeData.h"
#include "game/FerryOfficeCharacterContactQa.h"
#include "game/FerryOfficeJob.h"
#include "game/FerryOfficePhysicsParity.h"
#include "game/FerryOfficePlaythroughQa.h"
#include "game/FerryOfficeVehiclePhysicsQa.h"
#include "game/PrototypeScene.h"
#include "game/PrototypeWorld.h"
#include "game/SandboxLayer.h"
#include "game/SceneDefinition.h"
#include "game/SceneGuidanceSurface.h"
#include "game/SceneLoader.h"
#include "game/ScenePresentation.h"
#include "game/SceneRenderSurface.h"
#include "game/SceneRuntimePackage.h"
#include "game/SceneRuntimePolicy.h"
#include "game/SceneRuntimeSurface.h"
#include "game/ThirdPersonCamera.h"
#include "game/TraversalSystem.h"
#include "game/VehicleController.h"
#include "game/WorldState.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <array>
#include <cstdint>
#include <iterator>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

struct TestFailure {
    std::string name;
    std::string message;
};

std::vector<TestFailure> failures;

std::filesystem::path DefaultScenePathForTests()
{
    return std::filesystem::path(ENGINE_SOURCE_ROOT) / "data" / "scenes" / "ferry_office.scene.json";
}

std::filesystem::path PilotScenePathForTests()
{
    return std::filesystem::path(ENGINE_SOURCE_ROOT) / "data" / "scenes" / "veyra_reach_pilot.scene.json";
}

std::filesystem::path WriteTempSceneWithPlayerStartYaw(float yawDegrees)
{
    std::ifstream input(DefaultScenePathForTests());
    nlohmann::json scene = nlohmann::json::parse(input);
    scene["playerStart"]["yawDegrees"] = yawDegrees;

    const std::filesystem::path tempPath = std::filesystem::temp_directory_path() / "tidebreak-v025-player-start-yaw.scene.json";
    std::ofstream output(tempPath);
    output << scene.dump(2);
    return tempPath;
}

const SceneVehicleDefinition* FindVehicle(const SceneDefinition& scene, std::string_view id)
{
    for (const SceneVehicleDefinition& vehicle : scene.vehicles) {
        if (vehicle.id == id) {
            return &vehicle;
        }
    }

    return nullptr;
}

const SceneRouteMarkerDefinition* FindRouteMarker(const SceneDefinition& scene, std::string_view id)
{
    for (const SceneRouteMarkerDefinition& marker : scene.routeMarkers) {
        if (marker.id == id) {
            return &marker;
        }
    }

    return nullptr;
}

const SceneVisualPlaceholderDefinition* FindVisualPlaceholder(const SceneDefinition& scene, std::string_view id)
{
    for (const SceneVisualPlaceholderDefinition& placeholder : scene.visualPlaceholders) {
        if (placeholder.id == id) {
            return &placeholder;
        }
    }

    return nullptr;
}

const SceneInteractableDefinition* FindSceneInteractable(const SceneDefinition& scene, std::string_view id)
{
    for (const SceneInteractableDefinition& interactable : scene.interactables) {
        if (interactable.id == id) {
            return &interactable;
        }
    }

    return nullptr;
}

const SceneObjectiveMarkerDefinition* FindObjectiveMarker(const SceneDefinition& scene, std::string_view id)
{
    for (const SceneObjectiveMarkerDefinition& marker : scene.objectiveMarkers) {
        if (marker.id == id) {
            return &marker;
        }
    }

    return nullptr;
}

bool ContainsString(const std::vector<std::string>& values, std::string_view expected)
{
    for (const std::string& value : values) {
        if (value == expected) {
            return true;
        }
    }
    return false;
}

void Expect(bool condition, const std::string& name, const std::string& message)
{
    if (!condition) {
        failures.push_back({name, message});
    }
}

void ExpectNear(float actual, float expected, float tolerance, const std::string& name, const std::string& message)
{
    if (std::abs(actual - expected) > tolerance) {
        failures.push_back({name, message});
    }
}

std::vector<std::uint8_t> ReadBinaryFile(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>());
}

std::uint16_t ReadLittleEndianU16(const std::vector<std::uint8_t>& bytes, std::size_t offset)
{
    return static_cast<std::uint16_t>(bytes[offset])
        | (static_cast<std::uint16_t>(bytes[offset + 1]) << 8);
}

std::uint32_t ReadLittleEndianU32(const std::vector<std::uint8_t>& bytes, std::size_t offset)
{
    return static_cast<std::uint32_t>(bytes[offset])
        | (static_cast<std::uint32_t>(bytes[offset + 1]) << 8)
        | (static_cast<std::uint32_t>(bytes[offset + 2]) << 16)
        | (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
}

std::int32_t ReadLittleEndianI32(const std::vector<std::uint8_t>& bytes, std::size_t offset)
{
    return static_cast<std::int32_t>(ReadLittleEndianU32(bytes, offset));
}

class CountingRenderer final : public engine::IRenderer {
public:
    struct LineCall {
        engine::Vec3 from {};
        engine::Vec3 to {};
        engine::Color color {};
    };

    struct BoxCall {
        engine::Vec3 center {};
        engine::Vec3 halfExtents {};
        engine::Color color {};
    };

    bool initialize(const engine::RendererConfig&) override { return true; }
    void beginFrame(unsigned long long) override {}
    void setDebugCamera(const engine::DebugCamera& camera) override
    {
        debugCamera = camera;
        debugCameraSet = true;
    }
    void drawDebugGridAndAxes() override {}
    void drawDebugLine(engine::Vec3 from, engine::Vec3 to, engine::Color color) override { lineCalls.push_back({from, to, color}); }
    void drawDebugSolidBox(engine::Vec3 center, engine::Vec3 halfExtents, engine::Color color) override { solidBoxCalls.push_back({center, halfExtents, color}); }
    void drawDebugFlatTriangles(std::span<const engine::Vec3>, engine::Color) override { ++flatTriangleDrawCount; }
    void drawDebugBox(engine::Vec3 center, engine::Vec3 halfExtents, engine::Color color) override { boxCalls.push_back({center, halfExtents, color}); }
    void drawDebugText(std::string_view) override {}
    void endFrame() override {}
    void shutdown() override {}
    std::string name() const override { return "counting-test-renderer"; }

    unsigned int flatTriangleDrawCount = 0;
    engine::DebugCamera debugCamera {};
    bool debugCameraSet = false;
    std::vector<LineCall> lineCalls;
    std::vector<BoxCall> solidBoxCalls;
    std::vector<BoxCall> boxCalls;
};

bool ColorNear(engine::Color actual, engine::Color expected, float tolerance = 0.001f)
{
    return std::abs(actual.r - expected.r) <= tolerance
        && std::abs(actual.g - expected.g) <= tolerance
        && std::abs(actual.b - expected.b) <= tolerance
        && std::abs(actual.a - expected.a) <= tolerance;
}

float Luminance(engine::Color color)
{
    return color.r * 0.2126f + color.g * 0.7152f + color.b * 0.0722f;
}

std::size_t CountLinesWithColor(const CountingRenderer& renderer, engine::Color color)
{
    std::size_t count = 0;
    for (const CountingRenderer::LineCall& line : renderer.lineCalls) {
        if (ColorNear(line.color, color)) {
            ++count;
        }
    }
    return count;
}

std::size_t CountRouteMarkerSegments(const SceneDefinition& scene)
{
    std::size_t count = 0;
    for (const SceneRouteMarkerDefinition& route : scene.routeMarkers) {
        if (route.points.size() >= 2) {
            count += route.points.size() - 1;
        }
    }
    return count;
}

const SceneMeshAssetDefinition* FindMeshAsset(const SceneDefinition& scene, std::string_view id)
{
    for (const SceneMeshAssetDefinition& asset : scene.meshAssets) {
        if (asset.id == id) {
            return &asset;
        }
    }

    return nullptr;
}

const SceneMeshInstanceDefinition* FindMeshInstance(const SceneDefinition& scene, std::string_view id)
{
    for (const SceneMeshInstanceDefinition& instance : scene.meshInstances) {
        if (instance.id == id) {
            return &instance;
        }
    }

    return nullptr;
}

void TestSmokeArgumentsEnableBoundedHeadlessRun()
{
    const char* argv[] = {"EngineApp", "--smoke-test"};
    const auto result = engine::ParseArguments(2, argv);

    Expect(result.errors.empty(), "TestSmokeArgumentsEnableBoundedHeadlessRun", "Smoke arguments should not emit errors.");
    Expect(result.config.smokeTest, "TestSmokeArgumentsEnableBoundedHeadlessRun", "Smoke test flag should be true.");
    Expect(result.config.headless, "TestSmokeArgumentsEnableBoundedHeadlessRun", "Smoke test should default to headless.");
    Expect(result.config.maxFrames == 3, "TestSmokeArgumentsEnableBoundedHeadlessRun", "Smoke test should default to 3 frames.");
}

void TestFramesArgumentOverridesSmokeDefault()
{
    const char* argv[] = {"EngineApp", "--smoke-test", "--frames", "8"};
    const auto result = engine::ParseArguments(4, argv);

    Expect(result.errors.empty(), "TestFramesArgumentOverridesSmokeDefault", "Frames override should parse cleanly.");
    Expect(result.config.maxFrames == 8, "TestFramesArgumentOverridesSmokeDefault", "Frames should be overridden to 8.");
}

void TestInvalidRendererIsRejected()
{
    const char* argv[] = {"EngineApp", "--renderer", "pbr"};
    const auto result = engine::ParseArguments(3, argv);

    Expect(!result.errors.empty(), "TestInvalidRendererIsRejected", "Invalid renderer should emit an error.");
}

void TestCaptureArgumentsSelectOutputPath()
{
    const char* argv[] = {"EngineApp", "--capture-frame", "build/captures/frame.bmp"};
    const auto result = engine::ParseArguments(3, argv);

    Expect(result.errors.empty(), "TestCaptureArgumentsSelectOutputPath", "Capture frame argument should parse cleanly.");
    Expect(result.config.captureFramePath.generic_string() == "build/captures/frame.bmp",
        "TestCaptureArgumentsSelectOutputPath",
        "Capture frame argument should preserve the requested output file.");
    Expect(result.config.captureDir.empty(),
        "TestCaptureArgumentsSelectOutputPath",
        "Capture frame argument should not also set a generated capture directory.");
}

void TestCaptureDirArgumentsSelectOutputDirectory()
{
    const char* argv[] = {"EngineApp", "--capture-dir=build/captures"};
    const auto result = engine::ParseArguments(2, argv);

    Expect(result.errors.empty(), "TestCaptureDirArgumentsSelectOutputDirectory", "Capture dir argument should parse cleanly.");
    Expect(result.config.captureDir.generic_string() == "build/captures",
        "TestCaptureDirArgumentsSelectOutputDirectory",
        "Capture dir argument should preserve the requested output directory.");
    Expect(result.config.captureFramePath.empty(),
        "TestCaptureDirArgumentsSelectOutputDirectory",
        "Capture dir argument should not also set an exact capture path.");
}

void TestCaptureArgumentsRejectAmbiguousDestination()
{
    const char* argv[] = {"EngineApp", "--capture-frame", "a.bmp", "--capture-dir", "build/captures"};
    const auto result = engine::ParseArguments(5, argv);

    Expect(!result.errors.empty(),
        "TestCaptureArgumentsRejectAmbiguousDestination",
        "Capture frame and capture dir should be mutually exclusive.");
}

void TestHelpTextMentionsCaptureFlags()
{
    const std::string help = engine::BuildHelpText();

    Expect(help.find("--capture-frame") != std::string::npos,
        "TestHelpTextMentionsCaptureFlags",
        "Help text should document exact frame capture output.");
    Expect(help.find("--capture-dir") != std::string::npos,
        "TestHelpTextMentionsCaptureFlags",
        "Help text should document generated capture output.");
}

void TestQaPlaythroughArgumentsSelectScenarioAndReportPath()
{
    const char* argv[] = {
        "EngineApp",
        "--qa-playthrough",
        "ferry-office-service-call",
        "--qa-playthrough-report",
        "build/playthroughs/report.json",
    };
    const auto result = engine::ParseArguments(5, argv);

    Expect(result.errors.empty(),
        "TestQaPlaythroughArgumentsSelectScenarioAndReportPath",
        "QA playthrough arguments should parse cleanly.");
    Expect(result.config.qaPlaythrough == "ferry-office-service-call",
        "TestQaPlaythroughArgumentsSelectScenarioAndReportPath",
        "Config should preserve the requested QA playthrough scenario.");
    Expect(result.config.qaPlaythroughReportPath.generic_string() == "build/playthroughs/report.json",
        "TestQaPlaythroughArgumentsSelectScenarioAndReportPath",
        "Config should preserve the requested QA playthrough report path.");
}

void TestQaPlaythroughArgumentsRejectUnknownScenario()
{
    const char* argv[] = {"EngineApp", "--qa-playthrough", "job-two"};
    const auto result = engine::ParseArguments(3, argv);

    Expect(!result.errors.empty(),
        "TestQaPlaythroughArgumentsRejectUnknownScenario",
        "Unknown QA playthrough scenarios should be rejected.");
}

void TestHelpTextMentionsQaPlaythroughFlags()
{
    const std::string help = engine::BuildHelpText();

    Expect(help.find("--qa-playthrough") != std::string::npos,
        "TestHelpTextMentionsQaPlaythroughFlags",
        "Help text should document the QA playthrough scenario flag.");
    Expect(help.find("--qa-playthrough-report") != std::string::npos,
        "TestHelpTextMentionsQaPlaythroughFlags",
        "Help text should document the QA playthrough report flag.");
}

void TestQaPhysicsParityArgumentsSelectScenarioAndReportPath()
{
    const char* argv[] = {
        "EngineApp",
        "--qa-physics-parity",
        "ferry-office-collision",
        "--qa-physics-report",
        "build/physics/ferry-office-collision-parity.json",
    };
    const auto result = engine::ParseArguments(5, argv);

    Expect(result.errors.empty(),
        "TestQaPhysicsParityArgumentsSelectScenarioAndReportPath",
        "QA physics parity arguments should parse cleanly.");
    Expect(result.config.qaPhysicsParity == "ferry-office-collision",
        "TestQaPhysicsParityArgumentsSelectScenarioAndReportPath",
        "Config should preserve the requested QA physics parity scenario.");
    Expect(result.config.qaPhysicsReportPath.generic_string() == "build/physics/ferry-office-collision-parity.json",
        "TestQaPhysicsParityArgumentsSelectScenarioAndReportPath",
        "Config should preserve the requested QA physics parity report path.");
}

void TestQaPhysicsParityArgumentsRejectUnknownScenario()
{
    const char* argv[] = {"EngineApp", "--qa-physics-parity", "vehicle-constraint"};
    const auto result = engine::ParseArguments(3, argv);

    Expect(!result.errors.empty(),
        "TestQaPhysicsParityArgumentsRejectUnknownScenario",
        "Unknown QA physics parity scenarios should be rejected.");
}

void TestQaPhysicsParityArgumentsAcceptCharacterContactScenario()
{
    const char* argv[] = {
        "EngineApp",
        "--qa-physics-parity",
        "ferry-office-character-contact",
        "--qa-physics-report",
        "build/physics/ferry-office-character-contact-report.json",
    };
    const auto result = engine::ParseArguments(5, argv);

    Expect(result.errors.empty(),
        "TestQaPhysicsParityArgumentsAcceptCharacterContactScenario",
        "QA physics parity should accept the Ferry Office character/contact scenario.");
    Expect(result.config.qaPhysicsParity == "ferry-office-character-contact",
        "TestQaPhysicsParityArgumentsAcceptCharacterContactScenario",
        "Config should preserve the requested character/contact scenario.");
}

void TestQaPhysicsParityArgumentsAcceptVehicleFeasibilityScenario()
{
    const char* argv[] = {
        "EngineApp",
        "--qa-physics-parity",
        "ferry-office-vehicle-feasibility",
        "--qa-physics-report",
        "build/physics/ferry-office-vehicle-feasibility-report.json",
    };
    const auto result = engine::ParseArguments(5, argv);

    Expect(result.errors.empty(),
        "TestQaPhysicsParityArgumentsAcceptVehicleFeasibilityScenario",
        "QA physics parity should accept the Ferry Office vehicle feasibility scenario.");
    Expect(result.config.qaPhysicsParity == "ferry-office-vehicle-feasibility",
        "TestQaPhysicsParityArgumentsAcceptVehicleFeasibilityScenario",
        "Config should preserve the requested vehicle feasibility scenario.");
}

void TestQaPhysicsParityArgumentsAcceptVehicleRuntimeComparisonScenario()
{
    const char* argv[] = {
        "EngineApp",
        "--qa-physics-parity",
        "ferry-office-vehicle-runtime-comparison",
        "--qa-physics-report",
        "build/physics/vehicle-runtime-comparison.json",
    };
    const auto result = engine::ParseArguments(5, argv);

    Expect(result.errors.empty(),
        "TestQaPhysicsParityArgumentsAcceptVehicleRuntimeComparisonScenario",
        "QA physics parity should accept the explicit Ferry Office vehicle runtime comparison scenario.");
    Expect(result.config.qaPhysicsParity == "ferry-office-vehicle-runtime-comparison",
        "TestQaPhysicsParityArgumentsAcceptVehicleRuntimeComparisonScenario",
        "Config should preserve the requested vehicle runtime comparison scenario.");
}

void TestQaCaptureStateArgumentsSelectMidChainRouteState()
{
    const char* argv[] = {"EngineApp", "--qa-capture-state", "relay-to-service-log"};
    const auto result = engine::ParseArguments(3, argv);

    Expect(result.errors.empty(),
        "TestQaCaptureStateArgumentsSelectMidChainRouteState",
        "QA capture state argument should parse cleanly.");
    Expect(result.config.qaCaptureState == "relay-to-service-log",
        "TestQaCaptureStateArgumentsSelectMidChainRouteState",
        "Config should preserve the requested route-guidance capture state.");

    const char* lowDockArgv[] = {"EngineApp", "--qa-capture-state", "low-dock-drain-access"};
    const auto lowDockResult = engine::ParseArguments(3, lowDockArgv);
    Expect(lowDockResult.errors.empty(),
        "TestQaCaptureStateArgumentsSelectMidChainRouteState",
        "QA capture state should accept the low dock drain access scenario.");
    Expect(lowDockResult.config.qaCaptureState == "low-dock-drain-access",
        "TestQaCaptureStateArgumentsSelectMidChainRouteState",
        "Config should preserve the requested low dock drain access capture state.");

    const char* vehicleArgv[] = {"EngineApp", "--qa-capture-state", "vehicle-dock-road-forward"};
    const auto vehicleResult = engine::ParseArguments(3, vehicleArgv);
    Expect(vehicleResult.errors.empty(),
        "TestQaCaptureStateArgumentsSelectMidChainRouteState",
        "QA capture state should accept the vehicle dock-road scenario.");
    Expect(vehicleResult.config.qaCaptureState == "vehicle-dock-road-forward",
        "TestQaCaptureStateArgumentsSelectMidChainRouteState",
        "Config should preserve the requested vehicle capture state.");

    const char* badArgv[] = {"EngineApp", "--qa-capture-state", "unknown"};
    const auto badResult = engine::ParseArguments(3, badArgv);
    Expect(!badResult.errors.empty(),
        "TestQaCaptureStateArgumentsSelectMidChainRouteState",
        "Unknown QA capture states should be rejected.");
}

void TestVehicleRuntimeArgumentsDefaultToDeterministicFallback()
{
    const char* argv[] = {"EngineApp"};
    const auto result = engine::ParseArguments(1, argv);

    Expect(result.errors.empty(),
        "TestVehicleRuntimeArgumentsDefaultToDeterministicFallback",
        "Default arguments should parse cleanly.");
    Expect(!result.config.vehicleRuntimeAdapterEnabled,
        "TestVehicleRuntimeArgumentsDefaultToDeterministicFallback",
        "Live vehicle runtime adapters should be off by default.");
    Expect(result.config.vehicleRuntimeBackend == engine::physics::PhysicsBackend::Simple,
        "TestVehicleRuntimeArgumentsDefaultToDeterministicFallback",
        "The deterministic fallback should keep the simple backend as the neutral default value.");
}

void TestVehicleRuntimeArgumentsAcceptPreferred()
{
    const char* argv[] = {"EngineApp", "--vehicle-runtime", "preferred"};
    const auto result = engine::ParseArguments(3, argv);

    Expect(result.errors.empty(),
        "TestVehicleRuntimeArgumentsAcceptPreferred",
        "The preferred live vehicle runtime argument should parse cleanly.");
    Expect(result.config.vehicleRuntimeAdapterEnabled == engine::physics::IsJoltPhysicsAvailable(),
        "TestVehicleRuntimeArgumentsAcceptPreferred",
        "Preferred runtime should enable the live adapter only when the Jolt backend is available.");
    const engine::physics::PhysicsBackend expectedBackend = engine::physics::IsJoltPhysicsAvailable()
        ? engine::physics::PhysicsBackend::Jolt
        : engine::physics::PhysicsBackend::Simple;
    Expect(result.config.vehicleRuntimeBackend == expectedBackend,
        "TestVehicleRuntimeArgumentsAcceptPreferred",
        "Preferred runtime should choose Jolt in opt-in builds and deterministic fallback otherwise.");
}

void TestVehicleRuntimeArgumentsAcceptJoltOptIn()
{
    const char* argv[] = {"EngineApp", "--vehicle-runtime", "jolt"};
    const auto result = engine::ParseArguments(3, argv);

    Expect(result.errors.empty(),
        "TestVehicleRuntimeArgumentsAcceptJoltOptIn",
        "The opt-in live vehicle runtime argument should parse cleanly.");
    Expect(result.config.vehicleRuntimeAdapterEnabled,
        "TestVehicleRuntimeArgumentsAcceptJoltOptIn",
        "Selecting the opt-in runtime should enable live vehicle adapter driving.");
    Expect(result.config.vehicleRuntimeBackend == engine::physics::PhysicsBackend::Jolt,
        "TestVehicleRuntimeArgumentsAcceptJoltOptIn",
        "The opt-in runtime should select the Jolt physics backend.");
}

void TestVehicleRuntimeArgumentsRejectUnknownRuntime()
{
    const char* argv[] = {"EngineApp", "--vehicle-runtime", "arcade"};
    const auto result = engine::ParseArguments(3, argv);

    Expect(!result.errors.empty(),
        "TestVehicleRuntimeArgumentsRejectUnknownRuntime",
        "Unknown live vehicle runtime backends should be rejected.");
}

void TestHelpTextMentionsQaPhysicsParityFlags()
{
    const std::string help = engine::BuildHelpText();

    Expect(help.find("--qa-physics-parity") != std::string::npos,
        "TestHelpTextMentionsQaPhysicsParityFlags",
        "Help text should document the QA physics parity scenario flag.");
    Expect(help.find("--qa-physics-report") != std::string::npos,
        "TestHelpTextMentionsQaPhysicsParityFlags",
        "Help text should document the QA physics parity report flag.");
}

void TestHelpTextMentionsVehicleRuntimeFlag()
{
    const std::string help = engine::BuildHelpText();

    Expect(help.find("--vehicle-runtime") != std::string::npos,
        "TestHelpTextMentionsVehicleRuntimeFlag",
        "Help text should document the live vehicle runtime opt-in flag.");
}

void TestCursorCaptureArguments()
{
    {
        const char* argv[] = {"EngineApp"};
        const auto result = engine::ParseArguments(1, argv);

        Expect(result.errors.empty(), "TestCursorCaptureArguments", "Default arguments should parse cleanly.");
        Expect(result.config.captureCursor, "TestCursorCaptureArguments", "Windowed play should default to captured cursor mode.");
    }

    {
        const char* argv[] = {"EngineApp", "--free-cursor"};
        const auto result = engine::ParseArguments(2, argv);

        Expect(result.errors.empty(), "TestCursorCaptureArguments", "Free cursor argument should parse cleanly.");
        Expect(!result.config.captureCursor, "TestCursorCaptureArguments", "Free cursor argument should disable cursor capture.");
    }

    {
        const char* argv[] = {"EngineApp", "--show-cursor"};
        const auto result = engine::ParseArguments(2, argv);

        Expect(result.errors.empty(), "TestCursorCaptureArguments", "Show cursor alias should parse cleanly.");
        Expect(!result.config.captureCursor, "TestCursorCaptureArguments", "Show cursor alias should disable cursor capture.");
    }

    {
        const char* argv[] = {"EngineApp", "--free-cursor", "--capture-cursor"};
        const auto result = engine::ParseArguments(3, argv);

        Expect(result.errors.empty(), "TestCursorCaptureArguments", "Cursor mode arguments should parse cleanly.");
        Expect(result.config.captureCursor, "TestCursorCaptureArguments", "Later capture cursor argument should win.");
    }
}

void TestUiModeArguments()
{
    {
        const char* argv[] = {"EngineApp"};
        const auto result = engine::ParseArguments(1, argv);

        Expect(result.errors.empty(), "TestUiModeArguments", "Default arguments should parse cleanly.");
        Expect(result.config.uiMode == engine::UiMode::Playtest,
            "TestUiModeArguments",
            "Windowed play should default to player-facing playtest UI mode.");
    }

    {
        const char* argv[] = {"EngineApp", "--ui-mode", "debug"};
        const auto result = engine::ParseArguments(3, argv);

        Expect(result.errors.empty(), "TestUiModeArguments", "Debug UI mode should parse cleanly.");
        Expect(result.config.uiMode == engine::UiMode::Debug,
            "TestUiModeArguments",
            "Debug UI mode should preserve full development telemetry.");
    }

    {
        const char* argv[] = {"EngineApp", "--ui-mode=minimal"};
        const auto result = engine::ParseArguments(2, argv);

        Expect(result.errors.empty(), "TestUiModeArguments", "Minimal UI mode should parse cleanly.");
        Expect(result.config.uiMode == engine::UiMode::Minimal,
            "TestUiModeArguments",
            "Minimal UI mode should expose the smallest player-facing overlay.");
    }

    {
        const char* argv[] = {"EngineApp", "--debug-ui"};
        const auto result = engine::ParseArguments(2, argv);

        Expect(result.errors.empty(), "TestUiModeArguments", "Debug UI alias should parse cleanly.");
        Expect(result.config.uiMode == engine::UiMode::Debug,
            "TestUiModeArguments",
            "Debug UI alias should select full telemetry mode.");
    }

    {
        const char* argv[] = {"EngineApp", "--playtest-ui"};
        const auto result = engine::ParseArguments(2, argv);

        Expect(result.errors.empty(), "TestUiModeArguments", "Playtest UI alias should parse cleanly.");
        Expect(result.config.uiMode == engine::UiMode::Playtest,
            "TestUiModeArguments",
            "Playtest UI alias should select the player-facing overlay.");
    }

    {
        const char* argv[] = {"EngineApp", "--ui-mode", "cinematic"};
        const auto result = engine::ParseArguments(3, argv);

        Expect(!result.errors.empty(), "TestUiModeArguments", "Invalid UI mode should emit an error.");
    }
}

void TestInputStateTracksDebugOverlayToggleEdge()
{
    engine::InputState input;
    input.debugOverlayTogglePressed = true;

    Expect(input.debugOverlayTogglePressed,
        "TestInputStateTracksDebugOverlayToggleEdge",
        "InputState should expose a pressed-edge debug overlay toggle for F1.");
}

void TestSceneArgumentSelectsRuntimeScenePath()
{
    const char* argv[] = {"EngineApp", "--scene", "data/scenes/ferry_office.scene.json"};
    const auto result = engine::ParseArguments(3, argv);

    Expect(result.errors.empty(),
        "TestSceneArgumentSelectsRuntimeScenePath",
        "Scene path argument should parse cleanly.");
    Expect(result.config.scenePath.generic_string() == "data/scenes/ferry_office.scene.json",
        "TestSceneArgumentSelectsRuntimeScenePath",
        "Config should preserve the selected runtime scene path.");
}

void TestNormalizePathKeepsAssetPathsInsideBase()
{
    const std::filesystem::path base = "C:/project";
    const auto normalized = engine::NormalizePath(base, "assets/../assets/grid.txt");

    Expect(normalized.generic_string() == "C:/project/assets/grid.txt",
        "TestNormalizePathKeepsAssetPathsInsideBase",
        "Normalized path should collapse parent directory segments.");
}

void TestClockStartsAtFrameZeroAndTicksForward()
{
    engine::Clock clock;
    clock.reset();
    Expect(clock.frameIndex() == 0, "TestClockStartsAtFrameZeroAndTicksForward", "Reset clock should start at frame zero.");
    const double delta = clock.tick();
    Expect(clock.frameIndex() == 1, "TestClockStartsAtFrameZeroAndTicksForward", "Tick should advance frame index.");
    Expect(delta >= 0.0, "TestClockStartsAtFrameZeroAndTicksForward", "Delta time should not be negative.");
}

void TestNullRendererRecordsFrameAndDebugDraw()
{
    engine::NullRenderer renderer;
    engine::RendererConfig config;
    config.headless = true;

    Expect(renderer.initialize(config), "TestNullRendererRecordsFrameAndDebugDraw", "Null renderer should initialize.");
    renderer.beginFrame(7);
    renderer.drawDebugGridAndAxes();
    renderer.drawDebugSolidBox({0.0f, 0.5f, 0.0f}, {1.0f, 0.5f, 1.0f}, {0.4f, 0.6f, 0.8f, 1.0f});
    const std::array<engine::Vec3, 3> triangle {{
        {-0.5f, 0.0f, 0.0f},
        {0.5f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
    }};
    renderer.drawDebugFlatTriangles(triangle, {0.4f, 0.6f, 0.8f, 1.0f});
    renderer.endFrame();

    Expect(renderer.frameCount() == 7, "TestNullRendererRecordsFrameAndDebugDraw", "Null renderer should record frame index.");
    Expect(renderer.debugDrawCount() == 3, "TestNullRendererRecordsFrameAndDebugDraw", "Null renderer should count grid, solid box, and flat mesh draw calls.");
    renderer.shutdown();
}

void TestBmpWriterCreatesTopDown32BitBmp()
{
    const std::filesystem::path outputPath = std::filesystem::temp_directory_path() / "tidebreak-v029-bmp-writer-test.bmp";
    std::filesystem::remove(outputPath);

    const std::array<std::uint8_t, 8> bgraPixels {{
        0, 0, 255, 255,
        0, 255, 0, 255,
    }};

    const bool written = engine::WriteBgraBmp(outputPath, 2, 1, bgraPixels);
    const std::vector<std::uint8_t> bytes = ReadBinaryFile(outputPath);

    Expect(written, "TestBmpWriterCreatesTopDown32BitBmp", "BMP writer should report success for valid BGRA pixels.");
    Expect(bytes.size() == 62, "TestBmpWriterCreatesTopDown32BitBmp", "2x1 32-bit BMP should contain a 54-byte header plus 8 bytes of pixels.");
    if (bytes.size() >= 62) {
        Expect(bytes[0] == 'B' && bytes[1] == 'M',
            "TestBmpWriterCreatesTopDown32BitBmp",
            "BMP file should start with the BM signature.");
        Expect(ReadLittleEndianU32(bytes, 2) == 62,
            "TestBmpWriterCreatesTopDown32BitBmp",
            "BMP file size should be written into the file header.");
        Expect(ReadLittleEndianU32(bytes, 10) == 54,
            "TestBmpWriterCreatesTopDown32BitBmp",
            "BMP pixel data offset should point after the file and info headers.");
        Expect(ReadLittleEndianI32(bytes, 18) == 2 && ReadLittleEndianI32(bytes, 22) == -1,
            "TestBmpWriterCreatesTopDown32BitBmp",
            "BMP info header should store width and a negative top-down height.");
        Expect(ReadLittleEndianU16(bytes, 28) == 32,
            "TestBmpWriterCreatesTopDown32BitBmp",
            "BMP writer should emit 32-bit BGRA pixels.");
        Expect(std::equal(bgraPixels.begin(), bgraPixels.end(), bytes.begin() + 54),
            "TestBmpWriterCreatesTopDown32BitBmp",
            "BMP payload should preserve the input top-down BGRA pixels.");
    }

    std::filesystem::remove(outputPath);
}

void TestDebugBitmapTextBuildsGlyphQuadsInsideBounds()
{
    engine::DebugBitmapTextLayout layout;
    layout.viewportWidth = 160;
    layout.viewportHeight = 72;
    layout.originX = 4;
    layout.originY = 5;
    layout.glyphScale = 2;
    layout.lineSpacing = 3;
    layout.rightPadding = 4;
    layout.bottomPadding = 4;

    const std::vector<engine::DebugBitmapTextQuad> quads =
        engine::BuildDebugBitmapTextQuads("Objective:\nPrompt E", layout);

    Expect(!quads.empty(),
        "TestDebugBitmapTextBuildsGlyphQuadsInsideBounds",
        "Debug bitmap text should emit visible glyph quads.");

    bool foundSecondLine = false;
    const int secondLineTop = layout.originY + (7 * layout.glyphScale) + layout.lineSpacing;
    for (const engine::DebugBitmapTextQuad& quad : quads) {
        Expect(quad.left >= layout.originX
                && quad.top >= layout.originY
                && quad.right <= layout.viewportWidth - layout.rightPadding
                && quad.bottom <= layout.viewportHeight - layout.bottomPadding,
            "TestDebugBitmapTextBuildsGlyphQuadsInsideBounds",
            "Debug bitmap text quads should stay inside the configured viewport padding.");
        if (quad.top >= secondLineTop) {
            foundSecondLine = true;
        }
    }

    Expect(foundSecondLine,
        "TestDebugBitmapTextBuildsGlyphQuadsInsideBounds",
        "Debug bitmap text should honor newline layout.");
}

void TestDebugBitmapTextWrapsLongLinesBeforeOverflow()
{
    engine::DebugBitmapTextLayout layout;
    layout.viewportWidth = 42;
    layout.viewportHeight = 80;
    layout.originX = 2;
    layout.originY = 2;
    layout.glyphScale = 1;
    layout.glyphSpacing = 1;
    layout.lineSpacing = 1;
    layout.rightPadding = 2;
    layout.bottomPadding = 2;

    const std::vector<engine::DebugBitmapTextQuad> quads =
        engine::BuildDebugBitmapTextQuads("ABCDE ABCDE", layout);

    Expect(!quads.empty(),
        "TestDebugBitmapTextWrapsLongLinesBeforeOverflow",
        "Debug bitmap text should emit quads for wrapped text.");

    bool wrapped = false;
    const int firstLineBottom = layout.originY + (7 * layout.glyphScale);
    for (const engine::DebugBitmapTextQuad& quad : quads) {
        Expect(quad.right <= layout.viewportWidth - layout.rightPadding,
            "TestDebugBitmapTextWrapsLongLinesBeforeOverflow",
            "Wrapped debug bitmap text should not overflow the right edge.");
        if (quad.top >= firstLineBottom + layout.lineSpacing) {
            wrapped = true;
        }
    }

    Expect(wrapped,
        "TestDebugBitmapTextWrapsLongLinesBeforeOverflow",
        "Debug bitmap text should wrap long lines before they overflow.");
}

void TestDebugProjectionKeepsLongVisibleLinesWhenEndpointsAreOffscreen()
{
    engine::DebugCamera camera;
    camera.position = {0.0f, 1.0f, -4.0f};
    camera.target = {0.0f, 1.0f, 0.0f};

    engine::ProjectedPoint from;
    engine::ProjectedPoint to;
    const bool projected = engine::ProjectWorldLine(
        camera,
        16.0f / 9.0f,
        {-20.0f, 1.0f, 5.0f},
        {20.0f, 1.0f, 5.0f},
        from,
        to);

    Expect(projected,
        "TestDebugProjectionKeepsLongVisibleLinesWhenEndpointsAreOffscreen",
        "Debug projection should keep long ground and route lines if the segment crosses the camera view.");
    Expect(from.x < -1.0f && to.x > 1.0f,
        "TestDebugProjectionKeepsLongVisibleLinesWhenEndpointsAreOffscreen",
        "Projected debug line endpoints should be allowed to sit outside the viewport for renderer clipping.");
}

void TestDebugProjectionClipsLinesAndTrianglesAgainstNearPlane()
{
    engine::DebugCamera camera;
    camera.position = {0.0f, 1.0f, 0.0f};
    camera.target = {0.0f, 1.0f, 1.0f};

    engine::ProjectedPoint lineFrom;
    engine::ProjectedPoint lineTo;
    const bool lineProjected = engine::ProjectWorldLine(
        camera,
        16.0f / 9.0f,
        {-1.0f, 1.0f, -1.0f},
        {1.0f, 1.0f, 5.0f},
        lineFrom,
        lineTo);

    Expect(lineProjected,
        "TestDebugProjectionClipsLinesAndTrianglesAgainstNearPlane",
        "Debug lines crossing the near plane should be clipped instead of disappearing.");

    engine::ProjectedPolygon triangle;
    const bool triangleProjected = engine::ProjectWorldTriangle(
        camera,
        16.0f / 9.0f,
        {-3.0f, 0.0f, -1.0f},
        {3.0f, 0.0f, 4.0f},
        {-3.0f, 0.0f, 4.0f},
        triangle);

    Expect(triangleProjected && triangle.pointCount >= 3,
        "TestDebugProjectionClipsLinesAndTrianglesAgainstNearPlane",
        "Debug solid ground triangles crossing the near plane should still produce a visible clipped polygon.");
}

void TestDebugProjectionSortsProjectedTrianglesBackToFront()
{
    engine::DebugCamera camera;
    camera.position = {0.0f, 1.0f, 0.0f};
    camera.target = {0.0f, 1.0f, 1.0f};

    engine::ProjectedTriangle nearTriangle;
    engine::ProjectedTriangle farTriangle;
    const bool nearProjected = engine::ProjectWorldTriangleWithDepth(
        camera,
        16.0f / 9.0f,
        {-0.5f, 0.5f, 2.0f},
        {0.5f, 0.5f, 2.0f},
        {0.0f, 1.5f, 2.0f},
        nearTriangle);
    const bool farProjected = engine::ProjectWorldTriangleWithDepth(
        camera,
        16.0f / 9.0f,
        {-0.5f, 0.5f, 7.0f},
        {0.5f, 0.5f, 7.0f},
        {0.0f, 1.5f, 7.0f},
        farTriangle);

    Expect(nearProjected && farProjected,
        "TestDebugProjectionSortsProjectedTrianglesBackToFront",
        "Depth-aware debug projection should still project near and far triangles.");
    Expect(farTriangle.cameraDepth > nearTriangle.cameraDepth,
        "TestDebugProjectionSortsProjectedTrianglesBackToFront",
        "Projected debug triangles should expose camera depth for painter ordering.");

    std::vector<engine::ProjectedTriangle> triangles {nearTriangle, farTriangle};
    engine::SortProjectedTrianglesBackToFront(triangles);
    Expect(triangles.front().cameraDepth == farTriangle.cameraDepth,
        "TestDebugProjectionSortsProjectedTrianglesBackToFront",
        "Painter ordering should place farther projected debug triangles before nearer ones.");
}

void TestDebugWorldToClipMatrixMatchesCpuProjection()
{
    engine::DebugCamera camera;
    camera.position = {1.0f, 2.0f, -4.0f};
    camera.target = {0.0f, 1.0f, 3.0f};
    camera.nearPlane = 0.1f;

    engine::DebugWorldToClipMatrix matrix;
    const bool built = engine::BuildDebugWorldToClipMatrix(camera, 16.0f / 9.0f, matrix, 120.0f);

    const engine::Vec3 point {0.4f, 1.6f, 5.5f};
    engine::ProjectedPoint projected;
    const bool cpuProjected = engine::ProjectWorldPoint(camera, 16.0f / 9.0f, point, projected);
    const engine::DebugClipPoint clipPoint = engine::TransformWorldPoint(matrix, point);

    Expect(built,
        "TestDebugWorldToClipMatrixMatchesCpuProjection",
        "Debug world-to-clip matrix should build for a valid camera.");
    Expect(cpuProjected && clipPoint.w > camera.nearPlane,
        "TestDebugWorldToClipMatrixMatchesCpuProjection",
        "The chosen world point should be in front of the debug camera.");
    ExpectNear(clipPoint.x / clipPoint.w, projected.x, 0.0005f,
        "TestDebugWorldToClipMatrixMatchesCpuProjection",
        "Matrix projection X should match the existing CPU debug projection.");
    ExpectNear(clipPoint.y / clipPoint.w, projected.y, 0.0005f,
        "TestDebugWorldToClipMatrixMatchesCpuProjection",
        "Matrix projection Y should match the existing CPU debug projection.");
}

void TestDebugWorldToClipMatrixMapsDepthIntoDx11Range()
{
    engine::DebugCamera camera;
    camera.position = {0.0f, 1.0f, 0.0f};
    camera.target = {0.0f, 1.0f, 1.0f};
    camera.nearPlane = 0.1f;

    engine::DebugWorldToClipMatrix matrix;
    const bool built = engine::BuildDebugWorldToClipMatrix(camera, 1.0f, matrix, 100.0f);

    const engine::DebugClipPoint nearPoint = engine::TransformWorldPoint(matrix, {0.0f, 1.0f, 0.1f});
    const engine::DebugClipPoint midPoint = engine::TransformWorldPoint(matrix, {0.0f, 1.0f, 10.0f});
    const engine::DebugClipPoint farPoint = engine::TransformWorldPoint(matrix, {0.0f, 1.0f, 100.0f});

    Expect(built,
        "TestDebugWorldToClipMatrixMapsDepthIntoDx11Range",
        "Debug world-to-clip matrix should build for a forward-facing camera.");
    ExpectNear(nearPoint.z / nearPoint.w, 0.0f, 0.0005f,
        "TestDebugWorldToClipMatrixMapsDepthIntoDx11Range",
        "The near plane should map to DirectX NDC depth 0.");
    Expect(midPoint.z / midPoint.w > 0.0f && midPoint.z / midPoint.w < 1.0f,
        "TestDebugWorldToClipMatrixMapsDepthIntoDx11Range",
        "A point between near and far should map inside DirectX NDC depth 0..1.");
    ExpectNear(farPoint.z / farPoint.w, 1.0f, 0.0005f,
        "TestDebugWorldToClipMatrixMapsDepthIntoDx11Range",
        "The far plane should map to DirectX NDC depth 1.");
}

void TestStaticMeshLoaderLoadsCommittedUnitBox()
{
    const std::filesystem::path meshPath = std::filesystem::path(ENGINE_SOURCE_ROOT) / "assets" / "models" / "unit_box.gltf";
    const engine::StaticMeshLoadResult result = engine::LoadStaticMeshFromGltf(meshPath);

    Expect(result.ok(),
        "TestStaticMeshLoaderLoadsCommittedUnitBox",
        "Committed unit box glTF should load successfully.");
    Expect(result.mesh.vertices.size() == 8,
        "TestStaticMeshLoaderLoadsCommittedUnitBox",
        "Unit box should expose eight unique position vertices.");
    Expect(result.mesh.indices.size() == 36,
        "TestStaticMeshLoaderLoadsCommittedUnitBox",
        "Unit box should expose twelve indexed triangles.");
    ExpectNear(result.mesh.bounds.min.x, -0.5f, 0.001f,
        "TestStaticMeshLoaderLoadsCommittedUnitBox",
        "Unit box min x should be -0.5m.");
    ExpectNear(result.mesh.bounds.max.y, 0.5f, 0.001f,
        "TestStaticMeshLoaderLoadsCommittedUnitBox",
        "Unit box max y should be 0.5m.");
}

void TestStaticMeshLoaderLoadsV018PropKit()
{
    const std::array<std::filesystem::path, 4> propPaths {{
        std::filesystem::path(ENGINE_SOURCE_ROOT) / "assets" / "models" / "service_road_sign.gltf",
        std::filesystem::path(ENGINE_SOURCE_ROOT) / "assets" / "models" / "road_edge_post.gltf",
        std::filesystem::path(ENGINE_SOURCE_ROOT) / "assets" / "models" / "service_barrier.gltf",
        std::filesystem::path(ENGINE_SOURCE_ROOT) / "assets" / "models" / "utility_box.gltf",
    }};

    for (const std::filesystem::path& meshPath : propPaths) {
        const engine::StaticMeshLoadResult result = engine::LoadStaticMeshFromGltf(meshPath);

        Expect(result.ok(),
            "TestStaticMeshLoaderLoadsV018PropKit",
            "Every v0.18 original prop mesh should load through the tiny glTF subset.");
        if (result.ok()) {
            Expect(!result.mesh.vertices.empty(),
                "TestStaticMeshLoaderLoadsV018PropKit",
                "v0.18 prop meshes should expose position vertices.");
            Expect(result.mesh.indices.size() >= 3 && result.mesh.indices.size() % 3 == 0,
                "TestStaticMeshLoaderLoadsV018PropKit",
                "v0.18 prop meshes should expose triangle indices.");
            Expect(result.mesh.bounds.max.y > result.mesh.bounds.min.y,
                "TestStaticMeshLoaderLoadsV018PropKit",
                "v0.18 prop meshes should have useful vertical bounds.");
        }
    }
}

void TestStaticMeshLoaderReportsMissingAsset()
{
    const std::filesystem::path meshPath = std::filesystem::path(ENGINE_SOURCE_ROOT) / "assets" / "models" / "missing_mesh.gltf";
    const engine::StaticMeshLoadResult result = engine::LoadStaticMeshFromGltf(meshPath);

    Expect(!result.ok(),
        "TestStaticMeshLoaderReportsMissingAsset",
        "Missing glTF asset should fail clearly.");
    Expect(result.error.find("not found") != std::string::npos,
        "TestStaticMeshLoaderReportsMissingAsset",
        "Missing glTF asset failure should mention not found.");
}

void TestStaticMeshLoaderReportsInvalidJsonTypesWithoutThrowing()
{
    const std::filesystem::path meshPath = std::filesystem::temp_directory_path() / "tidebreak_bad_accessor_type.gltf";
    {
        std::ofstream file(meshPath);
        file << R"({
            "buffers": [{"uri": "data:application/octet-stream;base64,AA=="}],
            "bufferViews": [{"buffer": 0, "byteOffset": 0, "byteLength": 1}],
            "accessors": [{"bufferView": 0, "componentType": 5126, "count": 1, "type": "VEC3"}],
            "meshes": [{"primitives": [{"attributes": {"POSITION": "not-an-index"}, "indices": 0}]}]
        })";
    }

    bool threw = false;
    engine::StaticMeshLoadResult result;
    try {
        result = engine::LoadStaticMeshFromGltf(meshPath);
    } catch (...) {
        threw = true;
    }
    std::filesystem::remove(meshPath);

    Expect(!threw,
        "TestStaticMeshLoaderReportsInvalidJsonTypesWithoutThrowing",
        "Invalid but parseable glTF JSON should report a load error instead of throwing.");
    Expect(!result.ok() && !result.error.empty(),
        "TestStaticMeshLoaderReportsInvalidJsonTypesWithoutThrowing",
        "Invalid glTF accessor types should produce a clear loader error.");
}

void TestStaticMeshLoaderHonorsInterleavedPositionStride()
{
    const std::filesystem::path meshPath = std::filesystem::temp_directory_path() / "tidebreak_interleaved_positions.gltf";
    {
        std::ofstream file(meshPath);
        file << R"({
            "asset": {"version": "2.0"},
            "buffers": [{"uri": "data:application/octet-stream;base64,AAAAAAAAAAAAAAAAAADGQgAAgD8AAAAAAAAAAAAAxkIAAAAAAACAPwAAAAAAAMZCAAABAAIA", "byteLength": 54}],
            "bufferViews": [
                {"buffer": 0, "byteOffset": 0, "byteLength": 48, "byteStride": 16},
                {"buffer": 0, "byteOffset": 48, "byteLength": 6}
            ],
            "accessors": [
                {"bufferView": 0, "componentType": 5126, "count": 3, "type": "VEC3"},
                {"bufferView": 1, "componentType": 5123, "count": 3, "type": "SCALAR"}
            ],
            "meshes": [{"primitives": [{"attributes": {"POSITION": 0}, "indices": 1}]}]
        })";
    }

    const engine::StaticMeshLoadResult result = engine::LoadStaticMeshFromGltf(meshPath);

    Expect(result.ok(),
        "TestStaticMeshLoaderHonorsInterleavedPositionStride",
        result.error.empty() ? "Interleaved POSITION data should load." : result.error);
    ExpectNear(result.mesh.bounds.max.x, 1.0f, 0.001f,
        "TestStaticMeshLoaderHonorsInterleavedPositionStride",
        "The loader should advance POSITION reads by byteStride instead of treating padding as vertex X data.");
    ExpectNear(result.mesh.bounds.max.y, 1.0f, 0.001f,
        "TestStaticMeshLoaderHonorsInterleavedPositionStride",
        "The loader should advance POSITION reads by byteStride instead of treating padding as vertex Y data.");
}

void TestStaticMeshBuildsTransformedTriangleList()
{
    engine::StaticMeshAsset mesh;
    mesh.vertices = {
        {{-0.5f, 0.0f, 0.0f}},
        {{0.5f, 0.0f, 0.0f}},
        {{0.0f, 1.0f, 0.0f}},
    };
    mesh.indices = {0, 1, 2};
    mesh.bounds = engine::ComputeBounds(mesh.vertices);

    engine::StaticMeshInstance instance;
    instance.position = {2.0f, 0.0f, 3.0f};
    instance.scale = {2.0f, 1.0f, 1.0f};
    const std::vector<engine::Vec3> triangles = engine::BuildFlatTriangleList(mesh, instance);

    Expect(triangles.size() == 3,
        "TestStaticMeshBuildsTransformedTriangleList",
        "One indexed triangle should expand to three renderer vertices.");
    ExpectNear(triangles[0].x, 1.0f, 0.001f,
        "TestStaticMeshBuildsTransformedTriangleList",
        "Instance scale and translation should affect x position.");
    ExpectNear(triangles[2].y, 1.0f, 0.001f,
        "TestStaticMeshBuildsTransformedTriangleList",
        "Instance transform should preserve local y after scale.");
}

void TestSceneLoaderLoadsDefaultFerryOfficeScene()
{
    const SceneLoadResult result = LoadSceneDefinition(DefaultScenePathForTests());

    Expect(result.ok(),
        "TestSceneLoaderLoadsDefaultFerryOfficeScene",
        "Default Ferry Office scene JSON should load successfully.");
    Expect(result.scene.id == "ferry-office",
        "TestSceneLoaderLoadsDefaultFerryOfficeScene",
        "Loaded scene should expose the authored scene id.");
    Expect(result.scene.colliders.size() == 10,
        "TestSceneLoaderLoadsDefaultFerryOfficeScene",
        "Loaded scene should expose authored static colliders.");
    Expect(result.scene.visualPlaceholders.size() == 32,
        "TestSceneLoaderLoadsDefaultFerryOfficeScene",
        "Loaded scene should expose authored visual placeholders.");
    Expect(result.scene.sceneMaterials.size() == 25,
        "TestSceneLoaderLoadsDefaultFerryOfficeScene",
        "Loaded scene should expose authored presentation materials for every current scene color key.");
    Expect(result.scene.meshInstances.size() >= 15,
        "TestSceneLoaderLoadsDefaultFerryOfficeScene",
        "Loaded scene should expose authored mesh instances, including the v0.18 prop style kit.");
    Expect(result.scene.interactables.size() == 17,
        "TestSceneLoaderLoadsDefaultFerryOfficeScene",
        "Loaded scene should expose authored interactables.");
    Expect(result.scene.traversalAffordances.size() == 1,
        "TestSceneLoaderLoadsDefaultFerryOfficeScene",
        "Loaded scene should expose authored traversal affordances.");
    Expect(IsFerryOfficeRegressionScene(result.scene),
        "TestSceneLoaderLoadsDefaultFerryOfficeScene",
        "Ferry Office should identify as the regression scene at runtime.");
}

void TestSceneLoaderLoadsPilotSliceMetadata()
{
    const SceneLoadResult result = LoadSceneDefinition(PilotScenePathForTests());

    Expect(result.ok(),
        "TestSceneLoaderLoadsPilotSliceMetadata",
        "Pilot target-slice scene JSON should load successfully.");
    Expect(result.scene.id == "veyra-reach-pilot",
        "TestSceneLoaderLoadsPilotSliceMetadata",
        "Pilot scene should expose its authored scene id.");
    Expect(result.scene.sliceMetadata.kind == "target-slice-scaffold",
        "TestSceneLoaderLoadsPilotSliceMetadata",
        "Pilot scene should expose target-slice metadata to runtime code.");
    Expect(result.scene.sliceMetadata.worldId == "veyra-reach",
        "TestSceneLoaderLoadsPilotSliceMetadata",
        "Pilot scene should expose its world id.");
    Expect(IsTargetSliceScaffoldScene(result.scene),
        "TestSceneLoaderLoadsPilotSliceMetadata",
        "Pilot scene should identify as a target-slice scaffold.");
    Expect(result.scene.vehicles.empty(),
        "TestSceneLoaderLoadsPilotSliceMetadata",
        "Pilot scaffold should not silently inherit a Ferry Office service vehicle.");
}

void TestSceneRuntimePolicyKeepsFallbackAsFerryOfficeRegression()
{
    SceneDefinition unloadedScene;
    const SceneRuntimePolicy policy = BuildSceneRuntimePolicy(false, unloadedScene);

    Expect(policy.usesFerryOfficeBehavior,
        "TestSceneRuntimePolicyKeepsFallbackAsFerryOfficeRegression",
        "The built-in fallback should preserve Ferry Office regression behavior.");
    Expect(!policy.usesNeutralPresentation,
        "TestSceneRuntimePolicyKeepsFallbackAsFerryOfficeRegression",
        "Fallback should keep the established Ferry Office debug/presentation path.");
    Expect(policy.allowsFallbackVehicle,
        "TestSceneRuntimePolicyKeepsFallbackAsFerryOfficeRegression",
        "Fallback should keep the service-yard vehicle fallback.");
    Expect(policy.drawsWorldStateDebug,
        "TestSceneRuntimePolicyKeepsFallbackAsFerryOfficeRegression",
        "Fallback should keep Ferry Office world-state debug evidence.");
    Expect(policy.guidancePolicy == SceneGuidancePolicy::FerryOfficeActive,
        "TestSceneRuntimePolicyKeepsFallbackAsFerryOfficeRegression",
        "Fallback should use Ferry Office active route/objective guidance policy.");
}

void TestSceneRuntimePolicyClassifiesRegressionAndTargetSliceRoles()
{
    const SceneLoadResult ferry = LoadSceneDefinition(DefaultScenePathForTests());
    const SceneLoadResult pilot = LoadSceneDefinition(PilotScenePathForTests());
    Expect(ferry.ok() && pilot.ok(),
        "TestSceneRuntimePolicyClassifiesRegressionAndTargetSliceRoles",
        "Scene fixtures should load before runtime policy checks.");
    if (!ferry.ok() || !pilot.ok()) {
        return;
    }

    const SceneRuntimePolicy ferryPolicy = BuildSceneRuntimePolicy(true, ferry.scene);
    Expect(ferryPolicy.usesFerryOfficeBehavior,
        "TestSceneRuntimePolicyClassifiesRegressionAndTargetSliceRoles",
        "Ferry Office regression-testbed should use Ferry Office behavior.");
    Expect(!ferryPolicy.usesNeutralPresentation,
        "TestSceneRuntimePolicyClassifiesRegressionAndTargetSliceRoles",
        "Ferry Office regression-testbed should not use neutral target-slice presentation.");
    Expect(ferryPolicy.allowsFallbackVehicle,
        "TestSceneRuntimePolicyClassifiesRegressionAndTargetSliceRoles",
        "Ferry Office regression-testbed should allow the historical fallback vehicle path.");
    Expect(ferryPolicy.drawsWorldStateDebug,
        "TestSceneRuntimePolicyClassifiesRegressionAndTargetSliceRoles",
        "Ferry Office regression-testbed should draw world-state debug evidence.");
    Expect(ferryPolicy.guidancePolicy == SceneGuidancePolicy::FerryOfficeActive,
        "TestSceneRuntimePolicyClassifiesRegressionAndTargetSliceRoles",
        "Ferry Office regression-testbed should use active Ferry Office route/objective guidance.");

    const SceneRuntimePolicy pilotPolicy = BuildSceneRuntimePolicy(true, pilot.scene);
    Expect(!pilotPolicy.usesFerryOfficeBehavior,
        "TestSceneRuntimePolicyClassifiesRegressionAndTargetSliceRoles",
        "Target-slice scaffold should not use Ferry Office behavior.");
    Expect(pilotPolicy.usesNeutralPresentation,
        "TestSceneRuntimePolicyClassifiesRegressionAndTargetSliceRoles",
        "Target-slice scaffold should use neutral presentation.");
    Expect(!pilotPolicy.allowsFallbackVehicle,
        "TestSceneRuntimePolicyClassifiesRegressionAndTargetSliceRoles",
        "Target-slice scaffold should not inherit the fallback service vehicle.");
    Expect(!pilotPolicy.drawsWorldStateDebug,
        "TestSceneRuntimePolicyClassifiesRegressionAndTargetSliceRoles",
        "Target-slice scaffold should not draw Ferry Office world-state debug evidence.");
    Expect(pilotPolicy.guidancePolicy == SceneGuidancePolicy::AllAuthored,
        "TestSceneRuntimePolicyClassifiesRegressionAndTargetSliceRoles",
        "Target-slice scaffold should show its authored route/objective markers without Ferry Office job gating.");
}

void TestSceneRuntimePolicyTreatsUnknownLoadedSceneAsNeutral()
{
    SceneDefinition genericScene;
    genericScene.id = "generic-authored-scene";
    genericScene.sliceMetadata.kind = "experimental-sandbox";

    const SceneRuntimePolicy policy = BuildSceneRuntimePolicy(true, genericScene);

    Expect(!policy.usesFerryOfficeBehavior,
        "TestSceneRuntimePolicyTreatsUnknownLoadedSceneAsNeutral",
        "A loaded non-regression scene should not silently borrow Ferry Office behavior.");
    Expect(policy.usesNeutralPresentation,
        "TestSceneRuntimePolicyTreatsUnknownLoadedSceneAsNeutral",
        "A loaded non-regression scene should use neutral presentation until a stronger role exists.");
    Expect(!policy.allowsFallbackVehicle,
        "TestSceneRuntimePolicyTreatsUnknownLoadedSceneAsNeutral",
        "A loaded non-regression scene should not inherit the service-yard fallback vehicle.");
    Expect(!policy.drawsWorldStateDebug,
        "TestSceneRuntimePolicyTreatsUnknownLoadedSceneAsNeutral",
        "A loaded non-regression scene should not draw Ferry Office world-state debug evidence.");
    Expect(policy.guidancePolicy == SceneGuidancePolicy::AllAuthored,
        "TestSceneRuntimePolicyTreatsUnknownLoadedSceneAsNeutral",
        "A loaded non-regression scene should use neutral authored guidance.");
}

void TestSceneRuntimePackageKeepsFallbackAsFerryOfficeSubmission()
{
    const SceneRuntimePackage package = BuildSceneRuntimePackage(false, SceneDefinition {});

    Expect(package.policy.usesFerryOfficeBehavior,
        "TestSceneRuntimePackageKeepsFallbackAsFerryOfficeSubmission",
        "Fallback package should preserve Ferry Office behavior.");
    Expect(!package.policy.usesNeutralPresentation,
        "TestSceneRuntimePackageKeepsFallbackAsFerryOfficeSubmission",
        "Fallback package should preserve Ferry Office presentation.");
    Expect(package.policy.allowsFallbackVehicle,
        "TestSceneRuntimePackageKeepsFallbackAsFerryOfficeSubmission",
        "Fallback package should keep the fallback service-yard vehicle.");
    Expect(package.renderSubmission.drawsFallbackFerryOfficeMoodBase,
        "TestSceneRuntimePackageKeepsFallbackAsFerryOfficeSubmission",
        "Fallback package should draw the built-in Ferry Office mood base.");
    Expect(!package.renderSubmission.drawsSceneAuthoredVisualPlaceholders,
        "TestSceneRuntimePackageKeepsFallbackAsFerryOfficeSubmission",
        "Fallback package should not claim authored placeholder submission.");
    Expect(package.renderSubmission.drawsFerryOfficeWorldStateCues,
        "TestSceneRuntimePackageKeepsFallbackAsFerryOfficeSubmission",
        "Fallback package should keep Ferry Office world-state cues.");
}

void TestSceneRuntimePackageCombinesFerryOfficePolicyAndAuthoredSubmission()
{
    const SceneLoadResult ferry = LoadSceneDefinition(DefaultScenePathForTests());
    Expect(ferry.ok(),
        "TestSceneRuntimePackageCombinesFerryOfficePolicyAndAuthoredSubmission",
        "Ferry Office fixture should load before package checks.");
    if (!ferry.ok()) {
        return;
    }

    const SceneRuntimePackage package = BuildSceneRuntimePackage(true, ferry.scene);

    Expect(package.policy.usesFerryOfficeBehavior,
        "TestSceneRuntimePackageCombinesFerryOfficePolicyAndAuthoredSubmission",
        "Ferry Office package should preserve regression-testbed behavior.");
    Expect(!package.policy.usesNeutralPresentation,
        "TestSceneRuntimePackageCombinesFerryOfficePolicyAndAuthoredSubmission",
        "Ferry Office package should not use neutral target-slice presentation.");
    Expect(!package.renderSubmission.drawsFallbackFerryOfficeMoodBase,
        "TestSceneRuntimePackageCombinesFerryOfficePolicyAndAuthoredSubmission",
        "Loaded Ferry Office package should use authored scene geometry instead of fallback mood base.");
    Expect(package.renderSubmission.drawsSceneAuthoredVisualPlaceholders,
        "TestSceneRuntimePackageCombinesFerryOfficePolicyAndAuthoredSubmission",
        "Loaded Ferry Office package should submit authored placeholders.");
    Expect(package.renderSubmission.drawsSceneAuthoredMeshes,
        "TestSceneRuntimePackageCombinesFerryOfficePolicyAndAuthoredSubmission",
        "Loaded Ferry Office package should submit authored mesh instances.");
    Expect(package.renderSubmission.drawsFerryOfficeWorldStateCues,
        "TestSceneRuntimePackageCombinesFerryOfficePolicyAndAuthoredSubmission",
        "Loaded Ferry Office package should keep Ferry Office world-state cues.");
}

void TestSceneRuntimePackageCombinesTargetSliceNeutralPolicyAndSubmission()
{
    const SceneLoadResult pilot = LoadSceneDefinition(PilotScenePathForTests());
    Expect(pilot.ok(),
        "TestSceneRuntimePackageCombinesTargetSliceNeutralPolicyAndSubmission",
        "Veyra pilot fixture should load before package checks.");
    if (!pilot.ok()) {
        return;
    }

    const SceneRuntimePackage package = BuildSceneRuntimePackage(true, pilot.scene);

    Expect(!package.policy.usesFerryOfficeBehavior,
        "TestSceneRuntimePackageCombinesTargetSliceNeutralPolicyAndSubmission",
        "Target-slice package should not borrow Ferry Office behavior.");
    Expect(package.policy.usesNeutralPresentation,
        "TestSceneRuntimePackageCombinesTargetSliceNeutralPolicyAndSubmission",
        "Target-slice package should use neutral presentation.");
    Expect(!package.policy.allowsFallbackVehicle,
        "TestSceneRuntimePackageCombinesTargetSliceNeutralPolicyAndSubmission",
        "Target-slice package should not inherit the fallback vehicle.");
    Expect(package.policy.guidancePolicy == SceneGuidancePolicy::AllAuthored,
        "TestSceneRuntimePackageCombinesTargetSliceNeutralPolicyAndSubmission",
        "Target-slice package should use all-authored guidance.");
    Expect(!package.renderSubmission.drawsFallbackFerryOfficeMoodBase,
        "TestSceneRuntimePackageCombinesTargetSliceNeutralPolicyAndSubmission",
        "Target-slice package should not draw Ferry Office fallback mood base.");
    Expect(package.renderSubmission.drawsSceneAuthoredVisualPlaceholders,
        "TestSceneRuntimePackageCombinesTargetSliceNeutralPolicyAndSubmission",
        "Target-slice package should submit authored visual placeholders.");
    Expect(package.renderSubmission.drawsSceneAuthoredMeshes,
        "TestSceneRuntimePackageCombinesTargetSliceNeutralPolicyAndSubmission",
        "Target-slice package should use the authored mesh submission path.");
    Expect(!package.renderSubmission.drawsFerryOfficeWorldStateCues,
        "TestSceneRuntimePackageCombinesTargetSliceNeutralPolicyAndSubmission",
        "Target-slice package should not draw Ferry Office world-state cues.");
}

void TestSceneRuntimePackageTreatsGenericLoadedSceneAsNeutralAuthored()
{
    SceneDefinition genericScene;
    genericScene.id = "generic-authored-scene";
    genericScene.sliceMetadata.kind = "experimental-sandbox";

    const SceneRuntimePackage package = BuildSceneRuntimePackage(true, genericScene);

    Expect(!package.policy.usesFerryOfficeBehavior,
        "TestSceneRuntimePackageTreatsGenericLoadedSceneAsNeutralAuthored",
        "Generic loaded package should not inherit Ferry Office behavior.");
    Expect(package.policy.usesNeutralPresentation,
        "TestSceneRuntimePackageTreatsGenericLoadedSceneAsNeutralAuthored",
        "Generic loaded package should use neutral presentation.");
    Expect(package.policy.guidancePolicy == SceneGuidancePolicy::AllAuthored,
        "TestSceneRuntimePackageTreatsGenericLoadedSceneAsNeutralAuthored",
        "Generic loaded package should use all-authored guidance.");
    Expect(!package.renderSubmission.drawsFallbackFerryOfficeMoodBase,
        "TestSceneRuntimePackageTreatsGenericLoadedSceneAsNeutralAuthored",
        "Generic loaded package should not draw fallback Ferry Office mood base.");
    Expect(package.renderSubmission.drawsSceneAuthoredVisualPlaceholders,
        "TestSceneRuntimePackageTreatsGenericLoadedSceneAsNeutralAuthored",
        "Generic loaded package should submit authored placeholders.");
    Expect(package.renderSubmission.drawsSceneAuthoredMeshes,
        "TestSceneRuntimePackageTreatsGenericLoadedSceneAsNeutralAuthored",
        "Generic loaded package should use authored mesh submission.");
    Expect(!package.renderSubmission.drawsFerryOfficeWorldStateCues,
        "TestSceneRuntimePackageTreatsGenericLoadedSceneAsNeutralAuthored",
        "Generic loaded package should not draw Ferry Office world-state cues.");
}

void TestSceneGuidanceSurfaceShowsAllAuthoredNeutralMarkers()
{
    SceneGuidanceContext context;
    context.guidancePolicy = SceneGuidancePolicy::AllAuthored;
    context.vehicleAvailable = true;

    Interactable interactable;
    interactable.id = 42;
    interactable.name = "Veyra Hillside Relay";
    interactable.enabled = true;
    interactable.consumed = false;

    TraversalAffordance traversal;
    traversal.id = 7;
    traversal.name = "Veyra service ledge";
    traversal.enabled = true;

    Expect(ShouldDrawRouteGuidanceMarker(context, "veyra-hillside-route"),
        "TestSceneGuidanceSurfaceShowsAllAuthoredNeutralMarkers",
        "Neutral target-slice guidance should show authored route markers without Ferry Office active-route ids.");
    Expect(ShouldDrawObjectiveGuidanceMarker(context, "veyra-hillside-objective"),
        "TestSceneGuidanceSurfaceShowsAllAuthoredNeutralMarkers",
        "Neutral target-slice guidance should show authored objective markers without Ferry Office phase names.");
    Expect(ShouldDrawInteractableGuidanceMarker(context, interactable),
        "TestSceneGuidanceSurfaceShowsAllAuthoredNeutralMarkers",
        "Neutral target-slice guidance should show authored interactables without Ferry Office name checks.");
    Expect(ShouldDrawTraversalGuidanceMarker(context, traversal),
        "TestSceneGuidanceSurfaceShowsAllAuthoredNeutralMarkers",
        "Neutral target-slice guidance should show authored traversal markers without Ferry Office job gating.");
    Expect(ShouldDrawVehicleGuidance(context),
        "TestSceneGuidanceSurfaceShowsAllAuthoredNeutralMarkers",
        "Neutral target-slice guidance should show vehicle guidance when the scene actually has a vehicle.");
}

void TestSceneGuidanceSurfaceKeepsFerryOfficeActiveGuidanceScoped()
{
    SceneGuidanceContext context;
    context.guidancePolicy = SceneGuidancePolicy::FerryOfficeActive;
    context.ferryOfficePhase = FerryOfficeJobPhase::CollectManifest;
    context.activeRouteMarkerId = "manifest-to-vault-route";
    context.activeObjectiveMarkerId = "ferry-manifest-marker";

    Interactable futureInteractable;
    futureInteractable.id = 9;
    futureInteractable.name = FerryOffice::Names::ServiceRunMarker;
    futureInteractable.enabled = true;
    futureInteractable.consumed = false;

    TraversalAffordance traversal;
    traversal.id = 5;
    traversal.enabled = true;

    Expect(ShouldDrawRouteGuidanceMarker(context, "manifest-to-vault-route"),
        "TestSceneGuidanceSurfaceKeepsFerryOfficeActiveGuidanceScoped",
        "Ferry Office active guidance should show the context-provided active route.");
    Expect(!ShouldDrawRouteGuidanceMarker(context, "future-dock-road-route"),
        "TestSceneGuidanceSurfaceKeepsFerryOfficeActiveGuidanceScoped",
        "Ferry Office active guidance should hide future route markers.");
    Expect(ShouldDrawObjectiveGuidanceMarker(context, "ferry-manifest-marker"),
        "TestSceneGuidanceSurfaceKeepsFerryOfficeActiveGuidanceScoped",
        "Ferry Office active guidance should show the context-provided active objective.");
    Expect(!ShouldDrawObjectiveGuidanceMarker(context, "service-run-marker"),
        "TestSceneGuidanceSurfaceKeepsFerryOfficeActiveGuidanceScoped",
        "Ferry Office active guidance should hide future objective markers.");
    Expect(!ShouldDrawInteractableGuidanceMarker(context, futureInteractable),
        "TestSceneGuidanceSurfaceKeepsFerryOfficeActiveGuidanceScoped",
        "Ferry Office active guidance should not expose the service-run marker at the manifest phase.");
    Expect(!ShouldDrawTraversalGuidanceMarker(context, traversal),
        "TestSceneGuidanceSurfaceKeepsFerryOfficeActiveGuidanceScoped",
        "Ferry Office active guidance should not expose traversal unless the phase or focus asks for it.");
    Expect(!ShouldDrawVehicleGuidance(context),
        "TestSceneGuidanceSurfaceKeepsFerryOfficeActiveGuidanceScoped",
        "Vehicle guidance should be hidden when no vehicle is available.");
}

void TestSceneRenderSurfacePlansFallbackFerryOfficeSubmission()
{
    const SceneRuntimePolicy policy = BuildSceneRuntimePolicy(false, SceneDefinition {});
    const SceneRenderSubmissionPlan plan = BuildSceneRenderSubmissionPlan(false, policy);

    Expect(plan.drawsFallbackFerryOfficeMoodBase,
        "TestSceneRenderSurfacePlansFallbackFerryOfficeSubmission",
        "Fallback runtime should draw the built-in Ferry Office mood base.");
    Expect(!plan.drawsSceneAuthoredVisualPlaceholders,
        "TestSceneRenderSurfacePlansFallbackFerryOfficeSubmission",
        "Fallback runtime should not claim scene-authored visual placeholder submission.");
    Expect(!plan.drawsSceneAuthoredMeshes,
        "TestSceneRenderSurfacePlansFallbackFerryOfficeSubmission",
        "Fallback runtime should not claim scene-authored mesh submission.");
    Expect(plan.drawsDebugColliders,
        "TestSceneRenderSurfacePlansFallbackFerryOfficeSubmission",
        "Fallback debug rendering should preserve collider visualization.");
    Expect(plan.drawsFerryOfficeWorldStateCues,
        "TestSceneRenderSurfacePlansFallbackFerryOfficeSubmission",
        "Fallback Ferry Office behavior should preserve world-state visual cues.");
}

void TestSceneRenderSurfacePlansLoadedFerryOfficeRegressionSubmission()
{
    const SceneLoadResult ferry = LoadSceneDefinition(DefaultScenePathForTests());
    Expect(ferry.ok(),
        "TestSceneRenderSurfacePlansLoadedFerryOfficeRegressionSubmission",
        "Default Ferry Office scene should load before render-surface planning.");
    if (!ferry.ok()) {
        return;
    }

    const SceneRuntimePolicy policy = BuildSceneRuntimePolicy(true, ferry.scene);
    const SceneRenderSubmissionPlan plan = BuildSceneRenderSubmissionPlan(true, policy);

    Expect(!plan.drawsFallbackFerryOfficeMoodBase,
        "TestSceneRenderSurfacePlansLoadedFerryOfficeRegressionSubmission",
        "Loaded Ferry Office regression scene should use authored scene geometry instead of fallback mood base.");
    Expect(plan.drawsSceneAuthoredVisualPlaceholders,
        "TestSceneRenderSurfacePlansLoadedFerryOfficeRegressionSubmission",
        "Loaded Ferry Office regression scene should submit authored visual placeholders.");
    Expect(plan.drawsSceneAuthoredMeshes,
        "TestSceneRenderSurfacePlansLoadedFerryOfficeRegressionSubmission",
        "Loaded Ferry Office regression scene should submit authored mesh instances.");
    Expect(plan.drawsDebugColliders,
        "TestSceneRenderSurfacePlansLoadedFerryOfficeRegressionSubmission",
        "Loaded Ferry Office debug rendering should keep collider visualization.");
    Expect(plan.drawsFerryOfficeWorldStateCues,
        "TestSceneRenderSurfacePlansLoadedFerryOfficeRegressionSubmission",
        "Loaded Ferry Office regression scene should keep Ferry Office world-state cues.");
}

void TestSceneRenderSurfacePlansTargetSliceNeutralSubmission()
{
    const SceneLoadResult pilot = LoadSceneDefinition("data/scenes/veyra_reach_pilot.scene.json");
    Expect(pilot.ok(),
        "TestSceneRenderSurfacePlansTargetSliceNeutralSubmission",
        "Veyra pilot scene should load before render-surface planning.");
    if (!pilot.ok()) {
        return;
    }

    const SceneRuntimePolicy policy = BuildSceneRuntimePolicy(true, pilot.scene);
    const SceneRenderSubmissionPlan plan = BuildSceneRenderSubmissionPlan(true, policy);

    Expect(!plan.drawsFallbackFerryOfficeMoodBase,
        "TestSceneRenderSurfacePlansTargetSliceNeutralSubmission",
        "Target-slice scaffold should not inherit Ferry Office fallback mood base.");
    Expect(plan.drawsSceneAuthoredVisualPlaceholders,
        "TestSceneRenderSurfacePlansTargetSliceNeutralSubmission",
        "Target-slice scaffold should submit its authored visual placeholders.");
    Expect(plan.drawsSceneAuthoredMeshes,
        "TestSceneRenderSurfacePlansTargetSliceNeutralSubmission",
        "Target-slice scaffold should use the normal authored mesh submission path when mesh data exists.");
    Expect(plan.drawsDebugColliders,
        "TestSceneRenderSurfacePlansTargetSliceNeutralSubmission",
        "Target-slice debug rendering should still allow authored collider visualization.");
    Expect(!plan.drawsFerryOfficeWorldStateCues,
        "TestSceneRenderSurfacePlansTargetSliceNeutralSubmission",
        "Target-slice scaffold should not draw Ferry Office world-state cues.");
}

void TestNeutralSceneRuntimeSurfaceBuildsPresentationWithoutFerryOfficeTerms()
{
    NeutralSceneRuntimeView view;
    view.sceneDefinitionLoaded = true;
    view.sceneName = "Veyra Reach Pilot Slice";
    view.sceneRole = "target-slice-scaffold";
    view.objectiveText = "Inspect neutral slice markers; no authored job is active.";
    view.colliderCount = 1;
    view.interactableCount = 1;
    view.routeCount = 1;
    view.markerCount = 2;
    view.vehicleAvailable = false;

    const std::string text = BuildNeutralScenePresentationText(view, false);

    Expect(text.find("Scene: Veyra Reach Pilot Slice | role=target-slice-scaffold") != std::string::npos,
        "TestNeutralSceneRuntimeSurfaceBuildsPresentationWithoutFerryOfficeTerms",
        "Neutral presentation should identify the loaded target slice.");
    Expect(text.find("Objective: Inspect neutral slice markers; no authored job is active.") != std::string::npos,
        "TestNeutralSceneRuntimeSurfaceBuildsPresentationWithoutFerryOfficeTerms",
        "Neutral presentation should use the authored neutral objective text.");
    Expect(text.find("Status: colliders=1 | interactables=1 | routes=1 | markers=2 | vehicle=none") != std::string::npos,
        "TestNeutralSceneRuntimeSurfaceBuildsPresentationWithoutFerryOfficeTerms",
        "Neutral presentation should expose authored-scene counts without Ferry Office job state.");
    Expect(text.find("Ferry Office") == std::string::npos,
        "TestNeutralSceneRuntimeSurfaceBuildsPresentationWithoutFerryOfficeTerms",
        "Neutral presentation should not mention Ferry Office.");
    Expect(text.find("Job:") == std::string::npos,
        "TestNeutralSceneRuntimeSurfaceBuildsPresentationWithoutFerryOfficeTerms",
        "Neutral presentation should not show a Ferry Office job row.");
    Expect(text.find("service gate=") == std::string::npos,
        "TestNeutralSceneRuntimeSurfaceBuildsPresentationWithoutFerryOfficeTerms",
        "Neutral presentation should not show Ferry Office gate state.");
}

void TestNeutralSceneRuntimeSurfaceBuildsDebugWithoutFerryOfficeTelemetry()
{
    NeutralSceneRuntimeView view;
    view.sceneDefinitionLoaded = true;
    view.sceneId = "veyra-reach-pilot";
    view.sceneRole = "target-slice-scaffold";
    view.worldId = "veyra-reach";
    view.sliceId = "pilot-hillside-service-road";
    view.objectiveText = "Neutral target-slice scaffold loaded.";
    view.completionSummary = "complete=false role=neutral-target-slice";
    view.playerPosition = {-2.0f, 0.0f, -1.5f};
    view.playerYawRadians = engine::Radians(35.0f);
    view.playerHorizontalSpeed = 0.0f;
    view.playerGrounded = true;
    view.cameraYawRadians = engine::Radians(35.0f);
    view.cameraPitchRadians = engine::Radians(22.0f);
    view.cameraDistance = 6.0f;
    view.colliderCount = 1;
    view.interactableCount = 1;
    view.routeCount = 1;
    view.markerCount = 2;
    view.vehicleAvailable = false;
    view.physicsText = "none";
    view.vehicleRuntimeText = "none";

    const std::string text = BuildNeutralSceneDebugText(view);

    Expect(text.find("scene=veyra-reach-pilot loaded=yes role=target-slice-scaffold") != std::string::npos,
        "TestNeutralSceneRuntimeSurfaceBuildsDebugWithoutFerryOfficeTelemetry",
        "Neutral debug text should identify the target-slice runtime role.");
    Expect(text.find("sceneCounts=colliders:1 interactables:1 routes:1 markers:2 vehicle:none") != std::string::npos,
        "TestNeutralSceneRuntimeSurfaceBuildsDebugWithoutFerryOfficeTelemetry",
        "Neutral debug text should report authored-scene counts.");
    Expect(text.find("player=(-2.00,0.00,-1.50)") != std::string::npos,
        "TestNeutralSceneRuntimeSurfaceBuildsDebugWithoutFerryOfficeTelemetry",
        "Neutral debug text should preserve player telemetry.");
    Expect(text.find("roadSegment=dock-road") == std::string::npos,
        "TestNeutralSceneRuntimeSurfaceBuildsDebugWithoutFerryOfficeTelemetry",
        "Neutral debug text should not leak Ferry Office road telemetry.");
    Expect(text.find("roadBounds=") == std::string::npos,
        "TestNeutralSceneRuntimeSurfaceBuildsDebugWithoutFerryOfficeTelemetry",
        "Neutral debug text should not leak Ferry Office vehicle bounds.");
    Expect(text.find("jobObjective=") == std::string::npos,
        "TestNeutralSceneRuntimeSurfaceBuildsDebugWithoutFerryOfficeTelemetry",
        "Neutral debug text should not show Ferry Office job telemetry.");
    Expect(text.find("worldState={") == std::string::npos,
        "TestNeutralSceneRuntimeSurfaceBuildsDebugWithoutFerryOfficeTelemetry",
        "Neutral debug text should not expose Ferry Office world-state telemetry.");
}

void TestSceneLoaderLoadsV018VisualIdentityPropKit()
{
    const SceneLoadResult result = LoadSceneDefinition(DefaultScenePathForTests());

    Expect(result.ok(),
        "TestSceneLoaderLoadsV018VisualIdentityPropKit",
        "Default Ferry Office scene JSON should load before querying v0.18 props.");

    for (const std::string_view assetId : {
             "service-road-sign-mesh",
             "road-edge-post-mesh",
             "service-barrier-mesh",
             "utility-box-mesh",
         }) {
        Expect(FindMeshAsset(result.scene, assetId) != nullptr,
            "TestSceneLoaderLoadsV018VisualIdentityPropKit",
            "Scene data should expose each v0.18 original mesh asset id.");
    }

    for (const std::string_view instanceId : {
             "mesh-service-road-sign",
             "mesh-dock-road-edge-post-a",
             "mesh-dock-road-edge-post-b",
             "mesh-service-yard-barrier-cue",
             "mesh-maintenance-utility-box",
         }) {
        Expect(FindMeshInstance(result.scene, instanceId) != nullptr,
            "TestSceneLoaderLoadsV018VisualIdentityPropKit",
            "Scene data should expose each v0.18 visual identity mesh instance id.");
    }
}

void TestSceneLoaderLoadsFirstJobMarkers()
{
    const SceneLoadResult result = LoadSceneDefinition(DefaultScenePathForTests());
    const SceneInteractableDefinition* confirm = FindSceneInteractable(result.scene, "service-run-confirm-marker");
    const SceneObjectiveMarkerDefinition* checkpoint = FindObjectiveMarker(result.scene, "service-run-checkpoint-marker");
    const SceneRouteMarkerDefinition* route = FindRouteMarker(result.scene, "route-dock-road-to-service-confirm");

    Expect(result.ok(),
        "TestSceneLoaderLoadsFirstJobMarkers",
        "Default scene should load before querying first job markers.");
    Expect(confirm != nullptr,
        "TestSceneLoaderLoadsFirstJobMarkers",
        "Scene data should include a service-run confirmation interactable.");
    if (confirm != nullptr) {
        Expect(confirm->name == FerryOffice::Names::ServiceRunMarker,
            "TestSceneLoaderLoadsFirstJobMarkers",
            "Service-run confirmation marker should preserve its stable gameplay name.");
        Expect(confirm->radius >= 1.5f,
            "TestSceneLoaderLoadsFirstJobMarkers",
            "Service-run confirmation marker should have a usable focus radius.");
    }
    Expect(checkpoint != nullptr,
        "TestSceneLoaderLoadsFirstJobMarkers",
        "Scene data should include a vehicle service-run checkpoint marker.");
    if (checkpoint != nullptr) {
        ExpectNear(checkpoint->position.x, 19.35f, 0.01f,
            "TestSceneLoaderLoadsFirstJobMarkers",
            "Service-run checkpoint should be authored at the dock-road end.");
    }
    Expect(route != nullptr && route->points.size() >= 2,
        "TestSceneLoaderLoadsFirstJobMarkers",
        "Scene data should include a route from the dock road to the service confirmation marker.");
}

void TestSceneLoaderLoadsDockRoadRelayBeat()
{
    const SceneLoadResult result = LoadSceneDefinition(DefaultScenePathForTests());
    const SceneInteractableDefinition* relay = FindSceneInteractable(result.scene, "dock-road-relay");
    const SceneInteractableDefinition* clearanceTag = FindSceneInteractable(result.scene, "dock-road-clearance-tag");
    const SceneRouteMarkerDefinition* route = FindRouteMarker(result.scene, "route-service-confirm-to-relay");
    const SceneRouteMarkerDefinition* clearanceRoute = FindRouteMarker(result.scene, "route-service-log-to-clearance-tag");
    const SceneObjectiveMarkerDefinition* marker = FindObjectiveMarker(result.scene, "dock-road-relay-marker");
    const SceneObjectiveMarkerDefinition* clearanceMarker = FindObjectiveMarker(result.scene, "dock-road-clearance-marker");
    const SceneMeshInstanceDefinition* clearanceTagMesh = FindMeshInstance(result.scene, "mesh-dock-road-clearance-tag");

    Expect(result.ok(),
        "TestSceneLoaderLoadsDockRoadRelayBeat",
        "Default scene should load before querying the dock-road relay beat.");
    Expect(relay != nullptr,
        "TestSceneLoaderLoadsDockRoadRelayBeat",
        "Scene data should include the second service beat relay interactable.");
    if (relay != nullptr) {
        Expect(relay->name == FerryOffice::Names::DockRoadRelay,
            "TestSceneLoaderLoadsDockRoadRelayBeat",
            "Dock-road relay should preserve its stable gameplay name.");
        Expect(relay->radius >= 1.3f,
            "TestSceneLoaderLoadsDockRoadRelayBeat",
            "Dock-road relay should have a usable focus radius.");
    }
    Expect(route != nullptr,
        "TestSceneLoaderLoadsDockRoadRelayBeat",
        "Scene data should route from the service-run marker to the relay.");
    Expect(marker != nullptr,
        "TestSceneLoaderLoadsDockRoadRelayBeat",
        "Scene data should expose an objective marker for the relay.");
    Expect(clearanceTag != nullptr,
        "TestSceneLoaderLoadsDockRoadRelayBeat",
        "Scene data should include the dock-road clearance tag consequence.");
    if (clearanceTag != nullptr) {
        Expect(clearanceTag->name == FerryOffice::Names::DockRoadClearanceTag,
            "TestSceneLoaderLoadsDockRoadRelayBeat",
            "Dock-road clearance tag should preserve its stable gameplay name.");
        Expect(clearanceTag->radius >= 1.3f,
            "TestSceneLoaderLoadsDockRoadRelayBeat",
            "Dock-road clearance tag should have a usable focus radius.");
    }
    Expect(clearanceRoute != nullptr,
        "TestSceneLoaderLoadsDockRoadRelayBeat",
        "Scene data should route from the relay log to the dock-road clearance tag.");
    Expect(clearanceMarker != nullptr,
        "TestSceneLoaderLoadsDockRoadRelayBeat",
        "Scene data should expose an objective marker for the clearance tag.");
    Expect(clearanceTagMesh != nullptr && clearanceTagMesh->assetId == "clearance-tag-mesh",
        "TestSceneLoaderLoadsDockRoadRelayBeat",
        "Scene data should expose an authored clearance-tag mesh prop at the endpoint.");
}

void TestSceneLoaderLoadsHarborPartsJobBeat()
{
    const SceneLoadResult result = LoadSceneDefinition(DefaultScenePathForTests());
    const SceneInteractableDefinition* partsCrate = FindSceneInteractable(result.scene, "harbor-parts-crate");
    const SceneInteractableDefinition* partsShelf = FindSceneInteractable(result.scene, "ferry-office-parts-shelf");
    const SceneRouteMarkerDefinition* crateRoute = FindRouteMarker(result.scene, "route-clearance-tag-to-harbor-parts");
    const SceneRouteMarkerDefinition* shelfRoute = FindRouteMarker(result.scene, "route-harbor-parts-to-office-shelf");
    const SceneObjectiveMarkerDefinition* crateMarker = FindObjectiveMarker(result.scene, "harbor-parts-marker");
    const SceneObjectiveMarkerDefinition* shelfMarker = FindObjectiveMarker(result.scene, "ferry-office-parts-shelf-marker");
    const SceneInteractableDefinition* workBoard = FindSceneInteractable(result.scene, "ferry-office-work-board");
    const SceneRouteMarkerDefinition* workBoardRoute = FindRouteMarker(result.scene, "route-parts-shelf-to-work-board");
    const SceneObjectiveMarkerDefinition* workBoardMarker = FindObjectiveMarker(result.scene, "ferry-office-work-board-marker");
    const SceneInteractableDefinition* handoffNote = FindSceneInteractable(result.scene, "ferry-office-handoff-note");
    const SceneRouteMarkerDefinition* handoffRoute = FindRouteMarker(result.scene, "route-work-board-to-handoff-note");
    const SceneObjectiveMarkerDefinition* handoffMarker = FindObjectiveMarker(result.scene, "ferry-office-handoff-marker");
    const SceneInteractableDefinition* stormPump = FindSceneInteractable(result.scene, "storm-pump-switch");
    const SceneInteractableDefinition* stormTicket = FindSceneInteractable(result.scene, "storm-pump-ticket");
    const SceneRouteMarkerDefinition* stormPumpRoute = FindRouteMarker(result.scene, "route-handoff-note-to-storm-pump");
    const SceneRouteMarkerDefinition* stormTicketRoute = FindRouteMarker(result.scene, "route-storm-pump-to-ticket");
    const SceneObjectiveMarkerDefinition* stormPumpMarker = FindObjectiveMarker(result.scene, "storm-pump-marker");
    const SceneObjectiveMarkerDefinition* stormTicketMarker = FindObjectiveMarker(result.scene, "storm-pump-ticket-marker");
    const SceneInteractableDefinition* lowDockDrain = FindSceneInteractable(result.scene, "low-dock-drain-marker");
    const SceneRouteMarkerDefinition* lowDockDrainRoute = FindRouteMarker(result.scene, "route-storm-ticket-to-low-dock-drain");
    const SceneObjectiveMarkerDefinition* lowDockDrainMarker = FindObjectiveMarker(result.scene, "low-dock-drain-objective-marker");
    const SceneInteractableDefinition* drainLog = FindSceneInteractable(result.scene, "ferry-office-drain-log");
    const SceneRouteMarkerDefinition* drainLogRoute = FindRouteMarker(result.scene, "route-low-dock-drain-to-office-log");
    const SceneObjectiveMarkerDefinition* drainLogMarker = FindObjectiveMarker(result.scene, "ferry-office-drain-log-marker");
    const SceneVisualPlaceholderDefinition* drainLogStatus = FindVisualPlaceholder(result.scene, "ferry-office-drain-log-status-light");

    Expect(result.ok(),
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Default scene should load before querying the harbor parts job beat.");
    Expect(partsCrate != nullptr && partsCrate->name == FerryOffice::Names::HarborPartsCrate,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should include the harbor parts pickup after road clearance.");
    Expect(partsShelf != nullptr && partsShelf->name == FerryOffice::Names::FerryOfficePartsShelf,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should include the Ferry Office delivery shelf.");
    Expect(crateRoute != nullptr && crateRoute->points.size() >= 2,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should route from the clearance tag to the parts crate.");
    Expect(shelfRoute != nullptr && shelfRoute->points.size() >= 2,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should route from the parts crate back to the office shelf.");
    Expect(crateMarker != nullptr && shelfMarker != nullptr,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should expose objective markers for the second job beat.");
    Expect(workBoard != nullptr && workBoard->name == FerryOffice::Names::FerryOfficeWorkBoard,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should include the Ferry Office work-board signoff after parts delivery.");
    Expect(workBoardRoute != nullptr && workBoardRoute->points.size() >= 2,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should route from the parts shelf to the work board.");
    Expect(workBoardMarker != nullptr,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should expose an objective marker for the work-board signoff.");
    Expect(handoffNote != nullptr && handoffNote->name == FerryOffice::Names::FerryOfficeHandoffNote,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should include the Ferry Office handoff note after the work-board signoff.");
    Expect(handoffRoute != nullptr && handoffRoute->points.size() >= 2,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should route from the work board to the handoff note.");
    Expect(handoffMarker != nullptr,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should expose an objective marker for the handoff note.");
    Expect(stormPump != nullptr && stormPump->name == FerryOffice::Names::StormPumpSwitch,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should include the storm pump reset as the next compact job seed.");
    Expect(stormTicket != nullptr && stormTicket->name == FerryOffice::Names::StormPumpTicket,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should include a Ferry Office ticket closeout for the storm pump reset.");
    Expect(stormPumpRoute != nullptr && stormPumpRoute->points.size() >= 2,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should route from the handoff note to the storm pump.");
    Expect(stormTicketRoute != nullptr && stormTicketRoute->points.size() >= 2,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should route from the storm pump back to the office ticket.");
    Expect(stormPumpMarker != nullptr && stormTicketMarker != nullptr,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should expose objective markers for the storm pump job seed.");
    Expect(lowDockDrain != nullptr && lowDockDrain->name == FerryOffice::Names::LowDockDrainMarker,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should include the low dock drain clear-tag follow-up.");
    Expect(lowDockDrainRoute != nullptr && lowDockDrainRoute->points.size() >= 2,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should route from the storm pump ticket back to the low dock drain.");
    Expect(lowDockDrainMarker != nullptr,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should expose an objective marker for the low dock drain clear tag.");
    Expect(drainLog != nullptr && drainLog->name == "Ferry Office Drain Log",
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should include the Ferry Office drain log closeout after the low dock drain clear tag.");
    Expect(drainLogRoute != nullptr && drainLogRoute->points.size() >= 2,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should route from the low dock drain back to the Ferry Office drain log.");
    Expect(drainLogMarker != nullptr,
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should expose an objective marker for the drain log closeout.");
    Expect(drainLogStatus != nullptr && drainLogStatus->colorKey == "ferry-office-drain-log-state",
        "TestSceneLoaderLoadsHarborPartsJobBeat",
        "Scene data should include a visible office-side drain log state cue.");
}

void TestSceneLoaderLoadsInteractableActionPrerequisites()
{
    const SceneLoadResult result = LoadSceneDefinition(DefaultScenePathForTests());
    const SceneInteractableDefinition* relay = FindSceneInteractable(result.scene, "dock-road-relay");
    const SceneInteractableDefinition* clearanceTag = FindSceneInteractable(result.scene, "dock-road-clearance-tag");
    const SceneInteractableDefinition* partsCrate = FindSceneInteractable(result.scene, "harbor-parts-crate");
    const SceneInteractableDefinition* partsShelf = FindSceneInteractable(result.scene, "ferry-office-parts-shelf");

    Expect(result.ok(),
        "TestSceneLoaderLoadsInteractableActionPrerequisites",
        "Default scene should load before checking authored action prerequisites.");
    Expect(relay != nullptr && ContainsString(relay->requiredWorldFlags, "ferryOfficeJobComplete"),
        "TestSceneLoaderLoadsInteractableActionPrerequisites",
        "Dock Road Relay should declare that the first service call must be complete.");
    Expect(clearanceTag != nullptr && ContainsString(clearanceTag->requiredWorldFlags, "dockRoadRelayLogged"),
        "TestSceneLoaderLoadsInteractableActionPrerequisites",
        "Dock Road Clearance Tag should declare the relay log prerequisite.");
    Expect(partsCrate != nullptr && ContainsString(partsCrate->requiredWorldFlags, "dockRoadClearanceTagged"),
        "TestSceneLoaderLoadsInteractableActionPrerequisites",
        "Harbor Parts Crate should declare the dock-road clearance prerequisite.");
    Expect(partsShelf != nullptr && ContainsString(partsShelf->requiredWorldFlags, "harborPartsPickedUp"),
        "TestSceneLoaderLoadsInteractableActionPrerequisites",
        "Ferry Office Parts Shelf should declare the parts pickup prerequisite.");
    const SceneInteractableDefinition* workBoard = FindSceneInteractable(result.scene, "ferry-office-work-board");
    Expect(workBoard != nullptr && ContainsString(workBoard->requiredWorldFlags, "harborPartsDelivered"),
        "TestSceneLoaderLoadsInteractableActionPrerequisites",
        "Ferry Office Work Board should declare the parts delivery prerequisite.");
    const SceneInteractableDefinition* handoffNote = FindSceneInteractable(result.scene, "ferry-office-handoff-note");
    Expect(handoffNote != nullptr && ContainsString(handoffNote->requiredWorldFlags, "ferryOfficeBoardUpdated"),
        "TestSceneLoaderLoadsInteractableActionPrerequisites",
        "Ferry Office Handoff Note should declare the work-board prerequisite.");
    const SceneInteractableDefinition* stormPump = FindSceneInteractable(result.scene, "storm-pump-switch");
    Expect(stormPump != nullptr && ContainsString(stormPump->requiredWorldFlags, "ferryOfficeHandoffFiled"),
        "TestSceneLoaderLoadsInteractableActionPrerequisites",
        "Storm Pump Switch should declare the filed handoff prerequisite.");
    const SceneInteractableDefinition* stormTicket = FindSceneInteractable(result.scene, "storm-pump-ticket");
    Expect(stormTicket != nullptr && ContainsString(stormTicket->requiredWorldFlags, "stormPumpReset"),
        "TestSceneLoaderLoadsInteractableActionPrerequisites",
        "Storm Pump Ticket should declare the storm pump reset prerequisite.");
    const SceneInteractableDefinition* lowDockDrain = FindSceneInteractable(result.scene, "low-dock-drain-marker");
    Expect(lowDockDrain != nullptr && ContainsString(lowDockDrain->requiredWorldFlags, "stormPumpTicketClosed"),
        "TestSceneLoaderLoadsInteractableActionPrerequisites",
        "Low Dock Drain Marker should declare the closed pump ticket prerequisite.");
    const SceneInteractableDefinition* drainLog = FindSceneInteractable(result.scene, "ferry-office-drain-log");
    Expect(drainLog != nullptr && ContainsString(drainLog->requiredWorldFlags, "lowDockDrainCleared")
            && ContainsString(drainLog->worldFlagsSetWhenReady, "lowDockDrainLogged"),
        "TestSceneLoaderLoadsInteractableActionPrerequisites",
        "Ferry Office Drain Log should declare the low dock drain clear prerequisite and log flag.");
}

void TestSceneLoaderReportsMissingSceneFile()
{
    const SceneLoadResult result = LoadSceneDefinition(std::filesystem::path(ENGINE_SOURCE_ROOT) / "data" / "scenes" / "missing.scene.json");

    Expect(!result.ok(),
        "TestSceneLoaderReportsMissingSceneFile",
        "Missing scene file should fail cleanly.");
    Expect(result.error.find("not found") != std::string::npos || result.error.find("Could not read") != std::string::npos,
        "TestSceneLoaderReportsMissingSceneFile",
        "Missing scene failure should explain that the file could not be read.");
}

void TestSceneLoaderLoadsVehicleBoundsAndRoadMarkers()
{
    const SceneLoadResult result = LoadSceneDefinition(DefaultScenePathForTests());
    const SceneVehicleDefinition* vehicle = FindVehicle(result.scene, "service-yard-vehicle");
    const SceneRouteMarkerDefinition* route = FindRouteMarker(result.scene, "route-service-yard-to-dock-road");
    const SceneVisualPlaceholderDefinition* road = FindVisualPlaceholder(result.scene, "dock-road-segment");

    Expect(result.ok(),
        "TestSceneLoaderLoadsVehicleBoundsAndRoadMarkers",
        "Default scene should load before querying vehicle and road data.");
    Expect(vehicle != nullptr,
        "TestSceneLoaderLoadsVehicleBoundsAndRoadMarkers",
        "Loaded scene should include the service-yard vehicle.");
    if (vehicle != nullptr) {
        ExpectNear(vehicle->spawnPosition.x, 6.2f, 0.001f,
            "TestSceneLoaderLoadsVehicleBoundsAndRoadMarkers",
            "Vehicle spawn X should come from scene data.");
        ExpectNear(vehicle->boundsMax.x, 19.45f, 0.001f,
            "TestSceneLoaderLoadsVehicleBoundsAndRoadMarkers",
            "Vehicle road-test max X bound should come from scene data.");
        ExpectNear(vehicle->proxyHalfExtents.y, 0.53f, 0.001f,
            "TestSceneLoaderLoadsVehicleBoundsAndRoadMarkers",
            "Vehicle proxy height should come from scene data.");
    }
    Expect(route != nullptr && route->points.size() == 4,
        "TestSceneLoaderLoadsVehicleBoundsAndRoadMarkers",
        "Loaded scene should include the dock-road route marker polyline.");
    Expect(road != nullptr,
        "TestSceneLoaderLoadsVehicleBoundsAndRoadMarkers",
        "Loaded scene should include the dock-road visual placeholder.");
}

void TestPrototypeWorldBuildsFerryOfficeCollidersFromSceneData()
{
    const SceneLoadResult result = LoadSceneDefinition(DefaultScenePathForTests());
    PrototypeWorld world;
    world.buildFromSceneDefinition(result.scene);

    Expect(result.ok(),
        "TestPrototypeWorldBuildsFerryOfficeCollidersFromSceneData",
        "Default scene should load before building the world.");
    Expect(world.colliders().size() == result.scene.colliders.size(),
        "TestPrototypeWorldBuildsFerryOfficeCollidersFromSceneData",
        "PrototypeWorld should build static colliders from scene data.");
    const StaticCollider* gate = world.colliderByName(FerryOffice::Names::ServiceGateCollider);
    Expect(gate != nullptr && gate->blocksPlayer,
        "TestPrototypeWorldBuildsFerryOfficeCollidersFromSceneData",
        "Service gate collider should be loaded from scene data and start blocking.");
    const StaticCollider* lowDockAccess = world.colliderByName("low-dock-drain-flood-barrier");
    Expect(lowDockAccess != nullptr && lowDockAccess->blocksPlayer,
        "TestPrototypeWorldBuildsFerryOfficeCollidersFromSceneData",
        "Low dock drain access barrier should start as a blocking authored collider.");
    if (lowDockAccess != nullptr) {
        Expect(lowDockAccess->stateFlag == "stormPumpTicketClosed" && lowDockAccess->blocksWhenFlagFalse,
            "TestPrototypeWorldBuildsFerryOfficeCollidersFromSceneData",
            "Low dock drain access barrier should be tied to the storm pump ticket closeout state.");
    }
}

void TestPrototypeSceneLoadsInteractionsAndTraversalFromSceneData()
{
    const SceneLoadResult result = LoadSceneDefinition(DefaultScenePathForTests());
    PrototypeScene scene;
    scene.loadFromDefinition(result.scene);

    Expect(result.ok(),
        "TestPrototypeSceneLoadsInteractionsAndTraversalFromSceneData",
        "Default scene should load before building the prototype scene.");
    Expect(scene.interactions().interactableCount() == result.scene.interactables.size(),
        "TestPrototypeSceneLoadsInteractionsAndTraversalFromSceneData",
        "PrototypeScene should build interactables from scene data.");
    Expect(scene.traversal().affordanceCount() == result.scene.traversalAffordances.size(),
        "TestPrototypeSceneLoadsInteractionsAndTraversalFromSceneData",
        "PrototypeScene should build traversal affordances from scene data.");
    const InteractionFocus manifestFocus = scene.interactions().updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
    Expect(manifestFocus.hasFocus && manifestFocus.name == FerryOffice::Names::FerryManifest,
        "TestPrototypeSceneLoadsInteractionsAndTraversalFromSceneData",
        "Loaded scene interactables should preserve the Ferry Manifest focus behavior.");
}

void TestSceneLoaderRejectsUnknownWorldFlags()
{
    std::ifstream input(DefaultScenePathForTests());
    nlohmann::json scene = nlohmann::json::parse(input);
    scene["interactables"][0]["worldFlagsSet"] = nlohmann::json::array({"manifestCollected", "typoFlag"});

    const std::filesystem::path tempPath = std::filesystem::temp_directory_path() / "tidebreak-unknown-world-flag.scene.json";
    {
        std::ofstream output(tempPath);
        output << scene.dump(2);
    }

    const SceneLoadResult result = LoadSceneDefinition(tempPath);
    Expect(!result.ok(),
        "TestSceneLoaderRejectsUnknownWorldFlags",
        "Runtime scene loading should reject unknown authored world flags instead of silently dropping them.");
    Expect(result.error.find("typoFlag") != std::string::npos,
        "TestSceneLoaderRejectsUnknownWorldFlags",
        "Scene loader errors should name the unknown world flag.");
}

void TestBuiltInFerryOfficeFallbackContainsFollowupInteractables()
{
    PrototypeScene scene;

    const auto hasInteractable = [&](std::string_view name) {
        for (const Interactable& interactable : scene.interactions().interactables()) {
            if (interactable.name == name) {
                return true;
            }
        }
        return false;
    };

    Expect(hasInteractable(FerryOffice::Names::DockRoadRelay),
        "TestBuiltInFerryOfficeFallbackContainsFollowupInteractables",
        "Built-in fallback should include the Dock Road Relay follow-up interactable.");
    Expect(hasInteractable(FerryOffice::Names::RelayServiceLog),
        "TestBuiltInFerryOfficeFallbackContainsFollowupInteractables",
        "Built-in fallback should include the Relay Service Log interactable, not just its action binding.");
    Expect(hasInteractable(FerryOffice::Names::DockRoadClearanceTag),
        "TestBuiltInFerryOfficeFallbackContainsFollowupInteractables",
        "Built-in fallback should include the Dock Road Clearance Tag follow-up interactable.");
}

void TestPhysicsWorldCreatesAndShutsDownThroughVendorFreeInterface()
{
    auto world = engine::physics::CreatePhysicsWorld(engine::physics::PhysicsBackend::Simple);

    Expect(world != nullptr,
        "TestPhysicsWorldCreatesAndShutsDownThroughVendorFreeInterface",
        "Simple physics world factory should return a world instance.");

    engine::physics::PhysicsConfig config;
    config.backend = engine::physics::PhysicsBackend::Simple;
    config.fixedStepSeconds = 1.0f / 60.0f;

    Expect(world->initialize(config),
        "TestPhysicsWorldCreatesAndShutsDownThroughVendorFreeInterface",
        "Simple physics world should initialize.");
    Expect(world->isInitialized(),
        "TestPhysicsWorldCreatesAndShutsDownThroughVendorFreeInterface",
        "Physics world should report initialized state after initialize.");
    Expect(world->backendName() == std::string("simple"),
        "TestPhysicsWorldCreatesAndShutsDownThroughVendorFreeInterface",
        "Default test backend should remain vendor-free.");

    world->shutdown();
    Expect(!world->isInitialized(),
        "TestPhysicsWorldCreatesAndShutsDownThroughVendorFreeInterface",
        "Physics world should report shutdown state after shutdown.");
}

void TestPhysicsWorldRaycastHitsStaticBox()
{
    auto world = engine::physics::CreatePhysicsWorld(engine::physics::PhysicsBackend::Simple);

    engine::physics::PhysicsConfig config;
    config.backend = engine::physics::PhysicsBackend::Simple;
    Expect(world != nullptr && world->initialize(config),
        "TestPhysicsWorldRaycastHitsStaticBox",
        "Simple physics world should initialize for raycast tests.");

    engine::physics::BoxColliderDesc box;
    box.name = "physics-test-box";
    box.center = {0.0f, 0.5f, 3.0f};
    box.halfExtents = {0.5f, 0.5f, 0.5f};
    const engine::physics::BodyHandle handle = world->addStaticBox(box);

    Expect(handle.isValid(),
        "TestPhysicsWorldRaycastHitsStaticBox",
        "Adding a static box should return a valid opaque body handle.");

    const engine::physics::RaycastResult hit = world->raycast({0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, 10.0f);
    Expect(hit.hit,
        "TestPhysicsWorldRaycastHitsStaticBox",
        "Physics raycast should hit the static box.");
    Expect(hit.bodyName == "physics-test-box",
        "TestPhysicsWorldRaycastHitsStaticBox",
        "Physics raycast should preserve vendor-free body metadata.");
    ExpectNear(hit.distance, 2.5f, 0.01f,
        "TestPhysicsWorldRaycastHitsStaticBox",
        "Physics raycast distance should land on the box front face.");

    const engine::physics::RaycastResult miss = world->raycast({0.0f, 2.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, 1.0f);
    Expect(!miss.hit,
        "TestPhysicsWorldRaycastHitsStaticBox",
        "Physics raycast should report a clean miss.");

    world->shutdown();
}

void TestPhysicsWorldDebugLinesExposeStaticBoxes()
{
    auto world = engine::physics::CreatePhysicsWorld(engine::physics::PhysicsBackend::Simple);

    engine::physics::PhysicsConfig config;
    config.backend = engine::physics::PhysicsBackend::Simple;
    Expect(world != nullptr && world->initialize(config),
        "TestPhysicsWorldDebugLinesExposeStaticBoxes",
        "Simple physics world should initialize for debug line tests.");

    engine::physics::BoxColliderDesc box;
    box.name = "debug-box";
    box.center = {1.0f, 0.5f, 1.0f};
    box.halfExtents = {0.25f, 0.5f, 0.75f};
    world->addStaticBox(box);

    const std::vector<engine::physics::PhysicsDebugLine> lines = world->debugLines();
    Expect(lines.size() >= 12,
        "TestPhysicsWorldDebugLinesExposeStaticBoxes",
        "A static box should expose at least twelve debug line segments.");

    world->shutdown();
}

void TestPhysicsWorldOverlapBoxFindsStaticBox()
{
    auto world = engine::physics::CreatePhysicsWorld(engine::physics::PhysicsBackend::Simple);
    engine::physics::PhysicsConfig config;
    Expect(world->initialize(config), "TestPhysicsWorldOverlapBoxFindsStaticBox", "Simple physics world should initialize.");

    engine::physics::BoxColliderDesc box;
    box.name = "overlap-target";
    box.center = {1.0f, 0.5f, 1.0f};
    box.halfExtents = {0.5f, 0.5f, 0.5f};
    world->addStaticBox(box);

    const auto overlaps = world->overlapBox({1.25f, 0.5f, 1.25f}, {0.25f, 0.25f, 0.25f});

    Expect(overlaps.size() == 1,
        "TestPhysicsWorldOverlapBoxFindsStaticBox",
        "Physics overlap box should report an intersecting static box.");
    if (!overlaps.empty()) {
        Expect(overlaps.front().bodyName == "overlap-target",
            "TestPhysicsWorldOverlapBoxFindsStaticBox",
            "Physics overlap result should preserve the body name.");
    }
    world->shutdown();
}

void TestJoltBackendAvailabilityIsExplicit()
{
    const bool available = engine::physics::IsJoltPhysicsAvailable();

#if ENGINE_WITH_JOLT
    Expect(available,
        "TestJoltBackendAvailabilityIsExplicit",
        "ENGINE_WITH_JOLT builds should report Jolt availability.");

    auto world = engine::physics::CreatePhysicsWorld(engine::physics::PhysicsBackend::Jolt);
    engine::physics::PhysicsConfig config;
    config.backend = engine::physics::PhysicsBackend::Jolt;
    Expect(world != nullptr && world->initialize(config),
        "TestJoltBackendAvailabilityIsExplicit",
        "Jolt physics world should initialize when the backend is compiled in.");

    engine::physics::BoxColliderDesc box;
    box.name = "jolt-test-box";
    box.center = {0.0f, 0.5f, 2.0f};
    box.halfExtents = {0.5f, 0.5f, 0.5f};
    world->addStaticBox(box);

    const engine::physics::RaycastResult hit = world->raycast({0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, 10.0f);
    Expect(hit.hit && hit.bodyName == "jolt-test-box",
        "TestJoltBackendAvailabilityIsExplicit",
        "Jolt backend should support a static box raycast through the engine interface.");
    world->shutdown();
#else
    Expect(!available,
        "TestJoltBackendAvailabilityIsExplicit",
        "Default builds should report Jolt as unavailable unless explicitly enabled.");
    auto world = engine::physics::CreatePhysicsWorld(engine::physics::PhysicsBackend::Jolt);
    Expect(world == nullptr,
        "TestJoltBackendAvailabilityIsExplicit",
        "Default builds should not create a Jolt world without the opt-in backend.");
#endif
}

void TestVehicleControllerAcceleratesBrakesAndReversesDeterministically()
{
    VehicleController vehicle;
    VehicleControllerSettings settings;
    settings.maxForwardSpeed = 8.0f;
    settings.maxReverseSpeed = 3.0f;
    settings.acceleration = 8.0f;
    settings.braking = 12.0f;
    settings.drag = 0.5f;
    vehicle.setSettings(settings);
    vehicle.setPosition({0.0f, 0.0f, 0.0f});

    vehicle.beginFrame();
    vehicle.updateFocus({0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f});
    engine::InputState input;
    input.interactPressed = true;
    Expect(vehicle.tryEnter(input),
        "TestVehicleControllerAcceleratesBrakesAndReversesDeterministically",
        "Pressed interact near the vehicle should enter the vehicle.");

    input = {};
    input.moveForward = 1.0f;
    for (int i = 0; i < 10; ++i) {
        vehicle.beginFrame();
        vehicle.updateDriving(0.05f, input);
    }

    const float forwardSpeed = vehicle.state().speed;
    Expect(forwardSpeed > 0.0f,
        "TestVehicleControllerAcceleratesBrakesAndReversesDeterministically",
        "Throttle should increase forward vehicle speed.");
    Expect(forwardSpeed <= settings.maxForwardSpeed + 0.01f,
        "TestVehicleControllerAcceleratesBrakesAndReversesDeterministically",
        "Forward vehicle speed should remain clamped.");

    input.moveForward = -1.0f;
    for (int i = 0; i < 8; ++i) {
        vehicle.beginFrame();
        vehicle.updateDriving(0.05f, input);
    }

    Expect(vehicle.state().speed < forwardSpeed,
        "TestVehicleControllerAcceleratesBrakesAndReversesDeterministically",
        "Brake input should reduce forward speed.");

    for (int i = 0; i < 30; ++i) {
        vehicle.beginFrame();
        vehicle.updateDriving(0.05f, input);
    }

    Expect(vehicle.state().speed < 0.0f,
        "TestVehicleControllerAcceleratesBrakesAndReversesDeterministically",
        "Holding reverse from a stopped/braked state should move the vehicle backward.");
    Expect(vehicle.state().speed >= -settings.maxReverseSpeed - 0.01f,
        "TestVehicleControllerAcceleratesBrakesAndReversesDeterministically",
        "Reverse vehicle speed should remain clamped.");
}

void TestVehicleControllerSteeringChangesYawWhileMoving()
{
    VehicleController vehicle;
    vehicle.setPosition({0.0f, 0.0f, 0.0f});
    vehicle.setOccupiedForTesting(true);

    engine::InputState input;
    input.moveForward = 1.0f;
    input.moveRight = 1.0f;

    for (int i = 0; i < 12; ++i) {
        vehicle.beginFrame();
        vehicle.updateDriving(0.05f, input);
    }

    Expect(vehicle.state().yawRadians > 0.0f,
        "TestVehicleControllerSteeringChangesYawWhileMoving",
        "Steering right while moving should increase vehicle yaw.");
    Expect(engine::Length(vehicle.state().velocity) > 0.0f,
        "TestVehicleControllerSteeringChangesYawWhileMoving",
        "Vehicle velocity should reflect the driven movement.");
}

float RunDeterministicVehicleYawDelta(float throttle, float steer)
{
    VehicleController vehicle;
    vehicle.setPosition({0.0f, 0.0f, 0.0f});
    vehicle.setOccupiedForTesting(true);

    const float initialYaw = vehicle.state().yawRadians;
    engine::InputState input;
    input.moveForward = throttle;
    input.moveRight = steer;
    for (int i = 0; i < 90; ++i) {
        vehicle.beginFrame();
        vehicle.updateDriving(1.0f / 60.0f, input);
    }

    return engine::Degrees(vehicle.state().yawRadians - initialYaw);
}

void TestVehicleControllerSteeringSignSemantics()
{
    const float forwardLeft = RunDeterministicVehicleYawDelta(0.75f, -0.6f);
    const float forwardRight = RunDeterministicVehicleYawDelta(0.75f, 0.6f);
    const float reverseLeft = RunDeterministicVehicleYawDelta(-0.75f, -0.6f);
    const float reverseRight = RunDeterministicVehicleYawDelta(-0.75f, 0.6f);

    Expect(forwardLeft < -1.0f,
        "TestVehicleControllerSteeringSignSemantics",
        "Forward + left steering should decrease deterministic vehicle yaw.");
    Expect(forwardRight > 1.0f,
        "TestVehicleControllerSteeringSignSemantics",
        "Forward + right steering should increase deterministic vehicle yaw.");
    Expect(reverseLeft > 1.0f,
        "TestVehicleControllerSteeringSignSemantics",
        "Reverse + left steering should increase deterministic vehicle yaw because reversing flips steering direction.");
    Expect(reverseRight < -1.0f,
        "TestVehicleControllerSteeringSignSemantics",
        "Reverse + right steering should decrease deterministic vehicle yaw because reversing flips steering direction.");
}

void TestVehicleLowSpeedSteeringAssistKeepsTurnaroundReadable()
{
    VehicleController vehicle;
    VehicleControllerSettings settings;
    settings.maxForwardSpeed = 8.0f;
    settings.acceleration = 3.0f;
    settings.steeringRate = 2.0f;
    settings.minSteeringSpeedFactor = 0.38f;
    vehicle.setSettings(settings);
    vehicle.setPosition({0.0f, 0.0f, 0.0f});
    vehicle.setOccupiedForTesting(true);

    engine::InputState input;
    input.moveForward = 1.0f;
    input.moveRight = 1.0f;

    vehicle.beginFrame();
    vehicle.updateDriving(0.05f, input);

    Expect(vehicle.state().speed < 0.25f,
        "TestVehicleLowSpeedSteeringAssistKeepsTurnaroundReadable",
        "Test setup should keep the vehicle in a low-speed turn state.");
    Expect(vehicle.state().yawRadians > 0.03f,
        "TestVehicleLowSpeedSteeringAssistKeepsTurnaroundReadable",
        "Low-speed steering assist should make compact turn-around steering readable.");
}

void TestVehicleEnterExitUsesPressedEdgeAndSafeExit()
{
    VehicleController vehicle;
    vehicle.setPosition({0.0f, 0.0f, 0.0f});

    engine::InputState input;
    input.interactPressed = true;
    input.interactHeld = true;

    vehicle.beginFrame();
    vehicle.updateFocus({0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f});
    Expect(vehicle.tryEnter(input),
        "TestVehicleEnterExitUsesPressedEdgeAndSafeExit",
        "Interact pressed should enter when the vehicle is focused.");
    Expect(vehicle.state().occupied && vehicle.state().enteredThisFrame,
        "TestVehicleEnterExitUsesPressedEdgeAndSafeExit",
        "Vehicle should report occupied and entered for the pressed frame.");

    input.interactPressed = false;
    vehicle.beginFrame();
    Expect(!vehicle.tryExit(input, true),
        "TestVehicleEnterExitUsesPressedEdgeAndSafeExit",
        "Held interact without pressed edge should not exit.");
    Expect(vehicle.state().occupied,
        "TestVehicleEnterExitUsesPressedEdgeAndSafeExit",
        "Vehicle should stay occupied while interact is only held.");

    input.interactPressed = true;
    vehicle.beginFrame();
    Expect(!vehicle.tryExit(input, false),
        "TestVehicleEnterExitUsesPressedEdgeAndSafeExit",
        "Blocked exit point should keep the player in the vehicle.");
    Expect(vehicle.state().occupied && vehicle.state().exitBlockedThisFrame,
        "TestVehicleEnterExitUsesPressedEdgeAndSafeExit",
        "Vehicle should expose blocked exit state for debug text.");

    vehicle.beginFrame();
    Expect(vehicle.tryExit(input, true),
        "TestVehicleEnterExitUsesPressedEdgeAndSafeExit",
        "Pressed interact with a safe exit point should exit the vehicle.");
    Expect(!vehicle.state().occupied && vehicle.state().exitedThisFrame,
        "TestVehicleEnterExitUsesPressedEdgeAndSafeExit",
        "Vehicle should become unoccupied after a safe exit.");
}

void TestVehicleCameraTargetFollowsVehicle()
{
    VehicleController vehicle;
    VehicleControllerSettings settings;
    settings.cameraLookAhead = 0.85f;
    vehicle.setSettings(settings);
    vehicle.setPosition({2.0f, 0.0f, 3.0f});
    vehicle.setYawRadians(engine::Radians(45.0f));

    const CameraTarget target = vehicle.cameraTarget();
    const engine::Vec3 expectedPosition = vehicle.state().position + vehicle.forward() * settings.cameraLookAhead;

    ExpectNear(target.position.x, expectedPosition.x, 0.001f,
        "TestVehicleCameraTargetFollowsVehicle",
        "Vehicle camera target should look ahead of the vehicle X position.");
    ExpectNear(target.position.z, expectedPosition.z, 0.001f,
        "TestVehicleCameraTargetFollowsVehicle",
        "Vehicle camera target should look ahead of the vehicle Z position.");
    ExpectNear(target.yawRadians, engine::Radians(45.0f), 0.001f,
        "TestVehicleCameraTargetFollowsVehicle",
        "Vehicle camera target should use vehicle yaw.");
}

void TestThirdPersonCameraCanFollowTargetYawForVehicles()
{
    ThirdPersonCamera camera;
    ThirdPersonCameraSettings settings;
    settings.distance = 6.0f;
    settings.heightOffset = 1.6f;
    settings.targetYawFollowStrength = 8.0f;
    camera.setSettings(settings);

    CameraTarget target;
    target.position = {0.0f, 0.0f, 0.0f};
    target.yawRadians = engine::Radians(90.0f);

    engine::InputState input;
    camera.update(0.1f, input, target);

    Expect(camera.state().yawRadians > engine::Radians(40.0f),
        "TestThirdPersonCameraCanFollowTargetYawForVehicles",
        "Vehicle camera follow should rotate toward the target yaw without player input.");
    Expect(camera.state().yawRadians < engine::Radians(90.0f),
        "TestThirdPersonCameraCanFollowTargetYawForVehicles",
        "Vehicle camera follow should smooth toward target yaw instead of snapping.");
}

void TestVehicleDefaultTuningFitsSmallServiceYard()
{
    const VehicleControllerSettings settings;

    Expect(settings.maxForwardSpeed <= 8.0f,
        "TestVehicleDefaultTuningFitsSmallServiceYard",
        "Default forward speed should fit the compact service-yard road test.");
    Expect(settings.maxReverseSpeed <= 3.0f,
        "TestVehicleDefaultTuningFitsSmallServiceYard",
        "Default reverse speed should stay readable in the compact service yard.");
    Expect(settings.acceleration <= 7.0f,
        "TestVehicleDefaultTuningFitsSmallServiceYard",
        "Default acceleration should avoid launching across the service yard.");
    Expect(settings.braking <= 10.5f,
        "TestVehicleDefaultTuningFitsSmallServiceYard",
        "Default braking should be firm but not abrupt for the prototype yard.");
    Expect(settings.drag >= 2.2f,
        "TestVehicleDefaultTuningFitsSmallServiceYard",
        "Default drag should settle the placeholder vehicle quickly when input stops.");
    Expect(settings.steeringRate >= 1.9f,
        "TestVehicleDefaultTuningFitsSmallServiceYard",
        "Default steering should support a tight low-speed turn-around test.");
}

void TestVehicleFocusUsesFacingAndCloseFallback()
{
    VehicleController vehicle;
    VehicleControllerSettings settings;
    settings.enterRadius = 1.5f;
    settings.closeEnterRadius = 0.4f;
    settings.requiredFacingDot = 0.5f;
    vehicle.setSettings(settings);
    vehicle.setPosition({0.0f, 0.0f, 0.0f});

    VehicleFocus focus = vehicle.updateFocus({0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f});
    Expect(focus.canEnter,
        "TestVehicleFocusUsesFacingAndCloseFallback",
        "Facing the nearby vehicle should allow entry focus.");

    focus = vehicle.updateFocus({0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f});
    Expect(!focus.canEnter,
        "TestVehicleFocusUsesFacingAndCloseFallback",
        "A player outside close fallback and facing away should not focus the vehicle.");

    focus = vehicle.updateFocus({0.0f, 0.0f, -0.25f}, {0.0f, 0.0f, -1.0f});
    Expect(focus.canEnter,
        "TestVehicleFocusUsesFacingAndCloseFallback",
        "Close fallback should still allow entry when the player is very near the vehicle.");

    vehicle.setOccupiedForTesting(true);
    focus = vehicle.updateFocus({0.0f, 0.0f, -0.25f}, {0.0f, 0.0f, 1.0f});
    Expect(!focus.canEnter,
        "TestVehicleFocusUsesFacingAndCloseFallback",
        "An occupied vehicle should not expose an enter focus prompt.");
}

void TestVehicleBoundsClampStopsVehicle()
{
    VehicleController vehicle;
    VehicleControllerSettings settings;
    settings.boundsMinX = -1.0f;
    settings.boundsMaxX = 1.0f;
    settings.boundsMinZ = -0.2f;
    settings.boundsMaxZ = 0.2f;
    settings.maxForwardSpeed = 10.0f;
    settings.acceleration = 10.0f;
    vehicle.setSettings(settings);
    vehicle.setPosition({0.0f, 0.0f, 0.18f});
    vehicle.setOccupiedForTesting(true);

    engine::InputState input;
    input.moveForward = 1.0f;
    vehicle.beginFrame();
    vehicle.updateDriving(0.2f, input);

    ExpectNear(vehicle.state().position.z, 0.2f, 0.001f,
        "TestVehicleBoundsClampStopsVehicle",
        "Vehicle should clamp to the authored service-yard Z bound.");
    ExpectNear(vehicle.state().speed, 0.0f, 0.001f,
        "TestVehicleBoundsClampStopsVehicle",
        "Vehicle speed should reset after hitting service-yard bounds.");
    Expect(vehicle.state().hitBoundsThisFrame,
        "TestVehicleBoundsClampStopsVehicle",
        "Vehicle should expose bounds-hit telemetry for debug text.");
}

void TestVehicleDragUsesDeltaClamp()
{
    VehicleController vehicle;
    VehicleControllerSettings settings;
    settings.acceleration = 10.0f;
    settings.maxForwardSpeed = 10.0f;
    settings.drag = 2.0f;
    vehicle.setSettings(settings);
    vehicle.setPosition({0.0f, 0.0f, 0.0f});
    vehicle.setOccupiedForTesting(true);

    engine::InputState input;
    input.moveForward = 1.0f;
    vehicle.beginFrame();
    vehicle.updateDriving(0.1f, input);
    const float speedBeforeDrag = vehicle.state().speed;

    input = {};
    vehicle.beginFrame();
    vehicle.updateDriving(1.0f, input);

    ExpectNear(speedBeforeDrag, 1.0f, 0.001f,
        "TestVehicleDragUsesDeltaClamp",
        "Initial acceleration setup should create a known speed before drag.");
    ExpectNear(vehicle.state().speed, 0.8f, 0.001f,
        "TestVehicleDragUsesDeltaClamp",
        "Drag should use the same clamped timestep as vehicle movement.");
}

void TestVehicleExitPositionUsesSideAndBackOffsets()
{
    VehicleController vehicle;
    VehicleControllerSettings settings;
    settings.exitSideOffset = 1.65f;
    settings.exitBackOffset = 0.35f;
    vehicle.setSettings(settings);
    vehicle.setPosition({0.0f, 0.0f, 0.0f});
    vehicle.setYawRadians(0.0f);

    const engine::Vec3 exit = vehicle.exitPosition();

    ExpectNear(exit.x, -1.65f, 0.001f,
        "TestVehicleExitPositionUsesSideAndBackOffsets",
        "Vehicle exit should be placed to the left side at zero yaw.");
    ExpectNear(exit.z, -0.35f, 0.001f,
        "TestVehicleExitPositionUsesSideAndBackOffsets",
        "Vehicle exit should be slightly behind the vehicle at zero yaw.");
}

void TestVehicleReverseSteeringIsPredictable()
{
    VehicleController vehicle;
    VehicleControllerSettings settings;
    settings.maxForwardSpeed = 8.0f;
    settings.maxReverseSpeed = 3.0f;
    settings.acceleration = 8.0f;
    settings.steeringRate = 2.0f;
    vehicle.setSettings(settings);
    vehicle.setPosition({0.0f, 0.0f, 0.0f});
    vehicle.setOccupiedForTesting(true);

    engine::InputState input;
    input.moveRight = 1.0f;
    vehicle.beginFrame();
    vehicle.updateDriving(0.1f, input);
    ExpectNear(vehicle.state().yawRadians, 0.0f, 0.001f,
        "TestVehicleReverseSteeringIsPredictable",
        "Steering input at a standstill should not rotate the vehicle.");

    input.moveForward = -1.0f;
    for (int i = 0; i < 8; ++i) {
        vehicle.beginFrame();
        vehicle.updateDriving(0.05f, input);
    }
    const float yawBeforeReverseSteer = vehicle.state().yawRadians;

    vehicle.beginFrame();
    vehicle.updateDriving(0.05f, input);

    Expect(vehicle.state().speed < 0.0f,
        "TestVehicleReverseSteeringIsPredictable",
        "Reverse input should put the vehicle into reverse speed.");
    Expect(vehicle.state().yawRadians < yawBeforeReverseSteer,
        "TestVehicleReverseSteeringIsPredictable",
        "Steering right while reversing should turn the placeholder vehicle predictably backward.");
}

void TestSandboxLayerVehicleDebugTextIncludesRoadTestTelemetry()
{
    SandboxLayer layer;
    layer.onAttach();

    const std::string debug = layer.debugText();
    layer.onDetach();

    Expect(debug.find("vehicle=(6.20,0.00,-2.20)") != std::string::npos,
        "TestSandboxLayerVehicleDebugTextIncludesRoadTestTelemetry",
        "Sandbox debug text should expose the service-yard vehicle position.");
    Expect(debug.find("cameraMode=on-foot") != std::string::npos,
        "TestSandboxLayerVehicleDebugTextIncludesRoadTestTelemetry",
        "Sandbox debug text should expose current camera mode.");
    Expect(debug.find("physics=simple") != std::string::npos,
        "TestSandboxLayerVehicleDebugTextIncludesRoadTestTelemetry",
        "Sandbox debug text should expose the vehicle physics debug backend.");
    Expect(debug.find("exitClear=") != std::string::npos,
        "TestSandboxLayerVehicleDebugTextIncludesRoadTestTelemetry",
        "Sandbox debug text should expose whether the vehicle side exit marker is currently safe.");
}

void TestSandboxLayerDebugTextMarksLiveVehicleRuntimeAdapterWhenOptedIn()
{
    SandboxLayer layer(
        DefaultScenePathForTests(),
        engine::UiMode::Debug,
        engine::physics::PhysicsBackend::Simple,
        true);
    layer.onAttach();

    const std::string debug = layer.debugText();
    layer.onDetach();

    Expect(debug.find("vehicleRuntime=simple-live") != std::string::npos,
        "TestSandboxLayerDebugTextMarksLiveVehicleRuntimeAdapterWhenOptedIn",
        "Sandbox debug text should expose when the live adapter vehicle path is active.");
}

void TestSandboxLayerDebugTextIncludesDockRoadTelemetry()
{
    SandboxLayer layer(DefaultScenePathForTests(), engine::UiMode::Debug);
    layer.onAttach();

    const std::string debug = layer.debugText();
    layer.onDetach();

    Expect(debug.find("scene=ferry-office loaded=yes") != std::string::npos,
        "TestSandboxLayerDebugTextIncludesDockRoadTelemetry",
        "Sandbox debug text should expose that the runtime scene data loaded.");
    Expect(debug.find("roadSegment=dock-road") != std::string::npos,
        "TestSandboxLayerDebugTextIncludesDockRoadTelemetry",
        "Sandbox debug text should expose the authored dock road segment.");
    Expect(debug.find("roadBounds=(3.35,-5.05)-(19.45,0.95)") != std::string::npos,
        "TestSandboxLayerDebugTextIncludesDockRoadTelemetry",
        "Sandbox debug text should expose the finite vehicle road-test bounds.");
}

void TestScenePresentationKnowsEveryAuthoredSceneColorKey()
{
    const SceneLoadResult loadedScene = LoadSceneDefinition(DefaultScenePathForTests());
    Expect(loadedScene.ok(),
        "TestScenePresentationKnowsEveryAuthoredSceneColorKey",
        "Default scene should load before checking presentation color keys.");
    if (!loadedScene.ok()) {
        return;
    }

    for (const SceneVisualPlaceholderDefinition& placeholder : loadedScene.scene.visualPlaceholders) {
        Expect(IsKnownSceneColorKey(placeholder.colorKey),
            "TestScenePresentationKnowsEveryAuthoredSceneColorKey",
            "Every authored visual placeholder color key should be handled by the scene presentation palette.");
    }
    for (const SceneMeshInstanceDefinition& instance : loadedScene.scene.meshInstances) {
        Expect(IsKnownSceneColorKey(instance.colorKey),
            "TestScenePresentationKnowsEveryAuthoredSceneColorKey",
            "Every authored mesh instance color key should be handled by the scene presentation palette.");
    }
}

void TestScenePresentationHasMaterialForEveryAuthoredSceneColorKey()
{
    const SceneLoadResult loadedScene = LoadSceneDefinition(DefaultScenePathForTests());
    Expect(loadedScene.ok(),
        "TestScenePresentationHasMaterialForEveryAuthoredSceneColorKey",
        "Default scene should load before checking material coverage.");
    if (!loadedScene.ok()) {
        return;
    }

    auto hasMaterial = [&loadedScene](std::string_view key) {
        for (const SceneMaterialDefinition& material : loadedScene.scene.sceneMaterials) {
            if (material.key == key) {
                return true;
            }
        }
        return false;
    };

    for (const SceneVisualPlaceholderDefinition& placeholder : loadedScene.scene.visualPlaceholders) {
        Expect(hasMaterial(placeholder.colorKey),
            "TestScenePresentationHasMaterialForEveryAuthoredSceneColorKey",
            "Every visual placeholder color key should have an authored scene material preset.");
    }
    for (const SceneMeshInstanceDefinition& instance : loadedScene.scene.meshInstances) {
        Expect(hasMaterial(instance.colorKey),
            "TestScenePresentationHasMaterialForEveryAuthoredSceneColorKey",
            "Every mesh instance color key should have an authored scene material preset.");
    }
}

void TestScenePresentationDynamicPaletteStatesRemainDistinct()
{
    ScenePresentationState openGateState;
    openGateState.routeOpened = true;
    ScenePresentationState restoredPowerState;
    restoredPowerState.powerRestored = true;
    ScenePresentationState occupiedVehicleState;
    occupiedVehicleState.vehicleOccupied = true;
    ScenePresentationState relayResetState;
    relayResetState.dockRoadRelayReset = true;
    ScenePresentationState clearanceTaggedState;
    clearanceTaggedState.dockRoadClearanceTagged = true;
    ScenePresentationState handoffFiledState;
    handoffFiledState.ferryOfficeHandoffFiled = true;
    ScenePresentationState stormPumpResetState;
    stormPumpResetState.stormPumpReset = true;
    ScenePresentationState stormPumpTicketClosedState;
    stormPumpTicketClosedState.stormPumpTicketClosed = true;
    ScenePresentationState lowDockDrainClearedState;
    lowDockDrainClearedState.lowDockDrainCleared = true;
    ScenePresentationState lowDockDrainLoggedState;
    lowDockDrainLoggedState.lowDockDrainLogged = true;

    const engine::Color closedGate = SceneColorForKey("service-gate-state", {});
    const engine::Color openGate = SceneColorForKey("service-gate-state", openGateState);
    const engine::Color offlinePower = SceneColorForKey("oxidized-service-green", {});
    const engine::Color restoredPower = SceneColorForKey("oxidized-service-green", restoredPowerState);
    const engine::Color emptyVehicle = SceneColorForKey("service-vehicle-placeholder", {});
    const engine::Color occupiedVehicle = SceneColorForKey("service-vehicle-placeholder", occupiedVehicleState);
    const engine::Color relayWaiting = SceneColorForKey("dock-road-relay-state", {});
    const engine::Color relayReset = SceneColorForKey("dock-road-relay-state", relayResetState);
    const engine::Color clearanceWaiting = SceneColorForKey("dock-road-clearance-state", {});
    const engine::Color clearanceTagged = SceneColorForKey("dock-road-clearance-state", clearanceTaggedState);
    const engine::Color handoffPending = SceneColorForKey("ferry-office-handoff-state", {});
    const engine::Color handoffFiled = SceneColorForKey("ferry-office-handoff-state", handoffFiledState);
    const engine::Color stormPumpPending = SceneColorForKey("storm-pump-state", {});
    const engine::Color stormPumpReset = SceneColorForKey("storm-pump-state", stormPumpResetState);
    const engine::Color stormTicketPending = SceneColorForKey("storm-pump-ticket-state", {});
    const engine::Color stormTicketClosed = SceneColorForKey("storm-pump-ticket-state", stormPumpTicketClosedState);
    const engine::Color drainPending = SceneColorForKey("low-dock-drain-state", {});
    const engine::Color drainCleared = SceneColorForKey("low-dock-drain-state", lowDockDrainClearedState);
    const engine::Color drainLogPending = SceneColorForKey("ferry-office-drain-log-state", {});
    const engine::Color drainLogSigned = SceneColorForKey("ferry-office-drain-log-state", lowDockDrainLoggedState);

    Expect(!ColorNear(closedGate, openGate),
        "TestScenePresentationDynamicPaletteStatesRemainDistinct",
        "Service gate palette should still communicate closed versus open state.");
    Expect(!ColorNear(offlinePower, restoredPower),
        "TestScenePresentationDynamicPaletteStatesRemainDistinct",
        "Maintenance palette should still communicate offline versus restored power.");
    Expect(!ColorNear(emptyVehicle, occupiedVehicle),
        "TestScenePresentationDynamicPaletteStatesRemainDistinct",
        "Vehicle palette should still communicate empty versus occupied state.");
    Expect(relayReset.g > relayWaiting.g && relayReset.r < relayWaiting.r,
        "TestScenePresentationDynamicPaletteStatesRemainDistinct",
        "Dock Road Relay palette should shift from warning amber toward active green after reset.");
    Expect(clearanceTagged.g > clearanceWaiting.g && clearanceTagged.b > clearanceWaiting.b,
        "TestScenePresentationDynamicPaletteStatesRemainDistinct",
        "Dock Road clearance tag palette should shift from inactive metal toward clear cyan after tagging.");
    Expect(handoffFiled.g > handoffPending.g && handoffFiled.r < handoffPending.r,
        "TestScenePresentationDynamicPaletteStatesRemainDistinct",
        "Ferry Office handoff palette should shift from pending amber toward filed green after filing.");
    Expect(stormPumpReset.b > stormPumpPending.b && stormPumpReset.g > stormPumpPending.g,
        "TestScenePresentationDynamicPaletteStatesRemainDistinct",
        "Storm pump palette should shift from warning orange toward active blue after reset.");
    Expect(stormTicketClosed.g > stormTicketPending.g && stormTicketClosed.r < stormTicketPending.r,
        "TestScenePresentationDynamicPaletteStatesRemainDistinct",
        "Storm pump ticket palette should shift from pending amber toward closed green.");
    Expect(drainCleared.g > drainPending.g && drainCleared.r < drainPending.r,
        "TestScenePresentationDynamicPaletteStatesRemainDistinct",
        "Low dock drain palette should shift from pending amber toward clear green.");
    Expect(drainLogSigned.g > drainLogPending.g && drainLogSigned.r < drainLogPending.r,
        "TestScenePresentationDynamicPaletteStatesRemainDistinct",
        "Ferry Office drain log palette should shift from pending amber toward signed green.");
}

void TestScenePresentationMaterialPresetsStayBoundedForAuthoredKeys()
{
    const SceneLoadResult loadedScene = LoadSceneDefinition(DefaultScenePathForTests());
    Expect(loadedScene.ok(),
        "TestScenePresentationMaterialPresetsStayBoundedForAuthoredKeys",
        "Default scene should load before checking presentation material presets.");
    if (!loadedScene.ok()) {
        return;
    }

    auto expectBoundedMaterial = [](std::string_view key) {
        const SceneMaterial material = SceneMaterialForKey(key);
        Expect(material.minShade > 0.0f && material.minShade <= material.maxShade && material.maxShade <= 1.25f,
            "TestScenePresentationMaterialPresetsStayBoundedForAuthoredKeys",
            "Scene material shade bounds should remain conservative for authored color keys.");
        Expect(material.ambientShade > 0.0f && material.directionalShade >= 0.0f && material.topShade >= 0.0f && material.undersideShade >= 0.0f,
            "TestScenePresentationMaterialPresetsStayBoundedForAuthoredKeys",
            "Scene material shade terms should remain nonnegative and explicit.");
    };

    for (const SceneVisualPlaceholderDefinition& placeholder : loadedScene.scene.visualPlaceholders) {
        expectBoundedMaterial(placeholder.colorKey);
    }
    for (const SceneMeshInstanceDefinition& instance : loadedScene.scene.meshInstances) {
        expectBoundedMaterial(instance.colorKey);
    }
}

void TestScenePresentationUsesAuthoredSceneMaterialDefaults()
{
    const std::vector<SceneMaterialDefinition> authoredMaterials {
        {"dock-weathered-wood", "wet", {0.90f, 0.20f, 0.10f, 1.0f}},
    };

    const SceneMaterial authored = SceneMaterialForKey("dock-weathered-wood", authoredMaterials);
    const SceneMaterial fallback = SceneMaterialForKey("dock-weathered-wood");

    Expect(ColorNear(authored.baseColor, {0.90f, 0.20f, 0.10f, 1.0f}),
        "TestScenePresentationUsesAuthoredSceneMaterialDefaults",
        "Authored scene material base colors should override the built-in fallback palette.");
    Expect(authored.maxShade > fallback.maxShade,
        "TestScenePresentationUsesAuthoredSceneMaterialDefaults",
        "Authored scene material response should override the built-in fallback response.");
}

void TestScenePresentationMaterialPresetsVarySurfaceResponse()
{
    const SceneMaterial wetAsphalt = SceneMaterialForKey("dark-service-asphalt");
    const SceneMaterial mutedConcrete = SceneMaterialForKey("office-muted-concrete");
    const SceneMaterial paintedPost = SceneMaterialForKey("salt-white-road-post");

    Expect(wetAsphalt.maxShade > mutedConcrete.maxShade,
        "TestScenePresentationMaterialPresetsVarySurfaceResponse",
        "Wet asphalt should allow a stronger top-light response than matte concrete.");
    Expect(wetAsphalt.minShade < mutedConcrete.minShade,
        "TestScenePresentationMaterialPresetsVarySurfaceResponse",
        "Wet asphalt should be allowed to fall darker than matte concrete.");
    Expect(paintedPost.directionalShade > mutedConcrete.directionalShade,
        "TestScenePresentationMaterialPresetsVarySurfaceResponse",
        "Painted props should keep a clearer directional cue than matte concrete.");
}

void TestScenePresentationOvercastShadingAddsVolumeCue()
{
    const engine::Color base {0.50f, 0.50f, 0.50f, 1.0f};
    const engine::Color top = SceneShadedColor(
        base,
        {-1.0f, 0.0f, -1.0f},
        {1.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, -1.0f});
    const engine::Color underside = SceneShadedColor(
        base,
        {-1.0f, 0.0f, -1.0f},
        {1.0f, 0.0f, -1.0f},
        {1.0f, 0.0f, 1.0f});

    Expect(Luminance(top) > Luminance(underside),
        "TestScenePresentationOvercastShadingAddsVolumeCue",
        "Overcast shading should make upward faces read brighter than undersides.");
}

void TestScenePresentationShadedBoxSubmitsTwelveFaces()
{
    CountingRenderer renderer;
    renderer.initialize({});
    DrawSceneShadedBox(renderer, {0.0f, 0.5f, 0.0f}, {0.5f, 0.5f, 0.5f}, engine::Color {0.50f, 0.50f, 0.50f, 1.0f});

    Expect(renderer.flatTriangleDrawCount == 12,
        "TestScenePresentationShadedBoxSubmitsTwelveFaces",
        "A shaded presentation box should submit exactly twelve face triangles.");
}

void TestSandboxLayerDrawsEveryLoadedSceneMeshInstance()
{
    const std::filesystem::path defaultScenePath = DefaultScenePathForTests();
    std::ifstream input(defaultScenePath);
    std::string sceneText((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

    const std::string extraAsset =
        R"(,
    {
      "id": "alternate-unit-box-mesh",
      "path": "assets/models/unit_box.gltf",
      "format": "gltf",
      "units": "meter",
      "upAxis": "Y",
      "license": "project-original",
      "provenance": "Temporary test asset alias used to prove multi-asset runtime rendering.",
      "authoringBoundsHalfExtents": [0.5, 0.5, 0.5]
    })";
    const std::string extraInstance =
        R"(,
    {
      "id": "mesh-runtime-multi-asset-proof",
      "assetId": "alternate-unit-box-mesh",
      "position": [1.75, 0.5, -1.25],
      "yawDegrees": 0.0,
      "scale": [0.35, 0.35, 0.35],
      "colorKey": "dock-muted-sign-yellow"
    })";

    const std::string meshAssetNeedle = "\n  ],\n  \"meshInstances\"";
    const std::string meshInstanceNeedle = "\n  ],\n  \"interactables\"";
    const std::size_t meshAssetInsert = sceneText.find(meshAssetNeedle);
    const std::size_t meshInstanceInsert = sceneText.find(meshInstanceNeedle);
    Expect(meshAssetInsert != std::string::npos && meshInstanceInsert != std::string::npos,
        "TestSandboxLayerDrawsEveryLoadedSceneMeshInstance",
        "Default scene JSON should expose insertion points for mesh asset and instance test data.");
    if (meshAssetInsert == std::string::npos || meshInstanceInsert == std::string::npos) {
        return;
    }

    sceneText.insert(meshInstanceInsert, extraInstance);
    sceneText.insert(meshAssetInsert, extraAsset);

    const std::filesystem::path tempScenePath =
        std::filesystem::temp_directory_path() / "tidebreak-v018-multi-mesh.scene.json";
    {
        std::ofstream output(tempScenePath, std::ios::binary);
        output << sceneText;
    }

    const SceneLoadResult loadedScene = LoadSceneDefinition(tempScenePath);
    Expect(loadedScene.ok(),
        "TestSandboxLayerDrawsEveryLoadedSceneMeshInstance",
        "Temporary multi-asset scene should load successfully.");

    SandboxLayer layer(tempScenePath, engine::UiMode::Playtest);
    layer.onAttach();
    CountingRenderer renderer;
    renderer.initialize({});
    renderer.beginFrame(1);
    layer.onRender(renderer);
    renderer.endFrame();
    layer.onDetach();

    Expect(renderer.flatTriangleDrawCount >= loadedScene.scene.meshInstances.size(),
        "TestSandboxLayerDrawsEveryLoadedSceneMeshInstance",
        "Sandbox rendering should submit every authored scene mesh instance, including non-unit-box asset ids.");

    std::filesystem::remove(tempScenePath);
}

void TestSandboxLayerPlaytestRenderSuppressesRawDebugScaffolding()
{
    const SceneLoadResult loadedScene = LoadSceneDefinition(DefaultScenePathForTests());
    Expect(loadedScene.ok(),
        "TestSandboxLayerPlaytestRenderSuppressesRawDebugScaffolding",
        "Default scene should load before checking playtest render scaffolding.");

    SandboxLayer layer(DefaultScenePathForTests(), engine::UiMode::Playtest);
    layer.onAttach();
    CountingRenderer renderer;
    renderer.initialize({});
    renderer.beginFrame(1);
    layer.onRender(renderer);
    renderer.endFrame();
    layer.onDetach();

    Expect(renderer.flatTriangleDrawCount >= loadedScene.scene.meshInstances.size(),
        "TestSandboxLayerPlaytestRenderSuppressesRawDebugScaffolding",
        "Playtest rendering should keep authored mesh instances visible.");
    Expect(renderer.boxCalls.empty(),
        "TestSandboxLayerPlaytestRenderSuppressesRawDebugScaffolding",
        "Playtest rendering should suppress raw wire boxes, trigger radii, and debug outlines.");
    Expect(CountLinesWithColor(renderer, {0.70f, 0.92f, 1.0f, 1.0f}) == 0,
        "TestSandboxLayerPlaytestRenderSuppressesRawDebugScaffolding",
        "Playtest rendering should not draw raw route debug lines.");
}

void TestSandboxLayerDebugRenderKeepsRawDebugScaffolding()
{
    SandboxLayer layer(DefaultScenePathForTests(), engine::UiMode::Debug);
    layer.onAttach();
    CountingRenderer renderer;
    renderer.initialize({});
    renderer.beginFrame(1);
    layer.onRender(renderer);
    renderer.endFrame();
    layer.onDetach();

    Expect(!renderer.boxCalls.empty(),
        "TestSandboxLayerDebugRenderKeepsRawDebugScaffolding",
        "Debug rendering should keep wire boxes, trigger radii, and debug outlines available behind F1.");
    Expect(CountLinesWithColor(renderer, {0.70f, 0.92f, 1.0f, 1.0f}) > 0,
        "TestSandboxLayerDebugRenderKeepsRawDebugScaffolding",
        "Debug rendering should keep route debug lines available for validation.");
}

void TestSandboxLayerPlaytestTextPrioritizesObjectiveAndPrompt()
{
    SandboxLayer layer(DefaultScenePathForTests(), engine::UiMode::Playtest);
    layer.onAttach();

    engine::InputState input;
    layer.onUpdate(0.016, input);
    const std::string text = layer.debugText();
    layer.onDetach();

    Expect(text.find("Objective: Check the Ferry Manifest") != std::string::npos,
        "TestSandboxLayerPlaytestTextPrioritizesObjectiveAndPrompt",
        "Playtest UI should lead with the current player-facing job objective.");
    Expect(text.find("Press E: Collect Ferry Manifest") != std::string::npos,
        "TestSandboxLayerPlaytestTextPrioritizesObjectiveAndPrompt",
        "Playtest UI should keep the focused interaction prompt visible.");
    Expect(text.find("F1: debug") != std::string::npos,
        "TestSandboxLayerPlaytestTextPrioritizesObjectiveAndPrompt",
        "Playtest UI should document the debug overlay toggle.");
    Expect(text.find("player=(") == std::string::npos,
        "TestSandboxLayerPlaytestTextPrioritizesObjectiveAndPrompt",
        "Playtest UI should not expose raw player telemetry.");
    Expect(text.find("worldState={") == std::string::npos,
        "TestSandboxLayerPlaytestTextPrioritizesObjectiveAndPrompt",
        "Playtest UI should not expose the raw world-state dump.");
    Expect(text.find("phase=") == std::string::npos,
        "TestSandboxLayerPlaytestTextPrioritizesObjectiveAndPrompt",
        "Playtest UI should describe job progress without raw phase telemetry.");
}

void TestSandboxLayerPilotSliceUsesNeutralPresentation()
{
    SandboxLayer layer(PilotScenePathForTests(), engine::UiMode::Playtest);
    layer.onAttach();

    engine::InputState input;
    layer.onUpdate(0.016, input);
    const std::string text = layer.debugText();
    layer.onDetach();

    Expect(text.find("Scene: Veyra Reach Pilot Slice | role=target-slice-scaffold") != std::string::npos,
        "TestSandboxLayerPilotSliceUsesNeutralPresentation",
        "Pilot runtime smoke text should identify the target slice instead of the Ferry Office job.");
    Expect(text.find("Status: colliders=1 | interactables=1 | routes=1 | markers=2 | vehicle=none") != std::string::npos,
        "TestSandboxLayerPilotSliceUsesNeutralPresentation",
        "Pilot runtime smoke text should report neutral scene counts and no inherited vehicle.");
    Expect(text.find("Ferry Office") == std::string::npos,
        "TestSandboxLayerPilotSliceUsesNeutralPresentation",
        "Pilot target slice must not leak Ferry Office wording into playtest presentation.");
    Expect(text.find("Job:") == std::string::npos,
        "TestSandboxLayerPilotSliceUsesNeutralPresentation",
        "Pilot target slice must not present a Ferry Office job row.");
    Expect(text.find("service gate=") == std::string::npos,
        "TestSandboxLayerPilotSliceUsesNeutralPresentation",
        "Pilot target slice must not expose Ferry Office service-gate state.");
}

void TestSandboxLayerPilotSliceDebugTextAvoidsFerryOfficeTelemetry()
{
    SandboxLayer layer(PilotScenePathForTests(), engine::UiMode::Debug);
    layer.onAttach();

    engine::InputState input;
    layer.onUpdate(0.016, input);
    const std::string text = layer.debugText();
    layer.onDetach();

    Expect(text.find("scene=veyra-reach-pilot loaded=yes role=target-slice-scaffold") != std::string::npos,
        "TestSandboxLayerPilotSliceDebugTextAvoidsFerryOfficeTelemetry",
        "Pilot debug text should identify the target-slice role.");
    Expect(text.find("sceneCounts=colliders:1 interactables:1 routes:1 markers:2 vehicle:none") != std::string::npos,
        "TestSandboxLayerPilotSliceDebugTextAvoidsFerryOfficeTelemetry",
        "Pilot debug text should expose neutral scene counts instead of Ferry Office job telemetry.");
    Expect(text.find("roadSegment=dock-road") == std::string::npos,
        "TestSandboxLayerPilotSliceDebugTextAvoidsFerryOfficeTelemetry",
        "Pilot debug text must not claim the Ferry Office dock-road segment.");
    Expect(text.find("roadBounds=") == std::string::npos,
        "TestSandboxLayerPilotSliceDebugTextAvoidsFerryOfficeTelemetry",
        "Pilot debug text must not expose service-yard vehicle bounds.");
    Expect(text.find("jobObjective=") == std::string::npos,
        "TestSandboxLayerPilotSliceDebugTextAvoidsFerryOfficeTelemetry",
        "Pilot debug text must not expose Ferry Office job wording.");
    Expect(text.find("worldState={powerRestored") == std::string::npos,
        "TestSandboxLayerPilotSliceDebugTextAvoidsFerryOfficeTelemetry",
        "Pilot debug text must not expose Ferry Office world-state flags.");
}

void TestSandboxLayerPlaytestDefersFutureRouteGuidanceAtStart()
{
    SandboxLayer layer(DefaultScenePathForTests(), engine::UiMode::Playtest);
    layer.onAttach();

    engine::InputState input;
    layer.onUpdate(0.016, input);
    CountingRenderer renderer;
    renderer.initialize({});
    renderer.beginFrame(1);
    layer.onRender(renderer);
    renderer.endFrame();
    layer.onDetach();

    const engine::Color routeColor {0.70f, 0.92f, 1.0f, 1.0f};
    Expect(CountLinesWithColor(renderer, routeColor) == 0,
        "TestSandboxLayerPlaytestDefersFutureRouteGuidanceAtStart",
        "Playtest mode should not draw future route guidance before the Ferry Manifest starts the job loop.");
}

void TestSandboxLayerDebugPreservesFullRouteGuidanceAtStart()
{
    const SceneLoadResult result = LoadSceneDefinition(DefaultScenePathForTests());
    Expect(result.ok(),
        "TestSandboxLayerDebugPreservesFullRouteGuidanceAtStart",
        "Default scene should load before counting authored route guidance.");
    if (!result.ok()) {
        return;
    }

    SandboxLayer layer(DefaultScenePathForTests(), engine::UiMode::Debug);
    layer.onAttach();

    engine::InputState input;
    layer.onUpdate(0.016, input);
    CountingRenderer renderer;
    renderer.initialize({});
    renderer.beginFrame(1);
    layer.onRender(renderer);
    renderer.endFrame();
    layer.onDetach();

    const engine::Color routeColor {0.70f, 0.92f, 1.0f, 1.0f};
    Expect(CountLinesWithColor(renderer, routeColor) == CountRouteMarkerSegments(result.scene),
        "TestSandboxLayerDebugPreservesFullRouteGuidanceAtStart",
        "Debug mode should keep every authored route marker visible for validation.");
}

void TestSandboxLayerUsesScenePlayerStartYawForInitialComposition()
{
    const std::filesystem::path scenePath = WriteTempSceneWithPlayerStartYaw(-18.0f);
    SandboxLayer layer(scenePath, engine::UiMode::Debug);
    layer.onAttach();

    engine::InputState input;
    layer.onUpdate(0.016, input);
    const std::string text = layer.debugText();

    CountingRenderer renderer;
    renderer.initialize({});
    renderer.beginFrame(1);
    layer.onRender(renderer);
    renderer.endFrame();
    layer.onDetach();

    Expect(text.find("playerYaw=-18.00") != std::string::npos,
        "TestSandboxLayerUsesScenePlayerStartYawForInitialComposition",
        "Sandbox debug text should show that authored player start yaw becomes the initial player facing.");
    Expect(text.find("camera yaw=-18.00") != std::string::npos,
        "TestSandboxLayerUsesScenePlayerStartYawForInitialComposition",
        "Sandbox camera should start from the scene-authored player start yaw.");
    Expect(renderer.debugCameraSet && renderer.debugCamera.position.x > 0.9f,
        "TestSandboxLayerUsesScenePlayerStartYawForInitialComposition",
        "Initial debug camera should be offset sideways when the authored start yaw composes the scene off-axis.");
}

void TestSandboxLayerPlaytestKeepsRouteDebugHiddenAfterManifest()
{
    const SceneLoadResult result = LoadSceneDefinition(DefaultScenePathForTests());
    Expect(result.ok(),
        "TestSandboxLayerPlaytestKeepsRouteDebugHiddenAfterManifest",
        "Default scene should load before counting route guidance.");
    if (!result.ok()) {
        return;
    }

    SandboxLayer layer(DefaultScenePathForTests(), engine::UiMode::Playtest);
    layer.onAttach();

    engine::InputState input;
    layer.onUpdate(0.016, input);
    input.interactPressed = true;
    layer.onUpdate(0.016, input);
    input.interactPressed = false;
    layer.onUpdate(0.016, input);

    CountingRenderer renderer;
    renderer.initialize({});
    renderer.beginFrame(1);
    layer.onRender(renderer);
    renderer.endFrame();
    layer.onDetach();

    const engine::Color routeColor {0.70f, 0.92f, 1.0f, 1.0f};
    const std::size_t routeLines = CountLinesWithColor(renderer, routeColor);
    Expect(routeLines > 0,
        "TestSandboxLayerPlaytestKeepsRouteDebugHiddenAfterManifest",
        "After collecting the manifest, playtest mode should draw the active route guidance leg.");
    Expect(routeLines < CountRouteMarkerSegments(result.scene),
        "TestSandboxLayerPlaytestKeepsRouteDebugHiddenAfterManifest",
        "Playtest mode should draw only the active route leg, while F1 debug preserves all route validation.");
}

void TestSandboxLayerQaCaptureStateDrawsMidChainRouteGuidance()
{
    SandboxLayer layer(
        DefaultScenePathForTests(),
        engine::UiMode::Playtest,
        engine::physics::PhysicsBackend::Simple,
        false,
        "relay-to-service-log");
    layer.onAttach();

    CountingRenderer renderer;
    renderer.initialize({});
    renderer.beginFrame(1);
    layer.onRender(renderer);
    renderer.endFrame();
    const std::string text = layer.debugText();
    layer.onDetach();

    const engine::Color routeColor {0.70f, 0.92f, 1.0f, 1.0f};
    Expect(CountLinesWithColor(renderer, routeColor) > 0,
        "TestSandboxLayerQaCaptureStateDrawsMidChainRouteGuidance",
        "QA capture state should draw the active relay-to-service-log route guidance in playtest mode.");
    Expect(text.find("Relay Service Log") != std::string::npos,
        "TestSandboxLayerQaCaptureStateDrawsMidChainRouteGuidance",
        "QA capture state should preload the follow-up step that points to the Relay Service Log.");
}

void TestSandboxLayerQaCaptureStateDrawsLowDockDrainAccessGuidance()
{
    SandboxLayer layer(
        DefaultScenePathForTests(),
        engine::UiMode::Playtest,
        engine::physics::PhysicsBackend::Simple,
        false,
        "low-dock-drain-access");
    layer.onAttach();

    CountingRenderer renderer;
    renderer.initialize({});
    renderer.beginFrame(1);
    layer.onRender(renderer);
    renderer.endFrame();
    const std::string text = layer.debugText();
    layer.onDetach();

    const engine::Color routeColor {0.70f, 0.92f, 1.0f, 1.0f};
    Expect(CountLinesWithColor(renderer, routeColor) > 0,
        "TestSandboxLayerQaCaptureStateDrawsLowDockDrainAccessGuidance",
        "QA capture state should draw the active low-dock-drain route guidance in playtest mode.");
    Expect(text.find("low dock drain") != std::string::npos,
        "TestSandboxLayerQaCaptureStateDrawsLowDockDrainAccessGuidance",
        "QA capture state should preload the opened-access follow-up step that points to the Low Dock Drain.");
}

void TestSandboxLayerQaCaptureStateDrawsVehicleDockRoadPerspective()
{
    SandboxLayer layer(
        DefaultScenePathForTests(),
        engine::UiMode::Playtest,
        engine::physics::PhysicsBackend::Simple,
        false,
        "vehicle-dock-road-forward");
    layer.onAttach();

    engine::InputState input;
    layer.onUpdate(0.016, input);
    CountingRenderer renderer;
    renderer.initialize({});
    renderer.beginFrame(1);
    layer.onRender(renderer);
    renderer.endFrame();
    const std::string text = layer.debugText();
    layer.onDetach();

    const engine::Color routeColor {0.70f, 0.92f, 1.0f, 1.0f};
    Expect(CountLinesWithColor(renderer, routeColor) > 0,
        "TestSandboxLayerQaCaptureStateDrawsVehicleDockRoadPerspective",
        "Vehicle QA capture state should still draw the active dock-road route guidance in playtest mode.");
    Expect(text.find("mode=driving") != std::string::npos,
        "TestSandboxLayerQaCaptureStateDrawsVehicleDockRoadPerspective",
        "Vehicle QA capture state should put the presentation overlay into driving mode.");
    Expect(text.find("Drive: speed=") != std::string::npos,
        "TestSandboxLayerQaCaptureStateDrawsVehicleDockRoadPerspective",
        "Vehicle QA capture state should expose driving HUD text for the capture.");
}

void TestSandboxLayerDebugTextPreservesFullTelemetry()
{
    SandboxLayer layer(DefaultScenePathForTests(), engine::UiMode::Debug);
    layer.onAttach();

    engine::InputState input;
    layer.onUpdate(0.016, input);
    const std::string text = layer.debugText();
    layer.onDetach();

    Expect(text.find("jobObjective=") != std::string::npos,
        "TestSandboxLayerDebugTextPreservesFullTelemetry",
        "Debug UI should preserve the legacy job objective telemetry.");
    Expect(text.find("player=(") != std::string::npos,
        "TestSandboxLayerDebugTextPreservesFullTelemetry",
        "Debug UI should preserve raw player telemetry.");
    Expect(text.find("vehicle=(") != std::string::npos,
        "TestSandboxLayerDebugTextPreservesFullTelemetry",
        "Debug UI should preserve vehicle telemetry.");
    Expect(text.find("worldState={") != std::string::npos,
        "TestSandboxLayerDebugTextPreservesFullTelemetry",
        "Debug UI should preserve the raw world-state dump.");
}

void TestSandboxLayerMinimalTextStaysSmallButUseful()
{
    SandboxLayer layer(DefaultScenePathForTests(), engine::UiMode::Minimal);
    layer.onAttach();

    engine::InputState input;
    layer.onUpdate(0.016, input);
    const std::string text = layer.debugText();
    layer.onDetach();

    Expect(text.find("Objective: Check the Ferry Manifest") != std::string::npos,
        "TestSandboxLayerMinimalTextStaysSmallButUseful",
        "Minimal UI should still show the current objective.");
    Expect(text.find("Press E: Collect Ferry Manifest") != std::string::npos,
        "TestSandboxLayerMinimalTextStaysSmallButUseful",
        "Minimal UI should still show the focused prompt.");
    Expect(text.find("F1: debug") == std::string::npos,
        "TestSandboxLayerMinimalTextStaysSmallButUseful",
        "Minimal UI should avoid extra helper text.");
    Expect(text.find("jobPhase=") == std::string::npos && text.find("worldState={") == std::string::npos,
        "TestSandboxLayerMinimalTextStaysSmallButUseful",
        "Minimal UI should avoid raw debug telemetry.");
}

void TestSandboxLayerF1TogglesBetweenPlaytestAndDebugText()
{
    SandboxLayer layer(DefaultScenePathForTests(), engine::UiMode::Playtest);
    layer.onAttach();

    engine::InputState input;
    layer.onUpdate(0.016, input);
    const std::string playtestText = layer.debugText();

    input.debugOverlayTogglePressed = true;
    layer.onUpdate(0.016, input);
    const std::string debugText = layer.debugText();

    input.debugOverlayTogglePressed = false;
    layer.onUpdate(0.016, input);

    input.debugOverlayTogglePressed = true;
    layer.onUpdate(0.016, input);
    const std::string toggledBackText = layer.debugText();
    layer.onDetach();

    Expect(playtestText.find("worldState={") == std::string::npos,
        "TestSandboxLayerF1TogglesBetweenPlaytestAndDebugText",
        "Initial playtest UI should hide the raw world-state dump.");
    Expect(debugText.find("worldState={") != std::string::npos,
        "TestSandboxLayerF1TogglesBetweenPlaytestAndDebugText",
        "F1 should switch playtest UI into full debug telemetry.");
    Expect(toggledBackText.find("worldState={") == std::string::npos,
        "TestSandboxLayerF1TogglesBetweenPlaytestAndDebugText",
        "A second F1 press should return to playtest UI.");
}

void TestDebugWindowTitleUsesStableSingleLine()
{
    const std::string debugText =
        "objective=\"Collect the Ferry Manifest at the dock-side office approach.\" readyForExit=no sliceComplete=no\n"
        "focus=Ferry Manifest prompt=\"Press E: Collect Ferry Manifest\" travFocus=none\n"
        "vehicle=(6.20,0.00,-2.20) empty speed=0.00";

    const std::string title = engine::BuildDebugWindowTitle("Tidebreak Prototype", debugText);

    Expect(title.find('\n') == std::string::npos,
        "TestDebugWindowTitleUsesStableSingleLine",
        "Window title should never include the full multi-line debug UI.");
    Expect(title.size() <= 140,
        "TestDebugWindowTitleUsesStableSingleLine",
        "Window title should stay short enough to avoid title-bar repaint noise.");
    Expect(title.find("focus=") == std::string::npos,
        "TestDebugWindowTitleUsesStableSingleLine",
        "Window title should keep only the first debug summary line.");
}

void TestGameCodeDoesNotReferenceJoltVendorApi()
{
#ifdef ENGINE_SOURCE_ROOT
    const std::filesystem::path gameRoot = std::filesystem::path(ENGINE_SOURCE_ROOT) / "src" / "game";
    Expect(std::filesystem::exists(gameRoot),
        "TestGameCodeDoesNotReferenceJoltVendorApi",
        "Source root should expose src/game for vendor firewall checks.");

    for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(gameRoot)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const std::filesystem::path path = entry.path();
        if (path.extension() != ".h" && path.extension() != ".cpp") {
            continue;
        }

        std::ifstream file(path);
        const std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        const bool referencesJolt = contents.find("Jolt") != std::string::npos
            || contents.find("JPH::") != std::string::npos
            || contents.find("<Jolt/") != std::string::npos
            || contents.find("JPH/") != std::string::npos;

        Expect(!referencesJolt,
            "TestGameCodeDoesNotReferenceJoltVendorApi",
            "Game code should not reference Jolt vendor API: " + path.string());
    }
#endif
}

void TestVec3NormalizationKeepsDiagonalMovementAtUnitLength()
{
    const engine::Vec3 diagonal = engine::Normalize(engine::Vec3 {3.0f, 0.0f, 4.0f});
    ExpectNear(engine::Length(diagonal), 1.0f, 0.001f,
        "TestVec3NormalizationKeepsDiagonalMovementAtUnitLength",
        "Normalized diagonal vector should have unit length.");

    const engine::Vec3 zero = engine::Normalize(engine::Vec3 {});
    ExpectNear(engine::Length(zero), 0.0f, 0.001f,
        "TestVec3NormalizationKeepsDiagonalMovementAtUnitLength",
        "Zero vector should remain zero after normalization.");
}

void TestPlayerMovementIsCameraRelativeAndNormalized()
{
    PlayerController player;
    PlayerControllerSettings settings;
    settings.walkSpeed = 4.0f;
    settings.sprintSpeed = 8.0f;
    player.setSettings(settings);

    engine::InputState input;
    input.moveForward = 1.0f;
    input.moveRight = 1.0f;

    player.update(1.0f, input, 0.0f);

    ExpectNear(player.state().horizontalSpeed, settings.walkSpeed, 0.01f,
        "TestPlayerMovementIsCameraRelativeAndNormalized",
        "Diagonal movement should be normalized to walk speed.");
    Expect(player.state().facingYawRadians > 0.0f,
        "TestPlayerMovementIsCameraRelativeAndNormalized",
        "Facing yaw should follow the camera-relative movement direction.");
}

void TestPlayerSprintAndJumpRemainGroundedDeterministically()
{
    PlayerController player;
    PlayerControllerSettings settings;
    settings.walkSpeed = 4.0f;
    settings.sprintSpeed = 8.0f;
    settings.jumpImpulse = 5.0f;
    settings.gravity = 20.0f;
    player.setSettings(settings);

    engine::InputState input;
    input.moveForward = 1.0f;
    input.sprintHeld = true;
    input.jumpPressed = true;

    player.update(0.1f, input, 0.0f);
    Expect(player.state().position.y > 0.0f,
        "TestPlayerSprintAndJumpRemainGroundedDeterministically",
        "Jump should lift the player off the floor.");
    Expect(!player.state().grounded,
        "TestPlayerSprintAndJumpRemainGroundedDeterministically",
        "Player should not be grounded immediately after jump impulse.");

    input.jumpPressed = false;
    for (int i = 0; i < 60; ++i) {
        player.update(0.016f, input, 0.0f);
    }

    ExpectNear(player.state().position.y, 0.0f, 0.001f,
        "TestPlayerSprintAndJumpRemainGroundedDeterministically",
        "Player should land back on the floor.");
    Expect(player.state().grounded,
        "TestPlayerSprintAndJumpRemainGroundedDeterministically",
        "Player should be grounded after landing.");
}

void TestThirdPersonCameraClampsPitchAndSmooths()
{
    ThirdPersonCamera camera;
    ThirdPersonCameraSettings settings;
    settings.distance = 6.0f;
    settings.heightOffset = 1.6f;
    settings.minPitchRadians = -0.5f;
    settings.maxPitchRadians = 0.75f;
    settings.smoothing = 8.0f;
    camera.setSettings(settings);

    CameraTarget target;
    target.position = {0.0f, 0.0f, 0.0f};
    target.yawRadians = 0.0f;

    engine::InputState input;
    input.cameraPitchDelta = 100.0f;
    input.cameraYawDelta = 2.0f;

    camera.update(0.016f, input, target);
    Expect(camera.state().pitchRadians <= settings.maxPitchRadians,
        "TestThirdPersonCameraClampsPitchAndSmooths",
        "Camera pitch should clamp to the configured maximum.");
    Expect(camera.state().yawRadians > 0.0f,
        "TestThirdPersonCameraClampsPitchAndSmooths",
        "Camera yaw should respond to yaw input.");
    Expect(engine::Length(camera.state().position - target.position) > 0.1f,
        "TestThirdPersonCameraClampsPitchAndSmooths",
        "Camera should have a follow position offset from the target.");
}

void TestAabbOverlapAndClosestPoint()
{
    StaticCollider collider;
    collider.bounds.center = {0.0f, 1.0f, 0.0f};
    collider.bounds.halfExtents = {1.0f, 1.0f, 1.0f};

    Expect(collider.bounds.overlaps({0.5f, 1.0f, 0.5f}, {0.25f, 0.25f, 0.25f}),
        "TestAabbOverlapAndClosestPoint",
        "AABB should overlap a small box inside it.");
    Expect(!collider.bounds.overlaps({3.0f, 1.0f, 0.0f}, {0.25f, 0.25f, 0.25f}),
        "TestAabbOverlapAndClosestPoint",
        "AABB should not overlap a distant box.");

    const engine::Vec3 closest = collider.bounds.closestPoint({3.0f, 5.0f, -3.0f});
    ExpectNear(closest.x, 1.0f, 0.001f, "TestAabbOverlapAndClosestPoint", "Closest point should clamp X.");
    ExpectNear(closest.y, 2.0f, 0.001f, "TestAabbOverlapAndClosestPoint", "Closest point should clamp Y.");
    ExpectNear(closest.z, -1.0f, 0.001f, "TestAabbOverlapAndClosestPoint", "Closest point should clamp Z.");
}

void TestPrototypeWorldGroundClampAndGroundedState()
{
    PrototypeWorld world;
    world.setFloorHeight(0.0f);

    PlayerCollisionProxy proxy;
    proxy.position = {0.0f, -0.5f, 0.0f};
    proxy.velocity = {0.0f, -2.0f, 0.0f};
    proxy.radius = 0.35f;
    proxy.height = 1.8f;

    const CollisionResult result = world.resolvePlayer(proxy);
    ExpectNear(result.position.y, 0.0f, 0.001f,
        "TestPrototypeWorldGroundClampAndGroundedState",
        "World collision should clamp player to the floor height.");
    Expect(result.grounded,
        "TestPrototypeWorldGroundClampAndGroundedState",
        "Player should be grounded after floor clamp.");
    ExpectNear(result.velocity.y, 0.0f, 0.001f,
        "TestPrototypeWorldGroundClampAndGroundedState",
        "Downward velocity should be removed by floor collision.");
}

void TestPrototypeWorldPushesPlayerOutOfBox()
{
    PrototypeWorld world;
    world.addBox("center-box", {0.0f, 0.5f, 0.0f}, {1.0f, 0.5f, 1.0f});

    PlayerCollisionProxy proxy;
    proxy.previousPosition = {-2.0f, 0.0f, 0.0f};
    proxy.position = {0.0f, 0.0f, 0.0f};
    proxy.velocity = {5.0f, 0.0f, 0.0f};
    proxy.radius = 0.35f;
    proxy.height = 1.8f;

    const CollisionResult result = world.resolvePlayer(proxy);
    Expect(result.hitCount > 0,
        "TestPrototypeWorldPushesPlayerOutOfBox",
        "Player should report a collider hit.");
    Expect(result.position.x <= -1.34f || result.position.x >= 1.34f,
        "TestPrototypeWorldPushesPlayerOutOfBox",
        "Player should be pushed outside the expanded box.");
    Expect(engine::Length(result.lastPush) > 0.0f,
        "TestPrototypeWorldPushesPlayerOutOfBox",
        "Collision result should expose the push vector for debug drawing.");
}

void TestPlayerMovementUsesWorldCollisionForWall()
{
    PrototypeWorld world;
    world.addBox("wall", {0.0f, 0.5f, 1.2f}, {3.0f, 0.5f, 0.2f});

    PlayerController player;
    player.setWorld(&world);
    player.setPosition({0.0f, 0.0f, 0.0f});

    engine::InputState input;
    input.moveForward = 1.0f;
    for (int i = 0; i < 20; ++i) {
        player.update(0.05f, input, 0.0f);
    }

    Expect(player.state().position.z <= 0.66f,
        "TestPlayerMovementUsesWorldCollisionForWall",
        "Player should not move through a simple wall collider.");
    Expect(player.state().lastCollisionHitCount > 0,
        "TestPlayerMovementUsesWorldCollisionForWall",
        "Player should record collision hits from the world.");
}

void TestDiagonalMovementIntoObstacleDoesNotTunnel()
{
    PrototypeWorld world;
    world.addBox("corner", {1.0f, 0.5f, 1.0f}, {0.5f, 0.5f, 0.5f});

    PlayerController player;
    player.setWorld(&world);
    player.setPosition({0.0f, 0.0f, 0.0f});

    engine::InputState input;
    input.moveForward = 1.0f;
    input.moveRight = 1.0f;
    for (int i = 0; i < 30; ++i) {
        player.update(0.033f, input, 0.0f);
    }

    const StaticCollider* collider = world.colliderByName("corner");
    Expect(collider != nullptr,
        "TestDiagonalMovementIntoObstacleDoesNotTunnel",
        "Corner collider should exist.");
    if (collider) {
        Expect(!world.playerOverlapsCollider(player.state().position, player.settings().radius, player.settings().height, *collider),
            "TestDiagonalMovementIntoObstacleDoesNotTunnel",
            "Player proxy should not remain inside the corner collider after resolution.");
    }
}

void TestPrototypeWorldRaycastFindsNearestCollider()
{
    PrototypeWorld world;
    world.addBox("near", {0.0f, 0.5f, 2.0f}, {0.5f, 0.5f, 0.5f});
    world.addBox("far", {0.0f, 0.5f, 5.0f}, {0.5f, 0.5f, 0.5f});

    const RaycastHit hit = world.raycast({0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, 10.0f);
    Expect(hit.hit,
        "TestPrototypeWorldRaycastFindsNearestCollider",
        "Raycast should hit a collider.");
    Expect(hit.colliderName == "near",
        "TestPrototypeWorldRaycastFindsNearestCollider",
        "Raycast should return the nearest collider.");
    ExpectNear(hit.distance, 1.5f, 0.01f,
        "TestPrototypeWorldRaycastFindsNearestCollider",
        "Raycast distance should land on the near box front face.");
}

void TestInteractionFocusSelectsNearestFacingCandidate()
{
    InteractionSystem interactions;

    Interactable nearPickup;
    nearPickup.name = "Near Pickup";
    nearPickup.prompt = "Pick up Near Pickup";
    nearPickup.position = {0.0f, 0.5f, 1.25f};
    nearPickup.radius = 2.0f;
    nearPickup.type = InteractableType::Pickup;
    const int nearId = interactions.addInteractable(nearPickup);

    Interactable farInfo;
    farInfo.name = "Far Info";
    farInfo.prompt = "Read Far Info";
    farInfo.position = {0.0f, 0.5f, 3.0f};
    farInfo.radius = 4.0f;
    farInfo.type = InteractableType::Info;
    interactions.addInteractable(farInfo);

    const InteractionFocus focus = interactions.updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
    Expect(focus.hasFocus,
        "TestInteractionFocusSelectsNearestFacingCandidate",
        "A nearby interactable in front of the player should receive focus.");
    Expect(focus.interactableId == nearId,
        "TestInteractionFocusSelectsNearestFacingCandidate",
        "Nearest facing interactable should win focus.");
    Expect(focus.prompt == nearPickup.prompt,
        "TestInteractionFocusSelectsNearestFacingCandidate",
        "Focus should expose prompt text for debug display.");
}

void TestInteractionFocusIgnoresDisabledAndConsumedInteractables()
{
    InteractionSystem interactions;

    Interactable disabled;
    disabled.name = "Disabled Pickup";
    disabled.prompt = "Pick up Disabled Pickup";
    disabled.position = {0.0f, 0.5f, 0.75f};
    disabled.radius = 2.0f;
    disabled.type = InteractableType::Pickup;
    disabled.enabled = false;
    interactions.addInteractable(disabled);

    Interactable consumed;
    consumed.name = "Consumed Pickup";
    consumed.prompt = "Pick up Consumed Pickup";
    consumed.position = {0.0f, 0.5f, 1.0f};
    consumed.radius = 2.0f;
    consumed.type = InteractableType::Pickup;
    consumed.oneShot = true;
    consumed.consumed = true;
    interactions.addInteractable(consumed);

    Interactable active;
    active.name = "Active Info";
    active.prompt = "Read Active Info";
    active.position = {0.0f, 0.5f, 1.25f};
    active.radius = 2.0f;
    active.type = InteractableType::Info;
    const int activeId = interactions.addInteractable(active);

    const InteractionFocus focus = interactions.updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
    Expect(focus.hasFocus,
        "TestInteractionFocusIgnoresDisabledAndConsumedInteractables",
        "An active interactable should still receive focus.");
    Expect(focus.interactableId == activeId,
        "TestInteractionFocusIgnoresDisabledAndConsumedInteractables",
        "Disabled and consumed interactables should not win focus.");
}

void TestInteractPressedTriggersExactlyOnceWhenHeld()
{
    InteractionSystem interactions;

    Interactable toggle;
    toggle.name = "Test Button";
    toggle.prompt = "Toggle Test Button";
    toggle.position = {0.0f, 0.5f, 1.0f};
    toggle.radius = 2.0f;
    toggle.type = InteractableType::Toggle;
    const int toggleId = interactions.addInteractable(toggle);
    interactions.updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});

    engine::InputState input;
    input.interactHeld = true;
    input.interactPressed = true;
    const InteractionResult first = interactions.interact(input);
    Expect(first.triggered,
        "TestInteractPressedTriggersExactlyOnceWhenHeld",
        "Pressed edge should trigger the focused interactable.");

    input.interactPressed = false;
    const InteractionResult held = interactions.interact(input);
    Expect(!held.triggered,
        "TestInteractPressedTriggersExactlyOnceWhenHeld",
        "Holding interact without a new pressed edge should not trigger again.");

    const Interactable* state = interactions.interactableById(toggleId);
    Expect(state != nullptr && state->toggled,
        "TestInteractPressedTriggersExactlyOnceWhenHeld",
        "The toggle should remain in the first triggered state.");
}

void TestOneShotPickupConsumesAndDoesNotTriggerAgain()
{
    InteractionSystem interactions;

    Interactable pickup;
    pickup.name = "One Shot Pickup";
    pickup.prompt = "Pick up One Shot Pickup";
    pickup.position = {0.0f, 0.5f, 1.0f};
    pickup.radius = 2.0f;
    pickup.type = InteractableType::Pickup;
    pickup.oneShot = true;
    const int pickupId = interactions.addInteractable(pickup);
    interactions.updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});

    engine::InputState input;
    input.interactPressed = true;
    const InteractionResult first = interactions.interact(input);
    const Interactable* state = interactions.interactableById(pickupId);
    Expect(first.triggered && state != nullptr && state->consumed && !state->enabled,
        "TestOneShotPickupConsumesAndDoesNotTriggerAgain",
        "A one-shot pickup should be consumed and disabled after the first interaction.");

    const InteractionFocus focusAfterPickup = interactions.updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
    const InteractionResult second = interactions.interact(input);
    Expect(!focusAfterPickup.hasFocus && !second.triggered,
        "TestOneShotPickupConsumesAndDoesNotTriggerAgain",
        "A consumed pickup should not regain focus or trigger again.");
}

void TestToggleInteractableChangesStateOnEachPress()
{
    InteractionSystem interactions;

    Interactable toggle;
    toggle.name = "Door Button";
    toggle.prompt = "Toggle Door Button";
    toggle.position = {0.0f, 0.5f, 1.0f};
    toggle.radius = 2.0f;
    toggle.type = InteractableType::Toggle;
    const int toggleId = interactions.addInteractable(toggle);
    interactions.updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});

    engine::InputState input;
    input.interactPressed = true;
    const InteractionResult opened = interactions.interact(input);
    const Interactable* state = interactions.interactableById(toggleId);
    Expect(opened.triggered && state != nullptr && state->toggled,
        "TestToggleInteractableChangesStateOnEachPress",
        "First press should toggle the interactable on.");

    const InteractionResult closed = interactions.interact(input);
    state = interactions.interactableById(toggleId);
    Expect(closed.triggered && state != nullptr && !state->toggled,
        "TestToggleInteractableChangesStateOnEachPress",
        "A later pressed edge should toggle the interactable off.");
}

void TestNoFocusedInteractableMeansNoAction()
{
    InteractionSystem interactions;

    Interactable info;
    info.name = "Distant Info";
    info.prompt = "Read Distant Info";
    info.position = {0.0f, 0.5f, 10.0f};
    info.radius = 1.0f;
    info.type = InteractableType::Info;
    interactions.addInteractable(info);
    interactions.updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});

    engine::InputState input;
    input.interactPressed = true;
    const InteractionResult result = interactions.interact(input);
    Expect(!result.triggered,
        "TestNoFocusedInteractableMeansNoAction",
        "Interact should no-op when there is no focused candidate.");
}

void TestFerryOfficeDataDefinesStableNamesAndPositions()
{
    Expect(FerryOffice::Names::FerryManifest == "Ferry Manifest",
        "TestFerryOfficeDataDefinesStableNamesAndPositions",
        "Ferry Manifest name should be centralized for scene mappings and tests.");
    Expect(FerryOffice::Names::ServiceGateCollider == "service-gate",
        "TestFerryOfficeDataDefinesStableNamesAndPositions",
        "Service gate collider name should be centralized for collision state mapping.");
    Expect(FerryOffice::Positions::ServiceVaultStart.x == 2.8f && FerryOffice::Positions::ServiceVaultStart.z == -0.35f,
        "TestFerryOfficeDataDefinesStableNamesAndPositions",
        "Service vault start should preserve the v0.7.1 accessible-side placement.");
    Expect(FerryOffice::Positions::MaintenanceBox.z == 1.9f,
        "TestFerryOfficeDataDefinesStableNamesAndPositions",
        "Maintenance box position should preserve the v0.7.1 post-vault placement.");
}

void TestWorldStateSetsAndReadsFlags()
{
    WorldState state;
    Expect(!state.isFlagSet(WorldFlag::PowerRestored),
        "TestWorldStateSetsAndReadsFlags",
        "Power should start unrestored.");

    const bool changed = state.setFlag(WorldFlag::PowerRestored, true, "Maintenance Box");
    Expect(changed,
        "TestWorldStateSetsAndReadsFlags",
        "Setting a changed flag should report a change.");
    Expect(state.isFlagSet(WorldFlag::PowerRestored),
        "TestWorldStateSetsAndReadsFlags",
        "Power restored flag should be readable after setting it.");
    Expect(state.eventCount() == 1,
        "TestWorldStateSetsAndReadsFlags",
        "A changed flag should record one event.");
    Expect(state.lastEvent() != nullptr && state.lastEvent()->id == 1,
        "TestWorldStateSetsAndReadsFlags",
        "First world event should have id 1.");
    Expect(state.debugSummary().find("powerRestored=true") != std::string::npos,
        "TestWorldStateSetsAndReadsFlags",
        "Debug summary should include the power flag.");
}

void TestWorldStateExposesFirstJobFlags()
{
    WorldState state;

    Expect(!state.isFlagSet(WorldFlag::FerryOfficeJobStarted),
        "TestWorldStateExposesFirstJobFlags",
        "The first driver/fixer job should start inactive.");
    Expect(state.setFlag(WorldFlag::FerryOfficeJobStarted, true, "Ferry Manifest"),
        "TestWorldStateExposesFirstJobFlags",
        "Starting the job should record a remembered event.");
    Expect(state.setFlag(WorldFlag::ServiceVehicleUsed, true, "Service Yard Vehicle"),
        "TestWorldStateExposesFirstJobFlags",
        "Using the service vehicle should record a remembered event.");
    Expect(state.setFlag(WorldFlag::DockRoadReached, true, "Service Run Checkpoint"),
        "TestWorldStateExposesFirstJobFlags",
        "Reaching the dock road checkpoint should record a remembered event.");
    Expect(state.setFlag(WorldFlag::ServiceRunConfirmed, true, "Service Run Marker"),
        "TestWorldStateExposesFirstJobFlags",
        "Confirming the service run should record a remembered event.");
    Expect(state.setFlag(WorldFlag::FerryOfficeJobComplete, true, "Ferry Office Service Call"),
        "TestWorldStateExposesFirstJobFlags",
        "Completing the first job should record a remembered event.");
    Expect(state.debugSummary().find("ferryOfficeJobComplete=true") != std::string::npos,
        "TestWorldStateExposesFirstJobFlags",
        "Debug summary should expose first-job completion.");
}

void TestWorldStateRecordsEventsInOrder()
{
    WorldState state;
    state.setFlag(WorldFlag::ManifestCollected, true, "Ferry Manifest");
    state.setFlag(WorldFlag::RouteOpened, true, "Wall Button");

    const std::vector<WorldEvent>& events = state.events();
    Expect(events.size() == 2,
        "TestWorldStateRecordsEventsInOrder",
        "Two changed flags should record two events.");
    Expect(events.size() >= 2 && events[0].id == 1 && events[1].id == 2,
        "TestWorldStateRecordsEventsInOrder",
        "Event ids should be assigned in deterministic order.");
    Expect(events.size() >= 2 && events[0].flag == WorldFlag::ManifestCollected && events[1].flag == WorldFlag::RouteOpened,
        "TestWorldStateRecordsEventsInOrder",
        "Event records should preserve flag order.");
    Expect(state.lastEventText().find("routeOpened=true") != std::string::npos,
        "TestWorldStateRecordsEventsInOrder",
        "Last event text should describe the latest changed flag.");
}

void TestWorldStateIgnoresRepeatedSameFlagValue()
{
    WorldState state;
    const bool first = state.setFlag(WorldFlag::ServiceRouteUsed, true, "Service Barrier Vault");
    const bool repeated = state.setFlag(WorldFlag::ServiceRouteUsed, true, "Service Barrier Vault");
    const bool reset = state.setFlag(WorldFlag::ServiceRouteUsed, false, "Debug Reset");

    Expect(first,
        "TestWorldStateIgnoresRepeatedSameFlagValue",
        "First flag change should be recorded.");
    Expect(!repeated,
        "TestWorldStateIgnoresRepeatedSameFlagValue",
        "Setting the same value twice should not record another remembered event.");
    Expect(reset,
        "TestWorldStateIgnoresRepeatedSameFlagValue",
        "Changing a flag back should be recorded.");
    Expect(state.eventCount() == 2,
        "TestWorldStateIgnoresRepeatedSameFlagValue",
        "Only actual value changes should create events.");
}

void TestOneShotManifestPickupUpdatesWorldStateOnce()
{
    PrototypeScene scene;
    scene.interactions().updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});

    engine::InputState input;
    input.interactPressed = true;
    const InteractionResult first = scene.interactions().interact(input);
    const bool firstChangedState = scene.applyInteractionResult(first);
    const std::size_t eventCountAfterFirst = scene.worldState().eventCount();

    scene.interactions().updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
    const InteractionResult second = scene.interactions().interact(input);
    const bool secondChangedState = scene.applyInteractionResult(second);

    Expect(first.triggered && firstChangedState,
        "TestOneShotManifestPickupUpdatesWorldStateOnce",
        "Manifest pickup should trigger and update remembered state.");
    Expect(scene.worldState().isFlagSet(WorldFlag::ManifestCollected),
        "TestOneShotManifestPickupUpdatesWorldStateOnce",
        "Manifest pickup should set manifestCollected.");
    Expect(!second.triggered && !secondChangedState,
        "TestOneShotManifestPickupUpdatesWorldStateOnce",
        "Consumed one-shot pickup should not update remembered state again.");
    Expect(scene.worldState().eventCount() == eventCountAfterFirst,
        "TestOneShotManifestPickupUpdatesWorldStateOnce",
        "One-shot pickup should only record one world event.");
}

void TestMaintenanceInteractionRestoresPower()
{
    PrototypeScene scene;
    InteractionResult result;
    result.triggered = true;
    result.name = FerryOffice::Names::MaintenanceBox;
    result.type = InteractableType::Info;

    const bool changedState = scene.applyInteractionResult(result);
    Expect(changedState,
        "TestMaintenanceInteractionRestoresPower",
        "Maintenance box interaction should update remembered state.");
    Expect(scene.worldState().isFlagSet(WorldFlag::MaintenanceBoxInspected),
        "TestMaintenanceInteractionRestoresPower",
        "Maintenance box interaction should record inspection.");
    Expect(scene.worldState().isFlagSet(WorldFlag::PowerRestored),
        "TestMaintenanceInteractionRestoresPower",
        "Maintenance box interaction should restore power.");
    Expect(scene.worldState().eventCount() == 2,
        "TestMaintenanceInteractionRestoresPower",
        "Maintenance interaction should record inspection and power events.");
}

void TestWallButtonOpensRouteWithoutClosingItAgain()
{
    PrototypeScene scene;
    InteractionResult opened;
    opened.triggered = true;
    opened.name = FerryOffice::Names::WallButton;
    opened.type = InteractableType::Toggle;
    opened.toggled = true;

    InteractionResult closed = opened;
    closed.toggled = false;

    const bool openedChanged = scene.applyInteractionResult(opened);
    const bool closedChanged = scene.applyInteractionResult(closed);

    Expect(openedChanged,
        "TestWallButtonOpensRouteWithoutClosingItAgain",
        "First wall button interaction should open the remembered route.");
    Expect(!closedChanged,
        "TestWallButtonOpensRouteWithoutClosingItAgain",
        "A later wall button interaction should not close the route and risk trapping the player.");
    Expect(scene.worldState().isFlagSet(WorldFlag::RouteOpened),
        "TestWallButtonOpensRouteWithoutClosingItAgain",
        "The route should remain open after repeated wall button interactions.");
    Expect(scene.worldState().eventCount() == 1,
        "TestWallButtonOpensRouteWithoutClosingItAgain",
        "Repeated wall button interactions should not duplicate route-open events.");
}

void TestTraversalCompletionRecordsServiceRouteUsed()
{
    PrototypeScene scene;
    const bool first = scene.recordServiceRouteUsed();
    const bool repeated = scene.recordServiceRouteUsed();

    Expect(first,
        "TestTraversalCompletionRecordsServiceRouteUsed",
        "First traversal completion should update remembered state.");
    Expect(scene.worldState().isFlagSet(WorldFlag::ServiceRouteUsed),
        "TestTraversalCompletionRecordsServiceRouteUsed",
        "Traversal completion should set serviceRouteUsed.");
    Expect(!repeated,
        "TestTraversalCompletionRecordsServiceRouteUsed",
        "Repeated traversal completion should not duplicate the remembered event.");
    Expect(scene.worldState().eventCount() == 1,
        "TestTraversalCompletionRecordsServiceRouteUsed",
        "Repeated traversal completion should keep one service route event.");
}

void TestAuthoredTraversalCompletionBindingsControlWorldFlags()
{
    SceneDefinition sceneDefinition;
    sceneDefinition.id = "traversal-binding-test";

    SceneTraversalAffordanceDefinition unbound;
    unbound.id = "unbound-vault";
    unbound.name = "Unbound Vault";
    unbound.prompt = "Vault Unbound";
    unbound.type = "vault";
    unbound.startPosition = {0.0f, 0.0f, 0.0f};
    unbound.endPosition = {1.0f, 0.0f, 0.0f};
    unbound.focusRadius = 1.0f;
    unbound.requiredFacingDirection = {1.0f, 0.0f, 0.0f};
    unbound.requiredFacingDot = 0.1f;
    unbound.durationSeconds = 0.2f;
    sceneDefinition.traversalAffordances.push_back(unbound);

    SceneTraversalAffordanceDefinition bound = unbound;
    bound.id = "bound-vault";
    bound.name = "Bound Vault";
    bound.worldFlagsSetOnComplete = {"serviceRouteUsed"};
    sceneDefinition.traversalAffordances.push_back(bound);

    PrototypeScene scene(sceneDefinition);
    const int unboundId = scene.traversal().affordances()[0].id;
    const int boundId = scene.traversal().affordances()[1].id;

    const bool unboundChanged = scene.recordTraversalCompleted(unboundId);
    Expect(!unboundChanged && !scene.worldState().isFlagSet(WorldFlag::ServiceRouteUsed),
        "TestAuthoredTraversalCompletionBindingsControlWorldFlags",
        "An authored traversal without completion flags should not set serviceRouteUsed just because any traversal landed.");

    const bool boundChanged = scene.recordTraversalCompleted(boundId);
    Expect(boundChanged && scene.worldState().isFlagSet(WorldFlag::ServiceRouteUsed),
        "TestAuthoredTraversalCompletionBindingsControlWorldFlags",
        "An authored traversal completion binding should set its declared world flags.");
}

InteractionResult MakeSceneInteraction(std::string name, InteractableType type, bool toggled = false)
{
    InteractionResult result;
    result.triggered = true;
    result.name = std::move(name);
    result.type = type;
    result.toggled = toggled;
    return result;
}

void TestFerryOfficeSliceStartsIncomplete()
{
    PrototypeScene scene;

    Expect(!scene.isSliceReadyForExit(),
        "TestFerryOfficeSliceStartsIncomplete",
        "The Ferry Office slice should not start ready for the exit marker.");
    Expect(!scene.isSliceComplete(),
        "TestFerryOfficeSliceStartsIncomplete",
        "The Ferry Office slice should start incomplete.");
    Expect(!scene.worldState().isFlagSet(WorldFlag::ExitReached),
        "TestFerryOfficeSliceStartsIncomplete",
        "The exit marker should start unreached.");
    Expect(scene.currentObjectiveText().find("Ferry Manifest") != std::string::npos,
        "TestFerryOfficeSliceStartsIncomplete",
        "Initial objective should guide the player toward the Ferry Manifest.");
}

void TestFerryOfficeJobStartsIncompleteAndOrdersObjectives()
{
    FerryOfficeJob job;
    WorldState state;

    Expect(!job.isComplete(state),
        "TestFerryOfficeJobStartsIncompleteAndOrdersObjectives",
        "First driver/fixer job should start incomplete.");
    Expect(job.currentObjectiveText(state).find("manifest") != std::string::npos
            || job.currentObjectiveText(state).find("Manifest") != std::string::npos,
        "TestFerryOfficeJobStartsIncompleteAndOrdersObjectives",
        "Initial job objective should guide the player toward the manifest or office call.");

    job.recordJobStarted(state, "Ferry Manifest");
    state.setFlag(WorldFlag::ManifestCollected, true, "Ferry Manifest");
    Expect(job.currentObjectiveText(state).find("Service Barrier") != std::string::npos,
        "TestFerryOfficeJobStartsIncompleteAndOrdersObjectives",
        "After manifest collection, the job should guide the player toward the service route.");

    state.setFlag(WorldFlag::ServiceRouteUsed, true, "Service Barrier Vault");
    Expect(job.currentObjectiveText(state).find("Maintenance Box") != std::string::npos,
        "TestFerryOfficeJobStartsIncompleteAndOrdersObjectives",
        "After using the service route, the job should ask for maintenance/power work.");

    state.setFlag(WorldFlag::MaintenanceBoxInspected, true, "Maintenance Box");
    state.setFlag(WorldFlag::PowerRestored, true, "Maintenance Box");
    Expect(job.currentObjectiveText(state).find("service gate") != std::string::npos
            || job.currentObjectiveText(state).find("Wall Button") != std::string::npos,
        "TestFerryOfficeJobStartsIncompleteAndOrdersObjectives",
        "After power is restored, the job should ask the player to open the route.");

    state.setFlag(WorldFlag::RouteOpened, true, "Wall Button");
    Expect(job.currentObjectiveText(state).find("vehicle") != std::string::npos
            || job.currentObjectiveText(state).find("Vehicle") != std::string::npos,
        "TestFerryOfficeJobStartsIncompleteAndOrdersObjectives",
        "After the route opens, the job should ask the player to use the service vehicle.");
}

void TestFerryOfficeJobVehicleCheckpointRequiresOccupiedVehicle()
{
    FerryOfficeJob job;
    WorldState state;
    FerryOfficeJobConfig config;
    config.vehicleCheckpointPosition = {19.35f, 0.08f, -2.2f};
    config.vehicleCheckpointRadius = 1.25f;
    job.configure(config);

    const bool ignoredOnFoot = job.updateVehicleCheckpoint(state, {19.35f, 0.0f, -2.2f}, false);
    const bool reachedInVehicle = job.updateVehicleCheckpoint(state, {19.0f, 0.0f, -2.2f}, true);
    const bool repeated = job.updateVehicleCheckpoint(state, {19.0f, 0.0f, -2.2f}, true);

    Expect(!ignoredOnFoot,
        "TestFerryOfficeJobVehicleCheckpointRequiresOccupiedVehicle",
        "Dock road checkpoint should require the player to be in the vehicle.");
    Expect(reachedInVehicle,
        "TestFerryOfficeJobVehicleCheckpointRequiresOccupiedVehicle",
        "Driving into the service checkpoint should update job world state.");
    Expect(state.isFlagSet(WorldFlag::DockRoadReached),
        "TestFerryOfficeJobVehicleCheckpointRequiresOccupiedVehicle",
        "Vehicle checkpoint should set dockRoadReached.");
    Expect(!repeated && state.eventCount() == 1,
        "TestFerryOfficeJobVehicleCheckpointRequiresOccupiedVehicle",
        "Repeated vehicle checkpoint overlap should not duplicate events.");
}

void TestFerryOfficeJobCompletionRequiresAllJobFlags()
{
    FerryOfficeJob job;
    WorldState state;

    job.recordJobStarted(state, "Ferry Manifest");
    state.setFlag(WorldFlag::ManifestCollected, true, "Ferry Manifest");
    state.setFlag(WorldFlag::ServiceRouteUsed, true, "Service Barrier Vault");
    state.setFlag(WorldFlag::MaintenanceBoxInspected, true, "Maintenance Box");
    state.setFlag(WorldFlag::PowerRestored, true, "Maintenance Box");
    state.setFlag(WorldFlag::RouteOpened, true, "Wall Button");
    job.recordServiceVehicleUsed(state);
    state.setFlag(WorldFlag::DockRoadReached, true, "Service Run Checkpoint");

    const bool earlyComplete = job.confirmServiceRun(state, "Service Run Marker");
    Expect(earlyComplete,
        "TestFerryOfficeJobCompletionRequiresAllJobFlags",
        "Confirming at the service marker after all required steps should complete the job.");
    Expect(state.isFlagSet(WorldFlag::ServiceRunConfirmed),
        "TestFerryOfficeJobCompletionRequiresAllJobFlags",
        "Confirmation should set serviceRunConfirmed.");
    Expect(state.isFlagSet(WorldFlag::FerryOfficeJobComplete),
        "TestFerryOfficeJobCompletionRequiresAllJobFlags",
        "Confirmation should set ferryOfficeJobComplete.");

    const std::size_t eventCount = state.eventCount();
    const bool repeatedComplete = job.confirmServiceRun(state, "Service Run Marker");
    Expect(!repeatedComplete && state.eventCount() == eventCount,
        "TestFerryOfficeJobCompletionRequiresAllJobFlags",
        "Repeating a completed service run should not duplicate job events.");
}

void TestFerryOfficeFollowupStatusSummarizesEndpointChain()
{
    WorldState state;

    Expect(FerryOfficeFollowupStatusText(state).find("relay=later") != std::string::npos,
        "TestFerryOfficeFollowupStatusSummarizesEndpointChain",
        "Follow-up status should tell playtest mode the relay beat is still ahead.");

    state.setFlag(WorldFlag::DockRoadRelayReset, true, "Dock Road Relay");
    Expect(FerryOfficeFollowupStatusText(state).find("relay=reset") != std::string::npos
            && FerryOfficeFollowupStatusText(state).find("log=later") != std::string::npos,
        "TestFerryOfficeFollowupStatusSummarizesEndpointChain",
        "Follow-up status should show relay reset progress before the log beat.");

    state.setFlag(WorldFlag::DockRoadRelayLogged, true, "Relay Service Log");
    Expect(FerryOfficeFollowupStatusText(state).find("log=signed") != std::string::npos
            && FerryOfficeFollowupStatusText(state).find("road=later") != std::string::npos,
        "TestFerryOfficeFollowupStatusSummarizesEndpointChain",
        "Follow-up status should show relay log progress before the road clear tag.");

    state.setFlag(WorldFlag::DockRoadClearanceTagged, true, "Dock Road Clearance Tag");
    Expect(FerryOfficeFollowupStatusText(state).find("road=clear") != std::string::npos
            && FerryOfficeFollowupStatusText(state).find("parts=later") != std::string::npos,
        "TestFerryOfficeFollowupStatusSummarizesEndpointChain",
        "Follow-up status should show when the endpoint chain leaves the road clear.");

    state.setFlag(WorldFlag::HarborPartsPickedUp, true, "Harbor Parts Crate");
    Expect(FerryOfficeFollowupStatusText(state).find("parts=picked") != std::string::npos,
        "TestFerryOfficeFollowupStatusSummarizesEndpointChain",
        "Follow-up status should show when the Harbor Parts crate is being carried.");

    state.setFlag(WorldFlag::HarborPartsDelivered, true, "Ferry Office Parts Shelf");
    Expect(FerryOfficeFollowupStatusText(state).find("parts=delivered") != std::string::npos
            && FerryOfficeFollowupStatusText(state).find("board=later") != std::string::npos,
        "TestFerryOfficeFollowupStatusSummarizesEndpointChain",
        "Follow-up status should show when the Harbor Parts crate has been delivered before board signoff.");

    state.setFlag(WorldFlag::FerryOfficeBoardUpdated, true, "Ferry Office Work Board");
    Expect(FerryOfficeFollowupStatusText(state).find("board=updated") != std::string::npos
            && FerryOfficeFollowupStatusText(state).find("handoff=later") != std::string::npos,
        "TestFerryOfficeFollowupStatusSummarizesEndpointChain",
        "Follow-up status should show when the Ferry Office work board has been updated.");

    state.setFlag(WorldFlag::FerryOfficeHandoffFiled, true, "Ferry Office Handoff Note");
    Expect(FerryOfficeFollowupStatusText(state).find("handoff=filed") != std::string::npos,
        "TestFerryOfficeFollowupStatusSummarizesEndpointChain",
        "Follow-up status should show when the Ferry Office handoff note has been filed.");

    state.setFlag(WorldFlag::StormPumpReset, true, "Storm Pump Switch");
    state.setFlag(WorldFlag::StormPumpTicketClosed, true, "Storm Pump Ticket");
    Expect(FerryOfficeFollowupStatusText(state).find("pump=closed") != std::string::npos
            && FerryOfficeFollowupStatusText(state).find("drain=later") != std::string::npos,
        "TestFerryOfficeFollowupStatusSummarizesEndpointChain",
        "Follow-up status should show that the low dock drain remains after the pump ticket closes.");

    state.setFlag(WorldFlag::LowDockDrainCleared, true, "Low Dock Drain Marker");
    Expect(FerryOfficeFollowupStatusText(state).find("drain=clear") != std::string::npos
            && FerryOfficeFollowupStatusText(state).find("logbook=later") != std::string::npos,
        "TestFerryOfficeFollowupStatusSummarizesEndpointChain",
        "Follow-up status should show when the low dock drain has been tagged clear before the office log closeout.");

    WorldFlag drainLogFlag = WorldFlag::PowerRestored;
    Expect(TryWorldFlagFromName("lowDockDrainLogged", drainLogFlag),
        "TestFerryOfficeFollowupStatusSummarizesEndpointChain",
        "The drain log closeout should have a named world flag.");
    state.setFlag(drainLogFlag, true, "Ferry Office Drain Log");
    Expect(FerryOfficeFollowupStatusText(state).find("logbook=signed") != std::string::npos,
        "TestFerryOfficeFollowupStatusSummarizesEndpointChain",
        "Follow-up status should show when the office drain log has been signed.");
}

void TestFerryOfficeFollowupNextStepGuidesLongChain()
{
    WorldState state;

    Expect(FerryOfficeFollowupNextStepText(state).empty(),
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "Follow-up next-step text should stay hidden before the service call is complete.");
    Expect(FerryOfficeActiveRouteMarkerId(state, FerryOfficeJobPhase::CollectManifest).empty(),
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "Route guidance should stay hidden before the Ferry Office job starts.");

    state.setFlag(WorldFlag::FerryOfficeJobComplete, true, "Ferry Office Service Call");
    Expect(FerryOfficeFollowupNextStepText(state).find("Dock Road Relay") != std::string::npos,
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "The first follow-up next step should point to the relay.");
    Expect(FerryOfficeActiveRouteMarkerId(state, FerryOfficeJobPhase::Complete) == "route-service-confirm-to-relay",
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "Follow-up route guidance should send the player from service confirmation to the relay.");
    Expect(FerryOfficeActiveObjectiveMarkerId(state, FerryOfficeJobPhase::Complete) == "dock-road-relay-marker",
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "Follow-up objective guidance should highlight the relay marker.");

    state.setFlag(WorldFlag::DockRoadRelayReset, true, "Dock Road Relay");
    Expect(FerryOfficeFollowupNextStepText(state).find("Relay Service Log") != std::string::npos,
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "After the relay reset, the next step should point to the log.");
    Expect(FerryOfficeActiveRouteMarkerId(state, FerryOfficeJobPhase::Complete) == "route-relay-to-service-log",
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "Follow-up route guidance should send the player from relay to log.");

    state.setFlag(WorldFlag::DockRoadRelayLogged, true, "Relay Service Log");
    Expect(FerryOfficeFollowupNextStepText(state).find("clear") != std::string::npos,
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "After the log, the next step should point to the clear tag.");
    Expect(FerryOfficeActiveRouteMarkerId(state, FerryOfficeJobPhase::Complete) == "route-service-log-to-clearance-tag",
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "Follow-up route guidance should send the player from service log to clearance tag.");

    state.setFlag(WorldFlag::DockRoadClearanceTagged, true, "Dock Road Clearance Tag");
    Expect(FerryOfficeFollowupNextStepText(state).find("Harbor Parts") != std::string::npos,
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "After clearance, the next step should point to the Harbor Parts crate.");
    Expect(FerryOfficeActiveRouteMarkerId(state, FerryOfficeJobPhase::Complete) == "route-clearance-tag-to-harbor-parts",
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "Follow-up route guidance should send the player from clearance tag to harbor parts.");

    state.setFlag(WorldFlag::HarborPartsPickedUp, true, "Harbor Parts Crate");
    Expect(FerryOfficeFollowupNextStepText(state).find("Ferry Office") != std::string::npos,
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "After pickup, the next step should point back to the Ferry Office shelf.");
    Expect(FerryOfficeActiveRouteMarkerId(state, FerryOfficeJobPhase::Complete) == "route-harbor-parts-to-office-shelf",
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "Follow-up route guidance should send the player from harbor parts back to the office shelf.");

    state.setFlag(WorldFlag::HarborPartsDelivered, true, "Ferry Office Parts Shelf");
    Expect(FerryOfficeFollowupNextStepText(state).find("Work Board") != std::string::npos,
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "After delivery, the next step should point to the Ferry Office Work Board.");
    Expect(FerryOfficeActiveRouteMarkerId(state, FerryOfficeJobPhase::Complete) == "route-parts-shelf-to-work-board",
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "Follow-up route guidance should send the player from parts shelf to work board.");

    state.setFlag(WorldFlag::FerryOfficeBoardUpdated, true, "Ferry Office Work Board");
    Expect(FerryOfficeFollowupNextStepText(state).find("handoff") != std::string::npos,
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "After work-board signoff, the next step should point to the handoff note.");
    Expect(FerryOfficeActiveRouteMarkerId(state, FerryOfficeJobPhase::Complete) == "route-work-board-to-handoff-note",
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "Follow-up route guidance should send the player from work board to handoff note.");

    state.setFlag(WorldFlag::FerryOfficeHandoffFiled, true, "Ferry Office Handoff Note");
    Expect(FerryOfficeFollowupNextStepText(state).find("storm pump") != std::string::npos,
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "After the handoff note, the next step should point to the storm pump.");
    Expect(FerryOfficeActiveRouteMarkerId(state, FerryOfficeJobPhase::Complete) == "route-handoff-note-to-storm-pump",
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "Follow-up route guidance should send the player from handoff note to storm pump.");

    state.setFlag(WorldFlag::StormPumpReset, true, "Storm Pump Switch");
    Expect(FerryOfficeFollowupNextStepText(state).find("ticket") != std::string::npos,
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "After the storm pump reset, the next step should point to the office ticket.");
    Expect(FerryOfficeActiveRouteMarkerId(state, FerryOfficeJobPhase::Complete) == "route-storm-pump-to-ticket",
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "Follow-up route guidance should send the player from storm pump back to ticket.");

    state.setFlag(WorldFlag::StormPumpTicketClosed, true, "Storm Pump Ticket");
    Expect(FerryOfficeFollowupNextStepText(state).find("low dock drain") != std::string::npos,
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "After the storm pump ticket, the next step should point to the low dock drain.");
    Expect(FerryOfficeActiveRouteMarkerId(state, FerryOfficeJobPhase::Complete) == "route-storm-ticket-to-low-dock-drain",
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "Follow-up route guidance should send the player from pump ticket to low dock drain.");

    state.setFlag(WorldFlag::LowDockDrainCleared, true, "Low Dock Drain Marker");
    Expect(FerryOfficeFollowupNextStepText(state).find("Drain Log") != std::string::npos,
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "After the low dock drain is clear, the next step should send the player back to the Ferry Office drain log.");
    Expect(FerryOfficeActiveRouteMarkerId(state, FerryOfficeJobPhase::Complete) == "route-low-dock-drain-to-office-log",
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "Follow-up route guidance should send the player from low dock drain back to the office log.");
    Expect(FerryOfficeActiveObjectiveMarkerId(state, FerryOfficeJobPhase::Complete) == "ferry-office-drain-log-marker",
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "Follow-up objective guidance should highlight the Ferry Office drain log marker.");

    WorldFlag drainLogFlag = WorldFlag::PowerRestored;
    Expect(TryWorldFlagFromName("lowDockDrainLogged", drainLogFlag),
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "The drain log closeout should have a named world flag.");
    state.setFlag(drainLogFlag, true, "Ferry Office Drain Log");
    Expect(FerryOfficeFollowupNextStepText(state).find("complete") != std::string::npos,
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "After the office drain log is signed, the next step should acknowledge follow-up completion.");
    Expect(FerryOfficeActiveRouteMarkerId(state, FerryOfficeJobPhase::Complete).empty(),
        "TestFerryOfficeFollowupNextStepGuidesLongChain",
        "Follow-up route guidance should hide itself after the long chain is complete.");
}

void TestFerryOfficeObjectiveTextGuidesRouteSteps()
{
    PrototypeScene scene;

    Expect(scene.currentObjectiveText().find("Ferry Manifest") != std::string::npos,
        "TestFerryOfficeObjectiveTextGuidesRouteSteps",
        "The first objective should call out the Ferry Manifest.");

    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::FerryManifest), InteractableType::Pickup));
    Expect(scene.currentObjectiveText().find("right-side") != std::string::npos
            && scene.currentObjectiveText().find("Maintenance Box") != std::string::npos,
        "TestFerryOfficeObjectiveTextGuidesRouteSteps",
        "After the manifest, the objective should spatially guide the player toward the right-side service vault and maintenance box.");

    scene.recordServiceRouteUsed();
    Expect(scene.currentObjectiveText().find("Maintenance Box") != std::string::npos
            && scene.currentObjectiveText().find("restore power") != std::string::npos,
        "TestFerryOfficeObjectiveTextGuidesRouteSteps",
        "After traversal, the objective should guide the player to restore power at the Maintenance Box.");

    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::MaintenanceBox), InteractableType::Info));
    Expect(scene.currentObjectiveText().find("Wall Button") != std::string::npos
            && scene.currentObjectiveText().find("service gate") != std::string::npos,
        "TestFerryOfficeObjectiveTextGuidesRouteSteps",
        "After maintenance, the objective should guide the player back to the wall button and service gate.");

    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::WallButton), InteractableType::Info));
    Expect(scene.currentObjectiveText().find("open service gate") != std::string::npos
            && scene.currentObjectiveText().find("Exit Summary Marker") != std::string::npos,
        "TestFerryOfficeObjectiveTextGuidesRouteSteps",
        "After opening the route, the objective should guide the player through the gate to the exit marker.");
}

void TestFerryOfficeCompletionRequiresRememberedLoop()
{
    PrototypeScene scene;

    const bool earlyExit = scene.recordExitReached();
    Expect(!earlyExit && !scene.worldState().isFlagSet(WorldFlag::ExitReached),
        "TestFerryOfficeCompletionRequiresRememberedLoop",
        "The exit marker should not complete the slice before required remembered flags are set.");

    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::FerryManifest), InteractableType::Pickup));
    scene.recordServiceRouteUsed();
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::MaintenanceBox), InteractableType::Info));
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::WallButton), InteractableType::Toggle, true));

    Expect(scene.isSliceReadyForExit(),
        "TestFerryOfficeCompletionRequiresRememberedLoop",
        "Manifest, service route, maintenance, power, and route flags should make the slice ready for exit.");
    Expect(!scene.isSliceComplete(),
        "TestFerryOfficeCompletionRequiresRememberedLoop",
        "The slice should still require the exit marker before completion.");

    const bool exitChanged = scene.recordExitReached();
    Expect(exitChanged,
        "TestFerryOfficeCompletionRequiresRememberedLoop",
        "Reaching the exit marker after required flags should record exitReached.");
    Expect(scene.isSliceComplete(),
        "TestFerryOfficeCompletionRequiresRememberedLoop",
        "All required remembered flags plus exitReached should complete the slice.");
    Expect(scene.completionSummary().find("complete=true") != std::string::npos,
        "TestFerryOfficeCompletionRequiresRememberedLoop",
        "Completion summary should expose the completed state for debug text.");
}

void TestFerryOfficeExitMarkerRequiresReadyState()
{
    PrototypeScene scene;
    const bool blocked = scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::ExitMarker), InteractableType::Info));

    Expect(!blocked,
        "TestFerryOfficeExitMarkerRequiresReadyState",
        "Exit marker interaction should not change remembered state before the loop is ready.");
    Expect(!scene.worldState().isFlagSet(WorldFlag::ExitReached),
        "TestFerryOfficeExitMarkerRequiresReadyState",
        "Exit reached should remain false before the required loop is complete.");

    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::FerryManifest), InteractableType::Pickup));
    scene.recordServiceRouteUsed();
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::MaintenanceBox), InteractableType::Info));
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::WallButton), InteractableType::Toggle, true));
    const bool completed = scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::ExitMarker), InteractableType::Info));

    Expect(completed,
        "TestFerryOfficeExitMarkerRequiresReadyState",
        "Exit marker interaction should record exitReached once the required loop is ready.");
    Expect(scene.worldState().isFlagSet(WorldFlag::ExitReached),
        "TestFerryOfficeExitMarkerRequiresReadyState",
        "Exit reached flag should be set after a valid exit marker interaction.");
}

void TestWallButtonOpenLatchesServiceGateColliderState()
{
    PrototypeScene scene;

    Expect(scene.isServiceGateBlocking(),
        "TestWallButtonOpenLatchesServiceGateColliderState",
        "The service gate collider should block the route before the route is opened.");

    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::WallButton), InteractableType::Toggle, true));
    Expect(!scene.isServiceGateBlocking(),
        "TestWallButtonOpenLatchesServiceGateColliderState",
        "Opening the route should disable the service gate blocking collider.");

    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::WallButton), InteractableType::Toggle, false));
    Expect(!scene.isServiceGateBlocking(),
        "TestWallButtonOpenLatchesServiceGateColliderState",
        "Repeated wall button interactions should leave the service gate open.");
}

void TestStormPumpTicketOpensLowDockDrainAccessBarrier()
{
    const SceneLoadResult result = LoadSceneDefinition(DefaultScenePathForTests());
    PrototypeScene scene;
    scene.loadFromDefinition(result.scene);

    const StaticCollider* barrierBefore = scene.world().colliderByName("low-dock-drain-flood-barrier");
    Expect(barrierBefore != nullptr && barrierBefore->blocksPlayer,
        "TestStormPumpTicketOpensLowDockDrainAccessBarrier",
        "Low dock drain access should be blocked before the storm pump ticket is closed.");

    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::FerryManifest), InteractableType::Pickup));
    scene.recordServiceRouteUsed();
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::MaintenanceBox), InteractableType::Info));
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::WallButton), InteractableType::Info));
    scene.recordServiceVehicleUsed();
    scene.updateJobVehicleCheckpoint({19.0f, 0.0f, -2.2f}, true);
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::ServiceRunMarker), InteractableType::Info));
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::DockRoadRelay), InteractableType::Info));
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::RelayServiceLog), InteractableType::Info));
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::DockRoadClearanceTag), InteractableType::Info));
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::HarborPartsCrate), InteractableType::Info));
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::FerryOfficePartsShelf), InteractableType::Info));
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::FerryOfficeWorkBoard), InteractableType::Info));
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::FerryOfficeHandoffNote), InteractableType::Info));
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::StormPumpSwitch), InteractableType::Info));

    const StaticCollider* barrierBeforeTicket = scene.world().colliderByName("low-dock-drain-flood-barrier");
    Expect(barrierBeforeTicket != nullptr && barrierBeforeTicket->blocksPlayer,
        "TestStormPumpTicketOpensLowDockDrainAccessBarrier",
        "Resetting the pump alone should not open the low dock drain access barrier.");

    const bool closedTicket = scene.applyInteractionResult(
        MakeSceneInteraction(std::string(FerryOffice::Names::StormPumpTicket), InteractableType::Info));
    const StaticCollider* barrierAfter = scene.world().colliderByName("low-dock-drain-flood-barrier");
    Expect(closedTicket && scene.worldState().isFlagSet(WorldFlag::StormPumpTicketClosed),
        "TestStormPumpTicketOpensLowDockDrainAccessBarrier",
        "Closing the storm pump ticket should set the remembered state that opens the low dock route.");
    Expect(barrierAfter != nullptr && !barrierAfter->blocksPlayer,
        "TestStormPumpTicketOpensLowDockDrainAccessBarrier",
        "Closing the storm pump ticket should make the low dock drain access barrier passable.");
}

void TestFerryOfficeServiceVaultFocusesFromAccessibleSide()
{
    PrototypeScene scene;

    const TraversalFocus focus = scene.traversal().updateFocus({2.8f, 0.0f, -0.35f}, {0.0f, 0.0f, 1.0f});
    Expect(focus.hasFocus,
        "TestFerryOfficeServiceVaultFocusesFromAccessibleSide",
        "The service vault should focus from the player-accessible side of the barrier.");
    Expect(focus.name == FerryOffice::Names::ServiceVault,
        "TestFerryOfficeServiceVaultFocusesFromAccessibleSide",
        "The focused traversal affordance should be the Service Barrier Vault.");
}

void TestMaintenanceBoxIsNotFocusedBeforeServiceVault()
{
    PrototypeScene scene;

    const InteractionFocus focus = scene.interactions().updateFocus({2.8f, 0.0f, -0.35f}, {0.0f, 0.0f, 1.0f});
    Expect(!focus.hasFocus || focus.name != FerryOffice::Names::MaintenanceBox,
        "TestMaintenanceBoxIsNotFocusedBeforeServiceVault",
        "Maintenance Box should not be the focused action before the service vault is crossed.");
}

void TestFerryOfficeLoopCanCompleteThroughSceneSystems()
{
    PrototypeScene scene;
    PlayerController player;
    player.setWorld(&scene.world());
    player.setPosition({2.8f, 0.0f, -0.35f});

    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::FerryManifest), InteractableType::Pickup));

    scene.traversal().updateFocus(player.state().position, {0.0f, 0.0f, 1.0f});
    engine::InputState traversalInput;
    traversalInput.jumpPressed = true;
    TraversalActivation activation = scene.traversal().activationFromInput(traversalInput);
    player.update(0.0f, traversalInput, 0.0f, &activation);

    traversalInput.jumpPressed = false;
    activation = {};
    bool recordedServiceRoute = false;
    for (int i = 0; i < 12; ++i) {
        player.update(0.05f, traversalInput, 0.0f, &activation);
        if (player.state().traversalLandedThisFrame) {
            recordedServiceRoute = scene.recordServiceRouteUsed();
        }
    }

    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::MaintenanceBox), InteractableType::Info));
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::WallButton), InteractableType::Info));

    Expect(recordedServiceRoute,
        "TestFerryOfficeLoopCanCompleteThroughSceneSystems",
        "The scripted scene-system loop should record serviceRouteUsed when traversal lands.");
    Expect(scene.isSliceReadyForExit(),
        "TestFerryOfficeLoopCanCompleteThroughSceneSystems",
        "The scene systems should make the Ferry Office slice ready for exit after manifest, vault, maintenance, and gate actions.");

    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::ExitMarker), InteractableType::Info));
    Expect(scene.isSliceComplete(),
        "TestFerryOfficeLoopCanCompleteThroughSceneSystems",
        "The Ferry Office slice should complete after the exit marker records the ready state.");
}

void TestDockRoadRelayRequiresCompletedServiceCall()
{
    PrototypeScene scene;

    const bool earlyRelay = scene.applyInteractionResult(
        MakeSceneInteraction(std::string(FerryOffice::Names::DockRoadRelay), InteractableType::Info));
    Expect(!earlyRelay && !scene.worldState().isFlagSet(WorldFlag::DockRoadRelayReset),
        "TestDockRoadRelayRequiresCompletedServiceCall",
        "The second relay beat should not reset before the first service call is complete.");

    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::FerryManifest), InteractableType::Pickup));
    scene.recordServiceRouteUsed();
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::MaintenanceBox), InteractableType::Info));
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::WallButton), InteractableType::Info));
    scene.recordServiceVehicleUsed();
    scene.updateJobVehicleCheckpoint(FerryOffice::Positions::ServiceRunCheckpoint, true);
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::ServiceRunMarker), InteractableType::Info));

    const bool relayReset = scene.applyInteractionResult(
        MakeSceneInteraction(std::string(FerryOffice::Names::DockRoadRelay), InteractableType::Info));
    Expect(relayReset,
        "TestDockRoadRelayRequiresCompletedServiceCall",
        "The second relay beat should reset after the first service call is complete.");
    Expect(scene.worldState().isFlagSet(WorldFlag::DockRoadRelayReset),
        "TestDockRoadRelayRequiresCompletedServiceCall",
        "Dock-road relay reset should be remembered in world state.");
    Expect(scene.currentJobObjectiveText().find("relay") != std::string::npos
            || scene.currentJobObjectiveText().find("Relay") != std::string::npos,
        "TestDockRoadRelayRequiresCompletedServiceCall",
        "The objective should acknowledge the relay follow-up beat.");

    const bool relayLogged = scene.applyInteractionResult(
        MakeSceneInteraction(std::string(FerryOffice::Names::RelayServiceLog), InteractableType::Info));
    Expect(relayLogged && scene.worldState().isFlagSet(WorldFlag::DockRoadRelayLogged),
        "TestDockRoadRelayRequiresCompletedServiceCall",
        "The relay service log should complete only after the dock-road relay has been reset.");

    const bool clearanceTagged = scene.applyInteractionResult(
        MakeSceneInteraction(std::string(FerryOffice::Names::DockRoadClearanceTag), InteractableType::Info));
    Expect(clearanceTagged && scene.worldState().isFlagSet(WorldFlag::DockRoadClearanceTagged),
        "TestDockRoadRelayRequiresCompletedServiceCall",
        "The dock-road clearance tag should complete only after the relay reset has been logged.");
    Expect(scene.currentJobObjectiveText().find("Harbor Parts") != std::string::npos,
        "TestDockRoadRelayRequiresCompletedServiceCall",
        "The objective should send the player into the Harbor Parts follow-up after dock-road clearance.");
}

void TestHarborPartsJobRequiresClearedDockRoad()
{
    PrototypeScene scene;

    const bool earlyPickup = scene.applyInteractionResult(
        MakeSceneInteraction(std::string(FerryOffice::Names::HarborPartsCrate), InteractableType::Info));
    Expect(!earlyPickup && !scene.worldState().isFlagSet(WorldFlag::HarborPartsPickedUp),
        "TestHarborPartsJobRequiresClearedDockRoad",
        "The harbor parts crate should not be available before dock-road clearance is tagged.");

    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::FerryManifest), InteractableType::Pickup));
    scene.recordServiceRouteUsed();
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::MaintenanceBox), InteractableType::Info));
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::WallButton), InteractableType::Info));
    scene.recordServiceVehicleUsed();
    scene.updateJobVehicleCheckpoint(FerryOffice::Positions::ServiceRunCheckpoint, true);
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::ServiceRunMarker), InteractableType::Info));
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::DockRoadRelay), InteractableType::Info));
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::RelayServiceLog), InteractableType::Info));
    scene.applyInteractionResult(MakeSceneInteraction(std::string(FerryOffice::Names::DockRoadClearanceTag), InteractableType::Info));

    const bool pickedUp = scene.applyInteractionResult(
        MakeSceneInteraction(std::string(FerryOffice::Names::HarborPartsCrate), InteractableType::Info));
    Expect(pickedUp && scene.worldState().isFlagSet(WorldFlag::HarborPartsPickedUp),
        "TestHarborPartsJobRequiresClearedDockRoad",
        "The harbor parts crate should be picked up after the dock road is clear.");
    Expect(scene.currentJobObjectiveText().find("parts") != std::string::npos
            || scene.currentJobObjectiveText().find("Parts") != std::string::npos,
        "TestHarborPartsJobRequiresClearedDockRoad",
        "The objective should point toward the parts delivery after pickup.");

    const bool delivered = scene.applyInteractionResult(
        MakeSceneInteraction(std::string(FerryOffice::Names::FerryOfficePartsShelf), InteractableType::Info));
    Expect(delivered && scene.worldState().isFlagSet(WorldFlag::HarborPartsDelivered),
        "TestHarborPartsJobRequiresClearedDockRoad",
        "The Ferry Office parts shelf should remember the delivered crate.");

    const bool signedOff = scene.applyInteractionResult(
        MakeSceneInteraction(std::string(FerryOffice::Names::FerryOfficeWorkBoard), InteractableType::Info));
    Expect(signedOff && scene.worldState().isFlagSet(WorldFlag::FerryOfficeBoardUpdated),
        "TestHarborPartsJobRequiresClearedDockRoad",
        "The Ferry Office work board should remember the final parts return signoff.");

    const bool handoffFiled = scene.applyInteractionResult(
        MakeSceneInteraction(std::string(FerryOffice::Names::FerryOfficeHandoffNote), InteractableType::Info));
    Expect(handoffFiled && scene.worldState().isFlagSet(WorldFlag::FerryOfficeHandoffFiled),
        "TestHarborPartsJobRequiresClearedDockRoad",
        "The Ferry Office handoff note should remember the final crew handoff.");
    Expect(scene.currentJobObjectiveText().find("storm pump") != std::string::npos,
        "TestHarborPartsJobRequiresClearedDockRoad",
        "The objective should send the player toward the storm pump after the handoff note.");

    const bool pumpReset = scene.applyInteractionResult(
        MakeSceneInteraction(std::string(FerryOffice::Names::StormPumpSwitch), InteractableType::Info));
    Expect(pumpReset && scene.worldState().isFlagSet(WorldFlag::StormPumpReset),
        "TestHarborPartsJobRequiresClearedDockRoad",
        "The storm pump switch should remember the reset after the handoff note is filed.");
    Expect(scene.currentJobObjectiveText().find("ticket") != std::string::npos,
        "TestHarborPartsJobRequiresClearedDockRoad",
        "The objective should route the reset pump back to the Ferry Office ticket.");

    const bool ticketClosed = scene.applyInteractionResult(
        MakeSceneInteraction(std::string(FerryOffice::Names::StormPumpTicket), InteractableType::Info));
    Expect(ticketClosed && scene.worldState().isFlagSet(WorldFlag::StormPumpTicketClosed),
        "TestHarborPartsJobRequiresClearedDockRoad",
        "The storm pump ticket should close after the pump has been reset.");
    Expect(scene.currentJobObjectiveText().find("low dock drain") != std::string::npos,
        "TestHarborPartsJobRequiresClearedDockRoad",
        "The objective should route the closed pump ticket back to the low dock drain.");

    const bool drainCleared = scene.applyInteractionResult(
        MakeSceneInteraction(std::string(FerryOffice::Names::LowDockDrainMarker), InteractableType::Info));
    Expect(drainCleared && scene.worldState().isFlagSet(WorldFlag::LowDockDrainCleared),
        "TestHarborPartsJobRequiresClearedDockRoad",
        "The low dock drain marker should remember the final clear tag after the ticket is closed.");
    Expect(scene.currentJobObjectiveText().find("Drain Log") != std::string::npos,
        "TestHarborPartsJobRequiresClearedDockRoad",
        "The objective should route the clear drain back to the Ferry Office drain log.");

    const bool drainLogged = scene.applyInteractionResult(
        MakeSceneInteraction(std::string(FerryOffice::Names::FerryOfficeDrainLog), InteractableType::Info));
    Expect(drainLogged && scene.worldState().isFlagSet(WorldFlag::LowDockDrainLogged),
        "TestHarborPartsJobRequiresClearedDockRoad",
        "The Ferry Office drain log should remember the post-drain closeout.");
}

void TestPrototypeSceneAppliesAuthoredInteractableFlagBindings()
{
    SceneDefinition sceneDefinition;
    sceneDefinition.id = "binding-test";

    SceneInteractableDefinition immediate;
    immediate.id = "custom-power-switch";
    immediate.name = "Custom Power Switch";
    immediate.prompt = "Use Custom Power Switch";
    immediate.type = "info";
    immediate.position = {0.0f, 0.0f, 0.0f};
    immediate.radius = 1.0f;
    immediate.worldFlagsSet = {"powerRestored"};
    sceneDefinition.interactables.push_back(immediate);

    SceneInteractableDefinition gated;
    gated.id = "custom-route-switch";
    gated.name = "Custom Route Switch";
    gated.prompt = "Use Custom Route Switch";
    gated.type = "info";
    gated.position = {1.0f, 0.0f, 0.0f};
    gated.radius = 1.0f;
    gated.requiredWorldFlags = {"powerRestored"};
    gated.worldFlagsSetWhenReady = {"routeOpened"};
    sceneDefinition.interactables.push_back(gated);

    PrototypeScene scene(sceneDefinition);

    const bool blocked = scene.applyInteractionResult(MakeSceneInteraction("Custom Route Switch", InteractableType::Info));
    Expect(!blocked && !scene.worldState().isFlagSet(WorldFlag::RouteOpened),
        "TestPrototypeSceneAppliesAuthoredInteractableFlagBindings",
        "Authored gated bindings should not fire before their required flags are set.");

    const bool powered = scene.applyInteractionResult(MakeSceneInteraction("Custom Power Switch", InteractableType::Info));
    Expect(powered && scene.worldState().isFlagSet(WorldFlag::PowerRestored),
        "TestPrototypeSceneAppliesAuthoredInteractableFlagBindings",
        "Authored immediate bindings should set their world flags without a hardcoded interaction name.");

    const bool opened = scene.applyInteractionResult(MakeSceneInteraction("Custom Route Switch", InteractableType::Info));
    Expect(opened && scene.worldState().isFlagSet(WorldFlag::RouteOpened),
        "TestPrototypeSceneAppliesAuthoredInteractableFlagBindings",
        "Authored gated bindings should set ready flags after their prerequisites are met.");
}

void TestFerryOfficePlaythroughQaCompletesJobAndWritesReport()
{
    const std::filesystem::path reportPath =
        std::filesystem::temp_directory_path() / "tidebreak-v032-playthrough-report.json";
    std::filesystem::remove(reportPath);

    const FerryOfficePlaythroughQaResult result =
        RunFerryOfficeServiceCallPlaythroughQa(DefaultScenePathForTests(), reportPath);

    Expect(result.passed,
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "The automated Ferry Office playthrough QA should pass on the committed scene.");
    Expect(result.finalPhase == FerryOfficeJobPhase::Complete,
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "The automated playthrough should finish in the complete job phase.");
    Expect(result.vehicleRuntimeBackend == "deterministic",
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "The default playthrough QA should record deterministic vehicle runtime evidence.");

    const WorldState& state = result.finalWorldState;
    Expect(state.isFlagSet(WorldFlag::ManifestCollected),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should collect the manifest.");
    Expect(state.isFlagSet(WorldFlag::ServiceRouteUsed),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should record the service route traversal.");
    Expect(state.isFlagSet(WorldFlag::MaintenanceBoxInspected) && state.isFlagSet(WorldFlag::PowerRestored),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should inspect maintenance and restore power.");
    Expect(state.isFlagSet(WorldFlag::RouteOpened),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should open the service gate.");
    Expect(state.isFlagSet(WorldFlag::ServiceVehicleUsed),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should use the service vehicle.");
    Expect(state.isFlagSet(WorldFlag::DockRoadReached),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should reach the dock-road checkpoint.");
    Expect(state.isFlagSet(WorldFlag::ServiceRunConfirmed) && state.isFlagSet(WorldFlag::FerryOfficeJobComplete),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should confirm and complete the service run.");
    Expect(state.isFlagSet(WorldFlag::DockRoadRelayReset),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should complete the second dock-road relay beat.");
    Expect(state.isFlagSet(WorldFlag::DockRoadRelayLogged),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should log the relay reset as a follow-up consequence.");
    Expect(state.isFlagSet(WorldFlag::DockRoadClearanceTagged),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should tag the dock road clear as a visible local consequence.");
    Expect(state.isFlagSet(WorldFlag::HarborPartsPickedUp),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should collect the harbor parts crate.");
    Expect(state.isFlagSet(WorldFlag::HarborPartsDelivered),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should deliver the harbor parts to the Ferry Office shelf.");
    Expect(state.isFlagSet(WorldFlag::FerryOfficeBoardUpdated),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should update the Ferry Office work board after delivery.");
    Expect(state.isFlagSet(WorldFlag::FerryOfficeHandoffFiled),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should file the Ferry Office handoff note.");
    Expect(state.isFlagSet(WorldFlag::StormPumpReset),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should reset the storm pump as the second compact job seed.");
    Expect(state.isFlagSet(WorldFlag::StormPumpTicketClosed),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should close the storm pump ticket.");
    Expect(state.isFlagSet(WorldFlag::LowDockDrainCleared),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should tag the low dock drain clear after the pump ticket.");
    Expect(state.isFlagSet(WorldFlag::LowDockDrainLogged),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should log the low dock drain closeout back at the Ferry Office.");
    Expect(result.steps.size() >= 9,
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should report each major authored phase.");
    const auto hasStep = [&result](std::string_view name) {
        for (const FerryOfficePlaythroughQaStep& step : result.steps) {
            if (step.name == name) {
                return true;
            }
        }
        return false;
    };
    Expect(hasStep("serviceVehicleRuntime"),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should enter the service vehicle through runtime input.");
    Expect(hasStep("dockRoadRuntimeCheckpoint"),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should drive the service vehicle to the dock-road checkpoint.");
    Expect(hasStep("serviceVehicleRuntimeExit"),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should exit the service vehicle through runtime input.");
    Expect(hasStep("dockRoadRelayReset"),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should record the second dock-road relay beat.");
    Expect(hasStep("dockRoadRelayLogged"),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should record the relay reset log beat.");
    Expect(hasStep("dockRoadClearanceTagged"),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should record the dock-road clearance tag beat.");
    Expect(hasStep("harborPartsPickedUp"),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should record the Harbor Parts pickup beat.");
    Expect(hasStep("harborPartsDelivered"),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should record the Harbor Parts delivery beat.");
    Expect(hasStep("ferryOfficeBoardUpdated"),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should record the work-board signoff beat.");
    Expect(hasStep("ferryOfficeHandoffFiled"),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should record the handoff note beat.");
    Expect(hasStep("stormPumpReset"),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should record the storm pump reset beat.");
    Expect(hasStep("stormPumpTicketClosed"),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should record the storm pump ticket closeout beat.");
    Expect(hasStep("lowDockDrainCleared"),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should record the low dock drain clear-tag beat.");
    Expect(hasStep("lowDockDrainLogged"),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should record the Ferry Office drain log closeout beat.");
    Expect(std::filesystem::exists(reportPath),
        "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
        "QA playthrough should write a report artifact.");

    {
        std::ifstream input(reportPath);
        const nlohmann::json report = nlohmann::json::parse(input);
        Expect(report["schema"] == "v0.32-ferry-office-playthrough-qa",
            "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
            "QA playthrough report should use the v0.32 schema.");
        Expect(report["passed"] == true,
            "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
            "QA playthrough report should mark the run as passed.");
        Expect(report["final"]["flags"]["ferryOfficeJobComplete"] == true,
            "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
            "QA playthrough report should expose final job completion.");
        Expect(report["final"]["flags"]["dockRoadRelayReset"] == true,
            "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
            "QA playthrough report should expose the dock-road relay reset.");
        Expect(report["final"]["flags"]["dockRoadRelayLogged"] == true,
            "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
            "QA playthrough report should expose the dock-road relay sign-off.");
        Expect(report["final"]["flags"]["dockRoadClearanceTagged"] == true,
            "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
            "QA playthrough report should expose the dock-road clearance tag.");
        Expect(report["final"]["flags"]["harborPartsPickedUp"] == true,
            "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
            "QA playthrough report should expose the Harbor Parts pickup.");
        Expect(report["final"]["flags"]["harborPartsDelivered"] == true,
            "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
            "QA playthrough report should expose the Harbor Parts delivery.");
        Expect(report["final"]["flags"]["ferryOfficeBoardUpdated"] == true,
            "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
            "QA playthrough report should expose the work-board signoff.");
        Expect(report["final"]["flags"]["ferryOfficeHandoffFiled"] == true,
            "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
            "QA playthrough report should expose the handoff note.");
        Expect(report["final"]["flags"]["stormPumpReset"] == true,
            "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
            "QA playthrough report should expose the storm pump reset.");
        Expect(report["final"]["flags"]["stormPumpTicketClosed"] == true,
            "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
            "QA playthrough report should expose the storm pump ticket closeout.");
        Expect(report["final"]["flags"]["lowDockDrainCleared"] == true,
            "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
            "QA playthrough report should expose the low dock drain clear tag.");
        Expect(report["final"]["flags"]["lowDockDrainLogged"] == true,
            "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
            "QA playthrough report should expose the Ferry Office drain log closeout.");
        Expect(report["vehicleRuntime"]["backend"] == "deterministic",
            "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
            "QA playthrough report should expose the vehicle runtime backend.");
        Expect(report["vehicleRuntime"]["framesToCheckpoint"] > 0,
            "TestFerryOfficePlaythroughQaCompletesJobAndWritesReport",
            "QA playthrough report should expose vehicle checkpoint timing.");
    }

    std::filesystem::remove(reportPath);
}

void TestFerryOfficePhysicsParityBuildsSceneStaticWorldAndWritesReport()
{
    const std::filesystem::path reportPath =
        std::filesystem::temp_directory_path() / "tidebreak-v033-physics-parity-report.json";
    std::filesystem::remove(reportPath);

    const auto result = RunFerryOfficePhysicsParityQa(
        DefaultScenePathForTests(),
        reportPath,
        engine::physics::PhysicsBackend::Simple);

    Expect(result.passed,
        "TestFerryOfficePhysicsParityBuildsSceneStaticWorldAndWritesReport",
        result.error.empty() ? "Physics parity should pass against the engine simple backend." : result.error);
    Expect(result.sceneId == "ferry-office",
        "TestFerryOfficePhysicsParityBuildsSceneStaticWorldAndWritesReport",
        "Physics parity should report the loaded Ferry Office scene.");
    Expect(result.staticColliderCount == 10,
        "TestFerryOfficePhysicsParityBuildsSceneStaticWorldAndWritesReport",
        "Physics parity should mirror all authored static colliders.");
    Expect(result.floorProbes.size() >= 3,
        "TestFerryOfficePhysicsParityBuildsSceneStaticWorldAndWritesReport",
        "Physics parity should include dock/office/road floor probes.");
    Expect(result.raycastProbes.size() >= 4,
        "TestFerryOfficePhysicsParityBuildsSceneStaticWorldAndWritesReport",
        "Physics parity should include deterministic scene raycast probes.");
    Expect(result.overlapProbes.size() >= 4,
        "TestFerryOfficePhysicsParityBuildsSceneStaticWorldAndWritesReport",
        "Physics parity should include deterministic player-overlap probes.");
    Expect(std::filesystem::exists(reportPath),
        "TestFerryOfficePhysicsParityBuildsSceneStaticWorldAndWritesReport",
        "Physics parity should write the requested JSON report.");

    std::ifstream input(reportPath);
    const nlohmann::json report = nlohmann::json::parse(input);
    input.close();
    Expect(report["schema"] == "v0.33-ferry-office-physics-parity",
        "TestFerryOfficePhysicsParityBuildsSceneStaticWorldAndWritesReport",
        "Physics parity report should use the v0.33 schema.");
    Expect(report["passed"] == true,
        "TestFerryOfficePhysicsParityBuildsSceneStaticWorldAndWritesReport",
        "Physics parity report should mark the simple-backend run as passed.");
    Expect(report["backend"] == "simple",
        "TestFerryOfficePhysicsParityBuildsSceneStaticWorldAndWritesReport",
        "Physics parity report should include the tested backend name.");

    std::filesystem::remove(reportPath);
}

void TestFerryOfficePhysicsParityHandlesUnavailableJoltExplicitly()
{
    if (engine::physics::IsJoltPhysicsAvailable()) {
        return;
    }

    const auto result = RunFerryOfficePhysicsParityQa(
        DefaultScenePathForTests(),
        {},
        engine::physics::PhysicsBackend::Jolt);

    Expect(!result.passed,
        "TestFerryOfficePhysicsParityHandlesUnavailableJoltExplicitly",
        "Default builds should not pretend Jolt parity ran.");
    Expect(!result.error.empty(),
        "TestFerryOfficePhysicsParityHandlesUnavailableJoltExplicitly",
        "Unavailable Jolt parity should explain the opt-in build requirement.");
}

void TestFerryOfficeCharacterContactQaComparesPlayerProxyAndWritesReport()
{
    const std::filesystem::path reportPath =
        std::filesystem::temp_directory_path() / "tidebreak-v034-character-contact-report.json";
    std::filesystem::remove(reportPath);

    const auto result = RunFerryOfficeCharacterContactQa(
        DefaultScenePathForTests(),
        reportPath,
        engine::physics::PhysicsBackend::Simple);

    Expect(result.passed,
        "TestFerryOfficeCharacterContactQaComparesPlayerProxyAndWritesReport",
        result.error.empty() ? "Character contact QA should pass against the engine simple backend." : result.error);
    Expect(result.sceneId == "ferry-office",
        "TestFerryOfficeCharacterContactQaComparesPlayerProxyAndWritesReport",
        "Character contact QA should report the loaded Ferry Office scene.");
    Expect(result.staticColliderCount == 10,
        "TestFerryOfficeCharacterContactQaComparesPlayerProxyAndWritesReport",
        "Character contact QA should mirror authored static colliders.");
    Expect(result.probes.size() >= 7,
        "TestFerryOfficeCharacterContactQaComparesPlayerProxyAndWritesReport",
        "Character contact QA should include grounding, blockers, clear lane, corner, and opened-gate probes.");
    bool foundOpenedGate = false;
    for (const FerryOfficeCharacterContactProbeResult& probe : result.probes) {
        foundOpenedGate = foundOpenedGate || probe.name == "opened-gate-clear";
        Expect(probe.passed,
            "TestFerryOfficeCharacterContactQaComparesPlayerProxyAndWritesReport",
            "Character contact probe should pass: " + probe.name + " / " + probe.message);
    }
    Expect(foundOpenedGate,
        "TestFerryOfficeCharacterContactQaComparesPlayerProxyAndWritesReport",
        "Character contact QA should include an opened-gate nonblocking probe.");
    Expect(std::filesystem::exists(reportPath),
        "TestFerryOfficeCharacterContactQaComparesPlayerProxyAndWritesReport",
        "Character contact QA should write the requested JSON report.");

    std::ifstream input(reportPath);
    const nlohmann::json report = nlohmann::json::parse(input);
    input.close();
    Expect(report["schema"] == "v0.34-ferry-office-character-contact",
        "TestFerryOfficeCharacterContactQaComparesPlayerProxyAndWritesReport",
        "Character contact report should use the v0.34 schema.");
    Expect(report["passed"] == true,
        "TestFerryOfficeCharacterContactQaComparesPlayerProxyAndWritesReport",
        "Character contact report should mark the simple-backend run as passed.");
    Expect(report["probes"].size() >= 7,
        "TestFerryOfficeCharacterContactQaComparesPlayerProxyAndWritesReport",
        "Character contact report should include the expected probe set.");

    std::filesystem::remove(reportPath);
}

void TestFerryOfficeCharacterContactQaHandlesUnavailableBackendExplicitly()
{
    if (engine::physics::IsJoltPhysicsAvailable()) {
        return;
    }

    const auto result = RunFerryOfficeCharacterContactQa(
        DefaultScenePathForTests(),
        {},
        engine::physics::OptInPhysicsBackend());

    Expect(!result.passed,
        "TestFerryOfficeCharacterContactQaHandlesUnavailableBackendExplicitly",
        "Default builds should not pretend opt-in character contact QA ran.");
    Expect(!result.error.empty(),
        "TestFerryOfficeCharacterContactQaHandlesUnavailableBackendExplicitly",
        "Unavailable opt-in character contact QA should explain the build requirement.");
}

void TestFerryOfficeVehiclePhysicsQaWritesFeasibilityReport()
{
    const std::filesystem::path reportPath =
        std::filesystem::temp_directory_path() / "tidebreak-v035-vehicle-feasibility-report.json";
    std::filesystem::remove(reportPath);

    const auto result = RunFerryOfficeVehiclePhysicsQa(
        DefaultScenePathForTests(),
        reportPath,
        engine::physics::PhysicsBackend::Simple);

    Expect(result.passed,
        "TestFerryOfficeVehiclePhysicsQaWritesFeasibilityReport",
        result.error.empty() ? "Vehicle physics QA should pass against the engine simple backend baseline." : result.error);
    Expect(result.sceneId == "ferry-office",
        "TestFerryOfficeVehiclePhysicsQaWritesFeasibilityReport",
        "Vehicle physics QA should report the loaded Ferry Office scene.");
    Expect(result.vehicleId == "service-yard-vehicle",
        "TestFerryOfficeVehiclePhysicsQaWritesFeasibilityReport",
        "Vehicle physics QA should use the authored service-yard vehicle.");
    Expect(result.samples.size() >= 4,
        "TestFerryOfficeVehiclePhysicsQaWritesFeasibilityReport",
        "Vehicle physics QA should record feasibility samples across the scripted run.");
    Expect(result.recommendation == "defer" || result.recommendation == "promote",
        "TestFerryOfficeVehiclePhysicsQaWritesFeasibilityReport",
        "Vehicle physics QA should make an explicit promote/defer recommendation.");
    Expect(std::filesystem::exists(reportPath),
        "TestFerryOfficeVehiclePhysicsQaWritesFeasibilityReport",
        "Vehicle physics QA should write the requested JSON report.");

    std::ifstream input(reportPath);
    const nlohmann::json report = nlohmann::json::parse(input);
    input.close();
    Expect(report["schema"] == "v0.35-ferry-office-vehicle-feasibility",
        "TestFerryOfficeVehiclePhysicsQaWritesFeasibilityReport",
        "Vehicle feasibility report should use the v0.35 schema.");
    Expect(report["scenario"] == "ferry-office-vehicle-feasibility",
        "TestFerryOfficeVehiclePhysicsQaWritesFeasibilityReport",
        "Vehicle feasibility report should expose the scenario.");
    Expect(report["vehicle"]["id"] == "service-yard-vehicle",
        "TestFerryOfficeVehiclePhysicsQaWritesFeasibilityReport",
        "Vehicle feasibility report should include the tested vehicle id.");
    Expect(report["samples"].size() >= 4,
        "TestFerryOfficeVehiclePhysicsQaWritesFeasibilityReport",
        "Vehicle feasibility report should include sampled metrics.");

    std::filesystem::remove(reportPath);
}

void TestFerryOfficeVehiclePhysicsQaHandlesUnavailableBackendExplicitly()
{
    if (engine::physics::IsJoltPhysicsAvailable()) {
        return;
    }

    const auto result = RunFerryOfficeVehiclePhysicsQa(
        DefaultScenePathForTests(),
        {},
        engine::physics::OptInPhysicsBackend());

    Expect(!result.passed,
        "TestFerryOfficeVehiclePhysicsQaHandlesUnavailableBackendExplicitly",
        "Default builds should not pretend opt-in vehicle physics QA ran.");
    Expect(!result.error.empty(),
        "TestFerryOfficeVehiclePhysicsQaHandlesUnavailableBackendExplicitly",
        "Unavailable opt-in vehicle physics QA should explain the build requirement.");
}

void TestVehicleRuntimeAdapterSimpleStepsFrameByFrame()
{
    engine::physics::VehicleRuntimeConfig config;
    config.vehicleId = "service-yard-vehicle";
    config.spawnPosition = {6.2f, 0.0f, -2.2f};
    config.spawnYawRadians = engine::Radians(88.0f);
    config.halfExtents = {0.58f, 0.53f, 0.92f};
    config.boundsMin = {3.35f, -5.05f};
    config.boundsMax = {19.45f, 0.95f};

    std::unique_ptr<engine::physics::IVehicleRuntimeAdapter> adapter =
        engine::physics::CreateVehicleRuntimeAdapter(engine::physics::PhysicsBackend::Simple);

    Expect(adapter != nullptr,
        "TestVehicleRuntimeAdapterSimpleStepsFrameByFrame",
        "Simple vehicle runtime adapter should always be available for fallback comparison.");
    if (!adapter) {
        return;
    }

    Expect(adapter->initialize(config),
        "TestVehicleRuntimeAdapterSimpleStepsFrameByFrame",
        "Simple vehicle runtime adapter should initialize with authored vehicle config.");

    const engine::physics::VehicleRuntimeState initial = adapter->state();
    for (int frame = 0; frame < 40; ++frame) {
        adapter->step({0.6f, 0.15f, 0.0f}, config.fixedStepSeconds);
    }
    const engine::physics::VehicleRuntimeState moved = adapter->state();

    Expect(adapter->backendName() == "simple",
        "TestVehicleRuntimeAdapterSimpleStepsFrameByFrame",
        "Simple runtime adapter should expose its backend name.");
    Expect(moved.frameIndex == 40,
        "TestVehicleRuntimeAdapterSimpleStepsFrameByFrame",
        "Runtime adapter should advance one frame at a time.");
    Expect(engine::Length(moved.position - initial.position) > 0.1f,
        "TestVehicleRuntimeAdapterSimpleStepsFrameByFrame",
        "Runtime adapter should move the vehicle during scripted throttle input.");
    Expect(moved.wheelContactCount == 4,
        "TestVehicleRuntimeAdapterSimpleStepsFrameByFrame",
        "Simple adapter should provide stable fallback wheel-contact telemetry.");
    Expect(!moved.outOfBounds,
        "TestVehicleRuntimeAdapterSimpleStepsFrameByFrame",
        "Simple runtime adapter should stay inside authored service-yard bounds for this probe.");
}

void TestSimpleVehicleRuntimeAdapterRespondsToStaticRoadEdgeCollision()
{
    engine::physics::VehicleRuntimeConfig config;
    config.vehicleId = "service-yard-vehicle";
    config.spawnPosition = {0.0f, 0.0f, 0.0f};
    config.spawnYawRadians = engine::Radians(90.0f);
    config.halfExtents = {0.58f, 0.53f, 0.92f};
    config.boundsMin = {-3.0f, -3.0f};
    config.boundsMax = {8.0f, 3.0f};
    config.staticObstacles.push_back({"authored-road-edge", {2.6f, 0.53f, 0.0f}, {0.18f, 0.53f, 2.0f}});

    std::unique_ptr<engine::physics::IVehicleRuntimeAdapter> adapter =
        engine::physics::CreateVehicleRuntimeAdapter(engine::physics::PhysicsBackend::Simple);

    Expect(adapter != nullptr,
        "TestSimpleVehicleRuntimeAdapterRespondsToStaticRoadEdgeCollision",
        "Simple vehicle runtime adapter should always be available.");
    if (!adapter) {
        return;
    }

    Expect(adapter->initialize(config),
        "TestSimpleVehicleRuntimeAdapterRespondsToStaticRoadEdgeCollision",
        "Simple vehicle runtime adapter should initialize with authored static obstacles.");

    for (int frame = 0; frame < 120; ++frame) {
        adapter->step({1.0f, 0.0f, 0.0f}, config.fixedStepSeconds);
    }
    const engine::physics::VehicleRuntimeState state = adapter->state();
    adapter->shutdown();

    const float maxCenterXBeforeEdge = 2.6f - 0.18f - config.halfExtents.x;
    Expect(state.position.x <= maxCenterXBeforeEdge + 0.05f,
        "TestSimpleVehicleRuntimeAdapterRespondsToStaticRoadEdgeCollision",
        "Authored static road-edge obstacle should block the simple runtime vehicle instead of allowing it through.");
    Expect(std::abs(state.speed) < 0.25f,
        "TestSimpleVehicleRuntimeAdapterRespondsToStaticRoadEdgeCollision",
        "Collision response should damp vehicle speed after contacting an authored road edge.");
}

engine::physics::VehicleRuntimeConfig OpenVehicleRuntimeConfigForTests()
{
    engine::physics::VehicleRuntimeConfig config;
    config.vehicleId = "service-yard-vehicle";
    config.spawnPosition = {0.0f, 0.0f, 0.0f};
    config.spawnYawRadians = engine::Radians(88.0f);
    config.halfExtents = {0.58f, 0.53f, 0.92f};
    config.boundsMin = {-25.0f, -25.0f};
    config.boundsMax = {25.0f, 25.0f};
    return config;
}

float RunVehicleRuntimeYawDelta(
    engine::physics::PhysicsBackend backend,
    const engine::physics::VehicleRuntimeConfig& config,
    engine::physics::VehicleRuntimeInput input)
{
    std::unique_ptr<engine::physics::IVehicleRuntimeAdapter> adapter =
        engine::physics::CreateVehicleRuntimeAdapter(backend);
    Expect(adapter != nullptr,
        "RunVehicleRuntimeYawDelta",
        "Requested vehicle runtime adapter should be available for steering sign testing.");
    if (!adapter) {
        return 0.0f;
    }

    Expect(adapter->initialize(config),
        "RunVehicleRuntimeYawDelta",
        "Vehicle runtime adapter should initialize for steering sign testing.");
    for (int frame = 0; frame < 90; ++frame) {
        adapter->step({0.0f, 0.0f, 0.0f}, config.fixedStepSeconds);
    }
    const float initialYaw = adapter->state().yawRadians;
    const int inputFrameCount = input.throttle < 0.0f ? 150 : 90;
    for (int frame = 0; frame < inputFrameCount; ++frame) {
        adapter->step(input, config.fixedStepSeconds);
    }
    float yawDelta = std::fmod(engine::Degrees(adapter->state().yawRadians - initialYaw), 360.0f);
    if (yawDelta > 180.0f) {
        yawDelta -= 360.0f;
    } else if (yawDelta < -180.0f) {
        yawDelta += 360.0f;
    }
    adapter->shutdown();
    return yawDelta;
}

std::string VehicleRuntimeSteeringSignDetails(
    float forwardLeft,
    float forwardRight,
    float reverseLeft,
    float reverseRight)
{
    return "Yaw deltas: forwardLeft=" + std::to_string(forwardLeft)
        + ", forwardRight=" + std::to_string(forwardRight)
        + ", reverseLeft=" + std::to_string(reverseLeft)
        + ", reverseRight=" + std::to_string(reverseRight) + ".";
}

void TestVehicleRuntimeAdapterSteeringSignSemantics()
{
    const engine::physics::VehicleRuntimeConfig config = OpenVehicleRuntimeConfigForTests();

    const float simpleForwardLeft =
        RunVehicleRuntimeYawDelta(engine::physics::PhysicsBackend::Simple, config, {0.75f, -0.6f, 0.0f});
    const float simpleForwardRight =
        RunVehicleRuntimeYawDelta(engine::physics::PhysicsBackend::Simple, config, {0.75f, 0.6f, 0.0f});
    const float simpleReverseLeft =
        RunVehicleRuntimeYawDelta(engine::physics::PhysicsBackend::Simple, config, {-0.75f, -0.6f, 0.0f});
    const float simpleReverseRight =
        RunVehicleRuntimeYawDelta(engine::physics::PhysicsBackend::Simple, config, {-0.75f, 0.6f, 0.0f});

    Expect(simpleForwardLeft < -1.0f && simpleForwardRight > 1.0f
            && simpleReverseLeft > 1.0f && simpleReverseRight < -1.0f,
        "TestVehicleRuntimeAdapterSteeringSignSemantics",
        "Simple runtime adapter should match deterministic forward/reverse steering sign semantics. "
            + VehicleRuntimeSteeringSignDetails(
                simpleForwardLeft,
                simpleForwardRight,
                simpleReverseLeft,
                simpleReverseRight));

    if (!engine::physics::IsJoltPhysicsAvailable()) {
        return;
    }

    const float joltForwardLeft =
        RunVehicleRuntimeYawDelta(engine::physics::PhysicsBackend::Jolt, config, {0.75f, -0.6f, 0.0f});
    const float joltForwardRight =
        RunVehicleRuntimeYawDelta(engine::physics::PhysicsBackend::Jolt, config, {0.75f, 0.6f, 0.0f});
    const float joltReverseLeft =
        RunVehicleRuntimeYawDelta(engine::physics::PhysicsBackend::Jolt, config, {-0.75f, -0.6f, 0.0f});
    const float joltReverseRight =
        RunVehicleRuntimeYawDelta(engine::physics::PhysicsBackend::Jolt, config, {-0.75f, 0.6f, 0.0f});

    Expect(joltForwardLeft < -1.0f,
        "TestVehicleRuntimeAdapterSteeringSignSemantics",
        "Jolt runtime should turn left, not right, for forward + left input. "
            + VehicleRuntimeSteeringSignDetails(joltForwardLeft, joltForwardRight, joltReverseLeft, joltReverseRight));
    Expect(joltForwardRight > 1.0f,
        "TestVehicleRuntimeAdapterSteeringSignSemantics",
        "Jolt runtime should turn right, not left, for forward + right input. "
            + VehicleRuntimeSteeringSignDetails(joltForwardLeft, joltForwardRight, joltReverseLeft, joltReverseRight));
    Expect(joltReverseLeft > 1.0f,
        "TestVehicleRuntimeAdapterSteeringSignSemantics",
        "Jolt runtime should match deterministic reverse + left steering semantics. "
            + VehicleRuntimeSteeringSignDetails(joltForwardLeft, joltForwardRight, joltReverseLeft, joltReverseRight));
    Expect(joltReverseRight < -1.0f,
        "TestVehicleRuntimeAdapterSteeringSignSemantics",
        "Jolt runtime should match deterministic reverse + right steering semantics. "
            + VehicleRuntimeSteeringSignDetails(joltForwardLeft, joltForwardRight, joltReverseLeft, joltReverseRight));
}

void TestJoltVehicleRuntimeTapThrottleDoesNotCoastForever()
{
    if (!engine::physics::IsJoltPhysicsAvailable()) {
        return;
    }

    engine::physics::VehicleRuntimeConfig config;
    config.vehicleId = "service-yard-vehicle";
    config.spawnPosition = {6.2f, 0.0f, -2.2f};
    config.spawnYawRadians = engine::Radians(88.0f);
    config.halfExtents = {0.58f, 0.53f, 0.92f};
    config.boundsMin = {3.35f, -5.05f};
    config.boundsMax = {19.45f, 0.95f};

    std::unique_ptr<engine::physics::IVehicleRuntimeAdapter> adapter =
        engine::physics::CreateVehicleRuntimeAdapter(engine::physics::PhysicsBackend::Jolt);
    Expect(adapter != nullptr,
        "TestJoltVehicleRuntimeTapThrottleDoesNotCoastForever",
        "Jolt runtime adapter should exist in the opt-in build.");
    if (!adapter) {
        return;
    }

    Expect(adapter->initialize(config),
        "TestJoltVehicleRuntimeTapThrottleDoesNotCoastForever",
        "Jolt runtime adapter should initialize with the authored service vehicle config.");

    adapter->step({1.0f, 0.0f, 0.0f}, config.fixedStepSeconds);
    for (int frame = 0; frame < 90; ++frame) {
        adapter->step({}, config.fixedStepSeconds);
    }

    const engine::physics::VehicleRuntimeState settled = adapter->state();
    adapter->shutdown();
    Expect(std::abs(settled.speed) < 0.25f,
        "TestJoltVehicleRuntimeTapThrottleDoesNotCoastForever",
        "A tiny throttle tap should settle quickly enough for manual service-yard control.");
}

void TestJoltVehicleRuntimeReportsReverseAsNegativeSpeed()
{
    if (!engine::physics::IsJoltPhysicsAvailable()) {
        return;
    }

    engine::physics::VehicleRuntimeConfig config;
    config.vehicleId = "service-yard-vehicle";
    config.spawnPosition = {6.2f, 0.0f, -2.2f};
    config.spawnYawRadians = engine::Radians(88.0f);
    config.halfExtents = {0.58f, 0.53f, 0.92f};
    config.boundsMin = {3.35f, -5.05f};
    config.boundsMax = {19.45f, 0.95f};

    std::unique_ptr<engine::physics::IVehicleRuntimeAdapter> adapter =
        engine::physics::CreateVehicleRuntimeAdapter(engine::physics::PhysicsBackend::Jolt);
    Expect(adapter != nullptr,
        "TestJoltVehicleRuntimeReportsReverseAsNegativeSpeed",
        "Jolt runtime adapter should exist in the opt-in build.");
    if (!adapter) {
        return;
    }

    Expect(adapter->initialize(config),
        "TestJoltVehicleRuntimeReportsReverseAsNegativeSpeed",
        "Jolt runtime adapter should initialize with the authored service vehicle config.");

    for (int frame = 0; frame < 80; ++frame) {
        adapter->step({-1.0f, 0.0f, 0.0f}, config.fixedStepSeconds);
    }

    const engine::physics::VehicleRuntimeState reverse = adapter->state();
    adapter->shutdown();
    Expect(reverse.speed < -0.10f,
        "TestJoltVehicleRuntimeReportsReverseAsNegativeSpeed",
        "Reverse input should report negative longitudinal speed so live controls keep reversing instead of turning S into brake forever.");
}

void TestJoltVehicleRuntimeReachesServiceCheckpointWithinRouteBudget()
{
    if (!engine::physics::IsJoltPhysicsAvailable()) {
        return;
    }

    engine::physics::VehicleRuntimeConfig config;
    config.vehicleId = "service-yard-vehicle";
    config.spawnPosition = {6.2f, 0.0f, -2.2f};
    config.spawnYawRadians = engine::Radians(88.0f);
    config.halfExtents = {0.58f, 0.53f, 0.92f};
    config.boundsMin = {3.35f, -5.05f};
    config.boundsMax = {19.45f, 0.95f};

    std::unique_ptr<engine::physics::IVehicleRuntimeAdapter> adapter =
        engine::physics::CreateVehicleRuntimeAdapter(engine::physics::PhysicsBackend::Jolt);
    Expect(adapter != nullptr,
        "TestJoltVehicleRuntimeReachesServiceCheckpointWithinRouteBudget",
        "Jolt runtime adapter should exist in the opt-in build.");
    if (!adapter) {
        return;
    }

    Expect(adapter->initialize(config),
        "TestJoltVehicleRuntimeReachesServiceCheckpointWithinRouteBudget",
        "Jolt runtime adapter should initialize for service-run route pace QA.");

    constexpr int routeBudgetFrames = 240;
    const engine::Vec3 checkpoint {19.35f, 0.08f, -2.2f};
    bool reached = false;
    bool stable = true;
    for (int frame = 0; frame < routeBudgetFrames; ++frame) {
        adapter->step({0.72f, 0.0f, 0.0f}, config.fixedStepSeconds);
        const engine::physics::VehicleRuntimeState state = adapter->state();
        const float distance = engine::Length(engine::Vec2 {state.position.x - checkpoint.x, state.position.z - checkpoint.z});
        stable = stable && !state.outOfBounds && state.wheelContactCount >= 2;
        if (distance <= 1.8f) {
            reached = true;
            break;
        }
    }
    adapter->shutdown();

    Expect(stable && reached,
        "TestJoltVehicleRuntimeReachesServiceCheckpointWithinRouteBudget",
        "Jolt route pace should reach the authored service-run checkpoint within the automated 240-frame budget.");
}

void TestFerryOfficeVehicleRuntimeComparisonQaWritesReport()
{
    const std::filesystem::path reportPath =
        std::filesystem::temp_directory_path() / "tidebreak-v036-vehicle-runtime-comparison-report.json";
    std::filesystem::remove(reportPath);

    const auto result = RunFerryOfficeVehicleRuntimeComparisonQa(
        DefaultScenePathForTests(),
        reportPath,
        engine::physics::PhysicsBackend::Simple);

    Expect(result.passed,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        result.error.empty() ? "Vehicle runtime comparison should pass against the simple runtime adapter fallback." : result.error);
    Expect(result.scenario == "ferry-office-vehicle-runtime-comparison",
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison QA should expose the explicit runtime scenario.");
    Expect(result.deterministicSamples.size() >= 4 && result.adapterSamples.size() == result.deterministicSamples.size(),
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison QA should record paired deterministic and adapter samples.");
    Expect(result.controlChecks.size() >= 3,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison QA should include manual-control confidence checks for tap, brake, reverse, and coast behavior.");
    for (const auto& check : result.controlChecks) {
        Expect(check.passed,
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            check.message);
    }
    Expect(result.routeChecks.size() == 2,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison QA should include deterministic and adapter service-run route checks.");
    for (const auto& check : result.routeChecks) {
        Expect(check.passed && check.checkpointReached,
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            check.message);
    }
    Expect(result.obstacleChecks.size() == 2,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison QA should include deterministic and adapter obstacle-proxy steering checks.");
    for (const auto& check : result.obstacleChecks) {
        Expect(check.passed && check.clearedObstacleProxy,
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            check.message);
        Expect(check.collisionBacked && check.obstacleCollisionClear && check.obstacleOverlapFrames == 0,
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            "Runtime comparison obstacle checks should clear an overlap-backed obstacle probe.");
        Expect(check.maxCameraYawDeltaDegrees >= 0.0f,
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            "Runtime comparison obstacle checks should include camera-follow yaw stability telemetry.");
    }
    Expect(result.drivingFeelChecks.size() >= 12,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison QA should include deterministic and adapter driving-feel checks for route, controls, steering, and camera.");
    bool foundDeterministicFeel = false;
    bool foundAdapterFeel = false;
    for (const auto& check : result.drivingFeelChecks) {
        foundDeterministicFeel = foundDeterministicFeel || check.backendName == "deterministic";
        foundAdapterFeel = foundAdapterFeel || check.backendName == "simple";
        Expect(check.passed,
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            check.message);
    }
    Expect(foundDeterministicFeel && foundAdapterFeel,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Driving-feel checks should cover the deterministic baseline and selected runtime adapter.");
    Expect(result.inputSemanticsChecks.size() == 8,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison QA should include sign-sensitive deterministic and adapter input semantics checks.");
    bool foundDeterministicInputSemantics = false;
    bool foundAdapterInputSemantics = false;
    for (const auto& check : result.inputSemanticsChecks) {
        foundDeterministicInputSemantics = foundDeterministicInputSemantics || check.backendName == "deterministic";
        foundAdapterInputSemantics = foundAdapterInputSemantics || check.backendName == "simple";
        Expect(check.passed && check.actualSign == check.expectedSign,
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            check.failureReason.empty() ? "Input semantics check should pass." : check.failureReason);
    }
    Expect(foundDeterministicInputSemantics && foundAdapterInputSemantics,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Input semantics checks should cover the deterministic baseline and selected runtime adapter.");
    Expect(result.routePaceProbes.size() >= 3,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison QA should include route-pace sensitivity probes for the selected adapter.");
    bool foundFullThrottlePace = false;
    for (const auto& probe : result.routePaceProbes) {
        foundFullThrottlePace = foundFullThrottlePace || probe.throttle >= 0.99f;
        Expect(probe.passed && probe.checkpointReached && !probe.hitBounds,
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            probe.message);
    }
    Expect(foundFullThrottlePace,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Route-pace sensitivity should include a full-throttle adapter probe.");
    Expect(result.roadEdgeChecks.size() == 2,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison QA should include deterministic and adapter authored road-edge checks.");
    for (const auto& check : result.roadEdgeChecks) {
        Expect(check.passed && check.collisionBacked && check.roadEdgeClear && check.edgeOverlapFrames == 0,
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            check.message);
        Expect(check.authoredEdgeIds.size() >= 2 && check.minCollisionClearance > 0.0f,
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            "Road-edge checks should name authored edge ids and expose positive clearance.");
    }
    Expect(result.broadRouteChecks.size() == 2,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison QA should include deterministic and adapter broad-route collision-response checks.");
    for (const auto& check : result.broadRouteChecks) {
        Expect(check.passed && check.collisionResponseBacked && check.blockedByAuthoredEdge,
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            check.message);
        Expect(check.turnCompleted && check.reverseCompleted && check.readabilityStable,
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            "Broad-route check should cover a turn, reverse recovery, and camera readability.");
        Expect(check.unconstrainedWouldCrossEdge && check.authoredEdgeIds.size() >= 2,
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            "Broad-route check should prove the same script would cross authored road edge without collision response.");
        Expect(check.edgeContactAfterReverseFrames > 0 && !check.blockedEdgeId.empty(),
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            "Broad-route check should identify the authored edge contacted after the reverse/readability segment.");
    }
    Expect(result.extendedRouteChecks.size() == 2,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison QA should include deterministic and adapter extended recorded-input route checks.");
    for (const auto& check : result.extendedRouteChecks) {
        Expect(check.passed && check.inputRecorded && check.authoredRouteId == "route-long-authored-driving-evidence",
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            check.message);
        Expect(check.turnCount >= 3 && check.routeProgressMeters >= 12.0f && check.frameCount >= 360,
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            "Extended-route check should cover a longer authored route with several turns.");
        Expect(check.reverseCompleted && check.cameraResetApplied && check.readabilityStableAfterReset,
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            "Extended-route check should prove reverse plus camera reset/readability evidence.");
        Expect(check.roadEdgeCollisionBacked && check.edgeContactFrames > 0 && !check.blockedEdgeId.empty(),
            "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
            "Extended-route check should include authored road-edge response evidence.");
    }
    Expect(result.maxPositionDelta <= 0.75f,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Simple runtime adapter should stay close to the deterministic fallback path.");
    Expect(std::filesystem::exists(reportPath),
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison QA should write the requested JSON report.");

    std::ifstream input(reportPath);
    const nlohmann::json report = nlohmann::json::parse(input);
    input.close();
    Expect(report["schema"] == "v0.37-ferry-office-vehicle-runtime-comparison",
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Vehicle runtime comparison report should use the v0.37 schema.");
    Expect(report["scenario"] == "ferry-office-vehicle-runtime-comparison",
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Vehicle runtime comparison report should expose the scenario.");
    Expect(report["deterministic"]["samples"].size() >= 4,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison report should include deterministic baseline samples.");
    Expect(report["adapter"]["backend"] == "simple",
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison report should include adapter backend metadata.");
    Expect(report["controlChecks"].size() >= 3,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison report should include controls-focused QA checks.");
    Expect(report["routeChecks"].size() == 2,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison report should include service-run route completion checks.");
    Expect(report["obstacleChecks"].size() == 2,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison report should include obstacle-proxy steering checks.");
    Expect(report["obstacleChecks"][0].contains("collisionBacked")
            && report["obstacleChecks"][0].contains("obstacleCollisionClear")
            && report["obstacleChecks"][0].contains("obstacleOverlapFrames")
            && report["obstacleChecks"][0].contains("minCollisionClearance"),
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison report should expose collision-backed obstacle telemetry.");
    Expect(report["obstacleChecks"][0].contains("maxCameraYawDeltaDegrees")
            && report["obstacleChecks"][0].contains("finalCameraYawDegrees"),
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison report should expose camera-aware obstacle telemetry.");
    Expect(report["drivingFeelChecks"].size() >= 12,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison report should expose deterministic and adapter driving-feel telemetry.");
    Expect(report["drivingFeelChecks"][0].contains("backend")
            && report["drivingFeelChecks"][0].contains("value")
            && report["drivingFeelChecks"][0].contains("minValue")
            && report["drivingFeelChecks"][0].contains("maxValue"),
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison driving-feel checks should include backend and threshold telemetry.");
    Expect(report["inputSemanticsChecks"].size() == 8,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison report should expose sign-sensitive input semantics checks.");
    Expect(report["inputSemanticsChecks"][0].contains("inputName")
            && report["inputSemanticsChecks"][0].contains("throttle")
            && report["inputSemanticsChecks"][0].contains("steer")
            && report["inputSemanticsChecks"][0].contains("yawDeltaDegrees")
            && report["inputSemanticsChecks"][0].contains("expectedSign")
            && report["inputSemanticsChecks"][0].contains("actualSign")
            && report["inputSemanticsChecks"][0].contains("failureReason"),
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Input semantics checks should include input, yaw sign, and failure telemetry.");
    Expect(report["routePaceProbes"].size() >= 3,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison report should expose route-pace sensitivity telemetry.");
    Expect(report["routePaceProbes"][0].contains("throttle")
            && report["routePaceProbes"][0].contains("framesToCheckpoint")
            && report["routePaceProbes"][0].contains("finalSpeed"),
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Route-pace probes should include input and completion telemetry.");
    Expect(report["roadEdgeChecks"].size() == 2,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison report should expose authored road-edge collision evidence.");
    Expect(report["roadEdgeChecks"][0].contains("authoredEdgeIds")
            && report["roadEdgeChecks"][0].contains("collisionBacked")
            && report["roadEdgeChecks"][0].contains("roadEdgeClear")
            && report["roadEdgeChecks"][0].contains("minCollisionClearance")
            && report["roadEdgeChecks"][0].contains("maxCameraYawDeltaDegrees"),
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Road-edge checks should include authored ids, collision clearance, and camera telemetry.");
    Expect(report["broadRouteChecks"].size() == 2,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison report should expose broader live-like route checks.");
    Expect(report["broadRouteChecks"][0].contains("collisionResponseBacked")
            && report["broadRouteChecks"][0].contains("blockedByAuthoredEdge")
            && report["broadRouteChecks"][0].contains("unconstrainedWouldCrossEdge")
            && report["broadRouteChecks"][0].contains("edgeContactAfterReverseFrames")
            && report["broadRouteChecks"][0].contains("blockedEdgeId")
            && report["broadRouteChecks"][0].contains("reverseDistance")
            && report["broadRouteChecks"][0].contains("maxCameraYawDeltaDegrees"),
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Broad-route checks should include collision response, turn/reverse, and camera readability telemetry.");
    Expect(report["extendedRouteChecks"].size() == 2,
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Runtime comparison report should expose extended recorded-input route evidence.");
    Expect(report["extendedRouteChecks"][0].contains("inputScriptName")
            && report["extendedRouteChecks"][0].contains("inputRecorded")
            && report["extendedRouteChecks"][0].contains("authoredRouteId")
            && report["extendedRouteChecks"][0].contains("turnCount")
            && report["extendedRouteChecks"][0].contains("cameraResetApplied")
            && report["extendedRouteChecks"][0].contains("readabilityStableAfterReset")
            && report["extendedRouteChecks"][0].contains("roadEdgeCollisionBacked"),
        "TestFerryOfficeVehicleRuntimeComparisonQaWritesReport",
        "Extended-route checks should include recorded-input, camera reset, and authored road-edge telemetry.");

    std::filesystem::remove(reportPath);
}

void TestFerryOfficeVehicleRuntimeComparisonQaHandlesUnavailableBackendExplicitly()
{
    if (engine::physics::IsJoltPhysicsAvailable()) {
        return;
    }

    const auto result = RunFerryOfficeVehicleRuntimeComparisonQa(
        DefaultScenePathForTests(),
        {},
        engine::physics::OptInPhysicsBackend());

    Expect(!result.passed,
        "TestFerryOfficeVehicleRuntimeComparisonQaHandlesUnavailableBackendExplicitly",
        "Default builds should not pretend opt-in vehicle runtime comparison QA ran.");
    Expect(!result.error.empty(),
        "TestFerryOfficeVehicleRuntimeComparisonQaHandlesUnavailableBackendExplicitly",
        "Unavailable opt-in vehicle runtime comparison QA should explain the build requirement.");
}

void TestSandboxDebugTextUsesReadableSections()
{
    SandboxLayer layer;
    layer.onAttach();
    const std::string text = layer.debugText();
    layer.onDetach();

    Expect(text.find('\n') != std::string::npos,
        "TestSandboxDebugTextUsesReadableSections",
        "Sandbox debug text should use multiple lines for GDI readability.");
    Expect(text.find("objective=") != std::string::npos && text.find("worldState={") != std::string::npos,
        "TestSandboxDebugTextUsesReadableSections",
        "Sandbox debug text should keep objective and world-state sections visible.");
}

void TestFerryOfficeOneShotManifestDoesNotDuplicateEvents()
{
    PrototypeScene scene;
    scene.interactions().updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});

    engine::InputState input;
    input.interactPressed = true;
    const InteractionResult first = scene.interactions().interact(input);
    scene.applyInteractionResult(first);
    const std::size_t eventCountAfterFirst = scene.worldState().eventCount();

    scene.interactions().updateFocus({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
    const InteractionResult second = scene.interactions().interact(input);
    scene.applyInteractionResult(second);

    Expect(first.triggered && !second.triggered,
        "TestFerryOfficeOneShotManifestDoesNotDuplicateEvents",
        "Ferry Manifest should be a one-shot micro-slice action.");
    Expect(scene.worldState().eventCount() == eventCountAfterFirst,
        "TestFerryOfficeOneShotManifestDoesNotDuplicateEvents",
        "Repeating the one-shot manifest action should not duplicate world events.");
}

TraversalAffordance MakeTestTraversalAffordance()
{
    TraversalAffordance affordance;
    affordance.name = "Test Vault";
    affordance.prompt = "Press Space: Vault Test Blocker";
    affordance.type = TraversalType::Vault;
    affordance.startPosition = {0.0f, 0.0f, 1.0f};
    affordance.endPosition = {0.0f, 0.0f, 2.25f};
    affordance.focusRadius = 1.5f;
    affordance.requiredFacingDirection = {0.0f, 0.0f, 1.0f};
    affordance.requiredFacingDot = 0.25f;
    affordance.durationSeconds = 0.25f;
    return affordance;
}

void TestTraversalFocusSelectsAvailableAffordance()
{
    TraversalSystem traversal;
    const int id = traversal.addAffordance(MakeTestTraversalAffordance());

    const TraversalFocus focus = traversal.updateFocus({0.0f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f});
    Expect(focus.hasFocus,
        "TestTraversalFocusSelectsAvailableAffordance",
        "A nearby traversal affordance in front of the player should receive focus.");
    Expect(focus.affordanceId == id,
        "TestTraversalFocusSelectsAvailableAffordance",
        "Traversal focus should expose the selected affordance id.");
    Expect(focus.prompt == "Press Space: Vault Test Blocker",
        "TestTraversalFocusSelectsAvailableAffordance",
        "Traversal focus should expose the prompt text.");
}

void TestTraversalFocusIgnoresDisabledAffordance()
{
    TraversalSystem traversal;
    TraversalAffordance disabled = MakeTestTraversalAffordance();
    disabled.enabled = false;
    traversal.addAffordance(disabled);

    const TraversalFocus focus = traversal.updateFocus({0.0f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f});
    Expect(!focus.hasFocus,
        "TestTraversalFocusIgnoresDisabledAffordance",
        "Disabled traversal affordances should not receive focus.");
}

void TestTraversalTriggerTakesPriorityOverJumpWhenFocused()
{
    TraversalSystem traversal;
    traversal.addAffordance(MakeTestTraversalAffordance());
    traversal.updateFocus({0.0f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f});

    engine::InputState input;
    input.jumpPressed = true;
    const TraversalActivation activation = traversal.activationFromInput(input);

    PlayerController player;
    player.setPosition({0.0f, 0.0f, 0.1f});
    player.update(0.1f, input, 0.0f, &activation);

    Expect(player.state().traversalMode == PlayerTraversalMode::Traversing,
        "TestTraversalTriggerTakesPriorityOverJumpWhenFocused",
        "Focused traversal should put the player into traversal state.");
    ExpectNear(player.state().velocity.y, 0.0f, 0.001f,
        "TestTraversalTriggerTakesPriorityOverJumpWhenFocused",
        "Traversal should suppress the normal jump impulse for that press.");
}

void TestTraversalStartsFromCurrentPlayerPositionToAvoidSnap()
{
    TraversalSystem traversal;
    traversal.addAffordance(MakeTestTraversalAffordance());
    traversal.updateFocus({0.25f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f});

    engine::InputState input;
    input.jumpPressed = true;
    const TraversalActivation activation = traversal.activationFromInput(input);

    PlayerController player;
    player.setPosition({0.25f, 0.0f, 0.1f});
    player.update(0.0f, input, 0.0f, &activation);

    Expect(player.state().traversalMode == PlayerTraversalMode::Traversing,
        "TestTraversalStartsFromCurrentPlayerPositionToAvoidSnap",
        "Focused traversal should start immediately.");
    ExpectNear(player.state().position.x, 0.25f, 0.001f,
        "TestTraversalStartsFromCurrentPlayerPositionToAvoidSnap",
        "Traversal should keep the current player X position at activation instead of snapping to the marker.");
    ExpectNear(player.state().position.z, 0.1f, 0.001f,
        "TestTraversalStartsFromCurrentPlayerPositionToAvoidSnap",
        "Traversal should keep the current player Z position at activation instead of snapping to the marker.");
}

void TestTraversalCompletesAtTargetPosition()
{
    TraversalSystem traversal;
    traversal.addAffordance(MakeTestTraversalAffordance());
    traversal.updateFocus({0.0f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f});

    engine::InputState input;
    input.jumpPressed = true;
    TraversalActivation activation = traversal.activationFromInput(input);

    PlayerController player;
    player.setPosition({0.0f, 0.0f, 0.1f});
    player.update(0.1f, input, 0.0f, &activation);

    input.jumpPressed = false;
    activation = {};
    for (int i = 0; i < 10; ++i) {
        player.update(0.05f, input, 0.0f, &activation);
    }

    Expect(player.state().traversalMode == PlayerTraversalMode::Normal,
        "TestTraversalCompletesAtTargetPosition",
        "Traversal should return the player to normal state after the duration.");
    ExpectNear(player.state().position.x, 0.0f, 0.001f,
        "TestTraversalCompletesAtTargetPosition",
        "Traversal should end at the target X position.");
    ExpectNear(player.state().position.y, 0.0f, 0.001f,
        "TestTraversalCompletesAtTargetPosition",
        "Traversal should end grounded at the target Y position.");
    ExpectNear(player.state().position.z, 2.25f, 0.001f,
        "TestTraversalCompletesAtTargetPosition",
        "Traversal should end at the target Z position.");
}

void TestTraversalLandingRunsWorldResolveAndClearsVelocity()
{
    TraversalSystem traversal;
    traversal.addAffordance(MakeTestTraversalAffordance());
    traversal.updateFocus({0.0f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f});

    PrototypeWorld world;
    world.setFloorHeight(0.0f);
    world.addBox("landing-blocker", {0.0f, 0.5f, 2.25f}, {0.2f, 0.5f, 0.2f});

    engine::InputState input;
    input.jumpPressed = true;
    TraversalActivation activation = traversal.activationFromInput(input);

    PlayerController player;
    player.setWorld(&world);
    player.setPosition({0.0f, 0.0f, 0.1f});
    player.update(0.0f, input, 0.0f, &activation);

    input.jumpPressed = false;
    activation = {};
    for (int i = 0; i < 10; ++i) {
        player.update(0.05f, input, 0.0f, &activation);
    }

    Expect(player.state().traversalMode == PlayerTraversalMode::Normal,
        "TestTraversalLandingRunsWorldResolveAndClearsVelocity",
        "Traversal should return to normal state after landing.");
    Expect(player.state().grounded,
        "TestTraversalLandingRunsWorldResolveAndClearsVelocity",
        "Traversal landing should end grounded after world resolve.");
    ExpectNear(engine::Length(player.state().velocity), 0.0f, 0.001f,
        "TestTraversalLandingRunsWorldResolveAndClearsVelocity",
        "Traversal landing should clear velocity.");
    Expect(player.state().lastCollisionHitCount > 0,
        "TestTraversalLandingRunsWorldResolveAndClearsVelocity",
        "Traversal landing should run world collision resolve when the target overlaps a collider.");
    Expect(player.state().position.z < 2.24f || std::abs(player.state().position.x) > 0.01f,
        "TestTraversalLandingRunsWorldResolveAndClearsVelocity",
        "Traversal landing should not leave the player exactly inside the blocked target position.");
}

void TestTraversalDoesNotRetriggerWhileActive()
{
    TraversalSystem traversal;
    TraversalAffordance first = MakeTestTraversalAffordance();
    first.name = "First Vault";
    first.endPosition = {0.0f, 0.0f, 2.25f};
    const int firstId = traversal.addAffordance(first);

    TraversalAffordance second = MakeTestTraversalAffordance();
    second.name = "Second Vault";
    second.startPosition = {0.0f, 0.0f, 1.1f};
    second.endPosition = {5.0f, 0.0f, 5.0f};
    const int secondId = traversal.addAffordance(second);

    engine::InputState input;
    input.jumpPressed = true;
    PlayerController player;
    player.setPosition({0.0f, 0.0f, 0.1f});

    traversal.updateFocus({0.0f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f});
    TraversalActivation activation = traversal.activationFromInput(input);
    Expect(activation.affordanceId == firstId,
        "TestTraversalDoesNotRetriggerWhileActive",
        "The first traversal should be selected initially.");
    player.update(0.05f, input, 0.0f, &activation);

    TraversalActivation secondActivation;
    secondActivation.started = true;
    secondActivation.affordanceId = secondId;
    secondActivation.name = "Second Vault";
    secondActivation.type = TraversalType::Vault;
    secondActivation.startPosition = {0.0f, 0.0f, 1.1f};
    secondActivation.endPosition = {5.0f, 0.0f, 5.0f};
    secondActivation.durationSeconds = 0.25f;

    player.update(0.05f, input, 0.0f, &secondActivation);
    Expect(player.state().activeTraversalId == firstId,
        "TestTraversalDoesNotRetriggerWhileActive",
        "A new traversal request should be ignored while traversal is already active.");
}

void TestNormalJumpStillWorksWithoutTraversalFocus()
{
    PlayerController player;
    player.setPosition({0.0f, 0.0f, 0.0f});

    engine::InputState input;
    input.jumpPressed = true;
    player.update(0.1f, input, 0.0f);

    Expect(player.state().traversalMode == PlayerTraversalMode::Normal,
        "TestNormalJumpStillWorksWithoutTraversalFocus",
        "Normal jump should not enter traversal state without a traversal activation.");
    Expect(player.state().position.y > 0.0f,
        "TestNormalJumpStillWorksWithoutTraversalFocus",
        "Normal jump should still lift the player when no traversal affordance is focused.");
    Expect(!player.state().grounded,
        "TestNormalJumpStillWorksWithoutTraversalFocus",
        "Player should be airborne after a normal jump.");
}

void TestInteractionCanTriggerAfterTraversalCompletes()
{
    TraversalSystem traversal;
    traversal.addAffordance(MakeTestTraversalAffordance());
    traversal.updateFocus({0.0f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f});

    engine::InputState input;
    input.jumpPressed = true;
    TraversalActivation activation = traversal.activationFromInput(input);

    PlayerController player;
    player.setPosition({0.0f, 0.0f, 0.1f});
    player.update(0.0f, input, 0.0f, &activation);

    input.jumpPressed = false;
    activation = {};
    for (int i = 0; i < 10; ++i) {
        player.update(0.05f, input, 0.0f, &activation);
    }

    InteractionSystem interactions;
    Interactable info;
    info.name = "Post Traversal Marker";
    info.prompt = "Read Post Traversal Marker";
    info.position = player.state().position + engine::Vec3 {0.0f, 0.5f, 0.75f};
    info.radius = 1.5f;
    info.type = InteractableType::Info;
    info.message = "Interaction after traversal works.";
    interactions.addInteractable(info);

    interactions.updateFocus(player.state().position, {0.0f, 0.0f, 1.0f});
    engine::InputState interactInput;
    interactInput.interactPressed = true;
    const InteractionResult result = interactions.interact(interactInput);

    Expect(result.triggered,
        "TestInteractionCanTriggerAfterTraversalCompletes",
        "Interaction input should still trigger after traversal returns to normal state.");
}

} // namespace

int main()
{
    TestSmokeArgumentsEnableBoundedHeadlessRun();
    TestFramesArgumentOverridesSmokeDefault();
    TestInvalidRendererIsRejected();
    TestCaptureArgumentsSelectOutputPath();
    TestCaptureDirArgumentsSelectOutputDirectory();
    TestCaptureArgumentsRejectAmbiguousDestination();
    TestHelpTextMentionsCaptureFlags();
    TestQaPlaythroughArgumentsSelectScenarioAndReportPath();
    TestQaPlaythroughArgumentsRejectUnknownScenario();
    TestHelpTextMentionsQaPlaythroughFlags();
    TestQaPhysicsParityArgumentsSelectScenarioAndReportPath();
    TestQaPhysicsParityArgumentsRejectUnknownScenario();
    TestQaPhysicsParityArgumentsAcceptCharacterContactScenario();
    TestQaPhysicsParityArgumentsAcceptVehicleFeasibilityScenario();
    TestQaPhysicsParityArgumentsAcceptVehicleRuntimeComparisonScenario();
    TestQaCaptureStateArgumentsSelectMidChainRouteState();
    TestVehicleRuntimeArgumentsDefaultToDeterministicFallback();
    TestVehicleRuntimeArgumentsAcceptPreferred();
    TestVehicleRuntimeArgumentsAcceptJoltOptIn();
    TestVehicleRuntimeArgumentsRejectUnknownRuntime();
    TestHelpTextMentionsQaPhysicsParityFlags();
    TestHelpTextMentionsVehicleRuntimeFlag();
    TestCursorCaptureArguments();
    TestUiModeArguments();
    TestInputStateTracksDebugOverlayToggleEdge();
    TestSceneArgumentSelectsRuntimeScenePath();
    TestNormalizePathKeepsAssetPathsInsideBase();
    TestClockStartsAtFrameZeroAndTicksForward();
    TestNullRendererRecordsFrameAndDebugDraw();
    TestBmpWriterCreatesTopDown32BitBmp();
    TestDebugBitmapTextBuildsGlyphQuadsInsideBounds();
    TestDebugBitmapTextWrapsLongLinesBeforeOverflow();
    TestDebugProjectionKeepsLongVisibleLinesWhenEndpointsAreOffscreen();
    TestDebugProjectionClipsLinesAndTrianglesAgainstNearPlane();
    TestDebugProjectionSortsProjectedTrianglesBackToFront();
    TestDebugWorldToClipMatrixMatchesCpuProjection();
    TestDebugWorldToClipMatrixMapsDepthIntoDx11Range();
    TestStaticMeshLoaderLoadsCommittedUnitBox();
    TestStaticMeshLoaderLoadsV018PropKit();
    TestStaticMeshLoaderReportsMissingAsset();
    TestStaticMeshLoaderReportsInvalidJsonTypesWithoutThrowing();
    TestStaticMeshLoaderHonorsInterleavedPositionStride();
    TestStaticMeshBuildsTransformedTriangleList();
    TestSceneLoaderLoadsDefaultFerryOfficeScene();
    TestSceneLoaderLoadsPilotSliceMetadata();
    TestSceneRuntimePolicyKeepsFallbackAsFerryOfficeRegression();
    TestSceneRuntimePolicyClassifiesRegressionAndTargetSliceRoles();
    TestSceneRuntimePolicyTreatsUnknownLoadedSceneAsNeutral();
    TestSceneRuntimePackageKeepsFallbackAsFerryOfficeSubmission();
    TestSceneRuntimePackageCombinesFerryOfficePolicyAndAuthoredSubmission();
    TestSceneRuntimePackageCombinesTargetSliceNeutralPolicyAndSubmission();
    TestSceneRuntimePackageTreatsGenericLoadedSceneAsNeutralAuthored();
    TestSceneGuidanceSurfaceShowsAllAuthoredNeutralMarkers();
    TestSceneGuidanceSurfaceKeepsFerryOfficeActiveGuidanceScoped();
    TestSceneRenderSurfacePlansFallbackFerryOfficeSubmission();
    TestSceneRenderSurfacePlansLoadedFerryOfficeRegressionSubmission();
    TestSceneRenderSurfacePlansTargetSliceNeutralSubmission();
    TestNeutralSceneRuntimeSurfaceBuildsPresentationWithoutFerryOfficeTerms();
    TestNeutralSceneRuntimeSurfaceBuildsDebugWithoutFerryOfficeTelemetry();
    TestSceneLoaderLoadsV018VisualIdentityPropKit();
    TestSceneLoaderLoadsFirstJobMarkers();
    TestSceneLoaderLoadsDockRoadRelayBeat();
    TestSceneLoaderLoadsHarborPartsJobBeat();
    TestSceneLoaderLoadsInteractableActionPrerequisites();
    TestSceneLoaderReportsMissingSceneFile();
    TestSceneLoaderLoadsVehicleBoundsAndRoadMarkers();
    TestPrototypeWorldBuildsFerryOfficeCollidersFromSceneData();
    TestPrototypeSceneLoadsInteractionsAndTraversalFromSceneData();
    TestSceneLoaderRejectsUnknownWorldFlags();
    TestBuiltInFerryOfficeFallbackContainsFollowupInteractables();
    TestPhysicsWorldCreatesAndShutsDownThroughVendorFreeInterface();
    TestPhysicsWorldRaycastHitsStaticBox();
    TestPhysicsWorldDebugLinesExposeStaticBoxes();
    TestPhysicsWorldOverlapBoxFindsStaticBox();
    TestJoltBackendAvailabilityIsExplicit();
    TestVehicleControllerAcceleratesBrakesAndReversesDeterministically();
    TestVehicleControllerSteeringChangesYawWhileMoving();
    TestVehicleControllerSteeringSignSemantics();
    TestVehicleLowSpeedSteeringAssistKeepsTurnaroundReadable();
    TestVehicleEnterExitUsesPressedEdgeAndSafeExit();
    TestVehicleCameraTargetFollowsVehicle();
    TestVehicleDefaultTuningFitsSmallServiceYard();
    TestVehicleFocusUsesFacingAndCloseFallback();
    TestVehicleBoundsClampStopsVehicle();
    TestVehicleDragUsesDeltaClamp();
    TestVehicleExitPositionUsesSideAndBackOffsets();
    TestVehicleReverseSteeringIsPredictable();
    TestSandboxLayerVehicleDebugTextIncludesRoadTestTelemetry();
    TestSandboxLayerDebugTextMarksLiveVehicleRuntimeAdapterWhenOptedIn();
    TestSandboxLayerDebugTextIncludesDockRoadTelemetry();
    TestScenePresentationKnowsEveryAuthoredSceneColorKey();
    TestScenePresentationHasMaterialForEveryAuthoredSceneColorKey();
    TestScenePresentationDynamicPaletteStatesRemainDistinct();
    TestScenePresentationMaterialPresetsStayBoundedForAuthoredKeys();
    TestScenePresentationUsesAuthoredSceneMaterialDefaults();
    TestScenePresentationMaterialPresetsVarySurfaceResponse();
    TestScenePresentationOvercastShadingAddsVolumeCue();
    TestScenePresentationShadedBoxSubmitsTwelveFaces();
    TestSandboxLayerDrawsEveryLoadedSceneMeshInstance();
    TestSandboxLayerPlaytestRenderSuppressesRawDebugScaffolding();
    TestSandboxLayerDebugRenderKeepsRawDebugScaffolding();
    TestSandboxLayerPlaytestTextPrioritizesObjectiveAndPrompt();
    TestSandboxLayerPilotSliceUsesNeutralPresentation();
    TestSandboxLayerPilotSliceDebugTextAvoidsFerryOfficeTelemetry();
    TestSandboxLayerPlaytestDefersFutureRouteGuidanceAtStart();
    TestSandboxLayerDebugPreservesFullRouteGuidanceAtStart();
    TestSandboxLayerUsesScenePlayerStartYawForInitialComposition();
    TestSandboxLayerPlaytestKeepsRouteDebugHiddenAfterManifest();
    TestSandboxLayerQaCaptureStateDrawsMidChainRouteGuidance();
    TestSandboxLayerQaCaptureStateDrawsLowDockDrainAccessGuidance();
    TestSandboxLayerQaCaptureStateDrawsVehicleDockRoadPerspective();
    TestSandboxLayerDebugTextPreservesFullTelemetry();
    TestSandboxLayerMinimalTextStaysSmallButUseful();
    TestSandboxLayerF1TogglesBetweenPlaytestAndDebugText();
    TestDebugWindowTitleUsesStableSingleLine();
    TestGameCodeDoesNotReferenceJoltVendorApi();
    TestVec3NormalizationKeepsDiagonalMovementAtUnitLength();
    TestPlayerMovementIsCameraRelativeAndNormalized();
    TestPlayerSprintAndJumpRemainGroundedDeterministically();
    TestThirdPersonCameraClampsPitchAndSmooths();
    TestThirdPersonCameraCanFollowTargetYawForVehicles();
    TestAabbOverlapAndClosestPoint();
    TestPrototypeWorldGroundClampAndGroundedState();
    TestPrototypeWorldPushesPlayerOutOfBox();
    TestPlayerMovementUsesWorldCollisionForWall();
    TestDiagonalMovementIntoObstacleDoesNotTunnel();
    TestPrototypeWorldRaycastFindsNearestCollider();
    TestInteractionFocusSelectsNearestFacingCandidate();
    TestInteractionFocusIgnoresDisabledAndConsumedInteractables();
    TestInteractPressedTriggersExactlyOnceWhenHeld();
    TestOneShotPickupConsumesAndDoesNotTriggerAgain();
    TestToggleInteractableChangesStateOnEachPress();
    TestNoFocusedInteractableMeansNoAction();
    TestFerryOfficeDataDefinesStableNamesAndPositions();
    TestWorldStateSetsAndReadsFlags();
    TestWorldStateExposesFirstJobFlags();
    TestWorldStateRecordsEventsInOrder();
    TestWorldStateIgnoresRepeatedSameFlagValue();
    TestOneShotManifestPickupUpdatesWorldStateOnce();
    TestMaintenanceInteractionRestoresPower();
    TestWallButtonOpensRouteWithoutClosingItAgain();
    TestTraversalCompletionRecordsServiceRouteUsed();
    TestAuthoredTraversalCompletionBindingsControlWorldFlags();
    TestFerryOfficeJobStartsIncompleteAndOrdersObjectives();
    TestFerryOfficeJobVehicleCheckpointRequiresOccupiedVehicle();
    TestFerryOfficeJobCompletionRequiresAllJobFlags();
    TestFerryOfficeFollowupStatusSummarizesEndpointChain();
    TestFerryOfficeFollowupNextStepGuidesLongChain();
    TestFerryOfficeSliceStartsIncomplete();
    TestFerryOfficeObjectiveTextGuidesRouteSteps();
    TestFerryOfficeCompletionRequiresRememberedLoop();
    TestDockRoadRelayRequiresCompletedServiceCall();
    TestHarborPartsJobRequiresClearedDockRoad();
    TestPrototypeSceneAppliesAuthoredInteractableFlagBindings();
    TestFerryOfficeExitMarkerRequiresReadyState();
    TestWallButtonOpenLatchesServiceGateColliderState();
    TestStormPumpTicketOpensLowDockDrainAccessBarrier();
    TestFerryOfficeServiceVaultFocusesFromAccessibleSide();
    TestMaintenanceBoxIsNotFocusedBeforeServiceVault();
    TestFerryOfficeLoopCanCompleteThroughSceneSystems();
    TestFerryOfficePlaythroughQaCompletesJobAndWritesReport();
    TestFerryOfficePhysicsParityBuildsSceneStaticWorldAndWritesReport();
    TestFerryOfficePhysicsParityHandlesUnavailableJoltExplicitly();
    TestFerryOfficeCharacterContactQaComparesPlayerProxyAndWritesReport();
    TestFerryOfficeCharacterContactQaHandlesUnavailableBackendExplicitly();
    TestFerryOfficeVehiclePhysicsQaWritesFeasibilityReport();
    TestFerryOfficeVehiclePhysicsQaHandlesUnavailableBackendExplicitly();
    TestVehicleRuntimeAdapterSimpleStepsFrameByFrame();
    TestSimpleVehicleRuntimeAdapterRespondsToStaticRoadEdgeCollision();
    TestVehicleRuntimeAdapterSteeringSignSemantics();
    TestJoltVehicleRuntimeTapThrottleDoesNotCoastForever();
    TestJoltVehicleRuntimeReportsReverseAsNegativeSpeed();
    TestJoltVehicleRuntimeReachesServiceCheckpointWithinRouteBudget();
    TestFerryOfficeVehicleRuntimeComparisonQaWritesReport();
    TestFerryOfficeVehicleRuntimeComparisonQaHandlesUnavailableBackendExplicitly();
    TestSandboxDebugTextUsesReadableSections();
    TestFerryOfficeOneShotManifestDoesNotDuplicateEvents();
    TestTraversalFocusSelectsAvailableAffordance();
    TestTraversalFocusIgnoresDisabledAffordance();
    TestTraversalTriggerTakesPriorityOverJumpWhenFocused();
    TestTraversalStartsFromCurrentPlayerPositionToAvoidSnap();
    TestTraversalCompletesAtTargetPosition();
    TestTraversalLandingRunsWorldResolveAndClearsVelocity();
    TestTraversalDoesNotRetriggerWhileActive();
    TestNormalJumpStillWorksWithoutTraversalFocus();
    TestInteractionCanTriggerAfterTraversalCompletes();

    if (!failures.empty()) {
        std::cerr << failures.size() << " test failure(s):\n";
        for (const auto& failure : failures) {
            std::cerr << "  " << failure.name << ": " << failure.message << '\n';
        }
        return 1;
    }

    std::cout << "EngineCoreTests passed.\n";
    return 0;
}
