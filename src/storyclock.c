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
#define SCENE_HOUSE 0
#define SCENE_OUTDOOR 1
#define ITEM_KEY 1
#define REGION_ITEM 10
#define REGION_INDOOR_NPC 11
#define REGION_OUTDOOR_NPC 12
#define REGION_BEACON 13
#define DRAW_ACTOR 1
#define DRAW_NPC 2
#define DRAW_ITEM 3
#define DRAW_TALL 4
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
    TILE_BEACON
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
    const char *dialogue_text;
    int failure;
};

static const unsigned char house_tiles[MAP_WIDTH * MAP_HEIGHT] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,3,3,0,0,0,0,0,4,4,0,1,
    1,0,3,3,0,0,0,0,0,4,4,0,1,
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
    1,0,1,1,0,0,0,0,0,1,1,0,1,
    1,0,1,1,0,0,0,0,0,1,1,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,1,0,0,0,0,0,0,1,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const unsigned char outdoor_tiles[MAP_WIDTH * MAP_HEIGHT] = {
    8,8,5,5,5,5,5,5,5,5,8,8,8,
    8,5,5,5,5,5,6,5,5,5,5,5,8,
    5,5,5,9,5,5,6,5,5,5,11,5,5,
    5,5,5,5,5,5,6,5,5,5,5,5,5,
    5,5,5,5,5,5,6,5,5,5,5,5,5,
    8,5,5,5,5,6,6,6,6,6,5,5,8,
    8,5,5,5,5,5,6,5,5,5,5,5,8,
    5,5,7,7,7,5,6,5,5,9,5,5,5,
    5,5,7,7,7,5,6,5,5,5,5,5,5,
    8,5,7,7,7,5,10,5,5,5,5,5,8,
    8,8,5,5,5,5,5,5,5,5,8,8,8
};

static const unsigned char outdoor_blocked[MAP_WIDTH * MAP_HEIGHT] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,1,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,1,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const struct ps_region house_regions[] = {
    { { 9 * TILE_SIZE + MAP_GUTTER, 4 * TILE_SIZE, 16, 16 }, REGION_ITEM },
    { { 2 * TILE_SIZE + MAP_GUTTER, 4 * TILE_SIZE, 16, 16 }, REGION_INDOOR_NPC }
};

static const struct ps_region outdoor_regions[] = {
    { { 3 * TILE_SIZE + MAP_GUTTER, 5 * TILE_SIZE, 16, 16 }, REGION_OUTDOOR_NPC },
    { { 10 * TILE_SIZE + MAP_GUTTER, 2 * TILE_SIZE, 16, 16 }, REGION_BEACON }
};

static const struct ps_scene scenes[] = {
    { MAP_WIDTH, MAP_HEIGHT, house_tiles,
      { MAP_WIDTH, MAP_HEIGHT, house_blocked },
      house_regions, ARRAYLEN(house_regions), { 6, 7 }, 117 },
    { MAP_WIDTH, MAP_HEIGHT, outdoor_tiles,
      { MAP_WIDTH, MAP_HEIGHT, outdoor_blocked },
      outdoor_regions, ARRAYLEN(outdoor_regions), { 6, 8 }, 73102 }
};

static const struct ps_anim_sheet actor_animation = {
    STORY_WALK_SHEET_WIDTH, STORY_WALK_SHEET_HEIGHT,
    STORY_WALK_FRAME_SIZE, STORY_WALK_FRAME_SIZE,
    4, 0, { 0, 3, 2, 1 }
};

