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

Decision: Resolve the traversal target through `PrototypeWorld::resolvePlayer` before returning the player to normal movement.

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

## v0.8 Prototype Scene Naming

Decision: Rename `TestWorld` / `TestScene` to `PrototypeWorld` / `PrototypeScene`.

Reason: After v0.7.1, these classes are no longer unit-test-like experiments; they are the active prototype world and micro-slice scene boundaries. A focused cleanup goal can absorb the rename churn while tests prove the Ferry Office behavior is unchanged.

## v0.8 Ferry Office Data

Decision: Add `FerryOfficeData` under `src/game` for stable prototype names, prompts, messages, positions, radii, and traversal constants instead of introducing a scene file format.

Reason: v0.8 is cleanup, not a content pipeline milestone. A small C++ data boundary removes repeated strings and coordinates while keeping the current prototype easy to compile, test, and refactor later.

## v0.8.1 Cursor Capture

Decision: Capture and hide the cursor by default during Win32 windowed play, using client-area confinement plus recentering for relative mouse/touchpad deltas. Add `--free-cursor` / `--show-cursor` for visible-cursor debugging and keep `Esc` as quit.

Reason: The visible desktop cursor was distracting during play, especially on a laptop/touchpad. Keeping this in the Win32 window boundary improves camera comfort without touching gameplay systems, adding dependencies, or changing smoke/headless behavior.

## v0.9 Solid Debug Presentation

Decision: Add `IRenderer::drawDebugSolidBox` to the existing renderer abstraction and use it for the Ferry Office dock, service yard, office, gate, traversal, interaction, and player placeholders.

Reason: The prototype needed to read more like a place without jumping to a real asset pipeline, materials, textures, model loading, lighting, or final art. A narrow solid debug primitive lets GDI and DX11 share the same scene intent while keeping wire/debug outlines visible and preserving all existing gameplay systems.

## v0.9 Tidebreak Window Title

Decision: Change the default window title from `AI-Native Engine Foundation` to `Tidebreak Prototype`.

Reason: v0.4.1 locked the project direction as Tidebreak, and v0.9 focuses on playtest readability. The runtime should identify the playable prototype rather than only the engine foundation.

## v0.9.1 Route Polyline and Marker Hierarchy

Decision: Keep the Ferry Office readability polish inside `SandboxLayer` by drawing a low route polyline through the intended loop and giving known prototype markers slightly different sizes, colors, and beacon heights.

Reason: The v0.9 scene read more like a place, but the action path was still visually ambiguous. The route polyline and marker hierarchy improve readability without adding UI, mission scripting, new entities, asset loading, or renderer architecture.

## v0.9.1 Objective-First Debug Text

Decision: Reorder the GDI debug text so objective and focused prompt appear before player/camera statistics.

Reason: During visual review, the most important playtest question was "where do I go next?" Objective-first debug text answers that faster while preserving all existing state/debug details for validation.

## v0.9.2 Physics Candidate

Decision: Choose Jolt Physics as Tidebreak's default production physics candidate, keep PhysX as a backup candidate, and do not choose Bullet unless a later test gives a strong reason.

Reason: Tidebreak needs game-focused rigid bodies, queries, triggers/sensors, character simulation, and future vehicles. Jolt is MIT licensed, modern C++, CMake-friendly, and aligned with custom engine work. PhysX is credible but heavier and more tied to the NVIDIA/Omniverse ecosystem. Bullet is permissive and proven but less compelling for a fresh C++20 game-engine foundation.

## v0.9.2 Vendor-Safe Physics Boundary

Decision: Add `src/engine/physics` with engine-owned types and keep Jolt private to `EngineCore`.

Reason: Choosing Jolt early should not mean allowing `JPH::*` types to spread through `src/game`. The gameplay code should talk to Tidebreak's own `IPhysicsWorld`, `BodyHandle`, box descriptors, raycast results, and debug lines so future migration or backend testing remains possible.

