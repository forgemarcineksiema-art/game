# Scene Runtime Policy Boundary Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a small explicit scene-role runtime policy so `SandboxLayer` no longer decides Ferry Office versus neutral target-slice behavior through scattered direct role checks.

**Architecture:** Introduce `SceneRuntimePolicy` in `src/game` as a tiny value-type boundary derived from `SceneDefinition` plus loaded/fallback state. `SandboxLayer` will hold one policy after scene load and use it for fallback vehicle, neutral presentation, world-state debug, and route/objective guidance decisions.

**Tech Stack:** C++20, existing `GamePrototype` CMake target, `tests/EngineCoreTests.cpp`, `scripts/verify.ps1`.

---

## Goal Prompt

```text
/goal Scene-role runtime policy boundary dla Tidebreak w C:\Users\Marcin\Documents\New project.

Cel:
Wyciagnac najmniejsza jawna scene-role runtime policy poza bezposrednie if-y w `SandboxLayer`, tak zeby Ferry Office regression-testbed i Veyra target-slice scaffold mialy testowalny kontrakt: fallback vehicle, world-state debug, job presentation, route/objective guidance i neutral debug/presentation nie moga dryfowac przez przypadek.

Dlaczego:
Po Veyra pilot smoke i runtime-to-authoring split hardening wiemy, ze drugi slice moze wejsc do runtime bez najglosniejszych Ferry Office leakow, ale decyzje nadal sa rozproszone w `SandboxLayer`. To jest ryzyko systemowe przed jakimkolwiek prawdziwym kawalkiem swiata: kazdy nowy slice bedzie kusil kolejnymi specjalnymi if-ami.

Zakres:
- Dodaj maly `SceneRuntimePolicy`/helper w `src/game`, ktory na podstawie `SceneDefinition` odpowiada na pytania: czy uzyc Ferry Office behavior, czy target-slice neutral presentation, czy fallback vehicle jest dozwolony, czy world-state debug ma byc rysowany, jaki tryb route/objective guidance obowiazuje.
- Przepnij tylko istniejace decyzje w `SandboxLayer`, ktore dotycza roli sceny; nie przenos calego pliku.
- Dodaj testy dla `regression-testbed`, `target-slice-scaffold`, generic loaded authored scene i fallback/default behavior.
- Zachowaj Ferry Office playthrough QA oraz Veyra runtime smoke/capture.
- Zaktualizuj `docs/STATUS.md`.

Non-goals:
- Nie dodawaj mapy, terrainu, contentu, misji, assetow, NPC, traffic, edytora ani renderer rewrite.
- Nie rozbudowuj Veyra pilot scene.
- Nie refaktoruj calego `SandboxLayer`.

Walidacja:
- `python tests\test_runtime_scene_smoke.py`
- `cmake --build --preset windows-vs2022-debug --target EngineCoreTests EngineApp`
- `build\windows-vs2022-debug\Debug\EngineCoreTests.exe`
- `python tools\runtime_scene_smoke.py --exe build\windows-vs2022-debug\Debug\EngineApp.exe --scene data\scenes\veyra_reach_pilot.scene.json --ui-mode debug --report-json build\runtime\veyra-reach-pilot-debug-smoke-report.json`
- `scripts\verify.ps1`
```

## Files

- Create: `src/game/SceneRuntimePolicy.h`
- Create: `src/game/SceneRuntimePolicy.cpp`
- Modify: `CMakeLists.txt`
- Modify: `src/game/SandboxLayer.h`
- Modify: `src/game/SandboxLayer.cpp`
- Modify: `tests/EngineCoreTests.cpp`
- Modify: `docs/STATUS.md`

## Tasks

### Task 1: Red Tests For Scene Runtime Policy

- [x] Add `#include "game/SceneRuntimePolicy.h"` to `tests/EngineCoreTests.cpp`.
- [x] Add tests proving:
  - fallback/no loaded scene uses Ferry Office regression behavior and allows fallback vehicle,
  - `regression-testbed` uses Ferry Office job/debug/guidance policy,
  - `target-slice-scaffold` uses neutral presentation, no fallback vehicle, no world-state debug, all-authored guidance,
  - a generic loaded non-regression scene is neutral rather than silently Ferry Office.
- [x] Run `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`.
- [x] Expected RED: compile fails because `game/SceneRuntimePolicy.h` does not exist yet.

### Task 2: Minimal Policy Module

- [x] Add `SceneRuntimePolicy.h/.cpp`.
- [x] Add the new `.cpp` to `GamePrototype` in `CMakeLists.txt`.
- [x] Implement only the role decisions needed by existing `SandboxLayer`.
- [x] Run `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`.
- [x] Run `build\windows-vs2022-debug\Debug\EngineCoreTests.exe`.

### Task 3: Wire Policy Into SandboxLayer

- [x] Add `SceneRuntimePolicy m_runtimePolicy` to `SandboxLayer`.
- [x] Set it after scene load and after fallback.
- [x] Replace direct role checks in fallback vehicle, QA capture state, presentation/debug text, world-state debug drawing, and route/objective guidance.
- [x] Keep Ferry Office behavior and Veyra smoke output unchanged.
- [x] Run focused C++ and runtime smoke gates.

### Task 4: Status And Full Validation

- [x] Update `docs/STATUS.md` with prompt, red/green evidence, commands, limits, and next prompt.
- [x] Run `python tests\test_runtime_scene_smoke.py`.
- [x] Run `cmake --build --preset windows-vs2022-debug --target EngineCoreTests EngineApp`.
- [x] Run `build\windows-vs2022-debug\Debug\EngineCoreTests.exe`.
- [x] Run Veyra debug runtime smoke.
- [x] Run `scripts\verify.ps1`.
- [x] Commit and push only if validation passes and the worktree contains only this goal.
