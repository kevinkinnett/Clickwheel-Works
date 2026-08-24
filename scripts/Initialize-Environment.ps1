$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot

podman build -t chronolith-rockbox-dev -f "$projectRoot\Containerfile" $projectRoot
if ($LASTEXITCODE -ne 0) {
    throw "Chronolith development image build failed with exit code $LASTEXITCODE."
}

$volumeName = podman volume ls --format '{{.Name}}' | Where-Object { $_ -eq 'chronolith-rockbox-src' }
if (-not $volumeName) {
    podman volume create chronolith-rockbox-src | Out-Null
}

podman run --rm `
    --mount type=volume,source=chronolith-rockbox-src,target=/rockbox `
    chronolith-rockbox-dev `
    sh -lc "if [ ! -d /rockbox/src/.git ]; then git clone --branch v4.0-final --depth 1 https://github.com/Rockbox/rockbox.git /rockbox/src && cd /rockbox/src && git switch -c chronolith-build; fi"

if ($LASTEXITCODE -ne 0) {
    throw "Rockbox 4.0 source setup failed with exit code $LASTEXITCODE."
}

Write-Host 'Chronolith build environment is ready.'
