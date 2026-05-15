# Architecture

The project is a small custom C++20 runtime with explicit boundaries between application, platform, renderer, and game prototype code.

## Application

`src/engine/application/Application.*` owns the runtime loop. It initializes engine systems, creates a platform window when requested, creates a renderer backend, gathers input, calls the game layer, updates debug window text, handles frame limits, and shuts everything down in order.

`src/engine/application/Engine.*` is the root engine object for foundational systems. It owns the clock and startup/shutdown lifecycle.

## Platform / Window

`src/engine/platform/Window.h` defines `IWindow`.

On Windows, `src/engine/platform/Win32Window.cpp` provides a small Win32 window implementation. It also owns the current input snapshot, cursor capture/hide behavior, relative mouse recentering for camera control, and title updates. Non-Windows builds can still use headless smoke mode through the null renderer path.

## Input

`src/engine/input/Input.h` defines `InputState` for the current prototype:

- movement axes,
- sprint,
- jump,
- interact held/pressed edge,
- camera yaw/pitch deltas,
- mouse deltas,
- quit.

The Win32 layer maps `W/A/S/D`, `Shift`, `Space`, `E`, `Esc`, mouse/touchpad movement, and arrow-key camera fallback into this state. `E` is tracked as both held and pressed-edge state so gameplay code can trigger one interaction per key press. Windowed play captures and hides the cursor by default, confines it to the client area while focused, recenters it for relative mouse deltas, and restores it on focus loss or shutdown. `--free-cursor` / `--show-cursor` keep the old visible-cursor behavior for debugging.

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
- `--capture-cursor`
- `--free-cursor`
- `--show-cursor`
- `--width <pixels>`
- `--height <pixels>`
- `--assets <path>`

## File System

`src/engine/core/FileSystem.*` contains small path helpers. Asset paths default to `assets`.

## Scene Data

v0.11 adds the first explicit scene/object authoring data:

```text
data/scenes/ferry_office.scene.json
```

This file mirrors the current Ferry Office and service-yard prototype: units, floor height, player start, scale references, static colliders, visual placeholders, interactables, traversal affordance, vehicle spawn/bounds, route markers, and objective markers.

Runtime loading is intentionally deferred. The active game still uses `PrototypeWorld`, `PrototypeScene`, `FerryOfficeData`, and `SandboxLayer` C++ setup. Until a loader or generator exists, changes to authored layout must keep the JSON and matching C++ constants synchronized.

Scene tools live under `tools`:

- `scene_data.py`: shared loader/validation helpers.
- `validate_scene.py`: hard validation for required ids, numeric vectors, uniqueness, radii/extents, traversal data, vehicle bounds, and scale sanity.
- `scene_report.py`: compact Codex-readable scene summary.
- `scale_audit.py`: suspicious-scale report for object sizes and vehicle dimensions.
- `mesh_report.py`: static mesh asset/reference summary.

v0.12 extends the scene file with `meshAssets` and `meshInstances`. These are validated authoring/runtime-candidate entries, not a full asset registry. v0.12.1 grows the Ferry Office mirror to 10 mesh instances for roof/facade/sign, service gate, maintenance box, dock props, service-yard crate, and vehicle body/cabin. Runtime still mirrors the important mesh instances explicitly in `SandboxLayer` until scene loading or code generation exists.

## Static Mesh Assets

`src/engine/assets/StaticMesh.*` is the first engine-owned static mesh boundary. It defines `StaticMeshAsset`, `StaticMeshVertex`, `StaticMeshInstance`, local bounds, a minimal glTF loader, and a helper that expands indexed mesh data into flat triangle lists for the current immediate renderer path.

The v0.12 loader supports only a tiny `.gltf` subset: one embedded base64 buffer, one primitive with `POSITION` float `VEC3`, and indexed triangle lists. It intentionally does not support materials, textures, node hierarchy, GLB, animation, skinning, mesh collision, or an asset registry.

## Physics

`src/engine/physics/PhysicsWorld.*` defines the first engine-owned physics boundary. It exposes vendor-free types:

- `PhysicsConfig`,
- `BodyHandle`,
- `BoxColliderDesc`,
- `DynamicBoxDesc`,
- `RaycastResult`,
- `PhysicsDebugLine`,
- `IPhysicsWorld`.

The default backend is a tiny dependency-free `simple` world used by normal validation. It supports static boxes, trigger boxes, a dynamic box placeholder, fixed-step updates, raycasts, and debug box line extraction. It is not a production physics engine and should not grow into one.

`src/engine/physics/JoltPhysicsWorld.cpp` is compiled only when `ENGINE_ENABLE_JOLT_PHYSICS=ON`. It keeps Jolt headers and `JPH::*` types private to `EngineCore`. Game code must not include Jolt headers or store Jolt handles directly.

The opt-in preset `windows-vs2022-debug-jolt` proves that Jolt can initialize/shutdown, create static boxes, support raycasts through the engine interface, and build with the current Windows toolchain. The normal `windows-vs2022-debug` preset remains dependency-free for reliable everyday validation.

`src/game/PrototypeWorld.*` remains the active Ferry Office authoring and collision owner for now. A later migration should mirror or move one tested behavior at a time into `engine::physics`; v0.9.2 deliberately does not rewrite player movement, traversal landing, or service-gate collision around Jolt.

## Renderer Interface

`src/engine/renderer/Renderer.h` defines `IRenderer`.

Backends:

- `NullRenderer`: headless fallback used by smoke tests and non-windowed validation.
- `GdiRenderer`: Windows fallback renderer that clears a window and draws a debug grid, axes, and simple primitive.
- `Dx11Renderer`: Windows DirectX 11 renderer that clears the swap chain and draws a minimal debug grid, axes, and primitive.

Renderer selection defaults to `auto`, which attempts DirectX 11 on Windows when a native window exists. If the primary renderer fails during startup, the application attempts the GDI fallback before giving up.

v0.2 adds debug primitive drawing to the renderer interface: debug camera, lines, boxes, grid/axes, and best-effort text. This is not a mesh/material pipeline.

v0.12 adds `IRenderer::drawDebugFlatTriangles` as a narrow immediate-mode static mesh submission path. DX11 and GDI use the same CPU debug projection model as solid boxes; the null renderer accepts the call for smoke/test safety. v0.12.1 uses this path for a small unit-box prop replacement pass only. This is still not a real material or GPU mesh-resource system.

## Game Layer

`src/game/SandboxLayer.*` is the first game-facing layer. It owns the prototype scene, player controller, third-person camera, and the current placeholder service-yard vehicle spike.

`src/game/PlayerController.*` implements deterministic camera-relative movement, sprint, jump, gravity, grounded state, facing yaw, and world-collision integration. It no longer owns static obstacle lists directly.

`src/game/ThirdPersonCamera.*` implements yaw/pitch orbit, distance, height offset, pitch clamp, and exponential follow smoothing. The update order is player first, camera second, render third to avoid frame-order jitter.

`src/game/VehicleController.*` is the v0.10 narrow vehicle feel spike, tuned in v0.13 for the compact service-yard road test. It owns a deterministic arcade-style placeholder vehicle state: position, yaw, speed, velocity, throttle/brake/steer telemetry, occupied state, focus range, pressed-edge enter/exit, safe exit checks, drag/coast behavior, and simple yard-bound clamping. It deliberately does not expose Jolt, vehicle constraints, wheels, suspension, damage, doors, seats, cargo, traffic, or tuning data files.

`src/game/PrototypeWorld.*` is the v0.3 static world/collision boundary. It owns named static AABB colliders, a floor height, player proxy resolution, overlap checks, ground checks, and a simple raycast query for future camera obstruction or interaction work. v0.7 adds a Ferry Office prototype layout builder and lets the scene toggle whether a named collider blocks the player.

`src/game/InteractionSystem.*` is the v0.4 gameplay interaction boundary. It owns lightweight interactable data, selects the best focus candidate from player position/facing/range, and executes simple built-in actions: pickup, toggle, and info. It deliberately has no scripting, inventory, dialogue tree, mission state, or UI framework.

