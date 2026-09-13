# NAK-283 Etappe 4 — die Beine der Etappe, einzeln gefahren (Bauer, 13.09.2026)
#
# Kein voller Kanon (Ende der Etappe 6), kein Laufzeit-Arm (Dirigentensache).
# Gefahren werden die Beine aus Auftrag und Bauplan §6.5 und jedes weitere
# Kanon-Bein, dessen Pruefbinary eine geaenderte Quelle dieser Etappe
# uebersetzt oder linkt. Welche das sind, sagt MSBuild selbst statt einer
# Include-Vermutung: CL.read.*.tlog nennt jede Datei, die cl.exe fuer ein Ziel
# geoeffnet hat, link.read.*.tlog jede Bibliothek, gegen die es gelinkt wurde;
# eine Bibliothek aus dem Baum (NakamaKern.lib mit DspKern.cpp) bringt die
# CL.read ihres eigenen Ziels mit. Das Skript liest die Kanonzeilen aus
# tools/beweise.ps1 (PowerShell-AST) und bricht ab, wenn ein betroffenes
# Kanon-Bein hier fehlt oder mit anderem Ziel oder anderen Argumenten stuende.
#
#   Auftrag, §6.5:     A16 A1 B7 B6 A3 B4 B5 B21 A2 - A2 liest und linkt keine
#                      geaenderte Quelle und laeuft als Gegenprobe. A15 ist
#                      STILLGELEGT (EqCopSunaNullTest seit 28.08.2026) - weder
#                      gebaut noch gefahren, nicht gezaehlt.
#   laut Trackinglog:  A4b B1 B2 B3c B8 B10 B11 B12 B13 B14 B15 B16 B17 B18
#                      B19 B20 B22 B23 B24 B25 B26 B27 B28 B29
#   Python-Beine, deren Skript ein Artefakt aus eq-copilot/build/plugin faehrt
#   oder liest:        A14 (baut und liest NakamaKern.lib, liest die Gen-VST3),
#                      A17 (EqCopIpcTest --phase-b-verify-binary),
#                      A22 (EqCopIpcLast), A23 (EqCopSonde012SourcesLatencyTest),
#                      A24 (EqCopSessionSoak)
#   nicht gefahren:    B3 B3b B9 - ihr Ziel liest und linkt keine geaenderte
#                      Quelle (bei jedem Lauf gemessen); die uebrigen Python-
#                      und alle Cargo-Beine beruehren kein Artefakt aus
#                      eq-copilot/build/plugin (ebenfalls gemessen).
#   Bau ohne eigenes Bein: EqCopilot_VST3 und NakamaProbeeq_VST3 (Warnungen;
#                      A14 liest die Gen-VST3).
#
# Aufruf aus dem Workspace-Root:
#   pwsh -File docs/beweise/roh/NAK-283-etappe-4-beinskript.ps1
# Rohausgabe: docs/beweise/roh/NAK-283-etappe-4-beine.txt

[CmdletBinding()]
param(
    [string] $Wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path,
    [string] $Ziel   = (Join-Path $PSScriptRoot 'NAK-283-etappe-4-beine.txt')
)

$ErrorActionPreference = 'Stop'
Set-Location $Wurzel
$utf8 = New-Object System.Text.UTF8Encoding $false

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
$fixtureVerzeichnis = Join-Path $Wurzel 'eq-copilot\fixtures'
$basis = '0f0bc1edbd0554eede8976f05a3eb3f4c376e19c'

# Die geaenderten Quellen dieser Etappe.
$quellen = @(
    'eq-copilot\plugin\sonde\SondeProcessor.h',
    'eq-copilot\plugin\sonde\SondeProcessor.cpp',
    'eq-copilot\plugin\dsp\DspKern.h',
    'eq-copilot\plugin\dsp\DspKern.cpp',
    'eq-copilot\plugin\src\HoerMarkierung.h',
    'eq-copilot\plugin\core\StampedAudioQueue.h',
    'eq-copilot\plugin\tests\SondeNullTestMain.cpp',
    'eq-copilot\plugin\tests\TransactionTestMain.cpp',
    'eq-copilot\plugin\tests\DspGoldenTestMain.cpp',
    'eq-copilot\plugin\tests\MarkierungTestMain.cpp'
) | ForEach-Object { Join-Path $Wurzel $_ }

