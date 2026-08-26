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

#define PS_DRAW_PRIORITY_OPTIONAL 0
#define PS_DRAW_PRIORITY_REQUIRED 1

struct ps_drawable
{
    int kind;
    int x;
    int y;
    int foot_y;
    int id;
    int retention_priority;
};

struct ps_draw_list
{
    struct ps_drawable *records;
    int capacity;
    int count;
    int discarded_count;
};

int ps_draw_list_init(struct ps_draw_list *list,
                      struct ps_drawable *records, int capacity);
void ps_draw_list_clear(struct ps_draw_list *list);
int ps_draw_list_add(struct ps_draw_list *list,
                     const struct ps_drawable *record);
int ps_draw_list_add_prioritized(struct ps_draw_list *list,
                                 const struct ps_drawable *record);
const struct ps_drawable *ps_draw_list_get(const struct ps_draw_list *list,
                                           int index);
int ps_draw_list_discarded(const struct ps_draw_list *list);

#endif

#ifdef POCKETSTEP_DRAW_IMPLEMENTATION
#ifndef POCKETSTEP_DRAW_IMPLEMENTATION_ONCE
#define POCKETSTEP_DRAW_IMPLEMENTATION_ONCE

static int ps_draw_list_insert(struct ps_draw_list *list,
                               const struct ps_drawable *record)
{
    int index = list->count;

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

int ps_draw_list_init(struct ps_draw_list *list,
                      struct ps_drawable *records, int capacity)
{
    if (list == 0 || records == 0 || capacity <= 0)
        return 0;
    list->records = records;
    list->capacity = capacity;
    list->count = 0;
    list->discarded_count = 0;
    return 1;
}

void ps_draw_list_clear(struct ps_draw_list *list)
{
    if (list != 0)
    {
        list->count = 0;
        list->discarded_count = 0;
    }
}

int ps_draw_list_add(struct ps_draw_list *list,
                     const struct ps_drawable *record)
{
    if (list == 0 || record == 0 || list->records == 0 ||
        list->capacity <= 0 || list->count < 0 ||
        list->count > list->capacity)
        return 0;
    if (list->count == list->capacity)
    {
        list->discarded_count++;
        return 0;
    }
    return ps_draw_list_insert(list, record);
}

int ps_draw_list_add_prioritized(struct ps_draw_list *list,
                                 const struct ps_drawable *record)
{
    int discard_index;
    int lowest_priority;
    int index;

    if (list == 0 || record == 0 || list->records == 0 ||
        list->capacity <= 0 || list->count < 0 ||
        list->count > list->capacity)
        return 0;
    if (list->count < list->capacity)
        return ps_draw_list_insert(list, record);

    discard_index = 0;
    lowest_priority = list->records[0].retention_priority;
    for (index = 1; index < list->count; ++index)
    {
        int priority = list->records[index].retention_priority;

        if (priority <= lowest_priority)
        {
            lowest_priority = priority;
            discard_index = index;
        }
    }
    list->discarded_count++;
    if (record->retention_priority <= lowest_priority)
        return 0;

    for (index = discard_index; index + 1 < list->count; ++index)
        list->records[index] = list->records[index + 1];
    list->count--;
    return ps_draw_list_insert(list, record);
}

const struct ps_drawable *ps_draw_list_get(const struct ps_draw_list *list,
                                           int index)
{
    if (list == 0 || list->records == 0 ||
        index < 0 || index >= list->count)
        return 0;
    return &list->records[index];
}

int ps_draw_list_discarded(const struct ps_draw_list *list)
{
    if (list == 0)
        return 0;
    return list->discarded_count;
}

#endif
#endif
