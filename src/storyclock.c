/***************************************************************************
 * Story Clock
 * An autonomous, original top-down story vignette for Rockbox.
 *
 * Copyright (C) 2026 Kevin and OpenAI
 * SPDX-License-Identifier: GPL-2.0-or-later
 ****************************************************************************/

#include "plugin.h"
#include "lib/helper.h"
#include "lib/pluginlib_actions.h"
#include "lib/pluginlib_exit.h"

#define POCKETSTEP_IMPLEMENTATION
#include "pocketstep.h"
#define POCKETSTEP_GRID_IMPLEMENTATION
#include "pocketstep_grid.h"
#define POCKETSTEP_STORY_IMPLEMENTATION
#include "pocketstep_story.h"
#define POCKETSTEP_DRAW_IMPLEMENTATION
#include "pocketstep_draw.h"
#define POCKETSTEP_ANIM_IMPLEMENTATION
#include "pocketstep_anim.h"
#define POCKETSTEP_SCENE_IMPLEMENTATION
#include "pocketstep_scene.h"
#include "storyclock_assets.h"

#if !defined(HAVE_LCD_COLOR)
#error Story Clock requires a color display
#endif

#define FRAME_TICKS MAX(1, HZ / 20)
#define MAP_WIDTH 13
#define MAP_HEIGHT 11
#define TILE_SIZE 16
#define MAP_GUTTER 6
#define ROUTE_CAPACITY (MAP_WIDTH * MAP_HEIGHT)
#define DRAWABLE_CAPACITY 48
#define SCENE_HOUSE 0
#define SCENE_COTTAGE 1
#define SCENE_GREEN 2
#define SCENE_MILL 3
#define SCENE_MARKET 4
#define SCENE_GATE 5
#define SCENE_FIELDS 6
#define SCENE_GARDEN 7
#define ITEM_KEY 1
#define REGION_ITEM 10
#define REGION_INDOOR_NPC 11
#define REGION_OUTDOOR_NPC 12
#define REGION_BEACON 13
#define REGION_GREEN_INN 20
#define REGION_GREEN_HEALER 21
#define REGION_MILL_DOOR 22
#define REGION_MARKET_SHOP 23
#define REGION_MARKET_INN 24
#define REGION_GATE_SMITH 25
#define REGION_FIELDS_HOUSE 26
#define REGION_GARDEN_SHED 27
#define REGION_GREEN_NPC 30
#define REGION_MILL_NPC 31
#define REGION_MARKET_NPC 32
#define REGION_GATE_NPC 33
#define REGION_FIELDS_NPC 34
#define REGION_GARDEN_NPC 35
#define DRAW_ACTOR 1
#define DRAW_NPC 2
#define DRAW_ITEM 3
#define DRAW_TALL 4
#define DRAW_GATE_FOREGROUND 5
#define OUTDOOR_PROP_CRATE 100
#define OUTDOOR_PROP_BARREL 101
#define OUTDOOR_PROP_WELL 102
#define OUTDOOR_PROP_ANVIL 103
#define PALETTE_AUTO 0
#define PALETTE_DAY 1
#define PALETTE_EVENING 2
#define PALETTE_NIGHT 3

enum tile_kind
{
    TILE_FLOOR,
    TILE_WALL,
    TILE_RUG,
    TILE_BED,
    TILE_TABLE,
    TILE_GRASS,
    TILE_PATH,
    TILE_WATER,
    TILE_TREE,
    TILE_FLOWER,
    TILE_DOOR,
    TILE_BEACON,
    TILE_SHRUB,
    TILE_ROCK,
    TILE_CROP_WHEAT,
    TILE_CROP_CORN,
    TILE_GARDEN_BLOOM,
    TILE_GARDEN_HERB
};

struct palette
{
    unsigned int ground;
    unsigned int ground_alt;
    unsigned int dark;
    unsigned int mid;
    unsigned int light;
    unsigned int accent;
    unsigned int water;
    unsigned int text;
    unsigned int box;
};

struct story_state
{
    struct ps_body actor;
    struct ps_grid_workspace workspace;
    struct ps_grid_cell route_cells[ROUTE_CAPACITY];
    struct ps_grid_path route;
    int route_index;
    int route_active;
    int active_action;
    int scene;
    int facing;
    int walk_distance;
    int item_collected;
    int dialogue_frames;
    int dialogue_speaker;
    int npc_facing[8];
    uint32_t loop_index;
    int initialized;
    int itinerary_index;
    const char *dialogue_text;
    int failure;
};

static const unsigned char house_tiles[MAP_WIDTH * MAP_HEIGHT] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,2,2,2,0,0,0,0,1,
    1,0,0,0,0,2,2,2,0,0,0,0,1,
    1,0,0,0,0,2,2,2,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,10,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const unsigned char house_blocked[MAP_WIDTH * MAP_HEIGHT] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,0,0,0,0,0,0,1,1,1,
    1,1,1,1,0,0,0,0,0,0,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,1,0,0,0,0,0,0,1,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,1,1,
    1,1,1,0,0,0,0,0,0,1,1,1,1,
    1,1,1,0,0,0,0,0,0,1,1,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const unsigned char cottage_tiles[MAP_WIDTH * MAP_HEIGHT] = {
    8,8,5,5,5,5,5,5,5,5,8,8,8,
    8,5,12,5,5,5,6,5,5,5,5,5,8,
    5,5,5,9,5,5,6,5,5,5,11,5,5,
    5,5,5,5,5,5,6,5,5,5,13,5,5,
    5,5,12,5,5,5,6,5,5,5,5,5,5,
    8,5,5,5,5,6,6,6,6,6,5,5,8,
    8,5,5,13,5,5,6,5,5,5,5,5,8,
    5,5,7,7,7,5,6,5,5,9,5,5,5,
    5,5,7,7,7,5,6,5,5,5,12,5,5,
    8,5,7,7,7,5,10,5,5,5,5,5,8,
    8,8,5,5,5,5,5,5,5,5,8,8,8
};

static const unsigned char cottage_blocked[MAP_WIDTH * MAP_HEIGHT] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,1,1,1,1,1,1,0,0,0,1,
    1,0,0,1,1,1,1,1,1,0,1,0,1,
    1,1,1,1,1,1,1,1,1,0,1,1,1,
    1,0,0,1,1,1,0,1,1,0,1,1,1,
    1,0,0,1,0,0,0,0,0,0,0,0,1,
    1,0,0,1,0,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const unsigned char green_tiles[MAP_WIDTH * MAP_HEIGHT] = {
    8,8,5,5,5,5,6,5,5,5,5,8,8,
    8,5,5,5,5,5,6,5,5,5,5,5,8,
    5,5,12,5,5,5,6,5,5,12,5,5,5,
    5,5,5,5,5,5,6,5,5,5,5,5,5,
    5,5,5,5,5,5,6,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,
    5,5,5,5,13,5,6,5,5,5,5,5,5,
    5,5,8,5,5,5,6,5,5,5,8,5,5,
    5,5,5,9,5,5,6,5,5,9,5,5,5,
    8,5,5,5,5,5,6,5,5,5,5,5,8,
    8,8,5,5,5,5,6,5,5,5,5,8,8
};

static const unsigned char green_blocked[MAP_WIDTH * MAP_HEIGHT] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,0,1,1,1,1,1,1,
    1,1,1,1,1,1,0,1,1,1,1,1,1,
    1,1,1,1,1,1,0,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,0,0,0,1,0,0,0,0,0,0,0,1,
    1,0,1,0,0,0,0,0,0,0,1,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,0,1,1,1,1,1,1
};

static const unsigned char mill_tiles[MAP_WIDTH * MAP_HEIGHT] = {
    8,8,7,7,7,5,5,5,5,5,5,5,8,
    8,5,7,7,7,5,5,5,5,5,5,5,8,
    5,5,7,7,7,5,5,5,5,5,5,5,5,
    5,5,7,7,7,5,5,5,5,5,5,5,5,
    5,5,7,7,7,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,
    5,5,7,7,7,5,5,5,5,12,5,5,5,
    5,5,7,7,7,5,5,9,5,5,5,5,5,
    5,5,7,7,7,5,5,5,5,5,5,5,5,
    8,5,7,7,7,5,5,5,5,5,5,5,8,
    8,8,7,7,7,5,5,5,5,5,5,8,8
};

static const unsigned char mill_blocked[MAP_WIDTH * MAP_HEIGHT] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,1,1,1,0,0,1,1,1,1,1,1,
    1,0,1,1,1,0,0,1,1,1,1,1,1,
    1,0,1,1,1,0,0,1,1,1,1,1,1,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const unsigned char market_tiles[MAP_WIDTH * MAP_HEIGHT] = {
    8,5,5,5,5,5,5,5,5,5,5,5,8,
    5,5,5,5,5,5,5,5,5,5,5,5,5,
    5,5,5,5,5,5,5,5,5,5,5,5,5,
    5,5,5,5,5,5,5,5,5,5,5,5,5,
    5,5,5,6,5,5,5,5,5,6,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,
    5,5,5,5,5,5,5,5,5,5,5,5,5,
    5,12,5,5,13,5,5,5,12,5,5,9,5,
    5,5,5,5,5,5,5,5,5,5,5,5,5,
    8,5,5,5,5,5,5,5,5,5,5,5,8,
    8,8,5,5,5,5,5,5,5,5,5,8,8
};

