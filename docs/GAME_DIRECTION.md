# Game Direction

Last updated: 2026-05-17

## Working Title

`Tidebreak`

This is a working title only. It is meant to evoke an isolated coastal region, broken infrastructure, pressure between locals and outside interests, and the idea that player choices leave visible marks.

## Genre

Compact third-person narrative vehicle/crime/action sandbox.

The project should feel like a compact, authored place with systemic reactions rather than a huge open world. It is an original game direction and must not copy names, places, characters, maps, missions, logos, or assets from any commercial game.

## Current Creative North Star

`Tidebreak` is a compact vehicle/crime/action sandbox about a story protagonist moving through coastal places where risk, vehicles, conflict, and world reaction matter.

The project should not be built as a direct "mini GTA" or as a field-work simulator. The useful reference is the player-facing promise of a modern vehicle/crime/action sandbox: a protagonist enters risky situations, uses vehicles and physical space, breaks rules, triggers consequences, and escapes or escalates. The setting can be coastal, provincial, industrial, resort-like, rural, or urban depending on the location. The game does not need a city/gangster tone by default.

The first production truth is still a small authored place, not a huge map. But the reason to build that place is crime/action play, not another sequence of service prompts. Vehicles are a central pillar because they create arrival, pursuit, escape, pressure, collisions, route choice, and identity. Interactions and remembered state matter only when they create visible risk, leverage, consequence, or new action.

## Influence Matrix

These references are design lenses, not cloning targets.

| Element | Best Pattern |
| --- | --- |
| Small dense map | `Bully` / `Yakuza` |
| Physics and collision ambition | `GTA IV` |
| Vehicle/crime/action promise | `GTA IV` / `GTA VI` as high-level ambition lenses |
| Vehicle feel | `Mafia 2` / `GTA IV` |
| World reactions | `Red Dead Redemption 2` / `GTA IV` |
| Humor and chaos | `Saints Row 2` |
| Systemic objects | `Watch Dogs` |
| City/island mood | `Mafia 2` / `Cyberpunk 2077` filtered through Veyra Reach |
| Local missions | `Bully` / `Yakuza` |
| Social memory | `Kingdom Come: Deliverance` / `Red Dead Redemption 2`-lite |

The filter is important: use the references for lessons about density, weight, reactivity, humor, atmosphere, and local structure. Do not copy their worlds, brands, mission designs, characters, tone wholesale, or copyrighted assets.

## Core Fantasy

You are a story protagonist who can turn a place into a situation.

The player arrives in a coastal region where normal authority, private interests, local loyalties, and illicit work overlap. People remember who was helped, threatened, ignored, exposed, chased, damaged, robbed, or used. The fantasy is not about being a worker/operator who completes service tickets. It is about moving through a grounded place as a character with pressure on them, creating action through vehicles, trespass, theft, favors, intimidation, damage, escape, and consequences.

## Fictional World

The first setting is `Veyra Reach`, a fictional island chain and coastal district separated from the mainland by storms, old ferries, damaged roads, and local politics.

The region should be:

- small enough to hand-author,
- grounded and readable,
- original and non-branded,
- built from docks, hillside paths, service roads, weathered public buildings, workshops, homes, old industrial sites, and natural barriers,
- reactive through simple remembered state before it becomes large or complex.

## Player Role

The player is a story protagonist with strong driving skill, local pressure, and room to become dangerous.

They are not a superhero, soldier, celebrity, or chosen-one ruler. They also should not be framed mainly as a maintenance operator. They are a capable outsider or returning local who can drive, move, observe, intimidate, steal, improvise, help, betray, escape, and make visible choices. The exact tone depends on location: a port can be heavy and industrial, a resort can be colorful and satirical, a wet service road can be lonely and tense, and a town center can be social and reactive.

## Core Gameplay Loop

1. Arrive in a place with tension, opportunity, or threat.
2. Read the space: routes, witnesses, hazards, vehicles, access, cover, exits, and valuable targets.
3. Use on-foot movement and vehicle control to enter, trespass, steal, damage, recover, threaten, chase, escape, or improvise.
4. Trigger a world response: attention, alarm, blocked access, pursuit pressure, changed state, or a character/faction consequence.
5. Resolve the situation by leaving, hiding, returning, escalating, or accepting a consequence.
6. See the world remember meaningful actions through visible state and later content, not only text/debug flags.

## Game Pillars

- **Readable third-person feel:** movement, camera, and interaction should be clear before the world grows.
- **Grounded driving as action:** vehicles should feel weighty and useful for arrival, escape, pursuit, route choice, pressure, and identity.
- **Small place, high consequence:** the game should remember meaningful actions in compact locations rather than simulate a huge world badly.
- **Crime/action verbs before service verbs:** stealing, trespassing, escaping, chasing, damaging, recovering, and provoking response are more important than another neutral `E` prompt.
- **Location-specific tone:** Tidebreak can be grim, colorful, satirical, lonely, or tense depending on the place. The tone should come from location and conflict, not one global filter.
- **Veyra-first authorship:** Veyra Reach should become a memorable playable region before any mainland/city expansion is promised.
- **Cinematic restraint:** camera and presentation can become polished, but v0.x work should stay playable and debuggable.
- **Original grounded fiction:** no cloned cities, gangs, missions, characters, brands, logos, or famous maps.
- **AI-friendly iteration:** every system needs clear docs, validation commands, and narrow next steps.

## What This Game Is Not

- Not a Grand Theft Auto clone.
- Not a Mafia clone.
- Not an Uncharted clone.
- Not a Red Dead Redemption clone.
- Not a pure truck simulator.
- Not a service-worker/operator simulator.
- Not a city-first open-world promise.
- Not a generic urban gangster game by default.
- Not a combat-only action game.
- Not an open-world promise in v0.x.
- Not a mission scripting platform yet.
- Not an asset showcase.
- Not a physics-engine experiment.

## Prototype Slice Status

`The Ferry Office` was the first validated prototype slice. It is now a regression/debug testbed, not the production target for the first real game slice.

The Ferry Office proved movement, camera, collision, interaction focus, traversal, remembered state, vehicle runtime evidence, scene data, capture tooling, and QA discipline. It also proved a risk: a long chain of `E` prompts, service tasks, logs, and state flags can look like progress while failing to become a compelling crime/action game.

The next real slice should use the Veyra target-slice surface to prove a small crime/action loop:

1. arrive by vehicle or start near a vehicle-relevant location,
2. identify a risky target or conflict,
3. enter or trespass on foot,
4. trigger a visible response or pressure,
5. use vehicle/space to escape, reposition, or escalate,
6. leave a visible consequence behind.

This does not require full police AI, traffic, combat, dialogue, economy, save/load, final art, or a large map. It does require the loop to feel like a crime/action situation rather than a checklist.

## Engine Priorities

1. Reliable Windows build and validation.
2. Smooth third-person movement and camera.
3. Static world and collision stability.
4. Interaction focus and remembered local state.
5. Debug visualization and manual testability.
6. A small traversal prototype before vehicles.
7. Original placeholder assets and simple scene data.
8. Later promotion from `TestWorld` / `TestScene` to `PrototypeWorld` / `PrototypeScene` when the slice needs it.
