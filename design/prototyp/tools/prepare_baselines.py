#!/usr/bin/env python3
"""Derive Nakama Gen artboards and scale assets from the immutable user exports.

The user exports are presentation images (3840x2520).  Their central
3040x1720 rectangles are the four-times rendered 760x430 plug-in surfaces.
Every output scale is sampled directly from that 4x crop; no tier is enlarged
from the 100% image.
"""

from __future__ import annotations

import hashlib
import json
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[2] / "assets" / "figma" / "nakama-gen-2026-08-24"
GOLDENS = ROOT / "goldens"
SCALES = ROOT / "scales"
CROP_BOX = (400, 400, 3440, 2120)
EXPECTED_SOURCE_SIZE = (3840, 2520)
EXPECTED_ARTBOARD_SIZE = (3040, 1720)
TARGETS = {
    "1x": (760, 430),
    "1.25x": (950, 538),
    "1.5x": (1140, 645),
    "2x": (1520, 860),
}
SOURCES = {
    "overview": GOLDENS / "overview-user-original-3840x2520.png",
    "eq-center": GOLDENS / "eq-center-user-original-3840x2520.png",
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def write_png(image: Image.Image, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    image.save(path, format="PNG", optimize=True)


def main() -> None:
    report: dict[str, object] = {
        "crop": list(CROP_BOX),
        "sourceSize": list(EXPECTED_SOURCE_SIZE),
        "artboardSize": list(EXPECTED_ARTBOARD_SIZE),
        "files": {},
    }

    for surface, source_path in SOURCES.items():
        if not source_path.is_file():
            raise SystemExit(f"Missing immutable source export: {source_path}")

        with Image.open(source_path) as source:
            if source.size != EXPECTED_SOURCE_SIZE:
                raise SystemExit(
                    f"Unexpected size for {source_path.name}: {source.size}, "
                    f"expected {EXPECTED_SOURCE_SIZE}"
                )
            crop = source.convert("RGBA").crop(CROP_BOX)

        if crop.size != EXPECTED_ARTBOARD_SIZE:
            raise SystemExit(f"Crop invariant failed for {surface}: {crop.size}")

        artboard_path = GOLDENS / f"{surface}-artboard@4x.png"
        write_png(crop, artboard_path)
        outputs: dict[str, object] = {
            "source": {
                "path": source_path.relative_to(ROOT).as_posix(),
                "sha256": sha256(source_path),
            },
            "artboard4x": {
                "path": artboard_path.relative_to(ROOT).as_posix(),
                "sha256": sha256(artboard_path),
                "width": crop.width,
                "height": crop.height,
            },
            "scales": {},
        }

        for tier, size in TARGETS.items():
            scaled = crop.resize(size, Image.Resampling.LANCZOS)
            target_path = SCALES / f"{surface}@{tier}.png"
            write_png(scaled, target_path)
            outputs["scales"][tier] = {
                "path": target_path.relative_to(ROOT).as_posix(),
                "sha256": sha256(target_path),
                "width": size[0],
                "height": size[1],
                "derivedDirectlyFrom": "artboard4x",
            }

        report["files"][surface] = outputs

    print(json.dumps(report, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
