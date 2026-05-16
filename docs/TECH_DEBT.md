# Technical Debt

Last updated: 2026-05-16

This file lists known foundation issues during the current playable-build phase. It is not a mandate to fix everything immediately. Future goals should pick the smallest debt item that blocks their milestone.

## Current Priority Triage

### Blocking Playable Build

- No confirmed gameplay blocker remains for the current Ferry Office Service Call after v0.22 bounded review and v0.23 baseline validation.
- Run UX was the active blocker for hand-play: the correct command existed but was too developer-shaped. v0.23 addresses this with a tested `scripts/play.ps1` wrapper rather than an installer or packaging system.

### Fix Soon

1. Use `docs\GAMEPLAY_REVIEW.md` before selecting another autonomous milestone. v0.95 addressed the first physical-world-consequence gap by opening Low Dock Drain access after the Storm Pump Ticket, v0.96 addressed the vehicle-feel evidence gap with Jolt-first steering/brake/reverse/route/camera metrics, v0.97 tightened vehicle camera follow, v0.98 proved Jolt route pace is not limited by the scripted throttle, and v0.99 narrowed Jolt route pace from 212 to 169 frames with a conservative straight-drive assist. The next high-value vehicle debt is Jolt-first controls/camera feel evidence, not deterministic-only polish or unmeasured mass/torque tweaks.
2. Use `python tools\capture_visual_smoke.py`, `python tools\capture_visual_smoke.py --scenario relay-to-service-log --report-json build\captures\capture_visual_smoke_midchain_report.json`, `python tools\capture_visual_smoke.py --scenario low-dock-drain-access --report-json build\captures\capture_visual_smoke_low_dock_report.json`, `python tools\playthrough_qa.py`, `python tools\physics_parity_qa.py`, opt-in `python tools\character_contact_qa.py`, opt-in `python tools\vehicle_physics_qa.py`, and opt-in `python tools\vehicle_runtime_qa.py` as bounded visual/behavioral/physics evidence before asking for a manual pass. The playthrough QA now rejects reports missing runtime service-vehicle enter, dock-road checkpoint, and exit steps; the vehicle runtime QA rejects reports missing tap/brake/reverse/coast control checks, service-run route completion checks, collision-backed obstacle checks, or deterministic/Jolt driving-feel checks.
3. The Jolt vehicle switch now has comparable first-job live-loop evidence, camera-aware obstacle-proxy steering evidence, collision-backed obstacle replay telemetry, v0.96 driving-feel metrics, v0.97 tighter camera follow, v0.98 route-pace sensitivity evidence, and v0.99 controlled route-pace improvement. Deterministic reaches the service-run checkpoint in 139 frames; Jolt reaches it in 169 frames with no fallback or bounds hit. v0.99 records Jolt route probes at 169, 163, and 158 frames for throttle 0.72, 0.86, and 1.0. `scripts\play.ps1` prefers Jolt only when the selected executable was built with Jolt; keep direct app/QA defaults deterministic until the next promotion milestone deliberately changes that.
4. Keep the Blender/static mesh path narrow and honest. v0.27 adds one controlled original Blender prop; avoid more quantity until visual evidence proves prop language, not renderer readability, world consequence, or vehicle feel is the blocker.

### Acceptable For Now

