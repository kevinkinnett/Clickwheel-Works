# Clickwheel Works

This repository contains three animated clocks for the iPod Photo/Color and a
small C99 game library extracted from the clock experiments. The target is
Rockbox `ipodcolor`, tested on a 60 GB A1099 / EMC 2022.

## PocketStep

[`pocketstep/pocketstep.h`](pocketstep/pocketstep.h) is a header-only
fixed-point motion and collision library for embedded C games. It has no
Rockbox dependency, heap
allocation, floating point, file I/O, or operating-system calls. Mushroom Clock
uses it for ground and block collision, pipe contacts, moving power-ups, actor
overlap, strict stomp classification, destructible colliders, and
bottom-anchored character resizing.

The optional `pocketstep_grid.h` and `pocketstep_story.h` modules add
fixed-memory tile routing, facing interaction probes, and completion-driven
autonomous sequences. Story Clock uses them for its top-down vignette.

Run its host tests without booting Rockbox.

```powershell
.\scripts\Test-PocketStep.ps1
```

The API and limits are documented in the
[`PocketStep README`](pocketstep/README.md). PocketStep also lives in its own
[GitHub repository](https://github.com/kevinkinnett/PocketStep). The engine uses
the MIT License. The Rockbox plugins use GPL-2.0-or-later.

## Chronolith

Chronolith is an animated CRT command-terminal clock for the 220×176 color
display in the iPod Photo/Color (Rockbox target `ipodcolor`). It was built
specifically for the 60 GB A1099 / EMC 2022.

The display combines a dot-matrix clock, rotating battery reticle, live power
and CPU history, seconds rail, moving CRT scan beam, and a countdown warning
during the last five seconds of each minute. It reports battery percentage and
voltage, estimated battery time, CPU clock, runtime, charging state, USB state,
and the hold switch. Phosphor green is the default. Amber diagnostic and red
emergency profiles are available from the click wheel.

The first orbital design is preserved in `src/chronolith-orbit.c`. The active
terminal design lives in `src/chronolith.c`.

## Mushroom Clock

Mushroom Clock is a separate animated 8-bit platform clock and the working demo
for PocketStep. A pixel runner crosses a looping stage using fixed-point motion,
gravity, and collision checks against the ground, blocks, pipe, enemies, and
power-ups. Each surface level has two pairs of blocks. One question block
releases a coin. The other releases a coin, mushroom, or star.
Their order, position within each pair, and height can change between laps.
The mushroom changes the runner into a taller form. Big Mario breaks individual
bricks when he hits them from below. The goal flag stays at the
top until the runner reaches the pole, then slides down. The HUD labels the date and seconds and shows the
iPod battery as a gauge with a percentage. Each surface lap randomly chooses a
power-up, enemy type, enemy position and direction, and one of three sparse
block templates. Consecutive laps cannot reuse the same layout or exact
configuration. A lap may use the ground route, retreat before a second
approach, or cross the block tops. The star makes the runner alternate colors,
adds a sparkle aura, and lets him knock enemies away. Stomps and star hits have
separate death animations. Side contact leaves the enemy alive and hurts or
kills the runner.
Auto mode follows the time of day.
At 50 seconds before each hour, the next natural lap reset starts an underground
stage. Underground laps rotate through three block layouts, vary the question
blocks, have no flag, use a turtle enemy, and end with a poison mushroom that
kills the runner. The next lap reset after 30 seconds past the hour returns
above ground. Day, night, and sunset can also be selected
with the click wheel. The simulator retains a manual underground selection for
repeatable visual testing; the iPod build schedules it only from the clock.

## Story Clock

Story Clock is the temporary internal name for an autonomous top-down story
screensaver. It is not a playable game. Luma wakes in a 13×11 tile house,
collects an ember key, speaks with Mira, follows a collision-safe route outside,
meets Tovin, reaches the hill beacon, and then begins the vignette again.

The program uses caller-owned breadth-first-search storage, fixed-point motion
between tile centers, a small foot collider, foot-Y depth sorting, two-line
automatically timed dialogue, and an inventory indicator. The clock continues
to read Rockbox time during movement and conversations. Outdoor colors follow
day, evening, and night while the map and collision cells remain unchanged.
Simulator-only scenario files can force a palette for deterministic review.
All maps, characters, dialogue, and pixel art are original to this project.

## Controls

- Scroll the click wheel to switch between Phosphor, Magi Amber, and Emergency.
- Press Select, Previous, or Next to switch between 12- and 24-hour time.
- Press Menu to exit.

The plugin keeps the backlight awake while it is open. Rockbox resumes the
normal backlight setting when it closes.

## Run the simulator

Podman Desktop must be installed and its machine must be running. From
PowerShell:

```powershell
.\scripts\Initialize-Environment.ps1
.\scripts\Build-Simulator.ps1 -Plugin storyclock
.\scripts\Run-Simulator.ps1 -Plugin storyclock
```

Then open
`http://localhost:6080/vnc.html?autoconnect=1&resize=scale`. The simulator is
served only on localhost. Stop it when finished:

```powershell
.\scripts\Stop-Simulator.ps1
```

For a repeatable headless visual test:

```powershell
.\scripts\Capture-Simulator.ps1
```

That boots Rockbox, launches Chronolith, operates the virtual click wheel, and
writes full-device and LCD-only PNGs under `artifacts/screenshots`.

The `-Plugin` argument accepts `chronolith`, `mushroomclock`, or `storyclock`.
The selected plugin becomes the simulator's `autostart.rock`; the source files
do not need to be edited.

To build and capture Mushroom Clock instead:

```powershell
.\scripts\Build-Simulator.ps1 -Plugin mushroomclock
.\scripts\Capture-MushroomClock.ps1
```

Mushroom Clock also has eight deterministic review scenarios. They cover
mushroom growth, an all-coin run, a star attack, the high route, retreat, a
stomp, a fatal side collision, and the underground poison run. These controls
exist only in the simulator; laps on the iPod remain random.

```powershell
.\scripts\Record-MushroomScenarios.ps1
.\scripts\Build-MushroomScenarioSheets.ps1
```

The first command writes one 15-second LCD-only MP4 per scenario under
`artifacts/video`. The second creates per-scenario contact sheets and a combined
review sheet under `artifacts/screenshots/mushroomclock/scenarios`.

Record the complete autonomous Story Clock loop and build its LCD-only review
sheet with:

```powershell
.\scripts\Record-StoryClock.ps1 -Scenario day
.\scripts\Build-StoryClockContactSheet.ps1 -Scenario day
```

`Scenario` can be `auto`, `day`, `evening`, or `night`. The MP4 is written to
`artifacts/video`; the 4×2 review sheet is under
`artifacts/screenshots/storyclock`.

The captures go under `artifacts/screenshots`. The repository ignores that
directory because recordings and device builds are local products.

## Build for the iPod

```powershell
.\scripts\Build-Device.ps1
```

This creates:

- `artifacts/device/rockbox-ipodcolor-4.0-official-plus-chronolith.zip` is the
  recommended install: Rockbox's official 4.0 iPod Color package with the
  matching Chronolith plugin added.
- `artifacts/device/rockbox-ipodcolor-chronolith-4.0.zip` is the complete matching
  development build, including Chronolith.
- `artifacts/device/chronolith-ipodcolor-rockbox-4.0.rock` is the standalone plugin
  for this exact custom Rockbox build.
- `artifacts/device/mushroomclock-ipodcolor-rockbox-4.0.rock` is the standalone
  Mushroom Clock plugin for Rockbox 4.0.
- `artifacts/device/storyclock-ipodcolor-rockbox-4.0.rock` is the standalone
  Story Clock plugin for Rockbox 4.0.
- `artifacts/device/rockbox-ipodcolor-4.0-official-plus-clocks.zip` is official
  Rockbox 4.0 with all three clock plugins added.
- `artifacts/device/rockbox-chronolith.ipod` is the firmware binary.
- `artifacts/device/SHA256SUMS` contains checksums for the device artifacts.

The device build uses Debian's GCC 12 bare-metal ARM compiler with Rockbox's
legacy `-fcommon` compatibility mode. The generated firmware, codecs, and
plugins all complete successfully. The final Chronolith plugin is 8.8 KB on
disk and uses 8,962 bytes of code/data plus 1,180 bytes of static memory.

## Install

Back up the iPod first. The recommended package upgrades the Rockbox files to
official 4.0 and adds Chronolith; it does not replace the existing Rockbox
bootloader. Both clock plugins have been simulator-tested and run on the
physical iPod. Story Clock has been simulator-tested; its new device artifact
has not yet been exercised on the physical iPod. The generated custom firmware
artifact was not flashed during plugin iteration.

1. Connect the iPod in disk mode and expose hidden files.
2. Extract `rockbox-ipodcolor-4.0-official-plus-chronolith.zip` into the iPod's
   root, allowing it to merge with and replace files under `.rockbox`.
3. Safely eject and reboot into Rockbox.
4. Open **Plugins → Applications → Chronolith**.

The standalone `.rock` should not be copied onto Rockbox 3.15 because Rockbox
plugins are tied to the firmware plugin API. Use the complete ZIP so the
firmware and plugin stay matched.

## Source and license

The plugins live in `src/chronolith.c`, `src/mushroomclock.c`, and
`src/storyclock.c`. The build is pinned to Rockbox
`v4.0-final` (`e094c599fa60236527f9e272e0b8309d7696e399`). Chronolith is
licensed under GPL-2.0-or-later, matching Rockbox.
