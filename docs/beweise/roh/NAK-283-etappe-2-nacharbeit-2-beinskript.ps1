# NAK-283 Etappe 2, Nacharbeit 2 — das betroffene Bein, einzeln und zehnmal
# wiederholt (Bauer, 12.09.2026)
#
# Die Nacharbeit 2 aendert AUSSCHLIESSLICH eine Testdatei
# (`Sonde012ProjectReloadTest.cpp`); der Produktcode bleibt bytegleich (Beleg:
# `git diff --stat <basis>..HEAD -- <die Produktpfade>` ist leer, Manifest §26).
# Gefahren wird deshalb nur das eine Bein, das diese Datei uebersetzt:
#
#   B14  EqCopSonde012ProjectReloadTest   (M-01, M-02, M-06, M-71, M-72 u. a.)
#
# WARUM ZEHN WIEDERHOLUNGEN. Der Defekt dieser Runde war ein Rennen zwischen
# zwei Sperren: der Aufbau verglich den Modellbestand EINMALIG im Fenster
# zwischen `bindungMutex` (State sichtbar) und `SourcesModel::mutex` (Modell
# publiziert). Ein einzelner gruener Lauf beweist gegen so ein Fenster nichts -
# erst eine Wiederholung ohne einen einzigen Ausreisser stuetzt die Zusage von
# §6.1 Punkt 5 ("Erwartet: gruen"). Gemessen wird je Lauf der Zaehlstand, nicht
# nur der Exitcode: eine stille Verschiebung der Pruefzahl waere sonst
# unsichtbar.
#
# Kein Nulltest, kein Passthrough-Bein, kein Kanon, kein Laufzeit-Arm: ohne
# Produktdiff gibt es dort nichts zu messen (Auftrag §BEWEISEN).
#
# Aufruf aus dem Workspace-Root:
#   pwsh -File docs/beweise/roh/NAK-283-etappe-2-nacharbeit-2-beinskript.ps1
# Rohausgabe: docs/beweise/roh/NAK-283-etappe-2-nacharbeit-2-beine.txt

