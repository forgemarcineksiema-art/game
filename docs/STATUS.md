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

## v0.10 Baseline - 2026-05-15

Goal: add the first narrow placeholder vehicle feel spike on top of the v0.9.2 physics foundation. The vehicle should prove enter/exit, basic acceleration/braking/reverse, steering, vehicle camera follow, debug visibility, and a tiny service-yard driving area while preserving the Ferry Office micro-slice. This goal must not add NPC AI, traffic, combat, police/chase systems, missions, cargo economy, inventory, save/load, online/multiplayer, an asset pipeline, final car art, complex damage, a full vehicle framework, physics engine rewrite, or renderer/material overhaul.

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
- `docs/PHYSICS_DECISION.md`

Baseline commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
```

Baseline results:

- `scripts/verify.ps1`: passed.
- Doctor found all required docs and structure. Existing warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- Default configure/build/test path passed with preset `windows-vs2022-debug`.
- Default CTest passed 2/2 tests.
- Null smoke run passed and reported engine `v0.9.2`, app `Tidebreak Prototype`, initial Ferry Manifest focus, and all Ferry Office remembered-state flags false.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed; built `Jolt.lib`, `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 2/2 tests green.

Implementation notes from code review:

- Keep the vehicle as a small `src/game` controller for this spike, but do not expose Jolt or `JPH::*` to game code.
- Use `SandboxLayer` as the low-risk mode switch for on-foot versus vehicle update/camera/debug flow.
- Do not turn the vehicle into a Ferry Office `Interactable`; that would compete with existing micro-slice focus and world-state mappings.
- Keep the current Ferry Office systems unchanged unless a small integration guard is required.

## v0.10 Short Implementation Plan

1. Add vehicle tests first for deterministic acceleration, braking, reverse, steering, pressed-edge enter/exit behavior, camera target selection, and Jolt vendor-firewall expectations.
2. Add `src/game/VehicleController.h/.cpp` with simple arcade-style vehicle state, tuning, safe enter/exit, and optional yard bounds.
3. Wire the vehicle into `SandboxLayer` as a separate driving mode: on foot keeps the existing player/traversal/interaction flow; in vehicle skips on-foot movement and follows the vehicle camera target.
4. Add service-yard debug geometry, vehicle proxy rendering, forward direction, enter/exit prompt, and vehicle telemetry text.
5. Use the engine-owned physics boundary non-invasively for a small vehicle/yard validation path while keeping Jolt private to `src/engine`.
6. Update architecture, runbook, roadmap, decisions, technical debt, manual checklist, and this status file.
7. Run default and Jolt validation, bounded GDI/DX11 runs, status report, then commit and push if validation passes.

## v0.10 Implementation Notes - 2026-05-15

Changed:

- Updated project version to `0.10.0`.
- Added `src/game/VehicleController.h`.
- Added `src/game/VehicleController.cpp`.
- Added deterministic vehicle tests for acceleration, braking, reverse, steering, enter/exit pressed-edge behavior, safe exit blocking, and vehicle camera target data.
- Added an automated source firewall test that scans `src/game` and fails if game code references `Jolt`, `JPH::`, `<Jolt/`, or `JPH/`.
- Integrated a service-yard vehicle in `SandboxLayer`.
- Added a vehicle camera mode using the existing `ThirdPersonCamera` with separate distance/height/smoothing settings.
- Added service-yard debug geometry, vehicle body/cabin proxy, heading line, enter radius, exit marker, yard bounds, telemetry debug text, and simple physics debug lines.
- Used `engine::physics::IPhysicsWorld` through the dependency-free `simple` backend for a small service-yard validation/debug world. Jolt remains private to `src/engine`.

Test-first note:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
```

Result before implementation: failed as expected after tests included `game/VehicleController.h` before the module existed.

Key error:

```text
tests\EngineCoreTests.cpp(15,10): error C1083: Nie można otworzyć pliku dołącz: 'game/VehicleController.h': No such file or directory
```

Focused checks after implementation:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
```

Result:

- Build passed.
- CTest passed 2/2 tests.

Known limitations after implementation:

- Vehicle movement is a deterministic arcade-style placeholder, not Jolt VehicleConstraint.
- Vehicle collision is currently yard-bound clamping plus a safe exit overlap check, not rigid-body collision against the full world.
- The service-yard road loop is debug geometry only.
- There are no wheels, suspension, tire model, vehicle animation, doors, seats, cargo, traffic, damage, persistence, or final vehicle art.
- Full manual driving feel still needs a human playtest on the target laptop.

## v0.10 Final Validation - 2026-05-15

Commands run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 300
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 300
python tools\status_report.py
rg "Jolt|JPH::|JPH/" src\game
```

Results:

- `scripts/doctor.ps1`: completed. Existing warnings remain: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg` are not in the plain PowerShell PATH.
- `scripts/configure.ps1`: passed with preset `windows-vs2022-debug`.
- `scripts/build.ps1`: passed; built `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 2/2 tests green (`EngineCoreTests`, `EngineSmokeTest`).
- `scripts/verify.ps1`: passed; repeated doctor/configure/build/test and null smoke. Smoke log reported engine `v0.10.0`, app `Tidebreak Prototype`, initial Ferry Manifest focus, vehicle state `empty`, `cameraMode=on-foot`, and vehicle physics backend `simple`.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed; built `Jolt.lib`, `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 2/2 tests green.
- GDI bounded run: passed; window created, `gdi-fallback` renderer ran for 300 frames, clean shutdown. Debug text reported the new vehicle telemetry and `physics=simple`.
- DX11 bounded run: passed; hardware DX11 device failed and fell back to WARP, `dx11` renderer ran for 300 frames, clean shutdown.
- `python tools\status_report.py`: completed and showed the expected v0.10 modified/new files before commit.
- `rg "Jolt|JPH::|JPH/" src\game`: no matches; game code does not directly reference Jolt.

Known limitations after v0.10:

- The vehicle is a feel spike, not a finished driving system.
- Vehicle movement is deterministic placeholder logic. It does not use Jolt VehicleConstraint, wheels, suspension, tire friction, or a real drivetrain.
- Vehicle world handling is limited to service-yard bounds and safe exit checks.
- The service-yard is debug placeholder geometry and still needs human driving playtest on the target laptop.
- DX11 still falls back to WARP in this environment.
- No NPC AI, traffic, combat, missions, cargo economy, inventory, save/load, online/multiplayer, asset pipeline, final car art, complex damage, or full vehicle framework was added.

Recommended next goal:

Run v0.10.1 Vehicle Feel Tuning + Service Yard Road Test Polish.

## v0.11 Baseline - 2026-05-15

Goal: create a scene/object authoring foundation and Codex-friendly world tools before adding more gameplay. This goal must preserve the v0.10 Ferry Office and service-yard vehicle behavior while adding explicit scene data, validation/report tooling, scale conventions, scene authoring docs, and art direction notes. It must not add NPC AI, traffic, combat, missions, inventory, save/load, online/multiplayer, a full asset pipeline, model loading, animation runtime, new traversal types, Jolt vehicle constraints, final art, or a renderer overhaul.

Required context read before coding:

- `AGENTS.md`
- `docs/STATUS.md`
- `docs/ARCHITECTURE.md`
- `docs/ROADMAP.md`
- `docs/GAME_DIRECTION.md`
- `docs/VERTICAL_SLICE.md`
- `docs/TECH_DEBT.md`
- `docs/MANUAL_TEST_CHECKLIST.md`
- `docs/DECISIONS.md`
- `docs/PHYSICS_DECISION.md`

Baseline commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
```

Baseline results:

- `scripts/verify.ps1`: passed.
- Doctor found all expected v0.10 docs and structure. Existing warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- Default configure/build/test path passed with preset `windows-vs2022-debug`.
- Default CTest passed 2/2 tests.
- Null smoke run passed and reported engine `v0.10.0`, app `Tidebreak Prototype`, initial Ferry Manifest focus, service-yard vehicle state `empty`, `cameraMode=on-foot`, and `physics=simple`.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed; built `Jolt.lib`, `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 2/2 tests green.

## v0.11 Short Implementation Plan

1. Add a human-readable `data/scenes/ferry_office.scene.json` that mirrors the current Ferry Office and service-yard prototype: units, floor height, colliders, visual placeholders, interactables, traversal affordance, player start, vehicle spawn/bounds, and route/objective markers.
2. Add Python scene tooling with no new dependencies:
   - `tools/scene_data.py` shared loader/validation helpers,
   - `tools/scene_report.py` for concise Codex-readable scene summaries,
   - `tools/validate_scene.py` for required-id, numeric, uniqueness, radius, extent, and scale checks,
   - `tools/scale_audit.py` for suspicious-size reporting.
3. Add lightweight Python tests for the scene tools and wire them into CTest and `scripts/verify.ps1` if the dependency-free path stays reliable.
4. Update `scripts/doctor.ps1` and `tools/status_report.py` so future runs see the scene docs/data/tools as first-class project artifacts.
5. Create `docs/ASSET_GUIDE.md`, `docs/SCENE_AUTHORING.md`, and `docs/ART_DIRECTION.md`.
6. Update architecture, decisions, roadmap, runbook, manual checklist, technical debt, and this status file.
7. Run the full validation matrix, then commit and push if validation passes.

## v0.11 Implementation Notes - 2026-05-15

Changed:

- Updated project version to `0.11.0`.
- Added `data/scenes/ferry_office.scene.json`.
- Added `tools/scene_data.py`.
- Added `tools/scene_report.py`.
- Added `tools/validate_scene.py`.
- Added `tools/scale_audit.py`.
- Added `tests/test_scene_tools.py`.
- Added CTest coverage for the Python scene tool tests.
- Updated `scripts/verify.ps1` so the default validation path runs `python tools\validate_scene.py` after CTest.
- Updated `scripts/doctor.ps1` and `tools/status_report.py` to treat the new scene data/docs/tools as first-class artifacts.
- Added `docs/ASSET_GUIDE.md`.
- Added `docs/SCENE_AUTHORING.md`.
- Added `docs/ART_DIRECTION.md`.
- Updated `AGENTS.md`, architecture, runbook, roadmap, decisions, technical debt, and manual test checklist.

Scene data contents:

- Scene id/name and meter/Y-up/+Z-forward unit convention.
- Floor height and player start.
- Scale references for player, vehicle, doors, and lane widths.
- 9 static Ferry Office colliders.
- 14 visual placeholder boxes for dock, office, maintenance side, service yard, rails, water bands, and props.
- 5 interactables: Ferry Manifest, Wall Button, Ferry Office Notice, Maintenance Box, Exit Summary Marker.
- 1 traversal affordance: Service Barrier Vault.
- 1 vehicle spawn with service-yard bounds and controls.
- Route and objective markers for the Ferry Office loop.

