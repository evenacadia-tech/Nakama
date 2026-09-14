<#
NAK-286 Etappe 2 - Rotskript der Matrixzeilen (Manifest docs/beweise/NAK-286.md, §5.1).

Aufruf aus der Repo-Wurzel (pwsh):
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe2.ps1                 alle Mutationen
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe2.ps1 -Nur M-24,M-31  nur diese Zeilen
  pwsh -NoProfile -File docs/beweise/roh/NAK-286-rotskript-etappe2.ps1 -NurPruefen     nur Traegertexte zaehlen

Je Mutation, in dieser Reihenfolge (§5.1):
  1. SHA-256 jeder betroffenen Quelle, Originalbytes gesichert (Speicher und %TEMP%\nak286-rot).
  2. Mutation genau an der Traegerzeile: jeder Alttext muss genau einmal vorkommen.
  3. LastWriteTime = jetzt (NAK-230), Bau EqCopBriefkastenTest, Binary juenger als die Quelle.
  4. Bein B30 roh: gueltig nur mit Exit ungleich 0 und einer [ROT]-Zeile, die den Traeger nennt.
  5. Bytegleiche Ruecknahme aus den gesicherten Bytes, SHA-256 gleich Schritt 1,
     LastWriteTime = jetzt, Bau, Binary juenger, B30 gruen (Exit 0).
Rohausgabe: docs/beweise/roh/NAK-286-rot-M-<nn>.txt, je Mutation ein Abschnitt.
Eine ungueltige Mutation bricht den Lauf nach der Ruecknahme ab.
#>
param([string[]] $Nur = @(), [switch] $NurPruefen)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$wurzel = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
Set-Location $wurzel
$cmake = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$exe = Join-Path $wurzel 'eq-copilot\build\plugin\EqCopBriefkastenTest_artefacts\Release\EqCopBriefkastenTest.exe'
$tmp = Join-Path $env:TEMP 'nak286-rot'
New-Item -ItemType Directory -Force $tmp | Out-Null
$utf8 = New-Object System.Text.UTF8Encoding($false)

$BK = 'eq-copilot/plugin/core/diagnose/Briefkasten.cpp'
$BH = 'eq-copilot/plugin/core/diagnose/Briefkasten.h'
$DA = 'eq-copilot/plugin/src/DiagnoseAntwort.cpp'
$AN = 'eq-copilot/plugin/src/prozessor/Analyse.cpp'
$PP = 'eq-copilot/plugin/src/PluginProcessor.cpp'
$SP = 'eq-copilot/plugin/sonde/SondeProcessor.cpp'
$AE = 'eq-copilot/plugin/src/AnalyseEngine.cpp'
$FR = 'eq-copilot/plugin/core/analysis/featureengine/Frame.h'
$ZE = 'eq-copilot/plugin/core/analysis/featureengine/Zeit.h'

function Ersetzung ([string] $datei, [string] $alt, [string] $neu) { [pscustomobject]@{ Datei = $datei; Alt = $alt; Neu = $neu } }
function Mutation ([string] $zeile, [string] $teil, [string] $beschreibung, [string] $traeger, [int] $mindestens, [object[]] $ersetzungen)
{
    [pscustomobject]@{ Zeile = $zeile; Teil = $teil; Beschreibung = $beschreibung; Traeger = $traeger; Mindestens = $mindestens; Ersetzungen = $ersetzungen }
}

# Die Mutationen, je an der Zeile, die die Zusage traegt (Rotbeweisspalte der Matrix).
$mutationen = New-Object System.Collections.Generic.List[object]

