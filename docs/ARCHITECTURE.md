# Architecture

The project is a small custom C++20 runtime with explicit boundaries between application, platform, renderer, and game prototype code.

## Application

`src/engine/application/Application.*` owns the runtime loop. It initializes engine systems, creates a platform window when requested, creates a renderer backend, gathers input, calls the game layer, updates debug window text, handles frame limits, and shuts everything down in order.

`src/engine/application/Engine.*` is the root engine object for foundational systems. It owns the clock and startup/shutdown lifecycle.

## Platform / Window

`src/engine/platform/Window.h` defines `IWindow`.

On Windows, `src/engine/platform/Win32Window.cpp` provides a small Win32 window implementation. It also owns the current input snapshot and title updates. Non-Windows builds can still use headless smoke mode through the null renderer path.

## Input

`src/engine/input/Input.h` defines `InputState` for the current prototype:

- movement axes,
- sprint,
- jump,
- interact held/pressed edge,
- camera yaw/pitch deltas,
- mouse deltas,
- quit.

The Win32 layer maps `W/A/S/D`, `Shift`, `Space`, `E`, `Esc`, mouse movement, and arrow-key camera fallback into this state. `E` is tracked as both held and pressed-edge state so gameplay code can trigger one interaction per key press. Mouse movement is safe window-hover delta, not captured or locked.

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

`src/game/SandboxLayer.*` is the first game-facing layer. It owns the prototype scene, player controller, and third-person camera.

`src/game/PlayerController.*` implements deterministic camera-relative movement, sprint, jump, gravity, grounded state, facing yaw, and world-collision integration. It no longer owns static obstacle lists directly.

`src/game/ThirdPersonCamera.*` implements yaw/pitch orbit, distance, height offset, pitch clamp, and exponential follow smoothing. The update order is player first, camera second, render third to avoid frame-order jitter.

`src/game/TestWorld.*` is the v0.3 static world/collision boundary. It owns named static AABB colliders, a floor height, player proxy resolution, overlap checks, ground checks, and a simple raycast query for future camera obstruction or interaction work.

`src/game/InteractionSystem.*` is the v0.4 gameplay interaction boundary. It owns lightweight interactable data, selects the best focus candidate from player position/facing/range, and executes simple built-in actions: pickup, toggle, and info. It deliberately has no scripting, inventory, dialogue tree, mission state, or UI framework.

`src/game/TraversalSystem.*` is the v0.5 traversal affordance boundary. It owns narrow, game-layer traversal affordances with start/end positions, focus radius, facing requirement, prompt, enabled flag, and duration. It produces an activation request when `Space` is pressed while an affordance is focused.

`src/game/WorldState.*` is the v0.6 local remembered-state boundary for the prototype scene. It owns boolean flags, deterministic event records, event ids, last-event text, and compact debug summary text. It is runtime-only and does not implement save/load, mission scripting, inventory, dialogue, or persistence.

`src/game/TestScene.*` defines the tiny neutral debug scene. It asks `TestWorld` to build the collision test layout: floor, wall, narrow passage, corner case, low step-like blocker, and crate. It also owns four debug interactables: a one-shot Ferry Manifest pickup, a repeatable wall button/toggle, a repeatable info marker, and a maintenance box.

`src/game/TestScene.*` also owns the first v0.5 traversal affordance: a service-barrier vault path used as access-gating groundwork for The Ferry Office.

`src/game/TestScene.*` maps prototype gameplay results to remembered state: Ferry Manifest sets `manifestCollected`, Wall Button sets `routeOpened`, Maintenance Box sets `maintenanceBoxInspected` and `powerRestored`, and Service Barrier Vault completion sets `serviceRouteUsed`. This mapping stays in the scene layer so `InteractionSystem` and `TraversalSystem` remain generic.

`src/game/SandboxLayer.*` integrates player/camera/world/traversal/interaction update order. It updates traversal focus first from the current player position/facing, gives a focused traversal activation to the player, updates interaction focus from the corrected player position/facing, executes `E` interactions, updates the camera, then renders world collision, traversal, and interaction debug markers.

In v0.5.1, traversal activation uses the player's current position as the runtime traversal start while keeping the authored affordance target fixed. This avoids a visible snap to the start marker when the player presses `Space` inside the focus radius. The authored start marker remains a focus/debug marker, not a mandatory teleport point.

Traversal completion now resolves the authored target through `TestWorld::resolvePlayer` before returning the controller to normal movement. This keeps landing grounded, clears velocity, and lets the existing static collision path correct a blocked target instead of leaving the player inside a collider.

Collision support is intentionally primitive:

- static axis-aligned boxes,
- floor height query,
- vertical player cylinder/capsule approximation represented by radius and height,
- horizontal push-out,
- simple raycast against AABBs.

No physics engine, rigid bodies, slopes, ramps, moving platforms, or terrain streaming are present.

Interaction support is intentionally primitive:

- point/radius focus volumes,
- nearest candidate with a simple facing preference,
- built-in pickup/toggle/info actions,
- debug text/log prompts instead of a UI layer.

No scripting, inventory, doors with collision changes, dialogue, mission triggers, save/load, or persistent save data are present.

World state support is intentionally primitive:

- boolean local flags only,
- deterministic in-memory event records,
- event id/order/count,
- debug summary text and logs,
- scene-level mapping from known prototype actions to flags.

No save/load, mission scripting, dialogue trees, global quest graph, inventory, or persistence exists.

Traversal support is intentionally primitive:

- one affordance type, `Vault`,
- point/radius focus around the traversal start,
- player-facing requirement with close proximity tolerance,
- `Space` trigger only when focused,
- deterministic position interpolation from current player position to the target with a small arc,
- world collision resolve at landing,
- no animation system, IK, full parkour, ledge hanging, or physics engine.

## Interaction Focus Design Notes

Current v0.4.1 behavior uses player-facing plus a close proximity fallback:

- Player-facing is stable for a third-person avatar because focus follows where the character is meaningfully oriented.
- Camera-facing can feel better when inspecting objects without moving, but it can also focus objects behind or beside the avatar unless carefully filtered.
- Proximity fallback keeps very close objects usable even when the facing vector is imperfect.

For the first vertical slice, keep player-facing as the default and add camera-facing only if manual tests show repeated frustration. A good future design is likely:

1. close proximity always allowed,
2. player-facing preferred for action objects,
3. camera-facing or raycast assist for inspect/read objects,
4. collision raycast used only when occlusion becomes a real problem.

## Naming Direction

`TestWorld` and `TestScene` should remain unchanged in v0.4.1 because they still represent a test layout. When v0.5 or the vertical slice turns this into a reusable prototype scene, rename them to `PrototypeWorld` and `PrototypeScene` in one focused refactor with tests and docs updated together.

## Tools and Scripts

- `scripts/doctor.ps1`: environment and structure check.
- `scripts/configure.ps1`: one-command CMake configure.
- `scripts/build.ps1`: one-command build.
- `scripts/verify.ps1`: doctor, configure, build, tests, smoke run.
- `scripts/run.ps1`: run the app.
- `tools/status_report.py`: compact status report for AI agents.
