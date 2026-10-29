param(
    [Parameter(Mandatory = $true)][string]$TestClient,
    [Parameter(Mandatory = $true)][ValidateSet('Probe', 'Native', 'NativeSV')][string]$Mode,
    [Parameter(Mandatory = $true)][string]$ProbeMpq,
    [Parameter(Mandatory = $true)][string]$NativeMpq,
    [string]$NativeSVMpq
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path -LiteralPath $TestClient).Path
$originalClient = 'G:\World of Warcraft 3.3.5a\World of Warcraft 3.3.5a'
if ($root.TrimEnd('\').Equals($originalClient, [StringComparison]::OrdinalIgnoreCase)) { throw 'Solo se permite cambiar la copia aislada, no el cliente principal.' }
$exe = Join-Path $root 'Wow.exe'
if ((Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash -ne '3F783E6F9EC1D7631C70FB29AABEFA20277ABD79349751327D1C8E0DF0A5C583') { throw 'La copia no contiene el ejecutable de ensayo esperado.' }
if (Test-Path -LiteralPath (Join-Path $root 'Interface\AddOns\WorldDigimon')) { throw 'La copia aislada no debe contener el addon WorldDigimon.' }
$live = Get-Process -Name Wow -ErrorAction SilentlyContinue | Where-Object { $_.Path -and $_.Path.Equals($exe, [StringComparison]::OrdinalIgnoreCase) }
if ($live) { throw 'Cierra el cliente de ensayo antes de cambiar el parche.' }

$target = Join-Path $root 'Data\esES\patch-esES-b.MPQ'
$known = @(
    '15D43B9C5CB744B86539D08861543A398D9FF251C4BE351A8F76D795BB2EA3F7',
    'E0659E8BFC76DA6C1479A1A3CC610CEA8561B2E4DADC9D886B4DEBD898E0F7D9',
    'E7BC8787B12DC78AE75DBF5D3C3A0874D72B898598F2F500D8CF7B7A9A0D66BA'
)
if ((Test-Path -LiteralPath $target) -and ((Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash -notin $known)) { throw 'El parche de destino no coincide con ningún ensayo conocido.' }
$path = if ($Mode -eq 'Probe') { $ProbeMpq } elseif ($Mode -eq 'Native') { $NativeMpq } else { $NativeSVMpq }
if (-not $path) { throw 'Falta la ruta del parche seleccionado.' }
$source = (Resolve-Path -LiteralPath $path).Path
$expected = if ($Mode -eq 'Probe') { $known[0] } elseif ($Mode -eq 'Native') { $known[1] } else { $known[2] }
if ((Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash -ne $expected) { throw 'El parche de origen ha cambiado.' }

$temporary = Join-Path (Split-Path -Parent $target) 'patch-esES-b.pending.MPQ'
if (Test-Path -LiteralPath $temporary) { throw 'Hay un parche pendiente anterior; revísalo antes de continuar.' }
Copy-Item -LiteralPath $source -Destination $temporary
if ((Get-FileHash -LiteralPath $temporary -Algorithm SHA256).Hash -ne $expected) { throw 'La copia del parche no coincide.' }
if (Test-Path -LiteralPath $target) {
    $backup = Join-Path (Split-Path -Parent $target) ("patch-esES-b.before-$Mode-" + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.MPQ')
    [IO.File]::Replace($temporary, $target, $backup)
    Write-Output "Copia anterior: $backup"
} else {
    Move-Item -LiteralPath $temporary -Destination $target
}
Write-Output "Parche activo en la copia aislada: $Mode"
Write-Output "SHA-256: $expected"
