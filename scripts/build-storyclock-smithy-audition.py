#!/usr/bin/env python3
"""Build native-resolution Story Clock smithy asset audition sheets."""

from pathlib import Path

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / "assets" / "storyclock"
REVIEW = ROOT / "artifacts" / "asset-review" / "smithy"
KENNEY = (
    REVIEW
    / "candidates"
    / "kenney-roguelike-indoors"
    / "Spritesheet"
    / "roguelikeIndoor_transparent.png"
)
NINJA_FLOOR = (
    REVIEW
    / "candidates"
    / "ninja-adventure-godot"
    / "content"
    / "map"
    / "tileset_interior_floor.png"
)
WIDTH = 220
HEIGHT = 176


def keyed(image: Image.Image, threshold: int = 232) -> Image.Image:
    rgba = image.convert("RGBA")
    pixels = []
    for red, green, blue, alpha in rgba.getdata():
        spread = max(red, green, blue) - min(red, green, blue)
        if min(red, green, blue) >= threshold and spread <= 15:
            pixels.append((red, green, blue, 0))
        else:
            pixels.append((red, green, blue, alpha))
    rgba.putdata(pixels)
    return rgba


def crop_scaled(image: Image.Image, box, size, key=False) -> Image.Image:
    result = image.crop(box)
    if key:
        result = keyed(result)
    return result.resize(size, Image.Resampling.NEAREST)


def actor_frame(path: Path) -> Image.Image:
    sheet = Image.open(path).convert("RGBA")
    frame_width = sheet.width // 4
    frame_height = sheet.height // 4
    return sheet.crop((0, 0, frame_width, frame_height)).resize(
        (20, 20), Image.Resampling.NEAREST
    )


def kenney_cell(sheet: Image.Image, column: int, row: int) -> Image.Image:
    left = column * 17
    top = row * 17
    return sheet.crop((left, top, left + 16, top + 16))


