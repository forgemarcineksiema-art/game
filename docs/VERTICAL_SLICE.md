# Vertical Slice

Last updated: 2026-05-14

## Slice Name

`The Ferry Office`

## Purpose

Define the first playable micro-scenario for the project direction locked in `docs/GAME_DIRECTION.md`.

This document defines the micro-slice target and current v0.7 prototype state. It does not authorize adding missions, NPC AI, combat, vehicles, inventory, physics libraries, final art, or asset pipelines.

## Player Experience Target

In about 10 minutes, the player should understand:

- where they are,
- how to move and look around,
- how to read collision and traversal affordances,
- how to focus and interact with objects,
- how one small action changes the world state,
- why the region remembers meaningful actions.

## Scenario Summary

The player arrives at a closed ferry office on Veyra Reach. The service gate is blocked, the manifest marker is near the dock office, and the maintenance box is reachable through the Service Barrier Vault route. The player collects or inspects the manifest, uses traversal to reach maintenance, restores local power, opens the service route, and reaches an exit marker with a visible remembered-state summary.

## Required Prototype Beats

1. Start at a dock-side spawn point.
2. Walk to a visible ferry office landmark.
3. Encounter a blocked direct route.
4. Interact with a pickup-like manifest marker.
5. Show a debug state change: `manifestCollected=true`.
6. Use simple traversal to reach a maintenance box.
7. Interact with the maintenance box.
8. Show a debug state change: `powerRestored=true`.
9. Open or visually change the debug service gate.
10. End at an exit marker with the slice state summary visible.

## Systems Required Before This Slice Feels Real

- Stable player movement and jump.
- Camera orbit and follow with no normal-movement jitter.
- Static collision that handles walls, corners, and narrow paths.
- Interaction focus with predictable prompt behavior.
- A small world-state model for remembered local flags. v0.7 uses `powerRestored`, `manifestCollected`, `serviceRouteUsed`, `maintenanceBoxInspected`, `routeOpened`, and `exitReached`.
- Simple traversal through the Service Barrier Vault.
- Debug text that can show slice state without a full UI framework.

## v0.7 Prototype State Hooks

- Completing the Service Barrier Vault records `serviceRouteUsed=true`.
- Inspecting the Maintenance Box records `maintenanceBoxInspected=true` and `powerRestored=true`.
- Collecting the Ferry Manifest records `manifestCollected=true`.
- Toggling the Wall Button records `routeOpened=true` or `routeOpened=false`.
- Reaching the Exit Summary Marker records `exitReached=true` only after the required loop is ready.
- `routeOpened=true` disables the `service-gate` blocking collider; closing the route enables it again.
- GDI debug text/logs show current objective, current flags, event count, last event, completion state, and whether the current action changed remembered state.

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

## Historical v0.5 Direction From Slice Needs

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
