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

- [ ] At spawn, the pickup focus prompt appears: `Press E: Pick up Test Item`.
- [ ] Pressing `E` once on the pickup logs/records the pickup result.
- [ ] Holding `E` does not repeatedly trigger the pickup.
- [ ] After pickup consumption, the pickup no longer regains focus.
- [ ] Moving near and facing the toggle marker shows the toggle prompt.
- [ ] Pressing `E` on the toggle changes its state/color/debug result.
- [ ] Moving near and facing the info marker shows the info prompt.
- [ ] Pressing `E` on the info marker logs/records the info message.
- [ ] If an object does not focus, moving closer and facing it resolves the issue.

## Exit

- [ ] `Esc` requests quit.
- [ ] Closing the window exits cleanly.
- [ ] Logs show startup, runtime renderer, sandbox attach/detach, renderer shutdown, and engine shutdown.

