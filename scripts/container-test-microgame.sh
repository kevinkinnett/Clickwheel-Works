#!/bin/sh
set -eu

mkdir -p /tmp/microgame-tests
cc -std=c99 -Wall -Wextra -Werror \
    /project/tests/test_microgame.c \
    -o /tmp/microgame-tests/test_microgame
/tmp/microgame-tests/test_microgame
