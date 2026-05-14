# game

Custom C++ action/adventure engine foundation.

This repository is intentionally starting small: CMake, a minimal engine loop, validation scripts, AI-facing docs, tiny renderer backends for DirectX 11, GDI fallback, and headless smoke tests, plus third-person player/camera, static world collision, and interaction prototypes. It is not a clone of any commercial game and must not include copyrighted assets, names, maps, characters, logos, or mission designs.

## Quick Start

```powershell
scripts/doctor.ps1
scripts/configure.ps1
scripts/build.ps1
scripts/verify.ps1
```

Run the smoke path:

```powershell
scripts/run.ps1 -Args @("--smoke-test", "--frames", "3")
```

Read `AGENTS.md` and `docs/RUNBOOK.md` before making engine changes.

Direction docs:

- `docs/GAME_DIRECTION.md`
- `docs/VERTICAL_SLICE.md`
- `docs/TECH_DEBT.md`
- `docs/MANUAL_TEST_CHECKLIST.md`

## Prototype Controls

- `W/A/S/D`: camera-relative movement
- `Shift`: sprint
- `Space`: jump
- `E`: interact with the focused debug object
- Mouse movement over the window: orbit camera
- Arrow keys: camera fallback controls
- `Esc`: quit
