# NAK-283 Etappe 5 — die Beine der Etappe, einzeln gefahren (Bauer, 13.09.2026)
#
# Kein voller Kanon (Dirigentensache nach dieser Etappe, §6.8), kein
# Laufzeit-Arm. Gefahren werden die Beine aus Auftrag und Bauplan §6.6 und
# JEDES weitere Kanon-Bein, dessen Pruefbinary eine geaenderte Quelle dieser
# Etappe uebersetzt oder linkt. Welche das sind, sagt MSBuild selbst statt
# einer Include-Vermutung: CL.read.*.tlog nennt jede Datei, die cl.exe fuer ein
# Ziel geoeffnet hat, link.read.*.tlog jede Bibliothek, gegen die es gelinkt
# wurde; eine Bibliothek aus dem Baum bringt die CL.read ihres eigenen Ziels
# mit. Anders als in Etappe 4 steht die Trackinglog-Auswahl nicht als Liste im
# Skript, sondern wird aus den Logs GERECHNET: Auftrag ∪ {Plugin-Beine, deren
# Ziel eine geaenderte Quelle liest oder linkt} ∪ {Python-Beine, deren
# Artefakt das tut}. Das Skript liest die Kanonzeilen aus tools/beweise.ps1
# (PowerShell-AST) und faehrt jedes Bein mit genau den Argumenten des Kanons.
#
#   Auftrag, §6.6:   B16 B18 B17 B19 B20 B22 B23 B24 B25 B9 B5 A10 B3c A26,
#                    dazu A16 und A1 als Wache fuer den Audiopfad.
#   Python mit Artefakt aus eq-copilot/build/plugin (Muster Etappe 4):
#                    A14 NakamaKern + EqCopilot_VST3, A17 EqCopIpcTest,
#                    A22 EqCopIpcLast, A23 EqCopSonde012SourcesLatencyTest,
#                    A24 EqCopSessionSoak - gefahren, wenn ihr Artefakt eine
#                    geaenderte Quelle liest oder linkt.
#   Bau ohne eigenes Bein: EqCopilot_VST3 und NakamaProbeeq_VST3 (Warnungen).
#   A15 ist STILLGELEGT - weder gebaut noch gefahren, nicht gezaehlt.
#
# Aufruf aus dem Workspace-Root:
#   pwsh -File docs/beweise/roh/NAK-283-etappe-5-beinskript.ps1
# Rohausgabe: docs/beweise/roh/NAK-283-etappe-5-beine.txt

