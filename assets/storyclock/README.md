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