static const unsigned char market_blocked[MAP_WIDTH * MAP_HEIGHT] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,0,1,1,1,1,1,1,
    1,1,1,1,1,1,0,1,1,1,1,1,1,
    1,1,1,1,1,1,0,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,0,0,1,0,0,0,1,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const unsigned char gate_tiles[MAP_WIDTH * MAP_HEIGHT] = {
    8,8,5,5,5,5,6,5,5,5,5,8,8,
    8,5,5,5,5,5,6,5,5,8,5,8,8,
    5,5,5,5,5,5,6,5,5,5,5,5,5,
    5,5,5,5,5,5,6,5,5,8,5,8,5,
    5,5,5,5,5,5,6,5,5,5,5,5,5,
    5,5,5,5,5,5,6,5,5,8,5,8,5,
    5,5,12,5,5,5,6,5,5,5,5,5,5,
    5,5,5,5,5,5,6,5,5,8,5,8,5,
    8,5,5,5,5,5,6,5,5,5,5,5,8,
    8,5,5,5,5,5,6,5,5,5,5,5,8,
    8,8,8,8,8,8,6,8,8,8,8,8,8
};

static const unsigned char gate_blocked[MAP_WIDTH * MAP_HEIGHT] = {
    1,1,1,1,1,1,0,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,1,0,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,1,0,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,1,0,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,0,0,0,1,1,1,1,1,
    1,1,1,1,1,0,0,0,1,1,1,1,1,
    1,1,1,1,1,0,0,0,1,1,1,1,1,
    1,1,1,1,1,1,0,1,1,1,1,1,1
};

static const unsigned char fields_tiles[MAP_WIDTH * MAP_HEIGHT] = {
    5,5,5,5,5,5,6,5,5,5,5,8,8,
    5,5,5,5,5,5,6,5,5,5,5,5,8,
    5,5,5,5,5,5,6,5,5,5,5,5,5,
    5,5,5,5,5,5,6,5,5,5,5,5,5,
    5,5,5,10,6,6,6,5,5,5,5,5,5,
    5,12,5,5,5,5,6,6,5,5,5,5,5,
    5,14,14,14,5,5,6,5,5,5,5,5,5,
    5,14,14,14,5,5,6,6,6,5,5,5,5,
    5,15,15,15,5,5,6,5,5,5,5,5,5,
    5,15,15,15,5,5,6,5,5,5,5,5,5,
    8,8,5,5,5,5,6,5,5,5,5,8,8
};

