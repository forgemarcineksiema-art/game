# Roadmap

This roadmap is intentionally realistic. The project is a custom engine workbench, not an attempt to recreate the full scope of any commercial game.

## v0.1 - AI-Native Engine Foundation

- CMake C++20 skeleton.
- Engine loop, logging, clock, config, filesystem helpers.
- Renderer abstraction with DirectX 11, Windows GDI fallback, and null backends.
- Minimal visible runtime path where supported.
- Headless smoke-test path.
- AI-facing docs and validation scripts.

## v0.2 - Third-Person Player Controller + Camera Prototype

- Add a controllable placeholder capsule or simple proxy.
- Add third-person follow camera math.
- Add keyboard/mouse input mapping.
- Keep collisions simple or mocked.
- Validate with smoke tests and one manual windowed run.

Status: implemented in this branch as a focused prototype with debug primitives, camera-relative movement, sprint, jump, simple obstacle push-out, and lightweight tests.

## v0.3 - World / Collision Prototype

- Add a simple static world representation.
- Add primitive collision queries.
- Add debug collision visualization.
- Keep asset loading simple and original.

Status: implemented in this branch as a static AABB world with floor query, player proxy resolution, simple raycast, debug collision visualization, and lightweight tests.

## v0.4 - Interaction System

- Add interactable objects and focus detection.
- Add command/action abstraction.
- Add simple prompts in debug UI or logs.

Status: implemented in this branch as a focused prototype with three debug interactables, `E` pressed-edge input, range/facing focus detection, pickup/toggle/info actions, debug prompts, marker visualization, logs, and lightweight tests.

## v0.4.1 - Foundation Review + Game Direction Lock

- Review input, player, camera, collision, and interaction boundaries.
- Lock the original game direction in `docs/GAME_DIRECTION.md`.
- Define the first 10-minute micro-scenario in `docs/VERTICAL_SLICE.md`.
- Record technical debt before v0.5 in `docs/TECH_DEBT.md`.
- Add a manual checklist for player/camera/collision/interactions.
- Choose the v0.5 direction.

Status: implemented as a documentation and review milestone. No major gameplay system was added.

## v0.5 - Traversal Prototype

- Build one narrow on-foot traversal prototype.
- Start with a simple access-gating move such as mantle, climb-up, vault, or ledge step.
- Do not add vehicles in the same milestone.
- Keep physics lightweight and explicit; do not add a physics library yet.

Status: implemented as a first contextual service-barrier vault with debug affordance markers, `Space` trigger priority over jump when focused, deterministic player repositioning, scene integration, and lightweight tests.

## v0.5.1 - Traversal Feel + Camera / Collision Polish

- Polish traversal start/end placement and prompt clarity.
- Tune camera behavior during traversal.
- Review collision handoff before and after traversal.
- Keep the scope on feel and stability, not additional traversal types.

Status: implemented as a traversal stability pass. Traversal now starts from the player's current position to avoid marker snap, resolves landing through world collision, exposes clearer debug state, and keeps jump/interactions validated.

## v0.6 - World Event / Remembered State Prototype

- Add a minimal event bus or scene event model.
- Add remembered local world state for the Ferry Office micro-scenario.
- Let existing interactions/traversal trigger simple persistent prototype events.
- Avoid NPC AI, complex planners, mission scripting, inventory, save/load, or large content systems.
- Keep scope small enough for deterministic tests.

Status: implemented as a local remembered-state prototype with boolean flags, deterministic event records, debug summary text/logs, scene-level mappings from existing interactions/traversal, and lightweight tests.

## v0.7 - The Ferry Office Micro-Slice

- Build one original micro-scenario using project-owned placeholder assets.
- Exercise player, camera, world, interaction, traversal, and remembered-state systems.
- Keep the slice small enough to validate in one run.
- Do not add full mission scripting, NPC AI, inventory, save/load, vehicles, combat, final art, or asset pipelines.

Status: implemented as a debug micro-slice with a Ferry Office layout, manifest pickup, Service Barrier Vault, Maintenance Box power restore, service-gate route opening, Exit Summary Marker, objective/debug summary text, route-open collision state, and lightweight tests.

## v0.7.1 - Micro-Slice Playtest Polish

- Play the Ferry Office loop by hand and tighten prompt placement, marker readability, and objective ordering.
- Improve camera/collision feel only where the micro-slice exposes friction.
- Keep scope on polish and validation, not a new major system.
- Decide whether `TestWorld` / `TestScene` should be renamed to `PrototypeWorld` / `PrototypeScene`.

Status: implemented as a focused polish pass. The Service Barrier Vault start is now on the player-accessible side, the Maintenance Box no longer steals focus before traversal, the Wall Button latches the service gate open to avoid trapping, GDI debug text is split into readable sections, and scene-system tests cover the complete micro-slice loop. A full human keyboard/mouse playthrough is still recommended.

## v0.8 - Prototype Scene Naming + Data Cleanup

- Rename stale `TestWorld` / `TestScene` boundaries to `PrototypeWorld` / `PrototypeScene`.
- Centralize Ferry Office debug names, prompts, positions, marker radii, and traversal constants.
- Reduce direct string-name coupling in scene state mapping and debug rendering.
- Preserve the existing Ferry Office micro-slice behavior.
- Prove behavior stability with focused tests and the existing validation scripts.

Status: implemented as a cleanup milestone with no new gameplay systems.

## v0.8.1 - Input / Camera Comfort + Cursor Capture Polish

- Hide and confine the cursor by default during windowed play.
- Keep a visible cursor mode for debugging and remote sessions.
- Preserve mouse/touchpad camera, arrow-key fallback, `Esc` quit, and smoke/headless behavior.
- Keep scope on comfort, not new gameplay systems.

Status: implemented as an input/camera comfort polish pass.

## v0.9 - Atmospheric Ferry Office / Island Mood Prototype

- Make the Ferry Office debug micro-slice read more like a dock-side island place.
- Add simple solid placeholder presentation while preserving wire/debug visibility.
- Keep movement, camera, collision, interaction, traversal, world state, and cursor behavior unchanged.
- Do not add vehicles, NPC AI, missions, asset loading, materials, final art, or renderer complexity.

Status: implemented as a visual-readability pass using solid debug boxes, a Tidebreak window title, a dock/service-yard color palette, and a committed GDI screenshot artifact for review.

## v0.9.1 - Ferry Office Visual Playtest + Readability Polish

- Review the v0.9 scene through GDI visual evidence and a limited input playtest attempt.
- Make only small clarity changes: route signposting, marker hierarchy, objective wording, and debug text ordering.
- Preserve movement, camera, collision, interactions, traversal, world state, cursor capture, and validation behavior.
- Keep the prototype in debug-placeholder presentation; do not add asset loading or new gameplay systems.

Status: implemented as a readability polish pass with a route polyline, stronger objective/focus debug text, clearer Ferry Office objective wording, tuned marker hierarchy, and an updated GDI screenshot reference.

## v0.9.2 - Physics Foundation Decision + Jolt Integration Spike

- Decide the long-term physics candidate before vehicle/player systems grow around custom collision.
- Add `docs/PHYSICS_DECISION.md` comparing Jolt, PhysX, Bullet, and temporary custom collision.
- Add a vendor-safe `src/engine/physics` boundary with engine-owned types and no Jolt exposure to `src/game`.
- Keep normal validation dependency-free while adding an explicit opt-in Jolt preset.
- Prove Jolt can configure, build, initialize, create a static box, raycast through the engine interface, and shut down.

Status: implemented as an architecture/dependency spike. Jolt is the primary production candidate, PhysX remains backup, Bullet is not preferred, and existing Ferry Office gameplay remains on the tested prototype collision path until a later migration goal.

## v0.10 - Vehicle Feel Spike on Physics Foundation

- Add one placeholder service-yard vehicle.
- Prove enter/exit, throttle, braking, reverse, steering, and a separate vehicle camera target.
- Keep the controller narrow, deterministic, and game-layer scoped.
- Use the engine-owned physics boundary non-invasively and keep Jolt private to `src/engine`.
- Preserve the Ferry Office micro-slice and default dependency-free validation.

Status: implemented as a first vehicle feel spike. The service-yard vehicle can be entered with `E` when no Ferry Office interactable is focused, driven with `W/S/A/D`, exited with `E` when the side exit marker is clear, and viewed through a vehicle camera mode. Jolt VehicleConstraint, wheels, suspension, traffic, cargo, and vehicle persistence are intentionally deferred.

