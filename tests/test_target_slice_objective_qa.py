#!/usr/bin/env python3
"""Unit tests for the Veyra target-slice objective acquisition QA wrapper."""

from __future__ import annotations

import json
import pathlib
import sys
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import target_slice_objective_qa  # noqa: E402


class TargetSliceObjectiveQaTests(unittest.TestCase):
    def test_report_requires_live_focus_interact_and_completion_evidence(self) -> None:
        report = {
            "schema": "v0.99-target-slice-objective-acquisition-qa",
            "scenario": "veyra-target-objective-acquisition",
            "passed": True,
            "scene": {"id": "veyra-reach-pilot", "path": "data/scenes/veyra_reach_pilot.scene.json"},
            "input": {
                "scriptName": "recorded-veyra-target-objective-v1",
                "framesToFocus": 58,
                "framesToInteract": 59,
            },
            "focus": {
                "acquired": True,
                "name": "Pilot Service Marker",
                "prompt": "Inspect Pilot Slice Marker",
                "distance": 1.1,
            },
            "interaction": {
                "triggered": True,
                "message": "Pilot slice marker inspected; target-slice objective gate confirmed.",
            },
            "riskyAction": {
                "id": "pilot-cache-risk-response",
                "attempted": True,
                "triggered": True,
                "interactableName": "Suspicious Cargo Cache",
                "framesToAction": 82,
                "message": "Suspicious cargo cache disturbed; local response triggered.",
            },
            "localResponse": {
                "stateId": "pilot-local-alerted",
                "active": True,
                "framesToResponse": 82,
                "summary": "response=pilot-local-alerted",
            },
            "exitRecovery": {
                "stateId": "pilot-escape-confirmed",
                "complete": True,
                "interactableName": "Pilot Escape Marker",
                "framesToExit": 111,
                "message": "Pilot escape route confirmed.",
            },
            "contact": {
                "attempted": True,
                "hit": True,
                "recoveredControl": True,
                "colliderName": "pilot-road-edge-collider",
                "framesToContact": 31,
                "framesToRecovery": 44,
                "hitCount": 1,
                "position": {"x": 1.4, "y": 0.0, "z": -0.25},
                "push": {"x": -0.08, "y": 0.0, "z": 0.0},
                "normal": {"x": -1.0, "y": 0.0, "z": 0.0},
            },
            "final": {
                "objectiveId": "inspect-pilot-service-marker",
                "objectiveComplete": True,
                "completionSummary": "complete=true role=target-slice-authored-objective targetObjective=inspect-pilot-service-marker consequence=pilot-marker-confirmed riskyAction=pilot-cache-risk-response complete=true responseState=pilot-local-alerted active=true exitRecovery=pilot-escape-confirmed complete=true",
                "completionEventText": "Target-slice objective confirmed: pilot-service-marker",
                "playerPosition": {"x": 0.4, "y": 0.0, "z": 1.55},
            },
            "error": "",
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            report_path = pathlib.Path(temp_dir) / "report.json"
            report_path.write_text(json.dumps(report), encoding="utf-8")

            loaded = target_slice_objective_qa.load_and_validate_report(report_path)

        self.assertEqual("veyra-target-objective-acquisition", loaded["scenario"])

    def test_report_rejects_missing_contact_evidence(self) -> None:
        report = {
            "schema": "v0.99-target-slice-objective-acquisition-qa",
            "scenario": "veyra-target-objective-acquisition",
            "passed": True,
            "scene": {"id": "veyra-reach-pilot", "path": "data/scenes/veyra_reach_pilot.scene.json"},
            "input": {
                "scriptName": "recorded-veyra-target-objective-v1",
                "framesToFocus": 58,
                "framesToInteract": 59,
            },
            "focus": {
                "acquired": True,
                "name": "Pilot Service Marker",
                "prompt": "Inspect Pilot Slice Marker",
                "distance": 1.1,
            },
            "interaction": {
                "triggered": True,
                "message": "Pilot slice marker inspected; target-slice objective gate confirmed.",
            },
            "riskyAction": {
                "id": "pilot-cache-risk-response",
                "attempted": True,
                "triggered": True,
                "interactableName": "Suspicious Cargo Cache",
                "framesToAction": 82,
                "message": "Suspicious cargo cache disturbed; local response triggered.",
            },
            "localResponse": {
                "stateId": "pilot-local-alerted",
                "active": True,
                "framesToResponse": 82,
                "summary": "response=pilot-local-alerted",
            },
            "exitRecovery": {
                "stateId": "pilot-escape-confirmed",
                "complete": True,
                "interactableName": "Pilot Escape Marker",
                "framesToExit": 111,
                "message": "Pilot escape route confirmed.",
            },
            "final": {
                "objectiveId": "inspect-pilot-service-marker",
                "objectiveComplete": True,
                "completionSummary": "complete=true role=target-slice-authored-objective targetObjective=inspect-pilot-service-marker consequence=pilot-marker-confirmed riskyAction=pilot-cache-risk-response complete=true responseState=pilot-local-alerted active=true exitRecovery=pilot-escape-confirmed complete=true",
                "completionEventText": "Target-slice objective confirmed: pilot-service-marker",
                "playerPosition": {"x": 0.4, "y": 0.0, "z": 1.55},
            },
            "error": "",
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            report_path = pathlib.Path(temp_dir) / "report.json"
            report_path.write_text(json.dumps(report), encoding="utf-8")

            with self.assertRaisesRegex(ValueError, "contact"):
                target_slice_objective_qa.load_and_validate_report(report_path)

    def test_report_rejects_contact_without_recovery(self) -> None:
        report = {
            "schema": "v0.99-target-slice-objective-acquisition-qa",
            "scenario": "veyra-target-objective-acquisition",
            "passed": True,
            "scene": {"id": "veyra-reach-pilot", "path": "data/scenes/veyra_reach_pilot.scene.json"},
            "input": {
                "scriptName": "recorded-veyra-target-objective-v1",
                "framesToFocus": 58,
                "framesToInteract": 59,
            },
            "focus": {
                "acquired": True,
                "name": "Pilot Service Marker",
                "prompt": "Inspect Pilot Slice Marker",
                "distance": 1.1,
            },
            "interaction": {
                "triggered": True,
                "message": "Pilot slice marker inspected; target-slice objective gate confirmed.",
            },
            "riskyAction": {
                "id": "pilot-cache-risk-response",
                "attempted": True,
                "triggered": True,
                "interactableName": "Suspicious Cargo Cache",
                "framesToAction": 82,
                "message": "Suspicious cargo cache disturbed; local response triggered.",
            },
            "localResponse": {
                "stateId": "pilot-local-alerted",
                "active": True,
                "framesToResponse": 82,
                "summary": "response=pilot-local-alerted",
            },
            "exitRecovery": {
                "stateId": "pilot-escape-confirmed",
                "complete": True,
                "interactableName": "Pilot Escape Marker",
                "framesToExit": 111,
                "message": "Pilot escape route confirmed.",
            },
            "contact": {
                "attempted": True,
                "hit": True,
                "recoveredControl": False,
                "colliderName": "pilot-road-edge-collider",
                "framesToContact": 31,
                "framesToRecovery": -1,
                "hitCount": 1,
                "position": {"x": 1.4, "y": 0.0, "z": -0.25},
                "push": {"x": -0.08, "y": 0.0, "z": 0.0},
                "normal": {"x": -1.0, "y": 0.0, "z": 0.0},
            },
            "final": {
                "objectiveId": "inspect-pilot-service-marker",
                "objectiveComplete": True,
                "completionSummary": "complete=true role=target-slice-authored-objective targetObjective=inspect-pilot-service-marker consequence=pilot-marker-confirmed riskyAction=pilot-cache-risk-response complete=true responseState=pilot-local-alerted active=true exitRecovery=pilot-escape-confirmed complete=true",
                "completionEventText": "Target-slice objective confirmed: pilot-service-marker",
                "playerPosition": {"x": 0.4, "y": 0.0, "z": 1.55},
            },
            "error": "",
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            report_path = pathlib.Path(temp_dir) / "report.json"
            report_path.write_text(json.dumps(report), encoding="utf-8")

            with self.assertRaisesRegex(ValueError, "recovery"):
                target_slice_objective_qa.load_and_validate_report(report_path)

    def test_report_rejects_direct_completion_without_focus(self) -> None:
        report = {
            "schema": "v0.99-target-slice-objective-acquisition-qa",
            "scenario": "veyra-target-objective-acquisition",
            "passed": True,
            "scene": {"id": "veyra-reach-pilot", "path": "data/scenes/veyra_reach_pilot.scene.json"},
            "input": {
                "scriptName": "recorded-veyra-target-objective-v1",
                "framesToFocus": -1,
                "framesToInteract": 1,
            },
            "focus": {"acquired": False, "name": "", "prompt": "", "distance": 0.0},
            "interaction": {"triggered": True, "message": "shortcut"},
            "riskyAction": {
                "id": "pilot-cache-risk-response",
                "attempted": True,
                "triggered": True,
                "interactableName": "Suspicious Cargo Cache",
                "framesToAction": 2,
                "message": "Suspicious cargo cache disturbed; local response triggered.",
            },
            "localResponse": {
                "stateId": "pilot-local-alerted",
                "active": True,
                "framesToResponse": 2,
                "summary": "response=pilot-local-alerted",
            },
            "exitRecovery": {
                "stateId": "pilot-escape-confirmed",
                "complete": True,
                "interactableName": "Pilot Escape Marker",
                "framesToExit": 3,
                "message": "Pilot escape route confirmed.",
            },
            "final": {
                "objectiveId": "inspect-pilot-service-marker",
                "objectiveComplete": True,
                "completionSummary": "complete=true targetObjective=inspect-pilot-service-marker riskyAction=pilot-cache-risk-response responseState=pilot-local-alerted exitRecovery=pilot-escape-confirmed",
                "completionEventText": "Target-slice objective confirmed: pilot-service-marker",
                "playerPosition": {"x": 1.0, "y": 0.0, "z": 2.0},
            },
            "error": "",
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            report_path = pathlib.Path(temp_dir) / "report.json"
            report_path.write_text(json.dumps(report), encoding="utf-8")

            with self.assertRaisesRegex(ValueError, "focus"):
                target_slice_objective_qa.load_and_validate_report(report_path)

    def test_report_rejects_ferry_office_wording(self) -> None:
        report = {
            "schema": "v0.99-target-slice-objective-acquisition-qa",
            "scenario": "veyra-target-objective-acquisition",
            "passed": True,
            "scene": {"id": "veyra-reach-pilot", "path": "data/scenes/veyra_reach_pilot.scene.json"},
            "input": {
                "scriptName": "recorded-veyra-target-objective-v1",
                "framesToFocus": 21,
                "framesToInteract": 22,
            },
            "focus": {
                "acquired": True,
                "name": "Pilot Service Marker",
                "prompt": "Inspect Pilot Slice Marker",
                "distance": 1.0,
            },
            "interaction": {"triggered": True, "message": "Ferry Office leaked"},
            "riskyAction": {
                "id": "pilot-cache-risk-response",
                "attempted": True,
                "triggered": True,
                "interactableName": "Suspicious Cargo Cache",
                "framesToAction": 21,
                "message": "Suspicious cargo cache disturbed; local response triggered.",
            },
            "localResponse": {
                "stateId": "pilot-local-alerted",
                "active": True,
                "framesToResponse": 21,
                "summary": "response=pilot-local-alerted",
            },
            "exitRecovery": {
                "stateId": "pilot-escape-confirmed",
                "complete": True,
                "interactableName": "Pilot Escape Marker",
                "framesToExit": 22,
                "message": "Pilot escape route confirmed.",
            },
            "contact": {
                "attempted": True,
                "hit": True,
                "recoveredControl": True,
                "colliderName": "pilot-road-edge-collider",
                "framesToContact": 10,
                "framesToRecovery": 20,
                "hitCount": 1,
                "position": {"x": 1.4, "y": 0.0, "z": 0.0},
                "push": {"x": -0.08, "y": 0.0, "z": 0.0},
                "normal": {"x": -1.0, "y": 0.0, "z": 0.0},
            },
            "final": {
                "objectiveId": "inspect-pilot-service-marker",
                "objectiveComplete": True,
                "completionSummary": "complete=true targetObjective=inspect-pilot-service-marker riskyAction=pilot-cache-risk-response responseState=pilot-local-alerted exitRecovery=pilot-escape-confirmed",
                "completionEventText": "Target-slice objective confirmed: pilot-service-marker",
                "playerPosition": {"x": 1.0, "y": 0.0, "z": 2.0},
            },
            "error": "",
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            report_path = pathlib.Path(temp_dir) / "report.json"
            report_path.write_text(json.dumps(report), encoding="utf-8")

            with self.assertRaisesRegex(ValueError, "Ferry Office"):
                target_slice_objective_qa.load_and_validate_report(report_path)

    def test_report_rejects_old_objective_only_evidence_without_risky_action(self) -> None:
        report = {
            "schema": "v0.99-target-slice-objective-acquisition-qa",
            "scenario": "veyra-target-objective-acquisition",
            "passed": True,
            "scene": {"id": "veyra-reach-pilot", "path": "data/scenes/veyra_reach_pilot.scene.json"},
            "input": {
                "scriptName": "recorded-veyra-target-objective-v1",
                "framesToFocus": 58,
                "framesToInteract": 59,
            },
            "focus": {
                "acquired": True,
                "name": "Pilot Service Marker",
                "prompt": "Inspect Pilot Slice Marker",
                "distance": 1.1,
            },
            "interaction": {
                "triggered": True,
                "message": "Pilot slice marker inspected; target-slice objective gate confirmed.",
            },
            "contact": {
                "attempted": True,
                "hit": True,
                "recoveredControl": True,
                "colliderName": "pilot-road-edge-collider",
                "framesToContact": 31,
                "framesToRecovery": 44,
                "hitCount": 1,
                "position": {"x": 1.4, "y": 0.0, "z": -0.25},
                "push": {"x": -0.08, "y": 0.0, "z": 0.0},
                "normal": {"x": -1.0, "y": 0.0, "z": 0.0},
            },
            "final": {
                "objectiveId": "inspect-pilot-service-marker",
                "objectiveComplete": True,
                "completionSummary": "complete=true role=target-slice-authored-objective targetObjective=inspect-pilot-service-marker consequence=pilot-marker-confirmed",
                "completionEventText": "Target-slice objective confirmed: pilot-service-marker",
                "playerPosition": {"x": 0.4, "y": 0.0, "z": 1.55},
            },
            "error": "",
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            report_path = pathlib.Path(temp_dir) / "report.json"
            report_path.write_text(json.dumps(report), encoding="utf-8")

            with self.assertRaisesRegex(ValueError, "riskyAction"):
                target_slice_objective_qa.load_and_validate_report(report_path)


if __name__ == "__main__":
    unittest.main()
