## Context

PocketStep is a header-only C engine intended for constrained Rockbox targets. Its draw list uses caller-owned fixed storage, stable insertion by `foot_y`, and no graphics API. See `proposal.md` for the failure that motivated this change and `specs/prioritized-draw-retention/spec.md` for the behavior contract.

The current `ps_draw_list_add` operation reports a full list by returning zero. That contract is useful to existing callers and remains valid. Story Clock needs an opt-in operation that can protect actors when replaceable scenery has already consumed the list.

## Goals / Non-Goals

**Goals:**

- Retain essential drawables under capacity pressure without heap allocation.
- Make every overflow decision reproducible from the same input sequence.
- Keep foot-position ordering and equal-depth stability unchanged.
- Preserve the existing insertion operation for current PocketStep callers.
- Let an application detect that visual records were discarded during a frame.

**Non-Goals:**

- Dynamic list growth, heap allocation, or a second rendering pass.
- Screen-space culling or visibility calculations inside PocketStep.
- Asset-manifest transparency and crop assertions. Those belong in the asset compiler, not the draw list.
- Automatic classification of application-specific drawable kinds.

## Decisions

### Append priority to the drawable record

`struct ps_drawable` will append an integer `retention_priority` field. PocketStep will define `PS_DRAW_PRIORITY_OPTIONAL` as zero and `PS_DRAW_PRIORITY_REQUIRED` as one. Callers may use other integer levels because the replacement rule compares values rather than recognizing only the two macros.

Appending the field keeps existing five-value positional initializers valid in C. Their omitted field becomes zero, which is the optional default. An alternative parallel priority array would require more caller storage and a second initialization contract. Encoding priority into `kind` or `id` would corrupt application payload semantics.

### Add an opt-in prioritized operation

`ps_draw_list_add` will preserve its current success and failure behavior. A new `ps_draw_list_add_prioritized` operation will share the same validation and stable insertion path but may replace a retained record when the list is full.

Changing `ps_draw_list_add` directly would silently alter existing applications. A second operation makes the capacity policy explicit at the call site and lets applications migrate one queue at a time.

### Replace the last lowest-priority record

On a full list, prioritized insertion scans for the lowest retained priority. It accepts the incoming record only if the incoming priority is higher. Among records at the lowest priority, it removes the one with the greatest current draw-order index, then inserts the incoming record by `foot_y` using the existing stable rule.

Keeping existing records when priorities tie prevents late additions from churning an otherwise stable frame. Choosing the last lowest-priority record makes the result deterministic and takes advantage of the list's existing order without storing insertion sequence numbers. Choosing the first candidate would also be deterministic, but it would discard backmost scenery first and make gaps near the top of a scene more noticeable.

### Count discarded records per cleared list

`struct ps_draw_list` will append `discarded_count`. A small accessor will return the count after validating the list pointer. Both rejected incoming records and replaced retained records increase the count by one. Invalid API calls do not change it. `ps_draw_list_clear` resets `count` and `discarded_count`, matching a frame-oriented queue.

A boolean overflow flag would show that something happened but not how much pressure a scene created. The counter gives tests and diagnostic overlays more useful information at the same fixed memory cost.

### Classify Story Clock records at its queue boundary

Story Clock's queue helper will assign required priority to the player actor, NPCs, and interactive items. Trees, building layers, fences, water overlays, and other replaceable scenery will use optional priority. Story Clock will use prioritized insertion while still checking its return value.

The application remains responsible for choosing its own importance policy. PocketStep does not inspect drawable kinds.

## Risks / Trade-offs

- [A caller ignores discard diagnostics] -> The insertion result still reports whether the incoming record survived, and Story Clock continues to check that result.
- [A required set alone exceeds capacity] -> Equal-priority incoming records are rejected, so callers must size storage for their required records. Documentation and tests will state this limit.
- [Appending structure fields exposes compiler warnings in strict downstream builds] -> PocketStep documents the optional zero default and updates its own positional initializers to name the priority explicitly.
- [Replacement adds linear work to an already linear insertion] -> Lists are intentionally small and bounded. The full-list path performs at most two scans and one shift, with no allocation.

## Migration Plan

1. Append the priority and diagnostic fields, constants, accessor, and prioritized insertion operation to `pocketstep_draw.h`.
2. Update draw-list tests to cover the old operation, prioritized replacement, tie rejection, stable depth order, and counter reset.
3. Migrate Story Clock's queue helper to assign priorities and call the new operation.
4. Update PocketStep documentation, then run the engine tests and rebuild Story Clock for the simulator and device target.

Rollback consists of restoring the previous header and Story Clock queue call. No serialized state or on-device data format changes.
