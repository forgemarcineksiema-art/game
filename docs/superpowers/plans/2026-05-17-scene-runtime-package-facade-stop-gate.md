# Scene Runtime Package Facade Stop Gate Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add the smallest scene-runtime facade that packages the existing scene role decisions and then records whether further `SandboxLayer` extraction should stop before target-system work.

**Architecture:** Keep existing scene policy, neutral text, guidance, and render-surface modules. Add `SceneRuntimePackage` as the single runtime decision package built at scene load time; it owns `SceneRuntimePolicy` plus derived `SceneRenderSubmissionPlan` so `SandboxLayer` no longer rebuilds role/render decisions through separate ad hoc calls. Do not move drawing, input, camera, vehicles, scene loading, or mission/job behavior.

**Tech Stack:** C++20, existing `GamePrototype`, `EngineCoreTests`, PowerShell validation scripts.

---

## Goal Packet

```text
/goal Scene runtime package facade + stop/continue gate dla Tidebreak w C:\Users\Marcin\Documents\New project.

Cel:
Po policy/text/guidance/render-submission splitach nie isc automatycznie w kolejny mikrosplit. Zrobic jeden porzadkujacy facade/contract dla scene runtime decisions, ktory zbiera role policy i render-submission plan za mala powierzchnia uzywana przez `SandboxLayer`, a potem zapisac decyzje stop/continue: czy dalsze wydzielanie z `SandboxLayer` ma jeszcze sens przed pierwszym prawdziwym target-system goalem.

Dlaczego:
Ostatnie goal'e rozdzielily kilka konkretnych decyzji, ale `SandboxLayer` nadal trzyma osobne entry pointy dla policy i render planu. To grozi warstwa rozproszonych helperow zamiast realnej powierzchni runtime dla Veyra/przyszlych slice'ow. Nastepny krok ma sprawdzic, czy te seamy skladaja sie w jeden maly kontrakt, i zatrzymac architektoniczne dlubanie, jesli nie ma kolejnego dowodu.

Zakres:
- Dodaj `src/game/SceneRuntimePackage.h/.cpp`.
- `SceneRuntimePackage` ma zawierac `SceneRuntimePolicy` i `SceneRenderSubmissionPlan`.
- `SandboxLayer` ma przechowywac jedna paczke decyzji zamiast osobnego `m_runtimePolicy` plus lokalnego `buildSceneRenderSubmissionPlan()`.
- Zachowaj `SceneRuntimeSurface`, `SceneGuidanceSurface` i `SceneRenderSurface` jako osobne testowalne moduły; facade ma je komponowac, nie wchłaniac.
- Dodaj testy dla fallback/no-loaded-scene, Ferry Office regression-testbed, Veyra target-slice scaffold i generic loaded neutral scene.
- Zapisz w `docs/STATUS.md` decyzje stop/continue dla dalszego `SandboxLayer` extraction.

Non-goals:
- Nie dodawaj contentu, terenu, road systemu, asset passu, misji, NPC, edytora, HUD polishu, vehicle/Jolt work ani renderer rewrite.
- Nie przepisuj calego `SandboxLayer`.
- Nie przenos rysowania, asset loadingu, inputu, kamery, pojazdow ani job logic.
- Nie rob kolejnego splitu po tym bez jawnej decyzji stop/continue.

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

- Create: `src/game/SceneRuntimePackage.h`
- Create: `src/game/SceneRuntimePackage.cpp`
- Modify: `src/game/SandboxLayer.h`
- Modify: `src/game/SandboxLayer.cpp`
- Modify: `tests/EngineCoreTests.cpp`
- Modify: `CMakeLists.txt`
- Modify: `docs/STATUS.md`

## Tasks

### Task 1: Red Test For Scene Runtime Package

- [x] Add `#include "game/SceneRuntimePackage.h"` to `tests/EngineCoreTests.cpp`.
- [x] Add tests proving `BuildSceneRuntimePackage(...)`:
  - fallback/no-loaded-scene keeps Ferry Office behavior and fallback render submission,
  - loaded Ferry Office keeps Ferry Office behavior but uses authored scene render submission,
  - Veyra target-slice uses neutral behavior and authored render submission without Ferry Office world-state cues,
  - generic loaded non-regression scenes use neutral behavior and authored render submission.
