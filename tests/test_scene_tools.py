#!/usr/bin/env python3
"""Tests for Tidebreak scene data tools."""

from __future__ import annotations

import copy
import pathlib
import sys
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import scene_data  # noqa: E402


class SceneToolTests(unittest.TestCase):
    def setUp(self) -> None:
        self.scene = scene_data.load_scene(scene_data.DEFAULT_SCENE_PATH)

    def test_default_scene_validates_without_errors(self) -> None:
        result = scene_data.validate_scene(self.scene)

        self.assertEqual([], result.errors)

    def test_required_ferry_office_ids_exist(self) -> None:
        ids = scene_data.collect_ids(self.scene)

        for required_id in [
            "ferry-office",
            "player-start-dock",
            "service-gate",
            "service-barrier",
            "ferry-manifest",
            "maintenance-box",
            "wall-button",
            "exit-summary-marker",
            "service-barrier-vault",
            "service-yard-vehicle",
        ]:
            self.assertIn(required_id, ids)

    def test_duplicate_ids_are_reported(self) -> None:
        scene = copy.deepcopy(self.scene)
        duplicate = copy.deepcopy(scene["colliders"][0])
        duplicate["center"] = [8.0, 0.5, 8.0]
        scene["colliders"].append(duplicate)

        result = scene_data.validate_scene(scene)

        self.assertTrue(any("Duplicate id" in error for error in result.errors))

    def test_bad_interactable_radius_is_reported(self) -> None:
        scene = copy.deepcopy(self.scene)
        scene["interactables"][0]["radius"] = 0.0

        result = scene_data.validate_scene(scene)

        self.assertTrue(any("radius must be positive" in error for error in result.errors))

    def test_scene_summary_reports_core_counts(self) -> None:
        summary = scene_data.build_summary(self.scene)

        self.assertEqual("ferry-office", summary.scene_id)
        self.assertGreaterEqual(summary.collider_count, 9)
        self.assertEqual(5, summary.interactable_count)
        self.assertEqual(1, summary.traversal_count)
        self.assertEqual(1, summary.vehicle_count)


if __name__ == "__main__":
    unittest.main()
