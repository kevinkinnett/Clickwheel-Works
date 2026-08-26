#!/bin/sh
set -eu

build_dir=/rockbox/src/build-sim-ipodcolor
output_dir=/project/artifacts/screenshots/storyclock/village
scenario_file="$build_dir/simdisk/storyclock-scenario.txt"
display=:99

mkdir -p "$output_dir"
rm -f "$output_dir"/*.png

Xvfb "$display" -screen 0 900x700x24 -nolisten tcp \
    >/tmp/storyclock-village-xvfb.log 2>&1 &
xvfb_pid=$!
cleanup()
{
    rm -f "$scenario_file"
    if [ -n "${sim_pid:-}" ]; then
        kill "$sim_pid" 2>/dev/null || true
    fi
    kill "$xvfb_pid" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

export DISPLAY="$display"
export SDL_AUDIODRIVER=dummy
export SDL_VIDEODRIVER=x11
export XDG_RUNTIME_DIR=/tmp
export LIBGL_ALWAYS_SOFTWARE=1
export LP_NUM_THREADS=1
sleep 1

capture_scene()
{
    code=$1
    palette=$2
    scene=$3
    printf '%s\n' "$code" > "$scenario_file"
    cd "$build_dir"
    ./rockboxui --root simdisk >/tmp/storyclock-village-simulator.log 2>&1 &
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
        cat /tmp/storyclock-village-simulator.log >&2 || true
        exit 1
    fi

    xdotool windowmove --sync "$window_id" 20 15
    xdotool windowfocus --sync "$window_id"
    eval "$(xdotool getwindowgeometry --shell "$window_id")"
    sleep 3
    ffmpeg -hide_banner -loglevel error -y \
        -f x11grab -draw_mouse 0 -video_size "${WIDTH}x${HEIGHT}" \
        -i "${DISPLAY}+${X},${Y}" -frames:v 1 \
        -vf 'crop=220:176:20:15' \
        "$output_dir/$palette-$scene.png"
    kill "$sim_pid" 2>/dev/null || true
    wait "$sim_pid" 2>/dev/null || true
    sim_pid=''
}

scenes='cottage green mill market gate fields garden'
palette_index=0
for palette in day evening night; do
    scene_index=0
    for scene in $scenes; do
        capture_scene $((40 + palette_index * 7 + scene_index)) "$palette" "$scene"
        scene_index=$((scene_index + 1))
    done
    ffmpeg -hide_banner -loglevel error -y \
        -i "$output_dir/$palette-cottage.png" \
        -i "$output_dir/$palette-green.png" \
        -i "$output_dir/$palette-mill.png" \
        -i "$output_dir/$palette-market.png" \
        -i "$output_dir/$palette-gate.png" \
        -i "$output_dir/$palette-fields.png" \
        -i "$output_dir/$palette-garden.png" \
        -filter_complex 'hstack=inputs=7' -frames:v 1 \
        "$output_dir/$palette-village-sheet.png"
    palette_index=$((palette_index + 1))
done

identify "$output_dir"/*-village-sheet.png
