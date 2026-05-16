# Physics Decision

Last updated: 2026-05-16

## Recommendation

Choose Jolt Physics as Tidebreak's default production physics candidate unless a later production-scale spike exposes a concrete blocker.

Keep PhysX as the backup candidate. Do not choose Bullet for the main engine path unless a future, narrow test gives a strong reason.

This decision does not mean every current gameplay collision path should be rewritten immediately. v0.9.2 adds a vendor-safe `src/engine/physics` boundary and an opt-in Jolt backend spike. v0.33 adds a QA-only Ferry Office static-collision parity bridge, v0.34 adds a player-proxy contact probe, and v0.35 adds a wheeled vehicle feasibility probe. Existing Ferry Office gameplay still uses the tested prototype paths until a later goal migrates one behavior at a time. v0.48 adds deterministic runtime enter-drive-exit-confirm playthrough evidence for the first service job, but not Jolt default promotion.

## Why Decide Now

Tidebreak will need physics for:

- static world collision,
- player/world queries,
- triggers and remembered-state volumes,
- raycasts/sweeps/overlaps,
- future vehicle feel,
- camera obstruction,
- dynamic props later.

Choosing late would risk rewriting core gameplay, debug tooling, scene data, and vehicle behavior after too much code depends on custom AABB collision. Choosing early gives us a stable direction while the engine is still small.

## Candidates

## Jolt

Source: https://github.com/jrouwe/JoltPhysics and https://jrouwe.github.io/JoltPhysics/

Assessment:

- License/commercial: MIT, simple for commercial use.
- Build/CMake: CMake-friendly. v0.9.2 integrates it through an explicit `windows-vs2022-debug-jolt` preset using pinned `v5.5.0` FetchContent.
- Windows-first: supports Windows desktop and Visual Studio.
- Laptop expectation: good fit. v0.9.2 disables optional CPU instruction sets beyond baseline SSE2 for the spike to avoid surprising older laptop CPU requirements.
- Character path: supports game character simulation with rigid-body and virtual character approaches.
- Vehicle path: includes wheeled vehicles, tracked vehicles, and motorcycles.
- Queries: raycasts, shape tests, shape casts, and broadphase tests are supported.
- Triggers/sensors: supports sensors/trigger volumes.
- Debug visualization: includes debug renderer hooks; v0.9.2 exposes engine-owned debug line data first.
- Determinism/fixed timestep: Jolt documents deterministic simulation limits and expects disciplined stepping.
- Lock-in risk: manageable if game code talks to `src/engine/physics`, not `JPH::*`.
- Fit: best match for a custom game engine that wants serious physics without adopting a full commercial engine stack.

Decision: primary production candidate.

## PhysX

Source: https://nvidia-omniverse.github.io/PhysX/physx/5.6.0/index.html and https://github.com/NVIDIA-Omniverse/PhysX

Assessment:

- License/commercial: CPU source is available under BSD-3 terms, with NVIDIA GPU binaries available at no cost according to the PhysX 5 documentation.
- Build/CMake: mature but heavier. The official direction is tied to the NVIDIA/Omniverse ecosystem.
- Windows-first: strong Windows support.
- Laptop expectation: CPU path is viable; GPU-exclusive features are not a good baseline for a weak/integrated laptop target.
- Character path: PhysX has a kinematic character controller module, but character controllers remain highly game-specific.
- Vehicle path: PhysX has a vehicle SDK and the 5.x vehicle system is flexible.
- Queries: raycasts, sweeps, overlaps, and scene queries are strong.
- Triggers/sensors: supported.
- Debug visualization: strong tooling around PhysX/Omniverse, but that can pull the project toward a larger ecosystem.
- Determinism/fixed timestep: strong simulation tech, but the integration discipline is still ours.
- Lock-in risk: higher than Jolt because PhysX is a larger middleware-style dependency.
- Fit: credible backup if Jolt fails vehicle or query requirements, but heavier than Tidebreak needs today.

Decision: backup candidate.

## Bullet

Source: https://github.com/bulletphysics/bullet3

Assessment:

