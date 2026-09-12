# NAK-283 Etappe 2 — Rotbeweise je Matrixzeile (Bauer, 12.09.2026)
#
# Ablauf je Zeile nach docs/beweise/NAK-283.md §6.1:
#   1. SHA-256 der unveraenderten Quelldatei notieren
#   2. Mutation GENAU an der Zeile, die die Zusage traegt
#   3. LastWriteTime setzen (NAK-230: eine zurueckgespielte Quelle kann aelter
#      sein als ihr Objektfile; MSBuild baut dann nicht neu und meldet Exit 0)
#   4. Bein einzeln fahren -> erwartet ROT
#   5. Bytegleiche Ruecknahme (Originalbytes zurueckspielen), SHA-256 gegen
#      Schritt 1, Zeitstempel erneut setzen, Bein erneut fahren -> erwartet GRUEN
#
# Aufruf aus dem Workspace-Root:
#   pwsh -File docs/beweise/roh/NAK-283-etappe-2-rotskript.ps1
#   pwsh -File docs/beweise/roh/NAK-283-etappe-2-rotskript.ps1 -Nur MUT-A
#
# Rohausgaben: docs/beweise/roh/NAK-283-rot-M-<nn>-etappe-2.txt

[CmdletBinding()]
param(
    [string] $Nur = '',
    [string] $Wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
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
$rohOrdner = Join-Path $Wurzel 'docs\beweise\roh'

function Exe-Pfad([string] $ziel) {
    Join-Path $build ("plugin\{0}_artefacts\Release\{0}.exe" -f $ziel)
}

function Zeilenende([string] $text) {
    if ($text -match "`r`n") { return "`r`n" } else { return "`n" }
}

# Eine Mutation: Datei, Suchzeilen, Ersatzzeilen. Suchen und Ersetzen laufen
# zeilenweise und setzen das in der DATEI gefundene Zeilenende wieder ein -
# der Baum bleibt in seinen eigenen Zeilenenden (CLAUDE.md gitattributes).
function Mutiere([string] $datei, [string[]] $alt, [string[]] $neu) {
    $t = [System.IO.File]::ReadAllText($datei)
    $le = Zeilenende $t
    $suche = ($alt -join $le)
    if (-not $t.Contains($suche)) { throw "Mutationsmuster nicht gefunden in $datei" }
    $treffer = ([regex]::Matches($t, [regex]::Escape($suche))).Count
    if ($treffer -ne 1) { throw "Mutationsmuster $treffer-mal gefunden in $datei (erwartet: 1)" }
    $t = $t.Replace($suche, ($neu -join $le))
    [System.IO.File]::WriteAllText($datei, $t, (New-Object System.Text.UTF8Encoding $false))
}

function Hash-Von([string] $datei) { (Get-FileHash -Algorithm SHA256 $datei).Hash }

function Baue([string[]] $ziele) {
    $args = @('--build', $build, '--config', 'Release', '--target') + $ziele
    $aus = & $cmake @args 2>&1
    return @{ ExitCode = $LASTEXITCODE; Text = ($aus -join "`n") }
}

function Fahre([string] $ziel, [string[]] $argumente) {
    $exe = Exe-Pfad $ziel
    $aus = & $exe @argumente 2>&1
    return @{ ExitCode = $LASTEXITCODE; Text = ($aus -join "`n") }
}

# ── Die Mutationen ────────────────────────────────────────────────────────
$sourcesModel = Join-Path $Wurzel 'eq-copilot\plugin\src\SourcesModel.cpp'
$ipc          = Join-Path $Wurzel 'eq-copilot\plugin\src\prozessor\Ipc.cpp'
$state        = Join-Path $Wurzel 'eq-copilot\plugin\state\NakamaState.cpp'

$mutationen = @(
    [pscustomobject]@{
        Kennung = 'MUT-A'
        Zeilen  = 'M-01a, M-02, M-03, M-06'
        Datei   = $sourcesModel
        Was     = 'SourcesModel: die Folgenummernpruefung entfernen - nur der Generationsvergleich bleibt, wie am Basis-SHA'
        Alt     = @(
            '    if (folge <= zuletztUebernommeneFolge)',
            '    {',
            '        ++ueberholtZaehler;',
            '        return Publikation::ueberholt;',
            '    }')
        Neu     = @('    // MUTATION NAK-283 Rotbeweis: Folgenummernpruefung entfernt.')
        Ziele   = @('EqCopSonde012ProjectReloadTest')
        Laeufe  = @(
            @{ Ziel = 'EqCopSonde012ProjectReloadTest'; Arg = @('--nur','283m01'); Datei = 'NAK-283-rot-M-01a-etappe-2.txt' },
            @{ Ziel = 'EqCopSonde012ProjectReloadTest'; Arg = @('--nur','283m02'); Datei = 'NAK-283-rot-M-02-etappe-2.txt' },
            @{ Ziel = 'EqCopSonde012ProjectReloadTest'; Arg = @('--nur','283m03'); Datei = 'NAK-283-rot-M-03-etappe-2.txt' },
            @{ Ziel = 'EqCopSonde012ProjectReloadTest'; Arg = @('--nur','283m06'); Datei = 'NAK-283-rot-M-06-etappe-2.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-B'
        Zeilen  = 'M-01b, M-72'
        Datei   = $ipc
        Was     = 'Ipc: den Ueberholt-Ausstieg auf den Reload-Zweig legen (Ausstieg VOR meldeHostDirty und v3StateRevision)'
        Alt     = @(
            '            sourcesPublikationUeberholt.fetch_add (1);',
            '            break;')
        Neu     = @(
            '            sourcesPublikationUeberholt.fetch_add (1);',
            '            return;   // MUTATION NAK-283 Rotbeweis: Ausstieg vor Dirty und Revision')
        Ziele   = @('EqCopSonde012ProjectReloadTest')
        Laeufe  = @(
            @{ Ziel = 'EqCopSonde012ProjectReloadTest'; Arg = @('--nur','283m01'); Datei = 'NAK-283-rot-M-01b-etappe-2.txt' },
            @{ Ziel = 'EqCopSonde012ProjectReloadTest'; Arg = @('--nur','283m72'); Datei = 'NAK-283-rot-M-72-etappe-2.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-C'
        Zeilen  = 'M-04'
        Datei   = $sourcesModel
        Was     = 'SourcesModel: den Generationsvergleich entfernen - ein Riegel, der nur noch die Folgenummer prueft'
        Alt     = @(
            '    if (generation != reloadGeneration)',
            '    {',
            '        ++reloadAbgewiesenZaehler;',
            '        return Publikation::reloadAbgewiesen;',
            '    }')
        Neu     = @('    // MUTATION NAK-283 Rotbeweis: Generationsvergleich entfernt.')
        Ziele   = @('EqCopSonde012ProjectReloadTest')
        Laeufe  = @(
            @{ Ziel = 'EqCopSonde012ProjectReloadTest'; Arg = @('--nur','m39'); Datei = 'NAK-283-rot-M-04-etappe-2.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-D'
        Zeilen  = 'M-05'
        Datei   = $sourcesModel
        Was     = 'SourcesModel: den fruehen return bei Gleichheit VOR die Uebernahme der Folgenummer legen (heutige Reihenfolge)'
        Alt     = @(
            '    zuletztUebernommeneFolge = folge;',
            '    if (neu == persistenteMitglieder)',
            '        return Publikation::uebernommen;  // Publikation fand statt, ohne Aenderung.')
        Neu     = @(
            '    if (neu == persistenteMitglieder)',
            '        return Publikation::uebernommen;  // MUTATION NAK-283 Rotbeweis: vor der Nummer',
            '    zuletztUebernommeneFolge = folge;')
        Ziele   = @('EqCopSonde012ProjectReloadTest')
        Laeufe  = @(
            @{ Ziel = 'EqCopSonde012ProjectReloadTest'; Arg = @('--nur','283m05'); Datei = 'NAK-283-rot-M-05-etappe-2.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-E'
        Zeilen  = 'M-71'
        Datei   = $ipc
        Was     = 'Ipc: ++folge OHNE obere Schranke - die Nummer springt am Anschlag auf 0'
        Alt     = @(
            '    if (sourcesMitgliederFolge < std::numeric_limits<std::uint64_t>::max())',
            '        ++sourcesMitgliederFolge;',
            '    return sourcesMitgliederFolge;')
        Neu     = @(
            '    ++sourcesMitgliederFolge;   // MUTATION NAK-283 Rotbeweis: ohne obere Schranke',
            '    return sourcesMitgliederFolge;')
        Ziele   = @('EqCopSonde012ProjectReloadTest')
        Laeufe  = @(
            @{ Ziel = 'EqCopSonde012ProjectReloadTest'; Arg = @('--nur','283m71'); Datei = 'NAK-283-rot-M-71-etappe-2.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-F'
        Zeilen  = 'M-07, M-11b'
        Datei   = $state
        Was     = 'NakamaState: in entferneIntent die Revisionspruefung wieder HINTER das erase schieben (heutige Reihenfolge)'
        Alt     = @(
            '    const bool passendeVorhanden = std::any_of (z.sourceIntents.begin(), z.sourceIntents.end(),',
            '        [&] (const SourceIntent& s) { return s.quelleId == quelleId && s.passageId == passageId; });',
            '    if (! passendeVorhanden)',
            '        return true;',
            '    if (! bestandsrevisionHeben (z, grund)) return false;',
            '    z.sourceIntents.erase (std::remove_if (z.sourceIntents.begin(), z.sourceIntents.end(),',
            '        [&] (const SourceIntent& s) { return s.quelleId == quelleId && s.passageId == passageId; }),',
            '        z.sourceIntents.end());',
            '    veraendert = true;')
        Neu     = @(
            '    // MUTATION NAK-283 Rotbeweis: erase VOR dem Riegel (Stand vor der Etappe).',
            '    const auto vorher = z.sourceIntents.size();',
            '    z.sourceIntents.erase (std::remove_if (z.sourceIntents.begin(), z.sourceIntents.end(),',
            '        [&] (const SourceIntent& s) { return s.quelleId == quelleId && s.passageId == passageId; }),',
            '        z.sourceIntents.end());',
            '    if (z.sourceIntents.size() == vorher)',
            '        return true;',
            '    if (! bestandsrevisionHeben (z, grund)) return false;',
            '    veraendert = true;')
        Ziele   = @('EqCopSonde014IntentTest', 'EqCopStateMigrationTest')
        Laeufe  = @(
            @{ Ziel = 'EqCopSonde014IntentTest';  Arg = @(); Datei = 'NAK-283-rot-M-07-etappe-2.txt' },
            @{ Ziel = 'EqCopStateMigrationTest';  Arg = @(); Datei = 'NAK-283-rot-M-11b-etappe-2.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-G'
        Zeilen  = 'M-08'
        Datei   = $state
        Was     = 'NakamaState: in entferneSchutzangabe die Revisionspruefung wieder HINTER das erase schieben'
        Alt     = @(
            '    // NAK-283 F11 (M-08): Ablehnung ohne Mutation, wie `entferneIntent`.',
            '    if (std::find (z.schutzangaben.begin(), z.schutzangaben.end(), eintrag)',
            '            == z.schutzangaben.end())',
            '        return true;',
            '    if (! bestandsrevisionHeben (z, grund)) return false;',
            '    z.schutzangaben.erase (std::remove (z.schutzangaben.begin(), z.schutzangaben.end(), eintrag),',
            '                           z.schutzangaben.end());',
            '    veraendert = true;')
        Neu     = @(
            '    // MUTATION NAK-283 Rotbeweis: erase VOR dem Riegel.',
            '    const auto vorher = z.schutzangaben.size();',
            '    z.schutzangaben.erase (std::remove (z.schutzangaben.begin(), z.schutzangaben.end(), eintrag),',
            '                           z.schutzangaben.end());',
            '    if (z.schutzangaben.size() == vorher)',
            '        return true;',
            '    if (! bestandsrevisionHeben (z, grund)) return false;',
            '    veraendert = true;')
        Ziele   = @('EqCopSonde014IntentTest')
        Laeufe  = @(
            @{ Ziel = 'EqCopSonde014IntentTest'; Arg = @(); Datei = 'NAK-283-rot-M-08-etappe-2.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-H'
        Zeilen  = 'M-09'
        Datei   = $state
        Was     = 'NakamaState: in entferneBeziehung die Revisionspruefung wieder HINTER das erase schieben'
        Alt     = @(
            '    // NAK-283 F11 (M-09): Ablehnung ohne Mutation, wie `entferneIntent`.',
            '    const bool passendeVorhanden = std::any_of (',
            '        z.intentBeziehungen.begin(), z.intentBeziehungen.end(),',
            '        [&] (const IntentBeziehung& k) { return k.quelleA == quelleA && k.quelleB == quelleB; });',
            '    if (! passendeVorhanden)',
            '        return true;',
            '    if (! bestandsrevisionHeben (z, grund)) return false;',
            '    z.intentBeziehungen.erase (std::remove_if (z.intentBeziehungen.begin(), z.intentBeziehungen.end(),',
            '        [&] (const IntentBeziehung& k) { return k.quelleA == quelleA && k.quelleB == quelleB; }),',
            '        z.intentBeziehungen.end());',
            '    veraendert = true;')
        Neu     = @(
            '    // MUTATION NAK-283 Rotbeweis: erase VOR dem Riegel.',
            '    const auto vorher = z.intentBeziehungen.size();',
            '    z.intentBeziehungen.erase (std::remove_if (z.intentBeziehungen.begin(), z.intentBeziehungen.end(),',
            '        [&] (const IntentBeziehung& k) { return k.quelleA == quelleA && k.quelleB == quelleB; }),',
            '        z.intentBeziehungen.end());',
            '    if (z.intentBeziehungen.size() == vorher)',
            '        return true;',
            '    if (! bestandsrevisionHeben (z, grund)) return false;',
            '    veraendert = true;')
        Ziele   = @('EqCopSonde014IntentTest')
        Laeufe  = @(
            @{ Ziel = 'EqCopSonde014IntentTest'; Arg = @(); Datei = 'NAK-283-rot-M-09-etappe-2.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-I'
        Zeilen  = 'M-10, M-11a'
        Datei   = $state
        Was     = 'NakamaState: die obere Schranke der Assistentenrevision entfernen - a.revision += 1 laeuft an int64max ueber'
        Alt     = @(
            '    if (a.revision >= std::numeric_limits<juce::int64>::max())',
            '    {',
            '        grund = "assistant revision would overflow";',
            '        return false;',
            '    }',
            '    a.revision += 1;',
            '    return true;')
        Neu     = @(
            '    (void) grund;',
            '    a.revision += 1;   // MUTATION NAK-283 Rotbeweis: ohne obere Schranke',
            '    return true;')
        Ziele   = @('EqCopSonde014IntentTest', 'EqCopStateMigrationTest')
        Laeufe  = @(
            @{ Ziel = 'EqCopSonde014IntentTest'; Arg = @(); Datei = 'NAK-283-rot-M-10-etappe-2.txt' },
            @{ Ziel = 'EqCopStateMigrationTest'; Arg = @(); Datei = 'NAK-283-rot-M-11a-etappe-2.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-J'
        Zeilen  = 'M-12 (Regressionswache)'
        Datei   = $state
        Was     = 'NakamaState: die Bestandsschranke auf int64max - 1 vorziehen - der Normalfall unter der Grenze wird abgewiesen'
        Alt     = @(
            '    if (z.intentBestandRevision >= std::numeric_limits<juce::int64>::max())')
        Neu     = @(
            '    if (z.intentBestandRevision >= std::numeric_limits<juce::int64>::max() - 1)   // MUTATION NAK-283 Rotbeweis')
        Ziele   = @('EqCopSonde014IntentTest')
        Laeufe  = @(
            @{ Ziel = 'EqCopSonde014IntentTest'; Arg = @(); Datei = 'NAK-283-rot-M-12-etappe-2.txt' })
    }
)

$bilanz = @()
foreach ($m in $mutationen) {
    if ($Nur -and $m.Kennung -ne $Nur) { continue }
    Write-Host "=== $($m.Kennung) ($($m.Zeilen)) ===" -ForegroundColor Cyan

    $sicherung = [System.IO.File]::ReadAllBytes($m.Datei)
    $hashVorher = Hash-Von $m.Datei

    $kopfZeilen = @(
        "NAK-283 Etappe 2 - Rotbeweis $($m.Kennung) fuer $($m.Zeilen)",
        "Datei:      $($m.Datei.Replace($Wurzel + '\', ''))",
        "Mutation:   $($m.Was)",
        "SHA-256 vorher: $hashVorher",
        "")

    try {
        Mutiere $m.Datei $m.Alt $m.Neu
        (Get-Item $m.Datei).LastWriteTime = Get-Date
        $bau = Baue $m.Ziele
        if ($bau.ExitCode -ne 0) { throw "Bau der mutierten Fassung fehlgeschlagen: $($bau.Text)" }
        foreach ($z in $m.Ziele) {
            $exe = Exe-Pfad $z
            if ((Get-Item $exe).LastWriteTime -lt (Get-Item $m.Datei).LastWriteTime) {
                throw "Binary $z ist aelter als die mutierte Quelle - MSBuild hat nicht neu gebaut"
            }
        }
        $rotLaeufe = @()
        foreach ($l in $m.Laeufe) {
            $r = Fahre $l.Ziel $l.Arg
            $rotLaeufe += [pscustomobject]@{ Lauf = $l; Ergebnis = $r }
            Write-Host ("  ROT   {0} {1} -> Exit {2}" -f $l.Ziel, ($l.Arg -join ' '), $r.ExitCode)
        }
    }
    finally {
        # Bytegleiche Ruecknahme: die ORIGINALBYTES zurueck, nie eine zweite
        # Textersetzung - so ist die Gleichheit strukturell und der Hash belegt sie.
        [System.IO.File]::WriteAllBytes($m.Datei, $sicherung)
    }

    $hashNachher = Hash-Von $m.Datei
    if ($hashNachher -ne $hashVorher) { throw "Ruecknahme NICHT bytegleich in $($m.Datei)" }
    (Get-Item $m.Datei).LastWriteTime = Get-Date
    $bau2 = Baue $m.Ziele
    if ($bau2.ExitCode -ne 0) { throw "Bau der zurueckgespielten Fassung fehlgeschlagen: $($bau2.Text)" }

    foreach ($e in $rotLaeufe) {
        $l = $e.Lauf
        $gruen = Fahre $l.Ziel $l.Arg
        Write-Host ("  GRUEN {0} {1} -> Exit {2}" -f $l.Ziel, ($l.Arg -join ' '), $gruen.ExitCode)
        $ziel = Join-Path $rohOrdner $l.Datei
        $inhalt = $kopfZeilen + @(
            "Bein:       $($l.Ziel) $($l.Arg -join ' ')",
            "SHA-256 nachher (bytegleiche Ruecknahme): $hashNachher",
            "Urteil:     ROT Exit $($e.Ergebnis.ExitCode) / GRUEN Exit $($gruen.ExitCode)",
            "",
            "--- MUTIERT (erwartet: rot) ---------------------------------------",
            $e.Ergebnis.Text,
            "",
            "--- ZURUECKGESPIELT (erwartet: gruen) -----------------------------",
            $gruen.Text)
        [System.IO.File]::WriteAllLines($ziel, $inhalt, (New-Object System.Text.UTF8Encoding $false))
        $bilanz += [pscustomobject]@{
            Kennung = $m.Kennung; Bein = "$($l.Ziel) $($l.Arg -join ' ')"
            Rot = $e.Ergebnis.ExitCode; Gruen = $gruen.ExitCode
            HashGleich = ($hashNachher -eq $hashVorher); Datei = $l.Datei
        }
    }
}

Write-Host ''
Write-Host '=== Bilanz ===' -ForegroundColor Cyan
$bilanz | Format-Table -AutoSize
$schlecht = @($bilanz | Where-Object { $_.Rot -eq 0 -or $_.Gruen -ne 0 -or -not $_.HashGleich })
if ($schlecht.Count -gt 0) {
    Write-Host "NICHT BESTANDEN: $($schlecht.Count) Rotbeweis(e)" -ForegroundColor Red
    exit 1
}
Write-Host "Alle $($bilanz.Count) Rotbeweise bestanden (rot != 0, gruen == 0, Hash bytegleich)" -ForegroundColor Green
exit 0
