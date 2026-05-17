"""Unit tests for neutral runtime scene smoke validation."""

from __future__ import annotations

import pathlib
import sys
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
TOOLS = ROOT / "tools"
sys.path.insert(0, str(TOOLS))

import runtime_scene_smoke  # noqa: E402


class RuntimeSceneSmokeTests(unittest.TestCase):
    def test_target_slice_output_rejects_ferry_office_overlay_leak(self) -> None:
        output = "\n".join(
            [
                "[info] Loaded runtime scene data: veyra-reach-pilot from data/scenes/veyra_reach_pilot.scene.json",
                "Objective: Check the Ferry Manifest to start the Ferry Office service call.",
                "Job: Collect manifest | in progress",
            ]
        )

        result = runtime_scene_smoke.validate_runtime_output(
            output,
            expected_scene_id="veyra-reach-pilot",
            expected_kind="target-slice-scaffold",
            forbidden_terms=runtime_scene_smoke.DEFAULT_FORBIDDEN_TARGET_SLICE_TERMS,
        )

        self.assertFalse(result.passed)
        self.assertTrue(any("Ferry Office" in failure for failure in result.failures))
        self.assertTrue(any("Job:" in failure for failure in result.failures))

    def test_target_slice_output_rejects_ferry_office_debug_telemetry_leak(self) -> None:
        output = "\n".join(
            [
                "[info] Loaded runtime scene data: veyra-reach-pilot from data/scenes/veyra_reach_pilot.scene.json",
                "scene=veyra-reach-pilot loaded=yes roadSegment=dock-road roadBounds=(-9.50,-7.50)-(9.50,8.50)",
                "worldState={powerRestored=false manifestCollected=false serviceRunConfirmed=false}",
            ]
        )

        result = runtime_scene_smoke.validate_runtime_output(
            output,
            expected_scene_id="veyra-reach-pilot",
            expected_kind="target-slice-scaffold",
            forbidden_terms=runtime_scene_smoke.DEFAULT_FORBIDDEN_TARGET_SLICE_TERMS,
        )

        self.assertFalse(result.passed)
        self.assertTrue(any("roadSegment=dock-road" in failure for failure in result.failures))
        self.assertTrue(any("worldState={" in failure for failure in result.failures))

    def test_target_slice_output_passes_with_neutral_overlay(self) -> None:
        output = "\n".join(
            [
                "[info] Loaded runtime scene data: veyra-reach-pilot from data/scenes/veyra_reach_pilot.scene.json",
                "Scene: Veyra Reach Pilot Slice | role=target-slice-scaffold",
                "Objective: Inspect neutral slice markers; no authored job is active.",
                "Status: colliders=1 | interactables=1 | routes=1 | markers=2 | vehicle=none",
            ]
        )

        result = runtime_scene_smoke.validate_runtime_output(
            output,
            expected_scene_id="veyra-reach-pilot",
            expected_kind="target-slice-scaffold",
            forbidden_terms=runtime_scene_smoke.DEFAULT_FORBIDDEN_TARGET_SLICE_TERMS,
        )

        self.assertTrue(result.passed, result.failures)
        self.assertEqual([], result.failures)

    def test_expected_scene_id_is_required(self) -> None:
        output = "Loaded runtime scene data: ferry-office from data/scenes/ferry_office.scene.json"

        result = runtime_scene_smoke.validate_runtime_output(
            output,
            expected_scene_id="veyra-reach-pilot",
            expected_kind="target-slice-scaffold",
            forbidden_terms=[],
        )

        self.assertFalse(result.passed)
        self.assertIn("expected scene id 'veyra-reach-pilot' was not observed", result.failures)


if __name__ == "__main__":
    unittest.main()
