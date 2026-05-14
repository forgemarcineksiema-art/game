# Project Status

Last updated: 2026-05-15

## Milestone A - Inspection Findings

- Repository root: `C:/Users/Marcin/Documents/New project`
- Git state before v0.1 work: `main...origin/main`, clean working tree.
- Remote: `origin https://github.com/forgemarcineksiema-art/game.git`
- Existing tracked files: `README.md`
- Existing source files: none.
- Existing build files: none.
- Existing docs: none.
- Existing scripts/tools/tests/assets: none.
- Existing generated files or old experiments: none found outside `.git`.
- Worktree state: normal checkout (`.git` is both git dir and common dir), not a linked worktree or submodule.

## Environment Snapshot

- `cmake --version`: CMake 4.3.2 found at `C:/Program Files/CMake/bin/cmake.exe`.
- `python`: Python 3.11 and 3.13 launchers found.
- `cl`, `clang++`, `g++`: not found in current PATH during initial inspection.
- `msbuild`, `ninja`: not found in current PATH during initial inspection.
- `vcpkg`: not found in current PATH during initial inspection.

## Short Implementation Plan

1. Create the durable repository structure: docs, scripts, tools, source, tests, and assets placeholders.
2. Add AI-facing documentation first so future Codex runs have rules, commands, architecture notes, and a roadmap.
3. Add a small CMake C++20 executable with focused engine modules: application, engine loop, logging, time, config, filesystem, input placeholder, renderer interface, fallback renderer, and sandbox game layer.
4. Add a smoke-test CLI path that initializes the engine and runs a bounded number of frames without needing a window.
5. Add lightweight tests for non-renderer logic without pulling in a heavy test dependency.
6. Add PowerShell/Python workbench commands for doctor, configure/build, verify, and status reporting.
7. Run the available validation commands, record exact results here, and keep any compiler/graphics blockers honest.

## Commands Run

```powershell
git rev-parse --show-toplevel
git status --short --branch
rg --files -uu
Get-ChildItem -Force -Recurse -Depth 3 | Select-Object Mode,Length,LastWriteTime,FullName
git remote -v
git log --oneline -5
cmake --version
where.exe cmake
where.exe cl
where.exe clang++
where.exe g++
where.exe python
where.exe vcpkg
git rev-parse --git-dir
git rev-parse --git-common-dir
git rev-parse --show-superproject-working-tree
where.exe msbuild
where.exe ninja
where.exe vswhere
```

## Changes Made for v0.1

- Added CMake project files: `CMakeLists.txt`, `CMakePresets.json`.
- Added engine modules under `src/engine`:
  - application loop,
  - engine lifecycle,
  - logger,
  - clock,
  - config parser,
  - filesystem helpers,
  - input placeholder,
  - platform window interface,
  - Win32 window implementation,
  - renderer interface,
  - DirectX 11 renderer,
  - GDI fallback renderer,
  - null headless renderer.
- Added sandbox runtime under `src/game`.
- Added lightweight C++ tests under `tests`.
- Added AI workbench scripts:
  - `scripts/doctor.ps1`,
  - `scripts/configure.ps1`,
  - `scripts/build.ps1`,
  - `scripts/verify.ps1`,
  - `scripts/run.ps1`,
  - `scripts/clean.ps1`.
- Added status helper: `tools/status_report.py`.
- Added durable docs:
  - `AGENTS.md`,
  - `docs/RUNBOOK.md`,
  - `docs/ARCHITECTURE.md`,
  - `docs/ROADMAP.md`,
  - `docs/DECISIONS.md`,
  - `docs/AI_WORKFLOW.md`.

## Additional Commands Run

```powershell
python tools/status_report.py
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
cmake --list-presets
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug
build\windows-vs2022-debug\Debug\EngineApp.exe --smoke-test --frames 3
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 3
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 3
python tools/status_report.py
powershell -ExecutionPolicy Bypass -File scripts/run.ps1 -Args @('--smoke-test','--frames','3')
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
```

## Validation Results

- `scripts/doctor.ps1`: passed. Warnings remain for tools not found in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, `vcpkg`.
- `scripts/configure.ps1`: passed using preset `windows-vs2022-debug`.
- `scripts/build.ps1`: passed and produced:
  - `build/windows-vs2022-debug/Debug/EngineCore.lib`,
  - `build/windows-vs2022-debug/Debug/EngineApp.exe`,
  - `build/windows-vs2022-debug/Debug/EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug`: passed, 2/2 tests.
- `EngineApp.exe --smoke-test --frames 3`: passed using the null renderer.
- `scripts/verify.ps1`: passed; ran doctor, configure, build, CTest, and smoke run.
- `EngineApp.exe --renderer gdi --frames 3`: passed; created a Win32 window and initialized the GDI fallback renderer.
- `EngineApp.exe --renderer dx11 --frames 3`: passed; created a Win32 window and initialized the DirectX 11 renderer. Hardware/debug device creation failed in this environment and the renderer fell back to WARP, then exited cleanly.
- `scripts/run.ps1 -Args @('--smoke-test','--frames','3')`: passed using the null renderer.
- Final `scripts/verify.ps1` after documentation updates: passed.

## Known Issues / Blockers

- C++ tools are not visible in the plain PATH, but CMake successfully built through the Visual Studio 2022 generator.
- DX11 hardware/debug device creation failed during the short validation run, but the WARP fallback initialized and rendered the bounded smoke path.
- `vcpkg` is not installed or not in PATH. This is acceptable for v0.1 because no third-party dependencies were added.

## Next Honest Step

Use the v0.1 foundation for the next goal: v0.2 third-person player controller + camera prototype. Start by reading `AGENTS.md`, `docs/AI_WORKFLOW.md`, and this status file, then run `scripts/verify.ps1` before changing code.

## v0.2 Baseline - 2026-05-14

Required v0.1 docs read before coding:

- `AGENTS.md`
- `docs/STATUS.md`
- `docs/RUNBOOK.md`
- `docs/ARCHITECTURE.md`
- `docs/ROADMAP.md`
- `docs/AI_WORKFLOW.md`

Baseline command:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
```

Baseline result:

- Passed.
- Doctor found expected project files and scripts.
- Doctor warnings remain: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg` are not visible in the plain PATH.
- CMake configured with `windows-vs2022-debug`.
- Build produced `EngineCore.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- CTest passed 2/2 tests.
- Headless smoke run passed with the null renderer.

## v0.2 Short Implementation Plan

1. Add tests first for the new deterministic logic: vector math, camera clamp/smoothing, player movement normalization, and frame-bounded smoke behavior.
2. Add minimal math under `src/engine/math` with `Vec2`, `Vec3`, clamp, lerp, radians/degrees, normalization, yaw helpers, and camera orbit helpers only as needed.
3. Extend input under `src/engine/input` with an `InputState` that carries movement axes, sprint, jump, camera yaw/pitch input, mouse deltas, and quit.
4. Extend the Win32 window path to update keyboard and mouse state safely. If mouse capture is risky, keep arrow-key camera fallback working and document limitations.
5. Add v0.2 game modules under `src/game`: a player controller, third-person camera controller, and a small sandbox scene with floor/obstacles.
6. Extend the renderer debug interface with simple world-space debug primitives: grid, boxes, player proxy, camera marker, and text/debug lines where supported.
7. Keep smoke mode deterministic and bounded. Smoke mode should exercise the update path without requiring a real window or user input.
8. Update docs and run the full validation matrix: doctor, configure, build, CTest, verify, smoke run wrapper, short GDI windowed run, and short DX11 windowed run if practical.

## v0.2 Changes Made

- Updated project version to `0.2.0`.
- Added minimal math in `src/engine/math/Math.h`.
- Extended `InputState` and the Win32 window path for:
  - `W/A/S/D` movement,
  - `Shift` sprint,
  - `Space` jump,
  - mouse camera delta,
  - arrow-key camera fallback,
  - `Esc` quit.
- Added `PlayerController` with camera-relative movement, sprint, jump/gravity, grounded state, facing yaw, and simple obstacle push-out.
- Added `ThirdPersonCamera` with yaw/pitch orbit, distance, height offset, pitch clamp, and exponential smoothing.
- Added `TestScene` with simple obstacle boxes.
- Extended renderer debug support with debug camera, lines, boxes, and best-effort text.
- Updated `SandboxLayer` to render a floor/grid, obstacle boxes, player proxy, facing line, camera target marker, debug logs, and window-title debug state.
- Added lightweight tests for math/player/camera behavior in `tests/EngineCoreTests.cpp`.

## v0.2 Commands Run So Far

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/build.ps1; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }; ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/run.ps1 -Args @('--smoke-test','--frames','3')
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 30
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 30
```

## v0.2 Intermediate Results

- Initial TDD build failed as expected because `engine/math/Math.h` did not exist yet.
- First implementation build failed on Windows-specific issues:
  - missing `GET_X_LPARAM` / `GET_Y_LPARAM` include,
  - `min/max` macro conflict with `std::max`.
- Build passed after adding `windowsx.h` and `NOMINMAX`.
- CTest initially failed because the movement normalization test asserted one second of position movement while the controller intentionally clamps large frame deltas.
- Test was corrected to assert `horizontalSpeed`, the intended movement normalization contract.
- Build + CTest then passed: 2/2 tests.
- Smoke run passed with null renderer.
- Short GDI windowed run passed.
- Short DX11 windowed run passed; DX11 again used WARP after hardware/debug device creation failed.

## v0.2 Final Validation - 2026-05-14

Commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
powershell -ExecutionPolicy Bypass -File scripts/run.ps1 -Args @('--smoke-test','--frames','3')
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 30
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 30
```

Results:

- `scripts/doctor.ps1`: passed. Warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- `scripts/configure.ps1`: passed using `windows-vs2022-debug`.
- `scripts/build.ps1`: passed and produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 2/2 tests.
- `scripts/verify.ps1`: passed; ran doctor, configure, build, CTest, and null-renderer smoke run.
- `scripts/run.ps1 -Args @('--smoke-test','--frames','3')`: passed with the null renderer.
- `EngineApp.exe --renderer gdi --frames 30`: passed; created a Win32 window and initialized the GDI fallback renderer.
- `EngineApp.exe --renderer dx11 --frames 30`: passed; created a Win32 window and initialized DirectX 11 through WARP after hardware/debug device creation failed.

## v0.2 Known Issues / Limitations

- Mouse-look is window-hover delta only. The cursor is not captured, hidden, or locked in v0.2; arrow keys are the safe fallback.
- DX11 hardware/debug device creation still fails in this environment, but WARP initializes and the bounded DX11 run exits cleanly.
- Collision is intentionally simple: floor clamp plus lightweight horizontal push-out from debug obstacle boxes. No physics engine has been added.
- Debug rendering is line/box based. There is no mesh, material, animation, UI, or asset pipeline yet.

## v0.2 Next Honest Step

Use the v0.2 foundation for the next goal: v0.3 World / Collision Prototype. Keep it focused on static world representation, primitive collision queries, and debug collision visualization.

## v0.3 Baseline - 2026-05-14

Required docs read before coding:

- `AGENTS.md`
- `docs/STATUS.md`
- `docs/RUNBOOK.md`
- `docs/ARCHITECTURE.md`
- `docs/ROADMAP.md`
- `docs/AI_WORKFLOW.md`
- `docs/DECISIONS.md`

Baseline command:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
```