- [x] Register tests in `main()`.
- [x] Run `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`.
- [x] Expected RED: build fails because `game/SceneRuntimePackage.h` does not exist.

### Task 2: Minimal Package Facade

- [x] Create `src/game/SceneRuntimePackage.h` with:
  - `SceneRuntimePackage { SceneRuntimePolicy policy; SceneRenderSubmissionPlan renderSubmission; }`
  - `BuildSceneRuntimePackage(bool sceneDefinitionLoaded, const SceneDefinition& scene)`.
- [x] Create `src/game/SceneRuntimePackage.cpp` that calls `BuildSceneRuntimePolicy(...)` once and derives `BuildSceneRenderSubmissionPlan(...)` from the same policy.
- [x] Add `src/game/SceneRuntimePackage.cpp` to `GamePrototype` in `CMakeLists.txt`.
- [x] Run `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`.
- [x] Expected GREEN: direct package tests compile and pass.

### Task 3: Wire SandboxLayer To The Package

- [x] Replace `SceneRuntimePolicy m_runtimePolicy` with `SceneRuntimePackage m_sceneRuntime`.
- [x] Include `SceneRuntimePackage.h` in `SandboxLayer.h`.
- [x] Remove `SandboxLayer::buildSceneRenderSubmissionPlan()`.
- [x] Build `m_sceneRuntime` in `loadSceneDefinition()` for fallback and loaded scenes.
- [x] Replace `m_runtimePolicy.*` reads with `m_sceneRuntime.policy.*`.
- [x] In `onRender`, use `m_sceneRuntime.renderSubmission`.
- [x] Keep `buildNeutralSceneRuntimeView()` and `buildSceneGuidanceContext()` as `SandboxLayer` data collectors for now.
- [x] Run `build/windows-vs2022-debug/Debug/EngineCoreTests.exe`.
- [x] Expected GREEN.

### Task 4: Focused Runtime Validation

- [x] Run `python tests/test_runtime_scene_smoke.py`.
- [x] Run `cmake --build --preset windows-vs2022-debug --target EngineCoreTests EngineApp`.
- [x] Run Veyra playtest smoke:
  `python tools/runtime_scene_smoke.py --exe build/windows-vs2022-debug/Debug/EngineApp.exe --scene data/scenes/veyra_reach_pilot.scene.json --renderer gdi --ui-mode playtest --capture-frame build/captures/veyra-reach-pilot-runtime-smoke.bmp --report-json build/runtime/veyra-reach-pilot-smoke-report.json`
- [x] Run Veyra debug smoke:
  `python tools/runtime_scene_smoke.py --exe build/windows-vs2022-debug/Debug/EngineApp.exe --scene data/scenes/veyra_reach_pilot.scene.json --ui-mode debug --report-json build/runtime/veyra-reach-pilot-debug-smoke-report.json`
- [x] Run `scripts/verify.ps1`.

### Task 5: Status, Stop/Continue Decision, Publish

- [x] Update `docs/STATUS.md` with scope, evidence, validation, remaining limits, and a clear stop/continue decision.
- [x] The stop/continue decision should say: stop further micro-extraction unless a specific player-facing or target-system goal is blocked by `SandboxLayer`.
- [x] Remove transient `tools/__pycache__` / `tests/__pycache__` if created.
- [x] Run `scripts/verify.ps1` after final docs edits.
- [x] Run `git diff --check` and review `git status --short --branch`.
- [ ] If validation passed and changes are goal-related only, commit and push to `main`.