Test-first note:

```powershell
python tests\test_scene_tools.py
```

Result before implementation: failed as expected because `tools/scene_data.py` did not exist.

Key error:

```text
ModuleNotFoundError: No module named 'scene_data'
```

Focused checks after implementation:

```powershell
python tests\test_scene_tools.py
python tools\validate_scene.py
python tools\scene_report.py
python tools\scale_audit.py
```

Results:

- `tests\test_scene_tools.py`: passed, 5/5 tests.
- `tools\validate_scene.py`: passed.
- `tools\scene_report.py`: completed; reported `ferry-office`, 9 colliders, 14 visual placeholders, 5 interactables, 1 traversal affordance, 1 vehicle, 4 routes, and 3 objective markers.
- `tools\scale_audit.py`: completed; no suspicious scale issues found.

Known limitations after implementation:

- Runtime gameplay still uses the existing C++ setup. Scene JSON is a validated authoring mirror, not runtime-loaded data.
- Layout facts are intentionally duplicated between JSON and C++ until a later loader or generator exists.
- Scene validation does not yet compare every JSON value against C++ constants.
- There is no editor, gizmo, asset registry, prefab system, model loading, animation runtime, or final art.

## v0.11 Final Validation - 2026-05-15

Commands run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
python tools\scene_report.py
python tools\validate_scene.py
python tools\scale_audit.py
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 300
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 300
python tools\status_report.py
```

Results:

- `scripts/doctor.ps1`: completed. It now checks the v0.11 scene docs, scene data, and scene tools. Existing warnings remain: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg` are not in the plain PowerShell PATH.
- `scripts/configure.ps1`: passed with preset `windows-vs2022-debug`.
- `scripts/build.ps1`: passed; built `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 3/3 tests green (`EngineCoreTests`, `EngineSmokeTest`, `SceneToolTests`).
- `scripts/verify.ps1`: passed; repeated doctor/configure/build/test, ran scene validation, and completed null smoke. Smoke log reported engine `v0.11.0`, app `Tidebreak Prototype`, the existing Ferry Office start state, vehicle `empty`, `cameraMode=on-foot`, and `physics=simple`.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed; built `Jolt.lib`, `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 3/3 tests green.
- `python tools\scene_report.py`: completed; reported `ferry-office`, units `meter`, 9 colliders, 14 visual placeholders, 5 interactables, 1 traversal affordance, 1 vehicle, 4 route markers, 3 objective markers, and 0 validation errors/warnings.
- `python tools\validate_scene.py`: passed.
- `python tools\scale_audit.py`: completed; no suspicious scale issues found.
- GDI bounded run: passed; window created, `gdi-fallback` renderer ran for 300 frames, clean shutdown.
- DX11 bounded run: passed; hardware DX11 device failed and fell back to WARP, `dx11` renderer ran for 300 frames, clean shutdown.
- `python tools\status_report.py`: completed and showed the expected v0.11 modified/new files before commit.

Known limitations after v0.11:

- Scene data is not runtime-loaded yet.
- JSON/C++ layout drift remains possible if future work does not update both sides.
- No full editor, ECS, asset pipeline, model loading, animation runtime, generated scene code, final art, or renderer overhaul was added.
- DX11 still falls back to WARP in this environment.

Recommended next goal:

Run v0.12 Static Mesh + glTF Render Spike.

## v0.12 Baseline - 2026-05-15

Goal: add the first narrow static mesh / glTF render spike on top of v0.11. This should prove an engine-owned static mesh data path, a minimal glTF loading subset, scene-data mesh references, Codex-friendly asset validation, and safe renderer fallback behavior while preserving the existing Ferry Office, service-yard vehicle, physics, scene tools, and validation. This goal must not add NPC AI, traffic, combat, missions, inventory, save/load, online/multiplayer, animation runtime, skeletal meshes, physics gameplay migration, Jolt vehicle constraints, textures/materials, PBR, lighting/shadows/post-processing, terrain, an editor, full ECS, final art, or marketplace/copied assets.

Required context read before coding:

- `AGENTS.md`
- `docs/STATUS.md`
- `docs/ARCHITECTURE.md`
- `docs/ROADMAP.md`
- `docs/GAME_DIRECTION.md`
- `docs/VERTICAL_SLICE.md`
- `docs/TECH_DEBT.md`
- `docs/MANUAL_TEST_CHECKLIST.md`
- `docs/DECISIONS.md`
- `docs/PHYSICS_DECISION.md`
- `docs/ASSET_GUIDE.md`
- `docs/SCENE_AUTHORING.md`
- `docs/ART_DIRECTION.md`

Baseline commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
python tools/scene_report.py
python tools/validate_scene.py
python tools/scale_audit.py
```

Baseline results:

- `scripts/verify.ps1`: passed.
- Doctor found all expected v0.11 docs, scene data, and tools. Existing warnings remain for tools not visible in the plain PATH: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- Default configure/build/test path passed with preset `windows-vs2022-debug`.
- Default CTest passed 3/3 tests: `EngineCoreTests`, `EngineSmokeTest`, and `SceneToolTests`.
- Scene validation ran inside `verify.ps1` and passed.
- Null smoke run passed and reported engine `v0.11.0`, app `Tidebreak Prototype`, initial Ferry Manifest focus, service-yard vehicle state `empty`, `cameraMode=on-foot`, and `physics=simple`.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed; built `Jolt.lib`, `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 3/3 tests green.
- `python tools/scene_report.py`: completed; reported `ferry-office`, 9 colliders, 14 visual placeholders, 5 interactables, 1 traversal affordance, 1 vehicle, 4 route markers, and 3 objective markers.
- `python tools/validate_scene.py`: passed.
- `python tools/scale_audit.py`: completed; no suspicious scale issues found.

## v0.12 Short Implementation Plan

1. Add tests first for the static mesh data boundary, minimal glTF loader behavior, scene mesh references, missing asset validation, and mesh bounds computation.
2. Add a small engine-owned mesh module with `StaticMeshAsset`, `StaticMeshVertex`, `StaticMeshInstance`, bounds, and a minimal glTF/static mesh loader that keeps any parser details private.
3. Choose and document the v0.12 loader strategy. Prefer a tiny permissive glTF parser only if integration stays clean; otherwise implement a narrow documented loader subset for the original test assets.
4. Add original small placeholder mesh assets under `assets/models`, aligned with the v0.11 meter/Y-up/+Z-forward conventions.
5. Extend `data/scenes/ferry_office.scene.json` with `meshAssets` and `meshInstances`, and extend `tools/scene_data.py`, `validate_scene.py`, `scene_report.py`, and `scale_audit.py` for mesh reference checks.
6. Add a narrow renderer submission path so DX11 can draw flat-tinted static mesh triangles if practical, while GDI/null remain safe through bounds/proxy handling.
7. Integrate a few mesh instances into the Ferry Office/service-yard presentation without removing debug collision, route, traversal, interaction, vehicle, or world-state visibility.
8. Update architecture, decisions, roadmap, runbook, manual checklist, asset/scene/art docs, technical debt, and this status file.
9. Run the full default/Jolt/tool/windowed validation matrix, then commit and push if validation passes.

## v0.12 Implementation Notes - 2026-05-15

Changed:

- Updated project version to `0.12.0`.
- Added `src/engine/assets/StaticMesh.h`.
- Added `src/engine/assets/StaticMesh.cpp`.
- Added `IRenderer::drawDebugFlatTriangles`.
- Implemented flat triangle submission in the null, GDI, and DX11 renderers.
- Added original placeholder asset `assets/models/unit_box.gltf`.
- Extended `data/scenes/ferry_office.scene.json` to schema version 2 with `meshAssets` and `meshInstances`.
- Extended `tools/scene_data.py` with mesh asset and mesh instance validation.
- Extended `tools/scene_report.py` and `tools/scale_audit.py` coverage through shared scene validation helpers.
- Added `tools/mesh_report.py`.
- Updated `scripts/doctor.ps1`, `scripts/verify.ps1`, and `tools/status_report.py` for mesh artifacts/tools.
- Updated `SandboxLayer` to load the unit-box mesh and render a few explicit mesh instances that mirror scene data: ferry-office roof cap, service gate, maintenance box, and vehicle body.
- Added `docs/MESH_RENDERING.md`.
- Updated architecture, runbook, roadmap, decisions, technical debt, manual checklist, AI workflow, asset guide, scene authoring, and art direction docs.

Loader decision:

- v0.12 does not add cgltf, tinygltf, or Assimp.
- A tiny project-owned `.gltf` subset loader was added because this spike only needs embedded-buffer original placeholder meshes.
- Supported subset: `.gltf`, embedded `data:application/octet-stream;base64` buffer, `POSITION` float `VEC3`, indexed triangle list, unsigned-short or unsigned-int indices.
- Deferred: `.glb`, external buffers, materials, textures, normals in renderer input, UVs, node hierarchy, skeletal meshes, animations, mesh collision, and a real asset registry.

Test-first notes:

```powershell
python tests\test_scene_tools.py
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
```

Results before implementation:

- Python scene tests failed as expected because `SceneSummary` did not yet expose `mesh_asset_count` / `mesh_instance_count`, and mesh asset/path/instance validation did not exist.
- C++ build failed as expected because `engine/assets/StaticMesh.h` did not exist.

Focused checks after implementation:

```powershell
python tests\test_scene_tools.py
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
python tools\scene_report.py
python tools\validate_scene.py
python tools\scale_audit.py
python tools\mesh_report.py
```

Results:

- `tests\test_scene_tools.py`: passed, 10/10 tests.
- `scripts/build.ps1`: passed after CMake regenerated the Visual Studio build.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 3/3 tests green.
- `tools\scene_report.py`: completed; reported 1 mesh asset and 4 mesh instances.
- `tools\validate_scene.py`: passed.
- `tools\scale_audit.py`: completed; no suspicious scale issues found.
- `tools\mesh_report.py`: completed; reported `unit-box-mesh`, 4 uses, 8 vertices, and 36 indices.

Known limitations after implementation:

- Runtime still mirrors mesh instances explicitly in C++; scene JSON is not loaded at runtime.
- The mesh loader is intentionally narrow and should not be mistaken for a production glTF importer.
- DX11/GDI mesh rendering uses the existing CPU debug projection path and has no depth buffer, materials, textures, lighting, or true 3D resource lifetime.
- `assets/models/unit_box.gltf` is original placeholder geometry, not final art.

## v0.12 Final Validation - 2026-05-15