static const unsigned char fields_blocked[MAP_WIDTH * MAP_HEIGHT] = {
    1,1,1,1,1,1,0,1,1,1,1,1,1,
    1,1,1,1,1,1,0,0,0,0,0,1,1,
    1,1,1,1,1,1,0,0,0,0,0,0,1,
    1,1,1,1,1,1,0,0,0,0,0,0,1,
    1,1,1,0,0,0,0,0,0,0,0,0,1,
    1,1,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,0,0,0,0,0,1,1,1,1,
    1,1,1,1,0,0,0,0,0,1,0,0,1,
    1,1,1,1,0,0,0,0,0,1,0,0,1,
    1,1,1,1,0,0,0,0,0,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const unsigned char garden_tiles[MAP_WIDTH * MAP_HEIGHT] = {
    8,8,5,5,5,5,5,5,5,5,5,5,5,
    5,16,16,16,16,5,5,5,5,5,5,5,5,
    5,16,16,16,16,5,5,5,5,5,5,5,5,
    5,5,5,5,5,5,6,5,5,5,5,5,5,
    5,5,5,5,5,5,6,5,5,5,10,5,5,
    6,6,6,6,6,6,6,6,6,6,6,5,5,
    5,5,5,5,5,5,6,5,5,5,5,5,5,
    5,5,17,17,17,17,6,5,5,5,5,5,5,
    5,5,17,17,17,17,6,5,5,5,5,5,5,
    5,12,5,5,9,5,6,5,5,12,5,5,8,
    8,8,5,5,5,5,5,5,5,5,5,8,8
};

static const unsigned char garden_blocked[MAP_WIDTH * MAP_HEIGHT] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,0,0,0,1,1,1,1,1,
    1,1,1,1,1,0,0,0,1,1,1,1,1,
    1,0,0,0,0,0,0,0,1,1,1,1,1,
    1,0,0,0,0,0,0,0,1,1,0,1,1,
    0,0,0,0,0,0,0,0,0,0,0,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,1,1,1,1,0,0,0,0,0,0,1,
    1,0,1,1,1,1,0,0,0,0,0,0,1,
    1,1,0,0,0,0,0,0,0,1,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const struct ps_region house_regions[] = {
    { { 9 * TILE_SIZE + MAP_GUTTER, 4 * TILE_SIZE, 16, 16 }, REGION_ITEM },
    { { 2 * TILE_SIZE + MAP_GUTTER, 4 * TILE_SIZE, 16, 16 }, REGION_INDOOR_NPC }
};

static const struct ps_region cottage_regions[] = {
    { { 3 * TILE_SIZE + MAP_GUTTER, 5 * TILE_SIZE, 16, 16 }, REGION_OUTDOOR_NPC },
    { { 10 * TILE_SIZE + MAP_GUTTER, 2 * TILE_SIZE, 16, 16 }, REGION_BEACON }
};

static const struct ps_region green_regions[] = {
    { { 3 * TILE_SIZE + MAP_GUTTER, 3 * TILE_SIZE, 16, 16 }, REGION_GREEN_INN },
    { { 9 * TILE_SIZE + MAP_GUTTER, 3 * TILE_SIZE, 16, 16 }, REGION_GREEN_HEALER },
    { { 7 * TILE_SIZE + MAP_GUTTER, 6 * TILE_SIZE, 16, 16 }, REGION_GREEN_NPC }
};

static const struct ps_region mill_regions[] = {
    { { 9 * TILE_SIZE + MAP_GUTTER, 3 * TILE_SIZE, 16, 16 }, REGION_MILL_DOOR },
    { { 6 * TILE_SIZE + MAP_GUTTER, 6 * TILE_SIZE, 16, 16 }, REGION_MILL_NPC }
};

static const struct ps_region market_regions[] = {
    { { 3 * TILE_SIZE + MAP_GUTTER, 3 * TILE_SIZE, 16, 16 }, REGION_MARKET_SHOP },
    { { 9 * TILE_SIZE + MAP_GUTTER, 3 * TILE_SIZE, 16, 16 }, REGION_MARKET_INN },
    { { 8 * TILE_SIZE + MAP_GUTTER, 6 * TILE_SIZE, 16, 16 }, REGION_MARKET_NPC }
};

static const struct ps_region gate_regions[] = {
    { { 2 * TILE_SIZE + MAP_GUTTER, 7 * TILE_SIZE, 16, 16 }, REGION_GATE_SMITH },
    { { 7 * TILE_SIZE + MAP_GUTTER, 6 * TILE_SIZE, 16, 16 }, REGION_GATE_NPC }
};

static const struct ps_region fields_regions[] = {
    { { 3 * TILE_SIZE + MAP_GUTTER, 4 * TILE_SIZE, 16, 16 }, REGION_FIELDS_HOUSE },
    { { 8 * TILE_SIZE + MAP_GUTTER, 5 * TILE_SIZE, 16, 16 }, REGION_FIELDS_NPC }
};

static const struct ps_region garden_regions[] = {
    { { 10 * TILE_SIZE + MAP_GUTTER, 4 * TILE_SIZE, 16, 16 }, REGION_GARDEN_SHED },
    { { 7 * TILE_SIZE + MAP_GUTTER, 7 * TILE_SIZE, 16, 16 }, REGION_GARDEN_NPC }
};

static const struct ps_scene scenes[] = {
    { MAP_WIDTH, MAP_HEIGHT, house_tiles,
      { MAP_WIDTH, MAP_HEIGHT, house_blocked },
      house_regions, ARRAYLEN(house_regions), { 6, 7 }, 117 },
    { MAP_WIDTH, MAP_HEIGHT, cottage_tiles,
      { MAP_WIDTH, MAP_HEIGHT, cottage_blocked },
      cottage_regions, ARRAYLEN(cottage_regions), { 6, 4 }, 73102 },
    { MAP_WIDTH, MAP_HEIGHT, green_tiles,
      { MAP_WIDTH, MAP_HEIGHT, green_blocked },
      green_regions, ARRAYLEN(green_regions), { 6, 1 }, 73103 },
    { MAP_WIDTH, MAP_HEIGHT, mill_tiles,
      { MAP_WIDTH, MAP_HEIGHT, mill_blocked },
      mill_regions, ARRAYLEN(mill_regions), { 11, 5 }, 73104 },
    { MAP_WIDTH, MAP_HEIGHT, market_tiles,
      { MAP_WIDTH, MAP_HEIGHT, market_blocked },
      market_regions, ARRAYLEN(market_regions), { 1, 5 }, 73105 },
    { MAP_WIDTH, MAP_HEIGHT, gate_tiles,
      { MAP_WIDTH, MAP_HEIGHT, gate_blocked },
      gate_regions, ARRAYLEN(gate_regions), { 6, 1 }, 73106 },
    { MAP_WIDTH, MAP_HEIGHT, fields_tiles,
      { MAP_WIDTH, MAP_HEIGHT, fields_blocked },
      fields_regions, ARRAYLEN(fields_regions), { 6, 1 }, 73107 },
    { MAP_WIDTH, MAP_HEIGHT, garden_tiles,
      { MAP_WIDTH, MAP_HEIGHT, garden_blocked },
      garden_regions, ARRAYLEN(garden_regions), { 1, 5 }, 73108 }
};

static const struct ps_scene_link village_links[] = {
    { SCENE_COTTAGE, PS_GRID_DOWN, SCENE_GREEN, 6 },
    { SCENE_GREEN, PS_GRID_UP, SCENE_COTTAGE, 6 },
    { SCENE_GREEN, PS_GRID_LEFT, SCENE_MILL, 5 },
    { SCENE_MILL, PS_GRID_RIGHT, SCENE_GREEN, 5 },
    { SCENE_GREEN, PS_GRID_RIGHT, SCENE_MARKET, 5 },
    { SCENE_MARKET, PS_GRID_LEFT, SCENE_GREEN, 5 },
    { SCENE_GREEN, PS_GRID_DOWN, SCENE_GATE, 6 },
    { SCENE_GATE, PS_GRID_UP, SCENE_GREEN, 6 },
    { SCENE_GATE, PS_GRID_DOWN, SCENE_FIELDS, 6 },
    { SCENE_FIELDS, PS_GRID_UP, SCENE_GATE, 6 },
    { SCENE_MARKET, PS_GRID_RIGHT, SCENE_GARDEN, 5 },
    { SCENE_GARDEN, PS_GRID_LEFT, SCENE_MARKET, 5 }
};

static const struct ps_scene_entrance village_entrances[] = {
    { SCENE_GREEN, REGION_GREEN_INN, PS_SCENE_NO_DESTINATION,
      { 0, 0 }, PS_GRID_UP },
    { SCENE_GREEN, REGION_GREEN_HEALER, PS_SCENE_NO_DESTINATION,
      { 0, 0 }, PS_GRID_UP },
    { SCENE_MILL, REGION_MILL_DOOR, PS_SCENE_NO_DESTINATION,
      { 0, 0 }, PS_GRID_UP },
    { SCENE_MARKET, REGION_MARKET_SHOP, PS_SCENE_NO_DESTINATION,
      { 0, 0 }, PS_GRID_UP },
    { SCENE_MARKET, REGION_MARKET_INN, PS_SCENE_NO_DESTINATION,
      { 0, 0 }, PS_GRID_UP },
    { SCENE_GATE, REGION_GATE_SMITH, PS_SCENE_NO_DESTINATION,
      { 0, 0 }, PS_GRID_UP },
    { SCENE_FIELDS, REGION_FIELDS_HOUSE, PS_SCENE_NO_DESTINATION,
      { 0, 0 }, PS_GRID_UP },
    { SCENE_GARDEN, REGION_GARDEN_SHED, PS_SCENE_NO_DESTINATION,
      { 0, 0 }, PS_GRID_UP }
};

static const struct ps_scene_prop village_props[] = {
    { SCENE_COTTAGE, 10, 4, 5 * TILE_SIZE - 1,
      OUTDOOR_PROP_CRATE, PS_SCENE_PROP_SOLID },
    { SCENE_COTTAGE, 11, 4, 5 * TILE_SIZE - 1,
      OUTDOOR_PROP_BARREL, PS_SCENE_PROP_SOLID },
    { SCENE_GREEN, 4, 6, 7 * TILE_SIZE - 1,
      OUTDOOR_PROP_WELL, PS_SCENE_PROP_SOLID },
    { SCENE_GATE, 2, 8, 9 * TILE_SIZE - 1,
      OUTDOOR_PROP_ANVIL, PS_SCENE_PROP_SOLID },
    { SCENE_GATE, 3, 8, 9 * TILE_SIZE - 1,
      OUTDOOR_PROP_BARREL, PS_SCENE_PROP_SOLID },
    { SCENE_FIELDS, 1, 5, 6 * TILE_SIZE - 1,
      OUTDOOR_PROP_CRATE, PS_SCENE_PROP_SOLID }
};

static const struct ps_anim_sheet actor_animation = {
    STORY_WALK_SHEET_WIDTH, STORY_WALK_SHEET_HEIGHT,
    STORY_WALK_FRAME_SIZE, STORY_WALK_FRAME_SIZE,
    4, 0, { 0, 1, 2, 3 }
};

static const struct ps_anim_sheet npc_animation = {
    STORY_WALK_SHEET_WIDTH, STORY_WALK_SHEET_HEIGHT,
    STORY_WALK_FRAME_SIZE, STORY_WALK_FRAME_SIZE,
    4, 0, { 0, 1, 2, 3 }
};

static const struct palette indoor_palette = {
    LCD_RGBPACK(91, 69, 68), LCD_RGBPACK(117, 84, 69), LCD_RGBPACK(34, 31, 44),
    LCD_RGBPACK(121, 93, 85), LCD_RGBPACK(220, 190, 137), LCD_RGBPACK(240, 157, 81),
    LCD_RGBPACK(57, 101, 113), LCD_RGBPACK(248, 232, 188), LCD_RGBPACK(30, 27, 39)
};
static const struct palette day_palette = {
    LCD_RGBPACK(91, 158, 78), LCD_RGBPACK(112, 178, 88), LCD_RGBPACK(34, 55, 48),
    LCD_RGBPACK(76, 114, 72), LCD_RGBPACK(215, 225, 151), LCD_RGBPACK(245, 184, 70),
    LCD_RGBPACK(56, 134, 171), LCD_RGBPACK(250, 242, 197), LCD_RGBPACK(28, 42, 44)
};
static const struct palette evening_palette = {
    LCD_RGBPACK(117, 91, 91), LCD_RGBPACK(142, 105, 94), LCD_RGBPACK(49, 37, 59),
    LCD_RGBPACK(99, 75, 82), LCD_RGBPACK(231, 177, 126), LCD_RGBPACK(247, 126, 75),
    LCD_RGBPACK(72, 91, 137), LCD_RGBPACK(255, 224, 177), LCD_RGBPACK(42, 30, 51)
};
static const struct palette night_palette = {
    LCD_RGBPACK(34, 62, 75), LCD_RGBPACK(42, 75, 84), LCD_RGBPACK(16, 25, 43),
    LCD_RGBPACK(46, 77, 91), LCD_RGBPACK(153, 190, 176), LCD_RGBPACK(234, 201, 103),
    LCD_RGBPACK(34, 66, 111), LCD_RGBPACK(210, 228, 199), LCD_RGBPACK(13, 20, 36)
};

#define STORY_OPENING \
    { PS_STORY_ACTION_WAIT, 24, 0, 0, NULL }, \
    { PS_STORY_ACTION_SAY, 0, 0, 0, "Morning already? The ember clock is chiming." }, \
    { PS_STORY_ACTION_WALK, 8, 4, 0, NULL }, \
    { PS_STORY_ACTION_FACE, PS_GRID_RIGHT, 0, 0, NULL }, \
    { PS_STORY_ACTION_COLLECT, ITEM_KEY, REGION_ITEM, 0, NULL }, \
    { PS_STORY_ACTION_SAY, 0, 0, 0, "The ember key is warm. Mira will know why." }, \
    { PS_STORY_ACTION_WALK, 3, 4, 0, NULL }, \
    { PS_STORY_ACTION_FACE, PS_GRID_LEFT, 0, 0, NULL }, \
    { PS_STORY_ACTION_SAY, 1, 0, 0, "Mira: Take it to the hill beacon before the light fades." }, \
    { PS_STORY_ACTION_WALK, 6, 9, 0, NULL }, \
    { PS_STORY_ACTION_SCENE, SCENE_COTTAGE, 6, 4, NULL }, \
    { PS_STORY_ACTION_FACE, PS_GRID_DOWN, 0, 0, NULL }, \
    { PS_STORY_ACTION_WAIT, 18, 0, 0, NULL }, \
    { PS_STORY_ACTION_WALK, 4, 5, 0, NULL }, \
    { PS_STORY_ACTION_FACE, PS_GRID_LEFT, 0, 0, NULL }, \
    { PS_STORY_ACTION_SAY, 2, 0, 0, "Tovin: The river keeps old songs. The beacon keeps promises." }, \
    { PS_STORY_ACTION_WALK, 9, 2, 0, NULL }, \
    { PS_STORY_ACTION_FACE, PS_GRID_RIGHT, 0, 0, NULL }, \
    { PS_STORY_ACTION_SAY, 0, 0, 0, "The key turns. A small new star joins the evening sky." }, \
    { PS_STORY_ACTION_WALK, 6, 9, 0, NULL }, \
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_DOWN, 0, 0, NULL }, \
    { PS_STORY_ACTION_WALK, 6, 5, 0, NULL }, \
    { PS_STORY_ACTION_FACE, PS_GRID_RIGHT, 0, 0, NULL }, \
    { PS_STORY_ACTION_SAY, 3, 0, 0, "Eda: The hill light is awake. The whole green can feel it." }

