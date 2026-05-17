# Target-Slice Live Objective Acquisition Gate Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Prove the Veyra target-slice objective can be acquired and completed through a recorded live-like player input path rather than a direct `PrototypeScene::applyInteractionResult` shortcut.

**Architecture:** Add a small QA-only target-slice playthrough module beside the Ferry Office QA modules. It loads `veyra_reach_pilot.scene.json`, runs `PlayerController` along the authored route toward the existing marker, verifies `InteractionSystem` focus/prompt, sends `interactPressed`, then records objective completion evidence. `EngineApp --qa-playthrough veyra-target-objective-acquisition` exposes the gate and a Python wrapper validates its JSON report.

**Tech Stack:** C++20, existing `GamePrototype`, `EngineCoreTests`, Python QA wrapper/tests, PowerShell validation scripts.

---

## Files

- Create: `src/game/TargetSliceObjectiveQa.h`
- Create: `src/game/TargetSliceObjectiveQa.cpp`
- Create: `tools/target_slice_objective_qa.py`
- Create: `tests/test_target_slice_objective_qa.py`
- Modify: `src/engine/core/Config.cpp`
- Modify: `src/game/main.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/EngineCoreTests.cpp`
- Modify: `docs/STATUS.md`
- Modify: `docs/SCENE_AUTHORING.md`
- Modify: `docs/WORLD_SLICE_AUTHORING.md`

## Tasks

- [x] Baseline: run `git status --short --branch`, `git branch --show-current`, `python tools/status_report.py`, `scripts/doctor.ps1`, `scripts/configure.ps1`, and `scripts/build.ps1`.
- [x] Add failing config/C++ tests for the new QA playthrough scenario and report shape.
- [x] Add failing Python wrapper tests that require focus/prompt/interact/completion evidence.
- [x] Implement `TargetSliceObjectiveQa` using real `PlayerController`, `InteractionSystem::updateFocus`, and `InteractionSystem::interact`.
- [x] Wire `EngineApp --qa-playthrough veyra-target-objective-acquisition`.
- [x] Add the Python wrapper and CTest entry.
- [x] Update docs/status with evidence and remaining limits.
- [x] Run focused validation and full `scripts/verify.ps1`.
- [x] Commit/push only if validation passes and worktree is scoped.

## Acceptance

- The report proves `inputScriptName=recorded-veyra-target-objective-v1`.
- The report includes `framesToFocus`, `framesToInteract`, focus name/prompt, interaction message, final player position, objective id, completion summary, completion event, and forbidden-term validation.
- The gate fails if focus never appears, the wrong prompt appears, interact does not trigger, or the target objective stays incomplete.
- No Ferry Office job/world-state wording is required or emitted as proof for Veyra.
- No terrain, asset pass, mission framework, vehicle/Jolt work, or renderer rewrite is added.
