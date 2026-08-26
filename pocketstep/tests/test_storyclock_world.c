#include <stdio.h>
#include <stdlib.h>

#define POCKETSTEP_IMPLEMENTATION
#include "../pocketstep.h"
#define POCKETSTEP_GRID_IMPLEMENTATION
#include "../pocketstep_grid.h"
#define POCKETSTEP_SCENE_IMPLEMENTATION
#include "../pocketstep_scene.h"

#define W 13
#define H 11

static const unsigned char house[W * H] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,0,0,0,0,0,0,1,1,1,
    1,1,1,1,0,0,0,0,0,0,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,1,0,0,0,0,0,0,1,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,1,1,
    1,1,1,0,0,0,0,0,0,1,1,1,1,
    1,1,1,0,0,0,0,0,0,1,1,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const unsigned char cottage[W * H] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,1,1,1,1,1,1,0,0,0,1,
    1,0,0,1,1,1,1,1,1,0,1,0,1,
    1,1,1,1,1,1,1,1,1,0,1,1,1,
    1,0,0,1,1,1,0,1,1,0,1,1,1,
    1,0,0,1,0,0,0,0,0,0,0,0,1,
    1,0,0,1,0,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const unsigned char green[W * H] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,0,1,1,1,1,1,1,
    1,1,1,1,1,1,0,1,1,1,1,1,1,
    1,1,1,1,1,1,0,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,0,0,0,1,0,0,0,0,0,0,0,1,
    1,0,1,0,0,0,0,0,0,0,1,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,0,1,1,1,1,1,1
};

static const unsigned char mill[W * H] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,1,1,1,0,0,1,1,1,1,1,1,
    1,0,1,1,1,0,0,1,1,1,1,1,1,
    1,0,1,1,1,0,0,1,1,1,1,1,1,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const unsigned char market[W * H] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,0,1,1,1,1,1,1,
    1,1,1,1,1,1,0,1,1,1,1,1,1,
    1,1,1,1,1,1,0,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,0,0,1,0,0,0,1,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const unsigned char gate[W * H] = {
    1,1,1,1,1,1,0,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,1,0,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,1,0,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,1,0,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,0,0,0,1,1,1,1,1,
    1,1,1,1,1,0,0,0,1,1,1,1,1,
    1,1,1,1,1,0,0,0,1,1,1,1,1,
    1,1,1,1,1,1,0,1,1,1,1,1,1
};

