/***************************************************************************
 * Mushroom Clock
 * An animated 8-bit platform clock for Rockbox color targets.
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

#if !defined(HAVE_LCD_COLOR)
#error Mushroom Clock requires a color display
#endif

#define FRAME_TICKS MAX(1, HZ / 20)
#define GROUND_Y 146
#define WORLD_COUNT 5
#define SURFACE_LAYOUT_COUNT 3
#define UNDERGROUND_LAYOUT_COUNT 3
#define MAX_LEVEL_BLOCKS 4

#define POWER_NONE 0
#define POWER_MUSHROOM 1
#define POWER_STAR 2
#define POWER_POISON 3

#define ENEMY_GOOMBA 0
#define ENEMY_TURTLE 1

#define ENEMY_DEATH_NONE 0
#define ENEMY_DEATH_STOMP 1
#define ENEMY_DEATH_STAR 2

#define RUNNER_DEATH_NONE 0
#define RUNNER_DEATH_POISON 1
#define RUNNER_DEATH_ENEMY 2

#define SCENARIO_RANDOM -1
#define SCENARIO_MUSHROOM 0
#define SCENARIO_ALL_COINS 1
#define SCENARIO_STAR 2
#define SCENARIO_HIGH 3
#define SCENARIO_RETREAT 4
#define SCENARIO_STOMP 5
#define SCENARIO_SIDE_HIT 6
#define SCENARIO_UNDERGROUND 7
#define SCENARIO_COUNT 8

#define SOLID_GROUND 1
#define SOLID_PIPE 2
#define SOLID_FIRST_BASE 10
#define SOLID_SECOND_BASE 20

struct rgb
{
    int r;
    int g;
    int b;
};

struct world_palette
{
    struct rgb sky_top;
    struct rgb sky_bottom;
    struct rgb cloud;
    struct rgb hill;
    struct rgb ground;
    struct rgb brick;
    struct rgb pipe;
    struct rgb accent;
    struct rgb shadow;
    const char *name;
};

static const struct button_mapping *plugin_contexts[] = { pla_main_ctx };

static const struct world_palette worlds[WORLD_COUNT] = {
    { { 92, 176, 255}, {151, 218, 255}, {255,255,255}, { 70,190, 77},
      {213,126, 50}, {244,171, 66}, { 39,188, 68}, {255,215, 68},
      { 78, 44, 24}, "AUTO WORLD" },
    { { 92, 176, 255}, {151, 218, 255}, {255,255,255}, { 70,190, 77},
      {213,126, 50}, {244,171, 66}, { 39,188, 68}, {255,215, 68},
      { 78, 44, 24}, "WORLD 1-1" },
    { {  8,  20,  66}, { 18,  45, 105}, {148,180,232}, { 29, 94, 77},
      { 75, 63, 97}, {133,112,154}, { 35,125, 80}, {255,220, 92},
      { 15, 12, 35}, "NIGHT RUN" },
    { { 12,  10,  18}, { 35,  25,  42}, {112,105,120}, { 83, 61, 69},
      { 91, 61, 47}, {169, 93, 55}, { 35,139, 71}, {255,194, 51},
      { 17, 10, 11}, "UNDERGROUND" },
    { {244, 91,  69}, {255,184, 91}, {255,231,190}, { 71,146, 73},
      {177, 86, 43}, {236,139, 54}, { 29,154, 63}, {255,225, 82},
      { 73, 30, 28}, "SUNSET STAGE" }
};

static const unsigned char digit_rows[10][7] = {
    { 0x0e, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0e },
    { 0x04, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x0e },
    { 0x0e, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1f },
    { 0x1e, 0x01, 0x01, 0x0e, 0x01, 0x01, 0x1e },
    { 0x02, 0x06, 0x0a, 0x12, 0x1f, 0x02, 0x02 },
    { 0x1f, 0x10, 0x10, 0x1e, 0x01, 0x01, 0x1e },
    { 0x06, 0x08, 0x10, 0x1e, 0x11, 0x11, 0x0e },
    { 0x1f, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 },
    { 0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e },
    { 0x0e, 0x11, 0x11, 0x0f, 0x01, 0x02, 0x0c }
};

static int frame_number;
static int selected_world;
static int notice_frames;
static bool underground_requested;
static bool underground_cycle;
static int last_surface_signature = -1;
static int last_surface_layout = -1;
static int last_underground_layout = -1;
static int forced_scenario = SCENARIO_RANDOM;
static bool use_24_hour = true;

struct level_state
{
    int runner_x;
    int runner_y;
    int runner_vy;
    bool runner_on_surface;
    bool coin_block_hit;
    bool power_block_hit;
    int coin_block_bounce;
    int power_block_bounce;
    int coin_frames;
    int coin_origin_x;
    int coin_origin_y;
    int powerup_kind;
    int mushroom_mode;
    int mushroom_x;
    int mushroom_y;
    int mushroom_vy;
    int mushroom_direction;
    int powerup_roam_frames;
    int enemy_x;
    int enemy_direction;
    int enemy_kind;
    bool enemy_alive;
    int enemy_min_x;
    int enemy_max_x;
    int enemy_death_frames;
    int enemy_death_style;
    int enemy_death_x;
    int enemy_death_y;
    int enemy_death_vy;
    bool runner_big;
    int star_frames;
    int hurt_frames;
    int death_frames;
    int runner_death_cause;
    int first_x;
    int first_y;
    int first_count;
    int coin_index;
    bool first_broken[MAX_LEVEL_BLOCKS];
    int second_x;
    int second_y;
    int second_count;
    int power_index;
    bool second_broken[MAX_LEVEL_BLOCKS];
    int brick_debris_frames;
    int brick_debris_x;
    int brick_debris_y;
    int pipe_x;
    int pipe_y;
    bool use_high_route;
    bool power_in_first;
    int runner_direction;
    bool use_detour;
    bool detour_started;
    int detour_frames;
    bool flag_grabbed;
    int flag_y;
};

static struct level_state level;
static struct ps_world collision_world;

static fb_data color_of(struct rgb c)
{
    return LCD_RGBPACK(c.r, c.g, c.b);
}

static struct rgb mix_rgb(struct rgb a, struct rgb b, int amount, int total)
{
    struct rgb out;
    out.r = a.r + (b.r - a.r) * amount / total;
    out.g = a.g + (b.g - a.g) * amount / total;
    out.b = a.b + (b.b - a.b) * amount / total;
    return out;
}

static struct rgb scale_rgb(struct rgb c, int amount, int total)
{
    struct rgb out;
    out.r = c.r * amount / total;
    out.g = c.g * amount / total;
    out.b = c.b * amount / total;
    return out;
}

static void use_color(struct rgb c)
{
    rb->lcd_set_foreground(color_of(c));
}

static const struct world_palette *active_world(const struct tm *now)
{
    if (underground_cycle)
        return &worlds[3];
    if (selected_world != 0 && selected_world != 3)
        return &worlds[selected_world];
    if (now->tm_hour >= 6 && now->tm_hour < 17)
        return &worlds[1];
    if (now->tm_hour >= 17 && now->tm_hour < 20)
        return &worlds[4];
    return &worlds[2];
}

static bool in_underground_window(const struct tm *now)
{
    return (now->tm_min == 59 && now->tm_sec >= 10) ||
           (now->tm_min == 0 && now->tm_sec < 30);
}

static bool wants_underground(const struct tm *now)
{
    bool wanted = in_underground_window(now);
#ifdef SIMULATOR
    if (forced_scenario != SCENARIO_RANDOM)
        return forced_scenario == SCENARIO_UNDERGROUND;
    if (selected_world == 3)
        wanted = true;
#endif
    return wanted;
}

static const char *scenario_name(int scenario)
{
    static const char *names[SCENARIO_COUNT] = {
        "MUSHROOM GROWTH", "ALL COINS", "STAR ATTACK", "HIGH ROUTE",
        "RETREAT ROUTE", "CLEAN STOMP", "SIDE HIT", "POISON RUN"
    };

    if (scenario < 0 || scenario >= SCENARIO_COUNT)
        return NULL;
    return names[scenario];
}

static void load_simulator_scenario(void)
{
#ifdef SIMULATOR
    char value[12];
    int fd = rb->open("/mushroomclock-scenario.txt", O_RDONLY);
    int count;

    if (fd < 0)
        return;
    count = rb->read(fd, value, sizeof(value) - 1);
    rb->close(fd);
    if (count <= 0)
        return;
    value[count] = '\0';
    count = rb->atoi(value);
    if (count >= 0 && count < SCENARIO_COUNT)
        forced_scenario = count;
#endif
}

static int change_selected_world(int current, int direction)
{
#ifdef SIMULATOR
    static const int choices[] = { 0, 1, 2, 3, 4 };
#else
    static const int choices[] = { 0, 1, 2, 4 };
#endif
    int i;
    int count = ARRAYLEN(choices);

    for (i = 0; i < count; ++i)
    {
        if (choices[i] == current)
            return choices[(i + direction + count) % count];
    }
    return 0;
}

static void draw_sky(const struct tm *now, const struct world_palette *p)
{
    int y;
    int i;

    rb->lcd_set_background(color_of(p->sky_top));
    for (y = 0; y < GROUND_Y; y += 8)
    {
        struct rgb band = mix_rgb(p->sky_top, p->sky_bottom, y, GROUND_Y);
        use_color(band);
        rb->lcd_fillrect(0, y, LCD_WIDTH, MIN(8, GROUND_Y - y));
    }

    if (p == &worlds[2])
    {
        for (i = 0; i < 26; ++i)
        {
            int x = (i * 41 + i * i * 7 + 13) % LCD_WIDTH;
            int sy = 15 + (i * 23 + i * i * 3) % 95;
            use_color(i % 4 == (frame_number / 8) % 4 ? p->accent : p->cloud);
            rb->lcd_drawpixel(x, sy);
            if (i % 7 == 0)
                rb->lcd_drawpixel(x + 1, sy);
        }
    }

    if (p != &worlds[3])
    {
        int orb_x = 22;
        int orb_y = 43;
        int shimmer = (frame_number / 12) % 2;

        if (p == &worlds[2])
        {
            use_color(p->cloud);
            rb->lcd_fillrect(orb_x + 2, orb_y, 8, 12);
            rb->lcd_fillrect(orb_x, orb_y + 2, 12, 8);
            use_color(p->sky_top);
            rb->lcd_fillrect(orb_x + 6, orb_y, 6, 8);
        }
        else
        {
            use_color(p->accent);
            rb->lcd_fillrect(orb_x + 2, orb_y + 2, 8, 8);
            rb->lcd_fillrect(orb_x + 4, orb_y, 4, 12);
            rb->lcd_fillrect(orb_x, orb_y + 4, 12, 4);
            rb->lcd_fillrect(orb_x - 3 - shimmer, orb_y + 5, 2, 2);
            rb->lcd_fillrect(orb_x + 13 + shimmer, orb_y + 5, 2, 2);
            rb->lcd_fillrect(orb_x + 5, orb_y - 3 - shimmer, 2, 2);
            rb->lcd_fillrect(orb_x + 5, orb_y + 13 + shimmer, 2, 2);
        }
    }

    (void)now;
}

static void draw_cloud(int x, int y, const struct world_palette *p)
{
    use_color(p->cloud);
    rb->lcd_fillrect(x + 5, y, 17, 5);
    rb->lcd_fillrect(x, y + 5, 29, 7);
    use_color(scale_rgb(p->cloud, 4, 5));
    rb->lcd_fillrect(x + 3, y + 12, 23, 2);
}

static void draw_hill(int x, int base_y, int width, int height,
                      const struct world_palette *p)
{
    int row;
    use_color(scale_rgb(p->hill, 3, 4));
    for (row = 0; row < height; row += 3)
    {
        int inset = row * width / (height * 2);
        rb->lcd_fillrect(x + inset, base_y - row - 3,
                         MAX(1, width - inset * 2), 3);
    }
    use_color(p->hill);
    rb->lcd_fillrect(x + width / 2 - 2, base_y - height + 5, 4, 4);
    rb->lcd_fillrect(x + width / 3, base_y - height / 2, 4, 7);
}

static void draw_background_objects(const struct world_palette *p)
{
    int scroll = (frame_number / 4) % 250;
    int cloud_a = 185 - scroll;
    int cloud_b = 55 - scroll;

    if (cloud_a < -35)
        cloud_a += 250;
    if (cloud_b < -35)
        cloud_b += 250;

    if (p != &worlds[3])
    {
        draw_cloud(cloud_a, 63, p);
        draw_cloud(cloud_b, 76, p);
        draw_hill(-14, GROUND_Y, 74, 37, p);
        draw_hill(177, GROUND_Y, 62, 30, p);
    }
    else
    {
        use_color(p->brick);
        rb->lcd_fillrect(0, 61, LCD_WIDTH, 5);
        use_color(p->shadow);
        for (cloud_a = 0; cloud_a < LCD_WIDTH; cloud_a += 16)
            rb->lcd_drawrect(cloud_a, 61, 16, 8);
    }
}

static void draw_brick(int x, int y, const struct world_palette *p)
{
    use_color(p->shadow);
    rb->lcd_fillrect(x, y, 16, 16);
    use_color(p->brick);
    rb->lcd_fillrect(x + 1, y + 1, 14, 14);
    use_color(mix_rgb(p->brick, p->accent, 1, 4));
    rb->lcd_drawline(x + 2, y + 3, x + 13, y + 3);
    use_color(p->shadow);
    rb->lcd_drawline(x + 8, y + 4, x + 8, y + 9);
    rb->lcd_drawline(x + 1, y + 10, x + 14, y + 10);
    rb->lcd_drawline(x + 4, y + 11, x + 4, y + 14);
}

static void draw_brick_piece(int x, int y, const struct world_palette *p)
{
    use_color(p->shadow);
    rb->lcd_fillrect(x, y, 5, 5);
    use_color(p->brick);
    rb->lcd_fillrect(x + 1, y + 1, 3, 3);
}

static void draw_brick_debris(const struct world_palette *p)
{
    int age;
    int fall;
    int x;
    int y;

    if (level.brick_debris_frames <= 0)
        return;

    age = 24 - level.brick_debris_frames;
    fall = age * age / 16;
    x = level.brick_debris_x;
    y = level.brick_debris_y;
    draw_brick_piece(x + 1 - age / 2, y + 1 - age + fall, p);
    draw_brick_piece(x + 10 + age / 2, y + 1 - age + fall, p);
    draw_brick_piece(x + 1 - age, y + 9 - age / 2 + fall, p);
    draw_brick_piece(x + 10 + age, y + 9 - age / 2 + fall, p);
}

static void draw_question_block(int x, int y, bool used,
                                const struct world_palette *p)
{
    struct rgb face = used ? scale_rgb(p->brick, 3, 5) : p->accent;

    use_color(p->shadow);
    rb->lcd_fillrect(x, y, 16, 16);
    use_color(face);
    rb->lcd_fillrect(x + 1, y + 1, 14, 14);
    use_color(used ? scale_rgb(p->cloud, 2, 5) : p->cloud);
    rb->lcd_fillrect(x + 3, y + 2, 10, 2);
    rb->lcd_fillrect(x + 11, y + 4, 2, 4);
    rb->lcd_fillrect(x + 7, y + 7, 5, 2);
    rb->lcd_fillrect(x + 6, y + 9, 2, 3);
    rb->lcd_fillrect(x + 6, y + 13, 2, 2);
    use_color(p->shadow);
    rb->lcd_fillrect(x + 2, y + 2, 2, 2);
    rb->lcd_fillrect(x + 12, y + 12, 2, 2);
}

static void draw_ground(const struct world_palette *p)
{
    int x;
    int y;

    use_color(p->shadow);
    rb->lcd_fillrect(0, GROUND_Y, LCD_WIDTH, LCD_HEIGHT - GROUND_Y);
    use_color(p->ground);
    rb->lcd_fillrect(0, GROUND_Y + 1, LCD_WIDTH, LCD_HEIGHT - GROUND_Y - 1);
    use_color(p->accent);
    rb->lcd_fillrect(0, GROUND_Y + 1, LCD_WIDTH, 3);

    use_color(scale_rgb(p->brick, 4, 5));
    for (y = GROUND_Y + 5; y < LCD_HEIGHT; y += 8)
        rb->lcd_drawline(0, y, LCD_WIDTH - 1, y);
    for (x = -8; x < LCD_WIDTH; x += 16)
    {
        int offset = ((x / 16) & 1) ? 8 : 0;
        rb->lcd_drawline(x + offset, GROUND_Y + 5,
                         x + offset, LCD_HEIGHT - 1);
    }
}

static void draw_pipe(int x, int top, const struct world_palette *p)
{
    use_color(p->shadow);
    rb->lcd_fillrect(x + 3, top + 7, 24, GROUND_Y - top - 7);
    rb->lcd_fillrect(x, top, 30, 11);
    use_color(p->pipe);
    rb->lcd_fillrect(x + 5, top + 8, 20, GROUND_Y - top - 8);
    rb->lcd_fillrect(x + 2, top + 2, 26, 7);
    use_color(mix_rgb(p->pipe, p->cloud, 1, 3));
    rb->lcd_fillrect(x + 7, top + 3, 4, 5);
    rb->lcd_fillrect(x + 9, top + 11, 3, GROUND_Y - top - 12);
    use_color(scale_rgb(p->pipe, 2, 3));
    rb->lcd_fillrect(x + 22, top + 11, 3, GROUND_Y - top - 12);
}

static void draw_coin(int x, int y, const struct world_palette *p)
{
    int narrow = (frame_number / 3) % 4 == 0;
    use_color(p->shadow);
    rb->lcd_fillrect(x + (narrow ? 3 : 1), y, narrow ? 3 : 7, 10);
    use_color(p->accent);
    rb->lcd_fillrect(x + (narrow ? 4 : 2), y + 1, narrow ? 1 : 5, 8);
    if (!narrow)
    {
        use_color(p->cloud);
        rb->lcd_drawline(x + 3, y + 2, x + 3, y + 6);
    }
}

static void draw_mushroom(int x, int y, const struct world_palette *p)
{
    struct rgb red = {225, 47, 38};
    struct rgb cream = {255, 228, 174};

    use_color(p->shadow);
    rb->lcd_fillrect(x + 2, y, 9, 2);
    rb->lcd_fillrect(x, y + 2, 13, 6);
    use_color(red);
    rb->lcd_fillrect(x + 3, y + 1, 7, 2);
    rb->lcd_fillrect(x + 1, y + 3, 11, 4);
    use_color(cream);
    rb->lcd_fillrect(x + 2, y + 3, 3, 3);
    rb->lcd_fillrect(x + 9, y + 3, 2, 3);
    rb->lcd_fillrect(x + 3, y + 7, 7, 5);
    use_color(p->shadow);
    rb->lcd_drawpixel(x + 5, y + 9);
    rb->lcd_drawpixel(x + 8, y + 9);
}

static void draw_poison_mushroom(int x, int y,
                                 const struct world_palette *p)
{
    struct rgb purple = {145, 61, 181};
    struct rgb pale = {205, 194, 216};

    use_color(p->shadow);
    rb->lcd_fillrect(x + 2, y, 9, 2);
    rb->lcd_fillrect(x, y + 2, 13, 6);
    use_color(purple);
    rb->lcd_fillrect(x + 3, y + 1, 7, 2);
    rb->lcd_fillrect(x + 1, y + 3, 11, 4);
    use_color(pale);
    rb->lcd_fillrect(x + 2, y + 3, 3, 3);
    rb->lcd_fillrect(x + 9, y + 3, 2, 3);
    rb->lcd_fillrect(x + 3, y + 7, 7, 5);
    use_color(p->shadow);
    rb->lcd_drawline(x + 4, y + 9, x + 5, y + 10);
    rb->lcd_drawline(x + 5, y + 9, x + 4, y + 10);
    rb->lcd_drawline(x + 8, y + 9, x + 9, y + 10);
    rb->lcd_drawline(x + 9, y + 9, x + 8, y + 10);
}

static void draw_star(int x, int y, const struct world_palette *p)
{
    use_color(p->shadow);
    rb->lcd_fillrect(x + 5, y, 5, 4);
    rb->lcd_fillrect(x, y + 3, 15, 4);
    rb->lcd_fillrect(x + 2, y + 7, 11, 3);
    rb->lcd_fillrect(x + 3, y + 10, 4, 3);
    rb->lcd_fillrect(x + 8, y + 10, 4, 3);
    use_color(p->accent);
    rb->lcd_fillrect(x + 6, y + 1, 3, 4);
    rb->lcd_fillrect(x + 1, y + 4, 13, 2);
    rb->lcd_fillrect(x + 3, y + 6, 9, 3);
    rb->lcd_fillrect(x + 4, y + 9, 3, 3);
    rb->lcd_fillrect(x + 8, y + 9, 3, 3);
    use_color(p->cloud);
    rb->lcd_drawpixel(x + 5, y + 5);
    rb->lcd_drawpixel(x + 9, y + 5);
}

static void draw_goomba(int x, int y, const struct world_palette *p)
{
    struct rgb brown = {139, 76, 37};
    struct rgb cream = {242, 190, 118};
    int step = (frame_number / 4) % 2;

    use_color(p->shadow);
    rb->lcd_fillrect(x + 2, y, 10, 3);
    rb->lcd_fillrect(x, y + 3, 14, 8);
    use_color(brown);
    rb->lcd_fillrect(x + 3, y + 1, 8, 3);
    rb->lcd_fillrect(x + 1, y + 4, 12, 6);
    use_color(cream);
    rb->lcd_fillrect(x + 3, y + 4, 3, 4);
    rb->lcd_fillrect(x + 8, y + 4, 3, 4);
    use_color(p->shadow);
    rb->lcd_drawpixel(x + 5, y + 5);
    rb->lcd_drawpixel(x + 8, y + 5);
    rb->lcd_fillrect(x + 3, y + 9, 8, 2);
    rb->lcd_fillrect(x + (step ? 0 : 8), y + 11, 6, 2);
}

static void draw_turtle(int x, int y, int direction,
                        const struct world_palette *p)
{
    struct rgb green = { 42, 164, 67};
    struct rgb shell = { 23, 105, 49};
    struct rgb skin = {235, 202, 102};
    int step = (frame_number / 4) % 2;

    use_color(p->shadow);
    rb->lcd_fillrect(x + 2, y + 3, 11, 11);
    use_color(shell);
    rb->lcd_fillrect(x + 3, y + 4, 9, 9);
    use_color(green);
    rb->lcd_fillrect(x + 5, y + 5, 6, 7);
    rb->lcd_drawline(x + 4, y + 8, x + 11, y + 8);
    use_color(skin);
    if (direction < 0)
    {
        rb->lcd_fillrect(x, y, 7, 6);
        rb->lcd_fillrect(x + 1, y + 5, 5, 4);
    }
    else
    {
        rb->lcd_fillrect(x + 9, y, 7, 6);
        rb->lcd_fillrect(x + 10, y + 5, 5, 4);
    }
    use_color(p->shadow);
    rb->lcd_drawpixel(x + (direction < 0 ? 2 : 13), y + 2);
    rb->lcd_fillrect(x + (step ? 1 : 4), y + 14, 5, 3);
    rb->lcd_fillrect(x + (step ? 9 : 7), y + 14, 5, 3);
}

static void draw_enemy_death(const struct world_palette *p)
{
    int x = level.enemy_death_x / 16;
    int y = level.enemy_death_y / 16;

    if (level.enemy_death_frames <= 0)
        return;

    if (level.enemy_death_style == ENEMY_DEATH_STOMP)
    {
        use_color(p->shadow);
        rb->lcd_fillrect(x, GROUND_Y - 6, 14, 6);
        if (level.enemy_kind == ENEMY_TURTLE)
        {
            struct rgb shell = { 23, 105, 49};
            use_color(shell);
            rb->lcd_fillrect(x + 2, GROUND_Y - 5, 10, 4);
            use_color(p->cloud);
            rb->lcd_drawline(x + 4, GROUND_Y - 4,
                             x + 9, GROUND_Y - 4);
        }
        else
        {
            struct rgb brown = {139, 76, 37};
            use_color(brown);
            rb->lcd_fillrect(x + 1, GROUND_Y - 5, 12, 3);
            use_color(p->cloud);
            rb->lcd_drawpixel(x + 4, GROUND_Y - 4);
            rb->lcd_drawpixel(x + 9, GROUND_Y - 4);
        }
        return;
    }

    if (level.enemy_kind == ENEMY_TURTLE)
        draw_turtle(x, y, level.enemy_direction, p);
    else
        draw_goomba(x, y, p);

    use_color((level.enemy_death_frames / 2) & 1 ? p->accent : p->cloud);
    rb->lcd_drawline(x - 2, y + 2, x + 1, y + 5);
    rb->lcd_drawline(x - 2, y + 5, x + 1, y + 2);
    rb->lcd_drawline(x + 13, y - 1, x + 16, y + 2);
    rb->lcd_drawline(x + 13, y + 2, x + 16, y - 1);
}

static void draw_runner(int x, int y, bool big, int direction,
                        const struct world_palette *p)
{
    struct rgb red = {222, 48, 38};
    struct rgb blue = { 38, 75, 188};
    struct rgb skin = {255, 188, 112};
    struct rgb hair = { 91, 47, 25};
    int step = (frame_number / 3) % 2;

    if (level.star_frames > 0 && (frame_number / 3) % 2 != 0)
    {
        red = p->accent;
        blue = p->cloud;
        skin = p->cloud;
        hair = p->accent;
    }

    if (big)
    {
        int top = y - 8;

        use_color(red);
        rb->lcd_fillrect(x + 3, top, 9, 3);
        rb->lcd_fillrect(x + 1, top + 3, 13, 2);
        use_color(hair);
        rb->lcd_fillrect(x + (direction < 0 ? 10 : 2), top + 5, 4, 6);
        rb->lcd_fillrect(x + (direction < 0 ? 2 : 11), top + 6, 2, 2);
        use_color(skin);
        rb->lcd_fillrect(x + (direction < 0 ? 2 : 6), top + 5, 7, 6);
        rb->lcd_fillrect(x + (direction < 0 ? 0 : 13), top + 7, 2, 2);
        use_color(p->shadow);
        rb->lcd_drawpixel(x + (direction < 0 ? 3 : 11), top + 6);

        use_color(red);
        rb->lcd_fillrect(x + 3, top + 11, 9, 7);
        rb->lcd_fillrect(x + 1, top + 13, 3, 5);
        rb->lcd_fillrect(x + 12, top + 13, 3, 5);
        use_color(skin);
        rb->lcd_fillrect(x, top + 17, 4, 3);
        rb->lcd_fillrect(x + 12, top + 17, 3, 3);
        use_color(blue);
        rb->lcd_fillrect(x + 4, top + 14, 8, 7);
        rb->lcd_fillrect(x + 4, top + 12, 2, 3);
        rb->lcd_fillrect(x + 10, top + 12, 2, 3);

        use_color(p->shadow);
        if (step)
        {
            rb->lcd_fillrect(x + 2, top + 21, 6, 3);
            rb->lcd_fillrect(x + 10, top + 20, 5, 3);
        }
        else
        {
            rb->lcd_fillrect(x + 4, top + 21, 5, 3);
            rb->lcd_fillrect(x + 9, top + 21, 6, 3);
        }
        return;
    }

    use_color(red);
    rb->lcd_fillrect(x + 2, y, 8, 2);
    rb->lcd_fillrect(x + 1, y + 2, 11, 2);
    use_color(hair);
    rb->lcd_fillrect(x + (direction < 0 ? 8 : 2), y + 4, 3, 5);
    rb->lcd_fillrect(x + (direction < 0 ? 2 : 9), y + 5, 2, 2);
    use_color(skin);
    rb->lcd_fillrect(x + (direction < 0 ? 2 : 5), y + 4, 6, 5);
    rb->lcd_fillrect(x + (direction < 0 ? 0 : 11), y + 6, 2, 2);
    use_color(red);
    rb->lcd_fillrect(x + 3, y + 9, 7, 4);
    rb->lcd_fillrect(x + 1, y + 10, 3, 3);
    use_color(skin);
    rb->lcd_fillrect(x, y + 12, 3, 2);
    use_color(blue);
    rb->lcd_fillrect(x + 4, y + 11, 7, 4);
    if (step)
    {
        rb->lcd_fillrect(x + 2, y + 14, 5, 2);
        rb->lcd_fillrect(x + 9, y + 13, 4, 2);
    }
    else
    {
        rb->lcd_fillrect(x + 4, y + 14, 4, 2);
        rb->lcd_fillrect(x + 8, y + 14, 5, 2);
    }
    use_color(p->shadow);
    rb->lcd_drawpixel(x + (direction < 0 ? 3 : 9), y + 5);

}

static void draw_dead_runner(int x, int y,
                             const struct world_palette *p)
{
    draw_runner(x, y, false, level.runner_direction, p);
    use_color(p->cloud);
    rb->lcd_drawline(x + 6, y + 5, x + 7, y + 6);
    rb->lcd_drawline(x + 7, y + 5, x + 6, y + 6);
    use_color(p->shadow);
    rb->lcd_drawline(x - 2, y + 8, x + 1, y + 11);
    rb->lcd_drawline(x - 2, y + 11, x + 1, y + 8);
}

static bool overlaps(int ax, int ay, int aw, int ah,
                     int bx, int by, int bw, int bh)
{
    return ax < bx + bw && ax + aw > bx &&
           ay < by + bh && ay + ah > by;
}

static bool runner_touches_powerup(int runner_x, int runner_y)
{
    int power_x = level.mushroom_x / 16;
    int power_y = level.mushroom_y / 16;

    return overlaps(runner_x, runner_y, 13, 16,
                    power_x, power_y, 13, 13);
}

static int first_question_x(void)
{
    return level.first_x + level.coin_index * 16;
}

static int second_question_x(void)
{
    return level.second_x + level.power_index * 16;
}

static int power_block_x(void)
{
    return level.power_in_first ? first_question_x() : second_question_x();
}

static int power_block_y(void)
{
    return level.power_in_first ? level.first_y : level.second_y;
}

static int coin_block_x(void)
{
    return level.power_in_first ? second_question_x() : first_question_x();
}

static int coin_block_y(void)
{
    return level.power_in_first ? level.second_y : level.first_y;
}

static void rebuild_collision_world(void)
{
    int i;

    ps_world_clear(&collision_world);
    ps_world_add_solid(&collision_world, SOLID_GROUND,
                       -64, GROUND_Y, LCD_WIDTH + 128,
                       LCD_HEIGHT - GROUND_Y);
    for (i = 0; i < level.first_count; ++i)
    {
        if (!level.first_broken[i])
            ps_world_add_solid(&collision_world, SOLID_FIRST_BASE + i,
                               level.first_x + i * 16,
                               level.first_y, 16, 16);
    }
    for (i = 0; i < level.second_count; ++i)
    {
        if (!level.second_broken[i])
            ps_world_add_solid(&collision_world, SOLID_SECOND_BASE + i,
                               level.second_x + i * 16,
                               level.second_y, 16, 16);
    }
    ps_world_add_solid(&collision_world, SOLID_PIPE,
                       level.pipe_x, level.pipe_y,
                       30, GROUND_Y - level.pipe_y);
}

static void reset_level(void)
{
    bool previous_underground = underground_cycle;
    int layout = 0;
    int i;

    underground_cycle = underground_requested;
    if (previous_underground != underground_cycle)
        notice_frames = 30;

    if (underground_cycle)
    {
        int tries = 0;

        do
        {
            layout = rb->rand() % UNDERGROUND_LAYOUT_COUNT;
            tries++;
        }
        while (layout == last_underground_layout && tries < 8);
        last_underground_layout = layout;

        level.powerup_kind = POWER_POISON;
        level.enemy_kind = ENEMY_TURTLE;
        level.power_in_first = (rb->rand() & 1) != 0;
        level.use_high_route = false;

        if (layout == 0)
        {
            level.first_x = 36;
            level.first_y = 87;
            level.first_count = 4;
            level.second_x = 100;
            level.second_y = 103;
            level.second_count = 3;
            level.pipe_x = 180;
            level.pipe_y = 121;
            level.enemy_min_x = 154;
            level.enemy_max_x = 174;
        }
        else if (layout == 1)
        {
            level.first_x = 44;
            level.first_y = 103;
            level.first_count = 3;
            level.second_x = 104;
            level.second_y = 87;
            level.second_count = 3;
            level.pipe_x = 180;
            level.pipe_y = 121;
            level.enemy_min_x = 150;
            level.enemy_max_x = 172;
        }
        else
        {
            level.first_x = 40;
            level.first_y = 91;
            level.first_count = 3;
            level.second_x = 108;
            level.second_y = 107;
            level.second_count = 3;
            level.pipe_x = 184;
            level.pipe_y = 117;
            level.enemy_min_x = 152;
            level.enemy_max_x = 176;
        }

        level.coin_index = rb->rand() % level.first_count;
        level.power_index = rb->rand() % level.second_count;
    }
    else
    {
        int signature;
        int tries = 0;

#ifdef SIMULATOR
        if (forced_scenario != SCENARIO_RANDOM)
        {
            level.powerup_kind = POWER_NONE;
            level.enemy_kind = ENEMY_GOOMBA;
            layout = 0;
            level.power_in_first = false;
            level.coin_index = 0;
            level.power_index = 1;

            if (forced_scenario == SCENARIO_MUSHROOM)
            {
                level.powerup_kind = POWER_MUSHROOM;
                level.power_in_first = true;
                level.coin_index = 0;
                level.power_index = 1;
            }
            else if (forced_scenario == SCENARIO_ALL_COINS)
            {
                level.enemy_kind = ENEMY_TURTLE;
                layout = 1;
            }
            else if (forced_scenario == SCENARIO_STAR)
            {
                level.powerup_kind = POWER_STAR;
                level.power_in_first = true;
                level.coin_index = 0;
            }
            else if (forced_scenario == SCENARIO_HIGH)
            {
                level.enemy_kind = ENEMY_TURTLE;
                layout = 2;
            }
        }
        else
#endif
        {
            do
            {
                int power_roll = rb->rand() % 4;
                level.powerup_kind = power_roll == 0 ? POWER_NONE :
                                     power_roll == 1 ? POWER_STAR :
                                                       POWER_MUSHROOM;
                level.enemy_kind = rb->rand() % 2;
                layout = rb->rand() % SURFACE_LAYOUT_COUNT;
                level.power_in_first = (rb->rand() & 1) != 0;
                level.coin_index = rb->rand() & 1;
                level.power_index = rb->rand() & 1;
                signature = level.powerup_kind + level.enemy_kind * 4 +
                            layout * 8 + level.power_in_first * 24 +
                            level.coin_index * 48 + level.power_index * 96;
                tries++;
            }
            while ((signature == last_surface_signature ||
                    layout == last_surface_layout) && tries < 8);
            last_surface_signature = signature;
            last_surface_layout = layout;
        }

        if (layout == 0)
        {
            level.first_x = 52;
            level.first_y = 95;
            level.first_count = 2;
            level.second_x = 104;
            level.second_y = 103;
            level.second_count = 2;
            level.pipe_x = 168;
            level.pipe_y = 117;
            level.use_high_route = false;
            level.enemy_min_x = 132;
            level.enemy_max_x = 158;
        }
        else if (layout == 1)
        {
            level.first_x = 48;
            level.first_y = 103;
            level.first_count = 2;
            level.second_x = 104;
            level.second_y = 91;
            level.second_count = 2;
            level.pipe_x = 176;
            level.pipe_y = 121;
            level.use_high_route = false;
            level.enemy_min_x = 136;
            level.enemy_max_x = 164;
        }
        else
        {
            level.first_x = 52;
            level.first_y = 107;
            level.first_count = 2;
            level.second_x = 108;
            level.second_y = 91;
            level.second_count = 2;
            level.pipe_x = 184;
            level.pipe_y = 121;
            level.use_high_route = true;
            level.enemy_min_x = 142;
            level.enemy_max_x = 170;
        }

        if (level.powerup_kind == POWER_STAR && !level.use_high_route)
        {
            int power_x = power_block_x();
            level.enemy_min_x = MAX(level.enemy_min_x, power_x + 24);
            level.enemy_max_x = MIN(level.enemy_max_x, level.pipe_x - 8);
            if (level.enemy_max_x < level.enemy_min_x)
                level.enemy_max_x = level.enemy_min_x;
        }

#ifdef SIMULATOR
        if (forced_scenario != SCENARIO_RANDOM)
        {
            int fixed_enemy_x = layout == 2 ? 150 : 154;

            if (forced_scenario == SCENARIO_STOMP ||
                forced_scenario == SCENARIO_SIDE_HIT)
                fixed_enemy_x = 42;
            level.enemy_min_x = fixed_enemy_x;
            level.enemy_max_x = fixed_enemy_x;
        }
#endif
    }

    level.runner_x = -14 * 16;
    level.runner_y = (GROUND_Y - 16) * 16;
    level.runner_vy = 0;
    level.runner_on_surface = true;
    level.runner_direction = 1;
    level.use_detour = !underground_cycle && !level.use_high_route &&
                       (rb->rand() % 2 == 0);
#ifdef SIMULATOR
    if (forced_scenario != SCENARIO_RANDOM)
        level.use_detour = forced_scenario == SCENARIO_RETREAT;
#endif
    level.detour_started = false;
    level.detour_frames = 0;
    level.coin_block_hit = false;
    level.power_block_hit = false;
#ifdef SIMULATOR
    if (forced_scenario == SCENARIO_RETREAT)
    {
        level.coin_block_hit = true;
        level.power_block_hit = true;
    }
#endif
    level.coin_block_bounce = 0;
    level.power_block_bounce = 0;
    level.coin_frames = 0;
    level.coin_origin_x = coin_block_x();
    level.coin_origin_y = coin_block_y();
    level.mushroom_mode = 0;
    level.mushroom_x = (power_block_x() + 2) * 16;
    level.mushroom_y = power_block_y() * 16;
    level.mushroom_vy = 0;
    level.mushroom_direction = -1;
    level.powerup_roam_frames = 0;
    level.enemy_x = (level.enemy_min_x +
                     rb->rand() % (level.enemy_max_x -
                                   level.enemy_min_x + 1)) * 16;
    level.enemy_direction = rb->rand() & 1 ? 1 : -1;
#ifdef SIMULATOR
    if (forced_scenario != SCENARIO_RANDOM)
        level.enemy_direction = -1;
#endif
    level.enemy_alive = true;
    level.enemy_death_frames = 0;
    level.enemy_death_style = ENEMY_DEATH_NONE;
    level.runner_big = false;
    level.star_frames = 0;
    level.hurt_frames = 0;
    level.death_frames = 0;
    level.runner_death_cause = RUNNER_DEATH_NONE;
    for (i = 0; i < MAX_LEVEL_BLOCKS; ++i)
    {
        level.first_broken[i] = false;
        level.second_broken[i] = false;
    }
    level.brick_debris_frames = 0;
    level.flag_grabbed = false;
    level.flag_y = 66;
    rebuild_collision_world();
}

static void launch_runner(int velocity)
{
    if (level.runner_on_surface)
    {
        level.runner_vy = velocity;
        level.runner_on_surface = false;
    }
}

static void hit_coin_block(void)
{
    if (level.coin_block_hit)
        return;

    level.coin_block_hit = true;
    level.coin_block_bounce = 12;
    level.coin_frames = 36;
    level.coin_origin_x = coin_block_x();
    level.coin_origin_y = coin_block_y();
}

static void hit_power_block(void)
{
    if (level.power_block_hit)
        return;

    level.power_block_hit = true;
    level.power_block_bounce = 12;
    if (level.powerup_kind == POWER_NONE)
    {
        level.coin_frames = 36;
        level.coin_origin_x = power_block_x();
        level.coin_origin_y = power_block_y();
        return;
    }
    level.mushroom_mode = 1;
    level.mushroom_x = (power_block_x() + 2) * 16;
    level.mushroom_y = power_block_y() * 16;
    level.mushroom_vy = 0;
    level.mushroom_direction = -1;
}

static void break_brick(int solid_id)
{
    int index;

    if (solid_id >= SOLID_FIRST_BASE &&
        solid_id < SOLID_FIRST_BASE + level.first_count)
    {
        index = solid_id - SOLID_FIRST_BASE;
        if (index == level.coin_index || level.first_broken[index])
            return;
        level.first_broken[index] = true;
        level.brick_debris_x = level.first_x + index * 16;
        level.brick_debris_y = level.first_y;
    }
    else if (solid_id >= SOLID_SECOND_BASE &&
             solid_id < SOLID_SECOND_BASE + level.second_count)
    {
        index = solid_id - SOLID_SECOND_BASE;
        if (index == level.power_index || level.second_broken[index])
            return;
        level.second_broken[index] = true;
        level.brick_debris_x = level.second_x + index * 16;
        level.brick_debris_y = level.second_y;
    }
    else
        return;

    level.brick_debris_frames = 24;
    rebuild_collision_world();
}

static void update_enemy(void)
{
    if (!level.enemy_alive)
        return;

    level.enemy_x += level.enemy_direction * 6;
    if (level.enemy_x <= level.enemy_min_x * 16)
    {
        level.enemy_x = level.enemy_min_x * 16;
        level.enemy_direction = 1;
    }
    else if (level.enemy_x >= level.enemy_max_x * 16)
    {
        level.enemy_x = level.enemy_max_x * 16;
        level.enemy_direction = -1;
    }
}

static void defeat_enemy(int style)
{
    int height = level.enemy_kind == ENEMY_TURTLE ? 17 : 13;

    level.enemy_alive = false;
    level.enemy_death_style = style;
    level.enemy_death_frames = style == ENEMY_DEATH_STAR ? 30 : 18;
    level.enemy_death_x = level.enemy_x;
    level.enemy_death_y = (GROUND_Y - height) * 16;
    level.enemy_death_vy = style == ENEMY_DEATH_STAR ? -52 : 0;
}

static void update_enemy_death(void)
{
    if (level.enemy_death_frames <= 0)
        return;

    if (level.enemy_death_style == ENEMY_DEATH_STAR)
    {
        level.enemy_death_x += level.enemy_direction * 12;
        level.enemy_death_vy += 4;
        level.enemy_death_y += level.enemy_death_vy;
    }
    level.enemy_death_frames--;
}

static void update_mushroom(void)
{
    struct ps_body powerup;
    struct ps_move_result movement;

    if (level.mushroom_mode == 0)
        return;

    if (level.mushroom_mode == 1)
    {
        int block_y = power_block_y();
        level.mushroom_y -= (level.powerup_kind == POWER_POISON ||
                             level.powerup_kind == POWER_STAR) ? 16 : 8;
        if (level.mushroom_y <= (block_y - 12) * 16)
        {
            level.mushroom_y = (block_y - 12) * 16;
            level.mushroom_mode = 2;
            if (level.powerup_kind == POWER_STAR)
            {
                level.mushroom_direction = 1;
                level.powerup_roam_frames = 24;
            }
            else if (level.powerup_kind == POWER_POISON)
            {
                level.mushroom_direction = 1;
                level.powerup_roam_frames = 12;
            }
        }
        return;
    }

    powerup.x = level.mushroom_x;
    powerup.y = level.mushroom_y;
    powerup.vx = level.mushroom_direction *
                 (level.powerup_kind == POWER_STAR &&
                  level.powerup_roam_frames > 0 ? 32 : 8);
    powerup.vy = level.mushroom_vy;
    powerup.width = 13;
    powerup.height = 12;
    ps_apply_gravity(&powerup, 3, 48);
    movement = ps_move(&collision_world, &powerup);

    if (movement.hit_floor && level.powerup_kind == POWER_STAR)
        powerup.vy = -44;
    if (movement.hit_left || movement.hit_right)
        level.mushroom_direction = -level.mushroom_direction;
    if (level.powerup_kind == POWER_STAR &&
        powerup.x <= (level.first_x - 20) * PS_ONE)
    {
        powerup.x = (level.first_x - 20) * PS_ONE;
        level.mushroom_direction = 1;
    }
    else if (level.powerup_kind == POWER_STAR &&
             powerup.x >= (level.pipe_x - 18) * PS_ONE)
    {
        powerup.x = (level.pipe_x - 18) * PS_ONE;
        level.mushroom_direction = -1;
    }

    level.mushroom_x = powerup.x;
    level.mushroom_y = powerup.y;
    level.mushroom_vy = powerup.vy;
}

static void update_runner(void)
{
    struct ps_body runner;
    struct ps_body previous_runner;
    struct ps_body enemy;
    struct ps_move_result movement;
    int runner_x = level.runner_x / 16;
    int enemy_x = level.enemy_x / 16;
    int new_x;
    int new_y;
    int new_bottom;
    int impact_vy;
    bool hold_for_powerup = false;

    if (level.death_frames > 0)
    {
        level.runner_vy += 5;
        level.runner_y += level.runner_vy;
        level.death_frames--;
        if (level.death_frames == 0 ||
            level.runner_y > (LCD_HEIGHT + 24) * 16)
            reset_level();
        return;
    }

    if (level.detour_frames > 0)
    {
        level.detour_frames--;
        if (level.detour_frames == 0)
            level.runner_direction = 1;
    }

    if (level.runner_on_surface && level.runner_direction > 0)
    {
        int coin_x = coin_block_x();
        int power_x = power_block_x();

        if (level.use_detour && !level.detour_started &&
            level.enemy_alive && level.star_frames == 0 &&
            level.runner_y / 16 == GROUND_Y - 16 &&
            runner_x >= enemy_x - 38 && runner_x < enemy_x - 29)
        {
            level.detour_started = true;
            level.detour_frames = 24;
            level.runner_direction = -1;
        }
        else if (level.use_high_route &&
                 runner_x >= level.first_x - 27 &&
                 runner_x < level.first_x - 16)
            launch_runner(-76);
        else if (level.use_high_route &&
                 level.runner_y / 16 == level.first_y - 16 &&
                 runner_x >= level.first_x + level.first_count * 16 - 20 &&
                 runner_x < level.first_x + level.first_count * 16 - 8)
            launch_runner(-58);
#ifdef SIMULATOR
        else if (forced_scenario == SCENARIO_MUSHROOM &&
                 level.runner_big && !level.first_broken[1] &&
                 runner_x >= level.first_x - 1 &&
                 runner_x < level.first_x + 11)
            launch_runner(-60);
#endif
        else if (!level.coin_block_hit &&
                 runner_x >= coin_x - 20 && runner_x < coin_x - 8)
            launch_runner(-64);
        else if (!level.power_block_hit &&
                 runner_x >= power_x - 13 && runner_x < power_x - 1)
            launch_runner(-60);
        else if (level.enemy_alive && level.star_frames == 0 &&
#ifdef SIMULATOR
                 forced_scenario != SCENARIO_SIDE_HIT &&
#endif
                 runner_x >= enemy_x -
                             (level.detour_started ? 21 : 27) &&
                 runner_x < enemy_x -
                            (level.detour_started ? 12 : 16))
            launch_runner(-54);
        else if (runner_x >= level.pipe_x - 18 &&
                 runner_x < level.pipe_x - 5)
            launch_runner(-66);
    }

    runner.x = level.runner_x;
    runner.y = level.runner_y - (level.runner_big ? 8 * PS_ONE : 0);
    if (level.mushroom_mode == 2 && level.powerup_roam_frames == 0)
    {
        int power_center = level.mushroom_x / PS_ONE + 6;
        int runner_center = runner.x / PS_ONE + 6;
        int delta = power_center - runner_center;

        if (delta > 2)
            level.runner_direction = 1;
        else if (delta < -2)
            level.runner_direction = -1;
        else
            hold_for_powerup = true;
    }
    runner.vx = level.runner_direction * PS_ONE;
    if (level.mushroom_mode == 1 || level.powerup_roam_frames > 0 ||
        hold_for_powerup)
        runner.vx = 0;
    runner.vy = level.runner_vy;
    runner.width = 13;
    runner.height = level.runner_big ? 24 : 16;
    ps_apply_gravity(&runner, 4, 72);
    impact_vy = runner.vy;
    previous_runner = runner;
    movement = ps_move(&collision_world, &runner);

    level.runner_x = runner.x;
    level.runner_y = runner.y + (level.runner_big ? 8 * PS_ONE : 0);
    level.runner_vy = runner.vy;
    level.runner_on_surface = movement.hit_floor != 0;

    if (movement.hit_ceiling)
    {
        int first_question = SOLID_FIRST_BASE + level.coin_index;
        int second_question = SOLID_SECOND_BASE + level.power_index;

        level.runner_vy = 12;
        if (level.runner_big &&
            movement.vertical_id != first_question &&
            movement.vertical_id != second_question)
            break_brick(movement.vertical_id);
        else if (movement.vertical_id == first_question)
        {
            if (level.power_in_first)
                hit_power_block();
            else
                hit_coin_block();
        }
        else if (movement.vertical_id == second_question)
        {
            if (level.power_in_first)
                hit_coin_block();
            else
                hit_power_block();
        }
    }

    if (movement.hit_right && movement.horizontal_id == SOLID_PIPE &&
        level.runner_direction > 0)
    {
        level.runner_vy = -66;
        level.runner_on_surface = false;
    }

    new_x = level.runner_x / PS_ONE;
    new_y = level.runner_y / PS_ONE;
    new_bottom = new_y + 16;

    if (level.mushroom_mode != 0 && level.powerup_roam_frames == 0 &&
        runner_touches_powerup(level.runner_x / 16,
                               level.runner_y / 16))
    {
        level.mushroom_mode = 0;
        level.runner_direction = 1;
        if (level.powerup_kind == POWER_POISON)
        {
            level.death_frames = 60;
            level.runner_death_cause = RUNNER_DEATH_POISON;
            level.runner_vy = -76;
            level.runner_on_surface = false;
        }
        else if (level.powerup_kind == POWER_MUSHROOM)
            level.runner_big = true;
        else if (level.powerup_kind == POWER_STAR)
            level.star_frames = 120;
    }

    if (level.enemy_alive)
    {
        int enemy_height = level.enemy_kind == ENEMY_TURTLE ? 17 : 13;
        int enemy_top = GROUND_Y - enemy_height;
        runner.y = level.runner_y - (level.runner_big ? 8 * PS_ONE : 0);
        runner.vy = impact_vy;
        runner.height = level.runner_big ? 24 : 16;
        enemy.x = level.enemy_x;
        enemy.y = enemy_top * PS_ONE;
        enemy.vx = 0;
        enemy.vy = 0;
        enemy.width = 14;
        enemy.height = enemy_height;

        if (level.star_frames > 0 && ps_overlap(&runner, &enemy))
            defeat_enemy(ENEMY_DEATH_STAR);
        else if (ps_crossed_top(&previous_runner, &runner, &enemy, 2))
        {
            defeat_enemy(ENEMY_DEATH_STOMP);
            level.runner_y = (enemy_top - 16) * PS_ONE;
            level.runner_vy = -43;
            level.runner_on_surface = false;
        }
        else if (ps_overlap(&runner, &enemy) && level.hurt_frames == 0)
        {
            if (level.runner_big)
            {
                level.runner_big = false;
                level.hurt_frames = 32;
                level.runner_vy = -48;
                level.runner_on_surface = false;
            }
            else
            {
                level.death_frames = 60;
                level.runner_death_cause = RUNNER_DEATH_ENEMY;
                level.runner_vy = -76;
                level.runner_on_surface = false;
            }
        }
    }

    if (!underground_cycle && !level.flag_grabbed &&
        new_x + 13 >= 204 && new_x < 214 &&
        new_bottom > 63)
        level.flag_grabbed = true;

    if (level.runner_x > (LCD_WIDTH + 15) * 16)
        reset_level();
}

static void update_level(void)
{
    update_enemy();
    update_enemy_death();
    update_mushroom();
    update_runner();

    if (level.coin_block_bounce > 0)
        level.coin_block_bounce--;
    if (level.power_block_bounce > 0)
        level.power_block_bounce--;
    if (level.coin_frames > 0)
        level.coin_frames--;
    if (level.powerup_roam_frames > 0)
        level.powerup_roam_frames--;
    if (level.brick_debris_frames > 0)
        level.brick_debris_frames--;
    if (level.star_frames > 0)
        level.star_frames--;
    if (level.hurt_frames > 0)
        level.hurt_frames--;
    if (level.flag_grabbed && level.flag_y < 128)
        level.flag_y = MIN(128, level.flag_y + 2);
}

static void draw_powerup(const struct world_palette *p)
{
    int x = level.mushroom_x / 16;
    int y = level.mushroom_y / 16;

    if (level.powerup_kind == POWER_STAR)
        draw_star(x, y, p);
    else if (level.powerup_kind == POWER_POISON)
        draw_poison_mushroom(x, y, p);
    else
        draw_mushroom(x, y, p);
}

static void draw_level_action(const struct world_palette *p)
{
    int first_draw_y = level.first_y;
    int second_draw_y = level.second_y;
    int runner_x = level.runner_x / 16;
    int runner_y = level.runner_y / 16;
    int i;

    if (level.coin_block_bounce > 0)
    {
        int progress = 12 - level.coin_block_bounce;
        int bounce = progress < 6 ? progress / 2 : (12 - progress) / 2;
        if (level.power_in_first)
            second_draw_y -= bounce;
        else
            first_draw_y -= bounce;
    }

    if (level.power_block_bounce > 0)
    {
        int progress = 12 - level.power_block_bounce;
        int bounce = progress < 6 ? progress / 2 : (12 - progress) / 2;
        if (level.power_in_first)
            first_draw_y -= bounce;
        else
            second_draw_y -= bounce;
    }

    if (level.mushroom_mode == 1)
        draw_powerup(p);

    for (i = 0; i < level.first_count; ++i)
    {
        int x = level.first_x + i * 16;
        if (level.first_broken[i])
            continue;
        else if (i == level.coin_index)
            draw_question_block(x, first_draw_y,
                                level.power_in_first ?
                                level.power_block_hit :
                                level.coin_block_hit, p);
        else
            draw_brick(x, level.first_y, p);
    }
    for (i = 0; i < level.second_count; ++i)
    {
        int x = level.second_x + i * 16;
        if (level.second_broken[i])
            continue;
        else if (i == level.power_index)
            draw_question_block(x, second_draw_y,
                                level.power_in_first ?
                                level.coin_block_hit :
                                level.power_block_hit, p);
        else
            draw_brick(x, level.second_y, p);
    }
    draw_brick_debris(p);
    draw_pipe(level.pipe_x, level.pipe_y, p);

    if (level.coin_frames > 0)
    {
        int progress = 36 - level.coin_frames;
        int height = progress < 18 ? progress : 36 - progress;
        draw_coin(level.coin_origin_x + 4,
                  level.coin_origin_y - 11 - height, p);
    }

    if (level.mushroom_mode == 2)
        draw_powerup(p);

    if (level.enemy_alive)
    {
        if (level.enemy_kind == ENEMY_TURTLE)
            draw_turtle(level.enemy_x / 16, GROUND_Y - 17,
                        level.enemy_direction, p);
        else
            draw_goomba(level.enemy_x / 16, GROUND_Y - 13, p);
    }
    else
        draw_enemy_death(p);

    if (runner_x < LCD_WIDTH + 13)
    {
        if (level.death_frames > 0)
            draw_dead_runner(runner_x, runner_y, p);
        else if (level.hurt_frames == 0 ||
                 (level.hurt_frames / 2) % 2 == 0)
            draw_runner(runner_x, runner_y, level.runner_big,
                        level.runner_direction, p);

        if (level.star_frames > 0)
        {
            int phase = (frame_number / 2) & 1;
            use_color(phase ? p->accent : p->cloud);
            rb->lcd_drawpixel(runner_x - 2, runner_y + 3);
            rb->lcd_drawpixel(runner_x + 15, runner_y + 7);
            rb->lcd_drawpixel(runner_x + 2, runner_y - 3);
            rb->lcd_drawpixel(runner_x + 11, runner_y + 18);
        }
    }

    if (level.death_frames > 0)
    {
        const char *message = level.runner_death_cause ==
                              RUNNER_DEATH_POISON ? "POISONED!" : "MISS!";
        int width;
        int height;
        rb->lcd_getstringsize(message, &width, &height);
        use_color(p->shadow);
        rb->lcd_fillrect((LCD_WIDTH - width) / 2 - 5, 65,
                         width + 10, height + 6);
        use_color(p->accent);
        rb->lcd_drawrect((LCD_WIDTH - width) / 2 - 4, 66,
                         width + 8, height + 4);
        use_color(p->cloud);
        rb->lcd_putsxy((LCD_WIDTH - width) / 2, 69, message);
    }
}

static void draw_flag(const struct world_palette *p)
{
    int wave = (frame_number / 5) % 2;
    int flag_x = 198 - wave;

    use_color(p->cloud);
    rb->lcd_fillrect(211, 63, 2, GROUND_Y - 63);
    rb->lcd_fillrect(208, 61, 8, 3);
    use_color(p->shadow);
    rb->lcd_fillrect(flag_x, level.flag_y, 13, 9);
    use_color(p->accent);
    rb->lcd_fillrect(flag_x + 1, level.flag_y + 1, 11, 7);
    use_color(p->cloud);
    rb->lcd_fillrect(flag_x + 4, level.flag_y + 3, 3, 3);
}

static void draw_big_digit(int x, int y, int digit,
                           const struct world_palette *p)
{
    int row;
    int col;
    const int step = 4;
    const struct rgb blue_shadow = { 35, 61, 156 };

    for (row = 0; row < 7; ++row)
    {
        for (col = 0; col < 5; ++col)
        {
            if (digit_rows[digit][row] & (1 << (4 - col)))
            {
                use_color(p->shadow);
                rb->lcd_fillrect(x + col * step - 1,
                                 y + row * step - 1, 5, 5);
                use_color(blue_shadow);
                rb->lcd_fillrect(x + col * step + 1,
                                 y + row * step + 1, 3, 3);
                use_color(p->cloud);
                rb->lcd_fillrect(x + col * step,
                                 y + row * step, 3, 3);
                use_color(p->accent);
                rb->lcd_drawpixel(x + col * step,
                                  y + row * step);
            }
        }
    }
}

static void draw_clock(const struct tm *now, const struct world_palette *p)
{
    int hour = now->tm_hour;
    int colon_on = (frame_number / 10) % 2;

    if (!use_24_hour)
    {
        hour %= 12;
        if (hour == 0)
            hour = 12;
    }

    use_color(p->shadow);
    rb->lcd_fillrect(94, 13, 33, 10);
    use_color(p->accent);
    rb->lcd_drawrect(95, 14, 31, 8);
    use_color(p->cloud);
    rb->lcd_putsxy(99, 14, "TIME");

    draw_big_digit(51, 27, hour / 10, p);
    draw_big_digit(75, 27, hour % 10, p);
    draw_big_digit(112, 27, now->tm_min / 10, p);
    draw_big_digit(136, 27, now->tm_min % 10, p);

    use_color(colon_on ? p->accent : scale_rgb(p->cloud, 3, 5));
    rb->lcd_fillrect(101, 35, 5, 5);
    rb->lcd_fillrect(101, 47, 5, 5);
}

static void draw_battery_gauge(int x, int y, int battery,
                               const struct world_palette *p)
{
    int fill = MAX(0, MIN(100, battery)) * 13 / 100;
    struct rgb low = { 222, 48, 38 };

    use_color(p->cloud);
    rb->lcd_drawrect(x, y, 17, 8);
    rb->lcd_fillrect(x + 17, y + 2, 2, 4);
    use_color(p->shadow);
    rb->lcd_fillrect(x + 2, y + 2, 13, 4);
    if (fill > 0)
    {
        use_color(battery < 20 ? low : p->pipe);
        rb->lcd_fillrect(x + 2, y + 2, fill, 4);
    }
}

static void draw_hud(const struct tm *now, const struct world_palette *p)
{
    char right[28];
    char percent[8];
    int battery = rb->battery_level();

    rb->snprintf(percent, sizeof(percent), "%02d%%", MAX(0, battery));
    rb->snprintf(right, sizeof(right), "WORLD %02d-%02d SEC %02d",
                 now->tm_mon + 1, now->tm_mday, now->tm_sec);

    use_color(p->shadow);
    rb->lcd_fillrect(0, 0, LCD_WIDTH, 12);
    use_color(p->cloud);
    rb->lcd_putsxy(4, 2, "MARIO");
    draw_battery_gauge(39, 2, battery, p);
    rb->lcd_putsxy(62, 2, percent);
    rb->lcd_putsxy(110, 2, right);
}

static void draw_notice(const struct world_palette *p)
{
    const char *label;
    int width;
    int height;

    if (notice_frames <= 0)
        return;

    if (forced_scenario != SCENARIO_RANDOM)
        label = scenario_name(forced_scenario);
    else if (underground_cycle)
        label = worlds[3].name;
    else
        label = selected_world == 0 ? worlds[0].name : p->name;
    rb->lcd_getstringsize(label, &width, &height);
    use_color(p->shadow);
    rb->lcd_fillrect((LCD_WIDTH - width) / 2 - 5, 62,
                     width + 10, height + 6);
    use_color(p->accent);
    rb->lcd_drawrect((LCD_WIDTH - width) / 2 - 4, 63,
                     width + 8, height + 4);
    use_color(p->cloud);
    rb->lcd_putsxy((LCD_WIDTH - width) / 2, 66, label);
    notice_frames--;
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
    struct tm *start_time;
    (void)parameter;

    atexit(cleanup);
    rb->lcd_set_backdrop(NULL);
    rb->lcd_setfont(FONT_SYSFIXED);
    backlight_ignore_timeout();
    start_time = rb->get_time();
    load_simulator_scenario();
    rb->srand((unsigned int)*rb->current_tick ^
              (unsigned int)(start_time->tm_hour * 3600 +
                             start_time->tm_min * 60 +
                             start_time->tm_sec));
    last_surface_signature = -1;
    last_surface_layout = -1;
    last_underground_layout = -1;
    underground_requested = wants_underground(start_time);
    if (forced_scenario != SCENARIO_RANDOM)
        notice_frames = 36;
    reset_level();

    while (!quit)
    {
        struct tm *now = rb->get_time();
        const struct world_palette *p;
        int action;

        underground_requested = wants_underground(now);
        update_level();
        p = active_world(now);
        draw_sky(now, p);
        draw_background_objects(p);
        draw_ground(p);
        draw_level_action(p);
        if (!underground_cycle)
            draw_flag(p);
        draw_clock(now, p);
        draw_hud(now, p);
        draw_notice(p);
        rb->lcd_update();

        action = pluginlib_getaction(FRAME_TICKS, plugin_contexts, 1);
        switch (action)
        {
            case PLA_SCROLL_FWD:
            case PLA_SCROLL_FWD_REPEAT:
                selected_world = change_selected_world(selected_world, 1);
                notice_frames = 24;
                break;

            case PLA_SCROLL_BACK:
            case PLA_SCROLL_BACK_REPEAT:
                selected_world = change_selected_world(selected_world, -1);
                notice_frames = 24;
                break;

            case PLA_SELECT:
            case PLA_LEFT:
            case PLA_RIGHT:
                use_24_hour = !use_24_hour;
                notice_frames = 24;
                break;

            case PLA_UP:
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
