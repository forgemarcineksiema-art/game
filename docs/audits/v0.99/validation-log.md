# v0.99 Validation Log

All commands were run from `C:\Users\Marcin\Documents\New project` on 2026-05-17 unless noted.

## Required Commands

| Command | Result | Raw log | Notes |
| --- | --- | --- | --- |
| `git status --short` | exit 0 | `raw/01-git-status-short.log` | Shows `?? docs/audits/` because the audit directory had just been created. Direct pre-artifact check was clean. |
| `git branch --show-current` | exit 0 | `raw/02-git-branch-show-current.log` | `main`. |
| `git log --oneline --decorate --date=short --max-count=200` | exit 0 | `raw/03-git-log-200.log` | HEAD `f5e2fdb (HEAD -> main, origin/main) v0.99 tune jolt route pace`. |
| `scripts/doctor.ps1` | exit 0 | `raw/04-doctor.log` | Warned that `cl`, `clang++`, `g++`, `msbuild`, `ninja`, `vcpkg` are not in PATH. |
| `scripts/configure.ps1` | exit 0 | `raw/05-configure.log` | Configured `windows-vs2022-debug`. |
| `scripts/build.ps1` | exit 0 | `raw/06-build.log` | Built `EngineCore`, `GamePrototype`, `EngineApp`, `EngineCoreTests`. |
| `scripts/verify.ps1` | exit 0 | `raw/07-verify.log` | 11/11 default CTest passed; scene/assets/mesh validation passed; null smoke passed. |

## Optional Runtime And Tool Commands

| Command | Result | Raw log | Notes |
| --- | --- | --- | --- |
| `scripts/play.ps1 -Frames 5 -CaptureDir docs/audits/v0.99/screenshots/play-gdi` | exit 0 | `raw/08-play-gdi-frames-capture.log` | GDI bounded playable launch and capture passed. |
| `scripts/play.ps1 -Dx11 -Frames 5 -CaptureDir docs/audits/v0.99/screenshots/play-dx11` | exit 0 | `raw/09-play-dx11-frames-capture.log` | DX11 bounded launch/capture passed after hardware device failed and WARP was used. |
| `scripts/run.ps1 -Args @("--smoke-test", "--frames", "3")` | exit 0 | `raw/10-run-smoke-frames3.log` | Null smoke startup passed. |
| `python tools/scene_report.py` | exit 0 | `raw/11-scene-report.log` | 25 materials, 10 colliders, 20 mesh assets, 66 mesh instances, 17 interactables. |
| `python tools/validate_scene.py` | exit 0 | `raw/12-validate-scene.log` | Passed. |
| `python tools/scale_audit.py` | exit 0 | `raw/13-scale-audit.log` | No suspicious scale issues. |
| `python tools/mesh_report.py` | exit 0 | `raw/14-mesh-report.log` | 20 model files, all referenced. |
| `scripts/play.ps1 -DebugUi -Frames 5 -CaptureDir docs/audits/v0.99/screenshots/play-debug-gdi` | exit 0 | `raw/15-play-debug-gdi-capture.log` | Debug overlay capture passed. |
| `python tools/capture_visual_smoke.py --output-dir ...visual-smoke-initial --report-json ...visual-smoke-initial-report.json` | exit 0 | `raw/16-capture-visual-smoke-initial.log` | GDI/DX11 initial captures passed. |
| `python tools/capture_visual_smoke.py --scenario relay-to-service-log ...` | exit 0 | `raw/17-capture-visual-smoke-relay.log` | GDI/DX11 mid-chain captures passed. |
| `python tools/capture_visual_smoke.py --scenario low-dock-drain-access ...` | exit 0 | `raw/18-capture-visual-smoke-low-dock.log` | GDI/DX11 low-dock captures passed. |
| `python tools/validate_assets.py` | exit 0 | `raw/19-validate-assets.log` | 20 model files, asset validation passed. |
| `python tools/playthrough_qa.py --report-json ...playthrough-deterministic-report.json` | exit 0 | `raw/20-playthrough-qa-deterministic.log` | Deterministic, 21 events, checkpoint in 139 frames. |
| `cmake --preset windows-vs2022-debug-jolt` | exit 0 | `raw/21-configure-jolt.log` | Jolt preset configured. |
| `cmake --build --preset windows-vs2022-debug-jolt` | exit 0 | `raw/22-build-jolt.log` | Jolt preset built. |
| `ctest --preset windows-vs2022-debug-jolt --output-on-failure` | exit 0 | `raw/23-ctest-jolt.log` | 15/15 tests passed. |
| `python tools/physics_parity_qa.py --report-json ...physics-parity-report.json` | exit 0 | `raw/24-physics-parity-qa.log` | Jolt floor/raycast/overlap parity passed. |
| `python tools/character_contact_qa.py --report-json ...character-contact-report.json` | exit 0 | `raw/25-character-contact-qa.log` | Jolt character contact probes passed. |
| `python tools/vehicle_physics_qa.py --report-json ...vehicle-physics-report.json` | exit 0 | `raw/26-vehicle-physics-qa.log` | Jolt vehicle feasibility passed, recommendation `promote`. |
| `python tools/vehicle_runtime_qa.py --report-json ...vehicle-runtime-report.json` | exit 0 | `raw/27-vehicle-runtime-qa.log` | Jolt runtime comparison passed, route 169 vs deterministic 139. |
| `python tools/playthrough_qa.py --exe build/windows-vs2022-debug-jolt/Debug/EngineApp.exe --vehicle-runtime jolt --report-json ...playthrough-jolt-report.json` | exit 0 | `raw/28-playthrough-qa-jolt.log` | Jolt 21-event playthrough passed, checkpoint in 169 frames. |
| `ctest --preset windows-vs2022-debug-jolt --show-only=json-v1` | exit 0 | `raw/29-ctest-jolt-show-only.log` | Confirms `FerryOfficePlaythroughQaSmoke` lacks `--vehicle-runtime jolt`. |

