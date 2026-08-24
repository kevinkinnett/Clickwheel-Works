$ErrorActionPreference = 'Stop'

$scenarios = @(
    'mushroom',
    'empty',
    'star',
    'high',
    'retreat',
    'stomp',
    'sidehit',
    'poison'
)

foreach ($scenario in $scenarios) {
    & "$PSScriptRoot\Record-MushroomClock.ps1" -Scenario $scenario
}

Write-Host 'All Mushroom Clock scenario videos are ready under artifacts/video.'
