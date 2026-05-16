#!/usr/bin/env python3
"""Unit tests for the Ferry Office vehicle runtime comparison QA wrapper."""

from __future__ import annotations

import json
import pathlib
import sys
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import vehicle_runtime_qa  # noqa: E402


OBSTACLE_CHECKS = [
    {
        "backend": "deterministic",
        "passed": True,
        "clearedObstacleProxy": True,
        "frameCount": 180,
        "maxLateralOffset": 0.45,
        "minDistanceToObstacle": 1.1,
        "collisionBacked": True,
        "obstacleCollisionClear": True,
        "obstacleOverlapFrames": 0,
        "minCollisionClearance": 0.32,
        "maxCameraYawDeltaDegrees": 18.0,
        "finalCameraYawDegrees": 71.0,
        "finalPosition": [13.4, 0.0, -1.75],
        "finalYawDegrees": 72.0,
        "hitBounds": False,
    },
    {
        "backend": "jolt",
        "passed": True,
        "clearedObstacleProxy": True,
        "frameCount": 180,
        "maxLateralOffset": 0.40,
        "minDistanceToObstacle": 1.0,
        "collisionBacked": True,
        "obstacleCollisionClear": True,
        "obstacleOverlapFrames": 0,
        "minCollisionClearance": 0.28,
        "maxCameraYawDeltaDegrees": 20.0,
        "finalCameraYawDegrees": 73.0,
        "finalPosition": [13.0, 0.0, -1.8],
        "finalYawDegrees": 74.0,
        "hitBounds": False,
    },
]

DRIVING_FEEL_CHECKS = [
    {
        "backend": backend,
        "name": name,
        "passed": True,
        "value": value,
        "minValue": min_value,
        "maxValue": max_value,
        "units": units,
        "message": f"{backend} {name} passed.",
    }
    for backend in ("deterministic", "jolt")
    for name, value, min_value, max_value, units in (
        ("routeFramesToCheckpoint", 139.0 if backend == "deterministic" else 212.0, 1.0, 240.0, "frames"),
        ("routeMaxLateralDeviation", 0.08, 0.0, 0.85, "meters"),
        ("brakeStopDistance", 2.1, 0.0, 4.5, "meters"),
        ("reverseDistance", 1.2, 0.35, 6.0, "meters"),
        ("steeringYawResponse", 24.0, 4.0, 95.0, "degrees"),
        ("cameraYawLag", 18.0, 0.0, 65.0, "degrees"),
    )
]