Commands run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
python tools\scene_report.py
python tools\validate_scene.py
python tools\scale_audit.py
python tools\mesh_report.py
python tools\status_report.py
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 300
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 300
```

Results:

- `scripts/doctor.ps1`: completed. It now checks `docs/MESH_RENDERING.md`, `assets\models\unit_box.gltf`, and `tools\mesh_report.py`. Existing warnings remain: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg` are not in the plain PowerShell PATH.
- `scripts/configure.ps1`: passed with preset `windows-vs2022-debug`.
- `scripts/build.ps1`: passed; built `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 3/3 tests green.
- `scripts/verify.ps1`: passed; repeated doctor/configure/build/test, ran scene validation, ran mesh report, and completed null smoke. Smoke log reported engine `v0.12.0` and loaded `unit-box-mesh`.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed; built `Jolt.lib`, `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 3/3 tests green.
- `python tools\scene_report.py`: completed; reported 1 mesh asset and 4 mesh instances.
- `python tools\validate_scene.py`: passed.
- `python tools\scale_audit.py`: completed; no suspicious scale issues found.
- `python tools\mesh_report.py`: completed; reported `unit-box-mesh`, 4 uses, 8 vertices, and 36 indices.
- `python tools\status_report.py`: completed and showed the expected v0.12 modified/new files before commit.
- GDI bounded run: passed; window created, mesh asset loaded, `gdi-fallback` renderer ran for 300 frames, clean shutdown, cursor released.
- DX11 bounded run: passed; hardware DX11 device failed and fell back to WARP, mesh asset loaded, `dx11` renderer ran for 300 frames, clean shutdown, cursor released.

Visual evidence:

- No screenshot was captured in this run. The current native-window workbench does not yet have a reliable built-in screenshot helper for bounded GDI/DX11 captures. Runtime logs confirm both renderers ran with mesh loading enabled; visual screenshot capture remains a future tooling improvement.

Known limitations after v0.12:

- Mesh instances are rendered explicitly from C++ and mirrored in scene JSON; full runtime scene loading is still deferred.
- The glTF loader is intentionally tiny and supports only original embedded-buffer `.gltf` placeholder assets.
- DX11/GDI flat mesh rendering is still debug-projection rendering with no depth buffer, material system, texture sampling, lighting, shadows, or real mesh resource cache.
- GDI can show debug text; DX11 debug text remains a no-op.
- The unit-box asset is a proof placeholder, not final art.

Recommended next goal:

Run v0.12.1 Ferry Office Prop Replacement + Visual Scale Pass.

## v0.12.1 Baseline - 2026-05-15

Goal: use the v0.12 static mesh path for a small Ferry Office/service-yard prop replacement and visual scale pass. This goal must preserve gameplay, debug visibility, scene validation, Jolt opt-in validation, and the tiny mesh/render scope. It must not add final art, textures/materials, animation, an editor, runtime scene loading, a broad asset registry, mesh collision import, or new gameplay systems.

Required context read before coding:

- `AGENTS.md`
- `docs/STATUS.md`
- `docs/ARCHITECTURE.md`
- `docs/ROADMAP.md`
- `docs/GAME_DIRECTION.md`
- `docs/VERTICAL_SLICE.md`
- `docs/TECH_DEBT.md`
- `docs/MANUAL_TEST_CHECKLIST.md`
- `docs/DECISIONS.md`
- `docs/ASSET_GUIDE.md`
- `docs/SCENE_AUTHORING.md`
- `docs/ART_DIRECTION.md`
- `docs/MESH_RENDERING.md`

Baseline commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
python tools/scene_report.py
python tools/validate_scene.py
python tools/scale_audit.py
python tools/mesh_report.py
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 300
```

Baseline results:

- `scripts/verify.ps1`: passed. Default doctor/configure/build/test, scene validation, mesh report, and null smoke all completed.
- Default CTest passed 3/3 tests: `EngineCoreTests`, `EngineSmokeTest`, and `SceneToolTests`.
- Null smoke reported engine `v0.12.0`, loaded `unit-box-mesh`, and preserved the Ferry Office start state.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed; built `Jolt.lib`, `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 3/3 tests green.
- `python tools/scene_report.py`: completed; reported 1 mesh asset and 4 mesh instances.
- `python tools/validate_scene.py`: passed.
- `python tools/scale_audit.py`: completed; no suspicious scale issues found.
- `python tools/mesh_report.py`: completed; reported `unit-box-mesh`, 4 uses, 8 vertices, and 36 indices.
- GDI bounded visual review: passed; window created, cursor captured/released, `unit-box-mesh` loaded, `gdi-fallback` renderer ran for 300 frames, clean shutdown.

Baseline visual/readability observations:

- The scene has the correct gameplay/debug structure, but the mesh layer is still sparse: roof cap, service gate, maintenance box, and vehicle body only.
- The Ferry Office still needs a clearer front/facade cue so it reads as an office volume rather than mostly stacked service boxes.
- The vehicle would benefit from a simple cabin silhouette; the v0.12 body alone reads as a generic block.
- The dock/service-yard needs a few small scale props, such as bollards/crates/sign boards, to make the environment feel intentionally placed.
- Debug text and old markers remain the most important readability layer; any mesh additions must stay muted and draw under/around debug markers.

## v0.12.1 Short Implementation Plan

1. Add tests first for the new expected mesh scene ids/counts so scene data changes are guarded.
2. Reuse `assets/models/unit_box.gltf` for the pass unless a new mesh asset is truly needed; this keeps the loader subset stable.
3. Add a small set of purposeful mesh instances to `data/scenes/ferry_office.scene.json`: Ferry Office facade/sign cue, dock bollards, service-yard crate, and vehicle cabin.
4. Mirror those mesh instances in `SandboxLayer` through a more readable local mesh-instance table/helper rather than scattering ad-hoc draw calls.
5. Keep debug collision, route, interaction, traversal, vehicle, and world-state markers visible on top of/around mesh placeholders.
6. Update scene tools/tests/docs for the new mesh instance count and visual observations.
7. Run the full default/Jolt/tool/windowed validation matrix and capture/update screenshot evidence if practical.

## v0.12.1 Implementation Notes - 2026-05-15

Changed:

- Updated project version to `0.12.1`.
- Added scene-tool tests for the v0.12.1 readability mesh instance ids and a minimum mesh-instance count.
- Extended `data/scenes/ferry_office.scene.json` from 4 to 10 mesh instances while reusing the single original `unit_box.gltf` asset.
- Added mesh-backed placeholders for:
  - Ferry Office facade panel,
  - Ferry Office sign/timetable board cue,
  - dock bollard left/right,
  - service-yard crate,
  - service-yard vehicle cabin.
- Kept the existing mesh-backed roof cap, service gate, maintenance box, and vehicle body.
- Updated `SandboxLayer::drawStaticMeshDebug` with a small unit-box draw helper and a synchronization note for the JSON/C++ mirror.
- Captured a GDI visual reference at `docs/images/v0.12.1-gdi-screenshot.png`.
- Updated architecture, roadmap, decisions, technical debt, manual checklist, asset/scene/art docs, and mesh rendering docs.

Test-first note:

```powershell
python tests\test_scene_tools.py
```

Result before scene data changes:

- Failed as expected because the default scene still had 4 mesh instances and did not yet contain `mesh-ferry-office-facade-panel`.

Focused checks after scene/runtime changes:

```powershell
python tests\test_scene_tools.py
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
python tools\scene_report.py
python tools\validate_scene.py
python tools\scale_audit.py
python tools\mesh_report.py
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 300
```

Results:

- `tests\test_scene_tools.py`: passed, 11/11 tests.
- `scripts/build.ps1`: passed.
- `tools\scene_report.py`: completed; reported 1 mesh asset and 10 mesh instances.
- `tools\validate_scene.py`: passed.
- `tools\scale_audit.py`: completed; no suspicious scale issues found.
- `tools\mesh_report.py`: completed; reported `unit-box-mesh`, 10 uses, 8 vertices, and 36 indices.
- GDI bounded run: passed; window created, mesh asset loaded, `gdi-fallback` renderer ran for 300 frames, clean shutdown.

Screenshot note:

- The first full-screen screenshot attempt captured the console window instead of the game window, so it was discarded as visual evidence.
- A second targeted `PrintWindow` capture against the `Tidebreak Prototype` window succeeded and wrote `docs/images/v0.12.1-gdi-screenshot.png`.

Visual observations:

- The scene reads more clearly as a Ferry Office/service-yard blockout than v0.12 because the office now has a front/facade cue, a simple sign board, dock bollards, a service-yard crate, and a vehicle cabin silhouette.
- The service gate and route/debug markers remain visually dominant, which is useful for validation but still clearly debug/prototype presentation.
- Debug text, collision wires, route lines, interactable markers, traversal markers, and vehicle markers remain visible.
- The area is still far from commercial presentation: there are no real materials, textures, lighting, depth-aware mesh rendering, final meshes, or authored camera composition.

Known limitations after implementation:

- No new mesh files were added; every prop still reuses the unit-box placeholder.
- Mesh instances remain duplicated between scene JSON and `SandboxLayer` C++.
- Mesh placeholders remain visual-only and do not define collision or physics.
- The current screenshot capture is an ad-hoc validation artifact, not a reusable workbench command.

## v0.12.1 Final Validation - 2026-05-15

Commands run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure.ps1
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
python tools\scene_report.py
python tools\validate_scene.py
python tools\scale_audit.py
python tools\mesh_report.py
python tools\status_report.py
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 300
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 300
```

Results:

- `scripts/doctor.ps1`: completed. Existing warnings remain: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg` are not in the plain PowerShell PATH.
- `scripts/configure.ps1`: passed with preset `windows-vs2022-debug`.
- `scripts/build.ps1`: passed; built `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 3/3 tests green.
- `scripts/verify.ps1`: passed; repeated doctor/configure/build/test, ran scene validation, ran mesh report, and completed null smoke. Smoke log reported engine `v0.12.1`, loaded `unit-box-mesh`, and mesh report showed 10 mesh instances.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed; built `Jolt.lib`, `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 3/3 tests green.
- `python tools\scene_report.py`: completed; reported 1 mesh asset and 10 mesh instances.
- `python tools\validate_scene.py`: passed.
- `python tools\scale_audit.py`: completed; no suspicious scale issues found.
- `python tools\mesh_report.py`: completed; reported `unit-box-mesh`, 10 uses, 8 vertices, and 36 indices.
- `python tools\status_report.py`: completed and showed the expected v0.12.1 modified files plus `docs/images/v0.12.1-gdi-screenshot.png` before commit.
- GDI bounded run: passed; window created, cursor captured/released, mesh asset loaded, `gdi-fallback` renderer ran for 300 frames, clean shutdown.
- DX11 bounded run: passed; hardware DX11 device failed and fell back to WARP, mesh asset loaded, `dx11` renderer ran for 300 frames, clean shutdown, cursor released.

