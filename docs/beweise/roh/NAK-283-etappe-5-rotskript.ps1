# NAK-283 Etappe 5 — Rotbeweise je Matrixzeile M-44 bis M-60, M-73, M-74 (Bauer, 13.09.2026)
#
# Ablauf je Mutation nach docs/beweise/NAK-283.md §6.1:
#   1. Originalbytes und SHA-256 JEDER Quelldatei der Mutation sichern
#   2. Mutation genau an der Zeile, die die Zusage traegt - jedes Muster genau
#      einmal (sonst Abbruch), die Zeilenenden der Datei bleiben. Ein
#      BASISLAUF ersetzt statt eines Musters die ganze Datei durch ihren Stand
#      am Basis-SHA (git-Blob, Zeilenenden wie im Arbeitsbaum).
#   3. LastWriteTime setzen (NAK-230), Neubau der betroffenen Ziele; jedes
#      Binary muss juenger sein als die mutierte Quelle
#   4. Bein fahren -> ein Rotlauf zaehlt nur mit Exit != 0 UND einer
#      [ROT]-Zeile, die den Traeger der Zusage nennt (nie ein Nebeneffekt).
#      Eine BEOBACHTUNG (Erwartet = gruen) zaehlt nur mit einer [ok]-Zeile
#      desselben Traegers im mutierten Stand - die woertliche Matrixmutation,
#      die nicht rot macht, wird so belegt statt verschwiegen (§36.6 Abw. 1).
#   5. Originalbytes zurueck (nie eine zweite Textersetzung), SHA-256 gegen
#      Schritt 1, Zeitstempel, Neubau, Bein fahren -> GRUEN mit Exit 0 UND
#      einer [ok]-Zeile desselben Traegers
#
# Keine Zeile dieser Etappe steht in der Nebenlaeufigkeitsliste aus §6.1:
# jede Buehne ist ein deterministischer Sampleplan auf dem Worker-Pfad der
# Engine, ohne Thread und ohne Uhr. Regressionswachen (M-48, M-55, M-59
# Saettigung, M-73) werden absichtlich gebrochen und sind kein Beleg fuer
# ihren Befund.
#
# Aufruf aus dem Workspace-Root:
#   pwsh -File docs/beweise/roh/NAK-283-etappe-5-rotskript.ps1
#   pwsh -File docs/beweise/roh/NAK-283-etappe-5-rotskript.ps1 -Nur MUT-44
#   pwsh -File docs/beweise/roh/NAK-283-etappe-5-rotskript.ps1 -Muster   (nur Trefferzahl je Muster, schreibt nichts)
# Rohausgaben: docs/beweise/roh/NAK-283-rot-M-<nn>-etappe-5.txt,
# Bilanz:      docs/beweise/roh/NAK-283-etappe-5-rotbilanz.txt (nur beim vollen Lauf)

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
$basis     = 'bc0941e51bcdf8d3752362c3ca10cda628bd1438'

