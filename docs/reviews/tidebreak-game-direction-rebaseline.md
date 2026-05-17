# Tidebreak Game Direction Rebaseline

Date: 2026-05-17

## Verdict

Tidebreak should be treated as a compact narrative vehicle/crime/action sandbox, not a driver/fixer service-work game.

Ferry Office remains valuable as a regression/debug testbed. It should not keep receiving endpoint, prop, route, or office-log polish as if a longer checklist will become the game. Veyra Reach is the current future-facing target-slice surface, but it is still only a scaffold. Its next useful proof is not terrain, content quantity, or visual polish. Its next useful proof is a tiny risky-action plus world-response contract.

## Confirmed Current Project Facts

- CONFIRMED: `docs/GAME_DIRECTION.md` previously framed the player as an outsider driver/fixer and the core loop around local jobs, access, repairs, favors, and remembered consequences.
- CONFIRMED: `docs/VERTICAL_SLICE.md` still records Ferry Office as the historical first playable micro-scenario and a service-call chain.
- CONFIRMED: `docs/GAMEPLAY_REVIEW.md` already warned that too many Ferry Office beats had become administrative sign-offs: walk to marker, press `E`, set one flag, read text.
- CONFIRMED: `docs/WORLD_SLICE_AUTHORING.md` distinguishes `ferry-office` as `regression-testbed` and `veyra-reach-pilot` as `target-slice-scaffold`.
- CONFIRMED: `docs/STATUS.md` records that Veyra has target objective QA and authored contact/recovery evidence, but is not yet a playable world.

## Rebased Game Thesis

Tidebreak is about a story protagonist moving through coastal places where risk, vehicles, conflict, and world reaction matter.

The user-facing shorthand is "a mix of GTA IV and GTA VI", but only as a high-level ambition lens: grounded physical weight, vehicle/action pressure, strong protagonist context, dense place identity, and modern sandbox readability. It is not permission to copy copyrighted content, names, maps, characters, mission designs, brands, logos, or assets.

The game can borrow broad lessons from modern vehicle/crime/action sandboxes and physics-heavy open-world games without copying their names, maps, characters, mission designs, logos, brands, or assets. The target is not necessarily urban or gangster. Location controls tone:

- industrial port: heavier, dirtier, more grounded,
- resort or wealth district: brighter, more satirical,
- rural road or wet service route: lonely, tense, physically risky,
- small town center: social pressure, witnesses, reputational consequence,
- coastal illegal work site: smuggling, theft, trespass, debt, escape.

The shared rule is action under pressure. The player should not feel like a neutral operator clearing service tickets.

## What To Stop Doing

- Do not add another Ferry Office endpoint by default.
- Do not add another neutral `E` prompt that only sets a flag.
- Do not turn Veyra into a prettier empty testbed.
- Do not choose terrain, asset polish, route readability, or local feedback just because it is safe.
- Do not build a mission framework, wanted system, NPC AI, traffic, combat suite, or editor before the smallest action/response contract exists.

## What To Prove Next

The next playable-facing proof should be:

1. protagonist enters a small risky situation,
2. player action crosses a rule or creates conflict,
3. world responds visibly or through runtime state,
4. vehicle/route matters for escape, recovery, or escalation,
5. QA reports action, response, and completion separately.

This can be abstract and ugly at first. It cannot be another service checklist.

## Recommended Goal Sequence

### Goal 1: Veyra Risky Action Response Contract

Build the smallest target-slice runtime contract for a risky player action and local response.

Use Veyra because it already has neutral runtime, objective, and contact QA outside Ferry Office. Keep the action simple: a provisional valuable target, trespass line, access breach, or recovery marker. The response can be a visible state, route change, alarm/attention proxy, or escape requirement. The important part is not content richness; it is proving that Tidebreak can express crime/action causality outside Ferry Office.

### Goal 2: Vehicle-As-Action Resolution Gate

After Goal 1, require the vehicle or route to matter to the response resolution.

This should not retune Jolt again. It should use the accepted vehicle feel as a foundation and prove that arrival/escape/repositioning is part of the slice contract.

