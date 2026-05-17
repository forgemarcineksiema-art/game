# Tidebreak World Foundation Design

Date: 2026-05-17
Goal: make `Veyra Reach` the first authored, runtime-visible world foundation for Tidebreak.

## Decision

Veyra Reach becomes the first real target location, not another Ferry Office-sized QA scaffold. The runtime scene at `data/scenes/veyra_reach_pilot.scene.json` is no longer the hand-authored source of truth. It is generated from world/area authoring data under `data/worlds/veyra_reach`.

`Veyra Reach` for this milestone is `Cinder Harbor Reach`: a compact coastal/industrial hillside approach with a harbor edge, service-yard target site, connected road network, narrow shoreline edge, lookout/high point, utility landmarks, and an escape route. It is still low-detail original placeholder art, but it must read as a place: terrain, roads, water/shore, horizon/backdrop, infrastructure, risk target, and route choices.

## Stop Doing

- Stop treating Veyra as a marker/objective smoke scene.
- Stop hand-editing the generated runtime scene as the primary authoring surface.
- Stop using Ferry Office as the place where real game-world identity grows.
- Stop accepting single-route pads, lone colliders, and marker clusters as "world" proof.

## Systems To Move

- World source data: `data/worlds/veyra_reach/world.json` plus area files.
- Compiler/check tooling: `tools/world_author.py`.
- Preview tooling: an HTML/SVG top-down report generated from world data.
- Scene validation: drift checks prove generated runtime JSON matches source.
- Runtime capture: Veyra-specific capture states prove several readable perspectives.
- Docs/status: scene/world docs must say what is generated, what is placeholder, and what is runtime-visible.

## Source Of Truth

World source owns:

- world/area identity,
- named places,
- terrain patches and material keys,
- road ribbons, shoulders, route graph, and authored road-edge collision intent,
- shoreline/water/horizon/backdrop cues,
- landmarks and visible world-art anchors,
- interactables/objectives used by the existing target-slice action-response contract.

Generated runtime scene owns only the engine-facing representation:

- `sceneMaterials`,
- `colliders`,
- `visualPlaceholders`,
- `meshAssets`,
- `meshInstances`,
- `interactables`,
- `routeMarkers`,
- `objectiveMarkers`,
- `targetObjective`,
- `targetActionResponse`.

## Runtime Result

The first runtime Veyra view should show an overcast sky/background, ground and material variation, a connected road network, harbor water/shore edge, industrial yard and utility structures, lookout/horizon/backdrop silhouettes, and readable road-edge collision cues. The player should be able to answer: where am I, where can I drive, where can I walk, what is risky, and what is a landmark?

## Placeholder Honesty

Still placeholder:

- low-poly/fallback meshes,
- flat authored material colors,
- AABB collision,
- static water/backdrop,
- no traffic/NPC/police/combat/audio,
- no terrain heightfield, spline road mesh, PBR, textures, or streaming.

Not acceptable as placeholder:

- a blank void,
- one pad and one marker,
- debug-only route proof,
- hand-authored generated scene drift,
- Veyra looking like a QA playground.

## Evidence Standard

Important claims in status/final handoff use:

- CONFIRMED: fresh command, test, run, capture, or commit evidence.
- INFERRED: conclusion from inspected code/data/docs.
- UNVERIFIED: not proved in this work session.

## Skill Gate Note

The requested `superpowers:brainstorming` and `superpowers:writing-plans` skills normally require user approval before implementation. This `/goal` explicitly grants autonomous design-to-plan-to-implementation authority. I used those skills as design and planning checklists, not as a blocking approval gate.
