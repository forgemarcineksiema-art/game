# Blender Workflow

Last updated: 2026-05-16

This document records the controlled Blender-to-Tidebreak static prop and surface workflow spikes.

## Current Result

v0.20 result: Blender was not available in the Codex environment:

```powershell
blender --version
```

Result: `blender` was not recognized as a command in PATH.

Because of that, v0.20 does not claim a successful Blender export. The milestone adds an optional Blender checker, a documented fallback generator, one fallback-generated original prop, and validation coverage so the next Blender attempt has a clear path.

v0.20.1 result: Blender is available from PATH after the laptop restart:

```text
C:\Program Files\Blender Foundation\Blender 5.1\blender.EXE
Blender 5.1.1
```

The real headless workflow now exists:

```powershell
blender --background --python tools\blender\create_tidebreak_notice_board.py
```

It creates `assets/models/blender_ferry_notice_board.gltf`, a project-original procedural notice-board prop.

v0.27 adds a second controlled Blender script:

```powershell
blender --background --python tools\blender\create_tidebreak_cable_reel.py
```

It creates `assets/models/blender_cable_reel.gltf`, a small project-original procedural service-yard cable reel prop.

v0.42 adds a third controlled Blender script:

```powershell
blender --background --python tools\blender\create_tidebreak_wet_road_surface.py
```

It creates `assets/models/blender_wet_road_surface.gltf`, a low project-original procedural wet-road surface used as visual presentation geometry for the service yard, dock road, and turn-around pads. It is not collision, terrain, or road physics.

## Check Blender

Run:

```powershell
python tools/check_blender.py
```

This command exits successfully even when Blender is unavailable because Blender is not required for normal validation yet. To make a local setup fail if Blender is missing, run:

```powershell
python tools/check_blender.py --require
```

If Blender is installed outside PATH, pass the executable name or path:

```powershell
python tools/check_blender.py --command "C:\Program Files\Blender Foundation\Blender 4.3\blender.exe"
```

## Current Blender Path

Blender-authored static props should follow this discipline:

1. Model a simple original prop in meters.
2. Use Y-up and keep +Z as the meaningful forward direction where relevant.
3. Apply transforms before export.
4. Avoid textures, materials, animation, armatures, hidden duplicate meshes, and external dependencies.
5. Export to the current supported `.gltf` subset if possible.
6. Add a scene `meshAssets` entry with license and provenance.
7. Add one or more `meshInstances`.
8. Run the full asset validation commands.

The first real command is:

```powershell
blender --background --python tools\blender\create_tidebreak_notice_board.py
```

The current second proof prop command is:

```powershell
blender --background --python tools\blender\create_tidebreak_cable_reel.py
```

The current surface-piece command is:

```powershell
blender --background --python tools\blender\create_tidebreak_wet_road_surface.py
```

Blender 5.1.1 does not expose direct `GLTF_EMBEDDED` export in this environment. The script uses `GLTF_SEPARATE`, embeds the generated `.bin` buffer into the `.gltf`, and deletes the temporary `.bin`. Keep that post-export step small and deterministic; move to cgltf/tinygltf if broader Blender output is needed.

## v0.20 Fallback Helper

v0.20 adds:

```powershell
python tools/create_simple_prop_gltf.py --output assets/models/ferry_notice_board.gltf --overwrite
```

This creates a tiny original `ferry_notice_board.gltf` in the current embedded-buffer `.gltf` subset. It is useful for keeping Codex prop iteration repeatable while Blender is unavailable.

Important: this fallback file is not a Blender export. Its scene provenance says so explicitly.

The fallback remains useful as a known-good generator, but the preferred proof prop after v0.20.1 is `assets/models/blender_ferry_notice_board.gltf`.

## Validate Assets

After adding or changing a prop:

```powershell
python tools/validate_scene.py
python tools/validate_assets.py
python tools/mesh_report.py
python tools/scale_audit.py
powershell -ExecutionPolicy Bypass -File scripts/verify.ps1
```

`tools/validate_assets.py` should catch unsupported `.glb`, external buffers, unreferenced `.gltf` files, missing license/provenance, duplicate mesh ids, and unknown scene mesh references.

## Current Supported Subset

The runtime still supports only:

- `.gltf` JSON files,
- one embedded base64 buffer,
- one primitive with `POSITION` and `indices`,
- float `VEC3` positions,
- indexed triangle lists,
- flat-tinted renderer submission.

Still unsupported:

- `.glb`,
- external `.bin` buffers,
- textures/materials/PBR,
- normals/UV renderer behavior,
- animation/skeletal meshes,
- mesh collision import,
- asset registry/resource cache/hot reload.

## When To Switch Loader

Switch from the custom tiny loader to cgltf or tinygltf when Blender-authored assets cannot be kept inside the current subset without fragile normalizers, or when Tidebreak needs GLB, external buffers, normals, UVs, materials, multiple primitives/nodes, or better glTF error reporting.

The preferred next loader candidate remains cgltf; tinygltf is the backup. Assimp remains too broad for this stage.
