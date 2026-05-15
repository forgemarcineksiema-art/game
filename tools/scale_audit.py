#!/usr/bin/env python3
"""Report suspicious scale choices in Tidebreak scene data."""

from __future__ import annotations

import argparse
import pathlib
import sys

import scene_data


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Audit Tidebreak scene scale assumptions.")
    parser.add_argument(
        "scene",
        nargs="?",
        default=str(scene_data.DEFAULT_SCENE_PATH),
        help="Scene JSON path. Defaults to data/scenes/ferry_office.scene.json.",
    )
    parser.add_argument("--strict", action="store_true", help="Return nonzero if scale warnings exist.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    scene_path = pathlib.Path(args.scene)
    try:
        scene = scene_data.load_scene(scene_path)
    except (OSError, ValueError) as exc:
        print(f"[error] failed to load scene: {exc}")
        return 1

    warnings = scene_data.scale_warnings(scene)
    print(f"Scale audit: {scene.get('id', scene_path)}")
    if not warnings:
        print("No suspicious scale issues found.")
        return 0

    print("Suspicious scale notes:")
    for warning in warnings:
        print(f"  - {warning}")
    return 1 if args.strict else 0


if __name__ == "__main__":
    sys.exit(main())
