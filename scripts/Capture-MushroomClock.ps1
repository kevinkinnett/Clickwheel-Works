$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot

podman run --rm `
    --mount type=volume,source=chronolith-rockbox-src,target=/rockbox `
    --mount "type=bind,source=$projectRoot,target=/project" `
    chronolith-rockbox-dev `
    sh -lc "sed 's/\r$//' /project/scripts/container-capture-mushroomclock.sh > /tmp/capture-mushroomclock.sh && sh /tmp/capture-mushroomclock.sh"

if ($LASTEXITCODE -ne 0) {
    throw "Mushroom Clock simulator capture failed with exit code $LASTEXITCODE."
}