## v0.11 - Scene / Object Authoring Foundation + Codex World Tools

- Add a human-readable Ferry Office/service-yard scene data file.
- Add Codex-friendly scene report, validation, and scale-audit tools.
- Document units, scale, pivots, naming, Blender/glTF direction, scene authoring workflow, and art/mood direction.
- Keep runtime behavior unchanged and avoid a full editor, ECS, asset pipeline, model loading, animation runtime, or gameplay system.

Status: implemented as a tooling/data foundation. At v0.11, runtime still used the existing C++ prototype setup, while `data/scenes/ferry_office.scene.json` mirrored the scene and was validated by Python tools and CTest/verify. v0.15 later promoted that scene data into runtime loading.

## v0.12 - Static Mesh + glTF Render Spike

- Add a minimal engine-owned static mesh data boundary.
- Prove a tiny original glTF placeholder asset can load and compute bounds.
- Add scene-data mesh asset and mesh instance references.
- Extend Codex scene tools to validate/report mesh references.
- Render flat-tinted mesh triangles through the existing renderer abstraction where practical.
- Preserve debug visibility and avoid materials, textures, lighting, animation, editor work, or a full asset pipeline.

Status: implemented as a narrow static mesh/render spike. `assets/models/unit_box.gltf` is project-original, `data/scenes/ferry_office.scene.json` now references four mesh instances, GDI/DX11/null accept flat triangle submissions, and runtime mirrors a few mesh instances explicitly in `SandboxLayer`.

## v0.12.1 - Ferry Office Prop Replacement + Visual Scale Pass

- Use the v0.12 static mesh path to replace or augment a few purposeful Ferry Office/service-yard placeholders.
- Improve visual scale/readability for the office front, service gate, maintenance box, dock props, service-yard crate, and vehicle silhouette.
- Keep debug collision, interaction, traversal, route, vehicle, and world-state markers visible.
- Preserve the tiny glTF loader, no-texture/no-material renderer scope, scene validation, default validation, and Jolt opt-in validation.

Status: implemented as a small mesh-backed prop/scale pass using the existing original `unit_box.gltf`. Scene data now references 10 mesh instances, runtime mirrors them explicitly in `SandboxLayer`, and a new GDI screenshot reference records the v0.12.1 presentation.

## v0.13 - Vehicle Feel + Service Yard Road Test Polish

- Tune the existing deterministic service-yard vehicle for a compact road-test area.
- Improve acceleration, braking, reverse, drag/coast, low-speed steering, and vehicle camera readability without adding wheels, suspension, traffic, or Jolt VehicleConstraint.
- Widen and clarify the service-yard pad/bounds while keeping scene JSON and runtime constants synchronized.
- Add coverage for vehicle focus, bounds clamping, drag timestep clamping, exit placement, reverse steering, and scene vehicle bounds.
- Preserve the Ferry Office micro-slice, static mesh path, scene tools, default validation, and Jolt opt-in validation.

Status: implemented as a small feel/readability polish pass. The vehicle remains deterministic and game-layer scoped; it is better suited for the current service-yard road test, but still needs human playtest tuning before deeper vehicle physics or a larger road loop.

## v0.14 - First Island Road Segment / Dock Road Mood Pass

- Extend the Ferry Office/service-yard into a compact dock road segment so the vehicle has a meaningful out-and-back route.
- Keep the route placeholder-only: solid debug pads, rails, shore/water cue, road marker, finite vehicle bounds, and scene-data synchronization.
- Preserve the Ferry Office loop, interactions, traversal, world state, vehicle enter/exit, static mesh validation, default validation, and Jolt opt-in validation.
- Avoid traffic, missions, large map expansion, runtime scene loading, full vehicle physics, textures/materials, final art, and new gameplay systems.

Status: implemented as a compact dock road mood/layout pass. The service-yard now opens into a short road pad with a turn-around marker, shore/water cue, rail/curb bounds, route marker, expanded finite vehicle bounds, and updated scene validation tests. At v0.14 runtime layout remained mirrored explicitly in `SandboxLayer`; v0.15 later moved that layout into runtime scene loading.

## v0.15 - Runtime Scene Loading / Scene Data Source of Truth

- Load `data/scenes/ferry_office.scene.json` at runtime.
- Use scene data for static colliders, visual placeholders, mesh instances, interactables, traversal affordances, player start, vehicle spawn/proxy/bounds, route markers, and objective markers where practical.
- Keep Tidebreak-specific gameplay meaning in C++ for now: world-state effects, objective text, interaction result mapping, and traversal completion side effects.
- Add `--scene <path>` for custom scene experiments.
- Preserve the Ferry Office loop, service-yard vehicle, dock road segment, scene tools, default validation, and Jolt opt-in validation.
- Avoid editor/ECS/prefabs, mission scripting, asset pipeline expansion, full map work, or new gameplay systems.

Status: implemented as a source-of-truth migration. The runtime now loads the Ferry Office scene through `SceneLoader` / `SceneDefinition` using `nlohmann/json`, while behavior remains explicitly scene-owned C++.

## v0.16 - First Driver/Fixer Job Prototype

- Add one explicit Ferry Office Service Call job using existing systems.
- Use the manifest, Service Barrier Vault, Maintenance Box, service gate, service vehicle, dock-road checkpoint, and service-run confirmation marker.
- Keep the job as a small scene-owned helper, not a generic mission framework or scripting system.
- Preserve runtime scene loading, Ferry Office slice behavior, service-yard vehicle, dock road segment, scene tools, default validation, and Jolt opt-in validation.
- Avoid NPC AI, traffic, combat, inventory, economy, save/load, mission scripting, large map expansion, and new vehicle physics.

Status: implemented as the first driver/fixer job loop. `FerryOfficeJob` owns deterministic objective phases and completion checks while `WorldState` records job start, service vehicle use, dock-road checkpoint reach, service-run confirmation, and job completion.

## v0.17 - First Playable Presentation Mode

- Keep the existing Ferry Office Service Call as the only job.
- Add a player-facing playtest/presentation overlay while preserving full debug telemetry.
- Add a small runtime toggle between presentation and debug overlay.
- Reduce marker/debug clutter in normal play without hiding validation data.
- Fix only small presentation/readability bugs exposed by the current playable loop.

Status: implemented as a presentation/readability pass. Normal windowed play now defaults to `--ui-mode playtest`, `--ui-mode debug` keeps full raw telemetry, `--ui-mode minimal` provides a tiny objective/prompt/job view, and `F1` toggles the debug overlay at runtime. No Job #2 or new gameplay system was added.

## v0.18 - Island Service Road Visual Identity / First Real Prop Style Spike

- Keep the existing Ferry Office Service Call as the only job.
- Add a small original prop style kit for the island service road without adding textures, materials, lighting, final art, or a broader asset pipeline.
- Improve the Ferry Office / service yard / dock road read with service-road signs, road-edge posts, a barrier cue, and a maintenance utility box.
- Preserve playtest/debug/minimal UI modes, scene-data source of truth, static mesh tooling, default validation, and Jolt opt-in validation.
- Keep the work inside the current tiny `.gltf` static mesh subset.

Status: implemented as a visual-identity/readability pass. The scene now references 5 mesh assets and 15 mesh instances, including four new original v0.18 prop meshes, and `SandboxLayer` loads scene mesh assets by id instead of only drawing `unit-box-mesh`.

## v0.19 - Tiny Asset Pipeline Decision + Static Mesh Workflow Stabilization

- Decide whether to keep the custom tiny `.gltf` loader, switch to cgltf/tinygltf, or broaden the asset path.
- Document the near-term static mesh workflow in `docs/ASSET_PIPELINE_DECISION.md`.
- Improve Codex-facing mesh/asset reporting and validation so committed model files, scene references, provenance, unsupported formats, and bounds are visible.
- Keep the runtime mesh path narrow: no asset registry, material system, texture pipeline, renderer rewrite, editor, or final art.
- Preserve the Ferry Office Service Call, presentation modes, v0.18 prop visibility, default validation, and Jolt opt-in validation.

Status: implemented as a workflow-stabilization milestone. The project keeps the tiny custom `.gltf` loader for the short prototype phase, adds `tools/validate_assets.py`, expands `tools/mesh_report.py`, and makes asset validation part of `scripts/verify.ps1`.

## v0.20 - Blender-to-Tidebreak Static Prop Workflow Spike

