#!/usr/bin/env python3
"""Tests for Tidebreak scene data tools."""

from __future__ import annotations

import copy
import json
import pathlib
import sys
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import scene_data  # noqa: E402
import mesh_report  # noqa: E402
import check_blender  # noqa: E402
import create_simple_prop_gltf  # noqa: E402


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
            "service-run-confirm-marker",
            "service-barrier-vault",
            "service-yard-vehicle",
            "service-run-checkpoint-marker",
            "dock-road-relay",
            "relay-service-log",
            "dock-road-clearance-tag",
            "dock-road-clearance-status-tag",
            "mesh-dock-road-clearance-tag",
            "harbor-parts-crate",
            "ferry-office-parts-shelf",
            "ferry-office-work-board",
            "route-service-confirm-to-relay",
            "route-relay-to-service-log",
            "route-service-log-to-clearance-tag",
            "route-clearance-tag-to-harbor-parts",
            "route-harbor-parts-to-office-shelf",
            "route-parts-shelf-to-work-board",
            "dock-road-relay-marker",
            "relay-service-log-marker",
            "dock-road-clearance-marker",
            "harbor-parts-marker",
            "ferry-office-parts-shelf-marker",
            "ferry-office-work-board-marker",
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

    def test_unknown_world_flag_is_reported(self) -> None:
        scene = copy.deepcopy(self.scene)
        scene["interactables"][0]["worldFlagsSet"] = ["manifestCollected", "typoFlag"]

        result = scene_data.validate_scene(scene)

        self.assertTrue(any("typoFlag" in error for error in result.errors))

    def test_unknown_traversal_completion_flag_is_reported(self) -> None:
        scene = copy.deepcopy(self.scene)
        scene["traversalAffordances"][0]["worldFlagsSetOnComplete"] = ["missingTraversalFlag"]

        result = scene_data.validate_scene(scene)

        self.assertTrue(any("missingTraversalFlag" in error for error in result.errors))

    def test_scene_summary_reports_core_counts(self) -> None:
        summary = scene_data.build_summary(self.scene)

        self.assertEqual("ferry-office", summary.scene_id)
        self.assertEqual(21, summary.material_count)
        self.assertGreaterEqual(summary.collider_count, 9)
        self.assertEqual(27, summary.visual_count)
        self.assertEqual(13, summary.interactable_count)
        self.assertEqual(1, summary.traversal_count)
        self.assertEqual(1, summary.vehicle_count)
        self.assertEqual(13, summary.route_count)
        self.assertEqual(12, summary.objective_marker_count)

    def test_scene_summary_reports_mesh_assets_and_instances(self) -> None:
        summary = scene_data.build_summary(self.scene)

        self.assertGreaterEqual(summary.mesh_asset_count, 1)
        self.assertGreaterEqual(summary.mesh_instance_count, 9)

    def test_scene_materials_cover_authored_color_keys(self) -> None:
        material_keys = {material["key"] for material in self.scene["sceneMaterials"]}
        used_keys = {item["colorKey"] for item in self.scene["visualPlaceholders"]}
        used_keys.update(item["colorKey"] for item in self.scene["meshInstances"])

        self.assertEqual(set(), used_keys - material_keys)

    def test_missing_scene_material_for_used_color_key_is_reported(self) -> None:
        scene = copy.deepcopy(self.scene)
        missing_key = scene["visualPlaceholders"][0]["colorKey"]
        scene["sceneMaterials"] = [
            material for material in scene["sceneMaterials"] if material["key"] != missing_key
        ]

        result = scene_data.validate_scene(scene)

        self.assertTrue(any("has no sceneMaterials preset" in error for error in result.errors))

    def test_invalid_scene_material_response_is_reported(self) -> None:
        scene = copy.deepcopy(self.scene)
        scene["sceneMaterials"][0]["response"] = "sparkly"

        result = scene_data.validate_scene(scene)

        self.assertTrue(any("response must be one of" in error for error in result.errors))

    def test_traversal_prompt_omits_input_prefix_for_playtest_ui_composition(self) -> None:
        affordance = next(
            item for item in self.scene["traversalAffordances"] if item["id"] == "service-barrier-vault"
        )

        self.assertEqual("Vault Service Barrier", affordance["prompt"])
        self.assertNotIn("Press Space", affordance["prompt"])

    def test_service_run_marker_copy_does_not_claim_completion_before_job_is_ready(self) -> None:
        marker = next(
            item for item in self.scene["interactables"] if item["id"] == "service-run-confirm-marker"
        )

        self.assertEqual("Review Service Run Marker", marker["prompt"])
        self.assertNotIn("confirmed", marker["message"].lower())

    def test_v0121_readability_mesh_instances_exist(self) -> None:
        ids = scene_data.collect_ids(self.scene)

        for required_id in [
            "mesh-ferry-office-roof-cap",
            "mesh-ferry-office-facade-panel",
            "mesh-ferry-office-sign-board",
            "mesh-service-gate",
            "mesh-maintenance-box",
            "mesh-dock-bollard-left",
            "mesh-dock-bollard-right",
            "mesh-service-yard-crate",
            "mesh-service-yard-vehicle-body",
            "mesh-service-yard-vehicle-cabin",
        ]:
            self.assertIn(required_id, ids)

    def test_v018_visual_identity_prop_kit_exists(self) -> None:
        ids = scene_data.collect_ids(self.scene)
        mesh_assets = {asset["id"] for asset in self.scene["meshAssets"]}

        for required_asset_id in [
            "service-road-sign-mesh",
            "road-edge-post-mesh",
            "service-barrier-mesh",
            "utility-box-mesh",
        ]:
            self.assertIn(required_asset_id, mesh_assets)

        for required_instance_id in [
            "mesh-service-road-sign",
            "mesh-dock-road-edge-post-a",
            "mesh-dock-road-edge-post-b",
            "mesh-service-yard-barrier-cue",
            "mesh-maintenance-utility-box",
        ]:
            self.assertIn(required_instance_id, ids)

    def test_v020_ferry_notice_board_asset_and_instance_exist(self) -> None:
        ids = scene_data.collect_ids(self.scene)
        mesh_assets = {asset["id"]: asset for asset in self.scene["meshAssets"]}

        self.assertIn("ferry-notice-board-mesh", mesh_assets)
        self.assertIn("mesh-ferry-notice-board", ids)
        self.assertIn("fallback", mesh_assets["ferry-notice-board-mesh"]["provenance"].lower())
        self.assertIn("not a blender export", mesh_assets["ferry-notice-board-mesh"]["provenance"].lower())

    def test_v0201_blender_notice_board_asset_and_instance_exist(self) -> None:
        ids = scene_data.collect_ids(self.scene)
        mesh_assets = {asset["id"]: asset for asset in self.scene["meshAssets"]}

        self.assertIn("blender-ferry-notice-board-mesh", mesh_assets)
        self.assertIn("mesh-blender-ferry-notice-board", ids)
        self.assertIn("blender", mesh_assets["blender-ferry-notice-board-mesh"]["provenance"].lower())
        self.assertIn("5.1.1", mesh_assets["blender-ferry-notice-board-mesh"]["provenance"])

    def test_v025_composition_pass_start_and_landmarks_exist(self) -> None:
        ids = scene_data.collect_ids(self.scene)
        player_start = self.scene["playerStart"]

        self.assertEqual("player-start-dock", player_start["id"])
        self.assertLess(player_start["position"][0], 0.0)
        self.assertLess(player_start["yawDegrees"], 0.0)

        for required_id in [
            "office-approach-runner",
            "office-front-threshold",
            "service-yard-entry-lane-cue",
            "mesh-ferry-office-left-entry-post",
            "mesh-ferry-office-right-entry-post",
            "mesh-ferry-office-approach-notice-board",
            "mesh-service-yard-entry-post-a",
            "mesh-service-yard-entry-post-b",
            "mesh-dock-road-edge-post-c",
            "mesh-service-run-confirm-sign",
        ]:
            self.assertIn(required_id, ids)

    def test_v026_prop_identity_pass_reuses_existing_mesh_assets(self) -> None:
        ids = scene_data.collect_ids(self.scene)
        mesh_assets = {asset["id"] for asset in self.scene["meshAssets"]}
        mesh_instances = {instance["id"]: instance for instance in self.scene["meshInstances"]}

        self.assertGreaterEqual(len(mesh_assets), 7)
        self.assertGreaterEqual(len(mesh_instances), 31)

        expected_assets = {
            "mesh-manifest-counter-shelf": "unit-box-mesh",
            "mesh-manifest-paper-stack": "unit-box-mesh",
            "mesh-office-side-service-panel": "utility-box-mesh",
            "mesh-dock-cleat-left": "unit-box-mesh",
            "mesh-dock-cleat-right": "unit-box-mesh",
            "mesh-service-yard-tool-crate": "unit-box-mesh",
            "mesh-service-run-review-board": "ferry-notice-board-mesh",
        }
        for required_id, asset_id in expected_assets.items():
            self.assertIn(required_id, ids)
            self.assertEqual(asset_id, mesh_instances[required_id]["assetId"])

    def test_v027_blender_cable_reel_asset_and_instance_exist(self) -> None:
        ids = scene_data.collect_ids(self.scene)
        mesh_assets = {asset["id"]: asset for asset in self.scene["meshAssets"]}
        mesh_instances = {instance["id"]: instance for instance in self.scene["meshInstances"]}

        self.assertGreaterEqual(len(mesh_assets), 8)
        self.assertGreaterEqual(len(mesh_instances), 37)
        self.assertIn("blender-cable-reel-mesh", mesh_assets)
        self.assertIn("mesh-service-yard-cable-reel", ids)
        self.assertEqual("blender-cable-reel-mesh", mesh_instances["mesh-service-yard-cable-reel"]["assetId"])
        self.assertEqual("assets/models/blender_cable_reel.gltf", mesh_assets["blender-cable-reel-mesh"]["path"])
        self.assertIn("blender", mesh_assets["blender-cable-reel-mesh"]["provenance"].lower())
        self.assertIn("5.1.1", mesh_assets["blender-cable-reel-mesh"]["provenance"])

    def test_v038_storm_wet_dock_road_work_zone_reuses_existing_mesh_assets(self) -> None:
        ids = scene_data.collect_ids(self.scene)
        mesh_instances = {instance["id"]: instance for instance in self.scene["meshInstances"]}

        expected_assets = {
            "mesh-dock-road-storm-post-near": "road-edge-post-mesh",
            "mesh-dock-road-storm-post-far": "road-edge-post-mesh",
            "mesh-dock-road-storm-barrier": "service-barrier-mesh",
            "mesh-dock-road-maintenance-cabinet": "utility-box-mesh",
            "mesh-service-run-cable-reel": "blender-cable-reel-mesh",
        }
        for required_id, asset_id in expected_assets.items():
            self.assertIn(required_id, ids)
            self.assertEqual(asset_id, mesh_instances[required_id]["assetId"])
            self.assertIn("v0.38", mesh_instances[required_id]["notes"])

    def test_v042_wet_road_surface_asset_and_instances_exist(self) -> None:
        ids = scene_data.collect_ids(self.scene)
        mesh_assets = {asset["id"]: asset for asset in self.scene["meshAssets"]}
        mesh_instances = {instance["id"]: instance for instance in self.scene["meshInstances"]}

        self.assertIn("blender-wet-road-surface-mesh", mesh_assets)
        self.assertEqual("assets/models/blender_wet_road_surface.gltf", mesh_assets["blender-wet-road-surface-mesh"]["path"])
        self.assertIn("Blender 5.1.1", mesh_assets["blender-wet-road-surface-mesh"]["provenance"])

        expected_instances = {
            "mesh-service-yard-wet-surface": "service-yard-driving-pad",
            "mesh-dock-road-wet-surface": "dock-road-segment",
            "mesh-dock-road-turnaround-wet-surface": "dock-road-turnaround-pad",
        }
        for instance_id, placeholder_id in expected_instances.items():
            self.assertIn(instance_id, ids)
            self.assertEqual("blender-wet-road-surface-mesh", mesh_instances[instance_id]["assetId"])
            self.assertEqual("dark-service-asphalt", mesh_instances[instance_id]["colorKey"])
            self.assertEqual(placeholder_id, mesh_instances[instance_id]["replacesVisualPlaceholderId"])

    def test_v043_harbor_backdrop_asset_and_instances_exist(self) -> None:
        ids = scene_data.collect_ids(self.scene)
        material_keys = {material["key"] for material in self.scene["sceneMaterials"]}
        mesh_assets = {asset["id"]: asset for asset in self.scene["meshAssets"]}
        mesh_instances = {instance["id"]: instance for instance in self.scene["meshInstances"]}

        self.assertIn("misty-island-ground", material_keys)
        self.assertIn("blender-harbor-backdrop-mesh", mesh_assets)
        self.assertEqual("assets/models/blender_harbor_backdrop.gltf", mesh_assets["blender-harbor-backdrop-mesh"]["path"])
        self.assertIn("Blender 5.1.1", mesh_assets["blender-harbor-backdrop-mesh"]["provenance"])

        expected_instances = [
            "mesh-dock-road-harbor-backdrop",
            "mesh-dock-start-left-harbor-backdrop",
            "mesh-dock-start-right-harbor-backdrop",
        ]
        for instance_id in expected_instances:
            self.assertIn(instance_id, ids)
            self.assertEqual("blender-harbor-backdrop-mesh", mesh_instances[instance_id]["assetId"])
            self.assertEqual("misty-island-ground", mesh_instances[instance_id]["colorKey"])
            self.assertIn("v0.43", mesh_instances[instance_id]["notes"])

    def test_v044_harbor_water_surface_asset_and_instances_exist(self) -> None:
        ids = scene_data.collect_ids(self.scene)
        mesh_assets = {asset["id"]: asset for asset in self.scene["meshAssets"]}
        mesh_instances = {instance["id"]: instance for instance in self.scene["meshInstances"]}

        self.assertIn("blender-harbor-water-surface-mesh", mesh_assets)
        self.assertEqual(
            "assets/models/blender_harbor_water_surface.gltf",
            mesh_assets["blender-harbor-water-surface-mesh"]["path"],
        )
        self.assertIn("Blender 5.1.1", mesh_assets["blender-harbor-water-surface-mesh"]["provenance"])

        expected_instances = {
            "mesh-water-left-surface": "water-left-band",
            "mesh-water-right-surface": "water-right-band",
            "mesh-dock-road-water-surface": "dock-road-water-edge",
        }
        for instance_id, placeholder_id in expected_instances.items():
            self.assertIn(instance_id, ids)
            self.assertEqual("blender-harbor-water-surface-mesh", mesh_instances[instance_id]["assetId"])
            self.assertEqual("deep-harbor-blue", mesh_instances[instance_id]["colorKey"])
            self.assertEqual(placeholder_id, mesh_instances[instance_id]["replacesVisualPlaceholderId"])
            self.assertIn("v0.44", mesh_instances[instance_id]["notes"])

    def test_v058_clearance_tag_prop_asset_and_instance_exist(self) -> None:
        ids = scene_data.collect_ids(self.scene)
        mesh_assets = {asset["id"]: asset for asset in self.scene["meshAssets"]}
        mesh_instances = {instance["id"]: instance for instance in self.scene["meshInstances"]}

        self.assertIn("clearance-tag-mesh", mesh_assets)
        self.assertEqual("assets/models/clearance_tag.gltf", mesh_assets["clearance-tag-mesh"]["path"])
        self.assertIn("fallback", mesh_assets["clearance-tag-mesh"]["provenance"].lower())
        self.assertIn("v0.58", mesh_assets["clearance-tag-mesh"]["provenance"])
        self.assertIn("mesh-dock-road-clearance-tag", ids)
        self.assertEqual("clearance-tag-mesh", mesh_instances["mesh-dock-road-clearance-tag"]["assetId"])
        self.assertEqual("dock-road-clearance-state", mesh_instances["mesh-dock-road-clearance-tag"]["colorKey"])
        self.assertEqual("dock-road-clearance-status-tag", mesh_instances["mesh-dock-road-clearance-tag"]["replacesVisualPlaceholderId"])

    def test_duplicate_mesh_replacement_links_are_reported(self) -> None:
        scene = copy.deepcopy(self.scene)
        first = copy.deepcopy(scene["meshInstances"][0])
        second = copy.deepcopy(scene["meshInstances"][1])
        first["id"] = "duplicate-replacement-a"
        second["id"] = "duplicate-replacement-b"
        first["replacesVisualPlaceholderId"] = "dock-main-slab"
        second["replacesVisualPlaceholderId"] = "dock-main-slab"
        scene["meshInstances"] = [first, second]

        result = scene_data.validate_scene(scene)

        self.assertTrue(any("duplicate replacesVisualPlaceholderId" in error for error in result.errors))

    def test_v014_service_yard_vehicle_bounds_include_dock_road(self) -> None:
        vehicle = self.scene["vehicles"][0]

        self.assertEqual("service-yard-vehicle", vehicle["id"])
        self.assertEqual([6.2, 0.0, -2.2], vehicle["spawn"]["position"])
        self.assertEqual(88.0, vehicle["spawn"]["yawDegrees"])
        self.assertEqual([0.58, 0.53, 0.92], vehicle["proxyHalfExtents"])
        self.assertEqual(1.8, vehicle["enterRadius"])
        self.assertEqual([3.35, -5.05], vehicle["bounds"]["min"])
        self.assertEqual([19.45, 0.95], vehicle["bounds"]["max"])

    def test_v014_dock_road_authoring_ids_exist(self) -> None:
        ids = scene_data.collect_ids(self.scene)

        for required_id in [
            "dock-road-segment",
            "dock-road-turnaround-pad",
            "dock-road-water-edge",
            "dock-road-south-rail",
            "dock-road-north-curb",
            "dock-road-end-marker",
            "route-service-yard-to-dock-road",
            "dock-road-marker",
        ]:
            self.assertIn(required_id, ids)

    def test_v016_first_driver_fixer_job_authoring_ids_exist(self) -> None:
        ids = scene_data.collect_ids(self.scene)

        for required_id in [
            "service-run-confirm-marker",
            "service-run-checkpoint-marker",
            "route-dock-road-to-service-confirm",
            "dock-road-relay",
            "dock-road-relay-marker",
            "route-service-confirm-to-relay",
        ]:
            self.assertIn(required_id, ids)

    def test_route_marker_unknown_endpoint_is_reported(self) -> None:
        scene = copy.deepcopy(self.scene)
        scene["routeMarkers"][0]["from"] = "missing-route-source"

        result = scene_data.validate_scene(scene)

        self.assertTrue(any("from references unknown id" in error for error in result.errors))

    def test_service_run_route_and_checkpoint_are_aligned(self) -> None:
        checkpoint = next(
            marker for marker in self.scene["objectiveMarkers"] if marker["id"] == "service-run-checkpoint-marker"
        )
        confirm = next(
            interactable for interactable in self.scene["interactables"] if interactable["id"] == "service-run-confirm-marker"
        )
        route = next(
            route for route in self.scene["routeMarkers"] if route["id"] == "route-dock-road-to-service-confirm"
        )
        vehicle = self.scene["vehicles"][0]

        self.assertEqual("dock-road-marker", route["from"])
        self.assertEqual("service-run-confirm-marker", route["to"])
        self.assertEqual([checkpoint["position"][0], checkpoint["position"][2]], [route["points"][0][0], route["points"][0][2]])
        self.assertEqual([confirm["position"][0], confirm["position"][2]], [route["points"][-1][0], route["points"][-1][2]])
        self.assertLessEqual(vehicle["bounds"]["min"][0], checkpoint["position"][0])
        self.assertGreaterEqual(vehicle["bounds"]["max"][0], checkpoint["position"][0])
        self.assertLessEqual(vehicle["bounds"]["min"][1], checkpoint["position"][2])
        self.assertGreaterEqual(vehicle["bounds"]["max"][1], checkpoint["position"][2])

    def test_valid_mesh_reference_scene_validates(self) -> None:
        scene = copy.deepcopy(self.scene)
        scene["meshAssets"] = [
            {
                "id": "test-unit-box-mesh",
                "path": "assets/models/unit_box.gltf",
                "format": "gltf",
                "units": "meter",
                "upAxis": "Y",
                "license": "project-original",
                "provenance": "Created in-repo for validation.",
                "authoringBoundsHalfExtents": [0.5, 0.5, 0.5],
            }
        ]
        scene["meshInstances"] = [
            {
                "id": "test-unit-box-instance",
                "assetId": "test-unit-box-mesh",
                "position": [0.0, 0.5, 0.0],
                "yawDegrees": 0.0,
                "scale": [1.0, 1.0, 1.0],
                "colorKey": "office-muted-concrete",
                "replacesVisualPlaceholderId": "ferry-office-floor-slab",
                "linkedColliderId": "service-gate",
            }
        ]

        result = scene_data.validate_scene(scene)

        self.assertEqual([], result.errors)

    def test_mesh_asset_unsafe_path_is_reported(self) -> None:
        scene = copy.deepcopy(self.scene)
        scene["meshAssets"] = [
            {
                "id": "unsafe-mesh",
                "path": "../outside.gltf",
                "format": "gltf",
                "units": "meter",
                "upAxis": "Y",
                "license": "project-original",
                "provenance": "Created in-repo for validation.",
            }
        ]
        scene["meshInstances"] = []

        result = scene_data.validate_scene(scene)

        self.assertTrue(any("path" in error and "assets/" in error for error in result.errors))

    def test_mesh_instance_missing_asset_is_reported(self) -> None:
        scene = copy.deepcopy(self.scene)
        scene["meshAssets"] = []
        scene["meshInstances"] = [
            {
                "id": "missing-asset-instance",
                "assetId": "does-not-exist",
                "position": [0.0, 0.5, 0.0],
                "yawDegrees": 0.0,
                "scale": [1.0, 1.0, 1.0],
            }
        ]

        result = scene_data.validate_scene(scene)

        self.assertTrue(any("unknown mesh asset" in error for error in result.errors))

    def test_non_positive_mesh_instance_scale_is_reported(self) -> None:
        scene = copy.deepcopy(self.scene)
        scene["meshAssets"] = [
            {
                "id": "test-unit-box-mesh",
                "path": "assets/models/unit_box.gltf",
                "format": "gltf",
                "units": "meter",
                "upAxis": "Y",
                "license": "project-original",
                "provenance": "Created in-repo for validation.",
            }
        ]
        scene["meshInstances"] = [
            {
                "id": "bad-scale-instance",
                "assetId": "test-unit-box-mesh",
                "position": [0.0, 0.5, 0.0],
                "yawDegrees": 0.0,
                "scale": [1.0, 0.0, 1.0],
            }
        ]

        result = scene_data.validate_scene(scene)

        self.assertTrue(any("scale" in error and "positive" in error for error in result.errors))

    def test_mesh_report_scans_model_files_and_reports_reference_status(self) -> None:
        report = mesh_report.build_mesh_report(self.scene)
        files_by_path = {file.relative_path: file for file in report.files}

        self.assertIn("assets/models/unit_box.gltf", files_by_path)
        self.assertIn("assets/models/service_road_sign.gltf", files_by_path)
        self.assertIn("assets/models/clearance_tag.gltf", files_by_path)
        self.assertIn("assets/models/ferry_notice_board.gltf", files_by_path)
        self.assertIn("assets/models/blender_ferry_notice_board.gltf", files_by_path)
        self.assertTrue(files_by_path["assets/models/unit_box.gltf"].referenced)
        self.assertTrue(files_by_path["assets/models/clearance_tag.gltf"].referenced)
        self.assertTrue(files_by_path["assets/models/ferry_notice_board.gltf"].referenced)
        self.assertTrue(files_by_path["assets/models/blender_ferry_notice_board.gltf"].referenced)
        self.assertGreater(files_by_path["assets/models/unit_box.gltf"].vertex_count or 0, 0)
        self.assertGreater(files_by_path["assets/models/unit_box.gltf"].index_count or 0, 0)
        self.assertGreater(files_by_path["assets/models/clearance_tag.gltf"].vertex_count or 0, 0)
        self.assertGreater(files_by_path["assets/models/clearance_tag.gltf"].index_count or 0, 0)
        self.assertGreater(files_by_path["assets/models/ferry_notice_board.gltf"].vertex_count or 0, 0)
        self.assertGreater(files_by_path["assets/models/ferry_notice_board.gltf"].index_count or 0, 0)
        self.assertGreater(files_by_path["assets/models/blender_ferry_notice_board.gltf"].vertex_count or 0, 0)
        self.assertGreater(files_by_path["assets/models/blender_ferry_notice_board.gltf"].index_count or 0, 0)
        self.assertIsNotNone(files_by_path["assets/models/unit_box.gltf"].bounds_min)
        self.assertIsNotNone(files_by_path["assets/models/unit_box.gltf"].bounds_max)
        self.assertIsNotNone(files_by_path["assets/models/clearance_tag.gltf"].bounds_min)
        self.assertIsNotNone(files_by_path["assets/models/clearance_tag.gltf"].bounds_max)
        self.assertIsNotNone(files_by_path["assets/models/ferry_notice_board.gltf"].bounds_min)
        self.assertIsNotNone(files_by_path["assets/models/ferry_notice_board.gltf"].bounds_max)
        self.assertIsNotNone(files_by_path["assets/models/blender_ferry_notice_board.gltf"].bounds_min)
        self.assertIsNotNone(files_by_path["assets/models/blender_ferry_notice_board.gltf"].bounds_max)
        self.assertEqual([], [file.relative_path for file in report.files if file.suffix == ".gltf" and not file.referenced])

    def test_blender_check_reports_missing_command_without_throwing(self) -> None:
        result = check_blender.check_blender("__definitely_missing_blender_for_tidebreak_tests__")

        self.assertFalse(result.available)
        self.assertIn("__definitely_missing_blender_for_tidebreak_tests__", result.command)
        self.assertTrue(result.error)

    def test_fallback_prop_generator_writes_supported_embedded_gltf(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            output_path = pathlib.Path(temp_dir) / "ferry_notice_board.gltf"

            create_simple_prop_gltf.write_ferry_notice_board(output_path)
            metadata = mesh_report.asset_data.load_gltf_metadata(output_path)

        self.assertGreater(metadata.vertex_count or 0, 0)
        self.assertGreater(metadata.index_count or 0, 0)
        self.assertIsNotNone(metadata.bounds_min)
        self.assertIsNotNone(metadata.bounds_max)
        self.assertEqual([], metadata.errors)

    def test_fallback_clearance_tag_generator_writes_supported_embedded_gltf(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            output_path = pathlib.Path(temp_dir) / "clearance_tag.gltf"

            create_simple_prop_gltf.write_clearance_tag(output_path)
            metadata = mesh_report.asset_data.load_gltf_metadata(output_path)

        self.assertGreater(metadata.vertex_count or 0, 0)
        self.assertGreater(metadata.index_count or 0, 0)
        self.assertIsNotNone(metadata.bounds_min)
        self.assertIsNotNone(metadata.bounds_max)
        self.assertEqual([], metadata.errors)

    def test_blender_notice_board_script_exists_and_documents_export_contract(self) -> None:
        script_path = ROOT / "tools" / "blender" / "create_tidebreak_notice_board.py"

        self.assertTrue(script_path.exists())
        script_text = script_path.read_text(encoding="utf-8")
        self.assertIn("blender_ferry_notice_board.gltf", script_text)
        self.assertIn("export_scene.gltf", script_text)
        self.assertIn("GLTF_SEPARATE", script_text)
        self.assertIn("data:application/octet-stream;base64", script_text)

    def test_blender_cable_reel_script_exists_and_documents_export_contract(self) -> None:
        script_path = ROOT / "tools" / "blender" / "create_tidebreak_cable_reel.py"

        self.assertTrue(script_path.exists())
        script_text = script_path.read_text(encoding="utf-8")
        self.assertIn("blender_cable_reel.gltf", script_text)
        self.assertIn("export_scene.gltf", script_text)
        self.assertIn("GLTF_SEPARATE", script_text)
        self.assertIn("data:application/octet-stream;base64", script_text)

    def test_asset_workflow_reports_unreferenced_gltf_files(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            models_dir = pathlib.Path(temp_dir)
            (models_dir / "orphan.gltf").write_text(json.dumps({"asset": {"version": "2.0"}, "meshes": []}), encoding="utf-8")
            scene = copy.deepcopy(self.scene)
            scene["meshAssets"] = []
            scene["meshInstances"] = []

            result = scene_data.validate_asset_workflow(scene, models_dir=models_dir)

        self.assertTrue(any("unreferenced" in error and "orphan.gltf" in error for error in result.errors))

    def test_asset_workflow_reports_unsupported_glb_files(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            models_dir = pathlib.Path(temp_dir)
            (models_dir / "not-supported.glb").write_bytes(b"glTF")

            result = scene_data.validate_asset_workflow(self.scene, models_dir=models_dir)

        self.assertTrue(any("unsupported .glb" in error for error in result.errors))

    def test_asset_workflow_reports_external_buffers(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            models_dir = pathlib.Path(temp_dir)
            (models_dir / "external_buffer.gltf").write_text(
                json.dumps(
                    {
                        "asset": {"version": "2.0"},
                        "buffers": [{"uri": "external_buffer.bin", "byteLength": 12}],
                        "meshes": [],
                    }
                ),
                encoding="utf-8",
            )
            scene = copy.deepcopy(self.scene)
            scene["meshAssets"] = [
                {
                    "id": "external-buffer-mesh",
                    "path": "external_buffer.gltf",
                    "format": "gltf",
                    "units": "meter",
                    "upAxis": "Y",
                    "license": "project-original",
                    "provenance": "Created in test.",
                }
            ]
            scene["meshInstances"] = []

            result = scene_data.validate_asset_workflow(scene, models_dir=models_dir)

        self.assertTrue(any("external buffer" in error for error in result.errors))


if __name__ == "__main__":
    unittest.main()