## v0.9.2 Jolt as Opt-In Backend

Decision: Add `ENGINE_ENABLE_JOLT_PHYSICS` and `windows-vs2022-debug-jolt`, but keep everyday `scripts/verify.ps1` on the dependency-free default preset.

Reason: vcpkg is not currently installed or required, and normal validation must stay reliable. The Jolt preset proves the backend can configure/build/test when explicitly requested without making every future smoke test depend on network/dependency setup.

## v0.9.2 FetchContent for the Spike

Decision: Use pinned CMake `FetchContent` tag `v5.5.0` for the Jolt spike instead of adding a vcpkg manifest immediately.

Reason: vcpkg is not in PATH in the current environment. A pinned opt-in FetchContent path is enough to prove the dependency and adapter without changing the normal build contract. Revisit vcpkg manifest mode if/when Jolt becomes the default backend for production work.

## v0.9.2 Laptop-Safe Jolt Compile Baseline

Decision: Disable optional SSE4/AVX/FMA/LZCNT/TZCNT Jolt compile flags in the first spike preset and match the dynamic MSVC runtime.

Reason: The user is targeting a weak laptop with integrated/dedicated GPU options. The first physics foundation should avoid unnecessary CPU-instruction assumptions. Matching the MSVC runtime also avoids `/MTd` versus `/MDd` link failures in the Visual Studio debug build.

## v0.10 Deterministic Vehicle Feel Spike

Decision: Implement the first drivable vehicle as a narrow deterministic `src/game/VehicleController` instead of using Jolt VehicleConstraint immediately.

Reason: v0.10 needs to prove enter/exit, steering, throttle/brake/reverse feel, camera follow, and service-yard readability without turning the milestone into a full vehicle framework. Jolt remains the production physics candidate, but wheel constraints, suspension, tire friction, and vehicle tuning are too broad for the first gameplay spike.

## v0.10 Vehicle / Ferry Office Interaction Priority

Decision: Let `E` enter the vehicle only when no Ferry Office interactable is currently focused.

Reason: The Ferry Office micro-slice already depends on `E` for manifest, maintenance, wall button, notice, and exit marker actions. Keeping vehicle entry as a fallback prevents the service-yard spike from stealing core slice interactions.

## v0.10 Physics Boundary Usage

Decision: Use `engine::physics` from `SandboxLayer` for a small service-yard validation/debug world, but keep Jolt private to `src/engine` and keep live vehicle movement deterministic for now.

Reason: The goal is to exercise the engine-owned physics boundary without coupling game code to `JPH::*` or rewriting the prototype collision path. Tests now scan `src/game` for accidental Jolt references.

## v0.11 Scene Data Before More Gameplay

Decision: Add `data/scenes/ferry_office.scene.json` and scene tools before adding more gameplay systems.

Reason: The prototype scene had too many hardcoded positions spread across `FerryOfficeData`, `PrototypeWorld`, `PrototypeScene`, and `SandboxLayer`. A human-readable scene mirror plus validation gives future Codex runs a stable way to inspect object ids, scale, routes, markers, and vehicle placement before runtime loading exists.

## v0.11 Scene Data Mirror, Not Runtime Loader

Decision: Keep the v0.11 scene data as a validated authoring snapshot instead of loading it at runtime.

Reason: Runtime loading would touch gameplay setup, error handling, asset paths, and future serialization decisions. v0.11 is a production-foundation goal; preserving v0.10 gameplay is more important than rushing a partial loader. A later goal can either load the JSON directly or generate C++ data from it.

## v0.11 Python Tools Without New Dependencies

Decision: Build scene report, validation, and scale audit tools with Python standard library only.

Reason: Future Codex runs need dependable commands on the current Windows setup. Adding a schema or CLI dependency would make the workbench more fragile before the data format proves itself.

## v0.11 glTF Deferred To v0.12

Decision: Document Blender/glTF direction but do not add model loading or an asset pipeline in v0.11.

