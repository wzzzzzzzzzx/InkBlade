from __future__ import annotations

import argparse
from collections import deque
from dataclasses import dataclass
from pathlib import Path

from PIL import Image


@dataclass(frozen=True)
class SpriteJob:
    slot: str
    cell: int
    crop: tuple[float, float, float, float]
    padding: int = 24


ROOT = Path(__file__).resolve().parents[1]

CHARACTERS: dict[str, list[SpriteJob]] = {
    "mohen": [
        SpriteJob("idle_01", 0, (0.14, 0.04, 0.86, 0.96), 20),
        SpriteJob("run_01", 1, (0.12, 0.08, 0.94, 0.86), 22),
        SpriteJob("run_02", 3, (0.06, 0.12, 0.96, 0.86), 22),
        SpriteJob("jump_01", 2, (0.14, 0.06, 0.88, 0.90), 22),
        SpriteJob("dodge_01", 3, (0.06, 0.12, 0.96, 0.86), 22),
        SpriteJob("normal_01", 4, (0.10, 0.08, 0.56, 0.94), 22),
        SpriteJob("charging_01", 4, (0.10, 0.08, 0.56, 0.94), 24),
        SpriteJob("charge_release_01", 4, (0.10, 0.08, 0.56, 0.94), 28),
        SpriteJob("hit_01", 5, (0.13, 0.12, 0.78, 0.84), 22),
        SpriteJob("skill_01", 4, (0.10, 0.08, 0.56, 0.94), 28),
        SpriteJob("ultimate_01", 5, (0.13, 0.12, 0.78, 0.84), 30),
    ],
    "suxin": [
        SpriteJob("idle_01", 0, (0.18, 0.04, 0.82, 0.96), 20),
        SpriteJob("run_01", 1, (0.10, 0.10, 0.92, 0.84), 22),
        SpriteJob("run_02", 3, (0.07, 0.13, 0.95, 0.84), 22),
        SpriteJob("jump_01", 2, (0.16, 0.07, 0.86, 0.88), 22),
        SpriteJob("dodge_01", 3, (0.07, 0.13, 0.95, 0.84), 22),
        SpriteJob("normal_01", 4, (0.14, 0.08, 0.55, 0.94), 22),
        SpriteJob("charging_01", 4, (0.14, 0.08, 0.55, 0.94), 24),
        SpriteJob("charge_release_01", 4, (0.14, 0.08, 0.55, 0.94), 28),
        SpriteJob("hit_01", 5, (0.16, 0.13, 0.84, 0.84), 22),
        SpriteJob("skill_01", 4, (0.14, 0.08, 0.55, 0.94), 28),
        SpriteJob("ultimate_01", 5, (0.16, 0.13, 0.84, 0.84), 30),
    ],
}


def cell_box(index: int, width: int, height: int) -> tuple[int, int, int, int]:
    cols, rows = 4, 2
    cell_w = width // cols
    cell_h = height // rows
    col = index % cols
    row = index // cols
    return col * cell_w, row * cell_h, (col + 1) * cell_w, (row + 1) * cell_h


def crop_cell(sheet: Image.Image, job: SpriteJob) -> Image.Image:
    left, top, right, bottom = cell_box(job.cell, sheet.width, sheet.height)
    cell_w = right - left
    cell_h = bottom - top
    fx1, fy1, fx2, fy2 = job.crop
    box = (
        left + int(cell_w * fx1),
        top + int(cell_h * fy1),
        left + int(cell_w * fx2),
        top + int(cell_h * fy2),
    )
    return sheet.crop(box).convert("RGBA")


def remove_light_background(image: Image.Image) -> Image.Image:
    rgba = image.convert("RGBA")
    pixels = rgba.load()
    for y in range(rgba.height):
        for x in range(rgba.width):
            r, g, b, a = pixels[x, y]
            if a == 0:
                continue
            high = max(r, g, b)
            low = min(r, g, b)
            avg = (r + g + b) / 3
            sat = high - low
            if avg > 238 or (avg > 224 and sat < 38) or (avg > 232 and sat < 70):
                pixels[x, y] = (r, g, b, 0)
            elif avg > 214 and sat < 24:
                pixels[x, y] = (r, g, b, 0)
            elif avg > 218 and sat < 48:
                pixels[x, y] = (r, g, b, min(a, 70))
    return rgba


