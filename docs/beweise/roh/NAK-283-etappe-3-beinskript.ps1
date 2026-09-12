# NAK-283 Etappe 3 - die Beine der Etappe, einzeln (Bauer, 12.09.2026)
#
# Geaenderte Quellen dieser Etappe (Broker, Rust):
#   broker/src/coordinator/hypothese_verdrahtung.rs   (F02, R-283-1)
#   broker/src/coordinator/mod.rs                     (zwei Testhakenfelder)
#   broker/src/coordinator/zustand.rs                 (ein Zaehlerfeld, Abweichung 1)
#   broker/src/coordinator/invalidierung_verdrahtung.rs (F14)
#   broker/src/transport/server_v3/queues.rs          (F03, R-283-2)
#   broker/tests/sonde013_verdrahtung.rs, broker/tests/sonde014_verdrahtung.rs
#
# Gefahren werden die Beine des Auftrags - A4, A4-SI, A4b, A21, A5, A8, B3c,
# B13 - und JEDES weitere, das eine geaenderte Quelle uebersetzt oder linkt:
#   A31  sonde014_gegenbeispiele (cargo, Coordinator)
#   A22  pruefe_ipc_last.py        (Release eqcop-broker-v3probe: server_v3, queues.rs)
#   A23  pruefe_sonde012_sources_latency.py (Release eqcop-broker-sonde012-probe: Coordinator)
#   A24  pruefe_session_soak.py    (dieselbe Probe, Brokerneustart im Lauf)
#   A29  pruefe_p5_korpus.py       (liest das Ergebnis, das A4 ueber den Produktpfad schreibt)
# Die C++-Beine A4b, B3c und B13 uebersetzen keine geaenderte Quelle; sie laufen,
# weil der Auftrag sie nennt (B13 als Consumerprobe der neuen Reihenfolge).
#
# Dazu der Riegel aus M-25: `git grep -n "zurueck: Vec<(ClientKey, usize)>" broker/src/`
# muss leer sein.
#
# Aufruf aus dem Workspace-Root:
#   pwsh -NoProfile -File docs/beweise/roh/NAK-283-etappe-3-beinskript.ps1
# Rohausgabe: docs/beweise/roh/NAK-283-etappe-3-beine.txt

[CmdletBinding()]
param(
    [string] $Wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path,
    [string] $Ziel   = (Join-Path $PSScriptRoot 'NAK-283-etappe-3-beine.txt')
)

$ErrorActionPreference = 'Stop'
Set-Location $Wurzel

$cmake = (Get-Command cmake -ErrorAction SilentlyContinue).Source
if (-not $cmake) {
    foreach ($k in @(
        'C:\Program Files\CMake\bin\cmake.exe',
        'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe')) {
        if (Test-Path $k) { $cmake = $k; break }
    }
}
if (-not $cmake) { throw 'cmake nicht gefunden' }

$build = Join-Path $Wurzel 'eq-copilot\build'
function Exe-Pfad([string] $ziel) {
    Join-Path $build ("plugin\{0}_artefacts\Release\{0}.exe" -f $ziel)
}

$geaendert = @(
    'broker/src/coordinator/hypothese_verdrahtung.rs',
    'broker/src/coordinator/mod.rs',
    'broker/src/coordinator/zustand.rs',
    'broker/src/coordinator/invalidierung_verdrahtung.rs',
    'broker/src/transport/server_v3/queues.rs',
    'broker/tests/sonde013_verdrahtung.rs',
    'broker/tests/sonde014_verdrahtung.rs')

$zeilen = New-Object System.Collections.Generic.List[string]
function Schreibe([string[]] $neu) { foreach ($z in $neu) { $zeilen.Add($z) } }
function Sichern { [System.IO.File]::WriteAllLines($Ziel, $zeilen, (New-Object System.Text.UTF8Encoding $false)) }

Schreibe @(
    'NAK-283 Etappe 3 - Beine der Etappe, einzeln gefahren',
    "Zeitpunkt: $(Get-Date -Format o)",
    "HEAD: $(git rev-parse HEAD)",
    '',
    '--- git status --short ---')
