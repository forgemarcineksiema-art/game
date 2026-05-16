#!/usr/bin/env python3
"""Run and validate the opt-in Ferry Office vehicle runtime comparison QA."""

from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import sys
from typing import Any


ROOT = pathlib.Path(__file__).resolve().parents[1]
SCENARIO = "ferry-office-vehicle-runtime-comparison"
SCHEMA = "v0.36-ferry-office-vehicle-runtime-comparison"
MAX_POSITION_DELTA = 4.0
MAX_YAW_DELTA_DEGREES = 130.0
MAX_SPEED_DELTA = 5.0
MAX_JOLT_ROUTE_FRAMES = 240
MAX_OBSTACLE_PROGRESS_DELTA = 4.0
REQUIRED_CONTROL_CHECKS = {
    "tapThrottleCoast",
    "brakeStopsForwardMotion",
    "reverseMovesBackward",
    "reverseCoastSettles",
}
REQUIRED_DRIVING_FEEL_CHECKS = {
    "routeFramesToCheckpoint",
    "routeMaxLateralDeviation",
    "brakeStopDistance",
    "reverseDistance",
    "steeringYawResponse",
    "cameraYawLag",
}


def default_exe_path() -> pathlib.Path:
    return ROOT / "build" / "windows-vs2022-debug-jolt" / "Debug" / "EngineApp.exe"


def default_scene_path() -> pathlib.Path:
    return ROOT / "data" / "scenes" / "ferry_office.scene.json"


def default_report_path(scenario: str = SCENARIO) -> pathlib.Path:
    return ROOT / "build" / "physics" / f"{scenario}-report.json"


def _require_samples(report: dict[str, Any], key: str) -> list[dict[str, Any]]:
    section = report.get(key)
    if not isinstance(section, dict):
        raise ValueError(f"Vehicle runtime report is missing section: {key}")
    samples = section.get("samples")
    if not isinstance(samples, list) or not samples:
        raise ValueError(f"Vehicle runtime report is missing samples: {key}")
    for sample in samples:
        if sample.get("passed") is not True:
            raise ValueError(f"Vehicle runtime sample failed in {key}: {sample}")
        if key == "adapter" and int(sample.get("wheelContactCount", 0)) < 2:
            raise ValueError(f"Vehicle runtime adapter sample has too few wheel contacts: {sample}")
        if sample.get("outOfBounds") is True:
            raise ValueError(f"Vehicle runtime sample left authored bounds in {key}: {sample}")
    return samples


def _require_control_checks(report: dict[str, Any]) -> list[dict[str, Any]]:
    checks = report.get("controlChecks")
    if not isinstance(checks, list) or not checks:
        raise ValueError("Vehicle runtime report is missing controls-focused control checks.")

    names = set()
    for check in checks:
        if not isinstance(check, dict):
            raise ValueError(f"Vehicle runtime control check is invalid: {check}")
        names.add(str(check.get("name", "")))
        if check.get("passed") is not True:
            raise ValueError(f"Vehicle runtime control check failed: {check}")
        if "speed" not in check or "distance" not in check or "frameIndex" not in check:
            raise ValueError(f"Vehicle runtime control check is missing telemetry: {check}")

    missing = REQUIRED_CONTROL_CHECKS - names
    if missing:
        raise ValueError(f"Vehicle runtime report is missing required control checks: {sorted(missing)}")
    return checks


def _require_route_checks(report: dict[str, Any]) -> list[dict[str, Any]]:
    checks = report.get("routeChecks")
    if not isinstance(checks, list) or len(checks) < 2:
        raise ValueError("Vehicle runtime report is missing service-run route checks.")

    backends = set()
    for check in checks:
        if not isinstance(check, dict):
            raise ValueError(f"Vehicle runtime route check is invalid: {check}")
        backends.add(str(check.get("backend", "")))
        if check.get("passed") is not True or check.get("checkpointReached") is not True:
            raise ValueError(f"Vehicle runtime route check failed: {check}")
        if check.get("hitBounds") is True:
            raise ValueError(f"Vehicle runtime route check hit authored bounds: {check}")
        if int(check.get("framesToCheckpoint", -1)) <= 0:
            raise ValueError(f"Vehicle runtime route check is missing checkpoint timing: {check}")
        if check.get("backend") == "jolt" and int(check.get("framesToCheckpoint", 9999)) > MAX_JOLT_ROUTE_FRAMES:
            raise ValueError(f"Vehicle runtime route check exceeded Jolt route frame budget: {check}")
        if "minDistanceToCheckpoint" not in check or "finalPosition" not in check or "finalYawDegrees" not in check:
            raise ValueError(f"Vehicle runtime route check is missing telemetry: {check}")

    if "deterministic" not in backends or "jolt" not in backends:
        raise ValueError(f"Vehicle runtime route checks must include deterministic and jolt backends: {sorted(backends)}")
    return checks


