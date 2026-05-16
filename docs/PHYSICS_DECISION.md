# Physics Decision

Last updated: 2026-05-17

## Recommendation

Choose Jolt Physics as Tidebreak's default production physics candidate unless a later production-scale spike exposes a concrete blocker.

Keep PhysX as the backup candidate. Do not choose Bullet for the main engine path unless a future, narrow test gives a strong reason.

This decision does not mean every current gameplay collision path should be rewritten immediately. v0.9.2 adds a vendor-safe `src/engine/physics` boundary and an opt-in Jolt backend spike. v0.33 adds a QA-only Ferry Office static-collision parity bridge, v0.34 adds a player-proxy contact probe, and v0.35 adds a wheeled vehicle feasibility probe. Existing Ferry Office gameplay still uses the tested prototype paths until a later goal migrates one behavior at a time. v0.49 adds opt-in Jolt runtime enter-drive-exit-confirm playthrough evidence for the first service job, v0.68 promotes Jolt from exploratory option to preferred vehicle-runtime candidate, v0.70 begins a controlled preferred-runtime trial through the play wrapper, v0.96 makes vehicle-feel decisions Jolt-first with deterministic kept as baseline/fallback, v0.99 narrows Jolt route pace with a conservative straight-drive assist while preserving reverse, obstacle, and camera checks, the first post-v0.99 evidence gate adds an explicit Jolt playthrough CTest plus authored dock-road edge clearance checks, and the current post-v0.99 gate proves a broader reverse/turn/camera route blocked by authored road-edge runtime collision proxies. Direct app and QA defaults remain deterministic unless a runtime is explicitly requested.

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
- The Ferry Office parity report validates the current authored static colliders, 4 floor probes, 4 raycast probes, and 4 player-overlap-style probes against the current `PrototypeWorld` behavior. After the v0.95 Low Dock Drain access barrier, the current Ferry Office scene has 10 static colliders.

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
- v0.49 added the same first-job runtime playthrough path for the opt-in Jolt adapter: it reaches the dock-road checkpoint in 213 frames, exits at a clear position, confirms the service run, and reports no fallback or bounds hit.
- v0.59 tightens obstacle-proxy validation and tunes the opt-in Jolt adapter: Jolt now reaches the checkpoint in 212 frames, keeps the obstacle final-X progress gap to about 2.91 units, and `tools\vehicle_runtime_qa.py` reports recommendation `promote` for continued opt-in comparison evidence.
- v0.65 adds collision-backed obstacle replay telemetry to the runtime comparison: deterministic and Jolt obstacle replays both clear a QA-only overlap probe with zero overlap frames, while preserving the existing controls, route, camera, and progress checks.
- v0.68 refreshes the Jolt preset after the storm pump scene-content expansion and reruns the evidence stack: Jolt CTest passes 15/15; `tools\physics_parity_qa.py`, `tools\character_contact_qa.py`, `tools\vehicle_physics_qa.py`, and `tools\vehicle_runtime_qa.py` all pass; runtime comparison still reports `maxPositionDelta=1.49`, route completion in 212 frames for Jolt versus 139 for deterministic, zero obstacle overlap frames, and recommendation `promote`.
- v0.96 adds explicit driving-feel checks to the runtime comparison. The report now requires deterministic and Jolt results for route time, route lateral deviation, brake stop distance, reverse distance, steering yaw response, and camera yaw lag before the wrapper accepts a Jolt promote recommendation.
- v0.97 tightens the shared vehicle camera follow and the runtime-QA camera-lag thresholds: deterministic yaw lag is now 4.52 degrees and Jolt yaw lag is 12.24 degrees, while Jolt still completes the service-run checkpoint in 212 frames.
- v0.98 adds route-pace sensitivity probes. Jolt reaches the same checkpoint in 212 frames at throttle 0.72, 0.86, and 1.0, so the route-pace gap is not explained by conservative scripted throttle.
- v0.99 adds a conservative straight-drive assist inside the Jolt vehicle runtime. Jolt now reaches the same checkpoint in 169 frames at the normal 0.72 route throttle, with route-pace probes at 169, 163, and 158 frames for throttle 0.72, 0.86, and 1.0. Deterministic remains the baseline at 139 frames.
- `rg -n "Jolt|JPH::|<Jolt/|JPH/" src\game` returns no matches; Jolt types remain private to engine-owned physics code.

