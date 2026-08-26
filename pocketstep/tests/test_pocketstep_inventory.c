#include <assert.h>
#include <stdio.h>

#define POCKETSTEP_INVENTORY_IMPLEMENTATION
#include "../pocketstep_inventory.h"

static void test_initialization_and_queries(void)
{
    struct ps_inventory_slot storage[3];
    struct ps_inventory inventory;

    assert(!ps_inventory_init(0, storage, 3));
    assert(!ps_inventory_init(&inventory, 0, 3));
    assert(!ps_inventory_init(&inventory, storage, 0));
    assert(ps_inventory_init(&inventory, storage, 3));
    assert(ps_inventory_count(&inventory) == 0);
    assert(ps_inventory_quantity(&inventory, 1) == 0);
    assert(ps_inventory_get(&inventory, -1) == 0);
    assert(ps_inventory_get(&inventory, 0) == 0);
    assert(ps_inventory_count(0) == 0);
}

static void test_stable_order_and_stacking(void)
{
    struct ps_inventory_slot storage[3];
    struct ps_inventory inventory;

    assert(ps_inventory_init(&inventory, storage, 3));
    assert(ps_inventory_add(&inventory, 20, 1, 9) == PS_INVENTORY_OK);
    assert(ps_inventory_add(&inventory, 10, 2, 9) == PS_INVENTORY_OK);
    assert(ps_inventory_add(&inventory, 30, 1, 9) == PS_INVENTORY_OK);
    assert(ps_inventory_add(&inventory, 10, 3, 9) == PS_INVENTORY_OK);
    assert(ps_inventory_count(&inventory) == 3);
    assert(ps_inventory_get(&inventory, 0)->item_id == 20);
    assert(ps_inventory_get(&inventory, 1)->item_id == 10);
    assert(ps_inventory_get(&inventory, 1)->quantity == 5);
    assert(ps_inventory_get(&inventory, 2)->item_id == 30);
    assert(ps_inventory_quantity(&inventory, 10) == 5);
}

static void test_atomic_add_failures(void)
{
    struct ps_inventory_slot storage[2];
    struct ps_inventory inventory;

    assert(ps_inventory_init(&inventory, storage, 2));
    assert(ps_inventory_add(&inventory, 1, 2, 3) == PS_INVENTORY_OK);
    assert(ps_inventory_add(&inventory, 1, 2, 3) ==
           PS_INVENTORY_STACK_LIMIT);
    assert(ps_inventory_quantity(&inventory, 1) == 2);
    assert(ps_inventory_add(&inventory, 2, 1, 1) == PS_INVENTORY_OK);
    assert(ps_inventory_add(&inventory, 3, 1, 1) == PS_INVENTORY_FULL);
    assert(ps_inventory_count(&inventory) == 2);
    assert(ps_inventory_quantity(&inventory, 3) == 0);
    assert(ps_inventory_add(&inventory, 4, 2, 1) ==
           PS_INVENTORY_STACK_LIMIT);
    assert(ps_inventory_count(&inventory) == 2);
    assert(ps_inventory_add(&inventory, 0, 1, 1) == PS_INVENTORY_INVALID);
    assert(ps_inventory_add(&inventory, 5, 0, 1) == PS_INVENTORY_INVALID);
    assert(ps_inventory_add(&inventory, 5, 1, 0) == PS_INVENTORY_INVALID);
}

static void test_partial_and_complete_removal(void)
{
    struct ps_inventory_slot storage[4];
    struct ps_inventory inventory;

    assert(ps_inventory_init(&inventory, storage, 4));
    assert(ps_inventory_add(&inventory, 1, 4, 9) == PS_INVENTORY_OK);
    assert(ps_inventory_add(&inventory, 2, 1, 9) == PS_INVENTORY_OK);
    assert(ps_inventory_add(&inventory, 3, 2, 9) == PS_INVENTORY_OK);
    assert(ps_inventory_remove(&inventory, 1, 2) == PS_INVENTORY_OK);
    assert(ps_inventory_quantity(&inventory, 1) == 2);
    assert(ps_inventory_remove(&inventory, 2, 1) == PS_INVENTORY_OK);
    assert(ps_inventory_count(&inventory) == 2);
    assert(ps_inventory_get(&inventory, 0)->item_id == 1);
    assert(ps_inventory_get(&inventory, 1)->item_id == 3);
    assert(ps_inventory_remove(&inventory, 3, 3) ==
           PS_INVENTORY_NOT_FOUND);
    assert(ps_inventory_quantity(&inventory, 3) == 2);
    assert(ps_inventory_remove(&inventory, 99, 1) ==
           PS_INVENTORY_NOT_FOUND);
    assert(ps_inventory_remove(&inventory, 0, 1) == PS_INVENTORY_INVALID);
    assert(ps_inventory_remove(&inventory, 1, 0) == PS_INVENTORY_INVALID);
}

static void test_full_inventory_can_stack_and_clear(void)
{
    struct ps_inventory_slot storage[1];
    struct ps_inventory inventory;

    assert(ps_inventory_init(&inventory, storage, 1));
    assert(ps_inventory_add(&inventory, 7, 1, 5) == PS_INVENTORY_OK);
    assert(ps_inventory_add(&inventory, 7, 2, 5) == PS_INVENTORY_OK);
    assert(ps_inventory_quantity(&inventory, 7) == 3);
    ps_inventory_clear(&inventory);
    assert(ps_inventory_count(&inventory) == 0);
    assert(ps_inventory_quantity(&inventory, 7) == 0);
    assert(ps_inventory_add(&inventory, 8, 1, 1) == PS_INVENTORY_OK);
}

int main(void)
{
    test_initialization_and_queries();
    test_stable_order_and_stacking();
    test_atomic_add_failures();
    test_partial_and_complete_removal();
    test_full_inventory_can_stack_and_clear();
    puts("PocketStep inventory tests passed");
    return 0;
}
