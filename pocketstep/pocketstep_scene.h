/*
 * PocketStep Scene
 * Caller-owned fixed-screen scene metadata and cosmetic tile variation.
 *
 * Copyright (C) 2026 Kevin Kinnett
 * SPDX-License-Identifier: MIT
 *
 * Define POCKETSTEP_SCENE_IMPLEMENTATION in one translation unit before
 * including this file. The module depends on pocketstep_grid.h.
 */

#ifndef POCKETSTEP_SCENE_H
#define POCKETSTEP_SCENE_H

#include <stdint.h>
#include "pocketstep_grid.h"

struct ps_scene
{
    int width;
    int height;
    const unsigned char *tiles;
    struct ps_grid grid;
    const struct ps_region *regions;
    int region_count;
    struct ps_grid_cell spawn;
    uint32_t variation_seed;
};

#define PS_SCENE_NO_DESTINATION -1
#define PS_SCENE_PROP_SOLID 1

struct ps_scene_link
{
    int source_scene;
    int travel_direction;
    int target_scene;
    int preferred_offset;
};

struct ps_scene_entrance
{
    int source_scene;
    int region_id;
    int target_scene;
    struct ps_grid_cell spawn;
    int facing;
};

struct ps_scene_prop
{
    int scene;
    int column;
    int row;
    int foot_y;
    int asset_id;
    int flags;
};

int ps_scene_valid(const struct ps_scene *scene);
int ps_scene_tile(const struct ps_scene *scene, int x, int y);
int ps_scene_edge_entry(const struct ps_scene *scene,
                        int travel_direction, int preferred_offset,
                        struct ps_grid_cell *entry);
const struct ps_scene_link *ps_scene_link_find(
    const struct ps_scene_link *links, int link_count,
    int source_scene, int travel_direction);
int ps_scene_links_valid(const struct ps_scene_link *links, int link_count,
                         const struct ps_scene *scenes, int scene_count);
int ps_scene_links_reciprocal(const struct ps_scene_link *links,
                              int link_count,
                              const struct ps_scene *scenes,
                              int scene_count);
int ps_scene_link_follow(const struct ps_scene_link *links, int link_count,
                         const struct ps_scene *scenes, int scene_count,
                         int source_scene, int travel_direction,
                         int *target_scene, struct ps_grid_cell *entry);
const struct ps_scene_entrance *ps_scene_entrance_find(
    const struct ps_scene_entrance *entrances, int entrance_count,
    int source_scene, int region_id);
int ps_scene_entrances_valid(const struct ps_scene_entrance *entrances,
                             int entrance_count,
                             const struct ps_scene *scenes, int scene_count);
int ps_scene_props_valid(const struct ps_scene_prop *props, int prop_count,
                         const struct ps_scene *scenes, int scene_count);
int ps_tile_variation(int x, int y, uint32_t seed, int variation_count);

#endif

#ifdef POCKETSTEP_SCENE_IMPLEMENTATION
#ifndef POCKETSTEP_SCENE_IMPLEMENTATION_ONCE
#define POCKETSTEP_SCENE_IMPLEMENTATION_ONCE

int ps_scene_valid(const struct ps_scene *scene)
{
    int cell_count;

    if (scene == 0 || scene->tiles == 0 || scene->grid.blocked == 0 ||
        scene->width <= 0 || scene->height <= 0 ||
        scene->grid.width != scene->width ||
        scene->grid.height != scene->height || scene->region_count < 0 ||
        (scene->region_count > 0 && scene->regions == 0))
        return 0;
    cell_count = scene->width * scene->height;
    if (cell_count <= 0 || cell_count > PS_GRID_MAX_CELLS ||
        scene->spawn.x < 0 || scene->spawn.y < 0 ||
        scene->spawn.x >= scene->width || scene->spawn.y >= scene->height ||
        ps_grid_is_blocked(&scene->grid, scene->spawn.x, scene->spawn.y))
        return 0;
    return 1;
}

int ps_scene_tile(const struct ps_scene *scene, int x, int y)
{
    if (!ps_scene_valid(scene) || x < 0 || y < 0 ||
        x >= scene->width || y >= scene->height)
        return -1;
    return scene->tiles[y * scene->width + x];
}

int ps_scene_edge_entry(const struct ps_scene *scene,
                        int travel_direction, int preferred_offset,
                        struct ps_grid_cell *entry)
{
    int fixed;
    int minimum = 1;
    int maximum;
    int distance;
    int vertical_entry;

