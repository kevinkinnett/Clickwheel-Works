param(
    [ValidateSet('chronolith', 'mushroomclock', 'storyclock')]
    [string]$Plugin = 'chronolith'
)

$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot

& "$PSScriptRoot\Build-Simulator.ps1" -Plugin $Plugin

podman run --rm `
    --mount type=volume,source=chronolith-rockbox-src,target=/rockbox `
    --mount "type=bind,source=$projectRoot,target=/project" `
    chronolith-rockbox-dev `
    sh -lc "sed 's/\r$//' /project/scripts/container-capture-simulator.sh > /tmp/capture.sh && sh /tmp/capture.sh"

if ($LASTEXITCODE -ne 0) {
    throw "Rockbox simulator capture failed with exit code $LASTEXITCODE."
}
