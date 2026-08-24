#include <assert.h>
#include <stdio.h>

#define MICROGAME_IMPLEMENTATION
#include "../src/microgame.h"

static void test_floor_landing(void)
{
    struct mg_world world;
    struct mg_body body = { 20 * MG_ONE, 20 * MG_ONE,
                            0, 120 * MG_ONE, 12, 16 };
    struct mg_move_result result;

    mg_world_clear(&world);
    assert(mg_world_add_solid(&world, 7, 0, 100, 220, 76));
    result = mg_move(&world, &body);
    assert(result.hit_floor);
    assert(result.vertical_id == 7);
    assert(body.y == 84 * MG_ONE);
}

static void test_offscreen_entry_can_land(void)
{
    struct mg_world world;
    struct mg_body body = { -14 * MG_ONE, 130 * MG_ONE,
                            MG_ONE, 4, 13, 16 };
    struct mg_move_result result;

    mg_world_clear(&world);
    assert(mg_world_add_solid(&world, 7, -64, 146, 348, 30));
    result = mg_move(&world, &body);
    assert(result.hit_floor);
    assert(body.y == 130 * MG_ONE);
}

static void test_question_block_underside(void)
{
    struct mg_world world;
    struct mg_body body = { 52 * MG_ONE, 125 * MG_ONE,
                            0, -40 * MG_ONE, 13, 16 };
    struct mg_move_result result;

    mg_world_clear(&world);
    assert(mg_world_add_solid(&world, 23, 48, 96, 16, 16));
    result = mg_move(&world, &body);
    assert(result.hit_ceiling);
    assert(result.vertical_id == 23);
    assert(body.y == 112 * MG_ONE);
}

static void test_center_selects_adjacent_question_block(void)
{
    struct mg_world world;
    struct mg_body body = { 60 * MG_ONE, 125 * MG_ONE,
                            0, -40 * MG_ONE, 13, 16 };
    struct mg_move_result result;

    mg_world_clear(&world);
    assert(mg_world_add_solid(&world, 22, 48, 96, 16, 16));
    assert(mg_world_add_solid(&world, 23, 64, 96, 16, 16));
    result = mg_move(&world, &body);
    assert(result.hit_ceiling);
    assert(result.vertical_id == 23);
}

static void test_pipe_stops_horizontal_motion(void)
{
    struct mg_world world;
    struct mg_body body = { 140 * MG_ONE, 130 * MG_ONE,
                            30 * MG_ONE, 0, 13, 16 };
    struct mg_move_result result;

    mg_world_clear(&world);
    assert(mg_world_add_solid(&world, 31, 168, 117, 30, 29));
    result = mg_move(&world, &body);
    assert(result.hit_right);
    assert(result.horizontal_id == 31);
    assert(body.x == 155 * MG_ONE);
}

static void test_stomp_requires_top_crossing(void)
{
    struct mg_body enemy = { 100 * MG_ONE, 133 * MG_ONE,
                             0, 0, 14, 13 };
    struct mg_body old_side = { 88 * MG_ONE, 130 * MG_ONE,
                                MG_ONE, 0, 13, 16 };
    struct mg_body side = old_side;
    struct mg_body old_stomp = { 100 * MG_ONE, 110 * MG_ONE,
                                 0, 0, 13, 16 };
    struct mg_body stomp = old_stomp;

    side.x += MG_ONE;
    assert(mg_overlap(&side, &enemy));
    assert(!mg_crossed_top(&old_side, &side, &enemy, 2));

    stomp.y = 119 * MG_ONE;
    stomp.vy = 9 * MG_ONE;
    assert(mg_overlap(&stomp, &enemy));
    assert(mg_crossed_top(&old_stomp, &stomp, &enemy, 2));
}

int main(void)
{
    test_floor_landing();
    test_offscreen_entry_can_land();
    test_question_block_underside();
    test_center_selects_adjacent_question_block();
    test_pipe_stops_horizontal_motion();
    test_stomp_requires_top_crossing();
    puts("microgame tests passed");
    return 0;
}
