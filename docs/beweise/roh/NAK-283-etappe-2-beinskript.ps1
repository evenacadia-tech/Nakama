# NAK-283 Etappe 2 — die Beine der Etappe, einzeln gefahren (Bauer, 12.09.2026)
#
# Kein voller Kanon (der gehoert an das Ende der Etappe 6), kein Laufzeit-Arm.
# Gefahren wird, was eine geaenderte Quelle uebersetzt oder linkt:
#
#   aus Bauplan §6.3:  B14 B13 B27 B2 A12 B10 A1 A16 B8
#   zusaetzlich:       B1 B4 B29 — sie messen den Assistentenpfad
#                      (`assistentStarten`/`Weiter`/`Abbrechen`) und damit die
#                      geaenderte Signatur von `schrittAendern`; §6.3 nennt sie
#                      nicht. Als Abweichung im Manifest gefuehrt.
#   Bau ohne Lauf:     EqCopilot_VST3 (neue `warning C`?) und EqCopSessionSoak
#                      (bindet SourcesModel.cpp; sein Lauf ist ein Soak und
#                      gehoert in den Kanon, nicht in diese Etappe).
#
# Aufruf aus dem Workspace-Root:
#   pwsh -File docs/beweise/roh/NAK-283-etappe-2-beinskript.ps1
# Rohausgabe: docs/beweise/roh/NAK-283-etappe-2-beine.txt

[CmdletBinding()]
param(
    [string] $Wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path,
    [string] $Ziel   = (Join-Path $PSScriptRoot 'NAK-283-etappe-2-beine.txt')
)

$ErrorActionPreference = 'Stop'

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

# Die geaenderten Produktquellen dieser Etappe - gegen ihre Zeitstempel wird
# die Frische der Binaries geprueft (Pruefliste D, NAK-230).
$quellen = @(
    'eq-copilot\plugin\src\SourcesModel.h',
    'eq-copilot\plugin\src\SourcesModel.cpp',
    'eq-copilot\plugin\src\PluginProcessor.h',
    'eq-copilot\plugin\src\prozessor\Ipc.cpp',
    'eq-copilot\plugin\src\prozessor\State.cpp',
    'eq-copilot\plugin\state\NakamaState.cpp',
    'eq-copilot\plugin\tests\Sonde012ProjectReloadTest.cpp',
    'eq-copilot\plugin\tests\Sonde012SourcesModelTest.cpp',
    'eq-copilot\plugin\tests\Sonde014IntentTest.cpp',
    'eq-copilot\plugin\tests\StateMigrationTestMain.cpp'
) | ForEach-Object { Join-Path $Wurzel $_ }

$juengsteQuelle = ($quellen | ForEach-Object { (Get-Item $_).LastWriteTime } | Measure-Object -Maximum).Maximum

$beine = @(
    [pscustomobject]@{ Kuerzel='A1';  Ziel='EqCopNullTest';                    Art='plugin'; Arg=@() }
    [pscustomobject]@{ Kuerzel='A16'; Ziel='EqCopProbeeqNullTest';             Art='plugin'; Arg=@() }
    [pscustomobject]@{ Kuerzel='B1';  Ziel='EqCopIdentityTest';                Art='plugin'; Arg=@() }
    [pscustomobject]@{ Kuerzel='B2';  Ziel='EqCopStateMigrationTest';          Art='plugin'; Arg=@() }
    [pscustomobject]@{ Kuerzel='B4';  Ziel='EqCopQueueStressTest';             Art='plugin'; Arg=@() }
    [pscustomobject]@{ Kuerzel='B8';  Ziel='EqCopLebenslaufTest';              Art='plugin'; Arg=@() }
    [pscustomobject]@{ Kuerzel='B10'; Ziel='EqCopIpcTest';                     Art='plugin'; Arg=@() }
    [pscustomobject]@{ Kuerzel='B13'; Ziel='EqCopSonde012SourcesModelTest';    Art='plugin'; Arg=@() }
    [pscustomobject]@{ Kuerzel='B14'; Ziel='EqCopSonde012ProjectReloadTest';   Art='plugin'; Arg=@() }
    [pscustomobject]@{ Kuerzel='B27'; Ziel='EqCopSonde014IntentTest';          Art='plugin'; Arg=@() }
    [pscustomobject]@{ Kuerzel='B29'; Ziel='EqCopSonde014AssistentTest';       Art='plugin'; Arg=@() }
    [pscustomobject]@{ Kuerzel='A12'; Ziel='erzeuge_state_fixtures.py';        Art='python'; Arg=@('--pruefen') }
)

$nurBau = @('EqCopilot_VST3', 'EqCopSessionSoak')