Reason: Scale, naming, scene data, and validation need to be stable before imported meshes enter the renderer. The next narrow goal can test static mesh/glTF rendering against this authoring foundation.

## v0.12 Minimal Custom glTF Subset Loader

Decision: Implement a tiny engine-owned `.gltf` subset loader for original embedded-buffer placeholder assets instead of adding cgltf, tinygltf, or Assimp in v0.12.

Reason: v0.12 only needs to prove static mesh entry, scene references, bounds, and flat triangle rendering for a tiny project-owned asset. Assimp is too broad for this milestone, tinygltf would add a larger JSON/STB dependency surface, and cgltf is attractive but would still introduce third-party dependency management before the renderer has depth, matrices, materials, or real mesh resources. The custom loader is deliberately narrow and documented as a spike: embedded `.gltf`, `POSITION` float `VEC3`, indexed triangle list, no materials/textures/animations. Revisit cgltf or tinygltf when glTF coverage grows beyond this subset.

Dependency impact: no new third-party dependency was added.

## v0.12 Immediate Flat Triangle Rendering

Decision: Add `IRenderer::drawDebugFlatTriangles` rather than a renderer-owned static mesh resource system.

Reason: The current GDI and DX11 renderers are immediate debug renderers using CPU projection. A flat triangle call lets mesh instances render through the same safe path as solid debug boxes while avoiding premature GPU resource lifetime, materials, depth-buffer, shader, and asset registry decisions.

## v0.12 Scene Mesh References Remain A Mirror

Decision: Add `meshAssets` and `meshInstances` to `data/scenes/ferry_office.scene.json`, but keep runtime integration explicit in C++ for now.

Reason: v0.11 scene data is still an authoring mirror. Loading full scene data at runtime would combine two milestones: mesh rendering and scene source-of-truth migration. v0.12 keeps gameplay behavior stable while giving tools and future Codex runs validated mesh references.

## v0.12.1 Unit-Box Prop Replacement Pass

Decision: Reuse the original `assets/models/unit_box.gltf` mesh for the first Ferry Office prop replacement/scale pass instead of adding new mesh files or expanding the glTF loader.

Reason: v0.12.1 is about visual scale, placement, and scene-data consistency. Reusing one known-good original asset lets the scene add a facade cue, sign board, dock bollards, service-yard crate, and vehicle cabin while keeping the tiny loader subset, dependency surface, renderer path, and validation contract stable.

## v0.12.1 Runtime Mesh Mirror Remains Explicit

Decision: Keep v0.12.1 mesh placement mirrored in `SandboxLayer` with a small local helper and a synchronization comment, rather than introducing runtime scene JSON loading.

Reason: A runtime loader would be a separate architecture milestone touching error handling, asset lookup, scene ownership, and gameplay setup. The current goal only needed a safer prop/scale pass; explicit C++ keeps behavior predictable while scene tools validate the authored mirror.

## v0.13 Deterministic Vehicle Tuning Before Physics Migration

Decision: Retune the existing deterministic `VehicleController` for the compact service-yard road test instead of migrating to Jolt VehicleConstraint or adding wheel simulation.

Reason: The current milestone is about clarity, control feel, enter/exit safety, and service-yard readability. Lower forward/reverse speed, smoother acceleration/braking, stronger drag, tighter low-speed steering, and a slightly closer vehicle camera provide a better playtest baseline while preserving the engine-owned physics boundary and avoiding a premature full vehicle physics framework.

## v0.13 Service-Yard Constants Stay Local But Grouped

Decision: Keep service-yard runtime placement explicit in `SandboxLayer.cpp`, but group the vehicle spawn, bounds, pad, rails, back-stop, crate, and vehicle proxy constants near the top of the file and mirror them in `data/scenes/ferry_office.scene.json`.

Reason: v0.13 needs safer iteration without adding runtime scene loading. Grouped constants reduce accidental drift and make future Codex edits easier to audit, while the JSON scene tools continue to validate the authoring mirror.

