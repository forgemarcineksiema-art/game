cmake_minimum_required(VERSION 3.24)

project(GameEngineFoundation
    VERSION 0.11.0
    DESCRIPTION "AI-native C++ action/adventure engine foundation"
    LANGUAGES CXX
)

option(ENGINE_BUILD_TESTS "Build engine tests" ON)
option(ENGINE_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(ENGINE_ENABLE_JOLT_PHYSICS "Enable the opt-in Jolt physics backend spike" OFF)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(EngineCore STATIC
    src/engine/application/Application.cpp
    src/engine/application/Engine.cpp
    src/engine/core/Clock.cpp
    src/engine/core/Config.cpp
    src/engine/core/FileSystem.cpp
    src/engine/core/Logger.cpp
    src/engine/input/Input.cpp
    src/engine/platform/Window.cpp
    src/engine/physics/PhysicsWorld.cpp
    src/engine/renderer/NullRenderer.cpp
    src/engine/renderer/RendererFactory.cpp
)

target_include_directories(EngineCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(EngineCore PUBLIC cxx_std_20)

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
    include(FetchContent)

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
    src/game/InteractionSystem.cpp
    src/game/FerryOfficeData.cpp
    src/game/PlayerController.cpp
    src/game/SandboxLayer.cpp
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

target_link_libraries(GamePrototype PUBLIC EngineCore)

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
endif()
