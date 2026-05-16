# Tidebreak v0.99 Audit

Date: 2026-05-17
Repo: `C:\Users\Marcin\Documents\New project`
Commit audited: `f5e2fdb` (`main`, `origin/main`) plus audit-only untracked files created during this goal.

## Verdict

Tidebreak v0.99 is a real custom-engine workbench with a runnable Ferry Office prototype, a validated 21-event state chain, GDI/DX11 captures, scene/asset tools, and a strong Jolt vehicle-runtime evidence stack. It is not yet a robust playable vertical slice in the normal player sense.

The project can disappoint after launch if the expectation is "a small game". What launches is closer to a controlled prototype: readable low-poly place, overlay-driven objective text, one service-vehicle route, many authored `E` interactions, and a lot of validation scaffolding. The strongest foundation is the validation culture and engine boundary discipline. The biggest illusion is treating green QA reports as proof that the game feels good.

## Top Findings

| Priority | Status | Finding | Evidence |
| --- | --- | --- | --- |
| P0 | CONFIRMED | No P0 build/runtime blocker found in this session. | `docs/audits/v0.99/raw/07-verify.log`, `23-ctest-jolt.log` |
| P1 | CONFIRMED | `FerryOfficePlaythroughQaSmoke` under the Jolt preset does not force `--vehicle-runtime jolt`; a green Jolt CTest can hide deterministic playthrough coverage. | `CMakeLists.txt:269`, `tools/playthrough_qa.py:139`, `docs/audits/v0.99/raw/29-ctest-jolt-show-only.log` |
| P1 | CONFIRMED | `scripts/verify.ps1` is a default/headless foundation gate, not a visual/Jolt/gameplay gate. | `scripts/verify.ps1:31`, `scripts/verify.ps1:63`, `docs/audits/v0.99/raw/07-verify.log` |
| P1 | CONFIRMED | Playthrough QA proves a scripted state chain and runtime vehicle segment, not full keyboard/mouse navigation or player feel. | `src/game/FerryOfficePlaythroughQa.cpp:72`, `src/game/FerryOfficePlaythroughQa.cpp:512`, `docs/audits/v0.99/raw/playthrough-jolt-report.json` |
| P1 | CONFIRMED | Vehicle/Jolt evidence proves route pace and controls proxies, not full world vehicle collision. | `src/game/VehicleController.cpp:218`, `src/engine/physics/JoltVehicleRuntime.cpp`, `src/game/FerryOfficeVehiclePhysicsQa.cpp:500`, `docs/TECH_DEBT.md` vehicle section |
| P1 | CONFIRMED | Visual smoke proves nonblank renderer output, not visual quality or objective readability. | `tools/capture_visual_smoke.py`, `docs/audits/v0.99/raw/visual-smoke-initial-report.json` |
| P1 | CONFIRMED | GDI shaded triangle ordering can be misleading because `ScenePresentation` submits one triangle per renderer call while GDI sorts only inside a call. | `src/game/ScenePresentation.cpp:130`, `src/engine/renderer/GdiRenderer.cpp:167` |
| P2 | CONFIRMED | Route-time budgets are inconsistent: vehicle runtime QA uses 190 frames; playthrough QA and one core test still use/say 240 frames. | `src/game/FerryOfficeVehiclePhysicsQa.cpp:29`, `src/game/FerryOfficePlaythroughQa.cpp:25`, `tests/EngineCoreTests.cpp:4790` |
| P2 | CONFIRMED | Runtime version logs `0.23.0` while project docs/status discuss v0.99. | `CMakeLists.txt:4`, `src/engine/application/Engine.cpp:13`, runtime logs |
| P2 | CONFIRMED | Scene data is the default source of truth, but explicit bad scenes still fall back instead of failing hard. | `src/game/SandboxLayer.cpp:330`, `docs/SCENE_AUTHORING.md` drift-risk section |
| P2 | CONFIRMED | Recent Jolt runtime policy is documented mainly in `PHYSICS_DECISION.md`/`STATUS.md`, not the main `DECISIONS.md` ledger. | `git log -- docs/DECISIONS.md`, `docs/PHYSICS_DECISION.md` |

## Coverage Matrix

