#!/usr/bin/env python3
"""Create one original Blender-authored Tidebreak notice board prop.

Run with:
    blender --background --python tools/blender/create_tidebreak_notice_board.py
"""

from __future__ import annotations

import base64
import json
import pathlib
import sys

import bpy


ROOT = pathlib.Path(__file__).resolve().parents[2]
OUTPUT = ROOT / "assets" / "models" / "blender_ferry_notice_board.gltf"


def main() -> int:
    _reset_scene()
    mesh = _build_notice_board_mesh()
    notice_board = bpy.data.objects.new("tb_blender_ferry_notice_board", mesh)
    bpy.context.collection.objects.link(notice_board)
    bpy.context.view_layer.objects.active = notice_board
    notice_board.select_set(True)

    notice_board.location = (0.0, 0.0, 0.0)
    notice_board.rotation_euler = (0.0, 0.0, 0.0)
    notice_board.scale = (1.0, 1.0, 1.0)
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


def _build_notice_board_mesh() -> bpy.types.Mesh:
    vertices: list[tuple[float, float, float]] = []
    faces: list[tuple[int, int, int]] = []
    _append_box(vertices, faces, center=(0.0, 0.66, 0.0), half_extents=(0.055, 0.66, 0.055))
    _append_box(vertices, faces, center=(0.0, 1.16, 0.0), half_extents=(0.62, 0.25, 0.04))
    _append_box(vertices, faces, center=(0.0, 1.16, 0.048), half_extents=(0.52, 0.17, 0.012))
    _append_box(vertices, faces, center=(0.0, 0.04, 0.0), half_extents=(0.24, 0.04, 0.16))

    mesh = bpy.data.meshes.new("tb_blender_ferry_notice_board_mesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update(calc_edges=False)
    return mesh


def _append_box(
    vertices: list[tuple[float, float, float]],
    faces: list[tuple[int, int, int]],
    center: tuple[float, float, float],
    half_extents: tuple[float, float, float],
) -> None:
    # Author boxes in Tidebreak coordinates: X right, Y up, +Z forward.
    # Blender is Z-up and the glTF exporter converts Blender +Z to glTF +Y,
    # so map Tidebreak (x, y, z) to Blender (x, -z, y).
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
    faces.extend(tuple(base + index for index in triangle) for triangle in triangles)


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
