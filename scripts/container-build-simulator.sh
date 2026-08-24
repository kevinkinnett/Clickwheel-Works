#!/bin/sh
set -eu

rockbox_src=/rockbox/src
build_dir="$rockbox_src/build-sim-ipodcolor"

plugin_name=${CLOCK_PLUGIN:-chronolith}

case "$plugin_name" in
    chronolith|mushroomclock) ;;
    *) echo "Unknown clock plugin: $plugin_name" >&2; exit 2 ;;
esac

for source_name in chronolith mushroomclock; do
    cp "/project/src/$source_name.c" "$rockbox_src/apps/plugins/$source_name.c"
    sed -i 's/\r$//' "$rockbox_src/apps/plugins/$source_name.c"
done
cp /project/pocketstep/pocketstep.h "$rockbox_src/apps/plugins/pocketstep.h"
sed -i 's/\r$//' "$rockbox_src/apps/plugins/pocketstep.h"

if ! grep -qxF 'chronolith.c' "$rockbox_src/apps/plugins/SOURCES"; then
    sed -i '/^chessclock\.c$/a chronolith.c' "$rockbox_src/apps/plugins/SOURCES"
fi

if ! grep -qxF 'chronolith,apps' "$rockbox_src/apps/plugins/CATEGORIES"; then
    sed -i '/^clock,apps$/a chronolith,apps' "$rockbox_src/apps/plugins/CATEGORIES"
fi

if ! grep -qxF 'mushroomclock.c' "$rockbox_src/apps/plugins/SOURCES"; then
    sed -i '/^chronolith\.c$/a mushroomclock.c' "$rockbox_src/apps/plugins/SOURCES"
fi

if ! grep -qxF 'mushroomclock,apps' "$rockbox_src/apps/plugins/CATEGORIES"; then
    sed -i '/^chronolith,apps$/a mushroomclock,apps' "$rockbox_src/apps/plugins/CATEGORIES"
fi

# Simulator-only convenience: launch Chronolith immediately after Rockbox boots.
sed -i 's@/\*#define AUTOROCK\*/@#define AUTOROCK@' "$rockbox_src/apps/main.c"

mkdir -p "$build_dir"
cd "$build_dir"

if [ ! -f Makefile ]; then
    ../tools/configure --target=ipodcolor --type=S --no-ccache --sdl-threads
fi

make -j"$(nproc)"
make fullinstall

plugin_path=$(find "$build_dir/apps/plugins" -maxdepth 1 -name "$plugin_name.rock" -print -quit)
if [ -z "$plugin_path" ]; then
    echo "$plugin_name plugin was not produced." >&2
    exit 1
fi

cp "$plugin_path" "$build_dir/simdisk/.rockbox/rocks/apps/autostart.rock"
mkdir -p /project/artifacts/simulator
cp "$build_dir/apps/plugins/chronolith.rock" /project/artifacts/simulator/chronolith-ipodcolor-sim.rock
cp "$build_dir/apps/plugins/mushroomclock.rock" /project/artifacts/simulator/mushroomclock-ipodcolor-sim.rock

echo "Simulator ready: $build_dir/rockboxui"
echo "Plugin ready: $plugin_path"
