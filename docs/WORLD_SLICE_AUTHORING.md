# World Slice Authoring

Last updated: 2026-05-17

Purpose: define the first boundary between Ferry Office as a regression scene and future Veyra Reach target slices. This document does not authorize a full map, mission framework, terrain system, editor, NPC AI, traffic, production art, or renderer rewrite.

Post-v0.99 direction rebaseline: future target slices should serve Tidebreak as a narrative vehicle/crime/action sandbox, not as a service-work checklist. A target slice can be coastal, industrial, rural, resort-like, or urban depending on location. The common contract is risk, vehicle/space meaning, local response, and consequence.

## Scene Roles

Every authored scene can now declare `sliceMetadata`.

Required role values:

- `regression-testbed`: a scene kept primarily for build/runtime/QA regression evidence. Current example: `data/scenes/ferry_office.scene.json`.
- `target-slice-scaffold`: a minimal future-facing slice stub used to prove authoring boundaries before gameplay content. Current example: `data/scenes/veyra_reach_pilot.scene.json`.

## Current Scenes

`ferry-office`

- Role: `regression-testbed`.
- Purpose: preserve the validated Ferry Office prototype and QA surface.
- Do not treat it as the first real production game location by adding more endpoints, local reactions, or polish loops.

`veyra-reach-pilot`

- Role: `target-slice-scaffold`.
- Purpose: prove a second slice can exist outside Ferry Office with its own surface, road, collision, route, and marker intent.
- It is not a mission, map expansion, terrain implementation, art pass, or new runtime gameplay chain.
- It now carries one `targetObjective` gate bound to the existing Pilot Service Marker. This proves authored objective/consequence runtime for a target slice; it is still not a mission system.
- It now carries one `targetActionResponse` contract: `Suspicious Cargo Cache` triggers `pilot-local-alerted`, then `Pilot Escape Marker` proves exit/recovery. This is a testable runtime response contract, not a police/AI/mission framework.
- Its next useful evolution should make that response visible and spatially meaningful, not add a prettier scaffold or another neutral `E` prompt.

## Metadata Contract

`sliceMetadata` fields:

- `kind`: `regression-testbed` or `target-slice-scaffold`.
- `worldId`: stable world id, currently `veyra-reach`.
- `sliceId`: stable local slice id.
- `status`: short status string such as `active-regression-scene` or `scaffold-only`.
- `role`: one sentence describing why this scene exists.
- `intent`: one sentence describing what this scene proves.
- `surfaceTags`: high-level surface intent such as `wet-asphalt` or `hillside-ground`.
- `roadTags`: high-level road/route intent such as `narrow-service-road`.
- `deferredSystems`: explicit list of systems not implemented by the scene.
- `collisionPolicy`: current collision meaning for static, dynamic, and vehicle behavior.
- `authoringBoundaries`: what is allowed now, what is deferred, and which systems own the data.

## Tooling

Use the existing scene tools for a single scene:

```powershell
python tools/scene_report.py data/scenes/veyra_reach_pilot.scene.json
python tools/validate_scene.py data/scenes/veyra_reach_pilot.scene.json
python tools/scale_audit.py data/scenes/veyra_reach_pilot.scene.json
```

Use the world/slice report to compare scene roles:

```powershell
python tools/world_slice_report.py
```

Use the runtime smoke gate to prove the pilot slice is not dead JSON and does not leak Ferry Office presentation:

```powershell
python tools/runtime_scene_smoke.py --exe build/windows-vs2022-debug/Debug/EngineApp.exe --scene data/scenes/veyra_reach_pilot.scene.json
```

For `target-slice-scaffold`, runtime smoke must also expose `targetObjective=` evidence. A smoke output that only says "neutral slice markers" is no longer enough.

`tools/mesh_report.py` can be run on meshless target slices. It should print that project model files are not referenced by that specific scene, but zero Veyra meshes are valid until a goal explicitly adds authored mesh assets.

Use the target-slice objective QA gate to prove the Veyra marker can be reached and completed through recorded live-like player input:

```powershell
python tools/target_slice_objective_qa.py --exe build/windows-vs2022-debug/Debug/EngineApp.exe --scene data/scenes/veyra_reach_pilot.scene.json
```

This gate must report recorded input script name, authored `pilot-road-edge-collider` contact, contact push/normal, recovery timing, frames to focus, frames to interact, focus name/prompt, interaction result, and `targetObjective=inspect-pilot-service-marker` completion evidence. It is not proof that Veyra is playable as a world.

For Veyra, the same gate must also report:

- `riskyAction.id=pilot-cache-risk-response`,
- `riskyAction.interactableName=Suspicious Cargo Cache`,
- `localResponse.stateId=pilot-local-alerted`,
- `exitRecovery.stateId=pilot-escape-confirmed`,
- final summary evidence for `riskyAction=`, `responseState=`, and `exitRecovery=`.

Objective completion alone is stale evidence for the target slice. A report that reaches only `Pilot Service Marker` without risky-action/local-response/exit-recovery proof should fail.

Expected current roles:

- `ferry-office`: `regression-testbed`
- `veyra-reach-pilot`: `target-slice-scaffold`

## Boundaries

Allowed in a target-slice scaffold:

- scene identity,
- player start,
- surface and road intent tags,
- small static collision placeholders,
- visual placeholders,
- route markers,
- objective markers,
- one debug interaction marker,
- one authored `targetObjective` gate bound to an existing interactable marker,
- one authored `targetActionResponse` gate bound to a risky interactable and an exit/recovery interactable,
- validation and reporting tests.
- neutral runtime smoke/capture evidence.
- one tiny action/response proof if it is explicitly scoped as target-slice gameplay contract evidence.

Deferred:

- missions/jobs,
- NPCs,
- traffic,
- full police/wanted/pursuit AI,
- combat/weapons,
- save/load,
- terrain mesh/spline systems,
- new vehicle runtime logic,
- production art/materials,
- broad asset pipeline changes,
- renderer rewrite,
- `SandboxLayer` location-specific gameplay chains.
- inherited Ferry Office objective/job overlay.
- inherited Ferry Office service vehicle fallback.

## Crime/Action Slice Contract

A future target-slice gameplay gate is acceptable only when it stays small and proves these facts:

- The player action has risk or rule-breaking meaning in the scene.
- The scene responds visibly or through runtime state that can later drive presentation.
- Vehicle or route choice matters to escape, repositioning, recovery, or escalation.
- The report distinguishes authored objective completion from action/response evidence.
- Ferry Office job/world-state language does not leak into the target slice.

This is not permission to build a generic mission framework, wanted system, traffic simulation, large map, terrain pass, or production content pipeline.

## Acceptance

A new target slice is acceptable only when:

- it validates independently with `tools/validate_scene.py`,
- `tools/world_slice_report.py` lists it separately from Ferry Office,
- Ferry Office validation remains unchanged,
- it does not add new Ferry Office content,
- it does not require new `SandboxLayer` gameplay logic,
- it loads through `tools/runtime_scene_smoke.py` without Ferry Office job text,
- it exposes `targetObjective=` runtime evidence for any authored target objective,
- it passes `tools/target_slice_objective_qa.py` when the authored target objective is meant to be player-acquirable,
- `scripts/verify.ps1` passes.
