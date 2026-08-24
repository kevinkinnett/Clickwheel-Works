# Source notes: building animated clocks for a 2004 iPod

This is a factual handoff document for a later writing session. It is not meant
to establish a finished voice. It records what was observed, what was built,
what ran on the physical iPod, what failed during development, and which code
and media are worth using in an article.

Project dates: August 22 and 23, 2026.

## Short version

The project started with an old 60 GB color iPod that already ran Rockbox 3.15
and a simple clock plugin. The machine was identified as an iPod Photo/Color,
Rockbox target `ipodcolor`, rather than the third-generation monochrome iPod
suggested by the owner's first guess.

The work had four main parts:

1. Identify the hardware and preserve the working Rockbox installation.
2. Build a local Rockbox development and simulator environment pinned to
   Rockbox 4.0.
3. Create two native C plugins for the 220 by 176 display: Chronolith, a
   green-screen CRT system clock, and Mushroom Clock, an animated platform
   clock.
4. Iterate through simulator screenshots, install verified plugin builds on the
   physical iPod, and record an LCD-only video of one complete Mushroom Clock
   lap.

The final result runs as a normal Rockbox plugin. The existing Rockbox
bootloader was not replaced. Plugin files were staged, hashed after copying,
and followed by a normal Windows safe eject.

## Hardware and starting state

Facts observed from the case, Rockbox, the USB device, and the original disk:

- Apple model number: A1099.
- EMC number: 2022.
- Capacity: 60 GB.
- Rockbox hardware target: `ipodcolor`.
- Rockbox target ID: 13.
- CPU architecture reported by Rockbox: ARM.
- Memory reported by the old Rockbox installation: 32 MB.
- Display: 220 by 176 color LCD.
- Original Rockbox version: 3.15.
- Original Rockbox binary: `rockbox.ipod`.
- Original bootloader entry in `rbutil.log`:
  `bootloader-ipodcolor.ipod=2010-05-25T18:02:23`.
- Volume label when mounted in Windows: `KEVIN'S IPO`.
- Filesystem: FAT32.
- Reported disk size: 59,941,208,064 bytes.
- USB vendor and product IDs observed in Windows: Apple VID `05AC`, PID
  `1204`.
- The live hardware inspection also reported iPod P98, model M9830, with a
  Toshiba MK6006GAH disk.

The owner initially called it a third-generation iPod. That was a reasonable
memory of a twenty-year-old product line, but the color screen, A1099 case
number, and `ipodcolor` Rockbox target established the actual development
target.

The old installation was backed up before changing anything:

- Workspace backup:
  `backups/ipod-E-rockbox-3.15-20260822-112744`
- On-device backup created during the upgrade:
  `.rockbox-3.15-backup-20260822-112744`

The retained `rockbox-info.txt` from that backup is useful primary evidence. It
contains the target, target ID, version, memory, CPU, binary size, feature list,
and old compiler versions.

## Constraints that shaped the project

The limitations were the point rather than an inconvenience.

- The display is only 220 by 176 pixels.
- The target has 32 MB of memory and an ARM7-era CPU without the sort of
  graphics stack a modern application assumes.
- Rockbox plugins use the firmware's plugin API and must match the Rockbox
  build. A `.rock` file built for Rockbox 4.0 should not be copied onto Rockbox
  3.15.
- The iPod has no general network connection. A Rockbox plugin can read local
  system state, but it cannot casually call a web service.
- USB on this device is storage and input behavior, not an application data
  channel. A host-fed live dashboard would require work below the plugin layer,
  such as a custom firmware transport or accessory protocol. That work was
  explicitly deferred.
- The owner expected to use the clock while music was not playing, so the
  design did not depend on track metadata or audio visualization.

This led to two useful design rules. Use information already exposed by
Rockbox, and draw everything with cheap integer operations.

## Chronology

### 1. Identifying Rockbox and the device

The custom firmware was recognized as Rockbox. The iPod was running version
3.15. Once the case identifiers and disk were inspected, the build target was
set to `ipodcolor`.

The project pinned its source tree to Rockbox `v4.0-final`, commit
`e094c599fa60236527f9e272e0b8309d7696e399`. Rockbox's official iPod Color 4.0
package was kept alongside custom builds so the firmware and plugin APIs could
remain matched.