static const unsigned char fields[W * H] = {
    1,1,1,1,1,1,0,1,1,1,1,1,1,
    1,1,1,1,1,1,0,0,0,0,0,1,1,
    1,1,1,1,1,1,0,0,0,0,0,0,1,
    1,1,1,1,1,1,0,0,0,0,0,0,1,
    1,1,1,0,0,0,0,0,0,0,0,0,1,
    1,1,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,0,0,0,0,0,1,1,1,1,
    1,1,1,1,0,0,0,0,0,1,0,0,1,
    1,1,1,1,0,0,0,0,0,1,0,0,1,
    1,1,1,1,0,0,0,0,0,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const unsigned char garden[W * H] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,0,0,0,1,1,1,1,1,
    1,1,1,1,1,0,0,0,1,1,1,1,1,
    1,0,0,0,0,0,0,0,1,1,1,1,1,
    1,0,0,0,0,0,0,0,1,1,0,1,1,
    0,0,0,0,0,0,0,0,0,0,0,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,1,1,1,1,0,0,0,0,0,0,1,
    1,0,1,1,1,1,0,0,0,0,0,0,1,
    1,1,0,0,0,0,0,0,0,1,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const unsigned char blank_tiles[W * H] = { 0 };

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

static void require_unreachable(const unsigned char *cells,
                                int start_x, int start_y,
                                int end_x, int end_y)
{
    struct ps_grid grid = { W, H, cells };
    struct ps_grid_workspace workspace;
    struct ps_grid_cell route_cells[W * H];
    struct ps_grid_path path = { route_cells, W * H, 0 };

    require(ps_grid_find_path(&grid, start_x, start_y, end_x, end_y,
                              &workspace, &path) != PS_PATH_FOUND,
            "blocked destination unexpectedly has a route");
}

static void test_authored_routes_and_spawns(void)
{
    require(!house[7 * W + 6], "house spawn must be passable");
    require(!cottage[4 * W + 6], "cottage doorway spawn must be passable");
    require(!green[1 * W + 6], "green north entry must be passable");
    require(!mill[5 * W + 11], "mill east entry must be passable");
    require(!market[5 * W + 1], "market west entry must be passable");
    require(!gate[1 * W + 6], "gate north entry must be passable");
    require(!fields[1 * W + 6], "fields north entry must be passable");
    require(!garden[5 * W + 1], "garden west entry must be passable");
    require_route(house, 6, 7, 8, 4);
    require_route(house, 8, 4, 3, 4);
    require_route(house, 3, 4, 6, 9);
    require_route(cottage, 6, 4, 4, 5);
    require_route(cottage, 4, 5, 9, 2);
    require_route(cottage, 9, 2, 6, 9);
    require_route(green, 6, 1, 1, 5);
    require_route(green, 1, 5, 11, 5);
    require_route(green, 11, 5, 6, 9);
    require(green[2 * W + 5] && !green[2 * W + 6] &&
            green[2 * W + 7],
            "green north corridor must stay centered between facades");
    require_route(mill, 11, 5, 6, 5);
    require_route(market, 1, 5, 10, 5);
    require_route(gate, 6, 1, 6, 7);
    require_route(gate, 6, 9, 6, 1);
    require_unreachable(gate, 6, 1, 2, 8);
    require_route(fields, 6, 1, 7, 5);
    require_route(fields, 7, 5, 8, 7);
    require_route(fields, 8, 7, 6, 1);
    require(!fields[4 * W + 3], "farmhouse door cell must be passable");
    require(fields[4 * W + 2] && fields[4 * W + 1],
            "farmhouse footprint must stay blocked beside its door");
    require_unreachable(fields, 6, 1, 2, 6);
    require_route(garden, 1, 5, 6, 7);
    require_route(garden, 6, 7, 6, 4);
    require_route(garden, 6, 4, 10, 4);
    require_route(garden, 10, 4, 1, 5);
    require(!garden[4 * W + 10], "potting-shed door cell must be passable");
    require(garden[4 * W + 9] && garden[4 * W + 11],
            "potting-shed footprint must stay blocked beside its door");
    require_unreachable(garden, 1, 5, 2, 7);
}

static void test_village_links_entrances_and_props(void)
{
    static const struct ps_region green_regions[] = {
        { { 54,48,16,16 }, 20 }, { { 150,48,16,16 }, 21 }
    };
    static const struct ps_region mill_regions[] = {
        { { 150,48,16,16 }, 22 }
    };
    static const struct ps_region market_regions[] = {
        { { 54,48,16,16 }, 23 }, { { 150,48,16,16 }, 24 }
    };
    static const struct ps_region gate_regions[] = {
        { { 38,112,16,16 }, 25 }
    };
    static const struct ps_region fields_regions[] = {
        { { 54,64,16,16 }, 26 }, { { 134,80,16,16 }, 34 }
    };
    static const struct ps_region garden_regions[] = {
        { { 166,64,16,16 }, 27 }, { { 118,112,16,16 }, 35 }
    };
    static const struct ps_scene test_scenes[] = {
        { W,H,blank_tiles,{W,H,house},0,0,{6,7},1 },
        { W,H,blank_tiles,{W,H,cottage},0,0,{6,4},2 },
        { W,H,blank_tiles,{W,H,green},green_regions,2,{6,1},3 },
        { W,H,blank_tiles,{W,H,mill},mill_regions,1,{11,5},4 },
        { W,H,blank_tiles,{W,H,market},market_regions,2,{1,5},5 },
        { W,H,blank_tiles,{W,H,gate},gate_regions,1,{6,1},6 },
        { W,H,blank_tiles,{W,H,fields},fields_regions,2,{6,1},7 },
        { W,H,blank_tiles,{W,H,garden},garden_regions,2,{1,5},8 }
    };
    static const struct ps_scene_link links[] = {
        {1,PS_GRID_DOWN,2,6}, {2,PS_GRID_UP,1,6},
        {2,PS_GRID_LEFT,3,5}, {3,PS_GRID_RIGHT,2,5},
        {2,PS_GRID_RIGHT,4,5}, {4,PS_GRID_LEFT,2,5},
        {2,PS_GRID_DOWN,5,6}, {5,PS_GRID_UP,2,6},
        {5,PS_GRID_DOWN,6,6}, {6,PS_GRID_UP,5,6},
        {4,PS_GRID_RIGHT,7,5}, {7,PS_GRID_LEFT,4,5}
    };
    static const struct ps_scene_entrance entrances[] = {
        {2,20,PS_SCENE_NO_DESTINATION,{0,0},PS_GRID_UP},
        {2,21,PS_SCENE_NO_DESTINATION,{0,0},PS_GRID_UP},
        {3,22,PS_SCENE_NO_DESTINATION,{0,0},PS_GRID_UP},
        {4,23,PS_SCENE_NO_DESTINATION,{0,0},PS_GRID_UP},
        {4,24,PS_SCENE_NO_DESTINATION,{0,0},PS_GRID_UP},
        {5,25,PS_SCENE_NO_DESTINATION,{0,0},PS_GRID_UP},
        {6,26,PS_SCENE_NO_DESTINATION,{0,0},PS_GRID_UP},
        {7,27,PS_SCENE_NO_DESTINATION,{0,0},PS_GRID_UP}
    };
    static const struct ps_scene_prop props[] = {
        {1,10,4,79,100,PS_SCENE_PROP_SOLID},
        {1,11,4,79,101,PS_SCENE_PROP_SOLID},
        {2,4,6,111,102,PS_SCENE_PROP_SOLID},
        {5,2,8,143,103,PS_SCENE_PROP_SOLID},
        {5,3,8,143,101,PS_SCENE_PROP_SOLID},
        {6,1,5,95,100,PS_SCENE_PROP_SOLID}
    };
    struct ps_grid_cell entry;
    int target;
    int index;

    require(ps_scene_links_valid(links, 12, test_scenes, 8),
            "village links are invalid");
    require(ps_scene_links_reciprocal(links, 12, test_scenes, 8),
            "village links must have matching reciprocal edges");
    for (index = 0; index < 12; ++index)
    {
        require(ps_scene_link_follow(links, 12, test_scenes, 8,
                                     links[index].source_scene,
                                     links[index].travel_direction,
                                     &target, &entry),
                "village link cannot be followed");
        require(target == links[index].target_scene,
                "village link reached wrong scene");
        require(!test_scenes[target].grid.blocked[entry.y * W + entry.x],
                "village link selected a blocked entry");
    }
    require(ps_scene_entrances_valid(entrances, 8, test_scenes, 8),
            "inactive village entrances are invalid");
    require(ps_scene_entrance_find(entrances, 8, 4, 23) != 0,
            "market shop entrance is missing");
    require(ps_scene_entrance_find(entrances, 8, 6, 26) != 0,
            "farmhouse entrance is missing");
    require(ps_scene_entrance_find(entrances, 8, 7, 27) != 0,
            "potting-shed entrance is missing");
    require(ps_scene_props_valid(props, 6, test_scenes, 8),
            "cottage props do not match collision");

    require(ps_scene_link_follow(links, 12, test_scenes, 8,
                                 5, PS_GRID_DOWN, &target, &entry) &&
            target == 6 && entry.x == 6 && entry.y == 1,
            "south gate must enter fields at the north road");
    require(ps_scene_link_follow(links, 12, test_scenes, 8,
                                 6, PS_GRID_UP, &target, &entry) &&
            target == 5 && entry.x == 6 && entry.y == 9,
            "fields must return to the south gate road");
    require(ps_scene_link_follow(links, 12, test_scenes, 8,
                                 4, PS_GRID_RIGHT, &target, &entry) &&
            target == 7 && entry.x == 1 && entry.y == 5,
            "market must enter the garden at the west path");
    require(ps_scene_link_follow(links, 12, test_scenes, 8,
                                 7, PS_GRID_LEFT, &target, &entry) &&
            target == 4 && entry.x == 11 && entry.y == 5,
            "garden must return to the market east path");
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
    test_village_links_entrances_and_props();
    test_interactions_and_item_transition();
    puts("Story Clock world tests passed.");
    return 0;
}
