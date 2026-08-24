#!/bin/sh
set -eu

rockbox_src=/rockbox/src
build_dir="$rockbox_src/build-device-ipodcolor"
output_dir=/project/artifacts/device
official_url=https://download.rockbox.org/release/4.0/rockbox-ipodcolor-4.0.zip

cp /project/src/chronolith.c "$rockbox_src/apps/plugins/chronolith.c"
sed -i 's/\r$//' "$rockbox_src/apps/plugins/chronolith.c"
cp /project/src/mushroomclock.c "$rockbox_src/apps/plugins/mushroomclock.c"
sed -i 's/\r$//' "$rockbox_src/apps/plugins/mushroomclock.c"
cp /project/src/microgame.h "$rockbox_src/apps/plugins/microgame.h"
sed -i 's/\r$//' "$rockbox_src/apps/plugins/microgame.h"

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

plugin_path=$(find "$build_dir/apps/plugins" -maxdepth 1 -name 'chronolith.rock' -print -quit)
mushroom_path=$(find "$build_dir/apps/plugins" -maxdepth 1 -name 'mushroomclock.rock' -print -quit)
if [ -z "$plugin_path" ] || [ -z "$mushroom_path" ]; then
    echo 'One or more device-format clock plugins were not produced.' >&2
    exit 1
fi

cp "$plugin_path" "$output_dir/chronolith-ipodcolor-rockbox-4.0.rock"
cp "$mushroom_path" "$output_dir/mushroomclock-ipodcolor-rockbox-4.0.rock"
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
cp "$plugin_path" "$overlay_dir/.rockbox/rocks/apps/chronolith.rock"
cp "$mushroom_path" "$overlay_dir/.rockbox/rocks/apps/mushroomclock.rock"
cd "$overlay_dir"
zip -q -u "$combined_zip" .rockbox/rocks/apps/chronolith.rock
zip -q -u "$clocks_zip" \
    .rockbox/rocks/apps/chronolith.rock \
    .rockbox/rocks/apps/mushroomclock.rock

if [ -f "$build_dir/rockbox.ipod" ]; then
    cp "$build_dir/rockbox.ipod" "$output_dir/rockbox-chronolith.ipod"
fi

cd "$output_dir"
sha256sum \
    chronolith-ipodcolor-rockbox-4.0.rock \
    mushroomclock-ipodcolor-rockbox-4.0.rock \
    rockbox-chronolith.ipod \
    rockbox-ipodcolor-4.0-official-plus-clocks.zip \
    rockbox-ipodcolor-4.0-official-plus-chronolith.zip \
    rockbox-ipodcolor-custom-clocks-4.0.zip \
    rockbox-ipodcolor-chronolith-4.0.zip \
    > SHA256SUMS
ls -lh "$output_dir"
