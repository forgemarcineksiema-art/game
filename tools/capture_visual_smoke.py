#!/usr/bin/env python3
"""Run bounded renderer-owned frame captures and validate the BMP output."""

from __future__ import annotations

import argparse
from dataclasses import asdict, dataclass
import json
import pathlib
import struct
import subprocess
import sys
from typing import Any


ROOT = pathlib.Path(__file__).resolve().parents[1]
DEFAULT_EXE = ROOT / "build" / "windows-vs2022-debug" / "Debug" / "EngineApp.exe"
DEFAULT_SCENE = ROOT / "data" / "scenes" / "ferry_office.scene.json"


@dataclass(frozen=True)
class VisualThresholds:
    expected_width: int | None = 1280
    expected_height: int | None = 720
    min_unique_colors: int = 12
    min_different_pixels: int = 500
    min_luminance_range: int = 55
    min_dark_pixels: int = 500
    min_bright_pixels: int = 0
    min_warm_pixels: int = 0
    min_green_pixels: int = 0
    min_cool_pixels: int = 0
    min_neutral_pixels: int = 4
    min_overlay_text_pixels: int = 80


def resolve_repo_path(value: str | pathlib.Path) -> pathlib.Path:
    path = pathlib.Path(value)
    if path.is_absolute():
        return path
    return ROOT / path


def read_bmp_pixels(path: pathlib.Path) -> tuple[int, int, bytes, int]:
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
    if height_signed > 0:
        row_bytes = width * 4
        rows = [pixels[index : index + row_bytes] for index in range(0, len(pixels), row_bytes)]
        pixels = b"".join(reversed(rows))

    return width, height, pixels, len(data)


def analyze_bmp_capture(path: pathlib.Path, thresholds: VisualThresholds | None = None) -> dict[str, Any]:
    width, height, pixels, byte_count = read_bmp_pixels(path)
    unique_colors: set[tuple[int, int, int]] = set()
    different_pixels = 0
    luminance_min = 255
    luminance_max = 0
    luminance_total = 0
    red_min = green_min = blue_min = 255
    red_max = green_max = blue_max = 0
    scene_presence = {
        "dark_pixels": 0,
        "bright_pixels": 0,
        "warm_pixels": 0,
        "green_pixels": 0,
        "cool_pixels": 0,
        "neutral_pixels": 0,
        "overlay_text_pixels": 0,
    }

    first_bgr = pixels[:3]
    for index in range(0, len(pixels), 4):
        pixel_index = index // 4
        x = pixel_index % width
        y = pixel_index // width
        pixel = pixels[index : index + 4]
        blue = pixel[0]
        green = pixel[1]
        red = pixel[2]
        unique_colors.add((blue, green, red))
        if pixel[:3] != first_bgr:
            different_pixels += 1

        luminance = int((0.2126 * red) + (0.7152 * green) + (0.0722 * blue))
        luminance_min = min(luminance_min, luminance)
        luminance_max = max(luminance_max, luminance)
        luminance_total += luminance

        red_min = min(red_min, red)
        red_max = max(red_max, red)
        green_min = min(green_min, green)
        green_max = max(green_max, green)
        blue_min = min(blue_min, blue)
        blue_max = max(blue_max, blue)

        if luminance <= 55:
            scene_presence["dark_pixels"] += 1
        if luminance >= 200:
            scene_presence["bright_pixels"] += 1
        if red >= 145 and red >= green + 35 and red >= blue + 35:
            scene_presence["warm_pixels"] += 1
        if green >= 95 and green >= red + 25 and green >= blue - 15:
            scene_presence["green_pixels"] += 1
        if blue >= 130 and blue >= red + 25 and blue >= green + 10:
            scene_presence["cool_pixels"] += 1
        if max(red, green, blue) - min(red, green, blue) <= 24 and luminance >= 70:
            scene_presence["neutral_pixels"] += 1
        if x < min(width, 760) and y < min(height, 180) and luminance >= 185 and max(red, green, blue) - min(red, green, blue) <= 35:
            scene_presence["overlay_text_pixels"] += 1

    pixel_count = width * height
    stats: dict[str, Any] = {
        "path": str(path),
        "width": width,
        "height": height,
        "bytes": byte_count,
        "pixel_count": pixel_count,
        "unique_colors": len(unique_colors),
        "different_pixels": different_pixels,
        "luminance_min": luminance_min,
        "luminance_max": luminance_max,
        "luminance_range": luminance_max - luminance_min,
        "luminance_average": round(luminance_total / pixel_count, 2),
        "channels": {
            "red_range": red_max - red_min,
            "green_range": green_max - green_min,
            "blue_range": blue_max - blue_min,
        },
        "scene_presence": scene_presence,
    }

    if thresholds is not None:
        stats["thresholds"] = asdict(thresholds)

    return stats