## Creative Direction - Island Driver/Fixer Sandbox

Decision: Treat Tidebreak as an island-first third-person driver/fixer sandbox where every job leaves a mark on a small coastal community.

Reason: This gives the project a sharper identity than "mini GTA" or pure trucking. Veyra Reach should first become a compact, dense, memorable region with practical jobs, grounded driving, route access, systemic objects, humor, atmosphere, and remembered local consequences. A larger city/mainland region can become a later expansion, but not before the island has strong place, vehicle feel, and world-memory foundations.

Reference filter: learn map density from `Bully`/`Yakuza`, vehicle weight from `Mafia 2`/`GTA IV`, world reactivity from `RDR2`/`GTA IV`, object systems from `Watch Dogs`, local mission structure from `Bully`/`Yakuza`, and social memory from `Kingdom Come`/`RDR2`-lite. These are inspiration lenses only, not content or design to copy.

## v0.14 Dock Road Before Bigger Gameplay

Decision: Skip v0.13.1 as a formal goal and add a compact dock road/service route before the next gameplay milestone.

Reason: The v0.13 vehicle already feels promising, but it needs a meaningful, readable place to exist before further tuning or job design. A short service-yard-to-dock-road segment improves spatial context, driving purpose, and island mood while staying within debug geometry and scene-data validation.

## v0.14 Road Layout Remains Mirrored Explicitly

Decision: Keep the v0.14 road segment authored in `data/scenes/ferry_office.scene.json` and mirrored manually in `SandboxLayer.cpp`, rather than adding runtime scene loading in the same goal.

Reason: Runtime scene loading is now the right next architecture problem, but combining it with a road/mood pass would make failures harder to isolate. v0.14 keeps behavior predictable, adds tests for the authored ids and finite bounds, and makes the scene-data drift risk more visible for v0.15.

## v0.15 Runtime Scene JSON Loader

Decision: Use `nlohmann/json` through a pinned CMake `FetchContent` dependency for runtime scene loading, with all third-party parser types hidden inside `SceneLoader.cpp`.

Reason: v0.15 needs a real runtime source of truth for the Ferry Office scene. The scene format now contains nested objects, optional fields, ids, references, routes, vehicles, colliders, mesh instances, and future schema pressure. A tiny project-owned parser would be brittle, Python-only validation would not solve runtime drift, and generated C++ would introduce freshness/build choreography before the engine has a stable data pipeline. `nlohmann/json` is MIT licensed, CMake-friendly, widely used, and small enough for this milestone when scoped to the scene loader boundary.

Dependency impact: adds `nlohmann/json` as a default build dependency via `FetchContent`. Future Codex runs should expect configure to populate or reuse the CMake dependency cache. Gameplay code should depend on Tidebreak scene structs, not `nlohmann::json`.

Alternatives considered:

- `yyjson`: fast and permissively licensed, but lower-level C-style API is less ergonomic for the current small C++ loader.
- Generated C++ from JSON: avoids runtime dependency but creates stale generated-source risk and delays the source-of-truth migration.
- Python-only validation: useful as authoring validation, but not sufficient for runtime loading.
- Project-owned ad hoc parser: rejected because it would become fragile as scene data grows.

## v0.16 Explicit First Job Helper

Decision: Add `src/game/FerryOfficeJob.*` as a small scene-owned helper for the first Ferry Office Service Call instead of adding a generic mission scripting system.

Reason: The project needs one playable driver/fixer job loop before it needs a quest graph, scripting language, rewards, economy, save/load, dialogue, NPCs, or traffic. The helper can express deterministic phases, objective text, service-vehicle checkpoint detection, and completion requirements while keeping behavior visible in C++ and tests.

## v0.16 Scene-Authored Job Markers, C++ Job Meaning

