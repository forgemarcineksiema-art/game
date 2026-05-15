# AGENTS.md

Durable instructions for future Codex work in this repository.

## Mission

This repository is a custom C++ action/adventure engine workbench. It is inspired by the technology direction of large third-person open-world and cinematic adventure games, but it must never copy or include copyrighted game content, names, maps, characters, logos, mission designs, or ripped assets.

v0.1 is an engine foundation only. Keep changes boring, robust, and easy for future AI runs to validate.

## Required Commands

From the repository root:

```powershell
scripts/doctor.ps1
scripts/configure.ps1
scripts/build.ps1
scripts/verify.ps1
```

Run `scripts/verify.ps1` before claiming that a change builds, tests, or works. If a command fails, record the exact command and failure in `docs/STATUS.md`.

Before choosing a gameplay milestone, read:

- `docs/GAME_DIRECTION.md`
- `docs/VERTICAL_SLICE.md`
- `docs/TECH_DEBT.md`
- `docs/ROADMAP.md`

Before physics, vehicles, player collision rewrites, or world-query work, also read:

- `docs/PHYSICS_DECISION.md`

Before scene layout, map, object placement, asset, model, scale, or art-direction work, also read:

- `docs/SCENE_AUTHORING.md`
- `docs/ASSET_GUIDE.md`
- `docs/ART_DIRECTION.md`
- `docs/MESH_RENDERING.md`

## Run Commands

```powershell
scripts/play.ps1
scripts/run.ps1
scripts/run.ps1 -Args @("--smoke-test", "--frames", "3")
```

Direct executable form after a Visual Studio Debug build:

```powershell
build/windows-vs2022-debug/Debug/EngineApp.exe --smoke-test --frames 3
```

Scene authoring tools:

```powershell
python tools/scene_report.py
python tools/validate_scene.py
python tools/scale_audit.py
python tools/mesh_report.py
```

## Architecture Rules

- Keep engine code under `src/engine`.
- Keep game/sandbox code under `src/game`.
- Keep tests under `tests`.
- Keep tooling under `scripts` and `tools`.
- Keep original assets under `assets`.
- Keep authored scene data under `data/scenes`.
- Keep renderer code behind `IRenderer` and platform code behind `IWindow`.
- Prefer small modules with explicit ownership over broad utility files.
- Do not add advanced gameplay systems before the engine boot, validation, and renderer path are stable.

## Dependency Rules

- Use the C++ standard library first.
- Prefer vcpkg manifest mode only when a dependency is clearly worth adding.
- Record every new dependency in `docs/DECISIONS.md` with what problem it solves and why it is not overkill.
- Do not add physics, animation, AI, audio, scripting, networking, or asset import libraries in v0.1-style work unless a later milestone explicitly calls for them.

## Coding Conventions

- C++20.
- Namespaces use `engine`.
- Classes use `PascalCase`; functions and variables use `camelCase`.
- Prefer RAII and value types.
- Prefer simple interfaces over speculative abstraction.
- Keep comments sparse and useful.
- Do not pretend deferred systems are complete.

## Do Not

- Do not use Unity, Unreal, Godot, or another full game engine.
- Do not jump to DirectX 12 or Vulkan before the roadmap calls for it.
- Do not copy copyrighted content or game-specific designs from commercial titles.
- Do not delete user work or old experiments without documenting the move in `docs/STATUS.md`.
- Do not claim validation passed unless the command was run in the current work session.

## Definition of Done

A change is done only when:

- The intended code/docs/scripts are updated.
- `docs/STATUS.md` records what changed and what commands ran.
- `scripts/verify.ps1` ran, or the exact blocker is documented.
- For every completed `/goal`, commit the finished work and push it to the current git remote/branch after validation passes. Do not commit/push if validation fails, the worktree contains unrelated user changes, or the user explicitly says not to.
- The final response says what passed, what failed, and what remains.