- Check Blender availability honestly.
- Prove one Blender-exported original prop if Blender is available.
- If Blender is unavailable, document the exact blocker and keep a clearly labeled fallback path.
- Preserve the tiny static mesh subset, asset validation, scene data, runtime loading, default validation, and Jolt opt-in validation.
- Avoid final art, textures/materials, animation, editor, asset registry, resource cache, and new gameplay systems.

Status: complete. Blender was not available in PATH during the v0.20 run, so v0.20 added an optional Blender checker, a documented fallback generator, and one fallback-generated original notice-board prop without claiming Blender export success.

## v0.20.1 - Blender Export / Loader Compatibility Stabilization

- Verify Blender from PATH after the laptop restart.
- Run Blender headlessly to create one original procedural Tidebreak static prop.
- Export a constrained `.gltf` that remains compatible with the current tiny static mesh loader.
- Keep the v0.20 fallback prop honestly labeled.
- Preserve asset validation, scene validation, runtime loading, default validation, and Jolt opt-in validation.

Status: implemented. Blender 5.1.1 is available from PATH, `tools/blender/create_tidebreak_notice_board.py` exports `assets/models/blender_ferry_notice_board.gltf`, and the script post-embeds Blender's separate buffer so the current custom loader remains acceptable for this narrow prop workflow.

## v0.21 - Ferry Office Playable Build Polish / Bug Bash

- Keep the existing Ferry Office Service Call as the only job.
- Review the current playable loop through bounded/manual GDI runs and code/scene inspection.
- Fix only current-loop clarity or safety issues around prompts, marker wording, UI readability, interaction priority, vehicle enter/exit, or prop visibility.
- Triage `docs/TECH_DEBT.md` into blocking playable build, fix soon, and acceptable-for-now categories.
- Preserve runtime scene loading, playtest/debug/minimal UI modes, Blender/static mesh workflow, default validation, and Jolt opt-in validation.

Status: implemented as a focused playable-build polish pass. The traversal prompt now avoids duplicated input wording, the Service Run Marker no longer claims completion before job prerequisites are met, v0.21 bumps the runtime version, and `TECH_DEBT.md` now highlights vehicle/control polish as the strongest near-term blocker.

## v0.22 - Vehicle / Driving Control Polish

- Keep the existing Ferry Office Service Call as the only job.
- Polish the deterministic service-yard vehicle enough to make the current dock-road loop more trustworthy.
- Improve vehicle camera behavior, low-speed steering readability, and playtest driving status text.
- Preserve vehicle enter/exit safety, interaction priority, scene loading, static mesh assets, default validation, and Jolt opt-in validation.
- Avoid Jolt VehicleConstraint, wheel/suspension simulation, traffic, new vehicles, map expansion, or Job #2.

Status: implemented as a narrow vehicle-control polish pass. The vehicle camera now looks slightly ahead, vehicle camera mode gently follows vehicle yaw, low-speed steering has an explicit assist factor for compact turn-arounds, and playtest text while driving reports speed, checkpoint status, and exit clear/blocked state.

## v0.23 - Playable Build Packaging / Run UX Polish

- Add a simple playable launch path for the current Ferry Office Service Call.
- Default that path to GDI, playtest UI, and the authored Ferry Office scene.
- Keep lower-level debug, DX11, smoke/headless, scene-tool, Blender, and Jolt validation paths available.
- Add tests so the launch wrapper defaults, overrides, passthrough args, and missing-executable guidance do not drift.
- Preserve the existing gameplay loop, scene loading, vehicle control, static mesh path, default validation, and Jolt opt-in validation.

Status: implemented as a local run UX milestone. `scripts/play.ps1` now launches the current Ferry Office Service Call with GDI, playtest UI, and the authored Ferry Office scene by default, while tests cover defaults, overrides, passthrough args, and missing-executable guidance. This is not an installer, release packaging system, or gameplay expansion.

## v0.24 - Ferry Office Playable Presentation Polish

- Keep the existing Ferry Office Service Call as the only job.
- Start from bounded/default `scripts/play.ps1` evidence when a full human playthrough is not available.
- Make playtest/minimal presentation progressively reveal route, objective, traversal, interaction, and vehicle guidance based on the current job phase.
- Keep `--ui-mode debug` as the full validation/workbench view with all authored route markers and telemetry.
- Make player-facing job status less raw while preserving debug telemetry.
- Preserve scene loading, static mesh rendering, current vehicle behavior, default validation, and Jolt opt-in boundaries.
- Avoid Job #2, a mission framework, renderer rewrite, material/texture pipeline, new assets, packaging, save/settings persistence, or broad refactors.

Status: implemented as a bounded-evidence presentation polish pass. Playtest/minimal modes now defer future route guidance and late markers until the current Ferry Office job phase needs them, while debug mode keeps the complete authored guidance set for validation. The overlay job line now uses player-facing step text instead of raw phase telemetry.

## v0.25 - Ferry Office Composition Pass

- Keep the existing Ferry Office Service Call as the only job.
- Use the successful hand keyboard/mouse playthrough after v0.24 as the reason to skip v0.24.1 bugfix work.
- Improve starting camera composition, office approach signposting, facade silhouette, service-yard threshold readability, dock-road rhythm, and visibility of existing Service Run confirmation beat.
- Prefer `data/scenes/ferry_office.scene.json` layout and presentation edits; use C++ only where scene-authored player start orientation needs to affect runtime camera/facing.
- Preserve progressive playtest guidance, debug validation mode, current vehicle behavior, static mesh subset, scene tooling, and default validation.
- Avoid Job #2, new missions, NPCs, combat, Jolt VehicleConstraint, renderer rewrite, material/texture pipeline, asset registry, editor, packaging, save/settings persistence, or broad refactors.

Status: implemented as an authored composition/readability pass. The scene start now uses a slight off-axis dock approach, the runtime honors scene-authored player-start yaw for initial facing/camera composition, and the Ferry Office/service-yard/dock-road gained a small set of existing-asset landmark cues without adding new assets or gameplay systems.

## v0.26 - Ferry Office Prop / Identity Pass

- Keep the existing Ferry Office Service Call as the only job.
- Use the clean hand playthrough after v0.25 as the reason to move straight into small scene-composition polish rather than vehicle, mission, or engine expansion work.
- Add a controlled set of original prop/identity cues to the manifest area, office controls side, dock starting edge, service yard, and Service Run confirmation endpoint.
- Prefer existing `meshAssets` and `meshInstances` in `data/scenes/ferry_office.scene.json`; add no new model files unless a tiny Blender/procedural prop is clearly justified and remains inside the current embedded-buffer `.gltf` subset.
- Preserve progressive playtest guidance, debug validation mode, current vehicle behavior, static mesh subset, scene tooling, and default validation.
- Avoid Job #2, new missions, NPCs, combat, Jolt VehicleConstraint, renderer rewrite, material/texture pipeline, asset registry, editor, packaging, save/settings persistence, or broad refactors.

Status: implemented as a scene-data prop/identity pass. The Ferry Office scene now reuses the existing seven mesh assets for 31 total mesh instances, adding manifest counter/paperwork cues, a side service panel, dock cleats, a service-yard tool crate, and a Service Run review board without adding collision, new assets, or gameplay systems.

## v0.27 - Renderer/Depth Presentation Spike + Tiny Blender Props Pass

- Keep the existing Ferry Office Service Call as the only job.
- Improve debug-presentation readability where flat projected triangles overlap, without converting the renderer to a full world/view/projection, material, lighting, or depth-buffer path.
- Add a tiny painter-depth helper for projected debug triangles so GDI and DX11 draw solid boxes and flat mesh submissions back-to-front within each immediate triangle batch.
- Add one small original Blender-authored service-yard prop only if it stays in the existing embedded-buffer `.gltf` subset with scene provenance and validation coverage.
- Preserve progressive playtest guidance, debug validation mode, current vehicle behavior, scene loading, static mesh tooling, default validation, and Jolt opt-in boundaries.
- Avoid Job #2, new missions, NPCs, combat, Jolt VehicleConstraint, full renderer rewrite, material/texture pipeline, asset registry, editor, packaging, save/settings persistence, or broad refactors.

Status: implemented as a narrow presentation/readability spike. Debug projection now exposes camera depth for projected triangles, GDI/DX11 sort solid boxes and flat mesh triangle batches back-to-front, and the service yard gained one Blender-authored `blender_cable_reel.gltf` prop for 8 mesh assets and 32 mesh instances total.

