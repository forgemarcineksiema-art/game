# Technical Debt

Last updated: 2026-05-15

This file lists known foundation issues during v0.23. It is not a mandate to fix everything immediately. Future goals should pick the smallest debt item that blocks their milestone.

## v0.23 Priority Triage

### Blocking Playable Build

- No confirmed gameplay blocker remains for the current Ferry Office Service Call after v0.22 bounded review and v0.23 baseline validation.
- Run UX was the active blocker for hand-play: the correct command existed but was too developer-shaped. v0.23 addresses this with a tested `scripts/play.ps1` wrapper rather than an installer or packaging system.

### Fix Soon

1. Run a full human keyboard/mouse playthrough on the target laptop through `scripts/play.ps1` and capture any remaining vehicle feel issues around braking distance, reverse steering, checkpoint approach, exit placement, or cursor/camera comfort.
2. Keep the Blender/static mesh path narrow and honest. Add only a few controlled original props until cgltf/tinygltf is clearly justified.
3. Decide whether the next non-packaging pass is presentation polish, a hand-authored Blender prop pass, or a Jolt vehicle feasibility spike based on actual hand-play feedback.

### Acceptable For Now

1. No Job #2 yet. The existing first job needs polish and control confidence before more content.
2. Deterministic vehicle movement remains acceptable after v0.22 for the next playable-build pass; Jolt VehicleConstraint remains deferred until manual driving feedback proves deterministic tuning is the blocker.
3. `WorldState` and `FerryOfficeJob` remain explicit in-memory prototype systems, not a save/load or mission framework.
4. The tiny custom `.gltf` subset remains acceptable while assets are simple embedded-buffer static props.
5. GDI remains the most battle-tested visual/playtest renderer, but DX11 now has a simple debug text overlay and can be used for bounded playtest checks. DX11 still often falls back to WARP on this laptop.
6. Scene data is the runtime source of truth for layout, while behavior mappings stay in C++ until more job types prove a stable data shape.
7. v0.23 adds launch scripts, not release packaging, an installer, signing, updater, config UI, or save/settings persistence.

## Build / Toolchain

- Plain PowerShell PATH still cannot find `cl`, `clang++`, `g++`, `msbuild`, `ninja`, or `vcpkg`, though CMake can build through the Visual Studio generator.
- No vcpkg manifest exists. Jolt is currently an opt-in CMake FetchContent spike, not part of the default validation path.
- The version is stored only in CMake. There is no generated version header beyond the current compile definition.

## Renderer

- DX11 hardware/debug device creation fails in this environment and falls back to WARP.
- DX11 debug text now uses a small Win32 text overlay after `Present`. This makes `scripts/play.ps1 -Dx11` usable for playtest checks, but it is not a production HUD/text renderer.
- Debug boxes/lines/solid boxes and v0.12 flat mesh triangles are enough for prototypes but not a real mesh/material pipeline.
- v0.9 solid debug boxes and v0.12 mesh triangles are projected placeholder geometry with no depth buffer, sorting, lighting, textures, materials, or transparency.
- There is no resize handling, depth buffer, camera clip tuning, or resource lifetime stress testing.
- `IRenderer::drawDebugFlatTriangles` is immediate-mode and creates transient renderer data; it is not a GPU static mesh resource path.
- DX11 mesh rendering still uses CPU debug projection, not world/view/projection matrices.

## Input

- Input is polled in the Win32 window path, not a full input subsystem.
- `E` has both held and pressed-edge state.
- `Space` now behaves as a pressed-edge jump action in the Win32 path; this is correct for jumping, but docs and tests should keep that distinction explicit.
- Windowed mouse-look now captures/hides the cursor by default and recenters it for relative deltas, with `--free-cursor` / `--show-cursor` for debugging.
- Cursor capture is still basic Win32 `ClipCursor` + recentering, not raw input, a settings UI, or a full input subsystem.
- `F1` toggles the v0.17 debug overlay mode, but there is still no remapping/settings UI.
- There is no key rebinding or controller support.

## Player / Camera

- Movement is camera-relative and deterministic, but player acceleration/deceleration is immediate.
- Jump uses simple vertical velocity and gravity.
- Camera smoothing exists, but there is no camera obstruction, shoulder switching, collision, or indoor tuning.
- Focus currently uses player-facing, which can feel different from camera-facing if the player rotates the camera without moving.
- Traversal uses deterministic interpolation and still needs hands-on camera feel tuning during the traversal arc.

## World / Collision

- Static AABB-only collision.
- Player proxy is a simple vertical radius/height approximation, not a swept capsule.
- Ground is a flat floor-height query.
- No slopes, ramps, stairs, moving platforms, or step-up behavior.
- Raycast only checks static AABB colliders.
- Collision layouts are hardcoded in `PrototypeWorld::buildDefaultCollisionTestLayout` and `PrototypeWorld::buildFerryOfficePrototypeLayout`.
- The `service-gate` collider can be latched open by scene state, but there is no general dynamic-collider or door system.

