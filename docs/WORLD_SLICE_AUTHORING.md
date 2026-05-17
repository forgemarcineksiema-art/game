# World Slice Authoring

Last updated: 2026-05-17

Purpose: define the first boundary between Ferry Office as a regression scene and future Veyra Reach target slices. This document does not authorize a full map, mission framework, terrain system, editor, NPC AI, traffic, production art, or renderer rewrite.

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
- validation and reporting tests.
- neutral runtime smoke/capture evidence.

Deferred:

- missions/jobs,
- NPCs,
- traffic,
- save/load,
- terrain mesh/spline systems,
- new vehicle runtime logic,
- production art/materials,
- broad asset pipeline changes,
- renderer rewrite,
- `SandboxLayer` location-specific gameplay chains.
- inherited Ferry Office objective/job overlay.
- inherited Ferry Office service vehicle fallback.

## Acceptance

A new target slice is acceptable only when:

- it validates independently with `tools/validate_scene.py`,
- `tools/world_slice_report.py` lists it separately from Ferry Office,
- Ferry Office validation remains unchanged,
- it does not add new Ferry Office content,
- it does not require new `SandboxLayer` gameplay logic,
- it loads through `tools/runtime_scene_smoke.py` without Ferry Office job text,
- it exposes `targetObjective=` runtime evidence for any authored target objective,
- `scripts/verify.ps1` passes.
