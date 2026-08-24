#!/usr/bin/env python3
"""Verify immutable goldens, hashes, dimensions and direct 4x scale derivation."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path

from PIL import Image, ImageChops


REPO = Path(__file__).resolve().parents[3]
MANIFEST_PATH = REPO / "design" / "prototyp" / "contract" / "asset-manifest.json"


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def require_file(record: dict[str, object], label: str) -> Path:
    path = REPO / str(record["path"])
    if not path.is_file():
        raise AssertionError(f"{label}: missing {path}")
    expected_hash = record.get("sha256")
    if expected_hash and sha256(path) != expected_hash:
        raise AssertionError(f"{label}: SHA-256 mismatch for {path}")
    return path


def require_dimensions(path: Path, record: dict[str, object], label: str) -> None:
    with Image.open(path) as image:
        actual = image.size
    expected = (int(record["width"]), int(record["height"]))
    if actual != expected:
        raise AssertionError(f"{label}: dimensions {actual}, expected {expected}")


def main() -> None:
    manifest = json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))
    checked = 0

    if manifest["logicalStage"] != {"width": 760, "height": 430, "unit": "css-px"}:
        raise AssertionError("Logical stage contract changed unexpectedly")

    crop = manifest["goldenCrop"]
    if [crop[key] for key in ("x", "y", "width", "height", "renderScale")] != [
        400,
        400,
        3040,
        1720,
        4,
    ]:
        raise AssertionError("Golden crop contract changed unexpectedly")

    for surface_name, surface in manifest["surfaces"].items():
        original_path = require_file(surface["userOriginal"], f"{surface_name}.original")
        require_dimensions(original_path, surface["userOriginal"], f"{surface_name}.original")
        checked += 1

        golden_path = require_file(surface["pixelGolden4x"], f"{surface_name}.golden4x")
        require_dimensions(golden_path, surface["pixelGolden4x"], f"{surface_name}.golden4x")
        checked += 1

        with Image.open(original_path) as original:
            expected_crop = original.convert("RGBA").crop((400, 400, 3440, 2120))
        with Image.open(golden_path) as golden:
            actual_crop = golden.convert("RGBA")
        if ImageChops.difference(expected_crop, actual_crop).getbbox() is not None:
            raise AssertionError(f"{surface_name}: 4x golden is not the exact central user-export crop")

        structure_path = require_file(
            surface["figmaStructureControl"], f"{surface_name}.figmaStructureControl"
        )
        require_dimensions(
            structure_path,
            surface["figmaStructureControl"],
            f"{surface_name}.figmaStructureControl",
        )
        if surface["figmaStructureControl"].get("isPixelGolden") is not False:
            raise AssertionError(f"{surface_name}: Figma renderer must never be marked as pixel golden")
        checked += 1

        for tier, scale in surface["qualityTiers"].items():
            scale_path = require_file(scale, f"{surface_name}.quality.{tier}")
            require_dimensions(scale_path, scale, f"{surface_name}.quality.{tier}")
            if scale.get("derivedDirectlyFrom") != "pixelGolden4x":
                raise AssertionError(f"{surface_name}.{tier}: tier is not declared as direct 4x derivation")
            expected = actual_crop.resize(
                (int(scale["width"]), int(scale["height"])), Image.Resampling.LANCZOS
            )
            with Image.open(scale_path) as actual:
                actual_rgba = actual.convert("RGBA")
            if ImageChops.difference(expected, actual_rgba).getbbox() is not None:
                raise AssertionError(
                    f"{surface_name}.{tier}: pixels are not a direct LANCZOS derivation of the 4x golden"
                )
            checked += 1

    for index, record in enumerate(manifest["runtimeAssets"]):
        require_file(record, f"runtimeAsset[{index}]")
        checked += 1

    for index, record in enumerate(manifest["fonts"]["files"]):
        require_file(record, f"font[{index}]")
        checked += 1

    print(f"Baseline verification passed: {checked} hashed files and all scale derivations are valid.")


if __name__ == "__main__":
    main()
