## Why

Story Clock's cottage exterior works as a single vignette, but the world ends
at the edges of that screen. Expanding it into a small connected village gives
the autonomous character somewhere meaningful to travel and gives PocketStep a
real test of linked maps, reusable scene props, and varied story routes.

## What Changes

- Add Village Green, River Mill, Market Row, and South Gate exterior scenes
  around the existing Cottage Rise scene.
- Connect scene edges at matching tile coordinates so movement preserves the
  character's position and direction across the village.
- Add clear, collision-aware building entrances that can target future indoor
  scenes without implementing those interiors in this change.
- Add reusable fixed-memory scene links and static prop descriptions to
  PocketStep.
- Extend the autonomous story with several deterministic itineraries selected
  per loop instead of one fixed outdoor route.
- Add native-resolution open props and project-owned architecture for distinct
  village landmarks, with day, evening, and night variants.
- Add route, transition, collision, simulator, and device-build verification
  for the expanded village.

## Capabilities

### New Capabilities

- `scene-link-navigation`: Fixed-memory directional links between tile scenes,
  matching edge entry placement, entrance targets, and authored prop metadata.
- `branching-story-itineraries`: Deterministic selection and execution of one
  complete autonomous village route per story loop.
- `village-exterior-world`: The observable five-screen village, its districts,
  landmarks, building entrances, ambient motion, NPC encounters, and palette
  behavior.

### Modified Capabilities

None. The earlier completed changes have not been archived into main capability
baselines, so this change adds self-contained extension capabilities.

## Impact

- Extends PocketStep's optional scene and story helpers while keeping the
  library header-only, C99, allocator-free, and independent of Rockbox.
- Expands `src/storyclock.c`, its generated bitmap header, source-art manifest,
  licensing notes, and host tests.
- Increases the Story Clock plugin size but keeps all maps, routes, and graphics
  compiled into the plugin with no runtime file loading.
- Updates simulator recordings and the iPod Color device artifact.