## Physics Backend

- v0.9.2 adds `src/engine/physics` and an opt-in Jolt backend, but the live Ferry Office gameplay still uses `PrototypeWorld` collision.
- The default `simple` physics backend is only a deterministic validation/fallback layer. It should not grow into a production physics engine.
- The Jolt backend is validated through `windows-vs2022-debug-jolt`, not through `scripts/verify.ps1`.
- Jolt integration currently uses pinned FetchContent, not vcpkg manifest mode. Revisit dependency management before making Jolt the default backend.
- Jolt debug draw is exposed only as simple box debug lines for now. There is no full Jolt debug renderer bridge to `IRenderer`.
- No player, traversal, service-gate, or production vehicle behavior has been migrated to Jolt yet. v0.10 uses a deterministic vehicle controller and only uses `engine::physics` for a small service-yard validation/debug world.

## Vehicle

- v0.13 vehicle movement is still arcade-style and deterministic after a compact service-yard tuning pass: no wheel colliders, suspension, tire friction model, gearbox, engine curve, brake balance, or Jolt VehicleConstraint.
- The vehicle is game-layer scoped in `VehicleController`; there is no vehicle entity/component system.
- Vehicle collision is only safe yard-bound clamping plus a safe exit overlap check. It does not collide physically with all Ferry Office AABBs or dynamic bodies.
- Enter/exit has no animation, doors, seats, mount offsets per vehicle, or obstruction sweeps.
- Vehicle camera uses the existing third-person camera with alternate settings. v0.13 retunes distance/height/smoothing for the small yard, but there is no camera collision, chase-camera lag tuning, or reset-behind-vehicle command.
- The service-yard and v0.14 dock road segment are debug geometry only, not final art or a real road/terrain system.
- The v0.14 road extends vehicle bounds for a compact out-and-back route, but vehicle collision is still a finite bounds clamp rather than road-edge collision against all debug rails/curbs.
- Full Jolt vehicle integration is intentionally deferred until the placeholder feel is playtested. The next vehicle goal should still focus on manual feel/control polish rather than wheel physics.

## Interaction

- Interactables are still authored in `PrototypeScene`, though v0.8 centralizes Ferry Office names, prompts, positions, radii, and traversal constants in `FerryOfficeData`.
- Focus uses point/radius volumes with a facing preference and close proximity fallback.
- Toggle objects normally only change interaction state. The Ferry Office Wall Button is now a one-way opener that latches `routeOpened=true` to avoid closing the gate on the player.
- Pickup/world state is local runtime state only; there is no inventory, persistence, or save data.
- No UI framework exists, so prompts are debug text/logs only.

## World State

- `WorldState` is an in-memory local event ledger, not a save/load system.
- Flag mappings are hardcoded in `PrototypeScene`; v0.8 reduced string-id repetition by using centralized Ferry Office names.
- `FerryOfficeJob` is one explicit scene-owned job helper for the Ferry Office Service Call. It is not a generic mission graph, quest scripting layer, reward system, or persistence layer.
- Repeated same-value flag writes are ignored, which is correct for v0.6 but may need richer event semantics later.
- Debug summary text became longer in v0.7 because it now includes objective, completion, and `exitReached`. v0.17 adds playtest/minimal/debug text modes, but there is still no real UI overlay or HUD framework.
- There is no mission graph, quest scripting, dialogue integration, global event bus, or persistence layer.
- Slice completion is a scene helper, not a mission/objective scripting system.
- Job completion is also a scene helper. It proves one driver/fixer loop, but should not be generalized until more job types prove the real data shape.

## Traversal

- Traversal affordances are hardcoded in `PrototypeScene`; v0.8 moved the service-vault positions/tuning constants into `FerryOfficeData`.
- Only one affordance type exists: `Vault`.
- Traversal starts from the player's current position and resolves collision after landing, but it still skips continuous collision checks during the controlled motion.
- There is no animation, IK, ledge hang, full climb, wall climb, or physics-driven vault.
- Traversal prompt/debug visibility is functional but not polished.
- The current traversal route is one access gate inside the Ferry Office slice. v0.7.1 moved the affordance start to the player-accessible side, but it still needs a full human playtest feel pass.

## Naming / Architecture

- `TestWorld` and `TestScene` were renamed to `PrototypeWorld` and `PrototypeScene` in v0.8. Some historical documentation/status entries still mention the old names, but the active code boundary is renamed.
- `InteractionSystem` belongs in `src/game` for now. Promote it to `src/engine` only after multiple gameplay contexts prove a stable boundary.
- There is no scene serialization, asset registry, or editor.

## Scene / Authoring Data