1. No broad job framework yet. v0.62 adds scene action bindings for simple/gated world-flag beats, and v0.63 uses them for the Ferry Office Work Board signoff, but richer job behavior still belongs in explicit C++ until a second independent job proves a stable generic shape. New content should stay compact and extend playthrough QA.
2. Deterministic vehicle movement remains the dependency-free direct-app and QA default after v0.99. Jolt VehicleConstraint/runtime now has feasibility, runtime-comparison, live-switch, controls-focused runtime QA, service-run route evidence, the same first-job enter-drive-exit-confirm playthrough evidence, camera-aware obstacle-proxy steering evidence, tuned obstacle proxy progress, collision-backed overlap replay evidence, required driving-feel metrics, tighter vehicle camera follow, route-pace sensitivity probes, a controlled straight-drive assist, and a preferred play-wrapper path when the Jolt executable is selected. It should still replace broader gameplay collision only through deliberate migration milestones, not as a side effect of QA hardening.
3. `WorldState` and `FerryOfficeJob` remain explicit in-memory prototype systems, not a save/load, inventory, economy, or mission framework.
4. The tiny custom `.gltf` subset remains acceptable while assets are simple embedded-buffer static props.
5. GDI remains the most battle-tested visual/playtest renderer, but DX11 now has a tiny renderer-owned bitmap debug text overlay and can be used for bounded playtest checks. DX11 still often falls back to WARP on this laptop.
6. Scene data is the runtime source of truth for layout and simple/gated interactable flag bindings, while richer behavior mappings stay in C++ until more job types prove a stable data shape.
7. v0.23 adds launch scripts, not release packaging, an installer, signing, updater, config UI, or save/settings persistence.
8. v0.32 adds deterministic first-job QA coverage, not a replacement for human feel testing.
9. v0.33 adds opt-in Jolt static-collision parity coverage for Ferry Office scene queries, not a migration of live player, traversal, gate, or vehicle behavior.
10. v0.34 adds opt-in character/contact probe coverage against the mirrored Ferry Office static scene, not a live `PlayerController` migration.

## Build / Toolchain

- Plain PowerShell PATH still cannot find `cl`, `clang++`, `g++`, `msbuild`, `ninja`, or `vcpkg`, though CMake can build through the Visual Studio generator.
- No vcpkg manifest exists. Jolt is currently an opt-in CMake FetchContent spike, not part of the default validation path.
- The version is stored only in CMake. There is no generated version header beyond the current compile definition.

## Renderer

- DX11 hardware/debug device creation fails in this environment and falls back to WARP.
- DX11 debug text now uses a tiny dependency-free renderer-owned bitmap text path drawn before capture and `Present`. This makes `scripts/play.ps1 -Dx11` usable for playtest checks, but it is not a production HUD/text renderer.
- v0.29 adds renderer-owned 32-bit BMP capture for bounded local validation. GDI capture includes the GDI text overlay; v0.31 means DX11 capture now includes the renderer-owned bitmap text overlay as well as geometry/depth presentation.
- v0.30 strengthens the capture harness with dimension, color-diversity, luminance, broad scene-presence, GDI/DX11 dimension parity, and JSON report checks. v0.31 adds a conservative bright text-signal heuristic without OCR or pixel-perfect golden images.
- Debug boxes/lines/solid boxes and v0.12 flat mesh triangles are enough for prototypes but not a real mesh/material pipeline.
- v0.58 adds `clearance_tag.gltf` as a tiny generated fallback prop tinted by `dock-road-clearance-state`, not a real lighting, decal, texture, collision, or bespoke prop-material system.
- v0.38 makes playtest rendering cleaner and adds fixed overcast face shading for existing scene geometry, but this is still not real lighting, materials, textures, terrain, a production character model, or an art pipeline.
- v0.9 solid debug boxes and v0.12 mesh triangles remain placeholder geometry. v0.27 adds a small painter-depth sort for projected solid-box and flat-mesh triangle batches; v0.28 adds a DX11-only real world-to-clip matrix and depth-buffer path for solid boxes and flat mesh triangles.
- GDI remains on CPU projection/painter-depth. DX11 lines, wire boxes, grid, route/debug lines, and bitmap text are still overlay/debug-projection rendering rather than fully depth-aware renderer primitives.
- There is no resize handling, camera clip tuning, or resource lifetime stress testing for the new DX11 depth resources.
- There is no PNG encoder, pixel-perfect screenshot comparison, golden-image approval flow, semantic object detection, real font/Unicode shaping, production HUD/text system, or resize matrix for alternate capture dimensions. The v0.31 harness remains a broad smoke/heuristic check rather than visual proof that the playable composition is good.
- `IRenderer::drawDebugFlatTriangles` is immediate-mode and creates transient renderer data; it is not a GPU static mesh resource path.
- DX11 mesh rendering is still immediate-mode; v0.28 adds matrix/depth presentation, not GPU static mesh resources, batching, materials, textures, or asset residency.

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
- Scene-authored colliders with `stateFlag` / `blocksWhenFlagFalse` can now toggle simple AABB blocking for route consequences such as the service gate and Low Dock Drain access barrier. There is still no animated door, moving platform, swept dynamic collider, or production dynamic-collider system.

