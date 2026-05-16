# v0.99 Next Steps

## One Recommended Milestone

Recommended milestone: **Jolt-first live controls/camera and road-edge evidence gate**.

Goal:

Prove that the preferred Jolt vehicle runtime is not only passing route metrics, but is controllable and readable in a short live-input scenario with authored scene collision/road-edge evidence.

Why this next:

- v0.99 narrowed Jolt route pace from 212 to 169 frames, but route pace is not the same as driving feel.
- The project direction says driving/fixer gameplay is a pillar.
- Current playthrough QA is scripted and current vehicle collision is not full authored-world collision.
- Adding another content beat now would lengthen the checklist instead of strengthening the player verb.

## Scope

- Add a short live-input or `SandboxLayer`-driven QA scenario covering on-foot approach, vehicle enter, drive, brake/reverse, camera follow/reset behavior, exit.
- Add collision-backed vehicle replay against authored scene/road-edge geometry, not only synthetic obstacle/bounds probes.
- Add explicit Jolt playthrough CTest with `--vehicle-runtime jolt`.
- Decide whether to add `scripts/verify_gameplay.ps1` or `scripts/verify_jolt.ps1` as an opt-in stronger gate.
- Keep deterministic as baseline/fallback.

## Non-Goals

- No new job endpoint.
- No mission framework/editor.
- No broad map expansion.
- No traffic, NPCs, damage, garage, economy, save/load, audio, combat.
- No default Jolt promotion unless the new evidence explicitly supports it.
- No renderer rewrite, DX12/Vulkan, PBR/material pipeline, GLB importer.
- No player/world/traversal migration to Jolt in the same milestone.

## Ordered Work

P0:

- None found in this audit.

P1:

1. Add explicit Jolt playthrough CTest with `--vehicle-runtime jolt`.
2. Add a live-input gameplay QA probe for short on-foot + vehicle route.
3. Add authored road-edge/collider vehicle evidence.
4. Define visual/gameplay/Jolt verification gates separately from default `verify`.
5. Fix or document GDI shaded-triangle ordering before treating GDI screenshots as depth-order authority.

P2:

1. Resolve 190 vs 240 route-budget naming.
2. Update runtime version or stop using version as milestone marker.
3. Refresh `CONTEXT_MAP.md` snapshot or label older snapshots.
4. Add latest Jolt runtime policy to `DECISIONS.md`.
5. Consider `.last_preset` support or clearer messaging in `scripts/play.ps1`.

## Alternatives

Alternative A: visual objective readability pass.

Use only if capture review shows the current destination is unreadable without overlay. Keep it focused on in-world route comprehension, not generic props.

Alternative B: content-boundary cleanup.

Use only if the next planned content would extend `FerryOfficeJob` again. Migrate one small follow-up policy seam; do not build a full mission editor.

Alternative C: GDI renderer ordering fix.

Use if visual overlap artifacts block reliable screenshots. This is valid, but should not become a renderer rewrite.

## Do Not Do Next

- Do not add another log/sign/tag beat.
- Do not polish deterministic vehicle in isolation.
- Do not make Jolt default just because the current reports say `promote`.
- Do not expand map/content before live input and collision evidence improve.
- Do not convert the explicit job helper into a giant mission framework.
