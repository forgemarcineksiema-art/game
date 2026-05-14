cmake_minimum_required(VERSION 3.24)

project(GameEngineFoundation
    VERSION 0.5.1
    DESCRIPTION "AI-native C++ action/adventure engine foundation"
    LANGUAGES CXX
)

option(ENGINE_BUILD_TESTS "Build engine tests" ON)
option(ENGINE_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

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
    src/game/PlayerController.cpp
    src/game/SandboxLayer.cpp
    src/game/TestScene.cpp
    src/game/TestWorld.cpp
    src/game/ThirdPersonCamera.cpp
    src/game/TraversalSystem.cpp
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

    add_test(NAME EngineCoreTests COMMAND EngineCoreTests)
    add_test(NAME EngineSmokeTest COMMAND EngineApp --smoke-test --frames 3)
endif()
