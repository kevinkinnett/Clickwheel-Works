#include <assert.h>
#include <stdio.h>

#define POCKETSTEP_ANIM_IMPLEMENTATION
#include "../pocketstep_anim.h"

static const struct ps_anim_sheet sheet = {
    80, 80, 20, 20, 4, 0, { 0, 3, 2, 1 }
};

static void test_validation(void)
{
    struct ps_anim_sheet invalid = sheet;

    assert(ps_anim_sheet_valid(&sheet));
    invalid.frame_width = 0;
    assert(!ps_anim_sheet_valid(&invalid));
    invalid = sheet;
    invalid.frames_per_direction = 5;
    assert(!ps_anim_sheet_valid(&invalid));
    invalid = sheet;
    invalid.facing_rows[2] = 4;
    assert(!ps_anim_sheet_valid(&invalid));
}

static void test_direction_and_idle(void)
{
    struct ps_anim_frame frame;

    assert(ps_anim_select(&sheet, 1, 0, 99, 4, &frame));
    assert(frame.x == 0 && frame.y == 60);
    assert(frame.width == 20 && frame.height == 20);
    assert(frame.index == 0 && frame.row == 3);
}

static void test_distance_frames(void)
{
    struct ps_anim_frame frame;

    assert(ps_anim_select(&sheet, 2, 1, 0, 4, &frame));
    assert(frame.index == 0 && frame.x == 0 && frame.y == 40);
    assert(ps_anim_select(&sheet, 2, 1, 3, 4, &frame));
    assert(frame.index == 0);
    assert(ps_anim_select(&sheet, 2, 1, 4, 4, &frame));
    assert(frame.index == 1 && frame.x == 20);
    assert(ps_anim_select(&sheet, 2, 1, 16, 4, &frame));
    assert(frame.index == 0);
    assert(ps_anim_select(&sheet, 2, 1, 16, 4, &frame));
    assert(frame.index == 0);
}

static void test_invalid_selection(void)
{
    struct ps_anim_frame frame;

    assert(!ps_anim_select(&sheet, -1, 0, 0, 4, &frame));
    assert(!ps_anim_select(&sheet, 4, 0, 0, 4, &frame));
    assert(!ps_anim_select(&sheet, 0, 1, 0, 0, &frame));
    assert(!ps_anim_select(&sheet, 0, 1, -1, 4, &frame));
    assert(!ps_anim_select(&sheet, 0, 0, 0, 4, 0));
}

int main(void)
{
    test_validation();
    test_direction_and_idle();
    test_distance_frames();
    test_invalid_selection();
    puts("PocketStep animation tests passed");
    return 0;
}