### 2. Building a local simulator

A containerized development environment was created around Debian 12, Podman,
SDL2, Xvfb, xdotool, ImageMagick, noVNC, and the Rockbox source tree. The same
source files build in two configurations:

- Rockbox simulator target, configured with `--type=S` and SDL threads.
- Physical iPod Color target, configured with `--type=N` and the bare-metal ARM
  compiler.

The simulator runs inside Xvfb. x11vnc and websockify expose it only on
localhost through noVNC:

```text
http://localhost:6080/vnc.html?autoconnect=1&resize=scale
```

This made visual iteration possible without copying every experimental build
to a twenty-year-old hard drive.

### 3. Chronolith, the first custom clock

The first custom clock was inspired by the NERV and CRT interfaces associated
with *Neon Genesis Evangelion*. The design brief was concrete: black field,
green phosphor, pixelated type, scan effects, dense technical readouts, and a
machine-room feeling.

An earlier orbital design remains in `src/chronolith-orbit.c`. The active
terminal design is `src/chronolith.c`.

Chronolith includes:

- A dot-matrix clock.
- A rotating battery reticle.
- A 127-point history graph for battery voltage and CPU frequency.
- Battery percentage and voltage.
- Estimated remaining battery time.
- CPU frequency.
- Runtime.
- USB, charging, external-power, and hold-switch state.
- A moving CRT scan beam.
- A seconds rail and a blinking boundary warning during seconds 55 through 59.
- Phosphor green, amber diagnostic, and red emergency color profiles.

The graphs use real iPod data. No network service is involved. Rockbox provides
the values through its plugin API:

```c
static void sample_telemetry(bool seed_history)
{
    int i;

    battery_mv = rb->battery_voltage();
    battery_percent = clamp_int(rb->battery_level(), 0, 100);
    battery_minutes = rb->battery_time();
    runtime_minutes = rb->global_status->runtime;
#if (CONFIG_PLATFORM & PLATFORM_NATIVE)
    cpu_mhz = (int)(*rb->cpu_frequency / 1000000L);
#else
    cpu_mhz = CPU_FREQ / 1000000;
#endif
    usb_present = rb->usb_inserted();

    if (seed_history)
    {
        for (i = 0; i < HISTORY_POINTS; ++i)
        {
            voltage_history[i] = battery_mv;
            cpu_history[i] = cpu_mhz;
        }
        history_head = 0;
    }
    else
    {
        voltage_history[history_head] = battery_mv;
        cpu_history[history_head] = cpu_mhz;
        history_head = (history_head + 1) % HISTORY_POINTS;
    }
}
```

The plugin redraws at 20 Hz and samples telemetry every fifth frame, or about
four times per second:

```c
#define FRAME_TICKS MAX(1, HZ / 20)

while (!quit)
{
    struct tm *now = rb->get_time();
    const struct profile *p = &profiles[selected_profile];

    draw_crt_field(p);
    draw_header(now, p);
    draw_time(now, p);
    draw_sync_panel(now, p);
    if ((frame_number % 5) == 0)
        sample_telemetry(false);
    draw_reticle(now, p);
    draw_telemetry(p);
    draw_status(now, p);
    rb->lcd_update();

    /* Read click-wheel actions, then advance the frame. */
}
```

### 4. Updating Rockbox without replacing the bootloader

The `.rockbox` files were updated from 3.15 to official Rockbox 4.0 so the new
plugins would use the matching plugin API. The target and version were checked
after the copy:

```text
Target: ipodcolor
Version: 4.0
```

The existing bootloader was retained. A custom `rockbox.ipod` build exists in
the artifacts directory because the device build produces a complete firmware
tree, but that generated firmware binary was not written to the firmware
partition during plugin iteration.

Chronolith was installed as:

```text
.rockbox/rocks/apps/chronolith.rock
```

### 5. Mushroom Clock

The second plugin changed direction completely. It became a side-scrolling,
Mario-like platform scene that also works as a clock. The source calls the
character a runner, and every sprite is drawn from rectangles, lines, and
individual pixels. There are no imported sprite sheets or Nintendo image
files.

The final scene includes:

- A large four-digit clock drawn with a custom 5 by 7 bitmap font.
- A `TIME` marquee, date, seconds, and real iPod battery gauge.
- Day, night, underground, and sunset stages.
- Automatic day, sunset, and night selection by time of day.
- An hourly underground event, latched at a natural lap reset between 50
  seconds before the hour and 30 seconds after it.
- Moving clouds, hills, ground tiles, bricks, question blocks, a pipe, a
  Goomba-like enemy, a coin, a mushroom, and a goal flag above ground.
- A separate underground layout with no flag, a turtle, and a poison mushroom.
- A runner with gravity, jumping, surface collisions, enemy collision, and
  power-up state.
- Two sparse block pairs whose question-block roles can swap between laps.
- A large runner sprite after collecting the mushroom.
- Randomized surface laps pairing Goombas or direction-aware Koopas with a
  mushroom, no power-up, or a collectible star.
- Three sparse block templates, randomized enemy patrol ranges, a high route,
  and a retreat route that briefly reverses the runner before a second pass.
- A five-point star, timed sparkle aura, and contact kills while invincible.
- Swept stomp checks, side-hit damage, and separate stomp and star-kill enemy
  animations.
- A flag that descends only after the runner reaches the pole.

The state is small enough to read in one structure:

```c
struct level_state
{
    int runner_x;
    int runner_y;
    int runner_vy;
    bool runner_on_surface;
    bool coin_block_hit;
    bool power_block_hit;
    int coin_block_bounce;
    int power_block_bounce;
    int coin_frames;
    int powerup_kind;
    int mushroom_mode;
    int mushroom_x;
    int mushroom_y;
    int mushroom_vy;
    int mushroom_direction;
    int enemy_x;
    int enemy_direction;
    int enemy_kind;
    bool enemy_alive;
    int enemy_death_frames;
    int enemy_death_style;
    bool runner_big;
    int star_frames;
    int hurt_frames;
    int death_frames;
    int runner_death_cause;
    int first_x;
    int first_y;
    int first_count;
    int second_x;
    int second_y;
    int second_count;
    int pipe_x;
    int pipe_y;
    bool power_in_first;
    int runner_direction;
    bool use_high_route;
    bool use_detour;
    int detour_frames;
    bool flag_grabbed;
    int flag_y;
};
```

### 6. Fixed-point animation on a small ARM target

Positions are stored in sixteenth-pixel units. No floating-point math is
needed. Adding 16 to a position moves one pixel. Adding 8 moves half a pixel.
Gravity can add 3 or 4 fixed-point units per frame.

The runner's basic integration now goes through PocketStep:

```c
runner.x = level.runner_x;
runner.y = level.runner_y;
runner.vx = level.runner_direction * PS_ONE;
runner.vy = level.runner_vy;
runner.width = 13;
runner.height = 16;

ps_apply_gravity(&runner, 4, 72);
movement = ps_move(&collision_world, &runner);
```

At 20 frames per second, that is a one-pixel horizontal step per frame with
quarter-pixel vertical acceleration. Drawing divides the values by 16.

The library resolves horizontal and vertical movement separately. A solid has
a numeric ID, so the application can tell the difference between the ground,
a pipe, and each question block:

```c
if (movement.hit_ceiling &&
    movement.vertical_id == first_question)
    hit_coin_block();
```

PocketStep is a header-only C99 library with no Rockbox dependency, allocator,
or floating point. Host tests compile it with an ordinary C compiler. The same
header then builds into the Rockbox plugin.

### 7. Making actions have causes

Several early animations were driven by the wall clock rather than the scene.
The original flag position, for example, came directly from seconds. It moved
up and down but had no relationship to the runner. The final version sets a
state flag when the runner reaches the pole:

```c
if (!level.flag_grabbed && new_x + 13 >= 204 && new_x < 214 &&
    new_bottom > 63)
    level.flag_grabbed = true;

if (level.flag_grabbed && level.flag_y < 128)
    level.flag_y = MIN(128, level.flag_y + 2);
```

The same principle separated the two question blocks. One function owns the
coin event and another owns the mushroom event:

