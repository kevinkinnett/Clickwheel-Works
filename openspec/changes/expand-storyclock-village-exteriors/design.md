## Context

Story Clock currently owns two 13 by 11 scenes, one linear action array, and
application-specific outdoor prop drawing. PocketStep already supplies grid
pathfinding, edge entry selection, scene descriptions, story actions, sprite
selection, and foot-position draw ordering. All device data must remain static
or caller-owned because the iPod plugin cannot depend on heap allocation or
runtime image decoding.

The current generated cottage works because its door, collision footprint, and
path share the same tile column. The remaining village screens need the same
discipline. At 220 by 176 pixels, extra decoration can make a route unreadable
very quickly.

## Goals / Non-Goals

**Goals:**

- Represent the village as a connected scene graph with matching edge offsets.
- Keep maps, links, props, entrances, routes, and itineraries fixed-size and
  host-testable.
- Give each district one dominant landmark, one clear route, and restrained
  ambient motion.
- Make every itinerary directly recordable in the simulator.
- Keep architecture and props reusable for later indoor and outdoor screens.

**Non-Goals:**

- Implement the interiors behind the new building doors.
- Add player-controlled movement, combat, shops, inventory menus, or saving.
- Add scrolling maps, procedural map generation, or runtime asset loading.
- Replace existing character sprites or rewrite the pathfinder.

## Decisions

### Use Village Green as a hub

The scene graph is Cottage Rise north of Village Green, River Mill west,
Market Row east, and South Gate south. Each connection uses column six or row
five on both sides. This makes reciprocal transitions easy to validate and
gives later village additions obvious attachment points.

A single linear chain was considered. It would avoid revisiting the green, but
it would make the village feel like a corridor and make later branches harder
to add.

### Describe links, entrances, and props as caller-owned records

`pocketstep_scene.h` will add small records and validation helpers. A scene link
contains source scene, travel direction, destination scene, and preferred
offset. An entrance contains a region ID plus destination spawn and facing. A
prop contains position, foot position, asset ID, and flags. Story Clock owns
the arrays and rendering.

Embedding application callbacks or graphics pointers in PocketStep was
rejected because it would couple the portable headers to Rockbox and make host
tests harder.

### Select one complete itinerary at reset

PocketStep will select a script index from an application seed, loop index, and
script count. Story Clock then initializes the existing director with that
action array. Simulator scenarios can force an index.

A branch action inside the action stream was considered. Selecting the whole
script before execution is smaller, easier to test, and prevents half-finished
branches from sharing accidental state.

### Keep one map size and one transition grammar

All exterior scenes remain 13 by 11 tiles. Edge transitions use the existing
nearest-passable entry rule. Door transitions use explicit entrance metadata.
The actor walks to an edge before the scene changes and appears just inside the
opposite edge facing the same travel direction.

### Separate base tiles, architecture, props, and ambient animation

Each scene will draw in four stages: terrain tiles, fixed architecture,
depth-sorted props and characters, then dialogue. Static prop descriptors feed
the existing draw list. Ambient elements derive frames from the global frame
counter and do not affect collision.

### Use generated facades only for large landmarks

Village Green, River Mill, Market Row, and South Gate will use project-owned
generated architecture where a clear silhouette matters. Native 16-pixel CC0
art will supply fences, crates, barrels, signs, flowers, rocks, water details,
and small props. The manifest will crop sources once and emit day, evening, and
night arrays.

This avoids paying to generate every barrel and avoids shrinking an entire
generated scene into unreadable pixels.

### Keep paths sparse

Each screen reserves a two-tile-wide route corridor around required entries,
destinations, NPCs, and doors. Decorative solid cells stay outside those
corridors. Host tests copy the authored collision grids and verify every leg of
every itinerary.

## Risks / Trade-offs

- [Generated facades consume plugin space] -> Crop tightly, reduce with nearest
  neighbor, share small props, and check the final `.rock` size after each art
  pass.
- [Different source packs can look inconsistent] -> Use generated art for the
  landmarks, a shared palette tint, and only small neutral CC0 props.
- [Hub revisits can feel repetitive] -> Use different paths, NPC positions, and
  ambient states on outbound and return visits.
- [Scene-specific code can grow into another private engine] -> Put only data
  records and deterministic selection helpers in PocketStep. Keep narrative and
  asset choices in Story Clock.
- [Night tint can hide entrances] -> Preserve warm window and lantern accents
  and include native-scale night review sheets in acceptance checks.

## Migration Plan

1. Add and test the PocketStep records and itinerary selector without changing
   the existing Story Clock route.
2. Convert Cottage Rise to the new link and prop descriptions.
3. Add the four collision maps and placeholder architecture, then validate all
   links and routes.
4. Add final art, ambient motion, NPCs, dialogue, and palette variants one
   district at a time.
5. Record each forced itinerary and all palettes, then build the device plugin.

Rollback consists of removing the new optional metadata helpers and restoring
Story Clock's previous two-scene tables and linear action array. Existing
PocketStep pathfinding and asset compiler formats remain compatible.
