#!/usr/bin/env python3
"""Static mesh asset helpers for Tidebreak tooling."""

from __future__ import annotations

import base64
import binascii
from dataclasses import dataclass, field
import json
import pathlib
import struct
from typing import Any


ROOT = pathlib.Path(__file__).resolve().parents[1]
DEFAULT_MODELS_DIR = ROOT / "assets" / "models"
EMBEDDED_BUFFER_PREFIX = "data:application/octet-stream;base64,"


@dataclass(frozen=True)
class GltfMetadata:
    path: pathlib.Path
    suffix: str
    vertex_count: int | None = None
    index_count: int | None = None
    bounds_min: tuple[float, float, float] | None = None
    bounds_max: tuple[float, float, float] | None = None
    file_size: int = 0
    errors: list[str] = field(default_factory=list)
    warnings: list[str] = field(default_factory=list)

    @property
    def ok(self) -> bool:
        return not self.errors


def model_files(models_dir: pathlib.Path = DEFAULT_MODELS_DIR) -> list[pathlib.Path]:
    if not models_dir.exists():
        return []
    files: list[pathlib.Path] = []
    for suffix in ("*.gltf", "*.glb"):
        files.extend(models_dir.rglob(suffix))
    return sorted(path for path in files if path.is_file())


def display_path(path: pathlib.Path, models_dir: pathlib.Path = DEFAULT_MODELS_DIR) -> str:
    resolved = path.resolve()
    try:
        return resolved.relative_to(ROOT).as_posix()
    except ValueError:
        try:
            return resolved.relative_to(models_dir.resolve()).as_posix()
        except ValueError:
            return resolved.name


def resolve_scene_asset_path(path_text: str, models_dir: pathlib.Path = DEFAULT_MODELS_DIR) -> pathlib.Path:
    normalized = pathlib.PurePosixPath(path_text.replace("\\", "/"))
    if normalized.is_absolute() or ".." in normalized.parts:
        return pathlib.Path(path_text)
    if normalized.parts[:2] == ("assets", "models"):
        return ROOT / pathlib.Path(*normalized.parts)
    return models_dir / pathlib.Path(*normalized.parts)


def scene_asset_paths(scene: dict[str, Any], models_dir: pathlib.Path = DEFAULT_MODELS_DIR) -> dict[str, list[dict[str, Any]]]:
    paths: dict[str, list[dict[str, Any]]] = {}
    for asset in _as_list(scene.get("meshAssets")):
        item = _as_dict(asset)
        path_text = item.get("path")
        if not isinstance(path_text, str) or not path_text:
            continue
        resolved = resolve_scene_asset_path(path_text, models_dir)
        paths.setdefault(display_path(resolved, models_dir), []).append(item)
    return paths


def load_gltf_metadata(path: pathlib.Path) -> GltfMetadata:
    suffix = path.suffix.lower()
    errors: list[str] = []
    warnings: list[str] = []
    file_size = 0

    if suffix == ".glb":
        return GltfMetadata(path=path, suffix=suffix, file_size=_file_size(path), errors=["unsupported .glb file"])
    if suffix != ".gltf":
        return GltfMetadata(path=path, suffix=suffix, file_size=_file_size(path), errors=[f"unsupported mesh format '{suffix}'"])
    if not path.exists():
        return GltfMetadata(path=path, suffix=suffix, errors=["file does not exist"])

    file_size = _file_size(path)
    if file_size <= 0:
        errors.append("empty mesh file")

    try:
        gltf = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        return GltfMetadata(path=path, suffix=suffix, file_size=file_size, errors=[str(exc)])

    if not isinstance(gltf, dict):
        return GltfMetadata(path=path, suffix=suffix, file_size=file_size, errors=["glTF root must be an object"])

    buffers = _as_list(gltf.get("buffers"))
    for index, buffer in enumerate(buffers):
        item = _as_dict(buffer)
        uri = item.get("uri")
        if not isinstance(uri, str) or not uri:
            errors.append(f"buffer[{index}] is missing an embedded data URI")
            continue
        if not uri.startswith(EMBEDDED_BUFFER_PREFIX):
            errors.append(f"buffer[{index}] uses unsupported external buffer '{uri}'")

    meshes = _as_list(gltf.get("meshes"))
    accessors = _as_list(gltf.get("accessors"))
    primitive = _first_primitive(meshes)
    if not primitive:
        errors.append("no mesh primitive")
        return GltfMetadata(path=path, suffix=suffix, file_size=file_size, errors=errors, warnings=warnings)

    attributes = _as_dict(primitive.get("attributes"))
    position_accessor_index = attributes.get("POSITION")
    index_accessor_index = primitive.get("indices")
    vertex_count = _accessor_count(accessors, position_accessor_index)
    index_count = _accessor_count(accessors, index_accessor_index)
    bounds_min, bounds_max = _accessor_bounds(accessors, position_accessor_index)

    if bounds_min is None or bounds_max is None:
        computed_min, computed_max = _compute_position_bounds(gltf, position_accessor_index)
        bounds_min = bounds_min or computed_min
        bounds_max = bounds_max or computed_max

    if vertex_count is None:
        errors.append("POSITION accessor count is missing")
    if index_count is None:
        errors.append("index accessor count is missing")
    if bounds_min is None or bounds_max is None:
        errors.append("POSITION bounds are missing or unsupported")

    return GltfMetadata(
        path=path,
        suffix=suffix,
        vertex_count=vertex_count,
        index_count=index_count,
        bounds_min=bounds_min,
        bounds_max=bounds_max,
        file_size=file_size,
        errors=errors,
        warnings=warnings,
    )


