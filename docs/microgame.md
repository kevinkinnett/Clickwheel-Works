# Microgame

Microgame is a header-only C99 motion and collision library for tiny games. It
started inside Mushroom Clock after side impacts were repeatedly mistaken for
stomps. The library now owns that decision instead of leaving it to animation
code.

It has no allocator, floating point, file I/O, window system, or device API.
The default world stores 16 solid rectangles. Positions and velocities use
four fractional bits, so one pixel equals `MG_ONE`.

## What it handles

- Fixed-point velocity and gravity.
- Separate horizontal and vertical collision resolution.
- Stable landing on solid rectangles.
- Collider IDs for question blocks, pipes, ground, and application events.
- Actor overlap tests.
- Strict top crossing for stomps and similar interactions.

Microgame does not draw sprites, read controls, load maps, or decide what an
enemy should do. The application owns those choices. This keeps the same
physics usable in Rockbox, a command-line test, or another embedded target.

## Minimal use

Define the implementation once.

```c
#define MICROGAME_IMPLEMENTATION
#include "microgame.h"
```

Create a world and add whole-pixel solids.

```c
struct mg_world world;
struct mg_body player = {
    8 * MG_ONE, 32 * MG_ONE,
    MG_ONE, 0,
    12, 16
};

mg_world_clear(&world);
mg_world_add_solid(&world, 1, 0, 100, 220, 32);
```

Advance one fixed update.

```c
struct mg_move_result movement;

mg_apply_gravity(&player, 4, 72);
movement = mg_move(&world, &player);
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
if (mg_overlap(&player, &enemy)) {
    if (mg_crossed_top(&previous_player, &player, &enemy, 2))
        stomp_enemy();
    else
        hurt_player();
}
```

The two-pixel argument is a tolerance around the enemy's top edge. It does not
turn a horizontal impact into a stomp because the player's previous feet must
have been above that edge.

## Limits

`MG_MAX_SOLIDS` defaults to 16 and can be defined before including the header.
The collision solver assumes each update moves a body across no more than one
relevant solid on each axis. That matches a fixed-rate platform game on a
220x176 display. Fast projectiles should use smaller steps or a separate ray
test.

Microgame is MIT-licensed. The Rockbox demo remains GPL-2.0-or-later.