Decision: Author the current service-run confirmation marker, dock-road checkpoint marker, and final route marker in `data/scenes/ferry_office.scene.json`, but keep the meaning of those markers in `PrototypeScene` / `FerryOfficeJob`.

Reason: v0.15 made scene data the runtime source of truth for layout. v0.16 should use that for placement, but turning scene JSON into a mission script would be premature. This keeps Codex-friendly placement and validation while avoiding a hidden mission framework.

## v0.17 Playtest Presentation Mode

Decision: Add `--ui-mode playtest|debug|minimal`, make normal windowed play default to `playtest`, keep `debug` as the full telemetry view, and add an `F1` pressed-edge toggle between the current player-facing mode and debug mode.

Reason: The first Ferry Office Service Call feels good enough to play, but the raw debug wall makes the build read like a workbench. A small UI mode switch gives players objective/prompt/job clarity while preserving the complete Codex/development telemetry needed for validation. This is deliberately not a UI framework, HUD system, or mission presentation layer.

## v0.17 Debug Clutter Gating

Decision: In playtest/minimal mode, reserve heavy workbench visuals such as full grid, world bounds, collider boxes, camera target, and physics debug lines for debug mode while keeping essential route, interaction, traversal, vehicle, job, and completion markers visible.

Reason: The build should be understandable without hiding the systems Codex needs to inspect. Gating clutter by UI mode is smaller and safer than redesigning rendering, UI, markers, or scene data.

## v0.17 Route Endpoint Validation

Decision: Extend Python scene validation so route marker `from` / `to` ids must reference known scene ids.

Reason: v0.15 made scene JSON the layout source of truth, and v0.16/v0.17 depend on route/checkpoint markers for player guidance. Catching stale route endpoint ids in tools is cheaper than discovering broken signposting at runtime.

## v0.18 Tiny Original Prop Kit Before Job #2

Decision: Add a small original service-road prop kit (`service_road_sign`, `road_edge_post`, `service_barrier`, and `utility_box`) before adding another job or expanding gameplay.

Reason: The first Ferry Office Service Call already feels promising, but the scene still needs place identity. A few deliberate coastal-service silhouettes improve readability and mood without changing mechanics, adding final art, or growing the map.

## v0.18 Scene Mesh Asset Map, Not Asset Pipeline

Decision: Let `SandboxLayer` load the scene-authored mesh assets into a local `assetId -> StaticMeshAsset` map, while still avoiding an asset registry, resource cache, material system, editor, or renderer rewrite.

Reason: v0.18 needs multiple authored prop meshes, and the previous effective single-asset `unit-box-mesh` path would make scene data misleading. A local map keeps the runtime honest and tested while staying inside the static-mesh spike boundary.

## v0.19 Tiny Loader Stabilization Before Loader Swap

Decision: Keep the current tiny custom `.gltf` loader for the next short prototype phase instead of switching immediately to cgltf, tinygltf, or Assimp.

Reason: The active blocker is workflow discipline, not broad glTF coverage. Tidebreak currently needs original simple static props, scene references, license/provenance, bounds, and Codex-friendly validation more than it needs materials, textures, GLB, external buffers, or multi-format import. v0.19 therefore adds `tools/validate_assets.py`, expands `tools/mesh_report.py`, documents the decision in `docs/ASSET_PIPELINE_DECISION.md`, and makes asset validation part of `scripts/verify.ps1`.

Dependency impact: no new third-party dependency was added. cgltf is the preferred next loader candidate when Blender-authored assets, GLB, external buffers, normals, UVs, or materials become necessary. tinygltf remains a credible backup; Assimp remains too broad for this stage.

## v0.20 Honest Blender Check With Fallback Prop

Decision: Add an optional Blender availability checker and a tiny fallback `.gltf` prop generator, but do not claim a Blender export because Blender is not installed or not in PATH in this environment.

Reason: The project needs a practical DCC workflow, but pretending success would poison the asset pipeline. The fallback generator keeps one small original prop moving through scene data, validation, runtime loading, and rendering while preserving the truth that real Blender export remains blocked.

