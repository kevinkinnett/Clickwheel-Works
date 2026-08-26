# Story Clock generated art

RetroDiffusion generated these source PNGs through its HTTP API in August
2026. They are project assets for the original Story Clock world. None of the
prompts names or copies an existing game, character, map, or sprite.

The first asset pass cost $0.59:

- `indoor-tileset.png`: 16-pixel advanced tileset, warm oak floor and dark
  walnut wall, seed 73101.
- `outdoor-tileset.png`: 16-pixel advanced tileset, pale stone path and meadow
  grass, seed 73102.
- `luma-walk.png`: four-direction walking sheet for a traveler in a dark hat
  and rust coat, seed 73103.
- `mira-walk.png`: four-direction walking sheet for an older villager in a
  plum robe, seed 73104.
- `tovin-walk.png`: four-direction walking sheet for a villager in a green
  tunic, seed 73105.
- `objects-sheet.png`: RD Pro collection of the ember key, beacon, cottage
  furniture, plants, trees, and door, seed 73106.

The second pass cost $0.073 after two failed and automatically refunded batch
requests:

- `floor-tile.png`: warm oak single tile, seed 73117.
- `water-tile.png`: calm pond single tile, seed 73118.
- `rug.png`: transparent 48-pixel woven cottage rug, seed 73109.

A later OpenAI built-in image-generation pass produced
`outdoor-generated-v1.png`. The manifest extracts its meadow, path, water,
flower, shrub, rock, and tree artwork, then reduces each asset with
nearest-neighbor sampling for the 16-pixel world grid. PocketStep's `rotate`
operation derives the horizontal path from the vertical source tile.

`room-architecture-generated-v1.png` is the matching indoor architecture pass.
The manifest uses its quieter floorboards, carved wall trim, curtained window,
framed door, and fireplace. Furniture remains sourced from the cottage set,
with small code-drawn shadows to anchor it to the floor.

`village-house-generated-v1.png` is the north-edge cottage facade. Its centered
door and stone threshold align with column six, so Luma visibly exits the room
into the village instead of appearing on open grass. The manifest color-keys
the magenta source, reduces it to 96x72, and emits matching day, evening, and
night versions. `village-house-generated-v1.prompt.txt` retains both the
original generation prompt and the background-key edit prompt.

The expanded village uses five more project-owned sources from OpenAI's
built-in image-generation mode:

- `village-green-generated-v1.png`: the inn and healer facades around the
  Green's center path.
- `river-mill-generated-v1.png`: the mill facade and animated wheel source.
- `mill-wheel-animation-generated-v1.png`: a four-cell wheel strip with a
  stationary rim, axle, and mounting stand while the spokes rotate.
- `market-row-generated-v1.png`: two shops with distinct red and amber
  awnings.
- `south-gate-generated-v1.png`: the smithy wing, open gate arch, and lanterns.
- `village-well-generated-v1.png`: the depth-sorted central well.

`village-exteriors-generated-v1.prompts.txt` records the final prompts and
generation mode. The sources use deliberately flat magenta backdrops. Because
the generator produced small near-magenta variations, the manifest applies a
tolerant color key before nearest-neighbor reduction. The resulting landmarks
remain transparent and readable at the iPod's native 220x176 resolution.
`mill-wheel-animation-generated-v1.prompt.txt` records the later correction
prompt that replaced the mirrored wheel effect.

`assets.json` is the authoritative conversion recipe. Run the general compiler
with Python and Pillow:

```powershell
python scripts/compile-pocketstep-assets.py assets/storyclock/assets.json
```

The older command remains as a compatibility shortcut:

```powershell
python scripts/generate-storyclock-assets.py
```

The manifest downsizes the 48-pixel walk frames to 20 pixels with
nearest-neighbor sampling, extracts selected objects, assembles the open tiles,
produces evening and night variants, records provenance, and writes Rockbox
`RGB565SWAPPED` arrays to `src/storyclock_assets.h`. The compiler validates all
inputs before atomically replacing that header.

## Open asset layer

The outdoor ground, path, flowers, and two-part trees use selected 16-pixel
tiles from Kenney's Tiny Town 1.1 pack. Tiny Town is released under CC0, so it
can be redistributed and used in commercial projects without attribution.
The original `License.txt` is retained alongside the selected source tiles in
`third-party/kenney-tiny-town/`; credit is included as a courtesy.