Visual evidence:

- Captured GDI screenshot: `docs/images/v0.12.1-gdi-screenshot.png`.
- Screenshot shows a stronger Ferry Office facade/sign cue, dock bollards, service-yard crate, and vehicle cabin while preserving debug text, route lines, collision wires, interactable markers, traversal markers, and vehicle markers.

Known limitations after v0.12.1:

- The scene is more readable than v0.12, but still clearly debug-placeholder presentation.
- Mesh placeholders are still unit-box instances, not final authored props.
- Runtime scene loading, asset registry, material/textures, lighting, depth-aware mesh rendering, and mesh collision import remain deferred.
- DX11 still falls back to WARP in this environment.

Recommended next goal:

Run v0.13 Vehicle Feel + Service Yard Road Test Polish.

## v0.13 Baseline - 2026-05-15

Goal: polish the existing deterministic service-yard vehicle feel and tiny road-test area on top of v0.12.1. This goal must preserve the Ferry Office micro-slice, scene/mesh tooling, default validation, and Jolt opt-in validation. It must not add Jolt VehicleConstraint, wheel simulation, traffic, NPC AI, missions, inventory, save/load, new asset pipeline work, new renderer scope, or other major gameplay systems.

Required context read before coding:

- `AGENTS.md`
- `docs/STATUS.md`
- `docs/ARCHITECTURE.md`
- `docs/ROADMAP.md`
- `docs/GAME_DIRECTION.md`
- `docs/VERTICAL_SLICE.md`
- `docs/TECH_DEBT.md`
- `docs/MANUAL_TEST_CHECKLIST.md`
- `docs/DECISIONS.md`
- `docs/PHYSICS_DECISION.md`
- `docs/SCENE_AUTHORING.md`
- `docs/ASSET_GUIDE.md`
- `docs/MESH_RENDERING.md`

Baseline commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
python tools/scene_report.py
python tools/validate_scene.py
python tools/scale_audit.py
python tools/mesh_report.py
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 300
```

Baseline results:

- Git state before v0.13 work: `main...origin/main`, clean working tree.
- `scripts/verify.ps1`: passed. Default doctor/configure/build/test, scene validation, mesh report, and null smoke all completed.
- Default CTest passed 3/3 tests: `EngineCoreTests`, `EngineSmokeTest`, and `SceneToolTests`.
- Null smoke reported engine `v0.12.1`, loaded `unit-box-mesh`, and preserved the Ferry Office start state.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed; built `Jolt.lib`, `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 3/3 tests green.
- `python tools/scene_report.py`: completed; reported 1 mesh asset and 10 mesh instances.
- `python tools/validate_scene.py`: passed.
- `python tools/scale_audit.py`: completed; no suspicious scale issues found.
- `python tools/mesh_report.py`: completed; reported `unit-box-mesh`, 10 uses, 8 vertices, and 36 indices.
- GDI bounded vehicle review: passed; window created, `unit-box-mesh` loaded, `gdi-fallback` renderer ran for 300 frames, clean shutdown.

Baseline vehicle/readability observations:

- The current vehicle is deterministic and testable, but the default forward speed (`12 m/s`) is high for the very small service-yard bounds.
- Vehicle tuning currently lives mostly in `VehicleControllerSettings`, while `SandboxLayer` only overrides yard bounds and camera settings.
- The current GDI review confirms vehicle telemetry, spawn, and debug markers are visible at rest, but it does not prove hands-on driving feel in this environment.
- Manual driving on the target laptop is still needed after this goal; v0.13 can improve deterministic feel and validation confidence, not replace human feel testing.
- Scene/C++ drift risk is high around service-yard constants: spawn, yaw, bounds, pad, rails, road-test lines, vehicle mesh body/cabin, and physics debug rails are mirrored by hand.

Read-only subagent notes:

- Vehicle coverage gaps: focus facing/radius behavior, drag/coast behavior, large-delta clamp, bounds-hit behavior, exit-position geometry, reverse steering direction, stationary steering no-op, and scene/runtime vehicle constant sync.
- Scene drift risks: service-yard pad/rails/bounds/spawn are duplicated in JSON and `SandboxLayer`; the road-test center lines are C++ only and should be derived from shared local constants if touched.

## v0.13 Short Implementation Plan

1. Add failing tests first for vehicle focus/fallback, drag/delta clamp, bounds clamp, exit-position geometry, reverse steering behavior, and initial SandboxLayer vehicle telemetry.
2. Add small scene-tool tests for exact service-yard vehicle spawn/bounds/proxy dimensions so JSON changes stay guarded.
3. Keep the live vehicle deterministic and game-layer scoped. Do not move it to Jolt or add wheel/suspension simulation.
4. Tune vehicle settings conservatively for the small yard: lower max speed/reverse speed, smoother acceleration/braking, stronger drag, and readable low-speed steering.
5. Tune vehicle camera settings only through existing distance/height/smoothing controls.
6. Introduce a small `ServiceYardLayout` constants block in `SandboxLayer.cpp` to reduce hardcoded drift for spawn, bounds, pad/rail visuals, and physics debug rails.
7. If service-yard bounds or visuals change, update `data/scenes/ferry_office.scene.json` in the same change and rerun scene tools.
8. Update docs with exact tuning decisions, what remains deterministic, and why Jolt VehicleConstraint remains deferred.
9. Run the full default/Jolt/tool/windowed validation matrix and capture/update GDI screenshot evidence if practical.

## v0.13 Implementation Notes - 2026-05-15

Red/green checks:

- Added a scene-tool regression test requiring the service-yard vehicle spawn/proxy and widened bounds `[3.35, -5.05]..[9.25, 0.65]`.
- `python tests\test_scene_tools.py` failed first because scene data still had old bounds `[3.55, -4.55]..[8.95, 0.35]`.
- Added vehicle tests for default small-yard tuning, focus facing/close fallback, bounds clamp telemetry, drag timestep clamp, exit-position offsets, reverse steering, and SandboxLayer vehicle debug telemetry.
- `ctest --preset windows-vs2022-debug --output-on-failure` failed first on the default vehicle tuning expectations and the scene bounds regression.
- After implementation, `python tests\test_scene_tools.py` and `ctest --preset windows-vs2022-debug --output-on-failure` both passed.

Changed:

- Updated project version to `0.13.0`.
- Tuned the deterministic vehicle controller defaults for the compact service-yard road test: `maxForwardSpeed=7.5`, `maxReverseSpeed=2.75`, `acceleration=6.5`, `braking=10.0`, `drag=2.4`, and `steeringRate=2.05`.
- Tuned vehicle camera mode to a closer/firmer debug chase view: distance `7.25`, height offset `2.05`, smoothing `10.5`.
- Grouped service-yard runtime constants in `SandboxLayer.cpp` for spawn, yaw, bounds, pad, rails, back-stop, crate, vehicle body, and vehicle cabin.
- Widened the service-yard pad/bounds and mirrored the changes in `data/scenes/ferry_office.scene.json`.
- Added a physics debug back-stop box to match the visible service-yard back-stop.
- Updated roadmap, architecture, decisions, runbook, scene-authoring notes, tech debt, and manual test checklist for v0.13.

Visual/manual observations:

- Bounded GDI at-rest review showed the Ferry Office scene, service-yard vehicle telemetry, mesh placeholders, and debug markers still visible.
- The v0.13 screenshot at `docs/images/v0.13-gdi-screenshot.png` shows the larger service-yard pad/rails/back-stop area on the right and the vehicle proxy still readable.
- This environment can run bounded windows but does not replace a hands-on laptop driving playtest. Manual feel testing remains the next recommended step.

## v0.13 Final Validation - 2026-05-15

Commands and results:

- `powershell -ExecutionPolicy Bypass -File scripts\doctor.ps1`: passed. Same environment warnings remain: `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg` are not on PATH, but CMake/VS preset builds work.
- `powershell -ExecutionPolicy Bypass -File scripts\configure.ps1`: passed for `windows-vs2022-debug`.
- `powershell -ExecutionPolicy Bypass -File scripts\build.ps1`: passed; built `EngineCore.lib`, `GamePrototype.lib`, `EngineApp.exe`, and `EngineCoreTests.exe`.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 3/3 tests.
- `powershell -ExecutionPolicy Bypass -File scripts\verify.ps1`: passed; repeated doctor/configure/build/CTest, scene validation, mesh report, and null smoke. Null smoke reported engine `v0.13.0`.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed; built Jolt and project targets.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 3/3 tests.
- `python tools\scene_report.py`: passed; vehicle bounds now report `[3.35, -5.05]..[9.25, 0.65]`.
- `python tools\validate_scene.py`: passed.
- `python tools\scale_audit.py`: passed; no suspicious scale issues found.
- `python tools\mesh_report.py`: passed; `unit-box-mesh` has 10 uses, 8 vertices, and 36 indices.
- `python tools\status_report.py`: completed and reported the expected v0.13 modified files before cleanup/commit.
- `build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 300`: passed; GDI window created, `unit-box-mesh` loaded, ran 300 frames, clean shutdown.
- `build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 300`: passed; hardware DX11 device failed and WARP was used, then the DX11 run completed cleanly.
- Captured and visually checked `docs/images/v0.13-gdi-screenshot.png`.

Known limitations after v0.13:

- The vehicle is still deterministic placeholder code. No Jolt VehicleConstraint, wheel colliders, suspension, tire model, gearbox, damage, vehicle entity system, or traffic exists.
- Vehicle collision is still bounds clamp plus safe exit checks, not full physical collision against the world.
- Service-yard scene data is still mirrored manually in C++ constants. Runtime scene loading/source-of-truth migration remains deferred.
- Manual driving feel must be tested on the target laptop; bounded frame runs only prove startup, rendering, and deterministic state.
- DX11 still falls back to WARP in this environment.

Recommended next goal:

Run v0.13.1 Vehicle Manual Playtest + Control Polish.

## v0.13.1 Playtest Feedback Patch - 2026-05-15

User playtest notes after v0.13:

- Vehicle feel is strong enough to continue polishing.
- Vehicle exit marker needs clearer safe/blocked feedback.
- Scene/readability can still improve around the vehicle/service-yard UI cues.
- GDI/debug UI flickers during play.

Root-cause notes:

- GDI was rendering directly to the window device context and clearing the window every frame, which can make debug text/UI visibly flicker.
- Win32 default background erase could also contribute to flicker between immediate GDI frames.
- The window title was updated regularly with the full multi-line debug text, which could repaint the title bar noisily.
- Vehicle exit marker color only turned red on a failed exit press, so the player could not tell in advance whether the exit point was safe.

Patch:

