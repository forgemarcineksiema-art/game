#!/usr/bin/env python3
"""Run and validate the opt-in Ferry Office vehicle feasibility QA."""

from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import sys
from typing import Any


ROOT = pathlib.Path(__file__).resolve().parents[1]
SCENARIO = "ferry-office-vehicle-feasibility"
SCHEMA = "v0.35-ferry-office-vehicle-feasibility"


def default_exe_path() -> pathlib.Path:
    return ROOT / "build" / "windows-vs2022-debug-jolt" / "Debug" / "EngineApp.exe"


def default_scene_path() -> pathlib.Path:
    return ROOT / "data" / "scenes" / "ferry_office.scene.json"


def default_report_path(scenario: str = SCENARIO) -> pathlib.Path:
    return ROOT / "build" / "physics" / f"{scenario}-report.json"


def load_and_validate_report(report_path: pathlib.Path) -> dict[str, Any]:
    if not report_path.exists():
        raise FileNotFoundError(f"Vehicle physics report was not created: {report_path}")

    report = json.loads(report_path.read_text(encoding="utf-8"))
    if report.get("schema") != SCHEMA:
        raise ValueError(f"Unexpected vehicle physics report schema: {report.get('schema')}")
    if report.get("scenario") != SCENARIO:
        raise ValueError(f"Unexpected vehicle physics scenario: {report.get('scenario')}")
    if report.get("passed") is not True:
        raise ValueError(f"Vehicle physics report did not pass: {report.get('error', '')}")
    if report.get("backend") != "jolt":
        raise ValueError(f"Vehicle physics QA must run against opt-in Jolt backend, got: {report.get('backend')}")

    vehicle = report.get("vehicle")
    if not isinstance(vehicle, dict) or vehicle.get("id") != "service-yard-vehicle":
        raise ValueError(f"Vehicle physics report has unexpected vehicle block: {vehicle}")

    samples = report.get("samples")
    if not isinstance(samples, list) or len(samples) < 4:
        raise ValueError("Vehicle physics report is missing expected samples.")
    for sample in samples:
        if sample.get("passed") is not True:
            raise ValueError(f"Vehicle physics sample failed: {sample}")
        if int(sample.get("wheelContactCount", 0)) < 2:
            raise ValueError(f"Vehicle physics sample has too few wheel contacts: {sample}")
        if sample.get("outOfBounds") is True:
            raise ValueError(f"Vehicle physics sample left authored bounds: {sample}")

    final_state = report.get("final")
    if not isinstance(final_state, dict) or float(final_state.get("speed", 0.0)) < 0.0:
        raise ValueError(f"Vehicle physics report has invalid final state: {final_state}")
    if report.get("recommendation") not in {"promote", "defer"}:
        raise ValueError(f"Vehicle physics report has invalid recommendation: {report.get('recommendation')}")
    return report


def run_vehicle_physics(exe: pathlib.Path, scene: pathlib.Path, report_path: pathlib.Path, scenario: str = SCENARIO) -> dict[str, Any]:
    if not exe.exists():
        raise FileNotFoundError(f"EngineApp executable was not found: {exe}")
    if scenario != SCENARIO:
        raise ValueError(f"Unsupported vehicle physics scenario: {scenario}")

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
        raise RuntimeError(f"Vehicle physics QA command failed with exit code {result.returncode}.")

    report = load_and_validate_report(report_path)
    print(
        "Vehicle physics QA passed: "
        f"backend={report['backend']}, "
        f"samples={len(report['samples'])}, "
        f"recommendation={report['recommendation']}, "
        f"report={report_path}"
    )
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", default=str(default_exe_path()), help="Path to a Jolt-enabled EngineApp.exe.")
    parser.add_argument("--scene", default=str(default_scene_path()), help="Path to Ferry Office scene JSON.")
    parser.add_argument("--scenario", default=SCENARIO, help="QA vehicle physics scenario name.")
    parser.add_argument("--report-json", default=str(default_report_path()), help="Path for the JSON report.")
    args = parser.parse_args()

    try:
        run_vehicle_physics(
            pathlib.Path(args.exe),
            pathlib.Path(args.scene),
            pathlib.Path(args.report_json),
            args.scenario,
        )
    except Exception as exception:
        print(f"Vehicle physics QA failed: {exception}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