[CmdletBinding()]
param(
    [string] $Wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path,
    [string] $Ziel   = (Join-Path $PSScriptRoot 'NAK-283-etappe-5-beine.txt')
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
$basis = 'bc0941e51bcdf8d3752362c3ca10cda628bd1438'

# Die geaenderten Quellen dieser Etappe.
$quellen = @(
    'eq-copilot\plugin\core\analysis\FeatureEngine.h',
    'eq-copilot\plugin\core\analysis\featureengine\Spektrum.h',
    'eq-copilot\plugin\core\analysis\featureengine\Lautheit.h',
    'eq-copilot\plugin\core\analysis\featureengine\Frame.h',
    'eq-copilot\plugin\core\analysis\featureengine\Vertrag.h',
    'eq-copilot\plugin\tests\Sonde013EventWireTest.cpp',
    'eq-copilot\plugin\tests\Sonde013DynamicsTest.cpp'
) | ForEach-Object { Join-Path $Wurzel $_ }

# Auftrag: Kuerzel -> Grund. Die Argumente kommen aus dem Kanon.
$auftrag = [ordered]@{
    'A16' = 'Auftrag (Wache Audiopfad)'; 'A1' = 'Auftrag (Wache Audiopfad)'
    'B16' = 'Auftrag'; 'B18' = 'Auftrag'; 'B17' = 'Auftrag'; 'B19' = 'Auftrag'; 'B20' = 'Auftrag'
    'B22' = 'Auftrag'; 'B23' = 'Auftrag'; 'B24' = 'Auftrag'; 'B25' = 'Auftrag'; 'B9' = 'Auftrag'
    'B5' = 'Auftrag'; 'B3c' = 'Auftrag'; 'A10' = 'Auftrag'; 'A26' = 'Auftrag'
}
# Python-Beine, deren Skript ein Artefakt aus eq-copilot/build/plugin faehrt oder liest.
$pythonArtefakte = @{
    'A14' = @('NakamaKern', 'EqCopilot_VST3')
    'A17' = @('EqCopIpcTest')
    'A22' = @('EqCopIpcLast')
    'A23' = @('EqCopSonde012SourcesLatencyTest')
    'A24' = @('EqCopSessionSoak')
    # A34 faehrt dasselbe Skript mit `--selbsttest` (NAK-283 F13, ohne Lauf);
    # sein Text nennt das Soak-Artefakt, deshalb dieselbe Zuordnung.
    'A34' = @('EqCopSessionSoak')
}

# ── Kanon lesen (tools/beweise.ps1, PowerShell-AST) ───────────────────────
$parseFehler = $null
$ast = [System.Management.Automation.Language.Parser]::ParseFile((Join-Path $Wurzel 'tools\beweise.ps1'), [ref] $null, [ref] $parseFehler)
if ($parseFehler) { throw "tools/beweise.ps1 nicht lesbar: $($parseFehler[0].Message)" }
$kanon = @(foreach ($h in $ast.FindAll({ param($n) $n -is [System.Management.Automation.Language.HashtableAst] }, $true)) {
    $felder = @{}
    foreach ($paar in $h.KeyValuePairs) { $felder[$paar.Item1.Extent.Text.Trim("'")] = $paar.Item2 }
    if ($felder.ContainsKey('Kuerzel') -and $felder.ContainsKey('Art')) {
        $argAst = $felder['Argumente']
        $argWerte = @()
        if ($argAst) {
            foreach ($el in $argAst.FindAll({ param($n) $n -is [System.Management.Automation.Language.StringConstantExpressionAst] -or $n -is [System.Management.Automation.Language.VariableExpressionAst] }, $true)) {
                if ($el -is [System.Management.Automation.Language.VariableExpressionAst]) {
                    if ($el.VariablePath.UserPath -eq 'fixtureVerzeichnis') { $argWerte += $fixtureVerzeichnis }
                    else { throw "Kanon-Argument mit unbekannter Variable: $($el.Extent.Text)" }
                } else { $argWerte += $el.Value }
            }
        }
        [pscustomobject]@{
            Kuerzel     = $felder['Kuerzel'].Extent.Text.Trim("'")
            Name        = $felder['Name'].Extent.Text.Trim("'")
            Art         = $felder['Art'].Extent.Text.Trim("'")
            Argumente   = @($argWerte)
            ArgText     = $(if ($argAst) { $argAst.Extent.Text } else { '@()' })
            Ordner      = $(if ($felder.ContainsKey('Ordner')) { $felder['Ordner'].Extent.Text.Trim("'") } else { 'tools\eq-copilot' })
            Stillgelegt = $felder.ContainsKey('Stillgelegt')
        }
    }
})
if ($kanon.Count -eq 0) { throw 'keine Kanonzeile in tools/beweise.ps1 gefunden' }
$kanonPlugin = @($kanon | Where-Object { $_.Art -eq 'plugin' -and -not $_.Stillgelegt })
foreach ($k in @($auftrag.Keys)) {
    if (-not ($kanon | Where-Object { $_.Kuerzel -eq $k -and -not $_.Stillgelegt })) { throw "Auftragsbein $k steht nicht aktiv im Kanon" }
}

$zeilen = @(
    'NAK-283 Etappe 5 - Beine der Etappe, einzeln gefahren',
    "Zeitpunkt: $(Get-Date -Format o)",
    "HEAD: $(git -C $Wurzel rev-parse HEAD)",
    "Basis: $basis",
    'A15 EqCopSunaNullTest: STILLGELEGT seit 2026-08-28 (tools/beweise.ps1) - weder gebaut noch gefahren, nicht gezaehlt.',
    '')

# ── Quellen neu stempeln ──────────────────────────────────────────────────
# Jede Uebersetzungseinheit, die eine geaenderte Quelle liest, wird in DIESEM
# Bau uebersetzt - sonst saehe die Warnungspruefung nur, was seit dem letzten
# Bau neu war. Nur der Zeitstempel wechselt; die Bytes bleiben.
$hashVorher = @($quellen | ForEach-Object { (Get-FileHash -LiteralPath $_ -Algorithm SHA256).Hash })
foreach ($datei in $quellen) { (Get-Item -LiteralPath $datei).LastWriteTime = Get-Date }
$hashNachher = @($quellen | ForEach-Object { (Get-FileHash -LiteralPath $_ -Algorithm SHA256).Hash })
if (($hashVorher -join ',') -ne ($hashNachher -join ',')) { throw 'Quellbytes haben sich beim Stempeln veraendert' }
$zeilen += "Die $($quellen.Count) geaenderten Quellen sind vor dem Bau neu gestempelt ($(Get-Date -Format o)); SHA-256 vorher = nachher:"
for ($i = 0; $i -lt $quellen.Count; $i++) { $zeilen += "  $($hashNachher[$i])  $($quellen[$i].Replace($Wurzel + '\', '').Replace('\', '/'))" }
$zeilen += ''

# ── Bauen ─────────────────────────────────────────────────────────────────
$bauZiele = @(@($kanonPlugin | ForEach-Object { $_.Name }) + @($pythonArtefakte.Values | ForEach-Object { $_ }) +
              @('EqCopilot_VST3', 'NakamaProbeeq_VST3') | Sort-Object -Unique)
Write-Host "Baue $($bauZiele.Count) Ziele ..." -ForegroundColor Cyan
$bauArgs = @('--build', $build, '--config', 'Release', '--target') + $bauZiele
$ErrorActionPreference = 'Continue'
$bauUhr = [Diagnostics.Stopwatch]::StartNew()
$bauAus = & $cmake @bauArgs 2>&1
$bauExit = $LASTEXITCODE
$bauUhr.Stop()
$ErrorActionPreference = 'Stop'
$warnungen = @($bauAus | Select-String -Pattern 'warning C' | ForEach-Object { $_.Line.Trim() } | Sort-Object -Unique)

# "Keine neue Warnung" wird an der ZEILE gemessen: eine Warnung zaehlt als neu,
# wenn sie auf einer Zeile steht, die diese Etappe gegenueber dem Basis-SHA
# hinzugefuegt oder geaendert hat (git diff -U0).
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
$warnungsStellen = @($warnungen | ForEach-Object {
    if ($_ -match '^(?<datei>[A-Za-z]:\\[^(]+)\((?<zeile>\d+)(?:,\d+)?\): (?<text>warning C\d+)') {
        "$($Matches['datei'].Replace($Wurzel + '\', '').Replace('\', '/')):$($Matches['zeile']) $($Matches['text'])"
    }
} | Sort-Object -Unique)
$warnungenAusGeaendert = @($warnungen | Where-Object {
    if ($_ -notmatch '^(?<datei>[A-Za-z]:\\[^(]+)\((?<zeile>\d+)') { return $false }
    $rel = $Matches['datei'].Replace($Wurzel + '\', '').Replace('\', '/').ToLowerInvariant()
    $neueZeilen.ContainsKey($rel) -and $neueZeilen[$rel].Contains([int] $Matches['zeile'])
})
$zeilen += @(
    '--- BAU (Release) ---',
    "Ziele:   $($bauZiele -join ' ')",
    "Exit:    $bauExit ($([math]::Round($bauUhr.Elapsed.TotalSeconds, 1)) s)",
    "warning C (eindeutige Zeilen): $($warnungen.Count) an $($warnungsStellen.Count) Stellen; davon auf einer Zeile, die diese Etappe hinzugefuegt oder geaendert hat: $($warnungenAusGeaendert.Count)")
foreach ($w in $warnungsStellen) { $zeilen += "  Stelle: $w" }
foreach ($w in $warnungenAusGeaendert) { $zeilen += "  NEU: $w" }
$zeilen += ''
if ($bauExit -ne 0) {
    $zeilen += $bauAus
    [System.IO.File]::WriteAllLines($Ziel, $zeilen, $utf8)
    throw "Bau fehlgeschlagen (Exit $bauExit)"
}

# ── Abhaengigkeiten aus den MSBuild-Trackinglogs ──────────────────────────
function Lies-Tlog([string] $pfad) {
    foreach ($zeile in [System.IO.File]::ReadAllLines($pfad, [System.Text.Encoding]::Unicode)) {
        foreach ($teil in $zeile.TrimStart('^').Split('|')) { if ($teil) { $teil } }
    }
}

$bibliotheken = @{}
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
$beine = @()
$fehler = @()
foreach ($k in $kanonPlugin) {
    $a = $abh[$k.Name]
    $exe = Join-Path $build ("plugin\{0}_artefacts\Release\{0}.exe" -f $k.Name)
    if (-not [string]::Equals($exe, $a.Artefakt, [StringComparison]::OrdinalIgnoreCase)) { $fehler += "$($k.Kuerzel): Artefakt $exe, gelinkt wurde $($a.Artefakt)" }
    $grund = $null
    if ($auftrag.Contains($k.Kuerzel)) { $grund = $auftrag[$k.Kuerzel] }
    elseif ($a.Quellen.Count -gt 0) { $grund = 'Trackinglog' }
    if ($grund) {
        $beine += [pscustomobject]@{ Kuerzel = $k.Kuerzel; Ziel = $k.Name; Art = 'plugin'; Grund = $grund; Arg = $k.Argumente; ArgText = $k.ArgText; Artefakte = @($k.Name); Ordner = '' }
        $status = "gefahren ($grund)"
    } else { $status = 'nicht gefahren - liest und linkt keine geaenderte Quelle' }
    $zeilen += ('{0,-5} {1,-38} {2,-110} {3}' -f $k.Kuerzel, $k.Name, (Kurz $a.Quellen), $status)
}
foreach ($k in @($kanon | Where-Object { $_.Art -ne 'plugin' -and -not $_.Stillgelegt })) {
    $grund = $null
    $artefakte = @()
    $beruehrt = $false
    if ($k.Art -eq 'python') {
        $skript = Join-Path $Wurzel ($k.Ordner + '\' + $k.Name)
        if (-not (Test-Path -LiteralPath $skript)) { $fehler += "$($k.Kuerzel): Skript $skript fehlt" }
        else { $beruehrt = [System.IO.File]::ReadAllText($skript) -match 'build.{0,8}plugin|_artefacts|NakamaKern\.lib' }
    }
    if ($auftrag.Contains($k.Kuerzel)) { $grund = $auftrag[$k.Kuerzel] }
    if ($beruehrt) {
        if (-not $pythonArtefakte.ContainsKey($k.Kuerzel)) { $fehler += "$($k.Kuerzel) $($k.Name) beruehrt eq-copilot/build/plugin und hat keine Artefaktzuordnung" }
        else {
            $artefakte = $pythonArtefakte[$k.Kuerzel]
            $liest = @($artefakte | Where-Object { $abh[$_].Quellen.Count -gt 0 })
            if ($liest.Count -gt 0 -and -not $grund) { $grund = "Artefakt ($($artefakte -join ', '))" }
        }
    }
    if ($grund) {
        $beine += [pscustomobject]@{ Kuerzel = $k.Kuerzel; Ziel = $k.Name; Art = $k.Art; Grund = $grund; Arg = $k.Argumente; ArgText = $k.ArgText; Artefakte = $artefakte; Ordner = $k.Ordner }
        $status = "gefahren ($grund)"
    }
    elseif ($beruehrt) { $status = "nicht gefahren - $($k.Art), Artefakt $($artefakte -join ', ') liest und linkt keine geaenderte Quelle" }
    else { $status = "nicht gefahren - $($k.Art), beruehrt kein Artefakt aus eq-copilot/build/plugin" }
    $zeilen += ('{0,-5} {1,-38} {2}' -f $k.Kuerzel, $k.Name, $status)
}
foreach ($k in @($auftrag.Keys)) { if (-not ($beine | Where-Object { $_.Kuerzel -eq $k })) { $fehler += "Auftragsbein $k wurde nicht ausgewaehlt" } }
$zeilen += ''
if ($fehler.Count -gt 0) {
    $zeilen += @('AUSWAHL FEHLERHAFT:') + @($fehler | ForEach-Object { "  $_" })
    [System.IO.File]::WriteAllLines($Ziel, $zeilen, $utf8)
    throw "Beinauswahl weicht ab: $($fehler -join ' | ')"
}

# ── Frische der Artefakte (Pruefliste D, NAK-230) ─────────────────────────
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
        $skript = Join-Path $Wurzel ($b.Ordner + '\' + $b.Ziel)
        $aus = & py -3.13 $skript @($b.Arg) 2>&1
    }
    $exit = $LASTEXITCODE
    $ErrorActionPreference = 'Stop'
    $uhr.Stop()
    $sekunden = [math]::Round($uhr.Elapsed.TotalSeconds, 1)
    Write-Host " Exit $exit ($sekunden s)"
    $bilanz += [pscustomobject]@{ Kuerzel = $b.Kuerzel; Ziel = $b.Ziel; Grund = $b.Grund; Exit = $exit; Sekunden = $sekunden; ArgText = $b.ArgText }
    $zeilen += @(
        "--- $($b.Kuerzel)  $($b.Ziel) $($b.Arg -join ' ')  ->  Exit $exit ($sekunden s) ---",
        (@($aus | ForEach-Object { "$_" }) -join "`n"),
        '')
}

$zeilen += '--- BILANZ ---'
foreach ($z in $bilanz) { $zeilen += ("{0,-5} {1,-38} {2,-40} Exit {3}  ({4} s)" -f $z.Kuerzel, $z.Ziel, $z.Grund, $z.Exit, $z.Sekunden) }
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
