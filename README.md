# game

Custom C++ action/adventure engine foundation.

This repository is intentionally starting small: CMake, a minimal engine loop, validation scripts, AI-facing docs, and tiny renderer backends for DirectX 11, GDI fallback, and headless smoke tests. It is not a clone of any commercial game and must not include copyrighted assets, names, maps, characters, logos, or mission designs.

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
