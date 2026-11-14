param(
    [ValidateSet('Debug','Release')][string]$Configuration = 'Release',
    # Pasta do repo do site (WolffsRoomSite) para regenerar data/ antes de compilar.
    [string]$Site = ''
)

$ErrorActionPreference = 'Stop'
$rootPath = Split-Path -Parent $PSScriptRoot
$root = $rootPath.Replace('\', '/')

if ($Site) {
    # Precisa de Python com Pillow e PyYAML: pip install pillow pyyaml
    python (Join-Path $rootPath 'tools\gen_data.py') $Site
    if ($LASTEXITCODE -ne 0) { throw "gen_data.py falhou ($LASTEXITCODE)." }
}

# Mesmo container do VitaSeed
docker run --rm -v "${root}:/project" -w /project `
    atamanenko/vitasdk-softfp:latest sh -lc `
    "cmake -S . -B build-vita -DCMAKE_BUILD_TYPE=$Configuration && cmake --build build-vita -j2"

if ($LASTEXITCODE -ne 0) { throw "Build falhou ($LASTEXITCODE)." }

$vpk = Get-Item (Join-Path $rootPath 'artifacts\current\WolffsStore.vpk') -ErrorAction SilentlyContinue
if (-not $vpk) { throw "WolffsStore.vpk não foi gerado em artifacts\current." }
Write-Host "VPK: $($vpk.FullName)"