function Exe-Pfad([string] $ziel) { Join-Path $build ("plugin\{0}_artefacts\Release\{0}.exe" -f $ziel) }
function Zeilenende([string] $text) { if ($text -match "`r`n") { "`r`n" } else { "`n" } }
function Hash-Von([string] $datei) { (Get-FileHash -Algorithm SHA256 $datei).Hash }
function Relativ([string] $pfad) { $pfad.Replace($Wurzel + '\', '').Replace('\', '/') }

function Mutiere($aenderung) {
    $datei = $aenderung.Datei
    $t = [System.IO.File]::ReadAllText($datei)
    $le = Zeilenende $t
    if ($aenderung.ContainsKey('Basis')) {
        # Der Stand am Basis-SHA, byteweise aus dem Blob, mit den Zeilenenden
        # des Arbeitsbaums (core.autocrlf) - dieselben Bytes wie ein Checkout.
        $tmp = Join-Path ([System.IO.Path]::GetTempPath()) ("nak283-basis-" + [guid]::NewGuid().ToString('N') + '.txt')
        # cmd leitet byteweise um; die Pfade tragen kein Leerzeichen (geprueft),
        # also braucht der Befehlstext keine inneren Anfuehrungszeichen.
        if ($Wurzel.Contains(' ') -or $tmp.Contains(' ')) { throw "Pfad mit Leerzeichen: $Wurzel / $tmp" }
        & cmd /c ("git -C {0} show {1}:{2} > {3}" -f $Wurzel, $basis, (Relativ $datei), $tmp)
        if ($LASTEXITCODE -ne 0) { throw "git show am Basis-SHA gescheitert: $(Relativ $datei)" }
        $blob = [System.IO.File]::ReadAllText($tmp, $utf8)
        [System.IO.File]::Delete($tmp)
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
    if ($ziele.Count -eq 0) { return @{ ExitCode = 0; Text = '(kein Bauziel)' } }
    $argumente = @('--build', $build, '--config', 'Release', '--target') + $ziele
    $aus = & $cmake @argumente 2>&1
    return @{ ExitCode = $LASTEXITCODE; Text = ($aus -join "`n") }
}

function Fahre($lauf) {
    $argumente = @()
    if ($lauf.ContainsKey('Arg')) { $argumente = $lauf.Arg }
    if ($lauf.ContainsKey('Skript')) {
        $aus = & pwsh -NoProfile -File (Join-Path $Wurzel $lauf.Skript) @argumente 2>&1
    } else {
        $aus = & (Exe-Pfad $lauf.Ziel) @argumente 2>&1
    }
    return @{ ExitCode = $LASTEXITCODE; Text = ($aus -join "`n") }
}

function Traeger-Zeile([string] $text, [string] $marke, [string] $traeger) {
    foreach ($z in ($text -split "`n")) {
        $zt = $z.Trim()
        if ($zt.StartsWith($marke) -and $zt.Contains($traeger)) { return $zt }
    }
    return ''
}

# ── Die Quelldateien ──────────────────────────────────────────────────────
$engineH   = Join-Path $Wurzel 'eq-copilot\plugin\core\analysis\FeatureEngine.h'
$spektrumH = Join-Path $Wurzel 'eq-copilot\plugin\core\analysis\featureengine\Spektrum.h'
$lautheitH = Join-Path $Wurzel 'eq-copilot\plugin\core\analysis\featureengine\Lautheit.h'
$frameH    = Join-Path $Wurzel 'eq-copilot\plugin\core\analysis\featureengine\Frame.h'
$vertragH  = Join-Path $Wurzel 'eq-copilot\plugin\core\analysis\featureengine\Vertrag.h'
$riegel55  = 'docs/beweise/roh/NAK-283-etappe-5-riegel-M-55.ps1'

$B16 = 'EqCopSonde013EventWireTest'
$B18 = 'EqCopSonde013DynamicsTest'

function L([string] $zeile, [string] $kuerzel, [string] $ziel, [string] $traeger, [string] $datei, [string] $erwartet = 'rot') {
    @{ Zeile = $zeile; Kuerzel = $kuerzel; Ziel = $ziel; Traeger = $traeger; Datei = $datei; Erwartet = $erwartet }
}

# ── Die Mutationen ────────────────────────────────────────────────────────
$mutationen = @(
    # ── F06 (R-283-4): das Gate, Spektrum.h ────────────────────────────────
    [pscustomobject]@{
        Kennung = 'MUT-44'; Zeilen = 'M-44, M-45 (a), M-46, M-47'
        Was     = 'Spektrum.h rechneFenster: Δf aus der Gate-Rechnung entfernt - die ungeteilte Dichtesumme steht gegen kAktivGateDb (Stand am Basis-SHA)'
        Aenderungen = @(@{ Datei = $spektrumH
            Alt = @(
                '    const double binBreiteHz = s.fs / (double) s.punkte;',
                '    const double gesamt = summeBereich (s, 0, bins) * binBreiteHz;')
            Neu = @('    const double gesamt = summeBereich (s, 0, bins);   // MUTATION NAK-283 Rotbeweis M-44: Δf aus der Gate-Rechnung entfernt') })
        Ziele   = @($B16)
        Laeufe  = @(
            (L 'M-44' 'B16' $B16 'NAK-283 M-44: aktivgate_ist_samplerateunabhaengig' 'NAK-283-rot-M-44-etappe-5.txt'),
            (L 'M-45 (a)' 'B16' $B16 'bei L1 = -52,3 dBFS nehmen BEIDE Stufen' 'NAK-283-rot-M-45a-etappe-5.txt'),
            (L 'M-46' 'B16' $B16 '(a) -58,0 dBFS bei 44,1 kHz: aktivitaet 1 UND abdeckung 1' 'NAK-283-rot-M-46-etappe-5.txt'),
            (L 'M-47' 'B16' $B16 'NAK-283 M-47: aktives_fenster_liefert_bandwerte' 'NAK-283-rot-M-47-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-45'; Zeilen = 'M-45 (b)'
        Was     = 'Spektrum.h rechneFenster: Δf fest aus kHauptPunkte statt aus s.punkte - die Bassstufe steht 6,02 dB zu hoch'
        Aenderungen = @(@{ Datei = $spektrumH
            Alt = @('    const double binBreiteHz = s.fs / (double) s.punkte;')
            Neu = @('    const double binBreiteHz = s.fs / (double) kHauptPunkte;   // MUTATION NAK-283 Rotbeweis M-45 (b): Δf fest aus kHauptPunkte') })
        Ziele   = @($B16)
        Laeufe  = @((L 'M-45 (b)' 'B16' $B16 'bei L2 = -63,0 dBFS nimmt KEINE Stufe' 'NAK-283-rot-M-45b-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-48'; Zeilen = 'M-48 (Regressionswache, Matrixmutation woertlich)'
        Was     = 'Spektrum.h rechneFenster: die Nullpruefung vor dem Logarithmus entfaellt (Rotbeweisspalte woertlich) - BEOBACHTUNG erwartet: log10(+0) = -inf, und -inf > -60 ist nach IEEE 754 falsch; die Stille bleibt inaktiv'
        Aenderungen = @(@{ Datei = $spektrumH
            Alt = @(
                '    const bool aktiv = gesamt > 0.0',
                '                    && 10.0 * std::log10 (gesamt) > kAktivGateDb;')
            Neu = @('    const bool aktiv = 10.0 * std::log10 (gesamt) > kAktivGateDb;   // MUTATION NAK-283 Rotbeweis M-48: Nullpruefung entfaellt') })
        Ziele   = @($B16)
        Laeufe  = @(
            (L 'M-48 (a) Beobachtung' 'B16' $B16 '(a) digitale Stille: abdeckung 0, endlich' 'NAK-283-rot-M-48-beobachtung-etappe-5.txt' 'gruen'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-48b'; Zeilen = 'M-48 (Regressionswache)'
        Was     = 'Spektrum.h rechneFenster: die Nullpruefung als Freigabe - eine Dichtesumme von exakt 0 zaehlt als aktiv'
        Aenderungen = @(@{ Datei = $spektrumH
            Alt = @(
                '    const bool aktiv = gesamt > 0.0',
                '                    && 10.0 * std::log10 (gesamt) > kAktivGateDb;')
            Neu = @(
                '    const bool aktiv = gesamt >= 0.0   // MUTATION NAK-283 Rotbeweis M-48b: Dichtesumme 0 zaehlt als aktiv',
                '                    && (gesamt == 0.0 || 10.0 * std::log10 (gesamt) > kAktivGateDb);') })
        Ziele   = @($B16)
        Laeufe  = @(
            (L 'M-48 (a)' 'B16' $B16 '(a) digitale Stille: abdeckung 0, endlich' 'NAK-283-rot-M-48a-etappe-5.txt'),
            (L 'M-48 (b)' 'B16' $B16 '(b) Fenster mit Dichtesumme exakt 0 nach einer Sekunde Ton' 'NAK-283-rot-M-48b-etappe-5.txt'))
    }

    # ── F07 (R-283-5): der Startmerker, Spektrum.h / FeatureEngine.h ───────
    [pscustomobject]@{
        Kennung = 'MUT-49a'; Zeilen = 'M-49 (a), M-50, M-51 (A), M-52 Anfang, M-60 True Peak, M-74'
        Was     = 'Spektrum.h/FeatureEngine.h: Nachlauf und Filterreset wandern vom Startsample zurueck ans Binden, der Nachlaufwert wird dort verworfen (Stand am Basis-SHA)'
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
        Ziele   = @($B16, $B18)
        Laeufe  = @(
            (L 'M-49 (a)' 'B16' $B16 'NAK-283 M-49: vorab_gebundene_passage_traegt_kein_material_davor' 'NAK-283-rot-M-49-etappe-5.txt'),
            (L 'M-50' 'B16' $B16 'NAK-283 M-50: zwoelf_samples_vor_dem_fensteranfang_gehen_nicht_ein' 'NAK-283-rot-M-50-etappe-5.txt'),
            (L 'M-51 (A)' 'B16' $B16 'M-51 (A): nachlauf_beim_binden_geht_in_die_livemetrik_nicht_in_die_passage' 'NAK-283-rot-M-51A-etappe-5.txt'),
            (L 'M-52 Anfang' 'B16' $B16 'True-Peak-Kette am Anfang und hinter dem Ende' 'NAK-283-rot-M-52a-etappe-5.txt'),
            (L 'M-74' 'B16' $B16 'NAK-283 M-74: passagenmaximum_gleicht_dem_isolierten_lauf' 'NAK-283-rot-M-74-etappe-5.txt'),
            (L 'M-60 True Peak' 'B18' $B18 'True-Peak-Kette: das Passagenmaximum ist das der Passage' 'NAK-283-rot-M-60a-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-49b'; Zeilen = 'M-49 (b), M-73 (Regressionswache)'
        Was     = 'Spektrum.h verarbeiteSamples: Startmerker weggelassen - Reset bei jedem i == passVon, also an jedem Blockanfang in der Passage'
        Aenderungen = @(@{ Datei = $spektrumH
            Alt = @('        if (fensterBeginntHier && i == passVon)')
            Neu = @('        if (passVon < passBis && i == passVon)   // MUTATION NAK-283 Rotbeweis M-49b/M-73: ohne Startmerker, an jedem Blockanfang') })
        Ziele   = @($B16)
        Laeufe  = @((L 'M-73' 'B16' $B16 'NAK-283 M-73: passagenmaximum_haengt_nicht_an_der_blockgroesse' 'NAK-283-rot-M-73-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-50'; Zeilen = 'M-50'
        Was     = 'Spektrum.h verarbeiteSamples: der Reset um ein Sample nach hinten verschoben (i == passVon + 1)'
        Aenderungen = @(@{ Datei = $spektrumH
            Alt = @('        if (fensterBeginntHier && i == passVon)')
            Neu = @('        if (fensterBeginntHier && i == passVon + 1)   // MUTATION NAK-283 Rotbeweis M-50: ein Sample zu spaet') })
        Ziele   = @($B16)
        Laeufe  = @((L 'M-50' 'B16' $B16 'NAK-283 M-50: zwoelf_samples_vor_dem_fensteranfang_gehen_nicht_ein' 'NAK-283-rot-M-50b-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-51a'; Zeilen = 'M-51 (B), erste Richtung'
        Was     = 'Spektrum.h verarbeiteSamples: der Nachlauf am Startsample wird wieder verworfen'
        Aenderungen = @(@{ Datei = $spektrumH
            Alt = @(
                '            rahmenTruePeak = std::max (rahmenTruePeak, rest);',
                '            tp.zuruecksetzen();',
                '            kurzfensterNeuBeginnen();')
            Neu = @(
                '            (void) rest;   // MUTATION NAK-283 Rotbeweis M-51a: der Nachlauf am Startsample wird verworfen',
                '            tp.zuruecksetzen();',
                '            kurzfensterNeuBeginnen();') })
        Ziele   = @($B16)
        Laeufe  = @((L 'M-51 (B) Rahmen' 'B16' $B16 'am Startsample geht der Nachlauf in den laufenden Rahmen' 'NAK-283-rot-M-51Ba-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-51b'; Zeilen = 'M-51 (B), zweite Richtung'
        Was     = 'Spektrum.h verarbeiteSamples: der Nachlauf am Startsample wird zusaetzlich dem Passagenmaximum zugeschlagen'
        Aenderungen = @(@{ Datei = $spektrumH
            Alt = @(
                '            rahmenTruePeak = std::max (rahmenTruePeak, rest);',
                '            tp.zuruecksetzen();')
            Neu = @(
                '            rahmenTruePeak = std::max (rahmenTruePeak, rest);',
                '            passagenTruePeakRahmen = std::max (passagenTruePeakRahmen, rest);   // MUTATION NAK-283 Rotbeweis M-51b',
                '            tp.zuruecksetzen();') })
        Ziele   = @($B16)
        Laeufe  = @((L 'M-51 (B) Passage' 'B16' $B16 'und NICHT in das Passagenmaximum' 'NAK-283-rot-M-51Bb-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-52b'; Zeilen = 'M-52 Ende (True-Peak-Kette)'
        Was     = 'Spektrum.h verarbeiteSamples: der Nachlauf an passBis zaehlt nur noch fuer den Rahmen - die Endgrenze fuer die True-Peak-Kette ausgelassen'
        Aenderungen = @(@{ Datei = $spektrumH
            Alt = @(
                '            passagenTruePeakRahmen = std::max (passagenTruePeakRahmen, rest);',
                '            fensterEndetHier = false;   // genau EINMAL je Block')
            Neu = @('            fensterEndetHier = false;   // genau EINMAL je Block   MUTATION NAK-283 Rotbeweis M-52b: Nachlauf an passBis nicht fuer die Passage') })
        Ziele   = @($B16)
        Laeufe  = @((L 'M-52 Ende' 'B16' $B16 'True-Peak-Kette am Ende: der Nachlauf an passBis' 'NAK-283-rot-M-52b-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-52c'; Zeilen = 'M-52 Anfang (3-s-Zellen)'
        Was     = 'Spektrum.h verarbeiteSamples: das Kurzzeitfenster beginnt am Startsample NICHT neu - die Anfangsgrenze fuer die 3-s-Zellen ausgelassen'
        Aenderungen = @(@{ Datei = $spektrumH
            Alt = @(
                '            tp.zuruecksetzen();',
                '            kurzfensterNeuBeginnen();',
                '            passagenStartScharf = false;')
            Neu = @(
                '            tp.zuruecksetzen();',
                '            // MUTATION NAK-283 Rotbeweis M-52c: kein Neubeginn des Kurzzeitfensters am Startsample',
                '            passagenStartScharf = false;') })
        Ziele   = @($B16)
        Laeufe  = @((L 'M-52 3-s-Zellen' 'B16' $B16 '3-s-Zellen: das Kurzzeitfenster beginnt am Passagenanfang' 'NAK-283-rot-M-52c-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-52d'; Zeilen = 'M-52 Anfang und Ende (Headroom)'
        Was     = 'Frame.h baueFrame: Rahmen vor dem Anfang und hinter dem Ende einer gebundenen Passage gehen mit ihrem Rahmen-True-Peak in die Headroomverteilung ein'
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
        Laeufe  = @((L 'M-52 Headroom' 'B16' $B16 'Headroom: die Verteilung umfasst genau die Rahmen der Passage' 'NAK-283-rot-M-52d-etappe-5.txt'))
    }

    # ── F08 (R-283-5, Feinheit 7): die Headroomverteilung, Frame.h ─────────
    [pscustomobject]@{
        Kennung = 'MUT-53'; Zeilen = 'M-53, M-54, M-60 Headroom, M-52 Headroom'
        Was     = 'Frame.h baueFrame/fuelleSkalare: die Verteilung wieder aus dem 64er-Ring, auch mit gebundener Passage (Stand am Basis-SHA)'
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
            (L 'M-53' 'B18' $B18 'NAK-283 M-53: headroomverteilung_umfasst_die_ganze_passage' 'NAK-283-rot-M-53-etappe-5.txt'),
            (L 'M-54' 'B18' $B18 'NAK-283 M-54: headroomfenster_nennt_die_passagenlaenge' 'NAK-283-rot-M-54-etappe-5.txt'),
            (L 'M-60 Headroom' 'B18' $B18 'Headroom: die Verteilung beschreibt beide Haelften der ganzen Passage' 'NAK-283-rot-M-60b-etappe-5.txt'),
            (L 'M-52 Headroom (Ring)' 'B16' $B16 'Headroom: die Verteilung umfasst genau die Rahmen der Passage' 'NAK-283-rot-M-52e-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-54'; Zeilen = 'M-54'
        Was     = 'Frame.h fuelleSkalare: headroomFenster nennt hoechstens die Ringbelegung, waehrend die Perzentile die Passage beschreiben'
        Aenderungen = @(@{ Datei = $frameH
            Alt = @('                f.headroomFenster = h.rahmen;')
            Neu = @('                f.headroomFenster = std::min<std::uint32_t> (h.rahmen, (std::uint32_t) kVerteilungPlaetze);   // MUTATION NAK-283 Rotbeweis M-54') })
        Ziele   = @($B18)
        Laeufe  = @((L 'M-54' 'B18' $B18 'NAK-283 M-54: headroomfenster_nennt_die_passagenlaenge' 'NAK-283-rot-M-54b-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-55'; Zeilen = 'M-55 (a) (Regressionswache)'
        Was     = 'Frame.h fuelleSkalare: Mindestbelegung drei statt vier Rahmen'
        Aenderungen = @(@{ Datei = $frameH
            Alt = @('            if (h.rahmen >= 4u && ! h.nichtDarstellbar)')
            Neu = @('            if (h.rahmen >= 3u && ! h.nichtDarstellbar)   // MUTATION NAK-283 Rotbeweis M-55: drei statt vier') })
        Ziele   = @($B18)
        Laeufe  = @((L 'M-55 (a)' 'B18' $B18 'NAK-283 M-55: headroom_unter_vier_rahmen_bleibt_ungesetzt' 'NAK-283-rot-M-55-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-55b'; Zeilen = 'M-55 (b) (Riegel)'
        Was     = 'FeatureEngine.h: kVerteilungPlaetze = 65 - die Ressourcengrenze der Bandverteilungen steigt'
        Aenderungen = @(@{ Datei = $engineH
            Alt = @('inline constexpr int kVerteilungPlaetze = 64;')
            Neu = @('inline constexpr int kVerteilungPlaetze = 65;   // MUTATION NAK-283 Rotbeweis M-55 (b)') })
        Ziele   = @()
        Laeufe  = @(@{ Zeile = 'M-55 (b)'; Kuerzel = 'Riegel'; Skript = $riegel55; Traeger = 'R2 die Definition lautet kVerteilungPlaetze = 64'; Datei = 'NAK-283-rot-M-55b-etappe-5.txt'; Erwartet = 'rot' })
    }

    # ── F10 (R-283-5, Feinheit 8): Gueltigkeit je Zelle, Lautheit.h ────────
    [pscustomobject]@{
        Kennung = 'MUT-56'; Zeilen = 'M-56, M-57, M-60 Loudness'
        Was     = 'Lautheit.h zelleSchliessen: das Zellenmerkmal entfernt - keine Zelle traegt je "enthaelt ersetztes Material" (Stand am Basis-SHA)'
        Aenderungen = @(@{ Datei = $lautheitH
            Alt = @('    kurzZellenErsetzt[(std::size_t) kurzStand] = ersetzt ? 1u : 0u;')
            Neu = @(
                '    kurzZellenErsetzt[(std::size_t) kurzStand] = 0u;   // MUTATION NAK-283 Rotbeweis M-56: Zellenmerkmal entfernt',
                '    (void) ersetzt;') })
        Ziele   = @($B18)
        Laeufe  = @(
            (L 'M-56' 'B18' $B18 'der erste saubere Rahmen traegt kein Short-term' 'NAK-283-rot-M-56-etappe-5.txt'),
            (L 'M-57' 'B18' $B18 'bei 0,4 s traegt derselbe Frame Momentary mit und Short-term ohne' 'NAK-283-rot-M-57-etappe-5.txt'),
            (L 'M-60 Loudness' 'B18' $B18 'Loudnesszellen: Momentary kommt 0,4 s, Short-term 3,0 s' 'NAK-283-rot-M-60c-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-57'; Zeilen = 'M-57'
        Was     = 'Lautheit.h momentanLufs: Momentary an die 3-s-Frist gehaengt (fragt die Marken aller 30 Zellen)'
        Aenderungen = @(@{ Datei = $lautheitH
            Alt = @('    if (! zellenFensterSauber (kMomentZellen))')
            Neu = @('    if (! zellenFensterSauber (kKurzZellen))   // MUTATION NAK-283 Rotbeweis M-57: dieselbe Frist wie Short-term') })
        Ziele   = @($B18)
        Laeufe  = @((L 'M-57' 'B18' $B18 'bei 0,4 s traegt derselbe Frame Momentary mit und Short-term ohne' 'NAK-283-rot-M-57b-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-58a'; Zeilen = 'M-58 (a)'
        Was     = 'Lautheit.h kurzLufs: die Verriegelung lebt eine Zelle laenger als das 3-s-Fenster'
        Aenderungen = @(@{ Datei = $lautheitH
            Alt = @(
                '    if (! zellenFensterSauber (kKurzZellen))',
                '        return false;',
                '    double su = 0.0;')
            Neu = @(
                '    if (! zellenFensterSauber (kKurzZellen))',
                '        return false;',
                '    // MUTATION NAK-283 Rotbeweis M-58a: die Verriegelung lebt eine Zelle laenger als das Fenster',
                '    if (letztesErsetztesSample > 0u && verarbeiteteSamples - letztesErsetztesSample < (std::uint64_t) ((kKurzZellen + 1) * zellenSamples))',
                '        return false;',
                '    double su = 0.0;') })
        Ziele   = @($B18)
        Laeufe  = @((L 'M-58 (a)' 'B18' $B18 'Short-term - bei 2,9 s ungesetzt, bei 3,0 s gesetzt' 'NAK-283-rot-M-58a-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-58b'; Zeilen = 'M-58 (b)'
        Was     = 'Lautheit.h kurzLufs/momentanLufs: die Fenster fragen die Zellenmarken nicht - die Verriegelung lebt nur noch als Rahmenlatch (Stand am Basis-SHA)'
        Aenderungen = @(
            @{ Datei = $lautheitH
               Alt = @(
                   '    if (! zellenFensterSauber (kKurzZellen))',
                   '        return false;',
                   '    double su = 0.0;')
               Neu = @('    double su = 0.0;   // MUTATION NAK-283 Rotbeweis M-58b: kurzLufs ohne Zellenmarken') }
            @{ Datei = $lautheitH
               Alt = @(
                   '    if (! zellenFensterSauber (kMomentZellen))',
                   '        return false;')
               Neu = @('    // MUTATION NAK-283 Rotbeweis M-58b: momentanLufs ohne Zellenmarken') })
        Ziele   = @($B18)
        Laeufe  = @(
            (L 'M-58 (b) Momentary' 'B18' $B18 'Momentary - bei 0,3 s nach dem Ende der Ersetzung ungesetzt' 'NAK-283-rot-M-58b-etappe-5.txt'),
            (L 'M-58 (b) Short-term' 'B18' $B18 'Short-term - bei 2,9 s ungesetzt, bei 3,0 s gesetzt' 'NAK-283-rot-M-58c-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-59a'; Zeilen = 'M-59 Saettigung (Regressionswache)'
        Was     = 'Spektrum.h verarbeiteSamples: die Saettigungspruefung vor beiden Inkrementen entfernt - die Zaehler wrappen'
        Aenderungen = @(@{ Datei = $spektrumH
            Alt = @(
                '            if (rahmenNichtEndlich < 0xFFFFFFFFu) ++rahmenNichtEndlich;',
                '            if (evidenzNichtEndlich < 0xFFFFFFFFu) ++evidenzNichtEndlich;')
            Neu = @(
                '            ++rahmenNichtEndlich;    // MUTATION NAK-283 Rotbeweis M-59a: ohne Saettigung',
                '            ++evidenzNichtEndlich;') })
        Ziele   = @($B18)
        Laeufe  = @((L 'M-59 Saettigung' 'B18' $B18 'Regressionswache fuer die Saettigung' 'NAK-283-rot-M-59a-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-59b'; Zeilen = 'M-59 Gueltigkeitsmarke'
        Was     = 'Lautheit.h zelleSchliessen: die Gueltigkeitsmarke aus dem Zaehlerstand abgeleitet - gesetzt nur, wenn rahmenNichtEndlich > 0 und nicht am Anschlag'
        Aenderungen = @(@{ Datei = $lautheitH
            Alt = @('    const bool ersetzt = letztesErsetztesSample > zellenBeginn;')
            Neu = @(
                '    const bool ersetzt = rahmenNichtEndlich > 0u && rahmenNichtEndlich != 0xFFFFFFFFu;   // MUTATION NAK-283 Rotbeweis M-59b: Marke aus dem Zaehlerstand',
                '    (void) zellenBeginn;') })
        Ziele   = @($B18)
        Laeufe  = @((L 'M-59 Marke' 'B18' $B18 'die Zellenmarke steht, obwohl der Zaehler nicht mehr gestiegen ist' 'NAK-283-rot-M-59b-etappe-5.txt'))
    }

    # ── Basislauf: die fuenf Produktkoepfe am Basis-SHA, die neuen Tests ────
    [pscustomobject]@{
        Kennung = 'BASIS'; Zeilen = 'alle Zeilen der Etappe am Basis-SHA'
        Was     = "die fuenf Produktkoepfe FeatureEngine.h, Spektrum.h, Lautheit.h, Frame.h, Vertrag.h byteweise im Stand $basis (Zeilenenden des Arbeitsbaums), die Tests im Etappenstand"
        Aenderungen = @(
            @{ Datei = $engineH; Basis = $true }, @{ Datei = $spektrumH; Basis = $true },
            @{ Datei = $lautheitH; Basis = $true }, @{ Datei = $frameH; Basis = $true },
            @{ Datei = $vertragH; Basis = $true })
        Ziele   = @($B16, $B18)
        Laeufe  = @(
            (L 'M-44' 'B16' $B16 'NAK-283 M-44: aktivgate_ist_samplerateunabhaengig' 'NAK-283-rot-M-44-basis-etappe-5.txt'),
            (L 'M-45 (a)' 'B16' $B16 'bei L1 = -52,3 dBFS nehmen BEIDE Stufen' 'NAK-283-rot-M-45-basis-etappe-5.txt'),
            (L 'M-46' 'B16' $B16 '(a) -58,0 dBFS bei 44,1 kHz: aktivitaet 1 UND abdeckung 1' 'NAK-283-rot-M-46-basis-etappe-5.txt'),
            (L 'M-47' 'B16' $B16 'NAK-283 M-47: aktives_fenster_liefert_bandwerte' 'NAK-283-rot-M-47-basis-etappe-5.txt'),
            (L 'M-48 (Wache)' 'B16' $B16 '(a) digitale Stille: abdeckung 0, endlich' 'NAK-283-rot-M-48-basis-etappe-5.txt' 'gruen'),
            (L 'M-49' 'B16' $B16 'NAK-283 M-49: vorab_gebundene_passage_traegt_kein_material_davor' 'NAK-283-rot-M-49-basis-etappe-5.txt'),
            (L 'M-50' 'B16' $B16 'NAK-283 M-50: zwoelf_samples_vor_dem_fensteranfang_gehen_nicht_ein' 'NAK-283-rot-M-50-basis-etappe-5.txt'),
            (L 'M-51 (A)' 'B16' $B16 'M-51 (A): nachlauf_beim_binden_geht_in_die_livemetrik_nicht_in_die_passage' 'NAK-283-rot-M-51-basis-etappe-5.txt'),
            (L 'M-52 Anfang' 'B16' $B16 'True-Peak-Kette am Anfang und hinter dem Ende' 'NAK-283-rot-M-52-basis-etappe-5.txt'),
            (L 'M-73 (Wache)' 'B16' $B16 'NAK-283 M-73: passagenmaximum_haengt_nicht_an_der_blockgroesse' 'NAK-283-rot-M-73-basis-etappe-5.txt' 'gruen'),
            (L 'M-74' 'B16' $B16 'NAK-283 M-74: passagenmaximum_gleicht_dem_isolierten_lauf' 'NAK-283-rot-M-74-basis-etappe-5.txt'),
            (L 'M-53' 'B18' $B18 'NAK-283 M-53: headroomverteilung_umfasst_die_ganze_passage' 'NAK-283-rot-M-53-basis-etappe-5.txt'),
            (L 'M-54' 'B18' $B18 'NAK-283 M-54: headroomfenster_nennt_die_passagenlaenge' 'NAK-283-rot-M-54-basis-etappe-5.txt'),
            (L 'M-55 (Wache)' 'B18' $B18 'NAK-283 M-55: headroom_unter_vier_rahmen_bleibt_ungesetzt' 'NAK-283-rot-M-55-basis-etappe-5.txt' 'gruen'),
            (L 'M-56' 'B18' $B18 'der erste saubere Rahmen traegt kein Short-term' 'NAK-283-rot-M-56-basis-etappe-5.txt'),
            (L 'M-57' 'B18' $B18 'bei 0,4 s traegt derselbe Frame Momentary mit und Short-term ohne' 'NAK-283-rot-M-57-basis-etappe-5.txt'),
            (L 'M-58 (b)' 'B18' $B18 'Short-term - bei 2,9 s ungesetzt, bei 3,0 s gesetzt' 'NAK-283-rot-M-58-basis-etappe-5.txt'),
            (L 'M-60' 'B18' $B18 'Loudnesszellen: Momentary kommt 0,4 s, Short-term 3,0 s' 'NAK-283-rot-M-60-basis-etappe-5.txt'))
    }
    [pscustomobject]@{
        Kennung = 'MUT-55b-BASIS'; Zeilen = 'M-55 (b) (Riegel gegen den Basis-SHA)'
        Was     = 'keine Mutation: derselbe Riegel am Basis-SHA - Regressionswache, dort erwartet gruen'
        Aenderungen = @()
        Ziele   = @()
        Laeufe  = @(@{ Zeile = 'M-55 (b) Basis'; Kuerzel = 'Riegel'; Skript = $riegel55; Arg = @('-Revision', $basis); Traeger = 'R2 die Definition lautet kVerteilungPlaetze = 64'; Datei = 'NAK-283-rot-M-55b-basis-etappe-5.txt'; Erwartet = 'gruen' })
    }
)

# ── Nur die Muster pruefen (schreibt nichts) ─────────────────────────────
if ($Muster) {
    $falsch = 0
    foreach ($m in $mutationen) {
        foreach ($a in $m.Aenderungen) {
            if ($a.ContainsKey('Basis')) { continue }
            $t = [System.IO.File]::ReadAllText($a.Datei)
            $suche = ($a.Alt -join (Zeilenende $t))
            $n = ([regex]::Matches($t, [regex]::Escape($suche))).Count
            if ($n -ne 1) { $falsch++ }
            Write-Host ("{0,-14} {1,-12} {2} Treffer: {3}" -f $m.Kennung, (Split-Path $a.Datei -Leaf), $n, $a.Alt[0].Trim())
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
    $sicherung = @{}
    $hashVorher = @{}
    $hashMutiert = @{}
    foreach ($d in $dateien) { $sicherung[$d] = [System.IO.File]::ReadAllBytes($d); $hashVorher[$d] = Hash-Von $d }

    $rotLaeufe = @()
    try {
        foreach ($a in $m.Aenderungen) { Mutiere $a }
        foreach ($d in $dateien) { $hashMutiert[$d] = Hash-Von $d; (Get-Item $d).LastWriteTime = Get-Date }
        $bauRot = Baue $m.Ziele
        if ($bauRot.ExitCode -ne 0) { throw "Bau der mutierten Fassung fehlgeschlagen: $($bauRot.Text)" }
        foreach ($z in $m.Ziele) {
            foreach ($d in $dateien) {
                if ((Get-Item (Exe-Pfad $z)).LastWriteTime -lt (Get-Item $d).LastWriteTime) {
                    throw "Binary $z ist aelter als die mutierte Quelle $(Relativ $d) - MSBuild hat nicht neu gebaut"
                }
            }
        }
        foreach ($l in $m.Laeufe) {
            $r = Fahre $l
            $marke = if ($l.Erwartet -eq 'gruen') { '[ok]' } else { '[ROT]' }
            $tz = Traeger-Zeile $r.Text $marke $l.Traeger
            $rotLaeufe += [pscustomobject]@{ Lauf = $l; Ergebnis = $r; Traeger = $tz }
            Write-Host ("  MUTIERT {0} -> Exit {1}, Traeger ({2}) {3}" -f $l.Datei, $r.ExitCode, $marke, $(if ($tz) { 'gefunden' } else { 'NICHT gefunden' }))
        }
    }
    finally {
        foreach ($d in $dateien) { [System.IO.File]::WriteAllBytes($d, $sicherung[$d]) }
    }

    $hashNachher = @{}
    foreach ($d in $dateien) {
        $hashNachher[$d] = Hash-Von $d
        if ($hashNachher[$d] -ne $hashVorher[$d]) { throw "Ruecknahme NICHT bytegleich in $(Relativ $d)" }
        (Get-Item $d).LastWriteTime = Get-Date
    }
    $bauGruen = Baue $m.Ziele
    if ($bauGruen.ExitCode -ne 0) { throw "Bau der zurueckgespielten Fassung fehlgeschlagen: $($bauGruen.Text)" }
    foreach ($z in $m.Ziele) {
        foreach ($d in $dateien) {
            if ((Get-Item (Exe-Pfad $z)).LastWriteTime -lt (Get-Item $d).LastWriteTime) {
                throw "Binary $z ist aelter als die zurueckgespielte Quelle $(Relativ $d) - MSBuild hat nicht neu gebaut"
            }
        }
    }

    foreach ($e in $rotLaeufe) {
        $l = $e.Lauf
        $gl = $l.Clone()
        if ($gl.ContainsKey('Arg') -and $m.Kennung -like '*-BASIS') { $gl.Arg = @() }   # der Riegel gruen am Arbeitsbaum
        $g = Fahre $gl
        $tg = Traeger-Zeile $g.Text '[ok]' $l.Traeger
        Write-Host ("  ZURUECK {0} -> Exit {1}, Traeger {2}" -f $l.Datei, $g.ExitCode, $(if ($tg) { 'ok' } else { 'FEHLT' }))
        $kopf = @(
            "NAK-283 Etappe 5 - Rotbeweis $($m.Kennung) fuer $($l.Zeile)",
            "Zeitpunkt:  $(Get-Date -Format o)",
            "Mutation:   $($m.Was)",
            "Erwartet:   mutiert $($l.Erwartet) / zurueckgespielt gruen")
        if ($dateien.Count -eq 0) { $kopf += 'Datei:      (keine Mutation)' }
        foreach ($d in $dateien) {
            $kopf += "Datei:      $(Relativ $d)"
            $kopf += "  SHA-256 vorher:   $($hashVorher[$d])"
            $kopf += "  SHA-256 mutiert:  $($hashMutiert[$d])"
            $kopf += "  SHA-256 nachher:  $($hashNachher[$d]) (bytegleiche Ruecknahme: $($hashVorher[$d] -eq $hashNachher[$d]))"
        }
        $bein = if ($l.ContainsKey('Skript')) { "$($l.Skript) $(@($l.Arg) -join ' ')" } else { "$($l.Kuerzel) $($l.Ziel)" }
        $kopf += @(
            "Bein:       $bein",
            "Neubau:     mutiert $($m.Ziele -join ' ') / zurueckgespielt $($m.Ziele -join ' ')",
            "Traeger:    $($l.Traeger)",
            "Urteil:     MUTIERT Exit $($e.Ergebnis.ExitCode) / ZURUECKGESPIELT Exit $($g.ExitCode)",
            "Traegerzeile mutiert:        $($e.Traeger)",
            "Traegerzeile zurueckgespielt: $tg",
            "",
            "--- MUTIERT (erwartet: $($l.Erwartet)) --------------------------------",
            $e.Ergebnis.Text,
            "",
            "--- ZURUECKGESPIELT (erwartet: gruen) -----------------------------",
            $g.Text)
        [System.IO.File]::WriteAllLines((Join-Path $rohOrdner $l.Datei), $kopf, $utf8)
        $mutiertOk = if ($l.Erwartet -eq 'gruen') { $e.Ergebnis.ExitCode -eq 0 -or [bool] $e.Traeger } else { $e.Ergebnis.ExitCode -ne 0 }
        $bilanz += [pscustomobject]@{
            Kennung = $m.Kennung; Zeile = $l.Zeile; Datei = $l.Datei; Erwartet = $l.Erwartet
            Mutiert = $e.Ergebnis.ExitCode; Zurueck = $g.ExitCode
            TraegerMutiert = [bool] $e.Traeger; TraegerZurueck = [bool] $tg
            ExitMutiertOk = $mutiertOk
            HashGleich = (@($dateien | Where-Object { $hashVorher[$_] -ne $hashNachher[$_] }).Count -eq 0)
        }
    }
}

$zeilen = @('NAK-283 Etappe 5 - Rotbilanz', "Zeitpunkt: $(Get-Date -Format o)", "HEAD: $(git -C $Wurzel rev-parse HEAD)", "Basis: $basis", '')
foreach ($b in $bilanz) {
    $ok = $b.ExitMutiertOk -and $b.Zurueck -eq 0 -and $b.TraegerMutiert -and $b.TraegerZurueck -and $b.HashGleich
    $zeilen += ("{0,-14} {1,-24} erwartet {2,-5} mutiert Exit {3} (Traeger {4}) / zurueck Exit {5} (Traeger {6}), Hash gleich {7}  {8}  {9}" -f `
        $b.Kennung, $b.Zeile, $b.Erwartet, $b.Mutiert, $b.TraegerMutiert, $b.Zurueck, $b.TraegerZurueck, $b.HashGleich, $b.Datei, $(if ($ok) { 'BESTANDEN' } else { 'NICHT BESTANDEN' }))
}
$schlecht = @($bilanz | Where-Object { -not ($_.ExitMutiertOk -and $_.Zurueck -eq 0 -and $_.TraegerMutiert -and $_.TraegerZurueck -and $_.HashGleich) })
$zeilen += ''
$zeilen += ("URTEIL: {0} von {1} Laeufen bestanden (Rotlauf mit [ROT]-Traeger oder Beobachtung mit [ok]-Traeger, zurueckgespielt gruen mit [ok]-Traeger, Ruecknahme bytegleich)" -f ($bilanz.Count - $schlecht.Count), $bilanz.Count)
if (-not $Nur) { [System.IO.File]::WriteAllLines((Join-Path $rohOrdner 'NAK-283-etappe-5-rotbilanz.txt'), $zeilen, $utf8) }
$zeilen | ForEach-Object { Write-Host $_ }
if ($schlecht.Count -gt 0) { exit 1 }
exit 0
