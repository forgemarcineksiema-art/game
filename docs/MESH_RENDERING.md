# Static Mesh Rendering

Last updated: 2026-05-16

v0.12 adds the first narrow static mesh path. v0.19 stabilizes the surrounding asset workflow, v0.20 proves the Blender gap plus a small fallback prop workflow, v0.20.1 proves one real headless Blender export, v0.27 adds a tiny painter-depth presentation spike plus one more Blender prop, v0.39 moves scene palette/shading rules into a game-layer presentation boundary, v0.40 gives scene color keys tiny material-like shading presets, v0.41 authors those presets in scene data, and v0.42 adds a Blender-authored wet-road surface mesh for the current service-yard and dock-road pads. This remains a render spike and authoring bridge, not a full asset pipeline.

## What Exists

- `src/engine/assets/StaticMesh.*` defines engine-owned mesh types:
  - `StaticMeshVertex`,
  - `StaticMeshAsset`,
  - `StaticMeshInstance`,
  - `Bounds3`,
  - `LoadStaticMeshFromGltf`,
  - `BuildFlatTriangleList`.
- `IRenderer::drawDebugFlatTriangles` submits immediate flat-colored triangle lists.
- DX11 now renders debug solid boxes and flat mesh triangles through a real world-to-clip matrix path with a depth buffer. Lines, wire boxes, grid/axes, and debug text still use the existing debug projection / overlay path.
- `src/game/ScenePresentation.*` consumes `sceneMaterials` from the loaded scene, applies dynamic palette state, and owns fixed overcast face shading before scene boxes and static mesh triangles are submitted. It derives shading from triangle normals and authored color keys; it is presentation shading only, not a renderer lighting/material system.
- GDI renders projected triangle polygons as a fallback.
- Null renderer accepts the call and counts it for smoke/test visibility.
- `assets/models/unit_box.gltf` is a tiny original project-owned placeholder mesh.
- v0.18 adds four more original tiny static prop meshes: `service_road_sign.gltf`, `road_edge_post.gltf`, `service_barrier.gltf`, and `utility_box.gltf`.
- v0.20 adds `ferry_notice_board.gltf` through a project fallback generator because Blender is not available in the current environment.
- v0.20.1 adds `blender_ferry_notice_board.gltf` through a real headless Blender 5.1.1 script.
- v0.27 adds `blender_cable_reel.gltf` through a second real headless Blender 5.1.1 script.
- v0.42 adds `blender_wet_road_surface.gltf` through a third real headless Blender 5.1.1 script.
- `data/scenes/ferry_office.scene.json` now has `meshAssets` and `meshInstances`.
- v0.12.1 uses that single unit-box asset for a small prop kit: Ferry Office roof/facade/sign, service gate, maintenance box, dock bollards, service-yard crate, and service-yard vehicle body/cabin.
- v0.18 uses multiple scene-authored mesh asset ids for the first service-road prop style pass. `SandboxLayer` loads the authored mesh assets into a local `assetId -> StaticMeshAsset` map before drawing mesh instances.
- v0.19 adds `tools/validate_assets.py` and expands `tools/mesh_report.py` so Codex can audit every committed `.gltf`, reference status, vertex/index counts, bounds, license/provenance, and unsupported file errors.
- v0.20.1 proves that Blender's `GLTF_SEPARATE` output can be post-embedded into the current one-buffer loader subset for one controlled procedural prop.

## Supported glTF Subset

The v0.12 loader supports only a tiny static glTF subset:

- `.gltf` JSON files,
- one embedded `data:application/octet-stream;base64` buffer,
- one primitive with `POSITION` and `indices`,
- `POSITION` as `FLOAT` `VEC3`,
- indices as `UNSIGNED_SHORT` or `UNSIGNED_INT`,
- triangle-list mode.

It does not support:

- `.glb`,
- external buffer files,
- materials,
- textures,
- normals as renderer inputs,
- UVs,
- skinning,
- morphs,
- animations,
- cameras or lights,
- mesh collision.

`tools/validate_assets.py` deliberately fails on `.glb`, external buffers, unreferenced committed `.gltf` files, missing license/provenance, and invalid scene mesh references. This keeps the current custom loader honest until a later cgltf/tinygltf migration is justified.

## Scene Data Connection

Scene mesh data is authored in:

```text
data/scenes/ferry_office.scene.json
```

Use:

- `meshAssets` for source files, license/provenance, and authored bounds.
- `sceneMaterials` for color-key base colors and wet/matte/painted response families.
- `meshInstances` for position, yaw, scale, tint/color key, and optional links back to placeholders/colliders.

v0.15 loads these mesh asset and mesh instance records through the runtime scene loader. v0.18 loads every referenced scene mesh asset by id, not just `unit-box-mesh`. v0.41 loads `sceneMaterials` through the same scene source of truth. v0.42 uses one authored wet-road surface mesh instance set to replace the largest service-yard/dock-road placeholder slabs visually while leaving collision and vehicle physics unchanged. `SandboxLayer::drawStaticMeshDebug` still applies moving vehicle body/cabin placement, but `ScenePresentation` now owns color-key, material-preset, and dynamic palette choices such as service-gate, maintenance-power, and vehicle-occupied colors. The authored mesh instance list comes from `data/scenes/ferry_office.scene.json`.

## Renderer Notes

This is not a production renderer yet. v0.28 gives DX11 a first real depth-buffered world-to-clip matrix path for debug solid boxes and flat-mesh triangle submissions. GDI remains on the v0.27 projected painter-depth fallback. DX11 wire/debug lines, grid/axes, and the Win32 text overlay intentionally remain overlay-style debug rendering so validation markers stay readable.

There is still no material system, texture path, lighting, shader file pipeline, renderer-owned static mesh resource lifetime, resize handling, transparency sorting, or production HUD/font renderer. The current DX11 matrix/depth work is a narrow presentation and architecture spike for existing immediate debug geometry.

Keep wire/debug markers visible until a later overlay and asset pipeline exist.

The v0.12.1, v0.18, v0.20, v0.20.1, v0.27, v0.38, and v0.42 mesh instances are still composition placeholders. `ScenePresentation` shades their submitted faces with small wet/matte/painted response presets enough to improve volume readability, but they are not final art, material assets, or collision sources.

## How To Add A Simple Mesh

1. Add an original `.gltf` file under `assets/models`.
2. Keep it in meters, Y-up, and forward along +Z.
3. Add a `meshAssets` entry with `license` and `provenance`.
4. Add one or more `meshInstances`.
5. Run:

```powershell
python tools/validate_scene.py
python tools/validate_assets.py
python tools/mesh_report.py
python tools/scale_audit.py
scripts/verify.ps1
```

Every `.gltf` under `assets/models` should be referenced by scene data. Keep experimental files out of the repo or document them through a `meshAssets` entry with clear provenance.

## Loader Decision

See `docs/ASSET_PIPELINE_DECISION.md` for the v0.19 decision and `docs/BLENDER_WORKFLOW.md` for the v0.20/v0.20.1 workflow spikes. Short version: keep the tiny custom loader briefly, allow only controlled embedded-buffer Blender props, and move to cgltf or tinygltf when real Blender-authored assets, GLB, external buffers, normals, UVs, or materials become necessary.

## Deferred

- Real static mesh resource lifetime.
- Runtime scene hot reload or editing.
- Mesh/material asset registry.
- Texture/material/PBR work.
- GDI depth buffer or world-matrix rendering.
- DX11 resize-safe depth resources and broader resource lifetime stress tests.
- glTF node hierarchy and GLB support.
- Animation and skeletal mesh support.
- Mesh collision or physics import.
