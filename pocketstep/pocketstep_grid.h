/*
 * PocketStep Grid
 * Fixed-memory tile navigation and interaction regions for small C games.
 *
 * Copyright (C) 2026 Kevin Kinnett
 * SPDX-License-Identifier: MIT
 *
 * Define POCKETSTEP_GRID_IMPLEMENTATION in one translation unit before
 * including this file. The module depends on pocketstep.h for body and
 * rectangle types.
 */

#ifndef POCKETSTEP_GRID_H
#define POCKETSTEP_GRID_H

#include "pocketstep.h"

#ifndef PS_GRID_MAX_CELLS
#define PS_GRID_MAX_CELLS 143
#endif

#define PS_GRID_UP 0
#define PS_GRID_RIGHT 1
#define PS_GRID_DOWN 2
#define PS_GRID_LEFT 3

#define PS_PATH_NO_ROUTE 0
#define PS_PATH_FOUND 1
#define PS_PATH_INVALID -1
#define PS_PATH_CAPACITY -2

struct ps_grid_cell
{
    int x;
    int y;
};

struct ps_grid
{
    int width;
    int height;
    const unsigned char *blocked;
};

struct ps_grid_path
{
    struct ps_grid_cell *cells;
    int capacity;
    int count;
};

struct ps_grid_workspace
{
    int queue[PS_GRID_MAX_CELLS];
    int parent[PS_GRID_MAX_CELLS];
    unsigned char seen[PS_GRID_MAX_CELLS];
};

struct ps_region
{
    struct ps_rect bounds;
    int id;
};

int ps_grid_is_blocked(const struct ps_grid *grid, int x, int y);
int ps_grid_find_path(const struct ps_grid *grid,
                      int start_x, int start_y,
                      int destination_x, int destination_y,
                      struct ps_grid_workspace *workspace,
                      struct ps_grid_path *path);
struct ps_rect ps_facing_probe(const struct ps_body *body,
                               int facing, int reach_pixels);
int ps_region_find(const struct ps_region *regions, int region_count,
                   const struct ps_rect *probe);
int ps_region_find_facing(const struct ps_region *regions, int region_count,
                          const struct ps_body *body,
                          int facing, int reach_pixels);

#endif

#ifdef POCKETSTEP_GRID_IMPLEMENTATION
#ifndef POCKETSTEP_GRID_IMPLEMENTATION_ONCE
#define POCKETSTEP_GRID_IMPLEMENTATION_ONCE

static int ps_grid_valid(const struct ps_grid *grid)
{
    int cell_count;

    if (grid == 0 || grid->blocked == 0 ||
        grid->width <= 0 || grid->height <= 0)
        return 0;
    cell_count = grid->width * grid->height;
    return cell_count > 0 && cell_count <= PS_GRID_MAX_CELLS;
}

static int ps_grid_index(const struct ps_grid *grid, int x, int y)
{
    return y * grid->width + x;
}

int ps_grid_is_blocked(const struct ps_grid *grid, int x, int y)
{
    if (!ps_grid_valid(grid) || x < 0 || y < 0 ||
        x >= grid->width || y >= grid->height)
        return 1;
    return grid->blocked[ps_grid_index(grid, x, y)] != 0;
}

int ps_grid_find_path(const struct ps_grid *grid,
                      int start_x, int start_y,
                      int destination_x, int destination_y,
                      struct ps_grid_workspace *workspace,
                      struct ps_grid_path *path)
{
    static const int direction_x[4] = { 0, 1, 0, -1 };
    static const int direction_y[4] = { -1, 0, 1, 0 };
    int cell_count;
    int start;
    int destination;
    int head = 0;
    int tail = 0;
    int current;
    int route_length;
    int i;

    if (!ps_grid_valid(grid) || workspace == 0 || path == 0 ||
        path->capacity < 0 ||
        (path->capacity > 0 && path->cells == 0) ||
        ps_grid_is_blocked(grid, start_x, start_y) ||
        ps_grid_is_blocked(grid, destination_x, destination_y))
        return PS_PATH_INVALID;

    path->count = 0;
    start = ps_grid_index(grid, start_x, start_y);
    destination = ps_grid_index(grid, destination_x, destination_y);
    if (start == destination)
        return PS_PATH_FOUND;

    cell_count = grid->width * grid->height;
    for (i = 0; i < cell_count; ++i)
    {
        workspace->parent[i] = -1;
        workspace->seen[i] = 0;
    }

    workspace->queue[tail++] = start;
    workspace->seen[start] = 1;
    while (head < tail && !workspace->seen[destination])
    {
        int direction;
        int current_x;
        int current_y;

        current = workspace->queue[head++];
        current_x = current % grid->width;
        current_y = current / grid->width;
        for (direction = 0; direction < 4; ++direction)
        {
            int next_x = current_x + direction_x[direction];
            int next_y = current_y + direction_y[direction];
            int next;

            if (ps_grid_is_blocked(grid, next_x, next_y))
                continue;
            next = ps_grid_index(grid, next_x, next_y);
            if (workspace->seen[next])
                continue;
            workspace->seen[next] = 1;
            workspace->parent[next] = current;
            workspace->queue[tail++] = next;
            if (next == destination)
                break;
        }
    }

    if (!workspace->seen[destination])
        return PS_PATH_NO_ROUTE;

    route_length = 0;
    current = destination;
    while (current != start)
    {
        route_length++;
        current = workspace->parent[current];
    }
    if (route_length > path->capacity)
        return PS_PATH_CAPACITY;

    current = destination;
    for (i = route_length - 1; i >= 0; --i)
    {
        path->cells[i].x = current % grid->width;
        path->cells[i].y = current / grid->width;
        current = workspace->parent[current];
    }
    path->count = route_length;
    return PS_PATH_FOUND;
}

struct ps_rect ps_facing_probe(const struct ps_body *body,
                               int facing, int reach_pixels)
{
    struct ps_rect probe;

    probe.x = body->x / PS_ONE;
    probe.y = body->y / PS_ONE;
    probe.width = body->width;
    probe.height = body->height;
    if (reach_pixels < 0)
        reach_pixels = 0;
    if (facing == PS_GRID_UP)
        probe.y -= reach_pixels;
    else if (facing == PS_GRID_RIGHT)
        probe.x += reach_pixels;
    else if (facing == PS_GRID_DOWN)
        probe.y += reach_pixels;
    else if (facing == PS_GRID_LEFT)
        probe.x -= reach_pixels;
    return probe;
}

static int ps_rects_overlap(const struct ps_rect *a,
                            const struct ps_rect *b)
{
    return a->x < b->x + b->width && a->x + a->width > b->x &&
           a->y < b->y + b->height && a->y + a->height > b->y;
}

int ps_region_find(const struct ps_region *regions, int region_count,
                   const struct ps_rect *probe)
{
    int i;

    if (regions == 0 || probe == 0 || region_count <= 0)
        return -1;
    for (i = 0; i < region_count; ++i)
    {
        if (ps_rects_overlap(&regions[i].bounds, probe))
            return regions[i].id;
    }
    return -1;
}

int ps_region_find_facing(const struct ps_region *regions, int region_count,
                          const struct ps_body *body,
                          int facing, int reach_pixels)
{
    struct ps_rect probe;

    if (body == 0)
        return -1;
    probe = ps_facing_probe(body, facing, reach_pixels);
    return ps_region_find(regions, region_count, &probe);
}

#endif
#endif
