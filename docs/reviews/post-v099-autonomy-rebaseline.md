# Post-v0.99 Autonomy Re-baseline

Date: 2026-05-17

## Summary

- CONFIRMED: The immediate player-facing bug was real: the old Jolt adapter could pass route/checkpoint QA without proving `W+A`, `W+D`, reverse+left, and reverse+right steering signs.
- CONFIRMED: Jolt vehicle QA now emits and validates `inputSemanticsChecks` for deterministic and Jolt backends.
- CONFIRMED: The green report is `build/physics/jolt-input-semantics-report.json`.
- INTERPRETATION: Jolt is more credible after this goal, but still not proven as final manual feel or a universal default runtime.

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

- BET: Build a manual/live input capture-replay evidence harness for player verbs and camera/world interaction. It should record or replay real control sequences across on-foot, enter vehicle, forward/reverse steering, camera reset/readability, and authored collision contact, then emit evidence that is not just another checkpoint route.

Why this, not terrain:

- CONFIRMED: The failure came from QA blindness around input semantics, not from missing terrain variety.
- INFERRED: Terrain/world substrate may become important later, but adding it before a trustworthy manual/live evidence harness would create a larger surface area for the same class of false confidence.

Rejected direction 1:

- WARNING: Do not start a terrain/road/world-substrate implementation just because "zroznicowane tereny" was mentioned. The current evidence says the project first needs better proof around player verbs and runtime truth.

Rejected direction 2:

- WARNING: Do not promote Jolt as universal default from this fix alone. This goal proves sign semantics and the current recorded routes; it does not prove full manual play, all-world collision, slopes, dynamic obstacles, traffic, damage, or camera collision.

## Open Limits

- UNVERIFIED: Human/manual feel after the reverse semantics correction.
- UNVERIFIED: Jolt behavior on non-flat roads, slopes, dense dynamic obstacles, or real traffic-like blockers.
- UNVERIFIED: Whether the current reverse correction remains appropriate after replacing the current vehicle model/wheel setup.
- INFERRED: `SandboxLayer` and Ferry Office glue remain a pressure point; more gameplay logic there will make the next playable slice harder to reason about.

## One-Line Truth

INTERPRETATION: After v0.99, the project is strongest as an engine/QA testbed with increasingly useful gameplay evidence, but it still needs a truthful manual/live player-verb harness before bigger world, content, or default-runtime decisions are credible.
