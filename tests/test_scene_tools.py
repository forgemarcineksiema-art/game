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


if __name__ == "__main__":
    unittest.main()