| Subsystem | Sprawdzone pliki | Metoda weryfikacji | Werdykt | Najwieksze ryzyko |
| --- | --- | --- | --- | --- |
| Build/CMake/scripts | `CMakeLists.txt`, `CMakePresets.json`, `scripts/doctor.ps1`, `scripts/configure.ps1`, `scripts/build.ps1`, `scripts/verify.ps1`, `scripts/play.ps1`, `scripts/run.ps1` | `doctor`, `configure`, `build`, `verify`, `run` smoke, code inspection, subagent build/tools review | CONFIRMED stable default path | Green gates under-cover Jolt/live visuals/play wrapper |
| Runtime/application/config | `src/engine/application/*`, `src/engine/core/Config.*`, `src/game/main.cpp`, `src/game/SandboxLayer.*` | smoke runs, play wrapper runs, raw logs, code inspection | CONFIRMED starts in null/GDI/DX11 bounded modes | runtime version stale; wrapper/preset mismatch |
| Renderer/visuals | `src/engine/renderer/*`, `src/game/ScenePresentation.*`, `tools/capture_visual_smoke.py` | GDI/DX11 captures, visual smoke initial/relay/low-dock, screenshots viewed | CONFIRMED nonblank and readable prototype view | smoke does not prove composition; GDI sorting gap; DX11 WARP |
| Scene/world data | `data/scenes/ferry_office.scene.json`, `src/game/SceneLoader.*`, `src/game/SceneDefinition.*`, `src/game/PrototypeWorld.*` | scene report, validate scene, scale audit, code inspection | CONFIRMED coherent scene source for current layout | fallback behavior weakens strict source-of-truth claims |
| Assets/static mesh | `assets/models/*`, `src/engine/assets/StaticMesh.*`, `tools/validate_assets.py`, `tools/mesh_report.py` | asset validation, mesh report, visual captures | CONFIRMED all 20 model files referenced and validated | immediate-mode placeholder mesh path, no real materials/resources |
| Gameplay/job/state | `src/game/FerryOfficeJob.*`, `src/game/FerryOfficePlaythroughQa.*`, `src/game/WorldState.*`, `src/game/PrototypeScene.*` | deterministic and Jolt playthrough QA, code inspection, tests | CONFIRMED 21-event chain passes | scripted chain can be mistaken for full gameplay |
| Player/camera/traversal | `src/game/PlayerController.*`, `src/game/ThirdPersonCamera.*`, `src/game/TraversalSystem.*` | code inspection, C++ tests in `EngineCoreTests`, play/debug screenshot | INFERRED baseline works; no full human-feel proof | no acceleration/camera collision/full traversal collision |
| Vehicle/Jolt/physics | `src/game/VehicleController.*`, `src/engine/physics/*`, `src/game/FerryOfficeVehiclePhysicsQa.*` | Jolt configure/build/CTest, parity/contact/vehicle/runtime QA, Jolt playthrough | CONFIRMED strong opt-in evidence | route/controls metrics are proxies; world collision incomplete |
| Tests/QA/evidence | `tests/EngineCoreTests.cpp`, `tests/test_*.py`, `tools/*qa*.py` | `scripts/verify.ps1`, `ctest` default/Jolt, tool reports | CONFIRMED broad automated coverage | evidence names overstate what is actually covered |
| Docs/history | `docs/STATUS.md`, `docs/ROADMAP.md`, `docs/TECH_DEBT.md`, `docs/PHYSICS_DECISION.md`, `docs/CONTEXT_MAP.md`, git log | required doc read, `git log --max-count=200`, subagent git/docs review | CONFIRMED rich history and guardrails | stale snapshots and decision split across docs |

## Reality Check

1. Czy po odpaleniu gry moge sie rozczarowac? Tak, jesli oczekujesz pelnoprawnej gry zamiast silnego prototypu.
2. Co rozczaruje? Overlay prowadzi gracza, interakcje sa czesto marker/flag based, vehicle/world collision jest ograniczone, a dlugi chain jest bardziej checklistowy niz systemic.
3. Najmocniejszy fundament: walidacja + jasne engine/game/physics/scene boundaries.
4. Najwieksza iluzja postepu: liczba milestone'ow i zielonych QA raportow moze wygladac jak grywalnosc.
5. Najbardziej kruche technicznie: `SandboxLayer`/`FerryOfficeJob`/scene flag routing plus vehicle collision/runtime split.
6. Najbardziej zaniedbane jako gra: live input feel, camera/control readability, world understanding without overlay.
7. Nastepny milestone: Jolt-first live controls/camera + authored road-edge collision evidence gate.
8. Czego nie robic: nie dodawac kolejnego `E` prompt endpointu, mission frameworka, broad map expansion, renderer rewrite, ani promowac Jolt jako default na sile.

## Artifact Index

- `project-map.md` - subsystem map and integration risks.
- `validation-log.md` - commands, results, failures, screenshots manifest.
- `code-review.md` - technical findings by subsystem.
- `game-review.md` - player-experience audit.
- `git-history.md` - commit/history and documentation audit.
- `subagent-findings.md` - reconciled subagent findings.
- `reflections.md` - coordinator interpretation/opinion.
- `next-steps.md` - one recommended milestone and ordered P0/P1/P2.
- `checkpoint.md` - what is checked, unchecked, blocked, continuation plan.
