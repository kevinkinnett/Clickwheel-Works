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
/tmp/pocketstep-tests/test_pocketstep
/tmp/pocketstep-tests/test_pocketstep_grid
/tmp/pocketstep-tests/test_pocketstep_story
/tmp/pocketstep-tests/test_storyclock_world
