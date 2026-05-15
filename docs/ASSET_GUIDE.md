# Asset Guide

Last updated: 2026-05-15

This guide defines Tidebreak's early asset and scale conventions. v0.11 does not add a full asset pipeline or model loading; it gives future Codex runs a stable target before glTF work begins.

## Units And Axes

- World units are meters.
- Engine vectors use `[x, y, z]`.
- `Y` is up.
- `X` is right.
- `Z` is forward; yaw `0` faces `+Z`.
- Keep authored positions near the origin while the prototype is small. If an object is tens or hundreds of meters away, document why.

## Current Scale References

- Player height: `1.8m`.
- Player radius: `0.35m`.
- Current placeholder service vehicle visual size: about `1.16m` wide, `1.84m` long, `1.06m` high.
- Minimum comfortable on-foot path: about `1.2m`.
- Minimum vehicle test lane: about `3.0m`.
- Ferry Office door/gate openings should be at least `1.1m` wide unless intentionally blocked.

These values are practical prototype references, not final art dimensions.

## Pivot And Origin Rules

- Static props should use a centered pivot unless they are doors/gates.
- Doors and gates should use a hinge-side pivot when a real door system exists.
- Vehicle pivots should be centered near ground contact, with forward along `+Z`.
- Character pivots should be at ground contact under the character center.
- Collider boxes use `center` and `halfExtents`.

## Naming Rules

- Use lowercase kebab-case ids in scene data, for example `service-gate`.
- Display names can use title case, for example `Service Gate`.
- Keep ids stable once referenced by tools, docs, tests, or world-state mappings.
- Do not reuse ids across colliders, visual placeholders, interactables, traversal affordances, vehicles, routes, or objective markers.

## Placeholder Colors

Scene data uses `colorKey` strings, not final material assets. Current intent:

- `dock-weathered-wood`: damp dock boards and pier surfaces.
- `office-muted-concrete`: worn public-service building surfaces.
- `damp-service-concrete`: maintenance-side service pads.
- `deep-harbor-blue`: water-edge bands.
- `weathered-yard-rail`: simple service-yard barriers.
- `rusted-roof-trim`: rusty ferry-office silhouette accents.

Future renderer work can map these keys to material presets. Do not turn color keys into a material system in v0.11.

## Blender Direction

Future authored assets should come from Blender or another legal DCC tool and export through glTF/GLB. Expected export discipline:

- Apply transforms before export.
- Keep object scale near `1.0`.
- Use meaningful object names.
- Keep pivots intentional.
- Avoid hidden duplicate meshes.
- Include simple collision helper names only when the engine has a loader for them.
- Do not export copyrighted meshes, brands, maps, logos, or ripped game assets.

## glTF Direction

v0.12/v0.12.1 supports a tiny static `.gltf` subset through `src/engine/assets/StaticMesh.*`.

- Current committed proof asset: `assets/models/unit_box.gltf`.
- v0.12.1 deliberately reuses that asset for a small Ferry Office prop kit instead of adding new mesh files.
- Use meters, Y-up, +Z forward.
- Keep placeholder meshes small, original, and clearly documented.
- Add `license` and `provenance` for every scene `meshAssets` entry.
- Run `python tools/mesh_report.py` after adding or moving mesh references.

Still deferred:

- `.glb`,
- external buffers,
- textures/materials/PBR,
- broad asset registry,
- mesh collision import,
- skeletal animation assets.

Do not add a broad asset registry.
- Do not add skeletal animation assets.
- Do not add PBR/material complexity.
- Keep scene JSON, mesh instances, and debug geometry as the active authoring bridge.

## What Not To Add Yet

- Final art.
- Model loading.
- Asset cooking.
- Animation runtime.
- Texture/material pipeline.
- Procedural terrain.
- Physics-authored mesh collision.
- Marketplace or copied assets without clear license provenance.
