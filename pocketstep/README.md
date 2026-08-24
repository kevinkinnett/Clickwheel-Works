# PocketStep

PocketStep is a header-only C99 motion and collision library for tiny games. It
started inside Mushroom Clock after side impacts were repeatedly mistaken for
stomps. The library now owns that decision instead of leaving it to animation
code.

It has no allocator, floating point, file I/O, window system, or device API.
The default world stores 16 solid rectangles. Positions and velocities use
four fractional bits, so one pixel equals `PS_ONE`.

## What it handles

- Fixed-point velocity and gravity.
- Separate horizontal and vertical collision resolution.
- Stable landing on solid rectangles.
- Collider IDs for question blocks, pipes, ground, and application events.
- Collider removal and updates by ID for breakable or moving geometry.
- Body resizing that keeps an actor's feet planted.
- Actor overlap tests.
- Strict top crossing for stomps and similar interactions.

PocketStep does not draw sprites, read controls, load maps, or decide what an
enemy should do. The application owns those choices. This keeps the same
physics usable in Rockbox, a command-line test, or another embedded target.

PocketStep currently powers Mushroom Clock and the autonomous Story Clock in
[Clickwheel Works](https://github.com/kevinkinnett/Clickwheel-Works).

## Grid navigation

`pocketstep_grid.h` is an optional module for fixed-screen top-down games. It
stores no map data itself. The application supplies a row-major byte array in
which zero is passable and any other value is blocked. By default, a grid can
contain up to 143 cells. Define `PS_GRID_MAX_CELLS` before including the header
to choose another limit.

Define its implementation once after the core implementation.

```c
#define POCKETSTEP_IMPLEMENTATION
#include "pocketstep.h"
#define POCKETSTEP_GRID_IMPLEMENTATION
#include "pocketstep_grid.h"
```

Find a four-direction route with caller-owned storage.

```c
static const unsigned char blocked[15] = {
    0, 0, 0, 0, 0,
    0, 1, 1, 1, 0,
    0, 0, 0, 0, 0
};
struct ps_grid grid = { 5, 3, blocked };
struct ps_grid_workspace workspace;
struct ps_grid_cell steps[15];
struct ps_grid_path path = { steps, 15, 0 };

if (ps_grid_find_path(&grid, 0, 0, 4, 2,
                      &workspace, &path) == PS_PATH_FOUND)
    follow_steps(path.cells, path.count);
```

The returned path excludes the starting cell and includes the destination.
The search checks neighbors in up, right, down, left order, so equal shortest
routes resolve the same way on every run. `PS_PATH_NO_ROUTE` means the
destination cannot be reached. `PS_PATH_CAPACITY` means a route exists but does
not fit in the supplied path buffer; PocketStep returns no partial route.

Interaction regions are non-solid rectangles with numeric IDs. A facing query
shifts a body-sized probe in one of the four grid directions.

```c
struct ps_region chest = { { 32, 16, 16, 16 }, CHEST_ID };
int id = ps_region_find_facing(&chest, 1, &actor, PS_GRID_UP, 4);
```

## Autonomous stories

`pocketstep_story.h` is an optional completion-driven action sequencer. The
director keeps an index, wait counter, and state. The application supplies a
static action table and a callback that performs world-specific work.

```c
#define POCKETSTEP_STORY_IMPLEMENTATION
#include "pocketstep_story.h"

static const struct ps_story_action morning[] = {
    { PS_STORY_ACTION_WALK, 5, 3, 0, 0 },
    { PS_STORY_ACTION_FACE, PS_GRID_UP, 0, 0, 0 },
    { PS_STORY_ACTION_SAY, 0, 0, 0, "I should take this." },
    { PS_STORY_ACTION_COLLECT, PARCEL_ID, 0, 0, 0 },
    { PS_STORY_ACTION_END, 0, 0, 0, 0 }
};
```

Initialize a looping director and update it once per fixed game update.

```c
struct ps_story_director director;

ps_story_init(&director, morning, ARRAYLEN(morning), 1);
ps_story_update(&director, handle_action, reset_world, &game);
```

The callback returns `PS_STORY_ACTION_PENDING`, `PS_STORY_ACTION_DONE`, or
`PS_STORY_ACTION_FAILED`. A pending action remains current, so a walk can wait
for the actor to reach its destination. A failed action stops the director at
that index. Wait actions are handled internally in update counts. End actions
either set `PS_STORY_COMPLETE` or call the reset callback and return a looping
director to its opening action.

Story Clock combines these two optional modules on a 13 by 11 grid. A walk
action requests a deterministic BFS route only once, then remains pending while
the actor moves between cell centers. Dialogue and collection cannot start
early because the director advances only after the walk callback reports
completion. If a destination has no route, the callback fails and the
application keeps a visible diagnostic on screen instead of crossing a wall.

## Build and test

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Minimal use

Define the implementation once.

```c
#define POCKETSTEP_IMPLEMENTATION
#include "pocketstep.h"
```

Create a world and add whole-pixel solids.

```c
struct ps_world world;
struct ps_body player = {
    8 * PS_ONE, 32 * PS_ONE,
    PS_ONE, 0,
    12, 16
};

ps_world_clear(&world);
ps_world_add_solid(&world, 1, 0, 100, 220, 32);
```

Advance one fixed update.

```c
struct ps_move_result movement;

ps_apply_gravity(&player, 4, 72);
movement = ps_move(&world, &player);
if (movement.hit_floor)
    player.vy = 0;
```

Collider IDs let the application turn physics contacts into game events.

```c
if (movement.hit_ceiling && movement.vertical_id == QUESTION_BLOCK_ID)
    release_coin();
```

IDs also let the application change the world without rebuilding it.
Use a unique ID for each solid. Removal and updates affect the first matching
solid.

```c
ps_world_remove_solid(&world, BROKEN_BRICK_ID);
ps_world_update_solid(&world, MOVING_PLATFORM_ID, x, y, width, height);
```

Resize a body from its bottom edge when a power-up changes its height. The left
edge and bottom edge stay fixed.

```c
ps_body_resize_from_bottom(&player, player.width, 24);
```

For an enemy contact, test overlap and then classify the approach.

```c
if (ps_crossed_top(&previous_player, &player, &enemy, 2))
    stomp_enemy();
else if (ps_overlap(&player, &enemy))
    hurt_player();
```

The two-pixel argument is a tolerance around the enemy's top edge. It does not
turn a horizontal impact into a stomp because the player's previous feet must
have been above that edge. Do not place `ps_crossed_top` inside an overlap test.
The body can land exactly on the enemy's top edge without a strict overlap at
the end of the update.

## Limits

`PS_MAX_SOLIDS` defaults to 16 and can be defined before including the header.
The collision solver assumes each update moves a body across no more than one
relevant solid on each axis. That matches a fixed-rate platform game on a
220x176 display. Fast projectiles should use smaller steps or a separate ray
test.

PocketStep uses the MIT License.
