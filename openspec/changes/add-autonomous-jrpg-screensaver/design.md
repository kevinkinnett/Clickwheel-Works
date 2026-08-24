## Context

PocketStep is currently one header containing fixed-point body motion, solid
rectangles, overlap tests, mutable colliders, and stomp classification. It has
no allocator or Rockbox dependency. Clickwheel Works copies that header and
each plugin source into a persistent Rockbox 4.0 tree, then builds for the
`ipodcolor` simulator or device.

The new program has a 220 by 176 color display, a 20 Hz update loop, no required
viewer input, and little memory to waste on a general map or scripting runtime.
See `proposal.md` for motivation and the three delta specs for behavior.

## Goals / Non-Goals

**Goals:**

- Keep the existing PocketStep physics API backward compatible.
- Add reusable top-down navigation and story sequencing as optional headers.
- Keep every runtime buffer statically sized or caller owned.
- Make story progress depend on completed movement and interaction state.
- Produce a visually reviewable two-scene loop before adding content breadth.
- Keep simulator scenarios deterministic while the device follows its clock.

**Non-Goals:**

- A general RPG editor, save-file format, combat system, scrolling camera, or
  procedural map generator.
- Player-controlled movement, menus, character statistics, audio, or music.
- Compatibility with copyrighted maps, sprites, dialogue, or character names.
- More than one house screen, one outdoor screen, one item quest, and the NPCs
  needed for the opening story.

## Decisions

### Keep new engine systems in optional headers

Add `pocketstep_grid.h` for tile navigation and regions, and
`pocketstep_story.h` for the action director. Both use the same single-header
implementation convention as `pocketstep.h`. Mushroom Clock continues to
include only the physics header.

This keeps the small physics library readable and prevents a platform clock
from carrying route and dialogue code. Adding everything to `pocketstep.h` was
rejected because it would make the original library harder to embed. Keeping
all new code private to the screensaver was rejected because host-tested grid
navigation and completion-driven sequences are reusable engine behavior.

### Use a 13 by 11 fixed tile grid

Each scene uses 16-pixel tiles. Thirteen columns occupy 208 pixels and leave
six-pixel gutters on both sides. Eleven rows use the full 176-pixel height. A
dialogue box overlays the bottom three rows while active rather than resizing
or scrolling the map.

Fixed screens avoid a camera and guarantee that the whole route is visible.
An eight-pixel grid would allow more detail but would make characters and text
too small on the iPod. A scrolling map was rejected for the first release
because it adds camera, streaming, and edge-transition problems without
testing the core story loop.

### Use caller-owned breadth-first pathfinding

The grid module treats movement as uniform-cost, four-direction steps and uses
breadth-first search. The caller supplies route and workspace storage sized for
at most 143 cells. Neighbor order is fixed so equal routes resolve
deterministically.

Breadth-first search is smaller and easier to test than A-star at this map
size. Authored waypoints were rejected because they would only replay a path
and would not prove that NPC or item destinations can move later.

### Separate visual sprites from foot collision

Characters draw as 16 by 16 sprites. Navigation and overlap use a smaller box
around the bottom of each sprite. The actor moves toward tile centers in
fixed-point increments and changes walk frame from distance traveled.

Using the complete sprite as a collider was rejected because the character
could not overlap the visible fronts of furniture and trees. Drawing uses foot
Y as the ordering key for the actor, NPCs, items, and tall scenery.

### Use a portable action table with application callbacks

`pocketstep_story.h` defines fixed action records for walk, face, wait,
dialogue, collect, scene change, and end. The portable director owns the
current index, wait counter, completion state, and reset behavior. The
screensaver callback performs world-specific work and returns pending, done,
or failed.

This keeps Rockbox drawing, strings, scene tables, and inventory rules in the
plugin. A bytecode interpreter was rejected as unnecessary. A wall-clock
timeline was rejected because a delayed frame could skip visible causes, the
same problem found in early Mushroom Clock animations.

### Keep the story data static and explicit

The first story starts inside the house, collects one item, speaks with an
indoor NPC, exits, completes an outdoor interaction, pauses on a visible ending,
and resets. Static action arrays, map arrays, spawn tables, regions, and strings
live in the plugin binary.

No persistence is required. Every loop begins from a clean declared state.
Later variations can share scenes and choose among static scripts without
changing the director API.

### Render original indexed pixel art in code

Tiles and sprites use small palette-index arrays or compact row masks stored in
the plugin. Scene palettes provide indoor, day, evening, and night colors. The
clock remains visible in a small top overlay and reads Rockbox time every frame.

Code-owned indexed art matches the existing plugins, avoids file I/O, and makes
simulator captures reproducible. The visual density may recall handheld-era
JRPGs, but no existing map or sprite is traced or copied.

### Add one deterministic complete-story capture

The simulator can select a fixed story scenario through the same temporary-file
pattern used by Mushroom Clock. Its capture must include the house, item pickup,
dialogue, outdoor transition, and ending. The device does not expose simulator
scenario controls.

Build scripts should replace their two-name case statements and repeated copy
blocks with an explicit list of all supported plugins. This avoids adding a
third set of one-off conditions.

## Risks / Trade-offs

- [Breadth-first search workspace consumes scarce plugin memory] -> Cap the
  grid at 143 cells, use compact cell indexes, and assert storage sizes in host
  tests and device builds.
- [The autonomous actor can stall because of a bad destination] -> Return an
  explicit no-route result, enter a visible safe failure state in the
  simulator, and cover every authored destination with route tests.
- [Dialogue obscures the event it describes] -> Pause actor movement before
  opening the bottom overlay and keep speakers above the covered rows when
  possible.
- [Depth sorting becomes complicated] -> Sort only drawables with a foot Y and
  keep ground tiles in a separate first pass.
- [A single story becomes repetitive] -> Finish and validate one loop first;
  later OpenSpec changes can add scripts, randomized placements, and no-repeat
  selection.
- [New build-script generalization breaks existing clocks] -> Rebuild and
  launch Chronolith and Mushroom Clock as regression checks before accepting
  the new plugin.
- [The visual style resembles a commercial game too closely] -> Use original
  silhouettes, palette choices, place names, dialogue, and object designs.

## Migration Plan

1. Add and publish the optional PocketStep headers and host tests without
   changing existing functions.
2. Generalize the Rockbox build scripts while preserving both current plugin
   outputs.
3. Add the new plugin, capture tooling, and documentation.
4. Verify all three simulator plugins, the deterministic story capture, and
   the combined iPod package.
5. If the new program fails on device, remove it from Rockbox `SOURCES` and the
   combined archive while retaining the backward-compatible engine additions.

## Open Questions

- The public program name and story character names can change after the first
  visual capture without altering this architecture or its acceptance tests.
