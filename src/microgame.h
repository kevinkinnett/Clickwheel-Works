/*
 * Microgame
 * Fixed-point motion and collision for small C games.
 *
 * Copyright (C) 2026 Kevin and OpenAI
 * SPDX-License-Identifier: MIT
 *
 * Define MICROGAME_IMPLEMENTATION in one translation unit before including
 * this file. Positions and velocities use four fractional bits. Collider
 * coordinates and body sizes use whole pixels.
 */

#ifndef MICROGAME_H
#define MICROGAME_H

#define MG_SHIFT 4
#define MG_ONE (1 << MG_SHIFT)

#ifndef MG_MAX_SOLIDS
#define MG_MAX_SOLIDS 16
#endif

struct mg_rect
{
    int x;
    int y;
    int width;
    int height;
};

struct mg_solid
{
    struct mg_rect bounds;
    int id;
};

struct mg_world
{
    struct mg_solid solids[MG_MAX_SOLIDS];
    int solid_count;
};

struct mg_body
{
    int x;
    int y;
    int vx;
    int vy;
    int width;
    int height;
};

struct mg_move_result
{
    int previous_x;
    int previous_y;
    int hit_left;
    int hit_right;
    int hit_ceiling;
    int hit_floor;
    int horizontal_id;
    int vertical_id;
};

void mg_world_clear(struct mg_world *world);
int mg_world_add_solid(struct mg_world *world, int id,
                       int x, int y, int width, int height);
void mg_apply_gravity(struct mg_body *body, int acceleration,
                      int terminal_velocity);
struct mg_move_result mg_move(struct mg_world *world,
                              struct mg_body *body);
int mg_overlap(const struct mg_body *a, const struct mg_body *b);
int mg_crossed_top(const struct mg_body *previous,
                   const struct mg_body *current,
                   const struct mg_body *target,
                   int tolerance_pixels);

#endif

#ifdef MICROGAME_IMPLEMENTATION
#ifndef MICROGAME_IMPLEMENTATION_ONCE
#define MICROGAME_IMPLEMENTATION_ONCE

static int mg_ranges_overlap(int a0, int a1, int b0, int b1)
{
    return a0 < b1 && a1 > b0;
}

void mg_world_clear(struct mg_world *world)
{
    world->solid_count = 0;
}

int mg_world_add_solid(struct mg_world *world, int id,
                       int x, int y, int width, int height)
{
    struct mg_solid *solid;

    if (world->solid_count >= MG_MAX_SOLIDS || width <= 0 || height <= 0)
        return 0;

    solid = &world->solids[world->solid_count++];
    solid->bounds.x = x;
    solid->bounds.y = y;
    solid->bounds.width = width;
    solid->bounds.height = height;
    solid->id = id;
    return 1;
}

void mg_apply_gravity(struct mg_body *body, int acceleration,
                      int terminal_velocity)
{
    body->vy += acceleration;
    if (body->vy > terminal_velocity)
        body->vy = terminal_velocity;
}

struct mg_move_result mg_move(struct mg_world *world,
                              struct mg_body *body)
{
    struct mg_move_result result;
    int i;
    int candidate;
    int proposed;

    result.previous_x = body->x;
    result.previous_y = body->y;
    result.hit_left = 0;
    result.hit_right = 0;
    result.hit_ceiling = 0;
    result.hit_floor = 0;
    result.horizontal_id = -1;
    result.vertical_id = -1;

    candidate = body->x + body->vx;
    for (i = 0; i < world->solid_count; ++i)
    {
        const struct mg_solid *solid = &world->solids[i];
        int left = solid->bounds.x * MG_ONE;
        int right = (solid->bounds.x + solid->bounds.width) * MG_ONE;
        int top = solid->bounds.y * MG_ONE;
        int bottom = (solid->bounds.y + solid->bounds.height) * MG_ONE;
        int body_top = body->y;
        int body_bottom = body->y + body->height * MG_ONE;

        if (!mg_ranges_overlap(body_top, body_bottom, top, bottom))
            continue;

        if (body->vx > 0 &&
            body->x + body->width * MG_ONE <= left &&
            candidate + body->width * MG_ONE > left)
        {
            candidate = left - body->width * MG_ONE;
            result.hit_right = 1;
            result.horizontal_id = solid->id;
        }
        else if (body->vx < 0 && body->x >= right && candidate < right)
        {
            candidate = right;
            result.hit_left = 1;
            result.horizontal_id = solid->id;
        }
    }
    body->x = candidate;

    candidate = body->y + body->vy;
    proposed = candidate;
    for (i = 0; i < world->solid_count; ++i)
    {
        const struct mg_solid *solid = &world->solids[i];
        int left = solid->bounds.x * MG_ONE;
        int right = (solid->bounds.x + solid->bounds.width) * MG_ONE;
        int top = solid->bounds.y * MG_ONE;
        int bottom = (solid->bounds.y + solid->bounds.height) * MG_ONE;
        int body_left = body->x;
        int body_right = body->x + body->width * MG_ONE;
        int body_center = body_left + body->width * MG_ONE / 2;
        int resolved;

        if (!mg_ranges_overlap(body_left, body_right, left, right))
            continue;

        if (body->vy > 0 &&
            body->y + body->height * MG_ONE <= top &&
            proposed + body->height * MG_ONE > top)
        {
            resolved = top - body->height * MG_ONE;
            if (!result.hit_floor || resolved < candidate)
            {
                candidate = resolved;
                result.hit_floor = 1;
                result.vertical_id = solid->id;
            }
            else if (resolved == candidate &&
                     body_center >= left && body_center < right)
                result.vertical_id = solid->id;
        }
        else if (body->vy < 0 && body->y >= bottom && proposed < bottom)
        {
            resolved = bottom;
            if (!result.hit_ceiling || resolved > candidate)
            {
                candidate = resolved;
                result.hit_ceiling = 1;
                result.vertical_id = solid->id;
            }
            else if (resolved == candidate &&
                     body_center >= left && body_center < right)
                result.vertical_id = solid->id;
        }
    }
    body->y = candidate;

    if (result.hit_left || result.hit_right)
        body->vx = 0;
    if (result.hit_ceiling || result.hit_floor)
        body->vy = 0;
    return result;
}

int mg_overlap(const struct mg_body *a, const struct mg_body *b)
{
    return mg_ranges_overlap(a->x, a->x + a->width * MG_ONE,
                             b->x, b->x + b->width * MG_ONE) &&
           mg_ranges_overlap(a->y, a->y + a->height * MG_ONE,
                             b->y, b->y + b->height * MG_ONE);
}

int mg_crossed_top(const struct mg_body *previous,
                   const struct mg_body *current,
                   const struct mg_body *target,
                   int tolerance_pixels)
{
    int previous_bottom = previous->y + previous->height * MG_ONE;
    int current_bottom = current->y + current->height * MG_ONE;
    int target_top = target->y;

    if (current->vy <= 0)
        return 0;
    if (!mg_ranges_overlap(current->x,
                           current->x + current->width * MG_ONE,
                           target->x,
                           target->x + target->width * MG_ONE))
        return 0;
    return previous_bottom <= target_top + tolerance_pixels * MG_ONE &&
           current_bottom >= target_top;
}

#endif
#endif
