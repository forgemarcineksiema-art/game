# Tidebreak Context Map

Last updated: 2026-05-16

Purpose: give future Codex sessions and human planning a compact, current map of the project without rereading the entire repository. Treat this as a working model, then verify details against source, tests, and the latest status before editing.

## Current Repository Shape

- Root: `C:/Users/Marcin/Documents/New project`
- Branch: `main`, tracking `origin/main`
- Runtime: custom C++20 Windows-first engine workbench.
- Playable target: one compact Ferry Office service-call loop.
- Build system: CMake presets, Visual Studio Debug default, optional Jolt-enabled preset.
- Everyday validation entrypoint: `scripts/verify.ps1`.
- Player-facing local launch: `scripts/play.ps1`.

Recent history forms a clear arc:

- v0.29-v0.31: renderer-owned capture and DX11/GDI visual QA.
- v0.32: deterministic Ferry Office playthrough QA.
- v0.33: Jolt static scene-query parity QA.
- v0.34: Jolt character/contact probe QA.

## Architecture Map

### Engine Core

`src/engine/application`

- `Application` owns the loop: config, window creation, renderer selection, input polling, game layer update/render, capture, frame limit, shutdown.
- `IGameLayer` is the minimal game/runtime boundary.

`src/engine/core`

- `Config` parses CLI options, including renderer, scene, UI mode, capture, playthrough QA, and physics QA.
- `Clock`, `FileSystem`, and `Logger` are small standard-library foundations.

`src/engine/platform`

- `IWindow` abstracts window/input ownership.
- `Win32Window.cpp` owns keyboard/mouse state, cursor capture, frame title text, and Win32 event handling.

`src/engine/input`

- `InputState` is a snapshot, not a broad input framework.
- Current inputs cover movement, sprint, jump, interaction edge/held state, debug overlay toggle, camera deltas, mouse deltas, and quit.

`src/engine/renderer`

- `IRenderer` is the renderer boundary.
- Backends: `NullRenderer`, `GdiRenderer`, `Dx11Renderer`.
- Current rendering is debug/immediate-mode oriented: grids, lines, boxes, solid boxes, flat triangles, text, and BMP capture.
- DX11 has real depth/matrix support for solid boxes and flat static mesh triangles; GDI keeps CPU projection/painter ordering.

`src/engine/assets`

- `StaticMesh` owns the tiny static mesh path.
- Current loader supports a narrow embedded-buffer `.gltf` subset for original placeholder assets.
- Meshes are expanded to flat triangle lists for the current renderer path.

`src/engine/physics`

- `IPhysicsWorld` is the engine-owned physics boundary.
- Default backend is dependency-free `simple`.
- Optional backend is Jolt through `windows-vs2022-debug-jolt`.
- Game code should continue to talk through engine-owned types and helpers.

### Game Prototype

`src/game/SandboxLayer.*`

- Main integration point for scene load, player, camera, interactions, traversal, vehicle, job state, debug/presentation text, mesh submission, and marker drawing.
- This is the busiest file and the best place to understand the runtime flow before deeper changes.

`src/game/SceneDefinition.*` and `SceneLoader.*`

- Runtime scene data structs and JSON loading.
- Third-party JSON types stay inside the loader boundary.

`data/scenes/ferry_office.scene.json`

- Current source of truth for Ferry Office layout: player start, colliders, visual placeholders, mesh assets/instances, interactables, traversal, vehicle, route markers, objective markers.

`src/game/PrototypeWorld.*`

- Active world/collision behavior for live gameplay.
- Static AABB colliders, floor height, player proxy resolution, overlap checks, raycast, and gate blocking state.

`src/game/PlayerController.*`

- Camera-relative movement, sprint, jump, gravity, grounded state, facing yaw, traversal mode, and collision integration through `PrototypeWorld`.

`src/game/ThirdPersonCamera.*`

- Orbit/follow camera with pitch clamp, smoothing, and optional target-yaw follow for vehicle mode.

`src/game/InteractionSystem.*`

- Point/radius interactables, focus selection, one-shot pickup, toggle, and info interactions.

`src/game/TraversalSystem.*`

- One authored vault-style traversal affordance with focus and deterministic activation data.

`src/game/VehicleController.*`

