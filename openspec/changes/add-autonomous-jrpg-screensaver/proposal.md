## Why

Clickwheel Works needs a second PocketStep application that exercises top-down
movement and autonomous scene behavior rather than platform physics. A small
JRPG-style screensaver gives the engine a concrete reason to add grid
navigation, interactions, and story sequencing while remaining practical on
the 220 by 176 iPod Color display.

## What Changes

- Add fixed-size tile-grid navigation, pathfinding, and non-solid interaction
  regions to PocketStep without adding heap allocation or a device dependency.
- Add an autonomous action director that advances from completed movement and
  interaction events rather than wall-clock animation offsets.
- Add a new Rockbox screensaver plugin with an original 16-pixel tile set,
  house and outdoor scenes, NPC conversations, item collection, and a looping
  story.
- Present the current time and time-of-day palette as part of the scene while
  requiring no user control beyond exiting the plugin.
- Add deterministic simulator scenarios, visual captures, host tests, and an
  iPod Color build target for the new program.

## Capabilities

### New Capabilities

- `pocketstep-grid-navigation`: Fixed-memory tile maps, four-direction route
  finding, interaction regions, and scene spawn data for small top-down games.
- `autonomous-story-director`: Completion-driven action scripts for walking,
  waiting, facing, dialogue, item collection, and scene changes.
- `jrpg-screensaver`: The observable two-scene autonomous clock vignette,
  including its original artwork, NPCs, dialogue, item state, story loop, and
  simulator review behavior.

### Modified Capabilities

None. The repository has no existing OpenSpec capability baselines.

## Impact

- Extends `pocketstep/` and its host test suite, then republishes that directory
  to the standalone PocketStep repository.
- Adds a Rockbox plugin source file and includes it in simulator and device
  builds alongside Chronolith and Mushroom Clock.
- Adds simulator launch, deterministic capture, and review-sheet support for
  the new program.
- Keeps PocketStep header-only, C99, fixed-memory, and independent of Rockbox.
- Uses original names and pixel art rather than assets or characters from an
  existing JRPG series.