## v0.28 - DX11 Real Depth / Matrix Spike

- Keep the existing Ferry Office Service Call as the only job.
- Add a narrow DX11 real world/view/projection-style matrix path and depth buffer for existing debug solid boxes and flat static mesh triangles.
- Preserve GDI as the v0.27 painter-depth fallback and keep null rendering safe for smoke tests.
- Keep route/debug lines, wire boxes, grid/axes, and debug text usable as overlay-style diagnostics.
- Add focused matrix/depth regression coverage that proves the matrix matches the current debug camera projection and maps depth into DirectX NDC `0..1`.
- Preserve progressive playtest guidance, debug validation mode, current vehicle behavior, scene loading, static mesh tooling, default validation, and Jolt opt-in boundaries.
- Avoid Job #2, new missions, NPCs, combat, Jolt VehicleConstraint, material/texture pipeline, asset registry, editor, packaging, save/settings persistence, or broad renderer/resource rewrites.

Status: implemented as a DX11-only renderer architecture spike. DX11 now creates a depth-stencil target, uses depth-enabled state for debug solid boxes and flat mesh triangles, updates a world-to-clip matrix constant buffer from the existing `DebugCamera`, and keeps lines/text on the readable debug overlay path.

## v0.29 - Built-in Screenshot / Automated Visual QA Harness

- Keep gameplay unchanged.
- Add a small engine-owned frame capture path so bounded local validation can prove GDI/DX11 presentation without desktop-level screenshot hacks.
- Prefer simple 32-bit BMP output and no new heavy dependency.
- Support exact file output and generated directory output from CLI/wrapper paths.
- Add tests/tooling around capture flag parsing, wrapper forwarding, BMP output validity, and bounded capture file existence.
- Document GDI/DX11 capture limitations honestly, especially DX11's temporary Win32 text overlay.

Status: implemented as a local visual-validation harness. `--capture-frame` and `--capture-dir` request one renderer-owned BMP capture after a stable frame, GDI reads its back buffer, DX11 reads the swap-chain back buffer, `scripts/play.ps1` forwards capture options, and `tools/capture_visual_smoke.py` validates GDI/DX11 BMP existence and non-blank output.

## v0.30 - Capture Parity / Visual QA Assertions Pass

- Keep gameplay unchanged.
- Build on v0.29's engine-owned BMP capture path.
- Strengthen `tools/capture_visual_smoke.py` with expected dimensions, color/luminance diversity, broad Ferry Office start-view scene-presence heuristics, and GDI/DX11 parity checks without brittle pixel-perfect golden tests.
- Add optional JSON report output under `build\captures`.
- Add focused tests for BMP parsing/stat logic, scene-presence buckets, parity, and report writing.
- Document what the harness can and cannot prove, especially that DX11 capture still excludes the temporary Win32 text overlay.

Status: implemented as a capture QA hardening pass. The visual smoke harness now validates 1280x720 32-bit BMP captures for non-flat output, luminance range, warm/green-teal/cool/dark scene signals, matching GDI/DX11 dimensions, and writes a JSON report alongside the captures.

## v0.31 - DX11 Capture Overlay Parity / Renderer-Owned Debug Text Spike

- Keep gameplay unchanged.
- Reduce the biggest remaining v0.30 visual QA blind spot: DX11 captures should include debug/playtest text before `Present`.
- Prefer a tiny dependency-free bitmap/monospace renderer-owned debug text path over DirectWrite, a full HUD system, or broad renderer architecture work.
- Keep GDI behavior stable and keep the text path explicitly temporary/debug-oriented.
- Extend `tools\capture_visual_smoke.py` with a conservative overlay text signal check without OCR or pixel-perfect golden tests.
- Add focused tests for text layout/raster/stat behavior.
- Update validation docs and run the standard doctor/configure/build/verify plus the visual capture harness.

Status: implemented as a narrow DX11 capture parity spike. DX11 now draws debug/playtest text through a tiny renderer-owned bitmap text path before capture and `Present`, while GDI keeps its existing overlay behavior. The visual smoke harness validates broad scene signals plus a conservative top-left bright text signal for both renderers.

## v0.32 - Automated Ferry Office Playthrough QA Harness

- Keep gameplay unchanged.
- Add a narrow QA-only automated validation path for the existing Ferry Office Service Call.
- Verify the first job state chain without manual keyboard/mouse play: manifest, service route, maintenance/power, gate, service vehicle, dock-road checkpoint, service-run confirmation, and job completion.
- Keep the path deterministic, off by default, and explicitly non-production.
- Write a report artifact under `build\playthroughs` and add focused tests for config parsing, report validation, and state completion.
- Avoid Job #2, mission scripting, NPCs, save/load, new vehicle physics, broad input rewrites, or a human-play replacement claim.

Status: implemented as a bounded behavioral QA harness. `--qa-playthrough ferry-office-service-call` and `tools\playthrough_qa.py` run the authored Ferry Office Service Call state chain, assert all required flags through `ferryOfficeJobComplete`, and write `build\playthroughs\ferry-office-service-call-report.json`.

## v0.33 - Jolt Runtime Physics Bridge / Ferry Office Collision Parity Spike

- Keep default gameplay unchanged.
- Promote the opt-in Jolt backend from an isolated spike toward a runtime candidate by validating the authored Ferry Office static collision scene through the engine physics API.
- Add a QA-only `--qa-physics-parity ferry-office-collision` path and `tools\physics_parity_qa.py`.
- Mirror scene-authored static AABB colliders plus a validation floor body into the requested physics backend and compare floor, raycast, and player-overlap-style probes against `PrototypeWorld`.
- Preserve the simple backend and default verification path.
- Avoid migrating player movement, traversal, service-gate behavior, vehicle driving, Jolt VehicleConstraint, Job #2, NPCs, save/load, mission scripting, or broad collision rewrites.

Status: implemented as an opt-in static scene-query parity bridge. The Jolt-enabled path validates 9 Ferry Office static colliders, 4 floor probes, 4 raycast probes, and 4 overlap probes, writes `build\physics\ferry-office-collision-parity-report.json`, and keeps live gameplay on the existing prototype collision path.

## v0.34 - Jolt Character Contact / Player Collision Probe

- Keep default gameplay unchanged.
- Build on v0.33's mirrored Ferry Office static scene and add a QA-only character/contact scenario for the current player collision proxy.
- Add `--qa-physics-parity ferry-office-character-contact` and `tools\character_contact_qa.py`.
- Compare deterministic player-proxy cases against `PrototypeWorld`: dock-floor grounding, office wall blocking, service gate blocking, service barrier blocking, clear lane movement, corner pushout, and opened-gate clear movement.
- Preserve live `PlayerController` movement on `PrototypeWorld`.
- Avoid Jolt VehicleConstraint, wheel/suspension physics, Job #2, NPCs, save/load, mission scripting, or broad collision rewrites.

Status: implemented as an opt-in read-only character/contact probe. The Jolt-enabled path validates 7 player-proxy contact cases, writes `build\physics\ferry-office-character-contact-report.json`, and keeps live gameplay on the existing prototype collision path.

## v0.35 - Jolt Vehicle Feasibility / Promotion Spike

- Keep default gameplay unchanged.
- Build a QA-only service-vehicle probe that answers whether Jolt wheeled vehicles are ready for a runtime-adapter goal.
- Keep Jolt vendor types private to engine physics code.
- Use the authored Ferry Office `service-yard-vehicle`, a compact throttle/steer/brake/reverse script, and a JSON promote/defer report.
- Preserve the deterministic live vehicle controller as fallback evidence.
- Avoid replacing live gameplay, adding traffic, Job #2, damage, save/load, broad vehicle tuning, or mission expansion.

Status: implemented as an opt-in vehicle feasibility probe. The Jolt-enabled path validates 5 compact service-yard samples with 4 wheel contacts per sample, writes `build\physics\ferry-office-vehicle-feasibility-report.json`, recommends `promote`, and keeps live gameplay on the deterministic vehicle path.

## v0.36 - Jolt Vehicle Runtime Adapter Comparison Spike

- Keep live gameplay on the deterministic vehicle path by default.
- Add a vendor-free frame-stepped vehicle runtime adapter boundary.
- Add an opt-in Jolt runtime adapter behind an explicit QA switch.
- Compare the Jolt runtime adapter against the current deterministic `VehicleController` using the same authored Ferry Office service vehicle and compact scripted input.
- Write a paired JSON report with deterministic samples, adapter samples, comparison deltas, and promote/defer evidence.
- Avoid live replacement, Job #2, traffic, damage, broad map expansion, renderer polish, and Jolt vendor leakage into `src/game`.

