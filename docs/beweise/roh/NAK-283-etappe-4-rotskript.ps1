# NAK-283 Etappe 4 — Rotbeweise je Matrixzeile M-27 bis M-43 (Bauer, 13.09.2026)
#
# Ablauf je Mutation nach docs/beweise/NAK-283.md §6.1:
#   1. Originalbytes und SHA-256 JEDER Quelldatei der Mutation sichern
#   2. Mutation genau an der Zeile, die die Zusage traegt - jedes Muster genau
#      einmal (sonst Abbruch), die Zeilenenden der Datei bleiben
#   3. LastWriteTime setzen (NAK-230), Neubau der betroffenen Ziele; jedes
#      Binary muss juenger sein als die mutierte Quelle
#   4. Bein fahren -> ROT zaehlt nur mit Exit != 0 UND einer FEHLER-Zeile, die
#      den Traeger der Zusage nennt (nie ein Nebeneffekt)
#   5. Originalbytes zurueck (nie eine zweite Textersetzung), SHA-256 gegen
#      Schritt 1, Zeitstempel, Neubau, Bein fahren -> GRUEN mit Exit 0 UND
#      einer ok-Zeile desselben Traegers
#
# Keine Zeile dieser Etappe steht in der Nebenlaeufigkeitsliste aus §6.1; die
# Analysefaelle M-35 bis M-37 und M-41 halten den Worker deterministisch an
# (`mitAngehaltenerAnalyseFuerTest`, analyseSchloss), statt auf Zeit zu warten.
# Regressionswachen (M-28, M-29, M-31, M-33, M-34, M-37, M-38, M-42) werden
# absichtlich gebrochen und sind kein Beleg fuer ihren Befund.
#
# Aufruf aus dem Workspace-Root:
#   pwsh -File docs/beweise/roh/NAK-283-etappe-4-rotskript.ps1
#   pwsh -File docs/beweise/roh/NAK-283-etappe-4-rotskript.ps1 -Nur MUT-30
# Rohausgaben: docs/beweise/roh/NAK-283-rot-M-<nn>-etappe-4.txt,
# Bilanz:      docs/beweise/roh/NAK-283-etappe-4-rotbilanz.txt (nur beim vollen Lauf)