- Deterministic service-yard vehicle controller: enter/exit, throttle/brake/reverse, steering, drag, bounds, camera target, and telemetry.

`src/game/WorldState.*`

- Local remembered flags and ordered event log for the current prototype.

`src/game/FerryOfficeJob.*`

- Explicit first service-call job helper: phase ordering, objective text, checkpoint detection, completion.

`src/game/FerryOffice*Qa.*`

- Deterministic QA harnesses for service-call completion, static physics parity, and character/contact probes.

## Test And QA Surface

### C++ Tests

`tests/EngineCoreTests.cpp` is a broad lightweight executable. It covers:

- CLI/config parsing.
- renderer utility code, BMP capture, debug projection, DX11 matrix math, bitmap text layout.
- static mesh loading and transforms.
- scene loading and source-of-truth expectations.
- physics interface behavior and Jolt availability guard.
- vehicle behavior and camera target behavior.
- UI mode/presentation text behavior.
- player movement, camera, world collision, raycast.
- interactions, traversal, world state, Ferry Office scene loop.
- playthrough QA, physics parity QA, character contact QA report writing.
- source boundary guard for accidental direct Jolt references in `src/game`.

### Python Tests

- `tests/test_scene_tools.py`: scene reporting/validation helpers.
- `tests/test_run_scripts.py`: `scripts/play.ps1` defaults, overrides, dry-run behavior.
- `tests/test_capture_visual_smoke.py`: BMP/stat/parity/report logic.
- `tests/test_playthrough_qa.py`: playthrough report validation.
- `tests/test_physics_parity_tool.py`: physics parity report validation.
- `tests/test_character_contact_qa.py`: character contact report validation.

### Validation Commands

Default foundation:

```powershell
scripts/doctor.ps1
scripts/configure.ps1
scripts/build.ps1
scripts/verify.ps1
```

Scene/data/assets:

```powershell
python tools/scene_report.py
python tools/validate_scene.py
python tools/validate_assets.py
python tools/scale_audit.py
python tools/mesh_report.py
```

Runtime/visual/behavior QA:

```powershell
python tools/playthrough_qa.py
python tools/capture_visual_smoke.py
```

Opt-in physics QA:

```powershell
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
python tools/physics_parity_qa.py
python tools/character_contact_qa.py
ctest --preset windows-vs2022-debug-jolt --output-on-failure
```

## Validation Snapshot From Context Expansion

Commands run on 2026-05-16:

- `python tools/status_report.py`: ran; reported build outputs present. It also created a transient Python `tools/__pycache__` directory during imports.
- `scripts/doctor.ps1`: passed; expected plain-PATH warnings for compiler/tool binaries remained.
- `python tools/scene_report.py`: passed; scene has 9 colliders, 24 visual placeholders, 8 mesh assets, 32 mesh instances, 6 interactables, 1 traversal affordance, 1 vehicle, 6 route markers, and 5 objective markers.
- `python tools/validate_scene.py`: passed.
- `python tools/validate_assets.py`: passed.
- `python tools/scale_audit.py`: passed, no suspicious scale issues.
- `python tools/mesh_report.py`: passed, 8 referenced model files.
- `python tools/playthrough_qa.py`: passed; service-call phase `complete`, 10 events.
- `python tools/capture_visual_smoke.py`: passed for GDI and DX11 captures; DX11 used WARP in this environment.
- `python tools/physics_parity_qa.py`: passed with Jolt backend; floor=4, raycast=4, overlap=4.
- `python tools/character_contact_qa.py`: passed with Jolt backend; probes=7.

## External Context Checked

Checked online primary sources on 2026-05-16:

- Jolt Physics repo: latest release shown as `5.5.0` from 2025-12-28, matching the repository's pinned opt-in tag. Jolt documents game-oriented rigid bodies, queries, sensors, character simulation, vehicles, Visual Studio 2019+ support, C++17 use, and MIT licensing.
- Jolt documentation archive lists `5.5.0`, `5.4.0`, `5.3.0`, `5.2.0`, `5.1.0`, `5.0.0`, `4.0.2`, and `3.0.1`.
- Jolt `CharacterVirtual` docs describe a runtime character object moved by collision checks outside the normal physics body simulation, useful when the game wants exact update timing control.
- Jolt docs list `VehicleConstraint` as the built-in path for virtual wheels/tracks and vehicle behavior.
- CMake official docs confirm `CMakePresets.json` is the project-wide preset file and `CMakeUserPresets.json` is user-specific.
- nlohmann/json official repo shows latest `3.12.0` from 2025-04-11; this repo currently pins `3.11.3`. Treat an update as a small dependency-maintenance task, not as part of unrelated gameplay work.
- Microsoft Learn remains the source for Direct3D 11 core API reference.

