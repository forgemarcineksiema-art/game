# Scene Authoring

Last updated: 2026-05-15

v0.11 introduced scene data for Codex-friendly inspection and validation. v0.12 added mesh asset and mesh instance references. v0.12.1 expanded the Ferry Office mesh set for a focused prop/scale pass. v0.14 added the first dock road segment as authored placeholders and route markers. v0.15 makes `data/scenes/ferry_office.scene.json` the runtime source of truth for current layout data while keeping Tidebreak-specific behavior in C++.

## Scene Data Location

Primary scene data:

```text
data/scenes/ferry_office.scene.json
```

Required tools:

```powershell
python tools/scene_report.py
python tools/validate_scene.py
python tools/scale_audit.py
python tools/mesh_report.py
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
- route markers,
- objective markers.

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
- route markers and objective markers.

Still scene-owned C++ behavior:

- world-state flag effects,
- interaction result handling,
- traversal completion side effects,
- objective text and completion summary,
- dynamic state colors and vehicle camera/update behavior.

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

For route-changing blockers, document the controlling flag with fields such as `stateFlag` and `blocksWhenFlagFalse`.

## Visual Placeholders

Use `visualPlaceholders` for debug presentation shapes that are not gameplay colliders.

Required fields:

- `id`,
- `role`,
- `center`,
- `halfExtents`,
- `colorKey`.

Do not treat these as final meshes. They are authoring markers and visual composition placeholders.

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

v0.12/v0.12.1 supports only tiny original `.gltf` placeholder assets through the static mesh spike. The current Ferry Office prop pass intentionally reuses `unit_box.gltf` for multiple authored instances. Do not add materials, textures, animation, mesh collision, or imported third-party art through this scene format yet.

## Interactables

Use `interactables` for focusable debug actions.

Required fields:

- `id`,
- `name`,
- `prompt`,
- `type`,
- `position`,
- `radius`.

Optional fields can document world-state effects such as `worldFlagsSet` or `worldFlagsSetWhenReady`. Keep action meaning scene-owned; do not make `InteractionSystem` know Tidebreak-specific flags.

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

## Dock Road Segment

Use `visualPlaceholders` for road pads, shore/water cues, edge rails, curbs, bollards, and end markers. Use `routeMarkers` for Codex-readable route intent such as `route-service-yard-to-dock-road`. The road is an authored placeholder route, not a terrain system, road spline, traffic path, or full map.

## Avoiding Layout Drift

After v0.15, new layout edits should start in `data/scenes/ferry_office.scene.json`. Do not reintroduce duplicate placement constants in C++ unless a runtime behavior genuinely needs a named fallback or special-case dynamic rule.

Remaining drift risks:

- `FerryOfficeData` still contains stable names and fallback positions for behavior/tests.
- `PrototypeScene` still maps known interaction names to world-state flags.
- `SandboxLayer` still owns dynamic coloring, vehicle camera behavior, exit safety, and fallback values.
- There is no editor, prefab system, runtime scene reload, or schema file beyond code and Python validation.

## Definition Of Done For Scene Edits

- `python tools/validate_scene.py` passes.
- `python tools/scene_report.py` summarizes the expected ids/counts.
- `python tools/scale_audit.py` reports no surprising scale issues, or the issue is documented.
- `python tools/mesh_report.py` reports expected mesh asset usage when mesh references are involved.
- `scripts/verify.ps1` passes.
- `docs/STATUS.md` records commands and results.
