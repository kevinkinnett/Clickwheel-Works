FROM debian:12-slim

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        ffmpeg \
        gdb \
        git \
        imagemagick \
        libsdl2-dev \
        perl \
        procps \
        xdotool \
        xvfb \
        zip \
    && rm -rf /var/lib/apt/lists/*

# Debian's bare-metal ARM toolchain is compatible with the iPod's ARM7TDMI.
# Rockbox uses the historical arm-elf-eabi prefix, so expose matching aliases.
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        binutils-arm-none-eabi \
        gcc-arm-none-eabi \
    && for tool in /usr/bin/arm-none-eabi-*; do \
        name="$(basename "$tool")"; \
        ln -s "$tool" "/usr/local/bin/arm-elf-eabi-${name#arm-none-eabi-}"; \
    done \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        novnc \
        websockify \
        x11vnc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace/rockbox
