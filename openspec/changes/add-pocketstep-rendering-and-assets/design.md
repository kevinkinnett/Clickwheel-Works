## Context

PocketStep has three independent C99 headers for platform physics, top-down
navigation, and autonomous story sequencing. Story Clock currently keeps its
draw ordering, animation calculations, scene records, and Pillow conversion
logic inside the application. All runtime storage must remain fixed-size or
caller-owned, and the generated plugin must work without file I/O on a 60 MHz
iPod Color. See `proposal.md` for motivation and the capability specs for the
behavior contracts.

## Goals / Non-Goals

**Goals:**

- Preserve PocketStep's header-only, C99, allocator-free runtime model.
- Let applications use each new facility independently.
- Keep Rockbox calls out of portable headers.
- Make the asset build repeatable and reviewable from source PNGs and a manifest.
- Migrate Story Clock without changing its routes, timing, dialogue, or layout.

**Non-Goals:**

- A renderer, entity-component system, map editor, image decoder, or runtime
  asset loader.
- Scrolling cameras, general-purpose RPG logic, combat, or save games.
- Dynamic allocation or unbounded collections.
- Automatic interpretation of arbitrary third-party tileset layouts.
- Story branching and randomized script selection in this change.

## Decisions

### Add three optional runtime headers

Add `pocketstep_draw.h`, `pocketstep_anim.h`, and `pocketstep_scene.h`. Each
follows the existing declaration plus one-translation-unit implementation
pattern. Applications include only what they need.

Adding the APIs to `pocketstep.h` was rejected because Mushroom Clock's small
platform physics use does not need scenes or sprite sheets. A single top-down
header was rejected because draw ordering and animation are useful outside
tile-grid programs.

### Use caller-owned bounded draw lists

The caller supplies an array of drawable records and its capacity. Insertion
places each record in stable ascending foot-Y order. The intended lists contain
dozens of records, so insertion ordering keeps the code and memory use smaller
than a general sorting facility.

An internal fixed maximum was rejected because different displays and scenes
need different bounds. Sorting only at render time was rejected because it
would need a second API and temporary state with no benefit at this scale.

### Return source rectangles instead of drawing sprites

The animation module accepts a directional sheet description and movement
state, then returns a frame index and source rectangle. Applications pass that
rectangle to Rockbox, SDL, or another drawing system.

Callback-based rendering was rejected because it would mix frame selection
with device code. Storing animation timers in the engine was rejected for the
first version because distance-driven animation needs no separate clock and
matches the movement already used by Story Clock.

### Compose scene metadata around the existing grid API

A scene description references tile values, an existing `ps_grid`, regions,
their counts, and a spawn cell. It does not own route storage, story state, or
rendering data beyond tile identifiers. Validation checks pointer, dimension,
count, and spawn consistency.

Replacing `ps_grid` with a larger scene type was rejected because it would
break existing callers. Copying map arrays into an engine-owned structure was
rejected because it wastes memory and forces a global size limit.

### Use a stateless coordinate hash for visual variation

Tile variation uses integer coordinates, a caller seed, and the number of
choices. It has no random stream to advance. Collision, routes, and interaction
regions therefore remain unchanged when a scene changes its cosmetic seed.

A shared pseudo-random generator was rejected because draw order could then
change results. Authored variant arrays remain possible when a designer needs
exact placement.

### Drive the asset compiler with JSON

Create a general Python entry point that reads a JSON manifest and uses Pillow.
The manifest declares source collections, transforms, output arrays, dimensions,
transparent key, and provenance. Story Clock receives its own checked-in
manifest. Its old script becomes a small compatibility entry point or is
replaced by the general command in build documentation.

JSON was chosen because Python can parse it without another dependency. Python
code as configuration was rejected because validation, provenance extraction,
and review would remain tied to one program.

### Validate fully before atomic header replacement

The compiler builds and validates every output in memory, writes a temporary
file beside the target, then replaces the target only after success. Stable
ordering and fixed formatting make unchanged builds byte-identical.

Writing arrays as each asset completes was rejected because a late error could
leave a truncated but syntactically plausible header.

### Keep source licenses beside selected assets

The repository retains source license files under each asset project's
third-party directory. The manifest records the same provenance in structured
form, and the compiler emits a concise generated summary. The source license
file remains authoritative.

## Risks / Trade-offs

- [Small helpers grow into an application framework] -> Keep rendering,
  dialogue, inventory, map ownership, and device calls outside PocketStep.
- [Stable insertion becomes slow in a crowded scene] -> Document the intended
  small-list use and test capacity behavior; the iPod scenes stay below a few
  dozen drawables.
- [Animation sheet conventions differ] -> Store facing-row mappings and frame
  dimensions in caller data instead of imposing one sheet arrangement.
- [A manifest cannot express a future transform] -> Reject unknown operations
  clearly and add operations only with fixtures and deterministic tests.
- [Generated headers become large review diffs] -> Keep source manifests and
  assets authoritative, and verify regenerated bytes in tests.
- [Story Clock changes visually during migration] -> Compare a deterministic
  complete-story capture and retain the current generated header until the new
  compiler matches it.

## Migration Plan

1. Add each optional header with focused host tests and documentation.
2. Add the asset compiler, schema validation, fixtures, and deterministic-output
   tests without changing Story Clock.
3. Express Story Clock's current conversion in a checked-in manifest and verify
   that the new compiler produces valid Rockbox arrays.
4. Replace Story Clock's private ordering, animation, and scene helpers with the
   portable APIs one piece at a time.
5. Run all host tests, build the simulator and device plugins, and compare a
   full Story Clock recording.
6. If migration fails, restore Story Clock to its private helpers while keeping
   the independent optional modules and compiler tests.
