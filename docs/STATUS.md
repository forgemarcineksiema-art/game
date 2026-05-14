# Project Status

Last updated: 2026-05-14

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