- v0.15 makes `data/scenes/ferry_office.scene.json` the runtime source of truth for current layout data.
- v0.16 adds scene-authored service-run confirmation and checkpoint markers for the first driver/fixer job.
- Remaining duplication is behavior-oriented rather than pure layout-oriented: `FerryOfficeData` still holds stable names/fallback positions, `PrototypeScene` maps known action names to world-state flags, and `SandboxLayer` owns dynamic coloring, vehicle camera/exit behavior, and fallback values.
- Runtime scene loading is one-shot at startup. There is no hot reload, runtime editing, prefab system, scene diff, or editor.
- Invalid runtime scene paths currently log a warning and fall back to built-in Ferry Office setup so smoke/debug paths remain usable; stricter failure handling may be useful once multiple scenes exist.
- There is no generated C++ data path from scene JSON.
- There is no schema file beyond the Python validator.
- Scene tools validate ids, vectors, radii, extents, vehicle bounds, mesh references, route marker endpoints, and scale sanity, but they do not validate every runtime behavior mapping.
- There is no editor, gizmo, visual placement tool, asset registry, prefab system, or scene diff tool.

## Static Mesh / Assets

- v0.12 supports only a tiny `.gltf` subset: embedded base64 buffer, float position vertices, indexed triangle list.
- `.glb`, external buffers, materials, textures, normals in the renderer, UVs, node hierarchy, animation, skinning, and morph targets are not supported.
- `assets/models/unit_box.gltf` and the v0.18 prop-kit meshes are placeholder proof assets, not production art.
- `assets/models/ferry_notice_board.gltf` is a v0.20 fallback-generated proof prop, not a Blender export or production art.
- `assets/models/blender_ferry_notice_board.gltf` is a v0.20.1 Blender-exported procedural proof prop, not production art.
- There is no asset registry, mesh resource cache, file watcher, importer/cooker, mesh optimizer, LOD, or material assignment.
- Mesh instances render visually but do not define collision or physics shapes.
- The current loader is intentionally narrow. v0.19 decides to keep it briefly while asset workflow validation improves, then move to cgltf/tinygltf when real glTF coverage is needed.
- v0.12.1 improves prop scale by reusing `unit_box.gltf` for 10 mesh instances, but those are still flat-tinted placeholder blocks, not authored production meshes.
- v0.18 adds four original tiny `.gltf` prop meshes (`service_road_sign`, `road_edge_post`, `service_barrier`, and `utility_box`) and a scene-driven asset-id map in `SandboxLayer`, but this is still not an asset registry or resource cache.
- Mesh placement is now submitted from loaded scene data, but rendering still uses the immediate flat-triangle spike instead of real GPU mesh resources.
- v0.19 asset validation catches unreferenced `.gltf`, unsupported `.glb`, external buffers, missing provenance, duplicate mesh ids, unknown mesh instance asset ids, and duplicate replacement links. It still is not a glTF schema validator or Blender import pipeline.
- v0.20 adds `tools/check_blender.py` and `tools/create_simple_prop_gltf.py`. v0.20.1 confirms Blender 5.1.1 is available from PATH and adds `tools/blender/create_tidebreak_notice_board.py`.
- Blender 5.1.1 no longer exposes a direct `GLTF_EMBEDDED` export option; the v0.20.1 script exports `GLTF_SEPARATE`, embeds the generated `.bin` data URI, then deletes the `.bin`. This is acceptable for one tiny proof prop but should not quietly grow into a broad custom importer/cooker.

## Visual Readability

- v0.9 improves the Ferry Office read with solid placeholder color, but it is still a debug scene. It is not final art and should not be mistaken for the target commercial visual quality.
- v0.9.1 adds a route polyline, stronger marker hierarchy, and clearer objective wording, but the space still needs a real human playthrough on the target laptop.
- v0.11 documents art direction and placeholder color keys. v0.12/v0.12.1 add first static mesh rendering and a small prop replacement pass, and v0.18 adds the first original service-road prop kit, but still no final art, lighting, textures, materials, or model loading beyond a tiny proof subset.
- v0.14 makes the service-yard read more like a dock road with a shore/water edge cue and turn-around marker. v0.15 loads that layout from scene data, but it remains hand-authored placeholder geometry.
- v0.17 playtest mode puts objective/focus/job status first and reserves raw telemetry for debug mode, but it remains functional debug text rather than a polished UI.
- DX11 now has a simple debug text overlay, so bounded DX11 playtest checks are useful. GDI still remains the simpler renderer for overlay debugging, and a real renderer-owned text/HUD path is deferred.
- There is no authored composition pass for camera start angle, signposting, silhouettes, or route readability beyond simple colored volumes and the first flat-tinted prop-kit meshes.

## Deprecated Priority Lists

Older `Fix Soon` and `Known But Acceptable For Now` sections were folded into the current priority triage at the top of this file. Keep future triage updates there so the next goal can see the current blocking/fix-soon/acceptable split without rereading the entire debt log.
