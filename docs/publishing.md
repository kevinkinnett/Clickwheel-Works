# Repository layout

Clickwheel Works is the main repository. The engine lives in its self-contained
`pocketstep/` directory. That directory publishes to the separate PocketStep
repository with `git subtree`.

Its current HUD and hand-drawn characters deliberately resemble a Nintendo
game. Source availability does not grant rights to those names or character
designs. A public version should use an original runner, enemies, blocks,
power-ups, title, and screenshots. The clock mechanics and PocketStep code do
not depend on the current art.

Do not commit local build products, Rockbox source checkouts, iPod backups, or
device recordings. The repository's `.gitignore` already excludes `rockbox/`
and `artifacts/`.

Before a release:

1. Replace the Mushroom Clock placeholder art or omit that demo from the
   release package.
2. Run `scripts/Test-PocketStep.ps1`.
3. Build both simulator plugins and the iPod package.
4. Review the generated checksums under `artifacts/device` without committing
   those binaries unless the release policy calls for them.
