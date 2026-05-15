# Manual Test Checklist

Last updated: 2026-05-15

Use this after automated validation when a change affects player feel, camera, collision, renderer, or interactions.

## Setup

Build first:

```powershell
scripts/configure.ps1
scripts/build.ps1
```

Run GDI for readable debug text:

```powershell
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --ui-mode playtest
```

Run full debug telemetry:

```powershell
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --ui-mode debug
```

Run with visible desktop cursor for troubleshooting:

```powershell
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --free-cursor
```

Optional bounded DX11 run:

```powershell
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer dx11 --frames 120
```

Optional physics dependency spike:

```powershell
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
```

Optional scene authoring checks:

```powershell
python tools/scene_report.py
python tools/validate_scene.py
python tools/validate_assets.py
python tools/scale_audit.py
python tools/mesh_report.py
```

## Presentation Mode

- [ ] Default windowed play or `--ui-mode playtest` shows objective, focused prompt, job status, vehicle/checkpoint hints, completion state, and `F1` help without raw telemetry dominating the screen.
- [ ] `--ui-mode debug` shows full development telemetry: player/camera, traversal, vehicle, world state, scene, physics, and job details.
- [ ] `--ui-mode minimal` shows only objective, prompt, and job status.
- [ ] Pressing `F1` in playtest mode switches to the debug overlay.
- [ ] Pressing `F1` again returns to the previous playtest/minimal mode.
- [ ] Playtest mode keeps important route, traversal, interactable, vehicle, checkpoint, and completion markers visible.
- [ ] Playtest mode hides or reduces workbench clutter such as full collider boxes, world bounds, camera target, physics debug lines, and the full grid.
- [ ] Objective/prompt text does not flicker, overlap, or become unreadable during the Ferry Office Service Call.
- [ ] No Job #2 or extra objective chain appears.

## Player

- [ ] `W/A/S/D` moves the player camera-relative.
- [ ] Diagonal movement does not feel faster than straight movement.
- [ ] Holding `Shift` increases movement speed.
- [ ] Pressing `Space` jumps once.
- [ ] Holding `Space` does not repeatedly trigger jump after landing unless the key is released and pressed again.
- [ ] Releasing movement returns horizontal speed to zero.

## Camera

- [ ] Windowed play hides/confines the cursor by default after the game window is focused.
- [ ] Mouse/touchpad movement orbits the camera without the desktop cursor distracting from play.
- [ ] Running with `--free-cursor` keeps the desktop cursor visible for debugging.
- [ ] Arrow keys orbit the camera as a fallback.
- [ ] Pitch clamps and does not flip the camera.
- [ ] Normal walking does not create obvious camera jitter.
- [ ] The player remains visible during ordinary movement.

## Collision

- [ ] The player cannot walk through the ferry office walls.
- [ ] The player cannot pass through the closed service gate before `routeOpened=true`.
- [ ] Opening the service gate disables the gate blocker and lets the player pass the route.
- [ ] Pressing the Wall Button again does not close the service gate on the player.
- [ ] The player can slide or be pushed out of a corner without getting permanently stuck.
- [ ] The service barrier remains a blocker except for the intended Service Barrier Vault traversal.
- [ ] Grounded/air debug text changes as expected during jump.

## Physics Foundation

- [ ] Normal `scripts/verify.ps1` still passes without requiring Jolt.
- [ ] `windows-vs2022-debug-jolt` configures and builds when intentionally testing physics dependency work.
- [ ] Jolt tests pass through the engine-owned `IPhysicsWorld` API.
- [ ] `src/game` has no direct `JPH::*` or Jolt header usage.
- [ ] Ferry Office player/camera/collision behavior still feels unchanged after physics foundation work.

## Scene Authoring Tools

- [ ] `python tools/validate_scene.py` reports `Scene validation passed.`
- [ ] `python tools/validate_assets.py` reports `Asset validation passed.`
- [ ] `python tools/scene_report.py` reports scene id `ferry-office`.
- [ ] Scene report lists 9 colliders, 6 interactables, 1 traversal affordance, 1 vehicle, 6 route markers, and 5 objective markers.
- [ ] `python tools/scale_audit.py` reports no suspicious scale issues, or any issue is documented.
- [ ] `python tools/mesh_report.py` reports 7 mesh assets, 17 Ferry Office/service-yard/dock-road mesh instances, and 7 referenced `.gltf` model files.
- [ ] `data/scenes/ferry_office.scene.json` contains the service gate, manifest, maintenance box, wall button, exit marker, service vault, player start, and service-yard vehicle ids.
- [ ] `data/scenes/ferry_office.scene.json` contains `service-run-confirm-marker`, `service-run-checkpoint-marker`, and `route-dock-road-to-service-confirm`.
- [ ] `data/scenes/ferry_office.scene.json` contains `meshAssets` and `meshInstances` for the current static mesh spike.
- [ ] Runtime debug text reports `scene=ferry-office loaded=yes`.
- [ ] Any manual movement of scene layout objects starts in scene JSON; only behavior-specific C++ fallback/mapping is changed when necessary.