- Added a GDI memory backbuffer and blits once at end of frame to reduce UI/debug text flicker.
- Suppressed `WM_ERASEBKGND` for the Win32 game window.
- Added `BuildDebugWindowTitle` so the title bar uses a short single-line debug summary and updates only when the title changes.
- Added `exitClear=yes/no` to vehicle debug text.
- Made the vehicle exit marker show clear/blocked state before pressing `E`, with a larger ground ring, vertical beacon, and link line from the vehicle.

Validation:

- `powershell -ExecutionPolicy Bypass -File scripts\build.ps1`: passed.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 3/3 tests.

## v0.14 Baseline - 2026-05-15

Goal: add the first short island/dock road segment connected to the Ferry Office/service-yard prototype so the v0.13 vehicle has a more meaningful place to drive, without adding new gameplay systems, final art, full map expansion, runtime scene loading, Jolt VehicleConstraint, or vehicle physics migration.

Required context read before coding:

- `AGENTS.md`
- `docs/STATUS.md`
- `docs/GAME_DIRECTION.md`
- `docs/ROADMAP.md`
- `docs/ARCHITECTURE.md`
- `docs/TECH_DEBT.md`
- `docs/MANUAL_TEST_CHECKLIST.md`
- `docs/DECISIONS.md`
- `docs/SCENE_AUTHORING.md`
- `docs/ASSET_GUIDE.md`
- `docs/ART_DIRECTION.md`

Baseline commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
python tools\scene_report.py
python tools\validate_scene.py
python tools\scale_audit.py
python tools\mesh_report.py
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 300
```

Baseline results:

- Git state before v0.14 work: `main...origin/main`, clean working tree.
- `scripts\verify.ps1`: passed. Default doctor/configure/build/CTest, scene validation, mesh report, and null smoke all completed. Null smoke reported engine `v0.13.0` and vehicle debug text included `exitClear=yes`.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 3/3 tests.
- `python tools\scene_report.py`: completed; current authoring mirror still reports 14 visual placeholders, 10 mesh instances, 1 vehicle, and vehicle bounds `[3.35, -5.05]..[9.25, 0.65]`.
- `python tools\validate_scene.py`: passed.
- `python tools\scale_audit.py`: passed; no suspicious scale issues found.
- `python tools\mesh_report.py`: passed; `unit-box-mesh` has 10 scene uses.
- Bounded GDI review: passed; window created, cursor captured, `unit-box-mesh` loaded, GDI ran 300 frames, clean shutdown.

Baseline visual/drive observations:

- The vehicle/service-yard feel is promising but still spatially boxed into a small yard.
- The service-yard vehicle currently has no meaningful road beyond the immediate pad/rails/back-stop.
- The best low-risk road connection is to extend from the service-yard side of the scene into a short dock/service road with finite bounds and clear edge markers.
- The GDI/debug UI flicker patch is present in the baseline; no new UI system is needed for this goal.
- Scene/C++ drift risk remains the main production risk: road placeholders, route markers, vehicle bounds, and runtime debug drawing must be kept mirrored in `data/scenes/ferry_office.scene.json` and `SandboxLayer.cpp`.

## v0.14 Short Implementation Plan

1. Add failing tests first for scene road ids, road route marker presence, updated vehicle bounds that include the road segment, and road/debug telemetry in `SandboxLayer`.
2. Keep the road as placeholder authoring data plus explicit C++ runtime drawing. Do not introduce runtime scene loading in v0.14.
3. Extend service-yard vehicle bounds only enough for a compact dock road segment and turn-around/end marker.
4. Add road pad, edge rails/barriers, water/shore cue, lane/route line, and a road-end marker using existing solid debug boxes/lines and unit-box mesh path where useful.
5. Update `data/scenes/ferry_office.scene.json` with road visual placeholders, route marker(s), objective marker, and bounds changes.
6. Preserve Ferry Office interactions, traversal, remembered state, vehicle enter/exit, GDI debug UI patch, mesh tools, and Jolt opt-in validation.
7. Update docs with what road segment was added, why it remains placeholder-only, and what remains before a real island route/map.
8. Run full default/Jolt/tool/windowed validation and capture a v0.14 GDI screenshot if practical.

## v0.14 Implementation Notes

TDD / targeted checks before implementation:

- Added Python scene-tool tests expecting v0.14 dock road ids and expanded vehicle bounds.
- Added a C++ `SandboxLayer` debug-text test expecting `roadSegment=dock-road` and finite road bounds telemetry.
- `python tests\test_scene_tools.py`: failed as expected before implementation because `dock-road-segment` did not exist and vehicle bounds still ended at `[9.25, 0.65]`.
- `build\windows-vs2022-debug\Debug\EngineCoreTests.exe`: failed as expected before implementation because road telemetry did not exist.

Implemented changes:

- Bumped project version to `0.14.0`.
- Added a compact dock road segment connected to the service-yard east side:
  - road pad,
  - turn-around pad,
  - shore/water edge cue,
  - south rail,
  - north curb,
  - road-end marker,
  - route line from the service-yard vehicle toward the road end.
- Split the old full east service-yard rail into smaller entry posts so the road connection reads as open.
- Expanded deterministic vehicle bounds from `[3.35, -5.05]..[9.25, 0.65]` to `[3.35, -5.05]..[19.45, 0.95]`.
- Mirrored the road layout in `data/scenes/ferry_office.scene.json` with new visual placeholders, `route-service-yard-to-dock-road`, and `dock-road-marker`.
- Added `roadSegment=dock-road` and `roadBounds=(3.35,-5.05)-(19.45,0.95)` to debug text.

Targeted post-implementation checks:

- `python tests\test_scene_tools.py`: passed, 13 tests.
- `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`: passed.
- `build\windows-vs2022-debug\Debug\EngineCoreTests.exe`: passed.
- `python tools\validate_scene.py`: passed.
- `python tools\scale_audit.py`: passed; no suspicious scale issues found.
- `python tools\scene_report.py`: passed; now reports 21 visual placeholders, 5 route markers, 4 objective markers, and vehicle bounds `[3.35, -5.05]..[19.45, 0.95]`.

Known implementation limitation:

- The v0.14 road segment is still manually mirrored between JSON and `SandboxLayer.cpp`. This is acceptable for v0.14 but makes v0.15 Runtime Scene Loading / Scene Data Source of Truth the strongest next architecture goal.

## v0.14 Final Validation - 2026-05-15

Commands run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts\configure.ps1
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts\verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
python tools\scene_report.py
python tools\validate_scene.py
python tools\scale_audit.py
python tools\mesh_report.py
python tools\status_report.py
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 300
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 300
```

Results:

- `scripts\doctor.ps1`: passed; expected PATH warnings remain for `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- `scripts\configure.ps1`: passed for `windows-vs2022-debug`.
- `scripts\build.ps1`: passed.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 3/3 tests.
- `scripts\verify.ps1`: passed; null smoke reported engine `v0.14.0` and road telemetry `roadSegment=dock-road`.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 3/3 tests.
- `python tools\scene_report.py`: passed; scene reports 21 visual placeholders, 10 mesh instances, 5 route markers, 4 objective markers, and vehicle bounds `[3.35, -5.05]..[19.45, 0.95]`.
- `python tools\validate_scene.py`: passed.
- `python tools\scale_audit.py`: passed; no suspicious scale issues found.
- `python tools\mesh_report.py`: passed; `unit-box-mesh` still has 10 uses.
- `python tools\status_report.py`: completed and showed the expected v0.14 modified files before commit.
- `EngineApp.exe --renderer gdi --frames 300`: passed; window created, cursor captured, GDI ran 300 frames, clean shutdown.
- `EngineApp.exe --renderer dx11 --frames 300`: passed; hardware DX11 device failed and WARP was used, then the bounded run completed cleanly.
- Captured `docs/images/v0.14-gdi-screenshot.png` as the v0.14 visual reference.

Visual observations:

- The service-yard now has a clear east opening into a longer dock/service road segment.
- The yellow route line and road-end marker make the compact out-and-back drive more legible than the previous tiny yard pad.
- The dark road pad, rail/curb edges, and water/shore cue make the scene read more like a dock road placeholder.
- Debug text and collision/interaction/traversal markers remain visible, but the scene is still debug-heavy and not commercial art.

Recommended next goal:

Build v0.15 Runtime Scene Loading / Scene Data Source of Truth.

## v0.15 Baseline - 2026-05-15

Goal: make `data/scenes/ferry_office.scene.json` a runtime source of truth for the current Ferry Office, service-yard, and dock-road prototype layout without adding new gameplay systems.

Commands run before implementation:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
python tools\scene_report.py
python tools\validate_scene.py
python tools\scale_audit.py
python tools\mesh_report.py
```

Baseline results:

- `scripts\verify.ps1`: passed; null smoke reported engine `v0.14.0`, `roadSegment=dock-road`, and vehicle road bounds `(3.35,-5.05)-(19.45,0.95)`.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 3/3 tests.
- `python tools\scene_report.py`: passed; scene reports 9 colliders, 21 visual placeholders, 1 mesh asset, 10 mesh instances, 5 interactables, 1 traversal affordance, 1 vehicle, 5 route markers, 4 objective markers, and vehicle bounds `[3.35, -5.05]..[19.45, 0.95]`.
- `python tools\validate_scene.py`: passed.
- `python tools\scale_audit.py`: passed; no suspicious scale issues found.
- `python tools\mesh_report.py`: passed; `unit-box-mesh` has 10 uses.

Inspection notes:

- Static colliders are duplicated between `data/scenes/ferry_office.scene.json` and `PrototypeWorld::buildFerryOfficePrototypeLayout`.
- Vehicle spawn, yaw, bounds, proxy extents, and service-yard/dock-road visuals are duplicated between scene JSON and `SandboxLayer.cpp`.
- Interactables and traversal are duplicated between scene JSON and `FerryOfficeData` / `PrototypeScene`, but Ferry Office world-state behavior should remain scene-owned C++ for now.
- Mesh instances are authored in scene JSON but still submitted from a hand-mirrored C++ list.
- The current vehicle proxy height differs between JSON (`proxyHalfExtents.y = 0.53`) and runtime physics/debug body height (`0.34`), so v0.15 must resolve that deliberately.

## v0.15 Implementation Plan - 2026-05-15

1. Add failing C++ tests for scene path parsing, default scene loading, missing-scene errors, collider loading, vehicle bounds loading, route/objective markers, mesh instances, and scene-driven PrototypeScene setup.
2. Add a narrow `SceneDefinition` / `SceneLoader` boundary under `src/game` using `nlohmann/json` internally, with no third-party types exposed to gameplay code.
3. Add `--scene` config parsing and pass the selected scene path into `SandboxLayer`; default remains `data/scenes/ferry_office.scene.json`.
4. Build `PrototypeWorld` colliders, `PrototypeScene` interactables/traversal, vehicle spawn/bounds/proxy, visual placeholders, route markers, objective markers, and mesh instances from loaded scene data where practical.
5. Keep Tidebreak-specific behavior mapping in C++ for this goal: world-state flags, interaction results, traversal completion side effects, and objective text.
6. Update Python scene tools only if the schema/expectations change, then update docs with what is now loaded versus what remains hardcoded.
7. Run the full v0.15 validation set, then commit and push if validation passes.