Sources:

- https://github.com/jrouwe/JoltPhysics
- https://jrouwe.github.io/JoltPhysics/
- https://jrouwe.github.io/JoltPhysicsDocs/
- https://jrouwe.github.io/JoltPhysicsDocs/5.3.0/class_character_virtual.html
- https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html
- https://github.com/nlohmann/json
- https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d11-graphics-reference-d3d11-core

## Efficient Working Method

Use this when time or token budget is tight:

1. Start with `git status --short --branch` and `python tools/status_report.py`.
2. Read this file, then only the domain docs needed for the task.
3. Start or continue one explicit Codex `/goal`.
4. Pick one concrete capability or bug, not a bundle of several.
5. Find the owner module with `rg`, then read headers before implementation files.
6. Add or update a focused test first when the behavior has a stable contract.
7. Make the smallest code/data/doc change that proves the capability.
8. Run the narrowest meaningful validation while iterating.
9. Before calling work complete, run `scripts/verify.ps1` or record the exact blocker in `docs/STATUS.md`.
10. For physics/Jolt work, also run the Jolt preset/tooling that matches the touched behavior.
11. Update `docs/STATUS.md` with commands, results, commit/push status, and the next-goal prompt.

## Goal Operating Model

- Main Codex owns the goal and acts as project lead.
- Subagents are workers and reviewers with bounded instructions.
- A goal should advance Tidebreak toward durable engine/game/tooling capability, not simply make a prototype area nicer.
- Small goals are valid when they remove uncertainty or create validation.
- Powerful goals are preferred when they have a clear system boundary and acceptance criteria.
- Repeated investment in a prototype area should trigger a promote/replace/stop decision.
- Every completed goal ends with validation, `docs/STATUS.md`, guarded commit/push when allowed by `AGENTS.md`, and a ready prompt for the next goal.

## 3-5 Strong Next Directions

### 1. Input-Scripted Runtime QA For Ferry Office

Goal: add a runtime/input-driven QA path that drives the actual update loop with scripted input frames and asserts job progression.

Why it is strong now: current `playthrough_qa.py` proves scene/job systems, but it bypasses real keyboard-style input, camera/player update rhythm, and vehicle driving path. This would bridge the gap between deterministic state QA and manual play.

Likely write areas:

- `src/engine/application` for a bounded scripted-input mode, if needed.
- `src/game/SandboxLayer` for test seams or state snapshots, if needed.
- `src/game/FerryOffice*Qa.*` or a new QA runner.
- `tests/EngineCoreTests.cpp`
- `tools/*_qa.py`

Validation:

- focused C++ tests,
- new Python wrapper tests,
- `scripts/verify.ps1`,
- `python tools/playthrough_qa.py`,
- `python tools/capture_visual_smoke.py`.

### 2. Live Player Collision Migration Probe

Goal: let live player collision optionally use the engine physics boundary for the already-probed Ferry Office cases, while preserving the current player feel contract.

Why it is strong now: v0.33 and v0.34 created parity/contact evidence. The next small step is a controlled runtime switch or adapter, not a broad rewrite.

Likely write areas:

- `src/engine/physics/PhysicsWorld.*`
- `src/game/PrototypeWorld.*`
- `src/game/PlayerController.*`
- `src/game/SandboxLayer.*`
- `src/game/FerryOfficeCharacterContactQa.*`
- tests around matching positions, grounding, gate state, and corner pushout.

Validation:

- default `scripts/verify.ps1`,
- Jolt configure/build,
- `python tools/physics_parity_qa.py`,
- `python tools/character_contact_qa.py`,
- opt-in Jolt CTest.

### 3. Jolt Vehicle Feasibility Spike

Goal: create an opt-in vehicle physics experiment behind `engine::physics`, compare against the deterministic service-yard controller, and generate a QA report.

