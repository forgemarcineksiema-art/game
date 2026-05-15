#!/usr/bin/env python3
"""Unit tests for the renderer-owned visual capture smoke harness."""

from __future__ import annotations

import json
import pathlib
import struct
import sys
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import capture_visual_smoke  # noqa: E402


def write_test_bmp(path: pathlib.Path, width: int, height: int, pixels: list[tuple[int, int, int, int]]) -> None:
    assert len(pixels) == width * height
    pixel_bytes = b"".join(bytes((blue, green, red, alpha)) for blue, green, red, alpha in pixels)
    file_size = 54 + len(pixel_bytes)
    header = bytearray()
    header += b"BM"
    header += struct.pack("<IHHI", file_size, 0, 0, 54)
    header += struct.pack("<IiiHHIIiiII", 40, width, -height, 1, 32, 0, len(pixel_bytes), 0, 0, 0, 0)
    path.write_bytes(bytes(header) + pixel_bytes)


class CaptureVisualSmokeTests(unittest.TestCase):
    def test_visual_stats_track_scene_presence_buckets(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = pathlib.Path(temp_dir) / "scene.bmp"
            pixels = [
                (18, 24, 34, 255),
                (22, 30, 42, 255),
                (78, 210, 38, 255),
                (236, 112, 54, 255),
                (30, 36, 44, 255),
                (50, 70, 240, 255),
                (245, 235, 230, 255),
                (18, 24, 34, 255),
                (78, 210, 38, 255),
                (236, 112, 54, 255),
                (50, 70, 240, 255),
                (138, 130, 120, 255),
                (18, 24, 34, 255),
                (22, 30, 42, 255),
                (245, 235, 230, 255),
                (138, 130, 120, 255),
            ]
            write_test_bmp(path, 4, 4, pixels)

            thresholds = capture_visual_smoke.VisualThresholds(
                expected_width=4,
                expected_height=4,
                min_unique_colors=6,
                min_different_pixels=8,
                min_luminance_range=120,
                min_dark_pixels=4,
                min_bright_pixels=2,
                min_warm_pixels=2,
                min_green_pixels=2,
                min_cool_pixels=2,
                min_overlay_text_pixels=0,
            )

            stats = capture_visual_smoke.analyze_bmp_capture(path, thresholds)
            capture_visual_smoke.validate_capture_stats(stats, thresholds, "gdi")

            self.assertEqual(4, stats["width"])
            self.assertEqual(4, stats["height"])
            self.assertGreaterEqual(stats["unique_colors"], 6)
            self.assertGreaterEqual(stats["luminance_range"], 120)
            self.assertGreaterEqual(stats["scene_presence"]["dark_pixels"], 4)
            self.assertGreaterEqual(stats["scene_presence"]["warm_pixels"], 2)
            self.assertGreaterEqual(stats["scene_presence"]["green_pixels"], 2)
            self.assertGreaterEqual(stats["scene_presence"]["cool_pixels"], 2)

    def test_visual_stats_reject_flat_capture(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = pathlib.Path(temp_dir) / "flat.bmp"
            write_test_bmp(path, 4, 4, [(18, 24, 34, 255)] * 16)

            thresholds = capture_visual_smoke.VisualThresholds(
                expected_width=4,
                expected_height=4,
                min_unique_colors=2,
                min_different_pixels=2,
                min_luminance_range=10,
                min_dark_pixels=4,
            )

            stats = capture_visual_smoke.analyze_bmp_capture(path, thresholds)
            with self.assertRaisesRegex(ValueError, "flat|unique|luminance"):
                capture_visual_smoke.validate_capture_stats(stats, thresholds, "gdi")

    def test_green_bucket_accepts_tidebreak_teal_marker(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = pathlib.Path(temp_dir) / "teal-marker.bmp"
            write_test_bmp(
                path,
                2,
                2,
                [
                    (18, 24, 34, 255),
                    (91, 102, 28, 255),
                    (91, 102, 28, 255),
                    (236, 112, 54, 255),
                ],
            )

            stats = capture_visual_smoke.analyze_bmp_capture(path)

            self.assertGreaterEqual(stats["scene_presence"]["green_pixels"], 2)

    def test_overlay_text_signal_counts_top_left_bright_neutral_pixels(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = pathlib.Path(temp_dir) / "overlay-text.bmp"
            pixels = [
                (245, 235, 230, 255),
                (245, 235, 230, 255),
                (18, 24, 34, 255),
                (18, 24, 34, 255),
                (245, 235, 230, 255),
                (245, 235, 230, 255),
                (18, 24, 34, 255),
                (18, 24, 34, 255),
                (18, 24, 34, 255),
                (18, 24, 34, 255),
                (54, 112, 236, 255),
                (78, 210, 38, 255),
                (18, 24, 34, 255),
                (18, 24, 34, 255),
                (54, 112, 236, 255),
                (78, 210, 38, 255),
            ]
            write_test_bmp(path, 4, 4, pixels)

            thresholds = capture_visual_smoke.VisualThresholds(
                expected_width=4,
                expected_height=4,
                min_unique_colors=4,
                min_different_pixels=6,
                min_luminance_range=120,
                min_dark_pixels=4,
                min_bright_pixels=4,
                min_warm_pixels=2,
                min_green_pixels=2,
                min_cool_pixels=0,
                min_overlay_text_pixels=4,
            )

            stats = capture_visual_smoke.analyze_bmp_capture(path, thresholds)
            capture_visual_smoke.validate_capture_stats(stats, thresholds, "dx11")

            self.assertGreaterEqual(stats["scene_presence"]["overlay_text_pixels"], 4)

    def test_capture_parity_rejects_dimension_mismatch(self) -> None:
        gdi_stats = {"renderer": "gdi", "width": 1280, "height": 720}
        dx11_stats = {"renderer": "dx11", "width": 640, "height": 720}

        with self.assertRaisesRegex(ValueError, "dimension"):
            capture_visual_smoke.compare_capture_parity({"gdi": gdi_stats, "dx11": dx11_stats})

    def test_report_json_records_renderer_stats_and_parity(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            report_path = pathlib.Path(temp_dir) / "visual-report.json"
            captures = {
                "gdi": {"renderer": "gdi", "path": "gdi.bmp", "width": 1280, "height": 720, "unique_colors": 32},
                "dx11": {"renderer": "dx11", "path": "dx11.bmp", "width": 1280, "height": 720, "unique_colors": 36},
            }
            parity = {"dimensions_match": True, "unique_color_delta": 4}

            capture_visual_smoke.write_report(report_path, captures, parity)

            report = json.loads(report_path.read_text(encoding="utf-8"))
            self.assertEqual("v0.31-capture-visual-smoke", report["schema"])
            self.assertEqual(1280, report["captures"]["gdi"]["width"])
            self.assertTrue(report["parity"]["dimensions_match"])


if __name__ == "__main__":
    unittest.main()
