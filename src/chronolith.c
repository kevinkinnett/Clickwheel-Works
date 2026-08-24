/***************************************************************************
 * Chronolith Terminal
 * A NERV-inspired CRT chronograph for Rockbox color targets.
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
#define PROFILE_COUNT 3
#define HISTORY_POINTS 127
#define RETICLE_X 46
#define RETICLE_Y 112

struct rgb
{
    int r;
    int g;
    int b;
};

struct profile
{
    struct rgb phosphor;
    struct rgb bright;
    struct rgb dim;
    struct rgb accent;
    struct rgb alert;
    const char *name;
};

static const struct button_mapping *plugin_contexts[] = { pla_main_ctx };

static const struct profile profiles[PROFILE_COUNT] = {
    { { 24, 196,  82}, {154, 255, 180}, {  7,  55,  28},
      {242, 170,  35}, {239,  45,  35}, "PHOSPHOR" },
    { {220, 138,  26}, {255, 225, 139}, { 66,  39,   5},
      { 64, 213, 110}, {247,  48,  32}, "MAGI AMBER" },
    { {213,  39,  29}, {255, 147, 104}, { 64,  10,   8},
      {237, 183,  42}, {255,  33,  22}, "EMERGENCY" }
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
static int selected_profile;
static int notice_frames;
static bool use_24_hour = true;
static int voltage_history[HISTORY_POINTS];
static int cpu_history[HISTORY_POINTS];
static int history_head;
static int battery_mv;
static int battery_percent;
static int battery_minutes;
static int runtime_minutes;
static int cpu_mhz;
static bool external_power;
static bool battery_charging;
static bool usb_present;
static bool hold_active;

static fb_data color_of(struct rgb c)
{
    return LCD_RGBPACK(c.r, c.g, c.b);
}

static struct rgb scale_rgb(struct rgb c, int amount, int total)
{
    struct rgb out;
    out.r = c.r * amount / total;
    out.g = c.g * amount / total;
    out.b = c.b * amount / total;
    return out;
}

static struct rgb mix_rgb(struct rgb a, struct rgb b, int amount, int total)
{
    struct rgb out;
    out.r = a.r + (b.r - a.r) * amount / total;
    out.g = a.g + (b.g - a.g) * amount / total;
    out.b = a.b + (b.b - a.b) * amount / total;
    return out;
}

static void use_color(struct rgb c)
{
    rb->lcd_set_foreground(color_of(c));
}

static int clamp_int(int value, int minimum, int maximum)
{
    if (value < minimum)
        return minimum;
    if (value > maximum)
        return maximum;
    return value;
}

static void sample_telemetry(bool seed_history)
{
    int i;

    battery_mv = rb->battery_voltage();
    battery_percent = clamp_int(rb->battery_level(), 0, 100);
    battery_minutes = rb->battery_time();
    runtime_minutes = rb->global_status->runtime;
#if (CONFIG_PLATFORM & PLATFORM_NATIVE)
    cpu_mhz = (int)(*rb->cpu_frequency / 1000000L);
#else
    cpu_mhz = CPU_FREQ / 1000000;
#endif
    usb_present = rb->usb_inserted();

#if CONFIG_CHARGING
    external_power = rb->charger_inserted();
#else
    external_power = false;
#endif

#if CONFIG_CHARGING >= CHARGING_MONITOR
    battery_charging = rb->charging_state();
#else
    battery_charging = false;
#endif

#ifdef HAS_BUTTON_HOLD
    hold_active = rb->button_hold();
#else
    hold_active = false;
#endif

    if (seed_history)
    {
        for (i = 0; i < HISTORY_POINTS; ++i)
        {
            voltage_history[i] = battery_mv;
            cpu_history[i] = cpu_mhz;
        }
        history_head = 0;
    }
    else
    {
        voltage_history[history_head] = battery_mv;
        cpu_history[history_head] = cpu_mhz;
        history_head = (history_head + 1) % HISTORY_POINTS;
    }
}

static void draw_circle(int cx, int cy, int radius, struct rgb c)
{
    int x = radius;
    int y = 0;
    int error = 1 - radius;

    use_color(c);
    while (x >= y)
    {
        rb->lcd_drawpixel(cx + x, cy + y);
        rb->lcd_drawpixel(cx + y, cy + x);
        rb->lcd_drawpixel(cx - y, cy + x);
        rb->lcd_drawpixel(cx - x, cy + y);
        rb->lcd_drawpixel(cx - x, cy - y);
        rb->lcd_drawpixel(cx - y, cy - x);
        rb->lcd_drawpixel(cx + y, cy - x);
        rb->lcd_drawpixel(cx + x, cy - y);
        y++;
        if (error < 0)
            error += 2 * y + 1;
        else
        {
            x--;
            error += 2 * (y - x) + 1;
        }
    }
}

static void draw_corner(int x, int y, int sx, int sy, struct rgb c)
{
    use_color(c);
    rb->lcd_drawline(x, y, x + sx * 7, y);
    rb->lcd_drawline(x, y, x, y + sy * 7);
}

static void draw_crt_field(const struct profile *p)
{
    int x;
    int y;
    int scan_y = frame_number % LCD_HEIGHT;
    struct rgb black = { 0, 3, 2 };

    rb->lcd_set_background(color_of(black));
    use_color(black);
    rb->lcd_fillrect(0, 0, LCD_WIDTH, LCD_HEIGHT);

    use_color(scale_rgb(p->dim, 1, 4));
    for (x = 4; x < LCD_WIDTH; x += 18)
        rb->lcd_drawline(x, 18, x, LCD_HEIGHT - 10);
    for (y = 20; y < LCD_HEIGHT - 8; y += 12)
        rb->lcd_drawline(0, y, LCD_WIDTH - 1, y);

    use_color(scale_rgb(p->dim, 1, 3));
    for (y = 1; y < LCD_HEIGHT; y += 4)
        rb->lcd_drawline(0, y, LCD_WIDTH - 1, y);

    use_color(scale_rgb(p->phosphor, 1, 6));
    rb->lcd_drawline(0, scan_y, LCD_WIDTH - 1, scan_y);
    if (scan_y + 1 < LCD_HEIGHT)
        rb->lcd_drawline(0, scan_y + 1, LCD_WIDTH - 1, scan_y + 1);

    draw_corner(1, 1, 1, 1, p->accent);
    draw_corner(LCD_WIDTH - 2, 1, -1, 1, p->accent);
    draw_corner(1, LCD_HEIGHT - 2, 1, -1, p->accent);
    draw_corner(LCD_WIDTH - 2, LCD_HEIGHT - 2, -1, -1, p->accent);
}

static void draw_header(const struct tm *now, const struct profile *p)
{
    char seconds[16];
    int pulse = (frame_number / 5) % 2;

    use_color(p->accent);
    rb->lcd_fillrect(0, 12, LCD_WIDTH, 2);
    rb->lcd_drawrect(0, 0, 43, 11);
    rb->lcd_fillrect(0, 0, 3, 11);
    rb->lcd_putsxy(8, 2, "NERV");

    use_color(p->bright);
    rb->lcd_putsxy(48, 2, "CHRONOGRAPH // MAGI-03");

    rb->snprintf(seconds, sizeof(seconds), "T+%02d.%d",
                 now->tm_sec, (frame_number / 2) % 10);
    use_color(pulse ? p->bright : p->phosphor);
    rb->lcd_putsxy(LCD_WIDTH - 42, 16, seconds);
}

static void draw_matrix_digit(int x, int y, int digit,
                              const struct profile *p)
{
    int row;
    int col;
    const int step = 4;

    for (row = 0; row < 7; ++row)
    {
        for (col = 0; col < 5; ++col)
        {
            bool lit = (digit_rows[digit][row] & (1 << (4 - col))) != 0;
            if (lit)
            {
                use_color(scale_rgb(p->phosphor, 2, 5));
                rb->lcd_fillrect(x + col * step - 1, y + row * step - 1,
                                 5, 5);
                use_color(p->bright);
                rb->lcd_fillrect(x + col * step, y + row * step, 3, 3);
            }
            else
            {
                use_color(scale_rgb(p->dim, 2, 5));
                rb->lcd_drawpixel(x + col * step + 1,
                                  y + row * step + 1);
            }
        }
    }
}

static void draw_time(const struct tm *now, const struct profile *p)
{
    int hour = now->tm_hour;
    int colon_bright = (frame_number / 10) % 2;

    if (!use_24_hour)
    {
        hour %= 12;
        if (hour == 0)
            hour = 12;
    }

    use_color(scale_rgb(p->dim, 4, 5));
    rb->lcd_drawrect(5, 23, 108, 37);
    rb->lcd_drawline(8, 57, 109, 57);

    draw_matrix_digit(9, 27, hour / 10, p);
    draw_matrix_digit(32, 27, hour % 10, p);
    draw_matrix_digit(67, 27, now->tm_min / 10, p);
    draw_matrix_digit(90, 27, now->tm_min % 10, p);

    use_color(colon_bright ? p->accent : scale_rgb(p->accent, 1, 3));
    rb->lcd_fillrect(57, 34, 4, 4);
    rb->lcd_fillrect(57, 46, 4, 4);
}

static void draw_sync_panel(const struct tm *now, const struct profile *p)
{
    int i;
    int filled = now->tm_sec / 2;
    char label[20];

    use_color(scale_rgb(p->dim, 4, 5));
    rb->lcd_drawrect(118, 23, 97, 37);
    use_color(p->accent);
    rb->lcd_putsxy(123, 27, "SYNC SECTOR");

    rb->snprintf(label, sizeof(label), "SEC %02d", now->tm_sec);
    use_color(p->bright);
    rb->lcd_putsxy(123, 39, label);

    for (i = 0; i < 30; ++i)
    {
        int x = 123 + i * 3;
        use_color(i <= filled ? p->phosphor : scale_rgb(p->dim, 2, 5));
        rb->lcd_fillrect(x, 52, 2, i % 5 == 0 ? 5 : 3);
    }
}

static void reticle_point(int angle, int radius, int *x, int *y)
{
    *x = RETICLE_X + ((fp14_sin(angle) * radius) >> 14);
    *y = RETICLE_Y - ((fp14_sin(angle + 90) * radius) >> 14);
}

static void draw_reticle(const struct tm *now, const struct profile *p)
{
    int i;
    int x;
    int y;
    int angle = now->tm_sec * 6 + (frame_number % 20) * 6 / 20;
    int radius = 29;

    draw_circle(RETICLE_X, RETICLE_Y, 31, scale_rgb(p->dim, 4, 5));
    draw_circle(RETICLE_X, RETICLE_Y, 22, scale_rgb(p->dim, 4, 5));
    use_color(scale_rgb(p->dim, 4, 5));
    rb->lcd_drawline(RETICLE_X - 35, RETICLE_Y,
                     RETICLE_X + 35, RETICLE_Y);
    rb->lcd_drawline(RETICLE_X, RETICLE_Y - 35,
                     RETICLE_X, RETICLE_Y + 35);

    for (i = 0; i < 12; ++i)
    {
        int inner_x;
        int inner_y;
        reticle_point(i * 30, 25, &inner_x, &inner_y);
        reticle_point(i * 30, 31, &x, &y);
        if (i == now->tm_hour % 12)
            use_color(p->accent);
        else if (i < (battery_percent * 12 + 99) / 100)
            use_color(p->phosphor);
        else
            use_color(scale_rgb(p->dim, 3, 5));
        rb->lcd_drawline(inner_x, inner_y, x, y);
    }

    for (i = 4; i >= 0; --i)
    {
        reticle_point(angle - i * 5, radius - i, &x, &y);
        use_color(mix_rgb(p->dim, p->bright, 5 - i, 5));
        rb->lcd_drawline(RETICLE_X, RETICLE_Y, x, y);
    }

    use_color(p->alert);
    rb->lcd_fillrect(RETICLE_X - 2, RETICLE_Y - 2, 5, 5);

    if (now->tm_sec < 2)
    {
        int flare = 33 + (frame_number % 10);
        draw_circle(RETICLE_X, RETICLE_Y, flare,
                    scale_rgb(p->alert, 3, 4));
    }

    use_color(p->accent);
    rb->lcd_putsxy(19, 149, "BATT CORE");
}

static int history_y(int value, int minimum, int maximum)
{
    value = clamp_int(value, minimum, maximum);
    return 118 - (value - minimum) * 24 / (maximum - minimum);
}

static void draw_telemetry(const struct profile *p)
{
    int i;
    int grid_x;
    int last_voltage_y = 118;
    int last_cpu_y = 118;
    char line[32];

    use_color(scale_rgb(p->dim, 4, 5));
    rb->lcd_drawrect(84, 68, 131, 66);
    rb->lcd_drawline(84, 89, 214, 89);
    rb->lcd_drawline(84, 100, 214, 100);
    rb->lcd_drawline(84, 112, 214, 112);
    rb->lcd_drawline(84, 128, 214, 128);
    for (grid_x = 96; grid_x < 214; grid_x += 18)
        rb->lcd_drawline(grid_x, 69, grid_x, 133);

    use_color(p->accent);
    rb->lcd_putsxy(89, 72, "SYSTEM BUS // LIVE");
    rb->snprintf(line, sizeof(line), "BAT %03d%%  %d.%02dV",
                 battery_percent, battery_mv / 1000,
                 (battery_mv % 1000) / 10);
    use_color(p->phosphor);
    rb->lcd_putsxy(89, 82, line);

    for (i = 0; i < HISTORY_POINTS; ++i)
    {
        int index = (history_head + i) % HISTORY_POINTS;
        int x = 86 + i;
        int voltage_y = history_y(voltage_history[index], 3300, 4300);
        int cpu_y = history_y(cpu_history[index], 10, 120);

        if (i > 0)
        {
            use_color(p->phosphor);
            rb->lcd_drawline(x - 1, last_voltage_y, x, voltage_y);
            use_color(scale_rgb(p->accent, 4, 5));
            rb->lcd_drawline(x - 1, last_cpu_y, x, cpu_y);
        }
        last_voltage_y = voltage_y;
        last_cpu_y = cpu_y;
    }

    use_color(p->bright);
    rb->lcd_fillrect(211, last_voltage_y - 1, 3, 3);
    use_color(p->accent);
    rb->lcd_fillrect(212, last_cpu_y, 2, 2);

    if (battery_minutes >= 0)
        rb->snprintf(line, sizeof(line), "CPU %03dM ETA %02d:%02d",
                     cpu_mhz, battery_minutes / 60,
                     battery_minutes % 60);
    else
        rb->snprintf(line, sizeof(line), "CPU %03dM ETA --:--", cpu_mhz);
    use_color(p->accent);
    rb->lcd_putsxy(89, 121, line);
}

static void draw_status(const struct tm *now, const struct profile *p)
{
    char date[40];
    bool alert = now->tm_sec >= 55;
    int blink = (frame_number / 4) % 2;

    rb->snprintf(date, sizeof(date), "%04d.%02d.%02d // B%03d // U%d H%d",
                 now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
                 battery_percent, usb_present ? 1 : 0,
                 hold_active ? 1 : 0);
    use_color(p->phosphor);
    rb->lcd_putsxy(5, 162, date);

    if (alert)
    {
        use_color(blink ? p->alert : scale_rgb(p->alert, 2, 5));
        rb->lcd_fillrect(84, 139, 131, 18);
        rb->lcd_set_foreground(LCD_BLACK);
        rb->lcd_putsxy(91, 144, "BOUNDARY EVENT // T-05");
        if (blink)
        {
            use_color(p->alert);
            rb->lcd_drawrect(2, 2, LCD_WIDTH - 4, LCD_HEIGHT - 4);
        }
    }
    else
    {
        const char *power_mode = usb_present ? "USB" :
                                 battery_charging ? "CHG" :
                                 external_power ? "EXT" : "BAT";
        char runtime[32];

        use_color(scale_rgb(p->dim, 4, 5));
        rb->lcd_drawrect(84, 139, 131, 18);
        rb->snprintf(runtime, sizeof(runtime), "UP %02d:%02d // %s",
                     runtime_minutes / 60, runtime_minutes % 60, power_mode);
        use_color(p->bright);
        rb->lcd_putsxy(91, 144, runtime);
    }
}

static void draw_notice(const struct profile *p)
{
    int width;
    int height;

    if (notice_frames <= 0)
        return;

    rb->lcd_getstringsize(p->name, &width, &height);
    use_color(p->accent);
    rb->lcd_fillrect((LCD_WIDTH - width) / 2 - 5, 63,
                     width + 10, height + 6);
    rb->lcd_set_foreground(LCD_BLACK);
    rb->lcd_putsxy((LCD_WIDTH - width) / 2, 66, p->name);
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
    sample_telemetry(true);

    while (!quit)
    {
        struct tm *now = rb->get_time();
        const struct profile *p = &profiles[selected_profile];
        int action;

        draw_crt_field(p);
        draw_header(now, p);
        draw_time(now, p);
        draw_sync_panel(now, p);
        if ((frame_number % 5) == 0)
            sample_telemetry(false);
        draw_reticle(now, p);
        draw_telemetry(p);
        draw_status(now, p);
        draw_notice(p);
        rb->lcd_update();

        action = pluginlib_getaction(FRAME_TICKS, plugin_contexts, 1);
        switch (action)
        {
            case PLA_SCROLL_FWD:
            case PLA_SCROLL_FWD_REPEAT:
                selected_profile = (selected_profile + 1) % PROFILE_COUNT;
                notice_frames = 24;
                break;

            case PLA_SCROLL_BACK:
            case PLA_SCROLL_BACK_REPEAT:
                selected_profile = (selected_profile + PROFILE_COUNT - 1)
                                   % PROFILE_COUNT;
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
