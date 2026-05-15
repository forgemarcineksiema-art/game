# Asset Pipeline Decision

Last updated: 2026-05-15

## Decision

For the next short prototype phase, Tidebreak should keep the current tiny custom `.gltf` loader and stabilize the surrounding workflow:

- scene-authored `meshAssets` and `meshInstances`,
- explicit license/provenance per mesh asset,
- `tools/validate_assets.py`,
- `tools/mesh_report.py`,
- `tools/validate_scene.py`,
- `tools/scale_audit.py`,
- standard-library Python asset checks,
- original simple `.gltf` placeholder meshes under `assets/models`.

Do not switch to cgltf, tinygltf, or Assimp in v0.19.

The preferred next real loader candidate is `cgltf` once Tidebreak needs broader glTF coverage such as GLB, external buffers, Blender-authored props with more varied accessors, normals, UVs, materials, or a stricter import path. `tinygltf` remains a credible backup if a C++ header-only model and its JSON/STB dependency surface become more useful than cgltf's smaller C-style integration. Do not choose Assimp for the near-term prototype unless the project intentionally needs many non-glTF formats.

## Why Keep The Tiny Loader Briefly

The current runtime goal is not "load every art file"; it is to make the Ferry Office / dock road prototype legible while keeping Codex iteration safe. The existing loader already supports the current committed props:

- `.gltf` JSON,
- embedded `data:application/octet-stream;base64` buffer,
- one mesh primitive,
- `POSITION` float `VEC3`,
- indexed triangle list,
- computed or authored bounds,
- flat-tinted renderer submission.

The fragile part was not yet third-party glTF coverage. The fragile part was workflow discipline: knowing which files exist, which ones are referenced, whether provenance is present, whether unsupported formats slipped in, and whether future Codex runs can audit the scene without reading every C++ callsite.

v0.19 fixes that workflow first.

## Comparison

| Option | Fit Now | Commercial/License | Build Risk | Why / Why Not |
| --- | --- | --- | --- | --- |
| Current tiny custom loader | Best short-term fit | Project-owned | None | Already builds and loads current original props. Good while assets remain tiny, embedded-buffer placeholders. Must stay explicitly narrow. |
| cgltf | Best next loader candidate | MIT | Low/medium | Single-file C99 glTF 2.0 loader/writer with no external dependency. Supports GLB and broader glTF concepts, but still requires Tidebreak-owned mesh/material mapping. Good when current subset becomes too small. |
| tinygltf | Credible backup | MIT plus bundled permissive third-party headers | Medium | Header-only C++11 glTF library with ASCII and binary loading. More dependency surface (`json.hpp`, STB, base64) than cgltf, but ergonomic for C++. |
| Assimp | Not recommended now | 3-clause BSD style | Higher | Broad importer for many 3D formats. Useful if the project deliberately needs many formats, but too wide for a custom engine still defining one tiny static mesh workflow. |
| Blender-export-first without loader change | Good authoring discipline, incomplete runtime answer | Depends on authored assets | Low | Useful now as convention, but Blender-authored GLB/external-buffer/material output will quickly exceed the current loader unless exports are constrained. |

Sources reviewed:

- cgltf repository: https://github.com/jkuhlmann/cgltf
- tinygltf repository: https://github.com/syoyo/tinygltf
- Assimp repository: https://github.com/assimp/assimp
- Blender glTF 2.0 manual: https://docs.blender.org/manual/en/latest/addons/import_export/scene_gltf2.html

## Current Supported Subset

Tidebreak v0.19 supports this static mesh subset:

- source files live under `assets/models`,
- file extension must be `.gltf`,
- `.glb` is rejected by asset validation,
- buffers must be embedded base64 data URIs,
- external `.bin` buffers are rejected by asset validation,
- mesh must contain a primitive with `POSITION` and `indices`,
- `POSITION` must provide count and bounds, or use the currently supported embedded float data path to compute bounds,
- mesh assets must be scene-referenced with `license` and `provenance`,
- mesh instances must reference known scene asset ids.

