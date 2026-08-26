## 1. Asset sourcing and review

- [x] 1.1 Add a working-village sourcing table to the Story Clock asset notes that inventories existing candidates and records accepted or rejected farm, garden, and smithy choices with native-scale reasons.
- [x] 1.2 Acquire only the selected redistributable CC0 crop, chicken, and anvil sources, retain their local license and attribution files, and add complete provenance records.
- [x] 1.3 Build native-scale reuse mockups for South Fields, Herb Garden, and the smithy, then generate and document only any landmark that still lacks a readable reusable candidate.

## 2. Asset compilation

- [x] 2.1 Add manifest sources and deterministic crops for accepted farm, garden, animal, and smithy artwork, including day, evening, and night variants where required.
- [x] 2.2 Extend asset compiler tests for the new recipes, compile `src/storyclock_assets.h`, and confirm a second unchanged compile is byte-identical.

## 3. Connected district data

- [x] 3.1 Add South Fields and Herb Garden scene identifiers, tile maps, collision maps, regions, inactive entrances, props, and NPC state without changing the existing scene indices.
- [x] 3.2 Add reciprocal South Gate to South Fields and Market Row to Herb Garden links with matching edge offsets and passable spawn cells.
- [x] 3.3 Extend host world tests to validate new links, entrances, collision footprints, spawn cells, conversation cells, interaction cells, and every required route leg.

## 4. District rendering and activity

- [x] 4.1 Render South Fields as an open farm with a central road, restrained crop plots, a grounded farm building, and a fenced chicken area.
- [x] 4.2 Render Herb Garden as an enclosed garden with shaped beds, a bent walkable path, and a grounded greenhouse or potting shed.
- [x] 4.3 Add collision-neutral deterministic chicken, garden, and smithy ambient loops with readable frame timing.
- [x] 4.4 Add exterior smithy tools and forge activity at South Gate without obscuring its gate route or inactive door.

## 5. Autonomous stories

- [x] 5.1 Add farmer and gardener encounters using existing character art, correct conversation facing, and district-specific dialogue.
- [x] 5.2 Add complete farm and garden itineraries with interactions, reciprocal transitions, declared endings, and full reset behavior.
- [x] 5.3 Add forced simulator scenarios for the farm and garden and extend itinerary tests for deterministic selection and state reset.

## 6. Visual and device verification

- [x] 6.1 Run all PocketStep host tests, asset tests, and Story Clock world tests, then build the simulator plugin.
- [x] 6.2 Record the forced farm and garden itineraries and create native-scale day, evening, and night review sheets for the farm, garden, and smithy.
- [x] 6.3 Correct any floating landmarks, hidden entrances, muddy silhouettes, route overlaps, flickering animation, or palette failures found in the recordings and review sheets.
- [x] 6.4 Build the iPod Color `.rock`, compare its size with the current artifact, and verify the final plugin remains loadable within the target constraints.
