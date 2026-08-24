/***************************************************************************
 * Chronolith Orbit, archived first visual direction
 * A generative clock for Rockbox color targets.
 *
 * Copyright (C) 2026 Kevin and OpenAI
 * SPDX-License-Identifier: GPL-2.0-or-later
 ****************************************************************************/

#include "plugin.h"
#include "fixedpoint.h"
#include "lib/helper.h"
#include "lib/pluginlib_actions.h"
#include "lib/pluginlib_exit.h"

#if !defined(HAVE_LCD_COLOR)
#error Chronolith requires a color display
#endif

#define FRAME_TICKS MAX(1, HZ / 20)
#define ORBIT_CX (LCD_WIDTH / 2)
#define ORBIT_CY ((LCD_HEIGHT - 18) / 2)
#define ORBIT_RX (LCD_WIDTH / 2 - 12)
#define ORBIT_RY (LCD_HEIGHT / 2 - 24)
#define PARTICLE_COUNT 18
#define PALETTE_COUNT 5

struct rgb
{
    int r;
    int g;
    int b;
};

struct palette
{
    struct rgb top;
    struct rgb bottom;
    struct rgb dim;
    struct rgb primary;
    struct rgb hot;
    const char *name;
};

struct particle
{
    int x;
    int y;
    int vx;
    int vy;
    int life;
};

static const struct button_mapping *plugin_contexts[] = { pla_main_ctx };

static const struct palette palettes[PALETTE_COUNT] = {
    { {  2,  8, 25}, { 16,  4, 38}, { 35, 37, 76},
      { 78,220,255}, {255,119,198}, "AUTO" },
    { {  2, 18, 33}, {  5, 49, 58}, { 26, 67, 75},
      { 78,236,214}, {255,207, 92}, "TIDE" },
    { { 12,  3, 32}, { 35,  5, 51}, { 67, 31, 91},
      {153,116,255}, {255, 88,169}, "AURORA" },
    { { 23,  4,  7}, { 48, 12,  5}, { 83, 35, 20},
      {255,153, 61}, {255,235,150}, "EMBER" },
    { {  3,  8, 12}, {  7, 17, 20}, { 31, 49, 51},
      {156,221,210}, {232,255,244}, "GLASS" }
};

static struct particle particles[PARTICLE_COUNT];
static int selected_palette;
static int notice_frames;
static int frame_number;
static int previous_minute = -1;
static bool use_24_hour = true;

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

static const struct palette *active_palette(const struct tm *now)
{
    if (selected_palette != 0)
        return &palettes[selected_palette];

    if (now->tm_hour >= 6 && now->tm_hour < 11)
        return &palettes[1];
    if (now->tm_hour >= 11 && now->tm_hour < 18)
        return &palettes[4];
    if (now->tm_hour >= 18 && now->tm_hour < 22)
        return &palettes[3];
    return &palettes[2];
}

static void polar_point(int angle, int rx, int ry, int *x, int *y)
{
    *x = ORBIT_CX + ((fp14_sin(angle) * rx) >> 14);
    *y = ORBIT_CY - ((fp14_sin(angle + 90) * ry) >> 14);
}

static void draw_background(const struct palette *pal)
{
    int y;
    const int band = 8;

    for (y = 0; y < LCD_HEIGHT; y += band)
    {
        struct rgb c = mix_rgb(pal->top, pal->bottom, y, LCD_HEIGHT - 1);
        rb->lcd_set_foreground(color_of(c));
        rb->lcd_fillrect(0, y, LCD_WIDTH, MIN(band, LCD_HEIGHT - y));
    }
}

static void draw_stars(const struct palette *pal)
{
    int i;

    for (i = 0; i < 44; ++i)
    {
        int x = (i * 47 + i * i * 3 + 19) % LCD_WIDTH;
        int y = (i * 29 + i * i * 7 + 11) % (LCD_HEIGHT - 20);
        int pulse = (frame_number + i * 13) % 64;
        int light = pulse < 32 ? pulse : 63 - pulse;
        struct rgb c = mix_rgb(pal->dim, pal->primary, light, 62);

        rb->lcd_set_foreground(color_of(scale_rgb(c, 2 + i % 3, 5)));
        rb->lcd_drawpixel(x, y);
        if ((i % 13) == 0 && light > 23)
        {
            rb->lcd_drawpixel(x - 1, y);
            rb->lcd_drawpixel(x + 1, y);
        }
    }
}

