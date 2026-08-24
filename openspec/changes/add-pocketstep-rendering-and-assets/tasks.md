## 1. Portable draw ordering

- [x] 1.1 Add `pocketstep_draw.h` with caller-owned list initialization, stable foot-Y insertion, capacity failure, and record access.
- [x] 1.2 Add host tests for invalid initialization, mixed and equal depths, payload preservation, and full-list failure.
- [x] 1.3 Document the draw-list API, bounded-memory behavior, and graphics-independent usage in the PocketStep README.

## 2. Sprite animation selection

- [x] 2.1 Add `pocketstep_anim.h` with directional sheet descriptions, validation, idle selection, and distance-driven moving-frame selection.
- [x] 2.2 Add host tests for facing-row mapping, source rectangles, frame wrapping, pauses, idle frames, invalid descriptions, and deterministic replay.
- [x] 2.3 Document animation descriptors and Rockbox-neutral frame selection in the PocketStep README.

## 3. Scene descriptions and variation

- [x] 3.1 Add `pocketstep_scene.h` with caller-owned tile, grid, region, and spawn references plus scene validation.
- [x] 3.2 Add stateless coordinate-and-seed tile variation with explicit rejection of an empty variation set.
- [x] 3.3 Add host tests for valid scenes, each validation failure, grid compatibility, stable variation, seed changes, and in-range results.
- [x] 3.4 Document scene composition and the separation between cosmetic variation and collision data.

## 4. Manifest-driven asset compiler

- [x] 4.1 Add a general asset compiler entry point that loads JSON manifests, resolves paths relative to the manifest, and validates sources and unique output names.
- [x] 4.2 Implement crop, assembly, nearest-neighbor resize, alpha and color-key transparency, and time-of-day tint operations.
- [x] 4.3 Implement deterministic Rockbox RGB565-swapped arrays, dimension constants, provenance output, and atomic target replacement.
- [x] 4.4 Add compact PNG and manifest fixtures covering successful transforms, malformed colors, crop bounds, assembly errors, unknown operations, duplicate names, and missing sources.
- [x] 4.5 Add automated tests that verify exact dimensions, transparency, tint names, failure preservation, provenance, and byte-identical repeated builds.
- [x] 4.6 Document the manifest format, supported operations, licensing fields, and compiler command.

## 5. Story Clock migration

- [x] 5.1 Create a checked-in Story Clock asset manifest that reproduces the current generated arrays and records RetroDiffusion and Kenney provenance.
- [x] 5.2 Replace the Story Clock-specific converter with the general compiler command while retaining a convenient compatibility entry point if needed.
- [x] 5.3 Replace Story Clock's private drawable list and insertion logic with `pocketstep_draw.h`.
- [x] 5.4 Replace private directional frame calculations with `pocketstep_anim.h` without changing movement timing or sprite layout.
- [x] 5.5 Replace private scene records and tile-variant arithmetic with `pocketstep_scene.h` while preserving maps, routes, regions, and spawn points.
- [x] 5.6 Update simulator and device build scripts to copy every PocketStep optional header used by Story Clock.

## 6. Verification and handoff

- [x] 6.1 Run all PocketStep, grid, story, and Story Clock world host tests plus the new module and compiler tests.
- [x] 6.2 Build Chronolith, Mushroom Clock, and Story Clock for the iPod Color simulator to catch optional-header regressions.
- [x] 6.3 Build the iPod Color device package and verify the generated Story Clock plugin is present.
- [x] 6.4 Record a deterministic complete Story Clock loop and review indoor art, outdoor art, animation, routes, depth ordering, dialogue, and reset behavior.
- [x] 6.5 Update project documentation with the new reusable modules, asset workflow, license handling, and tested commands.
