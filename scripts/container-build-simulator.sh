#!/bin/sh
set -eu

rockbox_src=/rockbox/src
build_dir="$rockbox_src/build-sim-ipodcolor"

plugin_name=${CLOCK_PLUGIN:-chronolith}
supported_plugins='chronolith mushroomclock storyclock'

case " $supported_plugins " in
    *" $plugin_name "*) ;;
    *) echo "Unknown clock plugin: $plugin_name" >&2; exit 2 ;;
esac

for source_name in $supported_plugins; do
    cp "/project/src/$source_name.c" "$rockbox_src/apps/plugins/$source_name.c"
    sed -i 's/\r$//' "$rockbox_src/apps/plugins/$source_name.c"
done
for header_name in pocketstep pocketstep_grid pocketstep_story; do
    cp "/project/pocketstep/$header_name.h" "$rockbox_src/apps/plugins/$header_name.h"
    sed -i 's/\r$//' "$rockbox_src/apps/plugins/$header_name.h"
done

for source_name in $supported_plugins; do
    if ! grep -qxF "$source_name.c" "$rockbox_src/apps/plugins/SOURCES"; then
        sed -i "/^chessclock\\.c$/a $source_name.c" "$rockbox_src/apps/plugins/SOURCES"
    fi

    if ! grep -qxF "$source_name,apps" "$rockbox_src/apps/plugins/CATEGORIES"; then
        sed -i "/^clock,apps$/a $source_name,apps" "$rockbox_src/apps/plugins/CATEGORIES"
    fi
done

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
for source_name in $supported_plugins; do
    cp "$build_dir/apps/plugins/$source_name.rock" \
        "/project/artifacts/simulator/$source_name-ipodcolor-sim.rock"
done

echo "Simulator ready: $build_dir/rockboxui"
echo "Plugin ready: $plugin_path"