[CmdletBinding()]
param(
    [string] $Nur = '',
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
$basis     = '0f0bc1edbd0554eede8976f05a3eb3f4c376e19c'

function Exe-Pfad([string] $ziel) { Join-Path $build ("plugin\{0}_artefacts\Release\{0}.exe" -f $ziel) }
function Zeilenende([string] $text) { if ($text -match "`r`n") { "`r`n" } else { "`n" } }
function Hash-Von([string] $datei) { (Get-FileHash -Algorithm SHA256 $datei).Hash }
function Relativ([string] $pfad) { $pfad.Replace($Wurzel + '\', '').Replace('\', '/') }

function Mutiere([string] $datei, [string[]] $alt, [string[]] $neu) {
    $t = [System.IO.File]::ReadAllText($datei)
    $le = Zeilenende $t
    $suche = ($alt -join $le)
    $treffer = ([regex]::Matches($t, [regex]::Escape($suche))).Count
    if ($treffer -ne 1) { throw "Mutationsmuster $treffer-mal gefunden in $(Relativ $datei) (erwartet: 1): $($alt[0])" }
    $t = $t.Replace($suche, ($neu -join $le))
    [System.IO.File]::WriteAllText($datei, $t, $utf8)
}

function Baue([string[]] $ziele) {
    if ($ziele.Count -eq 0) { return @{ ExitCode = 0; Text = '(kein Bauziel)' } }
    $argumente = @('--build', $build, '--config', 'Release', '--target') + $ziele
    $aus = & $cmake @argumente 2>&1
    return @{ ExitCode = $LASTEXITCODE; Text = ($aus -join "`n") }
}

function Fahre($lauf, [string] $phase) {
    $argumente = $lauf.Arg
    if ($phase -eq 'rot'   -and $lauf.ContainsKey('ArgRot'))   { $argumente = $lauf.ArgRot }
    if ($phase -eq 'gruen' -and $lauf.ContainsKey('ArgGruen')) { $argumente = $lauf.ArgGruen }
    if ($null -eq $argumente) { $argumente = @() }
    if ($lauf.Art -eq 'skript') {
        $aus = & pwsh -NoProfile -File (Join-Path $Wurzel $lauf.Skript) @argumente 2>&1
    } else {
        $aus = & (Exe-Pfad $lauf.Ziel) @argumente 2>&1
    }
    return @{ ExitCode = $LASTEXITCODE; Text = ($aus -join "`n"); Argumente = ($argumente -join ' ') }
}

function Traeger-Zeile([string] $text, [string] $marke, [string] $traeger) {
    foreach ($z in ($text -split "`n")) {
        $zt = $z.Trim()
        if ($zt.StartsWith($marke) -and $zt.Contains($traeger)) { return $zt }
    }
    return ''
}

# ── Die Quelldateien ──────────────────────────────────────────────────────
$sondeCpp = Join-Path $Wurzel 'eq-copilot\plugin\sonde\SondeProcessor.cpp'
$sondeH   = Join-Path $Wurzel 'eq-copilot\plugin\sonde\SondeProcessor.h'
$kernCpp  = Join-Path $Wurzel 'eq-copilot\plugin\dsp\DspKern.cpp'
$queueH   = Join-Path $Wurzel 'eq-copilot\plugin\core\StampedAudioQueue.h'
$markH    = Join-Path $Wurzel 'eq-copilot\plugin\src\HoerMarkierung.h'
$riegel   = 'docs/beweise/roh/NAK-283-etappe-4-riegel-M-43.ps1'

# ── Die Mutationen, gruppiert nach Bauziel ────────────────────────────────
$mutationen = @(
    # ── A16 EqCopProbeeqNullTest ───────────────────────────────────────────
    [pscustomobject]@{
        Kennung = 'MUT-27'; Zeilen = 'M-27'
        Was     = 'SondeProcessor.cpp isBusesLayoutSupported: die Mono/Stereo-Einschraenkung entfernen - nur ein == aus bleibt (Stand am Basis-SHA)'
        Aenderungen = @(@{ Datei = $sondeCpp
            Alt = @(
                '    if (ein != aus)',
                '        return false;',
                '',
                '    return ein == juce::AudioChannelSet::mono() || ein == juce::AudioChannelSet::stereo();')
            Neu = @('    return ein == aus;   // MUTATION NAK-283 Rotbeweis M-27: Mono/Stereo-Einschraenkung entfernt') })
        Ziele   = @('EqCopProbeeqNullTest')
        Laeufe  = @(@{ Zeile = 'M-27'; Kuerzel = 'A16'; Ziel = 'EqCopProbeeqNullTest'; Arg = @(); Traeger = 'mehrkanallayout_wird_abgelehnt'; Datei = 'NAK-283-rot-M-27-etappe-4.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-28'; Zeilen = 'M-28 (Regressionswache)'
        Was     = 'SondeProcessor.cpp isBusesLayoutSupported: eine Fassung, die nur Stereo zulaesst'
        Aenderungen = @(@{ Datei = $sondeCpp
            Alt = @('    return ein == juce::AudioChannelSet::mono() || ein == juce::AudioChannelSet::stereo();')
            Neu = @('    return ein == juce::AudioChannelSet::stereo();   // MUTATION NAK-283 Rotbeweis M-28: nur Stereo') })
        Ziele   = @('EqCopProbeeqNullTest')
        Laeufe  = @(@{ Zeile = 'M-28'; Kuerzel = 'A16'; Ziel = 'EqCopProbeeqNullTest'; Arg = @(); Traeger = 'mono_und_stereo_bleiben_angenommen'; Datei = 'NAK-283-rot-M-28-etappe-4.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-29'; Zeilen = 'M-29 (Regressionswache)'
        Was     = 'SondeProcessor.cpp isBusesLayoutSupported: die Gleichheit von Ein- und Ausgang entfernen - jede Mono/Stereo-Kombination gilt'
        Aenderungen = @(@{ Datei = $sondeCpp
            Alt = @(
                '    if (ein != aus)',
                '        return false;',
                '',
                '    return ein == juce::AudioChannelSet::mono() || ein == juce::AudioChannelSet::stereo();')
            Neu = @(
                '    // MUTATION NAK-283 Rotbeweis M-29: Gleichheit von Ein- und Ausgang entfernt',
                '    return (ein == juce::AudioChannelSet::mono() || ein == juce::AudioChannelSet::stereo())',
                '        && (aus == juce::AudioChannelSet::mono() || aus == juce::AudioChannelSet::stereo());') })
        Ziele   = @('EqCopProbeeqNullTest')
        Laeufe  = @(@{ Zeile = 'M-29'; Kuerzel = 'A16'; Ziel = 'EqCopProbeeqNullTest'; Arg = @(); Traeger = 'ungleiche_und_deaktivierte_layouts_bleiben_abgelehnt'; Datei = 'NAK-283-rot-M-29-etappe-4.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-33'; Zeilen = 'M-33 (Regressionswache)'
        Was     = 'SondeProcessor.h: getTailLengthSeconds() { return 0.05; }'
        Aenderungen = @(@{ Datei = $sondeH
            Alt = @('    double getTailLengthSeconds() const override        { return 0.0; }')
            Neu = @('    double getTailLengthSeconds() const override        { return 0.05; }   // MUTATION NAK-283 Rotbeweis M-33') })
        Ziele   = @('EqCopProbeeqNullTest')
        Laeufe  = @(@{ Zeile = 'M-33'; Kuerzel = 'A16'; Ziel = 'EqCopProbeeqNullTest'; Arg = @(); Traeger = 'latenz_bleibt_null'; Datei = 'NAK-283-rot-M-33-etappe-4.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-34'; Zeilen = 'M-34 (Regressionswache)'
        Was     = 'SondeProcessor.cpp: eine reset-Fassung, die den Ausgangspuffer nullt - reset setzt eine Marke, der naechste processBlock loescht seinen Puffer'
        Aenderungen = @(
            @{ Datei = $sondeCpp
               Alt = @('void SondeProcessor::reset()', '{')
               Neu = @('static bool nakamaRotNachReset = false;   // MUTATION NAK-283 Rotbeweis M-34', '', 'void SondeProcessor::reset()', '{') }
            @{ Datei = $sondeCpp
               Alt = @('    dspKern->beendeAudiohistorie();')
               Neu = @('    dspKern->beendeAudiohistorie();', '    nakamaRotNachReset = true;   // MUTATION NAK-283 Rotbeweis M-34: der naechste Ausgangspuffer wird genullt') }
            @{ Datei = $sondeCpp
               Alt = @('void SondeProcessor::processBlock (juce::AudioBuffer<float>& puffer, juce::MidiBuffer&)', '{', '    juce::ScopedNoDenormals keineDenormals;')
               Neu = @('void SondeProcessor::processBlock (juce::AudioBuffer<float>& puffer, juce::MidiBuffer&)', '{', '    juce::ScopedNoDenormals keineDenormals;', '    if (nakamaRotNachReset) { puffer.clear(); nakamaRotNachReset = false; }   // MUTATION NAK-283 Rotbeweis M-34') })
        Ziele   = @('EqCopProbeeqNullTest')
        Laeufe  = @(@{ Zeile = 'M-34'; Kuerzel = 'A16'; Ziel = 'EqCopProbeeqNullTest'; Arg = @(); Traeger = 'nulltest_bleibt_bitidentisch_mit_host_reset'; Datei = 'NAK-283-rot-M-34-etappe-4.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-42b'; Zeilen = 'M-42 Haelfte b (Regressionswache)'
        Was     = 'DspKern.cpp verarbeiteStueck: ein Ausgangsriegel ausserhalb des engagiert-Rahmens - im Ruhezweig wird jeder nicht-endliche Ausgang verriegelt und gezaehlt'
        Aenderungen = @(@{ Datei = $kernCpp
            Alt = @(
                '    if (committedRuht)',
                '    {',
                '        // Der Uebergang der Hoermatrix hat hier nichts zu blenden - beide')
            Neu = @(
                '    if (committedRuht)',
                '    {',
                '        // MUTATION NAK-283 Rotbeweis M-42b: Ausgangsriegel ausserhalb des engagiert-Rahmens',
                '        for (int k = 0; k < numKanaele; ++k)',
                '            for (size_t i = 0; i < n; ++i)',
                '                if (! std::isfinite (kanaele[k][i])) { kanaele[k][i] = 0.0f; zaehlerEingaenge.fetch_add (1, std::memory_order_relaxed); }',
                '        // Der Uebergang der Hoermatrix hat hier nichts zu blenden - beide') })
        Ziele   = @('EqCopProbeeqNullTest')
        Laeufe  = @(@{ Zeile = 'M-42b'; Kuerzel = 'A16'; Ziel = 'EqCopProbeeqNullTest'; Arg = @(); Traeger = 'passthrough_sanitisiert_nichts'; Datei = 'NAK-283-rot-M-42b-etappe-4.txt' })
    }

    # ── B7 EqCopTransactionTest ────────────────────────────────────────────
    [pscustomobject]@{
        Kennung = 'MUT-30'; Zeilen = 'M-30, M-32'
        Was     = 'SondeProcessor.h/.cpp: die reset-Ueberschreibung entfernen - der Host-Reset faellt auf die leere JUCE-Basis (Stand am Basis-SHA)'
        Aenderungen = @(
            @{ Datei = $sondeH
               Alt = @('    void reset() override;')
               Neu = @('    // MUTATION NAK-283 Rotbeweis M-30/M-32: reset-Ueberschreibung entfernt') }
            @{ Datei = $sondeCpp
               Alt = @('void SondeProcessor::reset()', '{')
               Neu = @('#if 0 // MUTATION NAK-283 Rotbeweis M-30/M-32: reset-Ueberschreibung entfernt', 'void SondeProcessor::reset()', '{') }
            @{ Datei = $sondeCpp
               Alt = @('    dspKern->beendeAudiohistorie();', '}')
               Neu = @('    dspKern->beendeAudiohistorie();', '}', '#endif') })
        Ziele   = @('EqCopTransactionTest')
        Laeufe  = @(
            @{ Zeile = 'M-30'; Kuerzel = 'B7'; Ziel = 'EqCopTransactionTest'; Arg = @(); Traeger = 'host_reset_beendet_die_audiohistorie'; Datei = 'NAK-283-rot-M-30-etappe-4.txt' },
            @{ Zeile = 'M-32'; Kuerzel = 'B7'; Ziel = 'EqCopTransactionTest'; Arg = @(); Traeger = 'die_drei_hosteintritte_enden_in_derselben_audiohistorie'; Datei = 'NAK-283-rot-M-32-etappe-4.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-31'; Zeilen = 'M-31 (Regressionswache)'
        Was     = 'SondeProcessor.cpp reset: ruft zusaetzlich den neutralen Ladestart transaktion->ladestart (DspSatz {}, 0, {}, 0, grund) - derselbe Aufruf wie im Fehlerpfad von setStateInformation (Rotbeweisspalte woertlich)'
        Aenderungen = @(@{ Datei = $sondeCpp
            Alt = @('    dspKern->beendeAudiohistorie();')
            Neu = @(
                '    dspKern->beendeAudiohistorie();',
                '    juce::String rotGrund;   // MUTATION NAK-283 Rotbeweis M-31: reset verwirft den Zustand (neutraler Ladestart)',
                '    transaktion->ladestart (nakama::parameter::DspSatz {}, 0, {}, 0, rotGrund);') })
        Ziele   = @('EqCopTransactionTest')
        Laeufe  = @(@{ Zeile = 'M-31'; Kuerzel = 'B7'; Ziel = 'EqCopTransactionTest'; Arg = @(); Traeger = 'host_reset_laesst_parameter_und_zustand_unberuehrt'; Datei = 'NAK-283-rot-M-31-etappe-4.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-31b'; Zeilen = 'M-31 (Regressionswache, zweite Fassung)'
        Was     = 'SondeProcessor.cpp reset: neutraler Ladestart UND Publikation des neutralen wirksamen Zustands an den Kern - wie der neutrale Weg von setStateInformation'
        Aenderungen = @(@{ Datei = $sondeCpp
            Alt = @('    dspKern->beendeAudiohistorie();')
            Neu = @(
                '    dspKern->beendeAudiohistorie();',
                '    juce::String rotGrund;   // MUTATION NAK-283 Rotbeweis M-31b: reset verwirft den Zustand und publiziert ihn',
                '    transaktion->ladestart (nakama::parameter::DspSatz {}, 0, {}, 0, rotGrund);',
                '    publikationOffen = ! dspAusfuehrung->publiziereWirksam (transaktion->wirksam(), true);') })
        Ziele   = @('EqCopTransactionTest')
        Laeufe  = @(@{ Zeile = 'M-31b'; Kuerzel = 'B7'; Ziel = 'EqCopTransactionTest'; Arg = @(); Traeger = 'host_reset_laesst_parameter_und_zustand_unberuehrt'; Datei = 'NAK-283-rot-M-31b-etappe-4.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-35'; Zeilen = 'M-35'
        Was     = 'SondeProcessor.cpp processBlock: den Verwerfpfad des dritten Zustands entfernen - Tap ungueltig und Kern rechnet faellt durch, die Queue wird nie gerufen (Stand am Basis-SHA)'
        Aenderungen = @(@{ Datei = $sondeCpp
            Alt = @('            analyseQueue.verwirfOhneAudio (samples);')
            Neu = @('            // MUTATION NAK-283 Rotbeweis M-35: Verwerfpfad entfernt') })
        Ziele   = @('EqCopTransactionTest')
        Laeufe  = @(@{ Zeile = 'M-35'; Kuerzel = 'B7'; Ziel = 'EqCopTransactionTest'; Arg = @(); Traeger = 'ungueltiger_tap_bei_rechnendem_kern_hinterlaesst_eine_luecke'; Datei = 'NAK-283-rot-M-35-etappe-4.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-36'; Zeilen = 'M-36'
        Was     = 'StampedAudioQueue.h verwirfOhneAudio: der Verwerfpfad ohne Zaehler - weder der Block noch seine Frames zaehlen in der Queue; Strom und Luecke bleiben'
        Aenderungen = @(@{ Datei = $queueH
            Alt = @(
                '        ohneAudioDrops.fetch_add (1, std::memory_order_relaxed);',
                '        verworfeneFrames.fetch_add ((std::uint64_t) frames, std::memory_order_relaxed);')
            Neu = @('        // MUTATION NAK-283 Rotbeweis M-36: der Verwerfpfad zaehlt nicht') })
        Ziele   = @('EqCopTransactionTest')
        Laeufe  = @(@{ Zeile = 'M-36'; Kuerzel = 'B7'; Ziel = 'EqCopTransactionTest'; Arg = @(); Traeger = 'dsp_und_queuezaehler_beschreiben_dieselbe_verworfene_zeitspanne'; Datei = 'NAK-283-rot-M-36-etappe-4.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-37'; Zeilen = 'M-37 (Regressionswache)'
        Was     = 'SondeProcessor.cpp processBlock: ein Verwerfpfad, der VOR dem Passthrough-Zweig steht - jeder Block ohne gueltigen Tap faellt, auch im ruhenden Passthrough'
        Aenderungen = @(@{ Datei = $sondeCpp
            Alt = @('        else if (committedRuhtImPassthrough())', '        {')
            Neu = @(
                '        else if (dspKern->tapLaenge() == 0)   // MUTATION NAK-283 Rotbeweis M-37: Verwerfpfad vor dem Passthrough-Zweig',
                '        {',
                '            analyseQueue.verwirfOhneAudio (samples);',
                '        }',
                '        else if (committedRuhtImPassthrough())',
                '        {') })
        Ziele   = @('EqCopTransactionTest')
        Laeufe  = @(@{ Zeile = 'M-37'; Kuerzel = 'B7'; Ziel = 'EqCopTransactionTest'; Arg = @(); Traeger = 'ruhender_passthrough_erzeugt_keine_luecke'; Datei = 'NAK-283-rot-M-37-etappe-4.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-41'; Zeilen = 'M-41'
        Was     = 'SondeProcessor.cpp processBlock: die Pruefung aus der Kopierschleife der Analyse entfernen (Stand am Basis-SHA: blosse Verengung)'
        Aenderungen = @(@{ Datei = $sondeCpp
            Alt = @('                    if (! std::isfinite (wert) && std::isfinite (von[i])) { wert = 0.0f; ++verriegelt; }')
            Neu = @('                    // MUTATION NAK-283 Rotbeweis M-41: Pruefung aus der Kopierschleife entfernt') })
        Ziele   = @('EqCopTransactionTest')
        Laeufe  = @(@{ Zeile = 'M-41'; Kuerzel = 'B7'; Ziel = 'EqCopTransactionTest'; Arg = @(); Traeger = 'analysekopie_verengt_nicht_unbemerkt'; Datei = 'NAK-283-rot-M-41-etappe-4.txt' })
    }

    # ── B6 EqCopDspGoldenTest ──────────────────────────────────────────────
    [pscustomobject]@{
        Kennung = 'MUT-38'; Zeilen = 'M-38 (Regressionswache)'
        Was     = 'DspKern.cpp verarbeite: ein Kern, der den uebergrossen Block zurueckweist statt ihn zu stueckeln'
        Aenderungen = @(@{ Datei = $kernCpp
            Alt = @(
                '        zaehlerVerworfen.fetch_add (1, std::memory_order_relaxed);',
                '        int versatz = 0;',
                '        while (versatz < numSamples)',
                '        {',
                '            const int n = std::min (maxBlockGroesse, numSamples - versatz);',
                '            float* teil[2] = { kanaele[0] + versatz,',
                '                               numKanaele > 1 ? kanaele[1] + versatz : nullptr };',
                '            verarbeiteStueck (teil, numKanaele, n);',
                '            versatz += n;',
                '            if (versatz < numSamples && teilstueckHaken != nullptr)',
                '                teilstueckHaken (teilstueckKontext);',
                '        }')
            Neu = @(
                '        zaehlerVerworfen.fetch_add (1, std::memory_order_relaxed);',
                '        // MUTATION NAK-283 Rotbeweis M-38: der uebergrosse Block wird zurueckgewiesen statt gestueckelt') })
        Ziele   = @('EqCopDspGoldenTest')
        Laeufe  = @(@{ Zeile = 'M-38'; Kuerzel = 'B6'; Ziel = 'EqCopDspGoldenTest'; Arg = @(); Traeger = 'der_uebergrosse_block_gleicht_sample_exakt_dem_lauf_ohne_ueberlast'; Datei = 'NAK-283-rot-M-38-etappe-4.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-39'; Zeilen = 'M-39'
        Was     = 'DspKern.cpp Ausgangsschleife: den Ausgangsriegel entfernen - blosse Verengung (Stand am Basis-SHA)'
        Aenderungen = @(@{ Datei = $kernCpp
            Alt = @(
                '        float fl = (float) l;',
                '        if (! std::isfinite (fl) && std::isfinite (l)) { fl = 0.0f; zaehlerEingaenge.fetch_add (1, std::memory_order_relaxed); }',
                '        kanaele[0][i] = fl;',
                '        if (numKanaele > 1)',
                '        {',
                '            float fr = (float) r;',
                '            if (! std::isfinite (fr) && std::isfinite (r)) { fr = 0.0f; zaehlerEingaenge.fetch_add (1, std::memory_order_relaxed); }',
                '            kanaele[1][i] = fr;',
                '        }')
            Neu = @(
                '        kanaele[0][i] = (float) l;   // MUTATION NAK-283 Rotbeweis M-39: Ausgangsriegel entfernt',
                '        if (numKanaele > 1) kanaele[1][i] = (float) r;') })
        Ziele   = @('EqCopDspGoldenTest')
        Laeufe  = @(@{ Zeile = 'M-39'; Kuerzel = 'B6'; Ziel = 'EqCopDspGoldenTest'; Arg = @(); Traeger = 'endlicher_eingang_erzeugt_keinen_nichtendlichen_floatausgang'; Datei = 'NAK-283-rot-M-39-etappe-4.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-42a'; Zeilen = 'M-42 Haelfte a (Regressionswache)'
        Was     = 'DspKern.cpp Ausgangsschleife: ein Riegel, der auch endliches Material greift (jeder float mit Betrag >= 1)'
        Aenderungen = @(@{ Datei = $kernCpp
            Alt = @('        if (! std::isfinite (fl) && std::isfinite (l)) { fl = 0.0f; zaehlerEingaenge.fetch_add (1, std::memory_order_relaxed); }')
            Neu = @('        if (! (std::abs (fl) < 1.0f) && std::isfinite (l)) { fl = 0.0f; zaehlerEingaenge.fetch_add (1, std::memory_order_relaxed); }   // MUTATION NAK-283 Rotbeweis M-42a') })
        Ziele   = @('EqCopDspGoldenTest')
        Laeufe  = @(@{ Zeile = 'M-42a'; Kuerzel = 'B6'; Ziel = 'EqCopDspGoldenTest'; Arg = @(); Traeger = 'ausgangsriegel_beruehrt_endliches_material_nicht'; Datei = 'NAK-283-rot-M-42a-etappe-4.txt' })
    }

    # ── A3 EqCopMarkierungTest ─────────────────────────────────────────────
    [pscustomobject]@{
        Kennung = 'MUT-40'; Zeilen = 'M-40'
        Was     = 'HoerMarkierung.h tdf2Lauf: die Pruefung wieder auf y (double) legen, danach verengen (Stand am Basis-SHA)'
        Aenderungen = @(@{ Datei = $markH
            Alt = @(
                '            const float f = (float) y;',
                '            if (std::isfinite (f))',
                '                d[i] = f;')
            Neu = @(
                '            if (std::isfinite (y))   // MUTATION NAK-283 Rotbeweis M-40: Pruefung wieder auf y (double)',
                '                d[i] = (float) y;') })
        Ziele   = @('EqCopMarkierungTest')
        Laeufe  = @(@{ Zeile = 'M-40'; Kuerzel = 'A3'; Ziel = 'EqCopMarkierungTest'; Arg = @(); Traeger = 'endlicher_eingang_erzeugt_keinen_nichtendlichen_wetausgang'; Datei = 'NAK-283-rot-M-40-etappe-4.txt' })
    }

    # ── Riegel M-43 ────────────────────────────────────────────────────────
    [pscustomobject]@{
        Kennung = 'MUT-43'; Zeilen = 'M-43'
        Was     = 'SondeProcessor.cpp isBusesLayoutSupported: die alte Passthrough-Begruendung zurueck (Kommentar am Basis-SHA)'
        Aenderungen = @(@{ Datei = $sondeCpp
            Alt = @(
                '    // NAK-283 F04 (M-27 bis M-29): dieselbe Regel wie Gen',
                '    // (`src/prozessor/Hostbruecke.cpp`, isBusesLayoutSupported) - Eingang',
                '    // gleich Ausgang UND Mono oder Stereo, sonst ein klares Nein, nie still',
                '    // umgedeutet (Entwurf §48.2). Die alte Begruendung, ein Passthrough, der',
                '    // Kanaele erfinden oder verschlucken muesste,',
                '    // "waere kein Passthrough", traegt seit SONDE-015 Etappe 4a nicht mehr:',
                '    // hier rechnet ein DSP-Kern, der genau zwei Kanaele kennt (`processBlock`,',
                '    // `kanalZeiger[2]`). Ein angenommener Vierkanalbus bekaeme auf Kanal 1 und',
                '    // 2 EQ und Trim, auf 3 und 4 nichts; ihn zu bedienen waere Verarbeitung,',
                '    // die niemand eingeschaltet hat.')
            Neu = @(
                '    // Ein Passthrough, der Kanaele erfinden oder verschlucken muesste, waere',
                '    // kein Passthrough. Deshalb nur gleiche Ein-/Ausgangsbelegung, und keine',
                '    // deaktivierten Hauptbusse.') })
        Ziele   = @()
        Laeufe  = @(@{ Zeile = 'M-43'; Art = 'skript'; Skript = $riegel; Arg = @(); Traeger = 'R1 F04'; Datei = 'NAK-283-rot-M-43-etappe-4.txt' })
    }
    [pscustomobject]@{
        Kennung = 'MUT-43-BASIS'; Zeilen = 'M-43 (Riegel gegen den Basis-SHA)'
        Was     = 'keine Mutation: derselbe Riegel misst rot den Stand am Basis-SHA (git show), gruen den Arbeitsbaum'
        Aenderungen = @()
        Ziele   = @()
        Laeufe  = @(@{ Zeile = 'M-43 Basis'; Art = 'skript'; Skript = $riegel; Arg = @(); ArgRot = @('-Revision', $basis); ArgGruen = @(); Traeger = 'R3 F09'; Datei = 'NAK-283-rot-M-43-basis-etappe-4.txt' })
    }
)

# ── Lauf ──────────────────────────────────────────────────────────────────
$bilanz = @()
foreach ($m in $mutationen) {
    if ($Nur -and $m.Kennung -ne $Nur) { continue }
    Write-Host "=== $($m.Kennung) ($($m.Zeilen)) ===" -ForegroundColor Cyan

    $dateien = @($m.Aenderungen | ForEach-Object { $_.Datei } | Select-Object -Unique)
    $sicherung = @{}
    $hashVorher = @{}
    foreach ($d in $dateien) { $sicherung[$d] = [System.IO.File]::ReadAllBytes($d); $hashVorher[$d] = Hash-Von $d }

    $rotLaeufe = @()
    try {
        foreach ($a in $m.Aenderungen) { Mutiere $a.Datei $a.Alt $a.Neu }
        foreach ($d in $dateien) { (Get-Item $d).LastWriteTime = Get-Date }
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
            $r = Fahre $l 'rot'
            $tz = Traeger-Zeile $r.Text 'FEHLER' $l.Traeger
            $rotLaeufe += [pscustomobject]@{ Lauf = $l; Ergebnis = $r; Traeger = $tz }
            Write-Host ("  ROT   {0} -> Exit {1}, Traeger {2}" -f $l.Datei, $r.ExitCode, $(if ($tz) { 'gefallen' } else { 'NICHT gefallen' }))
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
        $g = Fahre $l 'gruen'
        $tg = Traeger-Zeile $g.Text 'ok' $l.Traeger
        Write-Host ("  GRUEN {0} -> Exit {1}, Traeger {2}" -f $l.Datei, $g.ExitCode, $(if ($tg) { 'ok' } else { 'FEHLT' }))
        $kopf = @(
            "NAK-283 Etappe 4 - Rotbeweis $($m.Kennung) fuer $($l.Zeile)",
            "Zeitpunkt:  $(Get-Date -Format o)",
            "Mutation:   $($m.Was)")
        if ($dateien.Count -eq 0) { $kopf += 'Datei:      (keine Mutation)' }
        foreach ($d in $dateien) {
            $kopf += "Datei:      $(Relativ $d)"
            $kopf += "  SHA-256 vorher:  $($hashVorher[$d])"
            $kopf += "  SHA-256 nachher: $($hashNachher[$d]) (bytegleiche Ruecknahme: $($hashVorher[$d] -eq $hashNachher[$d]))"
        }
        $bein = if ($l.Art -eq 'skript') { $l.Skript } else { "$($l.Kuerzel) $($l.Ziel)" }
        $kopf += @(
            "Bein:       $bein (rot: '$($e.Ergebnis.Argumente)', gruen: '$($g.Argumente)')",
            "Neubau:     rot $($m.Ziele -join ' ') / gruen $($m.Ziele -join ' ')",
            "Traeger:    $($l.Traeger)",
            "Urteil:     ROT Exit $($e.Ergebnis.ExitCode) / GRUEN Exit $($g.ExitCode)",
            "Traegerzeile rot:   $($e.Traeger)",
            "Traegerzeile gruen: $tg",
            "",
            "--- MUTIERT (erwartet: rot) ---------------------------------------",
            $e.Ergebnis.Text,
            "",
            "--- ZURUECKGESPIELT (erwartet: gruen) -----------------------------",
            $g.Text)
        [System.IO.File]::WriteAllLines((Join-Path $rohOrdner $l.Datei), $kopf, $utf8)
        $bilanz += [pscustomobject]@{
            Kennung = $m.Kennung; Zeile = $l.Zeile; Datei = $l.Datei
            Rot = $e.Ergebnis.ExitCode; Gruen = $g.ExitCode
            TraegerRot = [bool] $e.Traeger; TraegerGruen = [bool] $tg
            HashGleich = (@($dateien | Where-Object { $hashVorher[$_] -ne $hashNachher[$_] }).Count -eq 0)
        }
    }
}

$zeilen = @('NAK-283 Etappe 4 - Rotbilanz', "Zeitpunkt: $(Get-Date -Format o)", "HEAD: $(git -C $Wurzel rev-parse HEAD)", '')
foreach ($b in $bilanz) {
    $ok = $b.Rot -ne 0 -and $b.Gruen -eq 0 -and $b.TraegerRot -and $b.TraegerGruen -and $b.HashGleich
    $zeilen += ("{0,-13} {1,-11} rot Exit {2} (Traeger {3}) / gruen Exit {4} (Traeger {5}), Hash gleich {6}  {7}  {8}" -f `
        $b.Kennung, $b.Zeile, $b.Rot, $b.TraegerRot, $b.Gruen, $b.TraegerGruen, $b.HashGleich, $b.Datei, $(if ($ok) { 'BESTANDEN' } else { 'NICHT BESTANDEN' }))
}
$schlecht = @($bilanz | Where-Object { $_.Rot -eq 0 -or $_.Gruen -ne 0 -or -not $_.TraegerRot -or -not $_.TraegerGruen -or -not $_.HashGleich })
$zeilen += ''
$zeilen += ("URTEIL: {0} von {1} Rotbeweisen bestanden (rot mit Traeger, gruen mit Traeger, Ruecknahme bytegleich)" -f ($bilanz.Count - $schlecht.Count), $bilanz.Count)
if (-not $Nur) { [System.IO.File]::WriteAllLines((Join-Path $rohOrdner 'NAK-283-etappe-4-rotbilanz.txt'), $zeilen, $utf8) }
$zeilen | ForEach-Object { Write-Host $_ }
if ($schlecht.Count -gt 0) { exit 1 }
exit 0
