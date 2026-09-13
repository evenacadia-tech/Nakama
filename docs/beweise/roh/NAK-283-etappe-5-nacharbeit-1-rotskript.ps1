# NAK-283 Etappe 5 Nacharbeit 1 - Rotbeweis M-76 und die Neufahrten an Frame.h und M-60 (Bauer, 13.09.2026)
#
# Ablauf je Mutation nach docs/beweise/NAK-283.md §6.1, Bauform wie
# docs/beweise/roh/NAK-283-etappe-5-rotskript.ps1:
#   1. Originalbytes und SHA-256 JEDER Datei der Mutation sichern
#   2. Mutation genau an der Zeile, die die Zusage traegt - jedes Muster genau
#      einmal (sonst Abbruch), die Zeilenenden der Datei bleiben. Ein
#      BASISLAUF ersetzt die ganze Datei durch ihren Stand am genannten
#      Basis-SHA (git cat-file ohne Filter, Zeilenenden wie im Arbeitsbaum).
#   3. LastWriteTime setzen (NAK-230), Neubau der Ziele; jedes Binary muss
#      juenger sein als jede mutierte Datei
#   4. Jedes Ziel EINMAL fahren -> ein Rotlauf zaehlt nur mit Exit != 0 UND
#      einer [ROT]-Zeile fuer JEDEN Traeger des Laufs (M-76: Fenster UND
#      Maximum UND Verteilung) - nie an einem Nebeneffekt
#   5. Originalbytes zurueck (nie eine zweite Textersetzung), SHA-256 gegen
#      Schritt 1, Zeitstempel, Neubau, Ziel fahren -> GRUEN mit Exit 0 UND
#      einer [ok]-Zeile je Traeger
#
# Die Laeufe:
#   MUT-76     Zusagezeile M-76 in Frame.h: die Bedingung `rahmenNichtEndlich == 0` entfernt
#   BASIS-N1   Frame.h und Vertrag.h im Stand des Basis-SHA der Nacharbeit, die Tests im Endstand
#   BUEHNE-76  keine Produktmutation: die Buehne von M-76 OHNE regulaere Schlussperiode im
#              ersetzten Rahmen - belegt, warum die Buehne von der Matrixzeile abweicht (§42)
#   PB-76a..d  Pruefungsbrueche (tools/dirigent/pruefliste.md E: jede neue Pruefung einmal
#              absichtlich gebrochen) fuer die vier neuen Pruefungen, die MUT-76 gruen laesst:
#              a Buehnenpruefung, b Gegenprobe (beide am Test), c Perzentile, d "die Passage
#              bleibt gueltig" (beide als falscher Fix in Frame.h)
#   MUT-52d, MUT-53, MUT-54, MUT-55
#              die Etappe-5-Mutationen an Frame.h, Muster woertlich, am geaenderten Frame.h
#   MUT-49a, MUT-56, BASIS-E5
#              die drei Rotbeweise von M-60, dessen Fall diese Nacharbeit aendert (140 -> 138)
#
# Keine Zeile steht in der Nebenlaeufigkeitsliste aus §6.1: jede Buehne ist ein
# deterministischer Sampleplan auf dem Workerpfad der Engine.
#
# Aufruf aus dem Workspace-Root:
#   pwsh -File docs/beweise/roh/NAK-283-etappe-5-nacharbeit-1-rotskript.ps1
#   pwsh -File docs/beweise/roh/NAK-283-etappe-5-nacharbeit-1-rotskript.ps1 -Nur MUT-76
#   pwsh -File docs/beweise/roh/NAK-283-etappe-5-nacharbeit-1-rotskript.ps1 -Muster   (nur Trefferzahl je Muster, schreibt nichts)
# Rohausgaben: docs/beweise/roh/NAK-283-rot-M-76-etappe-5-nacharbeit-1*.txt,
#              docs/beweise/roh/NAK-283-etappe-5-nacharbeit-1-rot-*.txt
# Bilanz:      docs/beweise/roh/NAK-283-etappe-5-nacharbeit-1-rotbilanz.txt (nur beim vollen Lauf)

