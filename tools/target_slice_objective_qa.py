#!/usr/bin/env python3
"""Run and validate the Veyra target-slice live objective acquisition QA."""

from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import sys
from typing import Any

import runtime_scene_smoke


ROOT = pathlib.Path(__file__).resolve().parents[1]
SCENARIO = "veyra-target-objective-acquisition"
SCHEMA = "v0.99-target-slice-objective-acquisition-qa"
INPUT_SCRIPT_NAME = "recorded-veyra-target-objective-v1"
SCENE_ID = "veyra-reach-pilot"
OBJECTIVE_ID = "inspect-cinder-cache-marker"
FOCUS_NAME = "Cinder Harbor Marker"
FOCUS_PROMPT = "Confirm Cinder Harbor Reach"
CONTACT_COLLIDER_NAME = "pilot-road-edge-collider"
RISKY_ACTION_ID = "cinder-cache-risk-response"
RISKY_ACTION_NAME = "Suspicious Cargo Cache"
LOCAL_RESPONSE_STATE_ID = "cinder-local-alerted"
EXIT_RECOVERY_STATE_ID = "harbor-scar-escape-confirmed"
EXIT_RECOVERY_NAME = "Harbor Scar Escape Marker"
FORBIDDEN_TERMS = runtime_scene_smoke.DEFAULT_FORBIDDEN_TARGET_SLICE_TERMS + ["Ferry Office"]


def default_exe_path() -> pathlib.Path:
    return ROOT / "build" / "windows-vs2022-debug" / "Debug" / "EngineApp.exe"


def default_scene_path() -> pathlib.Path:
    return ROOT / "data" / "scenes" / "veyra_reach_pilot.scene.json"


def default_report_path() -> pathlib.Path:
    return ROOT / "build" / "playthroughs" / f"{SCENARIO}-report.json"


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def _json_text(report: dict[str, Any]) -> str:
    return json.dumps(report, sort_keys=True)


def _vector_length(vector: dict[str, Any]) -> float:
    x = float(vector.get("x", 0.0))
    y = float(vector.get("y", 0.0))
    z = float(vector.get("z", 0.0))
    return (x * x + y * y + z * z) ** 0.5


