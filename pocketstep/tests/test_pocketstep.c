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

static void test_world_solid_mutation(void)
{
    struct ps_world world;

    ps_world_clear(&world);
    assert(ps_world_add_solid(&world, 10, 16, 32, 16, 16));
    assert(ps_world_add_solid(&world, 11, 48, 64, 16, 16));
    assert(ps_world_update_solid(&world, 11, 52, 60, 20, 12));
    assert(world.solids[1].bounds.x == 52);
    assert(world.solids[1].bounds.y == 60);
    assert(world.solids[1].bounds.width == 20);
    assert(world.solids[1].bounds.height == 12);
    assert(!ps_world_update_solid(&world, 99, 0, 0, 1, 1));
    assert(ps_world_remove_solid(&world, 10));
    assert(world.solid_count == 1);
    assert(world.solids[0].id == 11);
    assert(!ps_world_remove_solid(&world, 99));
}

static void test_body_resize_preserves_bottom(void)
{
    struct ps_body body = { 20 * PS_ONE, 84 * PS_ONE,
                            0, 0, 13, 16 };

    assert(ps_body_resize_from_bottom(&body, 13, 24));
    assert(body.y == 76 * PS_ONE);
    assert(body.y + body.height * PS_ONE == 100 * PS_ONE);
    assert(ps_body_resize_from_bottom(&body, 15, 12));
    assert(body.width == 15);
    assert(body.y == 88 * PS_ONE);
    assert(body.y + body.height * PS_ONE == 100 * PS_ONE);
    assert(!ps_body_resize_from_bottom(&body, 0, 12));
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
    struct ps_body exact_touch = old_stomp;

    side.x += PS_ONE;
    assert(ps_overlap(&side, &enemy));
    assert(!ps_crossed_top(&old_side, &side, &enemy, 2));

    stomp.y = 119 * PS_ONE;
    stomp.vy = 9 * PS_ONE;
    assert(ps_overlap(&stomp, &enemy));
    assert(ps_crossed_top(&old_stomp, &stomp, &enemy, 2));

    exact_touch.y = 117 * PS_ONE;
    exact_touch.vy = 7 * PS_ONE;
    assert(!ps_overlap(&exact_touch, &enemy));
    assert(ps_crossed_top(&old_stomp, &exact_touch, &enemy, 2));
}

int main(void)
{
    test_floor_landing();
    test_offscreen_entry_can_land();
    test_question_block_underside();
    test_center_selects_adjacent_question_block();
    test_pipe_stops_horizontal_motion();
    test_world_solid_mutation();
    test_body_resize_preserves_bottom();
    test_stomp_requires_top_crossing();
    puts("PocketStep tests passed");
    return 0;
}
