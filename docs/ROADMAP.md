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

## Recommended Next Goal

Run a short human v0.26 visual/playability pass through `scripts/play.ps1`, checking whether the manifest props, service panel, dock cleats, service-yard crate, and Service Run review board improve readability without hiding prompts, markers, or route/debug geometry. Use that feedback to choose between a tiny authored Blender prop follow-up, vehicle-feel polish, or a renderer/depth presentation spike.