def load_and_validate_report(report_path: pathlib.Path) -> dict[str, Any]:
    if not report_path.exists():
        raise FileNotFoundError(f"Target-slice objective QA report was not created: {report_path}")

    report = json.loads(report_path.read_text(encoding="utf-8"))
    _require(report.get("schema") == SCHEMA, f"Unexpected target-slice objective QA schema: {report.get('schema')}")
    _require(report.get("scenario") == SCENARIO, f"Unexpected target-slice objective scenario: {report.get('scenario')}")
    _require(report.get("passed") is True, f"Target-slice objective QA did not pass: {report.get('error', '')}")

    scene = report.get("scene", {})
    _require(scene.get("id") == SCENE_ID, f"Unexpected target-slice scene id: {scene.get('id')}")

    input_block = report.get("input", {})
    _require(input_block.get("scriptName") == INPUT_SCRIPT_NAME, "Target-slice QA report is missing the recorded input script.")
    frames_to_focus = int(input_block.get("framesToFocus", -1))
    frames_to_interact = int(input_block.get("framesToInteract", -1))
    _require(frames_to_focus > 0, "Target-slice QA report did not prove live focus acquisition.")
    _require(
        frames_to_interact >= frames_to_focus,
        "Target-slice QA report interaction happened before focus acquisition.",
    )

    focus = report.get("focus", {})
    _require(focus.get("acquired") is True, "Target-slice QA report did not acquire focus.")
    _require(focus.get("name") == FOCUS_NAME, f"Unexpected target-slice focus name: {focus.get('name')}")
    _require(focus.get("prompt") == FOCUS_PROMPT, f"Unexpected target-slice focus prompt: {focus.get('prompt')}")
    _require(float(focus.get("distance", 99.0)) > 0.0, "Target-slice focus distance was not recorded.")

    contact = report.get("contact")
    _require(isinstance(contact, dict), "Target-slice QA report is missing contact evidence.")
    _require(contact.get("attempted") is True, "Target-slice QA report did not attempt authored contact.")
    _require(contact.get("hit") is True, "Target-slice QA report did not hit the authored contact collider.")
    _require(
        contact.get("colliderName") == CONTACT_COLLIDER_NAME,
        f"Unexpected target-slice contact collider: {contact.get('colliderName')}",
    )
    frames_to_contact = int(contact.get("framesToContact", -1))
    frames_to_recovery = int(contact.get("framesToRecovery", -1))
    _require(frames_to_contact > 0, "Target-slice QA report did not record frames to contact.")
    _require(contact.get("recoveredControl") is True, "Target-slice QA report did not prove recovery after contact.")
    _require(
        frames_to_recovery > frames_to_contact,
        "Target-slice QA report recovery timing did not follow contact.",
    )
    _require(int(contact.get("hitCount", 0)) > 0, "Target-slice QA report did not record a contact hit count.")
    _require(_vector_length(contact.get("push", {})) > 0.0, "Target-slice contact push vector was not recorded.")
    _require(_vector_length(contact.get("normal", {})) > 0.0, "Target-slice contact normal vector was not recorded.")
    _require(
        frames_to_focus >= frames_to_recovery,
        "Target-slice focus was acquired before contact recovery was proven.",
    )

    interaction = report.get("interaction", {})
    _require(interaction.get("triggered") is True, "Target-slice QA report did not trigger interaction input.")

    risky_action = report.get("riskyAction")
    _require(isinstance(risky_action, dict), "Target-slice QA report is missing riskyAction evidence.")
    _require(risky_action.get("id") == RISKY_ACTION_ID, f"Unexpected riskyAction id: {risky_action.get('id')}")
    _require(risky_action.get("attempted") is True, "Target-slice QA did not attempt risky action.")
    _require(risky_action.get("triggered") is True, "Target-slice QA did not trigger risky action.")
    _require(
        risky_action.get("interactableName") == RISKY_ACTION_NAME,
        f"Unexpected riskyAction interactable: {risky_action.get('interactableName')}",
    )
    frames_to_action = int(risky_action.get("framesToAction", -1))
    _require(frames_to_action > frames_to_recovery, "Risky action did not happen after contact recovery.")
    _require(
        "local response" in str(risky_action.get("message", "")).lower(),
        "Risky action message does not describe a local response.",
    )

    local_response = report.get("localResponse")
    _require(isinstance(local_response, dict), "Target-slice QA report is missing localResponse evidence.")
    _require(
        local_response.get("stateId") == LOCAL_RESPONSE_STATE_ID,
        f"Unexpected localResponse state: {local_response.get('stateId')}",
    )
    _require(local_response.get("active") is True, "Target-slice local response was not active.")
    frames_to_response = int(local_response.get("framesToResponse", -1))
    _require(frames_to_response >= frames_to_action, "Local response timing did not follow risky action.")

    exit_recovery = report.get("exitRecovery")
    _require(isinstance(exit_recovery, dict), "Target-slice QA report is missing exitRecovery evidence.")
    _require(
        exit_recovery.get("stateId") == EXIT_RECOVERY_STATE_ID,
        f"Unexpected exitRecovery state: {exit_recovery.get('stateId')}",
    )
    _require(exit_recovery.get("complete") is True, "Target-slice exit recovery did not complete.")
    _require(
        exit_recovery.get("interactableName") == EXIT_RECOVERY_NAME,
        f"Unexpected exitRecovery interactable: {exit_recovery.get('interactableName')}",
    )
    frames_to_exit = int(exit_recovery.get("framesToExit", -1))
    _require(frames_to_exit > frames_to_response, "Exit recovery timing did not follow local response.")

    final = report.get("final", {})
    _require(final.get("objectiveId") == OBJECTIVE_ID, f"Unexpected target-slice objective id: {final.get('objectiveId')}")
    _require(final.get("objectiveComplete") is True, "Target-slice objective did not complete.")
    completion_summary = str(final.get("completionSummary", ""))
    completion_event_text = str(final.get("completionEventText", ""))
    _require(
        f"targetObjective={OBJECTIVE_ID}" in completion_summary,
        f"Target-slice completion summary does not name the objective: {completion_summary}",
    )
    for expected in [
        f"riskyAction={RISKY_ACTION_ID}",
        f"responseState={LOCAL_RESPONSE_STATE_ID}",
        f"exitRecovery={EXIT_RECOVERY_STATE_ID}",
    ]:
        _require(expected in completion_summary, f"Target-slice completion summary is missing {expected}.")
    _require(
        "cinder-harbor-marker" in completion_event_text,
        f"Target-slice completion event does not name the authored marker: {completion_event_text}",
    )

    report_text = _json_text(report)
    for term in FORBIDDEN_TERMS:
        if term and term.lower() in report_text.lower():
            raise ValueError(f"Forbidden target-slice term leaked into report: {term}")

    return report


def run_qa(exe: pathlib.Path, scene: pathlib.Path, report_path: pathlib.Path) -> dict[str, Any]:
    if not exe.exists():
        raise FileNotFoundError(f"EngineApp executable was not found: {exe}")

    report_path.parent.mkdir(parents=True, exist_ok=True)
    command = [
        str(exe),
        "--qa-playthrough",
        SCENARIO,
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
        raise RuntimeError(f"Target-slice objective QA command failed with exit code {result.returncode}.")

    report = load_and_validate_report(report_path)
    contact = report["contact"]
    print(
        "Target-slice objective QA passed: "
        f"scene={report['scene']['id']}, "
        f"contact={contact['colliderName']}@{contact['framesToContact']}, "
        f"recovery={contact['framesToRecovery']}, "
        f"riskyAction={report['riskyAction']['framesToAction']}, "
        f"exit={report['exitRecovery']['framesToExit']}, "
        f"framesToFocus={report['input']['framesToFocus']}, "
        f"framesToInteract={report['input']['framesToInteract']}, "
        f"report={report_path}"
    )
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", default=str(default_exe_path()), help="Path to EngineApp.exe.")
    parser.add_argument("--scene", default=str(default_scene_path()), help="Path to Veyra target-slice scene JSON.")
    parser.add_argument("--report-json", default=str(default_report_path()), help="Path for the JSON report.")
    args = parser.parse_args()

    try:
        run_qa(pathlib.Path(args.exe), pathlib.Path(args.scene), pathlib.Path(args.report_json))
    except Exception as exception:
        print(f"Target-slice objective QA failed: {exception}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
