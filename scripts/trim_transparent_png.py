from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image


def alpha_bounds(image: Image.Image, padding: int) -> tuple[int, int, int, int]:
    rgba = image.convert("RGBA")
    alpha = rgba.getchannel("A")
    bbox = alpha.getbbox()
    if bbox is None:
        return (0, 0, rgba.width, rgba.height)
    left, top, right, bottom = bbox
    return (
        max(0, left - padding),
        max(0, top - padding),
        min(rgba.width, right + padding),
        min(rgba.height, bottom + padding),
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--out", required=True)
    parser.add_argument("--padding", type=int, default=24)
    args = parser.parse_args()

    src = Path(args.input)
    dst = Path(args.out)
    image = Image.open(src).convert("RGBA")
    cropped = image.crop(alpha_bounds(image, args.padding))
    dst.parent.mkdir(parents=True, exist_ok=True)
    cropped.save(dst)
    print(f"Wrote {dst} ({cropped.width}x{cropped.height})")


if __name__ == "__main__":
    main()
