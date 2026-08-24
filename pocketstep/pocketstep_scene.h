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

int ps_scene_valid(const struct ps_scene *scene);
int ps_scene_tile(const struct ps_scene *scene, int x, int y);
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