```c
static void hit_coin_block(void)
{
    if (level.coin_block_hit)
        return;

    level.coin_block_hit = true;
    level.coin_block_bounce = 12;
    level.coin_frames = 36;
}

static void hit_power_block(void)
{
    if (level.power_block_hit)
        return;

    level.power_block_hit = true;
    level.power_block_bounce = 12;
    level.mushroom_mode = 1;
    level.mushroom_x = 118 * 16;
    level.mushroom_y = 103 * 16;
    level.mushroom_vy = 0;
    level.mushroom_direction = 1;
}
```

### 8. Correct mushroom layering and large Mario

The first mushroom animation drew the mushroom over the face of its question
block. The motion was numerically correct but visually wrong. The final draw
order hides the mushroom behind the block while it emerges, then draws it in
front after it is free:

```c
if (level.mushroom_mode == 1)
    draw_mushroom(level.mushroom_x / 16,
                  level.mushroom_y / 16, p);

draw_brick(100, 103, p);
draw_question_block(116, power_block_y,
                    level.power_block_hit, p);

if (level.mushroom_mode == 2)
    draw_mushroom(level.mushroom_x / 16,
                  level.mushroom_y / 16, p);
```

Collecting the mushroom originally produced a short flashing state with
sparkles. The owner correctly pointed out that a mushroom should make Mario
large, not invincible. The state became a boolean that lasts until the lap
resets:

```c
if (level.mushroom_mode != 0 &&
    overlaps(level.runner_x / 16, level.runner_y / 16, 13, 16,
             level.mushroom_x / 16, level.mushroom_y / 16, 13, 12))
{
    level.mushroom_mode = 0;
    level.runner_big = true;
}
```

The larger sprite is 24 pixels tall. It keeps the same feet position by drawing
from `y - 8`, which avoided rewriting every surface coordinate:

```c
if (big)
{
    int top = y - 8;

    use_color(red);
    rb->lcd_fillrect(x + 3, top, 9, 3);
    rb->lcd_fillrect(x + 1, top + 3, 13, 2);
    /* Head, torso, overalls, arms, and two walking poses follow. */
    return;
}
```

## Bugs found through visual iteration

The simulator screenshots did more than confirm color choices. They exposed
semantic and geometry bugs that were difficult to spot by reading C.

### Upside-down hills

The first hill loop widened the shape in the wrong direction. The row math was
reversed so each higher row becomes narrower. This sounds trivial, but on a
220-pixel screen the silhouette is most of the background.

### Glitching sprites

Early positions were derived too directly from frame values, which made sprites
appear to jump. Persistent fixed-point state replaced those calculations. The
runner, enemy, and mushroom now advance from their previous position.

### No collision system

The first Mario-like animation was a collection of moving drawings. Adding
gravity and collisions made it a scene. The runner can land on blocks and the
pipe, hit blocks from below, stomp or contact the enemy, and collect the
mushroom.

That first collision code was still scattered through the animation update.
Adding randomized blocks and routes made its assumptions fight each other. We
extracted the rules into `pocketstep/pocketstep.h`, then added command-line tests for
floor landing, question-block underside hits, pipe walls, and stomps.

### The enemy feedback loop

A side collision with the enemy originally moved the runner backward, then a
later revision incorrectly killed the enemy to keep the scene moving. The final
rule is stricter. Only a descending feet-crossing counts as a stomp. Horizontal
contact leaves the enemy alive and hurts or kills the runner. Star contact is
the separate exception.

The regression test is small enough to state plainly. Two actors can overlap
from the side and `ps_crossed_top` must return false. If the runner's previous
feet were above the enemy and the new feet crossed its top, it returns true.

### The pipe trap

The lower block ledge originally ended two pixels before the pipe. The runner
could become trapped between the ledge's underside and the pipe wall. The ledge
was shortened from three blocks to two, widening the approach and giving the
collision solver room to resolve the jump.

This bug is a good article detail because the code compiled, the individual
collision checks were reasonable, and the scene still failed as a system.

### The ambiguous `MARIO x99` display

The first HUD placed a spinning coin beside the battery percentage and rendered
text similar to `MARIO x99`. It looked like a coin count, and the spinning coin
looked like a yellow box sliding back and forth. The fix was to remove that
visual pun and draw a real battery outline with a fill level and percentage.

### The flag that only tracked seconds

The initial flag rose and fell based on `tm_sec`. It was technically a time
indicator but visually meaningless. Connecting it to the pole collision gave
the movement an understandable cause.

