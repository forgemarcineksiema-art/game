#!/usr/bin/env python3
"""Compile Veyra Reach world authoring data into runtime scene JSON and preview reports."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import html
import json
import pathlib
import sys
from typing import Any

import scene_data


ROOT = pathlib.Path(__file__).resolve().parents[1]
DEFAULT_WORLD_ROOT = ROOT / "data" / "worlds" / "veyra_reach"
DEFAULT_SCENE = ROOT / "data" / "scenes" / "veyra_reach_pilot.scene.json"
WORLD_RELATIVE = "data/worlds/veyra_reach/world.json"


class WorldAuthorError(RuntimeError):
    """Raised when world source data cannot be compiled into a valid scene."""


@dataclass(frozen=True)
class WorldPackage:
    root: pathlib.Path
    world: dict[str, Any]
    areas: list[dict[str, Any]]


@dataclass(frozen=True)
class CheckResult:
    ok: bool
    messages: list[str]


def load_world_package(world_root: str | pathlib.Path = DEFAULT_WORLD_ROOT) -> WorldPackage:
    root = _resolve_repo_path(world_root)
    world_path = root / "world.json"
    try:
        world = json.loads(world_path.read_text(encoding="utf-8"))
    except OSError as exc:
        raise WorldAuthorError(f"failed to load world source {world_path}: {exc}") from exc
    except json.JSONDecodeError as exc:
        raise WorldAuthorError(f"invalid world JSON {world_path}: {exc}") from exc

    area_files = _as_list(world.get("areaFiles"))
    if not area_files:
        raise WorldAuthorError("world.areaFiles must list at least one area file.")

    areas: list[dict[str, Any]] = []
    for area_file in area_files:
        if not isinstance(area_file, str) or not area_file:
            raise WorldAuthorError("world.areaFiles entries must be non-empty strings.")
        area_path = root / area_file
        try:
            area = json.loads(area_path.read_text(encoding="utf-8"))
        except OSError as exc:
            raise WorldAuthorError(f"failed to load area source {area_path}: {exc}") from exc
        except json.JSONDecodeError as exc:
            raise WorldAuthorError(f"invalid area JSON {area_path}: {exc}") from exc
        area["_sourcePath"] = _repo_relative(area_path)
        areas.append(area)

    return WorldPackage(root=root, world=world, areas=areas)


def compile_scene(package: WorldPackage) -> dict[str, Any]:
    _validate_world_package(package)
    world = package.world
    scene_source = _as_dict(world.get("scene"))
    scene = {
        "schemaVersion": int(scene_source.get("schemaVersion", 2)),
        "id": _required_string(scene_source, "id", "world.scene"),
        "name": _required_string(scene_source, "name", "world.scene"),
        "description": _required_string(scene_source, "description", "world.scene"),
        "generatedFrom": {
            "world": WORLD_RELATIVE,
            "areas": [_repo_relative(package.root / area_file) for area_file in _as_list(world.get("areaFiles"))],
        },
        "sliceMetadata": _as_dict(scene_source.get("sliceMetadata")),
        "units": _as_dict(scene_source.get("units")),
        "floorHeight": _number(scene_source.get("floorHeight"), "world.scene.floorHeight"),
        "playerStart": _as_dict(scene_source.get("playerStart")),
        "scaleReference": _as_dict(scene_source.get("scaleReference")),
        "targetObjective": {},
        "targetActionResponse": {},
        "sceneMaterials": list(_as_list(world.get("materials"))),
        "colliders": [],
        "visualPlaceholders": [],
        "meshAssets": list(_as_list(world.get("meshAssets"))),
        "meshInstances": [],
        "interactables": [],
        "traversalAffordances": [],
        "vehicles": [],
        "routeMarkers": [],
        "objectiveMarkers": [],
    }

    for area in package.areas:
        scene["visualPlaceholders"].extend(_compile_terrain_patch(patch) for patch in _as_list(area.get("terrainPatches")))
        scene["visualPlaceholders"].extend(_compile_road_segment(road, index) for road in _as_list(area.get("roads")) for index in range(len(_as_list(road.get("points"))) - 1))
        scene["colliders"].extend(_copy_ordered(item, ["id", "kind", "center", "halfExtents", "blocksPlayer"]) for item in _as_list(area.get("colliders")))
        scene["meshInstances"].extend(_as_list(area.get("meshInstances")))
        scene["interactables"].extend(_as_list(area.get("interactables")))
        scene["routeMarkers"].extend(_as_list(area.get("routes")))
        scene["objectiveMarkers"].extend(_as_list(area.get("objectiveMarkers")))
        if _as_dict(area.get("targetObjective")):
            scene["targetObjective"] = _as_dict(area.get("targetObjective"))
        if _as_dict(area.get("targetActionResponse")):
            scene["targetActionResponse"] = _as_dict(area.get("targetActionResponse"))

    _validate_generated_scene(scene)
    return scene


def check_generated_scene(package: WorldPackage, scene_path: str | pathlib.Path = DEFAULT_SCENE) -> CheckResult:
    messages: list[str] = []
    try:
        generated = compile_scene(package)
    except WorldAuthorError as exc:
        return CheckResult(ok=False, messages=[str(exc)])

    validation = scene_data.validate_scene(generated)
    if validation.errors:
        return CheckResult(ok=False, messages=[f"generated scene invalid: {error}" for error in validation.errors])

    path = _resolve_repo_path(scene_path)
    try:
        current = json.loads(path.read_text(encoding="utf-8"))
    except OSError as exc:
        return CheckResult(ok=False, messages=[f"failed to read generated scene artifact: {exc}"])
    except json.JSONDecodeError as exc:
        return CheckResult(ok=False, messages=[f"generated scene artifact is invalid JSON: {exc}"])

    if _canonical_json(generated) != _canonical_json(current):
        messages.append(f"generated scene drift: {path} does not match data/worlds/veyra_reach source.")
        return CheckResult(ok=False, messages=messages)

    return CheckResult(ok=True, messages=["generated scene matches world source."])


def build_preview_html(package: WorldPackage, scene: dict[str, Any]) -> str:
    bounds = _scene_bounds(scene)
    min_x, max_x, min_z, max_z = bounds
    width = 1100
    height = 760
    margin = 55

    def sx(x: float) -> float:
        return margin + ((x - min_x) / max(max_x - min_x, 1.0)) * (width - margin * 2)

    def sy(z: float) -> float:
        return height - margin - ((z - min_z) / max(max_z - min_z, 1.0)) * (height - margin * 2)

    material_colors = {material["key"]: _rgb(material["baseColor"]) for material in scene["sceneMaterials"]}
    parts: list[str] = []
    parts.append("<!doctype html><html><head><meta charset=\"utf-8\"><title>Veyra Reach Preview</title>")
    parts.append("<style>body{font-family:Segoe UI,Arial,sans-serif;background:#111820;color:#dfe8e5;margin:24px}svg{background:#d7e1df;border:1px solid #6e7f7c} .label{font-size:11px;fill:#10201d} .layer{font-weight:700;color:#ffffff}</style>")
    parts.append("</head><body>")
    parts.append("<h1>Veyra Reach - Cinder Harbor Reach</h1>")
    parts.append("<p>Generated from <code>data/worlds/veyra_reach/world.json</code>. Layers: <span class=\"layer\">Terrain</span>, <span class=\"layer\">Roads</span>, <span class=\"layer\">Collision</span>, <span class=\"layer\">Landmarks</span>, markers, and route graph.</p>")
    parts.append(f"<svg viewBox=\"0 0 {width} {height}\" width=\"{width}\" height=\"{height}\" role=\"img\" aria-label=\"Veyra Reach top-down world preview\">")

    parts.append("<g id=\"Terrain\">")
    for visual in scene["visualPlaceholders"]:
        if "terrain" not in visual["role"] and "shore" not in visual["id"] and "water" not in visual["id"] and "horizon" not in visual["id"]:
            continue
        _rect(parts, visual, sx, sy, material_colors, opacity=0.75)
    parts.append("</g>")

    parts.append("<g id=\"Roads\">")
    for visual in scene["visualPlaceholders"]:
        if "road" in visual["role"]:
            _rect(parts, visual, sx, sy, material_colors, opacity=0.9)
    for route in scene["routeMarkers"]:
        points = " ".join(f"{sx(float(point[0])):.1f},{sy(float(point[2])):.1f}" for point in route["points"])
        parts.append(f"<polyline points=\"{points}\" fill=\"none\" stroke=\"#f2d36b\" stroke-width=\"3\" opacity=\"0.85\" />")
    parts.append("</g>")

    parts.append("<g id=\"Collision\">")
    for collider in scene["colliders"]:
        _rect(parts, collider, sx, sy, {"": "#cf4d34"}, opacity=0.35, stroke="#7c1e16")
    parts.append("</g>")

    parts.append("<g id=\"Landmarks\">")
    for marker in scene["objectiveMarkers"]:
        x, _, z = marker["position"]
        parts.append(f"<circle cx=\"{sx(float(x)):.1f}\" cy=\"{sy(float(z)):.1f}\" r=\"6\" fill=\"#f8f1ba\" stroke=\"#27352f\" />")
        parts.append(f"<text class=\"label\" x=\"{sx(float(x)) + 8:.1f}\" y=\"{sy(float(z)) - 8:.1f}\">{html.escape(marker['label'])}</text>")
    for place in _as_list(package.world.get("places")):
        x, _, z = place["position"]
        parts.append(f"<text class=\"label\" x=\"{sx(float(x)) + 5:.1f}\" y=\"{sy(float(z)) + 14:.1f}\">{html.escape(place['name'])}</text>")
    parts.append("</g>")

    parts.append("</svg>")
    parts.append("<h2>Named Places</h2><ul>")
    for place in _as_list(package.world.get("places")):
        parts.append(f"<li>{html.escape(place['name'])}</li>")
    parts.append("</ul>")
    parts.append("</body></html>\n")
    return "".join(parts)


def build_report(package: WorldPackage, scene: dict[str, Any], check: CheckResult | None = None) -> dict[str, Any]:
    validation = scene_data.validate_scene(scene)
    return {
        "schema": "tidebreak.worldAuthor.report.v1",
        "worldId": package.world.get("id", ""),
        "sceneId": scene.get("id", ""),
        "generatedFrom": scene.get("generatedFrom", {}),
        "counts": {
            "areas": len(package.areas),
            "materials": len(scene.get("sceneMaterials", [])),
            "colliders": len(scene.get("colliders", [])),
            "visualPlaceholders": len(scene.get("visualPlaceholders", [])),
            "meshAssets": len(scene.get("meshAssets", [])),
            "meshInstances": len(scene.get("meshInstances", [])),
            "interactables": len(scene.get("interactables", [])),
            "routes": len(scene.get("routeMarkers", [])),
            "objectiveMarkers": len(scene.get("objectiveMarkers", [])),
        },
        "bounds": _scene_bounds(scene),
        "validation": {
            "errors": validation.errors,
            "warnings": validation.warnings,
        },
        "drift": {
            "ok": check.ok if check is not None else None,
            "messages": check.messages if check is not None else [],
        },
    }


def write_scene(scene: dict[str, Any], path: str | pathlib.Path = DEFAULT_SCENE) -> None:
    resolved = _resolve_repo_path(path)
    resolved.parent.mkdir(parents=True, exist_ok=True)
    resolved.write_text(_canonical_json(scene), encoding="utf-8")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--world-root", default=str(DEFAULT_WORLD_ROOT))
    parser.add_argument("--scene", default=str(DEFAULT_SCENE))
    parser.add_argument("--write-scene", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--preview-html", default="")
    parser.add_argument("--report-json", default="")
    args = parser.parse_args(argv)

    try:
        package = load_world_package(args.world_root)
        scene = compile_scene(package)
        check = check_generated_scene(package, args.scene) if args.check else None
        if args.write_scene:
            write_scene(scene, args.scene)
            print(f"Wrote generated Veyra scene: {_resolve_repo_path(args.scene)}")
        if args.preview_html:
            preview_path = _resolve_repo_path(args.preview_html)
            preview_path.parent.mkdir(parents=True, exist_ok=True)
            preview_path.write_text(build_preview_html(package, scene), encoding="utf-8")
            print(f"Wrote Veyra world preview: {preview_path}")
        if args.report_json:
            report_path = _resolve_repo_path(args.report_json)
            report_path.parent.mkdir(parents=True, exist_ok=True)
            report_path.write_text(json.dumps(build_report(package, scene, check), indent=2, sort_keys=True) + "\n", encoding="utf-8")
            print(f"Wrote Veyra world report: {report_path}")
        if check is not None:
            for message in check.messages:
                print(message)
            if not check.ok:
                return 1
        if not args.write_scene and not args.check and not args.preview_html and not args.report_json:
            print(_canonical_json(scene), end="")
    except WorldAuthorError as exc:
        print(f"World authoring failed: {exc}", file=sys.stderr)
        return 1
    return 0


def _validate_world_package(package: WorldPackage) -> None:
    material_keys = _unique_strings(package.world.get("materials"), "key", "world.materials")
    asset_ids = _unique_strings(package.world.get("meshAssets"), "id", "world.meshAssets")
    place_ids = _unique_strings(package.world.get("places"), "id", "world.places")
    if not material_keys:
        raise WorldAuthorError("world.materials must not be empty.")
    if not package.areas:
        raise WorldAuthorError("world package must contain at least one area.")

    seen_source_ids: dict[str, str] = {}
    for area in package.areas:
        if not _as_list(area.get("terrainPatches")):
            raise WorldAuthorError(f"area {area.get('id', '<missing>')} must author terrainPatches.")
        if not _as_list(area.get("roads")):
            raise WorldAuthorError(f"area {area.get('id', '<missing>')} must author roads.")
        _collect_ids(seen_source_ids, area.get("terrainPatches"), "terrainPatches")
        _collect_ids(seen_source_ids, area.get("roads"), "roads")
        _collect_ids(seen_source_ids, area.get("colliders"), "colliders")
        _collect_ids(seen_source_ids, area.get("meshInstances"), "meshInstances")
        _collect_ids(seen_source_ids, area.get("interactables"), "interactables")
        _collect_ids(seen_source_ids, area.get("routes"), "routes")
        _collect_ids(seen_source_ids, area.get("objectiveMarkers"), "objectiveMarkers")
        for section, key_name in [("terrainPatches", "materialKey"), ("roads", "materialKey"), ("meshInstances", "colorKey")]:
            for item in _as_list(area.get(section)):
                key = item.get(key_name)
                if key not in material_keys:
                    raise WorldAuthorError(f"{section}.{item.get('id', '<missing>')}.{key_name} references unknown material key '{key}'.")
        for instance in _as_list(area.get("meshInstances")):
            asset_id = instance.get("assetId")
            if asset_id not in asset_ids:
                raise WorldAuthorError(f"meshInstances.{instance.get('id', '<missing>')}.assetId references unknown mesh asset '{asset_id}'.")
        route_endpoints = {item["id"] for item in _as_list(area.get("interactables")) if isinstance(item, dict) and "id" in item}
        route_endpoints.update(item["id"] for item in _as_list(area.get("objectiveMarkers")) if isinstance(item, dict) and "id" in item)
        for route in _as_list(area.get("routes")):
            for key in ["from", "to"]:
                if route.get(key) not in route_endpoints:
                    raise WorldAuthorError(f"route {route.get('id', '<missing>')}.{key} references unknown marker/interactable id '{route.get(key)}'.")
    if "cinder-harbor" not in {area.get("id") for area in package.areas}:
        raise WorldAuthorError("Veyra world package must include cinder-harbor area.")
    for required in ["greywinch-service-road", "stormwall-bend", "harbor-scar-overlook"]:
        if required not in place_ids:
            raise WorldAuthorError(f"world.places is missing required place '{required}'.")


def _validate_generated_scene(scene: dict[str, Any]) -> None:
    validation = scene_data.validate_scene(scene)
    if validation.errors:
        raise WorldAuthorError("generated scene failed scene validation: " + "; ".join(validation.errors))


def _compile_terrain_patch(patch: dict[str, Any]) -> dict[str, Any]:
    return {
        "id": patch["id"],
        "role": "veyra-terrain-" + str(patch.get("label", patch["id"])).replace(" ", "-"),
        "center": patch["center"],
        "halfExtents": patch["halfExtents"],
        "colorKey": patch["materialKey"],
    }


def _compile_road_segment(road: dict[str, Any], index: int) -> dict[str, Any]:
    points = _as_list(road.get("points"))
    start = _vec3(points[index], f"road {road.get('id')}.points[{index}]")
    end = _vec3(points[index + 1], f"road {road.get('id')}.points[{index + 1}]")
    width = float(road.get("width", 2.5))
    min_x = min(start[0], end[0]) - width * 0.5
    max_x = max(start[0], end[0]) + width * 0.5
    min_z = min(start[2], end[2]) - width * 0.5
    max_z = max(start[2], end[2]) + width * 0.5
    return {
        "id": f"{road['id']}-surface-{index + 1}",
        "role": "veyra-road-ribbon",
        "center": [round((min_x + max_x) * 0.5, 3), 0.0, round((min_z + max_z) * 0.5, 3)],
        "halfExtents": [round((max_x - min_x) * 0.5, 3), 0.025, round((max_z - min_z) * 0.5, 3)],
        "colorKey": road["materialKey"],
    }


def _copy_ordered(item: dict[str, Any], keys: list[str]) -> dict[str, Any]:
    return {key: item[key] for key in keys if key in item}


def _canonical_json(value: dict[str, Any]) -> str:
    return json.dumps(value, indent=2, sort_keys=True) + "\n"


def _scene_bounds(scene: dict[str, Any]) -> tuple[float, float, float, float]:
    xs: list[float] = []
    zs: list[float] = []
    for section in ["visualPlaceholders", "colliders"]:
        for item in _as_list(scene.get(section)):
            center = _vec3(item.get("center"), f"{section}.center")
            half = _vec3(item.get("halfExtents"), f"{section}.halfExtents")
            xs.extend([center[0] - half[0], center[0] + half[0]])
            zs.extend([center[2] - half[2], center[2] + half[2]])
    for section in ["meshInstances", "objectiveMarkers", "interactables"]:
        position_key = "position"
        for item in _as_list(scene.get(section)):
            if position_key in item:
                position = _vec3(item.get(position_key), f"{section}.position")
                xs.append(position[0])
                zs.append(position[2])
    if not xs or not zs:
        return (-10.0, 10.0, -10.0, 10.0)
    return (min(xs), max(xs), min(zs), max(zs))


def _rect(parts: list[str], item: dict[str, Any], sx: Any, sy: Any, material_colors: dict[str, str], opacity: float, stroke: str = "#1a2a26") -> None:
    center = _vec3(item.get("center"), "rect.center")
    half = _vec3(item.get("halfExtents"), "rect.halfExtents")
    min_x = center[0] - half[0]
    max_x = center[0] + half[0]
    min_z = center[2] - half[2]
    max_z = center[2] + half[2]
    fill = material_colors.get(str(item.get("colorKey", "")), "#a7b0aa")
    x = sx(min_x)
    y = sy(max_z)
    width = sx(max_x) - sx(min_x)
    height = sy(min_z) - sy(max_z)
    parts.append(f"<rect x=\"{x:.1f}\" y=\"{y:.1f}\" width=\"{width:.1f}\" height=\"{height:.1f}\" fill=\"{fill}\" stroke=\"{stroke}\" opacity=\"{opacity}\" />")


def _rgb(color: list[Any]) -> str:
    r = max(0, min(255, int(float(color[0]) * 255)))
    g = max(0, min(255, int(float(color[1]) * 255)))
    b = max(0, min(255, int(float(color[2]) * 255)))
    return f"rgb({r},{g},{b})"


def _unique_strings(items: Any, key: str, label: str) -> set[str]:
    seen: set[str] = set()
    for item in _as_list(items):
        value = item.get(key) if isinstance(item, dict) else None
        if not isinstance(value, str) or not value:
            raise WorldAuthorError(f"{label} entries must include non-empty {key}.")
        if value in seen:
            raise WorldAuthorError(f"duplicate source id/key '{value}' in {label}.")
        seen.add(value)
    return seen


def _collect_ids(seen: dict[str, str], items: Any, label: str) -> None:
    for item in _as_list(items):
        if not isinstance(item, dict):
            raise WorldAuthorError(f"{label} entries must be objects.")
        value = item.get("id")
        if not isinstance(value, str) or not value:
            raise WorldAuthorError(f"{label} entry is missing id.")
        if value in seen:
            raise WorldAuthorError(f"duplicate source id '{value}' in {label}; first seen in {seen[value]}.")
        seen[value] = label


def _required_string(item: dict[str, Any], key: str, label: str) -> str:
    value = item.get(key)
    if not isinstance(value, str) or not value:
        raise WorldAuthorError(f"{label}.{key} must be a non-empty string.")
    return value


def _number(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, int | float):
        raise WorldAuthorError(f"{label} must be numeric.")
    return float(value)


def _vec3(value: Any, label: str) -> tuple[float, float, float]:
    if not isinstance(value, list) or len(value) != 3:
        raise WorldAuthorError(f"{label} must be [x, y, z].")
    return (float(value[0]), float(value[1]), float(value[2]))


def _as_dict(value: Any) -> dict[str, Any]:
    return value if isinstance(value, dict) else {}


def _as_list(value: Any) -> list[Any]:
    return value if isinstance(value, list) else []


def _resolve_repo_path(value: str | pathlib.Path) -> pathlib.Path:
    path = pathlib.Path(value)
    return path if path.is_absolute() else ROOT / path


def _repo_relative(path: pathlib.Path) -> str:
    return pathlib.Path(path).resolve().relative_to(ROOT).as_posix()


if __name__ == "__main__":
    raise SystemExit(main())