## Notable Metrics

- Default `scripts/verify.ps1`: 11/11 CTest passed.
- Jolt `ctest`: 15/15 passed.
- Deterministic playthrough: `events=21`, `framesToCheckpoint=139`, no bounds hit.
- Jolt playthrough: `events=21`, `framesToCheckpoint=169`, no fallback, no bounds hit.
- Vehicle runtime QA: `controlChecks=4`, `routeChecks=2`, `obstacleChecks=2`, `drivingFeelChecks=12`, `routePaceProbes=3`, `maxPositionDelta=1.08`, recommendation `promote`.
- Jolt route pace probes: throttle `0.72 -> 169`, `0.86 -> 163`, `1.0 -> 158` frames.
- Initial visual smoke: GDI `82` unique colors, DX11 `49`; both 1280x720. DX11 used WARP on this machine.

## Screenshot Manifest

| Path | Command/source | Render path | Commit | Intended proof | Actual proof |
| --- | --- | --- | --- | --- | --- |
| `screenshots/play-gdi/capture-gdi-fallback.bmp` | `scripts/play.ps1 -Frames 5 -CaptureDir ...play-gdi` | GDI playtest | `f5e2fdb` | Start app and typical first playable frame | GDI wrapper path launches and captures first frame. |
| `screenshots/play-dx11/capture-dx11.bmp` | `scripts/play.ps1 -Dx11 -Frames 5 -CaptureDir ...play-dx11` | DX11 via WARP | `f5e2fdb` | DX11 bounded start | DX11 capture works, but hardware device failed and WARP was used. |
| `screenshots/play-debug-gdi/capture-gdi-fallback.bmp` | `scripts/play.ps1 -DebugUi -Frames 5 -CaptureDir ...play-debug-gdi` | GDI debug | `f5e2fdb` | Debug overlay truth state | Shows full flag/telemetry overlay and debug-marker clutter. |
| `screenshots/visual-smoke-initial/v0.31-gdi-capture.bmp` | `capture_visual_smoke.py`, initial | GDI | `f5e2fdb` | Initial gameplay view | Nonblank, readable overlay, Ferry Office frame visible. |
| `screenshots/visual-smoke-initial/v0.31-dx11-capture.bmp` | `capture_visual_smoke.py`, initial | DX11 via WARP | `f5e2fdb` | DX11 parity initial view | Nonblank DX11/WARP capture, not hardware proof. |
| `screenshots/visual-smoke-relay/v0.94-relay-to-service-log-gdi-capture.bmp` | `capture_visual_smoke.py --scenario relay-to-service-log` | GDI | `f5e2fdb` | Mid-chain route guidance | Overlay and in-world mid-chain scene render; route readability still overlay-heavy. |
| `screenshots/visual-smoke-relay/v0.94-relay-to-service-log-dx11-capture.bmp` | same | DX11 via WARP | `f5e2fdb` | DX11 mid-chain parity | Nonblank DX11/WARP mid-chain capture. |
| `screenshots/visual-smoke-low-dock/v0.95-low-dock-drain-access-gdi-capture.bmp` | `capture_visual_smoke.py --scenario low-dock-drain-access` | GDI | `f5e2fdb` | Visible low-dock state | Renders late-chain low-dock state; objective still text-led. |
| `screenshots/visual-smoke-low-dock/v0.95-low-dock-drain-access-dx11-capture.bmp` | same | DX11 via WARP | `f5e2fdb` | DX11 low-dock parity | Nonblank DX11/WARP late-chain capture. |

## Failures / Blockers

No required validation command failed.

One exploratory `rg` command with PowerShell wildcard paths failed with OS error 123 because `src\game\WorldState.*` style arguments were treated as invalid literal paths. It was not a validation command, did not affect the audit result, and the relevant search was replaced by broader `rg` inspections over `src/game`.

## Final Completion Checks

| Command | Result | Raw log | Notes |
| --- | --- | --- | --- |
| `scripts/verify.ps1` | exit 0 | `raw/30-final-verify.log` | Fresh final gate after audit/status docs were written; 11/11 default CTest passed. |
| `git diff --check` | exit 0 | `raw/31-git-diff-check.log` | Passed with only Git's CRLF normalization warning for `docs/STATUS.md`. |

## Interpretation

Validation is green for the checked commands. The important caveat is scope: default verification does not equal visual quality, full player navigation, or default Jolt promotion.
