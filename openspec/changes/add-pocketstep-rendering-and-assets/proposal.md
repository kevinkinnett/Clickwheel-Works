## Why

Story Clock proved that depth ordering, directional animation, scene metadata,
and bitmap conversion recur across small autonomous programs, but each program
currently has to rebuild those pieces. PocketStep should own the portable,
testable rules while a reusable tool handles Rockbox image conversion.

## What Changes

- Add an optional fixed-capacity draw list that performs stable foot-Y ordering
  without allocating memory or calling a graphics API.
- Add optional sprite-animation helpers for facing-row lookup, idle and moving
  frame selection, and distance-driven animation.
- Add caller-owned scene descriptors that group a tile grid, interaction
  regions, spawn position, and deterministic visual variation helpers.
- Add a manifest-driven asset compiler for crop, assembly, nearest-neighbor
  resize, transparency, time-of-day tinting, and Rockbox RGB565 header output.
- Migrate Story Clock to the reusable APIs and manifest while preserving its
  story, routes, visual result, simulator capture, and device output.
- Document third-party asset provenance in the manifest and generated output
  workflow.

## Capabilities

### New Capabilities

- `portable-draw-order`: Fixed-capacity, stable ordering of drawable records by
  their visible foot position.
- `sprite-animation-selection`: Portable selection of directional sprite frames
  from movement state and distance traveled.
- `scene-description-and-variation`: Caller-owned fixed-screen scene metadata
  and repeatable tile variation selection.
- `asset-compilation-pipeline`: Manifest-driven conversion of source PNG art to
  generated Rockbox bitmap headers with recorded provenance.

### Modified Capabilities

None. Story Clock's visible behavior does not change, so its existing
requirements need no delta.

## Impact

The change adds optional PocketStep headers and host tests, replaces Story
Clock's private draw and animation helpers, and generalizes
`scripts/generate-storyclock-assets.py`. It changes no existing PocketStep core,
grid, or story API. Generated assets remain compiled into the plugin, so the
iPod still needs no file I/O, allocator, or image decoder.
