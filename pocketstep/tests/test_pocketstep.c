#include <assert.h>
#include <stdio.h>

#define POCKETSTEP_IMPLEMENTATION
#include "../pocketstep.h"

static void test_floor_landing(void)
{
    struct ps_world world;
    struct ps_body body = { 20 * PS_ONE, 20 * PS_ONE,
                            0, 120 * PS_ONE, 12, 16 };
    struct ps_move_result result;

    ps_world_clear(&world);
    assert(ps_world_add_solid(&world, 7, 0, 100, 220, 76));
    result = ps_move(&world, &body);
    assert(result.hit_floor);
    assert(result.vertical_id == 7);
    assert(body.y == 84 * PS_ONE);
}

static void test_offscreen_entry_can_land(void)
{
    struct ps_world world;
    struct ps_body body = { -14 * PS_ONE, 130 * PS_ONE,
                            PS_ONE, 4, 13, 16 };
    struct ps_move_result result;

    ps_world_clear(&world);
    assert(ps_world_add_solid(&world, 7, -64, 146, 348, 30));
    result = ps_move(&world, &body);
    assert(result.hit_floor);
    assert(body.y == 130 * PS_ONE);
}

static void test_question_block_underside(void)
{
    struct ps_world world;
    struct ps_body body = { 52 * PS_ONE, 125 * PS_ONE,
                            0, -40 * PS_ONE, 13, 16 };
    struct ps_move_result result;

    ps_world_clear(&world);
    assert(ps_world_add_solid(&world, 23, 48, 96, 16, 16));
    result = ps_move(&world, &body);
    assert(result.hit_ceiling);
    assert(result.vertical_id == 23);
    assert(body.y == 112 * PS_ONE);
}

static void test_center_selects_adjacent_question_block(void)
{
    struct ps_world world;
    struct ps_body body = { 60 * PS_ONE, 125 * PS_ONE,
                            0, -40 * PS_ONE, 13, 16 };
    struct ps_move_result result;

    ps_world_clear(&world);
    assert(ps_world_add_solid(&world, 22, 48, 96, 16, 16));
    assert(ps_world_add_solid(&world, 23, 64, 96, 16, 16));
    result = ps_move(&world, &body);
    assert(result.hit_ceiling);
    assert(result.vertical_id == 23);
}

static void test_pipe_stops_horizontal_motion(void)
{
    struct ps_world world;
    struct ps_body body = { 140 * PS_ONE, 130 * PS_ONE,
                            30 * PS_ONE, 0, 13, 16 };
    struct ps_move_result result;

    ps_world_clear(&world);
    assert(ps_world_add_solid(&world, 31, 168, 117, 30, 29));
    result = ps_move(&world, &body);
    assert(result.hit_right);
    assert(result.horizontal_id == 31);
    assert(body.x == 155 * PS_ONE);
}

static void test_stomp_requires_top_crossing(void)
{
    struct ps_body enemy = { 100 * PS_ONE, 133 * PS_ONE,
                             0, 0, 14, 13 };
    struct ps_body old_side = { 88 * PS_ONE, 130 * PS_ONE,
                                PS_ONE, 0, 13, 16 };
    struct ps_body side = old_side;
    struct ps_body old_stomp = { 100 * PS_ONE, 110 * PS_ONE,
                                 0, 0, 13, 16 };
    struct ps_body stomp = old_stomp;

    side.x += PS_ONE;
    assert(ps_overlap(&side, &enemy));
    assert(!ps_crossed_top(&old_side, &side, &enemy, 2));

    stomp.y = 119 * PS_ONE;
    stomp.vy = 9 * PS_ONE;
    assert(ps_overlap(&stomp, &enemy));
    assert(ps_crossed_top(&old_stomp, &stomp, &enemy, 2));
}

int main(void)
{
    test_floor_landing();
    test_offscreen_entry_can_land();
    test_question_block_underside();
    test_center_selects_adjacent_question_block();
    test_pipe_stops_horizontal_motion();
    test_stomp_requires_top_crossing();
    puts("PocketStep tests passed");
    return 0;
}
