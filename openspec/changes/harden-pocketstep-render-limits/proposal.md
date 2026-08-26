## Why

PocketStep's fixed-capacity draw list currently rejects every record added after it fills. Story Clock exposed the dangerous case: decorative scenery filled the list before the player actor was queued, so the actor vanished while the scene otherwise kept running.

## What Changes

- Add an overflow-retention priority to each drawable and a priority-aware insertion operation. Priority affects only which records survive capacity pressure; visible foot position still controls draw order.
- When the priority-aware operation receives a higher-priority drawable on a full list, discard one lower-priority record by a documented deterministic rule and retain the incoming record.
- Preserve the existing insertion operation and its full-list behavior for source compatibility. The priority-aware operation also rejects an incoming drawable when it is not more important than any retained record.
- Count records discarded through either rejection or replacement, expose the count to callers, and reset it when the list is cleared.
- Mark Story Clock actors, NPCs, and interactive items as required while leaving replaceable scenery optional.
- Add regression tests and documentation for capacity pressure, stable ordering, deterministic replacement, and diagnostics.

## Capabilities

### New Capabilities

- `prioritized-draw-retention`: Protect required drawables on fixed-capacity lists through opt-in priority-aware insertion and report discarded records.

### Modified Capabilities

None.

## Impact

The public `pocketstep_draw.h` record and list structures gain fields, but the current initializer and insertion operation remain valid. Existing positional record initializers receive the optional default priority. The change affects PocketStep draw tests, Story Clock's draw queue, and the draw-order documentation. It adds no heap allocation, graphics dependency, or external library.