Important limits:

- This is still a vehicle-runtime trial, not broad gameplay collision migration.
- The deterministic `VehicleController` remains the dependency-free direct-app default, QA default, and fallback.
- The Jolt runtime adapter is proven against compact controls checks, a straight service-run route proxy, the same first-job enter-drive-exit-confirm playthrough loop as the deterministic controller, a camera-aware obstacle proxy, and a QA-only collision-backed obstacle replay. v0.70 uses that evidence to make `scripts\play.ps1` request `preferred`, which resolves to Jolt only when the executable supports it.

## v0.37 Live Opt-in Vehicle Runtime Switch Result

Added:

- `--vehicle-runtime jolt`, an explicit live playtest switch for the service-yard vehicle runtime.
- `--vehicle-runtime preferred`, which selects Jolt when the running executable has the backend available and otherwise resolves to deterministic fallback.
- `scripts\play.ps1 -VehicleRuntime jolt`, a wrapper shortcut for manual opt-in runs when pointed at the Jolt-enabled executable.
- `scripts\play.ps1` now defaults to `-VehicleRuntime preferred`, so the default dependency-free executable remains deterministic while a Jolt-enabled executable enters the Jolt live path without a second flag.
- `SandboxLayer` live adapter plumbing that keeps deterministic vehicle gameplay as the default and falls back honestly when the selected runtime backend is unavailable.
- Debug text now reports `vehicleRuntime=deterministic`, `vehicleRuntime=<backend>-live`, or an unavailable/init-failed state.

Validated so far:

- Default dependency-free builds keep `vehicleRuntime=deterministic` unless the explicit switch is requested.
- A default executable accepts `--vehicle-runtime jolt` but reports the selected backend unavailable and keeps deterministic fallback behavior.
- A default executable accepts `--vehicle-runtime preferred` and resolves it to deterministic fallback without an unavailable-backend warning.
- The opt-in `windows-vs2022-debug-jolt` executable initializes the switched path and reports `vehicleRuntime=jolt-live` in debug text.
- `rg -n "Jolt|JPH::|<Jolt/|JPH/" src\game` returns no matches; Jolt types remain private to engine-owned physics code.

Important limits:

- This is a preferred play-wrapper runtime trial, not full default vehicle promotion.
- The switched vehicle uses the v0.36 runtime adapter state for live driving, but player collision, traversal, service-gate behavior, dynamic objects, traffic, damage, audio, and production vehicle tuning remain unchanged.
- The next decision should come from preferred-runtime live evidence plus collision/route replay evidence, with human playtest as useful follow-up rather than the only gate.

## v0.91 Preferred Runtime Evidence Refresh

v0.91 refreshed the opt-in Jolt stack after the Ferry Office Drain Log content/readability changes.

Validated:

- `windows-vs2022-debug-jolt` configured and built.
- Jolt CTest passed 15/15.
- `tools\physics_parity_qa.py` passed with backend `jolt`: floor=4, raycast=4, overlap=4.
- `tools\character_contact_qa.py` passed with backend `jolt`: probes=7.
- `tools\vehicle_physics_qa.py` passed with backend `jolt`: samples=5, recommendation=`promote`.
- `tools\vehicle_runtime_qa.py` passed with backend `jolt`: samples=5, controlChecks=4, routeChecks=2, obstacleChecks=2, `maxPositionDelta=1.49`, `maxYawDeltaDegrees=29.10`, `maxSpeedDelta=2.25`, recommendation=`promote`.
- Jolt playthrough QA completed the 21-event Ferry Office chain with `framesToCheckpoint=212`, no fallback, and no bounds hit.
- Preferred runtime smoke confirmed the Jolt-enabled executable reports `vehicleRuntime=jolt-live`; the dependency-free executable reports `vehicleRuntime=deterministic`.
- `rg -n "Jolt|JPH::|<Jolt/|JPH/" src\game` returned no matches.