Baseline result:

- Passed.
- Doctor found expected project files and scripts.
- Doctor warnings remain: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg` are not visible in the plain PATH.
- CMake configured with `windows-vs2022-debug`.
- Build produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- CTest passed 2/2 tests.
- Headless smoke run passed with the null renderer and v0.2 player/camera debug log.

## v0.3 Short Implementation Plan

1. Add tests first for world/collision behavior: AABB overlap, ground clamp/check, player push-out from boxes, diagonal motion into colliders, wall blocking, and ray/segment query if practical.
2. Add a focused world/collision module under `src/game` so v0.3 stays game-prototype scoped and does not overclaim as a general engine physics system.
3. Move raw obstacle ownership out of `PlayerController`; let it calculate desired movement and ask `TestWorld` / collision helpers to resolve player proxy movement.
4. Replace the v0.2 ad-hoc scene with a named static collision layout: floor, boxes, narrow passage, corner case, wall, and a low step-like blocker.
5. Extend debug rendering and debug text to show collider count, player collision proxy, grounded state, and last push vector.
6. Keep camera collision deferred, but expose a simple raycast query and document how a future camera obstruction pass can use it.
7. Update docs and run the full validation matrix: doctor, configure, build, CTest, verify, smoke wrapper, short GDI run, and short DX11 run.

## v0.3 Changes Made

- Updated project version to `0.3.0`.
- Added `src/game/TestWorld.h` and `src/game/TestWorld.cpp`.
- Added static world collision data:
  - named collider id/name metadata,
  - AABB bounds,
  - floor height,
  - default collision test layout.
- Added collision queries:
  - AABB overlap,
  - closest point,
  - player proxy resolution against floor and boxes,
  - ground check,
  - player-vs-collider overlap check,
  - simple static AABB raycast.
- Updated `PlayerController` so it no longer owns raw obstacle lists. It now calculates movement intent and asks `TestWorld` to resolve position, velocity, grounded state, push vector, normal, and hit count.
- Updated `TestScene` to own a `TestWorld` and build a clearer test layout: wall, narrow passage, corner, low step-like blocker, and crate.
- Updated `SandboxLayer` debug rendering/text to show static colliders, floor bounds, player collision proxy, collision normal/push direction, grounded state, hit count, and collider count.
- Extended lightweight tests in `tests/EngineCoreTests.cpp` for AABB overlap/closest point, ground clamp, push-out, wall blocking, diagonal obstacle resolution, and raycast nearest hit.

## v0.3 Commands Run So Far

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/build.ps1; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }; ctest --preset windows-vs2022-debug --output-on-failure
```

## v0.3 Intermediate Results

- Initial TDD build failed as expected because `game/TestWorld.h` did not exist yet.
- Build passed after adding `TestWorld`, wiring CMake, and moving player collision integration to the world boundary.
- First CTest run failed because the wall blocking test used a zero-tolerance `0.65f` boundary. The collision result landed on the expected boundary with float precision, so the test was loosened to `0.66f`.
- Build + CTest then passed: 2/2 tests.

## v0.3 Final Validation - 2026-05-14

Commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
powershell -ExecutionPolicy Bypass -File scripts/run.ps1 -Args @('--smoke-test','--frames','3')
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 60
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 60
```

Results:

- `scripts/doctor.ps1`: passed. Warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- `scripts/configure.ps1`: passed using `windows-vs2022-debug`.
- `scripts/build.ps1`: passed and produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 2/2 tests.
- `scripts/verify.ps1`: passed; ran doctor, configure, build, CTest, and null-renderer smoke run.
- `scripts/run.ps1 -Args @('--smoke-test','--frames','3')`: passed with the null renderer.
- `EngineApp.exe --renderer gdi --frames 60`: passed; created a Win32 window and initialized the GDI fallback renderer.
- `EngineApp.exe --renderer dx11 --frames 60`: passed; created a Win32 window and initialized DirectX 11 through WARP after hardware/debug device creation failed.

## v0.3 Known Issues / Limitations

- Collision is static AABB-only.
- Player collision uses a vertical radius/height proxy and horizontal push-out, not a full swept capsule or rigid-body solver.
- Fast enough movement can still need future swept collision; v0.3 only hardens the current player speeds and test layout.
- Floor support is a flat floor-height query. No slopes, ramps, stairs, moving platforms, or terrain are implemented.
- Raycast only tests static AABB colliders.
- Camera collision/obstruction is not implemented yet, but `TestWorld::raycast` is available for a future camera obstruction pass.
- DX11 hardware/debug device creation still fails in this environment, but WARP initializes and bounded DX11 runs exit cleanly.

## v0.3 Next Honest Step

Use the v0.3 foundation for the next goal: v0.4 Interaction System. Keep it focused on interactable objects, focus detection, action commands, and debug prompts.

## v0.4 Baseline - 2026-05-14

Required docs read before coding:

- `AGENTS.md`
- `docs/STATUS.md`
- `docs/RUNBOOK.md`
- `docs/ARCHITECTURE.md`
- `docs/ROADMAP.md`
- `docs/AI_WORKFLOW.md`
- `docs/DECISIONS.md`

Baseline command:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
```

Baseline result:

- Passed.
- Doctor found expected project files and scripts.
- Doctor warnings remain: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg` are not visible in the plain PATH.
- CMake configured with `windows-vs2022-debug`.
- Build produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- CTest passed 2/2 tests.
- Headless smoke run passed with the null renderer and v0.3 world/collision debug log.

## v0.4 Short Implementation Plan

1. Add tests first for interaction behavior: nearest/facing focus selection, disabled/consumed filtering, pressed-edge execution, one-shot pickup consumption, repeatable toggle state, and no-focus no-op.
2. Extend `InputState` and Win32 input tracking with `E` interaction held/pressed edge while preserving existing movement, sprint, jump, mouse, arrow, and quit controls.
3. Add `src/game/InteractionSystem.h/.cpp` with focused interactable data, focus detection, action execution, prompt/result state, and no scripting/UI/inventory dependencies.
4. Extend `TestScene` with three debug interactables: pickup, toggle/button, and info marker.
5. Integrate `InteractionSystem` in `SandboxLayer`: update focus from player position/facing, execute on `interactPressed`, draw debug markers/highlight/range, and include prompt/result state in debug text/logs.
6. Update docs for controls, interaction boundary, limitations, and deferred systems.
7. Run the full validation matrix: doctor, configure, build, CTest, verify, smoke wrapper, short GDI run, and short DX11 run.

## v0.4 Changes Made

- Updated project version to `0.4.0`.
- Extended `engine::InputState` with `interactHeld` and `interactPressed`.
- Updated the Win32 input path so `E` produces a pressed-edge interaction action while preserving movement, sprint, jump, mouse, arrow camera fallback, and `Esc` quit.
- Added `src/game/InteractionSystem.h` and `src/game/InteractionSystem.cpp`.
- Added interaction data for:
  - id/name/prompt,
  - position and focus radius,
  - pickup/toggle/info type,
  - one-shot, enabled, consumed, and toggled state,
  - message/result text.
- Added focus detection using player position, facing direction, range, and nearest-candidate scoring.
- Added interaction execution for:
  - one-shot pickup consumption,
  - repeatable toggle state changes,
  - repeatable info/message markers.
- Updated `TestScene` with three debug interactables: `Test Pickup`, `Wall Button`, and `Info Marker`.
- Updated `SandboxLayer` to update focus, execute interactions, log results, draw interactable markers/ranges/highlights, and include prompt/result state in debug text.
- Added lightweight interaction tests for focus selection, disabled/consumed filtering, pressed-edge no-repeat while held, one-shot pickup consumption, toggle state changes, and no-focus no-op.

## v0.4 Commands Run So Far

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
```

## v0.4 Intermediate Results

- Initial TDD build failed as expected because `game/InteractionSystem.h` did not exist yet:
  - `error C1083: Cannot open include file: 'game/InteractionSystem.h': No such file or directory`
- Build passed after adding the interaction system, input edge state, scene integration, and CMake wiring.
- CTest passed after implementation: 2/2 tests.

## v0.4 Final Validation - 2026-05-14

Commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
powershell -ExecutionPolicy Bypass -File scripts/run.ps1 -Args @('--smoke-test','--frames','3')
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 90
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 90
```

Results:

- `scripts/doctor.ps1`: passed. Warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- `scripts/configure.ps1`: passed using `windows-vs2022-debug`.
- `scripts/build.ps1`: passed and produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 2/2 tests.
- `scripts/verify.ps1`: passed; ran doctor, configure, build, CTest, and null-renderer smoke run.
- `scripts/run.ps1 -Args @('--smoke-test','--frames','3')`: passed with the null renderer.
- `EngineApp.exe --renderer gdi --frames 90`: passed; created a Win32 window and initialized the GDI fallback renderer.
- `EngineApp.exe --renderer dx11 --frames 90`: passed; created a Win32 window and initialized DirectX 11 through WARP after hardware/debug device creation failed.
- Final `scripts/verify.ps1` after documentation updates: passed.

## v0.4 Known Issues / Limitations

- Interaction focus is point/radius based with a facing preference, not a physics overlap volume or full camera ray system.
- The toggle object changes interaction state only; it does not yet open a collision door or alter world geometry.
- The pickup is debug state only. There is no inventory, save data, UI framework, item database, or audio feedback.
- Info markers log/debug text only. There are no dialogue trees, scripting, mission triggers, or localization.
- GDI shows debug text; DX11 currently renders line/box markers but has no text overlay.
- DX11 hardware/debug device creation still fails in this environment, but WARP initializes and bounded DX11 runs exit cleanly.

## v0.4 Next Honest Step

Use the v0.4 foundation for the next goal: v0.5 Vehicle or Traversal Prototype Decision + first narrow prototype. Start by choosing exactly one narrow direction rather than adding both systems at once.

## v0.4.1 Baseline - 2026-05-14

Goal: foundation review plus game direction lock. No vehicles, traversal, NPC AI, combat, physics libraries, missions, inventory, or asset pipelines are being added in this goal.

Inspected before editing:

- `AGENTS.md`
- `docs/STATUS.md`
- `docs/RUNBOOK.md`
- `docs/ARCHITECTURE.md`
- `docs/ROADMAP.md`
- `docs/AI_WORKFLOW.md`
- `docs/DECISIONS.md`
- `src/engine/input/Input.h`
- `src/engine/platform/Win32Window.cpp`
- `src/game/PlayerController.cpp`
- `src/game/InteractionSystem.cpp`
- `src/game/SandboxLayer.cpp`
- `src/game/TestScene.cpp`
- `src/game/TestWorld.cpp`
- `tests/EngineCoreTests.cpp`

Baseline command:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
```

