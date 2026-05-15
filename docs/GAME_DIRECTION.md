# Game Direction

Last updated: 2026-05-15

## Working Title

`Tidebreak`

This is a working title only. It is meant to evoke an isolated coastal region, broken infrastructure, pressure between locals and outside interests, and the idea that player choices leave visible marks.

## Genre

Small third-person cinematic systemic sandbox/adventure.

The project should feel like a compact, authored place with systemic reactions rather than a huge open world. It is an original game direction and must not copy names, places, characters, maps, missions, logos, or assets from any commercial game.

## Current Creative North Star

`Tidebreak` is a third-person island driver/fixer sandbox where every job leaves a mark on a small coastal community.

The project should not be built as a direct "mini GTA". The stronger direction is island-first: start with a compact, dense, memorable region where driving, movement, local jobs, route access, repairs, favors, and remembered consequences matter. A larger city or mainland district can exist later as an expansion of the fiction, but the first production truth is Veyra Reach as a small, authored island place.

The long-term player fantasy is practical rather than superheroic: drive, fix, deliver, unlock, improvise, help, annoy, break, repair, and become known. Vehicles are an important long-term pillar, but they should serve place, jobs, access, and local memory rather than turn the game into a pure vehicle simulator.

## Influence Matrix

These references are design lenses, not cloning targets.

| Element | Best Pattern |
| --- | --- |
| Small dense map | `Bully` / `Yakuza` |
| Physics and collision ambition | `GTA IV` |
| Vehicle feel | `Mafia 2` / `GTA IV` |
| World reactions | `Red Dead Redemption 2` / `GTA IV` |
| Humor and chaos | `Saints Row 2` |
| Systemic objects | `Watch Dogs` |
| City/island mood | `Mafia 2` / `Cyberpunk 2077` filtered through Veyra Reach |
| Local missions | `Bully` / `Yakuza` |
| Social memory | `Kingdom Come: Deliverance` / `Red Dead Redemption 2`-lite |

The filter is important: use the references for lessons about density, weight, reactivity, humor, atmosphere, and local structure. Do not copy their worlds, brands, mission designs, characters, tone wholesale, or copyrighted assets.

## Core Fantasy

You are the outsider who can get things unstuck.

The player arrives in a cut-off island region where normal authority has become unreliable. People remember who helped, who was ignored, what was damaged, what was delivered, and what was repaired. The fantasy is not power through combat scale; it is power through movement, driving, access, tools, favors, and consequences.

## Fictional World

The first setting is `Veyra Reach`, a fictional island chain and coastal district separated from the mainland by storms, old ferries, damaged roads, and local politics.

The region should be:

- small enough to hand-author,
- grounded and readable,
- original and non-branded,
- built from docks, hillside paths, service roads, weathered public buildings, workshops, homes, old industrial sites, and natural barriers,
- reactive through simple remembered state before it becomes large or complex.

## Player Role

The player is an outsider driver/fixer.

They are not a superhero, soldier, crime boss, celebrity, or chosen-one ruler. They are a capable visitor with practical skills: driving, movement, observation, basic repair, negotiation, carrying small objects, unlocking access, and making visible choices.

## Core Gameplay Loop

1. Arrive at a small local problem.
2. Observe the space, people, routes, locked paths, hazards, and useful objects.
3. Move or drive through the environment using third-person traversal and grounded vehicle control.
4. Interact with objects, switches, doors, notes, tools, and people.
5. Change a local state: open access, repair a path, deliver an item, move a vehicle/object, mark a choice, or trigger a reaction.
6. See the world remember that change through debug state first, then visible scene changes later.
7. Return to a hub or nearby character with a new route, consequence, or lead.

## Game Pillars

- **Readable third-person feel:** movement, camera, and interaction should be clear before the world grows.
- **Grounded driving with purpose:** vehicles should feel weighty and useful for jobs, access, and identity, not like generic arcade props.
- **Small place, high memory:** the game should remember meaningful actions in a compact region rather than simulate a huge world badly.
- **Practical systemic play:** doors, routes, tools, hazards, and local state are more important than combat escalation.
- **Island-first authorship:** Veyra Reach should become a memorable place before any mainland/city expansion is promised.
- **Cinematic restraint:** camera and presentation can become polished, but v0.x work should stay playable and debuggable.
- **Original grounded fiction:** no cloned cities, gangs, missions, characters, brands, logos, or famous maps.
- **AI-friendly iteration:** every system needs clear docs, validation commands, and narrow next steps.

## What This Game Is Not

- Not a Grand Theft Auto clone.
- Not a Mafia clone.
- Not an Uncharted clone.
- Not a Red Dead Redemption clone.
- Not a pure truck simulator.
- Not a city-first open-world promise.
- Not a combat-first action game.
- Not an open-world promise in v0.x.
- Not a mission scripting platform yet.
- Not an asset showcase.
- Not a physics-engine experiment.

## First 10-Minute Vertical Slice

The first playable slice should be `The Ferry Office`.

The player starts near a damaged ferry office at the edge of Veyra Reach. A blocked service gate, a loose walkway, a locked office door, and a small local request create a compact loop:

1. Walk from the dock to the ferry office.
2. Learn that the office power is out and the side path is blocked.
3. Traverse a simple obstacle or narrow route to reach a maintenance box.
4. Interact with the maintenance box to restore local power.
5. Return through a changed route.
6. Pick up a small manifest or marker item.
7. Use an interaction to open the office or trigger a remembered state.
8. End with the world state visibly changed: power restored, route opened, item collected, and a log/state entry recorded.

The first slice should fit inside one small scene and prove feel, clarity, and remembered actions. It should not require vehicles, NPC AI, combat, inventory, save/load, or final art.

## Engine Priorities

1. Reliable Windows build and validation.
2. Smooth third-person movement and camera.
3. Static world and collision stability.
4. Interaction focus and remembered local state.
5. Debug visualization and manual testability.
6. A small traversal prototype before vehicles.
7. Original placeholder assets and simple scene data.
8. Later promotion from `TestWorld` / `TestScene` to `PrototypeWorld` / `PrototypeScene` when the slice needs it.