$mutationen.Add((Mutation 'M-23' '' 'Existenzpruefung entfernt: der Takt oeffnet anfrage.json direkt' 'M-23 ohne_anfrage_nur_existenzpruefung (gen|probeeq) \(600 Takte' 2 @(
    (Ersetzung $BK '        const auto a = fs->attribute (anfrage);' @'
        // ROTBEWEIS M-23: Existenzpruefung entfernt, der Takt oeffnet anfrage.json direkt
        DateiAttribute a;
        {
            std::array<char, kAnfrageGrenzeBytes + 1> vorab;
            std::size_t vorabGelesen = 0;
            if (fs->lies (anfrage, vorab.data(), vorab.size(), vorabGelesen))
            {
                a.existiert = true;
                a.groesse = (std::int64_t) vorabGelesen;
            }
        }
'@))))

$mutationen.Add((Mutation 'M-24' '' 'Schreibaufruf bei gueltiger Anfrage uebersprungen' 'M-24 anfrage_gen_genau_eine_antwort genau_eine_datei_im_ersten_takt' 1 @(
    (Ersetzung $BK '        if (schreibe (kennung, antwort))' '        if (false && schreibe (kennung, antwort))   // ROTBEWEIS M-24: Schreibaufruf uebersprungen'))))

$mutationen.Add((Mutation 'M-25' '' 'Ringabfrage entfernt' 'M-25 gleiche_kennung_keine_zweite_antwort 0_anlegeversuche' 1 @(
    (Ersetzung $BK '        if (ringEnthaelt (kennung))' '        if (false && ringEnthaelt (kennung))   // ROTBEWEIS M-25: Ringabfrage entfernt'))))

$mutationen.Add((Mutation 'M-26' '' 'Merker "beantwortet" an der Datei statt an der Kennung' 'M-26 neue_kennung_neue_antwort genau_eine_datei_fuer_L' 1 @(
    (Ersetzung $BK '    std::atomic<bool>     taktLaeuft { false };' @'
    std::atomic<bool>     taktLaeuft { false };
    bool                  dateiBeantwortet = false;   // ROTBEWEIS M-26
'@),
    (Ersetzung $BK '        if (ringEnthaelt (kennung))' '        if (dateiBeantwortet || ringEnthaelt (kennung))   // ROTBEWEIS M-26: Merker an der Datei'),
    (Ersetzung $BK '            ringMerke (kennung);' @'
            ringMerke (kennung);
            dateiBeantwortet = true;   // ROTBEWEIS M-26
'@))))

$mutationen.Add((Mutation 'M-27' '(1)' 'Groessenpruefung vor dem Oeffnen entfernt' 'M-27 fremde_anfragen_werden_ignoriert \(a\) 1025 Bytes' 1 @(
    (Ersetzung $BK '            if (a.groesse < 0 || (std::uint64_t) a.groesse > kAnfrageGrenzeBytes)' '            if (false && (a.groesse < 0 || (std::uint64_t) a.groesse > kAnfrageGrenzeBytes))   // ROTBEWEIS M-27 (1): Groessenpruefung entfernt'))))

$mutationen.Add((Mutation 'M-27' '(2)' 'Kennungsmuster auf "nicht leer" gelockert' 'M-27 fremde_anfragen_werden_ignoriert \(e\)' 1 @(
    (Ersetzung $BK '    if (zeichen == nullptr || anzahl != 32)' @'
    if (zeichen == nullptr || anzahl == 0)   // ROTBEWEIS M-27 (2): Kennungsmuster auf "nicht leer" gelockert
        return false;
    return true;
    if (true)
'@))))

$mutationen.Add((Mutation 'M-28' '' 'Lesefehler setzt den Merker aus F-10' 'M-28 gesperrte_anfrage_im_naechsten_takt naechster_takt_liest_und_antwortet' 1 @(
    (Ersetzung $BK '                return;   // gesperrt: kein Merker, der naechste Takt liest erneut (F-10)' @'
            {
                merker = { true, true, a.aenderungszeit, a.groesse, {} };   // ROTBEWEIS M-28: Lesefehler setzt den Merker
                return;
            }
'@))))

$mutationen.Add((Mutation 'M-29' '(1)' 'Versuchsgrenze entfernt' 'M-29 antwortordner_fehlt_drei_versuche \((a|b)\)' 1 @(
    (Ersetzung $BK '        if (offen.versuche >= kSchreibversuche)' '        if (false && offen.versuche >= kSchreibversuche)   // ROTBEWEIS M-29 (1): Versuchsgrenze entfernt'))))

$mutationen.Add((Mutation 'M-29' '(2)' 'Ordneranlage im Takt' 'M-29 antwortordner_fehlt_drei_versuche \(a\) antwort fehlt' 1 @(
    (Ersetzung $BK '        if (! ebenenIntakt (4))' @'
        if (! fs->attribute (ebene[3]).existiert)
            fs->legeOrdnerAn (ebene[3]);   // ROTBEWEIS M-29 (2): Ordneranlage im Takt
        if (! ebenenIntakt (4))
'@))))

$mutationen.Add((Mutation 'M-30' '' 'Briefkastenstart an den Editor gebunden: der Timer startet nur mit geoeffnetem Editor (beide Prozessoren)' 'M-30 antwort_ohne_editor (gen|probeeq) antwortet' 2 @(
    (Ersetzung $PP '    return briefkasten.starte (konfiguration, mitTimer);' '    return briefkasten.starte (konfiguration, mitTimer && getActiveEditor() != nullptr);   // ROTBEWEIS M-30: Takt nur mit Editor'),
    (Ersetzung $SP '    return briefkasten.starte (konfiguration, mitTimer);' '    return briefkasten.starte (konfiguration, mitTimer && getActiveEditor() != nullptr);   // ROTBEWEIS M-30: Takt nur mit Editor'))))

$mutationen.Add((Mutation 'M-31' '' 'je Rolle Abbruch ohne Messdaten wie im Knopfweg (keine Datei)' 'M-31 ohne_messdaten_antwort_mit_grund (gen|probeeq) \(im' 2 @(
    (Ersetzung $AN '    auszug.framesGebaut  = merkmalFrames.load();' @'
    if (m.zustand == MessZustand::keineDaten)
        return {};   // ROTBEWEIS M-31: Abbruch ohne Messdaten wie im Knopfweg
    auszug.framesGebaut  = merkmalFrames.load();
'@),
    (Ersetzung $SP '    auszug.framesGebaut  = framesGebaut.load();' @'
    if (m.zustand == eqcop::MessZustand::keineDaten)
        return {};   // ROTBEWEIS M-31: Abbruch ohne Messdaten wie im Knopfweg
    auszug.framesGebaut  = framesGebaut.load();
'@))))

$mutationen.Add((Mutation 'M-32' '' 'Rahmenquelle letzterProducerFrame statt merkmale.frame()' 'M-32 probeeq_antwort_ohne_v3_verbindung frame_aus_der_engine' 1 @(
    (Ersetzung $SP '        auszug.rahmen      = merkmale.frame();' '        auszug.rahmen      = letzterProducerFrame;   // ROTBEWEIS M-32: Rahmenquelle letzterProducerFrame'))))

$mutationen.Add((Mutation 'M-33' '(1)' 'Laufzeitkennung aus dem Antwortnamen entfernt' 'M-33 instanzen_je_eine_datei \((b|c)\)' 2 @(
    (Ersetzung $BK '                                 + instanz16 + "." + laufzeit32 + ".json");' '                                 + instanz16 + /* ROTBEWEIS M-33 (1): ohne Laufzeitkennung */ ".json");'))))

$mutationen.Add((Mutation 'M-33' '(2)' 'Umbenennen mit Ersetzen (MOVEFILE_REPLACE_EXISTING)' 'M-33 instanzen_je_eine_datei \(d\) keine datei wird je ersetzt' 1 @(
    (Ersetzung $BK '    return MoveFileExW (von.c_str(), nach.c_str(), MOVEFILE_WRITE_THROUGH) != FALSE;' '    return MoveFileExW (von.c_str(), nach.c_str(), MOVEFILE_WRITE_THROUGH | MOVEFILE_REPLACE_EXISTING) != FALSE;   // ROTBEWEIS M-33 (2)'))))

$mutationen.Add((Mutation 'M-34' '' 'Schleuse im Destruktor entfernt' 'M-34 zerstoerung_bei_laufendem_takt angehaltener takt abgewiesen' 1 @(
    (Ersetzung $BK '    k.schleuse.schliessen();' '    // ROTBEWEIS M-34: Schleuse im Destruktor entfernt'))))

$mutationen.Add((Mutation 'M-35' '' 'Takt aus processBlock gerufen (beide Prozessoren)' 'M-35 nulltest_mit_briefkasten 0 fassadenaufrufe' 1 @(
    (Ersetzung $PP @'
void EqCopilotProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
'@ @'
void EqCopilotProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    briefkasten.takt();   // ROTBEWEIS M-35: Takt aus processBlock
'@),
    (Ersetzung $SP @'
void SondeProcessor::processBlock (juce::AudioBuffer<float>& puffer, juce::MidiBuffer&)
{
'@ @'
void SondeProcessor::processBlock (juce::AudioBuffer<float>& puffer, juce::MidiBuffer&)
{
    briefkasten.takt();   // ROTBEWEIS M-35: Takt aus processBlock
'@))))

$mutationen.Add((Mutation 'M-36' '(1)' 'startTimerHz (30) statt startTimer (1000)' 'M-36 kadenz_hoechstens_ein_hertz intervall 1000 ms' 1 @(
    (Ersetzung $BK '        startTimer (kTaktMs);' '        startTimerHz (30);   // ROTBEWEIS M-36 (1)'))))

$mutationen.Add((Mutation 'M-36' '(2)' 'Takt auf einem eigenen Thread statt ueber den Timer' 'M-36 kadenz_hoechstens_ein_hertz barriere' 1 @(
    (Ersetzung $BK '#include <ctime>' @'
#include <ctime>
#include <thread>   // ROTBEWEIS M-36 (2)
'@),
    (Ersetzung $BK @'
void Briefkasten::timerCallback()
{
    const auto k = kern;
    Kern::takt (k);
}
'@ @'
void Briefkasten::timerCallback()
{
    // ROTBEWEIS M-36 (2): Takt auf einem eigenen Thread statt ueber den Timer
    const auto k = kern;
    std::thread ([k] { Kern::takt (k); }).detach();
}
'@))))

$mutationen.Add((Mutation 'M-37' '' 'Antwortbau veraendert den gespeicherten Zustand (Label), beide Prozessoren' 'M-37 state_und_parameter_unveraendert (gen|probeeq) \(state' 2 @(
    (Ersetzung $AN '        u.instanzId = zustand.common.instanceId;' @'
        u.instanzId = zustand.common.instanceId;
        zustand.common.label = zustand.common.label + "+";   // ROTBEWEIS M-37: Antwortbau veraendert den Zustand
'@),
    (Ersetzung $SP '        sensor.paarId   = c.pairId;' @'
        sensor.paarId   = c.pairId;
        zustand.common.label = zustand.common.label + "+";   // ROTBEWEIS M-37: Antwortbau veraendert den Zustand
'@))))

$mutationen.Add((Mutation 'M-38' '' 'Briefkastenstart ohne Bauzweig, auch im Testbau (beide Prozessoren)' 'M-38 testbau_startet_keinen_briefkasten startfenster' 1 @(
    (Ersetzung $PP @'
    briefkastenStarten (true);
#endif
'@ @'
#endif
    briefkastenStarten (true);   // ROTBEWEIS M-38: Start ohne Bauzweig
'@),
    (Ersetzung $SP @'
    briefkastenStarten (true);
#endif
'@ @'
#endif
    briefkastenStarten (true);   // ROTBEWEIS M-38: Start ohne Bauzweig
'@))))

$mutationen.Add((Mutation 'M-39' '' 'Feldreihenfolge in der herausgeloesten Rechnung vertauscht' 'M-39 festhalten_bytegleich' 1 @(
    (Ersetzung $DA @'
    wurzel->setProperty ("aktiv_sekunden", m.aktivSekunden);
    wurzel->setProperty ("gesamt_sekunden", m.gesamtSekunden);
'@ @'
    wurzel->setProperty ("gesamt_sekunden", m.gesamtSekunden);   // ROTBEWEIS M-39: Feldreihenfolge vertauscht
    wurzel->setProperty ("aktiv_sekunden", m.aktivSekunden);
'@))))

$mutationen.Add((Mutation 'M-40' '' 'zusaetzlicher Schluessel debug im Umschlag' 'M-40 antwort_feldmenge_wie_schema M-' 1 @(
    (Ersetzung $DA '    o->setProperty ("gruende", u.gruende);' @'
    o->setProperty ("gruende", u.gruende);
    o->setProperty ("debug", true);   // ROTBEWEIS M-40: zusaetzlicher Schluessel
'@))))

$mutationen.Add((Mutation 'M-48' '' 'Kennungsmuster auf "32 beliebige Zeichen" gelockert' 'M-48 pfadtraversal_unmoeglich \.\.\\waechter\\ \(32 Zeichen\)' 1 @(
    (Ersetzung $BK '    if (zeichen == nullptr || anzahl != 32)' @'
    if (zeichen == nullptr || anzahl != 32)
        return false;
    return true;   // ROTBEWEIS M-48: 32 beliebige Zeichen
    if (true)
'@))))

$mutationen.Add((Mutation 'M-49' '' 'Attributpruefung (Reparse-Punkt) entfernt' 'M-49 reparse_punkte_werden_ignoriert \((a|b)\)' 2 @(
    (Ersetzung $BK '    return a.reparse;' '    return false && a.reparse;   // ROTBEWEIS M-49: Attributpruefung entfernt'))))

$mutationen.Add((Mutation 'M-50' '' 'Label roh in den JSON-Text gesetzt' 'M-50 label_mit_steuerzeichen_bleibt_gueltiges_json antwort ist gueltiges' 1 @(
    (Ersetzung $DA @'
    const auto text = juce::JSON::toString (juce::var (o), true);
    return text.toStdString();
'@ @'
    auto text = juce::JSON::toString (juce::var (o), true);
    // ROTBEWEIS M-50: Label roh in den JSON-Text gesetzt
    if (auto* sn = u.snapshot.getDynamicObject())
    {
        const auto roh = sn->getProperty ("sensor")["label"].toString();
        text = text.replace (juce::JSON::toString (juce::var (roh), true), "\"" + roh + "\"");
    }
    return text.toStdString();
'@))))

$mutationen.Add((Mutation 'M-51' '(1)' 'Ringgroesse: Ring mit einem Eintrag' 'M-51 kennungsring_fifo_256 zweitjuengste_erneut_0_anlegeversuche' 1 @(
    (Ersetzung $BH 'inline constexpr int         kKennungsring       = 256;    ///< F-13' 'inline constexpr int         kKennungsring       = 1;      ///< F-13 ROTBEWEIS M-51 (1): Ring mit einem Eintrag'))))

$mutationen.Add((Mutation 'M-51' '(2)' 'Einmaligkeit: Ring vor dem Schreiben nicht gefragt' 'M-51 kennungsring_fifo_256 zweitjuengste_erneut_0_anlegeversuche' 1 @(
    (Ersetzung $BK '        if (ringEnthaelt (kennung))' '        if (false && ringEnthaelt (kennung))   // ROTBEWEIS M-51 (2): Ring vor dem Schreiben nicht gefragt'))))

$mutationen.Add((Mutation 'M-52' '' 'Zuordnung ueber "neuer als die letzte Antwort" nach der Uhr' 'M-52 uhrsprung_aendert_zuordnung_nicht' 1 @(
    (Ersetzung $BK '    std::atomic<bool>     taktLaeuft { false };' @'
    std::atomic<bool>     taktLaeuft { false };
    std::int64_t          letzteAntwortMs = -1;   // ROTBEWEIS M-52
'@),
    (Ersetzung $BK '        anf.erzeugtUtc = isoUtc (uhr->jetztUtcMs());' @'
        if (letzteAntwortMs >= 0 && uhr->jetztUtcMs() <= letzteAntwortMs)
            return;   // ROTBEWEIS M-52: nur neuer als die letzte Antwort
        anf.erzeugtUtc = isoUtc (uhr->jetztUtcMs());
'@),
    (Ersetzung $BK '            ringMerke (kennung);' @'
            ringMerke (kennung);
            letzteAntwortMs = uhr->jetztUtcMs();   // ROTBEWEIS M-52
'@))))

$mutationen.Add((Mutation 'M-53' '(a)' 'direkt in den Antwortnamen geschrieben' 'M-53 keine_leere_antwort \(a\)' 1 @(
    (Ersetzung $BK '        const auto temp = ziel + L".tmp-" + breit (std::to_string (pid));' '        const auto temp = ziel;   // ROTBEWEIS M-53 (a): direkt in den Antwortnamen'))))

$mutationen.Add((Mutation 'M-53' '(b)' 'Aufraeumer loescht die Temp-Datei, wenn das Umbenennen scheitert' 'M-53 keine_leere_antwort \(b\)' 1 @(
    (Ersetzung $BK '        return fs->benenneUmOhneErsetzen (temp, ziel);' @'
        if (fs->benenneUmOhneErsetzen (temp, ziel))
            return true;
        fs->loesche (temp);   // ROTBEWEIS M-53 (b): Aufraeumer loescht die Temp-Datei
        return false;
'@))))

$mutationen.Add((Mutation 'M-54' '' 'Sperre ueber den Haken zwischen Kopie und Schreiben gehalten (beide Prozessoren)' 'M-54 langsamer_takt_ohne_audiowirkung (gen|probeeq): beim schreiben' 2 @(
    (Ersetzung $AN @'
        auszug.bloeckeMax  = material.bloeckeMax();
    }
    auszug.framesGebaut  = merkmalFrames.load();
    auszug.schwerSamples = m.schwerVerarbeiteteSamples;
    auszug.samplerate    = m.samplerate;
    if (diagnoseHakenFuerTest)
        diagnoseHakenFuerTest();
'@ @'
        auszug.bloeckeMax  = material.bloeckeMax();
    // ROTBEWEIS M-54: die Sperre bleibt ueber den Haken hinaus gehalten
    auszug.framesGebaut  = merkmalFrames.load();
    auszug.schwerSamples = m.schwerVerarbeiteteSamples;
    auszug.samplerate    = m.samplerate;
    if (diagnoseHakenFuerTest)
        diagnoseHakenFuerTest();
    }
'@),
    (Ersetzung $SP @'
        auszug.bloeckeMax  = material.bloeckeMax();
    }
    auszug.framesGebaut  = framesGebaut.load();
    auszug.schwerSamples = m.schwerVerarbeiteteSamples;
    auszug.samplerate    = m.samplerate;
    if (diagnoseHakenFuerTest)
        diagnoseHakenFuerTest();
'@ @'
        auszug.bloeckeMax  = material.bloeckeMax();
    // ROTBEWEIS M-54: die Sperre bleibt ueber den Haken hinaus gehalten
    auszug.framesGebaut  = framesGebaut.load();
    auszug.schwerSamples = m.schwerVerarbeiteteSamples;
    auszug.samplerate    = m.samplerate;
    if (diagnoseHakenFuerTest)
        diagnoseHakenFuerTest();
    }
'@))))

$mutationen.Add((Mutation 'M-77' '(1)' 'Wurzel und Ebenen ungeprueft benutzt: der Takt startet trotz Startfehler' 'M-77 startfehler_wurzel_ohne_takt .*taktfenster' 2 @(
    (Ersetzung $BK @'
    if (! k.wurzel->wurzel (wurzelPfad) || wurzelPfad.empty())
        return setze (Startgrund::keineWurzel);
'@ '    k.wurzel->wurzel (wurzelPfad);   // ROTBEWEIS M-77 (1): Wurzel ungeprueft benutzt'),
    (Ersetzung $BK @'
        if (istVerlinkt (a))
            return setze (Startgrund::reparsePunkt);
'@ @'
        if (istVerlinkt (a))
            break;   // ROTBEWEIS M-77 (1): Ebenen ungeprueft benutzt
'@))))

$mutationen.Add((Mutation 'M-77' '(2)' 'Ebenenpruefung an der Fassade vorbei (GetFileAttributesExW direkt)' 'M-77 startfehler_wurzel_ohne_takt durchlauf 2 \(evenacadia reparse\) (gen|probeeq) startfenster' 2 @(
    (Ersetzung $BK '        const auto a = k.fs->attribute (pfad);' @'
        // ROTBEWEIS M-77 (2): Ebenenpruefung an der Fassade vorbei
        DateiAttribute a;
        {
            WIN32_FILE_ATTRIBUTE_DATA daten {};
            if (GetFileAttributesExW (pfad.c_str(), GetFileExInfoStandard, &daten) != FALSE)
            {
                a.existiert   = true;
                a.verzeichnis = (daten.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                a.reparse     = (daten.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
            }
        }
'@))))

$mutationen.Add((Mutation 'M-79' '(1)' 'Summenpaar in evidenzLeeren genullt; evidenzLeeren laeuft im selben Rahmenbau direkt nach dem Zuwachs, jeder Abschluss traegt deshalb +0 und die Summe nie mehr als 0 (Traeger: Zuwachs je Evidenzrahmen)' 'M-79 summenzaehler_ueber_abschluesse_und_grenze je_evidenzrahmen_genau_dessen_fensterzaehler' 1 @(
    (Ersetzung $FR @'
    evidenzFensterGesamt = 0;
    evidenzFensterAktiv = 0;
    evidenzSamples = 0;
'@ @'
    evidenzFensterGesamt = 0;
    evidenzFensterAktiv = 0;
    zSummeFensterGesamt = 0;   // ROTBEWEIS M-79 (1): Summe in evidenzLeeren genullt
    zSummeFensterAktiv = 0;
    evidenzSamples = 0;
'@))))

$mutationen.Add((Mutation 'M-79' '(2)' 'Summenpaar in grenzeZiehen genullt (Zeit.h voruebergehend mutiert, bytegleich zurueckgenommen)' 'M-79 summenzaehler_ueber_abschluesse_und_grenze an_der_grenze_stehen_sie' 1 @(
    (Ersetzung $ZE @'
    evidenzFensterGesamt = 0;
    evidenzFensterAktiv = 0;
    // SONDE-013 M-11: die Stereoevidenz ist ein Fenster wie jedes andere.
'@ @'
    evidenzFensterGesamt = 0;
    evidenzFensterAktiv = 0;
    zSummeFensterGesamt = 0;   // ROTBEWEIS M-79 (2): Summe in grenzeZiehen genullt
    zSummeFensterAktiv = 0;
    // SONDE-013 M-11: die Stereoevidenz ist ein Fenster wie jedes andere.
'@))))

$mutationen.Add((Mutation 'M-80' '(1)' 'Aufruf auswerten() im Worker entfernt, das Flag wird weiter geloescht' 'M-80 probeeq_publiziert_auf_anfrage \(b\) naechster takt schreibt' 1 @(
    (Ersetzung $SP '                analyseEngine.auswerten();' '                // ROTBEWEIS M-80 (1): Aufruf auswerten() entfernt'))))

$mutationen.Add((Mutation 'M-80' '(2)' 'Auswertung in jedem Workerdurchlauf statt auf Anfrage' 'M-80 probeeq_publiziert_auf_anfrage \(a\) 600 takte' 1 @(
    (Ersetzung $SP '            if (briefkastenAnfrage.exchange (false))' '            if (briefkastenAnfrage.exchange (false) || true)   // ROTBEWEIS M-80 (2): Auswertung ohne Anfrage'))))

$mutationen.Add((Mutation 'M-80' '(3)' 'Bedingung "nur nach einem Zug mit mindestens einem Block" wieder eingesetzt' 'M-80 probeeq_publiziert_auf_anfrage \(c1\)' 1 @(
    (Ersetzung $SP '            constexpr int kMaxBloeckeJeZug = 8;' @'
            constexpr int kMaxBloeckeJeZug = 8;
            int bloeckeImZug = 0;   // ROTBEWEIS M-80 (3)
'@),
    (Ersetzung $SP @'
                const auto* roh = analyseQueue.spitze();
                if (roh == nullptr)
                    break;
'@ @'
                const auto* roh = analyseQueue.spitze();
                if (roh == nullptr)
                    break;
                ++bloeckeImZug;   // ROTBEWEIS M-80 (3)
'@),
    (Ersetzung $SP '            if (briefkastenAnfrage.exchange (false))' '            if (bloeckeImZug > 0 && briefkastenAnfrage.exchange (false))   // ROTBEWEIS M-80 (3): nur nach einem Zug mit Block'))))

$mutationen.Add((Mutation 'M-80' '(4)' 'Flag nur nach einer Publikation geloescht' 'M-80 probeeq_publiziert_auf_anfrage \(c2\) ohne bloecke, ohne gueltige rate nach zwei weiteren' 1 @(
    (Ersetzung $SP @'
            if (briefkastenAnfrage.exchange (false))
            {
                material.festhalten();   // P-10: unmittelbar vor der Publikation
                analyseEngine.auswerten();
'@ @'
            if (briefkastenAnfrage.load())   // ROTBEWEIS M-80 (4): Flag nur nach einer Publikation geloescht
            {
                const auto revisionVorher = analyseEngine.snapshot().revision;
                material.festhalten();   // P-10: unmittelbar vor der Publikation
                analyseEngine.auswerten();
                if (analyseEngine.snapshot().revision != revisionVorher)
                    briefkastenAnfrage.store (false);
'@))))

$mutationen.Add((Mutation 'M-81' '(1)' 'Feld der schweren Materialzeit in fuelleBasis statt nur in auswerten() gesetzt' 'M-81 materialzeit_anker_leicht_und_schwer durchlauf [12].* gen schwer_sekunden' 1 @(
    (Ersetzung $AE '    s.verarbeiteteSamples = samplesGesamt;' @'
    s.verarbeiteteSamples = samplesGesamt;
    s.schwerVerarbeiteteSamples = samplesGesamt;   // ROTBEWEIS M-81 (1): in fuelleBasis gesetzt
'@))))

$mutationen.Add((Mutation 'M-81' '(2)' 'Materialende zur Kopierzeit statt vor der Publikation festgehalten (Gen)' 'M-81 materialzeit_anker_leicht_und_schwer durchlauf [12].* gen material_ende' 1 @(
    (Ersetzung $AN '        auszug.material    = material.festgehalten();' @'
        material.festhalten();   // ROTBEWEIS M-81 (2): Materialende zur Kopierzeit festgehalten
        auszug.material    = material.festgehalten();
'@))))

$mutationen.Add((Mutation 'M-81' '(4)' 'Summe aus den Blocklaengen statt aus den Stempeln gefuehrt' 'M-81 materialzeit_anker_leicht_und_schwer durchlauf 3 (gen|probeeq) teilbloecke' 2 @(
    (Ersetzung $DA '            if (! subtrahiere (block.projectSampleStart, letzterStart, differenz)' '            if (((differenz = (std::int64_t) letzteLaenge), false)   // ROTBEWEIS M-81 (4): Summe aus den Blocklaengen'))))

function Baue ([string] $marke)
{
    $log = Join-Path $tmp "bau-$marke.log"
    & $cmake --build eq-copilot/build --config Release --target EqCopBriefkastenTest *> $log
    return $LASTEXITCODE
}

function Fahre ([string] $ausgabe)
{
    $p = Start-Process -FilePath $exe -PassThru -NoNewWindow -RedirectStandardOutput $ausgabe -RedirectStandardError "$ausgabe.err"
    if (-not $p.WaitForExit(900000)) { $p.Kill(); return 99 }
    $p.WaitForExit()
    return $p.ExitCode
}

function JuengerAlsQuellen ([string[]] $dateien)
{
    $quelle = ($dateien | ForEach-Object { (Get-Item (Join-Path $wurzel $_)).LastWriteTime } | Measure-Object -Maximum).Maximum
    return (Get-Item $exe).LastWriteTime -gt $quelle
}

# Mit `pwsh -File` kommt `-Nur M-24,M-31` als EINE Zeichenkette an: an Kommas trennen.
$Nur = @($Nur | ForEach-Object { $_ -split ',' } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
$auswahl = @($mutationen | Where-Object { $Nur.Count -eq 0 -or $Nur -contains $_.Zeile })
$zusammenfassung = New-Object System.Collections.Generic.List[string]

foreach ($m in $auswahl)
{
    $name = ($m.Zeile + ' ' + $m.Teil).Trim()
    $dateien = @($m.Ersetzungen | ForEach-Object { $_.Datei } | Select-Object -Unique)
    $orig = @{}; $shaVor = @{}; $texte = @{}
    foreach ($d in $dateien)
    {
        $orig[$d] = [IO.File]::ReadAllBytes((Join-Path $wurzel $d))
        $shaVor[$d] = (Get-FileHash -Algorithm SHA256 -Path (Join-Path $wurzel $d)).Hash
        [IO.File]::WriteAllBytes((Join-Path $tmp ((Split-Path $d -Leaf) + '.orig')), $orig[$d])
        $texte[$d] = $utf8.GetString($orig[$d])
    }
    $diff = New-Object System.Collections.Generic.List[string]
    foreach ($e in $m.Ersetzungen)
    {
        $t = $texte[$e.Datei]
        $nl = if ($t.Contains("`r`n")) { "`r`n" } else { "`n" }
        $alt = $e.Alt -replace "`r?`n", $nl
        $neu = $e.Neu -replace "`r?`n", $nl
        $n = [regex]::Matches($t, [regex]::Escape($alt)).Count
        if ($n -ne 1) { throw "$name - Traegertext in $($e.Datei) $n-mal statt genau einmal: $($e.Alt)" }
        $i = $t.IndexOf($alt, [StringComparison]::Ordinal)
        $texte[$e.Datei] = $t.Substring(0, $i) + $neu + $t.Substring($i + $alt.Length)
        $diff.Add("Ersetzung in $($e.Datei):")
        foreach ($z in ($e.Alt -split "`r?`n")) { $diff.Add("  - $z") }
        foreach ($z in ($e.Neu -split "`r?`n")) { $diff.Add("  + $z") }
    }
    if ($NurPruefen) { Write-Output "PRUEFUNG OK $name"; continue }

    $shaMut = @{}
    $bauRot = -1; $exitRot = -1; $binRot = $false; $rotZeilen = @(); $traeger = @()
    $ausRot = Join-Path $tmp 'b30-rot.txt'
    try
    {
        foreach ($d in $dateien)
        {
            [IO.File]::WriteAllBytes((Join-Path $wurzel $d), $utf8.GetBytes($texte[$d]))
            (Get-Item (Join-Path $wurzel $d)).LastWriteTime = Get-Date
            $shaMut[$d] = (Get-FileHash -Algorithm SHA256 -Path (Join-Path $wurzel $d)).Hash
        }
        $bauRot = Baue (($m.Zeile + $m.Teil) -replace '[^A-Za-z0-9-]', '')
        $binRot = ($bauRot -eq 0) -and (JuengerAlsQuellen $dateien)
        if ($binRot) { $exitRot = Fahre $ausRot }
        if (Test-Path $ausRot)
        {
            $rotZeilen = @(Get-Content $ausRot | Where-Object { $_.StartsWith('[ROT] ') })
            $traeger = @($rotZeilen | Where-Object { $_ -match $m.Traeger })
        }
    }
    finally
    {
        foreach ($d in $dateien)
        {
            [IO.File]::WriteAllBytes((Join-Path $wurzel $d), $orig[$d])
            (Get-Item (Join-Path $wurzel $d)).LastWriteTime = Get-Date
        }
    }
    $shaNach = @{}; $shaGleich = $true
    foreach ($d in $dateien)
    {
        $shaNach[$d] = (Get-FileHash -Algorithm SHA256 -Path (Join-Path $wurzel $d)).Hash
        if ($shaNach[$d] -ne $shaVor[$d]) { $shaGleich = $false }
    }
    $bauGruen = Baue (($m.Zeile + $m.Teil + 'gruen') -replace '[^A-Za-z0-9-]', '')
    $binGruen = ($bauGruen -eq 0) -and (JuengerAlsQuellen $dateien)
    $ausGruen = Join-Path $tmp 'b30-gruen.txt'
    $exitGruen = if ($binGruen) { Fahre $ausGruen } else { -1 }
    $summeGruen = if (Test-Path $ausGruen) { (Get-Content $ausGruen | Where-Object { $_ -like 'NAK-286 BRIEFKASTEN:*' } | Select-Object -Last 1) } else { '' }

    $gruende = @()
    if (-not $binRot) { $gruende += "Bau rot Exit $bauRot oder Binary nicht juenger" }
    if ($exitRot -eq 0 -or $exitRot -eq -1) { $gruende += "Lauf rot Exit $exitRot" }
    if ($traeger.Count -lt $m.Mindestens) { $gruende += "Traegerzeilen $($traeger.Count) statt mindestens $($m.Mindestens)" }
    if (-not $shaGleich) { $gruende += 'SHA nach der Ruecknahme ungleich' }
    if (-not $binGruen) { $gruende += "Bau gruen Exit $bauGruen oder Binary nicht juenger" }
    if ($exitGruen -ne 0) { $gruende += "Lauf gruen Exit $exitGruen" }
    $ergebnis = if ($gruende.Count -eq 0) { 'ROTBEWEIS GUELTIG' } else { 'ROTBEWEIS UNGUELTIG (' + ($gruende -join '; ') + ')' }

    $roh = Join-Path $wurzel ('docs/beweise/roh/NAK-286-rot-' + $m.Zeile + '.txt')
    $zeilen = New-Object System.Collections.Generic.List[string]
    $zeilen.Add("=== NAK-286 Etappe 2, Rotbeweis $name ($(Get-Date -Format 'dd.MM.yyyy HH:mm:ss')) ===")
    $zeilen.Add("Mutation: $($m.Beschreibung)")
    $zeilen.Add("Traeger (Muster der [ROT]-Zeile, mindestens $($m.Mindestens)): $($m.Traeger)")
    foreach ($z in $diff) { $zeilen.Add($z) }
    foreach ($d in $dateien)
    {
        $zeilen.Add("SHA-256 $d vorher $($shaVor[$d]), mutiert $($shaMut[$d]), nach der Ruecknahme $($shaNach[$d]), gleich: $(if ($shaNach[$d] -eq $shaVor[$d]) { 'ja' } else { 'NEIN' })")
    }
    $zeilen.Add("Bau rot: Exit $bauRot, Binary juenger als die Quelle: $(if ($binRot) { 'ja' } else { 'nein' })")
    $zeilen.Add("B30 rot: Exit $exitRot, [ROT]-Zeilen $($rotZeilen.Count), davon Traeger $($traeger.Count)")
    foreach ($z in $traeger) { $zeilen.Add("  Traeger: $z") }
    $zeilen.Add('--- Rohausgabe B30, mutierter Bau ---')
    if (Test-Path $ausRot) { foreach ($z in (Get-Content $ausRot)) { $zeilen.Add($z) } }
    $zeilen.Add('--- Ende der Rohausgabe ---')
    $zeilen.Add("Bau gruen: Exit $bauGruen, Binary juenger als die Quelle: $(if ($binGruen) { 'ja' } else { 'nein' })")
    $zeilen.Add("B30 gruen: Exit $exitGruen, $summeGruen")
    $zeilen.Add("Ergebnis: $ergebnis")
    $zeilen.Add('')
    $text = ($zeilen -join "`n") + "`n"
    # Anhaengen, nie ueberschreiben: ein frueherer Lauf derselben Zeile bleibt als Verlauf stehen.
    [IO.File]::AppendAllText($roh, $text, $utf8)

    $shaText = ($dateien | ForEach-Object { "$(Split-Path $_ -Leaf) $($shaVor[$_].Substring(0,12))=$($shaNach[$_].Substring(0,12))" }) -join ', '
    $zeile = "$name | rot Exit $exitRot, Traeger $($traeger.Count) | gruen Exit $exitGruen | $shaText | $ergebnis"
    $zusammenfassung.Add($zeile)
    Write-Output $zeile
    if ($gruende.Count -gt 0) { break }
}

[IO.File]::WriteAllText((Join-Path $tmp 'zusammenfassung.txt'), (($zusammenfassung -join "`n") + "`n"), $utf8)
Write-Output "ROTSKRIPT ENDE: $($zusammenfassung.Count) Mutationen, ungueltig: $(@($zusammenfassung | Where-Object { $_ -like '*UNGUELTIG*' }).Count)"