def validate_capture_stats(stats: dict[str, Any], thresholds: VisualThresholds, renderer: str) -> None:
    if thresholds.expected_width is not None and stats["width"] != thresholds.expected_width:
        raise ValueError(f"{renderer} capture width {stats['width']} did not match expected {thresholds.expected_width}.")
    if thresholds.expected_height is not None and stats["height"] != thresholds.expected_height:
        raise ValueError(f"{renderer} capture height {stats['height']} did not match expected {thresholds.expected_height}.")
    if stats["unique_colors"] < thresholds.min_unique_colors:
        raise ValueError(f"{renderer} capture has only {stats['unique_colors']} unique colors.")
    if stats["different_pixels"] < thresholds.min_different_pixels:
        raise ValueError(f"{renderer} capture looks flat: only {stats['different_pixels']} pixels differ from the first pixel.")
    if stats["luminance_range"] < thresholds.min_luminance_range:
        raise ValueError(f"{renderer} capture luminance range {stats['luminance_range']} is too small.")

    scene_presence = stats["scene_presence"]
    required_scene_buckets = {
        "dark_pixels": thresholds.min_dark_pixels,
        "bright_pixels": thresholds.min_bright_pixels,
        "warm_pixels": thresholds.min_warm_pixels,
        "green_pixels": thresholds.min_green_pixels,
        "cool_pixels": thresholds.min_cool_pixels,
        "neutral_pixels": thresholds.min_neutral_pixels,
        "overlay_text_pixels": thresholds.min_overlay_text_pixels,
    }
    for bucket, minimum in required_scene_buckets.items():
        if minimum > 0 and scene_presence[bucket] < minimum:
            raise ValueError(f"{renderer} capture is missing expected scene signal {bucket}: {scene_presence[bucket]} < {minimum}.")


def compare_capture_parity(captures: dict[str, dict[str, Any]]) -> dict[str, Any]:
    if len(captures) < 2:
        return {
            "compared": False,
            "reason": "Only one renderer was requested.",
        }

    ordered = sorted(captures)
    baseline_name = ordered[0]
    baseline = captures[baseline_name]
    parity: dict[str, Any] = {
        "compared": True,
        "baseline": baseline_name,
        "dimensions_match": True,
        "renderers": ordered,
    }
    for renderer in ordered[1:]:
        stats = captures[renderer]
        if stats["width"] != baseline["width"] or stats["height"] != baseline["height"]:
            raise ValueError(
                f"Capture dimension mismatch: {baseline_name}={baseline['width']}x{baseline['height']}, "
                f"{renderer}={stats['width']}x{stats['height']}."
            )

    if "gdi" in captures and "dx11" in captures:
        gdi = captures["gdi"]
        dx11 = captures["dx11"]
        parity.update(
            {
                "unique_color_delta": abs(gdi["unique_colors"] - dx11["unique_colors"]),
                "luminance_range_delta": abs(gdi["luminance_range"] - dx11["luminance_range"]),
                "dark_pixel_delta": abs(gdi["scene_presence"]["dark_pixels"] - dx11["scene_presence"]["dark_pixels"]),
                "warm_pixel_delta": abs(gdi["scene_presence"]["warm_pixels"] - dx11["scene_presence"]["warm_pixels"]),
                "green_pixel_delta": abs(gdi["scene_presence"]["green_pixels"] - dx11["scene_presence"]["green_pixels"]),
                "cool_pixel_delta": abs(gdi["scene_presence"]["cool_pixels"] - dx11["scene_presence"]["cool_pixels"]),
            }
        )

    return parity