`src/game/TraversalSystem.*` is the v0.5 traversal affordance boundary. It owns narrow, game-layer traversal affordances with start/end positions, focus radius, facing requirement, prompt, enabled flag, and duration. It produces an activation request when `Space` is pressed while an affordance is focused.

`src/game/WorldState.*` is the v0.6 local remembered-state boundary for the prototype scene. It owns boolean flags, deterministic event records, event ids, last-event text, and compact debug summary text. It is runtime-only and does not implement save/load, mission scripting, inventory, dialogue, or persistence.

`src/game/PrototypeScene.*` defines the Ferry Office debug micro-slice. It asks `PrototypeWorld` to build a dock/ferry-office prototype layout with a closed service gate, a service barrier, office walls, dock rails, and maintenance-side blockers. It owns five debug interactables: a one-shot Ferry Manifest pickup, a repeatable Wall Button service-gate opener, a repeatable Ferry Office Notice, a Maintenance Box, and an Exit Summary Marker.

`src/game/PrototypeScene.*` also owns the first v0.5 traversal affordance: a service-barrier vault path used as access-gating groundwork for The Ferry Office.

`src/game/FerryOfficeData.*` centralizes the Ferry Office prototype's stable debug names, prompts, messages, important marker positions, radii, and traversal tuning constants. It is not a content pipeline; it is a small data cleanup so scene setup, state mapping, debug rendering, and tests do not repeat string ids and coordinates.

`data/scenes/ferry_office.scene.json` is now the Codex-facing scene mirror for those positions and ids. It should become the source of truth in a later goal, but v0.11 deliberately keeps it as a validated authoring snapshot so gameplay behavior stays unchanged.

`src/game/PrototypeScene.*` maps prototype gameplay results to remembered state: Ferry Manifest sets `manifestCollected`, Wall Button sets `routeOpened=true`, Maintenance Box sets `maintenanceBoxInspected` and `powerRestored`, Service Barrier Vault completion sets `serviceRouteUsed`, and the Exit Summary Marker can set `exitReached` only after the required loop is ready. This mapping stays in the scene layer so `InteractionSystem` and `TraversalSystem` remain generic.

`src/game/PrototypeScene.*` also exposes slice guidance helpers: current objective text, ready-for-exit status, completion status, completion summary text, and service-gate blocking state. `routeOpened` is synchronized to the named `service-gate` collider so the gate is not only visual in the prototype. v0.7.1 makes the Wall Button latch the route open instead of closing it again, which avoids trapping the player in the gate volume.

`src/game/SandboxLayer.*` integrates player/camera/world/traversal/interaction update order. On foot, it updates traversal focus first from the current player position/facing, gives a focused traversal activation to the player, updates interaction focus from the corrected player position/facing, lets the vehicle take `E` only when no Ferry Office interactable is focused, executes normal `E` interactions, updates the camera, then renders world collision, traversal, interaction, world-state, vehicle, and Ferry Office slice debug markers.

When the vehicle is occupied, `SandboxLayer` skips on-foot player movement, traversal activation, and Ferry Office interactions for that frame. `W/S/A/D` drive the vehicle, `E` exits only when the computed side exit position is clear, and the existing third-person camera follows the vehicle target with separate distance/height/smoothing settings. Exiting places the player beside the vehicle and returns the next frame to the normal on-foot flow.

v0.13 keeps service-yard vehicle placement explicit but reduces local drift by grouping spawn, bounds, pad, rail, back-stop, crate, body, and cabin constants at the top of `SandboxLayer.cpp`. These constants still mirror `data/scenes/ferry_office.scene.json`; they are not yet loaded from scene data at runtime.

In v0.5.1, traversal activation uses the player's current position as the runtime traversal start while keeping the authored affordance target fixed. This avoids a visible snap to the start marker when the player presses `Space` inside the focus radius. The authored start marker remains a focus/debug marker, not a mandatory teleport point.

Traversal completion now resolves the authored target through `PrototypeWorld::resolvePlayer` before returning the controller to normal movement. This keeps landing grounded, clears velocity, and lets the existing static collision path correct a blocked target instead of leaving the player inside a collider.

