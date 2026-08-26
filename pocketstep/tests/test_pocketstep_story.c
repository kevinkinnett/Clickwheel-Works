#include <assert.h>
#include <stdio.h>

#define POCKETSTEP_STORY_IMPLEMENTATION
#include "../pocketstep_story.h"

struct test_context
{
    int calls;
    int pending_calls;
    int reset_calls;
    int last_kind;
};

static int complete_action(const struct ps_story_action *action, void *data)
{
    struct test_context *context = (struct test_context *)data;
    context->calls++;
    context->last_kind = action->kind;
    return PS_STORY_ACTION_DONE;
}

static int pending_twice(const struct ps_story_action *action, void *data)
{
    struct test_context *context = (struct test_context *)data;
    context->calls++;
    context->last_kind = action->kind;
    context->pending_calls++;
    return context->pending_calls < 3 ?
           PS_STORY_ACTION_PENDING : PS_STORY_ACTION_DONE;
}

static int fail_action(const struct ps_story_action *action, void *data)
{
    struct test_context *context = (struct test_context *)data;
    context->calls++;
    context->last_kind = action->kind;
    return PS_STORY_ACTION_FAILED;
}

static void count_reset(void *data)
{
    struct test_context *context = (struct test_context *)data;
    context->reset_calls++;
}

static void test_order_and_wait(void)
{
    static const struct ps_story_action actions[] = {
        { PS_STORY_ACTION_WAIT, 3, 0, 0, 0 },
        { PS_STORY_ACTION_FACE, 2, 0, 0, 0 },
        { PS_STORY_ACTION_END, 0, 0, 0, 0 }
    };
    struct ps_story_director director;
    struct test_context context = { 0, 0, 0, 0 };

    assert(ps_story_init(&director, actions, 3, 0));
    assert(ps_story_update(&director, complete_action, 0, &context) ==
           PS_STORY_RUNNING);
    assert(context.calls == 0 && director.action_index == 0);
    ps_story_update(&director, complete_action, 0, &context);
    assert(context.calls == 0 && director.action_index == 0);
    ps_story_update(&director, complete_action, 0, &context);
    assert(context.calls == 0 && director.action_index == 1);
    ps_story_update(&director, complete_action, 0, &context);
    assert(context.calls == 1 && context.last_kind == PS_STORY_ACTION_FACE);
    assert(director.action_index == 2);
    assert(ps_story_update(&director, complete_action, 0, &context) ==
           PS_STORY_COMPLETE);
}

static void test_pending_action_blocks_next(void)
{
    static const struct ps_story_action actions[] = {
        { PS_STORY_ACTION_WALK, 4, 5, 0, 0 },
        { PS_STORY_ACTION_SAY, 0, 0, 0, "Arrived." },
        { PS_STORY_ACTION_END, 0, 0, 0, 0 }
    };
    struct ps_story_director director;
    struct test_context context = { 0, 0, 0, 0 };

    assert(ps_story_init(&director, actions, 3, 0));
    ps_story_update(&director, pending_twice, 0, &context);
    assert(director.action_index == 0 && context.calls == 1);
    ps_story_update(&director, pending_twice, 0, &context);
    assert(director.action_index == 0 && context.calls == 2);
    ps_story_update(&director, pending_twice, 0, &context);
    assert(director.action_index == 1 && context.calls == 3);
}

static void test_failure_stops_progress(void)
{
    static const struct ps_story_action actions[] = {
        { PS_STORY_ACTION_WALK, 9, 9, 0, 0 },
        { PS_STORY_ACTION_END, 0, 0, 0, 0 }
    };
    struct ps_story_director director;
    struct test_context context = { 0, 0, 0, 0 };

    assert(ps_story_init(&director, actions, 2, 0));
    assert(ps_story_update(&director, fail_action, 0, &context) ==
           PS_STORY_FAILED);
    assert(director.action_index == 0);
    assert(ps_story_update(&director, complete_action, 0, &context) ==
           PS_STORY_FAILED);
    assert(context.calls == 1);
}

static void test_loop_reset(void)
{
    static const struct ps_story_action actions[] = {
        { PS_STORY_ACTION_FACE, 1, 0, 0, 0 },
        { PS_STORY_ACTION_END, 0, 0, 0, 0 }
    };
    struct ps_story_director director;
    struct test_context context = { 0, 0, 0, 0 };

    assert(ps_story_init(&director, actions, 2, 1));
    ps_story_update(&director, complete_action, count_reset, &context);
    assert(director.action_index == 1);
    assert(ps_story_update(&director, complete_action,
                           count_reset, &context) == PS_STORY_RUNNING);
    assert(context.reset_calls == 1);
    assert(director.action_index == 0);
    ps_story_update(&director, complete_action, count_reset, &context);
    ps_story_reset(&director);
    assert(director.action_index == 0 &&
           director.state == PS_STORY_RUNNING);
}

static void test_itinerary_selection(void)
{
    static const struct ps_story_action first[] = {
        { PS_STORY_ACTION_WAIT, 1, 0, 0, 0 },
        { PS_STORY_ACTION_END, 0, 0, 0, 0 }
    };
    static const struct ps_story_action second[] = {
        { PS_STORY_ACTION_FACE, 2, 0, 0, 0 },
        { PS_STORY_ACTION_END, 0, 0, 0, 0 }
    };
    static const struct ps_story_script scripts[] = {
        { first, 2 }, { second, 2 }, { first, 2 },
        { second, 2 }, { first, 2 }
    };
    struct ps_story_director director;
    int selection = ps_story_itinerary_select(117, 4, 5);
    uint32_t seen = 0;
    uint32_t loop;

    assert(selection >= 0 && selection < 5);
    assert(ps_story_itinerary_select(117, 4, 5) == selection);
    assert(ps_story_itinerary_select(117, 4, 0) == -1);
    for (loop = 0; loop < 256; ++loop)
        seen |= (uint32_t)1 << ps_story_itinerary_select(117, loop, 5);
    assert(seen == 0x1f);
    assert(ps_story_script_valid(&scripts[0]));
    assert(ps_story_init_itinerary(&director, scripts, 5, 3, 0));
    assert(director.actions == second && director.action_count == 2);
    assert(ps_story_init_itinerary(&director, scripts, 5, 4, 1));
    assert(director.actions == first && director.looping == 1);
    assert(!ps_story_init_itinerary(&director, scripts, 5, 5, 0));
}

int main(void)
{
    test_order_and_wait();
    test_pending_action_blocks_next();
    test_failure_stops_progress();
    test_loop_reset();
    test_itinerary_selection();
    puts("PocketStep story tests passed");
    return 0;
}
