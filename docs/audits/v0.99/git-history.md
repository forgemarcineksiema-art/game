# v0.99 Git And Docs History Audit

## Git Snapshot

- Branch: `main`.
- Remote tracking: `origin/main`.
- Audited HEAD: `f5e2fdb v0.99 tune jolt route pace`.
- Start-of-work tree: clean before audit artifacts were created.
- Current audit changes: `docs/audits/` and `docs/STATUS.md` update only.

## Development Arc

The commit history is coherent:

1. `v0.1-v0.4`: engine, player/camera, world collision, interaction.
2. `v0.5-v0.8`: traversal, world state, Ferry Office micro-slice, prototype naming cleanup.
3. `v0.9-v0.20`: visual readability, physics decision, vehicles, scene authoring, static mesh/glTF, Blender/fallback asset workflow.
4. `v0.21-v0.38`: playable launch UX, DX11 overlay/capture, visual capture harness, playthrough QA, Jolt parity/contact/vehicle/runtime QA, cleaner playtest presentation.
5. `v0.39-v0.68`: scene presentation/material presets, water/backdrop/road mesh passes, follow-up chain growth, vehicle obstacle/QA improvements, Jolt direction decision.
6. `v0.69-v0.95`: preferred runtime trial, visible state cues, late-chain route guidance, service panel mesh, visual capture states, Low Dock access consequence.
7. `v0.96-v0.99`: Jolt-first driving-feel metrics, vehicle camera tightening, route-pace sensitivity, controlled Jolt straight-drive assist.

## Good Decisions

- Clear `AGENTS.md` and `AI_WORKFLOW.md` rules.
- Default validation stays dependency-free.
- Jolt is behind engine-owned APIs; no direct game-layer vendor leakage found by prior checks and subagent review.
- Scene data became default layout truth before adding too much content.
- Asset provenance is explicit and tools reject unreferenced/unsupported model files.
- The project has honest docs about what is not complete.

## Bad Or Heavy Decisions

- Too many milestone numbers in one day can make progress look broader than it is.
- `DECISIONS.md` is no longer the complete decision ledger; recent Jolt decisions live in `PHYSICS_DECISION.md`, `TECH_DEBT.md`, `ROADMAP.md`, and `STATUS.md`.
- `CONTEXT_MAP.md` includes older validation snapshots that can be mistaken for current state.
- `STATUS.md` is enormous and useful, but it is becoming an archive more than a current operator dashboard.

## Documentation Drift

P2 CONFIRMED: `CONTEXT_MAP.md` has historical counts and older validation snapshots near current guidance. It should either be refreshed after v0.99 or label old snapshots more loudly.

P2 CONFIRMED: `DECISIONS.md` lacks recent decisions for preferred Jolt runtime policy and v0.99 straight-drive assist. This does not lose the decision, but splits the authoritative story.

P2 CONFIRMED: runtime logs still show engine `0.23.0`, weakening artifact traceability.

## Git Hygiene

The repo is on `main` and `origin/main`. Audit artifacts are intentionally new. No unrelated user code changes were present before audit files were created.

Commit/push should happen only if final verification passes and the diff remains limited to audit/status artifacts.
