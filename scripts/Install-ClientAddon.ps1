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
foreach ($file in Get-ChildItem -LiteralPath $source -Recurse -File) {
    $relative = $file.FullName.Substring($source.Length).TrimStart('\')
    $target = Join-Path $destination $relative
    $targetDir = Split-Path -Parent $target
    New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
    if (Test-Path -LiteralPath $target) {
        $backupFile = Join-Path $backup $relative
        New-Item -ItemType Directory -Path (Split-Path -Parent $backupFile) -Force | Out-Null
        Copy-Item -LiteralPath $target -Destination $backupFile -Force
    }
    Copy-Item -LiteralPath $file.FullName -Destination $target -Force
}
Write-Output ('Complemento instalado en: ' + $destination)
Write-Output ('Copia previa de archivos reemplazados en: ' + $backup)
