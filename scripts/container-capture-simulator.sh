#!/bin/sh
set -eu

build_dir=/rockbox/src/build-sim-ipodcolor
shot_dir=/project/artifacts/screenshots
lcd_dir="$shot_dir/lcd"
display=:99

mkdir -p "$shot_dir" "$lcd_dir"
rm -f "$shot_dir"/*.png
rm -f "$lcd_dir"/*.png

Xvfb "$display" -screen 0 900x700x24 -nolisten tcp >/tmp/chronolith-xvfb.log 2>&1 &
xvfb_pid=$!
trap 'kill "$sim_pid" "$xvfb_pid" 2>/dev/null || true' EXIT INT TERM
export DISPLAY="$display"
export SDL_AUDIODRIVER=dummy
export SDL_VIDEODRIVER=x11
export XDG_RUNTIME_DIR=/tmp
export LIBGL_ALWAYS_SOFTWARE=1
export LP_NUM_THREADS=1
sleep 1

cd "$build_dir"
./rockboxui --root simdisk >/tmp/chronolith-simulator.log 2>&1 &
sim_pid=$!

window_id=''
attempt=0
while [ "$attempt" -lt 50 ]; do
    window_id=$(xdotool search --all --pid "$sim_pid" 2>/dev/null | head -n 1 || true)
    [ -n "$window_id" ] && break
    sleep 0.1
    attempt=$((attempt + 1))
done

if [ -z "$window_id" ]; then
    echo 'Rockbox simulator window did not appear.' >&2
    cat /tmp/chronolith-simulator.log >&2 || true
    exit 1
fi

xdotool windowfocus --sync "$window_id"
sleep 2
import -window "$window_id" "$shot_dir/01-phosphor.png"

sleep 2
import -window "$window_id" "$shot_dir/02-phosphor-motion.png"

xdotool key --window "$window_id" Down
sleep 1
import -window "$window_id" "$shot_dir/03-amber.png"

xdotool key --window "$window_id" Down
sleep 1
import -window "$window_id" "$shot_dir/04-emergency.png"

xdotool key --window "$window_id" Down
sleep 1
xdotool key --window "$window_id" space
sleep 1
import -window "$window_id" "$shot_dir/05-phosphor-12h.png"

sleep 2
import -window "$window_id" "$shot_dir/06-phosphor-motion-2.png"

for screenshot in "$shot_dir"/[0-9][0-9]-*.png; do
    name=$(basename "$screenshot")
    convert "$screenshot" -crop 220x176+20+15 +repage "$lcd_dir/$name"
done

convert \
    "$lcd_dir/01-phosphor.png" \
    "$lcd_dir/02-phosphor-motion.png" \
    "$lcd_dir/03-amber.png" \
    +append /tmp/chronolith-row-1.png
convert \
    "$lcd_dir/04-emergency.png" \
    "$lcd_dir/05-phosphor-12h.png" \
    "$lcd_dir/06-phosphor-motion-2.png" \
    +append /tmp/chronolith-row-2.png
convert /tmp/chronolith-row-1.png /tmp/chronolith-row-2.png \
    -append "$shot_dir/chronolith-terminal-sheet.png"

identify "$shot_dir"/*.png
identify "$lcd_dir"/*.png
