## Why

Story Clock reduces its inventory to one boolean and a `KEY:YES` label, which cannot support the village's growing set of objects or make item collection visually interesting. A real fixed-capacity inventory and an autonomous item screen will let collected objects become part of the story instead of a tiny status flag.

## What Changes

- Add a header-only PocketStep inventory module with caller-owned storage, stable item slots, stack quantities, capacity reporting, lookup, addition, removal, and reset operations.
- Replace Story Clock's key boolean with a 12-slot session inventory. The temporary Ember Key is consumed at the beacon, while district keepsakes remain across autonomous story loops until the plugin exits.
- Add one collectible or NPC reward to each district itinerary so the inventory grows as different village routes run.
- Replace `KEY:YES` with a compact satchel badge showing occupied slots while the world is visible.
- Add a full-screen pixel-art satchel overlay with a 4 by 3 item grid, empty-slot treatment, selected-item highlight, quantity, name, and short description.
- Add an application-owned story action that pauses movement, opens the overlay with a stepped pixel animation, focuses the newly collected item, cycles through occupied slots, and closes before the story continues.
- Keep Story Clock autonomous. The overlay requires no click-wheel selection and the existing exit controls continue to work while it is open.
- Reuse licensed project art for item icons where it fits, generate only missing icons, and keep every source and prompt in the asset manifest.
- Add deterministic simulator scenarios, screenshots, a complete open-and-close recording, engine tests, and Story Clock integration tests.

## Capabilities

### New Capabilities

- `fixed-capacity-inventory`: Caller-owned item storage with deterministic slot order, quantities, capacity failures, lookup, removal, and reset behavior for constrained C programs.
- `autonomous-inventory-overlay`: Story Clock item collection, session persistence, item consumption, compact status, and a self-running JRPG-style inventory presentation.

### Modified Capabilities

None.

## Impact

The change adds `pocketstep_inventory.h`, its unit tests, build-script entries, and PocketStep documentation. Story Clock gains inventory state, item definitions, district reward actions, overlay animation state, drawing code, simulator scenarios, icons, and capture artifacts. The generated asset header and device plugin size will change. No heap allocation, save-file format, network access, or playable menu input is added.
