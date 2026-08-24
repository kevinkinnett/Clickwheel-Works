/*
 * PocketStep Story
 * Completion-driven action sequences for small autonomous C games.
 *
 * Copyright (C) 2026 Kevin Kinnett
 * SPDX-License-Identifier: MIT
 *
 * Define POCKETSTEP_STORY_IMPLEMENTATION in one translation unit before
 * including this file.
 */

#ifndef POCKETSTEP_STORY_H
#define POCKETSTEP_STORY_H

#define PS_STORY_ACTION_WALK 1
#define PS_STORY_ACTION_FACE 2
#define PS_STORY_ACTION_WAIT 3
#define PS_STORY_ACTION_SAY 4
#define PS_STORY_ACTION_COLLECT 5
#define PS_STORY_ACTION_SCENE 6
#define PS_STORY_ACTION_END 7

#define PS_STORY_ACTION_FAILED -1
#define PS_STORY_ACTION_PENDING 0
#define PS_STORY_ACTION_DONE 1

#define PS_STORY_FAILED -1
#define PS_STORY_RUNNING 0
#define PS_STORY_COMPLETE 1

struct ps_story_action
{
    int kind;
    int a;
    int b;
    int c;
    const char *text;
};

struct ps_story_director
{
    const struct ps_story_action *actions;
    int action_count;
    int action_index;
    int wait_remaining;
    int action_started;
    int looping;
    int state;
};

typedef int (*ps_story_action_handler)(const struct ps_story_action *action,
                                       void *context);
typedef void (*ps_story_reset_handler)(void *context);

int ps_story_init(struct ps_story_director *director,
                  const struct ps_story_action *actions,
                  int action_count, int looping);
void ps_story_reset(struct ps_story_director *director);
int ps_story_update(struct ps_story_director *director,
                    ps_story_action_handler action_handler,
                    ps_story_reset_handler reset_handler,
                    void *context);
const struct ps_story_action *ps_story_current(
    const struct ps_story_director *director);

#endif

#ifdef POCKETSTEP_STORY_IMPLEMENTATION
#ifndef POCKETSTEP_STORY_IMPLEMENTATION_ONCE
#define POCKETSTEP_STORY_IMPLEMENTATION_ONCE

int ps_story_init(struct ps_story_director *director,
                  const struct ps_story_action *actions,
                  int action_count, int looping)
{
    if (director == 0 || actions == 0 || action_count <= 0)
        return 0;
    director->actions = actions;
    director->action_count = action_count;
    director->looping = looping != 0;
    ps_story_reset(director);
    return 1;
}

void ps_story_reset(struct ps_story_director *director)
{
    if (director == 0)
        return;
    director->action_index = 0;
    director->wait_remaining = 0;
    director->action_started = 0;
    director->state = PS_STORY_RUNNING;
}

const struct ps_story_action *ps_story_current(
    const struct ps_story_director *director)
{
    if (director == 0 || director->actions == 0 ||
        director->action_index < 0 ||
        director->action_index >= director->action_count)
        return 0;
    return &director->actions[director->action_index];
}

static void ps_story_advance(struct ps_story_director *director)
{
    director->action_index++;
    director->wait_remaining = 0;
    director->action_started = 0;
}

int ps_story_update(struct ps_story_director *director,
                    ps_story_action_handler action_handler,
                    ps_story_reset_handler reset_handler,
                    void *context)
{
    const struct ps_story_action *action;
    int result;

    if (director == 0 || director->state != PS_STORY_RUNNING)
        return director == 0 ? PS_STORY_FAILED : director->state;
    action = ps_story_current(director);
    if (action == 0)
    {
        director->state = PS_STORY_FAILED;
        return director->state;
    }

    if (action->kind == PS_STORY_ACTION_WAIT)
    {
        if (!director->action_started)
        {
            director->wait_remaining = action->a > 0 ? action->a : 0;
            director->action_started = 1;
        }
        if (director->wait_remaining > 0)
            director->wait_remaining--;
        if (director->wait_remaining == 0)
            ps_story_advance(director);
        return director->state;
    }

    if (action->kind == PS_STORY_ACTION_END)
    {
        if (director->looping)
        {
            if (reset_handler != 0)
                reset_handler(context);
            ps_story_reset(director);
        }
        else
            director->state = PS_STORY_COMPLETE;
        return director->state;
    }

    if (action_handler == 0)
    {
        director->state = PS_STORY_FAILED;
        return director->state;
    }
    director->action_started = 1;
    result = action_handler(action, context);
    if (result == PS_STORY_ACTION_DONE)
        ps_story_advance(director);
    else if (result == PS_STORY_ACTION_FAILED)
        director->state = PS_STORY_FAILED;
    else if (result != PS_STORY_ACTION_PENDING)
        director->state = PS_STORY_FAILED;
    return director->state;
}

#endif
#endif