    if (!ps_scene_valid(scene) || entry == 0 ||
        scene->width < 3 || scene->height < 3 ||
        travel_direction < PS_GRID_UP || travel_direction > PS_GRID_LEFT)
        return 0;
    vertical_entry = travel_direction == PS_GRID_UP ||
                     travel_direction == PS_GRID_DOWN;
    maximum = (vertical_entry ? scene->width : scene->height) - 2;
    if (preferred_offset < minimum)
        preferred_offset = minimum;
    if (preferred_offset > maximum)
        preferred_offset = maximum;
    if (travel_direction == PS_GRID_DOWN)
        fixed = 1;
    else if (travel_direction == PS_GRID_UP)
        fixed = scene->height - 2;
    else if (travel_direction == PS_GRID_RIGHT)
        fixed = 1;
    else
        fixed = scene->width - 2;

    for (distance = 0; distance <= maximum - minimum; ++distance)
    {
        int candidates[2];
        int count = distance == 0 ? 1 : 2;
        int index;

        candidates[0] = preferred_offset - distance;
        candidates[1] = preferred_offset + distance;
        for (index = 0; index < count; ++index)
        {
            int varying = candidates[index];
            int x;
            int y;

            if (varying < minimum || varying > maximum)
                continue;
            x = vertical_entry ? varying : fixed;
            y = vertical_entry ? fixed : varying;
            if (!ps_grid_is_blocked(&scene->grid, x, y))
            {
                entry->x = x;
                entry->y = y;
                return 1;
            }
        }
    }
    return 0;
}

const struct ps_scene_link *ps_scene_link_find(
    const struct ps_scene_link *links, int link_count,
    int source_scene, int travel_direction)
{
    int index;

    if (links == 0 || link_count <= 0)
        return 0;
    for (index = 0; index < link_count; ++index)
    {
        if (links[index].source_scene == source_scene &&
            links[index].travel_direction == travel_direction)
            return &links[index];
    }
    return 0;
}

int ps_scene_links_valid(const struct ps_scene_link *links, int link_count,
                         const struct ps_scene *scenes, int scene_count)
{
    int index;

    if (link_count < 0 || scene_count <= 0 || scenes == 0 ||
        (link_count > 0 && links == 0))
        return 0;
    for (index = 0; index < scene_count; ++index)
    {
        if (!ps_scene_valid(&scenes[index]))
            return 0;
    }
    for (index = 0; index < link_count; ++index)
    {
        struct ps_grid_cell entry;
        int other;

        if (links[index].source_scene < 0 ||
            links[index].source_scene >= scene_count ||
            links[index].target_scene < 0 ||
            links[index].target_scene >= scene_count ||
            links[index].travel_direction < PS_GRID_UP ||
            links[index].travel_direction > PS_GRID_LEFT ||
            !ps_scene_edge_entry(&scenes[links[index].target_scene],
                                 links[index].travel_direction,
                                 links[index].preferred_offset, &entry))
            return 0;
        for (other = 0; other < index; ++other)
        {
            if (links[other].source_scene == links[index].source_scene &&
                links[other].travel_direction ==
                    links[index].travel_direction)
                return 0;
        }
    }
    return 1;
}

static int ps_scene_opposite_direction(int direction)
{
    if (direction == PS_GRID_UP)
        return PS_GRID_DOWN;
    if (direction == PS_GRID_RIGHT)
        return PS_GRID_LEFT;
    if (direction == PS_GRID_DOWN)
        return PS_GRID_UP;
    if (direction == PS_GRID_LEFT)
        return PS_GRID_RIGHT;
    return -1;
}

int ps_scene_links_reciprocal(const struct ps_scene_link *links,
                              int link_count,
                              const struct ps_scene *scenes,
                              int scene_count)
{
    int index;

    if (!ps_scene_links_valid(links, link_count, scenes, scene_count))
        return 0;
    for (index = 0; index < link_count; ++index)
    {
        const struct ps_scene_link *link = &links[index];
        const struct ps_scene_link *reverse = ps_scene_link_find(
            links, link_count, link->target_scene,
            ps_scene_opposite_direction(link->travel_direction));

        if (reverse == 0 || reverse->target_scene != link->source_scene ||
            reverse->preferred_offset != link->preferred_offset)
            return 0;
    }
    return 1;
}

