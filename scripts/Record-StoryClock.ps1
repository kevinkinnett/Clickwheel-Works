param(
    [ValidateSet('auto', 'day', 'evening', 'night',
                 'market', 'mill', 'gate', 'farm', 'garden')]
    [string]$Scenario = 'day'
)

$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

& "$PSScriptRoot\Build-Simulator.ps1" -Plugin storyclock

podman run --rm `
    -e "STORYCLOCK_SCENARIO=$Scenario" `
    -v "${projectRoot}:/project" `
    -v chronolith-rockbox-src:/rockbox `
    chronolith-rockbox-dev `
    sh -lc "sed 's/\r$//' /project/scripts/container-record-storyclock.sh > /tmp/record-storyclock.sh && sh /tmp/record-storyclock.sh"

if ($LASTEXITCODE -ne 0) {
    throw "Story Clock recording failed with exit code $LASTEXITCODE."
}

Write-Host "Story Clock $Scenario video is ready under artifacts/video."
