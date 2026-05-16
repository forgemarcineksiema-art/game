# v0.99 Audit Checkpoint

## Checked

- Required docs from `AGENTS.md` and user prompt.
- `docs/AI_WORKFLOW.md`, `GAME_DIRECTION.md`, `VERTICAL_SLICE.md`, `TECH_DEBT.md`, `ROADMAP.md`.
- Physics context: `PHYSICS_DECISION.md`.
- Scene/assets/render context: `SCENE_AUTHORING.md`, `ASSET_GUIDE.md`, `ART_DIRECTION.md`, `MESH_RENDERING.md`.
- Additional orientation docs: `CONTEXT_MAP.md`, `STATUS.md`, `ARCHITECTURE.md`, `DECISIONS.md`, `GAMEPLAY_REVIEW.md`.
- Git status, branch, 200-commit log.
- Default configure/build/verify.
- GDI and DX11 bounded play wrapper captures.
- Scene, asset, mesh, scale tools.
- Visual smoke initial, relay-to-service-log, low-dock-drain-access.
- Deterministic and explicit Jolt playthrough QA.
- Jolt configure/build/CTest.
- Jolt physics parity, character contact, vehicle feasibility, vehicle runtime QA.
- Five subagent domain reviews integrated.

## Unchecked Or Only Partially Checked

- No manual human playtest feel judgement.
- No long free-play session.
- No hardware DX11 proof; local DX11 fell back to WARP.
- No resize/resource lifetime stress test.
- No full live keyboard/mouse navigation through the whole chain.
- No audio, save/load, NPC, dialogue, combat, inventory, mission framework, packaging because those systems do not exist.
- No online dependency freshness check in this audit; local repo was the source of truth.

## Blockers

No P0 blocker.

Known audit caveats:

- Default `verify` is green but does not cover all stronger evidence.
- Jolt CTest playthrough smoke name is misleading unless an explicit Jolt playthrough is also run.
- Visual smoke passed but remains heuristic.

## How To Continue

1. Read `docs/audits/v0.99/README.md`.
2. Use `validation-log.md` raw logs if a finding needs evidence.
3. Start the next goal from `next-steps.md`: Jolt-first live controls/camera and road-edge evidence gate.
4. Before claiming future work complete, run:
   - `scripts/verify.ps1`
   - explicit Jolt playthrough with `--vehicle-runtime jolt`
   - `python tools/vehicle_runtime_qa.py`
   - relevant visual smoke scenario
5. Do not use this audit as a license to add content until live input and collision evidence improve.
