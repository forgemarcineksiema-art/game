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

## v0.6 - NPC / Event Prototype

- Add a minimal event bus or scene event model.
- Add simple NPC placeholders with deterministic behavior.
- Avoid complex AI planners.

## v0.7 - Mission Slice

- Build one original micro-scenario using project-owned placeholder assets.
- Exercise player, camera, world, interaction, and event systems.
- Keep the slice small enough to validate in one run.

## Recommended Next Goal

Build v0.5.1 Traversal Feel + Camera / Collision Polish.
