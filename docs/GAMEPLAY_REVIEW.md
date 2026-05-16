# Gameplay Review

Last updated: 2026-05-16

Purpose: keep future Codex runs honest about Tidebreak as a game, not only as a validated engine workbench. Read this before choosing a new playable-content, presentation, movement, camera, vehicle, or world-state milestone.

## Current Read

Tidebreak now has a real prototype loop, not just isolated systems. The Ferry Office scene has a validated 21-event chain, one service vehicle route, active route guidance, remembered local flags, visible state cues, original placeholder props, GDI/DX11 capture evidence, and after v0.95 one repaired state that changes traversable space by opening Low Dock Drain access.

The strongest playable identity is:

- arrive at a compact coastal workplace,
- understand a blocked or broken local route,
- move and drive through the space,
- fix or deliver something practical,
- see the local place remember the change.

The weakest current identity is also clear: too many recent beats are administrative sign-offs. They are technically validated, but several have the same player verb: walk to marker, press `E`, set one flag, read text. This risks turning the slice into a checklist instead of a compact driver/fixer sandbox.

## What Is Working

- The scene is coherent enough to support a short authored chain.
- The service vehicle route is the best kinetic beat because it changes pace and requires movement through space.
- The world-state ledger is useful and validated.
- Active playtest route guidance makes the long chain less dependent on F1/debug mode.
- Visual smoke and playthrough QA give enough automated evidence to make provisional game decisions without waiting for manual review.
- Jolt vehicle evidence is strong enough to treat Jolt as the preferred production vehicle-runtime candidate, while deterministic remains the safe direct-app and QA baseline/fallback.

## Main Gameplay Problems

1. The chain is long, but many beats are shallow.

   A 21-event report proves coverage, not fun. Future content should add a new verb, spatial problem, route change, delivery constraint, driving challenge, or visible consequence. Do not add another log/sign/tag endpoint unless it unlocks one of those.

2. World consequences are still rare.

   v0.95 proves the right shape by opening Low Dock Drain access after the Storm Pump Ticket. Do not stop there. Future content should keep favoring physical route changes, blocked/unblocked spaces, moved objects, new access, or changed work zones over tiny color cues and text-only state.

3. The UI still carries too much design weight.

   The top-left overlay explains objectives, prompt, job state, follow-up state, and next step. That is useful, but the scene should carry more of the player's understanding through composition, landmarks, markers, state cues, and readable route geometry.

4. Driving is underused relative to the project identity.

   The vehicle route is validated and important, but most follow-up content returns to on-foot marker interactions. A stronger Tidebreak loop should periodically ask the player to drive with purpose: reposition, tow later, deliver, clear, reroute, or choose a route.

5. Architecture is close to its next pressure point.

   `SandboxLayer.cpp`, `FerryOfficeJob.cpp`, and `EngineCoreTests.cpp` are still workable, but the next content expansion should be careful. If a milestone adds more chain complexity, it should also reduce hardcoded route/objective policy or create a tighter content boundary.

## Milestone Selection Rules

Use these rules after orientation and before choosing the next milestone.

### Prefer Game Leverage

Prefer the milestone that improves one of these:

- a new player verb,
- stronger driving or movement feel,
- clearer objective understanding without more text,
- a physical world-state consequence,
- a more believable authored place,
- a safer path to the next compact job.

### Treat Validation As Evidence, Not The Game

Green validation means the repo can move. It does not mean the next safest improvement is the best game improvement. Ask what the player would notice without reading `docs/STATUS.md`.

### Avoid Checklist Growth

Do not add multiple consecutive beats that only set flags through `E` prompts. A new interaction should usually change space, route, vehicle use, visible state, or future choice.

### Do Not Pick Prop Polish By Default

A small mesh or visual cue is correct only when:

- the current objective is visually ambiguous,
- a capture proves the missing cue is in the player's view,
- it supports an active route, consequence, or interaction,
- and it does not delay a stronger movement/driving/world-state milestone.

### Use Architecture To Unlock Play

Refactor when it unblocks content, reduces risk in `SandboxLayer`, creates a reusable content boundary, or prevents route/objective duplication. Do not refactor only because a file is large.

## Strong Next Goal Shapes

### 1. Physical World Consequence

Best near-term shape: after an existing follow-up beat such as storm pump, low dock drain, or relay service work, make the world visibly and spatially change.

Examples:

- open a short service path,
- remove a small road obstruction,
- expose a new turnaround or dock-side shortcut,
- change a work zone from blocked to passable,
- move or lower a barrier after repair.

Validation:

- C++ tests for state-to-collider/route/presentation behavior,
- scene validation/report/scale audit,
- playthrough QA proves the state chain,
- visual smoke captures before/after or a named QA capture state.

### 2. Driving Feel Road-Test

Best if the next question is vehicle quality. Build an automated road-test report before tuning. For vehicle runtime decisions, make this Jolt-first: deterministic is the control/fallback, not the only feel target.

Metrics:

- time to checkpoint,
- steering response,
- stop distance,
- reverse behavior,
- yaw overshoot,
- bounds hits,
- route deviation,
- camera target stability.

Validation:

- deterministic baseline report,
- required Jolt/runtime-adapter comparison when the question is production vehicle direction,
- playthrough QA,
- focused C++ tests,
- `scripts\verify.ps1`.

### 3. Objective/Route Readability Without More Text

Best if captures show the player reaches a state but the destination is visually weak.

Improve:

- in-world active destination cue,
- route-side landmark,
- destination framing,
- prompt priority near clustered interactables,
- marker scale/color policy.

Avoid:

- another top-left line,
- another generic sign-off prop,
- visual detail that does not help the current objective.

### 4. Content Boundary For Second Job

Best if adding new content would otherwise extend `FerryOfficeJob` with another long hardcoded chain.

Improve:

- scene-authored action/route/objective binding shape,
- smaller helper for follow-up chains,
- tests that prove route/objective policy from data,
- migration of one existing follow-up segment, not a giant mission framework.

## Next Recommended Direction

Do not treat another Relay Service Log prop as the default next move. It is defensible only if a capture shows that specific destination is unreadable.

After v0.96, the better vehicle direction is:

```text
Jolt Live-Driving Promotion/Tuning Pass
```

Use the v0.96 driving-feel evidence as the baseline: Jolt passed route, reverse, brake, steering, camera, obstacle, and playthrough checks, but remains slower than deterministic on the authored checkpoint route. Tune or promote Jolt deliberately; do not spend the next vehicle pass polishing deterministic in isolation unless the fallback itself regresses.

If capture evidence shows the new Low Dock Drain access consequence is hard to read, the strongest alternate is:

```text
v0.96 Low Dock Access Readability Pass
```

Keep that alternate bounded to in-world readability and validation; do not add another administrative endpoint.

## Review Questions Before Any Next Goal

- Does the player get a new verb, a stronger feel, or a more visible consequence?
- Would the change still matter if the debug overlay were hidden?
- Is this improving Tidebreak, or only making the existing checklist longer?
- Does the milestone rely on another `E` prompt and flag without spatial change?
- Is the current weakness actually in content, driving, movement/camera, presentation, validation, or architecture?
- What automated evidence will substitute for a human playtest?
- Does the work reduce or increase pressure on `SandboxLayer` and `FerryOfficeJob`?
- What should a player remember from this milestone?