## Static Mesh Rendering

- [ ] GDI run still shows debug text and all old gameplay markers.
- [ ] GDI run shows flat mesh placeholders for the ferry office roof cap, facade/sign cue, service gate, maintenance box, dock bollards, service-yard crate, vehicle body, and vehicle cabin.
- [ ] GDI run shows the v0.18 original prop-kit silhouettes: service road sign, road-edge posts, service-yard barrier cue, and maintenance utility box.
- [ ] GDI run shows the v0.20 fallback-generated ferry notice board and the v0.20.1 Blender-exported notice-board prop near the office approach.
- [ ] DX11 bounded run exits cleanly with mesh submissions enabled.
- [ ] Mesh placeholders do not hide route, interaction, traversal, collision, or vehicle debug markers.
- [ ] `assets/models/unit_box.gltf`, the v0.18 prop-kit meshes, `assets/models/ferry_notice_board.gltf`, and `assets/models/blender_ferry_notice_board.gltf` remain small original placeholder assets with scene-data license/provenance.
- [ ] No `.glb`, external-buffer `.gltf`, unreferenced `.gltf`, or undocumented mesh asset appears in `assets/models`.

## Vehicle Spike

- [ ] The service-yard vehicle is visible near the dock/service-yard driving pad.
- [ ] Moving near the vehicle with no Ferry Office interaction focused shows the enter prompt.
- [ ] Pressing `E` enters the vehicle once.
- [ ] Holding `E` does not rapidly enter/exit on repeated frames.
- [ ] While driving, `W` accelerates forward.
- [ ] While driving, `S` brakes when moving forward and reverses after stopping.
- [ ] Releasing `W/S` lets drag settle the vehicle instead of coasting forever.
- [ ] `A/D` steer the vehicle and the heading marker turns with it.
- [ ] At low speed, the vehicle can turn around within the service-yard bounds without feeling twitchy.
- [ ] At low speed, steering still visibly rotates the vehicle enough to make the yard turn-around readable.
- [ ] Reverse steering is understandable when backing up.
- [ ] The vehicle camera follows slightly ahead of the vehicle and gently recenters toward vehicle yaw without obvious jitter.
- [ ] The service-yard bounds prevent the vehicle from drifting far outside the test pad.
- [ ] The service-yard pad/rails/back-stop visually communicate the vehicle start area.
- [ ] The vehicle can drive from the service-yard pad through the east opening onto the short dock road segment.
- [ ] The dock road segment has a readable road pad, shore/water edge cue, rail/curb edges, and turn-around/end marker.
- [ ] The expanded vehicle bounds keep the vehicle inside the service-yard/dock-road route instead of drifting into nowhere.
- [ ] Pressing `E` exits only when the side exit marker is clear.
- [ ] After exit, on-foot movement, camera, traversal, and Ferry Office interactions still work.
- [ ] Debug text reports vehicle occupancy, speed, throttle/brake/steer, camera mode, and physics backend status.
- [ ] Playtest text while driving reports speed, checkpoint status, and whether the exit marker is clear or blocked.
- [ ] The vehicle does not steal `E` from the Ferry Manifest, Maintenance Box, Wall Button, Ferry Office Notice, or Exit Summary Marker when those are focused.

## First Driver/Fixer Job

- [ ] Initial playtest text says to check/collect the Ferry Manifest.
- [ ] Debug mode still shows `jobObjective` and `jobPhase=collectManifest`.
- [ ] Collecting the Ferry Manifest records `manifestCollected=true` and starts the Ferry Office Service Call.
- [ ] Using the Service Barrier Vault records `serviceRouteUsed=true`.
- [ ] Inspecting the Maintenance Box records `maintenanceBoxInspected=true` and `powerRestored=true`.
- [ ] Opening the service gate records `routeOpened=true`.
- [ ] Entering the service-yard vehicle records `serviceVehicleUsed=true`.
- [ ] Driving the vehicle to the dock-road checkpoint records `dockRoadReached=true`.
- [ ] The Service Run Marker is visible near the dock-road end/exit side.
- [ ] Pressing `E` at the Service Run Marker before prerequisites does not complete the job.
- [ ] The Service Run Marker prompt does not falsely claim the run is confirmed before prerequisites are complete.
- [ ] Pressing `E` at the Service Run Marker after prerequisites records `serviceRunConfirmed=true` and `ferryOfficeJobComplete=true`.
- [ ] Repeating the Service Run Marker after completion does not duplicate world events.
- [ ] Ferry Office slice completion through the Exit Summary Marker still works separately from the driver/fixer job.

