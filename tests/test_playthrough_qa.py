#!/usr/bin/env python3
"""Unit tests for the automated Ferry Office playthrough QA wrapper."""

from __future__ import annotations

import importlib.util
import json
import pathlib
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
TOOL_PATH = ROOT / "tools" / "playthrough_qa.py"

spec = importlib.util.spec_from_file_location("playthrough_qa", TOOL_PATH)
playthrough_qa = importlib.util.module_from_spec(spec)
assert spec.loader is not None
spec.loader.exec_module(playthrough_qa)  # type: ignore[union-attr]


class PlaythroughQaTests(unittest.TestCase):
    def test_default_report_path_lives_under_build_playthroughs(self) -> None:
        report_path = playthrough_qa.default_report_path("ferry-office-service-call")

        self.assertEqual(ROOT / "build" / "playthroughs" / "ferry-office-service-call-report.json", report_path)

    def test_validate_report_requires_completed_ferry_office_flags(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            report_path = pathlib.Path(temp_dir) / "report.json"
            report_path.write_text(
                json.dumps(
                    {
                        "schema": "v0.32-ferry-office-playthrough-qa",
                        "scenario": "ferry-office-service-call",
                        "passed": True,
                        "steps": [
                            {"name": "serviceVehicleRuntime", "passed": True},
                            {"name": "dockRoadRuntimeCheckpoint", "passed": True},
                            {"name": "serviceVehicleRuntimeExit", "passed": True},
                            {"name": "dockRoadRelayReset", "passed": True},
                            {"name": "dockRoadRelayLogged", "passed": True},
                            {"name": "dockRoadClearanceTagged", "passed": True},
                            {"name": "harborPartsPickedUp", "passed": True},
                            {"name": "harborPartsDelivered", "passed": True},
                            {"name": "ferryOfficeBoardUpdated", "passed": True},
                        ],
                        "vehicleRuntime": {
                            "requested": "deterministic",
                            "backend": "deterministic",
                            "framesToCheckpoint": 139,
                            "hitBounds": False,
                            "fallbackUsed": False,
                        },
                        "final": {
                            "phase": "complete",
                            "flags": {
                                "manifestCollected": True,
                                "serviceRouteUsed": True,
                                "maintenanceBoxInspected": True,
                                "powerRestored": True,
                                "routeOpened": True,
                                "serviceVehicleUsed": True,
                                "dockRoadReached": True,
                                "serviceRunConfirmed": True,
                                "ferryOfficeJobComplete": True,
                                "dockRoadRelayReset": True,
                                "dockRoadRelayLogged": True,
                                "dockRoadClearanceTagged": True,
                                "harborPartsPickedUp": True,
                                "harborPartsDelivered": True,
                                "ferryOfficeBoardUpdated": True,
                            },
                        },
                    }
                )
                + "\n",
                encoding="utf-8",
            )

            report = playthrough_qa.load_and_validate_report(report_path)

            self.assertEqual("complete", report["final"]["phase"])
            self.assertEqual("deterministic", report["vehicleRuntime"]["backend"])

    def test_validate_report_rejects_missing_job_completion(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            report_path = pathlib.Path(temp_dir) / "report.json"
            report_path.write_text(
                json.dumps(
                    {
                        "schema": "v0.32-ferry-office-playthrough-qa",
                        "scenario": "ferry-office-service-call",
                        "passed": True,
                        "final": {
                            "phase": "confirmServiceRun",
                            "flags": {
                                "manifestCollected": True,
                                "serviceRouteUsed": True,
                                "maintenanceBoxInspected": True,
                                "powerRestored": True,
                                "routeOpened": True,
                                "serviceVehicleUsed": True,
                                "dockRoadReached": True,
                                "serviceRunConfirmed": True,
                                "ferryOfficeJobComplete": False,
                                "dockRoadRelayReset": False,
                                "dockRoadRelayLogged": False,
                                "dockRoadClearanceTagged": False,
                                "harborPartsPickedUp": False,
                                "harborPartsDelivered": False,
                                "ferryOfficeBoardUpdated": False,
                            },
                        },
                    }
                )
                + "\n",
                encoding="utf-8",
            )

            with self.assertRaisesRegex(ValueError, "ferryOfficeJobComplete"):
                playthrough_qa.load_and_validate_report(report_path)

    def test_validate_report_rejects_missing_runtime_vehicle_steps(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            report_path = pathlib.Path(temp_dir) / "report.json"
            report_path.write_text(
                json.dumps(
                    {
                        "schema": "v0.32-ferry-office-playthrough-qa",
                        "scenario": "ferry-office-service-call",
                        "passed": True,
                        "steps": [
                            {"name": "collectManifest", "passed": True},
                            {"name": "serviceRoute", "passed": True},
                            {"name": "maintenancePower", "passed": True},
                            {"name": "openServiceGate", "passed": True},
                        ],
                        "final": {
                            "phase": "complete",
                            "flags": {
                                "manifestCollected": True,
                                "serviceRouteUsed": True,
                                "maintenanceBoxInspected": True,
                                "powerRestored": True,
                                "routeOpened": True,
                                "serviceVehicleUsed": True,
                                "dockRoadReached": True,
                                "serviceRunConfirmed": True,
                                "ferryOfficeJobComplete": True,
                                "dockRoadRelayReset": True,
                                "dockRoadRelayLogged": True,
                                "dockRoadClearanceTagged": True,
                                "harborPartsPickedUp": True,
                                "harborPartsDelivered": True,
                                "ferryOfficeBoardUpdated": True,
                            },
                        },
                    }
                )
                + "\n",
                encoding="utf-8",
            )

            with self.assertRaisesRegex(ValueError, "runtime vehicle"):
                playthrough_qa.load_and_validate_report(report_path)

    def test_validate_report_rejects_missing_vehicle_runtime_block(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            report_path = pathlib.Path(temp_dir) / "report.json"
            report_path.write_text(
                json.dumps(
                    {
                        "schema": "v0.32-ferry-office-playthrough-qa",
                        "scenario": "ferry-office-service-call",
                        "passed": True,
                        "steps": [
                            {"name": "serviceVehicleRuntime", "passed": True},
                            {"name": "dockRoadRuntimeCheckpoint", "passed": True},
                            {"name": "serviceVehicleRuntimeExit", "passed": True},
                            {"name": "dockRoadRelayReset", "passed": True},
                            {"name": "dockRoadRelayLogged", "passed": True},
                            {"name": "dockRoadClearanceTagged", "passed": True},
                            {"name": "harborPartsPickedUp", "passed": True},
                            {"name": "harborPartsDelivered", "passed": True},
                            {"name": "ferryOfficeBoardUpdated", "passed": True},
                        ],
                        "final": {
                            "phase": "complete",
                            "flags": {
                                "manifestCollected": True,
                                "serviceRouteUsed": True,
                                "maintenanceBoxInspected": True,
                                "powerRestored": True,
                                "routeOpened": True,
                                "serviceVehicleUsed": True,
                                "dockRoadReached": True,
                                "serviceRunConfirmed": True,
                                "ferryOfficeJobComplete": True,
                                "dockRoadRelayReset": True,
                                "dockRoadRelayLogged": True,
                                "dockRoadClearanceTagged": True,
                                "harborPartsPickedUp": True,
                                "harborPartsDelivered": True,
                                "ferryOfficeBoardUpdated": True,
                            },
                        },
                    }
                )
                + "\n",
                encoding="utf-8",
            )

            with self.assertRaisesRegex(ValueError, "vehicle runtime"):
                playthrough_qa.load_and_validate_report(report_path)


if __name__ == "__main__":
    unittest.main()
