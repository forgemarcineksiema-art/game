#!/usr/bin/env python3
"""Unit tests for the Ferry Office physics parity QA wrapper."""

from __future__ import annotations

import json
import pathlib
import tempfile
import unittest

import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import physics_parity_qa  # noqa: E402


class PhysicsParityToolTests(unittest.TestCase):
    def test_report_validation_accepts_passed_ferry_office_parity(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            report_path = pathlib.Path(temp) / "physics-parity.json"
            report_path.write_text(
                json.dumps(
                    {
                        "schema": physics_parity_qa.SCHEMA,
                        "scenario": physics_parity_qa.SCENARIO,
                        "passed": True,
                        "backend": "jolt",
                        "staticColliderCount": 9,
                        "floorProbes": [{"name": "dock-start", "passed": True}],
                        "raycastProbes": [{"name": "service-gate-front", "passed": True}],
                        "overlapProbes": [{"name": "service-gate-blocked", "passed": True}],
                        "error": "",
                    }
                ),
                encoding="utf-8",
            )

            report = physics_parity_qa.load_and_validate_report(report_path)

        self.assertEqual("jolt", report["backend"])

    def test_report_validation_rejects_failed_probe(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            report_path = pathlib.Path(temp) / "physics-parity.json"
            report_path.write_text(
                json.dumps(
                    {
                        "schema": physics_parity_qa.SCHEMA,
                        "scenario": physics_parity_qa.SCENARIO,
                        "passed": False,
                        "backend": "jolt",
                        "staticColliderCount": 9,
                        "floorProbes": [{"name": "dock-start", "passed": True}],
                        "raycastProbes": [{"name": "service-gate-front", "passed": False}],
                        "overlapProbes": [{"name": "service-gate-blocked", "passed": True}],
                        "error": "Raycast parity failed.",
                    }
                ),
                encoding="utf-8",
            )

            with self.assertRaises(ValueError):
                physics_parity_qa.load_and_validate_report(report_path)


if __name__ == "__main__":
    unittest.main()
