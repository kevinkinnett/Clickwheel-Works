param(
    [ValidateSet('chronolith', 'mushroomclock', 'storyclock')]
    [string]$Plugin = 'chronolith'
)

$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$simulatorUrl = 'http://localhost:6080/vnc.html?autoconnect=1&resize=scale'

& "$PSScriptRoot\Build-Simulator.ps1" -Plugin $Plugin

Write-Host "Open $simulatorUrl"
podman container exists chronolith-simulator
if ($LASTEXITCODE -eq 0) {
    throw 'The clock simulator is already running. Use Stop-Simulator.ps1 first.'
}

podman run -d --rm `
    --name chronolith-simulator `
    -p 127.0.0.1:6080:6080 `
    --mount type=volume,source=chronolith-rockbox-src,target=/rockbox `
    --mount "type=bind,source=$projectRoot,target=/project" `
    chronolith-rockbox-dev `
    sh -lc "sed 's/\r$//' /project/scripts/container-run-simulator.sh > /tmp/run-simulator.sh && sh /tmp/run-simulator.sh" `
    | Out-Null

if ($LASTEXITCODE -ne 0) {
    throw "The Rockbox simulator could not start (exit code $LASTEXITCODE)."
}

Write-Host "$Plugin simulator is running. Use .\scripts\Stop-Simulator.ps1 when finished."
