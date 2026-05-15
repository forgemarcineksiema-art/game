#!/usr/bin/env python3
"""Print a compact Tidebreak scene report for Codex work."""

from __future__ import annotations

import argparse
import pathlib
import sys

import scene_data


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Print a Tidebreak scene summary.")
    parser.add_argument(
        "scene",
        nargs="?",
        default=str(scene_data.DEFAULT_SCENE_PATH),
        help="Scene JSON path. Defaults to data/scenes/ferry_office.scene.json.",
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

    summary = scene_data.build_summary(scene)
    validation = scene_data.validate_scene(scene)
    units = scene.get("units", {})
    print("Tidebreak scene report")
    print(f"scene: {summary.scene_id} ({summary.name})")
    print(f"path: {scene_path}")
    print(f"units: {units.get('linear', 'unknown')} | up={units.get('upAxis', '?')} forward={units.get('forwardAxis', '?')}")
    print(f"floor: {summary.floor_height:.2f}")
    print()
    print("counts:")
    print(f"  colliders: {summary.collider_count}")
    print(f"  visualPlaceholders: {summary.visual_count}")
    print(f"  interactables: {summary.interactable_count}")
    print(f"  traversalAffordances: {summary.traversal_count}")
    print(f"  vehicles: {summary.vehicle_count}")
    print(f"  routeMarkers: {summary.route_count}")
    print(f"  objectiveMarkers: {summary.objective_marker_count}")

    player_start = scene.get("playerStart", {})
    print()
    print("starts:")
    print(f"  player: {player_start.get('id')} at {scene_data.format_vec(player_start.get('position', [0, 0, 0]))} yaw={player_start.get('yawDegrees', 0)}")
    for vehicle in scene.get("vehicles", []):
        spawn = vehicle.get("spawn", {})
        bounds = vehicle.get("bounds", {})
        print(
            "  vehicle: "
            f"{vehicle.get('id')} at {scene_data.format_vec(spawn.get('position', [0, 0, 0]))} "
            f"yaw={spawn.get('yawDegrees', 0)} bounds={bounds.get('min')}..{bounds.get('max')}"
        )

    print()
    print("route:")
    for route in scene.get("routeMarkers", []):
        print(f"  {route.get('id')}: {route.get('from')} -> {route.get('to')}")

    print()
    print(f"validation: {len(validation.errors)} error(s), {len(validation.warnings)} warning(s)")
    if validation.errors:
        for error in validation.errors:
            print(f"  error: {error}")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
