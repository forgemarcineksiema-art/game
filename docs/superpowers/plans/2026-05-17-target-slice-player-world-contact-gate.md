# Target-Slice Player-World Contact Gate Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Extend the Veyra target-slice QA so recorded live-like input proves player contact with the authored road-edge collider, recovery/control after that contact, and final target objective completion.

**Architecture:** Keep this as a QA/evidence gate, not new content. Extend `TargetSliceObjectiveQa` to run a deterministic contact probe against the existing `pilot-road-edge-collider` before navigating to `Pilot Service Marker`; report collider name, frame, hit count, push/normal, recovery frame, focus/prompt, interact, and final target objective completion. Add the smallest collision telemetry seam needed for `PlayerController` to expose which authored collider resolved the contact.

**Tech Stack:** C++20, existing `GamePrototype`, `PrototypeWorld`, `PlayerController`, `TargetSliceObjectiveQa`, Python QA wrapper/tests, CMake/CTest, `scripts/verify.ps1`.

---

## Files

- Modify: `src/game/PrototypeWorld.h`
- Modify: `src/game/PrototypeWorld.cpp`
- Modify: `src/game/PlayerController.h`
- Modify: `src/game/PlayerController.cpp`
- Modify: `src/game/TargetSliceObjectiveQa.h`
- Modify: `src/game/TargetSliceObjectiveQa.cpp`
- Modify: `tools/target_slice_objective_qa.py`
- Modify: `tests/test_target_slice_objective_qa.py`
- Modify: `tests/EngineCoreTests.cpp`
- Modify: `docs/STATUS.md`
- Modify: `docs/WORLD_SLICE_AUTHORING.md`
- Modify: `docs/superpowers/plans/2026-05-17-target-slice-player-world-contact-gate.md`

## Tasks

- [x] Baseline: run `git status --short --branch`, `git branch --show-current`, `python tools/status_report.py`, `scripts/doctor.ps1`, `scripts/configure.ps1`, and `scripts/build.ps1`.
- [x] Add failing C++ assertions that `RunTargetSliceObjectiveAcquisitionQa` reports `contact.attempted`, `contact.hit`, `colliderName=pilot-road-edge-collider`, non-zero push/normal, `framesToContact`, `framesToRecovery`, and final objective completion after recovery.
- [x] Add failing Python wrapper tests that reject a report with no `contact` block and reject a report where contact happens but recovery is false.
- [x] Add minimal collision telemetry: `CollisionResult::lastColliderName` and `PlayerState::lastCollisionColliderName`.
- [x] Extend `TargetSliceObjectiveQa` recorded route: start in Veyra, drive the player into `pilot-road-edge-collider`, record collision evidence, steer away/recover, then acquire focus and interact with `Pilot Service Marker`.
- [x] Extend JSON report and Python validator with the `contact` block.
- [x] Update docs/status with evidence and remaining limits.
- [x] Run focused tests, `scripts/verify.ps1`, then commit/push only if the worktree is scoped and validation passes.

## Acceptance

- `tools/target_slice_objective_qa.py` rejects reports that only prove marker acquisition without contact evidence.
- Fresh report names `pilot-road-edge-collider` and includes `framesToContact`, `framesToRecovery`, `hitCount`, `push`, `normal`, contact position, and recovery evidence.
- The same run still proves focus/prompt/interact and `targetObjective=inspect-pilot-service-marker`.
- The report and debug evidence do not use Ferry Office job/world-state wording.
- No terrain, asset pass, mission, NPC, vehicle, Jolt, renderer, or broad `SandboxLayer` work is added.