- License/commercial: permissive zlib license.
- Build/CMake: available through vcpkg and CMake paths, but the project history and docs feel less aligned with a new C++20 game engine foundation.
- Windows-first: supported.
- Laptop expectation: CPU path is fine; experimental GPU/OpenCL path is not a fit for this project.
- Character path: possible, but less attractive than Jolt/PhysX for this project.
- Vehicle path: possible, but less compelling for the future Tidebreak vehicle-feel work.
- Queries: mature collision/query features.
- Triggers/sensors: possible via collision callbacks/overlaps.
- Debug visualization: available but not as clean a fit for our current renderer/debug path.
- Determinism/fixed timestep: possible with care, but not a decisive advantage here.
- Lock-in risk: moderate.
- Fit: known and proven, but not the best default for a fresh Windows-first custom engine in this project.

Decision: not preferred.

## Temporary Custom Collision

Assessment:

- License/commercial: project-owned.
- Build/CMake: no dependency risk.
- Windows-first/laptop: trivial.
- Character path: enough for the current Ferry Office debug slice only.
- Vehicle path: not enough.
- Queries: current static AABB raycast exists, but sweeps/overlaps/capsules are incomplete.
- Triggers/sensors: possible but would become hand-rolled engine complexity.
- Debug visualization: already works.
- Determinism/fixed timestep: easy now, but only because the feature set is tiny.
- Lock-in risk: high in a different way; we could lock ourselves into an underpowered homemade solver.
- Fit: good temporary gameplay safety net while Jolt is integrated behind the engine boundary.

Decision: keep temporarily for existing gameplay, but do not grow it into the main physics engine.

## v0.9.2 Spike Result

Added:

- `src/engine/physics/PhysicsWorld.h`
- `src/engine/physics/PhysicsWorld.cpp`
- `src/engine/physics/JoltPhysicsWorld.cpp`
- `ENGINE_ENABLE_JOLT_PHYSICS` CMake option
- `windows-vs2022-debug-jolt` CMake preset

Validated:

- Default dependency-free build still passes.
- Opt-in Jolt configure/build passes.
- Opt-in Jolt CTest passes.
- Jolt initializes and shuts down through `IPhysicsWorld`.
- Jolt can create a static box and raycast it through the engine-owned API.
- Game code does not include Jolt headers or expose Jolt types.

## v0.33 Runtime Parity Bridge Result

Added:

- `IPhysicsWorld::overlapBox()` as a vendor-free overlap-style query.
- `src/game/FerryOfficePhysicsParity.h/.cpp`, a QA-only runner that loads `data/scenes/ferry_office.scene.json`, builds the existing `PrototypeWorld`, mirrors authored static box colliders plus a simple scene-floor body into the requested engine physics backend, and compares deterministic probes.
- `--qa-physics-parity ferry-office-collision` and `--qa-physics-report <path>` CLI flags.
- `tools/physics_parity_qa.py`, which runs a Jolt-enabled `EngineApp.exe`, validates report schema `v0.33-ferry-office-physics-parity`, and checks floor, raycast, and overlap probe results.

Validated:

- Default dependency-free build still passes and still reports the opt-in backend unavailable when physics parity is requested without the Jolt preset.
- Opt-in `windows-vs2022-debug-jolt` configure/build passes.
- Opt-in CTest now includes `FerryOfficeJoltPhysicsParitySmoke` and passes.
- The Ferry Office parity report validates 9 authored static colliders, 4 floor probes, 4 raycast probes, and 4 player-overlap-style probes against the current `PrototypeWorld` behavior.

Important limits:

- The parity bridge is off by default and does not open a window or run gameplay.
- Live player movement, traversal landing, service-gate state changes, vehicle driving, and vehicle collision still use existing prototype systems.
- The floor body is a bounded validation helper for scene-query parity, not terrain, slopes, stairs, ramps, or a full ground model.
- Overlap parity currently validates static box overlap-style behavior through the engine physics API. It is not a character controller, sweep test, contact solver, dynamic rigid-body migration, or vehicle constraint.

## v0.34 Character Contact Probe Result

Added:

- `src/game/FerryOfficeCharacterContactQa.h/.cpp`, a QA-only runner that compares the current player collision proxy against opt-in physics contact candidates in the mirrored Ferry Office static scene.
- `--qa-physics-parity ferry-office-character-contact` as a second physics QA scenario.
- `tools/character_contact_qa.py`, which runs a Jolt-enabled `EngineApp.exe`, validates report schema `v0.34-ferry-office-character-contact`, and checks the contact probe set.

Validated:

- Default dependency-free build still passes and reports the opt-in backend unavailable for the character/contact scenario.
- Opt-in `windows-vs2022-debug-jolt` configure/build passes.
- Opt-in CTest now includes `FerryOfficeCharacterContactQaSmoke` and passes.
- The character/contact report validates 7 probes: dock-floor grounding, office back wall blocking, service gate blocking, service barrier blocking, dock lane clear movement, office corner pushout, and opened-gate clear movement.

Important limits:

- The v0.34 adapter is read-only QA. It uses physics overlap candidates plus the existing player-proxy resolution shape to compare against `PrototypeWorld`.
- It is not a live `PlayerController` migration, Jolt character controller, sweep/capsule implementation, dynamic contact solver, or vehicle physics step.
- The service-gate opened case is represented by omitting the gate body from that probe's mirrored static world; this proves the nonblocking expectation, not a general runtime dynamic-collider system.

## v0.35 Vehicle Feasibility Probe Result

Added:

- `src/engine/physics/VehicleProbe.h/.cpp`, an engine-owned vehicle feasibility API that returns vendor-free samples and promote/defer evidence.
- `src/engine/physics/JoltVehicleProbe.cpp`, an opt-in Jolt-only runner using a `VehicleConstraint` with four wheeled-vehicle settings, kept private to `src/engine/physics`.
- `src/game/FerryOfficeVehiclePhysicsQa.h/.cpp`, a QA-only Ferry Office service-vehicle report runner with schema `v0.35-ferry-office-vehicle-feasibility`.
- `--qa-physics-parity ferry-office-vehicle-feasibility`.
- `tools/vehicle_physics_qa.py`, which runs the Jolt-enabled `EngineApp.exe`, requires backend `jolt`, validates the authored `service-yard-vehicle`, checks samples, wheel contacts, bounds, and the promote/defer recommendation.

Validated so far:

- Default dependency-free build still passes and reports the opt-in backend unavailable for the vehicle scenario.
- The dependency-free simple baseline can run the same compact input script and produce a bounded report for C++ regression coverage.
- Opt-in Jolt configure/build passes.
- The Jolt vehicle feasibility report records 5 samples, 250 input frames, 4 wheel contacts per sample, in-bounds motion on the Ferry Office service-yard vehicle, and recommendation `promote`.
- `rg -n "Jolt|JPH::|<Jolt/|JPH/" src\game` returns no matches; Jolt types remain private to engine-owned physics code.

Important limits:

- This is a QA-only feasibility probe. It does not replace live `VehicleController` gameplay yet.
- The Jolt runner proves a compact scripted service-yard maneuver with a temporary floor/body setup, not a complete road, damage, traffic, suspension tuning, camera, audio, or gameplay vehicle system.
- The next physics/vehicle goal should build a narrow runtime adapter that can be compared against the deterministic vehicle path behind a switch, then decide whether to promote it into live play.

## v0.36 Vehicle Runtime Adapter Comparison Result

Added:

- `src/engine/physics/VehicleRuntime.h/.cpp`, a vendor-free frame-stepped vehicle runtime adapter boundary with a dependency-free simple fallback.
- `src/engine/physics/JoltVehicleRuntime.cpp`, an opt-in Jolt-only runtime adapter that initializes a compact wheeled vehicle scene, steps one frame at a time, and returns vendor-free telemetry.
- `--qa-physics-parity ferry-office-vehicle-runtime-comparison`, an explicit switch for comparing runtime adapter behavior without replacing live gameplay.
- `tools/vehicle_runtime_qa.py`, which runs a Jolt-enabled `EngineApp.exe`, validates report schema `v0.36-ferry-office-vehicle-runtime-comparison`, requires backend `jolt`, checks paired deterministic/adapter samples, and enforces comparison thresholds.

Validated so far:

- Default dependency-free build passes with the simple runtime adapter and reports the opt-in runtime backend unavailable when requested without the Jolt preset.
- Opt-in Jolt configure/build passes.
- The Jolt runtime comparison report records 5 paired samples against the deterministic `VehicleController` baseline, 4 wheel contacts per adapter sample, max position delta about 2.95 meters in the compact service-yard script, and recommendation `promote`.
- v0.45 added controls checks for tap/coast, brake, reverse, and reverse coast-down.
- v0.46 added service-run route checks: deterministic reached the authored checkpoint in 139 frames and untuned Jolt reached it in 301 frames, both in bounds.
- v0.47 tuned the opt-in Jolt route pace and tightened the route-check budget: deterministic still reaches the authored checkpoint in 139 frames and Jolt now reaches it in 213 frames under a 240-frame route budget.
- v0.48 added deterministic first-job runtime playthrough evidence: the QA path enters the authored service vehicle through runtime input, reaches the dock-road checkpoint in 139 frames, exits at a clear position, and confirms the service run.
- `rg -n "Jolt|JPH::|<Jolt/|JPH/" src\game` returns no matches; Jolt types remain private to engine-owned physics code.