Status: implemented as an explicit runtime-comparison spike. The Jolt-enabled path validates 5 paired deterministic/adapter samples, writes `build\physics\ferry-office-vehicle-runtime-comparison-report.json`, recommends `promote`, and keeps normal play on the deterministic vehicle fallback.

## v0.37 - Live Opt-in Jolt Vehicle Playtest Switch

- Keep deterministic vehicle gameplay as the default.
- Expose the Jolt vehicle runtime adapter through an explicit live playtest switch.
- Keep Jolt vendor types private to `src/engine/physics`.
- Surface active vehicle runtime telemetry in debug text.
- Add wrapper/test coverage so the opt-in path is easy to launch and the default path does not drift.
- Avoid Job #2, traffic, damage, broad road expansion, renderer polish, and default replacement.

Status: implemented as a manual-playtest switch. `--vehicle-runtime jolt` and `scripts/play.ps1 -VehicleRuntime jolt -ExecutablePath build\windows-vs2022-debug-jolt\Debug\EngineApp.exe` select the Jolt-backed live adapter when the opt-in preset is built, while normal play still reports `vehicleRuntime=deterministic`.

## v0.38 - Clean Playtest Presentation / Overcast Scene Shading

- Move the default playtest render away from raw debug scaffolding while keeping F1/debug validation intact.
- Apply a tiny overcast face-shading pass to existing scene boxes and static mesh triangles.
- Reuse existing mesh assets to add a compact storm-wet dock-road work-zone cue near the first vehicle route endpoint.
- Preserve gameplay, collision, interaction logic, current asset loader scope, GDI/DX11 capture validation, and the debug overlay.
- Avoid textures, materials, PBR, lighting systems, new model files, new gameplay, broader map work, or an editor.

Status: implemented as a first presentation upgrade toward a scene that reads like a game rather than an editor/debug view. Playtest mode suppresses raw wire boxes, trigger radii, route lines, and debug marker beacons; debug mode keeps them behind F1. Existing scene geometry now gets simple face shading, and the dock road reports 37 mesh instances while still using the same 8 referenced model files.

## v0.39 - Scene Presentation Material Boundary

- Move current scene palette and overcast face-shading rules out of `SandboxLayer`.
- Add a small `ScenePresentation` boundary for color-key lookup, dynamic state colors, and shaded scene submissions.
- Add focused regression coverage for authored color keys, dynamic palette differences, overcast volume cues, and shaded box triangle submission.
- Preserve renderer behavior, scene data, gameplay, collision, asset formats, GDI/DX11 visual capture validation, and debug overlay behavior.
- Avoid a real material system, textures, PBR, shader pipeline, renderer rewrite, new authored assets, gameplay changes, or map expansion.

Status: implemented as a code-ownership step after v0.38. `SandboxLayer` now asks `ScenePresentation` for scene colors and shaded geometry submission, which gives later material/lighting work a clear game-layer staging point before any renderer-level material system exists.

## v0.40 - Tiny Scene Material Presets

- Let scene color keys resolve to `SceneMaterial` presets instead of only raw colors.
- Give wet asphalt/damp surfaces, matte wood/concrete, and painted service props different conservative overcast shading responses.
- Keep `SceneColorForKey` as a compatibility helper for tests and debug paths that only need the base palette.
- Preserve scene JSON shape, asset formats, renderer interfaces, gameplay, collision, GDI/DX11 capture validation, and debug overlay behavior.
- Avoid textures, UVs, normals in renderer input, PBR, shader files, production material resources, new model files, gameplay changes, or map expansion.

Status: implemented as the first material-language step inside `ScenePresentation`. The current scene still renders through flat debug triangles, but the submitted colors now carry surface-specific overcast response terms instead of using one universal shade formula.

## v0.41 - Scene-Authored Material Presets

- Add `sceneMaterials` to the Ferry Office scene data.
- Load material presets through `SceneLoader` without exposing renderer types in `SceneDefinition`.
- Validate that every used `colorKey` has a matching material preset and that responses/colors stay bounded.
- Route `SandboxLayer` through the authored material presets while preserving dynamic state colors and fallback palette behavior.
- Preserve renderer interfaces, asset formats, gameplay, collision, GDI/DX11 capture validation, and debug overlay behavior.
- Avoid textures, UVs, normal maps, shader files, PBR, production material resources, new model files, gameplay changes, or map expansion.

Status: implemented as a data-ownership step after v0.40. The current presentation still uses flat shaded triangles, but surface intent now lives in the authored scene file as wet/matte/painted presets instead of being only hardcoded in C++.

## v0.42 - Wet Road Surface Geometry Pass

- Add one controlled Blender-authored wet-road surface mesh for the biggest service-yard and dock-road placeholder slabs.
- Replace only the visual presentation of the service-yard driving pad, dock-road segment, and dock-road turn-around pad with shallow surfaced geometry.
- Keep scene materials, static mesh validation, capture/playthrough QA, default validation, and DX11 capture compatibility intact.
- Preserve current collision, vehicle bounds, vehicle physics, gameplay, route markers, and debug overlay behavior.
- Avoid textures, UVs, renderer material resources, terrain, road splines, traffic, Job #2, or a broad map expansion.

Status: implemented as a focused visual surface pass. The Ferry Office scene now references 9 mesh assets and 40 mesh instances, including `blender_wet_road_surface.gltf` used three times to make the service-yard and dock-road slabs read less like flat debug plates while keeping all gameplay collision and vehicle behavior unchanged.

## v0.43 - Harbor Backdrop / Island Silhouette Pass

- Add one controlled Blender-authored harbor backdrop mesh for distant low shoreline and harbor-utility silhouettes.
- Use it around the existing dock-start and dock-road water-edge bands so the Ferry Office scene reads more like an island pier and less like a road floating in empty space.
- Add a small scene-authored material key for misty island ground/backdrop silhouettes.
- Preserve current gameplay, collision, vehicle bounds, road surfaces, scene validation, asset validation, DX11 visual smoke, and playthrough QA.
- Avoid terrain, water simulation, road splines, streaming world, city expansion, textures/materials/PBR, collision import, or map-boundary behavior.

Status: implemented as a background composition pass. The Ferry Office scene now references 10 mesh assets and 43 mesh instances, including `blender_harbor_backdrop.gltf` used three times around the existing water-edge cues while keeping all gameplay and collision unchanged.

## v0.44 - Harbor Water Surface Geometry Pass

- Add one controlled Blender-authored harbor water surface mesh with shallow static ripple geometry.
- Use it over the existing dock-start and dock-road water-edge placeholder bands so the shoreline reads as authored water instead of flat debug strips.
- Preserve current gameplay, collision, vehicle bounds, road surfaces, harbor backdrop, scene validation, asset validation, DX11 visual smoke, and playthrough QA.
- Avoid water simulation, transparency, fluid physics, shader/material systems, terrain, map expansion, or collision import.

Status: implemented as a focused water presentation pass. The Ferry Office scene now references 11 mesh assets and 46 mesh instances, including `blender_harbor_water_surface.gltf` used three times over the existing water-edge cues while keeping all gameplay and collision unchanged.

## v0.45 - Vehicle Runtime Controls QA Hardening

- Extend the existing Ferry Office vehicle runtime comparison report with controls-focused checks for short throttle tap/coast, braking, reverse motion, and reverse coast-down.
- Keep deterministic vehicle gameplay as the default and keep the Jolt live runtime behind the explicit opt-in path.
- Require the Python vehicle runtime QA wrapper to reject stale reports that do not include the controls-focused checks.
- Preserve current gameplay, scene data, renderer presentation, physics backend boundaries, and the existing live vehicle switch.
- Avoid default Jolt promotion, vehicle retuning, Job #2, traffic, damage, broader map work, or Jolt vendor leakage into `src/game`.

Status: implemented as a validation hardening pass. `tools\vehicle_runtime_qa.py` now requires four control checks in addition to paired deterministic/Jolt samples, and the Jolt-enabled runtime comparison still recommends `promote` for continued opt-in testing with `maxPositionDelta=2.95`.

## v0.46 - Vehicle Route Completion QA Proxy