function Bein([string] $Kuerzel, [string] $Name, [string] $Art, [string] $Grund,
              [object[]] $Arg = @(), [string[]] $Artefakte = @()) {
    if ($Art -eq 'plugin') { $Artefakte = @($Name) }
    [pscustomobject]@{ Kuerzel = $Kuerzel; Ziel = $Name; Art = $Art; Grund = $Grund; Arg = $Arg; Artefakte = $Artefakte }
}

$beine = @(
    (Bein 'A16' 'EqCopProbeeqNullTest'                'plugin' 'Auftrag')
    (Bein 'A1'  'EqCopNullTest'                       'plugin' 'Auftrag')
    (Bein 'B7'  'EqCopTransactionTest'                'plugin' 'Auftrag')
    (Bein 'B6'  'EqCopDspGoldenTest'                  'plugin' 'Auftrag')
    (Bein 'A3'  'EqCopMarkierungTest'                 'plugin' 'Auftrag')
    (Bein 'B4'  'EqCopQueueStressTest'                'plugin' 'Auftrag')
    (Bein 'B5'  'EqCopAnalysisGoldenTest'             'plugin' 'Auftrag')
    (Bein 'B21' 'EqCopSonde013InterventionRingTest'   'plugin' 'Auftrag')
    (Bein 'A2'  'EqCopGoldenTest'                     'plugin' 'Auftrag' @($fixtureVerzeichnis))
    (Bein 'A4b' 'EqCopPipeClientTest'                 'plugin' 'Trackinglog')
    (Bein 'B1'  'EqCopIdentityTest'                   'plugin' 'Trackinglog')
    (Bein 'B2'  'EqCopStateMigrationTest'             'plugin' 'Trackinglog')
    (Bein 'B3c' 'EqCopSchemaTest'                     'plugin' 'Trackinglog')
    (Bein 'B8'  'EqCopLebenslaufTest'                 'plugin' 'Trackinglog')
    (Bein 'B10' 'EqCopIpcTest'                        'plugin' 'Trackinglog')
    (Bein 'B11' 'EqCopSonde012HostChannelContextTest' 'plugin' 'Trackinglog')
    (Bein 'B12' 'EqCopSonde012LoudnessSourceTest'     'plugin' 'Trackinglog')
    (Bein 'B13' 'EqCopSonde012SourcesModelTest'       'plugin' 'Trackinglog')
    (Bein 'B14' 'EqCopSonde012ProjectReloadTest'      'plugin' 'Trackinglog')
    (Bein 'B15' 'EqCopShot'                           'plugin' 'Trackinglog' @('--sonde012-suite', 'eq-copilot/build/sonde012-shots'))
    (Bein 'B16' 'EqCopSonde013EventWireTest'          'plugin' 'Trackinglog')
    (Bein 'B17' 'EqCopSonde013TruePeakGoldenTest'     'plugin' 'Trackinglog')
    (Bein 'B18' 'EqCopSonde013DynamicsTest'           'plugin' 'Trackinglog')
    (Bein 'B19' 'EqCopSonde013StereoGoldenTest'       'plugin' 'Trackinglog')
    (Bein 'B20' 'EqCopSonde013QualityClassTest'       'plugin' 'Trackinglog')
    (Bein 'B22' 'EqCopSonde013FingerprintGoldenTest'  'plugin' 'Trackinglog')
    (Bein 'B23' 'EqCopSonde013PassageStateTest'       'plugin' 'Trackinglog')
    (Bein 'B24' 'EqCopSonde013PrePostGoldenTest'      'plugin' 'Trackinglog')
    (Bein 'B25' 'EqCopSonde013ExperimentGoldenTest'   'plugin' 'Trackinglog')
    (Bein 'B26' 'EqCopSonde013ExperimentUiTest'       'plugin' 'Trackinglog')
    (Bein 'B27' 'EqCopSonde014IntentTest'             'plugin' 'Trackinglog')
    (Bein 'B28' 'EqCopSonde014BefundTest'             'plugin' 'Trackinglog')
    (Bein 'B29' 'EqCopSonde014AssistentTest'          'plugin' 'Trackinglog')
    (Bein 'A14' 'pruefe_kern_identitaetsfrei.py'      'python' 'Artefakt' @() @('NakamaKern', 'EqCopilot_VST3'))
    (Bein 'A17' 'pruefe_installer_manifest.py'        'python' 'Artefakt' @() @('EqCopIpcTest'))
    (Bein 'A22' 'pruefe_ipc_last.py'                  'python' 'Artefakt' @() @('EqCopIpcLast'))
    (Bein 'A23' 'pruefe_sonde012_sources_latency.py'  'python' 'Artefakt' @() @('EqCopSonde012SourcesLatencyTest'))
    (Bein 'A24' 'pruefe_session_soak.py'              'python' 'Artefakt' @('--sonden', '16', '--minuten', '2', '--neustarts', '1') @('EqCopSessionSoak'))
)

