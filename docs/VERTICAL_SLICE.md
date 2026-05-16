# Vertical Slice

Last updated: 2026-05-16

## Slice Name

`The Ferry Office`

## Purpose

Define the first playable micro-scenario for the project direction locked in `docs/GAME_DIRECTION.md`.

This document defines the micro-slice target and prototype state. Early versions deliberately deferred vehicles; v0.16 adds one narrow service-vehicle job beat using the existing vehicle prototype without adding traffic, economy, NPC AI, full missions, inventory, save/load, final art, or asset pipelines.

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
- Pressing the Wall Button records `routeOpened=true`.
- Reaching the Exit Summary Marker records `exitReached=true` only after the required loop is ready.
- `routeOpened=true` disables the `service-gate` blocking collider. v0.7.1 keeps this as a latch-open route to avoid trapping the player.
- GDI debug text/logs show current objective, current flags, event count, last event, completion state, and whether the current action changed remembered state.

## v0.16 Driver/Fixer Job Extension

The first driver/fixer job is `Ferry Office Service Call`.

It extends the micro-slice with one vehicle requirement while keeping the flow explicit and debug-only:

1. Collect/check the Ferry Manifest to start the service call.
2. Use the Service Barrier Vault route.
3. Inspect the Maintenance Box and restore power.
4. Open the service gate.
5. Enter the service-yard vehicle.
6. Drive to the dock-road service checkpoint.
7. Exit near the Service Run Marker and confirm the service run.
8. Show `ferryOfficeJobComplete=true` in world/debug state.

This is still not a mission scripting system. It is one authored job helper used to prove that movement, interaction, traversal, vehicle driving, scene data, and remembered state can combine into a playable task.

## v0.17 Presentation Mode

v0.17 keeps the Ferry Office Service Call as the only playable job and adds a first player-facing presentation mode:

- `playtest` mode shows current objective, focused prompt, job status, vehicle/checkpoint hints, and completion state.
- `debug` mode keeps full raw telemetry for Codex/development validation.
- `minimal` mode keeps only objective, prompt, and job status.
- `F1` toggles the debug overlay during play.

The slice still uses debug text and markers, not a real UI framework. Job #2 remains intentionally deferred until the existing first job has stronger presentation, road identity, and prop style.

## v0.50 Dock Road Relay Follow-up

The first compact follow-up beat is `Dock Road Relay`.

After the Ferry Office Service Call completes at the service-run marker, the player can reset a small relay at the dock-road endpoint. This records `dockRoadRelayReset=true` and flips a small endpoint status cue from warning orange to reset green. The player can then log the reset at the nearby service board, recording `dockRoadRelayLogged=true`. This gives the service-run area a remembered visible/sign-off consequence without adding a mission framework, NPC AI, economy, save/load, new vehicle physics, or broader map expansion.

## v0.60 Harbor Parts Return Micro-slice

The first compact second-job style beat is `Harbor Parts Return`.

After the Dock Road relay has been reset, logged, and tagged clear, the player can collect a Harbor Parts Crate by the dock-road maintenance cabinet, recording `harborPartsPickedUp=true`. The player can then return to the Ferry Office and deliver it to the parts shelf, recording `harborPartsDelivered=true`. This makes the service-yard and office form a small return loop with visible remembered state while still avoiding a generic mission framework, inventory, save/load, NPC dispatcher, economy, or carried-object simulation.

## Systems Explicitly Deferred

- Additional vehicles beyond the one v0.16 service-yard vehicle beat.
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