## v0.15 Implementation Summary - 2026-05-15

TDD checkpoint:

- Added runtime scene-loader tests before implementation.
- Confirmed the initial red build:
  - `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`
  - Result: failed as expected with missing `game/SceneDefinition.h`.

Implemented changes:

- Bumped project version to `0.15.0`.
- Added `nlohmann/json` through pinned CMake `FetchContent` for runtime scene loading.
- Added `src/game/SceneDefinition.*` and `src/game/SceneLoader.*`.
- Added `--scene <path>` CLI parsing.
- `SandboxLayer` now loads `data/scenes/ferry_office.scene.json` by default.
- Runtime scene data now drives:
  - player start,
  - static colliders,
  - visual placeholders,
  - mesh instances,
  - interactables,
  - traversal affordances,
  - service-yard vehicle spawn, enter radius, proxy half extents, and bounds,
  - route markers,
  - objective markers.
- `PrototypeWorld` can build static colliders from `SceneDefinition`.
- `PrototypeScene` can build interactables and traversal affordances from `SceneDefinition`.
- Static mesh debug drawing now uses loaded scene mesh instances instead of a hand-mirrored list.
- GDI/null/DX11 debug text now reports `scene=ferry-office loaded=yes`.
- Scene paths and mesh asset paths resolve from the source root when the app is launched from a build directory.

Known limitations:

- Scene loading is one-shot at startup; there is no hot reload, editor, prefab system, scene diff, or runtime editing.
- Invalid runtime scene paths log a warning and fall back to built-in Ferry Office setup so smoke/debug paths remain usable.
- World-state effects, interaction result mapping, traversal completion side effects, objective text, and dynamic state coloring remain C++ scene behavior.
- The project now has a default `nlohmann/json` dependency; configure may populate or reuse the CMake dependency cache.
- DX11 still falls back to WARP on this machine, but bounded DX11 runs exit cleanly.

## v0.15 Final Validation - 2026-05-15

Commands run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts\configure.ps1
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts\verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
python tools\scene_report.py
python tools\validate_scene.py
python tools\scale_audit.py
python tools\mesh_report.py
python tools\status_report.py
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 300
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 300
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --scene data\scenes\ferry_office.scene.json --frames 300
```

Results:

- `scripts\doctor.ps1`: passed; expected PATH warnings remain for `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- `scripts\configure.ps1`: passed for `windows-vs2022-debug`.
- `scripts\build.ps1`: passed.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 3/3 tests.
- `scripts\verify.ps1`: passed; null smoke reported engine `v0.15.0`, loaded scene `ferry-office`, and `roadBounds=(3.35,-5.05)-(19.45,0.95)`.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 3/3 tests.
- `python tools\scene_report.py`: passed; scene reports 9 colliders, 21 visual placeholders, 1 mesh asset, 10 mesh instances, 5 interactables, 1 traversal affordance, 1 vehicle, 5 route markers, and 4 objective markers.
- `python tools\validate_scene.py`: passed.
- `python tools\scale_audit.py`: passed; no suspicious scale issues found.
- `python tools\mesh_report.py`: passed; `unit-box-mesh` has 10 uses.
- `python tools\status_report.py`: completed and showed the expected v0.15 modified/new files before commit.
- `EngineApp.exe --renderer gdi --frames 300`: passed; scene loaded, mesh loaded, GDI ran 300 frames, clean shutdown.
- `EngineApp.exe --renderer dx11 --frames 300`: passed; hardware DX11 device failed and WARP was used, then the bounded run completed cleanly.
- `EngineApp.exe --renderer gdi --scene data\scenes\ferry_office.scene.json --frames 300`: passed; explicit scene path loaded and the bounded run completed cleanly.

Recommended next goal:

Build v0.16 First Driver/Fixer Job Prototype.

## v0.16 Baseline - 2026-05-15

Goal: add the first narrow driver/fixer job prototype inside the existing Ferry Office, service-yard, and dock-road scene using existing movement, interaction, traversal, vehicle, world-state, scene-loading, and debug systems. This is not a generic mission system, economy, inventory, NPC, traffic, save/load, or scripting milestone.

Required context read before implementation:

- `AGENTS.md`
- `docs/STATUS.md`
- `docs/GAME_DIRECTION.md`
- `docs/VERTICAL_SLICE.md`
- `docs/ROADMAP.md`
- `docs/ARCHITECTURE.md`
- `docs/TECH_DEBT.md`
- `docs/SCENE_AUTHORING.md`
- `docs/MANUAL_TEST_CHECKLIST.md`
- `docs/DECISIONS.md`

Baseline commands:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
python tools\scene_report.py
python tools\validate_scene.py
python tools\scale_audit.py
python tools\mesh_report.py
```

Baseline results:

- Git state before v0.16 work: `main...origin/main`, clean working tree.
- `scripts\verify.ps1`: passed. Default doctor/configure/build/CTest, scene validation, mesh report, and null smoke all completed. Null smoke reported engine `v0.15.0`, loaded scene `ferry-office`, and the expected dock road bounds.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 3/3 tests.
- `python tools\scene_report.py`: passed; scene reports 9 colliders, 21 visual placeholders, 1 mesh asset, 10 mesh instances, 5 interactables, 1 traversal affordance, 1 vehicle, 5 route markers, and 4 objective markers.
- `python tools\validate_scene.py`: passed.
- `python tools\scale_audit.py`: passed; no suspicious scale issues found.
- `python tools\mesh_report.py`: passed; `unit-box-mesh` has 10 scene uses.

## v0.16 Short Implementation Plan

1. Add failing tests first for Ferry Office job state/progression, new job world flags, scene-authored service checkpoint ids, vehicle checkpoint completion, repeat-safe completion, and debug text exposure.
2. Add a small `FerryOfficeJob` game-layer helper that owns explicit objective phases and job completion logic without becoming a mission graph or scripting system.
3. Extend `WorldState` with only the remembered flags needed for the job loop: job start, service vehicle use, dock-road checkpoint, service-run confirmation, and job completion.
4. Add a scene-authored service-run checkpoint/confirmation marker to `data/scenes/ferry_office.scene.json` and keep the runtime using loaded scene data for its position/radius.
5. Wire existing mechanics into the job: manifest starts the job, traversal records service route use, maintenance restores power, wall button opens route, vehicle enter records service vehicle use, reaching the road checkpoint records dock-road progress, and confirmation completes the job.
6. Keep existing Ferry Office slice completion behavior intact; the new job can sit above it as a narrow prototype task, not a replacement mission system.
7. Update docs, especially stale `TECH_DEBT.md` wording, manual checklist, roadmap, architecture, scene authoring, vertical slice, and decisions.
8. Run the full default/Jolt/tool/windowed validation matrix and commit/push only after successful validation.

## v0.16 Implementation Summary - 2026-05-15

TDD checkpoint:

- Added failing C++ tests for missing `FerryOfficeJob`, first-job flags, scene-authored service-run markers, objective ordering, vehicle checkpoint behavior, and repeat-safe completion.
- Added failing Python scene-tool tests for `service-run-confirm-marker`, `service-run-checkpoint-marker`, and `route-dock-road-to-service-confirm`.
- Confirmed red results:
  - `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`: failed as expected with missing `game/FerryOfficeJob.h`.
  - `python tests\test_scene_tools.py`: failed as expected because the new v0.16 scene ids did not exist yet.

Implemented changes:

- Bumped project version to `0.16.0`.
- Added `src/game/FerryOfficeJob.*` as a narrow explicit Ferry Office Service Call helper.
- Extended `WorldState` with first-job flags:
  - `ferryOfficeJobStarted`,
  - `serviceVehicleUsed`,
  - `dockRoadReached`,
  - `serviceRunConfirmed`,
  - `ferryOfficeJobComplete`.
- Updated `PrototypeScene` so existing actions feed the job:
  - manifest/notice can start the job,
  - service route traversal remains recorded,
  - maintenance and power remain existing world-state flags,
  - wall button still opens the route,
  - Service Run Marker confirms/completes the job only after prerequisites.
- Updated `SandboxLayer` so entering the vehicle records service vehicle use and driving into the scene-authored dock-road checkpoint records `dockRoadReached`.
- Added scene data entries:
  - `service-run-confirm-marker`,
  - `service-run-checkpoint-marker`,
  - `route-dock-road-to-service-confirm`.
- Added GDI/debug text for `jobObjective`, `jobPhase`, `serviceVehicleUsed`, `dockRoadReached`, `serviceRunConfirmed`, and `jobComplete`.
- Updated docs for the new job loop and refreshed stale `TECH_DEBT.md` sections.

Targeted post-implementation checks:

- `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`: passed.
- `build\windows-vs2022-debug\Debug\EngineCoreTests.exe`: passed.
- `python tests\test_scene_tools.py`: passed, 14 tests.

Known implementation limitations:

- `FerryOfficeJob` is an explicit scene helper, not a generic mission framework.
- Service Run Marker interaction before prerequisites does not complete the job; a manual playtest should check whether prompt wording needs polish.
- Route marker endpoints are still not validated against known ids by Python tools.
- The vehicle remains deterministic placeholder movement; Jolt VehicleConstraint remains deferred.

## v0.16 Final Validation - 2026-05-15

Commands run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts\configure.ps1
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts\verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
python tools\scene_report.py
python tools\validate_scene.py
python tools\scale_audit.py
python tools\mesh_report.py
python tools\status_report.py
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 360
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 360
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --scene data\scenes\ferry_office.scene.json --frames 360
```

Results:

- `scripts\doctor.ps1`: passed; expected PATH warnings remain for `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- `scripts\configure.ps1`: passed for `windows-vs2022-debug`.
- `scripts\build.ps1`: passed.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 3/3 tests.
- `scripts\verify.ps1`: passed; null smoke reported engine `v0.16.0`, loaded scene `ferry-office`, 6 interactables, and first-job debug text with `jobPhase=collectManifest`.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 3/3 tests.
- `python tools\scene_report.py`: passed; scene reports 9 colliders, 21 visual placeholders, 1 mesh asset, 10 mesh instances, 6 interactables, 1 traversal affordance, 1 vehicle, 6 route markers, and 5 objective markers.
- `python tools\validate_scene.py`: passed.
- `python tools\scale_audit.py`: passed; no suspicious scale issues found.
- `python tools\mesh_report.py`: passed; `unit-box-mesh` still has 10 uses.
- `python tools\status_report.py`: completed and showed the expected v0.16 modified/new files before cleanup/commit.
- `EngineApp.exe --renderer gdi --frames 360`: passed; window created, cursor captured, scene loaded, job debug text rendered/logged, and clean shutdown.
- `EngineApp.exe --renderer dx11 --frames 360`: passed; hardware DX11 device failed and WARP was used, then the bounded run completed cleanly.
- `EngineApp.exe --renderer gdi --scene data\scenes\ferry_office.scene.json --frames 360`: passed; explicit scene path loaded and the bounded run completed cleanly.

Manual playtest note:

- A full human-controlled driver/fixer job playthrough was not completed in this automated Codex run. The next goal should manually test objective clarity, vehicle checkpoint readability, and Service Run Marker placement on the target laptop.

Recommended next goal:

Build v0.16.1 First Job Manual Playtest + Objective/Marker Polish.

## v0.17 Baseline - 2026-05-15

Goal: create the first playable presentation mode for the existing Ferry Office Service Call loop, reduce debug-workbench clutter, and fix/document only small presentation/control regressions without adding Job #2 or any new major gameplay system.

Required context read before implementation:

- `AGENTS.md`
- `docs/STATUS.md`
- `docs/GAME_DIRECTION.md`
- `docs/VERTICAL_SLICE.md`
- `docs/ROADMAP.md`
- `docs/ARCHITECTURE.md`
- `docs/TECH_DEBT.md`
- `docs/SCENE_AUTHORING.md`
- `docs/MANUAL_TEST_CHECKLIST.md`
- `docs/DECISIONS.md`
- `docs/ART_DIRECTION.md`

Baseline commands:

```powershell
git status --short --branch
powershell -ExecutionPolicy Bypass -File scripts\verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
python tools\scene_report.py
python tools\validate_scene.py
python tools\scale_audit.py
python tools\mesh_report.py
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 180
```

Baseline results:

- Git state before v0.17 work: `main...origin/main`, clean working tree.
- `scripts\verify.ps1`: passed. Default doctor/configure/build/CTest, scene validation, mesh report, and null smoke all completed. Null smoke reported engine `v0.16.0`, loaded scene `ferry-office`, 6 interactables, and first-job debug text with `jobPhase=collectManifest`.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 3/3 tests.
- `python tools\scene_report.py`: passed; scene reports 9 colliders, 21 visual placeholders, 1 mesh asset, 10 mesh instances, 6 interactables, 1 traversal affordance, 1 vehicle, 6 route markers, and 5 objective markers.
- `python tools\validate_scene.py`: passed.
- `python tools\scale_audit.py`: passed; no suspicious scale issues found.
- `python tools\mesh_report.py`: passed; `unit-box-mesh` still has 10 scene uses.
- `EngineApp.exe --renderer gdi --frames 180`: passed; window created, cursor captured, GDI ran 180 frames, clean shutdown.

Presentation / bug review notes:

- The user already completed a manual v0.16 playtest and reported that the current first job loop feels very good.
- The current GDI overlay is still a raw multi-line debug dump. Objective and prompt information exists, but it competes with player/camera/vehicle/world telemetry.
- The window title mirrors the first raw debug line, so normal play still reads like a developer workbench.
- Cursor capture/release behaved correctly during the bounded GDI run.
- No crash or obvious bounded-run startup bug was observed before v0.17 changes.

## v0.17 Short Implementation Plan

1. Add failing tests first for UI mode config parsing, F1/debug-toggle input edge state, playtest/debug/minimal text composition, and preservation of full debug telemetry in debug mode.
2. Add a small `UiMode` runtime config with `playtest`, `debug`, and `minimal` values. Windowed default should become `playtest`; smoke/headless behavior must remain stable.
3. Extend `InputState` / Win32 input with a pressed-edge F1 debug overlay toggle if it stays small and safe.
4. Extend `SandboxLayer` to accept an initial UI mode, toggle between playtest and debug on F1, and build separate player-facing presentation text from the existing full debug text.
5. Keep full debug telemetry available through `--ui-mode debug`; keep minimal mode for a very small objective/prompt/status view.
6. Reduce playtest-mode visual clutter by keeping core route/interaction/traversal/vehicle markers while reserving full collider/physics telemetry overlays for debug mode.
7. Update docs and run the full default/Jolt/tool/windowed validation matrix, including explicit playtest/debug UI mode runs.
8. Commit and push only after successful validation.

## v0.17 Implementation Summary - 2026-05-15

TDD checkpoint:

- Added failing C++ tests for missing UI mode config parsing, missing F1 debug-overlay input edge state, missing playtest/debug/minimal text composition, and missing runtime overlay toggle behavior.
- Added Python scene-tool tests for route endpoint validation and service-run route/checkpoint alignment.
- Confirmed red results:
  - `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`: failed as expected before implementation because `AppConfig::uiMode`, `engine::UiMode`, `InputState::debugOverlayTogglePressed`, and the `SandboxLayer` UI-mode constructor did not exist.
  - `python tests\test_scene_tools.py`: failed as expected before route endpoint validation existed.

Implemented changes:

- Bumped project version to `0.17.0`.
- Added `engine::UiMode` with `playtest`, `debug`, and `minimal` modes.
- Added command-line parsing for:
  - `--ui-mode playtest`
  - `--ui-mode debug`
  - `--ui-mode minimal`
  - `--playtest-ui`
  - `--debug-ui`
- Normal app config now defaults to `playtest` mode. Existing tests can still construct `SandboxLayer` in debug mode explicitly.
- Added `F1` pressed-edge input tracking in the Win32 input snapshot.
- Added `SandboxLayer` presentation/debug text composition:
  - playtest mode prioritizes objective, focused prompt, job status, vehicle/checkpoint hints, and completion state.
  - debug mode preserves the full raw telemetry wall for Codex/development validation.
  - minimal mode keeps only objective, focused prompt, and job phase/completion status.
- Added `F1` runtime overlay toggle from playtest/minimal to debug and back.
- Reduced playtest-mode workbench clutter by reserving full grid/axes, collider boxes, world bounds, camera target, and physics debug lines for debug mode while keeping essential route, traversal, interaction, vehicle, and job markers visible.
- Added route marker endpoint validation to `tools\scene_data.py`.
- Updated runbook, architecture, roadmap, tech debt, manual checklist, decisions, and vertical-slice docs for v0.17.

Targeted post-implementation checks:

- `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`: passed.
- `build\windows-vs2022-debug\Debug\EngineCoreTests.exe`: passed.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 3/3 tests.
- `python tests\test_scene_tools.py`: passed, 16 tests.
- `powershell -ExecutionPolicy Bypass -File scripts\build.ps1`: passed after the new UI mode flags were added to the app executable.
- `EngineApp.exe --renderer gdi --ui-mode playtest --frames 60`: passed after rebuild and showed the player-facing objective/prompt/job/status overlay.
- `EngineApp.exe --renderer gdi --ui-mode debug --frames 60`: passed after rebuild and showed the full raw telemetry overlay.
- `EngineApp.exe --renderer gdi --ui-mode minimal --frames 60`: passed after rebuild and showed the compact objective/prompt/job overlay.

Known implementation limitations:

- This is still GDI/debug-text presentation, not a real UI framework.
- DX11 still has no text overlay, so overlay review should use GDI.
- A full human-controlled v0.17 Ferry Office Service Call playthrough was not completed inside this automated Codex run; the user manually liked v0.16, and v0.17 bounded GDI runs verified startup/text modes.
- Job #2 remains intentionally deferred.

## v0.17 Final Validation - 2026-05-15

Commands run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts\configure.ps1
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts\verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
python tools\scene_report.py
python tools\validate_scene.py
python tools\scale_audit.py
python tools\mesh_report.py
python tools\status_report.py
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --frames 360
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 360
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --scene data\scenes\ferry_office.scene.json --frames 360
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --ui-mode playtest --frames 360
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --ui-mode debug --frames 360
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --ui-mode minimal --frames 360
```

Results:

- `scripts\doctor.ps1`: passed; expected PATH warnings remain for `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- `scripts\configure.ps1`: passed for `windows-vs2022-debug`.
- `scripts\build.ps1`: passed.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 3/3 tests.
- `scripts\verify.ps1`: passed; null smoke reported engine `v0.17.0`, loaded scene `ferry-office`, and the new playtest overlay text.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 3/3 tests.
- `python tools\scene_report.py`: passed; scene reports 9 colliders, 21 visual placeholders, 1 mesh asset, 10 mesh instances, 6 interactables, 1 traversal affordance, 1 vehicle, 6 route markers, and 5 objective markers.
- `python tools\validate_scene.py`: passed.
- `python tools\scale_audit.py`: passed; no suspicious scale issues found.
- `python tools\mesh_report.py`: passed; `unit-box-mesh` still has 10 uses.
- `python tools\status_report.py`: completed and showed the expected v0.17 modified files before cleanup/commit.
- `EngineApp.exe --renderer gdi --frames 360`: passed; default playtest overlay showed objective, prompt, job state, vehicle/checkpoint hints, service-gate/power status, and `F1` help.
- `EngineApp.exe --renderer dx11 --frames 360`: passed; hardware DX11 device failed and WARP was used, then the bounded run completed cleanly.
- `EngineApp.exe --renderer gdi --scene data\scenes\ferry_office.scene.json --frames 360`: passed; explicit scene path loaded and completed cleanly.
- `EngineApp.exe --renderer gdi --ui-mode playtest --frames 360`: passed; player-facing overlay was shown.
- `EngineApp.exe --renderer gdi --ui-mode debug --frames 360`: passed; full raw telemetry was shown.
- `EngineApp.exe --renderer gdi --ui-mode minimal --frames 360`: passed; compact objective/prompt/job overlay was shown.

Manual / visual notes:

- Bounded GDI runs confirmed the default playtest overlay is much less noisy than the v0.16 debug wall.
- Debug mode remains available and still exposes player/camera, traversal, vehicle, scene, world-state, physics, and job telemetry.
- Minimal mode works as a small objective/prompt/job readout.
- A new screenshot was not captured during this goal; the useful evidence for v0.17 was text/overlay behavior from bounded GDI runs rather than scene composition.

Known remaining issues:

- DX11 still falls back to WARP in this environment and still does not render text overlays.
- Presentation remains debug-text based, not a real HUD/UI framework.
- A full human-controlled v0.17 job playthrough should still be done on the target laptop before adding Job #2.
- Job #2 remains intentionally deferred.

Recommended next goal:

Build v0.18 Island Service Road Visual Identity / First Real Prop Style Spike.

## v0.18 Baseline - 2026-05-15

Goal: strengthen the Ferry Office / service yard / dock road visual identity with original mesh-backed props and a small readability pass, while preserving the existing Ferry Office Service Call, playtest/debug UI modes, scene loading, static mesh validation, default validation, and Jolt opt-in validation. No Job #2 or new gameplay system should be added.

Required context read before implementation:

- `AGENTS.md`
- `docs/STATUS.md`
- `docs/GAME_DIRECTION.md`
- `docs/VERTICAL_SLICE.md`
- `docs/ROADMAP.md`
- `docs/ARCHITECTURE.md`
- `docs/TECH_DEBT.md`
- `docs/SCENE_AUTHORING.md`
- `docs/ASSET_GUIDE.md`
- `docs/ART_DIRECTION.md`
- `docs/MESH_RENDERING.md`
- `docs/MANUAL_TEST_CHECKLIST.md`
- `docs/DECISIONS.md`

Baseline commands:

```powershell
git status --short --branch
powershell -ExecutionPolicy Bypass -File scripts\verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
python tools\scene_report.py
python tools\validate_scene.py
python tools\scale_audit.py
python tools\mesh_report.py
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --ui-mode playtest --frames 240
```

Baseline results:

- Git state before v0.18 work: `main...origin/main`, clean working tree.
- `scripts\verify.ps1`: passed. Default doctor/configure/build/CTest, scene validation, mesh report, and null smoke all completed. Null smoke reported engine `v0.17.0`, loaded scene `ferry-office`, `unit-box-mesh`, and the playtest overlay text.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 3/3 tests.
- `python tools\scene_report.py`: passed; scene reports 9 colliders, 21 visual placeholders, 1 mesh asset, 10 mesh instances, 6 interactables, 1 traversal affordance, 1 vehicle, 6 route markers, and 5 objective markers.
- `python tools\validate_scene.py`: passed.
- `python tools\scale_audit.py`: passed; no suspicious scale issues found.
- `python tools\mesh_report.py`: passed; `unit-box-mesh` has 10 scene uses.
- `EngineApp.exe --renderer gdi --ui-mode playtest --frames 240`: passed; bounded review showed the cleaner playtest overlay and current Ferry Office objective/prompt text.

Presentation / bug review notes:

- The scene now has a usable playtest overlay, but the environment still reads mostly as flat debug volumes and unit-box placeholders.
- The Ferry Office front/sign cue exists, but it is still a scaled box rather than a recognizable prop language.
- Dock/service-road mood needs stronger small-object silhouettes: road posts, signs, barriers, and utility/service props.
- Current mesh tooling and scene data are healthy, but adding new mesh asset ids requires runtime support beyond the current effective single-asset `unit-box-mesh` drawing path.
- DX11 text remains absent and hardware DX11 still falls back to WARP; visual overlay review should remain GDI-first.

Read-only subagent review notes:

- Mesh/render review found that scene data can describe multiple mesh assets, but runtime rendering currently loads/draws only `unit-box-mesh`. v0.18 should either reuse `unit_box.gltf` only or add a small `assetId -> StaticMeshAsset` map in `SandboxLayer`. The latter is the useful narrow integration step.
- Scene/tooling review recommended explicit v0.18 prop-id tests, validation for mesh instance replacement/linkage, avoiding brittle exact-count assertions as the main oracle, and documenting the v0.18 prop set in scene-authoring docs.

## v0.18 Short Implementation Plan

1. Add failing tests first:
   - C++ static mesh loading for at least one new committed original mesh.
   - C++ `SandboxLayer`/debug text or scene-runtime checks proving multiple mesh asset ids are accepted rather than only `unit-box-mesh`.
   - Python scene-tool tests for required v0.18 prop ids, new mesh asset ids, no duplicate mesh replacement links, and route/marker preservation.
2. Add a tiny original prop kit that stays inside the current glTF subset: embedded-buffer `.gltf`, one primitive, `POSITION` float `VEC3`, indexed triangles, no materials/textures/animation.
3. Keep runtime integration narrow by adding an asset map/cache inside `SandboxLayer` for loaded scene mesh assets. Do not add an asset registry, material system, resource cache, or loader expansion.
4. Update `data/scenes/ferry_office.scene.json` with new mesh assets/instances for a focused visual identity pass: service road sign, road edge posts, service barrier/rail cue, utility/service box, and dock/shore prop cues.
5. Improve the service-road/dock read by tuning prop placement, colors, and marker clarity while keeping playtest mode usable and debug mode complete.
6. Update scene tools/tests/docs so Codex can inspect and validate the new prop set.
7. Run the full default/Jolt/tool/windowed validation matrix, record exact results, then commit and push only after successful validation.

## v0.18 Implementation Notes

TDD / focused checks:

- Added C++ tests for the v0.18 prop-kit mesh files, scene-authored v0.18 mesh ids, and `SandboxLayer` rendering of every loaded scene mesh instance including non-`unit-box-mesh` assets.
- Added Python scene-tool tests for required v0.18 prop ids and duplicate `replacesVisualPlaceholderId` detection.
- Red checks before implementation:
  - `build\windows-vs2022-debug\Debug\EngineCoreTests.exe`: failed as expected because the new prop meshes and scene ids did not exist and non-unit-box scene assets were not rendered.
  - `python tests\test_scene_tools.py`: failed as expected because the v0.18 prop ids did not exist and duplicate mesh replacement links were not reported.
- Green checks after implementation:
  - `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`: passed.
  - `build\windows-vs2022-debug\Debug\EngineCoreTests.exe`: passed.
  - `python tests\test_scene_tools.py`: passed, 18 tests.

Code/data changes:

- Added four original tiny `.gltf` placeholder prop meshes:
  - `assets/models/service_road_sign.gltf`
  - `assets/models/road_edge_post.gltf`
  - `assets/models/service_barrier.gltf`
  - `assets/models/utility_box.gltf`
- Updated `data/scenes/ferry_office.scene.json` from 1 mesh asset / 10 mesh instances to 5 mesh assets / 15 mesh instances.
- Added scene-authored v0.18 prop instances for a service-road sign, road-edge posts, service-yard barrier cue, and maintenance utility box.
- Changed `SandboxLayer` static mesh loading from a single `unit-box-mesh` member to a local `assetId -> StaticMeshAsset` map, so scene data can submit multiple mesh asset ids without exposing a real asset registry yet.
- Extended scene validation for known color keys, `.gltf` mesh-asset format discipline, mesh instance color keys, and duplicate `replacesVisualPlaceholderId` links.
- Updated docs for v0.18 prop style, current mesh counts, limitations, and the recommended next asset-workflow stabilization step.

Tool observations after implementation:

- `python tools\scene_report.py`: passed; scene reports 9 colliders, 21 visual placeholders, 5 mesh assets, 15 mesh instances, 6 interactables, 1 traversal affordance, 1 vehicle, 6 route markers, and 5 objective markers.
- `python tools\validate_scene.py`: passed.
- `python tools\scale_audit.py`: passed; no suspicious scale issues found.
- `python tools\mesh_report.py`: passed; the new prop meshes load with small vertex/index counts and project-original provenance.

Known limitations:

- The new prop-kit meshes are original and more readable, but still flat-tinted placeholders, not commercial art.
- The static mesh path is still a tiny `.gltf` subset with no materials, textures, depth-aware renderer path, asset registry, resource cache, importer, GLB support, or collision import.
- DX11 still has no text overlay and should remain secondary to GDI for playtest presentation review.

## v0.18 Final Validation - 2026-05-15

Commands run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\doctor.ps1
powershell -ExecutionPolicy Bypass -File scripts\configure.ps1
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
ctest --preset windows-vs2022-debug --output-on-failure
powershell -ExecutionPolicy Bypass -File scripts\verify.ps1
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
python tools\scene_report.py
python tools\validate_scene.py
python tools\scale_audit.py
python tools\mesh_report.py
python tools\status_report.py
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --ui-mode playtest --frames 360
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --ui-mode debug --frames 360
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 360
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --scene data\scenes\ferry_office.scene.json --ui-mode playtest --frames 360
```

Results:

- `scripts\doctor.ps1`: passed; expected PATH warnings remain for `cl`, `clang++`, `g++`, `msbuild`, `ninja`, and `vcpkg`.
- `scripts\configure.ps1`: passed for `windows-vs2022-debug`.
- `scripts\build.ps1`: passed.
- `ctest --preset windows-vs2022-debug --output-on-failure`: passed, 3/3 tests.
- `scripts\verify.ps1`: passed; null smoke reported engine `v0.18.0`, loaded scene `ferry-office`, and all five static mesh assets.
- `cmake --preset windows-vs2022-debug-jolt`: passed.
- `cmake --build --preset windows-vs2022-debug-jolt`: passed.
- `ctest --preset windows-vs2022-debug-jolt --output-on-failure`: passed, 3/3 tests.
- `python tools\scene_report.py`: passed; scene reports 9 colliders, 21 visual placeholders, 5 mesh assets, 15 mesh instances, 6 interactables, 1 traversal affordance, 1 vehicle, 6 route markers, and 5 objective markers.
- `python tools\validate_scene.py`: passed.
- `python tools\scale_audit.py`: passed; no suspicious scale issues found.
- `python tools\mesh_report.py`: passed; `unit-box-mesh` has 10 uses, `road-edge-post-mesh` has 2 uses, and the three remaining v0.18 prop meshes each have 1 use.
- `python tools\status_report.py`: completed and showed the expected v0.18 modified/new files before commit.
- `EngineApp.exe --renderer gdi --ui-mode playtest --frames 360`: passed; playtest overlay stayed concise and the runtime loaded all five mesh assets.
- `EngineApp.exe --renderer gdi --ui-mode debug --frames 360`: passed; full telemetry stayed available.
- `EngineApp.exe --renderer dx11 --frames 360`: passed; hardware DX11 device failed and WARP was used, then the bounded run completed cleanly.
- `EngineApp.exe --renderer gdi --scene data\scenes\ferry_office.scene.json --ui-mode playtest --frames 360`: passed with the explicit scene path.

Visual evidence:

- Captured `docs/images/v0.18-gdi-screenshot.png` using the GDI playtest mode. The screenshot shows the existing Ferry Office Service Call overlay plus the stronger dock/service-road prop silhouettes.

Known remaining issues:

- DX11 still falls back to WARP in this environment and still does not render text overlays.
- The v0.18 prop kit improves place identity, but it is still placeholder presentation, not commercial-quality art.
- The next visual/asset risk is workflow stability: the project needs a decision on whether to keep the tiny custom `.gltf` subset briefly or move to cgltf/tinygltf and a more deliberate static mesh workflow.

Recommended next goal:

Build v0.19 Tiny Asset Pipeline Decision + Static Mesh Workflow Stabilization.
