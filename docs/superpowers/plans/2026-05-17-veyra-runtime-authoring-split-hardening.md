# Veyra Runtime-To-Authoring Split Hardening Plan

Goal: prove the Veyra Reach pilot target-slice runtime/debug path is not still borrowing Ferry Office presentation, vehicle fallback, road-test telemetry, or job/world-state wording.

## Scope

- [x] Confirm baseline build/verify and inspect the current target-slice smoke path.
- [x] Identify remaining Ferry Office assumptions in `SandboxLayer` presentation/debug output.
- [x] Add failing regressions for target-slice debug text and runtime smoke validation.
- [x] Add the smallest scene-role-aware split for neutral target-slice debug text.
- [x] Extend runtime smoke tooling so debug UI mode can be gated explicitly.
- [x] Update `scripts/verify.ps1` and `docs/STATUS.md`.
- [x] Run focused validation and full `scripts/verify.ps1`.

## Non-goals

- No new map/content/terrain/mission/asset/renderer pass.
- No broad `SandboxLayer` extraction.
- No Ferry Office behavior changes except preserving regressions.
