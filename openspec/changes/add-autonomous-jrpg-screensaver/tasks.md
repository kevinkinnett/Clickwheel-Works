## 1. PocketStep grid navigation

- [x] 1.1 Add the optional `pocketstep_grid.h` single-header module with fixed-size grid, cell, path, workspace, region, and result types.
- [x] 1.2 Implement blocked-cell and out-of-bounds queries for caller-owned rectangular grids.
- [x] 1.3 Implement deterministic four-direction breadth-first routing with explicit no-route and insufficient-capacity results.
- [x] 1.4 Implement facing probes and numeric interaction-region lookup without making regions solid.
- [x] 1.5 Add host tests for obstacle routes, shortest paths, deterministic ties, unreachable destinations, buffer capacity, bounds, and facing regions.
- [x] 1.6 Document the grid API, memory limits, neighbor order, and a minimal top-down example in the PocketStep README.

## 2. PocketStep story director

- [x] 2.1 Add the optional `pocketstep_story.h` single-header module with portable action records, action results, director state, and reset behavior.
- [x] 2.2 Implement ordered action dispatch, fixed-update waits, pending completion, safe failure, end-of-story, and loop reset.
- [x] 2.3 Add host tests proving that actions do not overlap, waits use update counts, failures stop unsafe progress, and resets restore the opening action.
- [x] 2.4 Document the action callback contract and a minimal autonomous sequence in the PocketStep README.

## 3. Rockbox build integration

- [x] 3.1 Choose a temporary internal plugin name and generalize simulator build support from two hard-coded plugins to an explicit supported-plugin list.
- [x] 3.2 Generalize device copying, Rockbox registration, package assembly, and checksum generation to include the third plugin.
- [x] 3.3 Generalize simulator launch and capture scripts so each supported plugin can become `autostart.rock` without changing source files by hand.
- [x] 3.4 Rebuild Chronolith and Mushroom Clock after the script changes and confirm their simulator artifacts remain available.

## 4. Screensaver world and rendering

- [x] 4.1 Add the new Rockbox plugin skeleton with a 20 Hz loop, current-time reads, backlight handling, clean exit, and simulator scenario loading.
- [x] 4.2 Define caller-owned 13 by 11 house and outdoor tile maps with collision cells, spawn points, exits, interaction regions, and authored route targets.
- [x] 4.3 Draw original indexed 16-pixel ground, wall, floor, furniture, tree, water, path, door, item, character, and NPC art.
- [x] 4.4 Implement fixed-point actor movement between tile centers with four facings and distance-driven walk frames.
- [x] 4.5 Implement foot-box collision and foot-Y draw ordering for the actor, NPCs, items, and tall scenery.
- [x] 4.6 Add indoor, day, evening, and night palettes without changing route or collision data.

## 5. Autonomous story loop

- [x] 5.1 Connect the PocketStep director callback to actor walking, facing, waiting, dialogue, item collection, scene changes, and story reset.
- [x] 5.2 Author the house sequence so the character wakes, crosses valid routes, collects the quest item, speaks with the indoor NPC, and reaches the exit.
- [x] 5.3 Author the outdoor sequence so the character spawns at the door, completes an NPC or object interaction, reaches a visible ending, and resets cleanly.
- [x] 5.4 Implement a two-line dialogue box with speaker name, continuation cue, text wrapping, and automatic duration based on visible character count.
- [x] 5.5 Add a compact inventory indicator and remove or alter the collected object's world drawing when the collection action completes.
- [x] 5.6 Keep the current time visible and updating during movement and dialogue, and select the outdoor palette from Rockbox time.
- [x] 5.7 Display a simulator-visible safe failure message if an authored destination has no valid route.

## 6. Deterministic review and delivery

- [x] 6.1 Add host or scenario tests for every authored destination, spawn, interaction ID, item transition, and complete story reset.
- [x] 6.2 Add a deterministic complete-story recording command and crop it to the iPod LCD rather than the simulator shell.
- [x] 6.3 Produce contact sheets covering the house, collection, indoor dialogue, scene transition, outdoor dialogue, ending, and second-loop reset.
- [x] 6.4 Review the capture for blocked routes, sprite jitter, unreadable text, incorrect depth, premature actions, and clock update failures, then correct any defects.
- [x] 6.5 Run all PocketStep host tests and build all three plugins for the `ipodcolor` simulator.
- [x] 6.6 Build the device package, verify plugin sizes and checksums, and confirm the combined archive contains all three `.rock` files.
- [x] 6.7 Update Clickwheel Works and PocketStep documentation with the new modules, screensaver behavior, controls, build commands, and capture locations.
- [x] 6.8 Publish the Clickwheel Works implementation and the identical PocketStep engine tree only after both repositories' workflows pass.
