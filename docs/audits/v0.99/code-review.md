# v0.99 Technical Code Review

Review stance: bugs, risks, behavioral regressions, missing tests. Findings are about the current repository state, not a proposed diff.

## Verdict

`REQUEST CHANGES` for process/QA truthfulness before the next substantial milestone. No P0 blocker was found, but several P1 issues can make future agents or humans over-trust the current state.

## Findings

| Priority | Status | Issue | Location |
| --- | --- | --- | --- |
| P1 | CONFIRMED | Jolt preset CTest playthrough is deterministic unless explicitly run with `--vehicle-runtime jolt`. | `CMakeLists.txt:269`, `tools/playthrough_qa.py:139` |
| P1 | CONFIRMED | Default `verify` excludes visual smoke and Jolt gameplay/vehicle evidence. | `scripts/verify.ps1:31`, `scripts/verify.ps1:63` |
| P1 | CONFIRMED | Playthrough QA is a scripted state-chain runner, not full live input/navigation. | `src/game/FerryOfficePlaythroughQa.cpp:72`, `src/game/FerryOfficePlaythroughQa.cpp:512` |
| P1 | CONFIRMED | Vehicle collision evidence remains incomplete; route/obstacle checks are proxy/bounds-oriented. | `src/game/VehicleController.cpp`, `src/game/FerryOfficeVehiclePhysicsQa.cpp`, `src/engine/physics/JoltVehicleRuntime.cpp` |
| P1 | CONFIRMED | GDI shaded triangle ordering is weaker than intended because per-triangle calls defeat renderer-local sorting. | `src/game/ScenePresentation.cpp:130`, `src/engine/renderer/GdiRenderer.cpp:167` |
| P2 | CONFIRMED | Route budget threshold is split between 190 and 240 frames. | `src/game/FerryOfficeVehiclePhysicsQa.cpp:29`, `src/game/FerryOfficePlaythroughQa.cpp:25`, `tests/EngineCoreTests.cpp:4790` |
| P2 | CONFIRMED | Runtime version is stale at `0.23.0`. | `CMakeLists.txt:4`, `src/engine/application/Engine.cpp:13` |
| P2 | CONFIRMED | `scripts/play.ps1` defaults to VS2022 Debug exe rather than `.last_preset`. | `scripts/play.ps1:72`, `scripts/run.ps1:18` |
| P2 | INFERRED | Route/objective/flag coupling is near its next pressure point. | `src/game/FerryOfficeJob.cpp:50`, `src/game/PrototypeScene.cpp:390`, `data/scenes/ferry_office.scene.json` |

## Build And Runtime

CONFIRMED: default build and verification pass. Evidence: `raw/05-configure.log`, `raw/06-build.log`, `raw/07-verify.log`.

P1: `scripts/verify.ps1` is not wrong, but its name can be overread. It runs CTest, scene validation, asset validation, mesh report, and null smoke. It does not run `capture_visual_smoke.py`, Jolt playthrough, vehicle runtime QA, physics parity, character contact, or vehicle feasibility. This is acceptable as a fast foundation gate only if docs/status keep saying that.

P2: `play.ps1` hardcodes `build\windows-vs2022-debug\Debug\EngineApp.exe` when `-ExecutablePath` is not supplied. `run.ps1` respects `.last_preset`; play does not. This can confuse machines using VS2026 or Ninja.

P2: `ENGINE_VERSION_STRING` comes from CMake project version `0.23.0`, and runtime logs show `Initializing engine v0.23.0` during v0.99. That makes audit logs less self-describing.

## Renderer And Visuals

CONFIRMED: GDI and DX11 bounded captures work. Evidence: `raw/16-capture-visual-smoke-initial.log`, `raw/17-capture-visual-smoke-relay.log`, `raw/18-capture-visual-smoke-low-dock.log`.

P1: visual smoke is only a smoke heuristic. The thresholds check dimensions, color diversity, luminance, and overlay text signal. They do not detect whether the objective is visually clear, whether the composition is good, or whether player-facing cues make sense.

P1: GDI ordering issue. `ScenePresentation` shades and submits triangle-by-triangle, while `GdiRenderer` sorts projected triangles only inside each call. DX11 depth hides this class of problem for DX11, but GDI remains the default play wrapper renderer.

P2: DX11 path is WARP-proven here, not hardware-proven. `play.ps1 -Dx11` and visual smoke logged "Hardware DX11 device failed; trying WARP."

## Scene, Assets, And World Data

CONFIRMED: `scene_report`, `validate_scene`, `scale_audit`, `validate_assets`, and `mesh_report` passed. Current scene is substantial: 25 materials, 10 colliders, 32 placeholders, 20 mesh assets, 66 instances, 17 interactables, 17 routes, 16 objectives.

P2: source-of-truth is strong but not strict. Runtime falls back to built-in Ferry Office setup if scene load fails. That is useful for smoke/debug but weakens "scene data is the one truth" for explicit `--scene` experiments.

P2: static mesh path is honest but placeholder. Assets are original and validated, but the loader is still tiny embedded `.gltf`, immediate flat triangles, no material/texture/resource cache/mesh collision path.

## Gameplay, Physics, Vehicle

CONFIRMED: deterministic and Jolt 21-event playthroughs passed in this session. Deterministic checkpoint: 139 frames. Jolt checkpoint: 169 frames. Jolt fallback: false. Bounds hit: false.

P1: playthrough QA is state-chain proof. `TriggerInteraction` can run interactions by name; post-vehicle chain triggers many interactions directly. That is valid for regression coverage, not proof that a player can naturally navigate the chain.

P1: Jolt vehicle evidence is strong for compact route/controls metrics, but not full vehicle-world collision. `VehicleController` still clamps bounds. Jolt runtime path is a controlled floor/vehicle route setup. Obstacle replay is a synthetic QA probe, not authored road-edge collision against the full scene.

P2: player movement/camera/traversal are still diagnostic baseline: immediate movement velocity, camera smoothing but no obstruction/collision, traversal collision resolved after landing.

## Tests And QA

CONFIRMED: C++ and Python tests provide broad coverage. `tests/EngineCoreTests.cpp` is also very large and acts as a mega-regression file.

P1: naming/coverage mismatch. `FerryOfficePlaythroughQaSmoke` in Jolt preset sounds like it covers Jolt runtime, but show-only CTest output proves it lacks `--vehicle-runtime jolt`.

P2: stale generated/old build reports can mislead. `build/physics` may contain unavailable reports from default build while `build/windows-vs2022-debug-jolt/physics` contains current Jolt evidence. Audit artifacts avoid this by writing fresh reports under `docs/audits/v0.99/raw`.

## Recommendation

Before adding content, fix evidence truth:

1. Add explicit Jolt playthrough CTest with `--vehicle-runtime jolt`.
2. Add a named runtime/visual/Jolt verification wrapper or documented gate.
3. Add live-input or `SandboxLayer`-driven gameplay QA for a short on-foot + vehicle path.
4. Add authored road-edge/scene-collider vehicle replay before further Jolt promotion.
5. Fix GDI triangle batching/sort or document GDI as non-authoritative for depth overlap.