## Physics Backend

- v0.9.2 adds `src/engine/physics` and an opt-in Jolt backend, but the live Ferry Office gameplay still uses `PrototypeWorld` collision.
- The default `simple` physics backend is only a deterministic validation/fallback layer. It should not grow into a production physics engine.
- The Jolt backend is validated through `windows-vs2022-debug-jolt`, not through `scripts/verify.ps1`.
- Jolt integration currently uses pinned FetchContent, not vcpkg manifest mode. Revisit dependency management before making Jolt the default backend.
- Jolt debug draw is exposed only as simple box debug lines for now. There is no full Jolt debug renderer bridge to `IRenderer`.
- v0.33 mirrors the authored Ferry Office static boxes plus a flat scene-floor body into the opt-in Jolt path and validates floor/raycast/overlap parity against `PrototypeWorld` through `tools\physics_parity_qa.py`.
- v0.34 adds `tools\character_contact_qa.py`, which compares the current player proxy against opt-in physics contact candidates for floor grounding, walls, the service gate, service barrier, a clear lane, a corner pushout, and an opened-gate case.
- v0.35 adds `tools\vehicle_physics_qa.py`, which validates an opt-in Jolt wheeled-vehicle feasibility script for the authored Ferry Office service vehicle and emits a promote/defer report.
- v0.36 adds `tools\vehicle_runtime_qa.py`, which compares a frame-stepped opt-in Jolt vehicle runtime adapter against the deterministic `VehicleController` fallback on the authored Ferry Office service vehicle. v0.47 enforces a 240-frame Jolt service-run route budget and records a 213-frame Jolt route pass. v0.48 adds deterministic enter-drive-exit-confirm runtime playthrough evidence for the first service job, v0.49 runs that same playthrough vehicle segment through the opt-in Jolt runtime in 213 frames, v0.96 requires deterministic/Jolt driving-feel checks for route, brake, reverse, steering, and camera behavior, and v0.99 tightens the Jolt route budget to 190 frames after a 169-frame route pass.
- v0.37 exposes that runtime adapter through `--vehicle-runtime jolt` and `scripts\play.ps1 -VehicleRuntime jolt` for manual playtest only.
- No live player, traversal, service-gate, dynamic collider toggling, or default production vehicle behavior has been migrated to Jolt yet. v0.10 uses a deterministic vehicle controller and only uses `engine::physics` for a small service-yard validation/debug world; v0.37 proves a live opt-in vehicle switch, not default replacement.

## Vehicle

- v0.13 vehicle movement is still arcade-style and deterministic after a compact service-yard tuning pass: no live wheel colliders, suspension, tire friction model, gearbox, engine curve, brake balance, or Jolt VehicleConstraint in gameplay.
- The vehicle is game-layer scoped in `VehicleController`; there is no vehicle entity/component system.
- Vehicle collision is only safe yard-bound clamping plus a safe exit overlap check. It does not collide physically with all Ferry Office AABBs or dynamic bodies.
- Enter/exit has no animation, doors, seats, mount offsets per vehicle, or obstruction sweeps.
- Vehicle camera uses the existing third-person camera with alternate settings. v0.13 retunes distance/height/smoothing for the small yard, but there is no camera collision, chase-camera lag tuning, or reset-behind-vehicle command.
- The service-yard and v0.14 dock road segment are debug geometry only, not final art or a real road/terrain system.
- The v0.14 road extends vehicle bounds for a compact out-and-back route, but vehicle collision is still a finite bounds clamp rather than road-edge collision against all debug rails/curbs.
- Full default Jolt vehicle integration remains deferred until fuller controls/camera, steering, obstacle, and road-edge replay evidence proves the switched path is worth promoting. v0.37 proves opt-in live selection for the authored service vehicle, v0.37.1 fixes the first obvious tap/coast/reverse control bug, v0.48 proves the deterministic first-job runtime vehicle beat, v0.49 proves the same beat through opt-in Jolt, v0.59 closes the scripted obstacle-progress gap, and v0.99 narrows the Jolt straight-route pace gap to 169 versus 139 deterministic. This is still not default Jolt replacement, full gameplay collision, complete tuning, or a complete road model.

## Interaction