def clean_light_edge_matte(image: Image.Image) -> Image.Image:
    rgba = image.convert("RGBA")
    src = rgba.load()
    transparent = rgba.getchannel("A")
    out = rgba.copy()
    dst = out.load()
    for y in range(rgba.height):
        for x in range(rgba.width):
            r, g, b, a = src[x, y]
            if a == 0:
                continue
            high = max(r, g, b)
            low = min(r, g, b)
            avg = (r + g + b) / 3
            sat = high - low
            near_alpha = False
            for nx in range(max(0, x - 1), min(rgba.width, x + 2)):
                for ny in range(max(0, y - 1), min(rgba.height, y + 2)):
                    if transparent.getpixel((nx, ny)) == 0:
                        near_alpha = True
                        break
                if near_alpha:
                    break
            if near_alpha and avg > 218 and sat < 42:
                dst[x, y] = (r, g, b, 0)
            elif 0 < a < 190:
                dst[x, y] = (r, g, b, 0)
            elif a >= 190:
                dst[x, y] = (r, g, b, 255)
    return out


def trim_alpha(image: Image.Image, padding: int) -> Image.Image:
    alpha = image.getchannel("A")
    bbox = alpha.getbbox()
    if bbox is None:
        return image
    left, top, right, bottom = bbox
    box = (
        max(0, left - padding),
        max(0, top - padding),
        min(image.width, right + padding),
        min(image.height, bottom + padding),
    )
    return image.crop(box)


def remove_non_body_components(image: Image.Image, min_pixels: int = 60) -> Image.Image:
    rgba = image.convert("RGBA")
    alpha = rgba.getchannel("A")
    mask = alpha.load()
    seen: set[tuple[int, int]] = set()
    remove: list[tuple[int, int]] = []
    for y in range(rgba.height):
        for x in range(rgba.width):
            if (x, y) in seen or mask[x, y] < 24:
                continue
            queue = deque([(x, y)])
            seen.add((x, y))
            component: list[tuple[int, int]] = []
            while queue:
                cx, cy = queue.popleft()
                component.append((cx, cy))
                for nx, ny in ((cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)):
                    if nx < 0 or ny < 0 or nx >= rgba.width or ny >= rgba.height:
                        continue
                    if (nx, ny) in seen or mask[nx, ny] < 24:
                        continue
                    seen.add((nx, ny))
                    queue.append((nx, ny))
            xs = [p[0] for p in component]
            ys = [p[1] for p in component]
            comp_w = max(xs) - min(xs) + 1
            comp_h = max(ys) - min(ys) + 1
            touches_edge = min(xs) < 8 or max(xs) > rgba.width - 9
            thin_sheet_line = comp_w <= 8 and comp_h >= 70
            edge_ruler = touches_edge and comp_w <= 14 and comp_h >= 50
            floor_mark = comp_w >= 24 and comp_h <= 26 and min(ys) > rgba.height - 72
            loose_vfx_chip = len(component) < 220 and (comp_w >= 18 or comp_h >= 18)
            if len(component) < min_pixels or thin_sheet_line or edge_ruler or floor_mark or loose_vfx_chip:
                remove.extend(component)
    pixels = rgba.load()
    for x, y in remove:
        r, g, b, _ = pixels[x, y]
        pixels[x, y] = (r, g, b, 0)
    return rgba


def process_character(character: str) -> None:
    concept = ROOT / "assets" / "art" / "characters" / character / "concept" / f"{character}_action_pose_sheet.png"
    body_dir = ROOT / "assets" / "art" / "characters" / character / "sprites" / "body"
    sheet = Image.open(concept).convert("RGBA")
    body_dir.mkdir(parents=True, exist_ok=True)

    for job in CHARACTERS[character]:
        sprite = crop_cell(sheet, job)
        sprite = remove_light_background(sprite)
        sprite = clean_light_edge_matte(sprite)
        sprite = remove_non_body_components(sprite)
        sprite = trim_alpha(sprite, job.padding)

        concept_name = f"{job.slot}_concept_01.png"
        concept_path = body_dir / concept_name
        runtime_path = body_dir / f"{job.slot}.png"
        sprite.save(concept_path)
        sprite.save(runtime_path)
        print(f"Wrote {runtime_path.relative_to(ROOT)} ({sprite.width}x{sprite.height})")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--character", choices=sorted(CHARACTERS.keys()))
    args = parser.parse_args()

    characters = [args.character] if args.character else sorted(CHARACTERS.keys())
    for character in characters:
        process_character(character)


if __name__ == "__main__":
    main()
