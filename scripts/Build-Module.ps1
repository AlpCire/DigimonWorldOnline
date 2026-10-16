param(
    [Parameter(Mandatory = $true)][string]$CorePath,
    [int]$Jobs = 6
)
$ErrorActionPreference = 'Stop'
$revision = '01339ccfab9cd5d3d28546f79f38ad74d394b148'
$project = Split-Path $PSScriptRoot -Parent
$core = (Resolve-Path -LiteralPath $CorePath).Path
$actual = (& git -C $core rev-parse HEAD).Trim()
if ($LASTEXITCODE -ne 0 -or $actual -ne $revision) {
    throw "Se requiere AzerothCore $revision. No se ha modificado el repositorio."
}
if ($Jobs -lt 1 -or $Jobs -gt 16) { throw 'Jobs debe estar entre 1 y 16.' }
$module = Join-Path $core 'modules\mod-digimon-tamer'
if (Test-Path -LiteralPath $module) {
    if (-not (Test-Path -LiteralPath (Join-Path $module 'src\DigimonTamer.cpp'))) {
        throw 'Ya existe otro contenido en la carpeta del módulo. Revísalo antes de instalar.'
    }
}
Copy-Item -LiteralPath (Join-Path $project 'modules\mod-digimon-tamer') -Destination (Join-Path $core 'modules') -Recurse -Force
& docker build -t world-digimon-build:alpha03 -f (Join-Path $project 'docker\Dockerfile.build') (Join-Path $project 'docker')
if ($LASTEXITCODE -ne 0) { throw 'No se pudo preparar el compilador.' }
& docker run --rm --cpus $Jobs --memory 24g `
    --mount "type=bind,source=$core,target=/src,readonly" `
    --mount "type=bind,source=$project\docker,target=/scripts,readonly" `
    --mount 'type=volume,source=world-digimon-build,target=/build' `
    --mount 'type=volume,source=world-digimon-dist,target=/dist' `
    --mount 'type=volume,source=world-digimon-ccache,target=/root/.cache/ccache' `
    --env "DIGIMON_BUILD_JOBS=$Jobs" world-digimon-build:alpha03 bash /scripts/build.sh
if ($LASTEXITCODE -ne 0) { throw 'La compilación falló; no se ha desplegado el resultado.' }
Write-Host 'Compilación instalada en el volumen world-digimon-dist. Revisar pruebas antes de desplegar.'
