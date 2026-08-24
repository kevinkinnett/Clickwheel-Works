$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot

podman run --rm `
    --mount type=volume,source=chronolith-rockbox-src,target=/rockbox `
    --mount "type=bind,source=$projectRoot,target=/project" `
    chronolith-rockbox-dev `
    sh -lc "sed 's/\r$//' /project/scripts/container-build-device.sh > /tmp/build-device.sh && sh /tmp/build-device.sh"

if ($LASTEXITCODE -ne 0) {
    throw "Rockbox device build failed with exit code $LASTEXITCODE."
}
