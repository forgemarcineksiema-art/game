# Gameplay Review

Last updated: 2026-05-17

Purpose: keep future Codex runs honest about Tidebreak as a game, not only as a validated engine workbench. Read this before choosing a new playable-content, presentation, movement, camera, vehicle, or world-state milestone.

## Current Read

Tidebreak now has a strong validated prototype loop, but its game identity has been re-baselined. The Ferry Office scene has a validated 21-event chain, one service vehicle route, active route guidance, remembered local flags, visible state cues, original placeholder props, GDI/DX11 capture evidence, Jolt vehicle evidence, and one repaired state that changes traversable space. That is useful regression evidence, not the target player fantasy.

The strongest playable identity is:

- arrive in a compact authored location with tension,
- read routes, witnesses, hazards, vehicles, exits, and valuable targets,
- use on-foot movement and grounded driving to create action,
- trespass, steal, recover, damage, chase, escape, threaten, or improvise,
- trigger visible world response and remembered consequence.

The weakest current identity is also clear: too many recent beats are administrative sign-offs. They are technically validated, but several have the same player verb: walk to marker, press `E`, set one flag, read text. This risks turning Tidebreak into a field-work/checklist simulator instead of a narrative vehicle/crime/action sandbox.

The game does not need to be urban or gangster by default. Tidebreak can move between heavy industrial ports, lonely service roads, resort corruption, small-town pressure, rural routes, and coastal illegal work. Tone should depend on location. The shared rule is that the player is a story protagonist in risky situations, not an operator completing neutral tasks.

## What Is Working

- Ferry Office is coherent enough to remain a regression/debug testbed.
- The vehicle route is the best kinetic beat because it changes pace and requires movement through space.
- The world-state ledger is useful and validated.
- Active playtest route guidance makes the long chain less dependent on F1/debug mode.
- Visual smoke and playthrough QA give enough automated evidence to make provisional game decisions without waiting for manual review.
- Jolt vehicle evidence is strong enough to treat Jolt as the preferred production vehicle-runtime candidate, while deterministic remains the safe direct-app and QA baseline/fallback.
- Veyra now has Cinder Harbor Reach as the first authored runtime world foundation: source world data, generated scene, connected roads, terrain/shore/backdrop cues, landmarks, target objective, risky cargo response, preview/report, and capture proof separate from Ferry Office assumptions.

## Main Gameplay Problems

1. The chain is long, but many beats are shallow.

   A 21-event report proves coverage, not fun. Future content should add a new crime/action verb, spatial problem, chase/escape pressure, route choice, valuable target, visible consequence, or world response. Do not add another log/sign/tag endpoint unless it directly supports action.

2. The current world responds like a prototype, not like a crime/action sandbox.

   v0.95 proves the right shape by opening Low Dock Drain access after the Storm Pump Ticket, and Veyra now proves authored contact plus a risky-action/local-response/exit-recovery state contract inside a real authored location. The next step is not another neutral repair consequence. It should make Cinder Harbor's risk/response visible, spatial, or pressure-bearing instead of leaving it as report/state text.

3. The UI still carries too much design weight.

   The top-left overlay explains objectives, prompt, job state, follow-up state, and next step. That is useful, but the scene should carry more of the player's understanding through composition, landmarks, markers, state cues, and readable route geometry.

4. Driving must become action, not only route completion.

   The vehicle route is validated and manually accepted, but it is still mostly QA and traversal. A stronger Tidebreak loop should ask the player to arrive, block, escape, chase, flee, reposition, ram later, switch routes, or recover from a bad situation.

5. Architecture is close to its next pressure point.

   `SandboxLayer.cpp`, `FerryOfficeJob.cpp`, and `EngineCoreTests.cpp` are still workable, but the next content expansion should be careful. If a milestone adds more chain complexity, it should also reduce hardcoded route/objective policy or create a tighter content boundary.

## Milestone Selection Rules

Use these rules after orientation and before choosing the next milestone.

### Prefer Game Leverage

Prefer the milestone that improves one of these:

- a new crime/action player verb,
- stronger driving or movement feel,
- clearer objective understanding without more text,
- a physical or social world response,
- a more believable authored place,
- a safer path to the first compact crime/action slice.

For Veyra specifically, preserve `data/worlds/veyra_reach` as the source of truth. Do not regress to hand-patching `data/scenes/veyra_reach_pilot.scene.json` as the design surface.

### Treat Validation As Evidence, Not The Game

Green validation means the repo can move. It does not mean the next safest improvement is the best game improvement. Ask what the player would notice without reading `docs/STATUS.md`.

