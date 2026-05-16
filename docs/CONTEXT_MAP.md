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
- v0.45-v0.95: Jolt vehicle runtime controls, service-run route proxy, route-pace tuning, deterministic service-vehicle runtime playthrough QA, opt-in Jolt first-job live-loop QA, the Dock Road Relay follow-up beat, relay/log/clearance endpoint consequences, compact playtest follow-up text, camera-aware obstacle-proxy vehicle steering evidence, a clearance-tag visual cue, a tiny original clearance-tag mesh prop, Jolt obstacle-progress tuning, a compact Harbor Parts return micro-slice, a follow-up next-step readability line, scene-authored action bindings for simple/gated flag beats, a Ferry Office Work Board signoff endpoint, a Ferry Office handoff note endpoint, collision-backed obstacle replay telemetry in vehicle runtime QA, a dynamic handoff-filed visual state cue, a storm pump job seed, a provisional Jolt vehicle-runtime direction decision, visible storm pump/ticket state cues, a preferred play-wrapper runtime trial for Jolt-enabled executables, a Low Dock Drain clear-tag follow-up, a Ferry Office Drain Log closeout, a visible office-side Drain Log state cue, a refreshed preferred Jolt evidence decision, active late-chain route guidance in playtest mode, a purpose-built office-side service-panel mesh cue, QA-only mid-chain route capture evidence for the Relay Service Log state, and a state-controlled Low Dock Drain access barrier that opens after the Storm Pump Ticket closes.
- Post-v0.95: `docs\GAMEPLAY_REVIEW.md` remains the gameplay diagnosis gate. The physical-consequence milestone landed, so the next default should shift toward driving-feel road-test evidence unless fresh capture review shows the Low Dock Drain access view needs a bounded readability fix.

## Architecture Map

### Engine Core

`src/engine/application`

- `Application` owns the loop: config, window creation, renderer selection, input polling, game layer update/render, capture, frame limit, shutdown.
- `IGameLayer` is the minimal game/runtime boundary.

`src/engine/core`

- `Config` parses CLI options, including renderer, scene, UI mode, capture, QA capture states, playthrough QA, and physics QA.
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
- After v0.91, Jolt remains the preferred production vehicle-runtime candidate: Jolt CTest, parity/contact/vehicle/runtime QA, Jolt playthrough QA, and preferred-wrapper smoke all pass against the current Ferry Office scene.

### Game Prototype

`src/game/SandboxLayer.*`

- Main integration point for scene load, player, camera, interactions, traversal, vehicle, job state, debug/presentation text, mesh submission, and marker drawing.
- This is the busiest file and the best place to understand the runtime flow before deeper changes.

`src/game/SceneDefinition.*` and `SceneLoader.*`

- Runtime scene data structs and JSON loading.
- Third-party JSON types stay inside the loader boundary.

`src/game/ScenePresentation.*`

- Current game-layer presentation boundary for scene-authored material presets, dynamic palette state, and fixed overcast face shading.
- This is the staging point for future material or lighting handoff work before those rules belong in the renderer.

`data/scenes/ferry_office.scene.json`

- Current source of truth for Ferry Office layout and presentation intent: player start, scene materials, colliders, visual placeholders, mesh assets/instances, interactables, traversal, vehicle, route markers, objective markers. After v0.95 it includes 25 scene materials, 10 colliders, 32 visual placeholders, 20 mesh assets, 66 mesh instances, 17 interactables, 17 route markers, and 16 objective markers, including the handoff-filed status cue, storm pump job seed, storm pump/ticket state cues, low dock drain clear-tag cue, Ferry Office Drain Log closeout, a visible office-side Drain Log state cue, active playtest route guidance, a purpose-built office-side service-panel mesh, a QA capture path for the Relay Service Log route state, and the Storm Pump Ticket controlled Low Dock Drain access barrier.

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

