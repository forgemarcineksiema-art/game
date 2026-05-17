"""Tests for Veyra Reach world-authoring compiler and preview tooling."""

from __future__ import annotations

import copy
import json
import pathlib
import sys
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
TOOLS = ROOT / "tools"
sys.path.insert(0, str(TOOLS))

import world_author  # noqa: E402


class WorldAuthorTests(unittest.TestCase):
    def setUp(self) -> None:
        self.world_root = ROOT / "data" / "worlds" / "veyra_reach"
        self.scene_path = ROOT / "data" / "scenes" / "veyra_reach_pilot.scene.json"

    def test_veyra_world_package_compiles_runtime_scene(self) -> None:
        package = world_author.load_world_package(self.world_root)

        scene = world_author.compile_scene(package)

        self.assertEqual("veyra-reach-pilot", scene["id"])
        self.assertEqual("data/worlds/veyra_reach/world.json", scene["generatedFrom"]["world"])
        self.assertEqual(["data/worlds/veyra_reach/areas/cinder_harbor.area.json"], scene["generatedFrom"]["areas"])
        self.assertEqual("target-slice-scaffold", scene["sliceMetadata"]["kind"])
        self.assertGreaterEqual(len(scene["visualPlaceholders"]), 18)
        self.assertGreaterEqual(len(scene["meshInstances"]), 18)
        self.assertGreaterEqual(len(scene["colliders"]), 7)
        self.assertGreaterEqual(len(scene["routeMarkers"]), 4)
        self.assertGreaterEqual(len(scene["objectiveMarkers"]), 8)

    def test_compiler_preserves_target_slice_action_contract(self) -> None:
        scene = world_author.compile_scene(world_author.load_world_package(self.world_root))

        self.assertEqual("inspect-cinder-cache-marker", scene["targetObjective"]["id"])
        self.assertEqual("Suspicious Cargo Cache", scene["targetActionResponse"]["riskyInteractableName"])
        self.assertEqual("Harbor Scar Escape Marker", scene["targetActionResponse"]["exitInteractableName"])
        self.assertIn("Cinder Harbor", scene["targetObjective"]["objectiveText"])

    def test_world_references_are_validated_before_scene_generation(self) -> None:
        package = world_author.load_world_package(self.world_root)
        broken_area = copy.deepcopy(package.areas[0])
        broken_area["roads"][0]["materialKey"] = "missing-road-material"
        broken = world_author.WorldPackage(root=package.root, world=package.world, areas=[broken_area])

        with self.assertRaisesRegex(world_author.WorldAuthorError, "missing-road-material"):
            world_author.compile_scene(broken)

    def test_generated_scene_drift_check_detects_stale_scene(self) -> None:
        package = world_author.load_world_package(self.world_root)
        scene = world_author.compile_scene(package)
        stale_scene = copy.deepcopy(scene)
        stale_scene["visualPlaceholders"][0]["center"][0] += 1.0

        with tempfile.TemporaryDirectory() as temp_dir:
            scene_path = pathlib.Path(temp_dir) / "veyra.scene.json"
            scene_path.write_text(json.dumps(stale_scene, indent=2), encoding="utf-8")

            result = world_author.check_generated_scene(package, scene_path)

        self.assertFalse(result.ok)
        self.assertTrue(result.messages)
        self.assertIn("generated scene drift", result.messages[0])

    def test_current_generated_scene_matches_world_source(self) -> None:
        package = world_author.load_world_package(self.world_root)

        result = world_author.check_generated_scene(package, self.scene_path)

        self.assertTrue(result.ok, result.messages)

    def test_preview_html_contains_world_layers_and_named_places(self) -> None:
        package = world_author.load_world_package(self.world_root)
        scene = world_author.compile_scene(package)

        html = world_author.build_preview_html(package, scene)

        self.assertIn("<svg", html)
        self.assertIn("Terrain", html)
        self.assertIn("Roads", html)
        self.assertIn("Collision", html)
        self.assertIn("Landmarks", html)
        self.assertIn("Cinder Harbor Reach", html)
        self.assertIn("Stormwall Bend", html)
        self.assertIn("Harbor Scar Overlook", html)


if __name__ == "__main__":
    unittest.main()
