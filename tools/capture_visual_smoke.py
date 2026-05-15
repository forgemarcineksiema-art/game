#!/usr/bin/env python3
"""Run bounded renderer-owned frame captures and validate the BMP output."""

from __future__ import annotations

import argparse
import pathlib
import struct
import subprocess
import sys


ROOT = pathlib.Path(__file__).resolve().parents[1]
DEFAULT_EXE = ROOT / "build" / "windows-vs2022-debug" / "Debug" / "EngineApp.exe"
DEFAULT_SCENE = ROOT / "data" / "scenes" / "ferry_office.scene.json"


def resolve_repo_path(value: str | pathlib.Path) -> pathlib.Path:
    path = pathlib.Path(value)
    if path.is_absolute():
        return path
    return ROOT / path


def read_bmp_stats(path: pathlib.Path) -> dict[str, int]:
    data = path.read_bytes()
    if len(data) < 54:
        raise ValueError(f"{path} is too small to be a BMP capture.")
    if data[:2] != b"BM":
        raise ValueError(f"{path} does not start with a BMP signature.")

    file_size = struct.unpack_from("<I", data, 2)[0]
    pixel_offset = struct.unpack_from("<I", data, 10)[0]
    info_header_size = struct.unpack_from("<I", data, 14)[0]
    width = struct.unpack_from("<i", data, 18)[0]
    height_signed = struct.unpack_from("<i", data, 22)[0]
    planes = struct.unpack_from("<H", data, 26)[0]
    bits_per_pixel = struct.unpack_from("<H", data, 28)[0]
    compression = struct.unpack_from("<I", data, 30)[0]
    height = abs(height_signed)

    if file_size != len(data):
        raise ValueError(f"{path} reports {file_size} bytes but contains {len(data)} bytes.")
    if info_header_size != 40 or planes != 1 or bits_per_pixel != 32 or compression != 0:
        raise ValueError(f"{path} is not an uncompressed 32-bit BMP capture.")
    if width <= 0 or height <= 0:
        raise ValueError(f"{path} has invalid dimensions {width}x{height_signed}.")

    expected_pixel_bytes = width * height * 4
    if pixel_offset + expected_pixel_bytes > len(data):
        raise ValueError(f"{path} pixel data is truncated.")

    pixels = data[pixel_offset : pixel_offset + expected_pixel_bytes]
    unique_colors: set[bytes] = set()
    colored_pixels = 0
    for index in range(0, len(pixels), 4):
        pixel = pixels[index : index + 4]
        unique_colors.add(pixel)
        if pixel[:3] != pixels[:3]:
            colored_pixels += 1
        if len(unique_colors) > 256 and colored_pixels > 1000:
            break

    if len(unique_colors) < 2 or colored_pixels < 100:
        raise ValueError(f"{path} looks blank: only {len(unique_colors)} sampled color(s).")

    return {
        "width": width,
        "height": height,
        "bytes": len(data),
        "unique_colors": len(unique_colors),
        "colored_pixels": colored_pixels,
    }


def run_capture(exe: pathlib.Path, scene: pathlib.Path, renderer: str, output_path: pathlib.Path, frames: int) -> dict[str, int]:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    if output_path.exists():
        output_path.unlink()

    command = [
        str(exe),
        "--renderer",
        renderer,
        "--ui-mode",
        "playtest",
        "--scene",
        str(scene),
        "--frames",
        str(frames),
        "--free-cursor",
        "--capture-frame",
        str(output_path),
    ]
    print("Running:", " ".join(command))
    result = subprocess.run(command, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False)
    print(result.stdout, end="")
    if result.returncode != 0:
        raise RuntimeError(f"{renderer} capture command failed with exit code {result.returncode}.")
    if not output_path.exists():
        raise FileNotFoundError(f"{renderer} capture did not create {output_path}.")

    stats = read_bmp_stats(output_path)
    print(
        f"{renderer}: {stats['width']}x{stats['height']}, "
        f"{stats['bytes']} bytes, sampled colors={stats['unique_colors']}"
    )
    return stats


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", default=str(DEFAULT_EXE), help="Path to EngineApp.exe.")
    parser.add_argument("--scene", default=str(DEFAULT_SCENE), help="Runtime scene JSON path.")
    parser.add_argument("--output-dir", default=str(ROOT / "build" / "captures"), help="Directory for BMP captures.")
    parser.add_argument("--frames", type=int, default=6, help="Bounded run length; capture occurs after a stable frame.")
    parser.add_argument(
        "--renderer",
        action="append",
        choices=("gdi", "dx11"),
        help="Renderer to capture. Repeatable; defaults to both GDI and DX11.",
    )
    args = parser.parse_args()

    exe = resolve_repo_path(args.exe)
    scene = resolve_repo_path(args.scene)
    output_dir = resolve_repo_path(args.output_dir)
    renderers = args.renderer or ["gdi", "dx11"]

    if not exe.exists():
        print(f"EngineApp executable was not found: {exe}", file=sys.stderr)
        return 2
    if not scene.exists():
        print(f"Scene file was not found: {scene}", file=sys.stderr)
        return 2

    try:
        for renderer in renderers:
            run_capture(exe, scene, renderer, output_dir / f"v0.29-{renderer}-capture.bmp", args.frames)
    except Exception as exc:
        print(f"Capture visual smoke failed: {exc}", file=sys.stderr)
        return 1

    print("Capture visual smoke passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
