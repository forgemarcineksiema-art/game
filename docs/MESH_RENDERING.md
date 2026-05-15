# Static Mesh Rendering

Last updated: 2026-05-15

v0.12 adds the first narrow static mesh path. It is a render spike and authoring bridge, not a full asset pipeline.

## What Exists

- `src/engine/assets/StaticMesh.*` defines engine-owned mesh types:
  - `StaticMeshVertex`,
  - `StaticMeshAsset`,
  - `StaticMeshInstance`,
  - `Bounds3`,
  - `LoadStaticMeshFromGltf`,
  - `BuildFlatTriangleList`.
- `IRenderer::drawDebugFlatTriangles` submits immediate flat-colored triangle lists.
- DX11 renders those triangles with the same CPU projection path used by solid debug boxes.
- GDI renders projected triangle polygons as a fallback.
- Null renderer accepts the call and counts it for smoke/test visibility.
- `assets/models/unit_box.gltf` is a tiny original project-owned placeholder mesh.
- `data/scenes/ferry_office.scene.json` now has `meshAssets` and `meshInstances`.
- v0.12.1 uses that single unit-box asset for a small prop kit: Ferry Office roof/facade/sign, service gate, maintenance box, dock bollards, service-yard crate, and service-yard vehicle body/cabin.

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

## Scene Data Connection

Scene mesh data is authored in:

```text
data/scenes/ferry_office.scene.json
```

Use:

- `meshAssets` for source files, license/provenance, and authored bounds.
- `meshInstances` for position, yaw, scale, tint/color key, and optional links back to placeholders/colliders.

v0.15 loads these mesh asset and mesh instance records through the runtime scene loader. `SandboxLayer::drawStaticMeshDebug` still applies dynamic state rules, such as service-gate color and moving vehicle body/cabin placement, but the authored mesh instance list now comes from `data/scenes/ferry_office.scene.json`.

## Renderer Notes

This is not a production renderer yet. DX11 has no depth buffer, no real world/view/projection matrix, no material system, and no texture path. Triangles draw in submission order using the current debug projection. That is acceptable for small placeholder props and visual proof, but not for final art.

Keep wire/debug markers visible until a later overlay and asset pipeline exist.

The v0.12.1 mesh instances are still flat-tinted composition placeholders. They improve scale/readability, but they are not final art, material assets, or collision sources.

## How To Add A Simple Mesh

1. Add an original `.gltf` file under `assets/models`.
2. Keep it in meters, Y-up, and forward along +Z.
3. Add a `meshAssets` entry with `license` and `provenance`.
4. Add one or more `meshInstances`.
5. Run:

```powershell
python tools/validate_scene.py
python tools/mesh_report.py
python tools/scale_audit.py
scripts/verify.ps1
```

## Deferred

- Real static mesh resource lifetime.
- Runtime scene hot reload or editing.
- Mesh/material asset registry.
- Texture/material/PBR work.
- Depth buffer and camera matrices.
- glTF node hierarchy and GLB support.
- Animation and skeletal mesh support.
- Mesh collision or physics import.