Baseline result:

- Passed.
- Doctor warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- CMake configured with `windows-vs2022-debug`.
- Build produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- CTest passed 2/2 tests.
- Headless smoke run passed with the null renderer and v0.4 interaction debug text.

## v0.4.1 Short Implementation Plan

1. Keep code changes minimal: bump the project version to `0.4.1`, but do not add new gameplay systems.
2. Add `docs/GAME_DIRECTION.md` to lock the working title, genre, core fantasy, world, player role, loop, pillars, non-goals, first 10-minute slice, and engine priorities.
3. Add `docs/VERTICAL_SLICE.md` to define the first playable micro-scenario and what systems it should prove.
4. Add `docs/TECH_DEBT.md` to capture known engine issues before v0.5, including input, camera, collision, renderer, and naming debt.
5. Add a manual test checklist for player/camera/collision/interactions so future windowed validation has an exact script.
6. Update architecture/runbook/roadmap/decisions/status with review findings, interaction focus design notes, TestWorld/TestScene rename guidance, and a clear v0.5 recommendation.
7. Run the required validation matrix and record exact results here.

## v0.4.1 Review Findings

- Input edge/held behavior: `E` now has both held and pressed-edge state. `Space` was also changed to pressed-edge behavior in v0.4, which is correct for jump but should be explicitly documented because earlier v0.2 notes described jump more loosely.
- Player movement and jump: movement remains camera-relative and tests cover diagonal normalization. Jump still uses `jumpPressed`; holding Space should not repeatedly re-jump after the v0.4 edge change.
- Camera feel: camera follows after player update, which avoids obvious update-order jitter. Mouse remains hover-delta only, with arrow fallback.
- Interaction focus: current implementation uses player-facing plus proximity fallback for very close interactables. This is acceptable for v0.4 but needs a documented design choice before adding richer interactable layouts.
- Collision and interaction marker placement: the current debug scene places the pickup directly ahead of the starting position and places toggle/info markers near the collision layout. Markers are debug-only and do not alter collision.
- Interaction boundary: `InteractionSystem` is still correctly in `src/game`. It should not be promoted into `src/engine` until future gameplay proves a stable API.
- Naming: `TestWorld` and `TestScene` were fine for v0.3/v0.4, but should later become `PrototypeWorld` and `PrototypeScene` once the slice stops being only a collision testbed.
- v0.5 direction: traversal is the better next milestone than vehicles because the first vertical slice needs readable on-foot exploration and access gating before it needs driving.

## v0.4.1 Changes Made

- Updated project version to `0.4.1`.
- Added `docs/GAME_DIRECTION.md` with the locked original direction: `Tidebreak`, a small third-person cinematic systemic sandbox/adventure set in the fictional island region of Veyra Reach.
- Added `docs/VERTICAL_SLICE.md` defining the first 10-minute micro-scenario: `The Ferry Office`.
- Added `docs/TECH_DEBT.md` with known build, renderer, input, player/camera, collision, interaction, and naming debt before v0.5.
- Added `docs/MANUAL_TEST_CHECKLIST.md` for player, camera, collision, interaction, and shutdown checks.
- Updated `docs/ARCHITECTURE.md` with interaction focus design notes and TestWorld/TestScene rename guidance.
- Updated `docs/RUNBOOK.md` to point to the manual checklist.
- Updated `docs/ROADMAP.md` to add v0.4.1 and choose `v0.5 - Traversal Prototype`.
- Updated `docs/DECISIONS.md` with game direction, focus policy, naming direction, and v0.5 traversal decision.
- Updated `docs/AI_WORKFLOW.md`, `AGENTS.md`, `README.md`, `scripts/doctor.ps1`, and `tools/status_report.py` so future AI runs and validation tools know about the new direction/checklist docs.
- No vehicles, traversal implementation, NPC AI, combat, physics libraries, missions, inventory, or asset pipelines were added.

## v0.4.1 Final Validation - 2026-05-14

Commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 90
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 90
python tools/status_report.py
```

Results:

- `scripts/doctor.ps1`: passed and now checks the v0.4.1 direction/checklist docs. Warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- `scripts/configure.ps1`: passed using `windows-vs2022-debug`.
- `scripts/build.ps1`: passed and produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 2/2 tests.
- `scripts/verify.ps1`: passed; ran doctor, configure, build, CTest, and null-renderer smoke run. Smoke logs show engine `v0.4.1`.
- `EngineApp.exe --renderer gdi --frames 90`: passed; created a Win32 window and initialized the GDI fallback renderer.
- `EngineApp.exe --renderer dx11 --frames 90`: passed; created a Win32 window and initialized DirectX 11 through WARP after hardware/debug device creation failed.
- `python tools/status_report.py`: passed and reports the new v0.4.1 docs as present.
- Final `scripts/verify.ps1` after status documentation updates: passed.

## v0.4.1 Known Issues / Limitations

- This goal intentionally added no new gameplay systems.
- DX11 hardware/debug device creation still falls back to WARP in this environment.
- Manual checklist was authored but not fully hand-played with human input in this run; bounded GDI and DX11 launches did pass.
- Traversal is only a recommendation and design target for v0.5; it is not implemented in v0.4.1.

## v0.4.1 Next Honest Step

Use the v0.4.1 foundation for the next goal: v0.5 Traversal Prototype. Prefer a single narrow traversal mechanic such as mantle, climb-up, vault, or ledge step, with no vehicles in the same goal.

## v0.5 Baseline - 2026-05-14

Goal: add one narrow on-foot traversal prototype for the Tidebreak / The Ferry Office direction. This goal must not add vehicles, full parkour, ledge hanging, wall climbing, animation, IK, physics engine integration, NPC AI, combat, inventory, save/load, mission scripting, final art, or asset pipelines.

Required docs read before coding:

- `AGENTS.md`
- `docs/GAME_DIRECTION.md`
- `docs/VERTICAL_SLICE.md`
- `docs/TECH_DEBT.md`
- `docs/MANUAL_TEST_CHECKLIST.md`
- `docs/STATUS.md`
- `docs/ROADMAP.md`
- `docs/ARCHITECTURE.md`
- `docs/DECISIONS.md`

Baseline command:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
```

Baseline result:

- Passed.
- Doctor found all expected v0.4.1 direction/checklist docs.
- Doctor warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- CMake configured with `windows-vs2022-debug`.
- Build produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- CTest passed 2/2 tests.
- Headless smoke run passed with the null renderer and engine `v0.4.1`.

## v0.5 Short Implementation Plan

1. Add failing tests first for traversal affordance focus, disabled affordance filtering, Space-trigger priority over jump, traversal completion at target, and no retrigger while traversal is active.
2. Add `src/game/TraversalSystem.h/.cpp` in the game layer with one narrow traversal affordance type: contextual vault/mantle over a low obstacle.
3. Extend `PlayerController` with a minimal traversal state: normal/traversing, start, target, elapsed/progress, duration, and active affordance id.
4. Use `Space` as the traversal trigger when a traversal affordance is focused; otherwise keep normal pressed-edge jump behavior.
5. Update `TestScene` with a low service-route traversal affordance and a maintenance-box-like interactable after the traversal path.
6. Update `SandboxLayer` to focus traversal before player update, pass traversal activation into the player, draw traversal markers/start/end/path, and show traversal state in debug text.
7. Update docs for architecture, runbook controls, roadmap status, decision rationale, tech debt, and manual traversal checklist.
8. Run the required validation matrix and record exact results here.

## v0.5 Changes Made

- Updated project version to `0.5.0`.
- Added `src/game/TraversalSystem.h` and `src/game/TraversalSystem.cpp`.
- Added one traversal affordance type: `Vault`.
- Added traversal data for id/name, type, start/end positions, focus radius, required facing direction/dot, enabled flag, prompt, and duration.
- Added traversal focus detection and `Space`-based activation from the current focused affordance.
- Extended `PlayerController` with traversal state:
  - `Normal`,
  - `Traversing`,
  - active traversal id,
  - traversal progress,
  - deterministic interpolation from start to target with a small arc.
- Gave traversal priority over normal jump when a traversal affordance is focused. Normal jump still works when no traversal affordance is focused.
- Updated `TestScene` with one service-barrier traversal affordance and one maintenance-box-like interactable reachable after the traversal route.
- Updated `SandboxLayer` to update traversal focus, pass traversal activation into the player, draw traversal start/end/path/focus markers, and include traversal state in debug text.
- Added lightweight traversal tests for focus, disabled filtering, jump priority, completion at target, no retrigger while active, and normal jump behavior without traversal focus.
- Updated docs for architecture, controls, roadmap, decisions, technical debt, and manual traversal checks.

## v0.5 Intermediate Results

- Initial TDD build failed as expected because `game/TraversalSystem.h` did not exist yet:
  - `error C1083: Cannot open include file: 'game/TraversalSystem.h': No such file or directory`
- Build + CTest passed after adding `TraversalSystem`, player traversal state, and CMake wiring.
- Build + CTest passed again after scene/debug integration.

## v0.5 Final Validation - 2026-05-14

Commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 120
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 120
python tools/status_report.py
```

Results:

- `scripts/doctor.ps1`: passed. Warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- `scripts/configure.ps1`: passed using `windows-vs2022-debug`.
- `scripts/build.ps1`: passed and produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 2/2 tests.
- `scripts/verify.ps1`: passed; ran doctor, configure, build, CTest, and null-renderer smoke run. Smoke logs show engine `v0.5.0`, 4 interactables, and traversal debug state.
- `EngineApp.exe --renderer gdi --frames 120`: passed; created a Win32 window and initialized the GDI fallback renderer.
- `EngineApp.exe --renderer dx11 --frames 120`: passed; created a Win32 window and initialized DirectX 11 through WARP after hardware/debug device creation failed.
- `python tools/status_report.py`: passed and reported the v0.5 working tree changes and expected docs/scripts.

## v0.5 Known Issues / Limitations

- Traversal is a single contextual `Vault` affordance, not full parkour.
- Traversal motion is deterministic interpolation with a small arc. There is no animation, IK, ledge hang, wall climb, or physics-engine integration.
- Collision is skipped during the controlled traversal motion and resumes after landing at the target; this is acceptable for the prototype but needs polish.
- The service barrier route and maintenance box are hardcoded in `TestScene`.
- Manual checklist traversal playthrough was documented but not fully human-played in this run; bounded GDI/DX11 launches passed.
- DX11 hardware/debug device creation still falls back to WARP in this environment.

## v0.5 Next Honest Step

Use the v0.5 foundation for the next goal: v0.5.1 Traversal Feel + Camera / Collision Polish. Keep it focused on start/end tuning, camera stability during traversal, and collision handoff before adding more traversal types.

## v0.5.1 Baseline - 2026-05-14

Goal: polish the existing Service Barrier Vault traversal feel, camera stability, and collision handoff without adding new gameplay systems, traversal types, vehicles, NPC AI, combat, missions, inventory, save/load, physics libraries, animation systems, IK, or asset pipelines.

Required docs read before coding:

- `AGENTS.md`
- `docs/GAME_DIRECTION.md`
- `docs/VERTICAL_SLICE.md`
- `docs/TECH_DEBT.md`
- `docs/MANUAL_TEST_CHECKLIST.md`
- `docs/STATUS.md`
- `docs/ROADMAP.md`
- `docs/ARCHITECTURE.md`
- `docs/DECISIONS.md`

Baseline command:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
```

Baseline result:

- Passed.
- Doctor found all expected v0.4.1/v0.5 docs and project structure.
- Doctor warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- CMake configured with `windows-vs2022-debug`.
- Build produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- CTest passed 2/2 tests.
- Headless smoke run passed with the null renderer and engine `v0.5.0`.

## v0.5.1 Short Implementation Plan

1. Add focused tests first for traversal start handoff, landing/grounded state, collision resolve after traversal completion, and existing Space/E behavior.
2. Keep the single Service Barrier Vault affordance and use the player's current position as the traversal start at activation time, while preserving the fixed target position.
3. Add minimal player traversal debug state so the runtime can report whether the current-position start path is being used.
4. Resolve the player against `TestWorld` after traversal completion so landing is grounded, velocity is clean, and overlap/pop risk is reduced.
5. Review camera update order and debug output during traversal without adding cinematic camera behavior or camera collision.
6. Update docs/manual checklist with the traversal start/end handoff behavior and known limitations.
7. Run the full validation matrix and record exact results here.

## v0.5.1 Changes Made

- Updated project version to `0.5.1`.
- Added traversal handoff tests for:
  - current-position traversal start to avoid visible snap,
  - grounded landing with clean velocity,
  - world collision resolve after traversal completion,
  - interaction still triggering after traversal returns to normal state.
- Kept the single Service Barrier Vault affordance and no new traversal types.
- Added `TraversalActivation::useCurrentPlayerPositionAsStart`, defaulting to current-position start behavior.
- Extended `PlayerState` with traversal start/target/debug fields and a one-frame landing flag.
- Updated `PlayerController` so traversal activation starts from the current player position while preserving the authored target.
- Updated traversal completion so the target position is resolved through `TestWorld::resolvePlayer` before returning to normal movement.
- Updated runtime debug text/rendering to show current-position traversal start, active traversal path, progress, and landing state.
- Updated docs for architecture, runbook troubleshooting, roadmap, technical debt, decisions, and manual traversal checks.

## v0.5.1 Intermediate Results

Commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }; ctest --preset windows-vs2022-debug --output-on-failure
```

Results:

- Initial TDD run failed as expected after adding the current-position traversal start test:
  - `TestTraversalStartsFromCurrentPlayerPositionToAvoidSnap` reported the player snapping to the authored start marker instead of preserving current X/Z.
- Build + CTest passed after implementing current-position start, landing world resolve, and traversal debug state: 2/2 tests.

## v0.5.1 Final Validation - 2026-05-14

Commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 180
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 180
python tools/status_report.py
```

Results:

- `scripts/doctor.ps1`: passed. Warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- `scripts/configure.ps1`: passed using `windows-vs2022-debug`.
- `scripts/build.ps1`: passed and produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 2/2 tests.
- `scripts/verify.ps1`: passed; ran doctor, configure, build, CTest, and null-renderer smoke run. Smoke logs show engine `v0.5.1`.
- `EngineApp.exe --renderer gdi --frames 180`: passed; created a Win32 window, initialized the GDI fallback renderer, and exited cleanly.
- `EngineApp.exe --renderer dx11 --frames 180`: passed; created a Win32 window and initialized DirectX 11 through WARP after hardware DX11 device creation failed.
- `python tools/status_report.py`: passed and reported the v0.5.1 working tree changes plus expected docs/scripts.

## v0.5.1 Known Issues / Limitations

- Traversal is still one contextual Service Barrier Vault. No new traversal type was added.
- Collision is resolved at traversal landing, but not continuously during the controlled traversal arc.
- Camera update order remains stable and simple; there is no traversal-specific cinematic camera or camera collision.
- Debug text is readable in GDI. DX11 still has no text overlay, so DX11 relies on line/box markers and logs.
- Manual checklist was updated, but the full human input playthrough was not performed in this run. Bounded GDI and DX11 launches passed.
- DX11 hardware device creation still falls back to WARP in this environment.

## v0.5.1 Next Honest Step

Use the stable traversal foundation for the next goal: v0.6 World Event / Remembered State Prototype. Keep it focused on local remembered state and simple scene events for The Ferry Office rather than NPC AI, missions, inventory, or save/load.

## v0.6 Baseline - 2026-05-14

Goal: add a small remembered-state/event layer for The Ferry Office prototype loop. This goal must not add NPC AI, vehicles, combat, inventory, save/load, mission scripting, dialogue trees, physics libraries, new traversal types, final art, or asset pipelines.

Required context read before coding:

- `AGENTS.md`
- `docs/GAME_DIRECTION.md`
- `docs/VERTICAL_SLICE.md`
- `docs/TECH_DEBT.md`
- `docs/MANUAL_TEST_CHECKLIST.md`
- `docs/STATUS.md`
- `docs/ROADMAP.md`
- `docs/ARCHITECTURE.md`
- `docs/DECISIONS.md`
- `docs/RUNBOOK.md`
- `src/game/InteractionSystem.*`
- `src/game/TestScene.*`
- `src/game/SandboxLayer.*`
- `tests/EngineCoreTests.cpp`

Baseline command:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
```

Baseline result:

- Passed.
- Doctor found all expected project docs and structure.
- Doctor warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- CMake configured with `windows-vs2022-debug`.
- Build produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- CTest passed 2/2 tests.
- Headless smoke run passed with the null renderer and engine `v0.5.1`.

## v0.6 Short Implementation Plan

1. Add failing tests first for a focused `WorldState` game-layer module: flag reads/writes, event id/order/count, repeated flag handling, interaction-to-state mapping, and traversal completion state.
2. Add `src/game/WorldState.h/.cpp` with boolean local flags, deterministic event records, last-event access, and debug summary text.
3. Keep state mapping in `TestScene` so `InteractionSystem` stays generic: manifest pickup, wall button/route toggle, maintenance box power restore, and service-route traversal completion.
4. Integrate `WorldState` in `SandboxLayer` by applying interaction results and traversal landing events after existing gameplay updates.
5. Add debug text/log output for current flags, event count, last event, and whether an action changed remembered state.
6. Add one visible debug feedback change using existing debug markers/gate color; do not add UI, assets, scripting, save/load, or mission logic.
7. Update docs and run the full validation matrix.

## v0.6 Changes Made

- Updated project version to `0.6.0`.
- Added `src/game/WorldState.h` and `src/game/WorldState.cpp`.
- Added local remembered flags:
  - `powerRestored`,
  - `manifestCollected`,
  - `serviceRouteUsed`,
  - `maintenanceBoxInspected`,
  - `routeOpened`.
- Added deterministic `WorldEvent` records with id, name, flag, value, source, event count, last event text, and debug summary text.
- Kept repeated same-value flag writes from duplicating remembered events.
- Updated `TestScene` to own world state and map existing prototype actions:
  - Ferry Manifest pickup -> `manifestCollected=true`,
  - Wall Button toggle -> `routeOpened=true/false`,
  - Maintenance Box interaction -> `maintenanceBoxInspected=true` and `powerRestored=true`,
  - Service Barrier Vault completion -> `serviceRouteUsed=true`.
- Renamed the spawn pickup from `Test Pickup` to `Ferry Manifest` for the Ferry Office loop.
- Updated `SandboxLayer` to apply remembered state after interaction results and traversal landing, log world events, show world flags/event summary in debug text, and draw route/power debug markers with state-dependent colors.
- Added lightweight tests for world flags, event order/count, repeated flag writes, one-shot manifest state, maintenance power restore, route toggle state, and traversal service-route state.
- Updated docs for architecture, roadmap, decisions, vertical slice, tech debt, manual checklist, and this status file.

## v0.6 Intermediate Results

Commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }; ctest --preset windows-vs2022-debug --output-on-failure
```

Results:

- Initial TDD build failed as expected after adding world-state tests because `game/WorldState.h` did not exist yet:
  - `error C1083: Cannot open include file: 'game/WorldState.h': No such file or directory`
- First implementation build then exposed a missing test include for `game/TestScene.h`; the test harness was corrected.
- Build + CTest passed after adding `WorldState`, scene mappings, sandbox debug/log integration, and CMake wiring: 2/2 tests.

## v0.6 Final Validation - 2026-05-14

Commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 180
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 180
python tools/status_report.py
```

Results:

- `scripts/doctor.ps1`: passed. Warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- `scripts/configure.ps1`: passed using `windows-vs2022-debug`.
- `scripts/build.ps1`: passed and produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 2/2 tests.
- `scripts/verify.ps1`: passed; ran doctor, configure, build, CTest, and null-renderer smoke run. Smoke logs show engine `v0.6.0`, the Ferry Manifest focus prompt, and world-state debug summary with all initial flags false.
- `EngineApp.exe --renderer gdi --frames 180`: passed; created a Win32 window, initialized the GDI fallback renderer, showed the Ferry Manifest focus prompt and world-state debug summary, and exited cleanly.
- `EngineApp.exe --renderer dx11 --frames 180`: passed; created a Win32 window and initialized DirectX 11 through WARP after hardware DX11 device creation failed.
- `python tools/status_report.py`: passed and reported the v0.6 working tree changes plus expected docs/scripts.

## v0.6 Known Issues / Limitations

- `WorldState` is runtime-only. There is no save/load, persistence, mission scripting, inventory, dialogue, or global event bus.
- State mapping is hardcoded in `TestScene` and should stay there until the Ferry Office micro-slice proves a broader boundary.
- Repeated same-value flag writes are ignored; this prevents duplicate remembered events for one-shot actions but may need richer event semantics later.
- Debug summary text is useful but long; a future UI/debug overlay pass should improve readability.
- The route-open debug gate changes color, but it does not yet alter collision geometry.
- Manual checklist was updated, but a full hand-played traversal/interactions/state pass was not performed in this run. Bounded GDI and DX11 launches passed.
- DX11 hardware device creation still falls back to WARP in this environment.

## v0.6 Next Honest Step

Use the remembered-state foundation for the next goal: v0.7 The Ferry Office Micro-Slice. Keep it focused on one playable prototype loop using existing movement, camera, collision, interaction, traversal, and world-state systems without adding NPC AI, vehicles, combat, inventory, save/load, final art, asset pipelines, or full mission scripting.

## v0.7 Baseline - 2026-05-14

Goal: build the first playable debug micro-slice, "The Ferry Office", on top of existing movement, camera, collision, interaction, traversal, and world-state systems. This goal must not add NPC AI, vehicles, combat, weapons, inventory, save/load, full mission scripting, dialogue trees, physics libraries, new traversal types, final art, asset pipelines, or a UI framework.

Required context read before coding:

- `AGENTS.md`
- `docs/GAME_DIRECTION.md`
- `docs/VERTICAL_SLICE.md`
- `docs/TECH_DEBT.md`
- `docs/MANUAL_TEST_CHECKLIST.md`
- `docs/STATUS.md`
- `docs/ROADMAP.md`
- `docs/ARCHITECTURE.md`
- `docs/DECISIONS.md`
- `src/game/WorldState.*`
- `src/game/TestWorld.*`
- `src/game/TestScene.*`
- `src/game/SandboxLayer.*`
- `tests/EngineCoreTests.cpp`

Baseline command:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
```

Baseline result:

- Passed.
- Doctor found all expected project docs and structure.
- Doctor warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- CMake configured with `windows-vs2022-debug`.
- Build produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- CTest passed 2/2 tests.
- Headless smoke run passed with the null renderer and engine `v0.6.0`.
- Smoke logs showed the initial Ferry Manifest focus prompt and the world-state debug summary with all remembered flags false.

## v0.7 Short Implementation Plan

1. Add failing tests first for Ferry Office slice completion, required remembered flags, exit-marker behavior, one-shot event stability, and route-open scene state.
2. Extend `WorldState` with the smallest local `exitReached` flag needed for a complete micro-slice summary.
3. Keep the slice orchestration in `TestScene`: objective text, completion summary, exit readiness, and mapping existing interactions/traversal into remembered state.
4. Evolve the debug layout into a Ferry Office prototype with dock/start, office, blocked service gate, Service Barrier Vault, Maintenance Box, manifest, and exit/summary marker.
5. Make at least one remembered state visibly affect the scene by syncing `routeOpened` to the service-gate debug/collision state and keeping marker colors state-driven.
6. Extend `SandboxLayer` debug text/rendering with current objective, completion state, exit marker, and slice summary without adding a UI framework.
7. Update docs, run the full validation matrix, then commit and push because AGENTS.md now requires completed goals to be published.

## v0.7 Changes Made

- Updated project version to `0.7.0`.
- Added `exitReached` to the local `WorldState` flags and debug summary.
- Added Ferry Office slice helpers to `TestScene`:
  - current objective text,
  - ready-for-exit status,
  - completion status,
  - completion summary,
  - exit marker recording,
  - service-gate blocking query.
- Replaced the default scene layout path with a Ferry Office prototype layout containing office walls, dock rails, a service gate, a service barrier, an office counter, and maintenance-side blockers.
- Added an Exit Summary Marker interactable and renamed the info marker to Ferry Office Notice.
- Synced `routeOpened` to the named `service-gate` collider so the route can physically open/close in the prototype.
- Extended `SandboxLayer` debug text with objective, ready-for-exit, slice completion, world state, and slice summary.
- Extended debug rendering with dock start, office marker, exit marker, route-gate state colors, and state-colored colliders.
- Added lightweight tests for slice start state, required remembered flags, exit-marker gating, service-gate collider changes, and one-shot manifest event stability.
- Updated architecture, roadmap, vertical slice, technical debt, manual checklist, decisions, and this status file.

## v0.7 Intermediate Results

Commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }; ctest --preset windows-vs2022-debug --output-on-failure
```

Results:

- Initial TDD build failed as expected after adding Ferry Office slice tests because `TestScene` did not yet expose `isSliceReadyForExit`, `isSliceComplete`, `currentObjectiveText`, `recordExitReached`, `completionSummary`, `isServiceGateBlocking`, and `WorldFlag::ExitReached`.
- Build + CTest passed after adding `exitReached`, scene completion helpers, Ferry Office layout, service-gate collider sync, debug rendering/text updates, and tests: 2/2 tests.

## v0.7 Final Validation - 2026-05-14

Commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 240
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 240
python tools/status_report.py
```

Results:

- `scripts/doctor.ps1`: passed. Warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- `scripts/configure.ps1`: passed using `windows-vs2022-debug`.
- `scripts/build.ps1`: passed and produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 2/2 tests.
- `scripts/verify.ps1`: passed; ran doctor, configure, build, CTest, and null-renderer smoke run. Smoke logs show engine `v0.7.0`, five interactables, objective text, initial Ferry Manifest prompt, all Ferry Office flags false including `exitReached=false`, and `sliceComplete=no`.
- `EngineApp.exe --renderer gdi --frames 240`: passed; created a Win32 window, initialized the GDI fallback renderer, showed the Ferry Office objective/debug state, and exited cleanly.
- `EngineApp.exe --renderer dx11 --frames 240`: passed; created a Win32 window and initialized DirectX 11 through WARP after hardware DX11 device creation failed.
- `python tools/status_report.py`: passed and reported the v0.7 working tree changes plus expected docs/scripts/build output.

## v0.7 Known Issues / Limitations

- The Ferry Office is a debug micro-slice only. It uses boxes, lines, text, and markers, not final art or a UI framework.
- Slice completion is a scene helper, not mission scripting.
- `WorldState` remains runtime-only; there is no save/load or persistence.
- `routeOpened` affects the `service-gate` collider, but there is no general dynamic door/entity system.
- `TestWorld` and `TestScene` names are now stale but were intentionally not renamed during v0.7 to avoid churn.
- DX11 hardware device creation still falls back to WARP in this environment.
- A full hands-on playthrough was not performed in this run. Bounded GDI and DX11 launches passed, and the manual checklist was updated for v0.7.1.

## v0.7 Next Honest Step

Use the completed micro-slice for the next goal: v0.7.1 Micro-Slice Playtest Polish. Keep it focused on hands-on loop feel, prompt placement, marker readability, objective ordering, and small camera/collision fixes exposed by playtesting. Do not start a new major system yet.

## v0.7.1 Baseline - 2026-05-14

Goal: polish the existing Ferry Office debug micro-slice through manual-playtest-driven fixes only. This goal must not add NPC AI, vehicles, combat, inventory, save/load, mission scripting, physics libraries, final art, asset pipelines, new traversal types, or UI frameworks.

Required context read before coding:

- `AGENTS.md`
- `docs/GAME_DIRECTION.md`
- `docs/VERTICAL_SLICE.md`
- `docs/TECH_DEBT.md`
- `docs/MANUAL_TEST_CHECKLIST.md`
- `docs/STATUS.md`
- `docs/ROADMAP.md`
- `docs/ARCHITECTURE.md`
- `docs/DECISIONS.md`

Baseline command:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
```

Baseline result:

- Passed.
- Doctor found all expected project docs and structure.
- Doctor warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- CMake configured with `windows-vs2022-debug`.
- Build produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- CTest passed 2/2 tests.
- Headless smoke run passed with the null renderer and engine `v0.7.0`.
- Smoke logs showed five interactables, initial Ferry Manifest focus, objective text, all Ferry Office flags false including `exitReached=false`, and `sliceComplete=no`.

## v0.7.1 Short Implementation Plan

1. Run a short GDI windowed playtest against `docs/MANUAL_TEST_CHECKLIST.md`, focusing on Ferry Office completion clarity, traversal focus, service-gate behavior, exit marker gating, and debug text readability.
2. Fix only blockers or obvious clarity problems found during playtest; do not introduce new systems or rename `TestWorld` / `TestScene` unless the rename is truly low-risk and necessary.
3. Add failing tests before any behavior change, especially for service-gate anti-trap behavior, objective ordering, exit readiness, or debug summary changes.
4. Keep changes in existing game-layer boundaries: `TestScene`, `TestWorld`, `SandboxLayer`, tests, and docs.
5. Re-run the required validation matrix, record exact results here, then commit and push after validation passes per `AGENTS.md`.

## v0.7.1 Playtest Findings

GDI playtest command:

```powershell
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 900
```

Result:

- The GDI windowed run exited cleanly.
- A scripted key-event pass using real Windows key events was not reliable enough to count as a full human playthrough; movement key holds were inconsistent in this environment.
- The run still exposed a real slice clarity issue:
  - `manifestCollected=true` and `routeOpened=true` could be recorded while `serviceRouteUsed=false`.
  - The objective remained stuck on "Use the Service Barrier Vault..." after the Wall Button and Exit Summary Marker interactions.
  - The Service Barrier Vault start marker was effectively on the wrong side of the barrier for the tested approach.
  - The Maintenance Box could be reached/focused before the intended traversal state was recorded.
  - Debug text was a single very long line and hard to read in GDI.
- Because the automated key-event pass is not a substitute for a person at the controls, the final status remains honest: v0.7.1 improves the loop and validates it through tests/bounded runs, but a full human keyboard/mouse playthrough is still recommended.

## v0.7.1 Changes Made

- Updated project version to `0.7.1`.
- Moved the Service Barrier Vault start to the player-accessible side of the barrier.
- Moved the Service Barrier Vault target and Maintenance Box so maintenance focus follows traversal instead of stealing focus before it.
- Changed the Wall Button from an open/close route toggle into a latch-open service-gate opener.
- Kept `routeOpened=true` as the state that disables the `service-gate` blocking collider; repeated Wall Button interactions no longer close the gate or duplicate events.
- Split sandbox debug text into readable sections and updated the GDI renderer to draw multi-line wrapped text.
- Kept `TestWorld` and `TestScene` names; the rename is documented as deferred.
- Added/updated lightweight tests for:
  - Wall Button latch-open behavior,
  - service-gate collider staying open,
  - Service Barrier Vault focus from the accessible side,
  - Maintenance Box not focusing before the vault,
  - complete Ferry Office loop through scene/player/traversal/world-state systems,
  - multi-line sandbox debug text.
- Updated architecture, vertical slice, technical debt, manual checklist, roadmap, decisions, and this status file.

## v0.7.1 Intermediate Results

Commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }; ctest --preset windows-vs2022-debug --output-on-failure
```

