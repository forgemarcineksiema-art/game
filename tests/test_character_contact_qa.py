#!/usr/bin/env python3
"""Unit tests for the Ferry Office character/contact QA wrapper."""

from __future__ import annotations

import json
import pathlib
import sys
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import character_contact_qa  # noqa: E402


class CharacterContactQaTests(unittest.TestCase):
    def test_report_validation_accepts_passed_contact_probes(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            report_path = pathlib.Path(temp) / "character-contact.json"
            report_path.write_text(
                json.dumps(
                    {
                        "schema": character_contact_qa.SCHEMA,
                        "scenario": character_contact_qa.SCENARIO,
                        "passed": True,
                        "backend": "jolt",
                        "staticColliderCount": 10,
                        "probes": [
                            {"name": "dock-floor-grounding", "passed": True},
                            {"name": "office-back-wall-blocked", "passed": True},
                            {"name": "service-gate-blocked", "passed": True},
                            {"name": "service-barrier-blocked", "passed": True},
                            {"name": "dock-lane-clear", "passed": True},
                            {"name": "office-corner-pushout", "passed": True},
                            {"name": "opened-gate-clear", "passed": True},
                        ],
                        "error": "",
                    }
                ),
                encoding="utf-8",
            )

            report = character_contact_qa.load_and_validate_report(report_path)

        self.assertEqual("jolt", report["backend"])

    def test_report_validation_rejects_missing_opened_gate_probe(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            report_path = pathlib.Path(temp) / "character-contact.json"
            report_path.write_text(
                json.dumps(
                    {
                        "schema": character_contact_qa.SCHEMA,
                        "scenario": character_contact_qa.SCENARIO,
                        "passed": True,
                        "backend": "jolt",
                        "staticColliderCount": 10,
                        "probes": [
                            {"name": "dock-floor-grounding", "passed": True},
                            {"name": "office-back-wall-blocked", "passed": True},
                            {"name": "service-gate-blocked", "passed": True},
                            {"name": "service-barrier-blocked", "passed": True},
                            {"name": "dock-lane-clear", "passed": True},
                            {"name": "office-corner-pushout", "passed": True},
                            {"name": "extra-clear", "passed": True},
                        ],
                        "error": "",
                    }
                ),
                encoding="utf-8",
            )

            with self.assertRaisesRegex(ValueError, "opened-gate-clear"):
                character_contact_qa.load_and_validate_report(report_path)


if __name__ == "__main__":
    unittest.main()
