## 1. PocketStep inventory module

- [x] 1.1 Add `pocketstep_inventory.h` with caller-owned slot and inventory structures, result codes, initialization, clear, indexed retrieval, and quantity lookup.
- [x] 1.2 Implement atomic add behavior for new slots, existing stacks, capacity failure, invalid input, and stack-limit failure.
- [x] 1.3 Implement partial and complete removal with stable left compaction and no mutation on invalid or excessive removal.
- [x] 1.4 Add inventory unit tests covering initialization, stable order, stacking, compaction, capacity, stack limits, queries, invalid input, and reset.
- [x] 1.5 Register the new test in CMake and the container test script, then document the inventory module and its fixed-memory limits in the PocketStep README.

## 2. Story Clock inventory state

- [x] 2.1 Add twelve caller-owned slots, a PocketStep inventory, recent-item state, and overlay phase state to Story Clock.
- [x] 2.2 Add the six-item Story Clock catalog with stable IDs, maximum stacks, names, two-line descriptions, and icon references.
- [x] 2.3 Initialize inventory once at plugin startup while resetting only transient inventory presentation state between story loops.
- [x] 2.4 Replace `item_collected` checks with inventory quantity for Ember Key drawing, interaction, collection, and simulator previews.
- [x] 2.5 Add application-owned grant, consume, and present action handlers with duplicate suppression and inventory-specific failure diagnostics.
- [x] 2.6 Extend host integration tests for session persistence, key consumption, district reward uniqueness, duplicate routes, and full-capacity failure.

## 3. Story rewards and sequencing

- [x] 3.1 Present the satchel after the Ember Key is first collected and consume the key when the beacon activates.
- [x] 3.2 Add Market Token, Brass Cog, Iron Charm, Seed Pouch, and Mint Sprig rewards at suitable authored moments in their district itineraries.
- [x] 3.3 Follow each successful district grant with a completion-driven inventory presentation and confirm repeated itineraries skip duplicate presentations.
- [x] 3.4 Update Story Clock narrative documentation to describe temporary quest items and persistent session keepsakes.

## 4. Item art and asset pipeline

- [x] 4.1 Audit the current manifest, generated sources, and licensed packs for reusable key, token, mechanism, metalwork, crop, plant, and satchel art.
- [x] 4.2 Add or generate only the missing 16 by 16 item and satchel icons, retaining license files or generation prompts beside every source.
- [x] 4.3 Add deterministic manifest entries for all inventory icons, compile `storyclock_assets.h`, and pass asset compiler provenance and reproducibility tests.
- [x] 4.4 Inspect every compiled icon at native scale and revise any silhouette that is unclear against the inventory cell colors.

## 5. Inventory presentation

- [x] 5.1 Replace `KEY:YES` with a compact satchel icon and occupied-slot count that updates after additions and removals.
- [x] 5.2 Draw the centered 204 by 160 panel, title and count bar, 4 by 3 grid, occupied and empty cells, pulsing focus, and right-side details.
- [x] 5.3 Implement eight-update stepped opening and closing phases, new-item hold timing, and deterministic twenty-update focus cycling.
- [x] 5.4 Keep world ambience visible behind the overlay while story movement and dialogue remain paused, and preserve immediate plugin exit handling.
- [x] 5.5 Add palette-aware colors and fixed-width item copy that remain legible indoors and during day, evening, and night.

## 6. Simulator review and capture

- [x] 6.1 Add deterministic scenario 70 with all six items and a complete presentation, plus scenario 71 with the empty overlay held open.
- [x] 6.2 Expose `inventory` and `inventory-empty` through the recording and contact-sheet scripts.
- [x] 6.3 Capture native-resolution indoor, day, evening, and night screenshots and correct layout, clipping, contrast, or text problems.
- [x] 6.4 Record one complete inventory opening, focus cycle, and closing MP4 for review.

## 7. Verification and artifacts

- [x] 7.1 Run the complete PocketStep and Story Clock host test suite and resolve failures.
- [x] 7.2 Build Story Clock for the simulator and verify both inventory scenarios run without route or inventory diagnostics.
- [x] 7.3 Build the iPod Color device target without installing it and update tracked plugin size and SHA-256 metadata.
- [x] 7.4 Validate `add-storyclock-inventory-overlay` with strict OpenSpec validation.
