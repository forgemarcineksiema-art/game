#!/usr/bin/env python3
"""Validate Tidebreak static mesh asset workflow."""

from __future__ import annotations

import argparse
import pathlib
import sys

import asset_data
import scene_data


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Validate Tidebreak static mesh assets and scene references.")
    parser.add_argument(
        "scene",
        nargs="?",
        default=None,
        help="Scene JSON path. Defaults to data/scenes/ferry_office.scene.json and uses every scene as the asset reference catalog.",
    )
    parser.add_argument(
        "--models-dir",
        default=str(asset_data.DEFAULT_MODELS_DIR),
        help="Directory to scan for .gltf/.glb model files. Defaults to assets/models.",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Treat warnings as failures.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    scene_path = pathlib.Path(args.scene) if args.scene else scene_data.DEFAULT_SCENE_PATH
    models_dir = pathlib.Path(args.models_dir)
    try:
        scene = scene_data.load_scene(scene_path)
    except (OSError, ValueError) as exc:
        print(f"[error] failed to load scene: {exc}")
        return 1

    additional_referenced_paths = _reference_catalog_paths(models_dir, scene_path) if args.scene is None else set()
    scene_result = scene_data.validate_scene(scene)
    asset_result = scene_data.validate_asset_workflow(
        scene,
        models_dir=models_dir,
        additional_referenced_paths=additional_referenced_paths,
    )
    errors = [*scene_result.errors, *asset_result.errors]
    warnings = [*scene_result.warnings, *asset_result.warnings]

    print(f"Asset validation: {scene.get('id', scene_path)}")
    print(f"modelsDir: {models_dir}")
    print(f"modelFiles: {len(asset_data.model_files(models_dir))}")

    if errors:
        print("Errors:")
        for error in errors:
            print(f"  - {error}")
    if warnings:
        print("Warnings:")
        for warning in warnings:
            print(f"  - {warning}")

    if errors or (args.strict and warnings):
        print("Asset validation failed.")
        return 1

    print("Asset validation passed.")
    return 0


def _reference_catalog_paths(models_dir: pathlib.Path, primary_scene_path: pathlib.Path) -> set[str]:
    referenced: set[str] = set()
    for scene_path in sorted(scene_data.SCENES_DIR.glob("*.scene.json")):
        if scene_path.resolve() == primary_scene_path.resolve():
            continue
        try:
            scene = scene_data.load_scene(scene_path)
        except (OSError, ValueError):
            continue
        referenced.update(asset_data.scene_asset_paths(scene, models_dir).keys())
    return referenced


if __name__ == "__main__":
    sys.exit(main())