- Extend the deterministic-vs-Jolt vehicle runtime comparison with a service-run route completion check.
- Drive both the deterministic baseline and selected runtime adapter from the authored service-yard vehicle spawn toward the scene-authored service-run checkpoint.
- Report checkpoint reached, frames to checkpoint, minimum checkpoint distance, final position/yaw, and bounds hits.
- Require `tools\vehicle_runtime_qa.py` to reject stale reports that do not include deterministic and Jolt route checks.
- Preserve deterministic vehicle gameplay as the default and keep Jolt live runtime behind the explicit opt-in path.
- Avoid default Jolt promotion, vehicle retuning, Job #2, traffic, damage, map expansion, or Jolt vendor leakage into `src/game`.

Status: implemented as an automated proxy for the deferred manual deterministic-vs-Jolt route comparison. The Jolt-enabled report now includes 5 paired samples, 4 controls checks, and 2 route checks. Deterministic reaches the service-run checkpoint in 139 frames; Jolt reaches it in 301 frames. Both stay in bounds, so Jolt remains a viable opt-in candidate, but deterministic stays default until Jolt route pace is tuned.

## v0.47 - Jolt Route Pace Tuning

- Tune only the opt-in Jolt vehicle runtime so the authored Ferry Office service-run route closes the v0.46 pace gap without changing deterministic default gameplay.
- Enforce a 240-frame service-run checkpoint budget in C++ route QA and the Python `tools\vehicle_runtime_qa.py` wrapper.
- Keep tap/coast, brake, reverse, reverse coast, bounds, and no-vendor-leak checks green.

Status: implemented as an opt-in Jolt vehicle runtime tuning pass. Deterministic still reaches the checkpoint in 139 frames; Jolt now reaches it in 213 frames under the 240-frame automated budget, keeps all controls checks green, stays in bounds, and remains behind the explicit Jolt runtime path.

## v0.48 - Input-Scripted Service Vehicle Runtime Playthrough QA

- Strengthen the Ferry Office Service Call playthrough QA so the vehicle beat uses runtime input-style behavior instead of direct world-state shortcuts.
- Enter the scene-authored service vehicle through `VehicleController` focus/interaction.
- Drive the authored service vehicle to the dock-road checkpoint under the 240-frame route budget.
- Exit the vehicle through runtime input after a clear-exit overlap check, then confirm the service run.
- Require the Python playthrough wrapper and C++ report test to reject reports that skip the runtime vehicle steps.
- Preserve deterministic default gameplay and keep Jolt behind the explicit opt-in runtime path.

Status: implemented as a stronger first-job behavioral QA pass. The default playthrough report now records `serviceVehicleRuntime`, `dockRoadRuntimeCheckpoint`, and `serviceVehicleRuntimeExit`; the service vehicle reaches the dock-road checkpoint in 139 frames and the job completes with 10 world events.

## v0.49 - Opt-in Jolt Live-loop Playthrough QA

- Extend the Ferry Office Service Call playthrough QA so the vehicle beat can use the selected runtime backend.
- Record requested vehicle runtime, actual backend, fallback use, bounds hit, checkpoint timing, final position, and final yaw in the playthrough report.
- Let `tools\playthrough_qa.py --vehicle-runtime jolt` validate the first service job through a Jolt-enabled build.
- Preserve deterministic default gameplay and keep vendor-specific Jolt details behind `src\engine\physics`.

Status: implemented as comparable first-job vehicle evidence for the opt-in Jolt path. The deterministic playthrough still reaches the dock-road checkpoint in 139 frames; the Jolt playthrough reaches it in 213 frames, exits at a clear position, confirms the service run, and reports no fallback or bounds hit. Jolt remains opt-in rather than default.

## v0.50 - Dock Road Relay Follow-up Beat

- Add one small authored follow-up interaction after the Ferry Office Service Call.
- Place a Dock Road Relay marker at the service-run endpoint using existing scene/prop language.
- Record a remembered `dockRoadRelayReset` world-state flag only after the first service call is complete.
- Extend playthrough QA so the first service job now ends with the relay reset consequence.
- Avoid a generic mission framework, new vehicle physics, new assets, save/load, NPCs, economy, or map expansion.

Status: implemented as a second compact service beat. The scene now has 7 interactables, 7 route markers, and 6 objective markers; the playthrough reaches the service-run checkpoint in 139 frames and completes with 11 world events, ending in `dockRoadRelayReset=true`.

## v0.51 - Relay Reset Presentation State

- Add a small authored status-light placeholder beside the Dock Road Relay.
- Add `dock-road-relay-state` to scene materials and dynamic presentation colors.
- Turn the relay cue from warning orange to reset green when `dockRoadRelayReset` is true.
- Keep this as a tiny world-state feedback cue, not a lighting system, save/load feature, new mesh asset, or broader job framework.

Status: implemented and validated. Scene data now has 19 scene materials and 25 visual placeholders, and the deterministic playthrough still completes the 11-event service-call plus relay loop.

## v0.52 - Relay Service Log Follow-up Beat

- Add a second tiny endpoint interaction that depends on the Dock Road Relay reset.
- Record `dockRoadRelayLogged` only after `dockRoadRelayReset`.
- Place the sign-off on the existing service-run review board instead of adding a new mesh asset or mission framework.
- Extend playthrough QA so the service-call follow-up chain now ends with the relay log.

Status: implemented and validated. Scene data now has 8 interactables, 8 route markers, and 7 objective markers; deterministic and opt-in Jolt playthroughs both complete with 12 events, with checkpoint timing still 139 frames for deterministic and 213 frames for Jolt.

## v0.53 - Vehicle Steering Obstacle Proxy QA

- Add an obstacle-proxy steering check to `ferry-office-vehicle-runtime-comparison`.
- Require deterministic and opt-in Jolt reports to prove bounded lateral steering response, forward progress, and no authored-bounds hits.
- Keep this as automated promotion evidence, not a physical cone/collision course or human feel replacement.

Status: implemented and validated. The passing Jolt report records 2 obstacle-proxy checks: deterministic reaches max lateral offset 1.96 and final x 18.82, while Jolt reaches max lateral offset 0.29 and final x 11.97 over the same 150-frame proxy. Both stay in bounds, but the behavioral difference keeps deterministic as the default for now.

## v0.54 - Dock Road Clearance Tag Consequence

- Add one tiny scene-authored endpoint interaction after `dockRoadRelayLogged`.
- Record `dockRoadClearanceTagged` only after the relay reset has been logged.
- Extend scene routes, objective markers, C++ tests, and playthrough QA so the service-call follow-up chain ends with a visible road-clear tag.
- Keep this as compact content growth, not a mission framework, save system, new mesh asset, or default vehicle-runtime promotion.

Status: implemented as a small remembered world-state consequence. Scene data now has 9 interactables, 9 route markers, and 8 objective markers; deterministic and opt-in Jolt playthroughs both complete with 13 events, with checkpoint timing still 139 frames for deterministic and 213 frames for Jolt.

## v0.55 - Dock Road Follow-up Readability Pass

- Add a compact playtest follow-up status for the relay reset, relay log, and road-clear tag.
- Keep initial playtest text uncluttered; show the follow-up summary only after the first service job completes or follow-up progress exists.
- Preserve raw debug telemetry behind `F1` and avoid adding a HUD framework or mission tracker.

Status: implemented and validated. Playtest mode now adds `Follow-up: relay=... | log=... | road=...` for the Dock Road endpoint chain; focused tests, deterministic playthrough QA, and GDI/DX11 visual smoke pass.

## v0.56 - Camera-aware Vehicle Obstacle Evidence

- Extend `ferry-office-vehicle-runtime-comparison` obstacle checks with camera-follow yaw telemetry.
- Validate deterministic and Jolt obstacle-proxy steering under the same scripted input.
- Allow the QA report to pass while recommending `defer` if Jolt remains stable but obstacle-route progress diverges too much for default promotion.

Status: implemented and validated. Both deterministic and Jolt obstacle checks stay camera-readable, with max camera yaw deltas of 4.8 and 2.9 degrees respectively, but Jolt still finishes much earlier on the route proxy (`x=11.97` versus deterministic `x=18.82`), so deterministic remains default and Jolt remains opt-in.

## v0.57 - Clearance Tag Visual Cue

- Add a small scene-authored status placeholder beside the Dock Road Clearance Tag endpoint.
- Add `dock-road-clearance-state` to scene materials, dynamic presentation colors, and scene validation tooling.
- Turn the clearance cue from inactive metal to clear cyan when `dockRoadClearanceTagged` is true.
- Keep this as spatial endpoint readability, not a lighting system, bespoke mesh asset, save/load state, or longer content chain.