static const struct ps_anim_sheet npc_animation = {
    STORY_WALK_SHEET_WIDTH, STORY_WALK_SHEET_HEIGHT,
    STORY_WALK_FRAME_SIZE, STORY_WALK_FRAME_SIZE,
    2, 0, { 3, 3, 3, 3 }
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

static const struct ps_story_action story_actions[] = {
    { PS_STORY_ACTION_WAIT, 24, 0, 0, NULL },
    { PS_STORY_ACTION_SAY, 0, 0, 0, "Morning already? The ember clock is chiming." },
    { PS_STORY_ACTION_WALK, 8, 4, 0, NULL },
    { PS_STORY_ACTION_FACE, PS_GRID_RIGHT, 0, 0, NULL },
    { PS_STORY_ACTION_COLLECT, ITEM_KEY, REGION_ITEM, 0, NULL },
    { PS_STORY_ACTION_SAY, 0, 0, 0, "The ember key is warm. Mira will know why." },
    { PS_STORY_ACTION_WALK, 3, 4, 0, NULL },
    { PS_STORY_ACTION_FACE, PS_GRID_LEFT, 0, 0, NULL },
    { PS_STORY_ACTION_SAY, 1, 0, 0, "Mira: Take it to the hill beacon before the light fades." },
    { PS_STORY_ACTION_WALK, 6, 9, 0, NULL },
    { PS_STORY_ACTION_SCENE, SCENE_OUTDOOR, 6, 8, NULL },
    { PS_STORY_ACTION_WAIT, 18, 0, 0, NULL },
    { PS_STORY_ACTION_WALK, 4, 5, 0, NULL },
    { PS_STORY_ACTION_FACE, PS_GRID_LEFT, 0, 0, NULL },
    { PS_STORY_ACTION_SAY, 2, 0, 0, "Tovin: The river keeps old songs. The beacon keeps promises." },
    { PS_STORY_ACTION_WALK, 9, 2, 0, NULL },
    { PS_STORY_ACTION_FACE, PS_GRID_RIGHT, 0, 0, NULL },
    { PS_STORY_ACTION_SAY, 0, 0, 0, "The key turns. A small new star joins the evening sky." },
    { PS_STORY_ACTION_WAIT, 55, 0, 0, NULL },
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
#endif
}

static void reset_story(void *context)
{
    struct story_state *state = context;

    state->scene = SCENE_HOUSE;
    state->facing = PS_GRID_DOWN;
    state->walk_distance = 0;
    state->item_collected = 0;
    state->dialogue_frames = 0;
    state->dialogue_speaker = 0;
    state->dialogue_text = NULL;
    state->failure = 0;
    state->route.count = 0;
    state->route_index = 0;
    state->route_active = 0;
    state->active_action = -1;
    place_actor(scenes[SCENE_HOUSE].spawn.x,
                scenes[SCENE_HOUSE].spawn.y);
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
    int length = rb->strlen(action->text);

    story.dialogue_speaker = action->a;
    story.dialogue_text = action->text;
    story.dialogue_frames = 28 + length / 2;
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
        source_y = tile == TILE_PATH || tile == TILE_DOOR ? 16 : 0;
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
    else if (tile == TILE_DOOR)
    {
        rb->lcd_bitmap_transparent((const fb_data *)story_door,
                                   x, y, 16, 16);
    }
}

static void draw_tall(int x, int y, int id, const struct palette *p)
{
    if (id == TILE_TREE)
    {
        const unsigned short *tree = outdoor_object_for_palette(
            p, story_tree_day, story_tree_evening, story_tree_night);
        rb->lcd_bitmap_transparent((const fb_data *)tree,
                                   x, y - 16, 16, 32);
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
    const unsigned short *sheet = id == 0 ? story_mira_walk : story_tovin_walk;
    struct ps_anim_frame frame;

    if (!ps_anim_select(&npc_animation, PS_GRID_DOWN, 1,
                        frame_number / 18, 1, &frame))
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
    ps_draw_list_add(list, &value);
}

static void draw_world(const struct palette *p)
{
    const struct ps_scene *scene = &scenes[story.scene];
    struct ps_drawable drawable_storage[32];
    struct ps_draw_list drawables;
    int row;
    int column;
    int actor_left = story.actor.x / PS_ONE + story.actor.width / 2 - 10;
    int actor_top = story.actor.y / PS_ONE + story.actor.height - 20;

    ps_draw_list_init(&drawables, drawable_storage,
                      ARRAYLEN(drawable_storage));
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
        rb->lcd_bitmap_transparent((const fb_data *)story_rug,
                                   tile_x(5), 3 * TILE_SIZE, 48, 48);
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
    else
        queue_drawable(&drawables, DRAW_NPC,
                       tile_x(3) - 2, 5 * TILE_SIZE - 4,
                       6 * TILE_SIZE - 1, 1);
    queue_drawable(&drawables, DRAW_ACTOR, actor_left, actor_top,
                   story.actor.y / PS_ONE + story.actor.height, 0);

    for (row = 0; row < drawables.count; ++row)
    {
        const struct ps_drawable *d = ps_draw_list_get(&drawables, row);
        if (d->kind == DRAW_ACTOR)
            draw_actor(d->x, d->y);
        else if (d->kind == DRAW_NPC)
            draw_npc(d->x, d->y, d->id);
        else if (d->kind == DRAW_ITEM)
            draw_item(d->x, d->y);
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
    static const char *speakers[] = { "LUMA", "MIRA", "TOVIN" };
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

static void draw_clock(const struct tm *now, const struct palette *p)
{
    char clock_text[8];
    char item_text[9];
    int width;
    int height;

    rb->snprintf(clock_text, sizeof(clock_text), "%02d:%02d",
                 now->tm_hour, now->tm_min);
    rb->lcd_getstringsize(clock_text, &width, &height);
    use_color(p->box);
    rb->lcd_fillrect(LCD_WIDTH - width - 10, 3, width + 7, height + 4);
    use_color(p->text);
    rb->lcd_putsxy(LCD_WIDTH - width - 7, 5, clock_text);
    rb->snprintf(item_text, sizeof(item_text), "KEY:%s",
                 story.item_collected ? "YES" : "--");
    use_color(p->box);
    rb->lcd_fillrect(3, 3, 48, height + 4);
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
    if (!ps_scene_valid(&scenes[SCENE_HOUSE]) ||
        !ps_scene_valid(&scenes[SCENE_OUTDOOR]))
        return PLUGIN_ERROR;
    reset_story(&story);
    ps_story_init(&director, story_actions, ARRAYLEN(story_actions), 1);

    while (!quit)
    {
        struct tm *now = rb->get_time();
        const struct palette *p = active_palette(now);
        int action;

        if (director.state == PS_STORY_RUNNING)
            ps_story_update(&director, handle_story_action, reset_story, &story);
        draw_world(p);
        draw_dialogue(p);
        draw_clock(now, p);
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
