## Context

The current Story Clock world contains six 13 by 11 scenes, including the room and five exterior scenes. Village Green connects Cottage Rise, River Mill, Market Row, and South Gate. South Gate already contains a smithy wing and an inactive entrance. Market Row has an unused east edge, and South Gate has an unused south edge.

PocketStep already provides fixed-memory scene links, inactive entrance metadata, static props, grid pathfinding, deterministic itinerary selection, sprite animation selection, and draw ordering. Story Clock owns its maps, routes, narrative state, asset selection, and ambient rendering. The iPod Color target has a 220 by 176 display and cannot load images at runtime.

The existing asset catalog includes generated terrain and vegetation, generated village facades, Kenney Tiny Town CC0 tiles, Luis Zuno's CC0 RPG Town sheet, PixelandBeans' CC BY 4.0 indoor furniture, and three character sheets. The new districts must use this material where it fits before adding anything else.

## Goals / Non-Goals

**Goals:**

- Add two connected districts without changing the existing hub layout or scene transition rules.
- Give the farm, garden, and smithy different silhouettes, path shapes, colors, and ambient behavior.
- Keep every visual and collision decision reviewable at native iPod scale.
- Make asset selection reproducible through local source files, license texts, manifest recipes, and written provenance.

**Non-Goals:**

- Add the farmhouse, greenhouse, or blacksmith interiors.
- Add player input, farming mechanics, inventory menus, or economic simulation.
- Add scrolling, procedural map generation, runtime image loading, or heap allocation.
- Change PocketStep's public API unless implementation uncovers a separate engine requirement that receives its own change proposal.

## Decisions

### Extend from unused outer edges

South Fields connects to the south edge of South Gate at column six. Herb Garden connects to the east edge of Market Row at row five. Both links are reciprocal and use the current nearest-passable entry rule.

```text
                         Cottage Rise
                              |
River Mill -- Village Green -- Market Row -- Herb Garden
                              |
                         South Gate
                              |
                         South Fields
```

Adding both districts directly to Village Green was considered. The hub already uses all four edges, and another hub-specific transition rule would make the geography harder to read. Extending through outer districts makes the village feel larger without changing the current map grammar.

### Keep the farm broad and the garden enclosed

South Fields uses a two-tile-wide central road aligned with the gate. Crop plots sit to either side with enough open grass for readable movement. A farm building occupies one upper corner, and a fenced chicken area occupies the opposite side. The farmer stands off the main road at a reachable conversation cell.

Herb Garden enters from the west and uses a bent or looping path between small planted beds. A greenhouse or potting shed sits toward the east or northeast. Trellis, bee, and watering details stay outside the route corridor.

Making both screens from a repeated field grid was considered. It would be cheap, but the screens would look interchangeable at 220 by 176. The different negative space and path geometry provide more visual distinction than adding more props.

### Improve the smithy in place

South Gate retains its existing facade, door, and gate collision. A small exterior work area adds an anvil, cooling trough or barrel, stacked fuel, and a deterministic forge animation. Existing Rowan dialogue can gain a smithy-specific pause or exchange.

A separate Forge Yard scene was considered. It would add another map before the farm and garden establish whether the working-village routes feel long enough. The design keeps that westward expansion available for a later change.

### Use a strict asset sourcing ladder

Each asset role follows this order:

1. Reuse an existing emitted asset or crop from a source already in `assets/storyclock`.
2. Import a compatible open asset when it fits the 16-pixel grid, top-down perspective, palette, and redistribution requirements.
3. Generate only the landmark or animation cell that remains missing after a native-scale reuse mockup.

The first implementation pass will evaluate these sources:

| Need | First candidate | License | Expected use |
| --- | --- | --- | --- |
| Ground, road, water, trees, flowers, shrubs, rocks | Existing Story Clock generated outdoor sheet and Kenney Tiny Town tiles | Project-owned and CC0 | Reuse directly through current variants |
| Fence, hay, building pieces, barrels, crates | Existing Luis Zuno RPG Town sheet | CC0 | Crop and palette-match through the manifest |
| Crop rows and growth variation | josehzz Farming Crops 16x16 | CC0 | Import selected mature crop cells |
| Chicken motion and pen details | Proyd Chicken and Pen | CC0 | Import a small walk or peck loop and selected pen pieces |
| Exterior anvil | AntumDeluge Anvil | CC0 | Import the 16-pixel orthographic sprite if it matches the scene |
| Farmer, gardener, blacksmith | Existing Tovin, Mira, and Rowan presentations | Project-owned generated artwork | Reuse with role-specific placement and dialogue |
| Greenhouse or farm facade | Existing Ansimuz building parts first | CC0 | Use a complete front-facing section as the potting shed; generate only if it fails native-scale review |

Broadly importing a large new tileset was considered. It would add many unused files and make the visual style harder to control. The implementation will retain only the original source files needed for license compliance and crop only accepted art into emitted arrays.

### Keep ambient behavior application-owned

Chicken walking or pecking, garden insects, watering, smoke, and sparks use small deterministic state loops derived from the existing global frame counter and itinerary state. Ambient actors do not participate in pathfinding and cannot change collision during a route.

Adding a general PocketStep ambient actor API was considered. Two scenes do not yet establish a stable reusable abstraction. Keeping the loops in Story Clock avoids adding an engine interface around application-specific behavior.

### Add two complete itineraries

The farm itinerary travels through South Gate, enters South Fields, speaks with the farmer, pauses at a farm detail, and returns or reaches a declared ending. The garden itinerary crosses Market Row, enters Herb Garden, speaks with the gardener, interacts with a plant or watering point, and completes its route. Simulator scenario indices force each itinerary for recording.

The new routes share the current director rather than branching inside an itinerary. Selecting a full action script at reset keeps capture runs deterministic and prevents state leaking between districts.

### Review art before accepting collision

The asset compiler produces day, evening, and night arrays. Contact sheets show each new scene at native scale in all three palettes. Collision overlays and route tests use the same authored map dimensions as the compiled plugin. A landmark is accepted only after its rendered footprint and blocked cells agree.

## Risks / Trade-offs

- [Open assets use mismatched palettes or perspective] -> Reject them at the native-scale mockup stage or apply a deterministic palette adjustment. Generate only the remaining gap.
- [Crop plots or garden beds consume the route corridor] -> Reserve required entry, conversation, interaction, and return cells before placing decoration.
- [A moving chicken appears to block Luma without collision] -> Keep its loop inside the fenced pen and away from Luma's route.
- [Small smoke, sparks, or insects flicker rather than animate] -> Hold each frame for multiple ticks and review a recording at simulator speed.
- [New bitmaps increase the plugin size] -> Reuse emitted terrain and props, crop tightly, emit only selected animation frames, and compare the final `.rock` size with the current build.
- [Generated architecture repeats the earlier floating-door problem] -> Generate a complete keyed facade with a threshold and ground contact, then validate it inside the full scene before acceptance.

## Migration Plan

1. Add reciprocal scene records, placeholder maps, and route tests without altering current itinerary behavior.
2. Import and document the selected CC0 sources. Build native-scale farm, garden, and smithy mockups from reused art.
3. Generate a facade only if the documented reuse mockup fails the acceptance gate.
4. Add final collision, entrances, props, NPCs, ambient loops, dialogue, and itineraries one district at a time.
5. Compile assets, run host and simulator tests, record forced farm and garden routes, review all palettes, and build the iPod plugin.

Rollback removes the two scene records and their links, restores the previous itinerary table, and removes only the new emitted assets and local third-party directories. Existing village scenes and PocketStep APIs remain compatible.