def write_report(report_path: pathlib.Path, captures: dict[str, dict[str, Any]], parity: dict[str, Any], scenario: str = "initial") -> None:
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report = {
        "schema": "v0.31-capture-visual-smoke",
        "scenario": scenario,
        "captures": captures,
        "parity": parity,
    }
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def read_bmp_stats(path: pathlib.Path) -> dict[str, Any]:
    return analyze_bmp_capture(path)


def qa_capture_state_args(scenario: str) -> list[str]:
    if scenario == "initial":
        return []
    if scenario == "relay-to-service-log":
        return ["--qa-capture-state", "relay-to-service-log"]
    raise ValueError(f"Unsupported capture scenario '{scenario}'.")


def run_capture(
    exe: pathlib.Path,
    scene: pathlib.Path,
    renderer: str,
    output_path: pathlib.Path,
    frames: int,
    thresholds: VisualThresholds,
    scenario: str = "initial",
) -> dict[str, Any]:
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
    command.extend(qa_capture_state_args(scenario))
    print("Running:", " ".join(command))
    result = subprocess.run(command, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False)
    print(result.stdout, end="")
    if result.returncode != 0:
        raise RuntimeError(f"{renderer} capture command failed with exit code {result.returncode}.")
    if not output_path.exists():
        raise FileNotFoundError(f"{renderer} capture did not create {output_path}.")

    stats = analyze_bmp_capture(output_path, thresholds)
    stats["renderer"] = renderer
    validate_capture_stats(stats, thresholds, renderer)
    print(
        f"{renderer}: {stats['width']}x{stats['height']}, "
        f"{stats['bytes']} bytes, colors={stats['unique_colors']}, "
        f"lumaRange={stats['luminance_range']}, "
        f"warm/green/cool/neutral="
        f"{stats['scene_presence']['warm_pixels']}/"
        f"{stats['scene_presence']['green_pixels']}/"
        f"{stats['scene_presence']['cool_pixels']}/"
        f"{stats['scene_presence']['neutral_pixels']}, "
        f"text={stats['scene_presence']['overlay_text_pixels']}"
    )
    return stats


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", default=str(DEFAULT_EXE), help="Path to EngineApp.exe.")
    parser.add_argument("--scene", default=str(DEFAULT_SCENE), help="Runtime scene JSON path.")
    parser.add_argument("--output-dir", default=str(ROOT / "build" / "captures"), help="Directory for BMP captures.")
    parser.add_argument("--report-json", default="", help="Optional JSON report path. Defaults into --output-dir.")
    parser.add_argument("--frames", type=int, default=6, help="Bounded run length; capture occurs after a stable frame.")
    parser.add_argument(
        "--scenario",
        choices=("initial", "relay-to-service-log"),
        default="initial",
        help="Capture state to preload. 'relay-to-service-log' exercises mid-chain playtest route guidance.",
    )
    parser.add_argument("--expected-width", type=int, default=1280, help="Expected capture width.")
    parser.add_argument("--expected-height", type=int, default=720, help="Expected capture height.")
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
    report_path = resolve_repo_path(args.report_json) if args.report_json else output_dir / "capture_visual_smoke_report.json"
    renderers = args.renderer or ["gdi", "dx11"]
    thresholds = VisualThresholds(expected_width=args.expected_width, expected_height=args.expected_height)

    if not exe.exists():
        print(f"EngineApp executable was not found: {exe}", file=sys.stderr)
        return 2
    if not scene.exists():
        print(f"Scene file was not found: {scene}", file=sys.stderr)
        return 2

    try:
        captures: dict[str, dict[str, Any]] = {}
        for renderer in renderers:
            capture_name = f"v0.31-{renderer}-capture.bmp"
            if args.scenario != "initial":
                capture_name = f"v0.94-{args.scenario}-{renderer}-capture.bmp"
            captures[renderer] = run_capture(
                exe,
                scene,
                renderer,
                output_dir / capture_name,
                args.frames,
                thresholds,
                args.scenario,
            )
        parity = compare_capture_parity(captures)
        write_report(report_path, captures, parity, args.scenario)
        print(f"Wrote visual smoke report: {report_path}")
    except Exception as exc:
        print(f"Capture visual smoke failed: {exc}", file=sys.stderr)
        return 1

    print("Capture visual smoke passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
