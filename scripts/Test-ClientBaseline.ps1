param(
    [Parameter(Mandatory = $true)][string]$ClientRoot,
    [Parameter(Mandatory = $true)][string]$ManifestPath
)

$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Resolve-Path -LiteralPath $ClientRoot).Path).TrimEnd('\')
$manifest = Get-Content -LiteralPath $ManifestPath -Raw -Encoding utf8 | ConvertFrom-Json
if ($manifest.schema -ne 1 -or -not $manifest.files) {
    throw 'El manifiesto de cliente no tiene el formato esperado.'
}

$results = foreach ($entry in $manifest.files) {
    $relative = [string]$entry.relativePath
    $full = [IO.Path]::GetFullPath((Join-Path $root $relative))
    if (-not $full.StartsWith($root + '\', [StringComparison]::OrdinalIgnoreCase)) {
        throw "Ruta fuera del cliente: $relative"
    }

    if (-not (Test-Path -LiteralPath $full -PathType Leaf)) {
        [pscustomobject]@{ File = $relative; Status = 'Falta'; Size = $null; Sha256 = $null }
        continue
    }

    $file = Get-Item -LiteralPath $full
    $hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $full).Hash
    $status = if ($file.Length -eq [long]$entry.size -and
                  $hash.Equals([string]$entry.sha256, [StringComparison]::OrdinalIgnoreCase)) {
        'Correcto'
    } else {
        'Diferente'
    }
    [pscustomobject]@{ File = $relative; Status = $status; Size = $file.Length; Sha256 = $hash }
}

$results | Format-Table -AutoSize File, Status, Size
if (@($results | Where-Object Status -ne 'Correcto').Count -gt 0) {
    exit 1
}
