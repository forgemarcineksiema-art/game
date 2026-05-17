# Tidebreak World Foundation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build Veyra Reach as a generated, runtime-visible coastal/industrial world foundation with compiler, preview, capture evidence, and docs.

**Architecture:** `data/worlds/veyra_reach` becomes the source of truth. `tools/world_author.py` compiles that data into `data/scenes/veyra_reach_pilot.scene.json`, validates references, checks drift, and writes an HTML/SVG preview. Runtime remains on the existing scene loader/renderer path; only capture-state support is extended for Veyra perspectives.

**Tech Stack:** Python 3.11 tooling/tests, existing JSON scene schema, C++20 `EngineApp`/`SandboxLayer`, current GDI/DX11/null renderer capture path, existing project-owned `.gltf` assets.

---

### Task 1: World Authoring Tooling

**Files:**
- Create: `tools/world_author.py`
- Create: `tests/test_world_author.py`
- Create: `data/worlds/veyra_reach/world.json`
- Create: `data/worlds/veyra_reach/areas/cinder_harbor.area.json`
- Modify: `CMakeLists.txt`

- [ ] Add failing Python tests that require `world_author.load_world_package`, `compile_scene`, `check_generated_scene`, and `build_preview_html`.
- [ ] Add source data for Veyra world identity, materials, mesh asset catalog, places, terrain patches, road ribbons, edge colliders, landmarks, interactables, routes, and objective markers.
- [ ] Implement compiler output with deterministic sorted JSON and a `generatedFrom` block.
- [ ] Implement `--write-scene`, `--check`, `--preview-html`, `--report-json`.
- [ ] Add the new tests to CTest.

### Task 2: Generated Veyra Runtime Scene

**Files:**
- Modify generated: `data/scenes/veyra_reach_pilot.scene.json`
- Modify: `tools/scene_data.py`
- Modify: `tests/test_scene_tools.py`
- Modify: `tests/EngineCoreTests.cpp`

- [ ] Add failing expectations that Veyra has multiple terrain/road/water/backdrop materials, several colliders, mesh assets/instances, named places/landmarks, multiple route markers, and a generated source marker.
- [ ] Extend known scene color keys for Veyra materials.
- [ ] Generate the runtime scene from world data.
- [ ] Update runtime/text count expectations to match the generated scene while preserving target objective/action-response ids.

### Task 3: Veyra Preview And Drift Proof

**Files:**
- Modify: `tools/world_author.py`
- Modify: `tests/test_world_author.py`
- Generate: `build/world_preview/veyra_reach_preview.html`
- Generate: `build/world_preview/veyra_reach_report.json`

- [ ] Prove preview output includes terrain, roads, collision, landmarks, markers, materials, and bounds.
- [ ] Prove `--check` passes on the generated scene.
- [ ] Prove a copied/stale scene fails drift check when source and generated JSON differ.

### Task 4: Veyra Runtime Capture States

**Files:**
- Modify: `src/engine/core/Config.cpp`
- Modify: `src/game/SandboxLayer.cpp`
- Modify: `tests/EngineCoreTests.cpp`
- Modify: `tools/capture_visual_smoke.py`
- Modify: `tests/test_capture_visual_smoke.py`

- [ ] Add failing tests for Veyra capture-state parsing and `capture_visual_smoke` scenario mapping.
- [ ] Add target-slice-safe Veyra QA capture states: start, road-network, shoreline-horizon, industrial-yard, lookout-landmark, collision-edge.
- [ ] Keep Ferry Office capture states unchanged.

### Task 5: Runtime Proof And Docs

**Files:**
- Modify: `docs/WORLD_SLICE_AUTHORING.md`
- Modify: `docs/SCENE_AUTHORING.md`
- Modify: `docs/ART_DIRECTION.md`
- Modify: `docs/ROADMAP.md`
- Modify: `docs/TECH_DEBT.md`
- Modify: `docs/STATUS.md`
- Generate: `build/captures/veyra_reach_capture_manifest.json`

- [ ] Run scene/world tools for Veyra.
- [ ] Run capture set with GDI and at least one DX11 smoke capture if available through the existing tool.
- [ ] Write capture manifest with command, render path, commit/worktree id, proof target, and result for every screenshot.
- [ ] Update docs with generated-source truth, visible runtime result, placeholder limits, and next step.

### Task 6: Verification, Review, Commit/Push

**Files:**
- All touched files.

- [ ] Run required validation commands listed in the goal.
- [ ] Run code-review checklist against the final diff.
- [ ] Run `scripts/verify.ps1`.
- [ ] If validation passes and the worktree contains only goal-related changes, commit and push to the current branch.
