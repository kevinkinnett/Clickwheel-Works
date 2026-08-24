param(
    [ValidateSet('auto', 'night', 'underground', 'sunset', 'variants')]
    [string]$World = 'auto',

    [ValidateSet('random', 'mushroom', 'coins', 'empty', 'star', 'high', 'retreat',
                 'stomp', 'sidehit', 'poison')]
    [string]$Scenario = 'random'
)

$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

podman run --rm `
    -e "CLOCK_WORLD=$World" `
    -e "CLOCK_SCENARIO=$Scenario" `
    -v "${projectRoot}:/project" `
    -v chronolith-rockbox-src:/rockbox `
    chronolith-rockbox-dev `
    sh -lc "sed 's/\r$//' /project/scripts/container-record-mushroomclock.sh > /tmp/record-mushroomclock.sh && sh /tmp/record-mushroomclock.sh"

if ($LASTEXITCODE -ne 0) {
    throw "Mushroom Clock recording failed with exit code $LASTEXITCODE."
}

Write-Host "Mushroom Clock $Scenario scenario video is ready under artifacts/video."