static const struct ps_story_action market_actions[] = {
    STORY_OPENING,
    { PS_STORY_ACTION_WALK, 11, 5, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_RIGHT, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 8, 5, 0, NULL },
    { PS_STORY_ACTION_FACE, PS_GRID_DOWN, 0, 0, NULL },
    { PS_STORY_ACTION_SAY, 5, 0, 0, "Sera: Fresh bread, lamp oil, and one story nobody believes." },
    { PS_STORY_ACTION_WAIT, 28, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 1, 5, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_LEFT, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 6, 1, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_UP, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 6, 4, 0, NULL },
    { PS_STORY_ACTION_WAIT, 45, 0, 0, NULL },
    { PS_STORY_ACTION_END, 0, 0, 0, NULL }
};

static const struct ps_story_action mill_actions[] = {
    STORY_OPENING,
    { PS_STORY_ACTION_WALK, 1, 5, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_LEFT, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 6, 5, 0, NULL },
    { PS_STORY_ACTION_FACE, PS_GRID_DOWN, 0, 0, NULL },
    { PS_STORY_ACTION_SAY, 4, 0, 0, "Bran: The wheel missed one beat, then caught the river again." },
    { PS_STORY_ACTION_WAIT, 28, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 11, 5, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_RIGHT, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 6, 1, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_UP, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 6, 4, 0, NULL },
    { PS_STORY_ACTION_WAIT, 45, 0, 0, NULL },
    { PS_STORY_ACTION_END, 0, 0, 0, NULL }
};

static const struct ps_story_action gate_actions[] = {
    STORY_OPENING,
    { PS_STORY_ACTION_WALK, 6, 9, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_DOWN, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 6, 6, 0, NULL },
    { PS_STORY_ACTION_FACE, PS_GRID_RIGHT, 0, 0, NULL },
    { PS_STORY_ACTION_SAY, 6, 0, 0, "Rowan: The south lanterns are lit. The road can find its way home." },
    { PS_STORY_ACTION_WALK, 6, 9, 0, NULL },
    { PS_STORY_ACTION_WAIT, 28, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 6, 1, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_UP, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 6, 1, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_UP, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 6, 4, 0, NULL },
    { PS_STORY_ACTION_WAIT, 45, 0, 0, NULL },
    { PS_STORY_ACTION_END, 0, 0, 0, NULL }
};

static const struct ps_story_action farm_actions[] = {
    STORY_OPENING,
    { PS_STORY_ACTION_WALK, 6, 9, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_DOWN, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 6, 6, 0, NULL },
    { PS_STORY_ACTION_FACE, PS_GRID_LEFT, 0, 0, NULL },
    { PS_STORY_ACTION_SAY, 6, 0, 0, "Rowan: The forge is awake. Orin's gate hinge will be ready by dusk." },
    { PS_STORY_ACTION_WALK, 6, 9, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_DOWN, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 7, 5, 0, NULL },
    { PS_STORY_ACTION_FACE, PS_GRID_RIGHT, 0, 0, NULL },
    { PS_STORY_ACTION_SAY, 7, 0, 0, "Orin: Wheat for the mill, corn for winter, and one hen with other plans." },
    { PS_STORY_ACTION_WALK, 8, 7, 0, NULL },
    { PS_STORY_ACTION_FACE, PS_GRID_RIGHT, 0, 0, NULL },
    { PS_STORY_ACTION_SAY, 0, 0, 0, "She pauses at the fence, then decides the crops are safer company." },
    { PS_STORY_ACTION_WAIT, 28, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 6, 1, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_UP, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 6, 1, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_UP, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 6, 1, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_UP, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 6, 4, 0, NULL },
    { PS_STORY_ACTION_WAIT, 45, 0, 0, NULL },
    { PS_STORY_ACTION_END, 0, 0, 0, NULL }
};

static const struct ps_story_action garden_actions[] = {
    STORY_OPENING,
    { PS_STORY_ACTION_WALK, 11, 5, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_RIGHT, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 11, 5, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_RIGHT, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 6, 7, 0, NULL },
    { PS_STORY_ACTION_FACE, PS_GRID_RIGHT, 0, 0, NULL },
    { PS_STORY_ACTION_SAY, 8, 0, 0, "Nel: The roses tell the bees where to land. The mint tells everyone else." },
    { PS_STORY_ACTION_WALK, 5, 6, 0, NULL },
    { PS_STORY_ACTION_FACE, PS_GRID_DOWN, 0, 0, NULL },
    { PS_STORY_ACTION_SAY, 0, 0, 0, "A drop slides from one mint leaf to the next without touching the soil." },
    { PS_STORY_ACTION_WAIT, 28, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 1, 5, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_LEFT, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 1, 5, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_LEFT, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 6, 1, 0, NULL },
    { PS_STORY_ACTION_LINK_SCENE, PS_GRID_UP, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 6, 4, 0, NULL },
    { PS_STORY_ACTION_WAIT, 45, 0, 0, NULL },
    { PS_STORY_ACTION_END, 0, 0, 0, NULL }
};

#undef STORY_OPENING

static const struct ps_story_script itineraries[] = {
    { market_actions, ARRAYLEN(market_actions) },
    { mill_actions, ARRAYLEN(mill_actions) },
    { gate_actions, ARRAYLEN(gate_actions) },
    { farm_actions, ARRAYLEN(farm_actions) },
    { garden_actions, ARRAYLEN(garden_actions) }
};

static const struct ps_story_action preview_actions[] = {
    { PS_STORY_ACTION_WAIT, 10000, 0, 0, NULL },
    { PS_STORY_ACTION_END, 0, 0, 0, NULL }
};

static const struct button_mapping story_context[] = {
    { PLA_CANCEL, BUTTON_MENU, BUTTON_NONE },
    { PLA_EXIT, BUTTON_PLAY, BUTTON_NONE },
    { PLA_CANCEL, BUTTON_LEFT, BUTTON_NONE },
    LAST_ITEM_IN_LIST
};
static const struct button_mapping *plugin_contexts[] = { story_context };

static struct story_state story;
static struct ps_story_director director;
static int simulator_palette = PALETTE_AUTO;
static int simulator_itinerary = -1;
static int simulator_preview_scene = -1;
static int frame_number;

static void use_color(unsigned int color)
{
    rb->lcd_set_foreground(color);
}

static int tile_x(int column)
{
    return MAP_GUTTER + column * TILE_SIZE;
}

static int body_tile_x(const struct ps_body *body)
{
    return ((body->x / PS_ONE) + body->width / 2 - MAP_GUTTER) / TILE_SIZE;
}

static int body_tile_y(const struct ps_body *body)
{
    return ((body->y / PS_ONE) + body->height / 2) / TILE_SIZE;
}

static void place_actor(int x, int y)
{
    story.actor.x = (tile_x(x) + 4) * PS_ONE;
    story.actor.y = (y * TILE_SIZE + 8) * PS_ONE;
    story.actor.vx = 0;
    story.actor.vy = 0;
    story.actor.width = 8;
    story.actor.height = 6;
}

static const struct palette *active_palette(const struct tm *now)
{
    int hour = now->tm_hour;

    if (story.scene == SCENE_HOUSE)
        return &indoor_palette;
    if (simulator_palette == PALETTE_DAY)
        return &day_palette;
    if (simulator_palette == PALETTE_EVENING)
        return &evening_palette;
    if (simulator_palette == PALETTE_NIGHT)
        return &night_palette;
    if (hour >= 7 && hour < 18)
        return &day_palette;
    if ((hour >= 5 && hour < 7) || (hour >= 18 && hour < 21))
        return &evening_palette;
    return &night_palette;
}

static void load_simulator_scenario(void)
{
#ifdef SIMULATOR
    char value[12];
    int fd = rb->open("/storyclock-scenario.txt", O_RDONLY);
    int count;

    if (fd < 0)
        return;
    count = rb->read(fd, value, sizeof(value) - 1);
    rb->close(fd);
    if (count <= 0)
        return;
    value[count] = '\0';
    count = rb->atoi(value);
    if (count >= PALETTE_AUTO && count <= PALETTE_NIGHT)
        simulator_palette = count;
    else if (count >= 10 && count <= 14)
    {
        simulator_palette = PALETTE_DAY;
        simulator_itinerary = count - 10;
    }
    else if (count >= 40 && count <= 60)
    {
        int preview = count - 40;

        simulator_palette = PALETTE_DAY + preview / 7;
        simulator_preview_scene = SCENE_COTTAGE + preview % 7;
    }
#endif
}

static void reset_story(void *context)
{
    struct story_state *state = context;
    int npc;

    if (state->initialized)
        state->loop_index++;
    else
        state->initialized = 1;
    state->itinerary_index = simulator_itinerary >= 0 ?
        simulator_itinerary : ps_story_itinerary_select(
            117, state->loop_index, ARRAYLEN(itineraries));

    state->scene = simulator_preview_scene >= 0 ?
        simulator_preview_scene : SCENE_HOUSE;
    state->facing = PS_GRID_DOWN;
    state->walk_distance = 0;
    state->item_collected = simulator_preview_scene >= 0;
    state->dialogue_frames = 0;
    state->dialogue_speaker = 0;
    for (npc = 0; npc < (int)ARRAYLEN(state->npc_facing); ++npc)
        state->npc_facing[npc] = PS_GRID_DOWN;
    state->dialogue_text = NULL;
    state->failure = 0;
    state->route.count = 0;
    state->route_index = 0;
    state->route_active = 0;
    state->active_action = -1;
    place_actor(scenes[state->scene].spawn.x,
                scenes[state->scene].spawn.y);
    if (simulator_preview_scene >= 0)
    {
        ps_story_init(&director, preview_actions, ARRAYLEN(preview_actions), 1);
        return;
    }
    ps_story_init_itinerary(&director, itineraries, ARRAYLEN(itineraries),
                            state->itinerary_index,
                            simulator_itinerary < 0);
}

