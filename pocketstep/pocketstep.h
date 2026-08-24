/*
 * PocketStep
 * Fixed-point motion and collision for small C games.
 *
 * Copyright (C) 2026 Kevin Kinnett
 * SPDX-License-Identifier: MIT
 *
 * Define POCKETSTEP_IMPLEMENTATION in one translation unit before including
 * this file. Positions and velocities use four fractional bits. Collider
 * coordinates and body sizes use whole pixels.
 */

#ifndef POCKETSTEP_H
#define POCKETSTEP_H

#define PS_SHIFT 4
#define PS_ONE (1 << PS_SHIFT)

#ifndef PS_MAX_SOLIDS
#define PS_MAX_SOLIDS 16
#endif

struct ps_rect
{
    int x;
    int y;
    int width;
    int height;
};

struct ps_solid
{
    struct ps_rect bounds;
    int id;
};

struct ps_world
{
    struct ps_solid solids[PS_MAX_SOLIDS];
    int solid_count;
};

struct ps_body
{
    int x;
    int y;
    int vx;
    int vy;
    int width;
    int height;
};

struct ps_move_result
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

void ps_world_clear(struct ps_world *world);
int ps_world_add_solid(struct ps_world *world, int id,
                       int x, int y, int width, int height);
void ps_apply_gravity(struct ps_body *body, int acceleration,
                      int terminal_velocity);
struct ps_move_result ps_move(struct ps_world *world,
                              struct ps_body *body);
int ps_overlap(const struct ps_body *a, const struct ps_body *b);
int ps_crossed_top(const struct ps_body *previous,
                   const struct ps_body *current,
                   const struct ps_body *target,
                   int tolerance_pixels);

#endif

#ifdef POCKETSTEP_IMPLEMENTATION
#ifndef POCKETSTEP_IMPLEMENTATION_ONCE
#define POCKETSTEP_IMPLEMENTATION_ONCE

static int ps_ranges_overlap(int a0, int a1, int b0, int b1)
{
    return a0 < b1 && a1 > b0;
}

void ps_world_clear(struct ps_world *world)
{
    world->solid_count = 0;
}

int ps_world_add_solid(struct ps_world *world, int id,
                       int x, int y, int width, int height)
{
    struct ps_solid *solid;

    if (world->solid_count >= PS_MAX_SOLIDS || width <= 0 || height <= 0)
        return 0;

    solid = &world->solids[world->solid_count++];
    solid->bounds.x = x;
    solid->bounds.y = y;
    solid->bounds.width = width;
    solid->bounds.height = height;
    solid->id = id;
    return 1;
}

void ps_apply_gravity(struct ps_body *body, int acceleration,
                      int terminal_velocity)
{
    body->vy += acceleration;
    if (body->vy > terminal_velocity)
        body->vy = terminal_velocity;
}

struct ps_move_result ps_move(struct ps_world *world,
                              struct ps_body *body)
{
    struct ps_move_result result;
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
        const struct ps_solid *solid = &world->solids[i];
        int left = solid->bounds.x * PS_ONE;
        int right = (solid->bounds.x + solid->bounds.width) * PS_ONE;
        int top = solid->bounds.y * PS_ONE;
        int bottom = (solid->bounds.y + solid->bounds.height) * PS_ONE;
        int body_top = body->y;
        int body_bottom = body->y + body->height * PS_ONE;

        if (!ps_ranges_overlap(body_top, body_bottom, top, bottom))
            continue;

        if (body->vx > 0 &&
            body->x + body->width * PS_ONE <= left &&
            candidate + body->width * PS_ONE > left)
        {
            candidate = left - body->width * PS_ONE;
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
        const struct ps_solid *solid = &world->solids[i];
        int left = solid->bounds.x * PS_ONE;
        int right = (solid->bounds.x + solid->bounds.width) * PS_ONE;
        int top = solid->bounds.y * PS_ONE;
        int bottom = (solid->bounds.y + solid->bounds.height) * PS_ONE;
        int body_left = body->x;
        int body_right = body->x + body->width * PS_ONE;
        int body_center = body_left + body->width * PS_ONE / 2;
        int resolved;

        if (!ps_ranges_overlap(body_left, body_right, left, right))
            continue;

        if (body->vy > 0 &&
            body->y + body->height * PS_ONE <= top &&
            proposed + body->height * PS_ONE > top)
        {
            resolved = top - body->height * PS_ONE;
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

int ps_overlap(const struct ps_body *a, const struct ps_body *b)
{
    return ps_ranges_overlap(a->x, a->x + a->width * PS_ONE,
                             b->x, b->x + b->width * PS_ONE) &&
           ps_ranges_overlap(a->y, a->y + a->height * PS_ONE,
                             b->y, b->y + b->height * PS_ONE);
}

int ps_crossed_top(const struct ps_body *previous,
                   const struct ps_body *current,
                   const struct ps_body *target,
                   int tolerance_pixels)
{
    int previous_bottom = previous->y + previous->height * PS_ONE;
    int current_bottom = current->y + current->height * PS_ONE;
    int target_top = target->y;

    if (current->vy <= 0)
        return 0;
    if (!ps_ranges_overlap(current->x,
                           current->x + current->width * PS_ONE,
                           target->x,
                           target->x + target->width * PS_ONE))
        return 0;
    return previous_bottom <= target_top + tolerance_pixels * PS_ONE &&
           current_bottom >= target_top;
}

#endif
#endif