### One block producing two unrelated items

The first interaction emitted both a coin and mushroom from the same block.
Separate block state, bounce state, and trigger functions made the scene easier
to read.

### Mushroom in front of its block

The emergence coordinates were fine. The layering was not. Drawing the rising
mushroom before the block fixed the occlusion without changing its physics.

### Mushroom as invincibility

The flashing power-up effect was replaced by a larger sprite. The feedback was
small, specific, and correct. It changed both the state model and drawing code.

## Clock and HUD design

The large clock is a small bitmap font embedded as ten arrays of seven rows.
Each row is a five-bit mask. Lit bits become layered pixel blocks with a dark
outline, blue shadow, white face, and yellow highlight. This is cheaper and
more controllable than loading a font at runtime.

The HUD uses live state:

```c
rb->snprintf(percent, sizeof(percent), "%02d%%", MAX(0, battery));
rb->snprintf(right, sizeof(right), "WORLD %02d-%02d SEC %02d",
             now->tm_mon + 1, now->tm_mday, now->tm_sec);

rb->lcd_putsxy(4, 2, "MARIO");
draw_battery_gauge(39, 2, battery, p);
rb->lcd_putsxy(62, 2, percent);
rb->lcd_putsxy(110, 2, right);
```

The clock can switch between 12-hour and 24-hour display. The click wheel also
selects the surface palettes. Auto mode uses day, sunset, or night according to
the current hour. Underground is an hourly event in the iPod build. The event
becomes eligible at minute 59, second 10, and stays eligible through minute 0,
second 29. Entry and exit occur only when the current lap resets, so the scene
never changes halfway through a run.

## Building for the simulator and iPod

The build scripts copy the two plugin sources into Rockbox's `apps/plugins`
tree and add them to `SOURCES` and `CATEGORIES`. The physical build uses:

```sh
../tools/configure --target=ipodcolor --type=N --no-ccache
```

The simulator build uses:

```sh
../tools/configure --target=ipodcolor --type=S --no-ccache --sdl-threads
```

Debian 12 supplies GCC 12's `arm-none-eabi` toolchain. Rockbox expects the old
`arm-elf-eabi` command prefix, so the container creates compatible symlinks.
The project also adds `-fcommon` for legacy Rockbox code that predates GCC's
change to `-fno-common` defaults:

```sh
if ! grep -q 'GCCOPTS=-fcommon ' Makefile; then
    sed -i 's/^export GCCOPTS=/export GCCOPTS=-fcommon /' Makefile
fi
```

The device build creates standalone plugins, a complete custom Rockbox ZIP,
official Rockbox 4.0 packages with plugin overlays, a firmware binary, and a
SHA-256 manifest. Only the appropriate official Rockbox files and standalone
plugins were installed during this session.

## Installation safety on the physical iPod

The installation workflow was intentionally conservative:

1. Confirm the removable volume label, FAT32 filesystem, size, and Rockbox
   metadata.
2. Confirm `Target: ipodcolor` and `Version: 4.0` before copying a 4.0 plugin.
3. Back up the existing plugin in the workspace.
4. Copy the replacement to a unique `.pending` filename on the iPod.
5. Compare the staged SHA-256 hash with the source.
6. Move the staged file into place.
7. Hash the installed file again.
8. Use Windows' normal shell `Eject` verb and wait for the drive letter to
   disappear.

Three Mushroom Clock device builds were installed during iteration:

| Stage | Bytes | SHA-256 |
|---|---:|---|
| Smoother movement and collision revision | 11,100 | `dcb08f7f70ba975933e52dfe7a7669d3086a72018f17e44874c8cbcf2ac0bc70` |
| HUD, separate blocks, event-driven flag, collision fixes | 12,456 | `23e67612f700d09360edff12d748a651ddeef6bf6a973387d0f86066801d35ac` |
| Correct emergence layering and large runner | 13,136 | `ea69058d362d6b1fab504430ca2aa660d42f5df28d2bde6039fdebce9abbb696` |

The two earlier plugin files were retained as:

- `backups/mushroomclock-ipod-before-20260822-200408.rock`
- `backups/mushroomclock-ipod-before-20260822-210408.rock`

The final 13,136-byte build was copied to the physical iPod, verified by hash,
and safely ejected. No later physical-device defect was reported in this
session.