[CmdletBinding()]
param(
    [string] $Nur = '',
    [switch] $Muster,
    [string] $Wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
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

$build     = Join-Path $Wurzel 'eq-copilot\build'
$rohOrdner = Join-Path $Wurzel 'docs\beweise\roh'
$utf8      = New-Object System.Text.UTF8Encoding $false
$basisN1   = 'd8c3f74c656bb88a08999edbf6705440a4f2e71a'   # Basis der Nacharbeit 1
$basisE5   = 'bc0941e51bcdf8d3752362c3ca10cda628bd1438'   # Basis der Etappe 5

function Exe-Pfad([string] $ziel) { Join-Path $build ("plugin\{0}_artefacts\Release\{0}.exe" -f $ziel) }
function Zeilenende([string] $text) { if ($text -match "`r`n") { "`r`n" } else { "`n" } }
function Hash-Von([string] $datei) { (Get-FileHash -Algorithm SHA256 -LiteralPath $datei).Hash }
function Relativ([string] $pfad) { $pfad.Replace($Wurzel + '\', '').Replace('\', '/') }

function Blob-Bytes([string] $sha, [string] $rel) {
    # Die Bytes des Blobs, ohne Filter und ohne Konsolenkodierung dazwischen.
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = 'git'
    foreach ($a in @('-C', $Wurzel, 'cat-file', 'blob', "${sha}:${rel}")) { $psi.ArgumentList.Add($a) }
    $psi.RedirectStandardOutput = $true
    $psi.UseShellExecute = $false
    $p = [System.Diagnostics.Process]::Start($psi)
    $ms = New-Object System.IO.MemoryStream
    $p.StandardOutput.BaseStream.CopyTo($ms)
    $p.WaitForExit()
    if ($p.ExitCode -ne 0) { throw "git cat-file gescheitert: ${sha}:${rel}" }
    return ,$ms.ToArray()
}

function Mutiere($aenderung) {
    $datei = $aenderung.Datei
    $t = [System.IO.File]::ReadAllText($datei, $utf8)
    $le = Zeilenende $t
    if ($aenderung.ContainsKey('Basis')) {
        $blob = $utf8.GetString((Blob-Bytes $aenderung.Basis (Relativ $datei)))
        $blob = $blob.Replace("`r`n", "`n")
        if ($le -eq "`r`n") { $blob = $blob.Replace("`n", "`r`n") }
        [System.IO.File]::WriteAllText($datei, $blob, $utf8)
        return
    }
    $suche = ($aenderung.Alt -join $le)
    $treffer = ([regex]::Matches($t, [regex]::Escape($suche))).Count
    if ($treffer -ne 1) { throw "Mutationsmuster $treffer-mal gefunden in $(Relativ $datei) (erwartet: 1): $($aenderung.Alt[0])" }
    $t = $t.Replace($suche, ($aenderung.Neu -join $le))
    [System.IO.File]::WriteAllText($datei, $t, $utf8)
}

function Baue([string[]] $ziele) {
    $argumente = @('--build', $build, '--config', 'Release', '--target') + $ziele
    $ErrorActionPreference = 'Continue'
    $aus = & $cmake @argumente 2>&1
    $code = $LASTEXITCODE
    $ErrorActionPreference = 'Stop'
    return @{ ExitCode = $code; Text = (@($aus | ForEach-Object { "$_" }) -join "`n") }
}

function Fahre([string] $ziel) {
    $ErrorActionPreference = 'Continue'
    $aus = & (Exe-Pfad $ziel) 2>&1
    $code = $LASTEXITCODE
    $ErrorActionPreference = 'Stop'
    return @{ ExitCode = $code; Text = (@($aus | ForEach-Object { "$_" }) -join "`n") }
}

function Pruefe-Frische([string[]] $ziele, [string[]] $dateien, [string] $stand) {
    foreach ($z in $ziele) {
        foreach ($d in $dateien) {
            if ((Get-Item -LiteralPath (Exe-Pfad $z)).LastWriteTime -lt (Get-Item -LiteralPath $d).LastWriteTime) {
                throw "Binary $z ist aelter als die $stand Datei $(Relativ $d) - MSBuild hat nicht neu gebaut"
            }
        }
    }
}

# Je Traeger die erste Zeile mit der Marke, die ihn enthaelt ('' = nicht gefunden).
function Traeger-Zeilen([string] $text, [string] $marke, [string[]] $traeger) {
    $zeilen = @($text -split "`n" | ForEach-Object { $_.Trim() })
    foreach ($tr in $traeger) {
        $treffer = @($zeilen | Where-Object { $_.StartsWith($marke) -and $_.Contains($tr) })
        [pscustomobject]@{ Traeger = $tr; Zeile = $(if ($treffer.Count -gt 0) { $treffer[0] } else { '' }) }
    }
}

# ── Die Dateien ───────────────────────────────────────────────────────────
$engineH   = Join-Path $Wurzel 'eq-copilot\plugin\core\analysis\FeatureEngine.h'
$spektrumH = Join-Path $Wurzel 'eq-copilot\plugin\core\analysis\featureengine\Spektrum.h'
$lautheitH = Join-Path $Wurzel 'eq-copilot\plugin\core\analysis\featureengine\Lautheit.h'
$frameH    = Join-Path $Wurzel 'eq-copilot\plugin\core\analysis\featureengine\Frame.h'
$vertragH  = Join-Path $Wurzel 'eq-copilot\plugin\core\analysis\featureengine\Vertrag.h'
$dynTest   = Join-Path $Wurzel 'eq-copilot\plugin\tests\Sonde013DynamicsTest.cpp'

$B16 = 'EqCopSonde013EventWireTest'
$B18 = 'EqCopSonde013DynamicsTest'

# ── Die Traeger (Pruefzeilen der Zusagen) ─────────────────────────────────
$pre76 = 'NAK-283 M-76: rahmen_mit_ersetzten_samples_geht_nicht_in_die_passagenverteilung_ein - '
$T76fenster    = $pre76 + 'headroomFenster zaehlt den Rahmen nicht'
$T76maximum    = $pre76 + 'das Passagenmaximum bleibt beim gemessenen Material'
$T76verteilung = $pre76 + 'die Verteilung traegt keinen Wert aus dem teilweise stillgelegten Rahmen'
$T76perzentile = $pre76 + 'P10, P50 und P95 gleichen dem Gegenlauf'
$T76gueltig    = $pre76 + 'die Passage selbst bleibt gueltig'
$T76buehne     = 'M-76: je 140 Rahmen; in der Buehne traegt genau EIN Rahmen ersetzte Samples'
$T76probe      = 'M-76: Gegenprobe - derselbe Rahmen mit echten Nullen statt NaN geht ein'
$pre60 = 'NAK-283 M-60: jede_analysehistorie_traegt_ihre_eigene_spanne - '
$T60truePeak = $pre60 + 'True-Peak-Kette: das Passagenmaximum ist das der Passage'
$T60headroom = $pre60 + 'Headroom: die Verteilung beschreibt beide Haelften der ganzen Passage'
$T60loudness = $pre60 + 'Loudnesszellen: Momentary kommt 0,4 s, Short-term 3,0 s'
$T53 = 'NAK-283 M-53: headroomverteilung_umfasst_die_ganze_passage'
$T54 = 'NAK-283 M-54: headroomfenster_nennt_die_passagenlaenge'
$T55 = 'NAK-283 M-55: headroom_unter_vier_rahmen_bleibt_ungesetzt'
$T52headroom = 'Headroom: die Verteilung umfasst genau die Rahmen der Passage'

$N1 = 'NAK-283-etappe-5-nacharbeit-1-rot-'
$R76 = 'NAK-283-rot-M-76-etappe-5-nacharbeit-1'

function L([string] $zeile, [string] $ziel, [string[]] $traeger, [string] $datei) {
    @{ Zeile = $zeile; Ziel = $ziel; Traeger = $traeger; Datei = $datei }
}

# Die eine Zeile der Buehne, an der Buehne und Gegenprobe sich unterscheiden.
$buehnenZeile = '                    return art == Art::buehne ? std::numeric_limits<float>::quiet_NaN() : 0.0f;'

# ── Die Mutationen ────────────────────────────────────────────────────────
$mutationen = @(
    # ── M-76: die Zusagezeile, der Basislauf und die Buehnenprobe ──────────
    [pscustomobject]@{
        Kennung = 'MUT-76'; Art = 'Rotbeweis'; Zeilen = 'M-76 (Zusagezeile); M-60 Headroom als Folge'
        Was     = 'Frame.h baueFrame: die Bedingung rahmenNichtEndlich == 0 entfernt - ein Rahmen mit ersetzten Samples geht wieder in Passagenmaximum und Headroomverteilung ein (Rotbeweisspalte M-76 woertlich)'
        Aenderungen = @(@{ Datei = $frameH
            Alt = @('    if (passagenTruePeakRahmen > 0.0 && rahmenNichtEndlich == 0 && ! headroomRing.empty())')
            Neu = @('    if (passagenTruePeakRahmen > 0.0 && ! headroomRing.empty())   // MUTATION NAK-283 Rotbeweis M-76: Bedingung rahmenNichtEndlich == 0 entfernt') })
        Ziele   = @($B18)
        Laeufe  = @(
            (L 'M-76' $B18 @($T76fenster, $T76maximum, $T76verteilung) ($R76 + '.txt')),
            (L 'M-60 Headroom (Folge M-76)' $B18 @($T60headroom) ($N1 + 'M-60-folge-M-76.txt')))
    }
    [pscustomobject]@{
        Kennung = 'BASIS-N1'; Art = 'Basislauf'; Zeilen = 'M-76 und M-60 Headroom am Basis-SHA der Nacharbeit'
        Was     = "Frame.h und Vertrag.h byteweise im Stand $basisN1 (Zeilenenden des Arbeitsbaums), die Tests im Endstand"
        Aenderungen = @(@{ Datei = $frameH; Basis = $basisN1 }, @{ Datei = $vertragH; Basis = $basisN1 })
        Ziele   = @($B18)
        Laeufe  = @(
            (L 'M-76' $B18 @($T76fenster, $T76maximum, $T76verteilung) ($R76 + '-basis.txt')),
            (L 'M-60 Headroom' $B18 @($T60headroom) ($N1 + 'M-60-basis-n1.txt')))
    }
    [pscustomobject]@{
        Kennung = 'BUEHNE-76'; Art = 'Buehnenprobe'; Zeilen = 'M-76 Buehnenprobe (keine Produktmutation)'
        Was     = 'Sonde013DynamicsTest.cpp, Buehne M-76: der ersetzte Rahmen endet mit dem Ton 0,9 statt mit einer Periode des regulaeren Tons seiner Haelfte; das Produkt bleibt im Endstand. Erwartet rot an Maximum und Verteilung: der Interpolator traegt den Nachklang in den naechsten, sauberen Rahmen (§39.9 Punkt 1 (d), NAK-295)'
        Aenderungen = @(@{ Datei = $dynTest
            Alt = @(
                '                if (art != Art::gegenlauf && n >= rahmenVon && n < rahmenVon + 4752)',
                '                {',
                '                    if (n < rahmenVon + 2400)')
            Neu = @(
                '                if (art != Art::gegenlauf && n >= rahmenVon && n < rahmenVon + 4800)   // BUEHNENPROBE NAK-283 M-76: ohne regulaere Schlussperiode',
                '                {',
                '                    if (n < rahmenVon + 2400 || n >= rahmenVon + 4752)') })
        Ziele   = @($B18)
        Laeufe  = @((L 'M-76 Buehnenprobe' $B18 @($T76maximum, $T76verteilung) ($R76 + '-buehnenprobe.txt')))
    }

    # ── M-76: Pruefungsbrueche fuer die vier Pruefungen, die MUT-76 gruen laesst ─
    [pscustomobject]@{
        Kennung = 'PB-76a'; Art = 'Pruefungsbruch'; Zeilen = 'M-76 Buehnenpruefung'
        Was     = 'Sonde013DynamicsTest.cpp, Buehne M-76: echte Nullen statt NaN auch in der Buehne - kein Rahmen traegt ersetzte Samples; die Buehnenpruefung muss das melden'
        Aenderungen = @(@{ Datei = $dynTest
            Alt = @($buehnenZeile)
            Neu = @('                    return 0.0f;   // PRUEFUNGSBRUCH NAK-283 M-76 (a): die Buehne ohne NaN') })
        Ziele   = @($B18)
        Laeufe  = @((L 'M-76 Buehnenpruefung' $B18 @($T76buehne) ($R76 + '-bruch-a.txt')))
    }
    [pscustomobject]@{
        Kennung = 'PB-76b'; Art = 'Pruefungsbruch'; Zeilen = 'M-76 Gegenprobe'
        Was     = 'Sonde013DynamicsTest.cpp, Buehne M-76: NaN statt echter Nullen auch in der Gegenprobe - ihr lauter Rahmen geht nicht ein; die Gegenprobe muss das melden'
        Aenderungen = @(@{ Datei = $dynTest
            Alt = @($buehnenZeile)
            Neu = @('                    return std::numeric_limits<float>::quiet_NaN();   // PRUEFUNGSBRUCH NAK-283 M-76 (b): die Gegenprobe mit NaN') })
        Ziele   = @($B18)
        Laeufe  = @((L 'M-76 Gegenprobe' $B18 @($T76probe) ($R76 + '-bruch-b.txt')))
    }
    [pscustomobject]@{
        Kennung = 'PB-76c'; Art = 'Pruefungsbruch'; Zeilen = 'M-76 Perzentile'
        Was     = 'Frame.h baueFrame, falscher Fix: der Rahmen mit ersetzten Samples geht als Stille (unterste Klasse) in die Verteilung, statt auszubleiben - das Passagenmaximum bleibt richtig, P50 kippt'
        Aenderungen = @(@{ Datei = $frameH
            Alt = @(
                '            headroomRing[0].ring.schiebe ((float) db);',
                '    }')
            Neu = @(
                '            headroomRing[0].ring.schiebe ((float) db);',
                '    }',
                '    if (rahmenNichtEndlich > 0 && passagenfenster.gesetzt && ! headroomRing.empty())   // PRUEFUNGSBRUCH NAK-283 M-76 (c): der ersetzte Rahmen als Stille',
                '        headroomRing[0].passageSchiebe (kHeadroomKlasseUntenDb + 0.5 * kHeadroomKlassenBreiteDb);') })
        Ziele   = @($B18)
        Laeufe  = @((L 'M-76 Perzentile' $B18 @($T76perzentile) ($R76 + '-bruch-c.txt')))
    }
    [pscustomobject]@{
        Kennung = 'PB-76d'; Art = 'Pruefungsbruch'; Zeilen = 'M-76 Passage bleibt gueltig'
        Was     = 'Frame.h baueFrame, falscher Fix (§39.9 Punkt 1 (b) woertlich gelesen): ein Rahmen mit ersetzten Samples bricht die Passage - keine Passagenverteilung bis zu ihrem Neuanfang'
        Aenderungen = @(@{ Datei = $frameH
            Alt = @('    if (passagenTruePeakRahmen > 0.0 && rahmenNichtEndlich == 0 && ! headroomRing.empty())')
            Neu = @(
                '    if (rahmenNichtEndlich > 0 && passagenfenster.gesetzt)   // PRUEFUNGSBRUCH NAK-283 M-76 (d): die Passage bricht am ersetzten Rahmen',
                '        passagenfensterGebrochen = true;',
                '    if (passagenTruePeakRahmen > 0.0 && rahmenNichtEndlich == 0 && ! headroomRing.empty())') })
        Ziele   = @($B18)
        Laeufe  = @((L 'M-76 Passage gueltig' $B18 @($T76gueltig) ($R76 + '-bruch-d.txt')))
    }

    # ── Frame.h: die Etappe-5-Mutationen, Muster woertlich ─────────────────
    [pscustomobject]@{
        Kennung = 'MUT-52d'; Art = 'Rotbeweis'; Zeilen = 'M-52 Anfang und Ende (Headroom)'
        Was     = 'Frame.h baueFrame: Rahmen vor dem Anfang und hinter dem Ende einer gebundenen Passage gehen mit ihrem Rahmen-True-Peak in die Headroomverteilung ein (Muster wie Etappe 5)'
        Aenderungen = @(@{ Datei = $frameH
            Alt = @(
                '            headroomRing[0].ring.schiebe ((float) db);',
                '    }')
            Neu = @(
                '            headroomRing[0].ring.schiebe ((float) db);',
                '    }',
                '    if (passagenfenster.gesetzt && passagenTruePeakRahmen <= 0.0 && rahmenTruePeak > 0.0 && ! headroomRing.empty())   // MUTATION NAK-283 Rotbeweis M-52d',
                '        headroomRing[0].passageSchiebe (20.0 * std::log10 (rahmenTruePeak));') })
        Ziele   = @($B16)
        Laeufe  = @((L 'M-52 Headroom' $B16 @($T52headroom) ($N1 + 'M-52d.txt')))
    }
    [pscustomobject]@{
        Kennung = 'MUT-53'; Art = 'Rotbeweis'; Zeilen = 'M-53, M-54, M-60 Headroom, M-52 Headroom'
        Was     = 'Frame.h baueFrame/fuelleSkalare: die Verteilung wieder aus dem 64er-Ring, auch mit gebundener Passage (Muster wie Etappe 5)'
        Aenderungen = @(
            @{ Datei = $frameH
               Alt = @(
                   '        if (passagenfenster.gesetzt)',
                   '            headroomRing[0].passageSchiebe (db);',
                   '        else',
                   '            headroomRing[0].ring.schiebe ((float) db);')
               Neu = @('        headroomRing[0].ring.schiebe ((float) db);   // MUTATION NAK-283 Rotbeweis M-53: wieder der 64er-Ring') }
            @{ Datei = $frameH
               Alt = @(
                   '        if (passagenfenster.gesetzt)',
                   '        {',
                   '            if (h.rahmen >= 4u && ! h.nichtDarstellbar)')
               Neu = @(
                   '        if (false)   // MUTATION NAK-283 Rotbeweis M-53: die Verteilung wieder aus dem 64er-Ring',
                   '        {',
                   '            if (h.rahmen >= 4u && ! h.nichtDarstellbar)') })
        Ziele   = @($B16, $B18)
        Laeufe  = @(
            (L 'M-53' $B18 @($T53) ($N1 + 'M-53.txt')),
            (L 'M-54' $B18 @($T54) ($N1 + 'M-54.txt')),
            (L 'M-60 Headroom' $B18 @($T60headroom) ($N1 + 'M-60b.txt')),
            (L 'M-52 Headroom (Ring)' $B16 @($T52headroom) ($N1 + 'M-52e.txt')))
    }
    [pscustomobject]@{
        Kennung = 'MUT-54'; Art = 'Rotbeweis'; Zeilen = 'M-54'
        Was     = 'Frame.h fuelleSkalare: headroomFenster nennt hoechstens die Ringbelegung, waehrend die Perzentile die Passage beschreiben (Muster wie Etappe 5)'
        Aenderungen = @(@{ Datei = $frameH
            Alt = @('                f.headroomFenster = h.rahmen;')
            Neu = @('                f.headroomFenster = std::min<std::uint32_t> (h.rahmen, (std::uint32_t) kVerteilungPlaetze);   // MUTATION NAK-283 Rotbeweis M-54') })
        Ziele   = @($B18)
        Laeufe  = @((L 'M-54' $B18 @($T54) ($N1 + 'M-54b.txt')))
    }
    [pscustomobject]@{
        Kennung = 'MUT-55'; Art = 'Rotbeweis'; Zeilen = 'M-55 (a) (Regressionswache)'
        Was     = 'Frame.h fuelleSkalare: Mindestbelegung drei statt vier Rahmen (Muster wie Etappe 5)'
        Aenderungen = @(@{ Datei = $frameH
            Alt = @('            if (h.rahmen >= 4u && ! h.nichtDarstellbar)')
            Neu = @('            if (h.rahmen >= 3u && ! h.nichtDarstellbar)   // MUTATION NAK-283 Rotbeweis M-55: drei statt vier') })
        Ziele   = @($B18)
        Laeufe  = @((L 'M-55 (a)' $B18 @($T55) ($N1 + 'M-55.txt')))
    }

    # ── M-60: sein Fall aendert sich, also alle drei Rotbeweise neu ─────────
    [pscustomobject]@{
        Kennung = 'MUT-49a'; Art = 'Rotbeweis'; Zeilen = 'M-60 True Peak'
        Was     = 'Spektrum.h/FeatureEngine.h: Nachlauf und Filterreset wandern vom Startsample zurueck ans Binden, der Nachlaufwert wird dort verworfen (Muster wie Etappe 5)'
        Aenderungen = @(
            @{ Datei = $spektrumH
               Alt = @(
                   '            const double rest = tp.nachlauf();',
                   '            rahmenTruePeak = std::max (rahmenTruePeak, rest);',
                   '            tp.zuruecksetzen();',
                   '            kurzfensterNeuBeginnen();')
               Neu = @('            kurzfensterNeuBeginnen();   // MUTATION NAK-283 Rotbeweis M-49a: Nachlauf und Reset am Startsample entfernt') }
            @{ Datei = $engineH
               Alt = @(
                   '        passagenStartScharf = true;',
                   '        for (auto& r : headroomRing) r.leeren();')
               Neu = @(
                   '        passagenStartScharf = true;',
                   '        (void) tp.nachlauf();   // MUTATION NAK-283 Rotbeweis M-49a: der Reset wandert wieder ans Binden',
                   '        tp.zuruecksetzen();',
                   '        for (auto& r : headroomRing) r.leeren();') })
        Ziele   = @($B18)
        Laeufe  = @((L 'M-60 True Peak' $B18 @($T60truePeak) ($N1 + 'M-60a.txt')))
    }
    [pscustomobject]@{
        Kennung = 'MUT-56'; Art = 'Rotbeweis'; Zeilen = 'M-60 Loudness'
        Was     = 'Lautheit.h zelleSchliessen: das Zellenmerkmal entfernt - keine Zelle traegt je "enthaelt ersetztes Material" (Muster wie Etappe 5)'
        Aenderungen = @(@{ Datei = $lautheitH
            Alt = @('    kurzZellenErsetzt[(std::size_t) kurzStand] = ersetzt ? 1u : 0u;')
            Neu = @(
                '    kurzZellenErsetzt[(std::size_t) kurzStand] = 0u;   // MUTATION NAK-283 Rotbeweis M-56: Zellenmerkmal entfernt',
                '    (void) ersetzt;') })
        Ziele   = @($B18)
        Laeufe  = @((L 'M-60 Loudness' $B18 @($T60loudness) ($N1 + 'M-60c.txt')))
    }
    [pscustomobject]@{
        Kennung = 'BASIS-E5'; Art = 'Basislauf'; Zeilen = 'M-60 am Basis-SHA der Etappe 5 (alle drei Traeger)'
        Was     = "die fuenf Produktkoepfe FeatureEngine.h, Spektrum.h, Lautheit.h, Frame.h, Vertrag.h byteweise im Stand $basisE5 (Zeilenenden des Arbeitsbaums), die Tests im Endstand"
        Aenderungen = @(
            @{ Datei = $engineH; Basis = $basisE5 }, @{ Datei = $spektrumH; Basis = $basisE5 },
            @{ Datei = $lautheitH; Basis = $basisE5 }, @{ Datei = $frameH; Basis = $basisE5 },
            @{ Datei = $vertragH; Basis = $basisE5 })
        Ziele   = @($B18)
        Laeufe  = @((L 'M-60' $B18 @($T60truePeak, $T60headroom, $T60loudness) ($N1 + 'M-60-basis-e5.txt')))
    }
)

# ── Nur die Muster pruefen (schreibt nichts) ─────────────────────────────
if ($Muster) {
    $falsch = 0
    foreach ($m in $mutationen) {
        foreach ($a in $m.Aenderungen) {
            if ($a.ContainsKey('Basis')) { continue }
            $t = [System.IO.File]::ReadAllText($a.Datei, $utf8)
            $suche = ($a.Alt -join (Zeilenende $t))
            $n = ([regex]::Matches($t, [regex]::Escape($suche))).Count
            if ($n -ne 1) { $falsch++ }
            Write-Host ("{0,-10} {1,-28} {2} Treffer: {3}" -f $m.Kennung, (Split-Path $a.Datei -Leaf), $n, $a.Alt[0].Trim())
        }
    }
    Write-Host "Muster mit falscher Trefferzahl: $falsch"
    if ($falsch -gt 0) { exit 1 }
    exit 0
}

# ── Lauf ──────────────────────────────────────────────────────────────────
$bilanz = @()
foreach ($m in $mutationen) {
    if ($Nur -and $m.Kennung -ne $Nur) { continue }
    Write-Host "=== $($m.Kennung) ($($m.Zeilen)) ===" -ForegroundColor Cyan

    $dateien = @($m.Aenderungen | ForEach-Object { $_.Datei } | Select-Object -Unique)
    $ziele = @($m.Ziele)
    $sicherung = @{}; $hashVorher = @{}; $hashMutiert = @{}; $hashNachher = @{}
    foreach ($d in $dateien) { $sicherung[$d] = [System.IO.File]::ReadAllBytes($d); $hashVorher[$d] = Hash-Von $d }

    $rot = @{}
    try {
        foreach ($a in $m.Aenderungen) { Mutiere $a }
        foreach ($d in $dateien) { $hashMutiert[$d] = Hash-Von $d; (Get-Item -LiteralPath $d).LastWriteTime = Get-Date }
        $bauRot = Baue $ziele
        if ($bauRot.ExitCode -ne 0) { throw "Bau der mutierten Fassung fehlgeschlagen: $($bauRot.Text)" }
        Pruefe-Frische $ziele $dateien 'mutierte'
        foreach ($z in $ziele) { if (@($m.Laeufe | Where-Object { $_.Ziel -eq $z }).Count -gt 0) { $rot[$z] = Fahre $z; Write-Host "  MUTIERT  $z -> Exit $($rot[$z].ExitCode)" } }
    }
    finally {
        foreach ($d in $dateien) { [System.IO.File]::WriteAllBytes($d, $sicherung[$d]) }
    }

    foreach ($d in $dateien) {
        $hashNachher[$d] = Hash-Von $d
        if ($hashNachher[$d] -ne $hashVorher[$d]) { throw "Ruecknahme NICHT bytegleich in $(Relativ $d)" }
        (Get-Item -LiteralPath $d).LastWriteTime = Get-Date
    }
    $bauGruen = Baue $ziele
    if ($bauGruen.ExitCode -ne 0) { throw "Bau der zurueckgespielten Fassung fehlgeschlagen: $($bauGruen.Text)" }
    Pruefe-Frische $ziele $dateien 'zurueckgespielte'
    $gruen = @{}
    foreach ($z in $ziele) { if ($rot.ContainsKey($z)) { $gruen[$z] = Fahre $z; Write-Host "  ZURUECK  $z -> Exit $($gruen[$z].ExitCode)" } }

    foreach ($l in $m.Laeufe) {
        $r = $rot[$l.Ziel]; $g = $gruen[$l.Ziel]
        $tr = @(Traeger-Zeilen $r.Text '[ROT]' $l.Traeger)
        $tg = @(Traeger-Zeilen $g.Text '[ok]' $l.Traeger)
        $alleRot = @($tr | Where-Object { -not $_.Zeile }).Count -eq 0
        $alleOk  = @($tg | Where-Object { -not $_.Zeile }).Count -eq 0
        $hashGleich = @($dateien | Where-Object { $hashVorher[$_] -ne $hashNachher[$_] }).Count -eq 0
        $ok = ($r.ExitCode -ne 0) -and $alleRot -and ($g.ExitCode -eq 0) -and $alleOk -and $hashGleich

        $kopf = @(
            "NAK-283 Etappe 5 Nacharbeit 1 - $($m.Art) $($m.Kennung) fuer $($l.Zeile)",
            "Zeitpunkt:  $(Get-Date -Format o)",
            "HEAD:       $(git -C $Wurzel rev-parse HEAD)",
            "Mutation:   $($m.Was)",
            "Erwartet:   mutiert rot (Exit != 0 und [ROT] an jedem Traeger) / zurueckgespielt gruen (Exit 0 und [ok] an jedem Traeger)")
        foreach ($d in $dateien) {
            $kopf += "Datei:      $(Relativ $d)"
            $kopf += "  SHA-256 vorher:   $($hashVorher[$d])"
            $kopf += "  SHA-256 mutiert:  $($hashMutiert[$d])"
            $kopf += "  SHA-256 nachher:  $($hashNachher[$d]) (bytegleiche Ruecknahme: $($hashVorher[$d] -eq $hashNachher[$d]))"
        }
        $kopf += @(
            "Bein:       $($l.Ziel)",
            "Neubau:     mutiert $($ziele -join ' ') / zurueckgespielt $($ziele -join ' ') (Binaries juenger als jede mutierte Datei: geprueft)",
            "Urteil:     MUTIERT Exit $($r.ExitCode) / ZURUECKGESPIELT Exit $($g.ExitCode) / $(if ($ok) { 'BESTANDEN' } else { 'NICHT BESTANDEN' })")
        foreach ($e in $tr) { $kopf += "Traeger mutiert:        $(if ($e.Zeile) { $e.Zeile } else { 'FEHLT: ' + $e.Traeger })" }
        foreach ($e in $tg) { $kopf += "Traeger zurueckgespielt: $(if ($e.Zeile) { $e.Zeile } else { 'FEHLT: ' + $e.Traeger })" }
        $kopf += @(
            '',
            '--- MUTIERT (erwartet: rot) ----------------------------------------',
            $r.Text,
            '',
            '--- ZURUECKGESPIELT (erwartet: gruen) -----------------------------',
            $g.Text)
        [System.IO.File]::WriteAllLines((Join-Path $rohOrdner $l.Datei), $kopf, $utf8)
        Write-Host ("  {0,-28} {1}" -f $l.Zeile, $(if ($ok) { 'BESTANDEN' } else { 'NICHT BESTANDEN' }))
        $bilanz += [pscustomobject]@{
            Kennung = $m.Kennung; Zeile = $l.Zeile; Datei = $l.Datei
            Mutiert = $r.ExitCode; TraegerRot = "$(@($tr | Where-Object { $_.Zeile }).Count)/$($tr.Count)"
            Zurueck = $g.ExitCode; TraegerOk = "$(@($tg | Where-Object { $_.Zeile }).Count)/$($tg.Count)"
            HashGleich = $hashGleich; Ok = $ok
        }
    }
}

$zeilen = @('NAK-283 Etappe 5 Nacharbeit 1 - Rotbilanz', "Zeitpunkt: $(Get-Date -Format o)", "HEAD: $(git -C $Wurzel rev-parse HEAD)", "Basis Nacharbeit 1: $basisN1", "Basis Etappe 5: $basisE5", '')
foreach ($b in $bilanz) {
    $zeilen += ("{0,-10} {1,-28} mutiert Exit {2} (ROT-Traeger {3}) / zurueck Exit {4} (ok-Traeger {5}), Hash gleich {6}  {7}  {8}" -f `
        $b.Kennung, $b.Zeile, $b.Mutiert, $b.TraegerRot, $b.Zurueck, $b.TraegerOk, $b.HashGleich, $b.Datei, $(if ($b.Ok) { 'BESTANDEN' } else { 'NICHT BESTANDEN' }))
}
$schlecht = @($bilanz | Where-Object { -not $_.Ok })
$zeilen += ''
$zeilen += ("URTEIL: {0} von {1} Laeufen bestanden (mutiert Exit != 0 mit [ROT] an jedem Traeger, zurueckgespielt Exit 0 mit [ok] an jedem Traeger, Ruecknahme bytegleich)" -f ($bilanz.Count - $schlecht.Count), $bilanz.Count)
if (-not $Nur) { [System.IO.File]::WriteAllLines((Join-Path $rohOrdner 'NAK-283-etappe-5-nacharbeit-1-rotbilanz.txt'), $zeilen, $utf8) }
$zeilen | ForEach-Object { Write-Host $_ }
if ($schlecht.Count -gt 0) { exit 1 }
exit 0
