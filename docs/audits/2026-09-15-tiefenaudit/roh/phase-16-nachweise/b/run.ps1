param(
    [string]$SourceRoot = 'C:\na-audit16-aff2d818',
    [string]$BuildRoot = 'C:\na-audit16-aff2d818\build-b',
    [ValidateSet('components','wire')][string]$Mode = 'components'
)
$ErrorActionPreference = 'Stop'
$harness = Join-Path $BuildRoot 'harness'
New-Item -ItemType Directory -Path $harness -Force | Out-Null
$broker = (Join-Path $SourceRoot 'broker').Replace('\','/')
$manifest = @"
[package]
name = "audit16-b"
version = "0.0.0"
edition = "2021"
[dependencies]
eqcop-broker = { path = "$broker" }
flatbuffers = "25.12.19"
serde_json = "1"
rusqlite = { version = "=0.40.2", default-features = false, features = ["bundled"] }
windows-sys = { version = "0.60", features = ["Win32_Foundation", "Win32_Storage_FileSystem", "Win32_System_Pipes", "Win32_System_IO", "Win32_Security"] }
[[bin]]
name = "audit16-b"
path = "main.rs"
"@
[IO.File]::WriteAllText((Join-Path $harness 'Cargo.toml'), $manifest)
Copy-Item -LiteralPath (Join-Path $SourceRoot 'broker\Cargo.lock') -Destination (Join-Path $harness 'Cargo.lock') -Force
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'components.rs') -Destination (Join-Path $harness 'components.rs') -Force
if ($Mode -eq 'wire') {
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'wire.rs') -Destination (Join-Path $harness 'wire.rs') -Force
    $main = 'mod components; mod wire; fn main() { wire::run(); }'
} else { $main = 'mod components; fn main() { components::run(); }' }
[IO.File]::WriteAllText((Join-Path $harness 'main.rs'), $main)
$env:CARGO_TARGET_DIR = Join-Path $BuildRoot 'target'
$env:AUDIT16_DATA = Join-Path $BuildRoot ('data-' + [guid]::NewGuid().ToString('N'))
$env:AUDIT16_SOURCE = $SourceRoot
$log = Join-Path $PSScriptRoot ($Mode + '-run.txt')
& cargo run --offline --manifest-path (Join-Path $harness 'Cargo.toml') 2>&1 | Tee-Object -FilePath $log
$code = $LASTEXITCODE
Add-Content -LiteralPath $log -Value ('EXIT=' + $code)
exit $code
