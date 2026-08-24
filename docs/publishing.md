# Publishing this project

The engine is ready to publish as `src/microgame.h`, its tests, CMake file, and
documentation. Mushroom Clock needs one more editorial pass before it should
be the public demo.

Its current HUD and hand-drawn characters deliberately resemble a Nintendo
game. Source availability does not grant rights to those names or character
designs. A public version should use an original runner, enemies, blocks,
power-ups, title, and screenshots. The clock mechanics and Microgame code do
not depend on the current art.

Do not commit local build products, Rockbox source checkouts, iPod backups, or
device recordings. The repository's `.gitignore` already excludes `rockbox/`
and `artifacts/`.

Before the first push:

1. Choose the repository name and replace the Mushroom Clock art or omit that
   demo from the initial release.
2. Run `scripts/Test-Microgame.ps1`.
3. Build both simulator plugins.
4. Build the iPod package.
5. Review the generated checksums under `artifacts/device` without committing
   those binaries unless the release policy calls for them.
