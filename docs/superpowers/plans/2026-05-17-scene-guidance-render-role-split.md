# Scene Guidance Render Role Split Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Extract the next small scene-role execution seam from `SandboxLayer`: guidance visibility policy for route/objective/interactable/traversal/vehicle markers.

**Architecture:** Keep Ferry Office as the regression-testbed path and Veyra/future slices as neutral authored-scene paths. Add a small `SceneGuidanceSurface` module that receives an explicit context and returns marker visibility decisions, while `SandboxLayer` remains the orchestrator that gathers runtime state and draws geometry.

**Tech Stack:** C++20, existing `GamePrototype` static library, existing `EngineCoreTests`, PowerShell validation scripts.

---

## Goal Packet

```text
/goal Scene guidance/render-role split audit dla Tidebreak w C:\Users\Marcin\Documents\New project.

Cel:
Po `SceneRuntimePolicy` i `SceneRuntimeSurface` nie isc w content, terrain ani visual polish. Wydzielic pierwszy neutralny guidance/render-role seam z `SandboxLayer`: decyzje, ktore markery route/objective/interactable/traversal/vehicle wolno rysowac dla Ferry Office active guidance versus neutral all-authored target-slice guidance.

Dlaczego:
`SceneRuntimeSurface` wyciagnal neutralny tekst, ale visibility/guidance wykonanie nadal siedzi w `SandboxLayer` razem z Ferry Office job phase. Przed rozbudowa Veyra trzeba miec male, testowalne miejsce, gdzie scena mowi "jestem regression-testbedem" albo "jestem neutralnym target slice'em", bez kolejnych lokalnych if-ow w runtime.

Zakres:
- Dodac `src/game/SceneGuidanceSurface.h/.cpp`.
- Przeniesc z `SandboxLayer` decyzje `shouldDrawRouteMarker`, `shouldDrawObjectiveMarker`, `shouldDrawInteractableMarker`, `shouldDrawTraversalMarker`, `shouldDrawVehicleGuidance` do czystych funkcji opartych o jawny kontekst.
- `SandboxLayer` ma tylko budowac `SceneGuidanceContext` z aktualnego runtime state i delegowac decyzje.
- Dodac red/green testy dla neutral all-authored target-slice guidance i Ferry Office active guidance.
- Zaktualizowac `docs/STATUS.md`.

Non-goals:
- Nie dodawac mapy, terenu, road systemu, assetow, misji, NPC, edytora, HUD polishu ani renderer rewrite.
- Nie przepisywac calego `SandboxLayer`.
- Nie usuwac Ferry Office jako regression-testbedu.
- Nie zmieniac Jolt/vehicle runtime.

Walidacja:
- RED/GREEN `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`
- `build/windows-vs2022-debug/Debug/EngineCoreTests.exe`
- `python tests/test_runtime_scene_smoke.py`
- `cmake --build --preset windows-vs2022-debug --target EngineCoreTests EngineApp`
- Veyra debug runtime smoke
- `scripts/verify.ps1`

Stop:
Zakoncz dopiero po aktualizacji statusu, pelnej walidacji, clean diff review oraz commit/push, jesli worktree zawiera tylko zmiany tego goala.
```

## Files

- Create: `src/game/SceneGuidanceSurface.h`
- Create: `src/game/SceneGuidanceSurface.cpp`
- Modify: `src/game/SandboxLayer.h`
- Modify: `src/game/SandboxLayer.cpp`
- Modify: `tests/EngineCoreTests.cpp`
- Modify: `CMakeLists.txt`
- Modify: `docs/STATUS.md`

## Tasks

### Task 1: Red Test For Guidance Surface

- [x] Add `#include "game/SceneGuidanceSurface.h"` to `tests/EngineCoreTests.cpp`.
- [x] Add direct tests proving:
  - neutral `AllAuthored` guidance returns true for arbitrary Veyra route/objective/interactable/traversal markers without Ferry Office names,
  - Ferry Office active guidance hides future route markers before the job starts,
  - Ferry Office active guidance only shows the current route/objective when provided by context,
  - vehicle guidance is hidden when no vehicle is available.
- [x] Register the tests in `main()`.
- [x] Run `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`.
- [x] Expected RED: build fails because `game/SceneGuidanceSurface.h` does not exist.

### Task 2: Minimal Guidance Surface Module

- [x] Create `src/game/SceneGuidanceSurface.h` with:
  - `SceneGuidanceContext`,
  - `ShouldDrawRouteGuidanceMarker`,
  - `ShouldDrawObjectiveGuidanceMarker`,
  - `ShouldDrawInteractableGuidanceMarker`,
  - `ShouldDrawTraversalGuidanceMarker`,
  - `ShouldDrawVehicleGuidance`.
- [x] Create `src/game/SceneGuidanceSurface.cpp` and move the existing visibility decisions there.
- [x] Add the `.cpp` file to `GamePrototype` in `CMakeLists.txt`.
- [x] Run `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`.
- [x] Expected GREEN: new direct guidance tests compile and pass.

### Task 3: Wire SandboxLayer To The New Seam

- [x] Include `SceneGuidanceSurface.h` from `SandboxLayer.h` or `SandboxLayer.cpp`.
- [x] Replace five `SandboxLayer::shouldDraw*` methods with one `buildSceneGuidanceContext()` helper plus calls into `SceneGuidanceSurface`.
- [x] Keep draw methods in `SandboxLayer`; this goal extracts decisions, not rendering.
- [x] Run `build/windows-vs2022-debug/Debug/EngineCoreTests.exe`.
- [x] Expected GREEN: existing Ferry Office guidance tests and Veyra smoke-facing tests still pass.

### Task 4: Runtime Smoke And Full Validation

- [x] Run `python tests/test_runtime_scene_smoke.py`.
- [x] Run `cmake --build --preset windows-vs2022-debug --target EngineCoreTests EngineApp`.
- [x] Run `python tools/runtime_scene_smoke.py --exe build/windows-vs2022-debug/Debug/EngineApp.exe --scene data/scenes/veyra_reach_pilot.scene.json --ui-mode debug --report-json build/runtime/veyra-reach-pilot-debug-smoke-report.json`.
- [x] Run `scripts/verify.ps1`.

### Task 5: Docs And Publish

- [x] Update `docs/STATUS.md` with scope, red/green evidence, validation, remaining limits, and the next recommended goal.
- [x] Remove transient `tools/__pycache__` if validation created it.
- [x] Review `git diff --check` and `git status --short --branch`.
- [x] If validation passed and changes are only goal-related, commit and push to `main`.
