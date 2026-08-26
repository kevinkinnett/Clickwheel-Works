## 1. PocketStep scene and itinerary support

- [x] 1.1 Add caller-owned scene-link lookup and validation helpers with host
  tests for linked, unlinked, blocked-entry, and invalid-destination cases.
- [x] 1.2 Add building-entrance and static-prop metadata with validation and
  host tests for disabled entrances, spawn bounds, foot positions, and flags.
- [x] 1.3 Add deterministic itinerary selection from seed, loop index, and
  itinerary count with host tests for repeatability, variation, and bad counts.
- [x] 1.4 Document the new optional PocketStep records and selection helper in
  the engine README.

## 2. Village maps and transitions

- [x] 2.1 Add Village Green, River Mill, Market Row, and South Gate tile and
  collision maps while preserving Cottage Rise as the northern scene.
- [x] 2.2 Author reciprocal scene links at column six and row five, then convert
  Cottage Rise props to the shared prop metadata.
- [x] 2.3 Add inactive entrance regions and collision footprints for every new
  building facade.
- [x] 2.4 Extend world tests to validate all reciprocal entries, authored route
  legs, inactive doors, prop collision cells, and unreachable failure cases.

## 3. Village art and asset pipeline

- [x] 3.1 Select reusable CC0 terrain and prop sources, retain their original
  license files, and record provenance in the Story Clock manifest.
- [x] 3.2 Create or generate readable source architecture for the Village Green,
  River Mill, Market Row, and South Gate landmarks.
- [x] 3.3 Add manifest recipes for architecture, props, and ambient frames with
  day, evening, and night variants and transparent nearest-neighbor output.
- [x] 3.4 Compile the bitmap header, inspect every landmark at native iPod scale,
  and adjust crops or placement until doors and route corridors remain clear.

## 4. Autonomous village behavior

- [x] 4.1 Add district NPCs, dialogue, and one ambient animation to every new
  exterior scene using shared draw ordering where depth matters.
- [x] 4.2 Author complete Market Row, River Mill, and South Gate itineraries that
  pass through Village Green and perform district-specific interactions.
- [x] 4.3 Select itineraries during story reset, reset all itinerary state at the
  loop boundary, and add simulator overrides for each route.
- [x] 4.4 Verify that day, evening, and night affect presentation and ambient
  details without changing maps, collision, or action order.

## 5. Verification and delivery

- [x] 5.1 Run the full PocketStep and Story Clock host test suites and fix all
  route, transition, asset compiler, and reset regressions.
- [x] 5.2 Build the simulator and record each forced district itinerary using
  iPod-display-only capture.
- [x] 5.3 Produce native-scale day, evening, and night review sheets covering all
  five exterior scenes and correct any unreadable or cluttered composition.
- [x] 5.4 Build the iPod Color plugin, record its size and checksum, and update
  project documentation with the village geography, assets, licenses, run
  commands, and artifact paths.
- [x] 5.5 Correct River Mill and South Gate architecture traversal, replace the
  mirrored wheel flip with readable rotation frames, and add regression tests.
- [x] 5.6 Re-record both corrected itineraries, inspect dense review sheets,
  rebuild the iPod Color plugin, and update its checksum.
