#!/usr/bin/env python3
"""Shared scene data helpers for Tidebreak tooling."""

from __future__ import annotations

from dataclasses import dataclass, field
import json
import pathlib
from typing import Any, Iterable


ROOT = pathlib.Path(__file__).resolve().parents[1]
DEFAULT_SCENE_PATH = ROOT / "data" / "scenes" / "ferry_office.scene.json"


REQUIRED_IDS = {
    "ferry-office",
    "player-start-dock",
    "service-gate",
    "service-barrier",
    "ferry-manifest",
    "maintenance-box",
    "wall-button",
    "exit-summary-marker",
    "service-barrier-vault",
    "service-yard-vehicle",
}


@dataclass
class ValidationResult:
    errors: list[str] = field(default_factory=list)
    warnings: list[str] = field(default_factory=list)

    @property
    def ok(self) -> bool:
        return not self.errors


@dataclass(frozen=True)
class SceneSummary:
    scene_id: str
    name: str
    floor_height: float
    collider_count: int
    visual_count: int
    mesh_asset_count: int
    mesh_instance_count: int
    interactable_count: int
    traversal_count: int
    vehicle_count: int
    route_count: int
    objective_marker_count: int


def load_scene(path: str | pathlib.Path = DEFAULT_SCENE_PATH) -> dict[str, Any]:
    scene_path = pathlib.Path(path)
    if not scene_path.is_absolute():
        scene_path = ROOT / scene_path

    with scene_path.open("r", encoding="utf-8") as file:
        scene = json.load(file)

    if not isinstance(scene, dict):
        raise ValueError(f"Scene root must be a JSON object: {scene_path}")
    return scene


def collect_ids(scene: dict[str, Any]) -> set[str]:
    ids: set[str] = set()
    _add_id(ids, scene.get("id"))
    _add_id(ids, _as_dict(scene.get("playerStart")).get("id"))
    for section in [
        "colliders",
        "visualPlaceholders",
        "meshAssets",
        "meshInstances",
        "interactables",
        "traversalAffordances",
        "vehicles",
        "routeMarkers",
        "objectiveMarkers",
    ]:
        for item in _as_list(scene.get(section)):
            _add_id(ids, _as_dict(item).get("id"))
    return ids


def build_summary(scene: dict[str, Any]) -> SceneSummary:
    return SceneSummary(
        scene_id=str(scene.get("id", "")),
        name=str(scene.get("name", "")),
        floor_height=float(scene.get("floorHeight", 0.0)),
        collider_count=len(_as_list(scene.get("colliders"))),
        visual_count=len(_as_list(scene.get("visualPlaceholders"))),
        mesh_asset_count=len(_as_list(scene.get("meshAssets"))),
        mesh_instance_count=len(_as_list(scene.get("meshInstances"))),
        interactable_count=len(_as_list(scene.get("interactables"))),
        traversal_count=len(_as_list(scene.get("traversalAffordances"))),
        vehicle_count=len(_as_list(scene.get("vehicles"))),
        route_count=len(_as_list(scene.get("routeMarkers"))),
        objective_marker_count=len(_as_list(scene.get("objectiveMarkers"))),
    )


