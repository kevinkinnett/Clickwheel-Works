#!/bin/sh
set -eu

build_dir=/rockbox/src/build-sim-ipodcolor
shot_dir=/project/artifacts/screenshots/mushroomclock
lcd_dir="$shot_dir/lcd"
display=:99

mkdir -p "$shot_dir" "$lcd_dir"
rm -f "$shot_dir"/*.png
rm -f "$lcd_dir"/*.png

Xvfb "$display" -screen 0 900x700x24 -nolisten tcp >/tmp/mushroomclock-xvfb.log 2>&1 &
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
./rockboxui --root simdisk >/tmp/mushroomclock-simulator.log 2>&1 &
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
    cat /tmp/mushroomclock-simulator.log >&2 || true
    exit 1
fi

xdotool windowfocus --sync "$window_id"
sleep 2
import -window "$window_id" "$shot_dir/01-auto-run.png"

sleep 3
import -window "$window_id" "$shot_dir/02-auto-block.png"

sleep 3
import -window "$window_id" "$shot_dir/03-auto-power.png"

sleep 5
import -window "$window_id" "$shot_dir/04-auto-flag.png"

xdotool key --window "$window_id" Down
xdotool key --window "$window_id" Down
sleep 1
import -window "$window_id" "$shot_dir/05-night.png"

xdotool key --window "$window_id" Down
sleep 13
import -window "$window_id" "$shot_dir/06-underground.png"

xdotool key --window "$window_id" Down
sleep 12
import -window "$window_id" "$shot_dir/07-sunset.png"

xdotool key --window "$window_id" Down
xdotool key --window "$window_id" space
sleep 2
import -window "$window_id" "$shot_dir/08-auto-12h.png"

for screenshot in "$shot_dir"/[0-9][0-9]-*.png; do
    name=$(basename "$screenshot")
    convert "$screenshot" -crop 220x176+20+15 +repage "$lcd_dir/$name"
done

convert \
    "$lcd_dir/01-auto-run.png" \
    "$lcd_dir/02-auto-block.png" \
    "$lcd_dir/03-auto-power.png" \
    "$lcd_dir/04-auto-flag.png" \
    +append /tmp/mushroomclock-row-1.png
convert \
    "$lcd_dir/05-night.png" \
    "$lcd_dir/06-underground.png" \
    "$lcd_dir/07-sunset.png" \
    "$lcd_dir/08-auto-12h.png" \
    +append /tmp/mushroomclock-row-2.png
convert /tmp/mushroomclock-row-1.png /tmp/mushroomclock-row-2.png \
    -append "$shot_dir/mushroomclock-sheet.png"

identify "$shot_dir"/*.png
identify "$lcd_dir"/*.png
