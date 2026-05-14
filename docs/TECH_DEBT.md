# Technical Debt

Last updated: 2026-05-14

This file lists known foundation issues before v0.5. It is not a mandate to fix everything immediately. Future goals should pick the smallest debt item that blocks their milestone.

## Build / Toolchain

- Plain PowerShell PATH still cannot find `cl`, `clang++`, `g++`, `msbuild`, `ninja`, or `vcpkg`, though CMake can build through the Visual Studio generator.
- No vcpkg manifest exists because no third-party dependencies are currently needed.
- The version is stored only in CMake. There is no generated version header beyond the current compile definition.

## Renderer

- DX11 hardware/debug device creation fails in this environment and falls back to WARP.
- DX11 debug text is currently a no-op; GDI shows debug text.
- Debug boxes/lines are enough for prototypes but not a mesh/material pipeline.
- There is no resize handling, depth buffer, camera clip tuning, or resource lifetime stress testing.

## Input

- Input is polled in the Win32 window path, not a full input subsystem.
- `E` has both held and pressed-edge state.
- `Space` now behaves as a pressed-edge jump action in the Win32 path; this is correct for jumping, but docs and tests should keep that distinction explicit.
- Mouse-look is hover-delta only. The cursor is not captured, hidden, centered, or locked.
- There is no key rebinding or controller support.

## Player / Camera

- Movement is camera-relative and deterministic, but player acceleration/deceleration is immediate.
- Jump uses simple vertical velocity and gravity.
- Camera smoothing exists, but there is no camera obstruction, shoulder switching, collision, or indoor tuning.
- Focus currently uses player-facing, which can feel different from camera-facing if the player rotates the camera without moving.
- Traversal uses deterministic interpolation and still needs hands-on camera feel tuning during the traversal arc.

## World / Collision

- Static AABB-only collision.
- Player proxy is a simple vertical radius/height approximation, not a swept capsule.
- Ground is a flat floor-height query.
- No slopes, ramps, stairs, moving platforms, or step-up behavior.
- Raycast only checks static AABB colliders.
- Collision layout is hardcoded in `TestWorld::buildDefaultCollisionTestLayout`.

## Interaction

- Interactables are hardcoded in `TestScene`.
- Focus uses point/radius volumes with a facing preference and close proximity fallback.
- Toggle objects only change interaction state; they do not alter collision or world geometry.
- Pickup/world state is local runtime state only; there is no inventory, persistence, or save data.
- No UI framework exists, so prompts are debug text/logs only.

## World State

- `WorldState` is an in-memory local event ledger, not a save/load system.
- Flag mappings are hardcoded in `TestScene`.
- Repeated same-value flag writes are ignored, which is correct for v0.6 but may need richer event semantics later.
- Debug summary text can become long as more flags are added.
- There is no mission graph, quest scripting, dialogue integration, global event bus, or persistence layer.

## Traversal

- Traversal affordances are hardcoded in `TestScene`.
- Only one affordance type exists: `Vault`.
- Traversal starts from the player's current position and resolves collision after landing, but it still skips continuous collision checks during the controlled motion.
- There is no animation, IK, ledge hang, full climb, wall climb, or physics-driven vault.
- Traversal prompt/debug visibility is functional but not polished.
- The current traversal route is a prototype access gate, not the full Ferry Office slice.

## Naming / Architecture

- `TestWorld` and `TestScene` are still acceptable for v0.4.1, but they should be renamed to `PrototypeWorld` and `PrototypeScene` when the project moves from test layout to first vertical slice.
- `InteractionSystem` belongs in `src/game` for now. Promote it to `src/engine` only after multiple gameplay contexts prove a stable boundary.
- There is no scene serialization, asset registry, or editor.

## Recommended Debt Before Or During v0.7

1. Keep `WorldState` runtime-only unless v0.7 explicitly needs persistence.
2. Keep state-to-action mapping in the scene layer until the slice proves a broader boundary.
3. Consider renaming `TestWorld` / `TestScene` to `PrototypeWorld` / `PrototypeScene` only as part of a focused vertical-slice cleanup.
4. Keep debug text readable if more remembered flags are added.
5. Avoid adding a mission scripting system before the micro-slice proves its minimal state flow.