Collision support is intentionally primitive:

- static axis-aligned boxes,
- floor height query,
- vertical player cylinder/capsule approximation represented by radius and height,
- horizontal push-out,
- simple raycast against AABBs.

No physics engine, rigid bodies, slopes, ramps, moving platforms, or terrain streaming are present.

v0.9.2 adds an engine physics boundary and an opt-in Jolt backend spike, but the Ferry Office runtime still uses the existing prototype collision path. v0.10 uses the engine-owned `engine::physics` API non-invasively for a simple service-yard validation/debug world while keeping the live vehicle controller deterministic. Treat this as a dependency and architecture foundation, not as a completed gameplay physics migration.

Interaction support is intentionally primitive:

- point/radius focus volumes,
- nearest candidate with a simple facing preference,
- built-in pickup/toggle/info actions,
- debug text/log prompts instead of a UI layer.

No scripting, inventory, general door framework, dialogue, mission triggers, save/load, or persistent save data are present. The slice has one scene-owned service-gate collider latch to prove the route-open state can affect the world.

World state support is intentionally primitive:

- boolean local flags only,
- deterministic in-memory event records,
- event id/order/count,
- debug summary text and logs,
- scene-level mapping from known prototype actions to flags.

Current Ferry Office flags are `powerRestored`, `manifestCollected`, `serviceRouteUsed`, `maintenanceBoxInspected`, `routeOpened`, and `exitReached`.

No save/load, mission scripting, dialogue trees, global quest graph, inventory, or persistence exists.

Traversal support is intentionally primitive:

- one affordance type, `Vault`,
- point/radius focus around the traversal start,
- player-facing requirement with close proximity tolerance,
- `Space` trigger only when focused,
- deterministic position interpolation from current player position to the target with a small arc,
- world collision resolve at landing,
- no animation system, IK, full parkour, ledge hanging, or physics engine.

## Visual Debug Presentation

v0.9 adds a narrow solid debug drawing path, `IRenderer::drawDebugSolidBox`, implemented by the null, GDI, and DX11 renderers. It exists to make prototype scenes read as simple places before a real mesh/material pipeline exists.

`SandboxLayer` now draws muted solid placeholder slabs and boxes for the Ferry Office dock, service yard, office walls, gate, traversal markers, interactables, and player proxy, then draws the existing wire/debug outlines on top. This keeps collision and state visibility intact while reducing the pure wireframe workbench feeling.

v0.10 adds a service-yard driving pad, a placeholder vehicle body/cabin, vehicle heading line, enter radius, safe exit marker, yard bounds, and physics debug lines. These are debug primitives only; there is still no vehicle mesh, material system, tire model, or final road art.

This is not an asset pipeline. There are still no textures, mesh loading, materials, lighting, shadows, post-processing, or scene serialization. Solid debug geometry should stay simple and disposable until a later art/asset milestone proves what the engine actually needs.

`docs/ASSET_GUIDE.md`, `docs/SCENE_AUTHORING.md`, and `docs/ART_DIRECTION.md` define the first scale, naming, Blender/glTF direction, scene editing workflow, and coastal-industrial mood target. These guide future asset/model work but do not add runtime model loading yet.

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

v0.8 renamed the stale `TestWorld` / `TestScene` boundaries to `PrototypeWorld` / `PrototypeScene`. The names now match their role: they are still hardcoded prototype-game scaffolding, but they are no longer described as temporary unit-test-only objects.

## Tools and Scripts

- `scripts/doctor.ps1`: environment and structure check.
- `scripts/configure.ps1`: one-command CMake configure.
- `scripts/build.ps1`: one-command build.
- `scripts/verify.ps1`: doctor, configure, build, tests, smoke run.
- `scripts/run.ps1`: run the app.
- `tools/status_report.py`: compact status report for AI agents.
- `tools/scene_report.py`: scene object/count/route/vehicle summary.
- `tools/validate_scene.py`: scene data validation gate.
- `tools/scale_audit.py`: scene scale sanity report.