- Interactables are still authored in `PrototypeScene`, though v0.8 centralizes Ferry Office names, prompts, positions, radii, and traversal constants in `FerryOfficeData`.
- Focus uses point/radius volumes with a facing preference and close proximity fallback.
- Toggle objects normally only change interaction state. The Ferry Office Wall Button is now a one-way opener that latches `routeOpened=true` to avoid closing the gate on the player.
- Pickup/world state is local runtime state only; there is no inventory, persistence, or save data.
- No UI framework exists, so prompts are debug text/logs only.

## World State

- `WorldState` is an in-memory local event ledger, not a save/load system.
- v0.32 adds an automated QA-only Ferry Office Service Call path that validates the first job's state chain and report output. It is deterministic behavioral coverage, not mission scripting or save/load.
- Simple interactable flag mappings can now come from scene action bindings. `PrototypeScene` still owns richer Ferry Office job helpers, traversal completion, vehicle checkpoint, objective text, and service-gate collider sync.
- `FerryOfficeJob` is one explicit scene-owned job helper for the Ferry Office Service Call. It is not a generic mission graph, quest scripting layer, reward system, or persistence layer.
- Repeated same-value flag writes are ignored, which is correct for v0.6 but may need richer event semantics later.
- Debug summary text became longer in v0.7 because it now includes objective, completion, and `exitReached`. v0.17 adds playtest/minimal/debug text modes, v0.55 adds compact follow-up status, and v0.61 adds compact follow-up next-step text, but there is still no real UI overlay, HUD framework, minimap, or route-arrow system.
- There is no mission graph, quest scripting, dialogue integration, global event bus, or persistence layer.
- Slice completion is a scene helper, not a mission/objective scripting system.
- Job completion is also a scene helper. It proves one driver/fixer loop, but should not be generalized until more job types prove the real data shape.
- The v0.54 playthrough QA runner now exercises the service-vehicle enter, drive, dock-road checkpoint, exit, confirm, Dock Road Relay reset, relay service log, and dock-road clear-tag beats through deterministic runtime behavior; opt-in Jolt also completes the same 13-event chain. It still does not simulate full keyboard/mouse navigation, camera movement, obstacle avoidance, or human-readable feel.

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
- v0.26 reuses the existing seven scene mesh assets for 31 total mesh instances, adding manifest counter/paperwork cues, a side service panel, dock cleats, a service-yard tool crate, and a Service Run review board without new model files or collision.
- v0.27 adds `assets/models/blender_cable_reel.gltf`, a second small Blender-authored procedural prop, and references it once in the service yard. It is still untextured placeholder geometry, not a material/asset-pipeline expansion.
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
- v0.24 adds a first progressive-guidance pass for playtest/minimal modes so future route, traversal, vehicle, checkpoint, and service-run markers wait for the relevant Ferry Office job phase. Debug mode still keeps the full authored guidance set for validation.
- v0.25 adds a first authored composition pass: scene-authored player-start yaw now drives initial facing/camera composition, and existing placeholder assets add office approach, service-yard threshold, dock-road rhythm, and Service Run confirmation cues.
- v0.26 adds a controlled existing-asset prop/identity pass around the manifest, Ferry Office controls side, dock edge, service yard, and Service Run endpoint. It improves authored read but is still built from placeholder flat-tinted meshes.
- v0.27 adds painter-depth ordering for projected debug triangles and one Blender cable reel prop. v0.28 follows with the first DX11 real depth-buffer/world-matrix path for solid boxes and flat mesh triangles, while keeping debug lines/text overlay-style.
- DX11 now has a tiny renderer-owned bitmap debug text overlay, so bounded DX11 playtest checks and BMP captures are useful. GDI still remains the simpler renderer for overlay troubleshooting, and a real production text/HUD path is deferred.
- The v0.28 presentation spike is still prototype presentation: there is no final art-quality signage, renderer-owned mesh resource path, lighting, materials, textures, authored terrain, resize-safe depth handling, or production HUD.

## Deprecated Priority Lists

Older `Fix Soon` and `Known But Acceptable For Now` sections were folded into the current priority triage at the top of this file. Keep future triage updates there so the next goal can see the current blocking/fix-soon/acceptable split without rereading the entire debt log.