Results:

- Initial TDD run failed as expected after adding polish tests:
  - Wall Button still closed `routeOpened`.
  - Service Barrier Vault did not focus from the accessible side.
  - Maintenance Box could win focus before the vault.
  - Sandbox debug text had no line breaks.
- Build + CTest passed after moving the traversal/maintenance markers, making the Wall Button latch the gate open, and splitting debug text.
- A full scene-system loop test initially failed because the test read `traversalLandedThisFrame` after its one-frame lifetime. The test was corrected to record `serviceRouteUsed` on the landing frame, matching runtime behavior, and CTest then passed.

## v0.7.1 Final Validation - 2026-05-14

Commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 240
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 240
python tools/status_report.py
```

Results:

- `scripts/doctor.ps1`: passed. Warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- `scripts/configure.ps1`: passed using `windows-vs2022-debug`.
- `scripts/build.ps1`: passed and produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 2/2 tests.
- `scripts/verify.ps1`: passed; ran doctor, configure, build, CTest, and null-renderer smoke run. Smoke logs show engine `v0.7.1`, five interactables, objective text, initial Ferry Manifest prompt, all Ferry Office flags false, `sliceComplete=no`, and multi-line debug output.
- `EngineApp.exe --renderer gdi --frames 240`: passed; created a Win32 window, initialized the GDI fallback renderer, showed multi-line Ferry Office debug text, and exited cleanly.
- `EngineApp.exe --renderer dx11 --frames 240`: passed; created a Win32 window and initialized DirectX 11 through WARP after hardware DX11 device creation failed.
- `python tools/status_report.py`: passed and reported the v0.7.1 working tree changes plus expected docs/scripts/build output.

## v0.7.1 Known Issues / Limitations

- A true human keyboard/mouse playthrough was not completed by Codex in this environment. A GDI key-event pass was attempted, but OS-level injected movement was not reliable enough to count as a manual completion.
- The complete Ferry Office loop is covered by a scene/player/traversal/world-state system test, and bounded GDI/DX11 launches pass.
- The service gate is now latch-open rather than a general open/close door. This is intentional to avoid trapping the player without adding a door system.
- `TestWorld` and `TestScene` names remain unchanged; rename cleanup is deferred.
- DX11 hardware device creation still falls back to WARP in this environment.
- There is still no UI framework, final art, save/load, mission scripting, physics library, NPC AI, vehicles, combat, or inventory.

## v0.7.1 Next Honest Step

Use the polished micro-slice foundation for a narrow cleanup goal: v0.8 Prototype Scene Naming + Data Cleanup. Keep it focused on naming/data organization for the existing prototype scene, not a new gameplay system.

## v0.8 Baseline - 2026-05-14

Goal: rename `TestWorld` / `TestScene` to `PrototypeWorld` / `PrototypeScene` if safe, reduce Ferry Office string-name coupling where practical, organize Ferry Office constants/data, preserve existing micro-slice behavior, and keep validation passing. This goal must not add new gameplay systems.

Required context read before coding:

- `AGENTS.md`
- `docs/STATUS.md`
- `docs/ARCHITECTURE.md`
- `docs/DECISIONS.md`
- `docs/ROADMAP.md`
- `docs/TECH_DEBT.md`

Baseline command:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
```

Baseline result:

- Passed.
- Doctor found all expected project docs and structure.
- Doctor warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- CMake configured with `windows-vs2022-debug`.
- Build produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- CTest passed 2/2 tests.
- Headless smoke run passed with the null renderer and engine `v0.7.1`.
- Smoke logs showed the existing Ferry Office start state, multi-line debug text, and `sliceComplete=no`.

## v0.8 Short Implementation Plan

1. Add guard tests first for the public prototype-scene naming boundary and Ferry Office behavior preservation.
2. Rename `TestWorld.*` to `PrototypeWorld.*` and `TestScene.*` to `PrototypeScene.*`, updating CMake, includes, type names, docs, and tests.
3. Add a small Ferry Office data header/source in `src/game` for stable ids/names/positions/prompts used by scene setup and state mapping.
4. Replace direct string-name coupling in scene state mapping and rendering with centralized Ferry Office ids/names where practical.
5. Keep all existing player, traversal, interaction, collision, renderer, and world-state behavior unchanged.
6. Run build/CTest after the refactor, then the full validation matrix, record exact results, commit, and push.

## v0.8 Changes Made

- Updated project version to `0.8.0`.
- Renamed active prototype boundaries:
  - `src/game/TestWorld.*` -> `src/game/PrototypeWorld.*`
  - `src/game/TestScene.*` -> `src/game/PrototypeScene.*`
- Updated `PlayerController`, `SandboxLayer`, tests, includes, and CMake to use `PrototypeWorld` / `PrototypeScene`.
- Added `src/game/FerryOfficeData.*` to centralize Ferry Office debug names, prompts, messages, important positions, interaction radii, and Service Barrier Vault tuning constants.
- Replaced direct Ferry Office string/position coupling in scene setup, service-gate collider lookup, state mapping, debug rendering, and focused regression tests where practical.
- Preserved the existing Ferry Office micro-slice behavior; no new gameplay system was added.
- Updated architecture, roadmap, decisions, and technical debt docs for the new naming/data boundary.

## v0.8 Intermediate Results

Commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
```

Results:

- Initial TDD build failed as expected after adding the Ferry Office data guard test before implementation:
  - `fatal error C1083: Cannot open include file: 'game/FerryOfficeData.h': No such file or directory`
- After adding `FerryOfficeData`, renaming the classes/files, and updating CMake/includes, `scripts/build.ps1` passed.
- `ctest --preset windows-vs2022-debug --output-on-failure` passed, 2/2 tests.

## v0.8 Final Validation - 2026-05-14

Commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 240
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 240
python tools/status_report.py
```

Results:

- `scripts/doctor.ps1`: passed. Warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- `scripts/configure.ps1`: passed using `windows-vs2022-debug`.
- `scripts/build.ps1`: passed and produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 2/2 tests.
- `scripts/verify.ps1`: passed; ran doctor, configure, build, CTest, and null-renderer smoke run. Smoke logs show engine `v0.8.0`, five interactables, the initial Ferry Manifest prompt, all Ferry Office flags false, and `sliceComplete=no`.
- `EngineApp.exe --renderer gdi --frames 240`: passed; created a Win32 window, initialized the GDI fallback renderer, showed the existing Ferry Office debug text, and exited cleanly.
- `EngineApp.exe --renderer dx11 --frames 240`: passed; created a Win32 window and initialized DirectX 11 through WARP after hardware DX11 device creation failed.
- `python tools/status_report.py`: passed and reported the expected v0.8 working tree changes before commit.

## v0.8 Known Issues / Limitations

- This was a naming/data cleanup only; no new gameplay systems, UI framework, scene serialization, content pipeline, NPC AI, vehicles, combat, inventory, save/load, or physics library were added.
- Historical docs/status entries still mention `TestWorld` / `TestScene` where they describe earlier milestones. Active code and current architecture docs now use `PrototypeWorld` / `PrototypeScene`.
- `FerryOfficeData` is still C++-authored prototype data, not a runtime scene format.
- DX11 hardware device creation still falls back to WARP in this environment.

## v0.8 Next Honest Step

Run a focused follow-up goal: v0.8.1 Ferry Office Manual Playtest Notes + Follow-up Fixes. Keep it manual-playtest driven and fix only clarity/completion issues that survive the cleanup.

## v0.8.1 Baseline - 2026-05-15

Goal: improve windowed input/camera comfort by making the mouse cursor less distracting for laptop/touchpad play while preserving all existing movement, interaction, traversal, smoke, renderer, and validation behavior. This goal must not add new gameplay systems.

Required context read before coding:

- `AGENTS.md`
- `docs/STATUS.md`
- `docs/RUNBOOK.md`
- `docs/ARCHITECTURE.md`
- `docs/TECH_DEBT.md`
- `docs/MANUAL_TEST_CHECKLIST.md`
- `docs/DECISIONS.md`

Baseline command:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
```

Baseline result:

- Passed.
- Doctor found all expected project docs and structure.
- Doctor warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- CMake configured with `windows-vs2022-debug`.
- Build produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- CTest passed 2/2 tests.
- Headless smoke run passed with the null renderer and engine `v0.8.0`.
- Smoke logs showed the existing Ferry Office start state, initial Ferry Manifest focus prompt, all Ferry Office flags false, and `sliceComplete=no`.

## v0.8.1 Short Implementation Plan

1. Inspect the current Win32 input/window path, config parsing, and application startup to find the smallest safe cursor behavior hook.
2. Add focused tests first for any new CLI/config cursor flags and default behavior.
3. Add a small cursor mode to `AppConfig` and pass it through application/window creation without affecting smoke/headless mode.
4. Implement safe Win32 cursor capture/hide behavior for windowed play, with a clear non-captured mode and preserved `Esc` quit behavior.
5. Preserve arrow-key camera fallback and existing `Space`/`E` pressed-edge behavior.
6. Update runbook, architecture, manual checklist, decisions, technical debt, and this status file.
7. Run the full validation matrix, record exact results, then commit and push.

## v0.8.1 Changes Made

- Updated project version to `0.8.1`.
- Added `AppConfig::captureCursor`, defaulting to captured cursor mode for windowed play.
- Added CLI flags:
  - `--capture-cursor`
  - `--free-cursor`
  - `--show-cursor`
- Added `IWindow::setCursorCaptured(bool)` and wired application startup to apply the configured cursor mode only after a real window is created.
- Implemented Win32 cursor capture/hide in the window boundary:
  - hides the cursor while captured and focused,
  - confines it to the client area,
  - recenters it for relative mouse/touchpad deltas,
  - releases/restores it on focus loss, close, and shutdown.
- Preserved `Esc` as quit; quitting restores the cursor during shutdown.
- Preserved arrow-key camera fallback, `Space`/`E` pressed-edge behavior, and smoke/headless behavior.
- Updated runbook, architecture, manual checklist, roadmap, decisions, and technical debt docs.

## v0.8.1 Intermediate Results

Commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
```