### Avoid Checklist Growth

Do not add multiple consecutive beats that only set flags through `E` prompts. A new interaction should usually create risk, change space, involve a vehicle, reveal a target, trigger response, or alter future choice.

### Do Not Pick Prop Polish By Default

A small mesh or visual cue is correct only when:

- the current objective is visually ambiguous,
- a capture proves the missing cue is in the player's view,
- it supports an active route, consequence, or interaction,
- and it does not delay a stronger movement/driving/world-state milestone.

### Use Architecture To Unlock Play

Refactor when it unblocks content, reduces risk in `SandboxLayer`, creates a reusable content boundary, or prevents route/objective duplication. Do not refactor only because a file is large.

## Strong Next Goal Shapes

### 1. First Crime/Action Loop Contract

Best near-term shape: define and prove the smallest gameplay contract for a Tidebreak crime/action situation outside Ferry Office.

Examples:

- a valuable target that can be taken, damaged, or recovered,
- an access/trespass line that makes the player action risky,
- a visible response state after the action,
- an escape/reposition requirement that gives the vehicle a purpose,
- a failure/pressure state that is observable in QA.

Validation:

- C++ tests for objective/action/response state,
- scene validation/report/scale audit,
- a recorded-input QA route through action and response,
- visual/runtime smoke showing response state without Ferry Office language.

### 2. Veyra Local Response Gate

Best if the next question is how the world answers the player. Use the existing Veyra scaffold to prove one local response after a risky action without building a mission framework.

Good response shapes:

- a blocked/changed route,
- a warning/attention state,
- a simple pursuit/pressure proxy,
- a locked/unlocked escape route,
- a visible marker/prop state that changes because of the risky action.

Validation:

- recorded-input target-slice QA,
- target-slice runtime smoke,
- scene tools,
- `scripts\verify.ps1`.

### 3. Vehicle-As-Action Gate

Best if the next slice needs the vehicle to be more than transport. Use Jolt as the preferred production candidate and deterministic as control/fallback.

Metrics:

- arrival to a risky location,
- enter/exit under an objective state,
- escape route progress,
- turn/reverse/reset readability,
- collision/contact response,
- bounds and failure reason.

Validation:

- recorded-input route report,
- vehicle runtime QA when vehicle behavior changes,
- target-slice smoke/capture if Veyra is used,
- `scripts\verify.ps1`.

### 4. Slice Architecture Boundary

Best if adding the crime/action loop would otherwise push Veyra-specific behavior into `SandboxLayer` or create a second hardcoded `FerryOfficeJob`.

Improve:

- scene-authored objective/action/response binding shape,
- a small target-slice action/response runtime helper,
- tests that prove route/objective policy from data,
- no ECS, editor, mission graph, traffic, police AI, or broad scripting system.

## Next Recommended Direction

Do not treat another Ferry Office endpoint, prop, route cue, low-dock reaction, or terrain/readability pass as the default next move. Ferry Office is now a regression/debug testbed, not the first real game location.

After the post-v0.99 Jolt input-semantics fix, manual play confirmation, and Veyra target-slice runtime/contact gates, the better direction is:

```text
Veyra First Crime/Action Loop Contract
```

Use the Ferry Office evidence as the regression baseline, not the content target. The next goal should define and prove the smallest target-slice gameplay contract for risky action plus response: not a full mission, not police AI, not traffic, not terrain, not visual polish. It should make Veyra prove a player-facing crime/action loop shape instead of another neutral objective marker.

The strongest alternate is a narrower architecture cleanup only if the action/response contract would otherwise force location-specific policy into `SandboxLayer`, `FerryOfficeJob`, or one growing scene JSON.

```text
Target-Slice Action/Response Runtime Boundary
```

Keep that alternate bounded to one action/response contract. Do not start an ECS, mission editor, broad content rewrite, generic engine rewrite, terrain pass, or another Ferry Office polish loop.

## Review Questions Before Any Next Goal

- Does the player get a crime/action verb, stronger feel, or a more visible consequence?
- Would the change still matter if the debug overlay were hidden?
- Is this improving Tidebreak, or only making the existing checklist longer?
- Does the milestone rely on another `E` prompt and flag without risk, response, spatial change, or vehicle meaning?
- Is the current weakness actually in content, driving, movement/camera, presentation, validation, or architecture?
- What automated evidence will substitute for a human playtest?
- Does the work reduce or increase pressure on `SandboxLayer` and `FerryOfficeJob`?
- What should a player remember from this milestone as an action scene?
