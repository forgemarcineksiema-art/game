# Scene Authoring

Last updated: 2026-05-17

v0.11 introduced scene data for Codex-friendly inspection and validation. v0.12 added mesh asset and mesh instance references. v0.12.1 expanded the Ferry Office mesh set for a focused prop/scale pass. v0.14 added the first dock road segment as authored placeholders and route markers. v0.15 makes `data/scenes/ferry_office.scene.json` the runtime source of truth for current layout data while keeping Tidebreak-specific behavior in C++. v0.18 adds the first original service-road prop-kit meshes and keeps them scene-authored. v0.20 adds one fallback-generated ferry notice board because Blender was unavailable; its provenance must stay explicit. v0.20.1 adds one real Blender-exported notice-board prop and keeps it scene-authored too. v0.42 adds a Blender-authored wet-road surface mesh for the service-yard, dock-road, and turn-around presentation slabs. v0.43 adds a Blender-authored harbor backdrop mesh around existing water-edge bands. v0.44 adds a Blender-authored harbor water surface mesh over those water-edge bands. v0.84 adds fallback-generated service-yard cart body/cab/wheel meshes to reduce the visible unit-box vehicle cluster while preserving the authored vehicle collider. v0.85 adds a fallback-generated service-gate mesh to replace the visible gate slab/slat cluster while preserving the authored route collider. v0.87 adds a fallback-generated facade-frame mesh to replace the broad Ferry Office wall and entry-post blockout while preserving authored collision and route space. v0.88 adds a fallback-generated non-text sign-panel mesh for the front facade sign cue. v0.90 adds a tiny visual placeholder state cue beside the Ferry Office Drain Log. v0.93 adds a fallback-generated non-text service-panel mesh for the office-side service/control cue. v0.99+ adds a minimal `targetObjective` gate for target-slice scaffolds so Veyra can prove authored objective/consequence runtime without using `FerryOfficeJob`. The Veyra world-foundation pass adds `tools/world_author.py`; Veyra layout is now compiled from world/area data, and the runtime scene is generated. The Cinder Harbor mesh/material pass adds Veyra-only fallback ground, road-plate, and shore-shelf meshes as generated world-art replacements over the generated terrain/road placeholders. The Cinder Harbor readability pass adds source-authored landmark, risk-site, and route-anchor mesh roles that compile into optional `readabilityRole` values on scene mesh instances.

## Scene Data Location

Primary regression scene data:

```text
data/scenes/ferry_office.scene.json
```

First target-slice scaffold:

```text
data/scenes/veyra_reach_pilot.scene.json
```

Veyra authoring source:

```text
data/worlds/veyra_reach/world.json
data/worlds/veyra_reach/areas/cinder_harbor.area.json
```

See `docs/WORLD_SLICE_AUTHORING.md` for the role split between Ferry Office as `regression-testbed` and new Veyra Reach target slices as `target-slice-scaffold`.

Required tools:

```powershell
python tools/scene_report.py
python tools/validate_scene.py
python tools/validate_assets.py
python tools/scale_audit.py
python tools/mesh_report.py
python tools/world_slice_report.py
python tools/runtime_scene_smoke.py
python tools/target_slice_objective_qa.py
python tools/world_author.py --check
```

## Scene Data Contract

The Ferry Office scene file describes:

- scene id and display name,
- units and axis conventions,
- floor height,
- player start,
- scale reference values,
- static box colliders,
- visual placeholder boxes,
- static mesh assets and mesh instances,
- interactable markers,
- traversal affordances,
- vehicle spawn and service-yard bounds,
- dock road visual placeholders and road-test route markers,
- first driver/fixer job service-run marker and checkpoint,
- route markers,
- objective markers.

New target-slice scaffolds should start smaller. Veyra is no longer authored primarily as a one-off scene JSON: change its terrain patches, roads, collision, landmarks, interactables, route markers, objective markers, mesh/material world-art pass, readability pass, `targetObjective`, and `targetActionResponse` in `data/worlds/veyra_reach`, then regenerate the runtime scene. Future target slices should follow that split when they need map-like world data.

