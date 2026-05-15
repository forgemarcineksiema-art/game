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

For scene work, run:

```powershell
python tools/scene_report.py
python tools/validate_scene.py
python tools/scale_audit.py
```

## 2. Pick One Safe Milestone

Choose one narrow roadmap step. Do not mix milestones unless the user explicitly asks. Prefer finishing build stability before adding gameplay.

## 3. Inspect Before Editing

Use fast searches:

```powershell
rg --files
rg "RelevantSymbol" src tests docs
```

Check existing patterns before inventing new ones.

## 4. Validate Incrementally

For behavior changes, add or update tests first when practical. Keep smoke mode working:

```powershell
scripts/verify.ps1
```

If full verification cannot run, run the strongest available subset and document the blocker.

## 5. Update Status

Every meaningful task should update `docs/STATUS.md` with:

- what was inspected,
- what changed,
- commands run,
- results,
- known issues,
- honest next steps.

## 6. Avoid Scope Creep

Do not add advanced systems just because the project direction is ambitious. v0.x milestones should prove one small engine capability at a time.

## 7. Leave Useful Notes

When stopping, leave the next AI run with:

- current build/test status,
- known blockers,
- recommended next command,
- recommended next goal.
