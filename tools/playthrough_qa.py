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
    "dockRoadRelayReset",
    "dockRoadRelayLogged",
    "dockRoadClearanceTagged",
    "harborPartsPickedUp",
    "harborPartsDelivered",
    "ferryOfficeBoardUpdated",
    "ferryOfficeHandoffFiled",
    "stormPumpReset",
    "stormPumpTicketClosed",
    "lowDockDrainCleared",
    "lowDockDrainLogged",
]
REQUIRED_RUNTIME_STEPS = [
    "serviceVehicleRuntime",
    "dockRoadRuntimeCheckpoint",
    "serviceVehicleRuntimeExit",
    "dockRoadRelayReset",
    "dockRoadRelayLogged",
    "dockRoadClearanceTagged",
    "harborPartsPickedUp",
    "harborPartsDelivered",
    "ferryOfficeBoardUpdated",
    "ferryOfficeHandoffFiled",
    "stormPumpReset",
    "stormPumpTicketClosed",
    "lowDockDrainCleared",
    "lowDockDrainLogged",
]


def default_exe_path() -> pathlib.Path:
    return ROOT / "build" / "windows-vs2022-debug" / "Debug" / "EngineApp.exe"


def default_scene_path() -> pathlib.Path:
    return ROOT / "data" / "scenes" / "ferry_office.scene.json"


def default_report_path(scenario: str = SCENARIO) -> pathlib.Path:
    return ROOT / "build" / "playthroughs" / f"{scenario}-report.json"


def load_and_validate_report(report_path: pathlib.Path, expected_vehicle_runtime: str | None = None) -> dict[str, Any]:
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

    vehicle_runtime = report.get("vehicleRuntime")
    if not isinstance(vehicle_runtime, dict):
        raise ValueError("Playthrough report is missing vehicle runtime evidence.")
    backend = vehicle_runtime.get("backend")
    if backend not in {"deterministic", "jolt"}:
        raise ValueError(f"Playthrough vehicle runtime backend is invalid: {backend}")
    if expected_vehicle_runtime is not None and backend != expected_vehicle_runtime:
        raise ValueError(
            f"Playthrough vehicle runtime backend was {backend}, expected {expected_vehicle_runtime}."
        )
    if vehicle_runtime.get("fallbackUsed") is True:
        raise ValueError(f"Playthrough vehicle runtime used fallback: {vehicle_runtime}")
    if vehicle_runtime.get("hitBounds") is True:
        raise ValueError(f"Playthrough vehicle runtime hit authored bounds: {vehicle_runtime}")
    if int(vehicle_runtime.get("framesToCheckpoint", -1)) <= 0:
        raise ValueError(f"Playthrough vehicle runtime is missing checkpoint timing: {vehicle_runtime}")

    return report


def run_playthrough(
    exe: pathlib.Path,
    scene: pathlib.Path,
    report_path: pathlib.Path,
    scenario: str = SCENARIO,
    vehicle_runtime: str = "deterministic",
) -> dict[str, Any]:
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
    if vehicle_runtime != "deterministic":
        command.extend(["--vehicle-runtime", vehicle_runtime])
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

    report = load_and_validate_report(report_path, vehicle_runtime)
    runtime = report["vehicleRuntime"]
    print(
        "Playthrough QA passed: "
        f"phase={report['final']['phase']}, "
        f"events={report['final']['eventCount']}, "
        f"vehicleRuntime={runtime['backend']}, "
        f"framesToCheckpoint={runtime['framesToCheckpoint']}, "
        f"report={report_path}"
    )
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", default=str(default_exe_path()), help="Path to EngineApp.exe.")
    parser.add_argument("--scene", default=str(default_scene_path()), help="Path to Ferry Office scene JSON.")
    parser.add_argument("--scenario", default=SCENARIO, help="QA playthrough scenario name.")
    parser.add_argument("--report-json", default=str(default_report_path()), help="Path for the JSON report.")
    parser.add_argument(
        "--vehicle-runtime",
        choices=("deterministic", "jolt"),
        default="deterministic",
        help="Vehicle runtime to use for the scripted service-vehicle loop.",
    )
    args = parser.parse_args()

    try:
        run_playthrough(
            pathlib.Path(args.exe),
            pathlib.Path(args.scene),
            pathlib.Path(args.report_json),
            args.scenario,
            args.vehicle_runtime,
        )
    except Exception as exception:
        print(f"Playthrough QA failed: {exception}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