Status: implemented and validated. Scene data now has 20 scene materials and 26 visual placeholders; deterministic playthrough QA still completes the 13-event service-call follow-up chain, and GDI/DX11 visual smoke remains green.

## v0.58 - Clearance Tag Mesh Prop

- Generate a tiny original `clearance_tag.gltf` fallback prop inside the existing embedded-buffer glTF subset.
- Add `clearance-tag-mesh` and `mesh-dock-road-clearance-tag` to the Ferry Office scene.
- Bind the mesh to `dock-road-clearance-state` so the endpoint prop still shifts from inactive metal to clear cyan after tagging.
- Keep this as a visual-only endpoint prop, not a texture/decal pass, collision source, save/load feature, or new asset pipeline.

Status: implemented and validated. Scene data now has 12 mesh assets, 47 mesh instances, and 12 referenced model files; deterministic playthrough QA still completes the 13-event service-call follow-up chain, and visual smoke remains green.

## v0.59 - Jolt Obstacle Progress Tuning

- Tighten `tools\vehicle_runtime_qa.py` so stale obstacle reports fail when deterministic and Jolt final X progress differs by more than 4.0 units.
- Tune only the opt-in Jolt vehicle runtime adapter to preserve compact steering-route progress.
- Keep deterministic as the default vehicle runtime while treating Jolt as a stronger opt-in candidate.
- Avoid collision-backed obstacle routes, default runtime promotion, vehicle damage, traffic, or vendor leakage into `src/game`.

Status: implemented and validated. Jolt vehicle runtime QA now passes with maxPositionDelta=1.49 and recommendation=`promote`; the obstacle proxy final-X gap shrank from 6.85 to about 2.91 units, Jolt reaches the service checkpoint in 212 frames, opt-in Jolt playthrough QA completes the 13-event chain, and deterministic remains default pending collision-backed evidence.

## v0.60 - Harbor Parts Return Micro-slice

- Add a compact second content loop after Dock Road clearance: pick up Harbor Parts by the dock-road cabinet and return them to the Ferry Office shelf.
- Record `harborPartsPickedUp` and `harborPartsDelivered` as remembered local state.
- Extend scene routes/objective markers and scripted playthrough QA to cover the return trip.
- Keep this as authored compact content, not an inventory system, save/load feature, economy, NPC job board, or generic mission framework.

Status: implemented and validated. Scene data now has 11 interactables, 11 route markers, and 10 objective markers; deterministic playthrough QA completes the 15-event chain with Harbor Parts pickup and delivery, while the initial playtest prompt still focuses the Ferry Manifest.

## v0.61 - Follow-up Next-step Readability

- Add compact next-step text for the post-service-call follow-up chain.
- Surface it in playtest mode after the service call reaches follow-up state.
- Keep minimal mode small and keep the initial prompt focused on the Ferry Manifest.
- Avoid a HUD framework, minimap, route-arrow system, or new content.

Status: implemented and validated. Playtest text now pairs the compact `Follow-up:` state summary with a `Next:` line for relay, log, clear tag, Harbor Parts pickup, Harbor Parts delivery, and completion; focused tests, deterministic playthrough QA, visual smoke, and `scripts\verify.ps1` pass.

## v0.62 - Scene Action Bindings

- Let interactables declare immediate world flags, prerequisite world flags, and ready-state world flags in scene data.
- Convert authored flag strings through the engine/game world-state table instead of ad hoc per-interactable string handling.
- Keep rich job behavior in C++ where it still adds logic, but remove the need for every simple flag beat to become another hardcoded branch.
- Preserve the existing Ferry Office, Dock Road, and Harbor Parts playthrough behavior.

Status: implemented and validated. Ferry Office scene data now declares prerequisites for exit, service confirmation, relay, log, clear tag, Harbor Parts pickup, and Harbor Parts delivery; custom binding tests prove non-hardcoded immediate and gated interactables work; the 15-event playthrough remains green.

## v0.63 - Ferry Office Work Board Signoff

- Add a final compact Ferry Office Work Board signoff after Harbor Parts delivery.
- Record `ferryOfficeBoardUpdated` as remembered local state.
- Use scene-authored prerequisites/action bindings for the work-board interaction.
- Extend scene routes/objective markers and scripted playthrough QA to cover the 16-event chain.
- Keep this as compact authored content, not a job-board UI, inventory system, save/load feature, or generic mission framework.

Status: implemented and validated. Scene data now has 12 interactables, 12 route markers, and 11 objective markers; deterministic playthrough QA completes the 16-event chain through Harbor Parts delivery and Ferry Office Work Board signoff.

## v0.64 - Ferry Office Handoff Note Beat

- Add one final small office-side handoff note after the Ferry Office Work Board signoff.
- Record `ferryOfficeHandoffFiled` as remembered local state through scene-authored action bindings.
- Extend the authored route/objective marker chain and playthrough QA so the return loop now ends with a filed next-crew handoff.
- Keep this as compact content, not a job-board UI, inventory system, save/load feature, NPC dispatcher, generic mission framework, or new asset pass.

Status: implemented and validated. Scene data now has 13 interactables, 13 route markers, and 12 objective markers; deterministic playthrough QA completes the 17-event chain through Ferry Office handoff filing.

## v0.65 - Collision-backed Vehicle Obstacle Replay

- Extend the deterministic-vs-Jolt vehicle runtime comparison with overlap-backed obstacle clearance telemetry.
- Require deterministic and opt-in Jolt obstacle replays to clear a QA-only physics trigger probe with zero overlap frames.
- Preserve existing controls, route, camera-aware obstacle, and progress-alignment evidence.
- Keep deterministic vehicle gameplay as the default and keep Jolt behind the explicit opt-in runtime path.
- Avoid live vehicle collision migration, default Jolt promotion, damage, traffic, road-edge collision, or vendor leakage into `src/game`.

Status: implemented and validated. The generated Jolt runtime comparison report now records collision-backed obstacle clearance for deterministic and Jolt replays, both with `obstacleOverlapFrames=0`; `tools\vehicle_runtime_qa.py` reports backend `jolt`, `maxPositionDelta=1.49`, and recommendation `promote` for continued opt-in evidence.

## v0.66 - Ferry Office Handoff State Cue

- Add a compact authored visual state cue for the Ferry Office Handoff Note endpoint.
- Drive the cue from `ferryOfficeHandoffFiled` through the existing scene presentation dynamic palette path.
- Keep it as a placeholder-level readability pass, not a bespoke asset or broader UI/job framework.

Status: implemented and validated. Scene data now has 21 authored scene materials and 27 visual placeholders; the handoff cue shifts from pending amber to filed green through `ScenePresentationState`, while scene validation, playthrough QA, visual smoke, and the main verify gate cover the change.

## v0.67 - Storm Pump Job Seed

- Add a compact second job seed after the Ferry Office handoff note.
- Route the player from the handoff note to the service-yard storm pump, then back to a Ferry Office pump ticket closeout.
- Record `stormPumpReset` and `stormPumpTicketClosed` as remembered local world state.
- Keep this as scene-authored compact content, not a generic mission framework, job-board UI, save/load system, or bespoke asset pass.

Status: implemented and validated. Scene data now has 15 interactables, 15 route markers, and 14 objective markers; deterministic playthrough QA completes a 19-event chain through storm pump reset and ticket closeout.

## v0.68 - Provisional Jolt Vehicle Runtime Direction

- Refresh the Jolt preset after the v0.67 scene/content expansion.
- Rerun Jolt CTest and the physics/vehicle QA tools against the current authored Ferry Office scene.
- Make a provisional vehicle-runtime direction decision from automated evidence.

Status: implemented and validated. Jolt CTest passes 15/15, physics parity/contact/vehicle feasibility/runtime QA all pass after rebuilding the stale Jolt executable, and Jolt is now the preferred vehicle-runtime candidate for the next live hardening/default-promotion milestone while deterministic remains the default runtime.

## v0.69 - Storm Pump State Cues

- Add compact authored visual state cues for the storm pump reset and Ferry Office pump ticket closeout.
- Drive them from `stormPumpReset` and `stormPumpTicketClosed` through the existing dynamic scene-palette path.
- Keep the work placeholder-level and validation-friendly, not a bespoke mesh/animation asset pass.

Status: implemented and validated. Scene data now has 23 authored scene materials and 29 visual placeholders; the storm pump cue shifts from warning orange to active blue, and the ticket cue shifts from pending amber to closed green.

