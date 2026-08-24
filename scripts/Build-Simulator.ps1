param(
    [ValidateSet('chronolith', 'mushroomclock', 'storyclock')]
    [string]$Plugin = 'chronolith'
)

$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot

podman run --rm `
    --env "CLOCK_PLUGIN=$Plugin" `
    --mount type=volume,source=chronolith-rockbox-src,target=/rockbox `
    --mount "type=bind,source=$projectRoot,target=/project" `
    chronolith-rockbox-dev `
    sh -lc "sed 's/\r$//' /project/scripts/container-build-simulator.sh > /tmp/build.sh && sh /tmp/build.sh"

if ($LASTEXITCODE -ne 0) {
    throw "Rockbox simulator build failed with exit code $LASTEXITCODE."
}
