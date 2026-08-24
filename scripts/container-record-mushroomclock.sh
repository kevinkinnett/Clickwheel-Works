#!/bin/sh
set -eu

build_dir=/rockbox/src/build-sim-ipodcolor
video_dir=/project/artifacts/video
world=${CLOCK_WORLD:-auto}
scenario=${CLOCK_SCENARIO:-random}
display=:99
capture_duration=16
output_duration=13
scenario_number=''

case "$world" in
    auto)
        key_count=0
        trim_start=1.4
        video_name=mushroomclock-complete-run.mp4
        video_title='Mushroom Clock complete run'
        ;;
    night)
        key_count=2
        trim_start=1.8
        video_name=mushroomclock-night-run.mp4
        video_title='Mushroom Clock night run'
        ;;
    underground)
        key_count=3
        trim_start=14.8
        capture_duration=29
        video_name=mushroomclock-underground-run.mp4
        video_title='Mushroom Clock underground run'
        ;;
    sunset)
        key_count=4
        trim_start=1.8
        video_name=mushroomclock-evening-run.mp4
        video_title='Mushroom Clock evening run'
        ;;
    variants)
        key_count=0
        trim_start=1.4
        capture_duration=55
        output_duration=52
        video_name=mushroomclock-surface-variants.mp4
        video_title='Mushroom Clock surface variants'
        ;;
    *)
        echo "Unknown Mushroom Clock world: $world" >&2
        exit 2
        ;;
esac

case "$scenario" in
    random) ;;
    mushroom) scenario_number=0; scenario_title='Mushroom growth' ;;
    empty) scenario_number=1; scenario_title='Empty block' ;;
    star) scenario_number=2; scenario_title='Star attack' ;;
    high) scenario_number=3; scenario_title='High route' ;;
    retreat) scenario_number=4; scenario_title='Retreat route' ;;
    stomp) scenario_number=5; scenario_title='Clean stomp' ;;
    sidehit) scenario_number=6; scenario_title='Side hit' ;;
    poison) scenario_number=7; scenario_title='Poison run' ;;
    *) echo "Unknown Mushroom Clock scenario: $scenario" >&2; exit 2 ;;
esac

if [ -n "$scenario_number" ]; then
    key_count=0
    trim_start=1.4
    capture_duration=18
    output_duration=15
    video_name="mushroomclock-scenario-$scenario.mp4"
    video_title="Mushroom Clock: $scenario_title"
fi

lcd_video="$video_dir/$video_name"
raw_video="$video_dir/.mushroomclock-$scenario-$world-capture.mp4"
scenario_file="$build_dir/simdisk/mushroomclock-scenario.txt"

mkdir -p "$video_dir"
rm -f "$lcd_video" "$raw_video"
if [ -n "$scenario_number" ]; then
    printf '%s\n' "$scenario_number" > "$scenario_file"
else
    rm -f "$scenario_file"
fi

Xvfb "$display" -screen 0 900x700x24 -nolisten tcp \
    >/tmp/mushroomclock-video-xvfb.log 2>&1 &
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
./rockboxui --root simdisk >/tmp/mushroomclock-video-simulator.log 2>&1 &
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
    cat /tmp/mushroomclock-video-simulator.log >&2 || true
    exit 1
fi

xdotool windowmove --sync "$window_id" 20 15
xdotool windowfocus --sync "$window_id"
eval "$(xdotool getwindowgeometry --shell "$window_id")"

if [ "$key_count" -gt 0 ]; then
    (
        sleep 1.45
        key_index=0
        while [ "$key_index" -lt "$key_count" ]; do
            xdotool key --window "$window_id" Down
            key_index=$((key_index + 1))
            sleep 0.01
        done
    ) &
    mode_pid=$!
fi

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
    -movflags +faststart -metadata title="$video_title" \
    "$lcd_video"
rm -f "$raw_video"

ffprobe -v error -select_streams v:0 \
    -show_entries stream=codec_name,width,height,r_frame_rate,duration \
    -show_entries format=filename,size,duration \
    -of default=noprint_wrappers=1 "$lcd_video"