def validate_scene(scene: dict[str, Any]) -> ValidationResult:
    result = ValidationResult()

    _require_string(scene, "id", "scene", result)
    _require_string(scene, "name", "scene", result)
    _require_number(scene, "floorHeight", "scene", result)

    units = _as_dict(scene.get("units"))
    for key in ["linear", "upAxis", "rightAxis", "forwardAxis"]:
        _require_string(units, key, "units", result)
    if units.get("linear") != "meter":
        result.warnings.append("units.linear should remain 'meter' for Tidebreak scene data.")

    player_start = _as_dict(scene.get("playerStart"))
    _require_string(player_start, "id", "playerStart", result)
    _validate_vec3(player_start.get("position"), "playerStart.position", result)
    _require_number(player_start, "yawDegrees", "playerStart", result)

    seen: dict[str, str] = {}
    _record_id(scene.get("id"), "scene.id", seen, result)
    _record_id(player_start.get("id"), "playerStart.id", seen, result)

    for collider in _as_list(scene.get("colliders")):
        item = _as_dict(collider)
        label = f"collider {item.get('id', '<missing-id>')}"
        _record_id(item.get("id"), label, seen, result)
        _require_string(item, "kind", label, result)
        _validate_vec3(item.get("center"), f"{label}.center", result)
        _validate_positive_vec3(item.get("halfExtents"), f"{label}.halfExtents", result)
        if "blocksPlayer" not in item:
            result.errors.append(f"{label}.blocksPlayer is required.")

    for visual in _as_list(scene.get("visualPlaceholders")):
        item = _as_dict(visual)
        label = f"visual {item.get('id', '<missing-id>')}"
        _record_id(item.get("id"), label, seen, result)
        _require_string(item, "role", label, result)
        _validate_vec3(item.get("center"), f"{label}.center", result)
        _validate_positive_vec3(item.get("halfExtents"), f"{label}.halfExtents", result)
        _require_string(item, "colorKey", label, result)

    for interactable in _as_list(scene.get("interactables")):
        item = _as_dict(interactable)
        label = f"interactable {item.get('id', '<missing-id>')}"
        _record_id(item.get("id"), label, seen, result)
        for key in ["name", "prompt", "type"]:
            _require_string(item, key, label, result)
        _validate_vec3(item.get("position"), f"{label}.position", result)
        _require_positive_number(item, "radius", label, result)

    for affordance in _as_list(scene.get("traversalAffordances")):
        item = _as_dict(affordance)
        label = f"traversal {item.get('id', '<missing-id>')}"
        _record_id(item.get("id"), label, seen, result)
        for key in ["name", "prompt", "type"]:
            _require_string(item, key, label, result)
        _validate_vec3(item.get("startPosition"), f"{label}.startPosition", result)
        _validate_vec3(item.get("endPosition"), f"{label}.endPosition", result)
        _validate_vec3(item.get("requiredFacingDirection"), f"{label}.requiredFacingDirection", result)
        _require_positive_number(item, "focusRadius", label, result)
        _require_positive_number(item, "durationSeconds", label, result)

    for vehicle in _as_list(scene.get("vehicles")):
        item = _as_dict(vehicle)
        label = f"vehicle {item.get('id', '<missing-id>')}"
        _record_id(item.get("id"), label, seen, result)
        _require_string(item, "name", label, result)
        spawn = _as_dict(item.get("spawn"))
        _validate_vec3(spawn.get("position"), f"{label}.spawn.position", result)
        _require_number(spawn, "yawDegrees", f"{label}.spawn", result)
        _validate_positive_vec3(item.get("proxyHalfExtents"), f"{label}.proxyHalfExtents", result)
        _require_positive_number(item, "enterRadius", label, result)
        _validate_bounds2(_as_dict(item.get("bounds")), f"{label}.bounds", result)

    for route in _as_list(scene.get("routeMarkers")):
        item = _as_dict(route)
        label = f"route {item.get('id', '<missing-id>')}"
        _record_id(item.get("id"), label, seen, result)
        _require_string(item, "from", label, result)
        _require_string(item, "to", label, result)
        points = _as_list(item.get("points"))
        if len(points) < 2:
            result.errors.append(f"{label}.points must contain at least two points.")
        for index, point in enumerate(points):
            _validate_vec3(point, f"{label}.points[{index}]", result)

    for marker in _as_list(scene.get("objectiveMarkers")):
        item = _as_dict(marker)
        label = f"objectiveMarker {item.get('id', '<missing-id>')}"
        _record_id(item.get("id"), label, seen, result)
        _require_string(item, "label", label, result)
        _validate_vec3(item.get("position"), f"{label}.position", result)

    asset_ids: set[str] = set()
    for asset in _as_list(scene.get("meshAssets")):
        item = _as_dict(asset)
        label = f"meshAsset {item.get('id', '<missing-id>')}"
        asset_id = item.get("id")
        _record_id(asset_id, label, seen, result)
        if isinstance(asset_id, str) and asset_id:
            asset_ids.add(asset_id)
        for key in ["path", "format", "units", "upAxis", "license", "provenance"]:
            _require_string(item, key, label, result)
        _validate_mesh_asset_path(item.get("path"), label, result)
        if item.get("format") not in ("gltf", "glb", None):
            result.errors.append(f"{label}.format must be 'gltf' or 'glb'.")
        if item.get("units") not in ("meter", None):
            result.warnings.append(f"{label}.units should remain 'meter'.")
        if item.get("upAxis") not in ("Y", None):
            result.warnings.append(f"{label}.upAxis should remain 'Y'.")
        if "authoringBoundsHalfExtents" in item:
            _validate_positive_vec3(item.get("authoringBoundsHalfExtents"), f"{label}.authoringBoundsHalfExtents", result)

    known_ids = set(seen)
    for instance in _as_list(scene.get("meshInstances")):
        item = _as_dict(instance)
        label = f"meshInstance {item.get('id', '<missing-id>')}"
        _record_id(item.get("id"), label, seen, result)
        _require_string(item, "assetId", label, result)
        asset_id = item.get("assetId")
        if isinstance(asset_id, str) and asset_id and asset_id not in asset_ids:
            result.errors.append(f"{label}.assetId references unknown mesh asset '{asset_id}'.")
        _validate_vec3(item.get("position"), f"{label}.position", result)
        _require_number(item, "yawDegrees", label, result)
        _validate_positive_scale(item.get("scale"), f"{label}.scale", result)
        for reference_key in ["replacesVisualPlaceholderId", "linkedColliderId"]:
            reference = item.get(reference_key)
            if reference is not None and (not isinstance(reference, str) or reference not in known_ids):
                result.errors.append(f"{label}.{reference_key} references unknown id '{reference}'.")

    ids = set(seen)
    for required_id in sorted(REQUIRED_IDS):
        if required_id not in ids:
            result.errors.append(f"Required id missing: {required_id}")

    result.warnings.extend(scale_warnings(scene))
    return result