static int begin_route(int destination_x, int destination_y)
{
    const struct ps_scene *scene = &scenes[story.scene];
    int result;

    story.route.cells = story.route_cells;
    story.route.capacity = ROUTE_CAPACITY;
    story.route.count = 0;
    result = ps_grid_find_path(&scene->grid,
                               body_tile_x(&story.actor),
                               body_tile_y(&story.actor),
                               destination_x, destination_y,
                               &story.workspace, &story.route);
    if (result != PS_PATH_FOUND)
    {
        story.failure = result;
        return 0;
    }
    story.route_index = 0;
    story.route_active = 1;
    return 1;
}

static int move_toward(int target, int *position)
{
    int speed = PS_ONE * 2;
    int delta = target - *position;
    int previous = *position;

    if (delta > speed)
        *position += speed;
    else if (delta < -speed)
        *position -= speed;
    else
        *position = target;
    delta = *position - previous;
    story.walk_distance += (delta < 0 ? -delta : delta) / PS_ONE;
    return *position == target;
}

static int update_route(void)
{
    struct ps_grid_cell target_cell;
    int target_x;
    int target_y;
    int reached_x;
    int reached_y;

    if (!story.route_active)
        return 1;
    if (story.route_index >= story.route.count)
    {
        story.route_active = 0;
        return 1;
    }
    target_cell = story.route.cells[story.route_index];
    target_x = (tile_x(target_cell.x) + 4) * PS_ONE;
    target_y = (target_cell.y * TILE_SIZE + 8) * PS_ONE;
    if (target_x > story.actor.x)
        story.facing = PS_GRID_RIGHT;
    else if (target_x < story.actor.x)
        story.facing = PS_GRID_LEFT;
    else if (target_y > story.actor.y)
        story.facing = PS_GRID_DOWN;
    else if (target_y < story.actor.y)
        story.facing = PS_GRID_UP;
    reached_x = move_toward(target_x, &story.actor.x);
    reached_y = move_toward(target_y, &story.actor.y);
    if (reached_x && reached_y)
    {
        story.route_index++;
        if (story.route_index >= story.route.count)
        {
            story.route_active = 0;
            return 1;
        }
    }
    return 0;
}

static int start_dialogue(const struct ps_story_action *action)
{
    static const int npc_columns[8] = { 2, 3, 7, 6, 8, 7, 8, 7 };
    static const int npc_rows[8] = { 4, 5, 6, 6, 6, 6, 5, 7 };
    int length = rb->strlen(action->text);
    int actor_x = body_tile_x(&story.actor);
    int actor_y = body_tile_y(&story.actor);

    story.dialogue_speaker = action->a;
    story.dialogue_text = action->text;
    story.dialogue_frames = 28 + length / 2;
    if (action->a >= 1 && action->a <= 8)
    {
        int npc = action->a - 1;
        story.npc_facing[npc] = ps_grid_direction_toward(
            npc_columns[npc], npc_rows[npc], actor_x, actor_y,
            story.npc_facing[npc]);
    }
    return PS_STORY_ACTION_PENDING;
}

static int handle_story_action(const struct ps_story_action *action,
                               void *context)
{
    struct story_state *state = context;

    if (state->active_action != director.action_index)
    {
        state->active_action = director.action_index;
        state->route_active = 0;
        if (action->kind == PS_STORY_ACTION_SAY)
            return start_dialogue(action);
    }

    switch (action->kind)
    {
        case PS_STORY_ACTION_WALK:
            if (!state->route_active && state->route.count == 0)
            {
                if (!begin_route(action->a, action->b))
                    return PS_STORY_ACTION_FAILED;
            }
            if (update_route())
            {
                state->route.count = 0;
                return PS_STORY_ACTION_DONE;
            }
            return PS_STORY_ACTION_PENDING;

        case PS_STORY_ACTION_FACE:
            state->facing = action->a;
            return PS_STORY_ACTION_DONE;

        case PS_STORY_ACTION_SAY:
            if (state->dialogue_frames > 0)
            {
                state->dialogue_frames--;
                return PS_STORY_ACTION_PENDING;
            }
            state->dialogue_text = NULL;
            return PS_STORY_ACTION_DONE;

        case PS_STORY_ACTION_COLLECT:
        {
            const struct ps_scene *scene = &scenes[state->scene];
            int region = ps_region_find_facing(scene->regions,
                                               scene->region_count,
                                               &state->actor,
                                               state->facing, 12);
            if (action->a != ITEM_KEY || region != action->b)
                return PS_STORY_ACTION_FAILED;
            state->item_collected = 1;
            return PS_STORY_ACTION_DONE;
        }

        case PS_STORY_ACTION_SCENE:
            if (action->a < 0 || action->a >= (int)ARRAYLEN(scenes))
                return PS_STORY_ACTION_FAILED;
            state->scene = action->a;
            state->route.count = 0;
            state->route_active = 0;
            state->facing = PS_GRID_UP;
            place_actor(action->b, action->c);
            return PS_STORY_ACTION_DONE;

        case PS_STORY_ACTION_EDGE_SCENE:
        {
            struct ps_grid_cell entry;

            if (action->a < 0 || action->a >= (int)ARRAYLEN(scenes) ||
                !ps_scene_edge_entry(&scenes[action->a], action->b,
                                     action->c, &entry))
                return PS_STORY_ACTION_FAILED;
            state->scene = action->a;
            state->route.count = 0;
            state->route_active = 0;
            state->facing = action->b;
            place_actor(entry.x, entry.y);
            return PS_STORY_ACTION_DONE;
        }

        case PS_STORY_ACTION_LINK_SCENE:
        {
            struct ps_grid_cell entry;
            int target_scene;

            if (!ps_scene_link_follow(village_links,
                                      ARRAYLEN(village_links),
                                      scenes, ARRAYLEN(scenes),
                                      state->scene, action->a,
                                      &target_scene, &entry))
                return PS_STORY_ACTION_FAILED;
            state->scene = target_scene;
            state->route.count = 0;
            state->route_active = 0;
            state->facing = action->a;
            place_actor(entry.x, entry.y);
            return PS_STORY_ACTION_DONE;
        }

        default:
            return PS_STORY_ACTION_FAILED;
    }
}

static const unsigned short *outdoor_tiles_for_palette(
    const struct palette *p)
{
    if (p == &night_palette)
        return story_outdoor_tiles_night;
    if (p == &evening_palette)
        return story_outdoor_tiles_evening;
    return story_outdoor_tiles_day;
}

static const unsigned short *outdoor_object_for_palette(
    const struct palette *p, const unsigned short *day,
    const unsigned short *evening, const unsigned short *night)
{
    if (p == &night_palette)
        return night;
    if (p == &evening_palette)
        return evening;
    return day;
}

static int outdoor_path_tile_at(int column, int row)
{
    int tile;

    if (column < 0 || column >= MAP_WIDTH || row < 0 || row >= MAP_HEIGHT)
        return 0;
    tile = scenes[story.scene].tiles[row * MAP_WIDTH + column];
    return tile == TILE_PATH || tile == TILE_DOOR;
}

static int outdoor_path_variant(int column, int row)
{
    int horizontal = outdoor_path_tile_at(column - 1, row) ||
                     outdoor_path_tile_at(column + 1, row);
    int vertical = outdoor_path_tile_at(column, row - 1) ||
                   outdoor_path_tile_at(column, row + 1);

    if (horizontal && vertical)
        return 2;
    return horizontal ? 1 : 0;
}

