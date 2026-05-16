#!/usr/bin/env python3
"""Unit tests for the Ferry Office vehicle physics QA wrapper."""

from __future__ import annotations

import json
import pathlib
import sys
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import vehicle_physics_qa  # noqa: E402


class VehiclePhysicsQaTests(unittest.TestCase):
    def test_report_validation_accepts_passing_vehicle_feasibility_report(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            report_path = pathlib.Path(temp) / "vehicle-feasibility.json"
            report_path.write_text(
                json.dumps(
                    {
                        "schema": vehicle_physics_qa.SCHEMA,
                        "scenario": vehicle_physics_qa.SCENARIO,
                        "passed": True,
                        "backend": "jolt",
                        "vehicle": {"id": "service-yard-vehicle"},
                        "inputFrameCount": 360,
                        "samples": [
                            {"name": "settled", "passed": True, "wheelContactCount": 4},
                            {"name": "accelerate", "passed": True, "wheelContactCount": 4},
                            {"name": "steer", "passed": True, "wheelContactCount": 4},
                            {"name": "brake", "passed": True, "wheelContactCount": 4},
                        ],
                        "final": {"position": [6.2, 0.0, -1.0], "speed": 0.0, "yawDegrees": 90.0},
                        "recommendation": "promote",
                        "recommendationReason": "Probe stayed stable.",
                        "error": "",
                    }
                ),
                encoding="utf-8",
            )

            report = vehicle_physics_qa.load_and_validate_report(report_path)

        self.assertEqual("jolt", report["backend"])
        self.assertEqual("promote", report["recommendation"])

    def test_report_validation_rejects_failed_sample(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            report_path = pathlib.Path(temp) / "vehicle-feasibility.json"
            report_path.write_text(
                json.dumps(
                    {
                        "schema": vehicle_physics_qa.SCHEMA,
                        "scenario": vehicle_physics_qa.SCENARIO,
                        "passed": True,
                        "backend": "jolt",
                        "vehicle": {"id": "service-yard-vehicle"},
                        "inputFrameCount": 360,
                        "samples": [
                            {"name": "settled", "passed": True, "wheelContactCount": 4},
                            {"name": "accelerate", "passed": False, "wheelContactCount": 0},
                            {"name": "steer", "passed": True, "wheelContactCount": 4},
                            {"name": "brake", "passed": True, "wheelContactCount": 4},
                        ],
                        "final": {"position": [6.2, 0.0, -1.0], "speed": 0.0, "yawDegrees": 90.0},
                        "recommendation": "defer",
                        "recommendationReason": "Probe lost contacts.",
                        "error": "",
                    }
                ),
                encoding="utf-8",
            )

            with self.assertRaisesRegex(ValueError, "sample failed"):
                vehicle_physics_qa.load_and_validate_report(report_path)


if __name__ == "__main__":
    unittest.main()