Why it is useful: the project has a working deterministic vehicle baseline and a Jolt backend. A contained feasibility report would answer whether Jolt vehicles are worth promoting for the current road scale.

Likely write areas:

- `src/engine/physics`
- new QA runner in `src/game`
- `tools/*vehicle*_qa.py`
- tests for configuration, report writing, backend unavailable behavior, and no vendor leakage into game-facing APIs.

Validation:

- default verify remains green,
- Jolt preset build/test,
- vehicle QA report,
- manual `scripts/play.ps1` only after the report is useful.

### 4. Renderer Resize/Text Quality Pass

Goal: improve confidence that GDI/DX11 presentation and debug/playtest text survive alternate window/capture dimensions.

Why it is useful: capture QA is already mature at 1280x720. Resize and text layout are natural next confidence gaps before more visual complexity.

Likely write areas:

- `src/engine/renderer`
- `src/engine/application`
- `tools/capture_visual_smoke.py`
- `tests/test_capture_visual_smoke.py`
- `tests/EngineCoreTests.cpp`

Validation:

- capture smoke at multiple dimensions,
- default verify,
- focused renderer utility tests.

### 5. Runtime Surface Area Cleanup Around `SandboxLayer`

Goal: extract one or two small collaborators from `SandboxLayer` where behavior is already stable: presentation text, guidance gating, or static mesh submission.

Why it is useful: `SandboxLayer` is doing many jobs. A careful extraction would make future feature work cheaper, especially input-scripted QA and collision migration.

Likely write areas:

- new focused game-layer helper files,
- `SandboxLayer.*`,
- existing presentation/guidance/static mesh tests.

Validation:

- default verify,
- playthrough QA,
- capture visual smoke.

## Recommendation

Best next move: Input-Scripted Runtime QA.

Reason: it makes every later gameplay or physics decision cheaper. It tests the actual live flow more honestly than the current state-only playthrough QA, and it should reduce time spent wondering whether a change broke hand-play before asking for manual feedback.

Second-best: Live Player Collision Migration Probe, but only after deciding how much runtime risk you want to take in the current session.

Ready next-goal prompt:

```text
Create a Codex goal for Tidebreak.

Repository rules:
- Follow AGENTS.md and docs/AI_WORKFLOW.md.
- Use docs/CONTEXT_MAP.md for orientation.
- Avoid polish unless it blocks validation or a production decision.
- Update docs/STATUS.md.
- Run scripts/verify.ps1 before claiming success.
- Commit and push only if validation passes and there are no unrelated user changes.

Goal:
Build input-scripted runtime QA for the Ferry Office Service Call.

Why now:
The current playthrough QA proves scene/job state directly, but it does not exercise the live runtime update loop through keyboard-style input, player/camera timing, traversal activation, vehicle entry/driving/exit, and presentation-safe state changes. This goal should turn the first playable loop into a stronger regression gate before deeper Jolt or vehicle migration work.

Scope:
- Add a QA-only scripted-input runtime path or runner that drives the real update loop without opening a normal interactive session.
- Cover the Ferry Office service-call path through manifest, service route, maintenance/power, gate, vehicle, checkpoint, service-run confirmation, and completion.
- Write a JSON report and Python wrapper similar to existing QA tools.
- Add focused C++ and Python tests.

Non-goals:
- No new job content.
- No visual polish.
- No Jolt vehicle migration.
- No broad input framework rewrite unless a tiny seam is required for the scripted runner.

Files/docs to read first:
- AGENTS.md
- docs/AI_WORKFLOW.md
- docs/CONTEXT_MAP.md
- docs/VERTICAL_SLICE.md
- docs/TECH_DEBT.md
- src/engine/application/Application.*
- src/engine/input/Input.h
- src/game/SandboxLayer.*
- src/game/FerryOfficePlaythroughQa.*
- tools/playthrough_qa.py
- tests/EngineCoreTests.cpp

Validation:
- scripts/verify.ps1
- python tools/playthrough_qa.py
- python tools/capture_visual_smoke.py

Use subagents:
- Worker: inspect the smallest seam for feeding scripted input into the runtime without overbuilding.
- Reviewer: check for scope creep, missing validation, and accidental replacement of existing deterministic QA.
```
