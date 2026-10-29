param(
    [Parameter(Mandatory = $true)][string]$SourceExe,
    [Parameter(Mandatory = $true)][string]$OutputExe
)

$ErrorActionPreference = 'Stop'
$expectedSha = 'BF644876709C591ACC17C0DA8CDF1814EDCC9F1E6BC109A8C0D5C38C79DC953C'
$source = (Resolve-Path -LiteralPath $SourceExe).Path
$output = [IO.Path]::GetFullPath($OutputExe)
if ($source.Equals($output, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'La salida debe ser una copia distinta del ejecutable original.'
}
if ((Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash -ne $expectedSha) {
    throw 'Este Wow.exe no coincide con la versión local auditada. No se aplica ningún cambio.'
}
if (Test-Path -LiteralPath $output) {
    throw 'La copia de salida ya existe. No se sobrescribe.'
}

# Offsets de la modificación FrameXML para 3.3.5a/12340; referencia:
# https://github.com/suprsokr/allow-custom-gluexml-mithril/blob/main/binary-patches/allow-custom-gluexml.json
# Cada entrada exige que los bytes originales de ESTA copia coincidan antes de modificarla.
$edits = @(
    @{ Offset = 0x1f41bf; Before = '74';         After = 'eb' },
    @{ Offset = 0x415a25; Before = '75';         After = 'eb' },
    @{ Offset = 0x415a3f; Before = '01';         After = '03' },
    @{ Offset = 0x415a95; Before = '01';         After = '03' },
    @{ Offset = 0x415b46; Before = '7f';         After = 'eb' },
    @{ Offset = 0x415b5f; Before = '83c0';       After = 'b803' },
    @{ Offset = 0x415b61; Before = '035e8be55d'; After = '000000ebed' }
)

$bytes = [IO.File]::ReadAllBytes($source)
foreach ($edit in $edits) {
    $before = [Convert]::FromHexString($edit.Before)
    $after = [Convert]::FromHexString($edit.After)
    if ($before.Length -ne $after.Length -or $edit.Offset + $before.Length -gt $bytes.Length) {
        throw "Cambio fuera de rango en $($edit.Offset)."
    }
    for ($i = 0; $i -lt $before.Length; $i++) {
        if ($bytes[$edit.Offset + $i] -ne $before[$i]) {
            throw "Bytes inesperados en $($edit.Offset). No se crea la copia."
        }
    }
    [Array]::Copy($after, 0, $bytes, $edit.Offset, $after.Length)
}

[IO.File]::WriteAllBytes($output, $bytes)
$hash = (Get-FileHash -LiteralPath $output -Algorithm SHA256).Hash
Write-Output "Copia de ensayo creada: $output"
Write-Output "SHA-256: $hash"