def _file_size(path: pathlib.Path) -> int:
    try:
        return path.stat().st_size
    except OSError:
        return 0


def _first_primitive(meshes: list[Any]) -> dict[str, Any] | None:
    for mesh in meshes:
        item = _as_dict(mesh)
        primitives = _as_list(item.get("primitives"))
        if primitives and isinstance(primitives[0], dict):
            return primitives[0]
    return None


def _accessor_count(accessors: list[Any], index: Any) -> int | None:
    accessor = _accessor(accessors, index)
    if accessor is None:
        return None
    count = accessor.get("count")
    return count if isinstance(count, int) else None


def _accessor_bounds(accessors: list[Any], index: Any) -> tuple[tuple[float, float, float] | None, tuple[float, float, float] | None]:
    accessor = _accessor(accessors, index)
    if accessor is None:
        return None, None
    return _vec3_or_none(accessor.get("min")), _vec3_or_none(accessor.get("max"))


def _compute_position_bounds(gltf: dict[str, Any], accessor_index: Any) -> tuple[tuple[float, float, float] | None, tuple[float, float, float] | None]:
    accessors = _as_list(gltf.get("accessors"))
    buffer_views = _as_list(gltf.get("bufferViews"))
    buffers = _as_list(gltf.get("buffers"))
    accessor = _accessor(accessors, accessor_index)
    if accessor is None:
        return None, None
    if accessor.get("componentType") != 5126 or accessor.get("type") != "VEC3":
        return None, None
    count = accessor.get("count")
    buffer_view_index = accessor.get("bufferView")
    if not isinstance(count, int) or not isinstance(buffer_view_index, int) or buffer_view_index < 0 or buffer_view_index >= len(buffer_views):
        return None, None
    buffer_view = _as_dict(buffer_views[buffer_view_index])
    buffer_index = buffer_view.get("buffer")
    if not isinstance(buffer_index, int) or buffer_index < 0 or buffer_index >= len(buffers):
        return None, None
    buffer_item = _as_dict(buffers[buffer_index])
    uri = buffer_item.get("uri")
    if not isinstance(uri, str) or not uri.startswith(EMBEDDED_BUFFER_PREFIX):
        return None, None
    try:
        data = base64.b64decode(uri[len(EMBEDDED_BUFFER_PREFIX) :], validate=True)
    except (ValueError, binascii.Error):
        return None, None

    offset = int(buffer_view.get("byteOffset", 0) or 0) + int(accessor.get("byteOffset", 0) or 0)
    stride = int(buffer_view.get("byteStride", 12) or 12)
    if stride < 12:
        return None, None

    values: list[tuple[float, float, float]] = []
    for index in range(count):
        start = offset + index * stride
        end = start + 12
        if end > len(data):
            return None, None
        values.append(struct.unpack_from("<fff", data, start))
    if not values:
        return None, None
    mins = tuple(min(value[axis] for value in values) for axis in range(3))
    maxs = tuple(max(value[axis] for value in values) for axis in range(3))
    return mins, maxs


def _accessor(accessors: list[Any], index: Any) -> dict[str, Any] | None:
    if not isinstance(index, int) or index < 0 or index >= len(accessors):
        return None
    item = accessors[index]
    return item if isinstance(item, dict) else None


def _as_dict(value: Any) -> dict[str, Any]:
    return value if isinstance(value, dict) else {}


def _as_list(value: Any) -> list[Any]:
    return value if isinstance(value, list) else []


def _vec3_or_none(value: Any) -> tuple[float, float, float] | None:
    if not isinstance(value, list) or len(value) != 3:
        return None
    numbers: list[float] = []
    for component in value:
        if isinstance(component, bool) or not isinstance(component, int | float):
            return None
        numbers.append(float(component))
    return (numbers[0], numbers[1], numbers[2])