Generated Veyra `meshInstances` may include optional `readabilityRole` metadata. Current allowed world-authoring roles are `landmark`, `risk-site`, and `route-anchor`. These roles are for authoring, preview/reporting, and capture readability; they do not add AI, mission scripting, or collision behavior.

Use stable lowercase kebab-case ids. Do not rely on runtime add-order numeric ids.

## Runtime Loading

Default runtime scene:

```powershell
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi
```

Explicit runtime scene path:

```powershell
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --scene data\scenes\ferry_office.scene.json
```

Loaded from scene data in v0.15:

- floor height and player start,
- static colliders,
- visual placeholders,
- mesh assets and mesh instances,
- interactables,
- traversal affordances,
- service-yard vehicle spawn, enter radius, proxy half extents, and bounds,
- service-run confirmation interactable and checkpoint marker,
- route markers and objective markers.

Still scene-owned C++ behavior:

- world-state flag effects,
- interaction result handling,
- traversal completion side effects,
- objective text and completion summary,
- dynamic state colors and vehicle camera/update behavior.

Target-slice scaffolds now have a neutral runtime smoke path. When a scene declares `sliceMetadata.kind=target-slice-scaffold`, runtime playtest text must identify the scene role and report neutral counts instead of showing Ferry Office objective/job rows. These scenes also do not inherit the fallback service-yard vehicle unless they author a vehicle entry.

Target-slice scaffolds can declare a single `targetObjective` object for the current runtime gate. Required fields:

- `id`,
- `objectiveText`,
- `debugObjectiveText`,
- `completionInteractableName`,
- `completionEventText`.

Optional `completionSummary` can add a short consequence tag. `completionInteractableName` must match an authored interactable `name`; `tools/validate_scene.py` rejects missing links. Use `tools/target_slice_objective_qa.py` when the objective must prove recorded live-like focus/interact/completion evidence. This is not a mission framework, branching quest system, or world-flag scripting model.

## Adding Or Moving Objects

1. Edit the appropriate section in `data/scenes/ferry_office.scene.json`.
2. Run:

```powershell
python tools/validate_scene.py
python tools/scene_report.py
python tools/scale_audit.py
python tools/mesh_report.py
```

3. Run `scripts/verify.ps1` before claiming success.
4. For layout/runtime changes, run a short GDI pass and confirm the object moved in-game.

## Colliders

Use `colliders` for gameplay/static collision intent.

Required fields:

- `id`,
- `kind`,
- `center`,
- `halfExtents`,
- `blocksPlayer`.

For route-changing blockers, document the controlling flag with `stateFlag` and `blocksWhenFlagFalse`. Runtime `PrototypeScene` syncs these authored AABBs from `WorldState`: when `blocksWhenFlagFalse` is true, the collider blocks until the named flag is set. v0.95 uses this for `low-dock-drain-flood-barrier`, which opens after `stormPumpTicketClosed`.

## Visual Placeholders

Use `visualPlaceholders` for debug presentation shapes that are not gameplay colliders.

Required fields:

- `id`,
- `role`,
- `center`,
- `halfExtents`,
- `colorKey`.

Do not treat these as final meshes. They are authoring markers and visual composition placeholders.

## Scene Materials

Use `sceneMaterials` to define the current presentation meaning for every `colorKey` used by visual placeholders and mesh instances.

Required fields:

- `key`,
- `response`,
- `baseColor`.

Allowed `response` values are `wet`, `matte`, and `painted`. `baseColor` uses normalized `[r, g, b, a]` values in `0..1`.

This is not a renderer material system yet. It is authored scene data that feeds `ScenePresentation` so flat placeholder geometry can still carry surface intent such as wet dock road, matte concrete, or painted service signage. `tools\validate_scene.py` fails when a used `colorKey` has no matching `sceneMaterials` preset.

## Mesh Assets And Instances

Use `meshAssets` for source files and legal/provenance metadata.

Required fields:

- `id`,
- `path`,
- `format`,
- `units`,
- `upAxis`,
- `license`,
- `provenance`.

Use `meshInstances` for scene placement.

Required fields:

- `id`,
- `assetId`,
- `position`,
- `yawDegrees`,
- `scale`.

Optional fields can link the instance to existing debug data:

