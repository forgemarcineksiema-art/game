# Runbook

Commands assume PowerShell from the repository root.

## Doctor

```powershell
scripts/doctor.ps1
```

Checks CMake, Python, git, expected folders, important docs, and available C++ build tools. Missing compilers are warnings unless `-Strict` is used, because Visual Studio generators can work even when `cl.exe` is not on PATH.

## Configure

```powershell
scripts/configure.ps1
```

Auto tries these presets in order:

1. `windows-vs2022-debug`
2. `windows-vs2026-debug`
3. `ninja-debug`

Specific preset:

```powershell
scripts/configure.ps1 -Preset windows-vs2022-debug
```

Opt-in Jolt physics spike preset:

```powershell
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
ctest --preset windows-vs2022-debug-jolt --output-on-failure
```

The Jolt preset is intentionally not part of the automatic `scripts/configure.ps1` candidate list. Use it when validating physics dependency work, not for the everyday path.

## Build

```powershell
scripts/build.ps1
```

Specific preset:

```powershell
scripts/build.ps1 -Preset windows-vs2022-debug
```

## Run

Playable build shortcut:

```powershell
scripts/play.ps1
```

This launches the current Ferry Office Service Call with the GDI renderer, `playtest` UI, and the default Ferry Office scene:

```text
--renderer gdi --ui-mode playtest --scene data\scenes\ferry_office.scene.json
```

Useful playable variants:

```powershell
scripts/play.ps1 -DebugUi
scripts/play.ps1 -MinimalUi
scripts/play.ps1 -Dx11 -Frames 360
scripts/play.ps1 -FreeCursor
scripts/play.ps1 -VehicleRuntime jolt -ExecutablePath build\windows-vs2022-debug-jolt\Debug\EngineApp.exe
scripts/play.ps1 -Frames 6 -CaptureFrame build\captures\v0.31-gdi.bmp
scripts/play.ps1 -Dx11 -Frames 6 -CaptureFrame build\captures\v0.31-dx11.bmp
scripts/play.ps1 -Args @("--frames", "360")
scripts/play.ps1 -DryRun
```

