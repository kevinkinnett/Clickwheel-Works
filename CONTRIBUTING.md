# Contributing

Build the host tests before touching the Rockbox simulator. They take about a
second and catch collision regressions without booting firmware.

```powershell
.\scripts\Test-PocketStep.ps1
```

Then build and capture Mushroom Clock.

```powershell
.\scripts\Build-Simulator.ps1 -Plugin mushroomclock
.\scripts\Capture-MushroomClock.ps1
```

Keep `pocketstep/pocketstep.h` independent of Rockbox. It must compile as C99 without
allocation, floating point, operating-system headers, or global state. Put
device APIs, clock behavior, drawing, and level selection in the application.

Add a host test for every collision bug. A visual capture is useful evidence,
but it is not a substitute for a repeatable assertion.
