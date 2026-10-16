param(
    [Parameter(Mandatory=$true)][string]$ClientPath,
    [Parameter(Mandatory=$true)][string]$BackupPath
)
$ErrorActionPreference = 'Stop'
if (Get-Process -Name Wow -ErrorAction SilentlyContinue) {
    throw 'Cierra WoW por completo antes de instalar o actualizar el complemento.'
}
$client = (Resolve-Path -LiteralPath $ClientPath).Path
if (-not (Test-Path -LiteralPath (Join-Path $client 'Wow.exe'))) { throw 'No se encuentra Wow.exe.' }
$source = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\client\WorldDigimon')).Path
$destination = Join-Path $client 'Interface\AddOns\WorldDigimon'
$backup = Join-Path $BackupPath ('WorldDigimon-' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $backup -Force | Out-Null
New-Item -ItemType Directory -Path $destination -Force | Out-Null
foreach ($name in @('WorldDigimon.toc', 'WorldDigimon.lua')) {
    $target = Join-Path $destination $name
    if (Test-Path -LiteralPath $target) { Copy-Item -LiteralPath $target -Destination (Join-Path $backup $name) }
    Copy-Item -LiteralPath (Join-Path $source $name) -Destination $target -Force
}
# The client caches the old custom item spell. Back up and remove only the
# item query cache, with WoW closed. No other cache or client file is touched.
$cacheRoot = Join-Path $client 'Cache\WDB'
if (Test-Path -LiteralPath $cacheRoot) {
    foreach ($locale in Get-ChildItem -LiteralPath $cacheRoot -Directory) {
        $cache = Join-Path $locale.FullName 'itemcache.wdb'
        if (Test-Path -LiteralPath $cache) {
            $resolvedCache = (Resolve-Path -LiteralPath $cache).Path
            if (-not $resolvedCache.StartsWith($client + '\', [StringComparison]::OrdinalIgnoreCase)) {
                throw 'La caché resuelta no pertenece al cliente indicado.'
            }
            Copy-Item -LiteralPath $resolvedCache -Destination (Join-Path $backup ($locale.Name + '-itemcache.wdb'))
            Remove-Item -LiteralPath $resolvedCache
        }
    }
}
Write-Output ('Complemento instalado en: ' + $destination)
Write-Output ('Copia previa en: ' + $backup)
