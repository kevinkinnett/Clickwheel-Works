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
}

static void test_stable_order_and_payload(void)
{
    struct ps_drawable storage[4];
    struct ps_draw_list list;
    const struct ps_drawable records[4] = {
        { 1, 10, 11, 50, 101 },
        { 2, 20, 21, 20, 102 },
        { 3, 30, 31, 50, 103 },
        { 4, 40, 41, 35, 104 }
    };

    assert(ps_draw_list_init(&list, storage, 4));
    assert(ps_draw_list_add(&list, &records[0]));
    assert(ps_draw_list_add(&list, &records[1]));
    assert(ps_draw_list_add(&list, &records[2]));
    assert(ps_draw_list_add(&list, &records[3]));
    assert(ps_draw_list_get(&list, 0)->id == 102);
    assert(ps_draw_list_get(&list, 1)->id == 104);
    assert(ps_draw_list_get(&list, 2)->id == 101);
    assert(ps_draw_list_get(&list, 3)->id == 103);
    assert(ps_draw_list_get(&list, 3)->x == 30);
    assert(ps_draw_list_get(&list, 3)->y == 31);
    assert(ps_draw_list_get(&list, 3)->kind == 3);
}

static void test_capacity_and_clear(void)
{
    struct ps_drawable storage[1];
    struct ps_draw_list list;
    struct ps_drawable first = { 1, 2, 3, 4, 5 };
    struct ps_drawable extra = { 6, 7, 8, 0, 9 };

    assert(ps_draw_list_init(&list, storage, 1));
    assert(ps_draw_list_add(&list, &first));
    assert(!ps_draw_list_add(&list, &extra));
    assert(list.count == 1 && storage[0].id == 5);
    assert(ps_draw_list_get(&list, -1) == 0);
    assert(ps_draw_list_get(&list, 1) == 0);
    ps_draw_list_clear(&list);
    assert(list.count == 0);
    assert(ps_draw_list_add(&list, &extra));
}

int main(void)
{
    test_initialization();
    test_stable_order_and_payload();
    test_capacity_and_clear();
    puts("PocketStep draw tests passed");
    return 0;
}
