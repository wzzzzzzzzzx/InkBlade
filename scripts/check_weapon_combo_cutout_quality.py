from __future__ import annotations

from pathlib import Path

import numpy as np
from PIL import Image

import extract_weapon_combo_sprites as extractor


ROOT = Path(__file__).resolve().parents[1]
MINIMUM_CORE_DETAIL_RETENTION = 0.94
MINIMUM_UPPER_BODY_SKIN_RETENTION = 0.95


def retention_metrics(source: Image.Image, sprite: Image.Image) -> tuple[float, float]:
    source_rgb = np.asarray(source.convert("RGB"))
    output_rgba = np.asarray(sprite.convert("RGBA"))
    height, width, _ = source_rgb.shape
    average = source_rgb.mean(axis=2)
    saturation = source_rgb.max(axis=2) - source_rgb.min(axis=2)
    red = source_rgb[:, :, 0]
    green = source_rgb[:, :, 1]
    blue = source_rgb[:, :, 2]
    rows, columns = np.indices((height, width))

    core_detail = (saturation >= 28) | (average <= 190)
    upper_body_zone = (
        (columns >= width * 0.18)
        & (columns <= width * 0.75)
        & (rows >= height * 0.15)
        & (rows <= height * 0.58)
    )
    skin_detail = (
        (red >= 155)
        & (green >= 110)
        & (blue >= 95)
        & ((red.astype(np.int16) - blue.astype(np.int16)) >= 8)
        & (saturation >= 10)
        & upper_body_zone
    )
    retained = output_rgba[:, :, 3] >= 128
    core_retention = float(
        np.count_nonzero(core_detail & retained) / np.count_nonzero(core_detail)
    )
    skin_retention = float(
        np.count_nonzero(skin_detail & retained) / np.count_nonzero(skin_detail)
    )
    return core_retention, skin_retention


def main() -> None:
    failures: list[str] = []
    for job in extractor.SHEETS:
        sheet = Image.open(extractor.SOURCE_DIR / job.file_name).convert("RGBA")
        output_dir = (
            ROOT
            / "assets"
            / "art"
            / "characters"
            / job.character
            / "sprites"
            / "attack"
            / job.weapon
        )
        for stage in range(3):
            source = extractor.panel_crop(sheet, stage)
            output = Image.open(output_dir / f"normal_{stage + 1:02d}.png")
            core_retention, skin_retention = retention_metrics(source, output)
            print(
                f"{job.character}/{job.weapon}/{stage + 1}: "
                f"core={core_retention:.3%}, skin={skin_retention:.3%}"
            )
            if (
                core_retention < MINIMUM_CORE_DETAIL_RETENTION
                or skin_retention < MINIMUM_UPPER_BODY_SKIN_RETENTION
            ):
                failures.append(
                    f"{job.character}/{job.weapon}/{stage + 1} "
                    f"retained core={core_retention:.3%}, "
                    f"skin={skin_retention:.3%}"
                )

    if failures:
        raise SystemExit("Cutout quality check failed:\n" + "\n".join(failures))


if __name__ == "__main__":
    main()
