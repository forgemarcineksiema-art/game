#!/usr/bin/env python3
"""Create one original Blender-authored Tidebreak harbor backdrop mesh.

Run with:
    blender --background --python tools/blender/create_tidebreak_harbor_backdrop.py
"""

from __future__ import annotations

import base64
import json
import pathlib

import bpy


ROOT = pathlib.Path(__file__).resolve().parents[2]
OUTPUT = ROOT / "assets" / "models" / "blender_harbor_backdrop.gltf"


def main() -> int:
    _reset_scene()
    mesh = _build_backdrop_mesh()
    backdrop = bpy.data.objects.new("tb_blender_harbor_backdrop", mesh)
    bpy.context.collection.objects.link(backdrop)
    bpy.context.view_layer.objects.active = backdrop
    backdrop.select_set(True)

    backdrop.location = (0.0, 0.0, 0.0)
    backdrop.rotation_euler = (0.0, 0.0, 0.0)
    backdrop.scale = (1.0, 1.0, 1.0)
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

    print(f"Tidebreak Blender backdrop exported: {OUTPUT}")
    print("asset: project-original procedural Blender geometry")
    print(f"blender: {bpy.app.version_string}")
    return 0


def _reset_scene() -> None:
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()


def _build_backdrop_mesh() -> bpy.types.Mesh:
    vertices: list[tuple[float, float, float]] = []
    faces: list[tuple[int, int, int]] = []

    # A low, compact silhouette that can be scaled into distant shore strips.
    _append_box(vertices, faces, center=(0.0, 0.045, 0.0), half_extents=(0.54, 0.045, 0.055))

    ridge_segments = [
        (-0.42, 0.13, 0.006, 0.11, 0.085, 0.045),
        (-0.24, 0.19, -0.004, 0.13, 0.145, 0.050),
        (-0.02, 0.155, 0.010, 0.16, 0.110, 0.050),
        (0.22, 0.225, -0.002, 0.14, 0.175, 0.048),
        (0.43, 0.145, 0.006, 0.12, 0.095, 0.045),
    ]
    for x, y, z, hx, hy, hz in ridge_segments:
        _append_box(vertices, faces, center=(x, y, z), half_extents=(hx, hy, hz))

    # Small pier/utility ticks keep the shape coastal without becoming signage.
    for x in (-0.34, -0.18, 0.10, 0.34):
        _append_box(vertices, faces, center=(x, 0.125, -0.075), half_extents=(0.018, 0.080, 0.018))

    mesh = bpy.data.meshes.new("tb_blender_harbor_backdrop_mesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update(calc_edges=False)
    return mesh


def _append_box(
    vertices: list[tuple[float, float, float]],
    faces: list[tuple[int, int, int]],
    center: tuple[float, float, float],
    half_extents: tuple[float, float, float],
) -> None:
    cx, cy, cz = _to_blender(center)
    hx, hy, hz = (half_extents[0], half_extents[2], half_extents[1])
    base = len(vertices)
    vertices.extend(
        [
            (cx - hx, cy - hy, cz - hz),
            (cx + hx, cy - hy, cz - hz),
            (cx + hx, cy + hy, cz - hz),
            (cx - hx, cy + hy, cz - hz),
            (cx - hx, cy - hy, cz + hz),
            (cx + hx, cy - hy, cz + hz),
            (cx + hx, cy + hy, cz + hz),
            (cx - hx, cy + hy, cz + hz),
        ]
    )
    triangles = [
        (0, 1, 2),
        (0, 2, 3),
        (5, 4, 7),
        (5, 7, 6),
        (4, 0, 3),
        (4, 3, 7),
        (1, 5, 6),
        (1, 6, 2),
        (3, 2, 6),
        (3, 6, 7),
        (4, 5, 1),
        (4, 1, 0),
    ]
    faces.extend(tuple(base + vertex for vertex in triangle) for triangle in triangles)


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
