// SONDE-003 — Wrapper-Unitfixture der Nakama-Hostbruecke.
//
// Zwei Haelften, beide sind das Ticket-Gate (Entwurf §65):
//
//   A · QUELLHASH-GATE  — misst die JUCE-Datei AUF PLATTE gegen den gepinnten
//       gepatchten Quellstand und prueft, dass die Aufrufstellen wirklich dort
//       stehen. Faellt, sobald jemand den Patch zurueckdreht oder JUCE
//       aktualisiert wird. Ein Riegel, den man nicht fallen sieht, ist keiner —
//       deshalb prueft der Test zusaetzlich, dass sich der unberuehrte Hash vom
//       gepatchten unterscheidet.
//
//   B · BRUECKENLOGIK   — faehrt die Abbildung gegen den ECHTEN
//       Steinberg::Vst::ProcessContext (nicht gegen einen Nachbau): Anwesenheit,
//       Gueltigkeitsbits, Sortierung mit Sample-Offset, Ueberlauf, numerische
//       Raender, Buslatenz-Nulldeutung, Fallbackbit, Allokationsfreiheit.
//
// Exit 0 nur, wenn ALLE Pruefungen bestanden sind.

#include "NakamaHostBridge.h"
#include "NakamaBrueckeStand.h"

// Der echte SDK-Header. Dadurch wird die Abbildung gegen die WIRKLICHEN
// Flag-Werte und Feldnamen instanziiert (inklusive des SDK-Tippfehlers
// "continousTimeSamples") — ein nachgebauter Kontext koennte still falsch sein.
#include <pluginterfaces/vst/ivstprocesscontext.h>

#include <juce_core/juce_core.h>
#include <juce_cryptography/juce_cryptography.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <new>
#include <string>
#include <vector>

using namespace eqcop::hostbruecke;
using VstKontext = Steinberg::Vst::ProcessContext;

//==============================================================================
// Allokationszaehler: der Blockpfad der Bruecke darf NICHTS anfordern.
namespace
{
    bool          zaehleAllokationen = false;
    std::uint64_t allokationen       = 0;
}

void* operator new (std::size_t groesse)
{
    if (zaehleAllokationen) ++allokationen;
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}

