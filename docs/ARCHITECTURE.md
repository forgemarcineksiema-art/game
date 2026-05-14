# Architecture

v0.1 establishes a small custom C++20 runtime with explicit seams between application, platform, renderer, and game code.

## Application

`src/engine/application/Application.*` owns the runtime loop. It initializes engine systems, creates a platform window when requested, creates a renderer backend, calls the game layer, handles frame limits, and shuts everything down in order.

`src/engine/application/Engine.*` is the root engine object for foundational systems. In v0.1 it owns the clock and startup/shutdown lifecycle.

## Platform / Window

`src/engine/platform/Window.h` defines `IWindow`.

On Windows, `src/engine/platform/Win32Window.cpp` provides a small Win32 window implementation. Non-Windows builds can still use headless smoke mode through the null renderer path.

## Input

`src/engine/input/Input.h` defines `InputState`. It is intentionally a placeholder in v0.1 so future work can add keyboard, mouse, controller, and action mapping without contaminating the engine loop.

## Time

`src/engine/core/Clock.*` uses `std::chrono::steady_clock` and tracks delta seconds, total seconds, and frame index.

## Logging

`src/engine/core/Logger.*` writes timestamped console logs with info, warning, and error levels. It has no external dependencies.

## Config

`src/engine/core/Config.*` parses command-line options into `AppConfig`. Important flags:

- `--smoke-test`
- `--frames <count>`
- `--headless`
- `--renderer <auto|null|gdi|dx11>`
- `--width <pixels>`
- `--height <pixels>`
- `--assets <path>`

## File System

`src/engine/core/FileSystem.*` contains small path helpers. Asset paths default to `assets`.

## Renderer Interface

`src/engine/renderer/Renderer.h` defines `IRenderer`.

Backends:

- `NullRenderer`: headless fallback used by smoke tests and non-windowed validation.
- `GdiRenderer`: Windows fallback renderer that clears a window and draws a debug grid, axes, and simple primitive.
- `Dx11Renderer`: Windows DirectX 11 renderer that clears the swap chain and draws a minimal debug grid, axes, and primitive.

Renderer selection defaults to `auto`, which attempts DirectX 11 on Windows when a native window exists. If the primary renderer fails during startup, the application attempts the GDI fallback before giving up.

## Game Layer

`src/game/SandboxLayer.*` is the first game-facing layer. It attaches to the app loop and requests the debug grid/axes render pass. Future gameplay prototypes should extend this layer or split into focused game modules.

## Tools and Scripts

- `scripts/doctor.ps1`: environment and structure check.
- `scripts/configure.ps1`: one-command CMake configure.
- `scripts/build.ps1`: one-command build.
- `scripts/verify.ps1`: doctor, configure, build, tests, smoke run.
- `scripts/run.ps1`: run the app.
- `tools/status_report.py`: compact status report for AI agents.
