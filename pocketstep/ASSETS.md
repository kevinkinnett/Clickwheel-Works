# PocketStep asset compiler

`tools/asset_compiler.py` turns PNG pixel art into C arrays for a Rockbox color
display. It reads a JSON manifest, performs every transform in order, validates
the complete result, and replaces the target header only after the build
succeeds.

Run it with Python and Pillow:

```powershell
python tools/asset_compiler.py path/to/assets.json
```

## Manifest structure

The top-level object accepts these fields:

- `format`: currently `1`.
- `output`: header path relative to the manifest.
- `guard`: C include-guard identifier.
- `transparent_color`: RGB value used for pixels whose alpha is below 128.
- `constants`: C macro names and non-negative integer values.
- `provenance`: records with `creator`, `source`, and `license` strings.
- `sources`: named PNG paths relative to the manifest.
- `assets`: ordered intermediate and emitted image declarations.

Each asset has a unique C identifier in `name`, an `operations` array, and an
optional `emit` boolean. An asset with `emit: false` remains available to later
assets but does not produce a C array.

An optional `variants` array emits names formed from the asset name and each
declared `suffix`. A variant can apply an RGB `tint` with `multiply` and `add`
triples. This is how Story Clock produces day, evening, and night arrays from
one source image.

## Operations

- `source` loads a named source PNG.
- `asset` copies an earlier intermediate asset.
- `crop` accepts `[x, y, width, height]` and rejects out-of-bounds rectangles.
- `resize` accepts `[width, height]` and always uses nearest-neighbor sampling.
- `flip` accepts `horizontal` or `vertical`.
- `color_key` makes pixels near an RGB color transparent. `tolerance` defaults
  to zero.
- `tint` multiplies and offsets RGB channels while preserving alpha.
- `assemble` creates a fixed-size canvas and composites source or earlier asset
  layers at declared positions. Layers can crop, flip, or resize their input.

The compiler rejects missing sources, invalid colors, duplicate names,
unsupported operations, bad crop bounds, and layers that exceed their assembly
canvas. It writes arrays in a fixed order and format, so two unchanged builds
produce byte-identical headers.

## Licenses

The manifest provenance block is a summary, not a replacement for license
files. Keep the source license next to redistributed third-party artwork. For
generated or commissioned work, record the creator, service or source URL, and
the applicable project license or usage terms.