int ps_scene_link_follow(const struct ps_scene_link *links, int link_count,
                         const struct ps_scene *scenes, int scene_count,
                         int source_scene, int travel_direction,
                         int *target_scene, struct ps_grid_cell *entry)
{
    const struct ps_scene_link *link;

    if (target_scene == 0 || entry == 0 ||
        !ps_scene_links_valid(links, link_count, scenes, scene_count))
        return 0;
    link = ps_scene_link_find(links, link_count,
                              source_scene, travel_direction);
    if (link == 0 ||
        !ps_scene_edge_entry(&scenes[link->target_scene],
                             link->travel_direction,
                             link->preferred_offset, entry))
        return 0;
    *target_scene = link->target_scene;
    return 1;
}

static int ps_scene_has_region_id(const struct ps_scene *scene, int region_id)
{
    int index;

    for (index = 0; index < scene->region_count; ++index)
    {
        if (scene->regions[index].id == region_id)
            return 1;
    }
    return 0;
}

const struct ps_scene_entrance *ps_scene_entrance_find(
    const struct ps_scene_entrance *entrances, int entrance_count,
    int source_scene, int region_id)
{
    int index;

    if (entrances == 0 || entrance_count <= 0)
        return 0;
    for (index = 0; index < entrance_count; ++index)
    {
        if (entrances[index].source_scene == source_scene &&
            entrances[index].region_id == region_id)
            return &entrances[index];
    }
    return 0;
}

int ps_scene_entrances_valid(const struct ps_scene_entrance *entrances,
                             int entrance_count,
                             const struct ps_scene *scenes, int scene_count)
{
    int index;

    if (entrance_count < 0 || scene_count <= 0 || scenes == 0 ||
        (entrance_count > 0 && entrances == 0))
        return 0;
    for (index = 0; index < entrance_count; ++index)
    {
        const struct ps_scene_entrance *entrance = &entrances[index];

        if (entrance->source_scene < 0 ||
            entrance->source_scene >= scene_count ||
            !ps_scene_valid(&scenes[entrance->source_scene]) ||
            !ps_scene_has_region_id(&scenes[entrance->source_scene],
                                    entrance->region_id) ||
            entrance->facing < PS_GRID_UP ||
            entrance->facing > PS_GRID_LEFT)
            return 0;
        if (entrance->target_scene == PS_SCENE_NO_DESTINATION)
            continue;
        if (entrance->target_scene < 0 ||
            entrance->target_scene >= scene_count ||
            !ps_scene_valid(&scenes[entrance->target_scene]) ||
            entrance->spawn.x < 0 || entrance->spawn.y < 0 ||
            entrance->spawn.x >= scenes[entrance->target_scene].width ||
            entrance->spawn.y >= scenes[entrance->target_scene].height ||
            ps_grid_is_blocked(&scenes[entrance->target_scene].grid,
                               entrance->spawn.x, entrance->spawn.y))
            return 0;
    }
    return 1;
}

int ps_scene_props_valid(const struct ps_scene_prop *props, int prop_count,
                         const struct ps_scene *scenes, int scene_count)
{
    int index;

    if (prop_count < 0 || scene_count <= 0 || scenes == 0 ||
        (prop_count > 0 && props == 0))
        return 0;
    for (index = 0; index < prop_count; ++index)
    {
        const struct ps_scene_prop *prop = &props[index];

        if (prop->scene < 0 || prop->scene >= scene_count ||
            !ps_scene_valid(&scenes[prop->scene]) ||
            prop->column < 0 || prop->row < 0 ||
            prop->column >= scenes[prop->scene].width ||
            prop->row >= scenes[prop->scene].height ||
            prop->foot_y < 0 || prop->asset_id < 0 ||
            (prop->flags & ~PS_SCENE_PROP_SOLID) != 0)
            return 0;
        if ((prop->flags & PS_SCENE_PROP_SOLID) != 0 &&
            !ps_grid_is_blocked(&scenes[prop->scene].grid,
                                prop->column, prop->row))
            return 0;
    }
    return 1;
}

int ps_tile_variation(int x, int y, uint32_t seed, int variation_count)
{
    uint32_t value;

    if (variation_count <= 0)
        return -1;
    value = (uint32_t)x * (uint32_t)0x8da6b343UL;
    value ^= (uint32_t)y * (uint32_t)0xd8163841UL;
    value ^= seed + (uint32_t)0xcb1ab31fUL;
    value ^= value >> 16;
    value *= (uint32_t)0x7feb352dUL;
    value ^= value >> 15;
    value *= (uint32_t)0x846ca68bUL;
    value ^= value >> 16;
    return (int)(value % (uint32_t)variation_count);
}

#endif
#endif