- `tile_0000.png`: plain grass
- `tile_0001.png`: detailed grass
- `tile_0002.png`: flowers
- `tile_0004.png` and `tile_0016.png`: tree crown and trunk
- `tile_0043.png`: grass-edged stone path

The facade is flanked by a fence, crate, and barrel from Luis Zuno's RPG Town
Pixel Art Assets. The pack is released under CC0. Its original transparent
atlas and `LICENSE.txt` are retained in `third-party/ansimuz-rpg-town/`; the
manifest crops the native 16-pixel props and emits palette-matched variants.

The indoor bed, bookcase, hanging plant, mirror, lamp, writing desk, and stool
come from Pixelandbeans' Cottage Core Bedroom Furniture Set. The source is
licensed CC BY 4.0 and is retained with its attribution and license details in
`third-party/pixelandbeans-cottage-core/`. The manifest crops and scales those
objects for the iPod's 220x176 display.

## Working village asset review

The farm, garden, and smithy expansion uses a reuse-first selection pass. The
native 220x176 mockups are local review products under
`artifacts/asset-review/working-village/` and are not shipped in the plugin.

| Need | Candidates reviewed | Decision | Native-scale reason |
| --- | --- | --- | --- |
| Farm ground, road, trees, flowers, and rocks | Existing generated outdoor sheet and Kenney Tiny Town | Accepted existing art | The shapes already match the village palette and 16-pixel grid. |
| Farm building | Existing generated cottage and Luis Zuno RPG Town building | Accepted existing cottage | The 96x72 cottage has a grounded threshold and leaves an open central road. |
| Crop plots | Existing flowers and shrubs, josehzz Farming Crops 16x16 | Accepted CC0 crop sheet | Mature wheat and corn remain distinct at 16x16 after black color-key removal. |
| Chicken activity | Existing character sheets, Proyd Chicken and Pen | Accepted CC0 chicken sheet | Two native brown-chicken frames form a readable walk or peck loop without resizing. |
| Chicken pen | Existing Luis Zuno fence and Proyd pen pieces | Accepted an eight-cell Proyd perimeter | The eight authored corner and rail cells form a 48x48 boundary with a transparent center. The unused ninth sprite-sheet cell contains an interior divider, and rotating the village picket fence made the side rails read as ladders. |
| Garden beds | Existing flowers and josehzz crop sheet | Accepted both reusable sources | Repeated blooms and leafy crops distinguish the beds without another terrain atlas. |
| Garden building | Existing cottage and Luis Zuno building sections | Accepted a complete Luis Zuno front as a potting shed | The 80x80 crop has a centered door, chimney, and visible ground contact without a clipped side wall. |
| Smithy tools | Existing barrel and facade anvil, AntumDeluge Anvil | Accepted existing barrel and CC0 anvil | The 16-pixel anvil reads beside the existing smithy after a deterministic palette lift. |
| New generated landmark | Farmhouse or greenhouse generation | Rejected for this pass | The reusable cottage and potting shed remain clear in all three native mockups. |

Three additional CC0 sources are retained with local licensing notes:

- `third-party/josehzz-farming-crops/` contains the original ZIP, extracted
  spritesheet, creator README, and license note.
- `third-party/proyd-chicken-pen/` contains the original chicken and pen sheet
  and license note.
- `third-party/antumdeluge-anvil/` contains the original 16-pixel anvil and
  license note.

The complete CC0 1.0 legal text is retained once at `LICENSES/CC0-1.0.txt`.

## Satchel and keepsake art

The inventory pass audited the existing Story Clock manifest and retained
sources before generating anything new. The Ember Key reuses `story_key`; the
Brass Cog comes from the generated mill wheel; the Iron Charm uses the CC0
AntumDeluge anvil; and the Seed Pouch and Mint Sprig use native cells from the
CC0 josehzz farming sheet. Their original licenses and attribution remain in
the third-party directories listed above.

Only two gaps required new art. OpenAI's built-in image-generation mode made
`inventory-satchel-generated-v1.png` and
`inventory-market-token-generated-v1.png`. Their complete prompts are retained
beside them as `.prompt.txt` files. The manifest crops each source, reduces it
with nearest-neighbor sampling, and emits a deterministic 16 by 16 icon. No
external paid generation API was used for this pass.
