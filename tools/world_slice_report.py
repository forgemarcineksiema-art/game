#!/usr/bin/env python3
"""Report Tidebreak scene roles for regression and target-slice planning."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import pathlib
import sys

import scene_data


@dataclass(frozen=True)
class SliceReportEntry:
    scene_id: str
    name: str
    path: pathlib.Path
    kind: str
    world_id: str
    slice_id: str
    status: str
    collider_count: int
    route_count: int
    objective_marker_count: int
    validation_errors: int
    validation_warnings: int


@dataclass(frozen=True)
class SliceReport:
    scenes: list[SliceReportEntry]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Report Tidebreak world/slice scene roles.")
    parser.add_argument(
        "scenes",
        nargs="*",
        help="Scene JSON paths. Defaults to all data/scenes/*.scene.json files.",
    )
    return parser.parse_args()


def build_report(paths: list[str | pathlib.Path] | None = None) -> SliceReport:
    scene_paths = [pathlib.Path(path) for path in paths] if paths else scene_data.all_scene_paths()
    entries: list[SliceReportEntry] = []

    for path in scene_paths:
        scene = scene_data.load_scene(path)
        metadata = scene_data.slice_metadata(scene)
        summary = scene_data.build_summary(scene)
        validation = scene_data.validate_scene(scene)
        entries.append(
            SliceReportEntry(
                scene_id=summary.scene_id,
                name=summary.name,
                path=path,
                kind=str(metadata.get("kind", "<missing>")),
                world_id=str(metadata.get("worldId", "<missing>")),
                slice_id=str(metadata.get("sliceId", "<missing>")),
                status=str(metadata.get("status", "<missing>")),
                collider_count=summary.collider_count,
                route_count=summary.route_count,
                objective_marker_count=summary.objective_marker_count,
                validation_errors=len(validation.errors),
                validation_warnings=len(validation.warnings),
            )
        )

    return SliceReport(scenes=entries)


def main() -> int:
    args = parse_args()
    try:
        report = build_report(args.scenes)
    except (OSError, ValueError) as exc:
        print(f"[error] failed to load world slice data: {exc}")
        return 1

    print("Tidebreak world/slice report")
    for entry in report.scenes:
        print()
        print(f"scene: {entry.scene_id} ({entry.name})")
        print(f"  path: {entry.path}")
        print(f"  kind: {entry.kind}")
        print(f"  world: {entry.world_id}")
        print(f"  slice: {entry.slice_id}")
        print(f"  status: {entry.status}")
        print(f"  colliders: {entry.collider_count}")
        print(f"  routes: {entry.route_count}")
        print(f"  objectiveMarkers: {entry.objective_marker_count}")
        print(f"  validation: {entry.validation_errors} error(s), {entry.validation_warnings} warning(s)")

    return 1 if any(entry.validation_errors for entry in report.scenes) else 0


if __name__ == "__main__":
    sys.exit(main())
