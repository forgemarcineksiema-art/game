#!/usr/bin/env python3
"""Print static mesh asset references for Tidebreak scene data."""

from __future__ import annotations

import argparse
import json
import pathlib
import sys
from typing import Any

import scene_data


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Print Tidebreak mesh asset and scene-instance summary.")
    parser.add_argument(
        "scene",
        nargs="?",
        default=str(scene_data.DEFAULT_SCENE_PATH),
        help="Scene JSON path. Defaults to data/scenes/ferry_office.scene.json.",
    )
    return parser.parse_args()


def load_gltf_counts(path: pathlib.Path) -> tuple[int | None, int | None, str | None]:
    try:
        gltf = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        return None, None, str(exc)

    meshes = gltf.get("meshes", [])
    accessors = gltf.get("accessors", [])
    if not isinstance(meshes, list) or not meshes:
        return None, None, "no meshes array"
    if not isinstance(accessors, list):
        return None, None, "accessors must be an array"

    primitive = _first_primitive(meshes)
    if not primitive:
        return None, None, "no mesh primitive"

    attributes = primitive.get("attributes", {})
    position_accessor = attributes.get("POSITION") if isinstance(attributes, dict) else None
    index_accessor = primitive.get("indices")
    vertex_count = _accessor_count(accessors, position_accessor)
    index_count = _accessor_count(accessors, index_accessor)
    return vertex_count, index_count, None


def _first_primitive(meshes: list[Any]) -> dict[str, Any] | None:
    for mesh in meshes:
        if not isinstance(mesh, dict):
            continue
        primitives = mesh.get("primitives", [])
        if isinstance(primitives, list) and primitives and isinstance(primitives[0], dict):
            return primitives[0]
    return None


def _accessor_count(accessors: list[Any], index: Any) -> int | None:
    if not isinstance(index, int) or index < 0 or index >= len(accessors):
        return None
    accessor = accessors[index]
    if not isinstance(accessor, dict):
        return None
    count = accessor.get("count")
    return count if isinstance(count, int) else None


def main() -> int:
    args = parse_args()
    scene_path = pathlib.Path(args.scene)
    try:
        scene = scene_data.load_scene(scene_path)
    except (OSError, ValueError) as exc:
        print(f"[error] failed to load scene: {exc}")
        return 1

    validation = scene_data.validate_scene(scene)
    print("Tidebreak mesh report")
    print(f"scene: {scene.get('id', scene_path)}")
    print(f"meshAssets: {len(scene.get('meshAssets', []))}")
    print(f"meshInstances: {len(scene.get('meshInstances', []))}")

    usage: dict[str, int] = {}
    for instance in scene.get("meshInstances", []):
        if isinstance(instance, dict):
            asset_id = instance.get("assetId")
            if isinstance(asset_id, str):
                usage[asset_id] = usage.get(asset_id, 0) + 1

    for asset in scene.get("meshAssets", []):
        if not isinstance(asset, dict):
            continue
        asset_id = asset.get("id", "<missing-id>")
        path_text = asset.get("path", "")
        path = scene_data.ROOT / path_text if isinstance(path_text, str) else scene_data.ROOT
        vertex_count, index_count, error = load_gltf_counts(path)
        print()
        print(f"asset: {asset_id}")
        print(f"  path: {path_text}")
        print(f"  uses: {usage.get(asset_id, 0)}")
        print(f"  license: {asset.get('license', '<missing>')}")
        print(f"  provenance: {asset.get('provenance', '<missing>')}")
        if error:
            print(f"  gltf: error: {error}")
        else:
            print(f"  gltf: vertices={vertex_count} indices={index_count}")

    if validation.errors:
        print()
        print("validation errors:")
        for error in validation.errors:
            print(f"  - {error}")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
