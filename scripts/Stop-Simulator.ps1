$ErrorActionPreference = 'Stop'

podman container exists chronolith-simulator
if ($LASTEXITCODE -ne 0) {
    Write-Host 'The Chronolith simulator is not running.'
    exit 0
}

podman stop --time 5 chronolith-simulator | Out-Null
if ($LASTEXITCODE -ne 0) {
    throw "The Chronolith simulator could not be stopped (exit code $LASTEXITCODE)."
}

Write-Host 'Chronolith simulator stopped.'
