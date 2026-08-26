#include <assert.h>
#include <stdio.h>

#define POCKETSTEP_DRAW_IMPLEMENTATION
#include "../pocketstep_draw.h"

static void test_initialization(void)
{
    struct ps_drawable storage[2];
    struct ps_draw_list list;

    assert(!ps_draw_list_init(0, storage, 2));
    assert(!ps_draw_list_init(&list, 0, 2));
    assert(!ps_draw_list_init(&list, storage, 0));
    assert(ps_draw_list_init(&list, storage, 2));
    assert(list.count == 0 && list.capacity == 2);
    assert(ps_draw_list_discarded(&list) == 0);
    assert(ps_draw_list_discarded(0) == 0);
}

static void test_stable_order_and_payload(void)
{
    struct ps_drawable storage[4];
    struct ps_draw_list list;
    const struct ps_drawable records[4] = {
        { 1, 10, 11, 50, 101, PS_DRAW_PRIORITY_REQUIRED },
        { 2, 20, 21, 20, 102, PS_DRAW_PRIORITY_OPTIONAL },
        { 3, 30, 31, 50, 103, PS_DRAW_PRIORITY_OPTIONAL },
        { 4, 40, 41, 35, 104, PS_DRAW_PRIORITY_REQUIRED }
    };

    assert(ps_draw_list_init(&list, storage, 4));
    assert(ps_draw_list_add_prioritized(&list, &records[0]));
    assert(ps_draw_list_add_prioritized(&list, &records[1]));
    assert(ps_draw_list_add_prioritized(&list, &records[2]));
    assert(ps_draw_list_add_prioritized(&list, &records[3]));
    assert(ps_draw_list_get(&list, 0)->id == 102);
    assert(ps_draw_list_get(&list, 1)->id == 104);
    assert(ps_draw_list_get(&list, 2)->id == 101);
    assert(ps_draw_list_get(&list, 3)->id == 103);
    assert(ps_draw_list_get(&list, 3)->x == 30);
    assert(ps_draw_list_get(&list, 3)->y == 31);
    assert(ps_draw_list_get(&list, 3)->kind == 3);
    assert(ps_draw_list_get(&list, 1)->retention_priority ==
           PS_DRAW_PRIORITY_REQUIRED);
    assert(ps_draw_list_discarded(&list) == 0);
}

static void test_optional_default(void)
{
    struct ps_drawable record = {
        .kind = 5,
        .x = 6,
        .y = 7,
        .foot_y = 8,
        .id = 9
    };

    assert(record.retention_priority == PS_DRAW_PRIORITY_OPTIONAL);
}

static void test_legacy_capacity_and_clear(void)
{
    struct ps_drawable storage[1];
    struct ps_draw_list list;
    struct ps_drawable first = {
        1, 2, 3, 4, 5, PS_DRAW_PRIORITY_OPTIONAL
    };
    struct ps_drawable extra = {
        6, 7, 8, 0, 9, PS_DRAW_PRIORITY_REQUIRED
    };

    assert(ps_draw_list_init(&list, storage, 1));
    assert(ps_draw_list_add(&list, &first));
    assert(!ps_draw_list_add(&list, &extra));
    assert(list.count == 1 && storage[0].id == 5);
    assert(ps_draw_list_discarded(&list) == 1);
    assert(ps_draw_list_get(&list, -1) == 0);
    assert(ps_draw_list_get(&list, 1) == 0);
    ps_draw_list_clear(&list);
    assert(list.count == 0);
    assert(ps_draw_list_discarded(&list) == 0);
    assert(ps_draw_list_add(&list, &extra));
}

