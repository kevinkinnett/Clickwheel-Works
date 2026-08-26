## 1. PocketStep draw API

- [x] 1.1 Append retention priority to `ps_drawable`, define optional and required priority constants, and initialize the draw-list discard counter.
- [x] 1.2 Add a shared stable insertion helper so legacy and priority-aware operations preserve the same foot-position order.
- [x] 1.3 Implement priority-aware full-list replacement using the last lowest-priority record and keep legacy full-list rejection unchanged.
- [x] 1.4 Expose the discard count, increment it for capacity rejection or replacement, and reset it when the list is cleared.

## 2. Engine regression tests

- [x] 2.1 Update draw test records to state their priority and confirm priorities do not alter stable depth ordering or payload values.
- [x] 2.2 Test legacy rejection, required-record replacement, equal-priority rejection, and deterministic selection among optional records.
- [x] 2.3 Test that an actor survives a scenery-filled list and that discard diagnostics count and reset each outcome.

## 3. Story Clock migration

- [x] 3.1 Classify the player, NPCs, and interactive items as required and classify replaceable scene layers as optional at the Story Clock queue boundary.
- [x] 3.2 Use priority-aware insertion in Story Clock while preserving its existing failure reporting and fixed draw-list capacity.
- [x] 3.3 Run the Story Clock world tests to confirm scene transitions and navigation remain unchanged.

## 4. Documentation and verification

- [x] 4.1 Document the optional default, priority-aware insertion, deterministic replacement rule, required-set capacity limit, and discard diagnostics in the PocketStep README.
- [x] 4.2 Run the complete PocketStep test script and resolve any failures.
- [x] 4.3 Build Story Clock for the simulator and iPod device target without installing it, then record artifact size and checksum changes if the project documentation tracks them.
- [x] 4.4 Validate `harden-pocketstep-render-limits` with strict OpenSpec validation.
