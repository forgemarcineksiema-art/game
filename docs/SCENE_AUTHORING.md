# Scene Authoring

Last updated: 2026-05-15

v0.11 introduces scene data for Codex-friendly inspection and validation. v0.12 adds mesh asset and mesh instance references. Runtime behavior still comes from explicit C++ prototype setup. Keep scene data and C++ layout changes in sync until a later goal adds runtime loading or generation.

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
- route markers,
- objective markers.

Use stable lowercase kebab-case ids. Do not rely on runtime add-order numeric ids.

## Adding Or Moving Objects

1. Edit the appropriate section in `data/scenes/ferry_office.scene.json`.
2. If runtime behavior must change before scene loading exists, update the matching C++ constants/setup in the same change.
3. Run:

```powershell
python tools/validate_scene.py
python tools/scene_report.py
python tools/scale_audit.py
python tools/mesh_report.py
```

4. Run `scripts/verify.ps1` before claiming success.

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

v0.12 supports only tiny original `.gltf` placeholder assets through the static mesh spike. Do not add materials, textures, animation, mesh collision, or imported third-party art through this scene format yet.

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

## Avoiding Layout Drift

Until runtime scene loading exists, any change to these areas must update both the scene data and matching C++:

- `src/game/FerryOfficeData.*`
- `src/game/PrototypeWorld.cpp`
- `src/game/PrototypeScene.cpp`
- `src/game/SandboxLayer.cpp`

Future goals should reduce this duplication by either loading scene data at runtime or generating a small C++ data file from the JSON.

## Definition Of Done For Scene Edits

- `python tools/validate_scene.py` passes.
- `python tools/scene_report.py` summarizes the expected ids/counts.
- `python tools/scale_audit.py` reports no surprising scale issues, or the issue is documented.
- `python tools/mesh_report.py` reports expected mesh asset usage when mesh references are involved.
- `scripts/verify.ps1` passes.
- `docs/STATUS.md` records commands and results.
