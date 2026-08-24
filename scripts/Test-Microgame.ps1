$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot

podman run --rm `
    --mount "type=bind,source=$projectRoot,target=/project" `
    chronolith-rockbox-dev `
    sh -lc "sed 's/\r$//' /project/scripts/container-test-microgame.sh > /tmp/test.sh && sh /tmp/test.sh"

if ($LASTEXITCODE -ne 0) {
    throw "Microgame tests failed with exit code $LASTEXITCODE."
}
