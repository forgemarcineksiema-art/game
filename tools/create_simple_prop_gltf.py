#!/usr/bin/env python3
"""Generate tiny original placeholder glTF props in Tidebreak's supported subset.

This is a fallback helper, not a Blender exporter. Use it only when the goal is
to keep simple original placeholder assets flowing while Blender is unavailable.
"""

from __future__ import annotations

import argparse
import base64
import json
import pathlib
import struct
import sys


ROOT = pathlib.Path(__file__).resolve().parents[1]
DEFAULT_OUTPUT = ROOT / "assets" / "models" / "ferry_notice_board.gltf"


def write_ferry_notice_board(output_path: pathlib.Path, overwrite: bool = True) -> None:
    if output_path.exists() and not overwrite:
        raise FileExistsError(f"{output_path} already exists; pass --overwrite to replace it")

    vertices: list[tuple[float, float, float]] = []
    indices: list[int] = []
    _append_box(vertices, indices, center=(0.0, 0.66, 0.0), half_extents=(0.06, 0.66, 0.06))
    _append_box(vertices, indices, center=(0.0, 1.18, 0.0), half_extents=(0.58, 0.24, 0.045))
    _append_box(vertices, indices, center=(0.0, 0.04, 0.0), half_extents=(0.22, 0.04, 0.16))

    _write_embedded_gltf(
        output_path=output_path,
        vertices=vertices,
        indices=indices,
        name="ferry_notice_board",
        generator="Tidebreak tools/create_simple_prop_gltf.py v0.20 fallback helper",
    )


def write_clearance_tag(output_path: pathlib.Path, overwrite: bool = True) -> None:
    if output_path.exists() and not overwrite:
        raise FileExistsError(f"{output_path} already exists; pass --overwrite to replace it")

    vertices: list[tuple[float, float, float]] = []
    indices: list[int] = []
    _append_box(vertices, indices, center=(0.0, 0.04, 0.0), half_extents=(0.14, 0.04, 0.10))
    _append_box(vertices, indices, center=(0.0, 0.42, 0.0), half_extents=(0.035, 0.38, 0.035))
    _append_box(vertices, indices, center=(0.0, 0.78, 0.0), half_extents=(0.26, 0.12, 0.035))
    _append_box(vertices, indices, center=(-0.18, 0.92, 0.0), half_extents=(0.055, 0.05, 0.03))

    _write_embedded_gltf(
        output_path=output_path,
        vertices=vertices,
        indices=indices,
        name="clearance_tag",
        generator="Tidebreak tools/create_simple_prop_gltf.py v0.58 clearance tag fallback helper",
    )


def _write_embedded_gltf(
    output_path: pathlib.Path,
    vertices: list[tuple[float, float, float]],
    indices: list[int],
    name: str,
    generator: str,
) -> None:
    vertex_bytes = b"".join(struct.pack("<fff", *vertex) for vertex in vertices)
    index_bytes = b"".join(struct.pack("<H", index) for index in indices)
    index_offset = _align4(len(vertex_bytes))
    binary = vertex_bytes + (b"\x00" * (index_offset - len(vertex_bytes))) + index_bytes

    mins = [min(vertex[axis] for vertex in vertices) for axis in range(3)]
    maxs = [max(vertex[axis] for vertex in vertices) for axis in range(3)]
    gltf = {
        "asset": {
            "version": "2.0",
            "generator": generator,
            "extras": {
                "tidebreakSource": "project-original fallback geometry",
                "notBlenderExport": True,
            },
        },
        "scene": 0,
        "scenes": [{"nodes": [0]}],
        "nodes": [{"mesh": 0, "name": name}],
        "meshes": [
            {
                "name": name,
                "primitives": [
                    {
                        "attributes": {"POSITION": 0},
                        "indices": 1,
                        "mode": 4,
                    }
                ],
            }
        ],
        "buffers": [
            {
                "byteLength": len(binary),
                "uri": "data:application/octet-stream;base64," + base64.b64encode(binary).decode("ascii"),
            }
        ],
        "bufferViews": [
            {
                "buffer": 0,
                "byteOffset": 0,
                "byteLength": len(vertex_bytes),
                "target": 34962,
            },
            {
                "buffer": 0,
                "byteOffset": index_offset,
                "byteLength": len(index_bytes),
                "target": 34963,
            },
        ],
        "accessors": [
            {
                "bufferView": 0,
                "byteOffset": 0,
                "componentType": 5126,
                "count": len(vertices),
                "type": "VEC3",
                "min": [round(value, 6) for value in mins],
                "max": [round(value, 6) for value in maxs],
            },
            {
                "bufferView": 1,
                "byteOffset": 0,
                "componentType": 5123,
                "count": len(indices),
                "type": "SCALAR",
                "min": [min(indices)],
                "max": [max(indices)],
            },
        ],
    }

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(gltf, indent=2) + "\n", encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Create a tiny original Tidebreak fallback prop glTF.")
    parser.add_argument("--output", default=str(DEFAULT_OUTPUT), help="Output .gltf path.")
    parser.add_argument("--overwrite", action="store_true", help="Replace an existing output file.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    output_path = pathlib.Path(args.output)
    try:
        write_ferry_notice_board(output_path, overwrite=args.overwrite)
    except FileExistsError as exc:
        print(f"[error] {exc}")
        return 1
    print(f"Wrote fallback Tidebreak prop: {output_path}")
    print("Note: this is generated fallback geometry, not a Blender export.")
    return 0


def _append_box(
    vertices: list[tuple[float, float, float]],
    indices: list[int],
    center: tuple[float, float, float],
    half_extents: tuple[float, float, float],
) -> None:
    cx, cy, cz = center
    hx, hy, hz = half_extents
    corners = [
        (cx - hx, cy - hy, cz - hz),
        (cx + hx, cy - hy, cz - hz),
        (cx + hx, cy + hy, cz - hz),
        (cx - hx, cy + hy, cz - hz),
        (cx - hx, cy - hy, cz + hz),
        (cx + hx, cy - hy, cz + hz),
        (cx + hx, cy + hy, cz + hz),
        (cx - hx, cy + hy, cz + hz),
    ]
    face_indices = [
        (0, 1, 2, 0, 2, 3),
        (5, 4, 7, 5, 7, 6),
        (4, 0, 3, 4, 3, 7),
        (1, 5, 6, 1, 6, 2),
        (3, 2, 6, 3, 6, 7),
        (4, 5, 1, 4, 1, 0),
    ]
    base_index = len(vertices)
    vertices.extend(corners)
    for face in face_indices:
        indices.extend(base_index + index for index in face)


def _align4(value: int) -> int:
    return (value + 3) & ~3


if __name__ == "__main__":
    sys.exit(main())
