# First Real Slice Direction Gate Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Decide and record the next Tidebreak implementation direction after Jolt vehicle feel is accepted, without extending Ferry Office as if it were the target game.

**Architecture:** This is a docs/status direction gate, not a runtime feature. It updates the documents that future autonomous runs consult before picking work: a new decision review, `GAMEPLAY_REVIEW.md`, `TECH_DEBT.md`, `CONTEXT_MAP.md`, and `STATUS.md`.

**Tech Stack:** Markdown project docs, existing `tools/status_report.py`, default `scripts/verify.ps1`, Git commit/push workflow.

---

### Task 1: Record The Direction Decision

**Files:**
- Create: `docs/reviews/first-real-slice-direction-gate.md`

- [ ] **Step 1: Write the decision document**

Create `docs/reviews/first-real-slice-direction-gate.md` with these exact sections:

```markdown
# First Real Slice Direction Gate

Date: 2026-05-17

## Verdict

## Evidence Read

## What Is No Longer The Active Blocker

## Candidate Matrix

## Recommended Next Implementation Goal

## Rejected Directions

## Acceptance Criteria For The Next Goal

## Prompt For The Next Goal
```

The verdict must say that Ferry Office remains a regression/debug testbed and that the next implementation should be a world/slice architecture scaffold for Veyra Reach, not another Ferry Office content/polish/world-response pass.

- [ ] **Step 2: Verify the document names evidence**

Run:

```powershell
rg -n "CONFIRMED|INFERRED|WARNING|BET|Ferry Office|Veyra Reach|SandboxLayer|scene JSON" docs\reviews\first-real-slice-direction-gate.md
```

Expected: the command prints evidence-tagged lines and mentions the current pressure points.

### Task 2: Update Milestone Selection Docs

**Files:**
- Modify: `docs/GAMEPLAY_REVIEW.md`
- Modify: `docs/TECH_DEBT.md`
- Modify: `docs/CONTEXT_MAP.md`

- [ ] **Step 1: Replace stale next-direction text in `GAMEPLAY_REVIEW.md`**

Change the "Next Recommended Direction" section so it no longer points at the old Jolt live-driving pass. It must point at the first-real-slice/world-architecture scaffold, and must explicitly reject another Ferry Office endpoint, terrain-only pass, and generic visual polish.

- [ ] **Step 2: Update `TECH_DEBT.md` current triage**

In `Current Priority Triage`, record that current manual vehicle input/physics/camera is accepted for the existing playtest path. Keep remaining Jolt risks scoped to all-world driving, non-flat surfaces, and dynamic blockers. Move the current high-value debt to scene/world architecture and first-real-slice scaffolding.

- [ ] **Step 3: Update `CONTEXT_MAP.md` post-v0.99 snapshot**

Add a short post-v0.99 note that Ferry Office is now treated as a regression testbed and that the next autonomous direction is a world/slice architecture scaffold before new content.

### Task 3: Update Status And Validate

**Files:**
- Modify: `docs/STATUS.md`

- [ ] **Step 1: Add a status entry**

Add a top status entry titled:

```markdown
## First Real Slice Direction Gate (2026-05-17)
```

It must include scope, evidence, chosen next implementation goal, rejected alternatives, validation commands, and commit/push status.

- [ ] **Step 2: Run docs validation**

Run:

```powershell
python tools\status_report.py
git diff --check
```

Expected: both pass. CRLF warnings are acceptable if `git diff --check` exits 0.

- [ ] **Step 3: Run full repository validation**

Run:

```powershell
scripts\verify.ps1
```

Expected: passes. If it fails, record the exact blocker in `docs/STATUS.md` and do not commit/push.

- [ ] **Step 4: Commit and push if scoped**

Run:

```powershell
git status --short --branch
git add docs\reviews\first-real-slice-direction-gate.md docs\superpowers\plans\2026-05-17-first-real-slice-direction-gate.md docs\GAMEPLAY_REVIEW.md docs\TECH_DEBT.md docs\CONTEXT_MAP.md docs\STATUS.md
git commit -m "Choose first real slice direction"
git push origin main
```

Expected: commit and push succeed only if the worktree contains only this goal's docs.