static void draw_hour_lattice(const struct tm *now,
                              const struct palette *pal)
{
    int i;
    int hour = now->tm_hour % 12;

    for (i = 0; i < 12; ++i)
    {
        int inner_x, inner_y, outer_x, outer_y;
        int angle = i * 30;
        struct rgb c = i == hour ? pal->hot : pal->dim;

        polar_point(angle, ORBIT_RX - 23, ORBIT_RY - 17,
                    &inner_x, &inner_y);
        polar_point(angle, ORBIT_RX - 9, ORBIT_RY - 7,
                    &outer_x, &outer_y);

        rb->lcd_set_foreground(color_of(scale_rgb(c, 2, 5)));
        rb->lcd_drawline(inner_x, inner_y, outer_x, outer_y);
        if (i == hour)
        {
            rb->lcd_set_foreground(color_of(c));
            rb->lcd_fillrect(outer_x - 2, outer_y - 2, 5, 5);
        }
        else
        {
            rb->lcd_drawpixel(outer_x, outer_y);
        }
    }
}

static void draw_minute_orbit(const struct tm *now,
                              const struct palette *pal)
{
    int i;

    for (i = 0; i < 60; ++i)
    {
        int x, y;
        struct rgb c;
        polar_point(i * 6, ORBIT_RX, ORBIT_RY, &x, &y);

        if (i < now->tm_min)
            c = mix_rgb(pal->primary, pal->hot, i, 59);
        else
            c = scale_rgb(pal->dim, 2, 5);

        rb->lcd_set_foreground(color_of(c));
        if ((i % 5) == 0)
            rb->lcd_fillrect(x - 1, y - 1, 3, 3);
        else
            rb->lcd_drawpixel(x, y);
    }
}

static void draw_comet(const struct tm *now, const struct palette *pal)
{
    int i;
    int subsecond = (frame_number % 20) * 6 / 20;
    int head_angle = now->tm_sec * 6 + subsecond;

    for (i = 7; i >= 0; --i)
    {
        int x, y;
        struct rgb c = mix_rgb(pal->dim, pal->hot, 8 - i, 8);
        polar_point(head_angle - i * 3, ORBIT_RX - i / 2,
                    ORBIT_RY - i / 3, &x, &y);
        rb->lcd_set_foreground(color_of(c));
        if (i < 2)
            rb->lcd_fillrect(x - 2, y - 2, 5, 5);
        else if (i < 5)
            rb->lcd_fillrect(x - 1, y - 1, 3, 3);
        else
            rb->lcd_drawpixel(x, y);
    }
}

static void draw_segment_rect(int x, int y, int w, int h,
                              struct rgb glow, struct rgb bright,
                              bool lit)
{
    if (!lit)
    {
        rb->lcd_set_foreground(color_of(scale_rgb(glow, 1, 3)));
        rb->lcd_fillrect(x, y, w, h);
        return;
    }

    rb->lcd_set_foreground(color_of(glow));
    rb->lcd_fillrect(x - 1, y - 1, w + 2, h + 2);
    rb->lcd_set_foreground(color_of(bright));
    rb->lcd_fillrect(x, y, w, h);
}

static void draw_digit(int x, int y, int digit, const struct palette *pal)
{
    static const unsigned char masks[10] = {
        0x3f, 0x06, 0x5b, 0x4f, 0x66,
        0x6d, 0x7d, 0x07, 0x7f, 0x6f
    };
    const int w = 17;
    const int h = 29;
    const int t = 3;
    unsigned char mask = masks[digit];
    struct rgb glow = scale_rgb(pal->primary, 2, 5);
    struct rgb bright = mix_rgb(pal->primary, pal->hot, 1, 5);

    draw_segment_rect(x + t, y, w - 2 * t, t,
                      glow, bright, mask & 0x01);
    draw_segment_rect(x + w - t, y + t, t, h / 2 - t,
                      glow, bright, mask & 0x02);
    draw_segment_rect(x + w - t, y + h / 2 + 1, t, h / 2 - t,
                      glow, bright, mask & 0x04);
    draw_segment_rect(x + t, y + h - t, w - 2 * t, t,
                      glow, bright, mask & 0x08);
    draw_segment_rect(x, y + h / 2 + 1, t, h / 2 - t,
                      glow, bright, mask & 0x10);
    draw_segment_rect(x, y + t, t, h / 2 - t,
                      glow, bright, mask & 0x20);
    draw_segment_rect(x + t, y + h / 2 - 1, w - 2 * t, t,
                      glow, bright, mask & 0x40);
}