def scale_warnings(scene: dict[str, Any]) -> list[str]:
    warnings: list[str] = []
    scale = _as_dict(scene.get("scaleReference"))
    player_height = _number_or_none(scale.get("playerHeight"))
    player_radius = _number_or_none(scale.get("playerRadius"))

    if player_height is None or not 1.4 <= player_height <= 2.2:
        warnings.append("scaleReference.playerHeight should be near a human scale in meters.")
    if player_radius is None or not 0.2 <= player_radius <= 0.6:
        warnings.append("scaleReference.playerRadius should stay near the current player proxy radius.")

    for visual in _as_list(scene.get("visualPlaceholders")):
        item = _as_dict(visual)
        extents = _vec3_or_none(item.get("halfExtents"))
        if extents is None:
            continue
        if max(extents) > 12.0:
            warnings.append(f"{item.get('id', '<visual>')} has a very large halfExtent; verify units are meters.")
        if min(extents) < 0.02:
            warnings.append(f"{item.get('id', '<visual>')} has a paper-thin halfExtent; verify it is intentional.")

    for vehicle in _as_list(scene.get("vehicles")):
        item = _as_dict(vehicle)
        extents = _vec3_or_none(item.get("proxyHalfExtents"))
        if extents is None:
            continue
        width = extents[0] * 2.0
        height = extents[1] * 2.0
        length = extents[2] * 2.0
        if not 1.0 <= width <= 2.6:
            warnings.append(f"{item.get('id', '<vehicle>')} width {width:.2f}m looks suspicious.")
        if not 0.8 <= height <= 2.6:
            warnings.append(f"{item.get('id', '<vehicle>')} height {height:.2f}m looks suspicious.")
        if not 1.5 <= length <= 6.5:
            warnings.append(f"{item.get('id', '<vehicle>')} length {length:.2f}m looks suspicious.")

    asset_bounds: dict[str, tuple[float, float, float]] = {}
    for asset in _as_list(scene.get("meshAssets")):
        item = _as_dict(asset)
        extents = _vec3_or_none(item.get("authoringBoundsHalfExtents"))
        if extents is None:
            continue
        asset_id = item.get("id", "<mesh>")
        if max(extents) > 8.0:
            warnings.append(f"{asset_id} has very large authored mesh bounds; verify units are meters.")
        if min(extents) < 0.01:
            warnings.append(f"{asset_id} has very small authored mesh bounds; verify export scale.")
        if isinstance(asset_id, str):
            asset_bounds[asset_id] = extents

    for instance in _as_list(scene.get("meshInstances")):
        item = _as_dict(instance)
        scale = _scale3_or_none(item.get("scale"))
        if scale is None:
            continue
        instance_id = item.get("id", "<meshInstance>")
        if max(scale) > 12.0:
            warnings.append(f"{instance_id} mesh scale is very large; verify meters and placeholder intent.")
        if min(scale) < 0.05:
            warnings.append(f"{instance_id} mesh scale is very small; verify it is intentional.")
        position = _vec3_or_none(item.get("position"))
        if position is not None and max(abs(component) for component in position) > 40.0:
            warnings.append(f"{instance_id} is far from the prototype origin; verify placement.")
        asset_id = item.get("assetId")
        if isinstance(asset_id, str) and asset_id in asset_bounds:
            world_half = tuple(asset_bounds[asset_id][i] * scale[i] for i in range(3))
            if max(world_half) > 12.0:
                warnings.append(f"{instance_id} world bounds look large after scale; verify instance scale.")

    return warnings


def _as_dict(value: Any) -> dict[str, Any]:
    return value if isinstance(value, dict) else {}


def _as_list(value: Any) -> list[Any]:
    return value if isinstance(value, list) else []


def _add_id(ids: set[str], value: Any) -> None:
    if isinstance(value, str) and value:
        ids.add(value)


def _record_id(value: Any, label: str, seen: dict[str, str], result: ValidationResult) -> None:
    if not isinstance(value, str) or not value:
        result.errors.append(f"{label} id must be a non-empty string.")
        return
    if value in seen:
        result.errors.append(f"Duplicate id '{value}' in {label}; first seen in {seen[value]}.")
        return
    seen[value] = label


