/*
 * PocketStep Draw
 * Stable, fixed-capacity depth ordering for small drawable sets.
 *
 * Copyright (C) 2026 Kevin Kinnett
 * SPDX-License-Identifier: MIT
 *
 * Define POCKETSTEP_DRAW_IMPLEMENTATION in one translation unit before
 * including this file.
 */

#ifndef POCKETSTEP_DRAW_H
#define POCKETSTEP_DRAW_H

struct ps_drawable
{
    int kind;
    int x;
    int y;
    int foot_y;
    int id;
};

struct ps_draw_list
{
    struct ps_drawable *records;
    int capacity;
    int count;
};

int ps_draw_list_init(struct ps_draw_list *list,
                      struct ps_drawable *records, int capacity);
void ps_draw_list_clear(struct ps_draw_list *list);
int ps_draw_list_add(struct ps_draw_list *list,
                     const struct ps_drawable *record);
const struct ps_drawable *ps_draw_list_get(const struct ps_draw_list *list,
                                           int index);

#endif

#ifdef POCKETSTEP_DRAW_IMPLEMENTATION
#ifndef POCKETSTEP_DRAW_IMPLEMENTATION_ONCE
#define POCKETSTEP_DRAW_IMPLEMENTATION_ONCE

int ps_draw_list_init(struct ps_draw_list *list,
                      struct ps_drawable *records, int capacity)
{
    if (list == 0 || records == 0 || capacity <= 0)
        return 0;
    list->records = records;
    list->capacity = capacity;
    list->count = 0;
    return 1;
}

void ps_draw_list_clear(struct ps_draw_list *list)
{
    if (list != 0)
        list->count = 0;
}

int ps_draw_list_add(struct ps_draw_list *list,
                     const struct ps_drawable *record)
{
    int index;

    if (list == 0 || record == 0 || list->records == 0 ||
        list->capacity <= 0 || list->count < 0 ||
        list->count >= list->capacity)
        return 0;
    index = list->count;
    while (index > 0 &&
           list->records[index - 1].foot_y > record->foot_y)
    {
        list->records[index] = list->records[index - 1];
        index--;
    }
    list->records[index] = *record;
    list->count++;
    return 1;
}

const struct ps_drawable *ps_draw_list_get(const struct ps_draw_list *list,
                                           int index)
{
    if (list == 0 || list->records == 0 ||
        index < 0 || index >= list->count)
        return 0;
    return &list->records[index];
}

#endif
#endif
