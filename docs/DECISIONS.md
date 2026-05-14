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

## v0.3 Static AABB World

Decision: Add `src/game/TestWorld.*` under the game prototype layer instead of creating a broad engine physics module.

Reason: v0.3 needs a stable collision playground, not a general-purpose physics engine. Keeping it in `src/game` leaves room to promote the boundary later after the API proves itself.

## v0.3 Player Collision Boundary

Decision: `PlayerController` calculates movement and vertical intent, then asks `TestWorld` to resolve the player proxy against static colliders and floor height.

Reason: This removes raw obstacle ownership from the controller while keeping the code simple and deterministic. It also makes the world collision path testable without creating a full ECS or scene system.

## v0.3 Raycast Scope

Decision: Add a simple AABB raycast query for static colliders only.

Reason: Future camera obstruction and interaction checks need a query shape. A single static raycast is useful groundwork without adding camera collision, interaction systems, or physics dependencies in v0.3.

## v0.3 Collision Limitations

Decision: Do not add slopes, ramps, moving colliders, swept collision, rigid bodies, or a physics dependency.

Reason: The goal is a small world/collision prototype. The current horizontal push-out is enough to stop walking through basic debug walls and boxes while keeping future work understandable.

## v0.4 Interaction System in Game Layer

Decision: Add `InteractionSystem` under `src/game` instead of promoting it to a broad engine module.

Reason: v0.4 is proving gameplay plumbing, not a reusable gameplay framework. Keeping it in the prototype layer makes it easy to change as future doors, pickups, vehicles, dialogue, and mission triggers clarify the real requirements.

## v0.4 Pressed-Edge Input

Decision: Track `E` as both `interactHeld` and `interactPressed` in the Win32 input snapshot.

Reason: Interactions should trigger once per key press, while future systems may still need held-state information. This keeps direct Win32 key checks out of gameplay code.

## v0.4 Deferred Gameplay Systems

Decision: Do not add scripting, inventory, dialogue trees, mission state, a UI framework, or physics-driven doors.

Reason: The milestone only needs focus detection, action execution, debug visibility, and testable behavior. Larger systems would hide whether the basic interaction contract is stable.

## v0.4 Dependencies

Decision: Add no new third-party dependencies.

Reason: Standard C++ and the existing debug renderer are enough for interaction data, tests, logging, and marker visualization.