static void test_prioritized_replacement_is_deterministic(void)
{
    struct ps_drawable storage[3];
    struct ps_draw_list list;
    const struct ps_drawable back_optional = {
        1, 0, 0, 10, 10, PS_DRAW_PRIORITY_OPTIONAL
    };
    const struct ps_drawable required = {
        2, 0, 0, 20, 20, PS_DRAW_PRIORITY_REQUIRED
    };
    const struct ps_drawable front_optional = {
        3, 0, 0, 30, 30, PS_DRAW_PRIORITY_OPTIONAL
    };
    const struct ps_drawable incoming = {
        4, 0, 0, 25, 40, PS_DRAW_PRIORITY_REQUIRED
    };

    assert(ps_draw_list_init(&list, storage, 3));
    assert(ps_draw_list_add_prioritized(&list, &back_optional));
    assert(ps_draw_list_add_prioritized(&list, &required));
    assert(ps_draw_list_add_prioritized(&list, &front_optional));
    assert(ps_draw_list_add_prioritized(&list, &incoming));
    assert(list.count == 3);
    assert(ps_draw_list_get(&list, 0)->id == 10);
    assert(ps_draw_list_get(&list, 1)->id == 20);
    assert(ps_draw_list_get(&list, 2)->id == 40);
    assert(ps_draw_list_discarded(&list) == 1);
}

static void test_equal_priority_keeps_existing_records(void)
{
    struct ps_drawable storage[2];
    struct ps_draw_list list;
    const struct ps_drawable first = {
        1, 0, 0, 10, 10, PS_DRAW_PRIORITY_OPTIONAL
    };
    const struct ps_drawable second = {
        2, 0, 0, 20, 20, PS_DRAW_PRIORITY_OPTIONAL
    };
    const struct ps_drawable incoming = {
        3, 0, 0, 5, 30, PS_DRAW_PRIORITY_OPTIONAL
    };

    assert(ps_draw_list_init(&list, storage, 2));
    assert(ps_draw_list_add_prioritized(&list, &first));
    assert(ps_draw_list_add_prioritized(&list, &second));
    assert(!ps_draw_list_add_prioritized(&list, &incoming));
    assert(ps_draw_list_get(&list, 0)->id == 10);
    assert(ps_draw_list_get(&list, 1)->id == 20);
    assert(ps_draw_list_discarded(&list) == 1);
}

static void test_actor_survives_scenery_overflow(void)
{
    struct ps_drawable storage[3];
    struct ps_draw_list list;
    const struct ps_drawable scenery[4] = {
        { 1, 0, 0, 10, 10, PS_DRAW_PRIORITY_OPTIONAL },
        { 1, 0, 0, 20, 20, PS_DRAW_PRIORITY_OPTIONAL },
        { 1, 0, 0, 30, 30, PS_DRAW_PRIORITY_OPTIONAL },
        { 1, 0, 0, 40, 40, PS_DRAW_PRIORITY_OPTIONAL }
    };
    const struct ps_drawable actor = {
        2, 0, 0, 25, 99, PS_DRAW_PRIORITY_REQUIRED
    };
    int index;
    int found_actor = 0;

    assert(ps_draw_list_init(&list, storage, 3));
    assert(ps_draw_list_add_prioritized(&list, &scenery[0]));
    assert(ps_draw_list_add_prioritized(&list, &scenery[1]));
    assert(ps_draw_list_add_prioritized(&list, &scenery[2]));
    assert(ps_draw_list_add_prioritized(&list, &actor));
    assert(!ps_draw_list_add_prioritized(&list, &scenery[3]));
    assert(!ps_draw_list_add_prioritized(&list, 0));
    for (index = 0; index < list.count; ++index)
    {
        if (ps_draw_list_get(&list, index)->id == actor.id)
            found_actor = 1;
    }
    assert(found_actor);
    assert(ps_draw_list_discarded(&list) == 2);
    ps_draw_list_clear(&list);
    assert(ps_draw_list_discarded(&list) == 0);
}

int main(void)
{
    test_initialization();
    test_stable_order_and_payload();
    test_optional_default();
    test_legacy_capacity_and_clear();
    test_prioritized_replacement_is_deterministic();
    test_equal_priority_keeps_existing_records();
    test_actor_survives_scenery_overflow();
    puts("PocketStep draw tests passed");
    return 0;
}
