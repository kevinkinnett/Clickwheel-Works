## 1. Baseline and asset audition

- [x] 1.1 Run the current PocketStep tests, Story Clock simulator build, and asset compiler to record a clean baseline before scene changes.
- [x] 1.2 Inventory the existing Story Clock sources for usable smithy architecture, floor, wall, anvil, barrel, crate, fireplace, furniture, tool, and ambient-animation roles.
- [x] 1.3 Obtain and inspect the Ninja Adventure and Kenney Roguelike Indoors candidates outside the shipped asset tree, verify their current licenses, and record the source URLs and versions reviewed.
- [x] 1.4 Build native 220 by 176 smithy mockups for every viable treatment using one shared room layout with Luma and Rowan visible for scale.
- [x] 1.5 Select the strongest coherent treatment, record why rejected candidates failed, and generate focused signature art only for roles with no reusable candidate that passes native review.
- [x] 1.6 Retain the accepted original source sheets, license text, attribution data, selected cells, transformations, and any final generation prompts under `assets/storyclock/`.

## 2. Smithy scene and interior presentation

- [x] 2.1 Append the smithy scene ID, tile map, collision grid, doorway region, spawn, and fixed scene record without changing existing scene or item identifiers.
- [x] 2.2 Connect the South Gate smithy doorway and smithy exit with reciprocal entrance records and matching exterior and interior spawn facings.
- [x] 2.3 Add host tests for reciprocal entrance validity, matching doorway alignment, passable route legs, blocked furniture footprints, and safe entrance and exit spawns.
- [x] 2.4 Add an app-owned scene presentation table and convert the opening house to the common indoor render stages without changing its visible layout.
- [x] 2.5 Add smithy architecture, depth-sorted furniture, Rowan placement, foreground overhangs, and an anchored forge-related ambient loop using placeholder assets where necessary.
- [x] 2.6 Verify draw-order and collision alignment with actors standing before and behind the forge, anvil, trough, workbench, and doorway.

## 3. Final asset compilation

- [x] 3.1 Add manifest sources and deterministic assemblies for the accepted smithy floor, walls, doorway, landmark, props, and ambient frames.
- [x] 3.2 Emit day, evening, and night-compatible smithy arrays with recorded crops, transparency, palette operations, and nearest-neighbor scaling.
- [x] 3.3 Recompile the unchanged manifest twice and verify byte-identical `storyclock_assets.h` output and passing asset-compiler tests.
- [x] 3.4 Replace placeholder smithy drawing with compiled assets and correct every native-resolution problem involving silhouette, actor contrast, floating props, hidden routes, collision, or unstable animation.
- [x] 3.5 Update `assets/storyclock/README.md` with the smithy identity profile, audition results, accepted provenance, rejected candidates, and generation basis if used.

## 4. Stateful autonomous visit

- [x] 4.1 Extend the South Gate itinerary to enter the smithy, walk to Rowan, complete the conversation and reward sequence, return to the doorway, and reappear at the matching exterior threshold.
- [x] 4.2 Make Rowan and Luma face one another during dialogue and restore the required route facings afterward.
- [x] 4.3 Select first-visit or repeat dialogue from the retained Iron Charm quantity, grant at most one charm per session, and skip the new-item overlay on repeat visits.
- [x] 4.4 Add host coverage for first-visit reward state, repeat-visit dialogue state, duplicate prevention, overlay suppression, and inventory persistence across story-loop reset.

## 5. Simulator and device verification

- [x] 5.1 Add a forced first-visit smithy simulator scenario with fixed inventory, itinerary, actor positions, dialogue, and animation timing.
- [x] 5.2 Add a repeatable smithy capture command that records entry, ambience, dialogue, Iron Charm presentation, exit, and South Gate return without manual timing.
- [x] 5.3 Run all PocketStep and Story Clock tests, build the simulator, and review the complete smithy recording at native resolution in every supported palette.
- [x] 5.4 Build the Rockbox device plugin and check the final binary size, compiler warnings, fixed-memory constraints, and absence of runtime image or filesystem dependencies.

## 6. Exterior route and interior detail refinement

- [x] 6.1 Block the South Gate facade footprint, open a ground-level approach below it, and route both the normal itinerary and forced review through the exterior threshold without drawing Luma over the building.
- [x] 6.2 Add host coverage that proves the exterior approach cells are passable, facade cells are blocked, and the route reaches the entrance without entering the facade footprint.
- [x] 6.3 Add memory-cheap smithy edge detail and solid storage props while preserving the entrance-to-Rowan aisle, actor contrast, prop collision, and forge animation.
- [x] 6.4 Record and review a fresh native-resolution smithy route, run all tests, and build the device plugin within the fixed Rockbox memory region.
