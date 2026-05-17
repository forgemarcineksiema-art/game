# First Real Slice Direction Gate

Date: 2026-05-17

## Verdict

BET: The next Tidebreak implementation goal should be **Veyra Reach pilot slice architecture scaffold**, not another Ferry Office content, polish, terrain, or local world-response pass.

INTERPRETATION: Ferry Office has done its current job. It should stay as a regression/debug testbed for build, scene data, interaction, Jolt vehicle runtime, camera, playthrough QA, visual capture, and authored collision evidence. It should not be treated as the first real game location by adding more endpoints to the same flat map.

WARNING: Building "a nicer real slice" with the current one-scene/one-integration-layer shape would likely produce the same debug playground with better labels. The project needs a boundary for the next world/slice before it needs more placed content.

## Evidence Read

- CONFIRMED: `docs/GAME_DIRECTION.md` defines Tidebreak as a compact island driver/fixer sandbox in Veyra Reach, not a city-first open-world promise or a pure physics experiment.
- CONFIRMED: `docs/VERTICAL_SLICE.md` still names Ferry Office as the first micro-slice, but the current chain has grown beyond the original 10-minute proof into a long validated testbed.
- CONFIRMED: `docs/audits/v0.99/README.md` says the v0.99 project can disappoint if expected to be a small game and identifies green QA reports as a major illusion of progress.
- CONFIRMED: `docs/audits/v0.99/game-review.md` calls the 21-event chain a validated authored checklist and says repeated `E` interactions are the weakest game pattern.
- CONFIRMED: `docs/reviews/post-v099-autonomy-rebaseline.md` records that current manual Jolt vehicle input/controls, physics, and camera are good enough to stop treating the vehicle verb as the active blocker.
- CONFIRMED: `docs/TECH_DEBT.md` identifies pressure in `SandboxLayer`, `FerryOfficeJob`, scene flag routing, static AABB collision, one-shot scene loading, lack of scene packages, no terrain/road substrate, no production asset/material path, and no mission graph/persistence.

## What Is No Longer The Active Blocker

- CONFIRMED: Current Jolt vehicle input semantics are covered by deterministic/Jolt sign-sensitive checks for `forward-left`, `forward-right`, `reverse-left`, and `reverse-right`.
- CONFIRMED: Manual play on 2026-05-17 accepted vehicle input/controls, physics, and camera in the current Ferry Office/Jolt playtest path.
- INTERPRETATION: Vehicle feel is still not "done forever", but it is no longer the best next blocker. Remaining Jolt risks belong to broader world conditions: non-flat surfaces, richer collision, dynamic blockers, traffic-like objects, and default-runtime policy.

## Candidate Matrix

| Candidate | Verdict | Reason |
| --- | --- | --- |
| Another Ferry Office world-response/content pass | Reject | CONFIRMED: the current chain is already long and many beats are administrative. More local reactions would keep investing in the debug playground. |
| Terrain/road/world-substrate implementation now | Reject as standalone | INFERRED: terrain/roads matter, but choosing them alone would overfit one complaint and ignore scene architecture, asset/render pipeline, player presence, and authoring boundaries. |
| Visual/readability/prop pass | Reject | CONFIRMED: v0.99 audit and captures showed visual proof is not game proof. It would not answer what the target game is. |
| Asset/render pipeline expansion | Reject as next primary goal | INFERRED: production visuals will matter, but a broader asset path without a target slice will become another showcase path. |
| Character/player presence pass | Defer | INFERRED: player identity matters, but it needs the target slice context; doing it inside Ferry Office risks polishing the testbed. |
| Manual/live capture-replay harness | Defer | CONFIRMED: it was the right answer before manual acceptance of current vehicle feel. It remains useful later, but it is no longer the next strategic question. |
| Scene/world architecture split plus first target slice scaffold | Choose | BET: this directly addresses the real risk: any new "world" on current structure becomes the same flat debug map, and more logic goes into `SandboxLayer`/one scene JSON. |

## Recommended Next Implementation Goal

**Goal name:** Veyra Reach pilot slice architecture scaffold.

**Outcome:** Create a small, validated authoring and runtime boundary for a second, non-Ferry target slice without building a full map or mission framework.

**What it should prove:**

- Ferry Office can remain a regression scene.
- A new target slice can exist as a separate scene/world package without copying the whole Ferry Office chain.
- The project has an explicit place to describe ground/surface/road semantics, authored collision, interaction markers, route topology, visual vocabulary, and future terrain needs without stuffing all of it into one growing scene JSON.
- `SandboxLayer` does not become the place where every future location, route, job, marker, and world-state rule is hardcoded.

**Smallest useful implementation shape:**

