#!/bin/sh
set -eu

rockbox_src=/rockbox/src
build_dir="$rockbox_src/build-device-ipodcolor"
output_dir=/project/artifacts/device
official_url=https://download.rockbox.org/release/4.0/rockbox-ipodcolor-4.0.zip
supported_plugins='chronolith mushroomclock storyclock'

for source_name in $supported_plugins; do
    cp "/project/src/$source_name.c" "$rockbox_src/apps/plugins/$source_name.c"
    sed -i 's/\r$//' "$rockbox_src/apps/plugins/$source_name.c"
done

for header_name in pocketstep pocketstep_grid pocketstep_story \
                   pocketstep_draw pocketstep_anim pocketstep_scene; do
    cp "/project/pocketstep/$header_name.h" "$rockbox_src/apps/plugins/$header_name.h"
    sed -i 's/\r$//' "$rockbox_src/apps/plugins/$header_name.h"
done
cp /project/src/storyclock_assets.h "$rockbox_src/apps/plugins/storyclock_assets.h"
sed -i 's/\r$//' "$rockbox_src/apps/plugins/storyclock_assets.h"

for source_name in $supported_plugins; do
    if ! grep -qxF "$source_name.c" "$rockbox_src/apps/plugins/SOURCES"; then
        sed -i "/^chessclock\\.c$/a $source_name.c" "$rockbox_src/apps/plugins/SOURCES"
    fi

    if ! grep -qxF "$source_name,apps" "$rockbox_src/apps/plugins/CATEGORIES"; then
        sed -i "/^clock,apps$/a $source_name,apps" "$rockbox_src/apps/plugins/CATEGORIES"
    fi
done

# AUTOROCK is useful for simulator captures only, never for the device firmware.
sed -i 's@^#define AUTOROCK@/*#define AUTOROCK*/@' "$rockbox_src/apps/main.c"

mkdir -p "$build_dir" "$output_dir"
cd "$build_dir"

if [ ! -f Makefile ]; then
    ../tools/configure --target=ipodcolor --type=N --no-ccache
fi

# Rockbox 4.0's legacy game ports expect GCC's pre-10 common-symbol behavior.
if ! grep -q 'GCCOPTS=-fcommon ' Makefile; then
    sed -i 's/^export GCCOPTS=/export GCCOPTS=-fcommon /' Makefile
fi

make -j"$(nproc)"
make zip

for source_name in $supported_plugins; do
    source_path="$build_dir/apps/plugins/$source_name.rock"
    if [ ! -f "$source_path" ]; then
        echo "Device-format plugin was not produced: $source_name" >&2
        exit 1
    fi
    cp "$source_path" "$output_dir/$source_name-ipodcolor-rockbox-4.0.rock"
done

plugin_path="$build_dir/apps/plugins/chronolith.rock"
cp "$build_dir/rockbox.zip" "$output_dir/rockbox-ipodcolor-chronolith-4.0.zip"
cp "$build_dir/rockbox.zip" "$output_dir/rockbox-ipodcolor-custom-clocks-4.0.zip"

official_zip=/tmp/rockbox-ipodcolor-4.0-official.zip
combined_zip="$output_dir/rockbox-ipodcolor-4.0-official-plus-chronolith.zip"
clocks_zip="$output_dir/rockbox-ipodcolor-4.0-official-plus-clocks.zip"
overlay_dir=$(mktemp -d)
wget -q "$official_url" -O "$official_zip"
cp "$official_zip" "$combined_zip"
cp "$official_zip" "$clocks_zip"
mkdir -p "$overlay_dir/.rockbox/rocks/apps"
for source_name in $supported_plugins; do
    cp "$build_dir/apps/plugins/$source_name.rock" \
        "$overlay_dir/.rockbox/rocks/apps/$source_name.rock"
done
cd "$overlay_dir"
zip -q -u "$combined_zip" .rockbox/rocks/apps/chronolith.rock
zip -q -u "$clocks_zip" \
    .rockbox/rocks/apps/chronolith.rock \
    .rockbox/rocks/apps/mushroomclock.rock \
    .rockbox/rocks/apps/storyclock.rock

if [ -f "$build_dir/rockbox.ipod" ]; then
    cp "$build_dir/rockbox.ipod" "$output_dir/rockbox-chronolith.ipod"
fi

cd "$output_dir"
sha256sum \
    chronolith-ipodcolor-rockbox-4.0.rock \
    mushroomclock-ipodcolor-rockbox-4.0.rock \
    storyclock-ipodcolor-rockbox-4.0.rock \
    rockbox-chronolith.ipod \
    rockbox-ipodcolor-4.0-official-plus-clocks.zip \
    rockbox-ipodcolor-4.0-official-plus-chronolith.zip \
    rockbox-ipodcolor-custom-clocks-4.0.zip \
    rockbox-ipodcolor-chronolith-4.0.zip \
    > SHA256SUMS
ls -lh "$output_dir"
