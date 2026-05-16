#!/usr/bin/env python3
"""Run and validate the automated Ferry Office Service Call playthrough QA."""

from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import sys
from typing import Any


ROOT = pathlib.Path(__file__).resolve().parents[1]
SCENARIO = "ferry-office-service-call"
SCHEMA = "v0.32-ferry-office-playthrough-qa"
REQUIRED_FLAGS = [
    "manifestCollected",
    "serviceRouteUsed",
    "maintenanceBoxInspected",
    "powerRestored",
    "routeOpened",
    "serviceVehicleUsed",
    "dockRoadReached",
    "serviceRunConfirmed",
    "ferryOfficeJobComplete",
]
REQUIRED_RUNTIME_STEPS = [
    "serviceVehicleRuntime",
    "dockRoadRuntimeCheckpoint",
    "serviceVehicleRuntimeExit",
]


def default_exe_path() -> pathlib.Path:
    return ROOT / "build" / "windows-vs2022-debug" / "Debug" / "EngineApp.exe"


def default_scene_path() -> pathlib.Path:
    return ROOT / "data" / "scenes" / "ferry_office.scene.json"


def default_report_path(scenario: str = SCENARIO) -> pathlib.Path:
    return ROOT / "build" / "playthroughs" / f"{scenario}-report.json"


def load_and_validate_report(report_path: pathlib.Path) -> dict[str, Any]:
    if not report_path.exists():
        raise FileNotFoundError(f"Playthrough report was not created: {report_path}")

    report = json.loads(report_path.read_text(encoding="utf-8"))
    if report.get("schema") != SCHEMA:
        raise ValueError(f"Unexpected playthrough report schema: {report.get('schema')}")
    if report.get("scenario") != SCENARIO:
        raise ValueError(f"Unexpected playthrough scenario: {report.get('scenario')}")
    if report.get("passed") is not True:
        raise ValueError(f"Playthrough report did not pass: {report.get('error', '')}")

    final = report.get("final", {})
    flags = final.get("flags", {})
    for flag in REQUIRED_FLAGS:
        if flags.get(flag) is not True:
            raise ValueError(f"Playthrough report is missing required completed flag: {flag}")

    if final.get("phase") != "complete":
        raise ValueError(f"Playthrough final phase was not complete: {final.get('phase')}")

    steps = report.get("steps", [])
    step_names = {step.get("name") for step in steps if isinstance(step, dict)}
    for step_name in REQUIRED_RUNTIME_STEPS:
        if step_name not in step_names:
            raise ValueError(f"Playthrough report is missing required runtime vehicle step: {step_name}")

    return report


def run_playthrough(exe: pathlib.Path, scene: pathlib.Path, report_path: pathlib.Path, scenario: str = SCENARIO) -> dict[str, Any]:
    if not exe.exists():
        raise FileNotFoundError(f"EngineApp executable was not found: {exe}")
    if scenario != SCENARIO:
        raise ValueError(f"Unsupported playthrough scenario: {scenario}")

    report_path.parent.mkdir(parents=True, exist_ok=True)
    command = [
        str(exe),
        "--qa-playthrough",
        scenario,
        "--scene",
        str(scene),
        "--qa-playthrough-report",
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
        raise RuntimeError(f"Playthrough QA command failed with exit code {result.returncode}.")

    report = load_and_validate_report(report_path)
    print(
        "Playthrough QA passed: "
        f"phase={report['final']['phase']}, "
        f"events={report['final']['eventCount']}, "
        f"report={report_path}"
    )
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", default=str(default_exe_path()), help="Path to EngineApp.exe.")
    parser.add_argument("--scene", default=str(default_scene_path()), help="Path to Ferry Office scene JSON.")
    parser.add_argument("--scenario", default=SCENARIO, help="QA playthrough scenario name.")
    parser.add_argument("--report-json", default=str(default_report_path()), help="Path for the JSON report.")
    args = parser.parse_args()

    try:
        run_playthrough(
            pathlib.Path(args.exe),
            pathlib.Path(args.scene),
            pathlib.Path(args.report_json),
            args.scenario,
        )
    except Exception as exception:
        print(f"Playthrough QA failed: {exception}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
