#!/usr/bin/env python3
"""Tests for Tidebreak PowerShell run UX helpers."""

from __future__ import annotations

import pathlib
import subprocess
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
PLAY_SCRIPT = ROOT / "scripts" / "play.ps1"


def run_play_script(*args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [
            "powershell",
            "-ExecutionPolicy",
            "Bypass",
            "-File",
            str(PLAY_SCRIPT),
            *args,
        ],
        cwd=ROOT,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )


def run_play_script_command(command: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [
            "powershell",
            "-ExecutionPolicy",
            "Bypass",
            "-Command",
            command,
        ],
        cwd=ROOT,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )


class RunScriptTests(unittest.TestCase):
    def test_play_script_dry_run_uses_playable_defaults(self) -> None:
        result = run_play_script("-DryRun")

        self.assertEqual(0, result.returncode, result.stdout)
        self.assertIn("EngineApp.exe", result.stdout)
        self.assertIn("--renderer gdi", result.stdout)
        self.assertIn("--ui-mode playtest", result.stdout)
        self.assertIn("--scene data\\scenes\\ferry_office.scene.json", result.stdout)

    def test_play_script_short_options_override_defaults(self) -> None:
        result = run_play_script("-DryRun", "-Dx11", "-DebugUi", "-Frames", "42", "-FreeCursor")

        self.assertEqual(0, result.returncode, result.stdout)
        self.assertIn("--renderer dx11", result.stdout)
        self.assertIn("--ui-mode debug", result.stdout)
        self.assertIn("--frames 42", result.stdout)
        self.assertIn("--free-cursor", result.stdout)

    def test_play_script_capture_frame_option_is_forwarded(self) -> None:
        result = run_play_script(
            "-DryRun",
            "-Frames",
            "6",
            "-CaptureFrame",
            "build\\captures\\v0.29-gdi.bmp",
        )

        self.assertEqual(0, result.returncode, result.stdout)
        self.assertIn("--frames 6", result.stdout)
        self.assertIn("--capture-frame build\\captures\\v0.29-gdi.bmp", result.stdout)

    def test_play_script_capture_dir_option_is_forwarded(self) -> None:
        result = run_play_script(
            "-DryRun",
            "-Dx11",
            "-CaptureDir",
            "build\\captures",
        )

        self.assertEqual(0, result.returncode, result.stdout)
        self.assertIn("--renderer dx11", result.stdout)
        self.assertIn("--capture-dir build\\captures", result.stdout)

    def test_play_script_passthrough_args_replace_default_renderer_and_ui(self) -> None:
        result = run_play_script_command(
            f"& '{PLAY_SCRIPT}' -DryRun -Args @('--renderer', 'null', '--ui-mode', 'minimal', '--frames', '3')"
        )

        self.assertEqual(0, result.returncode, result.stdout)
        self.assertEqual(1, result.stdout.count("--renderer"))
        self.assertEqual(1, result.stdout.count("--ui-mode"))
        self.assertIn("--renderer null", result.stdout)
        self.assertIn("--ui-mode minimal", result.stdout)
        self.assertIn("--frames 3", result.stdout)

    def test_play_script_missing_executable_explains_build_step(self) -> None:
        result = run_play_script(
            "-DryRun",
            "-ExecutablePath",
            "build\\missing-playable\\EngineApp.exe",
        )

        self.assertNotEqual(0, result.returncode, result.stdout)
        self.assertIn("Playable executable was not found", result.stdout)
        self.assertIn("scripts\\configure.ps1", result.stdout)
        self.assertIn("scripts\\build.ps1", result.stdout)


if __name__ == "__main__":
    unittest.main()
