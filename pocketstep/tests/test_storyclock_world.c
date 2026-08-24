#include <stdio.h>
#include <stdlib.h>

#define POCKETSTEP_IMPLEMENTATION
#include "../pocketstep.h"
#define POCKETSTEP_GRID_IMPLEMENTATION
#include "../pocketstep_grid.h"

#define W 13
#define H 11

static const unsigned char house[W * H] = {
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

static const unsigned char outdoor[W * H] = {
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

static void require(int condition, const char *message)
{
    if (!condition)
    {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static void require_route(const unsigned char *cells,
                          int start_x, int start_y, int end_x, int end_y)
{
    struct ps_grid grid = { W, H, cells };
    struct ps_grid_workspace workspace;
    struct ps_grid_cell route_cells[W * H];
    struct ps_grid_path path = { route_cells, W * H, 0 };
    int result = ps_grid_find_path(&grid, start_x, start_y, end_x, end_y,
                                   &workspace, &path);

    require(result == PS_PATH_FOUND, "authored destination has no route");
    require(path.count > 0, "non-trivial route is empty");
    require(path.cells[path.count - 1].x == end_x &&
            path.cells[path.count - 1].y == end_y,
            "route does not end at authored destination");
}

static void test_authored_routes_and_spawns(void)
{
    require(!house[7 * W + 6], "house spawn must be passable");
    require(!outdoor[8 * W + 6], "outdoor spawn must be passable");
    require_route(house, 6, 7, 8, 4);
    require_route(house, 8, 4, 3, 4);
    require_route(house, 3, 4, 6, 9);
    require_route(outdoor, 6, 8, 4, 5);
    require_route(outdoor, 4, 5, 9, 2);
}

static void test_interactions_and_item_transition(void)
{
    const struct ps_region house_regions[] = {
        { { 150, 64, 16, 16 }, 10 }, { { 38, 64, 16, 16 }, 11 }
    };
    const struct ps_region outdoor_regions[] = {
        { { 54, 80, 16, 16 }, 12 }, { { 166, 32, 16, 16 }, 13 }
    };
    struct ps_body actor = { 138 * PS_ONE, 72 * PS_ONE, 0, 0, 8, 6 };
    int collected = 0;

    require(ps_region_find_facing(house_regions, 2, &actor,
                                  PS_GRID_RIGHT, 12) == 10,
            "item interaction ID must face the item");
    collected = 1;
    require(collected, "collection must change inventory state");
    actor.x = 58 * PS_ONE;
    require(ps_region_find_facing(house_regions, 2, &actor,
                                  PS_GRID_LEFT, 12) == 11,
            "indoor NPC interaction ID must be reachable");
    actor.x = 74 * PS_ONE;
    actor.y = 88 * PS_ONE;
    require(ps_region_find_facing(outdoor_regions, 2, &actor,
                                  PS_GRID_LEFT, 12) == 12,
            "outdoor NPC interaction ID must be reachable");
    actor.x = 154 * PS_ONE;
    actor.y = 40 * PS_ONE;
    require(ps_region_find_facing(outdoor_regions, 2, &actor,
                                  PS_GRID_RIGHT, 12) == 13,
            "beacon interaction ID must be reachable");
    collected = 0;
    require(!collected, "story reset must clear inventory state");
}

int main(void)
{
    test_authored_routes_and_spawns();
    test_interactions_and_item_transition();
    puts("Story Clock world tests passed.");
    return 0;
}