## Interactions

- [ ] At spawn, the manifest focus prompt appears: `Press E: Collect Ferry Manifest`.
- [ ] Pressing `E` once on the pickup logs/records the pickup result.
- [ ] Holding `E` does not repeatedly trigger the pickup.
- [ ] After pickup consumption, the pickup no longer regains focus.
- [ ] Moving near and facing the Wall Button shows the service-gate prompt.
- [ ] Pressing `E` on the Wall Button opens the service gate and changes its marker/debug result.
- [ ] Moving near the Service Barrier Vault shows one clear Space prompt, not duplicated `Press Space` wording.
- [ ] Moving near and facing the Ferry Office Notice shows the info prompt.
- [ ] Pressing `E` on the Ferry Office Notice logs/records the info message.
- [ ] The Exit Summary Marker does not complete the slice before required remembered flags are set.
- [ ] If an object does not focus, moving closer and facing it resolves the issue.

## World State

- [ ] Initial debug text shows `powerRestored=false`, `manifestCollected=false`, `serviceRouteUsed=false`, `maintenanceBoxInspected=false`, `routeOpened=false`, and `exitReached=false`.
- [ ] Collecting the Ferry Manifest changes `manifestCollected=true` and increments `eventCount`.
- [ ] Pressing the Wall Button changes `routeOpened=true`, changes the debug gate marker color, and changes the service-gate blocking state.
- [ ] Repeating the Wall Button interaction leaves `routeOpened=true` and does not add another world event.
- [ ] Completing the service barrier traversal changes `serviceRouteUsed=true`.
- [ ] Inspecting the Maintenance Box changes `maintenanceBoxInspected=true` and `powerRestored=true`.
- [ ] After power is restored, the Maintenance Box marker changes to the restored-power debug color.
- [ ] Reaching the Exit Summary Marker after the required flags changes `exitReached=true`.
- [ ] Debug text shows `sliceComplete=yes` only after the exit marker records completion.
- [ ] Repeating a completed one-shot action does not add another remembered event.

## Ferry Office Micro-Slice

- [ ] Start near the dock-side marker and see an objective mentioning the Ferry Manifest.
- [ ] The scene reads as a dock/service-yard/ferry-office layout instead of only a wireframe test.
- [ ] Distinct colors separate the dock/floor, ferry office volume, service gate, traversal path, interactables, player, and exit marker.
- [ ] Solid placeholder geometry does not hide the important wire/debug outlines.
- [ ] The committed visual reference at `docs/images/v0.18-gdi-screenshot.png` roughly matches the current GDI presentation.
- [ ] The ferry office front/sign cue reads as a deliberate public/service building marker, not only a wall of debug colliders.
- [ ] The service-yard vehicle reads as a vehicle proxy with a separate cabin silhouette.
- [ ] The dock road extension reads as a short island service road connected to the service-yard, not random extra boxes.
- [ ] The road-end marker and shore/water edge cue help orient the driving route.
- [ ] The service road sign, road-edge posts, service-yard barrier cue, and utility box make the area read more like coastal service infrastructure than raw debug boxes.
- [ ] A light route polyline guides the eye from manifest to service vault, maintenance box, wall button, and exit marker.
- [ ] The objective text names the next spatial target instead of only the next abstract action.
- [ ] Collect the Ferry Manifest near the office approach.
- [ ] Move to the Service Barrier Vault and use `Space` to cross it.
- [ ] The Service Barrier Vault prompt appears from the player-accessible side of the barrier.
- [ ] Inspect the Maintenance Box after traversal and confirm power is restored.
- [ ] Return to the Wall Button and open the service gate.
- [ ] Pass through the opened service-gate route.
- [ ] Reach the Exit Summary Marker and press `E`.
- [ ] Debug text shows the current objective advancing through the loop.
- [ ] Debug text is split into readable sections instead of one very long line.
- [ ] Completion summary reports `complete=true` after all required flags are set.

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

- [ ] `Esc` requests quit and restores the cursor.
- [ ] Closing the window exits cleanly.
- [ ] Logs show startup, runtime renderer, sandbox attach/detach, renderer shutdown, and engine shutdown.
