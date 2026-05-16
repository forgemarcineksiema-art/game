#!/usr/bin/env python3
"""Create one original Blender-authored Tidebreak wet road surface mesh.

Run with:
    blender --background --python tools/blender/create_tidebreak_wet_road_surface.py
"""

from __future__ import annotations

import base64
import json
import pathlib

import bpy


ROOT = pathlib.Path(__file__).resolve().parents[2]
OUTPUT = ROOT / "assets" / "models" / "blender_wet_road_surface.gltf"


def main() -> int:
    _reset_scene()
    mesh = _build_surface_mesh()
    surface = bpy.data.objects.new("tb_blender_wet_road_surface", mesh)
    bpy.context.collection.objects.link(surface)
    bpy.context.view_layer.objects.active = surface
    surface.select_set(True)

    surface.location = (0.0, 0.0, 0.0)
    surface.rotation_euler = (0.0, 0.0, 0.0)
    surface.scale = (1.0, 1.0, 1.0)
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


def _build_surface_mesh() -> bpy.types.Mesh:
    vertices: list[tuple[float, float, float]] = []
    faces: list[tuple[int, int, int]] = []

    strip_width = 0.184
    gap = 0.012
    start_x = -0.5 + strip_width * 0.5
    for index in range(5):
        x = start_x + index * (strip_width + gap)
        top = 0.024 if index % 2 == 0 else 0.020
        _append_box(
            vertices,
            faces,
            center=(x, top * 0.5, 0.0),
            half_extents=(strip_width * 0.5, top * 0.5, 0.5),
        )

    # Low lips catch overcast shading and make scaled slabs read as a surfaced road piece.
    _append_box(vertices, faces, center=(-0.5, 0.018, 0.0), half_extents=(0.018, 0.018, 0.5))
    _append_box(vertices, faces, center=(0.5, 0.018, 0.0), half_extents=(0.018, 0.018, 0.5))

    mesh = bpy.data.meshes.new("tb_blender_wet_road_surface_mesh")
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
