# Asset Guide

Last updated: 2026-05-16

This guide defines Tidebreak's early asset and scale conventions. v0.44 still does not add a full asset pipeline, but the project now has a tiny static `.gltf` prop/surface/backdrop path for original placeholder meshes, stronger asset workflow validation, an honest Blender availability check, a fallback prop generator, and several controlled Blender-exported proof meshes.

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
- Current placeholder service vehicle visual size: about `1.16m` wide, `1.92m` long, `1.20m` high.
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

## Placeholder Materials

Scene data uses `colorKey` strings plus `sceneMaterials` presets, not final renderer material assets. Current intent:

- `dock-weathered-wood`: damp dock boards and pier surfaces.
- `office-muted-concrete`: worn public-service building surfaces.
- `damp-service-concrete`: maintenance-side service pads.
- `deep-harbor-blue`: water-edge bands.
- `misty-island-ground`: distant low island/harbor backdrop silhouettes.
- `weathered-yard-rail`: simple service-yard barriers.
- `rusted-roof-trim`: rusty ferry-office silhouette accents.
- `ferry-route-sign-blue`: dock-road/ferry-route sign cues.
- `salt-white-road-post`: pale road-edge posts against the dark dock road.
- `warning-service-orange`: low service barriers and practical caution cues.

`sceneMaterials` maps each used key to a normalized base color and a tiny response family: `wet`, `matte`, or `painted`. Future renderer work can promote these presets into richer material assets, but the current path is still flat placeholder geometry with presentation shading only.

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

v0.12-v0.18 supports a tiny static `.gltf` subset through `src/engine/assets/StaticMesh.*`.

- Current committed proof asset: `assets/models/unit_box.gltf`.
- v0.12.1 deliberately reuses that asset for a small Ferry Office prop kit.
- v0.18 adds four original prop-kit meshes: `service_road_sign.gltf`, `road_edge_post.gltf`, `service_barrier.gltf`, and `utility_box.gltf`.
- v0.20 adds `ferry_notice_board.gltf` through `tools/create_simple_prop_gltf.py` because Blender was unavailable during that run. This asset is project-original fallback geometry, not Blender export.
- v0.58 adds `clearance_tag.gltf` through the same fallback helper because the endpoint cue is tiny and does not justify Blender yet.
- v0.81 adds `ferry_office_canopy.gltf` through the same fallback helper because the Ferry Office roofline is a high-visibility first-frame silhouette and benefits from one cohesive authored mesh.
- v0.84 adds `service_yard_cart_body.gltf`, `service_yard_cart_cabin.gltf`, and `service_yard_cart_wheel.gltf` through the same fallback helper because the service vehicle was still a high-visibility unit-box cluster in the playable slice.
- v0.85 adds `ferry_office_service_gate.gltf` through the same fallback helper because the service gate was still a high-visibility unit-box slab/slat cluster in the playable slice.
- v0.87 adds `ferry_office_facade_frame.gltf` through the same fallback helper because the surrounding Ferry Office wall and entry-post pair still read as broad unit-box blockout around the improved gate/canopy.
- v0.20.1 adds `blender_ferry_notice_board.gltf` through `tools/blender/create_tidebreak_notice_board.py`. v0.27 adds `blender_cable_reel.gltf` through `tools/blender/create_tidebreak_cable_reel.py`. v0.42 adds `blender_wet_road_surface.gltf` through `tools/blender/create_tidebreak_wet_road_surface.py`. v0.43 adds `blender_harbor_backdrop.gltf` through `tools/blender/create_tidebreak_harbor_backdrop.py`. v0.44 adds `blender_harbor_water_surface.gltf` through `tools/blender/create_tidebreak_harbor_water_surface.py`. These scripts author in Tidebreak coordinates, map to Blender's Z-up space, export with Blender 5.1.1, and post-embed the buffer for the current loader subset.
- Use meters, Y-up, +Z forward.
- Keep placeholder meshes small, original, and clearly documented.
- Add `license` and `provenance` for every scene `meshAssets` entry.
- Run `python tools/validate_assets.py` and `python tools/mesh_report.py` after adding or moving mesh references.
- Every committed `.gltf` under `assets/models` should be referenced by scene data. Keep work-in-progress exports outside the repo until they are ready to be documented and validated.

Still deferred:

- `.glb`,
- external buffers,
- textures/materials/PBR,
- broad asset registry,
- mesh collision import,
- skeletal animation assets.

Near-term decision: keep the custom tiny `.gltf` loader while the project stabilizes naming, scale, provenance, validation, and the first DCC workflow. Switch to cgltf or tinygltf when Blender-authored assets cannot stay inside the current subset or when GLB, external buffers, normals, UVs, or materials become necessary. See `docs/ASSET_PIPELINE_DECISION.md` and `docs/BLENDER_WORKFLOW.md`.

## What Not To Add Yet

- Final art.
- Asset cooking.
- Animation runtime.
- Texture/material pipeline.
- Procedural terrain.
- Physics-authored mesh collision.
- Marketplace or copied assets without clear license provenance.
- Broad asset registry or resource cache.
