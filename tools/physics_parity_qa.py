#!/usr/bin/env python3
"""Run and validate the opt-in Ferry Office physics parity QA."""

from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import sys
from typing import Any


ROOT = pathlib.Path(__file__).resolve().parents[1]
SCENARIO = "ferry-office-collision"
SCHEMA = "v0.33-ferry-office-physics-parity"


def default_exe_path() -> pathlib.Path:
    return ROOT / "build" / "windows-vs2022-debug-jolt" / "Debug" / "EngineApp.exe"


def default_scene_path() -> pathlib.Path:
    return ROOT / "data" / "scenes" / "ferry_office.scene.json"


def default_report_path(scenario: str = SCENARIO) -> pathlib.Path:
    return ROOT / "build" / "physics" / f"{scenario}-parity-report.json"


def _require_passed_probes(report: dict[str, Any], key: str) -> None:
    probes = report.get(key)
    if not isinstance(probes, list) or not probes:
        raise ValueError(f"Physics parity report is missing probes: {key}")
    for probe in probes:
        if probe.get("passed") is not True:
            raise ValueError(f"Physics parity probe failed in {key}: {probe}")


def load_and_validate_report(report_path: pathlib.Path) -> dict[str, Any]:
    if not report_path.exists():
        raise FileNotFoundError(f"Physics parity report was not created: {report_path}")

    report = json.loads(report_path.read_text(encoding="utf-8"))
    if report.get("schema") != SCHEMA:
        raise ValueError(f"Unexpected physics parity report schema: {report.get('schema')}")
    if report.get("scenario") != SCENARIO:
        raise ValueError(f"Unexpected physics parity scenario: {report.get('scenario')}")
    if report.get("passed") is not True:
        raise ValueError(f"Physics parity report did not pass: {report.get('error', '')}")
    if report.get("staticColliderCount") != 9:
        raise ValueError(f"Unexpected Ferry Office static collider count: {report.get('staticColliderCount')}")

    _require_passed_probes(report, "floorProbes")
    _require_passed_probes(report, "raycastProbes")
    _require_passed_probes(report, "overlapProbes")
    return report


def run_physics_parity(exe: pathlib.Path, scene: pathlib.Path, report_path: pathlib.Path, scenario: str = SCENARIO) -> dict[str, Any]:
    if not exe.exists():
        raise FileNotFoundError(f"EngineApp executable was not found: {exe}")
    if scenario != SCENARIO:
        raise ValueError(f"Unsupported physics parity scenario: {scenario}")

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
        raise RuntimeError(f"Physics parity QA command failed with exit code {result.returncode}.")

    report = load_and_validate_report(report_path)
    print(
        "Physics parity QA passed: "
        f"backend={report['backend']}, "
        f"floor={len(report['floorProbes'])}, "
        f"raycast={len(report['raycastProbes'])}, "
        f"overlap={len(report['overlapProbes'])}, "
        f"report={report_path}"
    )
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", default=str(default_exe_path()), help="Path to a Jolt-enabled EngineApp.exe.")
    parser.add_argument("--scene", default=str(default_scene_path()), help="Path to Ferry Office scene JSON.")
    parser.add_argument("--scenario", default=SCENARIO, help="QA physics parity scenario name.")
    parser.add_argument("--report-json", default=str(default_report_path()), help="Path for the JSON report.")
    args = parser.parse_args()

    try:
        run_physics_parity(
            pathlib.Path(args.exe),
            pathlib.Path(args.scene),
            pathlib.Path(args.report_json),
            args.scenario,
        )
    except Exception as exception:
        print(f"Physics parity QA failed: {exception}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