Important limits:

- This is still a QA/runtime comparison switch, not a normal play-mode replacement.
- The deterministic `VehicleController` remains the live gameplay fallback.
- The Jolt runtime adapter is proven against compact controls checks and a straight service-run route proxy. It still needs the same enter-drive-exit-confirm live-loop replay evidence that v0.48 now covers for the deterministic controller before it becomes the default vehicle path.

## v0.37 Live Opt-in Vehicle Runtime Switch Result

Added:

- `--vehicle-runtime jolt`, an explicit live playtest switch for the service-yard vehicle runtime.
- `scripts\play.ps1 -VehicleRuntime jolt`, a wrapper shortcut for manual opt-in runs when pointed at the Jolt-enabled executable.
- `SandboxLayer` live adapter plumbing that keeps deterministic vehicle gameplay as the default and falls back honestly when the selected runtime backend is unavailable.
- Debug text now reports `vehicleRuntime=deterministic`, `vehicleRuntime=<backend>-live`, or an unavailable/init-failed state.

Validated so far:

- Default dependency-free builds keep `vehicleRuntime=deterministic` unless the explicit switch is requested.
- A default executable accepts `--vehicle-runtime jolt` but reports the selected backend unavailable and keeps deterministic fallback behavior.
- The opt-in `windows-vs2022-debug-jolt` executable initializes the switched path and reports `vehicleRuntime=jolt-live` in debug text.
- `rg -n "Jolt|JPH::|<Jolt/|JPH/" src\game` returns no matches; Jolt types remain private to engine-owned physics code.

Important limits:

- This is a manual playtest switch, not default vehicle promotion.
- The switched vehicle uses the v0.36 runtime adapter state for live driving, but player collision, traversal, service-gate behavior, dynamic objects, traffic, damage, audio, and production vehicle tuning remain unchanged.
- The next decision should come from fuller input-scripted live-loop replay evidence for deterministic versus `jolt-live`, with human playtest as useful follow-up rather than the only gate.

Important implementation choices:

- Jolt is opt-in for now. `scripts/verify.ps1` keeps using the default dependency-free preset.
- Jolt is fetched through pinned CMake `FetchContent` tag `v5.5.0` in the opt-in preset, not through the normal configure path.
- The spike forces Jolt to use the dynamic MSVC runtime to match this project.
- The spike disables optional SSE4/AVX/FMA/LZCNT/TZCNT paths so the first laptop baseline does not depend on newer CPU instruction support.
- The engine API exposes `BodyHandle`, `BoxColliderDesc`, `RaycastResult`, and `PhysicsDebugLine`, not `JPH::*`.

## Deferred Work

- Do not migrate player movement to Jolt in v0.9.2.
- Do not add vehicles in v0.9.2.
- Do not add physics scene serialization yet.
- Do not require vcpkg until the dependency strategy is revisited.
- Do not expose Jolt types to `src/game`.
- Do not treat the v0.33 parity bridge or v0.34 character/contact probe as a gameplay migration.

## Next Recommendation

v0.10 used the physics foundation without promoting full Jolt vehicle constraints yet:

- The live vehicle feel spike is deterministic and game-layer scoped in `VehicleController`.
- `SandboxLayer` uses `engine::physics::IPhysicsWorld` with the dependency-free `simple` backend for a small service-yard validation/debug world.
- Tests scan `src/game` to prevent accidental `Jolt` / `JPH::*` vendor references.
- The opt-in `windows-vs2022-debug-jolt` preset remains the proof that the Jolt backend still configures, builds, and tests through the engine API.

Next recommendation: keep deterministic vehicle gameplay as the default and run an opt-in Jolt live-loop comparison milestone. The v0.47 route proxy proves Jolt can reach the service-run checkpoint within the 240-frame budget, and v0.48 proves the deterministic first-job runtime vehicle loop, but default Jolt promotion remains premature until the switched path validates enter, drive, exit, and service-run confirmation through comparable evidence.