def _require_obstacle_checks(report: dict[str, Any]) -> list[dict[str, Any]]:
    checks = report.get("obstacleChecks")
    if not isinstance(checks, list) or len(checks) < 2:
        raise ValueError("Vehicle runtime report is missing obstacle-proxy steering checks.")

    backends = set()
    for check in checks:
        if not isinstance(check, dict):
            raise ValueError(f"Vehicle runtime obstacle check is invalid: {check}")
        backends.add(str(check.get("backend", "")))
        if check.get("passed") is not True or check.get("clearedObstacleProxy") is not True:
            raise ValueError(f"Vehicle runtime obstacle check failed: {check}")
        if check.get("hitBounds") is True:
            raise ValueError(f"Vehicle runtime obstacle check hit authored bounds: {check}")
        if check.get("collisionBacked") is not True or check.get("obstacleCollisionClear") is not True:
            raise ValueError(f"Vehicle runtime obstacle check is missing collision-backed clearance: {check}")
        if int(check.get("obstacleOverlapFrames", -1)) != 0:
            raise ValueError(f"Vehicle runtime obstacle check overlapped the collision probe: {check}")
        if int(check.get("frameCount", -1)) <= 0:
            raise ValueError(f"Vehicle runtime obstacle check is missing frame count: {check}")
        if (
            "maxLateralOffset" not in check
            or "minDistanceToObstacle" not in check
            or "minCollisionClearance" not in check
            or "finalPosition" not in check
            or "maxCameraYawDeltaDegrees" not in check
            or "finalCameraYawDegrees" not in check
        ):
            raise ValueError(f"Vehicle runtime obstacle check is missing telemetry: {check}")

    if "deterministic" not in backends or "jolt" not in backends:
        raise ValueError(f"Vehicle runtime obstacle checks must include deterministic and jolt backends: {sorted(backends)}")

    by_backend = {str(check.get("backend", "")): check for check in checks}
    deterministic_x = float(by_backend["deterministic"]["finalPosition"][0])
    jolt_x = float(by_backend["jolt"]["finalPosition"][0])
    progress_delta = abs(deterministic_x - jolt_x)
    if progress_delta > MAX_OBSTACLE_PROGRESS_DELTA:
        raise ValueError(
            "Vehicle runtime obstacle progress delta exceeded threshold: "
            f"deterministicX={deterministic_x:.2f}, joltX={jolt_x:.2f}, delta={progress_delta:.2f}"
        )
    return checks


def _require_driving_feel_checks(report: dict[str, Any]) -> list[dict[str, Any]]:
    checks = report.get("drivingFeelChecks")
    if not isinstance(checks, list) or not checks:
        raise ValueError("Vehicle runtime report is missing driving-feel checks.")

    by_backend: dict[str, set[str]] = {}
    for check in checks:
        if not isinstance(check, dict):
            raise ValueError(f"Vehicle runtime driving-feel check is invalid: {check}")
        backend = str(check.get("backend", ""))
        name = str(check.get("name", ""))
        by_backend.setdefault(backend, set()).add(name)
        if check.get("passed") is not True:
            raise ValueError(f"Vehicle runtime driving-feel check failed: {check}")
        for key in ("value", "minValue", "maxValue", "units"):
            if key not in check:
                raise ValueError(f"Vehicle runtime driving-feel check is missing telemetry: {check}")

    for backend in ("deterministic", "jolt"):
        missing = REQUIRED_DRIVING_FEEL_CHECKS - by_backend.get(backend, set())
        if missing:
            raise ValueError(f"Vehicle runtime report is missing {backend} driving-feel checks: {sorted(missing)}")
    return checks


def _require_route_pace_probes(report: dict[str, Any]) -> list[dict[str, Any]]:
    probes = report.get("routePaceProbes")
    if not isinstance(probes, list) or not probes:
        raise ValueError("Vehicle runtime report is missing route-pace sensitivity probes.")

    found_full_throttle = False
    for probe in probes:
        if not isinstance(probe, dict):
            raise ValueError(f"Vehicle runtime route-pace probe is invalid: {probe}")
        if probe.get("backend") != "jolt":
            continue
        throttle = float(probe.get("throttle", 0.0))
        found_full_throttle = found_full_throttle or throttle >= 0.99
        if probe.get("passed") is not True or probe.get("checkpointReached") is not True:
            raise ValueError(f"Vehicle runtime route-pace probe failed: {probe}")
        for key in ("framesToCheckpoint", "minDistanceToCheckpoint", "finalPosition", "finalSpeed"):
            if key not in probe:
                raise ValueError(f"Vehicle runtime route-pace probe is missing telemetry: {probe}")

    if not found_full_throttle:
        raise ValueError("Vehicle runtime report is missing a Jolt full-throttle route-pace probe.")
    return probes


