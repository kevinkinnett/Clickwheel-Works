param()

$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

& "$PSScriptRoot\Build-Simulator.ps1" -Plugin storyclock

podman run --rm `
    -v "${projectRoot}:/project" `
    -v chronolith-rockbox-src:/rockbox `
    chronolith-rockbox-dev `
    sh -lc "sed 's/\r$//' /project/scripts/container-capture-storyclock-village.sh > /tmp/capture-storyclock-village.sh && sh /tmp/capture-storyclock-village.sh"

if ($LASTEXITCODE -ne 0) {
    throw "Story Clock village capture failed with exit code $LASTEXITCODE."
}

Write-Host 'Story Clock village review sheets are ready under artifacts/screenshots/storyclock/village.'
