from __future__ import annotations

from collections import deque
from dataclasses import dataclass
from pathlib import Path

import cv2
import numpy as np
from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[1]
SOURCE_DIR = ROOT / "assets" / "art" / "review" / "weapon_combo_candidates"


@dataclass(frozen=True)
class ComboSheet:
    character: str
    weapon: str
    file_name: str


SHEETS = (
    ComboSheet("lingshuang", "longsword", "lingshuang_longsword_combo_master_v1.png"),
    ComboSheet("lingshuang", "broadsword", "lingshuang_broadsword_combo_master_v1.png"),
    ComboSheet("lingshuang", "dualblades", "lingshuang_dualblades_combo_master_v2.png"),
    ComboSheet("mohen", "longsword", "mohen_longsword_combo_master_v1.png"),
    ComboSheet("mohen", "broadsword", "mohen_broadsword_combo_master_v1.png"),
    ComboSheet("mohen", "dualblades", "mohen_dualblades_combo_master_v1.png"),
    ComboSheet("suxin", "longsword", "suxin_longsword_combo_master_v1.png"),
    ComboSheet("suxin", "broadsword", "suxin_broadsword_combo_master_v1.png"),
    ComboSheet("suxin", "dualblades", "suxin_dualblades_combo_master_v1.png"),
)


def is_connected_background(pixel: tuple[int, int, int, int]) -> bool:
    r, g, b, a = pixel
    if a == 0:
        return True
    high = max(r, g, b)
    low = min(r, g, b)
    average = (r + g + b) / 3
    saturation = high - low
    return average >= 214 and saturation <= 32


def connected_background_mask(image: Image.Image) -> bytearray:
    rgba = image.convert("RGBA")
    pixels = rgba.load()
    width, height = rgba.size
    mask = bytearray(width * height)
    queue: deque[tuple[int, int]] = deque()

    def add(x: int, y: int) -> None:
        index = y * width + x
        if mask[index] or not is_connected_background(pixels[x, y]):
            return
        mask[index] = 1
        queue.append((x, y))

    for x in range(width):
        add(x, 0)
        add(x, height - 1)
    for y in range(height):
        add(0, y)
        add(width - 1, y)

    while queue:
        x, y = queue.popleft()
        for nx, ny in (
            (x - 1, y),
            (x + 1, y),
            (x, y - 1),
            (x, y + 1),
            (x - 1, y - 1),
            (x + 1, y - 1),
            (x - 1, y + 1),
            (x + 1, y + 1),
        ):
            if 0 <= nx < width and 0 <= ny < height:
                add(nx, ny)
    return mask


def grabcut_foreground_mask(image: Image.Image) -> np.ndarray:
    rgb = np.asarray(image.convert("RGB"))
    height, width, _ = rgb.shape
    maximum = rgb.max(axis=2)
    minimum = rgb.min(axis=2)
    average = rgb.mean(axis=2)
    saturation = maximum - minimum

    mask = np.full((height, width), cv2.GC_PR_FGD, dtype=np.uint8)

    definite_white = (average >= 247) & (saturation <= 9)
    connected_white = np.asarray(
        connected_background_mask(
            Image.fromarray(
                np.dstack(
                    (
                        np.where(definite_white, 255, 0),
                        np.where(definite_white, 255, 0),
                        np.where(definite_white, 255, 0),
                        np.full((height, width), 255),
                    )
                ).astype(np.uint8),
                mode="RGBA",
            )
        ),
        dtype=np.uint8,
    ).reshape((height, width)).astype(bool)
    mask[connected_white] = cv2.GC_BGD

    probable_white = (average >= 231) & (saturation <= 18)
    mask[probable_white & ~connected_white] = cv2.GC_PR_BGD

    line_art = (average <= 202) | (saturation >= 30)
    line_art = cv2.morphologyEx(
        line_art.astype(np.uint8),
        cv2.MORPH_CLOSE,
        np.ones((3, 3), dtype=np.uint8),
        iterations=1,
    ).astype(bool)
    mask[line_art] = cv2.GC_FGD

    mask[: max(4, int(height * 0.08)), :] = cv2.GC_BGD
    mask[:, :3] = cv2.GC_BGD
    mask[:, -3:] = cv2.GC_BGD
    mask[-3:, :] = cv2.GC_BGD

    background_model = np.zeros((1, 65), np.float64)
    foreground_model = np.zeros((1, 65), np.float64)
    cv2.grabCut(
        cv2.cvtColor(rgb, cv2.COLOR_RGB2BGR),
        mask,
        None,
        background_model,
        foreground_model,
        8,
        cv2.GC_INIT_WITH_MASK,
    )
    return np.isin(mask, (cv2.GC_FGD, cv2.GC_PR_FGD))


