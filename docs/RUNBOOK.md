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

Normal windowed play defaults to `playtest`, which keeps objective, prompt, job state, vehicle/checkpoint hints, and completion status visible without the raw telemetry wall. Use `--ui-mode debug` or `--debug-ui` for full development telemetry. Use `--ui-mode minimal` or `--playtest-ui --ui-mode minimal` only when you want the smallest objective/prompt/status readout. `F1` toggles from playtest/minimal to debug and back during a GDI run.

GDI is the best renderer for overlay review because it supports debug text. DX11 bounded runs remain useful for renderer validation, but DX11 text is still intentionally absent.

Headless smoke mode:

```powershell
scripts/run.ps1 -Args @("--smoke-test", "--frames", "3")
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

## Scene Tools

Scene summary for Codex/world-authoring work:

```powershell
python tools/scene_report.py
```

Scene validation:

```powershell
python tools/validate_scene.py
```

Scale sanity audit:

```powershell
python tools/scale_audit.py
```

Mesh asset/reference report:

```powershell
python tools/mesh_report.py
```

Use an explicit scene path when experimenting with a copy:

```powershell
python tools/validate_scene.py data/scenes/ferry_office.scene.json
python tools/scene_report.py data/scenes/ferry_office.scene.json
python tools/scale_audit.py data/scenes/ferry_office.scene.json
python tools/mesh_report.py data/scenes/ferry_office.scene.json
```

`scripts/verify.ps1` runs `python tools/validate_scene.py` and `python tools/mesh_report.py` after CTest. Scene and mesh reference validation are part of the normal path.

## Static Mesh Rendering

v0.12 adds a small static mesh/glTF spike. Current supported project-original assets are:

```text
assets/models/unit_box.gltf
assets/models/service_road_sign.gltf
assets/models/road_edge_post.gltf
assets/models/service_barrier.gltf
assets/models/utility_box.gltf
```

The supported loader subset is intentionally tiny: `.gltf`, one embedded base64 buffer, `POSITION` float `VEC3`, indexed triangle list, no materials/textures/animation. v0.18 can load multiple scene-authored mesh asset ids, but this is still not an asset registry or production mesh pipeline. See `docs/MESH_RENDERING.md` before changing mesh loading or renderer behavior.

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
- If the runtime logs `Runtime scene load failed`, check the `--scene` path first, then run `python tools/validate_scene.py <path>`.
- If scene validation fails, inspect `data/scenes/ferry_office.scene.json` and run `python tools/scene_report.py` to see the current object counts and required ids.
- If mesh references fail, run `python tools/mesh_report.py` and verify referenced files exist under `assets/models` with license/provenance in scene data.
- v0.3+ collision is debug-only static AABB collision. If a collider layout feels odd, inspect `src/game/PrototypeWorld.cpp` and rerun `scripts/verify.ps1` after edits.
- v0.4 interaction focus is debug-only point/radius selection with a facing preference. If an object does not focus, move closer and face the marker, then press `E`.
- v0.5 traversal uses `Space` only when a traversal affordance is focused. If traversal does not trigger, move near the traversal start marker and face the path direction. If no traversal is focused, `Space` remains normal jump.
- v0.5.1 traversal starts from the player's current position inside the focus radius and lands through the world collision resolver. In GDI debug text, check `travStart=current`, `travProgress`, and `travLanded`.
- v0.14 vehicle movement is still a deterministic placeholder controller, not Jolt VehicleConstraint. If the vehicle does not enter, make sure no Ferry Office interaction prompt is currently focused. If exit is blocked, move/turn inside the service yard or dock road until the side exit marker is clear. Use the service-yard pad, east opening, short dock road, rail/curb edges, and turn-around/end marker as the intended compact road-test area.