## Automated screenshots

The screenshot process launches the Rockbox simulator in Xvfb, locates its SDL
window with xdotool, waits for selected animation moments, changes palettes by
sending virtual button presses, and captures PNGs with ImageMagick.

The Mushroom Clock test sheet includes:

- Runner entering the stage.
- Coin block interaction.
- Mushroom and power-up sequence.
- Flag interaction.
- Night palette.
- Underground palette.
- Sunset palette.
- Automatic palette in 12-hour mode.

The sequence caught the pipe trap because later screenshots showed the runner
at the same obstacle instead of at the flag. Extending the capture was a simple
but effective animation regression test.

## LCD-only video capture

The owner requested a video after the final plugin was installed. FFmpeg was
added to the container. The recorder captures the simulator's X11 window, crops
the exact 220 by 176 LCD rectangle, and enlarges it four times with nearest
neighbor scaling:

```sh
ffmpeg -f x11grab -draw_mouse 0 -framerate 20 \
    -video_size "${WIDTH}x${HEIGHT}" -i "${DISPLAY}+${X},${Y}" \
    -t 16 \
    -vf "crop=220:176:20:15,scale=880:704:flags=neighbor,format=yuv420p" \
    -an -c:v libx264 -preset slow -crf 16 "$raw_video"
```

The first recording included Rockbox's startup logo and ended after the next
lap had begun. Checkpoint frames established that the clock appears about 1.5
seconds into the raw capture and completes its lap before 14.5 seconds. A second
encoding trims the raw video to one clean 13-second run:

```sh
ffmpeg -ss 1.4 -i "$raw_video" -t 13 \
    -an -c:v libx264 -preset slow -crf 16 -pix_fmt yuv420p \
    -movflags +faststart "$lcd_video"
```

Final video facts:

- File: `artifacts/video/mushroomclock-complete-run.mp4`
- Codec: H.264.
- Resolution: 880 by 704.
- Frame rate: 20 fps.
- Duration: 13.000 seconds.
- Frames: 260.
- Size: 91,989 bytes.
- Audio: none.
- SHA-256:
  `0b9643fe0c23634d425f2fdf729c33f4cb6c6d05349ede1d224386291a090f8c`.

## Final device artifacts

The final manifest records:

| Artifact | Bytes | SHA-256 |
|---|---:|---|
| `chronolith-ipodcolor-rockbox-4.0.rock` | 8,964 | `2880de9c8832221a9068ca9a467aaaa81ee4baf6e402e7f08ee343fa8f597ad6` |
| `mushroomclock-ipodcolor-rockbox-4.0.rock` | 13,136 | `ea69058d362d6b1fab504430ca2aa660d42f5df28d2bde6039fdebce9abbb696` |
| `rockbox-chronolith.ipod` | 719,584 | `e2568a3553a68ebb3769805db9b4183c27a65cefdf13021ef7ff28b15e1b599b` |
| `rockbox-ipodcolor-4.0-official-plus-chronolith.zip` | 8,907,653 | `4148d6bfd29060120089654f084b59ccaa6d29b35661908083dc0c4be761091e` |
| `rockbox-ipodcolor-4.0-official-plus-clocks.zip` | 8,914,929 | `7415689dc254a5f7c9d020faaccb23b773166424d95f4c341e7d09b7be8eda24` |
| `rockbox-ipodcolor-custom-clocks-4.0.zip` | 8,837,790 | `d48680f37574807141de5f45e0a19e62540d0354f48854d025a94416ba7d008d` |

The complete checksums remain in `artifacts/device/SHA256SUMS`.

## Reproduction commands

From PowerShell in the repository:

```powershell
# Build the development container and obtain Rockbox v4.0-final.
.\scripts\Initialize-Environment.ps1

# Build and run Mushroom Clock in the simulator.
.\scripts\Build-Simulator.ps1 -Plugin mushroomclock
.\scripts\Run-Simulator.ps1

# Capture the screenshot regression sheet.
.\scripts\Capture-MushroomClock.ps1

# Build the ARM plugin and complete device packages.
.\scripts\Build-Device.ps1

# Record one LCD-only lap.
.\scripts\Record-MushroomClock.ps1

# Stop the live simulator.
.\scripts\Stop-Simulator.ps1
```

