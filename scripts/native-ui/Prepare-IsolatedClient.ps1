param(
    [Parameter(Mandatory = $true)][string]$SourceClient,
    [Parameter(Mandatory = $true)][string]$StagingRoot,
    [Parameter(Mandatory = $true)][string]$PatchedExe,
    [Parameter(Mandatory = $true)][string]$ProbeMpq
)

$ErrorActionPreference = 'Stop'
$source = (Resolve-Path -LiteralPath $SourceClient).Path
$exe = (Resolve-Path -LiteralPath $PatchedExe).Path
$mpq = (Resolve-Path -LiteralPath $ProbeMpq).Path
$dest = [IO.Path]::GetFullPath($StagingRoot)
if (Test-Path -LiteralPath $dest) { throw 'El directorio de prueba ya existe; no se sobrescribe.' }
if ((Get-FileHash -LiteralPath (Join-Path $source 'Wow.exe') -Algorithm SHA256).Hash -ne 'BF644876709C591ACC17C0DA8CDF1814EDCC9F1E6BC109A8C0D5C38C79DC953C') { throw 'El cliente original ha cambiado.' }
if ((Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash -ne '3F783E6F9EC1D7631C70FB29AABEFA20277ABD79349751327D1C8E0DF0A5C583') { throw 'El ejecutable de prueba ha cambiado.' }
if ((Get-FileHash -LiteralPath $mpq -Algorithm SHA256).Hash -ne '15D43B9C5CB744B86539D08861543A398D9FF251C4BE351A8F76D795BB2EA3F7') { throw 'El MPQ de prueba ha cambiado.' }
$requiredBytes = 23GB
if ([IO.DriveInfo]::new([IO.Path]::GetPathRoot($dest)).AvailableFreeSpace -lt $requiredBytes) { throw 'Espacio insuficiente para una copia aislada completa.' }

New-Item -ItemType Directory -Path $dest | Out-Null
try {
    Copy-Item -LiteralPath (Join-Path $source 'Data') -Destination (Join-Path $dest 'Data') -Recurse
    foreach ($name in @('Battle.net.dll', 'dbghelp.dll', 'DivxDecoder.dll', 'ijl15.dll', 'msvcr80.dll', 'Scan.dll', 'unicows.dll')) {
        Copy-Item -LiteralPath (Join-Path $source $name) -Destination (Join-Path $dest $name)
    }
    Copy-Item -LiteralPath $exe -Destination (Join-Path $dest 'Wow.exe')
    $wtf = Join-Path $source 'WTF\Config.wtf'
    if (Test-Path -LiteralPath $wtf) {
        New-Item -ItemType Directory -Path (Join-Path $dest 'WTF') | Out-Null
        Copy-Item -LiteralPath $wtf -Destination (Join-Path $dest 'WTF\Config.wtf')
    }
    Copy-Item -LiteralPath $mpq -Destination (Join-Path $dest 'Data\esES\patch-esES-b.MPQ')
    $sourceFiles = Get-ChildItem -LiteralPath (Join-Path $source 'Data') -Recurse -File
    foreach ($item in $sourceFiles) {
        $relative = $item.FullName.Substring((Join-Path $source 'Data').Length).TrimStart('\')
        $copy = Join-Path (Join-Path $dest 'Data') $relative
        if (-not (Test-Path -LiteralPath $copy) -or (Get-Item -LiteralPath $copy).Length -ne $item.Length) { throw "Copia incompleta: $relative" }
    }
    Write-Output "Cliente aislado preparado: $dest"
    Write-Output "Archivos Data comprobados por tamaño: $($sourceFiles.Count)"
    Write-Output 'No se han copiado los AddOns; el ensayo comprobará la carga por parche.'
} catch {
    Write-Warning "La copia quedó incompleta en $dest. Revise antes de repetir; no se elimina automáticamente."
    throw
}
