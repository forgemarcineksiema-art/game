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

v0.2 controls:

- `W/A/S/D`: camera-relative movement
- `Shift`: sprint
- `Space`: jump
- Mouse movement over the window: orbit camera
- Arrow keys: camera fallback controls
- `Esc`: quit

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

## Test

After configure/build:

```powershell
ctest --preset windows-vs2022-debug
```

The configured preset can be replaced with the preset recorded in `build/.last_preset`.

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

## Troubleshooting

- If CMake cannot find Visual Studio, install Visual Studio Build Tools with the C++ desktop workload, then rerun `scripts/configure.ps1`.
- If `ninja-debug` is selected, make sure both Ninja and a C++ compiler are in PATH.
- If the windowed app fails, run the headless path: `scripts/run.ps1 -Args @("--smoke-test", "--frames", "3")`.
- If DirectX 11 fails, try `scripts/run.ps1 -Args @("--renderer", "gdi", "--frames", "120")`.
- If all windowed rendering fails, keep `--renderer null --headless` working while the renderer issue is fixed.
- If mouse-look feels risky in a VM or remote session, use arrow keys for camera orbit. Mouse is not captured/locked in v0.2.
