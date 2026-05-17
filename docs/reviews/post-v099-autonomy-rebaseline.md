# Post-v0.99 Autonomy Re-baseline

Date: 2026-05-17

## Summary

- CONFIRMED: The immediate player-facing bug was real: the old Jolt adapter could pass route/checkpoint QA without proving `W+A`, `W+D`, reverse+left, and reverse+right steering signs.
- CONFIRMED: Jolt vehicle QA now emits and validates `inputSemanticsChecks` for deterministic and Jolt backends.
- CONFIRMED: The green report is `build/physics/jolt-input-semantics-report.json`.
- CONFIRMED: manual play on 2026-05-17 reported current Jolt vehicle input/controls, physics, and camera as good in the Ferry Office playtest path.
- INTERPRETATION: Jolt is more credible after this goal for the current player-facing vehicle verb, but still not proven as full-world driving or a universal default runtime.

## What Was Fixed

- CONFIRMED: `src/engine/physics/JoltVehicleRuntime.cpp` now maps Tidebreak steer semantics onto Jolt's opposite wheel steer convention.
- CONFIRMED: `src/engine/physics/JoltVehicleRuntime.cpp` applies a small reverse-steering semantics correction so reverse+steer matches the deterministic player-facing baseline.
- CONFIRMED: `src/game/FerryOfficeVehiclePhysicsQa.*` now reports `inputSemanticsChecks`.
- CONFIRMED: `tools/vehicle_runtime_qa.py` rejects reports that omit sign-sensitive input semantics checks or report the wrong sign.
- CONFIRMED: `tests/EngineCoreTests.cpp` and `tests/test_vehicle_runtime_qa.py` cover the regression.

## QA Gap

- CONFIRMED: Before this goal, `vehicle_runtime_qa.py` could return `recommendation=promote` without checking steering direction semantics.
- CONFIRMED: Earlier QA measured route progress, yaw response magnitude, road-edge evidence, and extended route completion; it did not prove that player input combinations turned in the expected direction.
- INTERPRETATION: This is why "green vehicle QA" was not manual-feel proof. It proved that the scripted route could complete, not that the core player verb felt trustworthy.

## Evidence

- CONFIRMED: pre-change `python tools/vehicle_runtime_qa.py --exe build/windows-vs2022-debug-jolt/Debug/EngineApp.exe --report-json build/physics/pre-jolt-input-semantics-report.json` passed while missing `inputSemanticsChecks`.
- CONFIRMED: red C++ regression exposed the old Jolt steering sign mismatch.
- CONFIRMED: green `build/physics/jolt-input-semantics-report.json` contains eight input checks.
- CONFIRMED: Jolt green yaw deltas: `forward-left=-164.946`, `forward-right=164.959`, `reverse-left=16.519`, `reverse-right=-10.051`.
- CONFIRMED: deterministic green yaw deltas: `forward-left=-59.712`, `forward-right=59.712`, `reverse-left=40.170`, `reverse-right=-40.170`.

## Ferry Office Status

- CONFIRMED: Ferry Office remains useful as a regression testbed for input, camera, scene data, road-edge, and runtime evidence.
- INTERPRETATION: Ferry Office should not be treated as the first real game location. It is a controlled playground with authored evidence hooks.
- WARNING: Adding more Ferry Office content, polish, props, or terrain language now would risk hiding the same problem again: scripted proof can look like progress while core player verbs remain under-proven.

## Direction Decision

Recommended next goal:

- BET: Run a first-real-slice direction gate that separates `Ferry Office as regression testbed` from `the actual game target`. The gate should evaluate target gameplay loop, world structure, terrain/road needs, scene architecture, asset/render pipeline, character/player presence, and authoring boundaries together, then choose the smallest implementation slice that proves what Tidebreak is becoming. It must not extend the current flat debug map with more content, props, terrain labels, or local interaction reactions.

Why this, not "world response":

- CONFIRMED: the immediate vehicle input/physics/camera blocker is now green in tests and accepted manually in the current playtest path.
- INTERPRETATION: the project discussion moved past "make Ferry Office react more" and toward "stop pretending Ferry Office can become the game by incremental cleanup".
- WARNING: a "world responds to player presence" pass on the current map would likely become another debug-playground improvement, not a real answer to what the game is.

Rejected direction 1:

- WARNING: Do not start a terrain/road/world-substrate implementation just because "zroznicowane tereny" was mentioned. Terrain might be part of the eventual answer, but choosing it reactively would repeat the same mistake as choosing polish/content reactively.

Rejected direction 2:

- WARNING: Do not run another Ferry Office content/polish/readability/world-reaction pass as the next strategic milestone. Ferry Office should keep catching regressions; it should not be mistaken for the target playable world.

## Open Limits

- CONFIRMED: current manual vehicle input/physics/camera feel is good enough to stop treating the vehicle verb as the active blocker.
- UNVERIFIED: Jolt behavior on non-flat roads, slopes, dense dynamic obstacles, or real traffic-like blockers.
- UNVERIFIED: Whether the current reverse correction remains appropriate after replacing the current vehicle model/wheel setup.
- INFERRED: `SandboxLayer` and Ferry Office glue remain a pressure point; more gameplay logic there will make the next playable slice harder to reason about.
- UNVERIFIED: what the first real target slice/system should be after deliberately leaving Ferry Office as a testbed.

## One-Line Truth

INTERPRETATION: After v0.99, the project is strongest as an engine/QA testbed with a now-credible current vehicle verb, but the next hard decision is not "improve Ferry Office"; it is choosing the first real target slice/world-system that proves what the game is outside the debug playground.
