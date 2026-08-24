#include <assert.h>
#include <stdio.h>

#define POCKETSTEP_IMPLEMENTATION
#include "../pocketstep.h"
#define POCKETSTEP_GRID_IMPLEMENTATION
#include "../pocketstep_grid.h"
#define POCKETSTEP_SCENE_IMPLEMENTATION
#include "../pocketstep_scene.h"

static const unsigned char tiles[6] = { 1, 2, 3, 4, 5, 6 };
static const unsigned char blocked[6] = { 0, 0, 0, 1, 0, 0 };
static const struct ps_region regions[1] = { { { 8, 8, 4, 4 }, 7 } };

static struct ps_scene valid_scene(void)
{
    struct ps_scene scene = {
        3, 2, tiles, { 3, 2, blocked }, regions, 1, { 1, 1 }, 19
    };
    return scene;
}

static void test_scene_validation(void)
{
    struct ps_scene scene = valid_scene();

    assert(ps_scene_valid(&scene));
    assert(ps_scene_tile(&scene, 2, 1) == 6);
    assert(ps_scene_tile(&scene, 3, 1) == -1);
    scene.spawn.x = 0;
    scene.spawn.y = 1;
    assert(!ps_scene_valid(&scene));
    scene = valid_scene();
    scene.grid.width = 2;
    assert(!ps_scene_valid(&scene));
    scene = valid_scene();
    scene.tiles = 0;
    assert(!ps_scene_valid(&scene));
    scene = valid_scene();
    scene.region_count = 1;
    scene.regions = 0;
    assert(!ps_scene_valid(&scene));
}

static void test_grid_compatibility(void)
{
    struct ps_scene scene = valid_scene();
    struct ps_grid_workspace workspace;
    struct ps_grid_cell cells[4];
    struct ps_grid_path path = { cells, 4, 0 };

    assert(ps_grid_find_path(&scene.grid, 0, 0, 2, 1,
                             &workspace, &path) == PS_PATH_FOUND);
    assert(path.count == 3);
}

static void test_variation(void)
{
    int first = ps_tile_variation(4, 7, 19, 3);
    int changed = 0;
    int x;

    assert(first >= 0 && first < 3);
    assert(ps_tile_variation(4, 7, 19, 3) == first);
    assert(ps_tile_variation(4, 7, 19, 0) == -1);
    for (x = 0; x < 32; ++x)
    {
        int a = ps_tile_variation(x, 5, 19, 4);
        int b = ps_tile_variation(x, 5, 20, 4);
        assert(a >= 0 && a < 4 && b >= 0 && b < 4);
        if (a != b)
            changed = 1;
    }
    assert(changed);
}

int main(void)
{
    test_scene_validation();
    test_grid_compatibility();
    test_variation();
    puts("PocketStep scene tests passed");
    return 0;
}
