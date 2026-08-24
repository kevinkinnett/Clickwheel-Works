"""Compile manifest-declared pixel art into deterministic Rockbox headers."""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import re
import tempfile
from typing import Any

from PIL import Image


IDENTIFIER = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
DEFAULT_TRANSPARENT = (255, 0, 255)


class AssetError(ValueError):
    """A manifest or source asset is invalid."""


def require_dict(value: Any, label: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise AssetError(f"{label} must be an object")
    return value


def require_list(value: Any, label: str) -> list[Any]:
    if not isinstance(value, list):
        raise AssetError(f"{label} must be an array")
    return value


def require_identifier(value: Any, label: str) -> str:
    if not isinstance(value, str) or not IDENTIFIER.fullmatch(value):
        raise AssetError(f"{label} must be a C identifier")
    return value


def require_int(value: Any, label: str, minimum: int | None = None) -> int:
    if not isinstance(value, int) or isinstance(value, bool):
        raise AssetError(f"{label} must be an integer")
    if minimum is not None and value < minimum:
        raise AssetError(f"{label} must be at least {minimum}")
    return value


def parse_size(value: Any, label: str) -> tuple[int, int]:
    items = require_list(value, label)
    if len(items) != 2:
        raise AssetError(f"{label} must contain width and height")
    return (require_int(items[0], f"{label}[0]", 1),
            require_int(items[1], f"{label}[1]", 1))


def parse_rect(value: Any, label: str) -> tuple[int, int, int, int]:
    items = require_list(value, label)
    if len(items) != 4:
        raise AssetError(f"{label} must contain x, y, width, and height")
    return (require_int(items[0], f"{label}[0]", 0),
            require_int(items[1], f"{label}[1]", 0),
            require_int(items[2], f"{label}[2]", 1),
            require_int(items[3], f"{label}[3]", 1))


def parse_color(value: Any, label: str, allow_alpha: bool) -> tuple[int, ...]:
    items = require_list(value, label)
    lengths = (3, 4) if allow_alpha else (3,)
    if len(items) not in lengths:
        expected = "RGB or RGBA" if allow_alpha else "RGB"
        raise AssetError(f"{label} must be an {expected} array")
    result = tuple(require_int(item, f"{label} component", 0)
                   for item in items)
    if any(item > 255 for item in result):
        raise AssetError(f"{label} components must be between 0 and 255")
    return result


def crop_image(image: Image.Image, rect_value: Any, label: str) -> Image.Image:
    x, y, width, height = parse_rect(rect_value, label)
    if x + width > image.width or y + height > image.height:
        raise AssetError(f"{label} extends outside {image.width}x{image.height}")
    return image.crop((x, y, x + width, y + height))


def flip_image(image: Image.Image, axis: Any, label: str) -> Image.Image:
    if axis == "horizontal":
        return image.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
    if axis == "vertical":
        return image.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
    raise AssetError(f"{label} axis must be horizontal or vertical")


def color_key(image: Image.Image, operation: dict[str, Any], label: str) -> Image.Image:
    key = parse_color(operation.get("color"), f"{label}.color", False)
    tolerance = require_int(operation.get("tolerance", 0),
                            f"{label}.tolerance", 0)
    if tolerance > 255:
        raise AssetError(f"{label}.tolerance must not exceed 255")
    result = image.convert("RGBA")
    pixels = []
    for red, green, blue, alpha in result.get_flattened_data():
        if (abs(red - key[0]) <= tolerance and
                abs(green - key[1]) <= tolerance and
                abs(blue - key[2]) <= tolerance):
            pixels.append((red, green, blue, 0))
        else:
            pixels.append((red, green, blue, alpha))
    result.putdata(pixels)
    return result


def tint_image(image: Image.Image, value: Any, label: str) -> Image.Image:
    settings = require_dict(value, label)
    multiply = require_list(settings.get("multiply", [1, 1, 1]),
                            f"{label}.multiply")
    add = require_list(settings.get("add", [0, 0, 0]), f"{label}.add")
    if len(multiply) != 3 or len(add) != 3:
        raise AssetError(f"{label} multiply and add must contain three values")
    if any(not isinstance(item, (int, float)) or isinstance(item, bool)
           for item in multiply + add):
        raise AssetError(f"{label} values must be numbers")
    result = image.convert("RGBA")
    pixels = []
    for red, green, blue, alpha in result.get_flattened_data():
        channels = (red, green, blue)
        tinted = tuple(max(0, min(255, int(channels[index] * multiply[index] +
                                           add[index])))
                       for index in range(3))
        pixels.append((*tinted, alpha))
    result.putdata(pixels)
    return result


class Compiler:
    def __init__(self, manifest_path: Path, manifest: dict[str, Any]):
        self.manifest_path = manifest_path
        self.base_dir = manifest_path.parent
        self.manifest = manifest
        self.sources: dict[str, Path] = {}
        self.images: dict[str, Image.Image] = {}
        self.outputs: list[tuple[str, Image.Image]] = []

    def validate_sources(self) -> None:
        sources = require_dict(self.manifest.get("sources"), "sources")
        for name, raw in sources.items():
            require_identifier(name, f"source name {name!r}")
            source = require_dict(raw, f"source {name}")
            relative = source.get("path")
            if not isinstance(relative, str) or not relative:
                raise AssetError(f"source {name}.path must be a non-empty string")
            path = (self.base_dir / relative).resolve()
            if not path.is_file():
                raise AssetError(f"source file does not exist: {path}")
            self.sources[name] = path

    def reference_image(self, reference: dict[str, Any], label: str) -> Image.Image:
        if "source" in reference and "asset" in reference:
            raise AssetError(f"{label} cannot name both source and asset")
        if "source" in reference:
            name = reference["source"]
            if name not in self.sources:
                raise AssetError(f"{label} names unknown source {name!r}")
            try:
                return Image.open(self.sources[name]).convert("RGBA")
            except OSError as error:
                raise AssetError(f"cannot read source {self.sources[name]}: {error}") from error
        if "asset" in reference:
            name = reference["asset"]
            if name not in self.images:
                raise AssetError(f"{label} names unavailable asset {name!r}")
            return self.images[name].copy()
        raise AssetError(f"{label} must name a source or earlier asset")

    def assemble(self, operation: dict[str, Any], label: str) -> Image.Image:
        width, height = parse_size(operation.get("size"), f"{label}.size")
        fill_value = operation.get("fill", [255, 0, 255, 0])
        fill = parse_color(fill_value, f"{label}.fill", True)
        if len(fill) == 3:
            fill = (*fill, 255)
        result = Image.new("RGBA", (width, height), fill)
        layers = require_list(operation.get("layers"), f"{label}.layers")
        if not layers:
            raise AssetError(f"{label}.layers must not be empty")
        for index, raw_layer in enumerate(layers):
            layer_label = f"{label}.layers[{index}]"
            layer = require_dict(raw_layer, layer_label)
            image = self.reference_image(layer, layer_label)
            if "crop" in layer:
                image = crop_image(image, layer["crop"], f"{layer_label}.crop")
            if "flip" in layer:
                image = flip_image(image, layer["flip"], f"{layer_label}.flip")
            if "resize" in layer:
                image = image.resize(parse_size(layer["resize"],
                                                f"{layer_label}.resize"),
                                     Image.Resampling.NEAREST)
            at = require_list(layer.get("at"), f"{layer_label}.at")
            if len(at) != 2:
                raise AssetError(f"{layer_label}.at must contain x and y")
            x = require_int(at[0], f"{layer_label}.at[0]", 0)
            y = require_int(at[1], f"{layer_label}.at[1]", 0)
            if x + image.width > width or y + image.height > height:
                raise AssetError(f"{layer_label} does not fit the assembly canvas")
            result.alpha_composite(image, (x, y))
        return result

    def apply_operations(self, raw_operations: Any, asset_name: str) -> Image.Image:
        operations = require_list(raw_operations, f"asset {asset_name}.operations")
        image: Image.Image | None = None
        for index, raw_operation in enumerate(operations):
            label = f"asset {asset_name}.operations[{index}]"
            operation = require_dict(raw_operation, label)
            kind = operation.get("op")
            if kind in {"source", "asset"}:
                image = self.reference_image(operation, label)
            elif kind == "assemble":
                image = self.assemble(operation, label)
            elif image is None:
                raise AssetError(f"{label} requires an image")
            elif kind == "crop":
                image = crop_image(image, operation.get("rect"), f"{label}.rect")
            elif kind == "resize":
                image = image.resize(parse_size(operation.get("size"),
                                                f"{label}.size"),
                                     Image.Resampling.NEAREST)
            elif kind == "flip":
                image = flip_image(image, operation.get("axis"), label)
            elif kind == "color_key":
                image = color_key(image, operation, label)
            elif kind == "tint":
                image = tint_image(image, operation, label)
            else:
                raise AssetError(f"{label} uses unsupported operation {kind!r}")
        if image is None:
            raise AssetError(f"asset {asset_name} did not produce an image")
        return image

    def build_assets(self) -> None:
        assets = require_list(self.manifest.get("assets"), "assets")
        seen_internal: set[str] = set()
        seen_output: set[str] = set()
        for index, raw_asset in enumerate(assets):
            label = f"assets[{index}]"
            asset = require_dict(raw_asset, label)
            name = require_identifier(asset.get("name"), f"{label}.name")
            if name in seen_internal:
                raise AssetError(f"duplicate asset name: {name}")
            seen_internal.add(name)
            image = self.apply_operations(asset.get("operations"), name)
            self.images[name] = image
            emit = asset.get("emit", True)
            if not isinstance(emit, bool):
                raise AssetError(f"{label}.emit must be true or false")
            variants = asset.get("variants")
            if variants is None:
                if emit:
                    self.add_output(name, image, seen_output)
                continue
            for variant_index, raw_variant in enumerate(require_list(
                    variants, f"{label}.variants")):
                variant_label = f"{label}.variants[{variant_index}]"
                variant = require_dict(raw_variant, variant_label)
                suffix = variant.get("suffix")
                if not isinstance(suffix, str) or not suffix or not IDENTIFIER.fullmatch(name + suffix):
                    raise AssetError(f"{variant_label}.suffix must form a C identifier")
                result = image.copy()
                if "tint" in variant:
                    result = tint_image(result, variant["tint"],
                                        f"{variant_label}.tint")
                self.add_output(name + suffix, result, seen_output)

    def add_output(self, name: str, image: Image.Image,
                   seen: set[str]) -> None:
        if name in seen:
            raise AssetError(f"duplicate output name: {name}")
        seen.add(name)
        self.outputs.append((name, image))


def rgb565_swapped(red: int, green: int, blue: int) -> int:
    return ((red >> 3) << 3) | (green >> 5) | \
           ((green & 0x1C) << 11) | ((blue >> 3) << 8)


def emit_array(name: str, image: Image.Image,
               transparent: tuple[int, int, int]) -> str:
    values = []
    for red, green, blue, alpha in image.convert("RGBA").get_flattened_data():
        if alpha < 128:
            red, green, blue = transparent
        values.append(rgb565_swapped(red, green, blue))
    lines = [f"static const unsigned short {name}[{len(values)}] = {{"]
    for start in range(0, len(values), 12):
        chunk = values[start:start + 12]
        lines.append("    " + ", ".join(f"0x{value:04x}" for value in chunk) + ",")
    lines.append("};")
    return "\n".join(lines)


def render_header(manifest: dict[str, Any], compiler: Compiler) -> str:
    guard = require_identifier(manifest.get("guard"), "guard")
    transparent = parse_color(manifest.get("transparent_color",
                                           list(DEFAULT_TRANSPARENT)),
                              "transparent_color", False)
    constants = require_dict(manifest.get("constants", {}), "constants")
    provenance = require_list(manifest.get("provenance", []), "provenance")
    lines = ["/* Generated by the PocketStep asset compiler.",
             " * Do not edit this file by hand."]
    if provenance:
        lines.append(" * Asset provenance:")
    for index, raw_entry in enumerate(provenance):
        entry = require_dict(raw_entry, f"provenance[{index}]")
        values = []
        for field in ("creator", "source", "license"):
            value = entry.get(field)
            if not isinstance(value, str) or not value:
                raise AssetError(f"provenance[{index}].{field} must be a non-empty string")
            if "*/" in value or "\n" in value or "\r" in value:
                raise AssetError(f"provenance[{index}].{field} contains unsafe text")
            values.append(value)
        lines.append(f" * - {values[0]} | {values[2]} | {values[1]}")
    lines.extend([" */", f"#ifndef {guard}", f"#define {guard}", ""])
    for name in sorted(constants):
        require_identifier(name, f"constant {name!r}")
        value = require_int(constants[name], f"constant {name}", 0)
        lines.append(f"#define {name} {value}")
    if constants:
        lines.append("")
    lines.append("\n\n".join(emit_array(name, image, transparent)
                               for name, image in compiler.outputs))
    lines.extend(["", f"#endif /* {guard} */", ""])
    return "\n".join(lines)


def compile_manifest(manifest_path: Path,
                     output_override: Path | None = None) -> Path:
    manifest_path = manifest_path.resolve()
    try:
        manifest = require_dict(json.loads(manifest_path.read_text(encoding="utf-8")),
                                "manifest")
    except OSError as error:
        raise AssetError(f"cannot read manifest {manifest_path}: {error}") from error
    except json.JSONDecodeError as error:
        raise AssetError(f"invalid JSON in {manifest_path}: {error}") from error
    if manifest.get("format") != 1:
        raise AssetError("manifest format must be 1")
    compiler = Compiler(manifest_path, manifest)
    compiler.validate_sources()
    compiler.build_assets()
    if not compiler.outputs:
        raise AssetError("manifest emits no assets")
    output_value = manifest.get("output")
    if output_override is not None:
        output = output_override.resolve()
    elif isinstance(output_value, str) and output_value:
        output = (manifest_path.parent / output_value).resolve()
    else:
        raise AssetError("output must be a non-empty path")
    header = render_header(manifest, compiler)
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary: Path | None = None
    try:
        with tempfile.NamedTemporaryFile("w", encoding="ascii", newline="\n",
                                         dir=output.parent, delete=False) as stream:
            temporary = Path(stream.name)
            stream.write(header)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, output)
    finally:
        if temporary is not None and temporary.exists():
            temporary.unlink()
    return output


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("--output", type=Path)
    arguments = parser.parse_args()
    try:
        output = compile_manifest(arguments.manifest, arguments.output)
    except AssetError as error:
        parser.exit(1, f"asset compiler: {error}\n")
    print(f"Wrote {output} ({output.stat().st_size} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
