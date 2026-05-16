# AI Workflow

Future Codex runs should use this workflow before changing the repository.

## 1. Start With Orientation

Run:

```powershell
git status --short --branch
python tools/status_report.py
```

Read:

- `AGENTS.md`
- `docs/CONTEXT_MAP.md`
- `docs/STATUS.md`
- `docs/ROADMAP.md`
- `docs/ARCHITECTURE.md`
- `docs/DECISIONS.md`
- `docs/GAME_DIRECTION.md`
- `docs/TECH_DEBT.md`

For physics, vehicles, player collision, or world-query work, also read:

- `docs/PHYSICS_DECISION.md`

For scene layout, object placement, scale, asset, model, or art-direction work, also read:

- `docs/SCENE_AUTHORING.md`
- `docs/ASSET_GUIDE.md`
- `docs/ART_DIRECTION.md`
- `docs/MESH_RENDERING.md`

For scene work, run:

```powershell
python tools/scene_report.py
python tools/validate_scene.py
python tools/scale_audit.py
python tools/mesh_report.py
```

## 2. Goal-Based Delivery

Use Codex `/goal` as the normal work packet for both large and small repository changes. The goal is not ceremony; it is the unit of leadership, validation, commit, push, and handoff.

Every goal starts with:

- **Outcome:** one concrete engine/game/tooling result.
- **Why now:** how this moves Tidebreak toward target systems or a shippable vertical slice.
- **Scope:** the smallest complete set of changes that proves the outcome.
- **Non-goals:** what will not be touched in this goal, especially polish that does not change the capability being proved.
- **Validation:** exact commands that will count as evidence.
- **Stop rule:** stop when the outcome is implemented, validated, documented in `docs/STATUS.md`, committed/pushed when allowed by `AGENTS.md`, and the next-goal prompt is written.

Goal size should be "just right":

- Use a small goal for a focused fix, validation gap, tool improvement, or one slice of a larger system.
- Use a powerful goal when it creates a durable capability boundary, such as a renderer path, physics migration seam, scene authoring contract, runtime QA mode, animation foundation, combat foundation, vegetation pipeline, or asset/runtime system.
- Do not split tightly coupled work just to make it look small.
- Do not bundle unrelated work just because the repo is open.

Avoid endless prototype loops:

- Do not spend a goal on debug readability, marker polish, minor visual cleanup, or wording unless that is blocking validation or decision-making.
- Prefer goals that retire prototype paths, promote proven systems toward target architecture, or answer a major production risk.
- If a polish idea appears during a systems goal, record it as a future candidate and keep moving.
- If the same prototype area receives repeated small goals, the next goal should decide whether to promote, replace, or stop investing in it.

Use subagents deliberately:

- Main Codex owns the goal, scope, final decisions, validation, commit, push, and next prompt.
- Use worker subagents for bounded research, implementation slices, or module-specific exploration.
- Use reviewer subagents for scope creep checks, spec compliance, code quality, architecture boundaries, and risk review.
- Do not let subagents expand the goal. Their output must be bounded recommendations or bounded patches.

Recommended subagent pattern:

```text
Main Codex defines the goal and stop rule.
Worker subagent handles one bounded task.
Reviewer subagent checks the result against scope, tests, and architecture.
Main Codex integrates, validates, updates STATUS, commits, pushes, and writes the next prompt.
```

At the end of every completed goal:

1. Update code/docs/scripts intended by the goal.
2. Update `docs/STATUS.md` with scope, changes, commands, results, blockers, and next-goal prompt.
3. Run `scripts/verify.ps1`. Add domain validation when relevant, such as capture QA, playthrough QA, or Jolt QA.
4. If validation passes and no unrelated user changes block the work, commit and push to the current remote/branch.
5. Mark the Codex goal complete only after the work is genuinely complete.
6. Final response says what passed, what failed, what remains, commit/push status, and the proposed next goal prompt.

Next-goal prompt template:

```text
Create a Codex goal for Tidebreak.

Repository rules:
- Follow AGENTS.md and docs/AI_WORKFLOW.md.
- Use docs/CONTEXT_MAP.md for orientation.
- Keep the goal focused on a durable engine/game/tooling capability.
- Avoid polish unless it blocks validation or a production decision.
- Update docs/STATUS.md.
- Run scripts/verify.ps1 before claiming success.
- Commit and push only if validation passes and the worktree has no unrelated user changes.

Goal:
[One concrete outcome.]

Why now:
[Why this goal advances Tidebreak beyond prototype churn.]

Scope:
- [Must do]
- [Must do]
- [Must do]

Non-goals:
- [Explicitly excluded]
- [Explicitly excluded]

Files/docs to read first:
- [Relevant docs and source areas]

Validation:
- scripts/verify.ps1
- [Domain-specific commands]

Use subagents:
- Worker: [bounded task]
- Reviewer: [bounded review]
```

## 3. Pick One Milestone

Choose one goal that advances a real capability. Small is fine when it unlocks confidence. Powerful is preferred when it has clear boundaries, strong validation, and does not mix unrelated systems.

## 4. Inspect Before Editing

Use fast searches:

```powershell
rg --files
rg "RelevantSymbol" src tests docs
```

Check existing patterns before inventing new ones.

## 5. Validate Incrementally

For behavior changes, add or update tests first when practical. Keep smoke mode working:

```powershell
scripts/verify.ps1
```

If full verification cannot run, run the strongest available subset and document the blocker.

## 6. Update Status

Every meaningful task should update `docs/STATUS.md` with:

- goal,
- scope and non-goals,
- what was inspected,
- what changed,
- commands run,
- results,
- known issues,
- commit/push status,
- next-goal prompt.

## 7. Avoid Scope Creep

Do not add systems just because the project direction is ambitious. Add them when the current goal can prove a useful slice with validation and a clear path toward production quality.

## 8. Leave Useful Notes

When stopping, leave the next AI run with:

- current build/test status,
- known blockers,
- recommended next command or validation,
- a ready-to-use prompt for the next goal.
