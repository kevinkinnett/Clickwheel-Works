param()

$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

& "$PSScriptRoot\Build-Simulator.ps1" -Plugin storyclock

podman run --rm `
    -v "${projectRoot}:/project" `
    -v chronolith-rockbox-src:/rockbox `
    chronolith-rockbox-dev `
    sh -lc "sed 's/\r$//' /project/scripts/container-capture-storyclock-inventory.sh > /tmp/capture-storyclock-inventory.sh && sh /tmp/capture-storyclock-inventory.sh"

if ($LASTEXITCODE -ne 0) {
    throw "Story Clock inventory capture failed with exit code $LASTEXITCODE."
}

Write-Host 'Story Clock inventory frames are ready under artifacts/screenshots/storyclock/inventory.'
