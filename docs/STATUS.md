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