static void draw_tile(int column, int row, int tile,
                      const struct palette *p)
{
    static const int variants[3] = { 0, 1, 3 };
    const unsigned short *atlas;
    int variant_index = ps_tile_variation(
        column, row, scenes[story.scene].variation_seed, ARRAYLEN(variants));
    int variant = variants[variant_index];
    int x = tile_x(column);
    int y = row * TILE_SIZE;
    int source_y;

    if (story.scene == SCENE_HOUSE)
    {
        if (tile == TILE_WALL)
        {
            atlas = story_indoor_tiles;
            source_y = 0;
            rb->lcd_bitmap_part((const fb_data *)atlas,
                                variant * 16, source_y,
                                STORY_TILE_ATLAS_WIDTH,
                                x, y, TILE_SIZE, TILE_SIZE);
        }
        else
            rb->lcd_bitmap_part((const fb_data *)story_floor_tiles,
                                ((column + row * 2) % 3) * 16, 0,
                                STORY_FLOOR_ATLAS_WIDTH,
                                x, y, TILE_SIZE, TILE_SIZE);
    }
    else
    {
        atlas = outdoor_tiles_for_palette(p);
        if (tile == TILE_PATH || tile == TILE_DOOR)
        {
            source_y = 16;
            variant = outdoor_path_variant(column, row);
        }
        else
            source_y = 0;
        rb->lcd_bitmap_part((const fb_data *)atlas,
                            variant * 16, source_y,
                            STORY_OUTDOOR_ATLAS_WIDTH,
                            x, y, TILE_SIZE, TILE_SIZE);
    }

    if (tile == TILE_WATER)
    {
        const unsigned short *water = outdoor_object_for_palette(
            p, story_water_day, story_water_evening, story_water_night);
        rb->lcd_bitmap((const fb_data *)water, x, y, 16, 16);
    }
    else if (tile == TILE_FLOWER)
    {
        const unsigned short *flower = outdoor_object_for_palette(
            p, story_flower_day, story_flower_evening, story_flower_night);
        rb->lcd_bitmap_transparent((const fb_data *)flower,
                                   x, y, 16, 16);
    }
    else if (tile == TILE_SHRUB)
    {
        const unsigned short *shrub = outdoor_object_for_palette(
            p, story_shrub_day, story_shrub_evening, story_shrub_night);
        rb->lcd_bitmap_transparent((const fb_data *)shrub,
                                   x, y, 16, 16);
    }
    else if (tile == TILE_ROCK)
    {
        const unsigned short *rock = outdoor_object_for_palette(
            p, story_rock_day, story_rock_evening, story_rock_night);
        rb->lcd_bitmap_transparent((const fb_data *)rock,
                                   x, y, 16, 16);
    }
    else if (tile == TILE_CROP_WHEAT)
    {
        const unsigned short *crop = outdoor_object_for_palette(
            p, story_farm_wheat_day, story_farm_wheat_evening,
            story_farm_wheat_night);
        rb->lcd_bitmap_transparent((const fb_data *)crop, x, y, 16, 16);
    }
    else if (tile == TILE_CROP_CORN)
    {
        const unsigned short *crop = outdoor_object_for_palette(
            p, story_farm_corn_day, story_farm_corn_evening,
            story_farm_corn_night);
        rb->lcd_bitmap_transparent((const fb_data *)crop, x, y, 16, 16);
    }
    else if (tile == TILE_GARDEN_BLOOM)
    {
        const unsigned short *bloom = outdoor_object_for_palette(
            p, story_garden_bloom_day, story_garden_bloom_evening,
            story_garden_bloom_night);
        rb->lcd_bitmap_transparent((const fb_data *)bloom, x, y, 16, 16);
    }
    else if (tile == TILE_GARDEN_HERB)
    {
        const unsigned short *herb = outdoor_object_for_palette(
            p, story_garden_herb_day, story_garden_herb_evening,
            story_garden_herb_night);
        rb->lcd_bitmap_transparent((const fb_data *)herb, x, y, 16, 16);
    }
}

static void draw_house_furniture(void)
{
    use_color(LCD_RGBPACK(70, 48, 48));
    rb->lcd_fillrect(tile_x(1) + 4, 47, 34, 3);
    rb->lcd_fillrect(tile_x(10) + 4, 39, 22, 3);
    rb->lcd_fillrect(tile_x(11) + 2, 105, 12, 2);
    rb->lcd_fillrect(tile_x(9) + 7, 121, 31, 3);
    rb->lcd_fillrect(tile_x(10) + 2, 132, 12, 2);

    rb->lcd_bitmap_transparent((const fb_data *)story_bed,
                               tile_x(1), 8, 40, 42);
    rb->lcd_bitmap_transparent((const fb_data *)story_bookshelf,
                               tile_x(10) + 2, 0, 28, 42);
    rb->lcd_bitmap_transparent((const fb_data *)story_hanging_plant,
                               tile_x(4), 0, 16, 32);
    rb->lcd_bitmap_transparent((const fb_data *)story_floor_lamp,
                               tile_x(11), 5 * TILE_SIZE, 16, 28);
    rb->lcd_bitmap_transparent((const fb_data *)story_writing_desk,
                               tile_x(9) + 4, 6 * TILE_SIZE, 36, 28);
    rb->lcd_bitmap_transparent((const fb_data *)story_stool,
                               tile_x(10), 7 * TILE_SIZE + 4, 16, 20);

    rb->lcd_bitmap_transparent((const fb_data *)story_door,
                               tile_x(5) + 8, 9 * TILE_SIZE, 32, 32);
}

static void draw_house_architecture(void)
{
    int column;
    int row;

    for (column = 1; column < MAP_WIDTH - 1; ++column)
    {
        rb->lcd_bitmap_transparent((const fb_data *)story_wall_trim_h,
                                   tile_x(column), 8, 16, 8);
        rb->lcd_bitmap_transparent((const fb_data *)story_wall_trim_h,
                                   tile_x(column), 10 * TILE_SIZE, 16, 8);
    }
    for (row = 1; row < MAP_HEIGHT - 1; ++row)
    {
        rb->lcd_bitmap_transparent((const fb_data *)story_wall_trim_v,
                                   tile_x(0) + 8, row * TILE_SIZE, 8, 16);
        rb->lcd_bitmap_transparent((const fb_data *)story_wall_trim_v,
                                   tile_x(12), row * TILE_SIZE, 8, 16);
    }
    rb->lcd_bitmap_transparent((const fb_data *)story_window,
                               tile_x(5) + 10, 0, 28, 28);
    rb->lcd_bitmap_transparent((const fb_data *)story_fireplace,
                               tile_x(1), 6 * TILE_SIZE + 2, 32, 28);
}

static void draw_outdoor_architecture(const struct palette *p)
{
    const unsigned short *house = outdoor_object_for_palette(
        p, story_village_house_day, story_village_house_evening,
        story_village_house_night);
    const unsigned short *fence = outdoor_object_for_palette(
        p, story_town_fence_day, story_town_fence_evening,
        story_town_fence_night);

    if (story.scene == SCENE_COTTAGE)
    {
        rb->lcd_bitmap_transparent((const fb_data *)fence,
                                   tile_x(0), 3 * TILE_SIZE, 48, 16);
        rb->lcd_bitmap_transparent((const fb_data *)fence,
                                   tile_x(10), 3 * TILE_SIZE, 48, 16);
        rb->lcd_bitmap_transparent((const fb_data *)house,
                                   tile_x(3), 0, 96, 72);
    }
    else if (story.scene == SCENE_GREEN)
    {
        const unsigned short *facades = outdoor_object_for_palette(
            p, story_green_facades_day, story_green_facades_evening,
            story_green_facades_night);
        rb->lcd_bitmap_transparent((const fb_data *)facades,
                                   tile_x(1), 0, 176, 64);
    }
    else if (story.scene == SCENE_MILL)
    {
        int wheel_frame = (frame_number / 5) & 3;
        const unsigned short *wheel_day;
        const unsigned short *wheel_evening;
        const unsigned short *wheel_night;
        const unsigned short *facade = outdoor_object_for_palette(
            p, story_mill_facade_day, story_mill_facade_evening,
            story_mill_facade_night);

        if (wheel_frame == 0)
        {
            wheel_day = story_mill_wheel_a_day;
            wheel_evening = story_mill_wheel_a_evening;
            wheel_night = story_mill_wheel_a_night;
        }
        else if (wheel_frame == 1)
        {
            wheel_day = story_mill_wheel_b_day;
            wheel_evening = story_mill_wheel_b_evening;
            wheel_night = story_mill_wheel_b_night;
        }
        else if (wheel_frame == 2)
        {
            wheel_day = story_mill_wheel_c_day;
            wheel_evening = story_mill_wheel_c_evening;
            wheel_night = story_mill_wheel_c_night;
        }
        else
        {
            wheel_day = story_mill_wheel_d_day;
            wheel_evening = story_mill_wheel_d_evening;
            wheel_night = story_mill_wheel_d_night;
        }
        const unsigned short *wheel = outdoor_object_for_palette(
            p, wheel_day, wheel_evening, wheel_night);
        rb->lcd_bitmap_transparent((const fb_data *)facade,
                                   tile_x(6), 0, 96, 64);
        rb->lcd_bitmap_transparent((const fb_data *)wheel,
                                   tile_x(4), TILE_SIZE, 32, 40);
    }
    else if (story.scene == SCENE_MARKET)
    {
        const unsigned short *facades = outdoor_object_for_palette(
            p, story_market_facades_day, story_market_facades_evening,
            story_market_facades_night);
        rb->lcd_bitmap_transparent((const fb_data *)facades,
                                   tile_x(1), 0, 176, 64);
    }
    else if (story.scene == SCENE_GATE)
    {
        const unsigned short *gate = outdoor_object_for_palette(
            p, story_south_gate_day, story_south_gate_evening,
            story_south_gate_night);
        rb->lcd_bitmap_transparent((const fb_data *)gate,
                                   tile_x(0) + 12, 4 * TILE_SIZE,
                                   144, 64);
    }
    else if (story.scene == SCENE_FIELDS)
    {
        const unsigned short *pen = outdoor_object_for_palette(
            p, story_chicken_pen_day, story_chicken_pen_evening,
            story_chicken_pen_night);

        rb->lcd_bitmap_transparent((const fb_data *)house,
                                   tile_x(0), 0, 96, 72);
        rb->lcd_bitmap_transparent((const fb_data *)pen,
                                   tile_x(9), 6 * TILE_SIZE, 48, 48);
    }
    else if (story.scene == SCENE_GARDEN)
    {
        const unsigned short *shed = outdoor_object_for_palette(
            p, story_potting_shed_day, story_potting_shed_evening,
            story_potting_shed_night);
        rb->lcd_bitmap_transparent((const fb_data *)shed,
                                   tile_x(8), 0, 80, 80);
    }
}

