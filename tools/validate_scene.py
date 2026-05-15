#!/usr/bin/env python3
"""Validate Tidebreak scene data."""

from __future__ import annotations

import argparse
import pathlib
import sys

import scene_data


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Validate Tidebreak scene data.")
    parser.add_argument(
        "scene",
        nargs="?",
        default=str(scene_data.DEFAULT_SCENE_PATH),
        help="Scene JSON path. Defaults to data/scenes/ferry_office.scene.json.",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Treat warnings as failures.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    scene_path = pathlib.Path(args.scene)
    try:
        scene = scene_data.load_scene(scene_path)
    except (OSError, ValueError) as exc:
        print(f"[error] failed to load scene: {exc}")
        return 1

    result = scene_data.validate_scene(scene)
    print(f"Scene validation: {scene.get('id', scene_path)}")
    if result.errors:
        print("Errors:")
        for error in result.errors:
            print(f"  - {error}")
    if result.warnings:
        print("Warnings:")
        for warning in result.warnings:
            print(f"  - {warning}")

    if result.errors or (args.strict and result.warnings):
        print("Scene validation failed.")
        return 1

    print("Scene validation passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