def base_room(floor_colors, wall_colors) -> Image.Image:
    image = Image.new("RGBA", (WIDTH, HEIGHT), floor_colors[0])
    draw = ImageDraw.Draw(image)
    draw.rectangle((0, 0, WIDTH - 1, 31), fill=wall_colors[0])
    draw.rectangle((0, 29, WIDTH - 1, 34), fill=wall_colors[1])
    draw.rectangle((0, 0, 5, HEIGHT - 1), fill=wall_colors[1])
    draw.rectangle((WIDTH - 6, 0, WIDTH - 1, HEIGHT - 1), fill=wall_colors[1])
    for y in range(36, HEIGHT, 16):
        for x in range(6, WIDTH - 6, 16):
            color = floor_colors[((x // 16) + (y // 16)) % len(floor_colors)]
            draw.rectangle((x, y, min(x + 15, WIDTH - 7), min(y + 15, HEIGHT - 1)), fill=color)
            draw.line((x, y, min(x + 15, WIDTH - 7), y), fill=(44, 35, 38, 255))
    draw.rectangle((94, 146, 125, 175), fill=(54, 35, 31, 255))
    draw.rectangle((98, 150, 121, 175), fill=(111, 67, 43, 255))
    return image


def add_actors(image: Image.Image) -> None:
    rowan = actor_frame(ASSETS / "tovin-walk.png")
    luma = actor_frame(ASSETS / "luma-walk.png")
    image.alpha_composite(rowan, (70, 91))
    image.alpha_composite(luma, (104, 116))


def existing_mockup() -> Image.Image:
    image = base_room(
        [(111, 82, 61, 255), (104, 74, 56, 255)],
        [(81, 59, 52, 255), (54, 36, 35, 255)],
    )
    architecture = Image.open(ASSETS / "room-architecture-generated-v1.png")
    cottage = Image.open(ASSETS / "third-party" / "pixelandbeans-cottage-core" / "cottage_core.png")
    rpg = Image.open(ASSETS / "third-party" / "ansimuz-rpg-town" / "transparent-bg-tiles.png")
    anvil = Image.open(ASSETS / "third-party" / "antumdeluge-anvil" / "anvil-16x16-black.png").convert("RGBA")
    fireplace = crop_scaled(architecture, (819, 565, 1139, 855), (42, 38), True)
    desk = crop_scaled(cottage, (187, 29, 228, 64), (36, 28))
    crate = rpg.crop((192, 224, 208, 240))
    barrel = rpg.crop((208, 224, 224, 240))
    image.alpha_composite(fireplace, (89, 4))
    image.alpha_composite(desk, (20, 55))
    image.alpha_composite(anvil, (66, 64))
    image.alpha_composite(crate, (178, 71))
    image.alpha_composite(barrel, (182, 102))
    add_actors(image)
    return image


def library_mockup() -> Image.Image:
    floor_sheet = Image.open(NINJA_FLOOR).convert("RGBA")
    floor = floor_sheet.crop((0, 176, 16, 192))
    image = Image.new("RGBA", (WIDTH, HEIGHT), (53, 52, 48, 255))
    for y in range(32, HEIGHT, 16):
        for x in range(6, WIDTH - 6, 16):
            image.alpha_composite(floor, (x, y))
    draw = ImageDraw.Draw(image)
    draw.rectangle((0, 0, WIDTH - 1, 31), fill=(69, 64, 61, 255))
    draw.rectangle((0, 28, WIDTH - 1, 35), fill=(42, 39, 40, 255))
    draw.rectangle((0, 0, 5, HEIGHT - 1), fill=(42, 39, 40, 255))
    draw.rectangle((WIDTH - 6, 0, WIDTH - 1, HEIGHT - 1), fill=(42, 39, 40, 255))
    kenney = Image.open(KENNEY).convert("RGBA")
    stove = kenney_cell(kenney, 14, 14).resize((32, 32), Image.Resampling.NEAREST)
    bench = kenney_cell(kenney, 4, 4).resize((32, 16), Image.Resampling.NEAREST)
    shelves = kenney_cell(kenney, 4, 12).resize((32, 16), Image.Resampling.NEAREST)
    table = kenney_cell(kenney, 5, 4).resize((32, 16), Image.Resampling.NEAREST)
    image.alpha_composite(stove, (94, 4))
    image.alpha_composite(shelves, (20, 40))
    image.alpha_composite(bench, (164, 54))
    image.alpha_composite(table, (24, 90))
    image.alpha_composite(kenney_cell(kenney, 21, 9), (175, 100))
    add_actors(image)
    return image


def hybrid_mockup() -> Image.Image:
    image = base_room(
        [(79, 69, 64, 255), (70, 61, 58, 255), (86, 74, 67, 255)],
        [(63, 57, 56, 255), (36, 31, 34, 255)],
    )
    signature = Image.open(ASSETS / "smithy-signature-generated-v1.png")
    forge = crop_scaled(signature, (610, 75, 840, 280), (52, 44), True)
    bellows = crop_scaled(signature, (595, 335, 845, 515), (34, 24), True)
    trough = crop_scaled(signature, (585, 575, 835, 740), (40, 24), True)
    tools = crop_scaled(signature, (130, 750, 490, 950), (48, 26), True)
    bench = crop_scaled(signature, (590, 750, 930, 980), (48, 30), True)
    coal = crop_scaled(signature, (1050, 745, 1320, 980), (32, 28), True)
    anvil = Image.open(ASSETS / "third-party" / "antumdeluge-anvil" / "anvil-16x16-black.png").convert("RGBA")
    image.alpha_composite(forge, (84, 0))
    image.alpha_composite(tools, (16, 37))
    image.alpha_composite(coal, (168, 42))
    image.alpha_composite(bench, (18, 78))
    image.alpha_composite(bellows, (163, 83))
    image.alpha_composite(trough, (148, 113))
    image.alpha_composite(anvil, (68, 70))
    add_actors(image)
    return image


def main() -> None:
    REVIEW.mkdir(parents=True, exist_ok=True)
    mockups = [
        ("01-existing-only.png", "EXISTING ONLY", existing_mockup()),
        ("02-library-mix.png", "OPEN LIBRARY MIX", library_mockup()),
        ("03-selected-hybrid.png", "SELECTED HYBRID", hybrid_mockup()),
    ]
    contact = Image.new("RGB", (WIDTH * 3, HEIGHT + 18), (24, 22, 25))
    draw = ImageDraw.Draw(contact)
    for index, (filename, label, mockup) in enumerate(mockups):
        mockup.convert("RGB").save(REVIEW / filename)
        contact.paste(mockup.convert("RGB"), (index * WIDTH, 18))
        draw.text((index * WIDTH + 4, 4), label, fill=(242, 214, 157))
    contact.save(REVIEW / "smithy-audition-contact-sheet.png")
    print(f"Wrote {len(mockups)} mockups and contact sheet to {REVIEW}")


if __name__ == "__main__":
    main()
