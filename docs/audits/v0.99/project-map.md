# Tidebreak v0.99 Project Map

## Current Shape

Tidebreak is a Windows-first custom C++20 engine workbench with one authored gameplay prototype: Ferry Office / Dock Road. It has grown from engine skeleton to scene data, immediate-mode renderer, low-poly placeholder asset path, world-state flags, a Ferry Office job helper, deterministic vehicle fallback, and opt-in Jolt vehicle-runtime evidence.

Current branch: `main`.
Audited HEAD: `f5e2fdb v0.99 tune jolt route pace`.

## Engine Core

- `src/engine/application/Application.*`: owns startup, window/renderer creation, update/render loop, frame capture, shutdown.
- `src/engine/core/Config.*`: parses CLI, including `--renderer`, `--scene`, `--ui-mode`, capture, QA, and `--vehicle-runtime`.
- `src/engine/platform/Win32Window.cpp`: Win32 input/window/cursor path.
- `src/engine/renderer/*`: `IRenderer`, Null, GDI, DX11. DX11 has real depth/matrix for solid boxes and flat triangles; GDI is CPU projected/painter-style.
- `src/engine/assets/StaticMesh.*`: tiny embedded-buffer `.gltf` static mesh loader and flat triangle builder.
- `src/engine/physics/*`: vendor-safe physics and vehicle runtime boundary. Jolt stays private to engine code.

Integration risks:

- Renderer surface is still debug/immediate-mode; `drawDebugFlatTriangles` is not a production mesh path.
- `ENGINE_VERSION_STRING` is currently `0.23.0` while docs/status discuss v0.99.
- DX11 is bounded-useful, but on this machine hardware device creation fell back to WARP.

## Game Runtime

- `src/game/SandboxLayer.*`: main integration choke point: scene load, player, camera, vehicle, presentation, world state, QA capture states, debug text, marker drawing.
- `src/game/PrototypeWorld.*`: active static AABB/floor/raycast/collision owner for live gameplay.
- `src/game/PlayerController.*`: camera-relative movement, jump, traversal motion, collision handoff.
- `src/game/ThirdPersonCamera.*`: orbit/follow camera, vehicle yaw follow; no camera obstruction.
- `src/game/VehicleController.*`: deterministic fallback vehicle with enter/exit, finite bounds clamp, camera target, safe exit checks.
- `src/game/FerryOfficeJob.*`: explicit Ferry Office service-call and follow-up policy.
- `src/game/WorldState.*`: in-memory boolean flags and event log.

Integration risks:

- `SandboxLayer.cpp` is large and owns too many runtime seams.
- `FerryOfficeJob.cpp` has hardcoded route/objective sequence for a long follow-up chain.
- The current gameplay evidence does not simulate full keyboard/mouse navigation.

## Scene And Assets

- `data/scenes/ferry_office.scene.json`: default source for scene materials, colliders, placeholders, mesh assets/instances, interactables, traversal, vehicle, routes, objectives.
- `tools/scene_report.py`: reported 25 scene materials, 10 colliders, 32 visual placeholders, 20 mesh assets, 66 mesh instances, 17 interactables, 17 route markers, 16 objective markers.
- `tools/validate_scene.py`, `validate_assets.py`, `scale_audit.py`, `mesh_report.py`: passed in this session.
- `assets/models/*`: 20 referenced `.gltf` model files; `unit_box.gltf` still has 25 uses.

Integration risks:

- Scene JSON is default truth, but explicit scene-load failure falls back to built-in Ferry Office setup.
- Mesh assets are validated and original, but still tiny-subset placeholders.
- There is no material, texture, asset registry, GPU resource lifetime, editor, prefab system, or hot reload.

## QA Surface

- Default gate: `scripts/verify.ps1`; passed in this session.
- Jolt gate: `cmake --preset windows-vs2022-debug-jolt`, build, `ctest`, physics/contact/vehicle/runtime/playthrough tools; passed when run explicitly.
- Visual evidence: `tools/capture_visual_smoke.py`; passed for initial, relay-to-service-log, and low-dock-drain-access scenarios.
- Playthrough: deterministic and Jolt reports passed; Jolt `framesToCheckpoint=169`, deterministic `139`.

Integration risks:

- `scripts/verify.ps1` does not run visual smoke or Jolt gameplay tools.
- Jolt CTest playthrough smoke does not pass `--vehicle-runtime jolt`.
- Visual smoke checks broad pixel heuristics, not gameplay comprehension.

## Subsystem Ownership Pressure

The next risky pressure point is not "more content". It is proof quality and runtime feel:

- Jolt is the preferred vehicle candidate but still needs live controls/camera and authored road-edge collision evidence.
- Player/camera/traversal baseline is serviceable but not yet comfortable game feel.
- Follow-up chain is long enough that further content should either change physical space or create a tighter content boundary.