## v0.70 - Preferred Jolt Live Runtime Trial

- Add `--vehicle-runtime preferred` as a controlled live-runtime selection.
- Make `scripts/play.ps1` default to `-VehicleRuntime preferred`.
- Resolve preferred to Jolt only in Jolt-enabled executables; dependency-free builds and direct app/QA defaults remain deterministic.
- Preserve deterministic as an explicit runtime and fallback path.

Status: implemented and validated. `--vehicle-runtime preferred` resolves to deterministic in the dependency-free executable and to `jolt-live` in the Jolt-enabled executable; `scripts/play.ps1` now requests preferred by default while direct app and QA defaults remain deterministic.

## v0.71 - Low Dock Drain Clear Tag

- Add a compact low dock drain clear-tag follow-up after the storm pump ticket.
- Route the player from the Ferry Office pump ticket back to the low dock drain beside the service-run marker.
- Record `lowDockDrainCleared` as remembered local world state and drive a compact dynamic status light from it.
- Keep this as authored content growth, not a larger job framework or water/pump simulation.

Status: implemented and validated. The deterministic and Jolt playthroughs now complete a 20-event Ferry Office chain including the low dock drain clear tag; scene/tool tests, visual smoke, vehicle runtime QA, Jolt CTest, and `scripts/verify.ps1` pass.

## v0.72 - Default Camera Composition Pass

- Improve the first playable frame by reducing empty background weight and showing more dock, office approach, and service-yard ground context.
- Keep this as a presentation camera tweak, not a new scene layout, prop pass, renderer rewrite, or gameplay expansion.
- Preserve existing player start, route guidance, scene data, and validation/capture tooling.

Status: implemented and validated. The default third-person camera now starts at a 22 degree pitch, making the dock and Ferry Office approach occupy more of the GDI/DX11 visual smoke captures while `scripts/verify.ps1` and scene/asset/mesh/capture checks pass.

## v0.73 - Ferry Office Gate Focal Hierarchy Pass

- Break up the flat closed service-gate/facade silhouette in the default Ferry Office first view.
- Add only visual-only gate structure using existing placeholder mesh/material language.
- Preserve service-gate collision, route state, prompts, and gameplay behavior.

Status: implemented and validated. The scene now has 50 mesh instances, including three visual-only closed-gate slat/seam meshes that make the Ferry Office gate read as weathered service infrastructure rather than one flat slab; `scripts/verify.ps1`, scene/asset/mesh/scale checks, and GDI/DX11 visual smoke pass.

## v0.74 - Dock Foreground Material Rhythm Pass

- Add subtle visual-only board seams to the starting dock foreground so the default frame reads less like a single flat slab.
- Use existing `unit-box-mesh` and `wet-timber` material language rather than adding new assets or clutter.
- Preserve dock collision, player start, route markers, and gameplay behavior.

Status: implemented and validated. The scene now has 54 mesh instances, including four visual-only dock-board seam meshes that add damp foreground rhythm and lead the eye toward the Ferry Office approach; `scripts/verify.ps1`, scene/asset/mesh/scale checks, and GDI/DX11 visual smoke pass.

## v0.75 - Overcast Background Separation Pass

- Adjust the default renderer clear color so the first frame reads as restrained coastal overcast rather than a near-black void.
- Keep this as a tiny atmospheric baseline change, not a sky, fog, lighting, post-process, or weather system.
- Preserve scene layout, gameplay, text overlay behavior, and validation/capture tooling.

Status: implemented and validated. GDI and DX11 now use a slightly lighter overcast blue-gray default clear color; `scripts/verify.ps1`, scene/asset/mesh/scale checks, and GDI/DX11 visual smoke pass while text contrast remains usable.

## v0.76 - Player Proxy Silhouette Cleanup

- Make the centered third-person stand-in read less like a single debug column in the default Ferry Office frame.
- Split the placeholder into compact leg, torso, shoulder, and head volumes while keeping it static and validation-friendly.
- Preserve player collision, camera behavior, prompts, route flow, and interaction behavior.

Status: implemented and validated. The default player proxy now has a clearer third-person silhouette in capture while staying within the existing placeholder renderer path; `scripts/verify.ps1`, scene/asset/mesh/scale checks, and GDI/DX11 visual smoke pass.

## v0.77 - Ferry Office Distant Shoreline Pass

- Reduce the empty upper-background read in the default Ferry Office frame.
- Reuse the existing harbor-backdrop mesh behind the Ferry Office roofline as a restrained distant shoreline silhouette.
- Preserve collision, route flow, camera behavior, prompts, and gameplay behavior.

Status: implemented and validated. The default Ferry Office frame now has a muted shoreline layer behind the roofline, giving the office more coastal depth without adding foreground clutter; `scripts/verify.ps1`, scene/asset/mesh/scale checks, and GDI/DX11 visual smoke pass.

## v0.78 - Ferry Office Sign Detail Pass

- Make the visible Ferry Office approach sign read less like a blank yellow placeholder slab.
- Add restrained visual-only sign rules using existing placeholder geometry and palette keys.
- Preserve prompts, route flow, collision, camera behavior, and gameplay behavior.

Status: implemented and validated. The approach notice board now has simple blue rule detail in the default frame, improving Ferry Office identity without readable text or texture scope; `scripts/verify.ps1`, scene/asset/mesh/scale checks, and GDI/DX11 visual smoke pass.

## v0.79 - Playtest Proxy Readability Pass

- Make the centered player proxy and visible service-vehicle area read less like raw debug blocks.
- Add simple player arms, compact service-vehicle/bay cues, and vehicle-linked mesh offset support for authored vehicle details.
- Preserve collision, player movement, vehicle runtime behavior, prompts, route flow, and interaction behavior.

Status: implemented and validated. The playtest frame now has a more character-like player proxy plus clearer service-yard vehicle/bay presentation while keeping the work in the existing placeholder renderer path; `scripts/verify.ps1`, scene/asset/mesh/scale checks, playthrough QA, and GDI/DX11 visual smoke pass.

## v0.80 - Ferry Office Roofline Grounding Pass

- Make the Ferry Office roofline feel attached to the facade rather than floating above it.
- Add restrained visual-only fascia and paired downposts using existing placeholder geometry and palette keys.
- Preserve collision, route flow, camera behavior, prompts, and gameplay behavior.

Status: implemented and validated. The default Ferry Office frame now has a clearer facade/roof connection, improving the authored building read without route clutter; `scripts/verify.ps1`, scene/asset/mesh/scale checks, and GDI/DX11 visual smoke pass.

## v0.81 - Ferry Office Canopy Mesh Pass

- Replace the largest high-visibility Ferry Office unit-box roof slab/front fascia with one small original canopy mesh.
- Use the existing static mesh path and scene material palette; avoid textures, readable text, collision changes, or route clutter.
- Preserve player start, camera behavior, prompts, route flow, and gameplay behavior.

Status: implemented and validated. The default Ferry Office frame now shows a shallow sloped canopy mesh instead of a stacked roof slab/front strip, making the office read more like authored coastal service architecture while `scripts/verify.ps1`, scene/asset/mesh/scale checks, and GDI/DX11 visual smoke pass.

## v0.82 - Overcast Sky Readability Pass

- Lift the default renderer clear color slightly so the first frame reads less like a dark void and more like damp coastal overcast.
- Keep this as a bounded presentation tweak, not a sky system, fog renderer, lighting pass, or weather feature.
- Preserve scene layout, gameplay behavior, text overlay readability, and capture validation.

Status: implemented and validated. The default background now uses a restrained lighter blue-gray clear color while the playtest overlay remains readable; `scripts/verify.ps1`, scene/asset/mesh/scale checks, and GDI/DX11 visual smoke pass.

## Recommended Next Goal

Use `python tools\capture_visual_smoke.py`, `python tools\playthrough_qa.py`, `python tools\physics_parity_qa.py`, opt-in `python tools\character_contact_qa.py`, opt-in `python tools\vehicle_physics_qa.py`, and opt-in `python tools\vehicle_runtime_qa.py` as bounded evidence before asking for manual play. After v0.82, the next useful future milestone should either improve route/material hierarchy around the service-yard-to-dock-road transition, replace another high-visibility repeated unit-box cue with a purposeful original mesh, add an automated route-walking proxy for the longer Ferry Office follow-up chain, or use preferred-runtime evidence to harden Jolt road-edge/live-control behavior.