static void draw_time(const struct tm *now, const struct palette *pal)
{
    int hour = now->tm_hour;
    int x = ORBIT_CX - 43;
    int y = ORBIT_CY - 15;
    struct rgb colon = mix_rgb(pal->primary, pal->hot,
                               now->tm_sec % 2, 1);

    if (!use_24_hour)
    {
        hour %= 12;
        if (hour == 0)
            hour = 12;
    }

    draw_digit(x, y, hour / 10, pal);
    draw_digit(x + 21, y, hour % 10, pal);

    rb->lcd_set_foreground(color_of(colon));
    rb->lcd_fillrect(x + 42, y + 8, 3, 3);
    rb->lcd_fillrect(x + 42, y + 19, 3, 3);

    draw_digit(x + 49, y, now->tm_min / 10, pal);
    draw_digit(x + 70, y, now->tm_min % 10, pal);
}

static void launch_particles(const struct palette *pal)
{
    int i;
    (void)pal;

    for (i = 0; i < PARTICLE_COUNT; ++i)
    {
        int angle = i * 360 / PARTICLE_COUNT;
        particles[i].x = ORBIT_CX * 16;
        particles[i].y = ORBIT_CY * 16;
        particles[i].vx = (fp14_sin(angle) * (12 + i % 5)) >> 14;
        particles[i].vy = -(fp14_sin(angle + 90) * (12 + i % 5)) >> 14;
        particles[i].life = 34 + i % 8;
    }
}

static void draw_particles(const struct palette *pal)
{
    int i;

    for (i = 0; i < PARTICLE_COUNT; ++i)
    {
        int x, y;
        struct rgb c;
        if (particles[i].life <= 0)
            continue;

        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].vy += 1;
        particles[i].life--;
        x = particles[i].x / 16;
        y = particles[i].y / 16;
        c = mix_rgb(pal->dim, pal->hot, particles[i].life, 42);
        rb->lcd_set_foreground(color_of(c));
        rb->lcd_drawpixel(x, y);
        if (particles[i].life > 27)
            rb->lcd_drawpixel(x + 1, y);
    }
}

static void draw_footer(const struct tm *now, const struct palette *pal)
{
    char buffer[32];
    int width, height;

    rb->snprintf(buffer, sizeof(buffer), "%s  %02d.%02d.%04d",
                 use_24_hour ? "24H" : "12H",
                 now->tm_mday, now->tm_mon + 1, now->tm_year + 1900);
    rb->lcd_getstringsize(buffer, &width, &height);
    rb->lcd_set_foreground(color_of(scale_rgb(pal->primary, 3, 5)));
    rb->lcd_putsxy((LCD_WIDTH - width) / 2, LCD_HEIGHT - height - 2, buffer);
}

static void draw_notice(const struct palette *pal)
{
    const char *label;
    int width, height;

    if (notice_frames <= 0)
        return;

    label = selected_palette == 0 ? "AUTO COLOR" : palettes[selected_palette].name;
    rb->lcd_getstringsize(label, &width, &height);
    rb->lcd_set_foreground(color_of(scale_rgb(pal->bottom, 4, 5)));
    rb->lcd_fillrect((LCD_WIDTH - width) / 2 - 4, 3, width + 8, height + 4);
    rb->lcd_set_foreground(color_of(pal->hot));
    rb->lcd_putsxy((LCD_WIDTH - width) / 2, 5, label);
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
    (void)parameter;

    atexit(cleanup);
    rb->lcd_set_backdrop(NULL);
    rb->lcd_setfont(FONT_SYSFIXED);
    backlight_ignore_timeout();
    rb->srand(*rb->current_tick);

    while (!quit)
    {
        struct tm *now = rb->get_time();
        const struct palette *pal = active_palette(now);
        int action;

        if (previous_minute >= 0 && previous_minute != now->tm_min)
            launch_particles(pal);
        previous_minute = now->tm_min;

        draw_background(pal);
        draw_stars(pal);
        draw_hour_lattice(now, pal);
        draw_minute_orbit(now, pal);
        draw_comet(now, pal);
        draw_particles(pal);
        draw_time(now, pal);
        draw_footer(now, pal);
        draw_notice(pal);
        rb->lcd_update();

        action = pluginlib_getaction(FRAME_TICKS, plugin_contexts, 1);
        switch (action)
        {
            case PLA_SCROLL_FWD:
            case PLA_SCROLL_FWD_REPEAT:
                selected_palette = (selected_palette + 1) % PALETTE_COUNT;
                notice_frames = 24;
                break;

            case PLA_SCROLL_BACK:
            case PLA_SCROLL_BACK_REPEAT:
                selected_palette = (selected_palette + PALETTE_COUNT - 1)
                                   % PALETTE_COUNT;
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
