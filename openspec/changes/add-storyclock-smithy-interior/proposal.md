## Why

Story Clock has several visible building entrances, but only the opening house has an interior. Reusing that room's furniture would make the village feel artificial, so the smithy should establish a repeatable way to give each interior its own architecture, props, animation, collision, and story purpose at the iPod's native resolution.

## What Changes

- Add an enterable smithy behind the existing South Gate smithy doorway and return Luma to the matching exterior doorway after the visit.
- Give the smithy a distinct sooted-stone and timber interior with a forge, anvil, quench trough, tool storage, work clutter, and readable walkways.
- Add restrained forge ambience such as fire, sparks, bellows motion, or quench steam without compromising frame rate or actor readability.
- Move the South Gate Iron Charm reward into an autonomous conversation with Rowan inside the smithy. Rowan faces Luma while speaking, the reward remains unique per session, and the existing inventory presentation shows it.
- Add a deterministic smithy review route and native-resolution still or recording so entrance placement, draw order, collision, animation, dialogue, inventory state, and return navigation can be checked quickly.
- Audit existing project art and compatible open 16 by 16 art before generating missing smithy assets. Record the audition and keep complete provenance for every accepted source.
- Define reusable interior presentation data so later interiors can choose their own floor, walls, palette treatment, ambient effects, and landmark props without inheriting opening-house assumptions.

## Capabilities

### New Capabilities

- `storyclock-village-interiors`: Defines distinct enterable village interiors, beginning with the South Gate smithy, including navigation, autonomous interaction, item state, rendering, collision, and deterministic review behavior.
- `storyclock-interior-assets`: Defines the reuse-first audition, provenance, deterministic conversion, native-resolution acceptance, and visual-identity rules for interior artwork.

### Modified Capabilities

None. The repository has not yet synced the completed change deltas into main specs, so this change records the new requirements as self-contained capabilities.

## Impact

- `src/storyclock.c`: scene data, smithy itinerary, NPC placement and facing, Iron Charm reward timing, interior drawing, ambient animation, and simulator selection.
- `assets/storyclock/` and `assets/storyclock/assets.json`: candidate notes, accepted source art, provenance, crops, palette conversion, and generated-art prompt records when needed.
- `scripts/generate-storyclock-assets.py` and capture scripts: deterministic asset output and smithy review capture.
- `pocketstep/tests/test_storyclock_world.c`: entrance topology, passability, interaction, unique reward, and return-spawn coverage.
- PocketStep scene presentation structures may gain fixed-memory interior metadata if app-owned data cannot remove the current `SCENE_HOUSE` special cases cleanly. No heap allocation, filesystem access, floating point, or Rockbox-specific dependency will enter PocketStep.
