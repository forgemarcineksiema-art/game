# Target-Slice Authored Objective Runtime Gate

**Goal:** Add the first small authored objective/consequence runtime gate for `target-slice-scaffold` scenes so Veyra can prove a non-Ferry scene can respond to player interaction without borrowing `FerryOfficeJob`.

**Architecture:** Keep Ferry Office as the regression/debug testbed. Add a tiny target-slice objective definition to scene data, parse it through `SceneLoader`, and execute it through a small `TargetSliceObjectiveRuntime` helper owned by `PrototypeScene`. `SandboxLayer` should only display the resulting neutral objective/debug text through the existing neutral runtime surface.

**Tech Stack:** C++20, existing `GamePrototype`, `EngineCoreTests`, Python runtime smoke tests, PowerShell verification scripts.

## Constraints

- Do not add terrain, new map content, vehicles, missions, NPCs, renderer work, or Jolt work.
- Do not move Ferry Office job logic into the new helper.
- Do not make target-slice completion depend on global `WorldFlag` names.
- Do not claim a playable slice; this proves an authored objective/consequence seam only.

## Files

- Create: `src/game/TargetSliceObjectiveRuntime.h`
- Create: `src/game/TargetSliceObjectiveRuntime.cpp`
- Modify: `src/game/SceneDefinition.h`
- Modify: `src/game/SceneLoader.cpp`
- Modify: `src/game/PrototypeScene.h`
- Modify: `src/game/PrototypeScene.cpp`
- Modify: `src/game/SceneRuntimeSurface.cpp`
- Modify: `data/scenes/veyra_reach_pilot.scene.json`
- Modify: `tests/EngineCoreTests.cpp`
- Modify: `tests/test_runtime_scene_smoke.py`
- Modify: `tools/runtime_scene_smoke.py`
- Modify: `docs/STATUS.md`

## Plan

- [x] Add failing C++ coverage for authored target-slice objective loading and completion.
- [x] Add failing Python runtime-smoke coverage that requires target objective evidence for target slices.
- [x] Implement the smallest scene definition/parser/runtime helper.
- [x] Wire `PrototypeScene` neutral objective text, completion state, and debug summary to the helper.
- [x] Update Veyra authored data with one objective bound to the existing service marker.
- [x] Run focused red/green validation.
- [x] Run full validation and commit/push only if clean and scoped.

## Verification

- `cmake --build --preset windows-vs2022-debug --target EngineCoreTests`
- `build/windows-vs2022-debug/Debug/EngineCoreTests.exe`
- `python tests/test_runtime_scene_smoke.py`
- `cmake --build --preset windows-vs2022-debug --target EngineCoreTests EngineApp`
- `python tools/runtime_scene_smoke.py --exe build/windows-vs2022-debug/Debug/EngineApp.exe --scene data/scenes/veyra_reach_pilot.scene.json --ui-mode debug --report-json build/runtime/veyra-reach-pilot-target-objective-debug-smoke-report.json`
- `scripts/verify.ps1`
