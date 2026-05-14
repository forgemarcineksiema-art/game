# Technical Decisions

## CMake Build System

Decision: Use CMake with `CMakePresets.json`.

Reason: CMake is a common C++ build system, works well on Windows, supports Visual Studio generators, and gives future AI runs predictable configure/build/test commands.

## C++20

Decision: Use C++20.

Reason: C++20 is modern enough for clean standard-library code while still being widely supported by current Windows toolchains.

## No vcpkg Dependencies in v0.1

Decision: Do not add vcpkg manifest dependencies yet.

Reason: v0.1 does not need external libraries. Avoiding dependencies keeps the foundation easier to configure and debug. vcpkg was also not found in PATH during initial inspection.

## Renderer Strategy

Decision: Add an `IRenderer` abstraction with DirectX 11, Windows GDI fallback, and headless null rendering.

Reason: DirectX 11 is the requested Windows-first renderer target and is available through the installed Visual Studio/Windows SDK toolchain. The v0.1 DX11 backend intentionally stays tiny: swap chain clear, simple shaders, debug grid/axes, and one primitive. A GDI fallback keeps windowed rendering available if DX11 startup fails, and the null renderer gives reliable headless smoke tests.

Dependencies used for DX11:

- Windows SDK `d3d11`: Direct3D 11 device, context, swap chain, and draw calls.
- Windows SDK `dxgi`: swap chain support.
- Windows SDK `d3dcompiler`: compile the tiny built-in smoke-test shaders.

These are platform SDK libraries, not third-party dependencies.

## No Full Game Engine

Decision: Do not use Unity, Unreal, Godot, or another existing full engine.

Reason: The project goal is to build a custom engine/workbench that future Codex runs can extend safely.

## No Heavy Test Framework

Decision: Use a small standalone C++ test executable instead of adding a test dependency.

Reason: v0.1 only needs lightweight validation for config, paths, time, and null renderer behavior. A dependency would be overkill at this stage.

## Deferred Systems

Deferred until later milestones:

- DirectX 11 resource management beyond the smoke path.
- Shader file loading or hot reload.
- Physics.
- Animation.
- Audio.
- Scripting.
- Networking.
- Asset import pipeline.
- NPC AI.
- Mission tooling.

## v0.2 Minimal Math Instead of a Library

Decision: Add `src/engine/math/Math.h` with only the vector and helper operations needed by the player/camera prototype.

Reason: v0.2 needs deterministic movement, yaw/pitch camera math, and debug projection. A full math dependency would be overkill and would add setup risk before the engine needs matrices, quaternions, or SIMD.

## v0.2 Input Snapshot

Decision: Extend the existing Win32 window path to provide an `InputState` snapshot instead of adding a global input manager.

Reason: The prototype only needs movement axes, sprint, jump, mouse deltas, arrow fallback, and quit. Keeping input inside the window boundary avoids spreading direct Win32 key checks through game code.

## v0.2 Mouse-Look Scope

Decision: Use safe mouse movement over the window plus arrow-key fallback. Do not capture, hide, or lock the cursor yet.

Reason: Cursor capture is easy to get wrong in short smoke runs and remote/VM sessions. v0.2 proves camera orbit and player feel without risking a hostile development loop.

## v0.2 Debug Rendering Scope

Decision: Extend `IRenderer` with simple debug lines, boxes, camera, and text instead of adding meshes/materials.

Reason: The goal is player/camera feel. A mesh pipeline, material system, or UI layer would be premature.

## v0.2 Dependencies

Decision: Add no new third-party dependencies.

Reason: Standard C++ and the existing Windows SDK path are enough for the prototype.