Decision:

- Keep Jolt as the preferred production vehicle-runtime candidate and keep the wrapper `preferred` behavior. The evidence still supports Jolt as the path to develop, while default dependency-free gameplay remains deterministic unless the executable supports the preferred backend.

Revisit when:

- Jolt road-edge collision, dynamic object contact, player collision migration, damage/traffic, or a broader authored driving route is added.

## v0.96 Jolt-First Driving Feel Evidence

v0.96 turns the vehicle-feel question into a Jolt-first comparison instead of a deterministic-only road test.

Validated:

- Default `EngineCoreTests.exe` passed.
- Jolt preset configured and built.
- Jolt CTest passed 15/15 after syncing parity/contact wrappers with the current 10-collider Ferry Office scene.
- `tools\vehicle_runtime_qa.py` passed with backend `jolt`: samples=5, controlChecks=4, routeChecks=2, obstacleChecks=2, drivingFeelChecks=12, `maxPositionDelta=1.49`, recommendation=`promote`.
- Deterministic playthrough QA completed the 21-event service-call chain with `framesToCheckpoint=139`.
- Jolt playthrough QA completed the same 21-event chain with `framesToCheckpoint=212`, no fallback, and no bounds hit.
- `rg -n "Jolt|jolt|JPH" src\game\FerryOfficeVehiclePhysicsQa.cpp src\game\FerryOfficeVehiclePhysicsQa.h` returned no matches.

Driving-feel evidence:

- Deterministic baseline: route=139 frames, lateral deviation=0.399m, brake stop=1.056m, reverse=2.795m, steering yaw response=21.73 degrees, camera yaw lag=6.13 degrees.
- Jolt runtime adapter: route=212 frames, lateral deviation=0.399m, brake stop=0.233m, reverse=2.552m, steering yaw response=41.22 degrees, camera yaw lag=16.00 degrees.

Decision:

- Keep Jolt as the preferred production vehicle-runtime candidate. Future vehicle-feel work should tune or promote Jolt first, with deterministic retained as the dependency-free baseline and fallback.

Revisit when:

- Jolt route pace, camera follow, road-edge collision, dynamic object contact, player collision migration, damage/traffic, or a broader authored driving route is added.

## v0.97 Vehicle Camera Follow Tightening

v0.97 improves live vehicle camera readability without changing Jolt route pace or shipping speculative torque/gearing changes.

Validated:

- Default `EngineCoreTests.exe` passed.
- Jolt preset build passed.
- Jolt CTest passed 15/15.
- `tools\vehicle_runtime_qa.py` passed with backend `jolt`: drivingFeelChecks=12, recommendation=`promote`.
- Jolt playthrough QA completed the 21-event service-call chain with `framesToCheckpoint=212`.

Evidence:

- Vehicle camera target-yaw follow strength increased from `3.5` to `5.0` in live play and QA proxy.
- Deterministic `cameraYawLag` improved from 6.13 to 4.52 degrees.
- Jolt `cameraYawLag` improved from 16.00 to 12.24 degrees.
- The QA ceiling is now 20 degrees deterministic and 15 degrees Jolt/runtime adapter.

Decision:

- Keep the tighter vehicle camera follow. Do not treat route pace as solved; tune Jolt pace only with a controlled route probe that preserves reverse contact and obstacle stability.

Revisit when:

- Route-pace tuning changes Jolt mass, engine torque, gearing, throttle shaping, or drivetrain behavior.

## v0.98 Jolt Route-Pace Sensitivity Evidence

v0.98 adds route-pace probes before another drivetrain tuning attempt.

Validated:

- Default `EngineCoreTests.exe` passed.
- Jolt preset build passed.
- Jolt CTest passed 15/15.
- `tools\vehicle_runtime_qa.py` passed with backend `jolt`: routePaceProbes=3, recommendation=`promote`.

Evidence:

- Jolt throttle 0.72: checkpoint in 212 frames, final speed 6.11, no bounds hit.
- Jolt throttle 0.86: checkpoint in 212 frames, final speed 6.12, no bounds hit.
- Jolt throttle 1.0: checkpoint in 212 frames, final speed 6.12, no bounds hit.

Decision:

- Do not treat the 212-frame route pace as a QA-script throttle issue. The next route-pace pass should tune drivetrain/force/gearing/drag behavior while preserving reverse contact and obstacle stability.

Revisit when:

- The Jolt vehicle runtime exposes tunable drivetrain parameters or the service route expands beyond the current straight checkpoint probe.

## v0.99 Controlled Jolt Straight-Drive Assist

v0.99 narrows the preferred Jolt runtime route-pace gap without treating deterministic vehicle feel as the product target.

Validated:

- Default `EngineCoreTests.exe` passed.
- Jolt preset build passed.
- Jolt CTest passed 15/15.
- `tools\vehicle_runtime_qa.py` passed with backend `jolt`: controlChecks=4, routeChecks=2, obstacleChecks=2, drivingFeelChecks=12, routePaceProbes=3, `maxPositionDelta=1.08`, recommendation=`promote`.
- Jolt playthrough QA completed the 21-event service-call chain with `framesToCheckpoint=169`, no fallback, and no bounds hit.

Evidence:

- A stronger assist reduced route time more aggressively but hit bounds in the obstacle replay, so it was rejected.
- The shipped assist applies only to forward throttle with nearly straight steering.
- Jolt route pace improved from 212 to 169 frames at the normal 0.72 throttle script.
- Route-pace probes now complete in 169 frames at throttle 0.72, 163 at 0.86, and 158 at 1.0.
- Jolt still passes route lateral deviation, brake stop, reverse distance, steering yaw response, camera yaw lag, and obstacle replay checks.

Decision:

- Keep Jolt as the production vehicle-runtime candidate and keep deterministic as the dependency-free baseline/fallback. The straight-drive assist is acceptable as a narrow service-route tuning aid, but it is not a final drivetrain or tire model.

Revisit when:

- Vehicle controls/camera feel is tuned, a broader road route is authored, road-edge collision is added, or Jolt becomes the direct-app default runtime.

## Post-v0.99 Jolt Live Evidence Gate

The post-v0.99 gate closes the audit gap where Jolt could pass opt-in QA while the standard Jolt CTest playthrough still used deterministic vehicle runtime.

Validated:

- Default `scripts\verify.ps1` passed.
- Jolt preset configured and built.
- Jolt CTest passed 16/16, including `FerryOfficeJoltPlaythroughQaSmoke`.
- `ctest --preset windows-vs2022-debug-jolt --show-only=json-v1` confirmed that `FerryOfficeJoltPlaythroughQaSmoke` runs `tools\playthrough_qa.py` with `--vehicle-runtime jolt`.
- Explicit Jolt playthrough QA completed the 21-event Ferry Office service-call chain with checkpoint in 169 frames.
- `tools\vehicle_runtime_qa.py` passed with backend `jolt`: samples=5, controlChecks=4, routeChecks=2, obstacleChecks=2, drivingFeelChecks=12, routePaceProbes=3, roadEdgeChecks=2, `maxPositionDelta=1.08`, recommendation=`promote`.
- `scripts\verify_jolt.ps1` now runs the opt-in Jolt configure/build/CTest/playthrough/runtime evidence gate.

Evidence:

- The authored road-edge probes use `dock-road-south-rail` and `dock-road-north-curb` from the Ferry Office scene as the concrete edge ids.
- Deterministic road-edge route: 139 frames, zero edge overlap frames, min clearance about 0.101m.
- Jolt road-edge route: 169 frames, zero edge overlap frames, min clearance about 0.100m.
- The report separates deterministic runtime, Jolt runtime, scripted playthrough, live-like input/control checks, driving-feel checks, route-pace probes, obstacle probes, and road-edge checks.

Decision:

- Keep Jolt as the production vehicle-runtime candidate and keep deterministic as dependency-free baseline/fallback.
- Treat the new gate as stronger gameplay evidence than v0.99, not as default-runtime approval.
- Keep `scripts\verify_jolt.ps1` opt-in so the normal default validation remains dependency-free and quick.

Important limit:

- `roadEdgeChecks` prove clearance and overlap behavior against authored edge volumes mirrored into the QA physics world. They do not yet prove broad vehicle collision response against every scene collider, dynamic object, curb, rail, or off-road failure mode.

Revisit when:

- A milestone adds real road-edge collision response, a broader authored driving route, curved-road camera/input evidence, manual playtest evidence, or default Jolt promotion.

## Actual Authored Road-Edge Collision Response Gate

This gate upgrades the previous road-edge evidence from clearance/overlap probing to runtime collision response in the opt-in vehicle adapter path.

Validated:

- Default `scripts\verify.ps1` passed.
- Jolt preset configured, built, and passed `ctest --preset windows-vs2022-debug-jolt --output-on-failure` with 16/16 tests.
- `scripts\verify_jolt.ps1` passed, including explicit Jolt playthrough QA and vehicle runtime QA.
- `tools\vehicle_runtime_qa.py` passed with backend `jolt`, broadRouteChecks=2, `maxPositionDelta=1.08`, and recommendation `promote`.

Evidence:

- `VehicleRuntimeConfig` now carries static vehicle obstacles, and both the simple adapter and Jolt adapter respond to them.
- Live opt-in vehicle runtime derives those obstacles from the authored Ferry Office `dock-road-south-rail` and `dock-road-north-curb` IDs.
- Deterministic broad-route QA is blocked by `dock-road-north-curb` after reverse: `edgeContactAfterReverseFrames=152`, `reverseDistance=0.366m`, `maxYawChangeDegrees=60.42`, `maxCameraYawDeltaDegrees=5.31`.
- Jolt broad-route QA is blocked by `dock-road-south-rail` after reverse: `edgeContactAfterReverseFrames=32`, `reverseDistance=1.356m`, `maxYawChangeDegrees=43.51`, `maxCameraYawDeltaDegrees=17.92`, `maxEdgePenetration=0.030m`.
- Jolt unconstrained telemetry would leave the corridor (`unconstrainedMinZ=-4.313`), while constrained telemetry stops at the authored runtime edge proxy (`edgeLimitZ=-2.35`, `constrainedMinZ=-2.380`).

Decision:

- Keep Jolt as the preferred production vehicle-runtime candidate.
- Treat the new gate as stronger than the v0.99 and first post-v0.99 evidence because it proves a collision response after straight, reverse, turn, and camera-readability segments.
- Do not promote Jolt to universal default yet.

Important limit:

- The road-edge vehicle obstacles are conservative runtime collision proxies derived from authored visual road-edge IDs. They are not full mesh collision import, final road modeling, dynamic object contact, traffic/damage evidence, or proof that every scene collider is ready for Jolt vehicle collision.

Revisit when:

- Jolt is tested on a longer authored route with multiple turns, human/manual input, camera reset/readability cases, dynamic blockers, or direct-app default promotion.

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

Next recommendation: use the preferred-runtime trial as the default vehicle-candidate direction, but keep broader migration incremental. The v0.91 Jolt path validates enter, drive, checkpoint, exit, service-run confirmation, camera-aware obstacle progress, controls checks, collision-backed obstacle replay telemetry, and preferred-wrapper fallback behavior, but it still has not become broad live vehicle collision, road-edge collision, damage, traffic, or full production vehicle feel.