### Goal 3: Location Tone Packet

Only after action/response works, author a tiny location-tone packet for the same target slice.

This should define what kind of place it is, what conflict lives there, what is visually readable, and what should be deferred. It is not a broad art pass.

## Ready Next Goal Prompt

```text
/goal Veyra risky action response contract dla Tidebreak w C:\Users\Marcin\Documents\New project.

Cel:
Zbudowac pierwszy maly dowod, ze Tidebreak idzie w narrative vehicle/crime/action sandbox, a nie w service-work checklist: w Veyra target-slice dodac minimalny risky-action + local-response contract, w ktorym gracz wykonuje prosta ryzykowna akcje, runtime rozroznia objective completion od action/response evidence, swiat pokazuje odpowiedz, a QA raportuje akcje, reakcje, failure reason i exit/recovery condition.

Dlaczego:
Ferry Office jest regression/debug testbedem i ma za duzo administracyjnych `E` promptow. Veyra ma juz neutralny target-slice runtime, objective QA i authored contact evidence, ale nadal nie dowodzi gry. Nastepny krok ma udowodnic najmniejsza prawde gry: ryzykowna akcja powoduje reakcje swiata.

Zakres:
- Przeczytaj AGENTS.md, docs/AI_WORKFLOW.md, docs/GAME_DIRECTION.md, docs/GAMEPLAY_REVIEW.md, docs/VERTICAL_SLICE.md, docs/WORLD_SLICE_AUTHORING.md, docs/TECH_DEBT.md, docs/ROADMAP.md i docs/STATUS.md.
- Dodaj najmniejszy target-slice action/response runtime helper albo rozszerz obecny target objective runtime, bez mieszania tego z FerryOfficeJob.
- Dodaj do Veyra jedna tymczasowa risky-action powierzchnie: valuable target, trespass/access breach, recovery marker albo rownowazny minimalny trigger.
- Raport QA ma osobno pokazac: targetObjective, riskyAction, responseState, framesToAction, framesToResponse, exit/recovery result, failure reason, leakage check.
- Zachowaj Ferry Office jako regression-testbed i nie dodawaj do niego contentu.
- Zaktualizuj docs/STATUS.md, docs/GAMEPLAY_REVIEW.md albo docs/WORLD_SLICE_AUTHORING.md tylko tam, gdzie claimy sie zmieniaja.

Non-goals:
- Nie dodawac pelnej misji, policji/wanted systemu, NPC AI, traffic, combat/weapons, ekonomii, save/load, edytora, terrain systemu, asset/render passu ani map expansion.
- Nie robic visual/readability polish passu.
- Nie dodawac kolejnego neutralnego `E` promptu bez ryzyka i odpowiedzi swiata.
- Nie przenosic Jolt/default-runtime decyzji, jesli nie dotykasz pojazdu.

Walidacja:
- git status --short --branch
- python tools/status_report.py
- scripts/doctor.ps1
- scripts/configure.ps1
- scripts/build.ps1
- python tests/test_target_slice_objective_qa.py
- build/windows-vs2022-debug/Debug/EngineCoreTests.exe
- python tools/target_slice_objective_qa.py --exe build/windows-vs2022-debug/Debug/EngineApp.exe --scene data/scenes/veyra_reach_pilot.scene.json --report-json build/playthroughs/veyra-risky-action-response-report.json
- python tools/runtime_scene_smoke.py --exe build/windows-vs2022-debug/Debug/EngineApp.exe --scene data/scenes/veyra_reach_pilot.scene.json --ui-mode debug --report-json build/runtime/veyra-risky-action-response-smoke-report.json
- python tools/validate_scene.py data/scenes/veyra_reach_pilot.scene.json
- python tools/world_slice_report.py
- scripts/verify.ps1

Stop rule:
Zakoncz dopiero, gdy action/response contract ma test, QA raport, STATUS update, verify wynik albo blocker, i jasna odpowiedz czego nadal nie udowodniono jako crime/action sandbox.
```