def load_and_validate_report(report_path: pathlib.Path) -> dict[str, Any]:
    if not report_path.exists():
        raise FileNotFoundError(f"Vehicle runtime comparison report was not created: {report_path}")

    report = json.loads(report_path.read_text(encoding="utf-8"))
    if report.get("schema") != SCHEMA:
        raise ValueError(f"Unexpected vehicle runtime report schema: {report.get('schema')}")
    if report.get("scenario") != SCENARIO:
        raise ValueError(f"Unexpected vehicle runtime scenario: {report.get('scenario')}")
    if report.get("passed") is not True:
        raise ValueError(f"Vehicle runtime report did not pass: {report.get('error', '')}")

    vehicle = report.get("vehicle")
    if not isinstance(vehicle, dict) or vehicle.get("id") != "service-yard-vehicle":
        raise ValueError(f"Vehicle runtime report has unexpected vehicle block: {vehicle}")

    deterministic_samples = _require_samples(report, "deterministic")
    adapter_samples = _require_samples(report, "adapter")
    if len(deterministic_samples) != len(adapter_samples):
        raise ValueError("Vehicle runtime comparison samples are not paired.")
    if report["adapter"].get("backend") != "jolt":
        raise ValueError(f"Vehicle runtime QA must run against opt-in Jolt backend, got: {report['adapter'].get('backend')}")
    _require_control_checks(report)
    _require_route_checks(report)
    _require_obstacle_checks(report)
    _require_driving_feel_checks(report)
    _require_route_pace_probes(report)

    comparison = report.get("comparison")
    if not isinstance(comparison, dict):
        raise ValueError("Vehicle runtime report is missing comparison metrics.")
    if float(comparison.get("maxPositionDelta", 999.0)) > MAX_POSITION_DELTA:
        raise ValueError(f"Vehicle runtime position delta exceeded threshold: {comparison}")
    if float(comparison.get("maxYawDeltaDegrees", 999.0)) > MAX_YAW_DELTA_DEGREES:
        raise ValueError(f"Vehicle runtime yaw delta exceeded threshold: {comparison}")
    if float(comparison.get("maxSpeedDelta", 999.0)) > MAX_SPEED_DELTA:
        raise ValueError(f"Vehicle runtime speed delta exceeded threshold: {comparison}")
    if comparison.get("recommendation") not in {"promote", "defer"}:
        raise ValueError(f"Vehicle runtime report has invalid recommendation: {comparison.get('recommendation')}")
    return report


def run_vehicle_runtime(exe: pathlib.Path, scene: pathlib.Path, report_path: pathlib.Path, scenario: str = SCENARIO) -> dict[str, Any]:
    if not exe.exists():
        raise FileNotFoundError(f"EngineApp executable was not found: {exe}")
    if scenario != SCENARIO:
        raise ValueError(f"Unsupported vehicle runtime scenario: {scenario}")

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
        raise RuntimeError(f"Vehicle runtime QA command failed with exit code {result.returncode}.")

    report = load_and_validate_report(report_path)
    comparison = report["comparison"]
    print(
        "Vehicle runtime QA passed: "
        f"backend={report['adapter']['backend']}, "
        f"samples={len(report['adapter']['samples'])}, "
        f"controlChecks={len(report['controlChecks'])}, "
        f"routeChecks={len(report['routeChecks'])}, "
        f"obstacleChecks={len(report['obstacleChecks'])}, "
        f"drivingFeelChecks={len(report['drivingFeelChecks'])}, "
        f"routePaceProbes={len(report['routePaceProbes'])}, "
        f"maxPositionDelta={comparison['maxPositionDelta']:.2f}, "
        f"recommendation={comparison['recommendation']}, "
        f"report={report_path}"
    )
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", default=str(default_exe_path()), help="Path to a Jolt-enabled EngineApp.exe.")
    parser.add_argument("--scene", default=str(default_scene_path()), help="Path to Ferry Office scene JSON.")
    parser.add_argument("--scenario", default=SCENARIO, help="QA vehicle runtime scenario name.")
    parser.add_argument("--report-json", default=str(default_report_path()), help="Path for the JSON report.")
    args = parser.parse_args()

    try:
        run_vehicle_runtime(
            pathlib.Path(args.exe),
            pathlib.Path(args.scene),
            pathlib.Path(args.report_json),
            args.scenario,
        )
    except Exception as exception:
        print(f"Vehicle runtime QA failed: {exception}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
