# v0.99 Game Review

## Player-Facing Verdict

Tidebreak now presents as a small low-poly third-person prototype with a coherent coastal service place. It has a player proxy, Ferry Office, dock, water, service vehicle, signs/props, active route guidance, and a long remembered-state chain. It can be played enough to understand the intended fantasy.

It can also disappoint because the game still depends heavily on overlay text, debug markers, scripted state, and one authored route. The strongest game moment is driving the service vehicle to a checkpoint. The weakest game pattern is repeated "walk to marker, press E, set flag, read overlay".

## What Feels Real

- The Ferry Office has identity: dock, facade, gate, service side, road, water, props.
- The route starts with a clear objective and visible prompt.
- The service vehicle is a real runtime beat in both deterministic and Jolt reports.
- The world remembers flags and can show late-chain state cues.
- The Low Dock access consequence is the right kind of direction: state changes traversable/readable space.

## What Can Disappoint

P1 CONFIRMED: The 21-event chain is not the same as a 10-minute player experience. It is a validated authored checklist. Evidence: Jolt and deterministic playthrough reports pass, but `FerryOfficePlaythroughQa.cpp` triggers many interactions directly and only scripts the vehicle segment.

P1 CONFIRMED: UI carries too much of the game. Screenshots show the top-left overlay tells objective, prompt, job state, follow-up state, and next step. The world helps, but does not yet carry enough understanding without text.

P1 CONFIRMED: Driving is the most kinetic verb but still under-proven as live feel. Vehicle runtime QA has useful metrics, but the route is compact and proxy-driven. There is no traffic, road-edge collision, broader route, damage, cargo, audio, or route choice.

P2 CONFIRMED: Player feel is baseline. Movement has no acceleration/deceleration feel target; camera has no obstruction; traversal is one deterministic vault-like affordance. That is okay for engine foundation, not enough for a game-feel claim.

P2 INFERRED: The current content arc is too administrative. Relay log, clearance tag, parts shelf, work board, handoff note, ticket, drain log all validate state, but many share the same player verb.

## Screenshots Reviewed

- Initial GDI playtest: readable Ferry Office and starting objective, but overlay is the main guide.
- Debug GDI: truthful workbench state with many flags, colliders, routes, and debug primitives.
- Relay-to-service-log: proves mid-chain route state, but the "why/where" is text-first.
- Low-dock-drain-access: proves late-chain capture state, but the frame can still read as a marked endpoint rather than a natural spatial task.

## Strengths As A Game

- Compact place-first direction is correct.
- The project is not pretending to be a city-scale open world.
- The vehicle route gives the slice a pace change.
- Remembered state is visible enough to be extended into real consequences.
- The no-copyright/original placeholder discipline is healthy.

## Weaknesses As A Game

- Too many interactions are administrative flag commits.
- No NPCs, social feedback, dialogue, inventory, save/load, audio, or systemic world response.
- No human-verified feel loop in this audit.
- No strong in-world objective reading without overlay.
- One vehicle route cannot yet carry the driver/fixer fantasy.

## Answer: Will Launch Disappoint?

Yes, if the expectation is a playable mini-adventure. No, if the expectation is a serious engine/game prototype with a visible first place and strong automated evidence.

The player-facing truth: Tidebreak v0.99 is promising because it has a coherent place and one useful vehicle job. It is fragile because the "game" is still mostly an authored, overlay-led validation chain.