Dependency impact: no new third-party dependency was added. Blender remains an optional local tool until a future goal installs/configures it and verifies export compatibility. If Blender output immediately exceeds the current tiny `.gltf` subset, cgltf remains the preferred loader stabilization path.

## v0.20.1 Blender 5.1 Headless Export With Post-Embedded Buffer

Decision: Use a headless Blender 5.1.1 script to create one original procedural notice-board prop, export `GLTF_SEPARATE`, then post-embed the generated buffer into `assets/models/blender_ferry_notice_board.gltf` for the current tiny loader subset.

Reason: Blender is now available from PATH, but its exporter in this environment does not offer direct embedded `.gltf`. A deterministic one-buffer post-embed step proves the DCC workflow without adding cgltf/tinygltf, materials, GLB support, resource caching, or a broad asset pipeline.

Dependency impact: no new third-party dependency was added. Blender remains an optional local DCC tool and is not required for default validation on machines without it. If future Blender assets require broader output handling, cgltf remains the preferred next loader candidate.

## v0.21 Playable Prompt Copy Stays Action/Status Oriented

Decision: Keep authored scene prompt strings action/status oriented, and let runtime presentation code add input prefixes or job readiness context.

Reason: v0.21 found two small playable-build clarity risks: the traversal affordance could duplicate `Press Space` wording because `SandboxLayer` already prefixes traversal prompts, and the Service Run Marker could imply confirmation before `FerryOfficeJob` prerequisites were actually satisfied. The fix keeps traversal prompt text as `Vault Service Barrier` and changes the Service Run Marker to status-neutral copy (`Review Service Run Marker`) without adding a mission scripting layer or dynamic prompt framework.

Dependency impact: no dependency or new gameplay system was added.

## v0.22 Vehicle Camera And Low-Speed Steering Polish

Decision: Keep the service-yard vehicle deterministic and game-layer scoped, but add a narrow camera/control polish pass: vehicle camera targets look slightly ahead of the vehicle, vehicle camera mode gently follows the vehicle yaw, and low-speed steering uses an explicit minimum steering speed factor.

Reason: The current blocker is trust in the existing dock-road driving loop, not new vehicle physics. A look-ahead camera target and yaw follow make the route easier to read while reversing/turning, and low-speed steering assist makes compact service-yard turn-arounds less dead without adding wheels, suspension, Jolt VehicleConstraint, or a full vehicle tuning framework.

Dependency impact: no dependency or new gameplay system was added. Jolt remains private to `src/engine` and is not used for live vehicle control in v0.22.

## v0.23 Playable Launch Wrapper

Decision: Add `scripts/play.ps1` as the player-facing local launch path for the current prototype, defaulting to GDI, `playtest` UI, and `data\scenes\ferry_office.scene.json`.

Reason: The prototype has become playable, but the previous launch flow still required remembering long development arguments or using the lower-level `scripts/run.ps1` preset wrapper. A small tested wrapper improves hand-play and presentation without adding an installer, release packaging system, config UI, save/settings persistence, or new gameplay feature. Low-level direct executable, `scripts/run.ps1`, smoke/headless, DX11, Jolt, Blender, and scene-tool workflows remain available for development validation.

Dependency impact: no dependency or new gameplay system was added.

## v0.23.1 Follow-up DX11 Text Overlay As Stopgap

Decision: Add a small Win32 text overlay to `Dx11Renderer` after swap-chain `Present`, and keep GDI frame pacing through a tiny `IRenderer::isFramePaced()` hook.

Reason: The playable build now has a good launch wrapper, but DX11 remained weak for playtest because objective/debug text was absent. A narrow Win32 overlay makes `scripts/play.ps1 -Dx11` useful without adding a UI framework, font system, material pipeline, renderer rewrite, or new gameplay. DX11 still falls back to WARP on this laptop and the overlay is not production presentation tech.

