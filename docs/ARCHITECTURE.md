# Architecture

The project is a small custom C++20 runtime with explicit boundaries between application, platform, renderer, and game prototype code.

## Application

`src/engine/application/Application.*` owns the runtime loop. It initializes engine systems, creates a platform window when requested, creates a renderer backend, gathers input, calls the game layer, updates debug window text, handles frame limits, and shuts everything down in order.

`src/engine/application/Engine.*` is the root engine object for foundational systems. In v0.1 it owns the clock and startup/shutdown lifecycle.

## Platform / Window

`src/engine/platform/Window.h` defines `IWindow`.

On Windows, `src/engine/platform/Win32Window.cpp` provides a small Win32 window implementation. It also owns the current input snapshot and title updates. Non-Windows builds can still use headless smoke mode through the null renderer path.

## Input

`src/engine/input/Input.h` defines `InputState` for v0.2:

- movement axes,
- sprint,
- jump,
- camera yaw/pitch deltas,
- mouse deltas,
- quit.

The Win32 layer maps `W/A/S/D`, `Shift`, `Space`, `Esc`, mouse movement, and arrow-key camera fallback into this state. Mouse movement is safe window-hover delta, not captured or locked.

## Math

`src/engine/math/Math.h` contains the intentionally small math foundation: `Vec2`, `Vec3`, basic operators, dot/cross/length/normalize, clamp/lerp, radians/degrees, yaw helpers, and exponential smoothing.

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

v0.2 adds debug primitive drawing to the renderer interface: debug camera, lines, boxes, grid/axes, and best-effort text. This is not a mesh/material pipeline.

## Game Layer

`src/game/SandboxLayer.*` is the first game-facing layer. It owns the v0.2 prototype scene, player controller, and third-person camera.

`src/game/PlayerController.*` implements deterministic camera-relative movement, sprint, jump, gravity, grounded state, facing yaw, and simple obstacle push-out.

`src/game/ThirdPersonCamera.*` implements yaw/pitch orbit, distance, height offset, pitch clamp, and exponential follow smoothing. The update order is player first, camera second, render third to avoid frame-order jitter.

`src/game/TestScene.*` defines the tiny neutral debug scene with floor/grid and a few obstacle boxes.

## Tools and Scripts

- `scripts/doctor.ps1`: environment and structure check.
- `scripts/configure.ps1`: one-command CMake configure.
- `scripts/build.ps1`: one-command build.
- `scripts/verify.ps1`: doctor, configure, build, tests, smoke run.
- `scripts/run.ps1`: run the app.
- `tools/status_report.py`: compact status report for AI agents.
