#!/bin/sh
set -eu

video_dir=/project/artifacts/video
sheet_dir=/project/artifacts/screenshots/mushroomclock/scenarios

mkdir -p "$sheet_dir"
rm -f "$sheet_dir"/scenario-summary-[0-9].png "$sheet_dir/empty-sheet.png"

for scenario in mushroom coins star high retreat stomp sidehit poison; do
    video="$video_dir/mushroomclock-scenario-$scenario.mp4"
    sheet="$sheet_dir/$scenario-sheet.png"
    representative="$sheet_dir/.$scenario-review.png"

    if [ ! -f "$video" ]; then
        echo "Missing scenario video: $video" >&2
        exit 1
    fi

    ffmpeg -hide_banner -loglevel error -y -i "$video" \
        -vf 'fps=1/2,scale=220:176:flags=neighbor,tile=4x2' \
        -frames:v 1 "$sheet"
    ffmpeg -hide_banner -loglevel error -y -ss 8 -i "$video" \
        -frames:v 1 -vf 'scale=220:176:flags=neighbor' "$representative"
done

ffmpeg -hide_banner -loglevel error -y \
    -i "$sheet_dir/.mushroom-review.png" \
    -i "$sheet_dir/.coins-review.png" \
    -i "$sheet_dir/.star-review.png" \
    -i "$sheet_dir/.high-review.png" \
    -i "$sheet_dir/.retreat-review.png" \
    -i "$sheet_dir/.stomp-review.png" \
    -i "$sheet_dir/.sidehit-review.png" \
    -i "$sheet_dir/.poison-review.png" \
    -filter_complex \
    'xstack=inputs=8:layout=0_0|220_0|440_0|660_0|0_176|220_176|440_176|660_176' \
    -frames:v 1 "$sheet_dir/scenario-summary.png"

rm -f "$sheet_dir"/.*-review.png
identify "$sheet_dir"/*.png