Dependency impact: no new dependency was added.

## v0.27 Painter-Depth Debug Projection, Not Renderer Rewrite

Decision: Add camera-depth metadata and back-to-front sorting for projected debug triangle batches, and keep the existing immediate `IRenderer` debug path rather than introducing a true depth buffer, world/view/projection matrix path, material system, or GPU mesh resources.

Reason: v0.26 increased the number of flat mesh props enough that arbitrary triangle order became a presentation liability, but the current prototype still benefits from GDI/DX11 parity and simple debug visibility. Painter-depth sorting improves face/mesh readability inside solid-box and flat-mesh batches with low risk, while leaving a future real renderer/depth-buffer milestone explicit.

Dependency impact: no dependency was added. The v0.27 cable reel uses the existing optional Blender 5.1.1 headless workflow and the current embedded-buffer `.gltf` subset.

## v0.28 DX11 Real Depth/Matrix Spike, Not Full Renderer Rewrite

Decision: Add a DX11-only world-to-clip matrix shader path and depth-stencil buffer for existing debug solid boxes and flat mesh triangle submissions, while preserving the `IRenderer` interface, GDI painter-depth fallback, null renderer behavior, and overlay-style debug lines/text.

Reason: v0.27 reduced arbitrary-looking overlap, but DX11 still flattened solid/mesh triangles to NDC with `z=0`, so a real depth buffer could not help until triangles reached the GPU in world space. v0.28 proves that the existing `DebugCamera` can drive a compact matrix constant buffer and depth test without introducing materials, textures, lighting, static GPU mesh resources, scene renderer ownership, or a broad renderer rewrite. Lines, wire boxes, grid/axes, and text intentionally stay depth-disabled/overlay-style so route, prompt, collision, and validation markers remain readable.

Dependency impact: no new dependency was added. This uses only the existing Windows SDK DX11/D3DCompiler path.

## v0.38 Clean Playtest Presentation And Fake Overcast Shading

Decision: Keep raw route lines, wire boxes, trigger radii, and marker beacons behind F1/debug mode, and add a small game-layer face-shading pass for current scene boxes and flat mesh triangles.

Reason: The current visual blocker is that normal play still looks like an editor/debug capture. A clean playtest render plus fixed overcast shading makes existing scene geometry read as volume without pretending the engine has a material, texture, lighting, terrain, or production asset system. Reusing the existing dock-road props for a storm-wet work-zone cue improves place identity without growing the loader or adding unvalidated art.

Dependency impact: no new dependency or asset format was added. The v0.38 dock-road work-zone uses the existing 8 referenced `.gltf` files and raises scene mesh instances from 32 to 37.

## v0.39 Scene Presentation Boundary Before Materials

Decision: Move scene color-key mapping and fixed overcast face shading from `SandboxLayer` into `src/game/ScenePresentation.*`, with focused tests that every authored scene color key is known and dynamic state palettes still differ.

Reason: `SandboxLayer` should not become the permanent home for palette, fake material, and lighting rules. A small scene-presentation boundary gives the next visual iterations a stable place to grow authored material presets, lighting inputs, or renderer handoff data while keeping the current renderer and scene format unchanged.

Dependency impact: no new dependency, asset format, renderer feature, texture path, or material system was added.

## v0.40 Tiny Scene Material Presets

Decision: Let each scene color key resolve to a small `SceneMaterial` preset with a base color and conservative shading response terms, while keeping `SceneColorForKey` as a compatibility helper.

Reason: The v0.39 boundary should immediately start carrying useful presentation meaning, not just move old code around. Wet asphalt, damp concrete, painted service props, and matte wood/concrete should not all react to the overcast shading pass in the same way. This creates a tiny material-language stepping stone without adding textures, normals, shader files, PBR, asset-format changes, or renderer-owned material resources.

Dependency impact: no new dependency, asset file, asset format, renderer feature, shader pipeline, or texture path was added.
