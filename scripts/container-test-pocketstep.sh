#!/bin/sh
set -eu

mkdir -p /tmp/pocketstep-tests
cc -std=c99 -Wall -Wextra -Werror \
    /project/pocketstep/tests/test_pocketstep.c \
    -o /tmp/pocketstep-tests/test_pocketstep
/tmp/pocketstep-tests/test_pocketstep