def remove_background_with_grabcut(image: Image.Image) -> Image.Image:
    rgba = image.convert("RGBA")
    foreground = grabcut_foreground_mask(rgba)
    binary = foreground.astype(np.uint8) * 255
    closed = cv2.morphologyEx(
        binary,
        cv2.MORPH_CLOSE,
        np.ones((3, 3), dtype=np.uint8),
        iterations=1,
    )
    distance_inside = cv2.distanceTransform(closed, cv2.DIST_L2, 3)
    distance_outside = cv2.distanceTransform(255 - closed, cv2.DIST_L2, 3)
    signed_distance = distance_inside - distance_outside
    alpha = np.clip((signed_distance + 1.25) * 102, 0, 255).astype(np.uint8)
    rgba.putalpha(Image.fromarray(alpha, mode="L"))
    return rgba


def remove_connected_light_background(image: Image.Image) -> Image.Image:
    rgba = image.convert("RGBA")
    width, height = rgba.size
    pixels = rgba.load()
    background = connected_background_mask(rgba)

    for y in range(height):
        for x in range(width):
            index = y * width + x
            if background[index]:
                r, g, b, _ = pixels[x, y]
                pixels[x, y] = (r, g, b, 0)

    # Soften only the one-pixel boundary next to removed background. Restricting
    # this to the boundary preserves white clothing and pale effects internally.
    source_alpha = rgba.getchannel("A")
    alpha = source_alpha.copy()
    alpha_pixels = alpha.load()
    for y in range(1, height - 1):
        for x in range(1, width - 1):
            if source_alpha.getpixel((x, y)) == 0:
                continue
            touches_background = any(
                background[ny * width + nx]
                for nx, ny in (
                    (x - 1, y),
                    (x + 1, y),
                    (x, y - 1),
                    (x, y + 1),
                )
            )
            if not touches_background:
                continue
            r, g, b, _ = pixels[x, y]
            average = (r + g + b) / 3
            saturation = max(r, g, b) - min(r, g, b)
            if average > 205 and saturation < 55:
                opacity = int(max(24, min(255, (250 - average) * 10 + saturation * 3)))
                alpha_pixels[x, y] = min(alpha_pixels[x, y], opacity)
    rgba.putalpha(alpha)
    return rgba


def remove_enclosed_light_background(image: Image.Image, character: str) -> Image.Image:
    rgba = image.convert("RGBA")
    pixels = rgba.load()
    width, height = rgba.size
    seen = bytearray(width * height)
    remove: list[tuple[int, int]] = []
    if character == "mohen":
        minimum_average = 214
        maximum_saturation = 32
        minimum_component_size = 300
    else:
        minimum_average = 235
        maximum_saturation = 15
        minimum_component_size = 500

    def enclosed_background(x: int, y: int) -> bool:
        r, g, b, a = pixels[x, y]
        if a < 16:
            return False
        average = (r + g + b) / 3
        saturation = max(r, g, b) - min(r, g, b)
        return average >= minimum_average and saturation <= maximum_saturation

    for y in range(height):
        for x in range(width):
            index = y * width + x
            if seen[index] or not enclosed_background(x, y):
                continue
            queue = deque([(x, y)])
            seen[index] = 1
            component: list[tuple[int, int]] = []
            while queue:
                cx, cy = queue.popleft()
                component.append((cx, cy))
                for nx, ny in (
                    (cx - 1, cy),
                    (cx + 1, cy),
                    (cx, cy - 1),
                    (cx, cy + 1),
                ):
                    if not (0 <= nx < width and 0 <= ny < height):
                        continue
                    neighbor = ny * width + nx
                    if seen[neighbor] or not enclosed_background(nx, ny):
                        continue
                    seen[neighbor] = 1
                    queue.append((nx, ny))

            if len(component) >= minimum_component_size:
                remove.extend(component)

    for x, y in remove:
        r, g, b, _ = pixels[x, y]
        pixels[x, y] = (r, g, b, 0)
    return rgba


def remove_panel_lines(image: Image.Image) -> Image.Image:
    rgba = image.convert("RGBA")
    alpha = rgba.getchannel("A")
    mask = alpha.load()
    width, height = rgba.size
    seen: set[tuple[int, int]] = set()
    remove: list[tuple[int, int]] = []

    for y in range(height):
        for x in range(width):
            if (x, y) in seen or mask[x, y] < 16:
                continue
            queue = deque([(x, y)])
            seen.add((x, y))
            component: list[tuple[int, int]] = []
            while queue:
                cx, cy = queue.popleft()
                component.append((cx, cy))
                for nx, ny in ((cx - 1, cy), (cx + 1, cy), (cx, cy - 1), (cx, cy + 1)):
                    if not (0 <= nx < width and 0 <= ny < height):
                        continue
                    if (nx, ny) in seen or mask[nx, ny] < 16:
                        continue
                    seen.add((nx, ny))
                    queue.append((nx, ny))

            xs = [point[0] for point in component]
            ys = [point[1] for point in component]
            component_width = max(xs) - min(xs) + 1
            component_height = max(ys) - min(ys) + 1
            touches_vertical_edge = min(xs) <= 2 or max(xs) >= width - 3
            sheet_divider = (
                touches_vertical_edge
                and component_width <= 9
                and component_height >= height * 0.45
            )
            if sheet_divider:
                remove.extend(component)

    pixels = rgba.load()
    for x, y in remove:
        r, g, b, _ = pixels[x, y]
        pixels[x, y] = (r, g, b, 0)
    return rgba