## Media to give the article session

Chronolith overview:

![Chronolith terminal profiles](../artifacts/screenshots/chronolith-terminal-sheet.png)

Mushroom Clock overview:

![Mushroom Clock sequence and palettes](../artifacts/screenshots/mushroomclock/mushroomclock-sheet.png)

Individual LCD-only Mushroom Clock frames:

- `artifacts/screenshots/mushroomclock/lcd/01-auto-run.png`
- `artifacts/screenshots/mushroomclock/lcd/02-auto-block.png`
- `artifacts/screenshots/mushroomclock/lcd/03-auto-power.png`
- `artifacts/screenshots/mushroomclock/lcd/04-auto-flag.png`
- `artifacts/screenshots/mushroomclock/lcd/05-night.png`
- `artifacts/screenshots/mushroomclock/lcd/06-underground.png`
- `artifacts/screenshots/mushroomclock/lcd/07-sunset.png`
- `artifacts/screenshots/mushroomclock/lcd/08-auto-12h.png`

Final video:

`artifacts/video/mushroomclock-complete-run.mp4`

## Useful article angles for the next session

These are handles supported by the work, not claims that must appear in the
article.

### Constraints made the interaction legible

At 220 by 176, a meaningless animation cannot hide in detail. The flag either
responds to Mario or it does not. The mushroom either emerges from behind the
block or paints over its face. The small display made causal mistakes obvious.

### Simulation made the physical hardware safer

The simulator was not only faster. It kept unfinished builds off an old hard
drive, let the scene run through repeatable checkpoints, and exposed the pipe
trap before another device installation.

### Generated code still needed visual judgment

The compiler accepted the upside-down hills, ambiguous battery display,
unmotivated flag, repeated enemy collision, and bad mushroom layering. A person
looking at the screen supplied the specifications that mattered.

### Old hardware can still expose useful live state

Chronolith did not need a cloud service. Rockbox already exposed battery,
voltage, CPU, runtime, USB, charging, and hold state. A local dashboard was more
appropriate for the device than inventing a network path.

### Safe automation needs narrow boundaries

The project generated complete firmware, but installation steps stayed narrow.
The bootloader remained untouched. Each plugin replacement had a backup, a
staging filename, two hash checks, and a safe eject.

## Claims the article should not overstate

- Do not call the device a third-generation iPod. Use iPod Photo/Color, A1099,
  EMC 2022, 60 GB, or Rockbox target `ipodcolor`.
- Do not say the bootloader was upgraded. The existing bootloader was retained.
- Do not imply the custom `rockbox.ipod` artifact was flashed during the plugin
  iterations.
- Do not claim the iPod streamed web data. Chronolith used local Rockbox APIs.
- Do not say the project used original Nintendo sprite assets. It drew small
  fan-art shapes from LCD primitives.
- Do not imply the video has audio. It is deliberately silent for later voice
  production.
- Do not claim every generated ZIP was installed. The official Rockbox 4.0
  files and standalone plugin builds were the relevant installed pieces.
- If calling Rockbox 4.0 the latest release, verify that statement again at
  publication time. The project itself is pinned to `v4.0-final`.
- The project is a personal, noncommercial fan experiment. Mario, Nintendo,
  NERV, and *Neon Genesis Evangelion* references should be described as
  inspirations, not affiliations or official assets.

## Source map

- `src/chronolith.c`: active CRT system clock.
- `src/chronolith-orbit.c`: preserved earlier orbital clock design.
- `src/mushroomclock.c`: final animated platform clock.
- `Containerfile`: Debian, Rockbox toolchain, simulator, noVNC, and FFmpeg
  dependencies.
- `scripts/container-build-simulator.sh`: simulator configuration and plugin
  integration.
- `scripts/container-build-device.sh`: ARM build, official package overlay, and
  checksums.
- `scripts/container-capture-simulator.sh`: Chronolith screenshot automation.
- `scripts/container-capture-mushroomclock.sh`: Mushroom Clock animation and
  palette checkpoints.
- `scripts/container-record-mushroomclock.sh`: LCD crop, scaling, H.264 encode,
  and lap trim.
- `README.md`: concise project instructions.
- `artifacts/device/SHA256SUMS`: final device artifact hashes.
