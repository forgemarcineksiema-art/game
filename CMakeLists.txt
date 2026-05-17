cmake_minimum_required(VERSION 3.24)

project(GameEngineFoundation
    VERSION 0.23.0
    DESCRIPTION "AI-native C++ action/adventure engine foundation"
    LANGUAGES CXX
)

option(ENGINE_BUILD_TESTS "Build engine tests" ON)
option(ENGINE_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(ENGINE_ENABLE_JOLT_PHYSICS "Enable the opt-in Jolt physics backend spike" OFF)

find_package(Python3 COMPONENTS Interpreter REQUIRED)
include(FetchContent)

set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(EngineCore STATIC
    src/engine/application/Application.cpp
    src/engine/assets/StaticMesh.cpp
    src/engine/application/Engine.cpp
    src/engine/core/Clock.cpp
    src/engine/core/Config.cpp
    src/engine/core/FileSystem.cpp
    src/engine/core/Logger.cpp
    src/engine/platform/Window.cpp
    src/engine/physics/PhysicsWorld.cpp
    src/engine/physics/VehicleProbe.cpp
    src/engine/physics/VehicleRuntime.cpp
    src/engine/renderer/BmpWriter.cpp
    src/engine/renderer/DebugBitmapText.cpp
    src/engine/renderer/NullRenderer.cpp
    src/engine/renderer/RendererFactory.cpp
)

target_include_directories(EngineCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(EngineCore PUBLIC cxx_std_20)
target_link_libraries(EngineCore PUBLIC nlohmann_json::nlohmann_json)

target_compile_definitions(EngineCore
    PUBLIC
        ENGINE_VERSION_STRING="${PROJECT_VERSION}"
        ENGINE_WITH_JOLT=$<BOOL:${ENGINE_ENABLE_JOLT_PHYSICS}>
)

if(WIN32)
    target_compile_definitions(EngineCore PUBLIC ENGINE_PLATFORM_WINDOWS=1)
    target_sources(EngineCore
        PRIVATE
            src/engine/renderer/Dx11Renderer.cpp
            src/engine/platform/Win32Window.cpp
            src/engine/renderer/GdiRenderer.cpp
    )
    target_link_libraries(EngineCore PUBLIC user32 gdi32 d3d11 dxgi d3dcompiler)
else()
    target_compile_definitions(EngineCore PUBLIC ENGINE_PLATFORM_WINDOWS=0)
endif()

if(ENGINE_ENABLE_JOLT_PHYSICS)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(CPP_EXCEPTIONS_ENABLED OFF CACHE BOOL "" FORCE)
    set(CPP_RTTI_ENABLED OFF CACHE BOOL "" FORCE)
    set(DEBUG_RENDERER_IN_DEBUG_AND_RELEASE ON CACHE BOOL "" FORCE)
    set(ENABLE_ALL_WARNINGS OFF CACHE BOOL "" FORCE)
    set(ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(GENERATE_DEBUG_SYMBOLS OFF CACHE BOOL "" FORCE)
    set(INTERPROCEDURAL_OPTIMIZATION OFF CACHE BOOL "" FORCE)
    set(OVERRIDE_CXX_FLAGS OFF CACHE BOOL "" FORCE)
    set(USE_AVX OFF CACHE BOOL "" FORCE)
    set(USE_AVX2 OFF CACHE BOOL "" FORCE)
    set(USE_AVX512 OFF CACHE BOOL "" FORCE)
    set(USE_F16C OFF CACHE BOOL "" FORCE)
    set(USE_FMADD OFF CACHE BOOL "" FORCE)
    set(USE_LZCNT OFF CACHE BOOL "" FORCE)
    set(USE_SSE4_1 OFF CACHE BOOL "" FORCE)
    set(USE_SSE4_2 OFF CACHE BOOL "" FORCE)
    set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)
    set(USE_TZCNT OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        JoltPhysics
        GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics.git
        GIT_TAG v5.5.0
        GIT_SHALLOW TRUE
        SOURCE_SUBDIR Build
    )
    FetchContent_MakeAvailable(JoltPhysics)

    target_sources(EngineCore
        PRIVATE
            src/engine/physics/JoltPhysicsWorld.cpp
            src/engine/physics/JoltVehicleProbe.cpp
            src/engine/physics/JoltVehicleRuntime.cpp
    )
    target_link_libraries(EngineCore PRIVATE Jolt::Jolt)
endif()

if(MSVC)
    target_compile_options(EngineCore PRIVATE /W4 /permissive-)
    if(ENGINE_WARNINGS_AS_ERRORS)
        target_compile_options(EngineCore PRIVATE /WX)
    endif()
else()
    target_compile_options(EngineCore PRIVATE -Wall -Wextra -Wpedantic)
    if(ENGINE_WARNINGS_AS_ERRORS)
        target_compile_options(EngineCore PRIVATE -Werror)
    endif()
endif()

add_library(GamePrototype STATIC
    src/game/FerryOfficeCharacterContactQa.cpp
    src/game/FerryOfficeJob.cpp
    src/game/FerryOfficePhysicsParity.cpp
    src/game/FerryOfficePlaythroughQa.cpp
    src/game/FerryOfficeVehiclePhysicsQa.cpp
    src/game/InteractionSystem.cpp
    src/game/PlayerController.cpp
    src/game/SandboxLayer.cpp
    src/game/SceneDefinition.cpp
    src/game/SceneGuidanceSurface.cpp
    src/game/SceneLoader.cpp
    src/game/ScenePresentation.cpp
    src/game/SceneRenderSurface.cpp
    src/game/SceneRuntimePackage.cpp
    src/game/SceneRuntimePolicy.cpp
    src/game/SceneRuntimeSurface.cpp
    src/game/TargetSliceObjectiveRuntime.cpp
    src/game/TargetSliceObjectiveQa.cpp
    src/game/PrototypeScene.cpp
    src/game/PrototypeWorld.cpp
    src/game/ThirdPersonCamera.cpp
    src/game/TraversalSystem.cpp
    src/game/VehicleController.cpp
    src/game/WorldState.cpp
)

target_include_directories(GamePrototype
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(GamePrototype
    PUBLIC
        ENGINE_SOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(GamePrototype PUBLIC EngineCore PRIVATE nlohmann_json::nlohmann_json)

if(MSVC)
    target_compile_options(GamePrototype PRIVATE /W4 /permissive-)
    if(ENGINE_WARNINGS_AS_ERRORS)
        target_compile_options(GamePrototype PRIVATE /WX)
    endif()
else()
    target_compile_options(GamePrototype PRIVATE -Wall -Wextra -Wpedantic)
    if(ENGINE_WARNINGS_AS_ERRORS)
        target_compile_options(GamePrototype PRIVATE -Werror)
    endif()
endif()

add_executable(EngineApp
    src/game/main.cpp
)

target_link_libraries(EngineApp PRIVATE GamePrototype)

if(ENGINE_BUILD_TESTS)
    enable_testing()

    add_executable(EngineCoreTests
        tests/EngineCoreTests.cpp
    )

    target_link_libraries(EngineCoreTests PRIVATE EngineCore GamePrototype)
    target_compile_definitions(EngineCoreTests
        PRIVATE
            ENGINE_SOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
    )

    add_test(NAME EngineCoreTests COMMAND EngineCoreTests)
    add_test(NAME EngineSmokeTest COMMAND EngineApp --smoke-test --frames 3)
    add_test(
        NAME SceneToolTests
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_scene_tools.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_test(
        NAME WorldAuthorTests
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_world_author.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_test(
        NAME RunScriptTests
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_run_scripts.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_test(
        NAME CaptureVisualSmokeTests
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_capture_visual_smoke.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_test(
        NAME RuntimeSceneSmokeTests
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_runtime_scene_smoke.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_test(
        NAME PlaythroughQaTests
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_playthrough_qa.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_test(
        NAME PhysicsParityToolTests
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_physics_parity_tool.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_test(
        NAME CharacterContactQaTests
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_character_contact_qa.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_test(
        NAME VehiclePhysicsQaTests
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_vehicle_physics_qa.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_test(
        NAME VehicleRuntimeQaTests
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_vehicle_runtime_qa.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_test(
        NAME TargetSliceObjectiveQaTests
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_target_slice_objective_qa.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    if(ENGINE_ENABLE_JOLT_PHYSICS)
        add_test(
            NAME FerryOfficeJoltPhysicsParitySmoke
            COMMAND
                ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/tools/physics_parity_qa.py
                --exe $<TARGET_FILE:EngineApp>
                --report-json ${CMAKE_CURRENT_BINARY_DIR}/physics/ferry-office-collision-parity-report.json
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        add_test(
            NAME FerryOfficeCharacterContactQaSmoke
            COMMAND
                ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/tools/character_contact_qa.py
                --exe $<TARGET_FILE:EngineApp>
                --report-json ${CMAKE_CURRENT_BINARY_DIR}/physics/ferry-office-character-contact-report.json
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        add_test(
            NAME FerryOfficeVehiclePhysicsQaSmoke
            COMMAND
                ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/tools/vehicle_physics_qa.py
                --exe $<TARGET_FILE:EngineApp>
                --report-json ${CMAKE_CURRENT_BINARY_DIR}/physics/ferry-office-vehicle-feasibility-report.json
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        add_test(
            NAME FerryOfficeVehicleRuntimeQaSmoke
            COMMAND
                ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/tools/vehicle_runtime_qa.py
                --exe $<TARGET_FILE:EngineApp>
                --report-json ${CMAKE_CURRENT_BINARY_DIR}/physics/ferry-office-vehicle-runtime-comparison-report.json
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        add_test(
            NAME FerryOfficeJoltPlaythroughQaSmoke
            COMMAND
                ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/tools/playthrough_qa.py
                --exe $<TARGET_FILE:EngineApp>
                --vehicle-runtime jolt
                --report-json ${CMAKE_CURRENT_BINARY_DIR}/playthroughs/ferry-office-service-call-jolt-report.json
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()
    add_test(
        NAME FerryOfficePlaythroughQaSmoke
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/playthrough_qa.py
            --exe $<TARGET_FILE:EngineApp>
            --report-json ${CMAKE_CURRENT_BINARY_DIR}/playthroughs/ferry-office-service-call-report.json
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_test(
        NAME TargetSliceObjectiveQaSmoke
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/target_slice_objective_qa.py
            --exe $<TARGET_FILE:EngineApp>
            --report-json ${CMAKE_CURRENT_BINARY_DIR}/playthroughs/veyra-target-objective-acquisition-report.json
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()