def remove_floor_plate(image: Image.Image) -> Image.Image:
    rgba = image.convert("RGBA")
    array = np.asarray(rgba).copy()
    height, width, _ = array.shape
    rgb = array[:, :, :3].astype(np.int16)
    average = rgb.mean(axis=2)
    saturation = rgb.max(axis=2) - rgb.min(axis=2)
    rows = np.indices((height, width))[0]
    light_neutral = (
        (array[:, :, 3] >= 16)
        & (average >= 205)
        & (saturation <= 32)
        & (rows >= int(height * 0.82))
    )
    horizontal_length = max(35, int(width * 0.08))
    floor_lines = cv2.morphologyEx(
        light_neutral.astype(np.uint8),
        cv2.MORPH_OPEN,
        np.ones((1, horizontal_length), dtype=np.uint8),
    )
    floor_lines = cv2.dilate(
        floor_lines,
        np.ones((3, 5), dtype=np.uint8),
        iterations=1,
    ).astype(bool)
    array[:, :, 3][floor_lines & light_neutral] = 0
    return Image.fromarray(array, mode="RGBA")


def clear_canvas_border(image: Image.Image, border: int = 12) -> Image.Image:
    rgba = image.convert("RGBA")
    pixels = rgba.load()
    width, height = rgba.size
    for y in range(height):
        for x in range(width):
            if x < border or x >= width - border or y < border or y >= height - border:
                r, g, b, _ = pixels[x, y]
                pixels[x, y] = (r, g, b, 0)
    return rgba


def panel_crop(sheet: Image.Image, stage: int) -> Image.Image:
    inset = 15
    panel_width = sheet.width // 3
    crop_width = panel_width - inset * 2
    panel_center = round(sheet.width * (stage + 0.5) / 3)
    panel_left = panel_center - crop_width // 2
    panel_right = panel_left + crop_width
    content_bottom = round(sheet.height * 0.885)
    return sheet.crop((panel_left, 0, panel_right, content_bottom)).convert("RGBA")


def extract_sheet(job: ComboSheet) -> list[Path]:
    source = SOURCE_DIR / job.file_name
    if not source.exists():
        raise FileNotFoundError(source)

    sheet = Image.open(source).convert("RGBA")
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
    output_dir.mkdir(parents=True, exist_ok=True)

    outputs: list[Path] = []
    expected_size: tuple[int, int] | None = None
    for stage in range(3):
        sprite = remove_background_with_grabcut(panel_crop(sheet, stage))
        sprite = remove_panel_lines(sprite)
        sprite = remove_floor_plate(sprite)
        sprite = clear_canvas_border(sprite)
        if expected_size is None:
            expected_size = sprite.size
        elif sprite.size != expected_size:
            raise RuntimeError(f"Unstable canvas for {job.character}/{job.weapon}")
        output = output_dir / f"normal_{stage + 1:02d}.png"
        sprite.save(output, optimize=True)
        outputs.append(output)
        print(f"Wrote {output.relative_to(ROOT)} ({sprite.width}x{sprite.height})")
    return outputs


def checkerboard(size: tuple[int, int], tile: int = 16) -> Image.Image:
    image = Image.new("RGB", size, (48, 55, 58))
    draw = ImageDraw.Draw(image)
    for y in range(0, size[1], tile):
        for x in range(0, size[0], tile):
            if (x // tile + y // tile) % 2 == 0:
                draw.rectangle((x, y, x + tile - 1, y + tile - 1), fill=(76, 85, 88))
    return image


def build_preview(groups: list[list[Path]]) -> Path:
    cell_width, cell_height = 320, 390
    preview = Image.new("RGB", (cell_width * 3, cell_height * len(groups)), (35, 40, 42))
    for row, group in enumerate(groups):
        for column, path in enumerate(group):
            sprite = Image.open(path).convert("RGBA")
            scale = min((cell_width - 16) / sprite.width, (cell_height - 16) / sprite.height)
            draw_size = (max(1, round(sprite.width * scale)), max(1, round(sprite.height * scale)))
            rendered = sprite.resize(draw_size, Image.Resampling.LANCZOS)
            cell = checkerboard((cell_width, cell_height))
            x = (cell_width - draw_size[0]) // 2
            y = (cell_height - draw_size[1]) // 2
            cell.paste(rendered, (x, y), rendered)
            preview.paste(cell, (column * cell_width, row * cell_height))

    output = ROOT / "build" / "weapon_combo_transparency_preview.png"
    output.parent.mkdir(parents=True, exist_ok=True)
    preview.save(output, optimize=True)
    print(f"Wrote {output.relative_to(ROOT)}")
    return output


def main() -> None:
    groups = [extract_sheet(job) for job in SHEETS]
    build_preview(groups)


if __name__ == "__main__":
    main()
