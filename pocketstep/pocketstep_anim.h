/*
 * PocketStep Animation
 * Portable directional sprite-frame selection for small games.
 *
 * Copyright (C) 2026 Kevin Kinnett
 * SPDX-License-Identifier: MIT
 *
 * Define POCKETSTEP_ANIM_IMPLEMENTATION in one translation unit before
 * including this file.
 */

#ifndef POCKETSTEP_ANIM_H
#define POCKETSTEP_ANIM_H

#define PS_ANIM_DIRECTION_COUNT 4

struct ps_anim_sheet
{
    int sheet_width;
    int sheet_height;
    int frame_width;
    int frame_height;
    int frames_per_direction;
    int idle_frame;
    int facing_rows[PS_ANIM_DIRECTION_COUNT];
};

struct ps_anim_frame
{
    int x;
    int y;
    int width;
    int height;
    int index;
    int row;
};

int ps_anim_sheet_valid(const struct ps_anim_sheet *sheet);
int ps_anim_select(const struct ps_anim_sheet *sheet,
                   int facing, int moving,
                   int distance_pixels, int distance_per_frame,
                   struct ps_anim_frame *frame);

#endif

#ifdef POCKETSTEP_ANIM_IMPLEMENTATION
#ifndef POCKETSTEP_ANIM_IMPLEMENTATION_ONCE
#define POCKETSTEP_ANIM_IMPLEMENTATION_ONCE

int ps_anim_sheet_valid(const struct ps_anim_sheet *sheet)
{
    int direction;

    if (sheet == 0 || sheet->sheet_width <= 0 || sheet->sheet_height <= 0 ||
        sheet->frame_width <= 0 || sheet->frame_height <= 0 ||
        sheet->frames_per_direction <= 0 || sheet->idle_frame < 0 ||
        sheet->idle_frame >= sheet->frames_per_direction ||
        sheet->frames_per_direction * sheet->frame_width > sheet->sheet_width)
        return 0;
    for (direction = 0; direction < PS_ANIM_DIRECTION_COUNT; ++direction)
    {
        int row = sheet->facing_rows[direction];
        if (row < 0 || (row + 1) * sheet->frame_height > sheet->sheet_height)
            return 0;
    }
    return 1;
}

int ps_anim_select(const struct ps_anim_sheet *sheet,
                   int facing, int moving,
                   int distance_pixels, int distance_per_frame,
                   struct ps_anim_frame *frame)
{
    int index;
    int row;

    if (!ps_anim_sheet_valid(sheet) || frame == 0 ||
        facing < 0 || facing >= PS_ANIM_DIRECTION_COUNT ||
        distance_pixels < 0 || (moving && distance_per_frame <= 0))
        return 0;
    index = moving ?
        (distance_pixels / distance_per_frame) % sheet->frames_per_direction :
        sheet->idle_frame;
    row = sheet->facing_rows[facing];
    frame->x = index * sheet->frame_width;
    frame->y = row * sheet->frame_height;
    frame->width = sheet->frame_width;
    frame->height = sheet->frame_height;
    frame->index = index;
    frame->row = row;
    return 1;
}

#endif
#endif