Results:

- Initial TDD build failed as expected after adding cursor config tests:
  - `error C2039: "captureCursor": is not a member of "engine::AppConfig"`
- After adding config flags and Win32 cursor capture behavior, `scripts/build.ps1` passed.
- `ctest --preset windows-vs2022-debug --output-on-failure` passed, 2/2 tests.

## v0.8.1 Final Validation - 2026-05-15

Commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 240
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 240
python tools/status_report.py
```

Results:

- `scripts/doctor.ps1`: passed. Warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- `scripts/configure.ps1`: passed using `windows-vs2022-debug`.
- `scripts/build.ps1`: passed and produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 2/2 tests.
- `scripts/verify.ps1`: passed; ran doctor, configure, build, CTest, and null-renderer smoke run. Smoke logs show engine `v0.8.1`, five interactables, the initial Ferry Manifest prompt, all Ferry Office flags false, and `sliceComplete=no`.
- `EngineApp.exe --renderer gdi --frames 240`: passed; created a Win32 window, logged `Cursor captured for windowed play.`, initialized the GDI fallback renderer, showed the existing Ferry Office debug text, logged `Cursor released.`, and exited cleanly.
- `EngineApp.exe --renderer dx11 --frames 240`: passed; created a Win32 window and initialized DirectX 11 through WARP after hardware DX11 device creation failed. The bounded run exited cleanly.
- `python tools/status_report.py`: passed and reported the expected v0.8.1 working tree changes before commit.

## v0.8.1 Known Issues / Limitations

- Cursor capture is implemented with Win32 client-area confinement and recentering, not raw input or a settings UI.
- `Esc` still quits the app rather than toggling cursor capture. This is intentionally simple and safe for now; `--free-cursor` / `--show-cursor` are available for visible-cursor debugging.
- A true human laptop/touchpad playtest still needs to confirm comfort on the target device.
- DX11 hardware device creation still falls back to WARP in this environment.
- No new gameplay systems, vehicles, NPC AI, combat, inventory, save/load, online/multiplayer, asset pipeline, physics library, or visual overhaul were added.

## v0.8.1 Next Honest Step

Run a focused follow-up goal: v0.9 Atmospheric Ferry Office / Island Mood Prototype. Keep it focused on making the existing micro-slice read like a place with mood and solid placeholder presentation, not on adding new gameplay systems.

## v0.9 Baseline - 2026-05-15

Goal: make the Ferry Office prototype visually read as a small dock-side/island ferry office area instead of a pure debug wireframe scene, while preserving movement, camera, collision, interaction, traversal, world-state, cursor capture, validation, and debug usability. This goal must not add new gameplay systems or an asset pipeline.

Required context read before coding:

- `AGENTS.md`
- `docs/STATUS.md`
- `docs/RUNBOOK.md`
- `docs/ARCHITECTURE.md`
- `docs/GAME_DIRECTION.md`
- `docs/VERTICAL_SLICE.md`
- `docs/TECH_DEBT.md`
- `docs/MANUAL_TEST_CHECKLIST.md`
- `docs/DECISIONS.md`

Baseline command:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
```

Baseline result:

- Passed.
- Doctor found all expected project docs and structure.
- Doctor warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- CMake configured with `windows-vs2022-debug`.
- Build produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- CTest passed 2/2 tests.
- Headless smoke run passed with the null renderer and engine `v0.8.1`.
- Smoke logs showed the existing Ferry Office start state, initial Ferry Manifest focus prompt, all Ferry Office flags false, and `sliceComplete=no`.

## v0.9 Short Implementation Plan

1. Inspect renderer/debug draw boundaries and scene rendering to pick the smallest safe solid placeholder path.
2. Add focused tests first for any renderer debug command accounting or config changes.
3. Extend debug rendering narrowly with solid placeholder geometry where practical while preserving wireframe/debug outlines.
4. Update `SandboxLayer` scene drawing with a restrained dock/ferry-office palette, solid floor/dock/service-yard forms, readable gate/traversal/interactable/player markers, and an intentional background/clear color.
5. Keep GDI and DX11 bounded runs working; document any renderer-specific visual limitations honestly.
6. Update architecture, roadmap, technical debt, manual checklist, decisions, and this status file.
7. Run the full validation matrix, capture or document visual evidence/observations, then commit and push.

## v0.9 Implementation Notes - 2026-05-15

Changed:

- Added `IRenderer::drawDebugSolidBox` as a narrow solid debug primitive.
- Implemented solid debug boxes in `NullRenderer`, `GdiRenderer`, and `Dx11Renderer`.
- Updated `SandboxLayer` to draw muted solid Ferry Office placeholders before existing wire/debug outlines:
  - dock/floor slabs,
  - service-yard area,
  - ferry office walls/roof cue,
  - service gate,
  - service barrier,
  - interactables,
  - traversal markers,
  - player proxy.
- Changed the default window title to `Tidebreak Prototype`.
- Updated the null renderer debug draw accounting test.
- Captured a GDI visual reference at `docs/images/v0.9-gdi-screenshot.png`.

Test-first note:

```text
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
```

Result before implementation: failed as expected after the test called `drawDebugSolidBox` before the renderer API existed.

Key error:

```text
tests\EngineCoreTests.cpp(137,14): error C2039: "drawDebugSolidBox": is not a member of "engine::NullRenderer"
```

Focused checks after implementation:

```text
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
```

Result: build passed; `EngineCoreTests` and `EngineSmokeTest` passed.

Visual observation from the GDI screenshot:

- The scene now has solid dock/service-yard/office/gate massing instead of reading as wireframe only.
- Existing wire outlines, prompts, objective/world-state text, player proxy, traversal markers, interactables, and exit marker remain visible.
- GDI debug text is still large and functional rather than polished UI.
- This is still placeholder/debug presentation, not final art.

## v0.9 Validation Results - 2026-05-15

Commands run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 240
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 240
python tools\status_report.py
```

Results:

- `scripts/doctor.ps1`: completed. Existing warnings remain: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg` are not in the plain PowerShell PATH.
- `scripts/configure.ps1`: passed with preset `windows-vs2022-debug`.
- `scripts/build.ps1`: passed; built `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 2/2 tests green (`EngineCoreTests`, `EngineSmokeTest`).
- `scripts/verify.ps1`: passed; repeated doctor/configure/build/test and null smoke. Smoke log reported engine `v0.9.0` and app `Tidebreak Prototype`.
- GDI bounded run: passed; window created, cursor captured, `gdi-fallback` renderer ran for 240 frames, cursor released, clean shutdown.
- DX11 bounded run: passed; hardware DX11 device failed and fell back to WARP, `dx11` renderer ran for 240 frames, cursor released, clean shutdown.
- `python tools\status_report.py`: completed and showed the expected v0.9 modified files plus the new `docs/images/` screenshot artifact before commit.

Visual evidence:

- Captured GDI screenshot: `docs/images/v0.9-gdi-screenshot.png`.

Known limitations after v0.9:

- Solid boxes are projected debug geometry with no depth buffer, sorting, lighting, transparency, textures, or real materials.
- GDI debug text is readable but visually heavy.
- DX11 still has no debug text overlay and used WARP in this environment.
- The scene reads more like a dock/service-yard/office prototype, but it is still far from commercial art quality.

Recommended next goal:

Run v0.9.1 Ferry Office Visual Playtest + Readability Polish before starting vehicles, so the current location can be judged and tightened by actual play.

## v0.9.1 Baseline - 2026-05-15

Goal: manually/visually review the v0.9 Ferry Office presentation and make only small clarity/readability polish changes. This goal must not add vehicles, NPC AI, combat, inventory, save/load, mission scripting, online, asset pipeline, model loading, textures/materials, lighting/shadows/post-processing, UI framework, or new traversal types.

Required context read before coding:

- `AGENTS.md`
- `docs/STATUS.md`
- `docs/ROADMAP.md`
- `docs/ARCHITECTURE.md`
- `docs/GAME_DIRECTION.md`
- `docs/VERTICAL_SLICE.md`
- `docs/TECH_DEBT.md`
- `docs/MANUAL_TEST_CHECKLIST.md`
- `docs/DECISIONS.md`
- `docs/images/v0.9-gdi-screenshot.png`

Baseline command:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
```

Baseline result:

- Passed.
- Doctor found all expected project docs and structure.
- Doctor warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- CMake configured with `windows-vs2022-debug`.
- Build produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- CTest passed 2/2 tests.
- Headless smoke run passed with the null renderer and engine `v0.9.0`.
- Smoke logs showed the existing Ferry Office start state, initial Ferry Manifest focus prompt, all Ferry Office flags false, and `sliceComplete=no`.

Initial v0.9 screenshot observations:

- Solid geometry makes the scene read much more like a constructed place than the earlier pure wireframe.
- The red service gate strongly dominates the first view and can read like the main object before the player understands the intended loop.
- The interactable/focus volumes still look like abstract debug boxes, so manifest, wall button, maintenance box, and exit need clearer color/signpost hierarchy.
- Debug text is useful but visually heavy and consumes the top of the screen.
- The start view does not explicitly sell "dock / ferry office / maintenance side"; it relies on color and layout rather than labels/signposting.

## v0.9.1 Short Implementation Plan

1. Run a GDI visual/playtest pass and record what can and cannot be honestly verified from this environment.
2. Inspect `SandboxLayer`, `FerryOfficeData`, and `PrototypeScene` for small polish points that improve route/objective clarity without new systems.
3. Apply the smallest readable-scene changes:
   - tune marker colors and sizes,
   - improve objective wording if it can guide the player better,
   - add simple original debug signpost forms using existing solid boxes/lines,
   - reduce visual competition where the gate overwhelms early focus,
   - preserve all existing gameplay behavior.
4. Add or update lightweight tests only for changed behavior such as objective text or renderer accounting.
5. Capture/update the GDI screenshot reference and record visual observations.
6. Run the full validation matrix, then commit and push if green.

## v0.9.1 Playtest / Visual Review Notes - 2026-05-15

GDI input playtest attempt:

```powershell
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 720 --free-cursor
```

Result:

- The app launched and accepted some OS-level injected keyboard input.
- The run successfully collected the Ferry Manifest and recorded `manifestCollected=true`.
- The scripted input did not reliably steer to the Service Barrier Vault; it drifted toward the Wall Button and opened the service gate before traversal.
- Because the input injection is not a reliable human/manual controller in this environment, this is not counted as a complete manual playthrough.

Observed clarity issues from the run and the v0.9 screenshot:

- After the manifest, the intended route to the Service Barrier Vault was too easy to miss.
- The old straight debug line from manifest to exit did not describe the actual intended loop.
- Marker hierarchy was still too flat; several interactables looked equally important.
- The debug text had all the right data but put player/camera stats before the objective and prompt.

Changes made:

- Updated project version to `0.9.1`.
- Added `TestFerryOfficeObjectiveTextGuidesRouteSteps` to lock objective wording around the intended loop.
- Reworded Ferry Office objectives so they name the next spatial target:
  - dock-side Ferry Manifest,
  - right-side Service Barrier / Maintenance Box,
  - service-side Maintenance Box power restore,
  - office Wall Button / service gate,
  - open service gate / Exit Summary Marker.
- Reordered sandbox debug text so objective and focus/prompt appear first.
- Replaced the single manifest-to-exit debug line with a low route polyline through:
  - Ferry Manifest,
  - Service Barrier Vault start/end,
  - Maintenance Box,
  - Wall Button,
  - Exit Summary Marker.
- Tuned known Ferry Office marker sizes, colors, and beacon heights for clearer visual hierarchy.
- Slightly reduced the closed gate's solid red dominance while keeping its wire/debug state visible.
- Added small dock/maintenance signpost forms using existing solid debug boxes.
- Captured updated visual evidence at `docs/images/v0.9.1-gdi-screenshot.png`.

Focused test results:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
```

Result:

- Initial CTest after adding the objective wording test failed as expected with 3 objective-guidance failures.
- After implementation, build passed and CTest passed 2/2.

Visual observation from the v0.9.1 GDI screenshot:

- Objective and focus are now the first debug lines, which makes the current task easier to read.
- The route polyline better explains the Ferry Office loop than the previous straight line.
- Manifest, maintenance, wall button, and exit markers now have more distinct size/color/beacon treatment.
- The scene is still debug-placeholder presentation and still far from final commercial art quality.

## v0.9.1 Final Validation - 2026-05-15

Commands run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 240
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 240
python tools\status_report.py
```

Results:

- `scripts/doctor.ps1`: completed. Existing warnings remain: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg` are not in the plain PowerShell PATH.
- `scripts/configure.ps1`: passed with preset `windows-vs2022-debug`.
- `scripts/build.ps1`: passed; built `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 2/2 tests green (`EngineCoreTests`, `EngineSmokeTest`).
- `scripts/verify.ps1`: passed; repeated doctor/configure/build/test and null smoke. Smoke log reported engine `v0.9.1` and app `Tidebreak Prototype`.
- GDI bounded run: passed; window created, `gdi-fallback` renderer ran for 240 frames, clean shutdown.
- DX11 bounded run: passed; hardware DX11 device failed and fell back to WARP, `dx11` renderer ran for 240 frames, clean shutdown.
- `python tools\status_report.py`: completed and showed the expected v0.9.1 modified files plus `docs/images/v0.9.1-gdi-screenshot.png` before commit.

Visual evidence:

- Captured GDI screenshot: `docs/images/v0.9.1-gdi-screenshot.png`.

Known limitations after v0.9.1:

- Full human keyboard/mouse completion still needs to be done by a person on the target laptop; OS-level input injection was not reliable enough to count.
- GDI debug text is more useful but still a debug overlay, not a UI system.
- DX11 still has no debug text overlay and used WARP in this environment.
- The scene is more readable than v0.9, but it remains placeholder/debug presentation rather than commercial-quality art.

Recommended next goal:

Run v0.10 Vehicle Feel Spike only if the human playtest agrees that the Ferry Office readability is good enough to move on.

## v0.9.2 Baseline - 2026-05-15

Goal: make an early, evidence-based physics foundation decision and add a narrow vendor-safe `src/engine/physics` boundary. Jolt is the default candidate for Tidebreak, with PhysX kept as backup and Bullet not preferred unless a future test changes the evidence. This goal must not add vehicles, NPC AI, combat, inventory, save/load, mission scripting, online/multiplayer, an asset pipeline, final art, a full physics-driven player rewrite, or large renderer changes.

Required context read before coding:

- `AGENTS.md`
- `docs/STATUS.md`
- `docs/ROADMAP.md`
- `docs/ARCHITECTURE.md`
- `docs/GAME_DIRECTION.md`
- `docs/VERTICAL_SLICE.md`
- `docs/TECH_DEBT.md`
- `docs/MANUAL_TEST_CHECKLIST.md`
- `docs/DECISIONS.md`
- `docs/RUNBOOK.md`
- `docs/AI_WORKFLOW.md`

Baseline command:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
```

Baseline result:

- Passed.
- Doctor found all expected project docs and structure.
- Doctor warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- CMake configured with `windows-vs2022-debug`.
- Build produced `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- CTest passed 2/2 tests.
- Headless smoke run passed with the null renderer and engine `v0.9.1`.
- Smoke logs showed the existing Ferry Office start state, initial Ferry Manifest focus prompt, all Ferry Office flags false, and `sliceComplete=no`.

## v0.9.2 Short Implementation Plan

1. Inspect the current CMake/source/test structure and dependency assumptions, then keep any physics dependency optional enough that baseline builds stay reliable.
2. Add tests first for vendor-free physics interface behavior: world creation/shutdown, static floor/box concepts, raycast hit/miss behavior, and debug line extraction.
3. Add `src/engine/physics` with engine-owned types and an `IPhysicsWorld` boundary so `src/game` does not include vendor physics headers.
4. Add a no-third-party fallback physics world that supports the spike queries deterministically and keeps validation green without vcpkg.
5. Attempt Jolt integration only if it can be done cleanly and repeatably in this environment; otherwise document the exact blocker and leave the engine boundary ready.
6. Create `docs/PHYSICS_DECISION.md` comparing Jolt, PhysX, Bullet, and temporary custom collision against Tidebreak needs.
7. Update architecture, runbook, roadmap, decisions, technical debt, and this status file.
8. Run the full validation matrix, then commit and push if validation passes.

## v0.9.2 Implementation Notes - 2026-05-15

Changed:

- Updated project version to `0.9.2`.
- Added `src/engine/physics/PhysicsWorld.h`.
- Added `src/engine/physics/PhysicsWorld.cpp`.
- Added `src/engine/physics/JoltPhysicsWorld.cpp`.
- Added `ENGINE_ENABLE_JOLT_PHYSICS`, defaulting to `OFF`.
- Added `windows-vs2022-debug-jolt` CMake configure/build/test presets.
- Added `docs/PHYSICS_DECISION.md`.
- Kept normal `scripts/configure.ps1` / `scripts/verify.ps1` on the dependency-free `windows-vs2022-debug` path.

Engine physics boundary:

- `IPhysicsWorld` exposes vendor-free engine types:
  - `PhysicsConfig`,
  - `BodyHandle`,
  - `BoxColliderDesc`,
  - `DynamicBoxDesc`,
  - `RaycastResult`,
  - `PhysicsDebugLine`.
- Default `simple` backend supports init/shutdown, static boxes, trigger boxes, a dynamic-box placeholder, fixed-step update, raycast hit/miss, and debug box lines.
- Jolt backend is private to `EngineCore`; `src/game` does not include Jolt headers or use `JPH::*` types.

Test-first note:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
```

Result before implementation: failed as expected after tests included `engine/physics/PhysicsWorld.h` before the module existed.

Key error:

```text
tests\EngineCoreTests.cpp(5,10): error C1083: Cannot open include file: 'engine/physics/PhysicsWorld.h': No such file or directory
```

Focused default checks after fallback implementation:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
```

Result:

- Build passed.
- CTest passed 2/2 tests.
- New physics tests pass on the dependency-free `simple` backend.

Jolt spike checks:

```powershell
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
```

Result:

- Initial `cmake --preset windows-vs2022-debug-jolt`: passed and fetched/configured Jolt `v5.5.0`.
- First Jolt build: `Jolt.lib` built, then our adapter failed on Jolt raycast API details. Fixed by including `CastResult.h` and keeping hit normals deferred.
- Second Jolt build: `Jolt.lib` built and our adapter compiled, then final link failed because Jolt defaulted to static MSVC runtime `/MTd` while this project uses dynamic `/MDd`.
- Fixed by forcing `USE_STATIC_MSVC_RUNTIME_LIBRARY=OFF` for the opt-in Jolt preset.
- Final Jolt configure/build: passed.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 2/2 tests.

Known limitations after implementation:

- Jolt is integrated as an opt-in backend, not the default runtime physics backend.
- Existing Ferry Office player/collision/traversal behavior still uses `PrototypeWorld` and was intentionally not rewritten.
- Jolt debug draw is represented only by simple engine-owned box debug lines for now.
- Jolt raycast hit normals are deferred in the adapter; hit state, distance, point, body handle, and body name are validated.
- The Jolt dependency path uses pinned FetchContent, not vcpkg manifest mode.

Vendor firewall check:

```powershell
rg "Jolt|JPH::|JPH/" src/game
```

Result:

- No matches; game code does not directly reference Jolt.

## v0.9.2 Final Validation - 2026-05-15

Commands run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 240
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 240
python tools/status_report.py
```

Results:

- `scripts/doctor.ps1`: completed. It now checks `docs/PHYSICS_DECISION.md`. Existing warnings remain: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg` are not in the plain PowerShell PATH.
- `scripts/configure.ps1`: passed with preset `windows-vs2022-debug`.
- `scripts/build.ps1`: passed; built `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 2/2 tests green (`EngineCoreTests`, `EngineSmokeTest`).
- `scripts/verify.ps1`: passed; repeated doctor/configure/build/test and null smoke. Smoke log reported engine `v0.9.2` and app `Tidebreak Prototype`.
- GDI bounded run: passed; window created, `gdi-fallback` renderer ran for 240 frames, clean shutdown.
- DX11 bounded run: passed; hardware DX11 device failed and fell back to WARP, `dx11` renderer ran for 240 frames, clean shutdown.
- `python tools/status_report.py`: completed and showed the expected v0.9.2 modified/new files before commit.

Additional opt-in Jolt validation:

```powershell
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
```

Results:

- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed; built `Jolt.lib`, `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 2/2 tests green.

Known limitations after v0.9.2:

- Jolt is opt-in and not yet used by Ferry Office runtime gameplay.
- The prototype collision path is still custom static AABB in `PrototypeWorld`.
- No vehicles, NPC AI, combat, inventory, save/load, mission scripting, online/multiplayer, asset pipeline, final art, full physics-driven player rewrite, or large renderer changes were added.
- vcpkg remains absent from PATH. This is not a blocker because the Jolt spike uses an explicit pinned FetchContent preset.

Recommended next goal:

Run v0.10 Vehicle Feel Spike on the physics foundation.