void operator delete (void* p) noexcept { std::free (p); }
void operator delete (void* p, std::size_t) noexcept { std::free (p); }
void* operator new[] (std::size_t groesse)
{
    if (zaehleAllokationen) ++allokationen;
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete[] (void* p) noexcept { std::free (p); }
void operator delete[] (void* p, std::size_t) noexcept { std::free (p); }

//==============================================================================
namespace
{

int fehler = 0;
int geprueft = 0;

void pruefe (bool ok, const juce::String& name)
{
    ++geprueft;
    std::cout << (ok ? "  ok      " : "  FEHLER  ") << name << std::endl;
    if (! ok) ++fehler;
}

/** Senke, die den letzten Befund mitschreibt — ohne zu allokieren. */
struct MitschriftSenke final : public Senke
{
    void nakamaBlockEmpfangen (const Blockbefund& b) noexcept override
    {
        ++aufrufe;
        letzter = b;                       // Kopie ohne Zeigerbesitz
        anzahlKopie = b.anzahl < 1024 ? b.anzahl : 1024;
        for (std::uint32_t i = 0; i < anzahlKopie; ++i)
            kopie[i] = b.ereignisse[i];
        letzter.ereignisse = kopie;
    }

    std::uint64_t  aufrufe { 0 };
    Blockbefund    letzter {};
    ParameterEvent kopie[1024] {};
    std::uint32_t  anzahlKopie { 0 };
};

/** Zeilenende-normalisierter SHA-256 einer Textdatei — dieselbe Vorschrift wie
    in cmake/NakamaBruecke.cmake (CRLF -> LF, dann Hash ueber die UTF-8-Bytes). */
juce::String quellhash (const juce::File& datei, bool& gelesen)
{
    juce::MemoryBlock roh;
    gelesen = datei.loadFileAsData (roh);
    if (! gelesen) return {};

    std::string norm;
    norm.reserve (roh.getSize());
    const char* d = static_cast<const char*> (roh.getData());
    for (size_t i = 0; i < roh.getSize(); ++i)
    {
        if (d[i] == '\r' && i + 1 < roh.getSize() && d[i + 1] == '\n') continue;
        norm.push_back (d[i]);
    }

    return juce::SHA256 (norm.data(), norm.size()).toHexString();
}

VstKontext vollerKontext()
{
    VstKontext c {};
    std::memset (&c, 0, sizeof (c));
    c.state = VstKontext::kPlaying
            | VstKontext::kRecording
            | VstKontext::kCycleActive
            | VstKontext::kCycleValid
            | VstKontext::kTempoValid
            | VstKontext::kProjectTimeMusicValid
            | VstKontext::kBarPositionValid
            | VstKontext::kContTimeValid;
    c.sampleRate            = 48000.0;
    c.projectTimeSamples    = 123456;
    c.continousTimeSamples  = 999888;      // SDK-Schreibweise, kein Tippfehler von uns
    c.projectTimeMusic      = 8.25;
    c.barPositionMusic      = 8.0;
    c.cycleStartMusic       = 4.0;
    c.cycleEndMusic         = 12.0;
    c.tempo                 = 128.0;
    c.timeSigNumerator      = 4;
    c.timeSigDenominator    = 4;
    return c;
}

} // namespace

//==============================================================================
int main()
{
    // Kein ScopedJuceInitialiser noetig: dieser Test benutzt nur juce_core
    // (Datei, SHA-256) und die Bruecke selbst — keine Nachrichtenschleife.
    std::cout << "== A · Quellhash-Gate (JUCE " << NAKAMA_JUCE_TAG_TEXT << ") ==" << std::endl;
    {
        const juce::File wrapper (NAKAMA_JUCE_WRAPPER_DATEI);
        const juce::File patch   (NAKAMA_BRUECKE_PATCH_DATEI);

        pruefe (wrapper.existsAsFile(), "JUCE-Wrapper liegt am gepinnten Ort: " + wrapper.getFullPathName());
        pruefe (patch.existsAsFile(),   "Patchdatei liegt im Repo: " + patch.getFileName());

        const juce::String unberuehrt { NAKAMA_JUCE_WRAPPER_SHA_UNBERUEHRT_TEXT };
        const juce::String gepatcht   { NAKAMA_JUCE_WRAPPER_SHA_GEPATCHT_TEXT };
        pruefe (unberuehrt != gepatcht && unberuehrt.length() == 64 && gepatcht.length() == 64,
                "die beiden gepinnten Hashes sind verschieden und vollstaendig (der Patch ist nicht leer)");

        bool gelesen = false;
        const juce::String ist = quellhash (wrapper, gelesen);
        pruefe (gelesen, "Wrapper liess sich lesen");
        pruefe (ist == gepatcht,
                "Wrapper-Quellhash == gepinnter GEPATCHTER Stand (gemessen: " + ist.substring (0, 16) + "…)");
        pruefe (ist != unberuehrt, "Wrapper ist NICHT der unberuehrte Auslieferungsstand");

        const juce::String text = wrapper.loadFileAsString();

        // Die Aufrufstellen — der Hash allein saehe eine falsche Datei nicht an.
        pruefe (text.contains ("#include <NakamaHostBridge.h>"),  "Patch: Bruecken-Header eingebunden");
        pruefe (text.contains ("public Vst::IAudioPresentationLatency,"), "Patch: IAudioPresentationLatency als Basisklasse");
        pruefe (text.contains ("UniqueBase<Vst::IAudioPresentationLatency>{},"), "Patch: Interface auch im queryInterface");
        pruefe (text.contains ("setAudioPresentationLatencySamples"), "Patch: Buslatenz-Methode umgesetzt");
        pruefe (text.contains ("nakamaBruecke.beginneBlock"),     "Patch: Blockbeginn verdrahtet");
        pruefe (text.contains ("nakamaBruecke.kontextAus"),       "Patch: Context-Anwesenheit verdrahtet");
        pruefe (text.contains ("nakamaBruecke.kontextFehlt"),     "Patch: fehlender Context wird ausdruecklich gemeldet");
        pruefe (text.contains ("nakamaBruecke.punkt"),            "Patch: alle Parameterpunkte werden beobachtet");
        pruefe (text.contains ("nakamaBruecke.uebergib"),         "Patch: Uebergabe an die Senke verdrahtet");
        pruefe (text.contains ("eqcop::hostbruecke::Bruecke nakamaBruecke;"), "Patch: Bruecke als Wrapper-Member");
        pruefe (text.contains ("dynamic_cast<eqcop::hostbruecke::Senke*>"),   "Patch: Senke wird EINMAL beim Erzeugen aufgeloest");

        // Der Patch NIMMT NICHTS WEG: JUCEs eigener Weg (letzter Punkt an den
        // Parameter) steht unveraendert daneben. Das ist der Beweis, dass die
        // Bruecke reine Beobachtung ist und Gate 1 nicht beruehrt.
        pruefe (text.contains ("if (const auto change = getPointFromQueue (paramQueue, numPoints - 1))"),
                "Patch nimmt nichts weg: JUCEs eigener Parameterweg steht unveraendert");
        pruefe (text.contains ("#ifndef NAKAMA_HOST_BRIDGE") && text.contains (" #define NAKAMA_HOST_BRIDGE 0"),
                "ohne NAKAMA_HOST_BRIDGE bleibt der Wrapper wortgleich JUCE (Vorbelegung 0)");

        const juce::String patchText = patch.loadFileAsString();
        pruefe (patchText.startsWith ("diff --git "), "Patchdatei ist ein echter Unified Diff");
        pruefe (patchText.contains ("juce_audio_plugin_client_VST3.cpp"), "Patch zielt auf den VST3-Wrapper");
    }

    std::cout << "== B · Context-Anwesenheit und Gueltigkeitsbits ==" << std::endl;
    {
        Bruecke b;
        MitschriftSenke senke;
        b.verbinde (&senke);

        // (1) Context fehlt — ausdruecklich.
        b.beginneBlock (512);
        b.kontextFehlt();
        b.uebergib();
        pruefe (senke.aufrufe == 1, "Senke wird genau einmal je Block gerufen");
        pruefe (! senke.letzter.kontext.processContextPresent, "fehlender Context: processContextPresent == false");
        pruefe (! senke.letzter.kontext.projectTimeSamples.gueltig, "fehlender Context: Projektzeit ist UNGUELTIG, nicht 0");
        pruefe (! senke.letzter.kontext.tempo.gueltig && ! senke.letzter.kontext.ppqPosition.gueltig,
                "fehlender Context: Tempo und PPQ ungueltig");

        // (2) Der genullte Context — genau der Fall, den JUCEs oeffentlicher
        //     Pfad nicht von (1) unterscheiden kann.
        VstKontext leer {};
        std::memset (&leer, 0, sizeof (leer));
        b.beginneBlock (512);
        b.kontextAus (leer);
        b.uebergib();
        pruefe (senke.letzter.kontext.processContextPresent,
                "genullter, aber VORHANDENER Context: processContextPresent == true (der Kernbeweis)");
        pruefe (senke.letzter.kontext.projectTimeSamples.gueltig && senke.letzter.kontext.projectTimeSamples.wert == 0,
                "genullter Context: Projektzeit gueltig und 0 — eine Aussage, keine Vermutung");
        pruefe (senke.letzter.kontext.sampleRate.gueltig && senke.letzter.kontext.sampleRate.wert == 0.0,
                "genullter Context: Samplerate 0 wird roh gefuehrt (KEINE Sekundenrechnung, sonst 0/0 = NaN)");
        pruefe (! senke.letzter.kontext.tempo.gueltig, "genullter Context: Tempo bleibt ungueltig");
        pruefe (senke.letzter.kontext.playing.gueltig && ! senke.letzter.kontext.playing.wert,
                "genullter Context: playing ist gueltig false — nicht 'unbekannt'");

        // (3) Voller Context.
        const auto voll = vollerKontext();
        b.beginneBlock (512);
        b.kontextAus (voll);
        b.uebergib();
        const auto& k = senke.letzter.kontext;
        pruefe (k.processContextPresent, "voller Context: anwesend");
        pruefe (k.projectTimeSamples.gueltig && k.projectTimeSamples.wert == 123456, "voller Context: projectTimeSamples 123456");
        pruefe (k.continuousTimeSamples.gueltig && k.continuousTimeSamples.wert == 999888,
                "voller Context: continousTimeSamples 999888 (SDK-Schreibweise korrekt gelesen)");
        pruefe (k.playing.gueltig && k.playing.wert,     "voller Context: playing true");
        pruefe (k.recording.gueltig && k.recording.wert, "voller Context: recording true");
        pruefe (k.tempo.gueltig && k.tempo.wert == 128.0, "voller Context: Tempo 128");
        pruefe (k.ppqPosition.gueltig && k.ppqPosition.wert == 8.25, "voller Context: PPQ 8.25");
        pruefe (k.barPositionPpq.gueltig && k.barPositionPpq.wert == 8.0, "voller Context: Taktbeginn 8.0 PPQ");
        pruefe (k.cycle.gueltig && k.cycle.aktiv && k.cycle.startPpq == 4.0 && k.cycle.endePpq == 12.0,
                "voller Context: Cycle 4.0–12.0 PPQ, aktiv (PPQ, nicht Samples)");

        // (4) Teil-Context: nur playing. Alles andere MUSS ungueltig bleiben.
        VstKontext teil {};
        std::memset (&teil, 0, sizeof (teil));
        teil.state = VstKontext::kPlaying;
        teil.sampleRate = 44100.0;
        teil.projectTimeSamples = 4410;
        b.beginneBlock (512);
        b.kontextAus (teil);
        b.uebergib();
        const auto& t = senke.letzter.kontext;
        pruefe (t.playing.gueltig && t.playing.wert, "Teil-Context: playing true");
        pruefe (! t.tempo.gueltig && ! t.ppqPosition.gueltig && ! t.barPositionPpq.gueltig
                && ! t.continuousTimeSamples.gueltig && ! t.cycle.gueltig,
                "Teil-Context: Tempo/PPQ/Takt/ContTime/Cycle bleiben ungueltig (nichts erfunden)");
        pruefe (! t.cycle.aktiv, "Teil-Context: Cycle nicht aktiv");

        // (5) Gegenpfad: beginneBlock loescht den Context des Vorblocks.
        b.beginneBlock (512);
        b.uebergib();
        pruefe (! senke.letzter.kontext.processContextPresent && ! senke.letzter.kontext.tempo.gueltig,
                "beginneBlock loescht den Vorblock-Context (kein Nachleuchten alter Werte)");
    }

    std::cout << "== C · Parameterpunkte: Reihenfolge, Offsets, Ueberlauf ==" << std::endl;
    {
        Bruecke b;
        MitschriftSenke senke;
        b.verbinde (&senke);

        // (1) Verkehrt herum geliefert -> nach Offset sortiert.
        b.beginneBlock (512);
        b.punkt (7, 400, 0.9);
        b.punkt (7, 100, 0.1);
        b.punkt (7, 256, 0.5);
        b.uebergib();
        const auto& e = senke.letzter;
        pruefe (e.anzahl == 3, "drei Punkte angekommen");
        pruefe (e.ereignisse[0].sampleOffset == 100
             && e.ereignisse[1].sampleOffset == 256
             && e.ereignisse[2].sampleOffset == 400, "Punkte sind nach Sample-Offset sortiert");
        pruefe (e.sampleAccurateAutomation, "sauberer Block: sampleAccurateAutomation == true");
        pruefe (e.blockGroesse == 512, "Blockgroesse wird mitgefuehrt");

        // (2) Gleicher Offset -> Hostreihenfolge bleibt (stabil).
        b.beginneBlock (512);
        b.punkt (1, 64, 0.11);
        b.punkt (2, 64, 0.22);
        b.punkt (3, 64, 0.33);
        b.punkt (4, 32, 0.44);
        b.uebergib();
        const auto& s = senke.letzter;
        pruefe (s.ereignisse[0].id == 4, "kleinerer Offset zuerst, auch wenn spaeter geliefert");
        pruefe (s.ereignisse[1].id == 1 && s.ereignisse[2].id == 2 && s.ereignisse[3].id == 3,
                "gleicher Offset behaelt die Hostreihenfolge (stabil sortiert)");

        // (3) Mehrere Queues gemischt -> global sortiert.
        b.beginneBlock (256);
        b.punkt (10, 0,   0.0);
        b.punkt (10, 128, 1.0);
        b.punkt (20, 64,  0.5);
        b.punkt (20, 192, 0.75);
        b.uebergib();
        const auto& m = senke.letzter;
        bool aufsteigend = true;
        for (std::uint32_t i = 1; i < m.anzahl; ++i)
            if (m.ereignisse[i - 1].sampleOffset > m.ereignisse[i].sampleOffset) aufsteigend = false;
        pruefe (aufsteigend && m.anzahl == 4, "Punkte aus mehreren Queues sind global aufsteigend");

        // (4) Ueberlauf: der GANZE Block verliert die Samplegenauigkeit,
        //     der letzte Blockwert bleibt aber abrufbar (Entwurf §53.7).
        b.beginneBlock (4096);
        for (std::uint32_t i = 0; i < kMaxParameterEreignisse + 5; ++i)
            b.punkt (42, (std::int32_t) i, 0.001 * (double) i);
        b.punkt (42, 4095, 0.777);       // faellt ebenfalls dem Ueberlauf zum Opfer
        b.uebergib();
        const auto& u = senke.letzter;
        pruefe (u.anzahl == kMaxParameterEreignisse, "Ueberlauf: es bleiben genau kMaxParameterEreignisse Punkte");
        pruefe (u.ueberlaeufe == 6, "Ueberlauf: alle 6 verworfenen Punkte sind gezaehlt");
        pruefe (! u.sampleAccurateAutomation, "Ueberlauf: sampleAccurateAutomation faellt fuer den GANZEN Block");
        float letzter = -1.0f;
        pruefe (u.hatLetztenBlockwert (42, letzter), "Ueberlauf: letzter Blockwert bleibt abrufbar (Rueckfallweg)");

        // (5) Numerische Raender der Offsets.
        b.beginneBlock (128);
        b.punkt (5, -1, 0.5);
        b.uebergib();
        pruefe (senke.letzter.ereignisse[0].sampleOffset == 0 && senke.letzter.unplausibleOffsets == 1
                && ! senke.letzter.sampleAccurateAutomation,
                "negativer Offset: geklemmt auf 0, gezaehlt, Samplegenauigkeit faellt");

        b.beginneBlock (128);
        b.punkt (5, 128, 0.5);
        b.uebergib();
        pruefe (senke.letzter.ereignisse[0].sampleOffset == 127 && senke.letzter.unplausibleOffsets == 1
                && ! senke.letzter.sampleAccurateAutomation,
                "Offset == Blockgroesse: geklemmt auf letztes Sample, gezaehlt, Samplegenauigkeit faellt");

        b.beginneBlock (0);              // Parameter-Flush: numSamples == 0
        b.punkt (5, 0, 0.25);
        b.uebergib();
        pruefe (senke.letzter.blockGroesse == 0 && senke.letzter.unplausibleOffsets == 0
                && senke.letzter.sampleAccurateAutomation,
                "Flush-Block (Blockgroesse 0) mit Offset 0 ist plausibel");

        b.beginneBlock (0);
        b.punkt (5, 5, 0.25);
        b.uebergib();
        pruefe (senke.letzter.unplausibleOffsets == 1 && ! senke.letzter.sampleAccurateAutomation,
                "Flush-Block mit Offset != 0 wird als unplausibel gemeldet");

        // (6) Numerische Raender der Werte: NICHT sanitisieren, sondern melden.
        b.beginneBlock (128);
        b.punkt (6, 10, std::numeric_limits<double>::quiet_NaN());
        b.uebergib();
        const float nanWert = senke.letzter.ereignisse[0].normalisedValue;
        pruefe (senke.letzter.unplausibleWerte == 1 && ! senke.letzter.sampleAccurateAutomation,
                "NaN-Parameterwert: gezaehlt, Samplegenauigkeit faellt");
        pruefe (! (nanWert == nanWert), "NaN-Parameterwert bleibt NaN (verworfen wird die Zusicherung, nicht der Messwert)");

        b.beginneBlock (128);
        b.punkt (6, 10, std::numeric_limits<double>::infinity());
        b.uebergib();
        pruefe (senke.letzter.unplausibleWerte == 1 && ! senke.letzter.sampleAccurateAutomation,
                "Inf-Parameterwert: gezaehlt, Samplegenauigkeit faellt");

        // (7) Gegenpfad: der naechste saubere Block ist wieder samplegenau.
        b.beginneBlock (128);
        b.punkt (6, 10, 0.5);
        b.uebergib();
        pruefe (senke.letzter.sampleAccurateAutomation && senke.letzter.unplausibleWerte == 0
                && senke.letzter.ueberlaeufe == 0,
                "Zaehler und Fallbackbit werden je Block zurueckgesetzt");

        // (8) Blockstruktur: leerer Block ist gueltig, nicht 'kaputt'.
        b.beginneBlock (128);
        b.uebergib();
        pruefe (senke.letzter.anzahl == 0 && senke.letzter.sampleAccurateAutomation,
                "Block ohne Automation: 0 Punkte, Zusicherung bleibt gueltig");
    }

    std::cout << "== D · Buslatenz: die zwei Bedeutungen der Null ==" << std::endl;
    {
        Bruecke b;
        MitschriftSenke senke;
        b.verbinde (&senke);

        b.beginneBlock (64);
        b.uebergib();
        pruefe (! senke.letzter.kontext.presentationLatency.hole (true, 0).gemeldet
                && senke.letzter.kontext.presentationLatency.hole (true, 0).samples == 0,
                "ohne Hostmeldung: gemeldet == false (nicht '0 Samples Latenz')");

        b.setzeBuslatenz (true, 0, 0);
        b.beginneBlock (64);
        b.uebergib();
        pruefe (senke.letzter.kontext.presentationLatency.hole (true, 0).gemeldet
                && senke.letzter.kontext.presentationLatency.hole (true, 0).samples == 0,
                "Host meldet 0: gemeldet == true bei samples == 0 (die Trennung, um die es geht)");

        b.setzeBuslatenz (false, 1, 480);
        b.beginneBlock (64);
        b.uebergib();
        const auto& tab = senke.letzter.kontext.presentationLatency;
        pruefe (tab.hole (false, 1).gemeldet && tab.hole (false, 1).samples == 480, "Ausgangsbus 1: 480 Samples");
        pruefe (! tab.hole (false, 0).gemeldet, "Ausgangsbus 0 bleibt ungemeldet (keine Streuung)");
        pruefe (tab.hole (true, 0).gemeldet, "Eingangsbus 0 behaelt seine fruehere Meldung ueber Bloecke hinweg");

        const auto vorher = b.verworfeneBusmeldungen();
        b.setzeBuslatenz (true, -1, 100);
        b.setzeBuslatenz (false, kMaxBusse, 100);
        b.setzeBuslatenz (false, 9999, 100);
        pruefe (b.verworfeneBusmeldungen() == vorher + 3,
                "Busindex ausserhalb [0, kMaxBusse): verworfen und gezaehlt, kein Speicherfehler");
        pruefe (! tab.hole (true, -1).gemeldet && ! tab.hole (false, kMaxBusse).gemeldet,
                "Abfrage ausserhalb des Bereichs liefert einen leeren Eintrag");
    }

    std::cout << "== E · Fallbackbit und Realtime-Zusicherungen ==" << std::endl;
    {
        // (1) Ohne Senke passiert nichts — das ist der Weg, den JEDER heutige
        //     Processor geht (keiner implementiert die Senke). Kein Absturz,
        //     kein Nebeneffekt: der Patch ist fuer das Produkt folgenlos.
        Bruecke ohne;
        pruefe (! ohne.istVerbunden(), "ohne Senke: Bruecke meldet sich als unverbunden");
        ohne.beginneBlock (256);
        ohne.kontextAus (vollerKontext());
        ohne.punkt (1, 10, 0.5);
        ohne.uebergib();
        pruefe (true, "ohne Senke: beginneBlock/kontextAus/punkt/uebergib laufen folgenlos durch");

        // (2) Das Fallbackbit selbst: der Verbraucher darf ohne
        //     sampleAccurateAutomation nur vom vorigen zum LETZTEN Blockwert
        //     rampen. Genau dieser letzte Wert muss abrufbar bleiben.
        Bruecke b;
        MitschriftSenke senke;
        b.verbinde (&senke);
        b.beginneBlock (256);
        b.punkt (77, 0,   0.10);
        b.punkt (77, 128, 0.60);
        b.punkt (77, 255, 0.90);
        b.uebergib();
        float letzter = -1.0f;
        pruefe (senke.letzter.hatLetztenBlockwert (77, letzter) && letzter > 0.89f && letzter < 0.91f,
                "Rueckfallweg: hatLetztenBlockwert liefert den LETZTEN Wert des Blocks (0.90)");
        float fremd = -1.0f;
        pruefe (! senke.letzter.hatLetztenBlockwert (78, fremd),
                "Rueckfallweg: unbekannte Parameter-ID liefert ehrlich 'nichts', nicht 0");

        // (3) Allokationsfreiheit des Blockpfads.
        allokationen = 0;
        zaehleAllokationen = true;
        for (int block = 0; block < 200; ++block)
        {
            b.beginneBlock (512);
            b.kontextAus (vollerKontext());
            for (int i = 0; i < 64; ++i)
                b.punkt ((StableParameterId) (i % 8), (std::int32_t) ((i * 7919) % 512), 0.001 * i);
            b.setzeBuslatenz (true, 0, 128);
            b.uebergib();
        }
        zaehleAllokationen = false;
        pruefe (allokationen == 0,
                "200 Bloecke à 64 Punkte: 0 Allokationen im Blockpfad (gezaehlt, nicht behauptet)");

        pruefe (std::atomic<std::uint64_t>::is_always_lock_free,
                "64-Bit-Atomics sind lock-frei — kein versteckter Mutex im Audiothread");
        pruefe (sizeof (Bruecke) < 32u * 1024u,
                "Bruecke bleibt vorallokiert und klein (" + juce::String ((int) sizeof (Bruecke)) + " Byte)");
    }

    std::cout << std::endl;
    if (fehler == 0)
        std::cout << "HOSTKONTEXT OK — " << geprueft << "/" << geprueft << " Pruefungen bestanden." << std::endl;
    else
        std::cout << "HOSTKONTEXT FEHLGESCHLAGEN — " << fehler << " von " << geprueft << " Pruefungen rot." << std::endl;

    return fehler == 0 ? 0 : 1;
}
