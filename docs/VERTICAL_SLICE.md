# Vertical Slice

Last updated: 2026-05-14

## Slice Name

`The Ferry Office`

## Purpose

Define the first playable micro-scenario for the project direction locked in `docs/GAME_DIRECTION.md`.

This document is design scope only. It does not authorize adding missions, NPC AI, combat, vehicles, inventory, physics libraries, final art, or asset pipelines in v0.4.1.

## Player Experience Target

In about 10 minutes, the player should understand:

- where they are,
- how to move and look around,
- how to read collision and traversal affordances,
- how to focus and interact with objects,
- how one small action changes the world state,
- why the region remembers meaningful actions.

## Scenario Summary

The player arrives at a closed ferry office on Veyra Reach. The main office door is locked, the service gate is blocked, and the power box is reachable only through a small traversal route. The player restores power, returns to the office, collects a manifest marker, and leaves with a visible state change recorded.

## Required Prototype Beats

1. Start at a dock-side spawn point.
2. Walk to a visible ferry office landmark.
3. Encounter a blocked direct route.
4. Use simple traversal to reach a maintenance box.
5. Interact with the maintenance box.
6. Show a debug state change: `powerRestored=true`.
7. Return through a newly readable route or opened debug gate.
8. Interact with a pickup-like manifest marker.
9. Show a debug state change: `manifestCollected=true`.
10. End at an exit marker with the slice state summary visible.

## Systems Required Before This Slice Feels Real

- Stable player movement and jump.
- Camera orbit and follow with no normal-movement jitter.
- Static collision that handles walls, corners, and narrow paths.
- Interaction focus with predictable prompt behavior.
- A small world-state model for remembered local flags. v0.6 provides the first runtime-only version with `powerRestored`, `manifestCollected`, `serviceRouteUsed`, `maintenanceBoxInspected`, and `routeOpened`.
- Simple traversal, recommended as the next v0.5 direction.
- Debug text that can show slice state without a full UI framework.

## v0.6 Prototype State Hooks

- Completing the Service Barrier Vault records `serviceRouteUsed=true`.
- Inspecting the Maintenance Box records `maintenanceBoxInspected=true` and `powerRestored=true`.
- Collecting the Ferry Manifest records `manifestCollected=true`.
- Toggling the Wall Button records `routeOpened=true` or `routeOpened=false`.
- GDI debug text/logs show current flags, event count, last event, and whether the current action changed remembered state.

## Systems Explicitly Deferred

- Vehicles.
- NPC AI.
- Combat.
- Weapons.
- Dialogue trees.
- Mission scripting.
- Inventory.
- Save/load.
- Physics engine integration.
- Asset import pipeline.
- Final art.

## v0.5 Recommendation From Slice Needs

Choose traversal for v0.5.

Reason: the slice depends on on-foot access gating, readable obstacles, and route changes. Vehicles would expand scene scale and control complexity before the first playable space has a strong walking, camera, collision, and interaction feel.

## Acceptance Checklist For The Future Slice

- The player can complete the slice without console commands.
- All required interactions have visible debug prompts.
- One-shot interactions cannot be repeated after completion.
- Toggle/state interactions visibly change the route or debug state.
- The camera remains stable in narrow spaces.
- Collision prevents walking through walls and allows the intended path.
- The slice can be validated in a bounded run plus a manual checklist.
