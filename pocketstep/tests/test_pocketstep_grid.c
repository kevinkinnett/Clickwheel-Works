#include <assert.h>
#include <stdio.h>

#define POCKETSTEP_IMPLEMENTATION
#include "../pocketstep.h"
#define POCKETSTEP_GRID_IMPLEMENTATION
#include "../pocketstep_grid.h"

static void test_blocked_queries(void)
{
    static const unsigned char cells[6] = {
        0, 1, 0,
        0, 0, 0
    };
    struct ps_grid grid = { 3, 2, cells };

    assert(!ps_grid_is_blocked(&grid, 0, 0));
    assert(ps_grid_is_blocked(&grid, 1, 0));
    assert(ps_grid_is_blocked(&grid, -1, 0));
    assert(ps_grid_is_blocked(&grid, 3, 0));
    assert(ps_grid_is_blocked(&grid, 0, 2));
}

static void test_route_around_obstacles(void)
{
    static const unsigned char cells[25] = {
        0, 0, 0, 0, 0,
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 0, 0, 0,
        0, 0, 0, 0, 0
    };
    struct ps_grid grid = { 5, 5, cells };
    struct ps_grid_workspace workspace;
    struct ps_grid_cell cells_out[12];
    struct ps_grid_path path = { cells_out, 12, 0 };
    int i;

    assert(ps_grid_find_path(&grid, 0, 0, 4, 4,
                             &workspace, &path) == PS_PATH_FOUND);
    assert(path.count == 8);
    assert(path.cells[path.count - 1].x == 4);
    assert(path.cells[path.count - 1].y == 4);
    for (i = 0; i < path.count; ++i)
        assert(!ps_grid_is_blocked(&grid, path.cells[i].x, path.cells[i].y));
}

static void test_deterministic_tie(void)
{
    static const unsigned char cells[9] = {
        0, 0, 0,
        0, 0, 0,
        0, 0, 0
    };
    struct ps_grid grid = { 3, 3, cells };
    struct ps_grid_workspace workspace;
    struct ps_grid_cell result[4];
    struct ps_grid_path path = { result, 4, 0 };

    assert(ps_grid_find_path(&grid, 1, 1, 0, 0,
                             &workspace, &path) == PS_PATH_FOUND);
    assert(path.count == 2);
    assert(path.cells[0].x == 1 && path.cells[0].y == 0);
    assert(path.cells[1].x == 0 && path.cells[1].y == 0);
}

static void test_unreachable_and_capacity(void)
{
    static const unsigned char blocked[9] = {
        0, 0, 0,
        1, 1, 1,
        0, 0, 0
    };
    static const unsigned char open[5] = { 0, 0, 0, 0, 0 };
    struct ps_grid blocked_grid = { 3, 3, blocked };
    struct ps_grid open_grid = { 5, 1, open };
    struct ps_grid_workspace workspace;
    struct ps_grid_cell result[3] = { { 77, 88 }, { 77, 88 }, { 77, 88 } };
    struct ps_grid_path path = { result, 3, 0 };

    assert(ps_grid_find_path(&blocked_grid, 0, 0, 0, 2,
                             &workspace, &path) == PS_PATH_NO_ROUTE);
    assert(path.count == 0);
    assert(ps_grid_find_path(&open_grid, 0, 0, 4, 0,
                             &workspace, &path) == PS_PATH_CAPACITY);
    assert(path.count == 0);
    assert(result[0].x == 77 && result[0].y == 88);
}

static void test_facing_regions(void)
{
    static const struct ps_region regions[2] = {
        { { 20, 10, 10, 8 }, 41 },
        { { 0, 10, 8, 8 }, 42 }
    };
    struct ps_body actor = { 10 * PS_ONE, 10 * PS_ONE,
                             0, 0, 8, 8 };

    assert(ps_region_find_facing(regions, 2, &actor,
                                 PS_GRID_RIGHT, 3) == 41);
    assert(ps_region_find_facing(regions, 2, &actor,
                                 PS_GRID_LEFT, 3) == 42);
    assert(ps_region_find_facing(regions, 2, &actor,
                                 PS_GRID_UP, 3) == -1);
}

int main(void)
{
    test_blocked_queries();
    test_route_around_obstacles();
    test_deterministic_tie();
    test_unreachable_and_capacity();
    test_facing_regions();
    puts("PocketStep grid tests passed");
    return 0;
}
