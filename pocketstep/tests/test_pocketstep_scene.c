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

static void test_edge_entry(void)
{
    static const unsigned char edge_tiles[25] = { 0 };
    static const unsigned char edge_blocked[25] = {
        1, 1, 1, 1, 1,
        1, 0, 1, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 1, 1, 1, 1
    };
    struct ps_scene scene = {
        5, 5, edge_tiles, { 5, 5, edge_blocked },
        0, 0, { 2, 2 }, 7
    };
    struct ps_grid_cell entry;

    assert(ps_scene_edge_entry(&scene, PS_GRID_DOWN, 2, &entry));
    assert(entry.x == 1 && entry.y == 1);
    assert(ps_scene_edge_entry(&scene, PS_GRID_UP, 2, &entry));
    assert(entry.x == 2 && entry.y == 3);
    assert(ps_scene_edge_entry(&scene, PS_GRID_RIGHT, 2, &entry));
    assert(entry.x == 1 && entry.y == 2);
    assert(ps_scene_edge_entry(&scene, PS_GRID_LEFT, 2, &entry));
    assert(entry.x == 3 && entry.y == 2);
    assert(!ps_scene_edge_entry(&scene, 9, 2, &entry));
}

static void test_scene_links(void)
{
    static const unsigned char link_tiles[25] = { 0 };
    static const unsigned char link_blocked[25] = {
        1,1,1,1,1, 1,0,1,0,1, 1,0,0,0,1,
        1,0,0,0,1, 1,1,1,1,1
    };
    static const struct ps_scene link_scenes[2] = {
        { 5,5,link_tiles,{5,5,link_blocked},0,0,{2,2},1 },
        { 5,5,link_tiles,{5,5,link_blocked},0,0,{2,2},2 }
    };
    struct ps_scene_link links[2] = {
        { 0, PS_GRID_DOWN, 1, 2 },
        { 1, PS_GRID_UP, 0, 2 }
    };
    struct ps_grid_cell entry;
    int target = -1;

    assert(ps_scene_links_valid(links, 2, link_scenes, 2));
    assert(ps_scene_link_find(links, 2, 0, PS_GRID_DOWN) == &links[0]);
    assert(!ps_scene_link_find(links, 2, 0, PS_GRID_LEFT));
    assert(ps_scene_link_follow(links, 2, link_scenes, 2,
                                0, PS_GRID_DOWN, &target, &entry));
    assert(target == 1 && entry.x == 1 && entry.y == 1);
    assert(ps_scene_links_reciprocal(links, 2, link_scenes, 2));
    links[1].preferred_offset = 1;
    assert(!ps_scene_links_reciprocal(links, 2, link_scenes, 2));
    links[1].preferred_offset = 2;
    links[0].target_scene = 3;
    assert(!ps_scene_links_valid(links, 2, link_scenes, 2));
    links[0].target_scene = 1;
    links[1] = links[0];
    assert(!ps_scene_links_valid(links, 2, link_scenes, 2));
}

static void test_entrances_and_props(void)
{
    static const unsigned char entrance_tiles[9] = { 0 };
    static const unsigned char entrance_blocked[9] = {
        1,1,1, 1,0,1, 1,1,1
    };
    static const struct ps_region entrance_regions[1] = {
        { { 8, 8, 8, 8 }, 20 }
    };
    static const struct ps_scene entrance_scenes[2] = {
        { 3,3,entrance_tiles,{3,3,entrance_blocked},
          entrance_regions,1,{1,1},1 },
        { 3,3,entrance_tiles,{3,3,entrance_blocked},0,0,{1,1},2 }
    };
    struct ps_scene_entrance entrances[1] = {
        { 0, 20, PS_SCENE_NO_DESTINATION, { 0, 0 }, PS_GRID_UP }
    };
    struct ps_scene_prop props[1] = {
        { 0, 0, 0, 15, 7, PS_SCENE_PROP_SOLID }
    };

    assert(ps_scene_entrances_valid(entrances, 1, entrance_scenes, 2));
    assert(ps_scene_entrance_find(entrances, 1, 0, 20) == &entrances[0]);
    entrances[0].target_scene = 1;
    entrances[0].spawn.x = 1;
    entrances[0].spawn.y = 1;
    assert(ps_scene_entrances_valid(entrances, 1, entrance_scenes, 2));
    entrances[0].spawn.x = 0;
    assert(!ps_scene_entrances_valid(entrances, 1, entrance_scenes, 2));
    assert(ps_scene_props_valid(props, 1, entrance_scenes, 2));
    props[0].column = 1;
    props[0].row = 1;
    assert(!ps_scene_props_valid(props, 1, entrance_scenes, 2));
    props[0].flags = 2;
    assert(!ps_scene_props_valid(props, 1, entrance_scenes, 2));
}

int main(void)
{
    test_scene_validation();
    test_grid_compatibility();
    test_variation();
    test_edge_entry();
    test_scene_links();
    test_entrances_and_props();
    puts("PocketStep scene tests passed");
    return 0;
}