Schreibe @(git status --short)
Schreibe @('', '--- git diff --numstat (gemessener Arbeitsstand) ---')
Schreibe @(git diff --numstat -- broker docs/beweise/NAK-283.md)
Schreibe @('', '--- geaenderte Quellen: Blob (git hash-object), SHA-256, Zeitstempel ---')
$juengste = [datetime]::MinValue
foreach ($d in $geaendert) {
    $p = Join-Path $Wurzel $d
    $blob = (git hash-object -- $d)
    $sha = (Get-FileHash -Algorithm SHA256 $p).Hash
    $mt = (Get-Item $p).LastWriteTime
    if ($mt -gt $juengste) { $juengste = $mt }
    Schreibe ("{0,-55} blob {1}  sha256 {2}  {3}" -f $d, $blob, $sha, $mt.ToString('o'))
}
Schreibe @("Juengste geaenderte Quelle: $($juengste.ToString('o'))", '')
Sichern

$bilanz = New-Object System.Collections.Generic.List[object]

function Lauf([string] $kuerzel, [string] $datei, [string[]] $argumente, [string] $was) {
    Write-Host ("=== {0} {1} ===" -f $kuerzel, $was) -ForegroundColor Cyan
    $t0 = Get-Date
    $aus = @(& $datei @argumente 2>&1 | ForEach-Object { "$_" })
    $exit = $LASTEXITCODE
    $sek = [int]((Get-Date) - $t0).TotalSeconds
    Write-Host ("    Exit {0} ({1} s)" -f $exit, $sek)
    Schreibe @(("--- {0}  {1}  ->  Exit {2}  ({3} s) ---" -f $kuerzel, $was, $exit, $sek), ("Befehl: {0} {1}" -f $datei, ($argumente -join ' ')))
    Schreibe $aus
    Schreibe ''
    $letzte = @($aus | Where-Object { $_ -match 'test result:|OK|gruen|GRUEN|bestanden|Pruefungen|URTEIL|ROT' } | Select-Object -Last 1)
    $bilanz.Add([pscustomobject]@{ Bein = $kuerzel; Was = $was; Exit = $exit; Sekunden = $sek; Letzte = ($letzte -join '') })
    Sichern
    return $exit
}

# -- Bau --------------------------------------------------------------------
$null = Lauf 'BAU-cargo' 'cargo' @('build', '--release', '--manifest-path', 'broker/Cargo.toml',
    '--bin', 'eqcop-broker-v3probe', '--bin', 'eqcop-broker-sonde012-probe', '--bin', 'eqcop-broker',
    '--color', 'never') 'Release-Broker (wie tools/beweise.ps1 fuer A22/A23/A24)'

$cppZiele = @('EqCopPipeClientTest', 'EqCopSchemaTest', 'EqCopSonde012SourcesModelTest',
    'EqCopIpcTest', 'EqCopIpcLast', 'EqCopSonde012SourcesLatencyTest', 'EqCopSessionSoak')
$bauAus = @(& $cmake @(@('--build', $build, '--config', 'Release', '--target') + $cppZiele) 2>&1 | ForEach-Object { "$_" })
$bauExit = $LASTEXITCODE
$warnungen = @($bauAus | Select-String -Pattern 'warning C' | ForEach-Object { $_.Line.Trim() } | Sort-Object -Unique)
Schreibe @('--- BAU-cmake (Release) ---', "Ziele: $($cppZiele -join ', ')", "Exit: $bauExit", "warning C (eindeutig): $($warnungen.Count)")
foreach ($w in $warnungen) { Schreibe "  $w" }
Schreibe ''
$bilanz.Add([pscustomobject]@{ Bein = 'BAU-cmake'; Was = ($cppZiele -join ' '); Exit = $bauExit; Sekunden = 0; Letzte = "warning C: $($warnungen.Count)" })
if ($bauExit -ne 0) { Schreibe $bauAus; Sichern; throw "cmake-Bau fehlgeschlagen (Exit $bauExit)" }

# -- Frische (Pruefliste D, NAK-230) ------------------------------------------
Schreibe '--- FRISCHE ---'
foreach ($bin in @('eqcop-broker-v3probe', 'eqcop-broker-sonde012-probe', 'eqcop-broker')) {
    $p = Join-Path $Wurzel "broker\target\release\$bin.exe"
    $mt = (Get-Item $p).LastWriteTime
    $urteil = if ($mt -ge $juengste) { 'frisch (juenger als die juengste geaenderte Quelle)' } else { 'VERALTET' }
    Schreibe ("{0,-40} {1}  {2}" -f "$bin.exe", $mt.ToString('o'), $urteil)
    if ($mt -lt $juengste) { Sichern; throw "Release-Binary $bin ist aelter als die geaenderten Quellen" }
}
foreach ($z in $cppZiele) {
    $p = Exe-Pfad $z
    Schreibe ("{0,-40} {1}  (C++: uebersetzt keine geaenderte Quelle dieser Etappe)" -f "$z.exe", (Get-Item $p).LastWriteTime.ToString('o'))
}
Schreibe ''
Sichern

