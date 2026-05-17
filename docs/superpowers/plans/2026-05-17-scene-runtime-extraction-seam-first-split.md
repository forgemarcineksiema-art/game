# Scene Runtime Extraction Seam First Split Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Extract the first neutral target-slice runtime surface from `SandboxLayer` so Veyra and future authored scenes no longer live only as "not Ferry Office" branches.

**Architecture:** Keep `SceneRuntimePolicy` as the role decision boundary. Add a small `SceneRuntimeSurface` module that owns neutral scene presentation/debug text from a plain view struct. `SandboxLayer` will still integrate live systems, but it will build a neutral view and delegate target-slice text execution to the new surface.

**Tech Stack:** C++20, existing `GamePrototype` CMake target, `tests/EngineCoreTests.cpp`, `tools/runtime_scene_smoke.py`, `scripts/verify.ps1`.

---

## Goal Prompt

```text
/goal Scene runtime extraction seam audit + first split dla Tidebreak w C:\Users\Marcin\Documents\New project.

Cel:
Po SceneRuntimePolicy nie dodawac contentu ani terrainu. Zrobic pierwszy maly, dowodowy split `SandboxLayer`: wydzielic neutral target-slice presentation/debug/guidance surface tak, zeby Veyra i przyszly slice nie dziedziczyly kolejnych ukrytych zaleznosci od Ferry Office.

Dlaczego:
SceneRuntimePolicy zamknela decyzje "jaka rola sceny?", ale nie rozdzielila wykonania tych rol. Najwieksze aktualne ryzyko systemowe to nadal jeden plik, ktory miesza scene loading, Ferry Office job chain, debug rendering, vehicle fallback, neutral slice presentation i guidance. Przed swiatem/contentem trzeba najpierw miec miejsce, do ktorego ten swiat moze wejsc bez udawania Ferry Office.

Zakres:
- Dodaj maly `SceneRuntimeSurface`/rownowazny modul w `src/game`.
- Przenies neutral target-slice presentation/debug text execution poza `SandboxLayer`.
- `SandboxLayer` moze dalej zbierac runtime state, ale nie powinien skladac neutralnego tekstu inline.
- Dodaj testy bezposrednio dla neutralnej powierzchni, ktore odrzucaja Ferry Office job/world-state/road wording.
- Zachowaj Ferry Office regression behavior, Veyra runtime smoke i standardowy verify.
- Zaktualizuj `docs/STATUS.md`.

Non-goals:
- Nie rob mapy, terenu, road systemu, asset passu, misji, NPC ani polishu.
- Nie przepisuj calego `SandboxLayer`.
- Nie usuwaj Ferry Office jako regression-testbedu.
- Nie tworz ECS, scene managera, mission frameworka ani edytora.

Walidacja:
- `python tests\test_runtime_scene_smoke.py`
- `cmake --build --preset windows-vs2022-debug --target EngineCoreTests EngineApp`
- `build\windows-vs2022-debug\Debug\EngineCoreTests.exe`
- `python tools\runtime_scene_smoke.py --exe build\windows-vs2022-debug\Debug\EngineApp.exe --scene data\scenes\veyra_reach_pilot.scene.json --ui-mode debug --report-json build\runtime\veyra-reach-pilot-debug-smoke-report.json`
- `scripts\verify.ps1`
```

## Files

- Create: `src/game/SceneRuntimeSurface.h`
- Create: `src/game/SceneRuntimeSurface.cpp`
- Modify: `CMakeLists.txt`
- Modify: `src/game/SandboxLayer.h`
- Modify: `src/game/SandboxLayer.cpp`
- Modify: `tests/EngineCoreTests.cpp`
- Modify: `docs/STATUS.md`

## Tasks

### Task 1: Red Tests For Neutral Runtime Surface

- [x] Add `#include "game/SceneRuntimeSurface.h"` to `tests/EngineCoreTests.cpp`.
- [x] Add `TestNeutralSceneRuntimeSurfaceBuildsPresentationWithoutFerryOfficeTerms`.
- [x] Add `TestNeutralSceneRuntimeSurfaceBuildsDebugWithoutFerryOfficeTelemetry`.
- [x] Run `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`.
- [x] Expected RED: compile fails because `game/SceneRuntimeSurface.h` does not exist yet.

### Task 2: Minimal Surface Module

- [x] Add `NeutralSceneRuntimeView` and the two builder functions in `SceneRuntimeSurface.h/.cpp`.
- [x] Add the new `.cpp` to `GamePrototype` in `CMakeLists.txt`.
- [x] Run `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`.
- [x] Run `build\windows-vs2022-debug\Debug\EngineCoreTests.exe`.

### Task 3: Wire SandboxLayer To The Surface

- [x] Add a private `buildNeutralSceneRuntimeView()` helper to `SandboxLayer`.
- [x] Replace inline neutral presentation/debug text assembly with `BuildNeutralScenePresentationText(...)` and `BuildNeutralSceneDebugText(...)`.
- [x] Remove the old inline neutral text methods.
- [x] Preserve Ferry Office text and all runtime smoke output contracts.

### Task 4: Validate And Publish

- [x] Run `python tests\test_runtime_scene_smoke.py`.
- [x] Run `cmake --build --preset windows-vs2022-debug --target EngineCoreTests EngineApp`.
- [x] Run `build\windows-vs2022-debug\Debug\EngineCoreTests.exe`.
- [x] Run Veyra debug runtime smoke.
- [x] Update `docs/STATUS.md`.
- [x] Run `scripts\verify.ps1`.
- [x] Commit and push only if validation passes and the worktree contains only this goal.