static void draw_outdoor_ambient(const struct palette *p)
{
    int phase = (frame_number / 5) & 3;

    if (story.scene == SCENE_GREEN)
    {
        use_color((phase & 1) ? p->accent : p->light);
        rb->lcd_fillrect(tile_x(8) + phase * 2, 7 * TILE_SIZE - phase, 2, 2);
        rb->lcd_fillrect(tile_x(8) + phase * 2 + 3, 7 * TILE_SIZE + 1,
                         2, 2);
    }
    else if (story.scene == SCENE_MARKET)
    {
        use_color(p->accent);
        rb->lcd_fillrect(tile_x(2) + 5, 4 * TILE_SIZE,
                         2 + (phase & 1), 4);
        rb->lcd_fillrect(tile_x(10) + 4, 4 * TILE_SIZE,
                         3 - (phase & 1), 4);
    }
    else if (story.scene == SCENE_GATE)
    {
        use_color((phase & 1) ? p->light : p->mid);
        rb->lcd_fillrect(tile_x(1) + 8 + phase, 4 * TILE_SIZE - 8 - phase * 3,
                         4, 3);
        use_color(p->accent);
        rb->lcd_fillrect(tile_x(2) + 7 + phase * 2,
                         8 * TILE_SIZE - 2 - phase, 2, 2);
        if (phase & 1)
            rb->lcd_fillrect(tile_x(2) + 12,
                             8 * TILE_SIZE - 7, 2, 2);
    }
    else if (story.scene == SCENE_FIELDS)
    {
        const unsigned short *chicken = (phase & 1) ?
            outdoor_object_for_palette(
                p, story_chicken_b_day, story_chicken_b_evening,
                story_chicken_b_night) :
            outdoor_object_for_palette(
                p, story_chicken_a_day, story_chicken_a_evening,
                story_chicken_a_night);
        rb->lcd_bitmap_transparent((const fb_data *)chicken,
                                   tile_x(10) + phase * 3,
                                   7 * TILE_SIZE + ((phase >> 1) * 4),
                                   16, 16);
    }
    else if (story.scene == SCENE_GARDEN)
    {
        use_color((phase & 1) ? p->accent : p->light);
        rb->lcd_fillrect(tile_x(2) + phase * 5,
                         3 * TILE_SIZE - 5 - (phase & 1), 2, 2);
        rb->lcd_fillrect(tile_x(4) + 7 - phase * 3,
                         2 * TILE_SIZE + 3 + (phase & 1), 2, 2);
        use_color(p->water);
        rb->lcd_fillrect(tile_x(7) + phase * 2,
                         8 * TILE_SIZE - phase * 2, 2, 2);
    }
}

static void draw_tall(int x, int y, int id, const struct palette *p)
{
    if (id == TILE_TREE)
    {
        const unsigned short *tree = outdoor_object_for_palette(
            p, story_tree_day, story_tree_evening, story_tree_night);
        rb->lcd_bitmap_transparent((const fb_data *)tree,
                                   x - 4, y - 20, 24, 36);
    }
    else if (id == TILE_BED)
    {
        rb->lcd_bitmap_transparent((const fb_data *)story_bed,
                                   x, y, 32, 32);
    }
    else if (id == TILE_TABLE)
    {
        rb->lcd_bitmap_transparent((const fb_data *)story_bookshelf,
                                   x, y, 32, 32);
    }
    else if (id == TILE_BEACON)
    {
        const unsigned short *beacon = outdoor_object_for_palette(
            p, story_beacon_day, story_beacon_evening, story_beacon_night);
        rb->lcd_bitmap_transparent((const fb_data *)beacon,
                                   x, y, 16, 16);
    }
    else if (id == OUTDOOR_PROP_CRATE)
    {
        const unsigned short *crate = outdoor_object_for_palette(
            p, story_town_crate_day, story_town_crate_evening,
            story_town_crate_night);
        rb->lcd_bitmap_transparent((const fb_data *)crate,
                                   x, y, 16, 16);
    }
    else if (id == OUTDOOR_PROP_BARREL)
    {
        const unsigned short *barrel = outdoor_object_for_palette(
            p, story_town_barrel_day, story_town_barrel_evening,
            story_town_barrel_night);
        rb->lcd_bitmap_transparent((const fb_data *)barrel,
                                   x, y, 16, 16);
    }
    else if (id == OUTDOOR_PROP_WELL)
    {
        const unsigned short *well = outdoor_object_for_palette(
            p, story_village_well_day, story_village_well_evening,
            story_village_well_night);
        rb->lcd_bitmap_transparent((const fb_data *)well,
                                   x - 8, y - 20, 32, 36);
    }
    else if (id == OUTDOOR_PROP_ANVIL)
    {
        const unsigned short *anvil = outdoor_object_for_palette(
            p, story_smithy_anvil_day, story_smithy_anvil_evening,
            story_smithy_anvil_night);
        rb->lcd_bitmap_transparent((const fb_data *)anvil,
                                   x, y, 16, 16);
    }
}

static void draw_gate_foreground(const struct palette *p)
{
    const unsigned short *gate;
    int phase;

    if (story.scene != SCENE_GATE)
        return;
    gate = outdoor_object_for_palette(
        p, story_south_gate_day, story_south_gate_evening,
        story_south_gate_night);
    rb->lcd_bitmap_transparent_part((const fb_data *)gate,
                                    0, 0, 144,
                                    tile_x(0) + 12, 4 * TILE_SIZE,
                                    144, 32);
    phase = (frame_number / 5) & 3;
    if (phase & 1)
    {
        use_color(p->accent);
        rb->lcd_fillrect(tile_x(6) - 3, 6 * TILE_SIZE + 2, 3, 3);
        rb->lcd_fillrect(tile_x(7) + 4, 6 * TILE_SIZE + 2, 3, 3);
    }
}

static void draw_actor(int x, int y)
{
    struct ps_anim_frame frame;

    if (!ps_anim_select(&actor_animation, story.facing,
                        story.route_active, story.walk_distance, 4, &frame))
        return;

    rb->lcd_bitmap_transparent_part((const fb_data *)story_luma_walk,
                                    frame.x, frame.y,
                                    STORY_WALK_SHEET_WIDTH,
                                    x, y,
                                    frame.width, frame.height);
}

static void draw_npc(int x, int y, int id)
{
    const unsigned short *sheet = (id == 0 || id == 7) ?
        story_mira_walk : story_tovin_walk;
    struct ps_anim_frame frame;

    if (!ps_anim_select(&npc_animation, story.npc_facing[id], 0,
                        0, 1, &frame))
        return;

    rb->lcd_bitmap_transparent_part((const fb_data *)sheet,
                                    frame.x, frame.y,
                                    STORY_WALK_SHEET_WIDTH,
                                    x, y,
                                    frame.width, frame.height);
}

static void draw_item(int x, int y)
{
    int bob = (frame_number / 8) & 1;

    rb->lcd_bitmap_transparent((const fb_data *)story_key,
                               x, y - bob, 16, 16);
}

static void queue_drawable(struct ps_draw_list *list,
                           int kind, int x, int y, int foot_y, int id)
{
    struct ps_drawable value;

    value.kind = kind;
    value.x = x;
    value.y = y;
    value.foot_y = foot_y;
    value.id = id;
    value.retention_priority =
        kind == DRAW_ACTOR || kind == DRAW_NPC || kind == DRAW_ITEM ?
        PS_DRAW_PRIORITY_REQUIRED : PS_DRAW_PRIORITY_OPTIONAL;
    if (!ps_draw_list_add_prioritized(list, &value))
        story.failure = PS_PATH_CAPACITY;
}

