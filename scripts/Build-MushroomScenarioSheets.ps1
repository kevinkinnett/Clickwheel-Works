$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

podman run --rm `
    -v "${projectRoot}:/project" `
    chronolith-rockbox-dev `
    sh -lc "sed 's/\r$//' /project/scripts/container-build-mushroom-scenario-sheets.sh > /tmp/sheets.sh && sh /tmp/sheets.sh"

if ($LASTEXITCODE -ne 0) {
    throw "Mushroom Clock scenario sheets failed with exit code $LASTEXITCODE."
}
