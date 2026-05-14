#!/usr/bin/env python3
"""Print a compact status report for future AI work."""

from __future__ import annotations

import pathlib
import subprocess
import sys


ROOT = pathlib.Path(__file__).resolve().parents[1]


def run_git(args: list[str]) -> str:
    try:
        completed = subprocess.run(
            ["git", *args],
            cwd=ROOT,
            check=False,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
    except FileNotFoundError:
        return "git not found"

    return completed.stdout.strip() or "(no output)"


def exists(path: str) -> str:
    return "yes" if (ROOT / path).exists() else "no"


def main() -> int:
    print("AI-Native Engine Foundation status")
    print(f"root: {ROOT}")
    print(f"git: {run_git(['status', '--short', '--branch'])}")
    print()
    print("important files:")
    for path in [
        "AGENTS.md",
        "CMakeLists.txt",
        "CMakePresets.json",
        "docs/RUNBOOK.md",
        "docs/ARCHITECTURE.md",
        "docs/ROADMAP.md",
        "docs/STATUS.md",
        "docs/DECISIONS.md",
        "docs/AI_WORKFLOW.md",
        "docs/GAME_DIRECTION.md",
        "docs/VERTICAL_SLICE.md",
        "docs/TECH_DEBT.md",
        "docs/MANUAL_TEST_CHECKLIST.md",
        "scripts/doctor.ps1",
        "scripts/configure.ps1",
        "scripts/build.ps1",
        "scripts/verify.ps1",
    ]:
        print(f"  {path}: {exists(path)}")

    print()
    print("build outputs:")
    for path in [
        "build/windows-vs2022-debug",
        "build/windows-vs2026-debug",
        "build/ninja-debug",
    ]:
        print(f"  {path}: {exists(path)}")

    status_path = ROOT / "docs" / "STATUS.md"
    if status_path.exists():
        print()
        print("status excerpt:")
        lines = status_path.read_text(encoding="utf-8").splitlines()
        for line in lines[:40]:
            print(f"  {line}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
