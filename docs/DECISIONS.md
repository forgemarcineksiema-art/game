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

## v0.4.1 Game Direction

Decision: Lock the project direction as a small third-person cinematic systemic sandbox/adventure set in the fictional isolated island region of Veyra Reach, with the player as an outsider/fixer and the world remembering meaningful actions.

Reason: The engine needs a concrete target before v0.5. A compact, remembered-action adventure gives clear priorities for movement, camera, collision, interaction, world state, and debug tooling without promising a huge open world.

## v0.4.1 Focus Policy

Decision: Keep interaction focus player-facing first, with close proximity fallback, and document camera-facing as a future assist rather than changing behavior in v0.4.1.

Reason: Player-facing focus is stable for action objects and matches the current avatar-oriented prototype. Camera-facing focus may be useful for inspection/read interactions, but it should be introduced only when manual tests prove the need.

## v0.4.1 Naming Direction

Decision: Keep `TestWorld` and `TestScene` for now, but plan to rename them to `PrototypeWorld` and `PrototypeScene` when the vertical slice replaces the pure test layout.

Reason: Renaming today would create churn without behavior value. A later focused rename makes sense once the scene is no longer only a testbed.

## v0.5 Direction

Decision: Choose traversal before vehicles for v0.5.

Reason: The first vertical slice needs on-foot access gating, readable routes, and stable camera/collision in tight spaces. Vehicles would force larger environments and more physics/control complexity before the core player-scale loop is strong.

## v0.5 Traversal System Scope

Decision: Add `TraversalSystem` under `src/game` with a single contextual `Vault` affordance type.

Reason: The Ferry Office needs one access-gating traversal action, not a general parkour framework. Keeping it in the game layer preserves the ability to reshape the API as the slice gets clearer.

## v0.5 Traversal Trigger

Decision: Use `Space` as the traversal trigger when a traversal affordance is focused, otherwise keep `Space` as normal jump.

Reason: The chosen mechanic is movement, not object interaction. Prioritizing traversal over jump only while focused gives a contextual movement action without breaking normal jump elsewhere.

## v0.5 Traversal Motion

Decision: Move the player deterministically from traversal start to target using interpolation and a small vertical arc, with no animation system or physics engine.

Reason: v0.5 needs reliable repositioning and tests before animation, IK, or richer collision handling. Deterministic motion is easy to validate and keeps the prototype small.

## v0.5.1 Traversal Start Handoff

Decision: Treat the authored traversal start as a focus/debug marker, but use the player's current position as the runtime traversal start when `Space` activates the affordance.

Reason: The player may press `Space` from anywhere inside the focus radius. Starting from the current position avoids a visible snap to the marker while preserving a fixed, authored landing target for the access gate.

## v0.5.1 Traversal Landing Handoff

Decision: Resolve the traversal target through `TestWorld::resolvePlayer` before returning the player to normal movement.

Reason: The collision system already owns floor/box correction. Reusing it at traversal completion keeps landing grounded, clears velocity, exposes collision debug state, and avoids adding a physics library or new traversal collision solver.

## v0.6 World State in Game Layer

Decision: Add `WorldState` under `src/game` as a local, runtime-only remembered-state/event ledger.

Reason: v0.6 needs to prove that The Ferry Office can remember meaningful actions, but it does not need save/load, missions, inventory, scripting, or a global quest system. Keeping this in the game layer makes the contract easy to test and easy to reshape as the micro-slice becomes clearer.

## v0.6 Scene-Level State Mapping

Decision: Map existing prototype actions to remembered flags in `TestScene` instead of teaching `InteractionSystem` or `TraversalSystem` about Tidebreak-specific world flags.

Reason: Interaction and traversal should stay generic. The scene owns the meaning of actions: Ferry Manifest means `manifestCollected`, Wall Button means `routeOpened`, Maintenance Box means `maintenanceBoxInspected` and `powerRestored`, and Service Barrier Vault completion means `serviceRouteUsed`.

## v0.7 Micro-Slice Without Mission Scripting

Decision: Build The Ferry Office loop inside `TestScene` with explicit objective/completion helpers instead of adding a mission scripting system.

Reason: v0.7 needs one playable debug loop, not a general quest framework. Scene-owned helpers keep completion testable and keep the engine foundation small.

## v0.7 Exit Reached Flag

Decision: Add `exitReached` as a local `WorldState` flag.

Reason: The slice needs a clear final state after the player reaches the Exit Summary Marker. A boolean flag matches the existing remembered-state model and avoids adding objectives, missions, or persistence.

## v0.7 Route State Affects Collision

Decision: Let `routeOpened` enable or disable the named `service-gate` collider through `TestScene`.

Reason: The micro-slice should prove that remembered state can visibly and physically affect the prototype world. Keeping the toggle scene-owned avoids inventing a general dynamic door or entity system too early.

## v0.7 Naming Cleanup Deferred

Decision: Do not rename `TestWorld` and `TestScene` during v0.7.

Reason: They are increasingly prototype-scene boundaries, but a rename would create broad file churn while the slice behavior is still being validated. Revisit in v0.7.1 after playtest polish.

## v0.7.1 Service Gate Latches Open

Decision: Treat the Ferry Office Wall Button as a one-way service-gate opener that records `routeOpened=true` and leaves the gate open.

Reason: The previous open/close toggle could create confusing debug state and risk closing a blocking collider on or near the player. A latch-open route is the smallest safe fix for the current micro-slice and avoids building a general dynamic door system.

## v0.7.1 Traversal Marker Placement

Decision: Move the Service Barrier Vault start to the player-accessible side of the barrier and keep the Maintenance Box out of focus until after the vault landing.

Reason: The v0.7 layout allowed the player to reach or focus maintenance before proving `serviceRouteUsed`. The polish pass should make the intended loop readable without adding new traversal types or mission scripting.

## v0.7.1 Debug Text Readability

Decision: Split the sandbox debug text into multiple sections and render GDI debug text with `DrawTextA` instead of a single `TextOutA` line.

Reason: The v0.7 debug string was too long for readable playtesting. Multi-line debug text preserves the no-UI-framework rule while making objective, focus, world state, and slice completion easier to read.

## v0.7.1 Naming Cleanup Deferred

Decision: Keep `TestWorld` and `TestScene` names during v0.7.1.

Reason: The polish work found behavior clarity issues that were more important than rename churn. A focused PrototypeWorld/PrototypeScene cleanup can be a later goal if the next milestone needs clearer scene naming.
