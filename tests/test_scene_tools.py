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
        self.assertEqual(6, summary.interactable_count)
        self.assertEqual(1, summary.traversal_count)
        self.assertEqual(1, summary.vehicle_count)

    def test_scene_summary_reports_mesh_assets_and_instances(self) -> None:
        summary = scene_data.build_summary(self.scene)

        self.assertGreaterEqual(summary.mesh_asset_count, 1)
        self.assertGreaterEqual(summary.mesh_instance_count, 9)

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
        self.assertTrue(files_by_path["assets/models/unit_box.gltf"].referenced)
        self.assertGreater(files_by_path["assets/models/unit_box.gltf"].vertex_count or 0, 0)
        self.assertGreater(files_by_path["assets/models/unit_box.gltf"].index_count or 0, 0)
        self.assertIsNotNone(files_by_path["assets/models/unit_box.gltf"].bounds_min)
        self.assertIsNotNone(files_by_path["assets/models/unit_box.gltf"].bounds_max)
        self.assertEqual([], [file.relative_path for file in report.files if file.suffix == ".gltf" and not file.referenced])

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
