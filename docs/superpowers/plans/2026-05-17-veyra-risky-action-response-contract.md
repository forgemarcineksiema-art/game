# Veyra Risky Action Response Contract Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Prove the first tiny Tidebreak crime/action contract in Veyra: a risky player action produces a distinct local response and exit/recovery evidence.

**Architecture:** Extend the existing target-slice objective surface instead of adding a mission framework. Scene data authors a small `targetActionResponse` contract; `TargetSliceObjectiveRuntime`/`PrototypeScene` track it separately from objective completion; `TargetSliceObjectiveQa` and `tools/target_slice_objective_qa.py` report and validate action/response evidence.

**Tech Stack:** C++20 game runtime, nlohmann/json scene loading, existing `EngineCoreTests`, Python unittest QA wrapper, existing CMake/PowerShell validation scripts.

---

### Task 1: Scene Contract And Runtime State

**Files:**
- Modify: `src/game/SceneDefinition.h`
- Modify: `src/game/SceneLoader.cpp`
- Modify: `src/game/TargetSliceObjectiveRuntime.h`
- Modify: `src/game/TargetSliceObjectiveRuntime.cpp`
- Modify: `src/game/PrototypeScene.h`
- Modify: `src/game/PrototypeScene.cpp`
- Modify: `data/scenes/veyra_reach_pilot.scene.json`
- Test: `tests/EngineCoreTests.cpp`

- [ ] **Step 1: Add failing C++ tests**

Add tests that expect Veyra scene data to expose `targetActionResponse`, expect runtime state to be incomplete before the risky interactable is triggered, and expect `PrototypeScene::completionSummary()` to include `riskyAction=...`, `responseState=...`, and `exitRecovery=...` after the action.

- [ ] **Step 2: Run RED**

Run: `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`

Expected: fails because target action/response fields and runtime helpers do not exist.

- [ ] **Step 3: Implement minimal scene/runtime fields**

Add `SceneTargetActionResponseDefinition` with id, risky interactable name, response state id/text, exit marker name, summary, and event text. Parse it from `targetActionResponse` in `SceneLoader.cpp`. Extend target-slice runtime state and `PrototypeScene` so risky action and exit/recovery are tracked separately from `targetObjective`.

- [ ] **Step 4: Run GREEN for C++**

Run: `cmake --build --preset windows-vs2022-debug --target EngineCoreTests && build/windows-vs2022-debug/Debug/EngineCoreTests.exe`

Expected: build succeeds and the new tests pass.

### Task 2: QA Report And Python Validation

**Files:**
- Modify: `src/game/TargetSliceObjectiveQa.h`
- Modify: `src/game/TargetSliceObjectiveQa.cpp`
- Modify: `tools/target_slice_objective_qa.py`
- Modify: `tests/test_target_slice_objective_qa.py`
- Modify: `src/engine/core/Config.cpp`
- Modify: `src/game/main.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Add failing Python validation tests**

Extend sample reports so `load_and_validate_report()` requires `riskyAction`, `localResponse`, and `exitRecovery`. Add rejection tests for reports that complete the old objective but lack action/response evidence.

- [ ] **Step 2: Run RED**

Run: `python tests/test_target_slice_objective_qa.py`

Expected: fails because current reports are accepted without action/response blocks.

- [ ] **Step 3: Implement QA report fields and scenario**

Add scenario `veyra-risky-action-response` using the same Veyra scene. The recorded route should contact the road edge, trigger the risky action interactable, observe response state, reach/trigger an exit recovery marker, then complete the objective. The JSON report must distinguish `targetObjective`, `riskyAction`, `localResponse`, and `exitRecovery`.

- [ ] **Step 4: Run GREEN for Python and focused C++**

Run:

```powershell
python tests/test_target_slice_objective_qa.py
cmake --build --preset windows-vs2022-debug --target EngineCoreTests EngineApp
build/windows-vs2022-debug/Debug/EngineCoreTests.exe
python tools/target_slice_objective_qa.py --exe build/windows-vs2022-debug/Debug/EngineApp.exe --scene data/scenes/veyra_reach_pilot.scene.json --report-json build/playthroughs/veyra-risky-action-response-report.json
```

Expected: all pass; report contains action/response/exit evidence and no Ferry Office language.

### Task 3: Docs, Scene Tools, And Final Validation

**Files:**
- Modify: `docs/STATUS.md`
- Modify: `docs/WORLD_SLICE_AUTHORING.md`
- Modify: `docs/GAMEPLAY_REVIEW.md` only if the proved claim changes.
- Test/commands: scene tools and full verify.

- [ ] **Step 1: Update docs**

Record the goal, scope, non-goals, commands, report evidence, remaining limits, and next recommended goal.

- [ ] **Step 2: Run final validation**

Run:

```powershell
python tools/validate_scene.py data/scenes/veyra_reach_pilot.scene.json
python tools/world_slice_report.py
python tools/runtime_scene_smoke.py --exe build/windows-vs2022-debug/Debug/EngineApp.exe --scene data/scenes/veyra_reach_pilot.scene.json --ui-mode debug --report-json build/runtime/veyra-risky-action-response-smoke-report.json
scripts/verify.ps1
```

Expected: all pass.

- [ ] **Step 3: Commit and push**

If the worktree contains only this goal's changes and validation passed:

```powershell
git add <goal files>
git commit -m "feat: add Veyra risky action response gate"
git push
```