- Add a documented world/slice package contract, likely under `docs/architecture/` and `data/scenes/` or a new `data/worlds/` layout.
- Add a tiny second target-slice stub such as `veyra_reach_pilot` with no promise of final art: spawn, camera start, one or two authored surfaces, one route/road semantic, one collider group, and one interaction marker.
- Extend scene tools to report/validate the package or target-slice metadata.
- Add a minimal smoke or tool test that proves Ferry Office and the new pilot slice can both be selected and validated without changing Ferry Office behavior.
- Leave gameplay content thin; the point is the boundary, not a new mission.

## Rejected Directions

- WARNING: Do not do "terrain" as a solo answer. The problem is not only ground variety; it is the lack of a target world architecture that can carry terrain, roads, collision, materials, player presence, and authoring without collapsing into a larger debug scene.
- WARNING: Do not do another Ferry Office reaction. The current map is useful as a controlled testbed, but extending it again confuses validation value with game value.
- WARNING: Do not start an editor, ECS, mission framework, renderer rewrite, full asset pipeline, traffic, NPC, save/load, or broad map expansion. The next step should be a scaffold small enough to validate and throw away if the shape is wrong.

## Acceptance Criteria For The Next Goal

- CONFIRMED: A new target-slice package or stub exists separately from `data/scenes/ferry_office.scene.json`.
- CONFIRMED: Ferry Office validation still passes unchanged.
- CONFIRMED: scene/world tooling can report/validate both Ferry Office and the new pilot slice metadata.
- CONFIRMED: docs explain which data belongs in the package now and which systems remain deliberately deferred.
- CONFIRMED: `SandboxLayer` gains no broad new location-specific gameplay chain.
- CONFIRMED: `scripts/verify.ps1` passes, or an exact blocker is recorded.

## Prompt For The Next Goal

```text
/goal Veyra Reach pilot slice architecture scaffold dla Tidebreak w C:\Users\Marcin\Documents\New project.

Cel:
Stworzyć minimalny, walidowany scaffold dla pierwszego target slice'u poza Ferry Office, bez udawania pełnej mapy ani nowej misji. Ferry Office ma zostać regression/debug testbedem. Nowy scaffold ma odpowiedzieć, gdzie i jak projekt zacznie budować faktyczny świat gry: osobny slice/package, podstawowe surface/road/collision/marker metadata, tool validation, i jasny boundary przed dalszym contentem.

Dlaczego:
Po naprawie i manualnym potwierdzeniu Jolt vehicle input/physics/camera aktywnym blockerem nie jest już vehicle feel. Aktywnym ryzykiem jest to, że każdy kolejny "world/content/terrain" pass na obecnej technologii stanie się tym samym płaskim debug playgroundem i dopchnie jeszcze więcej logiki do `SandboxLayer`, `FerryOfficeJob` oraz jednego scene JSON.

Zakres:
- Przeczytaj `AGENTS.md`, `docs/AI_WORKFLOW.md`, `docs/CONTEXT_MAP.md`, `docs/GAME_DIRECTION.md`, `docs/VERTICAL_SLICE.md`, `docs/GAMEPLAY_REVIEW.md`, `docs/TECH_DEBT.md`, `docs/SCENE_AUTHORING.md`, `docs/ASSET_GUIDE.md`, `docs/ART_DIRECTION.md`, `docs/MESH_RENDERING.md`, `docs/reviews/first-real-slice-direction-gate.md`.
- Zaprojektuj mały world/slice package contract dla drugiego target slice'u, bez pełnego edytora i bez mission frameworka.
- Dodaj minimalny stub nowego target slice'u oddzielony od Ferry Office, np. `veyra_reach_pilot`, z metadanymi surface/road/collision/markers wystarczającymi do walidacji.
- Rozszerz narzędzia raportujące/walidujące, żeby odróżniały Ferry Office regression scene od target slice scaffold.
- Nie buduj nowej misji, nie rozbudowuj Ferry Office, nie rób terrain-only passu, nie rób asset/render rewrite.
- Zaktualizuj `docs/STATUS.md` i dokument architektury/authoringu, żeby przyszłe runy nie wracały do "ulepszmy Ferry Office".

Walidacja:
- `git status --short --branch`
- `python tools/status_report.py`
- `python tools/scene_report.py`
- `python tools/validate_scene.py`
- `python tools/scale_audit.py`
- `python tools/mesh_report.py`
- nowe/zmienione testy narzędzi
- `scripts/verify.ps1`

Stop rule:
Zakończ dopiero, gdy istnieje oddzielny target-slice scaffold, Ferry Office nadal przechodzi walidację, narzędzia potrafią opisać oba światy/slice'y, `docs/STATUS.md` jest zaktualizowane, a repo jest committed/pushed tylko jeśli walidacja przejdzie i worktree jest scoped.
```
