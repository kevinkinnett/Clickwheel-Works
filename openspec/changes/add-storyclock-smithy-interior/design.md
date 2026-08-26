## Context

See `proposal.md` for motivation. Story Clock currently has eight 13 by 11 scenes, but only `SCENE_HOUSE` uses the indoor drawing path. South Gate already exposes `REGION_GATE_SMITH` as a disabled entrance and already awards the Iron Charm during its exterior itinerary. The fixed-capacity inventory can detect and reject duplicate rewards, and the inventory overlay already skips presentation when no new item was added.

The iPod target has a 220 by 176 display and no runtime image decoding. Maps, actions, assets, and state must remain static or caller-owned. The project compiles source PNGs to RGB565 arrays using deterministic manifest operations.

## Goals / Non-Goals

**Goals:**

- Make the smithy a complete autonomous visit rather than a decorative facade.
- Remove the rendering assumption that one named scene is the only indoor room.
- Prove a reuse-first interior art workflow before adding the inn, apothecary, shop, mill interior, farmhouse, and garden shed.
- Preserve fixed memory use, deterministic simulator runs, and native-resolution review.

**Non-Goals:**

- Implement every village interior in this change.
- Add player control, crafting, shops, equipment statistics, combat, saving, scrolling maps, or runtime procedural layouts.
- Replace the existing character sheets or restyle the village exterior.
- Add a large generic engine abstraction before a second application needs it.

## Decisions

### Add the smithy as scene eight with reciprocal door metadata

Append `SCENE_SMITHY` so existing scene identifiers and simulator scenario numbers remain stable. Replace the South Gate smithy's no-destination entrance with a destination near the bottom-center smithy doorway. Add a smithy exit region that returns to the exterior threshold below the facade door, facing down.

The interior uses the existing 13 by 11 grid. Its collision map reserves a simple loop from the doorway to Rowan and the forge work area, then back to the doorway. Large furniture occupies solid cells. Visual overhang may extend above its collision footprint, but the foot position defines depth.

Direct story-only scene changes were considered. Using reciprocal entrance records also validates the map as a connected place and gives later autonomous routes the same navigation rule.

### Use an app-owned interior presentation table

Add a small scene presentation record in Story Clock with scene kind, architecture identifier, landmark or ambient identifier, and palette behavior. Both the opening house and smithy use the indoor render stages:

1. floor and wall base
2. fixed architecture behind actors
3. depth-sorted solid props, ambient overlays, NPCs, and Luma
4. foreground overhangs, dialogue, and inventory UI

This removes `SCENE_HOUSE` checks from common indoor decisions without putting Rockbox graphics pointers or Story Clock asset IDs into PocketStep. A PocketStep API change is unnecessary unless implementation reveals a genuinely reusable, graphics-independent record.

Function pointers and a general scene-component system were considered. A fixed data table is smaller, easier to inspect, and sufficient for the seven planned interiors.

### Make the forge the smithy's dominant landmark

The layout puts a large masonry forge against the upper wall, an anvil and quench trough near its work zone, and tools, fuel, crates, and barrels along the sides. The lower-center doorway and middle aisle remain quiet so Luma never disappears into clutter.

The ambient loop changes only small sub-elements. Fire may use two or three frames, sparks may appear intermittently, and steam may use a short anchored loop. The forge base, rim, chimney, and collision footprint never flip or translate.

The exterior approach runs along the open ground below the facade before turning north into the door. The facade footprint remains blocked except for that threshold, so pathfinding cannot draw Luma across the building art.

The finished interior uses more edge detail than the first accepted pass. Timber braces, wall-mounted metal stock, ember lamps, crates, barrels, and small floor-edge clutter occupy blocked perimeter cells or background layers. The central aisle and Rowan conversation cells stay unchanged. Existing compiled props and code-drawn details take priority over additional large image arrays because the iPod plugin region has little spare memory.

### Run a native-scale asset audition before choosing the final sprites

Build the same smithy layout in up to three treatments:

- existing Story Clock sources only
- selected CC0 library art, with Ninja Adventure and Kenney Roguelike Indoors as the first candidates
- the strongest reusable base plus generated signature art for any failed forge, bellows, trough, or tool-rack role

The audition runs at 220 by 176 and includes Luma and Rowan for scale. It checks silhouette, palette, perspective, doorway grounding, actor contrast, and crop quality. Temporary review sheets stay under ignored `artifacts/`; accepted source files, license text, URL, version, selection notes, and prompts move into `assets/storyclock/`.

The repository will not absorb an entire large pack. It will retain the smallest original source sheets needed to reproduce accepted crops, plus the pack license and provenance. Generated art uses a flat keyed background or alpha, no text, no smoothing, a fixed top-down perspective, and a prompt stored beside the final source.

### Move the Iron Charm reward inside without adding another item type

Extend the existing South Gate itinerary to enter the smithy and speak with Rowan. The conversation action selects first-visit or repeat text from the existing Iron Charm inventory quantity. Rowan and Luma set their facing immediately before dialogue.

On the first visit, the existing unique grant action adds one Iron Charm and the existing inventory presentation opens. On a repeat visit, the grant reports no new item, the overlay action completes without opening, and repeat dialogue acknowledges the charm. The inventory remains the authoritative session state, so no duplicate `has_visited_smithy` flag can drift out of sync.

### Add a forced smithy simulator route and focused host tests

Reserve a deterministic simulator scenario for the complete first visit. The capture script records entry, ambience, dialogue, reward, inventory overlay, exit, and exterior return. A separate still or held-preview point exposes the room long enough to inspect draw order.

Host tests validate reciprocal entrances, passable route legs, blocked furniture, matching doorway columns, unique Iron Charm state, and repeat-visit behavior. The standard asset compiler determinism test covers the new manifest entries.

## Risks / Trade-offs

- [Mixed packs make the room look assembled from spare parts] -> Choose one base treatment, apply a common palette, and use other sources only for small neutral props.
- [Generated pixel art becomes muddy after reduction] -> Review every candidate at 220 by 176 and reject it before integration if its silhouette does not survive nearest-neighbor conversion.
- [The forge consumes too much plugin storage] -> Keep the base static and animate only small overlays with tightly cropped frames.
- [Furniture blocks autonomous paths] -> Author collision before decoration and test every route leg on the host.
- [Moving the charm changes familiar route timing] -> Keep the existing item ID and inventory lifecycle, then add a deterministic recording that makes the new timing reviewable.
- [A reusable scene table becomes premature engine work] -> Keep it app-owned in this change and move only graphics-independent behavior into PocketStep after a second program needs it.

## Migration Plan

1. Add the smithy scene, collision, reciprocal entrance, and placeholder render path while preserving the current exterior itinerary.
2. Build and review the three native-scale art treatments, then document the accepted sources and rejection reasons.
3. Compile the accepted smithy assets and replace placeholder drawing.
4. Move Rowan and the Iron Charm exchange into the smithy itinerary, including repeat dialogue and return navigation.
5. Add host coverage, deterministic capture, simulator verification, and device build verification.

Rollback restores `REGION_GATE_SMITH` to no destination and the previous exterior Iron Charm actions. Removing the appended scene and its asset arrays does not change existing scene identifiers, inventory IDs, or PocketStep APIs.