- Explicit first service-call job helper: phase ordering, objective text, checkpoint detection, completion, and active route/objective marker ids for the service-call plus follow-up chain.

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
- playthrough QA, runtime service-vehicle playthrough QA, physics parity QA, character contact QA report writing.
- QA-only capture-state parsing and playtest route rendering from the Relay Service Log mid-chain state.
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
python tools/capture_visual_smoke.py --scenario relay-to-service-log --report-json build\captures\capture_visual_smoke_midchain_report.json
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
- `python tools/scene_report.py`: passed; scene has 9 colliders, 26 visual placeholders, 20 scene materials, 12 mesh assets, 47 mesh instances, 13 interactables, 1 traversal affordance, 1 vehicle, 13 route markers, and 12 objective markers.
- `python tools/validate_scene.py`: passed.
- `python tools/validate_assets.py`: passed.
- `python tools/scale_audit.py`: passed, no suspicious scale issues.
- `python tools/mesh_report.py`: passed, 12 referenced model files.
- `python tools/playthrough_qa.py`: passed; service-call phase `complete`, 20 events. After v0.71 this report is also expected to include vehicle runtime evidence, runtime service-vehicle enter, dock-road checkpoint, exit steps, the Dock Road Relay reset step, the relay service log sign-off, the dock-road clear-tag consequence, Harbor Parts pickup, Harbor Parts delivery, Ferry Office Work Board signoff, Ferry Office handoff filing, storm pump reset, storm pump ticket closeout, and low dock drain clear-tag closeout.
- `python tools/capture_visual_smoke.py`: passed for GDI and DX11 captures; DX11 used WARP in this environment. After v0.55, playtest presentation also includes a compact `Follow-up: relay=... | log=... | road=...` line once endpoint follow-up state is relevant; after v0.58 the clearance-tag endpoint also has a tiny dynamic mesh prop; after v0.60 the initial playtest prompt still focuses `Collect Ferry Manifest` despite the new office shelf delivery interaction; after v0.61 the follow-up state also gets a compact `Next:` line; after v0.63 that line points delivered Harbor Parts to the Ferry Office Work Board before completion; after v0.66 the handoff endpoint also has an amber-to-green dynamic visual state cue.
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
2. Read this file, `docs/GAMEPLAY_REVIEW.md`, then only the other domain docs needed for the task.
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
- After v0.95, every autonomous milestone choice should include a gameplay diagnosis from `docs\GAMEPLAY_REVIEW.md`; green validation alone is not a sufficient reason to pick the smallest safe visual/data cleanup.
- Older roadmap/debt "do not do this yet" notes are context, not permanent vetoes. Re-check them against current evidence before choosing a goal.
- When a prototype path has enough validation, prefer a target-system promotion goal over more placeholder tuning.
- Every completed goal ends with validation, `docs/STATUS.md`, guarded commit/push when allowed by `AGENTS.md`, and a ready prompt for the next goal.

## 3-5 Strong Next Directions

### 1. Service Route Consequence / Physical World Change

Goal: make an existing repaired state visibly and spatially change the Ferry Office/service-road world.

Why it is strong now: the chain is long and validated, but too many beats are administrative sign-offs. A physical route/world consequence advances the driver/fixer fantasy more than another log, tag, or prop.

Likely shapes:

- open a short service path,
- remove or lower a small route obstruction,
- expose a new turnaround or dock-side shortcut,
- change a work zone from blocked to passable,
- make the storm pump, low dock drain, or relay work alter traversable or drivable space.

Validation:

- focused C++ tests for state-to-collider/route/presentation behavior,
- `python tools/playthrough_qa.py`,
- `python tools/capture_visual_smoke.py`,
- a named QA capture state if the changed world is mid-chain,
- scene/asset/mesh/scale tools if scene data changes,
- `scripts/verify.ps1`.

### 2. Driving Feel Road-Test Pass

Goal: build or extend automated road-test metrics before tuning deterministic/Jolt vehicle feel.

Why it is strong now: driving is Tidebreak's most kinetic player verb, and the project already has deterministic and Jolt evidence. The next vehicle improvement should measure controllability, not rely on "feels better" guesses.

Metrics:

- time to checkpoint,
- steering response,
- yaw overshoot,
- stop distance,
- reverse behavior,
- route deviation,
- bounds hits,
- camera target stability.

Validation:

- deterministic road-test report,
- optional Jolt comparison through `windows-vs2022-debug-jolt`,
- `python tools/vehicle_runtime_qa.py`,
- `python tools/playthrough_qa.py`,
- `scripts/verify.ps1`.

### 3. Objective/Route Readability Without More Text

Goal: reduce dependence on the top-left overlay by making an active destination readable in the world.

Why it is useful: v0.92 and v0.94 prove active guidance and mid-chain capture, but the overlay still carries most objective understanding. This is valid when capture evidence shows a current objective/destination is visually weak.

Improve:

- active destination framing,
- landmark placement,
- prompt priority near clustered interactables,
- in-world state cue scale/color,
- route-side object composition.

Avoid:

- another top-left line,
- a prop that does not help the active objective,
- another administrative sign-off endpoint.

### 4. Follow-up Content Boundary

Goal: stop the follow-up chain from becoming a longer hardcoded checklist before adding another substantial job.

Why it is useful: `FerryOfficeJob` now owns route/objective policy for a 21-event chain. If new content would extend that chain, first make one smaller data/helper boundary for follow-up beats.

Likely write areas:

- `src/game/FerryOfficeJob.*`,
- `src/game/SceneDefinition.*`,
- `src/game/SceneLoader.*`,
- `data/scenes/ferry_office.scene.json`,
- `tests/EngineCoreTests.cpp`,
- `tools/validate_scene.py`.

Validation:

- red/green tests proving one migrated follow-up route/objective policy,
- playthrough QA,
- scene validation/report,
- `scripts/verify.ps1`.

### 5. Runtime Surface Area Cleanup Around `SandboxLayer`

Goal: extract a small collaborator only if it unlocks one of the player-facing goals above.

Why it is useful: `SandboxLayer` is still the busiest integration point, but file size alone is not a game problem. Extract presentation text, guidance policy, or mesh submission only when a concrete next feature would otherwise make it worse.

Validation:

- focused C++ tests,
- playthrough QA,
- capture visual smoke,
- `scripts/verify.ps1`.

## Recommendation

Best next move after v0.95: `Driving Feel Road-Test Pass`, unless Low Dock Drain capture evidence shows a blocking route/readability issue.

Reason: the current 21-event chain now has one physical local consequence, and the strongest under-tested player verb is driving. Build evidence before tuning by hand or promoting a runtime.

Do not start broad map expansion, a generic mission framework, a big asset pass, or another small prop by default. A prop/readability pass is fine only when capture evidence proves an active objective is visually weak.

Ready next-goal prompt:

```text
Create a Codex goal for Tidebreak.

Repository rules:
- Follow AGENTS.md and docs/AI_WORKFLOW.md.
- Read docs/GAMEPLAY_REVIEW.md before selecting the milestone.
- Use docs/CONTEXT_MAP.md for orientation.
- Avoid polish unless it directly improves route/objective readability or blocks validation/a production decision.
- Update docs/STATUS.md.
- Run scripts/verify.ps1 before claiming success.
- Commit and push only if validation passes and there are no unrelated user changes.

Goal:
Build an automated Ferry Office driving-feel road-test report.

Why now:
v0.95 added the requested physical world consequence. The next player-facing bottleneck is likely vehicle quality: Tidebreak needs measurable steering, braking, reverse, bounds, route-deviation, and camera-target evidence before tuning or promoting vehicle runtime further.

Scope:
- Add or extend a QA report for the existing Ferry Office service-yard/dock-road drive.
- Record deterministic metrics such as time to checkpoint, steering/yaw response, stop distance, reverse behavior, route deviation, bounds hits, and camera target stability.
- Compare Jolt only if the default evidence is cheap and the preset is already available; keep deterministic as the direct-app baseline.
- Use the evidence to choose a provisional tuning/runtime next action.

Non-goals:
- No broad new job arc.
- No traffic, NPCs, damage, garage, economy, save/load, or multi-job framework.
- No new administrative sign-off beat.
- No generic mission framework.
- No default vehicle replacement.
- No Jolt vendor types leaking into game-facing APIs.

Files/docs to read first:
- AGENTS.md
- docs/AI_WORKFLOW.md
- docs/GAMEPLAY_REVIEW.md
- docs/CONTEXT_MAP.md
- docs/VERTICAL_SLICE.md
- docs/TECH_DEBT.md
- docs/ROADMAP.md
- src/game/SandboxLayer.*
- src/game/VehicleController.*
- src/game/ThirdPersonCamera.*
- src/game/FerryOfficeVehicle*Qa.*
- tools/vehicle_runtime_qa.py
- tools/vehicle_physics_qa.py
- data/scenes/ferry_office.scene.json
- tests/EngineCoreTests.cpp

Validation:
- scripts/verify.ps1
- python tools/playthrough_qa.py
- python tools/vehicle_runtime_qa.py
- python tools/vehicle_physics_qa.py if the touched path uses opt-in physics evidence
- cmake/ctest with `windows-vs2022-debug-jolt` if Jolt behavior is changed

Use subagents:
- Reviewer: check that the report measures useful driving behavior and does not smuggle in subjective tuning without evidence.
```