[CmdletBinding()]
param(
    [string] $Wurzel         = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path,
    [string] $Ziel           = (Join-Path $PSScriptRoot 'NAK-283-etappe-2-nacharbeit-2-beine.txt'),
    [int]    $Wiederholungen = 10
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
# ACHTUNG: PowerShell-Variablennamen sind NICHT case-sensitiv - ein `$ziel`
# neben dem Parameter `$Ziel` waere dieselbe Variable und wuerde den
# Ausgabepfad ueberschreiben (die Rohausgabe landete dann als Datei
# `EqCopSonde012ProjectReloadTest` im Workspace-Root). Deshalb `$bein`.
$bein = 'EqCopSonde012ProjectReloadTest'
$exe  = Join-Path $build ("plugin\{0}_artefacts\Release\{0}.exe" -f $bein)

# Die geaenderte Quelle dieser Runde - gegen ihren Zeitstempel wird die Frische
# des Binaries geprueft (Pruefliste D, NAK-230: `Copy-Item` und eine
# zurueckgespielte Quelle koennen aelter sein als ihr Objektfile; MSBuild baut
# dann NICHT neu und meldet trotzdem Exit 0).
$quelle = Join-Path $Wurzel 'eq-copilot\plugin\tests\Sonde012ProjectReloadTest.cpp'

# Eine echte Neuuebersetzung erzwingen: ein rein inkrementeller Bau uebersetzt
# nichts, und "null warning C" waere dann keine Messung, sondern die
# Abwesenheit einer Messung (Muster der Nacharbeit 1, §23.6).
(Get-Item $quelle).LastWriteTime = Get-Date
$juengsteQuelle = (Get-Item $quelle).LastWriteTime

$zeilen = @(
    'NAK-283 Etappe 2, Nacharbeit 2 - Bein B14, einzeln und zehnmal wiederholt',
    "Zeitpunkt: $(Get-Date -Format o)",
    "Geaenderte Quelle: $($quelle.Replace($Wurzel + '\', ''))  ($juengsteQuelle)",
    '')

# -- Bauen ----------------------------------------------------------------
Write-Host "Baue $bein ..." -ForegroundColor Cyan
$bauArgs = @('--build', $build, '--config', 'Release', '--target', $bein)
$bauAus  = & $cmake @bauArgs 2>&1
$bauExit = $LASTEXITCODE
$warnungen = @($bauAus | Select-String -Pattern 'warning C' | ForEach-Object { $_.Line.Trim() } | Sort-Object -Unique)
$zeilen += @(
    '--- BAU (Release) ---',
    "Ziel:    $bein",
    "Exit:    $bauExit",
    "warning C (eindeutig): $($warnungen.Count)")
foreach ($w in $warnungen) { $zeilen += "  $w" }
$zeilen += ''
if ($bauExit -ne 0) {
    $zeilen += $bauAus
    [System.IO.File]::WriteAllLines($Ziel, $zeilen, (New-Object System.Text.UTF8Encoding $false))
    throw "Bau fehlgeschlagen (Exit $bauExit)"
}

# -- Frische des Binaries gegen die Quelle (Pruefliste D) -----------------
$stand = (Get-Item $exe).LastWriteTime
$frisch = $stand -ge $juengsteQuelle
$zeilen += @(
    '--- FRISCHE DES BINARIES ---',
    ("{0,-36} {1}  {2}" -f $bein, $stand.ToString('o'), $(if ($frisch) { 'frisch' } else { 'VERALTET' })),
    '')
if (-not $frisch) {
    [System.IO.File]::WriteAllLines($Ziel, $zeilen, (New-Object System.Text.UTF8Encoding $false))
    throw "Binary $bein ist aelter als seine Quelle - MSBuild hat nicht neu gebaut"
}

# -- Lauf 1: einzeln, mit voller Rohausgabe -------------------------------
function Zaehlstand([string] $text) {
    $t = ($text -split "`n" | Select-String -Pattern 'SONDE-012 ProjectReload:' | Select-Object -Last 1)
    if ($t) { return $t.Line.Trim() }
    return '(kein Zaehlstand in der Ausgabe)'
}

Write-Host "  B14 $bein (Lauf 1, volle Ausgabe) ..." -NoNewline
$aus1  = (& $exe 2>&1) -join "`n"
$exit1 = $LASTEXITCODE
Write-Host " Exit $exit1"
$zeilen += @(
    "--- B14  $bein  (Lauf 1 von $($Wiederholungen + 1), volle Ausgabe)  ->  Exit $exit1 ---",
    $aus1,
    '')

# -- Laeufe 2..N+1: Wiederholungen, Zaehlstand und etwaige FEHLER-Zeilen --
$laeufe = @([pscustomobject]@{ Nr = 1; Exit = $exit1; Stand = (Zaehlstand $aus1) })
$zeilen += "--- B14 WIEDERHOLUNGEN (Lauf 2 bis $($Wiederholungen + 1)) ---"
for ($i = 2; $i -le ($Wiederholungen + 1); $i++) {
    Write-Host "  B14 Lauf $i/$($Wiederholungen + 1) ..." -NoNewline
    $aus  = (& $exe 2>&1) -join "`n"
    $exit = $LASTEXITCODE
    $stand = Zaehlstand $aus
    Write-Host " Exit $exit  ($stand)"
    $laeufe += [pscustomobject]@{ Nr = $i; Exit = $exit; Stand = $stand }
    $zeilen += ("Lauf {0,2}: Exit {1}  |  {2}" -f $i, $exit, $stand)
    # Ein roter Lauf wird nie stillschweigend zusammengefasst: seine
    # FEHLER-Zeilen stehen roh in der Datei.
    $fehlerZeilen = @($aus -split "`n" | Select-String -Pattern 'FEHLER' | ForEach-Object { $_.Line.TrimEnd() })
    foreach ($f in $fehlerZeilen) { $zeilen += "         $f" }
}
$zeilen += ''

# -- Bilanz ---------------------------------------------------------------
$zeilen += '--- BILANZ ---'
foreach ($l in $laeufe) { $zeilen += ("Lauf {0,2}: Exit {1}  |  {2}" -f $l.Nr, $l.Exit, $l.Stand) }
$rot     = @($laeufe | Where-Object { $_.Exit -ne 0 })
$staende = @($laeufe | ForEach-Object { $_.Stand } | Sort-Object -Unique)
$zeilen += @(
    '',
    ("Verschiedene Zaehlstaende: {0} ({1})" -f $staende.Count, ($staende -join ' | ')),
    ("URTEIL: {0} von {1} Laeufen Exit 0; warning C (eindeutig): {2}" -f ($laeufe.Count - $rot.Count), $laeufe.Count, $warnungen.Count))

[System.IO.File]::WriteAllLines($Ziel, $zeilen, (New-Object System.Text.UTF8Encoding $false))
Write-Host ''
$laeufe | Format-Table -AutoSize
if ($rot.Count -gt 0) {
    Write-Host "NICHT BESTANDEN: $($rot.Count) Lauf/Laeufe mit Exit != 0" -ForegroundColor Red
    exit 1
}
if ($staende.Count -ne 1) {
    Write-Host "NICHT BESTANDEN: $($staende.Count) verschiedene Zaehlstaende - die Pruefzahl schwankt" -ForegroundColor Red
    exit 1
}
Write-Host "Alle $($laeufe.Count) Laeufe Exit 0, ein einziger Zaehlstand: $($staende[0])" -ForegroundColor Green
exit 0