`scripts/play.ps1` prints the exact `EngineApp.exe` command it runs. If the Debug executable is missing, run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\configure.ps1
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
```

Use `scripts/run.ps1` when you want the lower-level "last configured preset" launcher instead of the current playable-build preset.

Windowed runtime, using the last configured preset:

```powershell
scripts/run.ps1
```

Prototype controls:

- `W/A/S/D`: camera-relative movement
- `Shift`: sprint
- `Space`: jump, or vault/mantle when a traversal affordance is focused
- `E`: interact with the focused debug object
- `F1`: toggle between the player-facing presentation overlay and full debug overlay
- Mouse/touchpad movement: orbit camera. Windowed play captures and hides the cursor by default.
- Arrow keys: camera fallback controls
- `Esc`: quit and restore the cursor

Vehicle spike controls:

- Approach the service-yard vehicle while no Ferry Office interactable is focused.
- `E`: enter the vehicle when the vehicle prompt is visible.
- In vehicle, `W`: throttle / accelerate.
- In vehicle, `S`: brake while moving forward, then reverse when stopped.
- In vehicle, `A/D`: steer.
- In vehicle, `E`: exit when the side exit marker is clear.
- On-foot controls resume after exit.

Live vehicle runtime switch:

```powershell
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
scripts/play.ps1 -VehicleRuntime jolt -ExecutablePath build\windows-vs2022-debug-jolt\Debug\EngineApp.exe -DebugUi
```

`--vehicle-runtime jolt` is an explicit manual-playtest switch. The default remains deterministic. In debug text, check `vehicleRuntime=jolt-live` before judging the switched path. If the normal dependency-free executable is launched with the Jolt runtime flag, it should report the selected runtime backend as unavailable and keep the deterministic fallback rather than pretending the opt-in backend ran.

First driver/fixer job loop:

- Collect/check the Ferry Manifest to start the Ferry Office Service Call.
- Use the Service Barrier Vault and inspect the Maintenance Box to restore power.
- Open the service gate with the Wall Button.
- Enter the service-yard vehicle and drive to the dock-road service checkpoint near the road-end marker.
- Exit when the side marker is clear, then press `E` at the Service Run Marker to confirm the run.
- GDI debug text shows `jobObjective`, `jobPhase`, `serviceVehicleUsed`, `dockRoadReached`, `serviceRunConfirmed`, and `jobComplete`.

Presentation / debug UI mode:

```powershell
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --ui-mode playtest
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --ui-mode debug
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --ui-mode minimal
```

Normal windowed play defaults to `playtest`, which keeps objective, prompt, job state, vehicle/checkpoint hints, and completion status visible without the raw telemetry wall. Use `--ui-mode debug` or `--debug-ui` for full development telemetry. Use `--ui-mode minimal` or `--playtest-ui --ui-mode minimal` only when you want the smallest objective/prompt/status readout. `F1` toggles from playtest/minimal to debug and back during a windowed run.

GDI remains the simplest renderer for overlay debugging. DX11 now mirrors debug/playtest text through a tiny renderer-owned bitmap text path before capture and `Present`, so `scripts/play.ps1 -Dx11` is usable for bounded playtest checks too; it is still not a production HUD/text renderer.

Renderer-owned frame capture:

```powershell
scripts/play.ps1 -Frames 6 -CaptureFrame build\captures\gdi-frame.bmp
scripts/play.ps1 -Dx11 -Frames 6 -CaptureFrame build\captures\dx11-frame.bmp
scripts/play.ps1 -Frames 6 -CaptureDir build\captures
python tools\capture_visual_smoke.py
python tools\capture_visual_smoke.py --report-json build\captures\v0.31-visual-report.json
python tools\capture_visual_smoke.py --scenario relay-to-service-log --report-json build\captures\capture_visual_smoke_midchain_report.json
```

`--capture-frame <path>` writes one 32-bit BMP to the exact path. `--capture-dir <path>` writes one generated `capture-<renderer>.bmp` file into the directory. Capture occurs after a stable frame, or on the last bounded frame if the run is shorter than the stable-frame threshold. GDI captures the back buffer including the GDI debug text. DX11 captures the swap-chain back buffer before `Present`, including the v0.31 renderer-owned bitmap debug text and existing geometry/depth presentation.

`tools\capture_visual_smoke.py` is the preferred bounded visual evidence path. It launches GDI and DX11, checks expected dimensions, BMP validity, non-flat color diversity, luminance range, broad Ferry Office start-view color signals for dark background, warm markers, green/teal markers, cool geometry, and a conservative top-left bright text signal. It also compares renderer capture dimensions and writes `build\captures\capture_visual_smoke_report.json` by default. `--scenario relay-to-service-log` forwards `--qa-capture-state relay-to-service-log` to the app, preloads a narrow QA-only Ferry Office mid-chain state, and captures active playtest route guidance near the Relay Service Log. This is not save/load, a manual-play shortcut, OCR, semantic object detection, or a pixel-perfect golden-image test.

Automated Ferry Office playthrough QA:

```powershell
python tools\playthrough_qa.py
build\windows-vs2022-debug\Debug\EngineApp.exe --qa-playthrough ferry-office-service-call --qa-playthrough-report build\playthroughs\ferry-office-service-call-report.json
```

`--qa-playthrough ferry-office-service-call` is a QA-only path, off by default. It does not open a window or play through keyboard/mouse input. It loads the authored Ferry Office scene, runs the existing scene/job systems through the first Service Call phases, asserts `manifestCollected`, `serviceRouteUsed`, `maintenanceBoxInspected`, `powerRestored`, `routeOpened`, `serviceVehicleUsed`, `dockRoadReached`, `serviceRunConfirmed`, and `ferryOfficeJobComplete`, and writes a JSON report under `build\playthroughs` by default. This is deterministic behavioral coverage, not a replacement for human feel review of movement, driving, camera, readability, or interaction ergonomics.

Opt-in Ferry Office physics parity QA:

```powershell
cmake --preset windows-vs2022-debug-jolt
cmake --build --preset windows-vs2022-debug-jolt
python tools\physics_parity_qa.py
python tools\character_contact_qa.py
build\windows-vs2022-debug-jolt\Debug\EngineApp.exe --qa-physics-parity ferry-office-collision --qa-physics-report build\physics\ferry-office-collision-parity-report.json
build\windows-vs2022-debug-jolt\Debug\EngineApp.exe --qa-physics-parity ferry-office-character-contact --qa-physics-report build\physics\ferry-office-character-contact-report.json
```

`--qa-physics-parity ferry-office-collision` is a QA-only path, off by default. It requires the opt-in Jolt-enabled executable for the real backend check. It loads the authored Ferry Office scene, mirrors the 9 static scene colliders plus a validation floor body into the engine physics API, compares floor, raycast, and player-overlap-style probes against `PrototypeWorld`, and writes `build\physics\ferry-office-collision-parity-report.json` by default. This proves static scene-query parity only; it does not migrate live player collision, traversal, service-gate toggling, or vehicle behavior.

`--qa-physics-parity ferry-office-character-contact` is also QA-only and off by default. It compares the current player collision proxy against opt-in physics overlap/contact candidates for dock-floor grounding, office wall blocking, service gate blocking, service barrier blocking, clear lane movement, corner pushout, and an opened-gate clear case. It writes `build\physics\ferry-office-character-contact-report.json` by default. This is a probe for future migration, not a live `PlayerController` change or a character-controller implementation.

Headless smoke mode:

```powershell
scripts/run.ps1 -Args @("--smoke-test", "--frames", "3")
scripts/play.ps1 -Args @("--renderer", "null", "--headless", "--smoke-test", "--frames", "3")
```

Renderer selection:

```powershell
scripts/run.ps1 -Args @("--renderer", "dx11", "--frames", "120")
scripts/run.ps1 -Args @("--renderer", "gdi", "--frames", "120")
scripts/run.ps1 -Args @("--renderer", "null", "--headless", "--frames", "120")
```

Runtime scene selection:

```powershell
scripts/run.ps1 -Args @("--renderer", "gdi", "--scene", "data/scenes/ferry_office.scene.json")
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi --scene data\scenes\ferry_office.scene.json --frames 300
```

If `--scene` is omitted, the app loads `data/scenes/ferry_office.scene.json`. If loading fails, the runtime logs a warning and falls back to the built-in Ferry Office layout so smoke/debug paths remain usable.

Cursor mode:

```powershell
scripts/run.ps1 -Args @("--renderer", "gdi", "--capture-cursor")
scripts/run.ps1 -Args @("--renderer", "gdi", "--free-cursor")
scripts/run.ps1 -Args @("--renderer", "gdi", "--show-cursor")
```

`--capture-cursor` is the default for windowed play. It hides and confines the cursor to the game window so mouse/touchpad camera movement is less distracting. `--free-cursor` and `--show-cursor` keep the desktop cursor visible for debugging or remote-session troubleshooting. Arrow keys continue to orbit the camera in either mode.

## Test

After configure/build:

```powershell
ctest --preset windows-vs2022-debug
```

The configured preset can be replaced with the preset recorded in `build/.last_preset`.

Jolt backend test:

```powershell
ctest --preset windows-vs2022-debug-jolt --output-on-failure
```

Ferry Office physics parity smoke:

```powershell
python tools\physics_parity_qa.py
python tools\character_contact_qa.py
```

## Scene Tools

Scene summary for Codex/world-authoring work:

```powershell
python tools/scene_report.py
```

Scene validation:

```powershell
python tools/validate_scene.py
```

Static mesh asset workflow validation:

```powershell
python tools/validate_assets.py
```

Scale sanity audit:

```powershell
python tools/scale_audit.py
```

Mesh asset/reference report:

```powershell
python tools/mesh_report.py
```

Blender workflow check and first procedural prop export:

```powershell
python tools/check_blender.py --require
blender --version
blender --background --python tools\blender\create_tidebreak_notice_board.py
python tools/validate_assets.py
python tools/mesh_report.py
```

Blender is not required for normal gameplay runs, but v0.20.1 proves it works locally from PATH with Blender 5.1.1.

Use an explicit scene path when experimenting with a copy:

```powershell
python tools/validate_scene.py data/scenes/ferry_office.scene.json
python tools/validate_assets.py data/scenes/ferry_office.scene.json
python tools/scene_report.py data/scenes/ferry_office.scene.json
python tools/scale_audit.py data/scenes/ferry_office.scene.json
python tools/mesh_report.py data/scenes/ferry_office.scene.json
```

`scripts/verify.ps1` runs `python tools/validate_scene.py`, `python tools/validate_assets.py`, and `python tools/mesh_report.py` after CTest. Scene and mesh asset/reference validation are part of the normal path.

## Static Mesh Rendering

v0.12 adds a small static mesh/glTF spike. Current supported project-original assets are:

```text
assets/models/unit_box.gltf
assets/models/service_road_sign.gltf
assets/models/road_edge_post.gltf
assets/models/service_barrier.gltf
assets/models/utility_box.gltf
assets/models/ferry_notice_board.gltf
assets/models/blender_ferry_notice_board.gltf
```

The supported loader subset is intentionally tiny: `.gltf`, one embedded base64 buffer, `POSITION` float `VEC3`, indexed triangle list, no materials/textures/animation. v0.18 can load multiple scene-authored mesh asset ids, v0.19 validates the asset workflow around those files, and v0.20 adds an optional Blender check plus one clearly labeled fallback-generated notice-board prop. This is still not an asset registry or production mesh pipeline. See `docs/MESH_RENDERING.md`, `docs/ASSET_PIPELINE_DECISION.md`, and `docs/BLENDER_WORKFLOW.md` before changing mesh loading or renderer behavior.

## Blender / Static Prop Workflow

Blender is optional for normal validation until the project explicitly makes it a required DCC dependency.

Check local Blender availability:

```powershell
python tools/check_blender.py
```

Require Blender in a local setup check:

```powershell
python tools/check_blender.py --require
```

Generate the v0.20 fallback prop again:

```powershell
python tools/create_simple_prop_gltf.py --output assets/models/ferry_notice_board.gltf --overwrite
```

The fallback helper is not Blender export. Use it only for simple original placeholder geometry while Blender/export compatibility is blocked.

## Verify

```powershell
scripts/verify.ps1
```

Runs doctor, configure, build, CTest, and a bounded smoke run when an executable exists.

## Clean

```powershell
scripts/clean.ps1
```

Removes the `build` directory.

## Status Report

```powershell
python tools/status_report.py
```

Prints git status, important file presence, build output presence, and the opening section of `docs/STATUS.md`.

## Manual Windowed Checklist

After changes that affect runtime feel, run the checklist in:

```powershell
docs/MANUAL_TEST_CHECKLIST.md
```

Use GDI first when debug text matters:

```powershell
build\windows-vs2022-debug\Debug\EngineApp.exe --renderer gdi
```

## Troubleshooting

- If CMake cannot find Visual Studio, install Visual Studio Build Tools with the C++ desktop workload, then rerun `scripts/configure.ps1`.
- If `ninja-debug` is selected, make sure both Ninja and a C++ compiler are in PATH.
- If the windowed app fails, run the headless path: `scripts/run.ps1 -Args @("--smoke-test", "--frames", "3")`.
- If DirectX 11 fails, try `scripts/run.ps1 -Args @("--renderer", "gdi", "--frames", "120")`.
- If all windowed rendering fails, keep `--renderer null --headless` working while the renderer issue is fixed.
- If captured mouse-look feels risky in a VM or remote session, run with `--free-cursor` and use arrow keys for camera orbit.
- If the Jolt preset fails while normal validation passes, keep using the default preset and inspect `docs/PHYSICS_DECISION.md`. The Jolt path is opt-in until the physics backend is promoted by a later goal.
- If `tools\physics_parity_qa.py` or `tools\character_contact_qa.py` fails because `build\windows-vs2022-debug-jolt\Debug\EngineApp.exe` is missing, configure/build the `windows-vs2022-debug-jolt` preset first. A default executable should report the opt-in physics backend as unavailable rather than pretending parity/contact QA ran.
- If the runtime logs `Runtime scene load failed`, check the `--scene` path first, then run `python tools/validate_scene.py <path>`.
- If scene validation fails, inspect `data/scenes/ferry_office.scene.json` and run `python tools/scene_report.py` to see the current object counts and required ids.
- If mesh references fail, run `python tools/validate_assets.py` and `python tools/mesh_report.py`. Verify referenced files exist under `assets/models`, every committed `.gltf` is scene-referenced, `.glb`/external buffers are not used yet, and license/provenance exists in scene data.
- v0.3+ collision is debug-only static AABB collision. If a collider layout feels odd, inspect `src/game/PrototypeWorld.cpp` and rerun `scripts/verify.ps1` after edits.
- v0.4 interaction focus is debug-only point/radius selection with a facing preference. If an object does not focus, move closer and face the marker, then press `E`.
- v0.5 traversal uses `Space` only when a traversal affordance is focused. If traversal does not trigger, move near the traversal start marker and face the path direction. If no traversal is focused, `Space` remains normal jump.
- v0.5.1 traversal starts from the player's current position inside the focus radius and lands through the world collision resolver. In GDI debug text, check `travStart=current`, `travProgress`, and `travLanded`.
- v0.14 vehicle movement is still a deterministic placeholder controller, not Jolt VehicleConstraint. If the vehicle does not enter, make sure no Ferry Office interaction prompt is currently focused. If exit is blocked, move/turn inside the service yard or dock road until the side exit marker is clear. Use the service-yard pad, east opening, short dock road, rail/curb edges, and turn-around/end marker as the intended compact road-test area.
