/*
 * PocketStep Inventory
 * Stable, fixed-capacity item stacks for constrained C programs.
 *
 * Copyright (C) 2026 Kevin Kinnett
 * SPDX-License-Identifier: MIT
 *
 * Define POCKETSTEP_INVENTORY_IMPLEMENTATION in one translation unit before
 * including this file.
 */

#ifndef POCKETSTEP_INVENTORY_H
#define POCKETSTEP_INVENTORY_H

#define PS_INVENTORY_OK 1
#define PS_INVENTORY_INVALID -1
#define PS_INVENTORY_FULL -2
#define PS_INVENTORY_STACK_LIMIT -3
#define PS_INVENTORY_NOT_FOUND -4

struct ps_inventory_slot
{
    int item_id;
    int quantity;
};

struct ps_inventory
{
    struct ps_inventory_slot *slots;
    int capacity;
    int count;
};

int ps_inventory_init(struct ps_inventory *inventory,
                      struct ps_inventory_slot *slots, int capacity);
void ps_inventory_clear(struct ps_inventory *inventory);
int ps_inventory_count(const struct ps_inventory *inventory);
const struct ps_inventory_slot *ps_inventory_get(
    const struct ps_inventory *inventory, int index);
int ps_inventory_quantity(const struct ps_inventory *inventory, int item_id);
int ps_inventory_add(struct ps_inventory *inventory, int item_id,
                     int quantity, int max_stack);
int ps_inventory_remove(struct ps_inventory *inventory, int item_id,
                        int quantity);

#endif

#ifdef POCKETSTEP_INVENTORY_IMPLEMENTATION
#ifndef POCKETSTEP_INVENTORY_IMPLEMENTATION_ONCE
#define POCKETSTEP_INVENTORY_IMPLEMENTATION_ONCE

static int ps_inventory_valid(const struct ps_inventory *inventory)
{
    return inventory != 0 && inventory->slots != 0 &&
           inventory->capacity > 0 && inventory->count >= 0 &&
           inventory->count <= inventory->capacity;
}

static int ps_inventory_find(const struct ps_inventory *inventory,
                             int item_id)
{
    int index;

    if (!ps_inventory_valid(inventory) || item_id <= 0)
        return -1;
    for (index = 0; index < inventory->count; ++index)
    {
        if (inventory->slots[index].item_id == item_id)
            return index;
    }
    return -1;
}

int ps_inventory_init(struct ps_inventory *inventory,
                      struct ps_inventory_slot *slots, int capacity)
{
    if (inventory == 0 || slots == 0 || capacity <= 0)
        return 0;
    inventory->slots = slots;
    inventory->capacity = capacity;
    inventory->count = 0;
    return 1;
}

void ps_inventory_clear(struct ps_inventory *inventory)
{
    if (inventory != 0)
        inventory->count = 0;
}

int ps_inventory_count(const struct ps_inventory *inventory)
{
    if (!ps_inventory_valid(inventory))
        return 0;
    return inventory->count;
}

const struct ps_inventory_slot *ps_inventory_get(
    const struct ps_inventory *inventory, int index)
{
    if (!ps_inventory_valid(inventory) ||
        index < 0 || index >= inventory->count)
        return 0;
    return &inventory->slots[index];
}

int ps_inventory_quantity(const struct ps_inventory *inventory, int item_id)
{
    int index = ps_inventory_find(inventory, item_id);

    if (index < 0)
        return 0;
    return inventory->slots[index].quantity;
}

int ps_inventory_add(struct ps_inventory *inventory, int item_id,
                     int quantity, int max_stack)
{
    int index;

    if (!ps_inventory_valid(inventory) || item_id <= 0 ||
        quantity <= 0 || max_stack <= 0)
        return PS_INVENTORY_INVALID;
    index = ps_inventory_find(inventory, item_id);
    if (index >= 0)
    {
        int retained = inventory->slots[index].quantity;

        if (retained <= 0)
            return PS_INVENTORY_INVALID;
        if (retained > max_stack || quantity > max_stack - retained)
            return PS_INVENTORY_STACK_LIMIT;
        inventory->slots[index].quantity = retained + quantity;
        return PS_INVENTORY_OK;
    }
    if (quantity > max_stack)
        return PS_INVENTORY_STACK_LIMIT;
    if (inventory->count == inventory->capacity)
        return PS_INVENTORY_FULL;
    inventory->slots[inventory->count].item_id = item_id;
    inventory->slots[inventory->count].quantity = quantity;
    inventory->count++;
    return PS_INVENTORY_OK;
}

int ps_inventory_remove(struct ps_inventory *inventory, int item_id,
                        int quantity)
{
    int index;
    int retained;

    if (!ps_inventory_valid(inventory) || item_id <= 0 || quantity <= 0)
        return PS_INVENTORY_INVALID;
    index = ps_inventory_find(inventory, item_id);
    if (index < 0)
        return PS_INVENTORY_NOT_FOUND;
    retained = inventory->slots[index].quantity;
    if (retained <= 0)
        return PS_INVENTORY_INVALID;
    if (quantity > retained)
        return PS_INVENTORY_NOT_FOUND;
    if (quantity < retained)
    {
        inventory->slots[index].quantity = retained - quantity;
        return PS_INVENTORY_OK;
    }
    for (; index + 1 < inventory->count; ++index)
        inventory->slots[index] = inventory->slots[index + 1];
    inventory->count--;
    return PS_INVENTORY_OK;
}

#endif
#endif
