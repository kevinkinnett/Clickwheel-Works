param(
    [ValidateSet('auto', 'day', 'evening', 'night',
                 'market', 'mill', 'gate', 'farm', 'garden')]
    [string]$Scenario = 'day'
)

$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

podman run --rm `
    -e "STORYCLOCK_SCENARIO=$Scenario" `
    -v "${projectRoot}:/project" `
    chronolith-rockbox-dev `
    sh -lc "mkdir -p /project/artifacts/screenshots/storyclock; ffmpeg -hide_banner -loglevel error -y -i /project/artifacts/video/storyclock-$Scenario-complete-story.mp4 -vf 'fps=1/6,scale=220:176:flags=neighbor,tile=4x3' -frames:v 1 /project/artifacts/screenshots/storyclock/$Scenario-story-sheet.png; identify /project/artifacts/screenshots/storyclock/$Scenario-story-sheet.png"

if ($LASTEXITCODE -ne 0) {
    throw "Story Clock contact sheet failed with exit code $LASTEXITCODE."
}