- `replacesVisualPlaceholderId`,
- `linkedColliderId`,
- `colorKey`.

v0.12-v1.00 supports only tiny original `.gltf` placeholder assets through the static mesh spike. The current Ferry Office prop/surface/backdrop set uses `unit_box.gltf`, the v0.18 service-road prop kit (`service_road_sign.gltf`, `road_edge_post.gltf`, `service_barrier.gltf`, `utility_box.gltf`), the v0.20 fallback-generated `ferry_notice_board.gltf`, the v0.20.1 Blender-exported `blender_ferry_notice_board.gltf`, the v0.27 Blender-exported `blender_cable_reel.gltf`, the v0.42 Blender-exported `blender_wet_road_surface.gltf`, the v0.43 Blender-exported `blender_harbor_backdrop.gltf`, the v0.44 Blender-exported `blender_harbor_water_surface.gltf`, the v0.81 fallback-generated `ferry_office_canopy.gltf`, the v0.84 fallback-generated service-yard cart body/cab/wheel meshes, the v0.85 fallback-generated `ferry_office_service_gate.gltf`, the v0.87 fallback-generated `ferry_office_facade_frame.gltf`, the v0.88 fallback-generated `ferry_office_sign_panel.gltf`, and the v0.93 fallback-generated `ferry_office_service_panel.gltf`. Cinder Harbor additionally uses Veyra-only fallback meshes `cinder_harbor_ground_patch.gltf`, `cinder_harbor_road_plate.gltf`, and `cinder_harbor_shore_shelf.gltf` generated from `tools/create_simple_prop_gltf.py` and referenced through `data/worlds/veyra_reach`. Do not add `.glb`, external buffers, materials, textures, animation, mesh collision, or imported third-party art through this scene format yet.

Asset workflow validation now expects every committed `.gltf` under `assets/models` to be referenced by scene data and documented with license/provenance. Use `python tools/validate_assets.py` before and after adding mesh files.

For Veyra, use `python tools/validate_assets.py data/scenes/veyra_reach_pilot.scene.json` and `python tools/mesh_report.py data/scenes/veyra_reach_pilot.scene.json`; Ferry Office is not expected to reference Cinder Harbor-only mesh assets.

## Interactables

Use `interactables` for focusable debug actions.

Required fields:

- `id`,
- `name`,
- `prompt`,
- `type`,
- `position`,
- `radius`.

Optional action-binding fields can record simple world-state effects:

- `worldFlagsSet`: flags set immediately when the interaction fires.
- `requiredWorldFlags`: flags that must already be true before ready-state flags fire.
- `worldFlagsSetWhenReady`: flags set only after every `requiredWorldFlags` entry is true.

Keep action meaning scene-owned; do not make `InteractionSystem` know Tidebreak-specific flags. These bindings are for small boolean world-state beats only; richer job behavior still belongs in C++ until a more stable data shape emerges.

## Traversal Affordances

Use `traversalAffordances` for movement access gates.

Required fields:

- `id`,
- `name`,
- `prompt`,
- `type`,
- `startPosition`,
- `endPosition`,
- `focusRadius`,
- `requiredFacingDirection`,
- `requiredFacingDot`,
- `durationSeconds`.

Only `vault` exists right now. Do not add new traversal types in scene data before code supports them.

## Vehicle Spawn And Bounds

Use `vehicles` for service-yard test vehicle data.

Required fields:

- `id`,
- `name`,
- `spawn.position`,
- `spawn.yawDegrees`,
- `proxyHalfExtents`,
- `enterRadius`,
- `bounds.min`,
- `bounds.max`.

Vehicle movement is still deterministic placeholder code. Jolt VehicleConstraint and real vehicle collision are deferred.

The service-yard vehicle bounds include the original yard plus the short dock road and turn-around marker: `[3.35, -5.05]..[19.45, 0.95]` in X/Z. In v0.15 these values are loaded by `SandboxLayer` at runtime.

## First Driver/Fixer Job Markers

v0.16 keeps rich job behavior in C++ while authoring the current job markers in scene data. v0.62 adds simple scene action bindings for boolean state beats and prerequisites:

- `service-run-confirm-marker`: an `interactables` entry used to review/confirm the Ferry Office Service Call after the required loop. Its authored prompt/message should stay status-neutral because job readiness is still enforced by `FerryOfficeJob`, while scene data mirrors the required flags for validation and simple binding.
- `service-run-checkpoint-marker`: an `objectiveMarkers` entry used by `FerryOfficeJob` as the vehicle checkpoint position.
- `route-dock-road-to-service-confirm`: a `routeMarkers` entry that makes the final dock-road-to-confirmation path visible to Codex tools and debug rendering.

Do not turn these entries into a generic mission scripting format yet. New job beats should stay explicit, small, and backed by tests.

Traversal affordance prompts should describe the action only, for example `Vault Service Barrier`. `SandboxLayer` adds the input prefix for playtest text, so scene data should not include `Press Space` in traversal prompt strings.

## Dock Road Segment

Use `visualPlaceholders` for road pads, shore/water cues, edge rails, curbs, bollards, and end markers. Use `routeMarkers` for Codex-readable route intent such as `route-service-yard-to-dock-road`. The road is an authored placeholder route, not a terrain system, road spline, traffic path, or full map.

v0.18 adds mesh-backed dock/service-road cues on top of that placeholder layout:

- `mesh-service-road-sign`,
- `mesh-dock-road-edge-post-a`,
- `mesh-dock-road-edge-post-b`,
- `mesh-service-yard-barrier-cue`,
- `mesh-maintenance-utility-box`.
- `mesh-ferry-notice-board`.

These props are visual identity anchors. Keep their ids stable unless tests and docs are updated together.

v0.42 adds three wet-road surface overlays on top of the largest road/pad placeholders:

- `mesh-service-yard-wet-surface` replaces `service-yard-driving-pad` visually.
- `mesh-dock-road-wet-surface` replaces `dock-road-segment` visually.
- `mesh-dock-road-turnaround-wet-surface` replaces `dock-road-turnaround-pad` visually.

These are shallow visual surface meshes only. They do not replace colliders, vehicle bounds, road physics, terrain, or material/texture systems.

v0.43 adds three harbor backdrop overlays:

- `mesh-dock-road-harbor-backdrop` behind the dock-road water edge.
- `mesh-dock-start-left-harbor-backdrop` along the left dock-start water band.
- `mesh-dock-start-right-harbor-backdrop` along the right dock-start water band.

These are distant composition silhouettes only. They do not replace water, terrain, colliders, vehicle bounds, streaming map content, or world boundaries.

v0.44 adds three harbor water surface overlays:

- `mesh-water-left-surface` replaces `water-left-band` visually.
- `mesh-water-right-surface` replaces `water-right-band` visually.
- `mesh-dock-road-water-surface` replaces `dock-road-water-edge` visually.

These are static visual surface meshes only. They do not replace water simulation, fluid physics, transparency, terrain, colliders, vehicle bounds, or renderer material systems.

## Avoiding Layout Drift

After v0.15, new layout edits should start in `data/scenes/ferry_office.scene.json`. Do not reintroduce duplicate placement constants in C++ unless a runtime behavior genuinely needs a named fallback or special-case dynamic rule.

Remaining drift risks:

- `FerryOfficeData` still contains stable names and fallback positions for behavior/tests.
- `PrototypeScene` now applies simple scene-authored action bindings for known flag names, but it still owns richer job helpers, traversal completion, service-gate collider sync, objective text, and vehicle checkpoint behavior.
- `SandboxLayer` still owns dynamic coloring, vehicle camera behavior, exit safety, and fallback values.
- There is no editor, prefab system, runtime scene reload, or schema file beyond code and Python validation.

## Definition Of Done For Scene Edits

- `python tools/validate_scene.py` passes.
- `python tools/validate_assets.py` passes when mesh assets are involved.
- `python tools/scene_report.py` summarizes the expected ids/counts.
- `python tools/scale_audit.py` reports no surprising scale issues, or the issue is documented.
- `python tools/mesh_report.py` reports expected mesh asset usage when mesh references are involved.
- `scripts/verify.ps1` passes.
- `docs/STATUS.md` records commands and results.
