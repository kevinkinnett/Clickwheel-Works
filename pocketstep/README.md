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
- Actor overlap tests.
- Strict top crossing for stomps and similar interactions.

PocketStep does not draw sprites, read controls, load maps, or decide what an
enemy should do. The application owns those choices. This keeps the same
physics usable in Rockbox, a command-line test, or another embedded target.

PocketStep currently powers Mushroom Clock in
[Clickwheel Works](https://github.com/kevinkinnett/Clickwheel-Works).

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

For an enemy contact, test overlap and then classify the approach.

```c
if (ps_overlap(&player, &enemy)) {
    if (ps_crossed_top(&previous_player, &player, &enemy, 2))
        stomp_enemy();
    else
        hurt_player();
}
```

The two-pixel argument is a tolerance around the enemy's top edge. It does not
turn a horizontal impact into a stomp because the player's previous feet must
have been above that edge.

## Limits

`PS_MAX_SOLIDS` defaults to 16 and can be defined before including the header.
The collision solver assumes each update moves a body across no more than one
relevant solid on each axis. That matches a fixed-rate platform game on a
220x176 display. Fast projectiles should use smaller steps or a separate ray
test.

PocketStep uses the MIT License.
