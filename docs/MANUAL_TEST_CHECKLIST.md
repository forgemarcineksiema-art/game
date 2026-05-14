# Manual Test Checklist

Last updated: 2026-05-14

Use this after automated validation when a change affects player feel, camera, collision, renderer, or interactions.

## Setup

Build first:

```powershell
scripts/configure.ps1
scripts/build.ps1
```

Run GDI for readable debug text:

```powershell
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi
```

Optional bounded DX11 run:

```powershell
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 120
```

## Player

- [ ] `W/A/S/D` moves the player camera-relative.
- [ ] Diagonal movement does not feel faster than straight movement.
- [ ] Holding `Shift` increases movement speed.
- [ ] Pressing `Space` jumps once.
- [ ] Holding `Space` does not repeatedly trigger jump after landing unless the key is released and pressed again.
- [ ] Releasing movement returns horizontal speed to zero.

## Camera

- [ ] Mouse movement over the window orbits the camera.
- [ ] Arrow keys orbit the camera as a fallback.
- [ ] Pitch clamps and does not flip the camera.
- [ ] Normal walking does not create obvious camera jitter.
- [ ] The player remains visible during ordinary movement.

## Collision

- [ ] The player cannot walk through the wide wall.
- [ ] The player cannot pass through the narrow passage posts.
- [ ] The player can slide or be pushed out of a corner without getting permanently stuck.
- [ ] The low step-like blocker remains a blocker in v0.4.1.
- [ ] Grounded/air debug text changes as expected during jump.

## Interactions

- [ ] At spawn, the manifest focus prompt appears: `Press E: Collect Ferry Manifest`.
- [ ] Pressing `E` once on the pickup logs/records the pickup result.
- [ ] Holding `E` does not repeatedly trigger the pickup.
- [ ] After pickup consumption, the pickup no longer regains focus.
- [ ] Moving near and facing the toggle marker shows the toggle prompt.
- [ ] Pressing `E` on the toggle changes its state/color/debug result.
- [ ] Moving near and facing the info marker shows the info prompt.
- [ ] Pressing `E` on the info marker logs/records the info message.
- [ ] If an object does not focus, moving closer and facing it resolves the issue.

## World State

- [ ] Initial debug text shows `powerRestored=false`, `manifestCollected=false`, `serviceRouteUsed=false`, `maintenanceBoxInspected=false`, and `routeOpened=false`.
- [ ] Collecting the Ferry Manifest changes `manifestCollected=true` and increments `eventCount`.
- [ ] Toggling the Wall Button changes `routeOpened` and changes the debug gate marker color.
- [ ] Completing the service barrier traversal changes `serviceRouteUsed=true`.
- [ ] Inspecting the Maintenance Box changes `maintenanceBoxInspected=true` and `powerRestored=true`.
- [ ] After power is restored, the Maintenance Box marker changes to the restored-power debug color.
- [ ] Repeating a completed one-shot action does not add another remembered event.

## Traversal

- [ ] Move to the service barrier traversal start marker near the low blocker.
- [ ] Facing the traversal path shows the traversal prompt.
- [ ] GDI debug text shows `travStart=current` when traversal begins.
- [ ] Pressing `Space` while focused starts the vault/mantle motion.
- [ ] The player starts from the current standing position inside the focus radius and does not visibly snap to the marker.
- [ ] The player reaches the end marker on the far side of the blocker.
- [ ] On landing, debug text returns to `traversal=normal`, `grounded`, and clean velocity/speed.
- [ ] Holding `Space` does not retrigger traversal while traversal is already active.
- [ ] Pressing `Space` away from traversal markers still performs normal jump.
- [ ] Camera follow remains stable during and immediately after traversal.
- [ ] The maintenance-box-like marker after the traversal route can be reached.
- [ ] Pressing `E` on the maintenance-box-like marker still works after traversal.

## Exit

- [ ] `Esc` requests quit.
- [ ] Closing the window exits cleanly.
- [ ] Logs show startup, runtime renderer, sandbox attach/detach, renderer shutdown, and engine shutdown.
