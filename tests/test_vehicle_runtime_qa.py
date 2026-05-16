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


if __name__ == "__main__":
    unittest.main()