Unsupported in v0.19:

- GLB,
- external buffers,
- textures,
- materials/PBR,
- renderer normals/UVs,
- mesh collision import,
- skeletal meshes,
- animation,
- morph targets,
- node hierarchy transforms beyond the scene instance transform,
- asset registry,
- resource cache,
- hot reload,
- editor.

## Source Of Truth

For now:

- `assets/models/*.gltf` are source mesh files.
- `data/scenes/ferry_office.scene.json` is the source of truth for mesh asset ids, provenance, and scene instances.
- Runtime `SceneLoader` loads scene data.
- `SandboxLayer` loads scene-authored mesh asset ids into a local static mesh map.
- Renderers receive immediate flat triangle submissions.

There is no cooker, registry, material database, or generated asset manifest yet.

## Required Codex Workflow

When adding or changing static mesh props:

1. Add only original `.gltf` files under `assets/models`.
2. Add or update `meshAssets` entries with `license` and `provenance`.
3. Add or update `meshInstances` entries with unique ids and valid transforms.
4. Run:

```powershell
python tools/validate_scene.py
python tools/validate_assets.py
python tools/mesh_report.py
python tools/scale_audit.py
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
```

5. Record results in `docs/STATUS.md`.

`tools/mesh_report.py` should show every `.gltf` under `assets/models`, whether it is referenced by scene data, vertex/index counts, bounds, license/provenance, and parse errors.

`tools/validate_assets.py` should fail for unreferenced committed `.gltf` files, unsupported `.glb` files, external buffers, missing provenance, duplicate scene mesh ids, and unknown instance asset ids.

## Triggers To Switch To cgltf Or tinygltf

Switch away from the custom loader when one or more of these becomes necessary:

- Blender-authored assets must be imported without hand-constraining every buffer.
- GLB becomes the preferred source format.
- external `.bin` buffers are needed.
- normals/UVs/materials are needed in runtime rendering.
- multiple primitives/nodes per asset matter.
- exporter variance causes repeated custom-loader bugs.
- error reporting needs to match real glTF validation more closely.
- asset work becomes blocked by the custom subset instead of by visual direction.

The likely migration path is:

1. Add cgltf behind `src/engine/assets`, with third-party types hidden in the loader implementation.
2. Keep `StaticMeshAsset` and `StaticMeshInstance` as engine-owned boundaries.
3. Preserve current scene-data `meshAssets` / `meshInstances`.
4. Add loader tests against the existing original props plus one Blender-exported test prop.
5. Only then add normals/UV/material fields as separate, explicit renderer milestones.

## What This Decision Does Not Do

This decision does not make the current visuals production quality. It only prevents the asset workflow from becoming accidental chaos while the renderer and art direction are still young.

It also does not block Blender. Blender remains the intended DCC direction; v0.19 simply says that the next Blender step should be a narrow workflow spike, not a silent expansion of the runtime loader in the middle of unrelated gameplay work.

## v0.20 Blender Workflow Update

The v0.20 spike checked Blender honestly and found that `blender` is not available in the current PATH. Because of that, the project does not claim a successful Blender export yet.

v0.20 adds:

- `tools/check_blender.py` for optional DCC availability checks,
- `tools/create_simple_prop_gltf.py` as a clearly labeled fallback generator,
- `assets/models/ferry_notice_board.gltf` as project-original fallback geometry, not Blender export,
- `docs/BLENDER_WORKFLOW.md` with the exact current blocker and next intended workflow.

This does not change the v0.19 recommendation. Keep the tiny loader briefly, but the first real Blender install/export pass should happen soon. If Blender produces `.gltf` files that require external buffers, GLB, multiple primitives, normals/UVs, or material dependencies, prefer stabilizing cgltf next instead of growing a fragile custom parser.
