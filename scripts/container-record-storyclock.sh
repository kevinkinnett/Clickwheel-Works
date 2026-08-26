#!/bin/sh
set -eu

build_dir=/rockbox/src/build-sim-ipodcolor
video_dir=/project/artifacts/video
scenario=${STORYCLOCK_SCENARIO:-day}
display=:99
capture_duration=76
output_duration=73
trim_start=1.4

case "$scenario" in
    auto) scenario_number=0 ;;
    day) scenario_number=1 ;;
    evening) scenario_number=2 ;;
    night) scenario_number=3 ;;
    market) scenario_number=10 ;;
    mill) scenario_number=11 ;;
    gate) scenario_number=12 ;;
    farm) scenario_number=13 ;;
    garden) scenario_number=14 ;;
    inventory)
        scenario_number=70
        capture_duration=10
        output_duration=9
        trim_start=0.5
        ;;
    inventory-empty)
        scenario_number=71
        capture_duration=8
        output_duration=7
        trim_start=0.5
        ;;
    dialogue)
        scenario_number=75
        capture_duration=13
        output_duration=12
        trim_start=0.5
        ;;
    *) echo "Unknown Story Clock scenario: $scenario" >&2; exit 2 ;;
esac

lcd_video="$video_dir/storyclock-$scenario-complete-story.mp4"
raw_video="$video_dir/.storyclock-$scenario-capture.mp4"
scenario_file="$build_dir/simdisk/storyclock-scenario.txt"

mkdir -p "$video_dir"
rm -f "$lcd_video" "$raw_video"
printf '%s\n' "$scenario_number" > "$scenario_file"

Xvfb "$display" -screen 0 900x700x24 -nolisten tcp \
    >/tmp/storyclock-video-xvfb.log 2>&1 &
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

cd "$build_dir"
./rockboxui --root simdisk >/tmp/storyclock-video-simulator.log 2>&1 &
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
    cat /tmp/storyclock-video-simulator.log >&2 || true
    exit 1
fi

xdotool windowmove --sync "$window_id" 20 15
xdotool windowfocus --sync "$window_id"
eval "$(xdotool getwindowgeometry --shell "$window_id")"

ffmpeg -hide_banner -loglevel warning -y \
    -f x11grab -draw_mouse 0 -framerate 20 \
    -video_size "${WIDTH}x${HEIGHT}" -i "${DISPLAY}+${X},${Y}" \
    -t "$capture_duration" \
    -vf "crop=220:176:20:15,scale=880:704:flags=neighbor,format=yuv420p" \
    -an -c:v libx264 -preset slow -crf 16 \
    "$raw_video"

ffmpeg -hide_banner -loglevel warning -y \
    -ss "$trim_start" -i "$raw_video" -t "$output_duration" \
    -an -c:v libx264 -preset slow -crf 16 -pix_fmt yuv420p \
    -movflags +faststart -metadata title="Story Clock $scenario story" \
    "$lcd_video"
rm -f "$raw_video"

ffprobe -v error -select_streams v:0 \
    -show_entries stream=codec_name,width,height,r_frame_rate,duration \
    -show_entries format=filename,size,duration \
    -of default=noprint_wrappers=1 "$lcd_video"
