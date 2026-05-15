#!/usr/bin/env python3
"""Optional Blender availability check for Tidebreak asset workflow."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import shutil
import subprocess
import sys


@dataclass(frozen=True)
class BlenderCheckResult:
    command: str
    available: bool
    version_line: str | None = None
    executable: str | None = None
    error: str | None = None


def check_blender(command: str = "blender") -> BlenderCheckResult:
    executable = shutil.which(command)
    if executable is None:
        return BlenderCheckResult(
            command=command,
            available=False,
            error=f"'{command}' was not found in PATH",
        )

    try:
        completed = subprocess.run(
            [executable, "--version"],
            check=False,
            capture_output=True,
            text=True,
            timeout=30,
        )
    except (OSError, subprocess.TimeoutExpired) as exc:
        return BlenderCheckResult(command=command, available=False, executable=executable, error=str(exc))

    output = (completed.stdout or completed.stderr).strip().splitlines()
    version_line = output[0].strip() if output else None
    if completed.returncode != 0:
        return BlenderCheckResult(
            command=command,
            available=False,
            executable=executable,
            version_line=version_line,
            error=f"`{command} --version` exited with code {completed.returncode}",
        )

    return BlenderCheckResult(
        command=command,
        available=True,
        executable=executable,
        version_line=version_line or "Blender version detected",
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Check whether Blender is available for Tidebreak prop export.")
    parser.add_argument("--command", default="blender", help="Blender executable to check. Defaults to `blender`.")
    parser.add_argument("--require", action="store_true", help="Exit with code 1 when Blender is unavailable.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    result = check_blender(args.command)
    if result.available:
        print("Blender: available")
        print(f"  command: {result.command}")
        print(f"  executable: {result.executable}")
        print(f"  version: {result.version_line}")
        return 0

    print("Blender: unavailable")
    print(f"  command: {result.command}")
    if result.executable:
        print(f"  executable: {result.executable}")
    if result.version_line:
        print(f"  version: {result.version_line}")
    print(f"  reason: {result.error or 'unknown'}")
    print("  note: Blender is optional for default validation; install it before claiming Blender export success.")
    return 1 if args.require else 0


if __name__ == "__main__":
    sys.exit(main())
