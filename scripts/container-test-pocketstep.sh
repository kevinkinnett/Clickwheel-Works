#!/bin/sh
set -eu

mkdir -p /tmp/pocketstep-tests
cc -std=c99 -Wall -Wextra -Werror \
    /project/pocketstep/tests/test_pocketstep.c \
    -o /tmp/pocketstep-tests/test_pocketstep
cc -std=c99 -Wall -Wextra -Werror \
    /project/pocketstep/tests/test_pocketstep_grid.c \
    -o /tmp/pocketstep-tests/test_pocketstep_grid
cc -std=c99 -Wall -Wextra -Werror \
    /project/pocketstep/tests/test_pocketstep_story.c \
    -o /tmp/pocketstep-tests/test_pocketstep_story
cc -std=c99 -Wall -Wextra -Werror \
    /project/pocketstep/tests/test_storyclock_world.c \
    -o /tmp/pocketstep-tests/test_storyclock_world
cc -std=c99 -Wall -Wextra -Werror \
    /project/pocketstep/tests/test_pocketstep_draw.c \
    -o /tmp/pocketstep-tests/test_pocketstep_draw
cc -std=c99 -Wall -Wextra -Werror \
    /project/pocketstep/tests/test_pocketstep_inventory.c \
    -o /tmp/pocketstep-tests/test_pocketstep_inventory
cc -std=c99 -Wall -Wextra -Werror \
    /project/pocketstep/tests/test_pocketstep_text.c \
    -o /tmp/pocketstep-tests/test_pocketstep_text
cc -std=c99 -Wall -Wextra -Werror \
    /project/pocketstep/tests/test_pocketstep_anim.c \
    -o /tmp/pocketstep-tests/test_pocketstep_anim
cc -std=c99 -Wall -Wextra -Werror \
    /project/pocketstep/tests/test_pocketstep_scene.c \
    -o /tmp/pocketstep-tests/test_pocketstep_scene
/tmp/pocketstep-tests/test_pocketstep
/tmp/pocketstep-tests/test_pocketstep_grid
/tmp/pocketstep-tests/test_pocketstep_story
/tmp/pocketstep-tests/test_storyclock_world
/tmp/pocketstep-tests/test_pocketstep_draw
/tmp/pocketstep-tests/test_pocketstep_inventory
/tmp/pocketstep-tests/test_pocketstep_text
/tmp/pocketstep-tests/test_pocketstep_anim
/tmp/pocketstep-tests/test_pocketstep_scene
python3 /project/pocketstep/tests/test_asset_compiler.py