def _require_string(item: dict[str, Any], key: str, label: str, result: ValidationResult) -> None:
    value = item.get(key)
    if not isinstance(value, str) or not value.strip():
        result.errors.append(f"{label}.{key} must be a non-empty string.")


def _require_number(item: dict[str, Any], key: str, label: str, result: ValidationResult) -> None:
    if _number_or_none(item.get(key)) is None:
        result.errors.append(f"{label}.{key} must be numeric.")


def _require_positive_number(item: dict[str, Any], key: str, label: str, result: ValidationResult) -> None:
    value = _number_or_none(item.get(key))
    if value is None:
        result.errors.append(f"{label}.{key} must be numeric.")
    elif value <= 0.0:
        result.errors.append(f"{label}.{key} must be positive.")


def _validate_vec3(value: Any, label: str, result: ValidationResult) -> None:
    if _vec3_or_none(value) is None:
        result.errors.append(f"{label} must be a numeric [x, y, z] vector.")


def _validate_positive_vec3(value: Any, label: str, result: ValidationResult) -> None:
    vector = _vec3_or_none(value)
    if vector is None:
        result.errors.append(f"{label} must be a numeric [x, y, z] vector.")
        return
    if any(component <= 0.0 for component in vector):
        result.errors.append(f"{label} must contain positive values.")


def _validate_bounds2(bounds: dict[str, Any], label: str, result: ValidationResult) -> None:
    min_value = _vec2_or_none(bounds.get("min"))
    max_value = _vec2_or_none(bounds.get("max"))
    if min_value is None:
        result.errors.append(f"{label}.min must be a numeric [x, z] vector.")
    if max_value is None:
        result.errors.append(f"{label}.max must be a numeric [x, z] vector.")
    if min_value is None or max_value is None:
        return
    if min_value[0] >= max_value[0] or min_value[1] >= max_value[1]:
        result.errors.append(f"{label} min must be less than max for both x and z.")


def _validate_mesh_asset_path(value: Any, label: str, result: ValidationResult) -> None:
    if not isinstance(value, str) or not value:
        return
    path = pathlib.PurePosixPath(value.replace("\\", "/"))
    suffix = path.suffix.lower()
    if suffix not in {".gltf", ".glb"}:
        result.errors.append(f"{label}.path must end in .gltf or .glb.")
    if path.is_absolute() or ".." in path.parts or not value.replace("\\", "/").startswith("assets/"):
        result.errors.append(f"{label}.path must be a repo-relative path under assets/.")
        return
    if not (ROOT / pathlib.Path(*path.parts)).exists():
        result.errors.append(f"{label}.path does not exist: {value}")


def _validate_positive_scale(value: Any, label: str, result: ValidationResult) -> None:
    scale = _scale3_or_none(value)
    if scale is None:
        result.errors.append(f"{label} must be a positive number or numeric [x, y, z] vector.")
        return
    if any(component <= 0.0 for component in scale):
        result.errors.append(f"{label} must contain positive values.")


def _number_or_none(value: Any) -> float | None:
    if isinstance(value, bool):
        return None
    if isinstance(value, int | float):
        return float(value)
    return None


def _vec3_or_none(value: Any) -> tuple[float, float, float] | None:
    if not isinstance(value, list) or len(value) != 3:
        return None
    numbers = [_number_or_none(component) for component in value]
    if any(component is None for component in numbers):
        return None
    return (numbers[0], numbers[1], numbers[2])  # type: ignore[return-value]


def _vec2_or_none(value: Any) -> tuple[float, float] | None:
    if not isinstance(value, list) or len(value) != 2:
        return None
    numbers = [_number_or_none(component) for component in value]
    if any(component is None for component in numbers):
        return None
    return (numbers[0], numbers[1])  # type: ignore[return-value]


def _scale3_or_none(value: Any) -> tuple[float, float, float] | None:
    scalar = _number_or_none(value)
    if scalar is not None:
        return (scalar, scalar, scalar)
    return _vec3_or_none(value)


def ids_by_section(scene: dict[str, Any]) -> dict[str, list[str]]:
    sections: dict[str, list[str]] = {
        "scene": [str(scene.get("id", ""))],
        "playerStart": [str(_as_dict(scene.get("playerStart")).get("id", ""))],
    }
    for section in [
        "colliders",
        "visualPlaceholders",
        "meshAssets",
        "meshInstances",
        "interactables",
        "traversalAffordances",
        "vehicles",
        "routeMarkers",
        "objectiveMarkers",
    ]:
        sections[section] = [
            str(_as_dict(item).get("id", ""))
            for item in _as_list(scene.get(section))
            if _as_dict(item).get("id")
        ]
    return sections


def format_vec(value: Iterable[Any]) -> str:
    return "(" + ", ".join(f"{float(component):.2f}" for component in value) + ")"
