#!/usr/bin/env python3
"""Generate tiny original placeholder glTF props in Tidebreak's supported subset.

This is a fallback helper, not a Blender exporter. Use it only when the goal is
to keep simple original placeholder assets flowing while Blender is unavailable.
"""

from __future__ import annotations

import argparse
import base64
import json
import math
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


def write_ferry_office_canopy(output_path: pathlib.Path, overwrite: bool = True) -> None:
    if output_path.exists() and not overwrite:
        raise FileExistsError(f"{output_path} already exists; pass --overwrite to replace it")

    vertices: list[tuple[float, float, float]] = []
    indices: list[int] = []
    _append_roof_canopy(vertices, indices)
    _append_box(vertices, indices, center=(-2.88, 0.17, 1.85), half_extents=(0.07, 0.16, 1.76))
    _append_box(vertices, indices, center=(2.88, 0.17, 1.85), half_extents=(0.07, 0.16, 1.76))

    _write_embedded_gltf(
        output_path=output_path,
        vertices=vertices,
        indices=indices,
        name="ferry_office_canopy",
        generator="Tidebreak tools/create_simple_prop_gltf.py v0.81 ferry office canopy fallback helper",
    )


def write_ferry_office_service_gate(output_path: pathlib.Path, overwrite: bool = True) -> None:
    if output_path.exists() and not overwrite:
        raise FileExistsError(f"{output_path} already exists; pass --overwrite to replace it")

    vertices: list[tuple[float, float, float]] = []
    indices: list[int] = []
    _append_box(vertices, indices, center=(0.0, 0.75, 0.0), half_extents=(2.45, 0.72, 0.10))
    _append_box(vertices, indices, center=(-1.22, 0.76, -0.12), half_extents=(1.08, 0.50, 0.055))
    _append_box(vertices, indices, center=(1.22, 0.76, -0.12), half_extents=(1.08, 0.50, 0.055))
    _append_box(vertices, indices, center=(0.0, 0.76, -0.18), half_extents=(0.055, 0.62, 0.08))
    _append_box(vertices, indices, center=(0.0, 1.28, -0.18), half_extents=(2.28, 0.07, 0.08))
    _append_box(vertices, indices, center=(0.0, 0.34, -0.18), half_extents=(2.28, 0.07, 0.08))
    _append_box(vertices, indices, center=(-2.32, 0.74, -0.18), half_extents=(0.08, 0.70, 0.08))
    _append_box(vertices, indices, center=(2.32, 0.74, -0.18), half_extents=(0.08, 0.70, 0.08))

    _write_embedded_gltf(
        output_path=output_path,
        vertices=vertices,
        indices=indices,
        name="ferry_office_service_gate",
        generator="Tidebreak tools/create_simple_prop_gltf.py v0.85 ferry office service gate fallback helper",
    )


def write_service_yard_cart_body(output_path: pathlib.Path, overwrite: bool = True) -> None:
    if output_path.exists() and not overwrite:
        raise FileExistsError(f"{output_path} already exists; pass --overwrite to replace it")

    vertices: list[tuple[float, float, float]] = []
    indices: list[int] = []
    _append_tapered_box(
        vertices,
        indices,
        y_min=0.18,
        y_max=0.62,
        lower_half_extents=(0.58, 0.96),
        upper_half_extents=(0.48, 0.84),
    )
    _append_tapered_box(
        vertices,
        indices,
        y_min=0.54,
        y_max=0.78,
        lower_half_extents=(0.44, 0.50),
        upper_half_extents=(0.36, 0.42),
        z_offset=0.38,
    )
    _append_box(vertices, indices, center=(-0.52, 0.78, -0.18), half_extents=(0.055, 0.12, 0.62))
    _append_box(vertices, indices, center=(0.52, 0.78, -0.18), half_extents=(0.055, 0.12, 0.62))
    _append_box(vertices, indices, center=(0.0, 0.78, -0.76), half_extents=(0.52, 0.12, 0.055))

    _write_embedded_gltf(
        output_path=output_path,
        vertices=vertices,
        indices=indices,
        name="service_yard_cart_body",
        generator="Tidebreak tools/create_simple_prop_gltf.py v0.84 service yard cart body fallback helper",
    )


def write_service_yard_cart_cabin(output_path: pathlib.Path, overwrite: bool = True) -> None:
    if output_path.exists() and not overwrite:
        raise FileExistsError(f"{output_path} already exists; pass --overwrite to replace it")

    vertices: list[tuple[float, float, float]] = []
    indices: list[int] = []
    _append_tapered_box(
        vertices,
        indices,
        y_min=0.62,
        y_max=1.08,
        lower_half_extents=(0.38, 0.36),
        upper_half_extents=(0.30, 0.28),
        z_offset=-0.18,
        upper_z_offset=-0.08,
    )
    _append_box(vertices, indices, center=(0.0, 1.14, -0.10), half_extents=(0.36, 0.055, 0.34))

    _write_embedded_gltf(
        output_path=output_path,
        vertices=vertices,
        indices=indices,
        name="service_yard_cart_cabin",
        generator="Tidebreak tools/create_simple_prop_gltf.py v0.84 service yard cart cabin fallback helper",
    )


