# Tidebreak Crime/Action Direction Design

Date: 2026-05-17

## Purpose

Lock the post-v0.99 game direction before more target-slice goals are chosen.

## Design Decision

Tidebreak is a compact third-person narrative vehicle/crime/action sandbox. It is not a service-worker simulator, not a Ferry Office checklist, and not a generic city/gangster clone.

The player is a story protagonist who uses vehicles, movement, space, risk, rule-breaking, and consequence. Tone is location-specific: a port can be heavy and industrial, a resort can be satirical, a service road can be lonely and tense, and a town center can be social and reactive.

## Current Roles

- Ferry Office: regression/debug testbed. Preserve validation value; stop treating it as the production slice.
- Veyra Reach Pilot: target-slice scaffold. Use it to prove future runtime/gameplay contracts outside Ferry Office.

## Next Capability To Prove

The next goal should prove a tiny risky-action plus local-response contract:

1. The player reaches a risky target or conflict.
2. The player performs an action that means more than "press E to set a flag."
3. The runtime records a response state separately from objective completion.
4. The scene shows or reports the response without Ferry Office language.
5. QA can report action, response, exit/recovery, and failure reason.

## Explicit Deferrals

Do not build a full mission system, police/wanted system, NPC AI, traffic, combat suite, save/load, economy, terrain system, editor, broad asset pipeline, renderer rewrite, or Ferry Office content extension as part of the next proof.

## Success Criteria

- `docs/GAME_DIRECTION.md` no longer frames the project primarily as driver/fixer service work.
- `docs/GAMEPLAY_REVIEW.md` points next work toward crime/action response, not another Ferry Office endpoint.
- `docs/WORLD_SLICE_AUTHORING.md` describes the target-slice action/response contract.
- `docs/reviews/tidebreak-game-direction-rebaseline.md` contains the ready next goal prompt.
- `docs/STATUS.md` records this direction-only goal and validation result.
