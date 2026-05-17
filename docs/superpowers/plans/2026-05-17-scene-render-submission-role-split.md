# Scene Render Submission Role Split Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Extract the next small scene-role execution seam from `SandboxLayer`: render-submission role decisions for authored scene geometry versus Ferry Office fallback/regression debug cues.

**Architecture:** Keep renderer APIs unchanged. Add a tiny `SceneRenderSurface` module that answers what the current scene role is allowed to submit: fallback Ferry Office mood base, scene-authored visual placeholders, scene-authored meshes, collider debug, world-state debug cues, slice debug markers, and vehicle/player debug surfaces. `SandboxLayer` remains the renderer orchestrator and draw-code owner for now.

**Tech Stack:** C++20, existing `GamePrototype`, `EngineCoreTests`, PowerShell validation scripts.

---

## Goal Packet

```text
/goal Scene render submission role split audit dla Tidebreak w C:\Users\Marcin\Documents\New project.

Cel:
Po `SceneGuidanceSurface` nie isc w content ani terrain. Zrobic kolejny najmniejszy split wykonania roli sceny: oddzielic scene-authored render submission od Ferry Office fallback/debug mood base w `SandboxLayer`, tak zeby neutralne target-slice sceny mialy testowalny render-submission seam bez dziedziczenia Ferry Office base geometry.

Dlaczego:
Mamy juz policy, neutral text surface i guidance visibility seam. `SandboxLayer` nadal miesza fallback Ferry Office mood base, scene-authored placeholders, mesh submission, world-state debug cues, player/vehicle draw i debug/playtest overlays. Przed prawdziwym Veyra/world work trzeba wyciagnac nastepny konkretny seam, ale tylko jeden: render submission role boundary, nie renderer rewrite.

Zakres:
- Dodaj `src/game/SceneRenderSurface.h/.cpp`.
- Przenies tylko decyzje, czy rysowac scene-authored placeholders/meshes versus Ferry Office fallback mood base/world-state cues/collider debug.
- Zachowaj samo rysowanie w `SandboxLayer` i istniejacym `IRenderer`.
- Dodaj testy dla fallback Ferry Office, loaded Ferry Office regression scene i Veyra target-slice scaffold.
- Zaktualizuj `docs/STATUS.md`.

Non-goals:
- Nie dodawaj mapy, terenu, road systemu, assetow, misji, NPC, edytora, HUD polishu ani renderer rewrite.
- Nie przepisuj calego `SandboxLayer`.
- Nie ruszaj Jolt/vehicle runtime.

Walidacja:
- RED/GREEN `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`
- `build/windows-vs2022-debug/Debug/EngineCoreTests.exe`
- `python tests/test_runtime_scene_smoke.py`
- Veyra playtest/debug runtime smoke
- `scripts/verify.ps1`

Stop:
Zakoncz dopiero po aktualizacji statusu, pelnej walidacji, clean diff review oraz commit/push, jesli worktree zawiera tylko zmiany tego goala.
```

## Files

- Create: `src/game/SceneRenderSurface.h`
- Create: `src/game/SceneRenderSurface.cpp`
- Modify: `src/game/SandboxLayer.h`
- Modify: `src/game/SandboxLayer.cpp`
- Modify: `tests/EngineCoreTests.cpp`
- Modify: `CMakeLists.txt`
- Modify: `docs/STATUS.md`

## Tasks

### Task 1: Red Test For Render Surface Policy

- [x] Add `#include "game/SceneRenderSurface.h"` to `tests/EngineCoreTests.cpp`.
- [x] Add direct tests proving:
  - fallback/unloaded scene uses Ferry Office fallback mood base and allows world-state debug cues,
  - loaded Ferry Office regression scene uses authored scene geometry and still allows Ferry Office world-state debug cues,
  - loaded Veyra target-slice scaffold uses authored scene geometry but not Ferry Office fallback mood base or world-state cues.
- [x] Register the tests in `main()`.
- [x] Run `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`.
- [x] Expected RED: build fails because `game/SceneRenderSurface.h` does not exist.

### Task 2: Minimal Render Surface Module

- [x] Create `src/game/SceneRenderSurface.h` with `SceneRenderSubmissionPlan` and `BuildSceneRenderSubmissionPlan(...)`.
- [x] Create `src/game/SceneRenderSurface.cpp` that derives the plan from `sceneDefinitionLoaded` and `SceneRuntimePolicy`.
- [x] Add `src/game/SceneRenderSurface.cpp` to `GamePrototype` in `CMakeLists.txt`.
- [x] Run `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`.
- [x] Expected GREEN: new direct render-surface tests compile and pass.

### Task 3: Wire SandboxLayer To The New Seam

- [x] Include `SceneRenderSurface.h` in `SandboxLayer`.
- [x] Add `SceneRenderSubmissionPlan buildSceneRenderSubmissionPlan() const`.
- [x] Replace direct render-role checks in `onRender`:
  - `m_sceneDefinitionLoaded` scene geometry branch,
  - fallback Ferry Office mood base branch,
  - full-debug collider debug,
  - world-state debug cue gate.
- [x] Keep existing draw methods and renderer calls unchanged.
- [x] Run `build/windows-vs2022-debug/Debug/EngineCoreTests.exe`.
- [x] Expected GREEN: existing runtime, capture, Ferry Office, and Veyra tests pass.

### Task 4: Focused Runtime Validation

- [x] Run `python tests/test_runtime_scene_smoke.py`.
- [x] Run `cmake --build --preset windows-vs2022-debug --target EngineCoreTests EngineApp`.
- [x] Run Veyra playtest smoke:
  `python tools/runtime_scene_smoke.py --exe build/windows-vs2022-debug/Debug/EngineApp.exe --scene data/scenes/veyra_reach_pilot.scene.json --renderer gdi --ui-mode playtest --capture-frame build/captures/veyra-reach-pilot-runtime-smoke.bmp --report-json build/runtime/veyra-reach-pilot-smoke-report.json`
- [x] Run Veyra debug smoke:
  `python tools/runtime_scene_smoke.py --exe build/windows-vs2022-debug/Debug/EngineApp.exe --scene data/scenes/veyra_reach_pilot.scene.json --ui-mode debug --report-json build/runtime/veyra-reach-pilot-debug-smoke-report.json`
- [x] Run `scripts/verify.ps1`.

### Task 5: Docs And Publish

- [x] Update `docs/STATUS.md` with scope, evidence, validation, remaining limits, and next-goal prompt.
- [x] Remove transient `tools/__pycache__` / `tests/__pycache__` if created.
- [x] Run `git diff --check` and review `git status --short --branch`.
- [ ] If validation passed and changes are goal-related only, commit and push to `main`.