static void draw_world(const struct palette *p)
{
    const struct ps_scene *scene = &scenes[story.scene];
    struct ps_drawable drawable_storage[DRAWABLE_CAPACITY];
    struct ps_draw_list drawables;
    int row;
    int column;
    int actor_left = story.actor.x / PS_ONE + story.actor.width / 2 - 10;
    int actor_top = story.actor.y / PS_ONE + story.actor.height - 20;

    ps_draw_list_init(&drawables, drawable_storage,
                      ARRAYLEN(drawable_storage));
    /* Keep the actor even if a future scene exceeds its decoration budget. */
    queue_drawable(&drawables, DRAW_ACTOR, actor_left, actor_top,
                   story.actor.y / PS_ONE + story.actor.height, 0);
    use_color(p->dark);
    rb->lcd_fillrect(0, 0, LCD_WIDTH, LCD_HEIGHT);
    for (row = 0; row < MAP_HEIGHT; ++row)
    {
        for (column = 0; column < MAP_WIDTH; ++column)
        {
            int tile = scene->tiles[row * MAP_WIDTH + column];
            draw_tile(column, row, tile, p);
            if (tile == TILE_TREE || tile == TILE_BEACON)
                queue_drawable(&drawables, DRAW_TALL,
                               tile_x(column), row * TILE_SIZE,
                               row * TILE_SIZE + 15, tile);
            else if ((tile == TILE_BED || tile == TILE_TABLE) &&
                     (column == 0 || scene->tiles[row * MAP_WIDTH +
                                                  column - 1] != tile) &&
                     (row == 0 || scene->tiles[(row - 1) * MAP_WIDTH +
                                               column] != tile))
                queue_drawable(&drawables, DRAW_TALL,
                               tile_x(column), row * TILE_SIZE,
                               row * TILE_SIZE + 31, tile);
        }
    }
    if (story.scene == SCENE_HOUSE)
        draw_house_architecture();
    else
    {
        draw_outdoor_architecture(p);
        draw_outdoor_ambient(p);
    }
    if (story.scene == SCENE_HOUSE)
        rb->lcd_bitmap_transparent((const fb_data *)story_rug,
                                   tile_x(5), 3 * TILE_SIZE, 48, 48);
    if (story.scene == SCENE_HOUSE)
        draw_house_furniture();
    if (story.scene == SCENE_HOUSE)
    {
        queue_drawable(&drawables, DRAW_NPC,
                       tile_x(2) - 2, 4 * TILE_SIZE - 4,
                       5 * TILE_SIZE - 1, 0);
        if (!story.item_collected)
            queue_drawable(&drawables, DRAW_ITEM,
                           tile_x(9), 4 * TILE_SIZE,
                           5 * TILE_SIZE - 1, ITEM_KEY);
    }
    else if (story.scene == SCENE_COTTAGE)
    {
        queue_drawable(&drawables, DRAW_NPC,
                       tile_x(3) - 2, 5 * TILE_SIZE - 4,
                       6 * TILE_SIZE - 1, 1);
    }
    else if (story.scene == SCENE_GREEN)
        queue_drawable(&drawables, DRAW_NPC,
                       tile_x(7) - 2, 6 * TILE_SIZE - 4,
                       7 * TILE_SIZE - 1, 2);
    else if (story.scene == SCENE_MILL)
        queue_drawable(&drawables, DRAW_NPC,
                       tile_x(6) - 2, 6 * TILE_SIZE - 4,
                       7 * TILE_SIZE - 1, 3);
    else if (story.scene == SCENE_MARKET)
        queue_drawable(&drawables, DRAW_NPC,
                       tile_x(8) - 2, 6 * TILE_SIZE - 4,
                       7 * TILE_SIZE - 1, 4);
    else if (story.scene == SCENE_GATE)
        queue_drawable(&drawables, DRAW_NPC,
                       tile_x(7) - 2, 6 * TILE_SIZE - 4,
                       7 * TILE_SIZE - 1, 5);
    else if (story.scene == SCENE_FIELDS)
        queue_drawable(&drawables, DRAW_NPC,
                       tile_x(8) - 2, 5 * TILE_SIZE - 4,
                       6 * TILE_SIZE - 1, 6);
    else if (story.scene == SCENE_GARDEN)
        queue_drawable(&drawables, DRAW_NPC,
                       tile_x(7) - 2, 7 * TILE_SIZE - 4,
                       8 * TILE_SIZE - 1, 7);
    if (story.scene != SCENE_HOUSE)
    {
        int prop_index;

        for (prop_index = 0;
             prop_index < (int)ARRAYLEN(village_props); ++prop_index)
        {
            const struct ps_scene_prop *prop = &village_props[prop_index];

            if (prop->scene == story.scene)
                queue_drawable(&drawables, DRAW_TALL,
                               tile_x(prop->column),
                               prop->row * TILE_SIZE,
                               prop->foot_y, prop->asset_id);
        }
    }
    if (story.scene == SCENE_GATE)
        queue_drawable(&drawables, DRAW_GATE_FOREGROUND,
                       0, 0, 6 * TILE_SIZE, 0);
    for (row = 0; row < drawables.count; ++row)
    {
        const struct ps_drawable *d = ps_draw_list_get(&drawables, row);
        if (d->kind == DRAW_ACTOR)
            draw_actor(d->x, d->y);
        else if (d->kind == DRAW_NPC)
            draw_npc(d->x, d->y, d->id);
        else if (d->kind == DRAW_ITEM)
            draw_item(d->x, d->y);
        else if (d->kind == DRAW_GATE_FOREGROUND)
            draw_gate_foreground(p);
        else
            draw_tall(d->x, d->y, d->id, p);
    }
}

static void split_dialogue(const char *text, char *line_one, int one_size,
                           char *line_two, int two_size)
{
    int length = rb->strlen(text);
    int split = MIN(length, 31);
    int i;

    if (length > split)
    {
        for (i = split; i > 12; --i)
        {
            if (text[i] == ' ')
            {
                split = i;
                break;
            }
        }
    }
    rb->strlcpy(line_one, text, MIN(one_size, split + 1));
    while (text[split] == ' ')
        split++;
    rb->strlcpy(line_two, text + split, two_size);
}

static void draw_dialogue(const struct palette *p)
{
    static const char *speakers[] = {
        "LUMA", "MIRA", "TOVIN", "EDA", "BRAN", "SERA", "ROWAN",
        "ORIN", "NEL"
    };
    char line_one[36];
    char line_two[36];
    int y = LCD_HEIGHT - 47;

    if (story.dialogue_text == NULL)
        return;
    split_dialogue(story.dialogue_text, line_one, sizeof(line_one),
                   line_two, sizeof(line_two));
    use_color(p->box);
    rb->lcd_fillrect(5, y, LCD_WIDTH - 10, 43);
    use_color(p->light);
    rb->lcd_drawrect(6, y + 1, LCD_WIDTH - 12, 41);
    use_color(p->accent);
    rb->lcd_putsxy(12, y + 4, speakers[story.dialogue_speaker]);
    use_color(p->text);
    rb->lcd_putsxy(12, y + 16, line_one);
    rb->lcd_putsxy(12, y + 27, line_two);
    if ((frame_number / 8) & 1)
    {
        use_color(p->accent);
        rb->lcd_fillrect(LCD_WIDTH - 18, y + 34, 5, 3);
    }
}

static void draw_status(const struct palette *p)
{
    char item_text[9];
    int width;
    int height;

    rb->snprintf(item_text, sizeof(item_text), "KEY:%s",
                 story.item_collected ? "YES" : "--");
    rb->lcd_getstringsize(item_text, &width, &height);
    use_color(p->box);
    rb->lcd_fillrect(3, 3, width + 7, height + 4);
    use_color(story.item_collected ? p->accent : p->text);
    rb->lcd_putsxy(6, 5, item_text);
}

static void draw_failure(const struct palette *p)
{
    if (!story.failure && director.state != PS_STORY_FAILED)
        return;
    use_color(p->box);
    rb->lcd_fillrect(12, 65, LCD_WIDTH - 24, 45);
    use_color(p->accent);
    rb->lcd_drawrect(13, 66, LCD_WIDTH - 26, 43);
    rb->lcd_putsxy(22, 75, "STORY ROUTE HALTED");
    use_color(p->text);
    rb->lcd_putsxy(22, 89, "Check authored destination.");
}

static void cleanup(void)
{
    backlight_use_settings();
    rb->lcd_setfont(FONT_UI);
    rb->lcd_set_backdrop(NULL);
}

enum plugin_status plugin_start(const void *parameter)
{
    bool quit = false;
    (void)parameter;

    atexit(cleanup);
    rb->lcd_set_backdrop(NULL);
    rb->lcd_setfont(FONT_SYSFIXED);
    backlight_ignore_timeout();
    load_simulator_scenario();
    {
        int scene_index;

        for (scene_index = 0;
             scene_index < (int)ARRAYLEN(scenes); ++scene_index)
        {
            if (!ps_scene_valid(&scenes[scene_index]))
                return PLUGIN_ERROR;
        }
    }
    if (!ps_scene_links_valid(village_links, ARRAYLEN(village_links),
                              scenes, ARRAYLEN(scenes)) ||
        !ps_scene_links_reciprocal(village_links,
                                   ARRAYLEN(village_links),
                                   scenes, ARRAYLEN(scenes)) ||
        !ps_scene_entrances_valid(village_entrances,
                                  ARRAYLEN(village_entrances),
                                  scenes, ARRAYLEN(scenes)) ||
        !ps_scene_props_valid(village_props, ARRAYLEN(village_props),
                              scenes, ARRAYLEN(scenes)))
        return PLUGIN_ERROR;
    reset_story(&story);

    while (!quit)
    {
        struct tm *now = rb->get_time();
        const struct palette *p = active_palette(now);
        int action;

        if (director.state == PS_STORY_RUNNING)
            ps_story_update(&director, handle_story_action, reset_story, &story);
        draw_world(p);
        draw_dialogue(p);
        draw_status(p);
        draw_failure(p);
        rb->lcd_update();

        action = pluginlib_getaction(FRAME_TICKS, plugin_contexts, 1);
        switch (action)
        {
            case PLA_CANCEL:
            case PLA_EXIT:
                quit = true;
                break;
            default:
                exit_on_usb(action);
                break;
        }
        frame_number++;
        rb->reset_poweroff_timer();
    }
    return PLUGIN_OK;
}
