#!/usr/bin/env python3
"""Create one original Blender-authored Tidebreak cable reel prop.

Run with:
    blender --background --python tools/blender/create_tidebreak_cable_reel.py
"""

from __future__ import annotations

import base64
import json
import math
import pathlib

import bpy


ROOT = pathlib.Path(__file__).resolve().parents[2]
OUTPUT = ROOT / "assets" / "models" / "blender_cable_reel.gltf"


def main() -> int:
    _reset_scene()
    mesh = _build_cable_reel_mesh()
    cable_reel = bpy.data.objects.new("tb_blender_cable_reel", mesh)
    bpy.context.collection.objects.link(cable_reel)
    bpy.context.view_layer.objects.active = cable_reel
    cable_reel.select_set(True)

    cable_reel.location = (0.0, 0.0, 0.0)
    cable_reel.rotation_euler = (0.0, 0.0, 0.0)
    cable_reel.scale = (1.0, 1.0, 1.0)
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)

    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.export_scene.gltf(
        filepath=str(OUTPUT),
        export_format="GLTF_SEPARATE",
        use_selection=True,
        export_apply=True,
        export_materials="NONE",
        export_yup=True,
    )
    _embed_exported_buffer(OUTPUT)

    print(f"Tidebreak Blender prop exported: {OUTPUT}")
    print("asset: project-original procedural Blender geometry")
    print(f"blender: {bpy.app.version_string}")
    return 0


def _reset_scene() -> None:
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()


def _build_cable_reel_mesh() -> bpy.types.Mesh:
    vertices: list[tuple[float, float, float]] = []
    faces: list[tuple[int, int, int]] = []

    _append_cylinder_x(vertices, faces, center=(-0.28, 0.38, 0.0), half_length=0.045, radius=0.38, segments=16)
    _append_cylinder_x(vertices, faces, center=(0.28, 0.38, 0.0), half_length=0.045, radius=0.38, segments=16)
    _append_cylinder_x(vertices, faces, center=(0.0, 0.38, 0.0), half_length=0.28, radius=0.18, segments=16)
    _append_cylinder_x(vertices, faces, center=(0.0, 0.38, 0.0), half_length=0.34, radius=0.055, segments=12)

    mesh = bpy.data.meshes.new("tb_blender_cable_reel_mesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update(calc_edges=False)
    return mesh


def _append_cylinder_x(
    vertices: list[tuple[float, float, float]],
    faces: list[tuple[int, int, int]],
    center: tuple[float, float, float],
    half_length: float,
    radius: float,
    segments: int,
) -> None:
    # Author in Tidebreak coordinates: X right, Y up, +Z forward.
    # The reel axis runs along X, so the circular faces sit in the Y/Z plane.
    left_center_index = len(vertices)
    vertices.append(_to_blender((center[0] - half_length, center[1], center[2])))
    right_center_index = len(vertices)
    vertices.append(_to_blender((center[0] + half_length, center[1], center[2])))
    left_start = len(vertices)

    for index in range(segments):
        angle = (index / segments) * math.tau
        y = center[1] + math.cos(angle) * radius
        z = center[2] + math.sin(angle) * radius
        vertices.append(_to_blender((center[0] - half_length, y, z)))
        vertices.append(_to_blender((center[0] + half_length, y, z)))

    for index in range(segments):
        next_index = (index + 1) % segments
        left_current = left_start + index * 2
        right_current = left_current + 1
        left_next = left_start + next_index * 2
        right_next = left_next + 1

        faces.append((left_center_index, left_next, left_current))
        faces.append((right_center_index, right_current, right_next))
        faces.append((left_current, left_next, right_next))
        faces.append((left_current, right_next, right_current))


def _to_blender(point: tuple[float, float, float]) -> tuple[float, float, float]:
    x, y, z = point
    return (x, -z, y)


def _embed_exported_buffer(gltf_path: pathlib.Path) -> None:
    gltf = json.loads(gltf_path.read_text(encoding="utf-8"))
    buffers = gltf.get("buffers", [])
    if len(buffers) != 1:
        raise RuntimeError(f"Expected exactly one exported buffer, found {len(buffers)}")
    uri = buffers[0].get("uri")
    if not isinstance(uri, str) or not uri:
        raise RuntimeError("Exported glTF buffer is missing a URI")
    if uri.startswith("data:"):
        return

    bin_path = gltf_path.parent / uri
    binary = bin_path.read_bytes()
    buffers[0]["uri"] = "data:application/octet-stream;base64," + base64.b64encode(binary).decode("ascii")
    buffers[0]["byteLength"] = len(binary)
    gltf_path.write_text(json.dumps(gltf, indent=2) + "\n", encoding="utf-8")
    bin_path.unlink()


if __name__ == "__main__":
    raise SystemExit(main())
