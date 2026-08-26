## Why

Story Clock's five exterior scenes establish a village, but most routes still pass through civic spaces rather than places where villagers live and work. Adding farmland, a garden, and visible smithy activity will give autonomous journeys more geographic variety and make South Gate and Market Row lead somewhere meaningful.

## What Changes

- Add South Fields below South Gate, with an open farm road, crop plots, a farmhouse or barn entrance, a chicken pen, a farmer encounter, and restrained farm activity.
- Add Herb Garden east of Market Row, with shaped garden beds, a winding route, a greenhouse or potting-shed entrance, a gardener encounter, and small insect or watering animation.
- Make the existing South Gate smithy readable as a working blacksmith through exterior props and ambient forge activity. Keep its interior deferred.
- Add complete autonomous itineraries for the farm and garden, including reciprocal scene transitions and deterministic simulator scenarios.
- Source art in a fixed order. Reuse project assets first, then compatible open artwork, then generate only the pieces still missing. Record source, license, attribution, edits, and generation details in the manifest and asset notes.
- Preserve uncluttered collision corridors and day, evening, and night readability on the 220 by 176 iPod Color display.

## Capabilities

### New Capabilities

- `working-village-districts`: South Fields, Herb Garden, smithy activity, their connected geography, autonomous visits, ambient motion, collision behavior, and time-of-day presentation.
- `storyclock-asset-sourcing`: The reuse-first art selection process, license and provenance records, native-resolution review, and the conditions under which generated art may fill a gap.

### Modified Capabilities

None. The earlier village and PocketStep changes remain unarchived, so this change builds on their implementation without declaring changes to main capability baselines.

## Impact

- Expands the Story Clock scene tables, collision maps, links, entrances, props, NPC state, ambient drawing, dialogue, and itinerary scripts in `src/storyclock.c`.
- Adds selected open-source files and, only if needed, generated source art under `assets/storyclock/`, with compiler recipes and provenance updates in `assets/storyclock/assets.json`.
- Extends host route and world tests, simulator scenario selection, capture scripts, contact sheets, and the Rockbox device build.
- Reuses the current PocketStep scene-link, prop, story, sprite, and draw-order helpers without changing their public APIs.
