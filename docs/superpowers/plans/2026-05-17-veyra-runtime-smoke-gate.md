# Veyra Runtime Smoke Gate Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Prove `veyra_reach_pilot.scene.json` can load through runtime as a neutral target-slice scaffold without Ferry Office job/presentation leakage.

**Architecture:** Keep Ferry Office as the regression scene and add only the smallest runtime role awareness needed for a non-Ferry target slice. Use `sliceMetadata.kind` from scene data to drive neutral presentation, disable inherited service-vehicle fallback for target-slice scaffolds, and add a repeatable smoke tool that fails on Ferry Office overlay terms.

**Tech Stack:** C++20 game/runtime layer, existing scene JSON loader, Python unittest/tooling, CMake/CTest, PowerShell verify wrapper.

---

### Task 1: Runtime Smoke Validator

**Files:**
- Create: `tests/test_runtime_scene_smoke.py`
- Create: `tools/runtime_scene_smoke.py`
- Modify: `CMakeLists.txt`

- [x] **Step 1: Write the failing Python test**

Add a test that imports `runtime_scene_smoke`, feeds it pilot-slice output containing `Ferry Office`, `Ferry Manifest`, and `Job:`, and expects validation failure.

- [x] **Step 2: Verify RED**

Run: `python tests\test_runtime_scene_smoke.py`

Expected: fail with `ModuleNotFoundError: No module named 'runtime_scene_smoke'`.

- [x] **Step 3: Add the validator**

Implement `validate_runtime_output()`, `run_runtime_smoke()`, optional `--capture-frame`, and optional `--report-json`.

- [x] **Step 4: Verify GREEN**

Run: `python tests\test_runtime_scene_smoke.py`

Expected: 3 tests pass.

### Task 2: Runtime Role Awareness

**Files:**
- Modify: `src/game/SceneDefinition.*`
- Modify: `src/game/SceneLoader.cpp`
- Modify: `src/game/PrototypeScene.*`
- Modify: `src/game/SandboxLayer.*`
- Modify: `tests/EngineCoreTests.cpp`

- [x] **Step 1: Add C++ regression coverage**

Add tests that load `veyra_reach_pilot.scene.json`, verify `sliceMetadata.kind=target-slice-scaffold`, and verify Sandbox playtest text contains neutral slice status with no `Ferry Office`, `Job:`, or service-gate text.

- [x] **Step 2: Implement minimal runtime support**

Parse `sliceMetadata`, expose helper predicates, suppress Ferry Office job behavior for target-slice scaffolds, and disable inherited fallback vehicle/presentation for the pilot slice.

- [x] **Step 3: Verify runtime smoke**

Run: `python tools\runtime_scene_smoke.py --exe build\windows-vs2022-debug\Debug\EngineApp.exe --scene data\scenes\veyra_reach_pilot.scene.json`

Expected: pass.

### Task 3: Standard Gate And Docs

**Files:**
- Modify: `scripts/doctor.ps1`
- Modify: `scripts/verify.ps1`
- Modify: `tools/status_report.py`
- Modify: `docs/AI_WORKFLOW.md`
- Modify: `docs/SCENE_AUTHORING.md`
- Modify: `docs/WORLD_SLICE_AUTHORING.md`
- Modify: `docs/CONTEXT_MAP.md`
- Modify: `docs/STATUS.md`

- [x] **Step 1: Wire the smoke gate into standard verification**

Add `runtime_scene_smoke.py` to doctor/status visibility and run it from `scripts\verify.ps1` with a GDI capture/report path.

- [x] **Step 2: Document the boundary**

Update scene/world-slice docs so future goals know the pilot is now runtime-loadable neutral evidence, not content.

- [x] **Step 3: Final verification**

Run: `scripts\verify.ps1`

Expected: CTest, scene/tools, runtime scene smoke, capture, and default smoke pass.
