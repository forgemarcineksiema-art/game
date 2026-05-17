#!/usr/bin/env python3
"""Print static mesh asset references for Tidebreak scene data."""

from __future__ import annotations

import argparse
from dataclasses import dataclass, field
import pathlib
import sys
from typing import Any

import asset_data
import scene_data


@dataclass(frozen=True)
class MeshFileReport:
    relative_path: str
    suffix: str
    referenced: bool
    asset_ids: list[str] = field(default_factory=list)
    use_count: int = 0
    license: str | None = None
    provenance: str | None = None
    vertex_count: int | None = None
    index_count: int | None = None
    bounds_min: tuple[float, float, float] | None = None
    bounds_max: tuple[float, float, float] | None = None
    file_size: int = 0
    errors: list[str] = field(default_factory=list)
    warnings: list[str] = field(default_factory=list)


@dataclass(frozen=True)
class MeshReport:
    files: list[MeshFileReport]
    validation_errors: list[str] = field(default_factory=list)
    validation_warnings: list[str] = field(default_factory=list)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Print Tidebreak mesh asset and scene-instance summary.")
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
    return parser.parse_args()


def load_gltf_counts(path: pathlib.Path) -> tuple[int | None, int | None, str | None]:
    metadata = asset_data.load_gltf_metadata(path)
    error = "; ".join(metadata.errors) if metadata.errors else None
    return metadata.vertex_count, metadata.index_count, error


def build_mesh_report(
    scene: dict[str, Any],
    models_dir: pathlib.Path = asset_data.DEFAULT_MODELS_DIR,
    additional_referenced_paths: set[str] | None = None,
    additional_assets_by_path: dict[str, list[dict[str, Any]]] | None = None,
) -> MeshReport:
    usage: dict[str, int] = {}
    for instance in _as_list(scene.get("meshInstances")):
        item = _as_dict(instance)
        asset_id = item.get("assetId")
        if isinstance(asset_id, str):
            usage[asset_id] = usage.get(asset_id, 0) + 1

    assets_by_path = asset_data.scene_asset_paths(scene, models_dir)
    for path, assets in (additional_assets_by_path or {}).items():
        if path not in assets_by_path:
            assets_by_path[path] = list(assets)
    additional_referenced_paths = set(additional_referenced_paths or set())
    additional_referenced_paths.update((additional_assets_by_path or {}).keys())
    reports: list[MeshFileReport] = []
    for path in asset_data.model_files(models_dir):
        relative_path = asset_data.display_path(path, models_dir)
        scene_assets = assets_by_path.get(relative_path, [])
        asset_ids = [str(asset.get("id", "<missing-id>")) for asset in scene_assets]
        metadata = asset_data.load_gltf_metadata(path)
        reports.append(
            MeshFileReport(
                relative_path=relative_path,
                suffix=path.suffix.lower(),
                referenced=bool(scene_assets) or relative_path in additional_referenced_paths,
                asset_ids=asset_ids,
                use_count=sum(usage.get(asset_id, 0) for asset_id in asset_ids),
                license=_first_string(scene_assets, "license"),
                provenance=_first_string(scene_assets, "provenance"),
                vertex_count=metadata.vertex_count,
                index_count=metadata.index_count,
                bounds_min=metadata.bounds_min,
                bounds_max=metadata.bounds_max,
                file_size=metadata.file_size,
                errors=metadata.errors,
                warnings=metadata.warnings,
            )
        )

    has_scene_meshes = bool(_as_list(scene.get("meshAssets")) or _as_list(scene.get("meshInstances")))
    validation = (
        scene_data.validate_asset_workflow(
            scene,
            models_dir=models_dir,
            additional_referenced_paths=additional_referenced_paths,
        )
        if has_scene_meshes
        else scene_data.ValidationResult()
    )
    return MeshReport(files=reports, validation_errors=validation.errors, validation_warnings=validation.warnings)


def main() -> int:
    args = parse_args()
    scene_path = pathlib.Path(args.scene) if args.scene else scene_data.DEFAULT_SCENE_PATH
    models_dir = pathlib.Path(args.models_dir)
    try:
        scene = scene_data.load_scene(scene_path)
    except (OSError, ValueError) as exc:
        print(f"[error] failed to load scene: {exc}")
        return 1

    scene_validation = scene_data.validate_scene(scene)
    additional_assets_by_path = _reference_catalog_assets(models_dir, scene_path) if args.scene is None else {}
    report = build_mesh_report(
        scene,
        models_dir=models_dir,
        additional_referenced_paths=set(additional_assets_by_path),
        additional_assets_by_path=additional_assets_by_path,
    )
    print("Tidebreak mesh report")
    print(f"scene: {scene.get('id', scene_path)}")
    print(f"meshAssets: {len(scene.get('meshAssets', []))}")
    print(f"meshInstances: {len(scene.get('meshInstances', []))}")
    print(f"modelFiles: {len(report.files)}")

    for file_report in report.files:
        print()
        print(f"file: {file_report.relative_path}")
        print(f"  referenced: {'yes' if file_report.referenced else 'no'}")
        print(f"  assetIds: {', '.join(file_report.asset_ids) if file_report.asset_ids else '<none>'}")
        print(f"  uses: {file_report.use_count}")
        print(f"  license: {file_report.license or '<missing>'}")
        print(f"  provenance: {file_report.provenance or '<missing>'}")
        print(f"  sizeBytes: {file_report.file_size}")
        if file_report.errors:
            print(f"  gltf: error: {'; '.join(file_report.errors)}")
        else:
            print(f"  gltf: vertices={file_report.vertex_count} indices={file_report.index_count}")
            print(
                "  bounds: "
                f"min={_format_vec(file_report.bounds_min)} "
                f"max={_format_vec(file_report.bounds_max)}"
            )

    errors = [*scene_validation.errors, *report.validation_errors]
    warnings = [*scene_validation.warnings, *report.validation_warnings]
    if warnings:
        print()
        print("validation warnings:")
        for warning in warnings:
            print(f"  - {warning}")
    if errors:
        print()
        print("validation errors:")
        for error in errors:
            print(f"  - {error}")
        return 1

    return 0


def _reference_catalog_assets(models_dir: pathlib.Path, primary_scene_path: pathlib.Path) -> dict[str, list[dict[str, Any]]]:
    referenced: dict[str, list[dict[str, Any]]] = {}
    for scene_path in sorted(scene_data.SCENES_DIR.glob("*.scene.json")):
        if scene_path.resolve() == primary_scene_path.resolve():
            continue
        try:
            scene = scene_data.load_scene(scene_path)
        except (OSError, ValueError):
            continue
        for path, assets in asset_data.scene_asset_paths(scene, models_dir).items():
            referenced.setdefault(path, []).extend(assets)
    return referenced


def _first_string(items: list[dict[str, Any]], key: str) -> str | None:
    for item in items:
        value = item.get(key)
        if isinstance(value, str) and value:
            return value
    return None


def _format_vec(value: tuple[float, float, float] | None) -> str:
    if value is None:
        return "<missing>"
    return "(" + ", ".join(f"{component:.2f}" for component in value) + ")"


def _as_dict(value: Any) -> dict[str, Any]:
    return value if isinstance(value, dict) else {}


def _as_list(value: Any) -> list[Any]:
    return value if isinstance(value, list) else []


if __name__ == "__main__":
    sys.exit(main())
