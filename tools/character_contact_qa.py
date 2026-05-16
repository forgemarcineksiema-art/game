#!/usr/bin/env python3
"""Run and validate the opt-in Ferry Office character/contact QA."""

from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import sys
from typing import Any


ROOT = pathlib.Path(__file__).resolve().parents[1]
SCENARIO = "ferry-office-character-contact"
SCHEMA = "v0.34-ferry-office-character-contact"


def default_exe_path() -> pathlib.Path:
    return ROOT / "build" / "windows-vs2022-debug-jolt" / "Debug" / "EngineApp.exe"


def default_scene_path() -> pathlib.Path:
    return ROOT / "data" / "scenes" / "ferry_office.scene.json"


def default_report_path(scenario: str = SCENARIO) -> pathlib.Path:
    return ROOT / "build" / "physics" / f"{scenario}-report.json"


def load_and_validate_report(report_path: pathlib.Path) -> dict[str, Any]:
    if not report_path.exists():
        raise FileNotFoundError(f"Character contact report was not created: {report_path}")

    report = json.loads(report_path.read_text(encoding="utf-8"))
    if report.get("schema") != SCHEMA:
        raise ValueError(f"Unexpected character contact report schema: {report.get('schema')}")
    if report.get("scenario") != SCENARIO:
        raise ValueError(f"Unexpected character contact scenario: {report.get('scenario')}")
    if report.get("passed") is not True:
        raise ValueError(f"Character contact report did not pass: {report.get('error', '')}")
    if report.get("staticColliderCount") != 9:
        raise ValueError(f"Unexpected Ferry Office static collider count: {report.get('staticColliderCount')}")

    probes = report.get("probes")
    if not isinstance(probes, list) or len(probes) < 7:
        raise ValueError("Character contact report is missing expected probes.")
    probe_names = {probe.get("name") for probe in probes}
    if "opened-gate-clear" not in probe_names:
        raise ValueError("Character contact report is missing opened-gate-clear probe.")
    for probe in probes:
        if probe.get("passed") is not True:
            raise ValueError(f"Character contact probe failed: {probe}")
    return report


def run_character_contact(exe: pathlib.Path, scene: pathlib.Path, report_path: pathlib.Path, scenario: str = SCENARIO) -> dict[str, Any]:
    if not exe.exists():
        raise FileNotFoundError(f"EngineApp executable was not found: {exe}")
    if scenario != SCENARIO:
        raise ValueError(f"Unsupported character contact scenario: {scenario}")

    report_path.parent.mkdir(parents=True, exist_ok=True)
    command = [
        str(exe),
        "--qa-physics-parity",
        scenario,
        "--scene",
        str(scene),
        "--qa-physics-report",
        str(report_path),
    ]
    print("Running:", " ".join(command))
    result = subprocess.run(
        command,
        cwd=ROOT,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    print(result.stdout, end="")
    if result.returncode != 0:
        raise RuntimeError(f"Character contact QA command failed with exit code {result.returncode}.")

    report = load_and_validate_report(report_path)
    print(
        "Character contact QA passed: "
        f"backend={report['backend']}, "
        f"probes={len(report['probes'])}, "
        f"report={report_path}"
    )
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", default=str(default_exe_path()), help="Path to an opt-in physics EngineApp.exe.")
    parser.add_argument("--scene", default=str(default_scene_path()), help="Path to Ferry Office scene JSON.")
    parser.add_argument("--scenario", default=SCENARIO, help="QA character contact scenario name.")
    parser.add_argument("--report-json", default=str(default_report_path()), help="Path for the JSON report.")
    args = parser.parse_args()

    try:
        run_character_contact(
            pathlib.Path(args.exe),
            pathlib.Path(args.scene),
            pathlib.Path(args.report_json),
            args.scenario,
        )
    except Exception as exception:
        print(f"Character contact QA failed: {exception}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
