## Context

Story Clock runs at 20 fixed updates per second on a 220 by 176 color display. It is an autonomous screensaver, so its story director owns timing and the click wheel is reserved for exit behavior. The current `item_collected` boolean controls the Ember Key sprite and a small `KEY:YES` label. `reset_story` clears that boolean every loop.

PocketStep modules use header-only C99, caller-owned arrays, integer return codes, and no heap allocation. The inventory must follow those constraints and remain useful outside Story Clock. See the proposal for motivation and the two capability specs for observable behavior.

## Goals / Non-Goals

**Goals:**

- Separate reusable item storage from Story Clock item names, icons, and presentation.
- Make acquisition readable without turning the screensaver into a playable menu.
- Let district rewards accumulate during one plugin session while keeping the repeatable looping story.
- Leave room for six more item types after the initial key and five keepsakes.
- Make review deterministic in the simulator rather than waiting for itinerary selection.

**Non-Goals:**

- Saving inventory to disk or restoring it after the plugin exits.
- Equipment, character statistics, item sorting, categories, shops, or combat use.
- Click-wheel navigation, manual cursor movement, or a pause menu.
- A generic UI toolkit inside PocketStep.
- New network or runtime asset-loading behavior.

## Decisions

### Add a small inventory module, not inventory fields to the story director

`pocketstep_inventory.h` will define a slot containing `item_id` and `quantity`, plus an inventory containing caller-owned slots, capacity, and occupied count. Its operations will initialize, clear, query by ID, retrieve by slot index, add, and remove. Addition receives an application-chosen maximum stack and uses explicit result codes for success, invalid input, full capacity, and stack overflow.

New item IDs append to the occupied range. Existing IDs change quantity in place. Removing a complete stack shifts later records left. This keeps grid order deterministic without holes or a second occupancy array.

Putting the storage in `pocketstep_story.h` would couple a general sequencer to one kind of game state. A bitset would be smaller for unique quest flags but could not represent quantities or stable acquisition order. A caller-owned slot array costs 96 bytes for twelve two-integer records and is still trivial on the target.

### Keep item metadata and story actions application-owned

Story Clock will define an item catalog with ID, name, two short description lines, icon asset, and maximum stack. The initial catalog is:

- Ember Key
- Market Token
- Brass Cog
- Iron Charm
- Seed Pouch
- Mint Sprig

The key and keepsakes use a maximum stack of one. Quantity still appears because the engine and overlay must support later consumables without redesign.

Three application-local action kinds will grant an item, present the recent acquisition, and consume an item. PocketStep's story director already delegates unknown action semantics to the application handler, so adding JRPG-specific constants to the reusable header would buy nothing. A grant sets `recent_item_id` only when it creates a new retained item. The following presentation action returns immediately when that value is zero, which suppresses duplicate keepsake screens.

The existing world-facing collect action remains responsible for the Ember Key. Each district NPC sequence gains one grant and one presentation. Beacon activation consumes the key. Inventory capacity or removal errors set an inventory-specific Story Clock diagnostic rather than reusing the route-capacity message.

### Persist slots across story resets, not plugin restarts

Plugin startup initializes the inventory once before the first story reset. `reset_story` clears routes, dialogue, and transient overlay state but leaves inventory slots intact. Key visibility and collection checks query key quantity instead of a boolean.

This gives the random itinerary order a visible memory. Repeating a district has no duplicate reward. The key remains a useful quest object because every loop collects it and consumes it at the beacon. Disk persistence would require versioning and error handling that the screensaver does not need yet.

### Use one pending action for the complete overlay sequence

The presentation handler will own four phases: opening, focus hold, focus cycle, and closing. Opening and closing each take eight updates. The new item remains focused for at least twenty updates before focus advances through occupied slots at a fixed twenty-update interval. The handler reports pending until closing finishes, so the next walk or dialogue cannot begin early.

The main loop continues drawing the current world and increments ambient animation time. It then draws the overlay last. Actor movement does not advance because the director is still executing the presentation action. The existing input poll remains outside the director and can exit at any phase.

A pair of open and close actions would let unrelated story work slip between them and would spread animation invariants across the action table. One completion-driven action matches the director's existing dialogue and walking model.

### Combine a Zelda-like grid with a Pokémon-like detail panel

The panel occupies a centered 204 by 160 rectangle. A title bar shows `SATCHEL` and occupied slots out of twelve. A 4 by 3 grid uses twelve 28-pixel cells on the left. The right side shows the focused icon at a larger visual scale, item name, `xN`, and two authored description lines. Empty cells use a low-contrast inset and occupied cells keep a clear icon silhouette. The focused cell gets a two-color pulsing border.

The grid borrows the immediate spatial scan of classic Zelda item screens. The right detail area borrows the name-and-description emphasis of Pokémon bag screens. Short fixed-width text keeps the legibility of Dragon Quest and EarthBound menus. Category tabs and nested lists would waste most of this display and add no value to an autonomous presentation.

Opening grows the outer frame in coarse pixel steps from the compact satchel badge location. Contents appear after the frame reaches full size. Closing reverses the steps. The effect uses filled rectangles and clipping by calculated bounds, with no alpha blending or large temporary bitmap.

The closed-world badge uses a satchel icon plus `NN/12` and replaces `KEY:YES`. Its count means occupied slots, not summed quantities.

### Reuse item art before generating gaps

The asset pass will audit the current manifest and licensed source packs first. The existing key, mill wheel or mechanism art, anvil, crop, and garden sources should cover most item silhouettes. Only the satchel, market token, or another icon that lacks a coherent source will be generated. Every new asset keeps its license or generation prompt beside the source and enters the deterministic manifest.

This is better than drawing icons procedurally in C. The generated RGB565 header remains the only runtime representation, while the source tree preserves provenance and lets the compiler reproduce the result.

### Add dedicated simulator review modes

Scenario value 70 will seed the six catalog items, pause on a representative exterior, and run a complete deterministic inventory presentation. Value 71 will hold the empty satchel open for empty-state review. The PowerShell recording and contact-sheet scripts will expose `inventory` and `inventory-empty` names rather than requiring numeric files.

The populated scenario is also the source for a native-resolution screenshot and open-and-close MP4. Palette-forced screenshots will check the overlay against indoor, day, evening, and night colors.

## Risks / Trade-offs

- [The overlay covers too much of the scene] -> Keep a four-pixel world margin and a stepped opening so the transition remains visually tied to the character's current location.
- [Descriptions become unreadable in the narrow detail panel] -> Author two short fixed lines per item and verify native-resolution captures rather than runtime word wrapping.
- [A repeated route tries to present an old keepsake] -> Clear `recent_item_id` before each grant and skip presentation when no new slot was created.
- [The key disappears before its world sprite updates] -> Derive sprite visibility directly from inventory quantity on every frame.
- [A future catalog exceeds twelve item types] -> Return a visible capacity failure now. Paging is deferred until content actually needs a thirteenth slot.
- [New icons increase plugin size] -> Prefer 16 by 16 sources and compile only the six item icons plus one satchel icon.

## Migration Plan

1. Add the PocketStep inventory module, tests, build entries, and README documentation.
2. Replace Story Clock's boolean with caller-owned inventory storage and catalog lookup.
3. Migrate key collection and beacon use, then add one persistent reward to each district itinerary.
4. Add the badge, overlay state machine, drawing code, icon assets, and simulator scenarios.
5. Run engine and world tests, regenerate assets, capture visual reviews, and build simulator and device targets.

Rollback restores the boolean key state and removes the inventory presentation actions. No save data or device installation needs migration.