# ── Kanon lesen (tools/beweise.ps1, PowerShell-AST) ───────────────────────
$parseFehler = $null
$ast = [System.Management.Automation.Language.Parser]::ParseFile((Join-Path $Wurzel 'tools\beweise.ps1'), [ref] $null, [ref] $parseFehler)
if ($parseFehler) { throw "tools/beweise.ps1 nicht lesbar: $($parseFehler[0].Message)" }
$kanon = @(foreach ($h in $ast.FindAll({ param($n) $n -is [System.Management.Automation.Language.HashtableAst] }, $true)) {
    $felder = @{}
    foreach ($paar in $h.KeyValuePairs) { $felder[$paar.Item1.Extent.Text.Trim("'")] = $paar.Item2.Extent.Text.Trim() }
    if ($felder.ContainsKey('Kuerzel') -and $felder.ContainsKey('Art')) {
        [pscustomobject]@{
            Kuerzel     = $felder['Kuerzel'].Trim("'")
            Name        = $felder['Name'].Trim("'")
            Art         = $felder['Art'].Trim("'")
            Argumente   = $felder['Argumente']
            Ordner      = $(if ($felder.ContainsKey('Ordner')) { $felder['Ordner'].Trim("'") } else { 'tools\eq-copilot' })
            Stillgelegt = $felder.ContainsKey('Stillgelegt')
        }
    }
})
if ($kanon.Count -eq 0) { throw 'keine Kanonzeile in tools/beweise.ps1 gefunden' }
$kanonPlugin = @($kanon | Where-Object { $_.Art -eq 'plugin' -and -not $_.Stillgelegt })

$zeilen = @(
    'NAK-283 Etappe 4 - Beine der Etappe, einzeln gefahren',
    "Zeitpunkt: $(Get-Date -Format o)",
    "HEAD: $(git -C $Wurzel rev-parse HEAD)",
    "Basis: $basis",
    'A15 EqCopSunaNullTest: STILLGELEGT seit 2026-08-28 (tools/beweise.ps1) - weder gebaut noch gefahren, nicht gezaehlt.',
    '')

# ── Quellen neu stempeln ──────────────────────────────────────────────────
# Jede Uebersetzungseinheit, die eine geaenderte Quelle liest, wird in DIESEM
# Bau uebersetzt - sonst saehe die Warnungspruefung nur, was seit dem letzten
# Bau neu war (etwa nach einem Rotbeweis, der nur sein eigenes Ziel baut).
# Nur der Zeitstempel wechselt; die Bytes bleiben (Hash vorher = nachher).
$hashVorher = @($quellen | ForEach-Object { (Get-FileHash -LiteralPath $_ -Algorithm SHA256).Hash })
foreach ($datei in $quellen) { (Get-Item -LiteralPath $datei).LastWriteTime = Get-Date }
$hashNachher = @($quellen | ForEach-Object { (Get-FileHash -LiteralPath $_ -Algorithm SHA256).Hash })
if (($hashVorher -join ',') -ne ($hashNachher -join ',')) { throw 'Quellbytes haben sich beim Stempeln veraendert' }
$zeilen += @("Die zehn geaenderten Quellen sind vor dem Bau neu gestempelt ($(Get-Date -Format o)); SHA-256 vorher = nachher.", '')