# -- Beine ------------------------------------------------------------------
$null = Lauf 'A4'    'cargo' @('test', '--manifest-path', 'broker/Cargo.toml', '--color', 'never') 'broker'
$null = Lauf 'A29'   'py' @('-3.13', 'tools/eq-copilot/pruefe_p5_korpus.py') 'pruefe_p5_korpus.py (nach A4: frisches Ergebnis)'
$null = Lauf 'A4-SI' 'cargo' @('test', '--manifest-path', 'broker/Cargo.toml', '--color', 'never', '--test', 'store_crash_matrix', '--', '--ignored', '--test-threads=1') 'subscription_server_integration'
$null = Lauf 'A21'   'cargo' @('test', '--manifest-path', 'broker/Cargo.toml', '--test', 'transport_fuzz', '--color', 'never') 'transport_fuzz'
$null = Lauf 'A31'   'cargo' @('test', '--manifest-path', 'broker/Cargo.toml', '--test', 'sonde014_gegenbeispiele', '--color', 'never', '--', '--nocapture', '--test-threads=1') 'sonde014_gegenbeispiele'
$null = Lauf 'A5'    'py' @('-3.13', 'tools/eq-copilot/pruefe_v3_vertrag.py', '--abdeckung') 'pruefe_v3_vertrag.py'
$null = Lauf 'A8'    'py' @('-3.13', 'tools/eq-copilot/erzeuge_v3_fixtures.py', '--pruefen') 'erzeuge_v3_fixtures.py'
$null = Lauf 'A22'   'py' @('-3.13', 'tools/eq-copilot/pruefe_ipc_last.py') 'pruefe_ipc_last.py'
$null = Lauf 'A23'   'py' @('-3.13', 'tools/eq-copilot/pruefe_sonde012_sources_latency.py') 'pruefe_sonde012_sources_latency.py'
$null = Lauf 'A24'   'py' @('-3.13', 'tools/eq-copilot/pruefe_session_soak.py', '--sonden', '16', '--minuten', '2', '--neustarts', '1') 'pruefe_session_soak.py'
$null = Lauf 'A4b'   (Exe-Pfad 'EqCopPipeClientTest') @() 'EqCopPipeClientTest'
$null = Lauf 'B3c'   (Exe-Pfad 'EqCopSchemaTest') @() 'EqCopSchemaTest'
$null = Lauf 'B13'   (Exe-Pfad 'EqCopSonde012SourcesModelTest') @() 'EqCopSonde012SourcesModelTest'

# -- Riegel M-25 --------------------------------------------------------------
$riegel = @(git grep -n 'zurueck: Vec<(ClientKey, usize)>' broker/src/ 2>&1 | ForEach-Object { "$_" })
$riegelExit = $LASTEXITCODE
$riegelUrteil = if ($riegelExit -eq 1 -and $riegel.Count -eq 0) { 'leer (git grep Exit 1)' } else { 'NICHT LEER' }
Schreibe @('--- RIEGEL M-25: git grep -n "zurueck: Vec<(ClientKey, usize)>" broker/src/ ---', "git grep Exit: $riegelExit", "Urteil: $riegelUrteil")
Schreibe $riegel
Schreibe ''
$bilanz.Add([pscustomobject]@{ Bein = 'Riegel M-25'; Was = 'git grep'; Exit = $(if ($riegelUrteil -like 'leer*') { 0 } else { 1 }); Sekunden = 0; Letzte = $riegelUrteil })

# -- Bilanz -----------------------------------------------------------------
Schreibe '--- BILANZ ---'
foreach ($b in $bilanz) {
    Schreibe ("{0,-12} Exit {1,3}  {2,5} s  {3}  |  {4}" -f $b.Bein, $b.Exit, $b.Sekunden, $b.Was, $b.Letzte)
}
$rot = @($bilanz | Where-Object { $_.Exit -ne 0 })
Schreibe ("URTEIL: {0} von {1} Schritten Exit 0" -f ($bilanz.Count - $rot.Count), $bilanz.Count)
Sichern
$bilanz | Format-Table -AutoSize
if ($rot.Count -gt 0) {
    Write-Host "NICHT BESTANDEN: $($rot.Count) Schritt(e) mit Exit != 0" -ForegroundColor Red
    exit 1
}
Write-Host "Alle $($bilanz.Count) Schritte Exit 0" -ForegroundColor Green
exit 0