$zeilen = @(
    'NAK-283 Etappe 2 - Beine der Etappe, einzeln gefahren',
    "Zeitpunkt: $(Get-Date -Format o)",
    "Juengste geaenderte Quelle: $juengsteQuelle",
    '')

# ── Bauen ─────────────────────────────────────────────────────────────────
$bauZiele = @($beine | Where-Object { $_.Art -eq 'plugin' } | ForEach-Object { $_.Ziel }) + $nurBau
Write-Host "Baue $($bauZiele.Count) Ziele ..." -ForegroundColor Cyan
$bauArgs = @('--build', $build, '--config', 'Release', '--target') + $bauZiele
$bauAus = & $cmake @bauArgs 2>&1
$bauExit = $LASTEXITCODE
$warnungen = @($bauAus | Select-String -Pattern 'warning C' | ForEach-Object { $_.Line.Trim() } | Sort-Object -Unique)
$zeilen += @(
    "--- BAU (Release) ---",
    "Ziele:   $($bauZiele -join ' ')",
    "Exit:    $bauExit",
    "warning C (eindeutig): $($warnungen.Count)")
foreach ($w in $warnungen) { $zeilen += "  $w" }
$zeilen += ''
if ($bauExit -ne 0) {
    $zeilen += $bauAus
    [System.IO.File]::WriteAllLines($Ziel, $zeilen, (New-Object System.Text.UTF8Encoding $false))
    throw "Bau fehlgeschlagen (Exit $bauExit)"
}

# ── Frische der Binaries gegen die Quellen (Pruefliste D) ─────────────────
$zeilen += '--- FRISCHE DER BINARIES ---'
$veraltet = @()
foreach ($b in ($beine | Where-Object { $_.Art -eq 'plugin' })) {
    $exe = Join-Path $build ("plugin\{0}_artefacts\Release\{0}.exe" -f $b.Ziel)
    $stand = (Get-Item $exe).LastWriteTime
    $ok = $stand -ge $juengsteQuelle
    if (-not $ok) { $veraltet += $b.Kuerzel }
    $zeilen += ("{0,-5} {1,-36} {2}  {3}" -f $b.Kuerzel, $b.Ziel, $stand.ToString('o'), $(if ($ok) { 'frisch' } else { 'VERALTET' }))
}
$zeilen += ''
if ($veraltet.Count -gt 0) {
    [System.IO.File]::WriteAllLines($Ziel, $zeilen, (New-Object System.Text.UTF8Encoding $false))
    throw "Binaries aelter als ihre Quellen: $($veraltet -join ' ')"
}

# ── Fahren ────────────────────────────────────────────────────────────────
$bilanz = @()
foreach ($b in $beine) {
    Write-Host "  $($b.Kuerzel) $($b.Ziel) ..." -NoNewline
    if ($b.Art -eq 'plugin') {
        $exe = Join-Path $build ("plugin\{0}_artefacts\Release\{0}.exe" -f $b.Ziel)
        $aus = & $exe @($b.Arg) 2>&1
    }
    else {
        $skript = Join-Path $Wurzel ('tools\eq-copilot\' + $b.Ziel)
        $aus = & py -3.13 $skript @($b.Arg) 2>&1
    }
    $exit = $LASTEXITCODE
    Write-Host " Exit $exit"
    $bilanz += [pscustomobject]@{ Kuerzel = $b.Kuerzel; Ziel = $b.Ziel; Exit = $exit }
    $zeilen += @(
        "--- $($b.Kuerzel)  $($b.Ziel) $($b.Arg -join ' ')  ->  Exit $exit ---",
        ($aus -join "`n"),
        '')
}

$zeilen += '--- BILANZ ---'
foreach ($z in $bilanz) { $zeilen += ("{0,-5} {1,-36} Exit {2}" -f $z.Kuerzel, $z.Ziel, $z.Exit) }
$rot = @($bilanz | Where-Object { $_.Exit -ne 0 })
$zeilen += ''
$zeilen += ("URTEIL: {0} von {1} Beinen Exit 0; neue warning C: {2}" -f ($bilanz.Count - $rot.Count), $bilanz.Count, $warnungen.Count)

[System.IO.File]::WriteAllLines($Ziel, $zeilen, (New-Object System.Text.UTF8Encoding $false))
Write-Host ''
$bilanz | Format-Table -AutoSize
if ($rot.Count -gt 0) {
    Write-Host "NICHT BESTANDEN: $($rot.Count) Bein(e) mit Exit != 0" -ForegroundColor Red
    exit 1
}
Write-Host "Alle $($bilanz.Count) Beine Exit 0" -ForegroundColor Green
exit 0
