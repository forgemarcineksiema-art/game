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

## Recommended Next Goal

Build v0.8 Prototype Scene Naming + Data Cleanup.
