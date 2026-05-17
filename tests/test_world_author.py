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

    def test_cinder_harbor_world_art_pass_compiles_primary_mesh_replacements(self) -> None:
        package = world_author.load_world_package(self.world_root)

        scene = world_author.compile_scene(package)
        assets = {asset["id"]: asset for asset in scene["meshAssets"]}
        replacements = {
            instance["replacesVisualPlaceholderId"]: instance
            for instance in scene["meshInstances"]
            if "replacesVisualPlaceholderId" in instance
        }

        for asset_id, path in {
            "cinder-harbor-ground-patch-mesh": "assets/models/cinder_harbor_ground_patch.gltf",
            "cinder-harbor-road-plate-mesh": "assets/models/cinder_harbor_road_plate.gltf",
            "cinder-harbor-shore-shelf-mesh": "assets/models/cinder_harbor_shore_shelf.gltf",
        }.items():
            self.assertIn(asset_id, assets)
            self.assertEqual(path, assets[asset_id]["path"])
            self.assertIn("project-original", assets[asset_id]["license"])

        for placeholder_id in [
            "veyra-hillside-ground-west",
            "veyra-hillside-ground-east",
            "saltbreak-layby-concrete",
            "old-pump-cut-yard",
            "harbor-rock-shore",
            "greywinch-service-road-surface-1",
            "greywinch-service-road-surface-2",
            "old-pump-cut-access-surface-2",
        ]:
            self.assertIn(placeholder_id, replacements)

    def test_cinder_harbor_world_art_pass_is_declared_in_area_source_and_report(self) -> None:
        package = world_author.load_world_package(self.world_root)
        area = package.areas[0]

        scene = world_author.compile_scene(package)
        report = world_author.build_report(package, scene)
        preview = world_author.build_preview_html(package, scene)

        self.assertEqual("cinder-harbor-first-world-art-mesh-material-pass", area["worldArtPass"]["id"])
        self.assertGreaterEqual(report["counts"]["worldArtReplacementMeshes"], 10)
        self.assertGreaterEqual(report["counts"]["primaryWorldArtAssets"], 3)
        self.assertIn("World-Art Meshes", preview)

    def test_cinder_harbor_readability_pass_compiles_landmark_risk_and_route_anchors(self) -> None:
        package = world_author.load_world_package(self.world_root)
        area = package.areas[0]

        scene = world_author.compile_scene(package)
        assets = {asset["id"]: asset for asset in scene["meshAssets"]}
        instances = {instance["id"]: instance for instance in scene["meshInstances"]}
        report = world_author.build_report(package, scene)
        preview = world_author.build_preview_html(package, scene)

        self.assertEqual("cinder-harbor-landmark-risk-readability-pass", area["readabilityPass"]["id"])
        self.assertEqual(
            {
                "harbor-scar-overlook",
                "reach-relay-hut",
                "suspicious-cargo-cache",
                "route-anchors",
            },
            set(area["readabilityPass"]["proofTargets"]),
        )
        for asset_id, path in {
            "cinder-harbor-overlook-mast-mesh": "assets/models/cinder_harbor_overlook_mast.gltf",
            "cinder-harbor-relay-tower-mesh": "assets/models/cinder_harbor_relay_tower.gltf",
            "cinder-harbor-cargo-tarp-mesh": "assets/models/cinder_harbor_cargo_tarp.gltf",
            "cinder-harbor-route-beacon-mesh": "assets/models/cinder_harbor_route_beacon.gltf",
        }.items():
            self.assertIn(asset_id, assets)
            self.assertEqual(path, assets[asset_id]["path"])
            self.assertIn("project-original", assets[asset_id]["license"])

        expected_roles = {
            "mesh-harbor-scar-overlook-mast": "landmark",
            "mesh-reach-relay-tower": "landmark",
            "mesh-suspicious-cargo-tarp": "risk-site",
            "mesh-low-beacon-route-anchor": "route-anchor",
            "mesh-greywinch-route-anchor": "route-anchor",
        }
        for instance_id, role in expected_roles.items():
            self.assertEqual(role, instances[instance_id]["readabilityRole"])

        self.assertGreaterEqual(report["counts"]["readabilityLandmarks"], 2)
        self.assertGreaterEqual(report["counts"]["readabilityRiskSites"], 1)
        self.assertGreaterEqual(report["counts"]["readabilityRouteAnchors"], 2)
        self.assertIn("Readability Anchors", preview)

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
