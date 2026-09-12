# NAK-283 Etappe 2, Nacharbeit 1 — die betroffenen Beine, einzeln gefahren
# (Bauer, 12.09.2026)
#
# Die Nacharbeit aendert AUSSCHLIESSLICH zwei Testdateien; der Produktcode der
# Etappe 2 bleibt bytegleich (Beleg: `git diff --stat <basis>..HEAD -- <die
# Produktpfade>` ist leer, Manifest §23). Gefahren werden deshalb nur die zwei
# Beine, die diese Dateien uebersetzen:
#
#   B14  EqCopSonde012ProjectReloadTest   (M-01, M-02, M-06, M-72)
#   B27  EqCopSonde014IntentTest          (M-08/M-09 am Host, M-12)
#
# Kein Nulltest, kein Passthrough-Bein, kein Kanon, kein Laufzeit-Arm: ohne
# Produktdiff gibt es dort nichts zu messen (Auftrag §BEWEISEN).
#
# Aufruf aus dem Workspace-Root:
#   pwsh -File docs/beweise/roh/NAK-283-etappe-2-nacharbeit-1-beinskript.ps1
# Rohausgabe: docs/beweise/roh/NAK-283-etappe-2-nacharbeit-1-beine.txt

[CmdletBinding()]
param(
    [string] $Wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path,
    [string] $Ziel   = (Join-Path $PSScriptRoot 'NAK-283-etappe-2-nacharbeit-1-beine.txt')
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

# Die geaenderten Quellen dieser Runde - gegen ihre Zeitstempel wird die
# Frische der Binaries geprueft (Pruefliste D, NAK-230).
$quellen = @(
    'eq-copilot\plugin\tests\Sonde012ProjectReloadTest.cpp',
    'eq-copilot\plugin\tests\Sonde014IntentTest.cpp'
) | ForEach-Object { Join-Path $Wurzel $_ }

$juengsteQuelle = ($quellen | ForEach-Object { (Get-Item $_).LastWriteTime } | Measure-Object -Maximum).Maximum

$beine = @(
    [pscustomobject]@{ Kuerzel='B14'; Ziel='EqCopSonde012ProjectReloadTest'; Arg=@() }
    [pscustomobject]@{ Kuerzel='B27'; Ziel='EqCopSonde014IntentTest';        Arg=@() }
)

$zeilen = @(
    'NAK-283 Etappe 2, Nacharbeit 1 - Beine der Runde, einzeln gefahren',
    "Zeitpunkt: $(Get-Date -Format o)",
    "Juengste geaenderte Quelle: $juengsteQuelle",
    '')

# -- Bauen ----------------------------------------------------------------
$bauZiele = @($beine | ForEach-Object { $_.Ziel })
Write-Host "Baue $($bauZiele.Count) Ziele ..." -ForegroundColor Cyan
$bauArgs = @('--build', $build, '--config', 'Release', '--target') + $bauZiele
$bauAus = & $cmake @bauArgs 2>&1
$bauExit = $LASTEXITCODE
$warnungen = @($bauAus | Select-String -Pattern 'warning C' | ForEach-Object { $_.Line.Trim() } | Sort-Object -Unique)
$zeilen += @(
    '--- BAU (Release) ---',
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

# -- Frische der Binaries gegen die Quellen (Pruefliste D) ----------------
$zeilen += '--- FRISCHE DER BINARIES ---'
$veraltet = @()
foreach ($b in $beine) {
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

# -- Fahren ---------------------------------------------------------------
$bilanz = @()
foreach ($b in $beine) {
    Write-Host "  $($b.Kuerzel) $($b.Ziel) ..." -NoNewline
    $exe = Join-Path $build ("plugin\{0}_artefacts\Release\{0}.exe" -f $b.Ziel)
    $aus = & $exe @($b.Arg) 2>&1
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
$zeilen += ("URTEIL: {0} von {1} Beinen Exit 0; warning C (eindeutig): {2}" -f ($bilanz.Count - $rot.Count), $bilanz.Count, $warnungen.Count)

[System.IO.File]::WriteAllLines($Ziel, $zeilen, (New-Object System.Text.UTF8Encoding $false))
Write-Host ''
$bilanz | Format-Table -AutoSize
if ($rot.Count -gt 0) {
    Write-Host "NICHT BESTANDEN: $($rot.Count) Bein(e) mit Exit != 0" -ForegroundColor Red
    exit 1
}
Write-Host "Alle $($bilanz.Count) Beine Exit 0" -ForegroundColor Green
exit 0
