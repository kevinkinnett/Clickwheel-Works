#!/bin/sh
set -eu

build_dir=/rockbox/src/build-sim-ipodcolor
display=:99

export DISPLAY="$display"
export SDL_AUDIODRIVER=dummy
export SDL_VIDEODRIVER=x11
export XDG_RUNTIME_DIR=/tmp
export LIBGL_ALWAYS_SOFTWARE=1
export LP_NUM_THREADS=1

Xvfb "$display" -screen 0 900x700x24 -nolisten tcp >/tmp/chronolith-xvfb.log 2>&1 &
xvfb_pid=$!
trap 'kill "$sim_pid" "$vnc_pid" "$web_pid" "$xvfb_pid" 2>/dev/null || true' EXIT INT TERM
sleep 1

x11vnc -display "$display" -forever -shared -nopw -rfbport 5900 \
    >/tmp/chronolith-vnc.log 2>&1 &
vnc_pid=$!

websockify --web=/usr/share/novnc 6080 localhost:5900 \
    >/tmp/chronolith-websockify.log 2>&1 &
web_pid=$!

cd "$build_dir"
./rockboxui --root simdisk >/tmp/chronolith-simulator.log 2>&1 &
sim_pid=$!

echo 'Clock simulator is available at http://localhost:6080/vnc.html?autoconnect=1&resize=scale'
wait "$sim_pid"