class VehicleRuntimeQaTests(unittest.TestCase):
    def test_report_validation_accepts_passing_runtime_comparison_report(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            report_path = pathlib.Path(temp) / "vehicle-runtime-comparison.json"
            samples = [
                {
                    "name": "accelerate",
                    "passed": True,
                    "position": [7.0, 0.0, -2.1],
                    "yawDegrees": 90.0,
                    "speed": 1.2,
                    "wheelContactCount": 4,
                    "outOfBounds": False,
                }
            ]
            report_path.write_text(
                json.dumps(
                    {
                        "schema": vehicle_runtime_qa.SCHEMA,
                        "scenario": vehicle_runtime_qa.SCENARIO,
                        "passed": True,
                        "vehicle": {"id": "service-yard-vehicle"},
                        "deterministic": {"backend": "deterministic", "samples": samples},
                        "adapter": {"backend": "jolt", "samples": samples},
                        "routeChecks": [
                            {
                                "backend": "deterministic",
                                "passed": True,
                                "checkpointReached": True,
                                "framesToCheckpoint": 137,
                                "minDistanceToCheckpoint": 0.35,
                                "finalPosition": [19.4, 0.0, -2.2],
                                "finalYawDegrees": 88.0,
                                "hitBounds": False,
                                "message": "Deterministic route reached the service-run checkpoint.",
                            },
                            {
                                "backend": "jolt",
                                "passed": True,
                                "checkpointReached": True,
                                "framesToCheckpoint": 142,
                                "minDistanceToCheckpoint": 0.42,
                                "finalPosition": [19.1, 0.0, -2.1],
                                "finalYawDegrees": 89.0,
                                "hitBounds": False,
                                "message": "Jolt route reached the service-run checkpoint.",
                            },
                        ],
                        "obstacleChecks": OBSTACLE_CHECKS,
                        "controlChecks": [
                            {
                                "name": "tapThrottleCoast",
                                "passed": True,
                                "frameIndex": 91,
                                "speed": 0.08,
                                "distance": 0.4,
                                "message": "Short throttle tap settled.",
                            },
                            {
                                "name": "brakeStopsForwardMotion",
                                "passed": True,
                                "frameIndex": 75,
                                "speed": 0.02,
                                "distance": 0.0,
                                "message": "Brake stopped forward motion.",
                            },
                            {
                                "name": "reverseMovesBackward",
                                "passed": True,
                                "frameIndex": 135,
                                "speed": -0.45,
                                "distance": 0.6,
                                "message": "Reverse moved backward.",
                            },
                            {
                                "name": "reverseCoastSettles",
                                "passed": True,
                                "frameIndex": 225,
                                "speed": -0.05,
                                "distance": 0.4,
                                "message": "Reverse coast settled.",
                            },
                        ],
                        "drivingFeelChecks": DRIVING_FEEL_CHECKS,
                        "comparison": {
                            "maxPositionDelta": 0.45,
                            "maxYawDeltaDegrees": 8.0,
                            "maxSpeedDelta": 0.5,
                            "recommendation": "promote",
                        },
                        "error": "",
                    }
                ),
                encoding="utf-8",
            )

            report = vehicle_runtime_qa.load_and_validate_report(report_path)

        self.assertEqual(report["adapter"]["backend"], "jolt")
        self.assertEqual(report["comparison"]["recommendation"], "promote")
        self.assertEqual(len(report["drivingFeelChecks"]), 12)

    def test_report_validation_rejects_large_runtime_delta(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            report_path = pathlib.Path(temp) / "vehicle-runtime-comparison.json"
            report_path.write_text(
                json.dumps(
                    {
                        "schema": vehicle_runtime_qa.SCHEMA,
                        "scenario": vehicle_runtime_qa.SCENARIO,
                        "passed": True,
                        "vehicle": {"id": "service-yard-vehicle"},
                        "deterministic": {"backend": "deterministic", "samples": [{"name": "accelerate", "passed": True}]},
                        "adapter": {"backend": "jolt", "samples": [{"name": "accelerate", "passed": True, "wheelContactCount": 4}]},
                        "routeChecks": [
                            {
                                "backend": "deterministic",
                                "passed": True,
                                "checkpointReached": True,
                                "framesToCheckpoint": 137,
                                "minDistanceToCheckpoint": 0.35,
                                "finalPosition": [19.4, 0.0, -2.2],
                                "finalYawDegrees": 88.0,
                                "hitBounds": False,
                            },
                            {
                                "backend": "jolt",
                                "passed": True,
                                "checkpointReached": True,
                                "framesToCheckpoint": 142,
                                "minDistanceToCheckpoint": 0.42,
                                "finalPosition": [19.1, 0.0, -2.1],
                                "finalYawDegrees": 89.0,
                                "hitBounds": False,
                            },
                        ],
                        "obstacleChecks": OBSTACLE_CHECKS,
                        "controlChecks": [
                            {"name": "tapThrottleCoast", "passed": True, "frameIndex": 91, "speed": 0.08, "distance": 0.4},
                            {"name": "brakeStopsForwardMotion", "passed": True, "frameIndex": 75, "speed": 0.02, "distance": 0.0},
                            {"name": "reverseMovesBackward", "passed": True, "frameIndex": 135, "speed": -0.45, "distance": 0.6},
                            {"name": "reverseCoastSettles", "passed": True, "frameIndex": 225, "speed": -0.05, "distance": 0.4},
                        ],
                        "drivingFeelChecks": DRIVING_FEEL_CHECKS,
                        "comparison": {
                            "maxPositionDelta": 9.0,
                            "maxYawDeltaDegrees": 8.0,
                            "maxSpeedDelta": 0.5,
                            "recommendation": "promote",
                        },
                        "error": "",
                    }
                ),
                encoding="utf-8",
            )

            with self.assertRaisesRegex(ValueError, "position delta"):
                vehicle_runtime_qa.load_and_validate_report(report_path)

    def test_report_validation_rejects_missing_control_checks(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            report_path = pathlib.Path(temp) / "vehicle-runtime-comparison.json"
            samples = [{"name": "accelerate", "passed": True, "wheelContactCount": 4, "outOfBounds": False}]
            report_path.write_text(
                json.dumps(
                    {
                        "schema": vehicle_runtime_qa.SCHEMA,
                        "scenario": vehicle_runtime_qa.SCENARIO,
                        "passed": True,
                        "vehicle": {"id": "service-yard-vehicle"},
                        "deterministic": {"backend": "deterministic", "samples": samples},
                        "adapter": {"backend": "jolt", "samples": samples},
                        "routeChecks": [
                            {
                                "backend": "deterministic",
                                "passed": True,
                                "checkpointReached": True,
                                "framesToCheckpoint": 137,
                                "minDistanceToCheckpoint": 0.35,
                                "finalPosition": [19.4, 0.0, -2.2],
                                "finalYawDegrees": 88.0,
                                "hitBounds": False,
                            },
                            {
                                "backend": "jolt",
                                "passed": True,
                                "checkpointReached": True,
                                "framesToCheckpoint": 142,
                                "minDistanceToCheckpoint": 0.42,
                                "finalPosition": [19.1, 0.0, -2.1],
                                "finalYawDegrees": 89.0,
                                "hitBounds": False,
                            },
                        ],
                        "comparison": {
                            "maxPositionDelta": 0.45,
                            "maxYawDeltaDegrees": 8.0,
                            "maxSpeedDelta": 0.5,
                            "recommendation": "promote",
                        },
                        "error": "",
                    }
                ),
                encoding="utf-8",
            )

            with self.assertRaisesRegex(ValueError, "control checks"):
                vehicle_runtime_qa.load_and_validate_report(report_path)

    def test_report_validation_rejects_missing_route_checks(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            report_path = pathlib.Path(temp) / "vehicle-runtime-comparison.json"
            samples = [{"name": "accelerate", "passed": True, "wheelContactCount": 4, "outOfBounds": False}]
            report_path.write_text(
                json.dumps(
                    {
                        "schema": vehicle_runtime_qa.SCHEMA,
                        "scenario": vehicle_runtime_qa.SCENARIO,
                        "passed": True,
                        "vehicle": {"id": "service-yard-vehicle"},
                        "deterministic": {"backend": "deterministic", "samples": samples},
                        "adapter": {"backend": "jolt", "samples": samples},
                        "controlChecks": [
                            {"name": "tapThrottleCoast", "passed": True, "frameIndex": 91, "speed": 0.08, "distance": 0.4},
                            {"name": "brakeStopsForwardMotion", "passed": True, "frameIndex": 75, "speed": 0.02, "distance": 0.0},
                            {"name": "reverseMovesBackward", "passed": True, "frameIndex": 135, "speed": -0.45, "distance": 0.6},
                            {"name": "reverseCoastSettles", "passed": True, "frameIndex": 225, "speed": -0.05, "distance": 0.4},
                        ],
                        "comparison": {
                            "maxPositionDelta": 0.45,
                            "maxYawDeltaDegrees": 8.0,
                            "maxSpeedDelta": 0.5,
                            "recommendation": "promote",
                        },
                        "error": "",
                    }
                ),
                encoding="utf-8",
            )

            with self.assertRaisesRegex(ValueError, "route checks"):
                vehicle_runtime_qa.load_and_validate_report(report_path)

    def test_report_validation_rejects_slow_jolt_route_check(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            report_path = pathlib.Path(temp) / "vehicle-runtime-comparison.json"
            samples = [{"name": "accelerate", "passed": True, "wheelContactCount": 4, "outOfBounds": False}]
            control_checks = [
                {"name": "tapThrottleCoast", "passed": True, "frameIndex": 91, "speed": 0.08, "distance": 0.4},
                {"name": "brakeStopsForwardMotion", "passed": True, "frameIndex": 75, "speed": 0.02, "distance": 0.0},
                {"name": "reverseMovesBackward", "passed": True, "frameIndex": 135, "speed": -0.45, "distance": 0.6},
                {"name": "reverseCoastSettles", "passed": True, "frameIndex": 225, "speed": -0.05, "distance": 0.4},
            ]
            report_path.write_text(
                json.dumps(
                    {
                        "schema": vehicle_runtime_qa.SCHEMA,
                        "scenario": vehicle_runtime_qa.SCENARIO,
                        "passed": True,
                        "vehicle": {"id": "service-yard-vehicle"},
                        "deterministic": {"backend": "deterministic", "samples": samples},
                        "adapter": {"backend": "jolt", "samples": samples},
                        "routeChecks": [
                            {
                                "backend": "deterministic",
                                "passed": True,
                                "checkpointReached": True,
                                "framesToCheckpoint": 139,
                                "minDistanceToCheckpoint": 1.7,
                                "finalPosition": [17.6, 0.0, -1.8],
                                "finalYawDegrees": 88.0,
                                "hitBounds": False,
                            },
                            {
                                "backend": "jolt",
                                "passed": True,
                                "checkpointReached": True,
                                "framesToCheckpoint": 301,
                                "minDistanceToCheckpoint": 1.8,
                                "finalPosition": [17.6, 1.1, -1.8],
                                "finalYawDegrees": 88.0,
                                "hitBounds": False,
                            },
                        ],
                        "controlChecks": control_checks,
                        "comparison": {
                            "maxPositionDelta": 2.95,
                            "maxYawDeltaDegrees": 25.0,
                            "maxSpeedDelta": 2.8,
                            "recommendation": "promote",
                        },
                        "error": "",
                    }
                ),
                encoding="utf-8",
            )

            with self.assertRaisesRegex(ValueError, "route frame budget"):
                vehicle_runtime_qa.load_and_validate_report(report_path)

    def test_report_validation_rejects_missing_obstacle_checks(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            report_path = pathlib.Path(temp) / "vehicle-runtime-comparison.json"
            samples = [{"name": "accelerate", "passed": True, "wheelContactCount": 4, "outOfBounds": False}]
            control_checks = [
                {"name": "tapThrottleCoast", "passed": True, "frameIndex": 91, "speed": 0.08, "distance": 0.4},
                {"name": "brakeStopsForwardMotion", "passed": True, "frameIndex": 75, "speed": 0.02, "distance": 0.0},
                {"name": "reverseMovesBackward", "passed": True, "frameIndex": 135, "speed": -0.45, "distance": 0.6},
                {"name": "reverseCoastSettles", "passed": True, "frameIndex": 225, "speed": -0.05, "distance": 0.4},
            ]
            route_checks = [
                {
                    "backend": "deterministic",
                    "passed": True,
                    "checkpointReached": True,
                    "framesToCheckpoint": 139,
                    "minDistanceToCheckpoint": 1.7,
                    "finalPosition": [17.6, 0.0, -1.8],
                    "finalYawDegrees": 88.0,
                    "hitBounds": False,
                },
                {
                    "backend": "jolt",
                    "passed": True,
                    "checkpointReached": True,
                    "framesToCheckpoint": 213,
                    "minDistanceToCheckpoint": 1.8,
                    "finalPosition": [17.6, 1.1, -1.8],
                    "finalYawDegrees": 88.0,
                    "hitBounds": False,
                },
            ]
            report_path.write_text(
                json.dumps(
                    {
                        "schema": vehicle_runtime_qa.SCHEMA,
                        "scenario": vehicle_runtime_qa.SCENARIO,
                        "passed": True,
                        "vehicle": {"id": "service-yard-vehicle"},
                        "deterministic": {"backend": "deterministic", "samples": samples},
                        "adapter": {"backend": "jolt", "samples": samples},
                        "routeChecks": route_checks,
                        "controlChecks": control_checks,
                        "comparison": {
                            "maxPositionDelta": 2.95,
                            "maxYawDeltaDegrees": 25.0,
                            "maxSpeedDelta": 2.8,
                            "recommendation": "promote",
                        },
                        "error": "",
                    }
                ),
                encoding="utf-8",
            )

            with self.assertRaisesRegex(ValueError, "obstacle-proxy"):
                vehicle_runtime_qa.load_and_validate_report(report_path)

    def test_report_validation_rejects_missing_driving_feel_checks(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            report_path = pathlib.Path(temp) / "vehicle-runtime-comparison.json"
            samples = [{"name": "accelerate", "passed": True, "wheelContactCount": 4, "outOfBounds": False}]
            control_checks = [
                {"name": "tapThrottleCoast", "passed": True, "frameIndex": 91, "speed": 0.08, "distance": 0.4},
                {"name": "brakeStopsForwardMotion", "passed": True, "frameIndex": 75, "speed": 0.02, "distance": 0.0},
                {"name": "reverseMovesBackward", "passed": True, "frameIndex": 135, "speed": -0.45, "distance": 0.6},
                {"name": "reverseCoastSettles", "passed": True, "frameIndex": 225, "speed": -0.05, "distance": 0.4},
            ]
            route_checks = [
                {
                    "backend": "deterministic",
                    "passed": True,
                    "checkpointReached": True,
                    "framesToCheckpoint": 139,
                    "minDistanceToCheckpoint": 1.7,
                    "finalPosition": [17.6, 0.0, -1.8],
                    "finalYawDegrees": 88.0,
                    "hitBounds": False,
                },
                {
                    "backend": "jolt",
                    "passed": True,
                    "checkpointReached": True,
                    "framesToCheckpoint": 213,
                    "minDistanceToCheckpoint": 1.8,
                    "finalPosition": [17.6, 1.1, -1.8],
                    "finalYawDegrees": 88.0,
                    "hitBounds": False,
                },
            ]
            report_path.write_text(
                json.dumps(
                    {
                        "schema": vehicle_runtime_qa.SCHEMA,
                        "scenario": vehicle_runtime_qa.SCENARIO,
                        "passed": True,
                        "vehicle": {"id": "service-yard-vehicle"},
                        "deterministic": {"backend": "deterministic", "samples": samples},
                        "adapter": {"backend": "jolt", "samples": samples},
                        "routeChecks": route_checks,
                        "obstacleChecks": OBSTACLE_CHECKS,
                        "controlChecks": control_checks,
                        "comparison": {
                            "maxPositionDelta": 2.95,
                            "maxYawDeltaDegrees": 25.0,
                            "maxSpeedDelta": 2.8,
                            "recommendation": "promote",
                        },
                        "error": "",
                    }
                ),
                encoding="utf-8",
            )

            with self.assertRaisesRegex(ValueError, "driving-feel"):
                vehicle_runtime_qa.load_and_validate_report(report_path)

    def test_report_validation_rejects_obstacle_checks_missing_camera_telemetry(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            report_path = pathlib.Path(temp) / "vehicle-runtime-comparison.json"
            stale_obstacle_checks = [
                {
                    key: value
                    for key, value in check.items()
                    if key not in {"maxCameraYawDeltaDegrees", "finalCameraYawDegrees"}
                }
                for check in OBSTACLE_CHECKS
            ]
            samples = [{"name": "accelerate", "passed": True, "wheelContactCount": 4, "outOfBounds": False}]
            report_path.write_text(
                json.dumps(
                    {
                        "schema": vehicle_runtime_qa.SCHEMA,
                        "scenario": vehicle_runtime_qa.SCENARIO,
                        "passed": True,
                        "vehicle": {"id": "service-yard-vehicle"},
                        "deterministic": {"backend": "deterministic", "samples": samples},
                        "adapter": {"backend": "jolt", "samples": samples},
                        "routeChecks": [
                            {
                                "backend": "deterministic",
                                "passed": True,
                                "checkpointReached": True,
                                "framesToCheckpoint": 137,
                                "minDistanceToCheckpoint": 0.35,
                                "finalPosition": [19.4, 0.0, -2.2],
                                "finalYawDegrees": 88.0,
                                "hitBounds": False,
                            },
                            {
                                "backend": "jolt",
                                "passed": True,
                                "checkpointReached": True,
                                "framesToCheckpoint": 142,
                                "minDistanceToCheckpoint": 0.42,
                                "finalPosition": [19.1, 0.0, -2.1],
                                "finalYawDegrees": 89.0,
                                "hitBounds": False,
                            },
                        ],
                        "obstacleChecks": stale_obstacle_checks,
                        "controlChecks": [
                            {"name": "tapThrottleCoast", "passed": True, "frameIndex": 91, "speed": 0.08, "distance": 0.4},
                            {"name": "brakeStopsForwardMotion", "passed": True, "frameIndex": 75, "speed": 0.02, "distance": 0.0},
                            {"name": "reverseMovesBackward", "passed": True, "frameIndex": 135, "speed": -0.45, "distance": 0.6},
                            {"name": "reverseCoastSettles", "passed": True, "frameIndex": 225, "speed": -0.05, "distance": 0.4},
                        ],
                        "comparison": {
                            "maxPositionDelta": 0.45,
                            "maxYawDeltaDegrees": 8.0,
                            "maxSpeedDelta": 0.5,
                            "recommendation": "promote",
                        },
                        "error": "",
                    }
                ),
                encoding="utf-8",
            )

            with self.assertRaisesRegex(ValueError, "telemetry"):
                vehicle_runtime_qa.load_and_validate_report(report_path)

    def test_report_validation_rejects_obstacle_checks_without_collision_evidence(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            report_path = pathlib.Path(temp) / "vehicle-runtime-comparison.json"
            stale_obstacle_checks = [
                {
                    key: value
                    for key, value in check.items()
                    if key not in {"collisionBacked", "obstacleCollisionClear", "obstacleOverlapFrames", "minCollisionClearance"}
                }
                for check in OBSTACLE_CHECKS
            ]
            samples = [{"name": "accelerate", "passed": True, "wheelContactCount": 4, "outOfBounds": False}]
            report_path.write_text(
                json.dumps(
                    {
                        "schema": vehicle_runtime_qa.SCHEMA,
                        "scenario": vehicle_runtime_qa.SCENARIO,
                        "passed": True,
                        "vehicle": {"id": "service-yard-vehicle"},
                        "deterministic": {"backend": "deterministic", "samples": samples},
                        "adapter": {"backend": "jolt", "samples": samples},
                        "routeChecks": [
                            {
                                "backend": "deterministic",
                                "passed": True,
                                "checkpointReached": True,
                                "framesToCheckpoint": 137,
                                "minDistanceToCheckpoint": 0.35,
                                "finalPosition": [19.4, 0.0, -2.2],
                                "finalYawDegrees": 88.0,
                                "hitBounds": False,
                            },
                            {
                                "backend": "jolt",
                                "passed": True,
                                "checkpointReached": True,
                                "framesToCheckpoint": 142,
                                "minDistanceToCheckpoint": 0.42,
                                "finalPosition": [19.1, 0.0, -2.1],
                                "finalYawDegrees": 89.0,
                                "hitBounds": False,
                            },
                        ],
                        "obstacleChecks": stale_obstacle_checks,
                        "controlChecks": [
                            {"name": "tapThrottleCoast", "passed": True, "frameIndex": 91, "speed": 0.08, "distance": 0.4},
                            {"name": "brakeStopsForwardMotion", "passed": True, "frameIndex": 75, "speed": 0.02, "distance": 0.0},
                            {"name": "reverseMovesBackward", "passed": True, "frameIndex": 135, "speed": -0.45, "distance": 0.6},
                            {"name": "reverseCoastSettles", "passed": True, "frameIndex": 225, "speed": -0.05, "distance": 0.4},
                        ],
                        "comparison": {
                            "maxPositionDelta": 0.45,
                            "maxYawDeltaDegrees": 8.0,
                            "maxSpeedDelta": 0.5,
                            "recommendation": "promote",
                        },
                        "error": "",
                    }
                ),
                encoding="utf-8",
            )

            with self.assertRaisesRegex(ValueError, "collision-backed"):
                vehicle_runtime_qa.load_and_validate_report(report_path)

    def test_report_validation_rejects_large_obstacle_progress_delta(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            report_path = pathlib.Path(temp) / "vehicle-runtime-comparison.json"
            stale_obstacle_checks = [
                dict(OBSTACLE_CHECKS[0], finalPosition=[18.8, 0.0, -0.24]),
                dict(OBSTACLE_CHECKS[1], finalPosition=[11.9, 0.0, -2.41]),
            ]
            samples = [{"name": "accelerate", "passed": True, "wheelContactCount": 4, "outOfBounds": False}]
            report_path.write_text(
                json.dumps(
                    {
                        "schema": vehicle_runtime_qa.SCHEMA,
                        "scenario": vehicle_runtime_qa.SCENARIO,
                        "passed": True,
                        "vehicle": {"id": "service-yard-vehicle"},
                        "deterministic": {"backend": "deterministic", "samples": samples},
                        "adapter": {"backend": "jolt", "samples": samples},
                        "routeChecks": [
                            {
                                "backend": "deterministic",
                                "passed": True,
                                "checkpointReached": True,
                                "framesToCheckpoint": 139,
                                "minDistanceToCheckpoint": 1.7,
                                "finalPosition": [17.6, 0.0, -1.8],
                                "finalYawDegrees": 88.0,
                                "hitBounds": False,
                            },
                            {
                                "backend": "jolt",
                                "passed": True,
                                "checkpointReached": True,
                                "framesToCheckpoint": 213,
                                "minDistanceToCheckpoint": 1.8,
                                "finalPosition": [17.6, 1.1, -1.8],
                                "finalYawDegrees": 88.0,
                                "hitBounds": False,
                            },
                        ],
                        "obstacleChecks": stale_obstacle_checks,
                        "controlChecks": [
                            {"name": "tapThrottleCoast", "passed": True, "frameIndex": 91, "speed": 0.08, "distance": 0.4},
                            {"name": "brakeStopsForwardMotion", "passed": True, "frameIndex": 75, "speed": 0.02, "distance": 0.0},
                            {"name": "reverseMovesBackward", "passed": True, "frameIndex": 135, "speed": -0.45, "distance": 0.6},
                            {"name": "reverseCoastSettles", "passed": True, "frameIndex": 225, "speed": -0.05, "distance": 0.4},
                        ],
                        "comparison": {
                            "maxPositionDelta": 2.95,
                            "maxYawDeltaDegrees": 25.0,
                            "maxSpeedDelta": 2.8,
                            "recommendation": "defer",
                        },
                        "error": "",
                    }
                ),
                encoding="utf-8",
            )

            with self.assertRaisesRegex(ValueError, "obstacle progress delta"):
                vehicle_runtime_qa.load_and_validate_report(report_path)


if __name__ == "__main__":
    unittest.main()
