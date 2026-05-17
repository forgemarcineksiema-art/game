"""Run and validate a bounded runtime smoke for non-default Tidebreak scenes."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import json
import pathlib
import subprocess
import sys


ROOT = pathlib.Path(__file__).resolve().parents[1]
DEFAULT_EXE = ROOT / "build" / "windows-vs2022-debug" / "Debug" / "EngineApp.exe"
DEFAULT_SCENE = ROOT / "data" / "scenes" / "veyra_reach_pilot.scene.json"
DEFAULT_FORBIDDEN_TARGET_SLICE_TERMS = [
    "Ferry Office",
    "Ferry Manifest",
    "service call",
    "Job:",
    "jobObjective=",
    "service gate=",
    "power=offline",
    "roadSegment=dock-road",
    "roadBounds=",
    "worldState={",
]


@dataclass(frozen=True)
class RuntimeSmokeValidation:
    passed: bool
    failures: list[str]


def _contains(text: str, token: str) -> bool:
    return token.lower() in text.lower()


def validate_runtime_output(
    output: str,
    *,
    expected_scene_id: str,
    expected_kind: str,
    forbidden_terms: list[str],
) -> RuntimeSmokeValidation:
    failures: list[str] = []
    if not _contains(output, f"Loaded runtime scene data: {expected_scene_id}"):
        failures.append(f"expected scene id '{expected_scene_id}' was not observed")
    if expected_kind and not _contains(output, expected_kind):
        failures.append(f"expected scene kind '{expected_kind}' was not observed")

    for term in forbidden_terms:
        if term and _contains(output, term):
            failures.append(f"forbidden term leaked into runtime output: {term}")

    return RuntimeSmokeValidation(passed=not failures, failures=failures)


def run_runtime_smoke(
    exe: pathlib.Path,
    scene: pathlib.Path,
    *,
    expected_scene_id: str,
    expected_kind: str,
    capture_frame: pathlib.Path | None = None,
    frames: int = 3,
    renderer: str = "null",
    ui_mode: str = "playtest",
) -> tuple[subprocess.CompletedProcess[str], RuntimeSmokeValidation]:
    command = [
        str(exe),
        "--renderer",
        renderer,
        "--ui-mode",
        ui_mode,
        "--frames",
        str(frames),
        "--scene",
        str(scene),
    ]
    if capture_frame is not None:
        capture_frame.parent.mkdir(parents=True, exist_ok=True)
        command.extend(["--free-cursor", "--capture-frame", str(capture_frame)])
    else:
        command.append("--smoke-test")
    completed = subprocess.run(
        command,
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    validation = validate_runtime_output(
        completed.stdout,
        expected_scene_id=expected_scene_id,
        expected_kind=expected_kind,
        forbidden_terms=DEFAULT_FORBIDDEN_TARGET_SLICE_TERMS,
    )
    if capture_frame is not None and (not capture_frame.exists() or capture_frame.stat().st_size <= 0):
        validation.failures.append(f"capture file was not written: {capture_frame}")
        validation = RuntimeSmokeValidation(False, validation.failures)
    if completed.returncode != 0:
        validation.failures.append(f"runtime command exited with {completed.returncode}")
        validation = RuntimeSmokeValidation(False, validation.failures)
    return completed, validation


def build_report(
    completed: subprocess.CompletedProcess[str],
    validation: RuntimeSmokeValidation,
    *,
    scene: pathlib.Path,
    expected_scene_id: str,
    expected_kind: str,
    capture_frame: pathlib.Path | None,
    ui_mode: str,
) -> dict[str, object]:
    return {
        "schema": "tidebreak.runtimeSceneSmoke.v1",
        "scene": str(scene),
        "expectedSceneId": expected_scene_id,
        "expectedKind": expected_kind,
        "uiMode": ui_mode,
        "exitCode": completed.returncode,
        "passed": validation.passed,
        "failures": validation.failures,
        "captureFrame": str(capture_frame) if capture_frame is not None else "",
    }


def _print_report(
    completed: subprocess.CompletedProcess[str],
    validation: RuntimeSmokeValidation,
    *,
    scene: pathlib.Path,
    expected_scene_id: str,
    expected_kind: str,
) -> None:
    print("Tidebreak runtime scene smoke")
    print(f"scene: {scene}")
    print(f"expectedSceneId: {expected_scene_id}")
    print(f"expectedKind: {expected_kind}")
    print(f"exitCode: {completed.returncode}")
    print(f"result: {'pass' if validation.passed else 'fail'}")
    if validation.failures:
        print("failures:")
        for failure in validation.failures:
            print(f"  - {failure}")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Run a bounded runtime smoke for a Tidebreak scene.")
    parser.add_argument("--exe", default=str(DEFAULT_EXE), help="Path to EngineApp.exe.")
    parser.add_argument("--scene", default=str(DEFAULT_SCENE), help="Scene JSON path.")
    parser.add_argument("--expected-scene-id", default="veyra-reach-pilot")
    parser.add_argument("--expected-kind", default="target-slice-scaffold")
    parser.add_argument("--frames", type=int, default=3)
    parser.add_argument("--renderer", default="null", choices=["null", "gdi", "dx11"])
    parser.add_argument("--ui-mode", default="playtest", choices=["playtest", "debug", "minimal"])
    parser.add_argument("--capture-frame", default="", help="Optional renderer-owned BMP capture path.")
    parser.add_argument("--report-json", default="", help="Optional JSON report output path.")
    args = parser.parse_args(argv)
    capture_frame = pathlib.Path(args.capture_frame) if args.capture_frame else None

    completed, validation = run_runtime_smoke(
        pathlib.Path(args.exe),
        pathlib.Path(args.scene),
        expected_scene_id=args.expected_scene_id,
        expected_kind=args.expected_kind,
        capture_frame=capture_frame,
        frames=args.frames,
        renderer=args.renderer,
        ui_mode=args.ui_mode,
    )
    _print_report(
        completed,
        validation,
        scene=pathlib.Path(args.scene),
        expected_scene_id=args.expected_scene_id,
        expected_kind=args.expected_kind,
    )
    if args.report_json:
        report_path = pathlib.Path(args.report_json)
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report = build_report(
            completed,
            validation,
            scene=pathlib.Path(args.scene),
            expected_scene_id=args.expected_scene_id,
            expected_kind=args.expected_kind,
            capture_frame=capture_frame,
            ui_mode=args.ui_mode,
        )
        report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")
    return 0 if validation.passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
