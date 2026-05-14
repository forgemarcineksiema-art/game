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

## v0.5 - Vehicle or Traversal Prototype

- Choose one narrow prototype: simple vehicle handling or advanced traversal.
- Do not build both in the same milestone.
- Keep physics lightweight and explicit.

## v0.6 - NPC / Event Prototype

- Add a minimal event bus or scene event model.
- Add simple NPC placeholders with deterministic behavior.
- Avoid complex AI planners.

## v0.7 - Mission Slice

- Build one original micro-scenario using project-owned placeholder assets.
- Exercise player, camera, world, interaction, and event systems.
- Keep the slice small enough to validate in one run.

## Recommended Next Goal

Build v0.5 Vehicle or Traversal Prototype Decision + first narrow prototype.