def write_service_yard_cart_wheel(output_path: pathlib.Path, overwrite: bool = True) -> None:
    if output_path.exists() and not overwrite:
        raise FileExistsError(f"{output_path} already exists; pass --overwrite to replace it")

    vertices: list[tuple[float, float, float]] = []
    indices: list[int] = []
    _append_wheel_cylinder(vertices, indices, radius=0.27, half_width=0.13, segments=12)

    _write_embedded_gltf(
        output_path=output_path,
        vertices=vertices,
        indices=indices,
        name="service_yard_cart_wheel",
        generator="Tidebreak tools/create_simple_prop_gltf.py v0.84 service yard cart wheel fallback helper",
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
    parser.add_argument(
        "--kind",
        choices=(
            "ferry-notice-board",
            "clearance-tag",
            "ferry-office-canopy",
            "ferry-office-service-gate",
            "service-yard-cart-body",
            "service-yard-cart-cabin",
            "service-yard-cart-wheel",
        ),
        default="ferry-notice-board",
        help="Prop kind to generate.",
    )
    parser.add_argument("--output", default=str(DEFAULT_OUTPUT), help="Output .gltf path.")
    parser.add_argument("--overwrite", action="store_true", help="Replace an existing output file.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    output_path = pathlib.Path(args.output)
    try:
        if args.kind == "clearance-tag":
            write_clearance_tag(output_path, overwrite=args.overwrite)
        elif args.kind == "ferry-office-canopy":
            write_ferry_office_canopy(output_path, overwrite=args.overwrite)
        elif args.kind == "ferry-office-service-gate":
            write_ferry_office_service_gate(output_path, overwrite=args.overwrite)
        elif args.kind == "service-yard-cart-body":
            write_service_yard_cart_body(output_path, overwrite=args.overwrite)
        elif args.kind == "service-yard-cart-cabin":
            write_service_yard_cart_cabin(output_path, overwrite=args.overwrite)
        elif args.kind == "service-yard-cart-wheel":
            write_service_yard_cart_wheel(output_path, overwrite=args.overwrite)
        else:
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


def _append_roof_canopy(
    vertices: list[tuple[float, float, float]],
    indices: list[int],
) -> None:
    base_index = len(vertices)
    vertices.extend(
        [
            (-2.95, 0.00, 0.00),
            (2.95, 0.00, 0.00),
            (2.95, 0.10, 3.70),
            (-2.95, 0.10, 3.70),
            (-2.95, 0.24, 0.00),
            (2.95, 0.24, 0.00),
            (2.95, 0.36, 3.70),
            (-2.95, 0.36, 3.70),
        ]
    )
    face_indices = [
        (0, 2, 1, 0, 3, 2),
        (4, 5, 6, 4, 6, 7),
        (0, 1, 5, 0, 5, 4),
        (3, 7, 6, 3, 6, 2),
        (0, 4, 7, 0, 7, 3),
        (1, 2, 6, 1, 6, 5),
    ]
    for face in face_indices:
        indices.extend(base_index + index for index in face)


def _append_tapered_box(
    vertices: list[tuple[float, float, float]],
    indices: list[int],
    y_min: float,
    y_max: float,
    lower_half_extents: tuple[float, float],
    upper_half_extents: tuple[float, float],
    z_offset: float = 0.0,
    upper_z_offset: float | None = None,
) -> None:
    lower_x, lower_z = lower_half_extents
    upper_x, upper_z = upper_half_extents
    top_z_offset = z_offset if upper_z_offset is None else upper_z_offset
    base_index = len(vertices)
    vertices.extend(
        [
            (-lower_x, y_min, z_offset - lower_z),
            (lower_x, y_min, z_offset - lower_z),
            (lower_x, y_min, z_offset + lower_z),
            (-lower_x, y_min, z_offset + lower_z),
            (-upper_x, y_max, top_z_offset - upper_z),
            (upper_x, y_max, top_z_offset - upper_z),
            (upper_x, y_max, top_z_offset + upper_z),
            (-upper_x, y_max, top_z_offset + upper_z),
        ]
    )
    face_indices = [
        (0, 1, 2, 0, 2, 3),
        (4, 7, 6, 4, 6, 5),
        (0, 4, 5, 0, 5, 1),
        (1, 5, 6, 1, 6, 2),
        (2, 6, 7, 2, 7, 3),
        (3, 7, 4, 3, 4, 0),
    ]
    for face in face_indices:
        indices.extend(base_index + index for index in face)


def _append_wheel_cylinder(
    vertices: list[tuple[float, float, float]],
    indices: list[int],
    radius: float,
    half_width: float,
    segments: int,
) -> None:
    base_index = len(vertices)
    for x in (-half_width, half_width):
        for segment in range(segments):
            angle = (segment / segments) * math.tau
            vertices.append((x, radius * 0.96 + radius * math.sin(angle), radius * math.cos(angle)))

    left_center = len(vertices)
    vertices.append((-half_width, radius * 0.96, 0.0))
    right_center = len(vertices)
    vertices.append((half_width, radius * 0.96, 0.0))

    for segment in range(segments):
        next_segment = (segment + 1) % segments
        left_a = base_index + segment
        left_b = base_index + next_segment
        right_a = base_index + segments + segment
        right_b = base_index + segments + next_segment
        indices.extend([left_a, right_a, right_b, left_a, right_b, left_b])
        indices.extend([left_center, left_b, left_a])
        indices.extend([right_center, right_a, right_b])


def _align4(value: int) -> int:
    return (value + 3) & ~3


if __name__ == "__main__":
    sys.exit(main())