# ── Bauen ─────────────────────────────────────────────────────────────────
# Jedes nicht stillgelegte Plugin-Ziel des Kanons wird gebaut, auch das nicht
# gefahrene: erst dann beschreibt sein Trackinglog den heutigen Stand.
$bauZiele = @(@($kanonPlugin | ForEach-Object { $_.Name }) + @($beine | ForEach-Object { $_.Artefakte }) +
              @('EqCopilot_VST3', 'NakamaProbeeq_VST3') | Sort-Object -Unique)
Write-Host "Baue $($bauZiele.Count) Ziele ..." -ForegroundColor Cyan
$bauArgs = @('--build', $build, '--config', 'Release', '--target') + $bauZiele
$ErrorActionPreference = 'Continue'
$bauAus = & $cmake @bauArgs 2>&1
$bauExit = $LASTEXITCODE
$ErrorActionPreference = 'Stop'
$warnungen = @($bauAus | Select-String -Pattern 'warning C' | ForEach-Object { $_.Line.Trim() } | Sort-Object -Unique)

# "Keine neue Warnung" wird an der ZEILE gemessen, nicht am Dateinamen: eine
# Warnung zaehlt als neu, wenn sie auf einer Zeile steht, die diese Etappe
# gegenueber dem Basis-SHA hinzugefuegt oder geaendert hat (git diff -U0).
# Vorbestehende Warnungen in einer geaenderten Datei (etwa C4456 in
# DspGoldenTestMain.cpp) bleiben sichtbar gelistet, zaehlen aber nicht als neu.
$neueZeilen = @{}
foreach ($datei in $quellen) {
    $rel = $datei.Replace($Wurzel + '\', '').Replace('\', '/')
    $menge = New-Object 'System.Collections.Generic.HashSet[int]'
    foreach ($z in @(& git -C $Wurzel diff -U0 $basis -- $rel)) {
        if ($z -match '^@@ -\d+(?:,\d+)? \+(\d+)(?:,(\d+))? @@') {
            $start = [int] $Matches[1]
            $anzahl = if ($Matches[2]) { [int] $Matches[2] } else { 1 }
            for ($i = 0; $i -lt $anzahl; $i++) { [void] $menge.Add($start + $i) }
        }
    }
    $neueZeilen[$rel.ToLowerInvariant()] = $menge
}
$warnungenAusGeaendert = @($warnungen | Where-Object {
    if ($_ -notmatch '^(?<datei>[A-Za-z]:\\[^(]+)\((?<zeile>\d+)') { return $false }
    $rel = $Matches['datei'].Replace($Wurzel + '\', '').Replace('\', '/').ToLowerInvariant()
    $neueZeilen.ContainsKey($rel) -and $neueZeilen[$rel].Contains([int] $Matches['zeile'])
})
$zeilen += @(
    '--- BAU (Release) ---',
    "Ziele:   $($bauZiele -join ' ')",
    "Exit:    $bauExit",
    "warning C (eindeutig): $($warnungen.Count); davon auf einer Zeile, die diese Etappe hinzugefuegt oder geaendert hat: $($warnungenAusGeaendert.Count)")
foreach ($w in $warnungen) { $zeilen += "  $w" }
foreach ($w in $warnungenAusGeaendert) { $zeilen += "  NEU: $w" }
$zeilen += ''
if ($bauExit -ne 0) {
    $zeilen += $bauAus
    [System.IO.File]::WriteAllLines($Ziel, $zeilen, $utf8)
    throw "Bau fehlgeschlagen (Exit $bauExit)"
}

# ── Abhaengigkeiten aus den MSBuild-Trackinglogs ──────────────────────────
function Lies-Tlog([string] $pfad) {
    # UTF-16 LE; '^' leitet die Eingabezeile ein, '|' trennt mehrere Eingaben
    foreach ($zeile in [System.IO.File]::ReadAllLines($pfad, [System.Text.Encoding]::Unicode)) {
        foreach ($teil in $zeile.TrimStart('^').Split('|')) { if ($teil) { $teil } }
    }
}

$bibliotheken = @{}   # .lib aus dem Baum -> Trackinglog-Ordner ihres Ziels
foreach ($w in @(Get-ChildItem -LiteralPath (Join-Path $build 'plugin') -Recurse -File -Filter 'Lib-link.write.*.tlog')) {
    foreach ($e in (Lies-Tlog $w.FullName)) { if ($e -match '\.LIB$') { $bibliotheken[$e] = $w.DirectoryName } }
}

$ordnerDaten = @{}
function Ordner-Daten([string] $ordner) {
    if ($ordnerDaten.ContainsKey($ordner)) { return $ordnerDaten[$ordner] }
    $gelesen = New-Object 'System.Collections.Generic.HashSet[string]' ([StringComparer]::OrdinalIgnoreCase)
    $cl = @(Get-ChildItem -LiteralPath $ordner -File -Filter 'CL.*read.*.tlog')
    if ($cl.Count -eq 0) { throw "Trackinglog ohne CL.read: $ordner" }
    foreach ($f in $cl) { foreach ($e in (Lies-Tlog $f.FullName)) { [void] $gelesen.Add($e) } }
    $libs = @()
    foreach ($f in @(Get-ChildItem -LiteralPath $ordner -File -Filter 'link.*read.*.tlog')) {
        foreach ($e in (Lies-Tlog $f.FullName)) { if ($bibliotheken.ContainsKey($e)) { $libs += $bibliotheken[$e] } }
    }
    $ausgaben = @(Get-ChildItem -LiteralPath $ordner -File | Where-Object { $_.Name -match '^(link|Lib-link)\.write\.\d+\.tlog$' } |
                  ForEach-Object { Lies-Tlog $_.FullName } | Where-Object { $_ -match '\.(EXE|VST3|LIB)$' } | Sort-Object -Unique)
    $daten = [pscustomobject]@{ Gelesen = $gelesen; Bibliotheken = @($libs | Sort-Object -Unique); Ausgaben = $ausgaben }
    $ordnerDaten[$ordner] = $daten
    return $daten
}

function Abhaengigkeit([string] $zielName) {
    $start = @(Get-ChildItem -LiteralPath (Join-Path $build "plugin\$zielName.dir\Release") -Directory -Filter '*.tlog' -ErrorAction SilentlyContinue)
    if ($start.Count -ne 1) { throw "Trackinglog von $zielName nicht eindeutig ($($start.Count) Ordner)" }
    $eigen = Ordner-Daten $start[0].FullName
    if ($eigen.Ausgaben.Count -ne 1) { throw "Artefakt von $zielName nicht eindeutig: $($eigen.Ausgaben -join ', ')" }
    $treffer = New-Object 'System.Collections.Generic.HashSet[string]' ([StringComparer]::OrdinalIgnoreCase)
    $besucht = New-Object 'System.Collections.Generic.HashSet[string]' ([StringComparer]::OrdinalIgnoreCase)
    $offen = [System.Collections.Generic.Queue[string]]::new()
    $offen.Enqueue($start[0].FullName)
    while ($offen.Count -gt 0) {
        $o = $offen.Dequeue()
        if (-not $besucht.Add($o)) { continue }
        $d = Ordner-Daten $o
        foreach ($datei in $quellen) { if ($d.Gelesen.Contains($datei)) { [void] $treffer.Add($datei) } }
        foreach ($l in $d.Bibliotheken) { $offen.Enqueue($l) }
    }
    [pscustomobject]@{ Artefakt = $eigen.Ausgaben[0]; Quellen = @($quellen | Where-Object { $treffer.Contains($_) }) }
}

$abh = @{}
foreach ($z in $bauZiele) { $abh[$z] = Abhaengigkeit $z }
function Kurz($liste) { if (@($liste).Count -eq 0) { '-' } else { (@($liste) | ForEach-Object { Split-Path $_ -Leaf }) -join ',' } }

# ── Auswahl gegen den Kanon ───────────────────────────────────────────────
$zeilen += '--- AUSWAHL (Kanon aus tools/beweise.ps1 gegen die Trackinglogs) ---'
$fehler = @()
foreach ($k in $kanonPlugin) {
    $a = $abh[$k.Name]
    $bein = @($beine | Where-Object { $_.Kuerzel -eq $k.Kuerzel })
    if ($bein.Count -gt 0) {
        $b = $bein[0]
        $status = "gefahren ($($b.Grund))"
        if ($b.Ziel -ne $k.Name) { $fehler += "$($k.Kuerzel): Ziel $($b.Ziel) statt $($k.Name)" }
        $erwartet = if ($b.Kuerzel -eq 'A2') { '@($fixtureVerzeichnis)' } else { '@(' + ((@($b.Arg) | ForEach-Object { "'$_'" }) -join ', ') + ')' }
        if (($k.Argumente -replace '\s', '') -ne ($erwartet -replace '\s', '')) { $fehler += "$($k.Kuerzel): Argumente $erwartet statt $($k.Argumente)" }
        $exe = Join-Path $build ("plugin\{0}_artefacts\Release\{0}.exe" -f $b.Ziel)
        if (-not [string]::Equals($exe, $a.Artefakt, [StringComparison]::OrdinalIgnoreCase)) { $fehler += "$($k.Kuerzel): gefahren wird $exe, gelinkt wurde $($a.Artefakt)" }
    }
    elseif ($a.Quellen.Count -gt 0) {
        $status = 'FEHLT'
        $fehler += "$($k.Kuerzel) $($k.Name) liest oder linkt $(Kurz $a.Quellen) und wird nicht gefahren"
    }
    else { $status = 'nicht gefahren - liest und linkt keine geaenderte Quelle' }
    $zeilen += ('{0,-5} {1,-38} {2,-110} {3}' -f $k.Kuerzel, $k.Name, (Kurz $a.Quellen), $status)
}
foreach ($k in @($kanon | Where-Object { $_.Art -ne 'plugin' -and -not $_.Stillgelegt })) {
    $bein = @($beine | Where-Object { $_.Kuerzel -eq $k.Kuerzel })
    $beruehrt = $false
    if ($k.Art -eq 'python') {
        $skript = Join-Path $Wurzel ($k.Ordner + '\' + $k.Name)
        if (-not (Test-Path -LiteralPath $skript)) { $fehler += "$($k.Kuerzel): Skript $skript fehlt" }
        else { $beruehrt = [System.IO.File]::ReadAllText($skript) -match 'build.{0,8}plugin|_artefacts|NakamaKern\.lib' }
    }
    if ($bein.Count -gt 0) {
        $status = "gefahren ($($bein[0].Grund): $($bein[0].Artefakte -join ', '))"
        if ($bein[0].Ziel -ne $k.Name) { $fehler += "$($k.Kuerzel): Skript $($bein[0].Ziel) statt $($k.Name)" }
        $erwartet = '@(' + ((@($bein[0].Arg) | ForEach-Object { "'$_'" }) -join ', ') + ')'
        if (($k.Argumente -replace '\s', '') -ne ($erwartet -replace '\s', '')) { $fehler += "$($k.Kuerzel): Argumente $erwartet statt $($k.Argumente)" }
    }
    elseif ($beruehrt) {
        $status = 'FEHLT'
        $fehler += "$($k.Kuerzel) $($k.Name) beruehrt eq-copilot/build/plugin und wird nicht gefahren"
    }
    else { $status = "nicht gefahren - $($k.Art), beruehrt kein Artefakt aus eq-copilot/build/plugin" }
    $zeilen += ('{0,-5} {1,-38} {2}' -f $k.Kuerzel, $k.Name, $status)
}
foreach ($b in $beine) {
    if (-not ($kanon | Where-Object { $_.Kuerzel -eq $b.Kuerzel -and $_.Name -eq $b.Ziel -and -not $_.Stillgelegt })) {
        $fehler += "$($b.Kuerzel) $($b.Ziel) steht so nicht als aktives Kanon-Bein in tools/beweise.ps1"
    }
}
$zeilen += ''
if ($fehler.Count -gt 0) {
    $zeilen += @('AUSWAHL FEHLERHAFT:') + @($fehler | ForEach-Object { "  $_" })
    [System.IO.File]::WriteAllLines($Ziel, $zeilen, $utf8)
    throw "Beinauswahl weicht vom Kanon ab: $($fehler -join ' | ')"
}

# ── Frische der Artefakte (Pruefliste D, NAK-230) ─────────────────────────
# Je Artefakt gegen GENAU die geaenderten Quellen, die sein Ziel liest oder
# linkt - nicht gegen die juengste Quelle ueberhaupt: ein Rotbeweis stempelt
# die zurueckgespielte Quelle neu (NAK-230), und ein Ziel, das sie nicht
# liest, baut MSBuild zu Recht nicht neu.
$zeilen += '--- FRISCHE DER ARTEFAKTE ---'
$veraltet = @()
foreach ($z in @($beine | ForEach-Object { $_.Artefakte } | Sort-Object -Unique)) {
    $a = $abh[$z]
    $stand = (Get-Item -LiteralPath $a.Artefakt).LastWriteTime
    if ($a.Quellen.Count -eq 0) {
        $urteil = 'liest und linkt keine geaenderte Quelle (Gegenprobe)'
        $bezug = '-'
    }
    else {
        $juengste = $a.Quellen | ForEach-Object { Get-Item -LiteralPath $_ } | Sort-Object LastWriteTime -Descending | Select-Object -First 1
        $bezug = "$($juengste.Name) $($juengste.LastWriteTime.ToString('o'))"
        $urteil = if ($stand -ge $juengste.LastWriteTime) { 'frisch' } else { 'VERALTET' }
        if ($urteil -eq 'VERALTET') { $veraltet += $z }
    }
    $zeilen += ('{0,-36} {1}  juengste Abhaengigkeit: {2}  {3}' -f $z, $stand.ToString('o'), $bezug, $urteil)
}
$zeilen += ''
if ($veraltet.Count -gt 0) {
    [System.IO.File]::WriteAllLines($Ziel, $zeilen, $utf8)
    throw "Artefakte aelter als ihre Quellen: $($veraltet -join ' ')"
}

# ── Fahren ────────────────────────────────────────────────────────────────
$bilanz = @()
foreach ($b in $beine) {
    Write-Host "  $($b.Kuerzel) $($b.Ziel) ..." -NoNewline
    $uhr = [Diagnostics.Stopwatch]::StartNew()
    $ErrorActionPreference = 'Continue'
    if ($b.Art -eq 'plugin') {
        $exe = Join-Path $build ("plugin\{0}_artefacts\Release\{0}.exe" -f $b.Ziel)
        $aus = & $exe @($b.Arg) 2>&1
    }
    else {
        $skript = Join-Path $Wurzel ('tools\eq-copilot\' + $b.Ziel)
        $aus = & py -3.13 $skript @($b.Arg) 2>&1
    }
    $exit = $LASTEXITCODE
    $ErrorActionPreference = 'Stop'
    $uhr.Stop()
    $sekunden = [math]::Round($uhr.Elapsed.TotalSeconds, 1)
    Write-Host " Exit $exit ($sekunden s)"
    $bilanz += [pscustomobject]@{ Kuerzel = $b.Kuerzel; Ziel = $b.Ziel; Grund = $b.Grund; Exit = $exit; Sekunden = $sekunden }
    $zeilen += @(
        "--- $($b.Kuerzel)  $($b.Ziel) $($b.Arg -join ' ')  ->  Exit $exit ($sekunden s) ---",
        (@($aus | ForEach-Object { "$_" }) -join "`n"),
        '')
}

$zeilen += '--- BILANZ ---'
foreach ($z in $bilanz) { $zeilen += ("{0,-5} {1,-38} {2,-12} Exit {3}  ({4} s)" -f $z.Kuerzel, $z.Ziel, $z.Grund, $z.Exit, $z.Sekunden) }
$rot = @($bilanz | Where-Object { $_.Exit -ne 0 })
$zeilen += ''
$zeilen += ("URTEIL: {0} von {1} Beinen Exit 0; warning C auf von dieser Etappe geaenderten Zeilen: {2}" -f ($bilanz.Count - $rot.Count), $bilanz.Count, $warnungenAusGeaendert.Count)

[System.IO.File]::WriteAllLines($Ziel, $zeilen, $utf8)
Write-Host ''
$bilanz | Format-Table -AutoSize
if ($rot.Count -gt 0 -or $warnungenAusGeaendert.Count -gt 0) {
    Write-Host "NICHT BESTANDEN: $($rot.Count) Bein(e) mit Exit != 0, $($warnungenAusGeaendert.Count) Warnung(en) auf geaenderten Zeilen" -ForegroundColor Red
    exit 1
}
Write-Host "Alle $($bilanz.Count) Beine Exit 0, keine Warnung auf einer geaenderten Zeile" -ForegroundColor Green
exit 0
