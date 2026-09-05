/*  EqCopSonde013PassageStateTest — die manuelle Passage als Projektintent
    (SONDE-013 M-25, M-69).

    §33.5 teilt die Wahrheit in zwei Haelften, und dieses Bein misst die
    zweite: der Store ist autoritativ fuer die Passage als Evidenzobjekt,
    `MainProjectState` haelt sie als Intent, der mit der FL-Projektdatei
    reist.

    ── WARUM DAS EIN EIGENES BEIN BRAUCHT ───────────────────────────────────

    Weil ein neues persistentes Feld die teuerste Stelle im ganzen Plugin ist.
    Die State-Invariante aus CLAUDE.md nennt fuenf Bedingungen, und sie sind
    erst zusammen eine Zusage:

      1. zuerst versionieren      → der Eigenschaftsname traegt `_v1`;
      2. Altstaende laden         → ein Stand ohne das Feld bleibt gueltig;
      3. unbekannte Felder        → zerstoeren keinen alten Consumer und
                                    reisen beim naechsten Speichern zurueck;
      4. Save und Load GEMEINSAM  → Bytegleichheit ueber zwei Runden;
      5. Host-Dirty               → jede persistente Aenderung meldet sich.

    Ein Test, der nur speichert und laedt, misst Punkt 4 und laesst die
    uebrigen offen. Deshalb faehrt dieses Bein jeden einzeln.

    ── DIE SCHARFE STELLE: SCHREIBER UND LESER BRAUCHEN DIESELBEN RIEGEL ────

    Eine Produkt-API, die einen Stand schreibt, den der eigene Leser als
    read-only zurueckgibt, ist stiller Datenverlust beim naechsten Oeffnen.
    Genau dieser Fehler ist am 23.08. beim `parameters`-Feld aufgefallen (der
    Kommentar dazu steht bis heute in `NakamaState.h`). Der Abschnitt
    „Die Raender" faehrt deshalb jede Grenze von BEIDEN Seiten: die API lehnt
    ab, UND ein von Hand gebauter Baum mit demselben Wert wird read-only.
*/

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>

#include "HoerMarkierung.h"
#include "NakamaState.h"
#include "PluginProcessor.h"

#include <pluginterfaces/vst/ivstprocesscontext.h>

// NAK-180 Nacharbeit 2 (WN-08/EP-18/R3b): der v3-Probe-Server. Bis hierher lag
// er allein in `IpcTestMain.cpp`, und `r01Resync` konnte den Handschlag
// deshalb nur ueber Testhaken nachbilden.
#include "IpcVerbindung.h"
#include "WireEnvelope.h"
#include "PipeToken.h"
#if defined(_WIN32)
 #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
 #endif
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <windows.h>
#endif

#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

using namespace eqcop;
namespace state = nakama::state;

// SONDE-013 Nacharbeit 3 (Befund C1): der Hoermarker verlangt einen BEWIESEN
// nicht aufnehmenden Host, und der Aufnahmezustand kommt ausschliesslich ueber
// die Hostbruecke (M-33). Ohne sie koennte dieses Bein keinen Eingriff
// erzeugen und damit auch keine Sequenz messen.
using VstKontext = Steinberg::Vst::ProcessContext;

namespace
{
// Der Probe-Server spricht den v3-Bootstrap unqualifiziert.
using namespace nakama::ipc;
// Das GETEILTE Testdouble - dieselbe Datei, die EqCopIpcTest einbindet
// (NAK-180 Nacharbeit 2, WN-08). Es gehoert in den anonymen Namensraum.
#include "V3TestServer.h"

int bestanden = 0;
int fehler = 0;

void pruefe (bool ok, const juce::String& was, const juce::String& zusatz = {})
{
    std::cout << (ok ? "[ok]   " : "[ROT]  ") << was.toRawUTF8();
    if (zusatz.isNotEmpty())
        std::cout << "  [" << zusatz.toRawUTF8() << "]";
    std::cout << std::endl;
    ok ? ++bestanden : ++fehler;
}

void abschnitt (const char* name)
{
    std::cout << std::endl << "== " << name << " ==" << std::endl;
}

struct DirtyZaehler final : public juce::AudioProcessorListener
{
    int nonParam = 0;
    void audioProcessorParameterChanged (juce::AudioProcessor*, int, float) override {}
    void audioProcessorChanged (juce::AudioProcessor*, const ChangeDetails& d) override
    {
        if (d.nonParameterStateChanged) ++nonParam;
    }
};

juce::String hex32 (int n)
{
    return juce::String::toHexString (n).paddedLeft ('0', 32);
}

juce::var i64 (juce::int64 v) { return juce::var (v); }

/** Ein gueltiger Schema-2-Baum der Klasse `main`, damit die Passagen ueberhaupt
    einen Platz haben. */
juce::ValueTree mainBaum()
{
    juce::ValueTree v ("NakamaState");
    v.setProperty ("schema", 2, nullptr);
    juce::ValueTree c ("Common");
    c.setProperty ("schema", 1, nullptr);
    c.setProperty ("instance_id", hex32 (0x1234), nullptr);
    c.setProperty ("plugin_kind", "main", nullptr);
    c.setProperty ("measurement_position", "insert", nullptr);
    c.setProperty ("label", "Leitstand", nullptr);
    v.appendChild (c, nullptr);
    juce::ValueTree m ("MainProject");
    m.setProperty ("schema", 1, nullptr);
    v.appendChild (m, nullptr);
    return v;
}

juce::MemoryBlock alsBlock (const juce::ValueTree& v)
{
    juce::MemoryBlock b;
    juce::MemoryOutputStream s (b, false);
    v.writeToStream (s);
    s.flush();
    return b;
}

/** Setzt `manual_passages_v1` von Hand — so entstehen Staende, die die
    Produkt-API gar nicht erzeugen kann. Genau die misst der Leser. */
juce::MemoryBlock baumMitPassagen (const juce::Array<juce::var>& flach)
{
    auto v = mainBaum();
    v.getChildWithName ("MainProject")
        .setProperty ("manual_passages_v1", juce::var (flach), nullptr);
    return alsBlock (v);
}

bool gleich (const juce::MemoryBlock& a, const juce::MemoryBlock& b)
{
    return a.getSize() == b.getSize()
        && std::memcmp (a.getData(), b.getData(), a.getSize()) == 0;
}

std::unique_ptr<EqCopilotProcessor> mainProzessor()
{
    auto p = std::make_unique<EqCopilotProcessor>();
    const auto saat = alsBlock (mainBaum());
    p->setStateInformation (saat.getData(), (int) saat.getSize());
    return p;
}

// ═════════════════════════════════════════════════════════════════════════
// NACHARBEIT 3 · die Passage bindet, und sie bindet GENAU EINE
// ═════════════════════════════════════════════════════════════════════════
//
// 🔑 Wiederpruefung 2 (Befunde C2, C3, C4, C7): die Runde 2 hat den Weg von
// der markierten Passage zur Engine gebaut, aber als EINEN globalen Slot ohne
// Passagen-, Zeit- und Epochenbindung. Die Beine dieser Runde fahren genau
// die Konstellationen, in denen ein globaler Slot faellt: zwei Passagen, ein
// Seek zwischen Markierung und Workerlauf, und Wiedergabe AUSSERHALB des
// markierten Fensters.

/** Ein Transport, den der Test bewegt. Ohne ihn misst dieses Bein einen Pfad,
    den das Produkt seit SONDE-008 nicht mehr hat: der Vergleichspegel
    verlangt gueltige Projektzeit UND „spielt". */
struct TestPlayHead : juce::AudioPlayHead
{
    bool spielt = true;
    juce::int64 pos = 0;
    juce::Optional<PositionInfo> getPosition() const override
    {
        PositionInfo p;
        p.setIsPlaying (spielt);
        p.setTimeInSamples (pos);
        return p;
    }
};

constexpr int kBlock = 512;
constexpr double kFs = 48000.0;

/** Ein Main mit GUELTIGER v3-Adresse — sonst baut `versuchKopfJson` keinen
    Kopf und kein Versuch entsteht, egal wie gut der Pegel ist. */
juce::MemoryBlock mainBaumMitBindung()
{
    juce::ValueTree v ("NakamaState");
    v.setProperty ("schema", 2, nullptr);
    juce::ValueTree c ("Common");
    c.setProperty ("schema", 1, nullptr);
    c.setProperty ("instance_id", hex32 (0x1234), nullptr);
    c.setProperty ("project_binding_id", hex32 (0x55), nullptr);
    c.setProperty ("plugin_kind", "main", nullptr);
    c.setProperty ("measurement_position", "insert", nullptr);
    c.setProperty ("label", "Leitstand", nullptr);
    v.appendChild (c, nullptr);
    juce::ValueTree m ("MainProject");
    m.setProperty ("schema", 1, nullptr);
    v.appendChild (m, nullptr);
    juce::MemoryBlock b;
    juce::MemoryOutputStream s (b, false);
    v.writeToStream (s);
    s.flush();
    return b;
}

std::unique_ptr<EqCopilotProcessor> mainProzessorMitBindung()
{
    auto p = std::make_unique<EqCopilotProcessor>();
    const auto saat = mainBaumMitBindung();
    p->setStateInformation (saat.getData(), (int) saat.getSize());
    return p;
}

/** Eine klassifizierte Quelle mit bekanntem Messpunkt — ohne sie liefert
    `versuchReferenzJson` einen leeren Text und der Versuch entsteht nicht. */
eqcop::SourcesModel::Sicht eineQuelle()
{
    eqcop::SourcesModel::Sicht s;
    s.subscriptionAktiv = true;
    s.mainDarfSchreiben = true;
    s.fuehrendesMain = hex32 (0x1234).toStdString();
    eqcop::SourcesModel::Zeile q;
    q.instanceId = hex32 (0xA1).toStdString();
    q.runtimeNonce = hex32 (0xB1).toStdString();
    q.pluginKind = "active_probe";
    q.mitgliedschaft = eqcop::SourcesModel::Mitgliedschaft::bestaetigt;
    q.control = eqcop::SourcesModel::Control::verbunden;
    q.messung = eqcop::SourcesModel::Messung::fresh;
    q.betrieb = eqcop::SourcesModel::Betrieb::active;
    q.lautheit = eqcop::SourcesModel::Lautheit::gueltig;
    q.messpunkt = eqcop::SourcesModel::Messpunkt::insert;
    q.descriptorVorhanden = true;
    q.hauptziel = true;
    s.quellen.push_back (q);
    return s;
}

/** Faehrt `anzahl` Bloecke und bewegt den Transport dabei wie ein Host. */
void fahre (EqCopilotProcessor& p, TestPlayHead& kopf,
            juce::AudioBuffer<float>& puffer, int anzahl, float wert = 0.5f)
{
    juce::MidiBuffer midi;
    for (int i = 0; i < anzahl; ++i)
    {
        for (int c = 0; c < puffer.getNumChannels(); ++c)
            for (int s = 0; s < puffer.getNumSamples(); ++s)
                puffer.setSample (c, s, wert);
        p.processBlock (puffer, midi);
        kopf.pos += puffer.getNumSamples();
    }
}

/** Wartet, bis eine Bedingung gilt — und faehrt dabei Audio, damit der
    Analyseworker ueberhaupt etwas zu tun bekommt. */
template <typename Pruefung>
bool warte (EqCopilotProcessor& p, TestPlayHead& kopf,
            juce::AudioBuffer<float>& puffer, Pruefung pruefung)
{
    for (int i = 0; i < 400; ++i)
    {
        fahre (p, kopf, puffer, 1);
        if (pruefung())
            return true;
        std::this_thread::sleep_for (std::chrono::milliseconds (2));
    }
    return false;
}

/** Faehrt so lange, bis der Vergleichspegel genug Bloecke AUFGENOMMEN hat.

    Eine feste Blockzahl waere eine Zeitannahme: der Analyseworker setzt das
    Fenster asynchron, und bis dahin nimmt der Audiothread nichts auf. Gemessen
    wird deshalb die Zahl, um die es geht — 400 ms bei 512 Samples sind
    37,5 Bloecke, und 60 lassen Luft, ohne eine Uhr zu befragen. */
bool fahreBisPegel (EqCopilotProcessor& p, TestPlayHead& kopf,
                    juce::AudioBuffer<float>& puffer, juce::uint64 mindestens = 60)
{
    for (int i = 0; i < 1200; ++i)
    {
        fahre (p, kopf, puffer, 1);
        if (p.versuchAufgenommeneBloecke() >= mindestens)
            return true;
    }
    return false;
}

juce::Array<juce::var> vier (juce::var a, juce::var b, juce::var c, juce::var d)
{
    juce::Array<juce::var> f;
    f.add (a); f.add (b); f.add (c); f.add (d);
    return f;
}

// ═════════════════════════════════════════════════════════════════════════
// M-25 · manual_passage_persists_in_main_project_state_and_marks_host_dirty
// ═════════════════════════════════════════════════════════════════════════
void m25()
{
    abschnitt ("M-25  manual_passage_persists_in_main_project_state_and_marks_host_dirty");

    auto p = mainProzessor();
    pruefe (! p->stateNurLesen() && p->manuellePassagen().empty(),
            "ein frisches Main-Projekt haelt keine Passage - kein erfundener Vorgabewert");

    DirtyZaehler dirty;
    p->addListener (&dirty);
    pruefe (p->merkeManuellePassage (hex32 (1), "Refrain", 480000, 960000),
            "eine markierte Passage wird gemerkt");
    pruefe (dirty.nonParam == 1, "und meldet GENAU einmal Host-Dirty",
            juce::String (dirty.nonParam));

    const auto gehalten = p->manuellePassagen();
    pruefe (gehalten.size() == 1 && gehalten[0].passageId == hex32 (1)
                && gehalten[0].label == "Refrain"
                && gehalten[0].projektStart == 480000 && gehalten[0].projektEnde == 960000,
            "alle vier Intentfelder stehen unveraendert im State");

    // Der Gegenweg gehoert in denselben Aenderungssatz (CLAUDE.md).
    pruefe (! p->vergissManuellePassage (hex32 (99)),
            "eine unbekannte passage_id zu vergessen aendert nichts");
    pruefe (dirty.nonParam == 1, "und meldet folglich auch kein Dirty",
            juce::String (dirty.nonParam));
    pruefe (p->vergissManuellePassage (hex32 (1)), "die gemerkte Passage laesst sich vergessen");
    pruefe (dirty.nonParam == 2, "das Vergessen meldet Dirty wie das Merken",
            juce::String (dirty.nonParam));
    pruefe (p->manuellePassagen().empty(), "und die Liste ist wieder leer");
    p->removeListener (&dirty);

    // Eine Sonde ist kein Projekthalter.
    auto sonde = std::make_unique<EqCopilotProcessor>();
    auto v = mainBaum();
    v.getChildWithName ("Common").setProperty ("plugin_kind", "legacy", nullptr);
    v.removeChild (v.getChildWithName ("MainProject"), nullptr);
    const auto sondenBytes = alsBlock (v);
    sonde->setStateInformation (sondenBytes.getData(), (int) sondenBytes.getSize());
    pruefe (! sonde->merkeManuellePassage (hex32 (1), "Refrain", 0, 100),
            "eine Nicht-main-Instanz haelt keine Passagen - §33.5 gibt sie dem MainProject");
}

// ═════════════════════════════════════════════════════════════════════════
// M-69 · save_load_roundtrip_is_byte_identical
// ═════════════════════════════════════════════════════════════════════════
void m69Roundtrip()
{
    abschnitt ("M-69  save_load_roundtrip_is_byte_identical");

    auto p = mainProzessor();
    // Absichtlich in UNSORTIERTER Reihenfolge und mit einem Label, das die
    // Grenze ausreizt: der Writer sortiert deterministisch, sonst haengt die
    // Bytegleichheit an der Eingabereihenfolge.
    pruefe (p->merkeManuellePassage (hex32 (3), "Bridge", 960000, 1440000), "Passage 3 gemerkt");
    pruefe (p->merkeManuellePassage (hex32 (1), "Intro", 0, 480000), "Passage 1 gemerkt");
    pruefe (p->merkeManuellePassage (hex32 (2), juce::String::repeatedString ("x", 120),
                                     480000, 960000),
            "Passage 2 mit 120-Zeichen-Label gemerkt");

    juce::MemoryBlock erst;
    p->getStateInformation (erst);

    auto q = std::make_unique<EqCopilotProcessor>();
    q->setStateInformation (erst.getData(), (int) erst.getSize());
    pruefe (! q->stateNurLesen(), "der geschriebene Stand ist fuer den eigenen Leser gueltig");

    const auto geladen = q->manuellePassagen();
    pruefe (geladen.size() == 3, "drei Passagen kommen zurueck",
            juce::String ((int) geladen.size()));
    pruefe (geladen.size() == 3 && geladen[0].passageId == hex32 (1)
                && geladen[1].passageId == hex32 (2) && geladen[2].passageId == hex32 (3),
            "und zwar nach Projektposition sortiert - die Reihenfolge ist deterministisch");
    pruefe (geladen.size() == 3 && geladen[1].label.length() == 120,
            "das 120-Zeichen-Label ueberlebt vollstaendig");

    juce::MemoryBlock zweit;
    q->getStateInformation (zweit);
    pruefe (gleich (erst, zweit), "Save→Load→Save ist bytegleich",
            juce::String ((int) erst.getSize()) + " vs " + juce::String ((int) zweit.getSize()));

    // Altstand: ein Projekt, das die Fassung noch nicht kennt, bleibt gueltig.
    auto alt = std::make_unique<EqCopilotProcessor>();
    const auto ohneFeld = alsBlock (mainBaum());
    alt->setStateInformation (ohneFeld.getData(), (int) ohneFeld.getSize());
    pruefe (! alt->stateNurLesen() && alt->manuellePassagen().empty(),
            "ein Altstand OHNE das Feld laedt normal und haelt keine Passagen");

    // Und die leere Liste schreibt die Eigenschaft nicht als leeres Array -
    // sonst unterschieden sich ein Projekt ohne Passagen und eines, dessen
    // letzte geloescht wurde, in den Bytes.
    juce::MemoryBlock altGespeichert;
    alt->getStateInformation (altGespeichert);
    auto zurueck = std::make_unique<EqCopilotProcessor>();
    zurueck->setStateInformation (altGespeichert.getData(), (int) altGespeichert.getSize());
    juce::MemoryBlock nochmal;
    zurueck->getStateInformation (nochmal);
    pruefe (gleich (altGespeichert, nochmal),
            "auch der Altstand ist ueber zwei Runden bytegleich");

    auto leerBaum = juce::ValueTree::readFromData (altGespeichert.getData(),
                                                   altGespeichert.getSize());
    pruefe (! leerBaum.getChildWithName ("MainProject").hasProperty ("manual_passages_v1"),
            "die leere Liste laesst die Eigenschaft weg statt sie als leeres Array zu schreiben");
}

// ═════════════════════════════════════════════════════════════════════════
// M-69 · unknown_newer_field_stays_read_only_with_original_bytes
// ═════════════════════════════════════════════════════════════════════════
void m69Unbekannt()
{
    abschnitt ("M-69  unknown_newer_field_stays_read_only_with_original_bytes");

    const auto eine = vier (hex32 (1), "Intro", i64 (0), i64 (480000));

    // (a) Ein ZUSAETZLICHES Feld aus einer spaeteren Fassung zerstoert den
    //     alten Consumer nicht und reist beim Speichern unveraendert zurueck.
    auto v = mainBaum();
    auto mp = v.getChildWithName ("MainProject");
    mp.setProperty ("manual_passages_v1", juce::var (eine), nullptr);
    mp.setProperty ("manual_passage_colours_v2", "aus der Zukunft", nullptr);
    const auto mitZukunft = alsBlock (v);

    auto p = std::make_unique<EqCopilotProcessor>();
    p->setStateInformation (mitZukunft.getData(), (int) mitZukunft.getSize());
    pruefe (! p->stateNurLesen(),
            "ein unbekanntes ZUSATZfeld macht den Stand nicht ungueltig - additive Erweiterung");
    pruefe (p->manuellePassagen().size() == 1,
            "die bekannte Passage wird trotzdem gelesen");

    juce::MemoryBlock heraus;
    p->getStateInformation (heraus);
    auto zurueck = juce::ValueTree::readFromData (heraus.getData(), heraus.getSize());
    pruefe (zurueck.getChildWithName ("MainProject")
                .getProperty ("manual_passage_colours_v2").toString() == "aus der Zukunft",
            "und das unbekannte Feld steht unveraendert im naechsten Save");

    // (b) Ein unbekanntes MAJOR bleibt read-only mit Originalbytes - dort
    //     reisen die Passagen gar nicht erst in die Struktur.
    auto zukunft = mainBaum();
    zukunft.setProperty ("schema", 3, nullptr);
    zukunft.getChildWithName ("MainProject")
        .setProperty ("manual_passages_v1", juce::var (eine), nullptr);
    const auto fremd = alsBlock (zukunft);

    auto q = std::make_unique<EqCopilotProcessor>();
    DirtyZaehler dirty;
    q->addListener (&dirty);
    q->setStateInformation (fremd.getData(), (int) fremd.getSize());
    pruefe (q->stateNurLesen(), "unbekanntes Major: read-only");
    pruefe (q->manuellePassagen().empty(),
            "und KEINE Passage wird interpretiert - ein fremder Stand wird nicht halb gelesen");
    pruefe (! q->merkeManuellePassage (hex32 (7), "Neu", 0, 100),
            "read-only verweigert auch das Merken");
    pruefe (! q->vergissManuellePassage (hex32 (1)),
            "und ebenso das Vergessen");
    juce::MemoryBlock zurueckBytes;
    q->getStateInformation (zurueckBytes);
    pruefe (gleich (zurueckBytes, fremd), "die Originalbytes gehen unveraendert zurueck");
    pruefe (dirty.nonParam == 0, "und nichts davon meldet Host-Dirty",
            juce::String (dirty.nonParam));
    q->removeListener (&dirty);
}

// ═════════════════════════════════════════════════════════════════════════
// Die Raender - jeder von BEIDEN Seiten
// ═════════════════════════════════════════════════════════════════════════
void raenderApi()
{
    abschnitt ("Die Raender, Seite 1: was die API ablehnt");

    auto p = mainProzessor();
    pruefe (! p->merkeManuellePassage ("nicht-hex", "x", 0, 100),
            "API: keine passage_id ausserhalb hex32");
    pruefe (! p->merkeManuellePassage (hex32 (1), juce::String::repeatedString ("y", 121), 0, 100),
            "API: kein Label ueber 120 Codepoints");
    pruefe (! p->merkeManuellePassage (hex32 (1), "x", -1, 100),
            "API: kein negativer Start - Projektsamples zaehlen ab Projektbeginn");
    pruefe (! p->merkeManuellePassage (hex32 (1), "x", 100, 100),
            "API: kein leeres Fenster - `[start, ende)` ist halboffen");
    pruefe (! p->merkeManuellePassage (hex32 (1), "x", 200, 100),
            "API: kein rueckwaerts laufendes Fenster");
    pruefe (p->merkeManuellePassage (hex32 (1), "x", 0, 100), "API: der Gutfall geht");
    pruefe (! p->merkeManuellePassage (hex32 (1), "andere", 500, 600),
            "API: keine zweite Passage mit derselben id");

    // Die Obergrenze aus §48.1.
    auto voll = mainProzessor();
    int gemerkt = 0;
    for (int i = 1; i <= state::maxManuellePassagen + 5; ++i)
        if (voll->merkeManuellePassage (hex32 (i), "x", i * 1000, i * 1000 + 500))
            ++gemerkt;
    pruefe (gemerkt == state::maxManuellePassagen,
            "API: die feste Obergrenze aus §48.1 haelt", juce::String (gemerkt));

    // Und was voll ist, laesst sich wieder leeren.
    pruefe (voll->vergissManuellePassage (hex32 (1))
                && voll->merkeManuellePassage (hex32 (999), "danach", 9000000, 9000500),
            "nach einem Vergessen ist wieder Platz fuer genau eine");
}

void raenderLeser()
{
    abschnitt ("Die Raender, Seite 2: was der Leser ablehnt");

    juce::Array<juce::var> doppelt = vier (hex32 (1), "a", i64 (0), i64 (100));
    doppelt.addArray (vier (hex32 (1), "b", i64 (200), i64 (300)));
    juce::Array<juce::var> dreier;
    dreier.add (hex32 (1)); dreier.add ("a"); dreier.add (i64 (0));

    struct Fall { const char* was; juce::Array<juce::var> flach; };
    const std::vector<Fall> faelle {
        { "keine passage_id ausserhalb hex32",
          vier ("nicht-hex", "a", i64 (0), i64 (100)) },
        { "kein Label ueber 120 Codepoints",
          vier (hex32 (1), juce::String::repeatedString ("y", 121), i64 (0), i64 (100)) },
        { "kein negativer Start", vier (hex32 (1), "a", i64 (-1), i64 (100)) },
        { "kein leeres Fenster", vier (hex32 (1), "a", i64 (100), i64 (100)) },
        { "kein rueckwaerts laufendes Fenster", vier (hex32 (1), "a", i64 (200), i64 (100)) },
        { "keine Bruchzahl als Projektsample - 44100.5 gibt es nicht",
          vier (hex32 (1), "a", juce::var (0.0), juce::var (100.5)) },
        { "keine Zahl als Text - stille Umwandlung waere eine erfundene Grenze",
          vier (hex32 (1), "a", juce::var ("0"), juce::var ("100")) },
        { "keine doppelte passage_id", doppelt },
        { "keine unvollstaendige Vierergruppe", dreier },
    };

    for (const auto& f : faelle)
    {
        const auto bytes = baumMitPassagen (f.flach);
        auto q = std::make_unique<EqCopilotProcessor>();
        q->setStateInformation (bytes.getData(), (int) bytes.getSize());
        juce::MemoryBlock heraus;
        q->getStateInformation (heraus);
        pruefe (q->stateNurLesen() && q->manuellePassagen().empty() && gleich (heraus, bytes),
                juce::String ("Leser: ") + f.was + " → read-only, Originalbytes zurueck");
    }

    // Die volle Liste ist erlaubt, eine darueber nicht.
    juce::Array<juce::var> genau;
    for (int i = 1; i <= state::maxManuellePassagen; ++i)
        genau.addArray (vier (hex32 (i), "a", i64 (i * 1000), i64 (i * 1000 + 500)));
    juce::Array<juce::var> einsZuviel = genau;
    einsZuviel.addArray (vier (hex32 (999), "a", i64 (9000000), i64 (9000500)));

    {
        const auto bytes = baumMitPassagen (genau);
        auto q = std::make_unique<EqCopilotProcessor>();
        q->setStateInformation (bytes.getData(), (int) bytes.getSize());
        pruefe (! q->stateNurLesen()
                    && (int) q->manuellePassagen().size() == state::maxManuellePassagen,
                "Leser: genau die Obergrenze ist gueltig",
                juce::String ((int) q->manuellePassagen().size()));
    }
    {
        const auto bytes = baumMitPassagen (einsZuviel);
        auto q = std::make_unique<EqCopilotProcessor>();
        q->setStateInformation (bytes.getData(), (int) bytes.getSize());
        pruefe (q->stateNurLesen(), "Leser: eine ueber der Obergrenze ist read-only");
    }
    {
        // Kein Array, sondern ein blanker Wert.
        auto v = mainBaum();
        v.getChildWithName ("MainProject")
            .setProperty ("manual_passages_v1", "kein Array", nullptr);
        const auto bytes = alsBlock (v);
        auto q = std::make_unique<EqCopilotProcessor>();
        q->setStateInformation (bytes.getData(), (int) bytes.getSize());
        pruefe (q->stateNurLesen(), "Leser: ein blanker Wert statt eines Arrays ist read-only");
    }
}

// ═════════════════════════════════════════════════════════════════════════
// Zahlenraender: int64 an seinen Enden
// ═════════════════════════════════════════════════════════════════════════
void zahlenraender()
{
    abschnitt ("Zahlenraender: `int64` an seinen Enden");

    auto p = mainProzessor();
    const auto gross = std::numeric_limits<juce::int64>::max();
    pruefe (p->merkeManuellePassage (hex32 (1), "am Rand", gross - 1, gross),
            "eine Passage am oberen int64-Rand wird gemerkt");
    juce::MemoryBlock b;
    p->getStateInformation (b);
    auto q = std::make_unique<EqCopilotProcessor>();
    q->setStateInformation (b.getData(), (int) b.getSize());
    const auto zurueck = q->manuellePassagen();
    pruefe (! q->stateNurLesen() && zurueck.size() == 1
                && zurueck[0].projektStart == gross - 1 && zurueck[0].projektEnde == gross,
            "und kommt exakt zurueck - kein Umweg ueber `double` unterwegs");

    juce::MemoryBlock zweit;
    q->getStateInformation (zweit);
    pruefe (gleich (b, zweit), "auch am Rand ist der Roundtrip bytegleich");
}

// ═════════════════════════════════════════════════════════════════════════
// R03 · die gespeicherte Passage erreicht die ENGINE
// ═════════════════════════════════════════════════════════════════════════
//
// 🔑 Nacharbeit 2 (Befund R03, M-03/M-25): `setzePassagenfenster` hatte
// ausserhalb der Tests keinen Aufrufer. `merkeManuellePassage` schrieb nur
// Plugin-State; weder Main noch Sonde uebertrugen die Grenzen an eine Engine,
// und die Passagenmetriken liefen weiter seit der letzten Transportgrenze. Der
// alte Wire-Test rief die Methode DIREKT auf und konnte an der fehlenden
// Produktkopplung nicht fallen.
//
// Dieser Fall faehrt den ganzen Weg: Produkt-API → Wunsch → Analyseworker →
// Engine. Er treibt echte Bloecke durch `processBlock`, damit der Worker
// wirklich laeuft.
void r03Passagenfenster()
{
    abschnitt ("R03  passagenfenster_erreicht_die_engine");
    auto p = mainProzessor();
    p->prepareToPlay (48000.0, 512);

    juce::AudioBuffer<float> puffer (2, 512);
    juce::MidiBuffer midi;
    auto blockFahren = [&]
    {
        puffer.clear();
        p->processBlock (puffer, midi);
    };
    auto wartenAufFenster = [&] (std::int64_t& start, std::int64_t& ende, bool erwartet)
    {
        for (int i = 0; i < 400; ++i)
        {
            blockFahren();
            if (p->passagenfensterInEngine (start, ende) == erwartet)
                return true;
            std::this_thread::sleep_for (std::chrono::milliseconds (5));
        }
        return false;
    };

    std::int64_t start = 0, ende = 0;
    pruefe (! p->passagenfensterInEngine (start, ende),
            "R03: ohne markierte Passage fuehrt die Engine kein Fenster");

    pruefe (p->merkeManuellePassage (hex32 (7), "Refrain", 480000, 960000),
            "R03: die Passage wird gemerkt");
    pruefe (wartenAufFenster (start, ende, true),
            "R03: passagenfenster_erreicht_die_engine - der Analyseworker setzt "
            "das Fenster, ohne dass ein Test die Engine anfasst");
    pruefe (start == 480000 && ende == 960000,
            "R03: und zwar mit GENAU den gespeicherten Grenzen",
            juce::String (start) + ".." + juce::String (ende));

    // Der Gegenpfad gehoert in denselben Aenderungssatz (CLAUDE.md).
    pruefe (p->vergissManuellePassage (hex32 (7)), "R03: die Passage wird vergessen");
    pruefe (wartenAufFenster (start, ende, false),
            "R03: und die Engine loest die Bindung wieder - danach gilt wieder "
            "die Transportepoche als Fenster");

    p->releaseResources();
}

// ═════════════════════════════════════════════════════════════════════════
// R01 · das Ueberlauf-Sticky hat einen Loeschpfad
// ═════════════════════════════════════════════════════════════════════════
//
// 🔑 Nacharbeit 2 (Befund R01, M-61): `v3Status()` las das Sticky, aber im
// Produkt gab es weder ein `store(false)` noch einen Aufruf von
// `InterventionsRing::resync()`. Nach dem ersten Ueberlauf meldete jeder
// Heartbeat dauerhaft `intervention_state_unknown`, und die Sperre auf starker
// Evidenz fiel nie wieder. Die beiden alten Beine prueften die isolierten
// Haelften und konnten an der fehlenden Kopplung nicht fallen.
void r01Resync()
{
    abschnitt ("R01  ueberlauf_sticky_wird_beim_bestaetigten_resync_geloescht");
    auto p = mainProzessorMitBindung();
    p->prepareToPlay (48000.0, 512);

    pruefe (! p->v3StatusFuerTest().interventionStateUnknown,
            "R01: frisch ist der Interventionszustand BEKANNT");

    // ── Der ECHTE Handschlag gegen einen Probe-Server ────────────────────
    //
    // 🔑 NAK-180 Nacharbeit 2 (WN-08/EP-18/R3b): bis hierher rief dieses Bein
    // `v3LinkFuerTest(true)` und nannte das Ergebnis Resync. Kein
    // ControlClient-Thread lief, keine Pipe wurde geoeffnet, kein Heartbeat
    // gebildet, nichts zugestellt - Fehler im echten Lebenszyklus blieben
    // gruen. Jetzt laeuft der Produktclient: `start()`, Pipe-Handschlag,
    // Hello/Welcome, Heartbeat-Takt, P0-Enqueue, Wire-Commit. Gemessen wird,
    // was der SERVER empfangen hat.
    const auto pipe = testPipeName ("sonde013.r01");
    pruefe (nakama::ipc::istProbePipename (pipe),
            "R01: der Testserver liegt im PROBE-Namensraum - nie auf der "
            "Produktionspipe (Paragraph 48.3)",
            juce::String (pipe));
    pruefe (p->v3ProbeGegenstelleFuerTest (pipe, testExeErwartung()),
            "R01: der echte ControlClient zeigt auf den Probe-Server");

    TestServer server (pipe);
    pruefe (server.starten(), "R01: der Probe-Server steht");

    // 🔑 Nacharbeit 3 (WA-04/N-04): der Sender wird VOR dem Ringfuellen
    // angehalten - und BESTAETIGT, nicht angenommen.
    //
    // Seit `mainProzessorMitBindung()` reiht der Analyseworker die Ringinhalte
    // wirklich als P0 ein. Erhielte er nach dem Fuellen einen Zug, entnaehme er
    // die Testereignisse vor oder waehrend des echten Handschlags
    // (`PluginProcessor.cpp:1320-1321`): die erste empfangene P0-Nachricht
    // waere dann ein `end` statt des erwarteten Heartbeats, und die Messung
    // unten fiele bei KORREKTEM Produktcode. Auch das Ring-Leeren konkurriert
    // sonst mit ihm. Die Pause traegt deshalb bis zum Ende des Messfensters.
    p->senderAnhaltenFuerTest (true);
    pruefe (p->warteAufSenderPauseFuerTest(),
            "R01/WA-04: der Sender ist BESTAETIGT angehalten, bevor der Ring "
            "gefuellt wird - der Handschlag misst danach keinen Wettlauf mit ihm");

    // Den Ring bis zum Ueberlauf fuellen — derselbe Weg wie im Audiothread.
    const int geschrieben = p->interventionsRingFuellenFuerTest();
    pruefe (geschrieben > 0, "R01: der Ring nimmt Ereignisse auf",
            juce::String (geschrieben));
    pruefe (p->v3StatusFuerTest().interventionStateUnknown,
            "R01: der Ueberlauf meldet sich als `intervention_state_unknown`");

    // Jetzt den Client wirklich starten. Der Aufbau urteilt bei GEFUELLTEM
    // Ring NICHT neutral - eine Selbstheilung waere genau das, was
    // Paragraph 34.2 verbietet.
    p->v3StartFuerTest();

    auto warteAuf = [] (int fristMs, auto&& bedingung)
    {
        const auto bis = std::chrono::steady_clock::now()
                       + std::chrono::milliseconds (fristMs);
        while (std::chrono::steady_clock::now() < bis)
        {
            if (bedingung())
                return true;
            std::this_thread::sleep_for (std::chrono::milliseconds (5));
        }
        return bedingung();
    };

    pruefe (warteAuf (8000, [&]
            {
                return p->controlV3Snapshot().status
                       == nakama::ipc::ControlClient::Status::verbunden;
            }),
            "R01/WN-08: der ECHTE Client ist ueber die Probe-Pipe verbunden - "
            "Hello und Welcome sind gelaufen, nicht ein Testhaken");

    // Der Heartbeat-Takt ist 1 Hz; der erste geht sofort nach dem Welcome.
    auto heartbeats = [&] ()
    {
        std::lock_guard<std::mutex> l (server.textMutex);
        return server.p0Texte;
    };
    auto sucheNeutral = [&] (bool wert)
    {
        const std::string gesucht = wert
            ? std::string (R"("intervention_state_unknown":true)")
            : std::string (R"("intervention_state_unknown":false)");
        for (const auto& t : heartbeats())
            if (t.find (gesucht) != std::string::npos)
                return true;
        return false;
    };

    // 🔑 Erst messen, DANN berichten. Die Argumente von `pruefe` werden in
    // unbestimmter Reihenfolge ausgewertet; ein `heartbeats().size()` im
    // Detailtext kann sonst VOR der Wartebedingung gelesen werden und einen
    // Stand melden, den die Zusage nie gesehen hat.
    const bool p0Kam = warteAuf (8000, [&] { return ! heartbeats().empty(); });
    const int p0NachStart = (int) heartbeats().size();
    pruefe (p0Kam,
            "R01/WN-08: der Server hat wirklich P0 empfangen - der Takt laeuft",
            juce::String (p0NachStart) + " P0");
    const bool trugTrue = sucheNeutral (true);
    const bool trugFalse = sucheNeutral (false);
    pruefe (trugTrue && ! trugFalse,
            "R01/WN-08: bei GEFUELLTEM Ring traegt der erste Heartbeat "
            "ausdrueckliches `true` - und KEIN `false`, das den Resync ausloeste",
            juce::String (p0NachStart) + " P0, `true` "
                + (trugTrue ? "steht" : "FEHLT") + ", `false` "
                + (trugFalse ? "STEHT" : "fehlt"));
    pruefe (p->v3StatusFuerTest().interventionStateUnknown,
            "R01: bei GEFUELLTEM Ring bleibt das Sticky stehen - eine "
            "Selbstheilung waere genau das, was Paragraph 34.2 verbietet");

    // Jetzt leeren und den Link NEU aufbauen - ueber den echten Reconnect,
    // nicht ueber einen Haken. Der bestaetigte Neuaufbau IST der Resync.
    //
    // 🔑 Nacharbeit 3 (WA-04): das Leeren laeuft unter DERSELBEN Pause.
    // Ein hier wieder laufender Sender entnaehme dieselben Ereignisse und
    // schickte sie los, waehrend das Messfenster noch offen ist.
    p->interventionsRingLeerenFuerTest();
    const auto vorReconnect = heartbeats().size();
    p->v3ReconnectFuerTest();

    const bool falseKam = warteAuf (10000, [&] { return sucheNeutral (false); });
    const int p0NachReconnect = (int) heartbeats().size();
    pruefe (falseKam && p0NachReconnect > (int) vorReconnect,
            "R01/WN-08: ueberlauf_sticky_wird_beim_bestaetigten_resync_geloescht - "
            "der erste Heartbeat des NEUEN Links traegt auf dem DRAHT das "
            "ausdrueckliche `false`, und genau das loest beim Broker "
            "`resync_bestaetigen` aus",
            juce::String ((int) vorReconnect) + " -> "
                + juce::String (p0NachReconnect) + " P0, `false` "
                + (falseKam ? "steht" : "FEHLT"));
    pruefe (! p->v3StatusFuerTest().interventionStateUnknown,
            "R01: und der lokale Zustand ist wieder bekannt");

    // Das Messfenster ist geschlossen - erst JETZT darf der Sender wieder.
    p->senderAnhaltenFuerTest (false);

    p->v3StopFuerTest();
    server.stoppen();
    p->releaseResources();
}

// ═════════════════════════════════════════════════════════════════════════
// R06 · der Vergleichspegel ist im Produkt verdrahtet
// ═════════════════════════════════════════════════════════════════════════
//
// 🔑 Nacharbeit 2 (Befund R06, M-07/M-20/M-43): `Vergleichspegel` und
// `Blindvergleich` waren uebersetzt und im Produkt UNBENUTZT;
// `nichtEndlicheSamples()` hatte ausserhalb der C++-Tests keinen Leser. Ein
// nichtendliches Sample im Vergleichsmaterial verriegelte lokal den Gain und
// blieb im Produkt ungezaehlt.
void r06Vergleichspegel()
{
    abschnitt ("R06  vergleichspegel_und_blindvergleich_im_produktpfad");
    auto p = mainProzessor();
    p->prepareToPlay (kFs, kBlock);
    TestPlayHead kopf;
    p->setPlayHead (&kopf);
    juce::AudioBuffer<float> puffer (2, kBlock);

    pruefe (! p->versuchLautheitAbgeglichen(),
            "R06: ohne markierte Passage misst der Vergleichspegel nicht");
    pruefe (p->versuchNichtEndlicheSamples() == 0,
            "R06: und zaehlt nichts");

    // Die markierte Passage schaltet die Vorabmessung ein (Paragraph 38.3).
    //
    // 🔑 Nacharbeit 3 (Befund C4): dieses Bein faehrt seither einen ECHTEN
    // Transport. Der Pegel nimmt nur Material aus dem Fenster der gebundenen
    // Passage, und ohne gueltige Projektzeit gaebe es davon keines.
    fahre (*p, kopf, puffer, 20);
    pruefe (p->merkeManuellePassage (hex32 (8), "Refrain", 0, 4800000),
            "R06: die Passage wird gemerkt");
    pruefe (warte (*p, kopf, puffer, [&] { return p->passagenfensterFuehrt (hex32 (8)); }),
            "R06: und die Engine fuehrt ihr Fenster");
    pruefe (fahreBisPegel (*p, kopf, puffer), "R06: der Pegel hat genug Material");
    (void) p->beginneVersuch (hex32 (8));
    pruefe (p->versuchLautheitAbgeglichen(),
            "R06: vergleichspegel_ist_im_produktpfad_eingefroren - der Pegel ist "
            "gesetzt, nicht nur messbar",
            juce::String (p->versuchMatchGainDb(), 3));
    pruefe (p->versuchNichtEndlicheSamples() == 0,
            "R06: und sauberes Material zaehlt nachweislich KEIN nichtendliches "
            "Sample - die Gegenprobe zum Fall darunter");

    // Und derselbe Weg mit beschaedigtem Material: der Zaehler REIST.
    auto q = mainProzessor();
    q->prepareToPlay (kFs, kBlock);
    TestPlayHead kopf2;
    q->setPlayHead (&kopf2);
    juce::AudioBuffer<float> puffer2 (2, kBlock);
    fahre (*q, kopf2, puffer2, 20);
    pruefe (q->merkeManuellePassage (hex32 (9), "Refrain", 0, 4800000),
            "R06: zweite Passage gemerkt");
    pruefe (warte (*q, kopf2, puffer2, [&] { return q->passagenfensterFuehrt (hex32 (9)); }),
            "R06: und auch hier fuehrt die Engine das Fenster");
    juce::MidiBuffer midi2;
    for (int i = 0; i < 200; ++i)
    {
        for (int c = 0; c < 2; ++c)
            for (int sp = 0; sp < puffer2.getNumSamples(); ++sp)
                puffer2.setSample (c, sp, sp == 0 ? std::numeric_limits<float>::quiet_NaN()
                                                  : 0.5f);
        q->processBlock (puffer2, midi2);
        kopf2.pos += puffer2.getNumSamples();
    }
    pruefe (! q->beginneVersuch (hex32 (9)),
            "R06: mit nichtendlichem Material entsteht KEIN Versuch - eine "
            "Klangwertung ohne Lautheitsabgleich ist unzulaessig (Paragraph 15)");
    pruefe (q->versuchNichtEndlicheSamples() > 0,
            "R06: der Zaehler hat einen PRODUKTLESER, statt nur lokal zu "
            "verriegeln",
            juce::String ((juce::int64) q->versuchNichtEndlicheSamples()));
    pruefe (! q->versuchLautheitAbgeglichen(),
            "R06: und der Pegel bleibt ungesetzt");

    p->setPlayHead (nullptr);
    q->setPlayHead (nullptr);
    p->releaseResources();
    q->releaseResources();
}

// ─────────────────────────────────────────────────────────────────────────
// C2 · die beim MARKIEREN erfasste Epoche entscheidet
// ─────────────────────────────────────────────────────────────────────────
void c2Markierungsepoche()
{
    abschnitt ("C2  seek_zwischen_markierung_und_worker_verwirft_das_fenster");
    auto p = mainProzessorMitBindung();
    p->prepareToPlay (kFs, kBlock);
    TestPlayHead kopf;
    p->setPlayHead (&kopf);
    juce::AudioBuffer<float> puffer (2, kBlock);

    fahre (*p, kopf, puffer, 40);
    const auto e0 = p->merkmaleTransportEpoche();

    // Ein Seek IST eine Transportgrenze (Paragraph 32.3). Danach zeigen
    // dieselben Samplegrenzen auf anderes Material.
    kopf.pos = 4000000;
    const bool gewechselt = warte (*p, kopf, puffer,
        [&] { return p->merkmaleTransportEpoche() > e0; });
    pruefe (gewechselt, "C2: der Seek erzeugt eine neue Transportepoche",
            juce::String ((juce::int64) p->merkmaleTransportEpoche()));
    const auto e1 = p->merkmaleTransportEpoche();

    // Der Fall des Befunds: die Markierung erfasste `e0`, der Worker laeuft
    // erst nach dem Seek. Bis zur Runde 2 las er die Epoche aus der Engine —
    // also `e1` gegen `e1` — und der Riegel konnte nie greifen.
    pruefe (p->passagenfensterWunschFuerTest (hex32 (0x21), 4000000, 4960000, e0),
            "C2: der Wunsch mit der ALTEN Epoche wird abgelegt");
    bool spaeterGesetzt = false;
    for (int i = 0; i < 120; ++i)
    {
        fahre (*p, kopf, puffer, 1);
        std::this_thread::sleep_for (std::chrono::milliseconds (2));
        if (p->passagenfensterFuehrt (hex32 (0x21)))
            spaeterGesetzt = true;
    }
    pruefe (! spaeterGesetzt,
            "C2: seek_zwischen_markierung_und_worker_verwirft_das_fenster - die "
            "Engine nimmt Grenzen einer FREMDEN Epoche nicht an");

    // Die Gegenprobe: derselbe Weg mit der Epoche, die jetzt gilt.
    pruefe (p->passagenfensterWunschFuerTest (hex32 (0x22), 4000000, 4960000, e1),
            "C2: derselbe Wunsch mit der aktuellen Epoche wird abgelegt");
    pruefe (warte (*p, kopf, puffer, [&] { return p->passagenfensterFuehrt (hex32 (0x22)); }),
            "C2: und DIESES Fenster fuehrt die Engine - der Riegel sperrt die "
            "richtige Haelfte, nicht alles");

    p->setPlayHead (nullptr);
    p->releaseResources();
}

// ─────────────────────────────────────────────────────────────────────────
// C3 · das Fenster gehoert GENAU EINER Passage
// ─────────────────────────────────────────────────────────────────────────
void c3ZweiPassagen()
{
    abschnitt ("C3  vergessen_und_beginnen_treffen_nur_die_eigene_passage");
    auto p = mainProzessorMitBindung();
    p->prepareToPlay (kFs, kBlock);
    TestPlayHead kopf;
    p->setPlayHead (&kopf);
    juce::AudioBuffer<float> puffer (2, kBlock);
    fahre (*p, kopf, puffer, 20);

    const auto a = hex32 (0x31), b = hex32 (0x32);
    pruefe (p->merkeManuellePassage (a, "A", 0, 960000), "C3: Passage A gemerkt");
    pruefe (warte (*p, kopf, puffer, [&] { return p->passagenfensterFuehrt (a); }),
            "C3: und die Engine fuehrt A");

    pruefe (p->merkeManuellePassage (b, "B", 960000, 1920000), "C3: Passage B gemerkt");
    pruefe (warte (*p, kopf, puffer, [&] { return p->passagenfensterFuehrt (b); }),
            "C3: die zweite Markierung haengt das Fenster auf B um");
    pruefe (! p->passagenfensterFuehrt (a), "C3: und A fuehrt es nicht mehr");

    // DER Befund: `vergiss(A)` loeschte bis zur Runde 2 bedingungslos das
    // globale Fenster und stoppte damit die Messung von B.
    pruefe (p->vergissManuellePassage (a), "C3: A wird vergessen");
    fahre (*p, kopf, puffer, 20);
    pruefe (p->passagenfensterFuehrt (b),
            "C3: vergessen_trifft_nur_die_eigene_passage - B laeuft weiter");

    // Und die zweite Haelfte: `beginne(A)` haengt das Fenster auf A um,
    // statt den zuletzt gesetzten globalen Stand einzufrieren.
    pruefe (p->merkeManuellePassage (a, "A", 0, 960000), "C3: A wird neu gemerkt");
    pruefe (p->merkeManuellePassage (hex32 (0x33), "C", 1920000, 2880000),
            "C3: und danach eine dritte Passage");
    pruefe (warte (*p, kopf, puffer, [&] { return p->passagenfensterFuehrt (hex32 (0x33)); }),
            "C3: C fuehrt das Fenster");
    (void) p->beginneVersuch (a);
    pruefe (warte (*p, kopf, puffer, [&] { return p->passagenfensterFuehrt (a); }),
            "C3: beginnen_bindet_die_eigene_passage - `beginneVersuch(A)` haengt "
            "das Fenster auf A um, statt den Stand von C einzufrieren");

    p->setPlayHead (nullptr);
    p->releaseResources();
}

// ─────────────────────────────────────────────────────────────────────────
// C4 · der Vergleichspegel nimmt nur Material AUS der Passage
// ─────────────────────────────────────────────────────────────────────────
void c4PegelNurImFenster()
{
    abschnitt ("C4  vergleichspegel_nimmt_nur_material_aus_der_passage");
    auto p = mainProzessorMitBindung();
    p->prepareToPlay (kFs, kBlock);
    TestPlayHead kopf;
    kopf.pos = 480000;                       // WEIT ausserhalb des Fensters
    p->setPlayHead (&kopf);
    juce::AudioBuffer<float> puffer (2, kBlock);
    fahre (*p, kopf, puffer, 20);

    const auto a = hex32 (0x41);
    pruefe (p->merkeManuellePassage (a, "Refrain", 0, 96000), "C4: Passage gemerkt");
    pruefe (warte (*p, kopf, puffer, [&] { return p->passagenfensterFuehrt (a); }),
            "C4: die Engine fuehrt das Fenster");

    // 250 Bloecke = rund 2,7 Sekunden — weit ueber der 400-ms-Schwelle, und
    // KEIN Sample davon liegt in der Passage.
    fahre (*p, kopf, puffer, 250);
    pruefe (p->versuchAufgenommeneBloecke() == 0,
            "C4: kein einziger Block ausserhalb der Passage geht in den Pegel",
            juce::String ((juce::int64) p->versuchAufgenommeneBloecke()));
    (void) p->beginneVersuch (a);
    pruefe (! p->versuchLautheitAbgeglichen(),
            "C4: vergleichspegel_nimmt_nur_material_aus_der_passage - fremdes "
            "Material erreicht die 400-ms-Schwelle nicht");

    // Die Gegenprobe im selben Lauf: eine Passage, durch die wirklich
    // gespielt wird. Der Seek dorthin ist eine Epochengrenze, also wird sie
    // DANACH markiert — genau wie der User es taete.
    kopf.pos = 0;
    fahre (*p, kopf, puffer, 20);
    const auto b = hex32 (0x42);
    pruefe (p->merkeManuellePassage (b, "Strophe", 0, 960000), "C4: zweite Passage gemerkt");
    pruefe (warte (*p, kopf, puffer, [&] { return p->passagenfensterFuehrt (b); }),
            "C4: die Engine fuehrt das zweite Fenster");
    pruefe (fahreBisPegel (*p, kopf, puffer),
            "C4: und der Pegel nimmt darin auf");
    pruefe (p->versuchAufgenommeneBloecke() > 0,
            "C4: Material IN der Passage wird aufgenommen",
            juce::String ((juce::int64) p->versuchAufgenommeneBloecke()));
    (void) p->beginneVersuch (b);
    pruefe (p->versuchLautheitAbgeglichen(),
            "C4: und der Pegel friert ein - der Riegel sperrt die richtige "
            "Haelfte, nicht alles");

    p->setPlayHead (nullptr);
    p->releaseResources();
}

// ─────────────────────────────────────────────────────────────────────────
// C7 · der Experimentpfad nimmt den Analyse-Steuerzug
// ─────────────────────────────────────────────────────────────────────────
void c7EngineZug()
{
    abschnitt ("C7  experimentpfad_liest_die_engine_unter_dem_steuerzug");
    auto p = mainProzessorMitBindung();
    p->setzeSourcesFixtureFuerTest (eineQuelle());
    p->prepareToPlay (kFs, kBlock);
    TestPlayHead kopf;
    p->setPlayHead (&kopf);
    juce::AudioBuffer<float> puffer (2, kBlock);
    fahre (*p, kopf, puffer, 20);

    const auto a = hex32 (0x51);
    pruefe (p->merkeManuellePassage (a, "Refrain", 0, 960000), "C7: Passage gemerkt");
    pruefe (warte (*p, kopf, puffer, [&] { return p->passagenfensterFuehrt (a); }),
            "C7: die Engine fuehrt das Fenster");
    pruefe (fahreBisPegel (*p, kopf, puffer), "C7: der Pegel hat genug Material");

    const auto vorher = p->analyseSteuerZuege();
    const bool begonnen = p->beginneVersuch (a);
    const auto nachher = p->analyseSteuerZuege();
    pruefe (begonnen, "C7: der Versuch beginnt");
    pruefe (nachher > vorher,
            "C7: experimentpfad_liest_die_engine_unter_dem_steuerzug - "
            "`beginneVersuch` nimmt den Zug, statt Fingerprint, Frame und "
            "Passagenepoche neben dem Analyseworker zu lesen",
            juce::String ((juce::int64) (nachher - vorher)));

    const auto vorKandidat = p->analyseSteuerZuege();
    pruefe (p->erfasseKandidat (true), "C7: der Kandidat wird erfasst");
    pruefe (p->analyseSteuerZuege() > vorKandidat,
            "C7: und `erfasseKandidat` nimmt ihn ebenso");

    p->setPlayHead (nullptr);
    p->releaseResources();
}

// ─────────────────────────────────────────────────────────────────────────
// C8 · nach einem Ergebnis ist der naechste Versuch wieder moeglich
// ─────────────────────────────────────────────────────────────────────────
void c8ZweiVersucheNacheinander()
{
    abschnitt ("C8  zwei_versuche_nacheinander_werden_beurteilt");
    auto p = mainProzessorMitBindung();
    p->setzeSourcesFixtureFuerTest (eineQuelle());
    p->prepareToPlay (kFs, kBlock);
    TestPlayHead kopf;
    p->setPlayHead (&kopf);
    juce::AudioBuffer<float> puffer (2, kBlock);
    fahre (*p, kopf, puffer, 20);

    auto versuchFahren = [&] (const juce::String& id, const juce::String& label,
                              int nummer) -> bool
    {
        if (! p->merkeManuellePassage (id, label, 0, 4800000))
            return false;
        if (! warte (*p, kopf, puffer, [&] { return p->passagenfensterFuehrt (id); }))
            return false;
        if (! fahreBisPegel (*p, kopf, puffer))
            return false;
        if (! p->beginneVersuch (id))
            return false;
        if (! p->erfasseKandidat (nummer % 2 == 0))
            return false;
        return p->urteileVersuch ("kandidat", {}, {});
    };

    pruefe (versuchFahren (hex32 (0x61), "Erster", 0),
            "C8: der ERSTE Versuch laeuft bis zum Urteil durch");
    pruefe (p->laufenderVersuch().isEmpty(),
            "C8: und ist danach geschlossen");
    // DER Befund: `urteileVersuch` leerte den `Blindvergleich` nicht. Er
    // behielt Urteil und `gainGesetzt`, und `uebernimmVergleichspegel` lehnte
    // jeden weiteren Pegel dauerhaft ab.
    pruefe (versuchFahren (hex32 (0x62), "Zweiter", 1),
            "C8: zwei_versuche_nacheinander_werden_beurteilt - der ZWEITE laeuft "
            "genauso durch, statt stumm am uebernommenen Pegel des ersten zu "
            "scheitern");

    p->setPlayHead (nullptr);
    p->releaseResources();
}

// ─────────────────────────────────────────────────────────────────────────
// C5 · der Nichtendlich-Zaehler reist im WIREZUSTAND
// ─────────────────────────────────────────────────────────────────────────
//
// 🔑 Wiederpruefung 2 (Befund C5, M-07/R06): die Runde 2 machte den Zaehler
// nur ueber einen lokalen Getter sichtbar, und der R06-Fall rief genau diesen
// Getter auf. „Reist in den Wirezustand" war damit eine Behauptung ueber eine
// Zeile, die es nicht gab — und der Test konnte an ihrem Fehlen nicht fallen.
// Dieser Fall liest den TEXT, der wirklich gesendet wurde.
void c5ZaehlerImWirezustand()
{
    abschnitt ("C5  nicht_endliche_samples_reisen_im_wirezustand");
    auto p = mainProzessorMitBindung();
    p->setzeSourcesFixtureFuerTest (eineQuelle());
    p->prepareToPlay (kFs, kBlock);
    TestPlayHead kopf;
    p->setPlayHead (&kopf);
    juce::AudioBuffer<float> puffer (2, kBlock);
    fahre (*p, kopf, puffer, 20);

    const auto a = hex32 (0x71);
    pruefe (p->merkeManuellePassage (a, "Refrain", 0, 4800000), "C5: Passage gemerkt");
    pruefe (warte (*p, kopf, puffer, [&] { return p->passagenfensterFuehrt (a); }),
            "C5: die Engine fuehrt das Fenster");
    pruefe (fahreBisPegel (*p, kopf, puffer), "C5: der Pegel hat genug Material");
    pruefe (p->beginneVersuch (a), "C5: der Versuch beginnt");

    const auto gesendet = juce::String (p->letzterVersuchP0FuerTest());
    pruefe (gesendet.isNotEmpty(), "C5: der Befehl steht auf der Leitung");
    const auto nachricht = juce::JSON::parse (gesendet);
    const auto referenz = nachricht.getProperty ("referenz", {});
    pruefe (referenz.hasProperty ("nicht_endliche_samples"),
            "C5: nicht_endliche_samples_reisen_im_wirezustand - das Feld steht "
            "im GESENDETEN Referenzobjekt, nicht nur in einem Getter");
    pruefe ((juce::int64) referenz.getProperty ("nicht_endliche_samples", -1) == 0,
            "C5: und traegt bei sauberem Material die 0 - NACHWEISLICH keines, "
            "nicht 'nicht gemessen'",
            juce::String ((juce::int64) referenz.getProperty ("nicht_endliche_samples", -1)));

    // Der Kandidat traegt sie ebenso: die Referenz ist dieselbe Bauform.
    pruefe (p->erfasseKandidat (true), "C5: der Kandidat wird erfasst");
    const auto kandidat = juce::JSON::parse (juce::String (p->letzterVersuchP0FuerTest()));
    pruefe (kandidat.getProperty ("referenz", {}).hasProperty ("nicht_endliche_samples"),
            "C5: und der Kandidat traegt sie auch");

    p->setPlayHead (nullptr);
    p->releaseResources();
}

// ─────────────────────────────────────────────────────────────────────────
// C1 · der Sequenzhandschlag nach einem bestaetigten Resync
// ─────────────────────────────────────────────────────────────────────────
//
// 🔑 Wiederpruefung 2 (Befund C1, M-61): der Fehler lag zwischen zwei
// Sprachen. Der Broker setzt mit `resync_bestaetigen(link, 0)` seine Basis auf
// 0 und liest das als „die naechste ist 1"; das Plugin vergab seine erste
// Sequenz als 0 und setzte den Zaehler bei Reconnect nicht zurueck. Der Broker
// verwarf damit die erste Intervention JEDER Verbindung als Luecke und setzte
// `taint.unknown` sofort wieder — der R01-Fix hob sich selbst auf.
//
// Beide Seiten waren fuer sich gruen. Nur ihr VERHAELTNIS war falsch, und
// deshalb misst dieses Bein gegen eine DRITTE Instanz:
// `eq-copilot/fixtures/v3/handschlag-v1.json` ist von Hand geschrieben und
// die Ausgabe keiner der beiden Implementierungen. Das Rust-Gegenstueck in
// `broker/tests/sonde013_verdrahtung.rs` liest dieselbe Datei.

juce::File findeDatei (const juce::String& relativ)
{
    auto ausCwd = juce::File::getCurrentWorkingDirectory().getChildFile (relativ);
    if (ausCwd.existsAsFile())
        return ausCwd;
    auto ordner = juce::File::getSpecialLocation (juce::File::currentExecutableFile)
                      .getParentDirectory();
    for (int i = 0; i < 10 && ordner.exists(); ++i)
    {
        auto kandidat = ordner.getChildFile (relativ);
        if (kandidat.existsAsFile())
            return kandidat;
        ordner = ordner.getParentDirectory();
    }
    return ausCwd;
}

/** Ein Blockvorlauf ueber die Hostbruecke, genau wie der gepatchte Wrapper.

    Ohne ihn ist der Aufnahmezustand UNBEKANNT, und ein unbekannter blockiert
    den Hoermarker wie ein aktiver (M-33). Ein Bein ohne Bruecke koennte keinen
    einzigen Eingriff erzeugen. */
void bruecheBlock (eqcop::hostbruecke::Bruecke& bruecke, const TestPlayHead& kopf,
                   int samples)
{
    VstKontext c {};
    c.state = VstKontext::kContTimeValid;
    if (kopf.spielt)
        c.state |= VstKontext::kPlaying;      // kein kRecording: nachweislich aus
    c.projectTimeSamples = kopf.pos;
    c.continousTimeSamples = kopf.pos;
    c.sampleRate = kFs;
    bruecke.beginneBlock ((std::uint32_t) samples);
    bruecke.kontextAus (c);
    bruecke.uebergib();
}

// ═════════════════════════════════════════════════════════════════════════
// NAK-180 Messlauf · der R2-Weg am echten ControlClient (N-04 bis N-12)
// ═════════════════════════════════════════════════════════════════════════
//
// Diese Faelle brauchen einen vollstaendigen Marker-Zyklus. Sie waren bis
// zum Messlauf blockiert, weil `naechstesInterventionsJsonFuerTest()` selbst
// am `interventionsRing` zog und damit ein ZWEITER Konsument neben dem
// Worker war: die beiden stahlen einander die Ereignisse, und jeder zweite
// Zyklus lieferte scheinbar kein `end` (Manifest §6.6). Der Leser liest
// jetzt den Mitschnitt des echten Senders.

namespace nak180
{

/// Ein Prozessor mit Transport, Bruecke und laufendem Sender.
struct Pruefstand
{
    std::unique_ptr<EqCopilotProcessor> p;      // MSVC-Stack: Heap (NAK-175)
    TestPlayHead kopf;
    eqcop::hostbruecke::Bruecke bruecke;
    juce::AudioBuffer<float> puffer { 2, kBlock };
    juce::MidiBuffer midi;
    double phase = 0.0;
    MarkierungsAuftrag auftrag;

    Pruefstand()
        : p (mainProzessorMitBindung())
    {
        p->setPlayConfigDetails (2, 2, kFs, kBlock);
        p->prepareToPlay (kFs, kBlock);
        p->testForciereEchtzeit (true);
        p->setzeEditorOffen (true);
        p->setPlayHead (&kopf);
        bruecke.verbinde (p.get());

        MarkierungsWunsch w;
        w.modus = MarkierungsModus::solo;
        w.istResonanz = false;
        w.fVon = 120.0; w.fBis = 300.0; w.fSchwerpunkt = 200.0;
        w.fs = kFs;
        baueMarkierungsAuftrag (auftrag, w);
    }
    ~Pruefstand()
    {
        bruecke.verbinde (nullptr);
        p->setPlayHead (nullptr);
        p->releaseResources();
    }

    void bloecke (int n)
    {
        const double d = 2.0 * juce::MathConstants<double>::pi * 200.0 / kFs;
        for (int b = 0; b < n; ++b)
        {
            for (int i = 0; i < kBlock; ++i)
            {
                const float v = 0.5f * (float) std::sin (phase);
                phase += d;
                puffer.setSample (0, i, v);
                puffer.setSample (1, i, v);
            }
            bruecheBlock (bruecke, kopf, kBlock);
            p->processBlock (puffer, midi);
            kopf.pos += kBlock;
        }
    }

    /// Der Wire-Commit ohne Draht: erst danach steht ein Ereignis in Zustand
    /// "zugestellt" (E6). Ohne ihn bliebe alles in der Queue, und ein Replay
    /// nach Linkwechsel waere zu Recht unnoetig - der Fall, den N-05 misst,
    /// traete gar nicht auf.
    std::size_t zustellen() { return p->zustelleAllesFuerTest(); }

    /** Alles, was der Sender bis jetzt WIRKLICH ABGESETZT hat.

        🔑 NAK-180 Nacharbeit 1 (EP-13/R7): jede Runde stellt erst zu, dann
        erntet sie. Mitschnitt und Zaehler entstehen seit dieser Runde am
        WIRE-COMMIT, nicht beim Einreihen — ein Bein ohne Pipe muss den Commit
        deshalb selbst fahren. Vorher erntete dieselbe Schleife das blosse
        Einreihen und nannte es „gesendet": ein gescheiterter Write oder ein
        vom Aufbaufilter verworfener Eintrag blieb unsichtbar, und der Test war
        gruen aus einem Grund, den das Produkt nicht kennt. */
    std::vector<juce::var> ernte (int fristMs = 400)
    {
        std::vector<juce::var> aus;
        auto ziehe = [&] (int frist)
        {
            const auto bis = std::chrono::steady_clock::now()
                           + std::chrono::milliseconds (std::max (0, frist));
            for (;;)
            {
                zustellen();      // der Wire-Commit ohne Draht
                const auto text = p->naechstesInterventionsJsonFuerTest (0);
                if (! text.empty())
                {
                    aus.push_back (juce::JSON::parse (juce::String (text)));
                    return true;
                }
                if (std::chrono::steady_clock::now() >= bis)
                    return false;
                std::this_thread::sleep_for (std::chrono::milliseconds (2));
            }
        };
        if (! ziehe (fristMs))
            return aus;
        while (ziehe (20))
            ;
        return aus;
    }

    /// Ein voller Zyklus: Marker an, klingen lassen, aus, ausfaden.
    void zyklus (int anBloecke = 40, int ausBloecke = 120)
    {
        p->markierungEinreichen (auftrag);
        bloecke (anBloecke);
        p->markierungAus();
        bloecke (ausBloecke);
    }
};

int zaehle (const std::vector<juce::var>& v, const char* typ)
{
    int n = 0;
    for (const auto& e : v)
        if (e.getProperty ("type", {}).toString() == typ)
            ++n;
    return n;
}

/// §6.6 · Der Nebenbefund ist an der Ursache behoben: DREI Zyklen liefern
/// DREI Enden, nicht jeder zweite.
void mitschnittIstVollstaendig()
{
    abschnitt ("NAK-180 §6.6  jeder_zyklus_liefert_sein_ende");
    Pruefstand s;
    int begins = 0, ends = 0;
    for (int z = 0; z < 3; ++z)
    {
        s.zyklus();
        const auto ernte = s.ernte();
        begins += zaehle (ernte, "audible_intervention_begin");
        ends   += zaehle (ernte, "audible_intervention_end");
    }
    pruefe (begins == 3 && ends == 3,
            "NAK-180 §6.6: jeder_zyklus_liefert_sein_ende - drei Marker-Zyklen "
            "ergeben drei Begin und drei End. Vorher zog der Testleser selbst am "
            "SPSC-Ring und stahl dem Worker jedes zweite Ereignis",
            juce::String (begins) + " Begin, " + juce::String (ends) + " End");
}

/// N-05/N-09 · Reconnect bei hoerbarem Marker: das Replay reist, das echte
/// `end` schliesst dasselbe Intervall.
void n05ReplayBeiHoerbaremMarker()
{
    abschnitt ("NAK-180 N-05/N-09  replay_bei_hoerbarem_marker");
    Pruefstand s;

    s.p->markierungEinreichen (s.auftrag);
    s.bloecke (40);
    pruefe (s.p->markierungHoerbar(), "N-05: der Marker klingt", {});
    const auto vorReconnect = s.ernte();
    const int begin1 = zaehle (vorReconnect, "audible_intervention_begin");
    pruefe (begin1 == 1, "N-05: sein Begin ist gesendet", juce::String (begin1));
    // 🔑 NAK-180 Nacharbeit 1 (EP-13/R7): der Mitschnitt entsteht am
    // WIRE-COMMIT. Dass das Begin oben geerntet wurde, IST damit der Beweis
    // der Zustellung - `ernte()` hat den Commit gefahren. Ein zweiter
    // `zustellen()` findet nichts mehr, und genau das ist die Zusage: nichts
    // blieb in der Queue, das Begin steht in Zustand "zugestellt" (E6), und
    // ein Replay nach dem Linkwechsel ist deshalb noetig statt doppelt.
    pruefe (s.zustellen() == 0,
            "N-05: und der Draht hat es bestaetigt - nichts blieb in der Queue");

    // Der Neuaufbau bei HOERBAREM Marker: kein `false`, dafuer ein Replay.
    s.p->v3LinkFuerTest (false);
    s.p->v3LinkFuerTest (true);
    s.bloecke (4);
    const auto nachAufbau = s.ernte();
    const int replay = zaehle (nachAufbau, "audible_intervention_begin");
    pruefe (replay == 1,
            "N-05: replay_traegt_dieselbe_intervention_id - der Neuaufbau stellt den "
            "wahren Zustand her, statt den Marker beim Broker verschwinden zu lassen",
            juce::String (replay) + " Replay-Begin");
    if (replay == 1 && begin1 == 1)
    {
        juce::String idErst, idReplay;
        for (const auto& e : vorReconnect)
            if (e.getProperty ("type", {}).toString() == "audible_intervention_begin")
                idErst = e.getProperty ("intervention_id", {}).toString();
        for (const auto& e : nachAufbau)
            if (e.getProperty ("type", {}).toString() == "audible_intervention_begin")
                idReplay = e.getProperty ("intervention_id", {}).toString();
        pruefe (idErst.isNotEmpty() && idErst == idReplay,
                "N-05: und es ist DIESELBE `intervention_id` - nur so schliesst das "
                "normale `end` genau dieses Intervall (M-58)",
                idErst + " == " + idReplay);
    }

    // Das echte Ende schliesst es.
    s.p->markierungAus();
    s.bloecke (120);
    const auto nachEnde = s.ernte();
    pruefe (zaehle (nachEnde, "audible_intervention_end") == 1,
            "N-09: das reguläre `end` folgt - der Ausfade wurde nicht abgebrochen");
}

/// N-08 · Ring nicht leer beim Aufbau, Marker still: kein Replay, das
/// Backlog reist in Reihenfolge.
void n08BacklogOhneReplay()
{
    abschnitt ("NAK-180 N-08  backlog_reist_vollstaendig_und_in_reihenfolge");
    Pruefstand s;
    s.zyklus();
    (void) s.ernte();                       // Zyklus 1 abgeräumt

    // 🔑 NAK-180 Nacharbeit 1 (EP-19): die Lage wird ERZWUNGEN, nicht gehofft.
    //
    // Bis hier entschied der Workertakt, ob Begin und End vor dem Linkwechsel
    // schon verarbeitet waren. Beide Lagen sind zulaessig, aber nur EINE ist
    // N-08: „Ring NICHT leer (Backlog aus der Trennungszeit)". War der Ring
    // beim Aufbau leer, bestanden saemtliche Assertions, ohne dass der Fall je
    // eingetreten waere - und die zweite Lage (zugestelltes Begin, Replay
    // davor) liess sogar die doppelte ID und die wiederholte Sequenz zu, die
    // N-05 zusagt und N-08 gerade ausschliesst. Der Sender steht deshalb,
    // waehrend der zweite Zyklus laeuft.
    s.p->senderAnhaltenFuerTest (true);
    s.p->markierungEinreichen (s.auftrag);
    s.bloecke (40);
    s.p->markierungAus();
    s.bloecke (120);
    const int backlog = s.p->interventionsRingFuellstandFuerTest();
    pruefe (backlog >= 2,
            "N-08: der Ring ist beim Linkaufbau WIRKLICH nicht leer - der "
            "Backlog-Zustand ist erzwungen, nicht zufaellig",
            juce::String (backlog));
    s.p->v3LinkFuerTest (false);
    s.p->v3LinkFuerTest (true);
    s.p->senderAnhaltenFuerTest (false);
    s.bloecke (6);

    const auto ernte = s.ernte();
    const int ends = zaehle (ernte, "audible_intervention_end");
    pruefe (ends == 1, "N-08: das Ende des Zyklus reist", juce::String (ends));

    // (a) Vor jedem `end` steht ein Begin DERSELBEN `intervention_id`.
    juce::String offeneId;
    bool endeOhneBegin = false;
    juce::StringArray beginIds;
    for (const auto& e : ernte)
    {
        const auto typ = e.getProperty ("type", {}).toString();
        const auto id  = e.getProperty ("intervention_id", {}).toString();
        if (typ == "audible_intervention_begin") { offeneId = id; beginIds.add (id); }
        else if (typ == "audible_intervention_end" && id != offeneId)
            endeOhneBegin = true;
    }
    pruefe (! endeOhneBegin,
            "N-08: kein_end_ohne_sein_begin - vor jedem `end` steht ein Begin "
            "derselben `intervention_id`; genau das traf beim Broker sonst auf "
            "nichts und setzte `unknown`");

    // (b) Keine ID kommt nach dem Wechsel ZWEIMAL als Begin - das waere die
    //     doppelte `intervention_id` aus N-27.
    juce::StringArray einmalig;
    bool doppelt = false;
    for (const auto& id : beginIds)
    {
        if (einmalig.contains (id)) doppelt = true;
        einmalig.add (id);
    }
    pruefe (! doppelt,
            "N-08: und keine `intervention_id` reist zweimal als Begin",
            beginIds.joinIntoString (", "));

    // (c) Die Sequenzen sind lueckenlos aufsteigend - der Riegel, an dem der
    //     Broker eine verlorene Nachricht erkennt (`sequenz_annehmen`).
    juce::int64 vorige = -1;
    bool luecke = false;
    juce::String folge;
    for (const auto& e : ernte)
    {
        const auto seq = (juce::int64) e.getProperty ("event_sequence", -1);
        folge += juce::String (seq) + " ";
        if (vorige >= 0 && seq != vorige + 1) luecke = true;
        vorige = seq;
    }
    pruefe (! luecke,
            "N-08: backlog_reist_lueckenlos - die Sequenzen steigen um genau eins; "
            "eine Luecke waere beim Broker ein verlorenes Ereignis",
            folge.trim());
}


/// N-10 · `prepareToPlay` bei hoerbarem Marker: das tote Begin bekommt sein
/// Paar, statt beim Broker offen zu bleiben.
void n10PrepareToPlayPaar()
{
    abschnitt ("NAK-180 N-10  prepare_to_play_schliesst_das_tote_begin");
    Pruefstand s;
    s.p->markierungEinreichen (s.auftrag);
    s.bloecke (40);
    pruefe (s.p->markierungHoerbar(), "N-10: der Marker klingt vor dem Prepare", {});
    (void) s.ernte();                       // Begin ist raus
    s.zustellen();                          // und auf dem Draht bestaetigt

    // `prepareToPlay` schaltet den Marker HART ab; sein `endete` kommt vom
    // Audiothread nie (HoerMarkierung `vorbereiten` meldet keinen Uebergang).
    s.p->prepareToPlay (kFs, kBlock);
    s.p->testForciereEchtzeit (true);
    s.p->setzeEditorOffen (true);
    s.p->v3LinkFuerTest (true);             // `reconnect()` ruft ihn im Produkt
    s.bloecke (6);

    const auto ernte = s.ernte();
    const int begins = zaehle (ernte, "audible_intervention_begin");
    const int ends   = zaehle (ernte, "audible_intervention_end");
    // ZWEI Begin sind richtig: das Replay des toten Intervalls UND ein neues,
    // weil `prepareToPlay` zwar `warHoerbar` loescht, `zielGesetzt()` aber
    // stehen laesst - der Marker faehrt im naechsten Block wieder hoch. Das
    // synthetische `end` schliesst genau das TOTE, nicht das neue.
    pruefe (begins == 2 && ends == 1,
            "N-10: prepare_to_play_paar - Replay-Begin UND synthetisches `end` fuer "
            "das tote Intervall, dazu das neue Begin des wieder hochfahrenden "
            "Markers; ohne das Paar bliebe das tote Intervall beim Broker fuer "
            "immer offen",
            juce::String (begins) + " Begin, " + juce::String (ends) + " End");
    for (const auto& e : ernte)
        if (e.getProperty ("type", {}).toString() == "audible_intervention_end")
        {
            // `project_sample_end: null` kommt beim Parsen als void an.
            const auto ende = e.getProperty ("project_sample_end", {});
            pruefe (ende.isVoid() || ende.isUndefined(),
                    "N-10: das synthetische Ende traegt KEINE Projektzeit - sie ist "
                    "hier ehrlich unbekannt, und der Broker invalidiert dann "
                    "fail-closed die ganze Sitzung (M-52)",
                    ende.toString());
            const auto tail = (juce::int64) e.getProperty ("tail_samples", -1);
            pruefe (tail >= (juce::int64) (kFs / 10.0),
                    "N-10: und den vollen Nachlauf der letzten gueltigen Rate",
                    juce::String (tail));
        }
}

/// N-11 · Eine nicht als `main` klassifizierte Instanz gibt KEINE
/// Neutralitaetsbehauptung ab - der lokale Ring-Resync gehoert ihr trotzdem.
/// N-28 · Ein ABGEWIESENES Begin (Zustand 0) ist trotzdem lokal offen: der
/// Neuaufbau replayt es, und das spaetere `end` schliesst das Intervall.
///
/// Ohne E7 bliebe nur das Sticky-Bit; das `end` traefe beim Broker auf kein
/// Begin, der Nachlauf startete nie, und der regulaere Pfad nullte nicht.
void n28BeginNachSendefehler()
{
    abschnitt ("NAK-180 N-28  replay_auch_nach_abgewiesenem_begin");
    Pruefstand s;

    // Die P0-Queue bis zum Rand fuellen, damit `sendeP0` das Begin ABWEIST.
    const auto voll = s.p->fuelleP0QueueFuerTest();
    pruefe (voll > 0, "N-28: die P0-Queue ist voll", juce::String ((int) voll));

    s.p->markierungEinreichen (s.auftrag);
    s.bloecke (40);
    pruefe (s.p->markierungHoerbar(), "N-28: der Marker klingt", {});
    (void) s.ernte (100);

    // Platz schaffen und neu aufbauen: das Begin ist nirgends - nicht auf dem
    // Draht, nicht in der Queue -, also MUSS es repliziert werden.
    s.p->leereP0QueueFuerTest();
    s.p->v3LinkFuerTest (false);

    // 🔑 NAK-180 Nacharbeit 1 (EP-20/R10): das Interleaving wird ERZWUNGEN.
    //
    // N-28 sagt zu, dass ein Replay, das der geweckte Worker WAEHREND des
    // Link-Callbacks einreiht, den Aufbaufilter ueberlebt - weil die
    // Generation vor dem Callback vergeben ist. Liefen Aufbau und Replay
    // nacheinander, bestuende der Test auch dann, wenn das Replay erst nach
    // der Rueckkehr entsteht: gemessen waere die Absicht, nicht das
    // Zusammentreffen. Die Ordnung traegt eine Bedingungsvariable, nie ein
    // `sleep`.
    std::mutex m;
    std::condition_variable cv;
    bool zugFertig = false;
    s.p->setzeZugHakenFuerTest ([&] (int phase)
    {
        if (phase != 2)
            return;
        {
            std::lock_guard<std::mutex> l (m);
            zugFertig = true;
        }
        cv.notify_all();
    });
    s.p->senderAnhaltenFuerTest (true);     // kein Zug, bevor der Callback steht

    std::string imCallbackGeerntet;
    s.p->v3LinkAufbauFuerTest ([&]
    {
        // Der Callback STEHT. Jetzt darf der Worker - und wir warten, bis
        // sein Zug wirklich durch ist, statt ihm Zeit zu schenken.
        s.p->senderAnhaltenFuerTest (false);
        std::unique_lock<std::mutex> l (m);
        cv.wait (l, [&] { return zugFertig; });
        l.unlock();
        // Noch INNERHALB des Callbacks: was hat der Zug abgesetzt?
        s.zustellen();
        imCallbackGeerntet = s.p->naechstesInterventionsJsonFuerTest (0);
    });
    s.p->setzeZugHakenFuerTest ({});

    const auto imCallback = juce::JSON::parse (juce::String (imCallbackGeerntet));
    pruefe (imCallback.getProperty ("type", {}).toString() == "audible_intervention_begin",
            "N-28: replay_waehrend_des_link_callbacks - das Replay entsteht IM "
            "Callback und ueberlebt den Aufbaufilter, weil die Generation vor dem "
            "Callback vergeben ist (R10)",
            juce::String (imCallbackGeerntet.substr (0, 90)));

    s.bloecke (6);
    const auto nachAufbau = s.ernte();
    pruefe (zaehle (nachAufbau, "audible_intervention_begin") == 0,
            "N-28: und GENAU EIN Replay - der Zustellstand des Hook-Replays "
            "verhindert das zweite (EP-09/N-27)",
            juce::String (zaehle (nachAufbau, "audible_intervention_begin")));

    s.p->markierungAus();
    s.bloecke (140);
    const auto nachEnde = s.ernte();
    pruefe (zaehle (nachEnde, "audible_intervention_end") == 1,
            "N-28: und das `end` folgt - der regulaere Pfad nullt");
}

/// N-29/N-35 · Die Ordnungsregel: ein `end` reist nie ohne sein Begin, und
/// Vergleich und Einreihen liegen im selben Zug.
void n29OrdnungsregelUndZug()
{
    abschnitt ("NAK-180 N-29/N-35  kein_end_ohne_begin_im_selben_zug");
    Pruefstand s;

    // Ein vollstaendiger Zyklus, zugestellt: das Begin steht in Zustand
    // "zugestellt" auf Generation G.
    s.p->markierungEinreichen (s.auftrag);
    s.bloecke (40);
    (void) s.ernte();
    s.zustellen();

    // Linkwechsel, DANN das Ende. Der Sendezug sieht: Begin auf G zugestellt,
    // laufende Generation G+1 -> Replay voran, dann das `end`.
    s.p->v3LinkFuerTest (false);
    s.p->v3LinkFuerTest (true);
    s.p->markierungAus();
    s.bloecke (140);

    const auto ernte = s.ernte();
    // Vor jedem `end` steht ein Begin derselben ID - das ist die Zusage.
    juce::String offeneId;
    bool endeOhneBegin = false;
    for (const auto& e : ernte)
    {
        const auto typ = e.getProperty ("type", {}).toString();
        const auto id  = e.getProperty ("intervention_id", {}).toString();
        if (typ == "audible_intervention_begin") offeneId = id;
        else if (typ == "audible_intervention_end" && id != offeneId) endeOhneBegin = true;
    }
    pruefe (! endeOhneBegin && zaehle (ernte, "audible_intervention_end") == 1,
            "N-29: kein_end_ohne_begin_desselben_links - nach dem Linkwechsel geht das "
            "Replay-Begin unmittelbar voran; ohne die Ordnungsregel traefe das `end` "
            "beim Broker auf nichts und setzte `unknown`");

    juce::int64 vorige = -1;
    bool luecke = false;
    juce::String folge;
    for (const auto& e : ernte)
    {
        const auto seq = (juce::int64) e.getProperty ("event_sequence", -1);
        folge += juce::String (seq) + " ";
        if (vorige >= 0 && seq != vorige + 1) luecke = true;
        vorige = seq;
    }
    pruefe (! luecke,
            "N-29: die Sequenzen bleiben lueckenlos - eine Vertauschung waere beim "
            "Broker ein verlorenes Ereignis",
            folge.trim());
}

/// N-35 · Vergleich und Einreihen liegen unter DERSELBEN Sperre.
///
/// 🔑 NAK-180 Nacharbeit 1 (EP-16): der Fall wird jetzt GEFAHREN, nicht
/// beschrieben. Bis hier wertete derselbe Abschnitt nur die sequentiell
/// erzeugte Ereignisfolge aus; kein Faden versuchte den Aufbau zwischen
/// Vergleich und Einreihen, und `sendeMutex` haette aufgeteilt werden koennen,
/// ohne dass etwas fiel. Der Testhaken im Zug haelt genau dort an, ein zweiter
/// Faden versucht den Linkwechsel, und gemessen wird, dass er erst NACH dem
/// Einreihen zum Zug kommt. Rotbeweis: den Aufbauzug ohne `sendeMutex` fahren
/// - dann laeuft der zweite Faden mitten durch, und das `end` traegt die
/// falsche Generation.
void n35SendezugGegenAufbauzug()
{
    abschnitt ("NAK-180 N-35  vergleich_und_einreihen_sind_ein_zug");
    Pruefstand s;

    // Ein zugestelltes Begin auf Generation G - die Lage, in der die
    // Zustandspruefung ueberhaupt etwas entscheidet. `wireGeneration` steht
    // erst nach dem ersten Aufbauzug ueber 0, und `0` heisst "keine Aussage".
    s.p->v3LinkFuerTest (true);
    s.p->markierungEinreichen (s.auftrag);
    s.bloecke (40);
    (void) s.ernte();
    s.zustellen();

    // 🔑 Nacharbeit 2 (WN-06): die Ordnung traegt keine Pause mehr.
    //
    // Bis hierher stand nach dem Start des Wechslers ein `sleep_for(30ms)`.
    // Er bewies weder, dass der Faden den Aufbauzug ueberhaupt BETRETEN hatte,
    // noch dass er an `sendeMutex` stand: bei verzoegerter Planung wurde
    // `weiter` gesetzt, bevor er lief, und der Test blieb auch mit
    // AUFGETEILTER Sperre gruen. Zwei Schranken IM Aufbauzug ersetzen ihn:
    // Phase 0 meldet den Eintritt (die Sperre wird als naechstes angefordert),
    // Phase 1 die Uebernahme. Faellt Phase 1, waehrend der Sendezug die Sperre
    // noch haelt, ist es nicht dieselbe Sperre - und genau das ist der
    // Rotbeweis.
    std::mutex m;
    std::condition_variable cv;
    bool amHaken = false;              // der Sendezug steht zwischen Vergleich und Einreihen
    bool wechslerAmEintritt = false;   // Phase 0: der Aufbauzug ist betreten
    bool wechslerHatSperre = false;    // Phase 1: die Sperre ist uebernommen
    bool weiter = false;
    std::uint64_t generationAmHaken = 0;
    bool sperreDurchbrochen = false;

    s.p->setzeAufbauZugHakenFuerTest ([&] (int phase)
    {
        {
            std::lock_guard<std::mutex> l (m);
            if (phase == 0) wechslerAmEintritt = true;
            else            wechslerHatSperre  = true;
        }
        cv.notify_all();
    });

    s.p->setzeZugHakenFuerTest ([&] (int phase)
    {
        if (phase != 1)
            return;
        generationAmHaken = s.p->wireGenerationFuerTest();
        {
            std::lock_guard<std::mutex> l (m);
            amHaken = true;
        }
        cv.notify_all();
        std::unique_lock<std::mutex> l (m);
        cv.wait (l, [&] { return weiter; });
        // Hat der Wechsler die Sperre uebernommen, waehrend dieser Zug sie
        // haelt? Unter EINER Sperre ist das unmoeglich.
        sperreDurchbrochen = wechslerHatSperre;
    });

    // Der zweite Faden will den Linkwechsel GENAU HIER erzwingen. Er nimmt
    // `sendeMutex` im Aufbauzug und steht deshalb, bis der Sendezug fertig
    // ist - das IST die Zusage.
    std::thread wechsler ([&]
    {
        {
            std::unique_lock<std::mutex> l (m);
            cv.wait (l, [&] { return amHaken; });
        }
        s.p->v3LinkFuerTest (false);
        s.p->v3LinkFuerTest (true);
    });
    std::thread ausloeser ([&]
    {
        std::unique_lock<std::mutex> l (m);
        // (a) Kausale Kante: der Wechsler steht WIRKLICH im Aufbauzug.
        cv.wait (l, [&] { return wechslerAmEintritt; });
        // (b) Falsifikationsbudget, nicht Ordnung: greift `sendeMutex`, KANN
        //     Phase 1 hier nicht fallen und die Frist laeuft ab. Ist die
        //     Sperre aufgeteilt, meldet sie sich sofort - und der Sendezug
        //     liest sie unten als `sperreDurchbrochen`.
        cv.wait_for (l, std::chrono::milliseconds (400),
                     [&] { return wechslerHatSperre; });
        weiter = true;
        l.unlock();
        cv.notify_all();
    });

    // Jetzt das Ende erzeugen - der Sendezug laeuft in den Haken. SYNCHRON,
    // damit nicht der 50-ms-Takt des Workers ueber den Zeitpunkt entscheidet.
    s.p->senderAnhaltenFuerTest (true);
    s.p->markierungAus();
    s.bloecke (140);
    s.p->interventionenSendenFuerTest();

    ausloeser.join();
    wechsler.join();
    s.p->setzeZugHakenFuerTest ({});
    s.p->setzeAufbauZugHakenFuerTest ({});
    s.p->senderAnhaltenFuerTest (false);

    pruefe (generationAmHaken != 0 && wechslerAmEintritt && ! sperreDurchbrochen,
            "N-35/WN-06: der Linkwechsel kommt NICHT zwischen Vergleich und "
            "Einreihen - der Wechsler steht BEWIESEN im Aufbauzug (Phase 0) und "
            "bekommt `sendeMutex` erst, wenn der Sendezug ihn freigibt",
            juce::String ((int) generationAmHaken)
                + (wechslerAmEintritt ? ", Eintritt bewiesen" : ", NIE EINGETRETEN")
                + (sperreDurchbrochen ? ", Sperre DURCHBROCHEN" : ", Sperre haelt"));

    // Und auf dem neuen Link steht kein `end` ohne sein Begin.
    s.bloecke (6);
    const auto nach = s.ernte();
    juce::String offene;
    bool ohneBegin = false;
    for (const auto& e : nach)
    {
        const auto typ = e.getProperty ("type", {}).toString();
        const auto id  = e.getProperty ("intervention_id", {}).toString();
        if (typ == "audible_intervention_begin") offene = id;
        else if (typ == "audible_intervention_end" && id != offene) ohneBegin = true;
    }
    pruefe (! ohneBegin,
            "N-35: und auf G+1 kommt kein `end` ohne Begin an");
}

void n11KeineBehauptungOhneMain()
{
    abschnitt ("NAK-180 N-11  keine_wireaussage_ohne_main");
    auto p = mainProzessor();               // ohne Bindung: keine gueltige Adresse
    p->prepareToPlay (kFs, kBlock);
    const int gefuellt = p->interventionsRingFuellenFuerTest();
    pruefe (gefuellt > 0 && p->v3StatusFuerTest().interventionStateUnknown,
            "N-11: der Ueberlauf steht", juce::String (gefuellt));
    p->interventionsRingLeerenFuerTest();
    p->v3LinkFuerTest (true);
    pruefe (! p->v3StatusFuerTest().interventionStateUnknown,
            "N-11: der LOKALE Resync laeuft trotzdem - Ring, Sticky-Bit und "
            "Sequenzzaehler sind Prozessorzustand und gehoeren jeder Klasse");
    p->releaseResources();
}

/// N-12 · Ein abgewiesener Aufbau-Heartbeat gibt seine Aussage zurueck.
void n12AussageKommtZurueck()
{
    abschnitt ("NAK-180 N-12  abgewiesener_heartbeat_gibt_die_aussage_zurueck");
    // 🔑 NAK-180 Nacharbeit 1 (EP-06/EP-18): gemessen am ECHTEN
    // Heartbeat-Schritt der Sendeschleife, nicht an einem isolierten Atomic.
    //
    // Der Schritt verbraucht die Aufbau-Aussage per CAS, vergibt die Marke,
    // bildet den Wiretext und reiht ihn ein. Weist die volle P0-Queue ab,
    // MUSS die Aussage zurueckkommen - sonst faende der Broker nie ein
    // `false`, und die Sitzung bliebe fuer immer gesperrt (D-01 in neuer
    // Form). Ein Atomic daneben konnte diese Kette nicht messen.
    auto p = mainProzessorMitBindung();
    p->prepareToPlay (kFs, kBlock);

    // (1) Ein neutraler Aufbau setzt die Aussage.
    p->v3LinkFuerTest (true);

    // (2) Die P0-Queue voll: der Heartbeat wird ABGEWIESEN.
    const auto voll = p->fuelleP0QueueFuerTest();
    pruefe (voll > 0, "N-12: die P0-Queue ist voll", juce::String ((int) voll));
    std::string text;
    const bool eingereiht = p->v3HeartbeatSchrittFuerTest (text, 1);
    pruefe (! eingereiht,
            "N-12: der Aufbau-Heartbeat wird abgewiesen - genau der Fall, dessen "
            "Rueckgabewert bisher verworfen wurde");
    pruefe (text.find ("\"intervention_state_unknown\":false") != std::string::npos,
            "N-12: er TRUG die Aussage - sie ist per CAS verbraucht",
            juce::String (text.substr (0, 80)));

    // (3) Platz schaffen: der naechste Heartbeat traegt die Aussage WIEDER.
    p->leereP0QueueFuerTest();
    std::string text2;
    const bool eingereiht2 = p->v3HeartbeatSchrittFuerTest (text2, 2);
    pruefe (eingereiht2
                && text2.find ("\"intervention_state_unknown\":false") != std::string::npos,
            "N-12: aussage_kommt_zurueck - der naechste erfolgreich eingereihte "
            "Heartbeat traegt sie; ohne die Rueckstellung waere sie fuer immer weg",
            juce::String (text2.substr (0, 80)));

    // (4) Und GENAU EINMAL: der dritte traegt sie nicht mehr.
    std::string text3;
    p->v3HeartbeatSchrittFuerTest (text3, 3);
    pruefe (text3.find ("intervention_state_unknown") == std::string::npos,
            "N-02/N-12: der naechste Heartbeat schweigt wieder - die Aussage reist "
            "genau einmal",
            juce::String (text3.substr (0, 80)));

    // (5) Gegenprobe zur Generationsbindung: eine Aussage, die einem NEUEREN
    //     Link gehoert, wird von der Rueckstellung nicht mitgenommen.
    std::atomic<std::uint64_t> flag { 0 };
    flag.store (9);                         // Callback von G+1
    std::uint64_t leer = 0;
    const bool ueberschrieben = flag.compare_exchange_strong (leer, 7);
    pruefe (! ueberschrieben && flag.load() == 9,
            "N-12: und sie nimmt die Aussage eines neueren Links NICHT mit");
    p->releaseResources();
}

/// N-36 · Ein `end`, das beim Aufbau von G+1 noch in der Queue liegt, reist
/// nie ohne sein Begin - und der Mitschnitt entsteht erst am Wire-Commit.
///
/// 🔑 NAK-180 Nacharbeit 1 (EP-01/EP-13): dieser Bruch braucht KEIN Rennen.
/// Er entsteht allein daraus, dass die Queue den Linkwechsel ueberlebt: der
/// Sender loeschte das offene Begin, sobald das `end` eingereiht war, und der
/// Aufbauzug des naechsten Links fand nichts mehr, was er voranstellen
/// koennte. Beim Broker traf das `end` auf nichts, und die Sitzung nullte nie.
///
/// Rotbeweis: `abschlussBegin` in `sendeEnde` nicht setzen -> kein Replay vor
/// dem `end`.
void n36EndeUeberlebtDenLinkwechsel()
{
    abschnitt ("NAK-180 N-36  end_in_der_queue_reist_nie_ohne_sein_begin");
    Pruefstand s;
    s.p->v3LinkFuerTest (true);

    // Ein zugestelltes Begin auf G.
    s.p->markierungEinreichen (s.auftrag);
    s.bloecke (40);
    (void) s.ernte();
    const auto gesendetVorEnde = s.p->interventionenGesendetFuerTest();

    // Das Ende entsteht - und wird EINGEREIHT, aber NICHT zugestellt.
    // Dafuer steht der Sender, und der Zug laeuft genau einmal synchron.
    s.p->senderAnhaltenFuerTest (true);
    s.p->markierungAus();
    s.bloecke (140);
    s.p->interventionenSendenFuerTest();

    // 🔑 EP-13/R7: eingereiht ist NICHT gesendet. Der Zaehler steht still,
    // bis der Draht bestaetigt.
    pruefe (s.p->interventionenGesendetFuerTest() == gesendetVorEnde,
            "N-36/EP-13: das eingereihte `end` zaehlt noch NICHT als gesendet - "
            "'gesendet' heisst Wire-Commit",
            juce::String ((int) s.p->interventionenGesendetFuerTest()) + " statt "
                + juce::String ((int) gesendetVorEnde));

    // Und JETZT stirbt der Link, waehrend das `end` in der Queue liegt.
    s.p->v3LinkFuerTest (false);
    s.p->v3LinkFuerTest (true);
    s.p->senderAnhaltenFuerTest (false);
    s.bloecke (6);

    const auto ernte = s.ernte();
    juce::String offeneId;
    bool endeOhneBegin = false;
    int begins = 0;
    for (const auto& e : ernte)
    {
        const auto typ = e.getProperty ("type", {}).toString();
        const auto id  = e.getProperty ("intervention_id", {}).toString();
        if (typ == "audible_intervention_begin") { offeneId = id; ++begins; }
        else if (typ == "audible_intervention_end" && id != offeneId)
            endeOhneBegin = true;
    }
    pruefe (! endeOhneBegin && begins >= 1,
            "N-36: der Aufbauzug stellt das Replay-Begin VOR das wartende `end` - "
            "ohne die Zustellpruefung traefe es beim Broker auf nichts",
            juce::String (begins) + " Begin, End ohne Begin: "
                + (endeOhneBegin ? "ja" : "nein"));
    pruefe (zaehle (ernte, "audible_intervention_end") == 1,
            "N-36: und das `end` reist genau einmal");
}

/// N-05/M-58 · Das einmalige `false` wartet den NACHLAUF ab.
///
/// 🔑 NAK-180 Nacharbeit 1 (EP-02): der Bericht endete bisher, sobald Ring,
/// lokales Begin und Marker leer waren. Bei einem laengeren Marker erreichte
/// das `false` den Broker noch waehrend `tail_samples_offen > 0`, wurde dort
/// verworfen und nie wiederholt - `berichtOffen` und die Aufbauaussage waren
/// mit ihm verbraucht, und die Sitzung blieb fuer immer gesperrt.
///
/// Rotbeweis: `nachlaufAbgelaufen()` immer `true` liefern lassen.
void n05NachlaufWirdAbgewartet()
{
    abschnitt ("NAK-180 N-05/M-58  false_wartet_den_nachlauf_ab");
    Pruefstand s;

    // Ein nicht neutraler Aufbau bei hoerbarem Marker oeffnet den Bericht.
    s.p->markierungEinreichen (s.auftrag);
    s.bloecke (40);
    pruefe (s.p->markierungHoerbar(), "N-05: der Marker klingt", {});
    s.p->v3LinkFuerTest (true);
    const auto g = s.p->wireGenerationFuerTest();
    pruefe (s.p->berichtOffenFuerTest() == g,
            "N-05: der Bericht ist offen", juce::String ((int) g));

    // Der Marker endet; das `end` geht auf den Draht und stellt die Frist.
    s.p->markierungAus();
    s.bloecke (140);
    (void) s.ernte();

    // 🔑 Unmittelbar danach ist der Nachlauf NOCH nicht abgelaufen: der Tail
    // eines 40-Block-Markers ist rund 0,9 s, dazu eine Heartbeat-Periode
    // Marge. Der Bericht MUSS deshalb offen bleiben.
    s.p->interventionenSendenFuerTest();
    pruefe (s.p->berichtOffenFuerTest() == g,
            "N-05/M-58: der Bericht bleibt offen, solange der Nachlauf laeuft - "
            "ein `false` waehrend `tail_samples_offen > 0` wird beim Broker "
            "verworfen und von niemandem wiederholt",
            juce::String ((int) s.p->berichtOffenFuerTest()));

    // Und nach Ablauf der Frist schliesst er - genau einmal.
    const auto bis = std::chrono::steady_clock::now() + std::chrono::seconds (8);
    while (s.p->berichtOffenFuerTest() != 0
           && std::chrono::steady_clock::now() < bis)
    {
        s.bloecke (2);
        s.p->interventionenSendenFuerTest();
    }
    pruefe (s.p->berichtOffenFuerTest() == 0,
            "N-05: nach dem Nachlauf schliesst der Bericht - der regulaere Pfad "
            "nullt die Sitzung ohne neuen Link",
            juce::String ((int) s.p->berichtOffenFuerTest()));
}

/// WN-01/N-36 · Ein `end`, das auf seinen WIRE-COMMIT wartet, macht den
/// Aufbau des naechsten Links NICHT neutral.
///
/// 🔑 NAK-180 Nacharbeit 2 (WN-01): `sendeEnde()` loescht `sendeBeginOffen`
/// schon beim EINREIHEN des `end`. Stirbt der Link in diesem Fenster, sagten
/// alle drei bisherigen Terme des Neutralitaetsurteils „neutral" - Ring leer,
/// kein offenes Begin, kein hoerbarer Marker -, obwohl der Broker weder
/// Abschluss noch Nachlauf gesehen hatte. G+1 trug dann ein ausdrueckliches
/// `false`, `resync_bestaetigen` loeschte das Unknown, und das unmittelbar
/// danach zugestellte `end` startete einen Tail, den derselbe R1-Resync
/// gerade weggeraeumt hatte (N-05/M-58).
///
/// Rotbeweis: den Term `! abschlussOffen` aus dem Praedikat nehmen.
void wn01WartendesEndeVerbietetNeutralitaet()
{
    abschnitt ("NAK-180 WN-01/N-36  wartendes_end_verbietet_das_neutralitaetsurteil");
    Pruefstand s;
    s.p->v3LinkFuerTest (true);
    const auto g1 = s.p->wireGenerationFuerTest();

    // Ein ZUGESTELLTES Begin auf G1 - nur so entsteht ueberhaupt ein
    // `abschlussBegin`, und nur dann kann der Aufbauzug etwas voranstellen.
    s.p->markierungEinreichen (s.auftrag);
    s.bloecke (40);
    (void) s.ernte();                     // `ernte()` faehrt den Wire-Commit mit

    // Das `end` entsteht und wird EINGEREIHT - der Sender steht, es geht nicht
    // auf den Draht.
    s.p->senderAnhaltenFuerTest (true);
    s.p->markierungAus();
    s.bloecke (140);
    s.p->interventionenSendenFuerTest();

    pruefe (s.p->interventionsRingFuellstandFuerTest() == 0
                && ! s.p->markierungHoerbar()
                && s.p->abschlussOffenFuerTest(),
            "WN-01: Ring leer, kein offenes Begin, Marker still - und TROTZDEM "
            "wartet ein `end` auf den Draht; genau diese Lage nannte das alte "
            "Praedikat neutral",
            juce::String (s.p->interventionsRingFuellstandFuerTest()) + " im Ring, "
                + (s.p->abschlussOffenFuerTest() ? "Abschluss offen" : "nichts offen"));

    // Der Linkwechsel.
    s.p->v3LinkFuerTest (false);
    s.p->v3LinkFuerTest (true);
    const auto g2 = s.p->wireGenerationFuerTest();
    pruefe (g2 == g1 + 1 && s.p->berichtOffenFuerTest() == g2
                && s.p->replayFaelligFuerTest() == g2,
            "WN-01: G+1 urteilt NICHT neutral - Bericht und Replay tragen SEINE "
            "Generation, statt dass ein `false` den frischen Tail wegraeumt",
            "G" + juce::String ((int) g1) + " -> G" + juce::String ((int) g2)
                + ", Bericht " + juce::String ((int) s.p->berichtOffenFuerTest()));

    // Und der erste Heartbeat von G+1 traegt ausdrueckliches `true` (R2).
    std::string hb;
    const bool eingereiht = s.p->v3HeartbeatSchrittFuerTest (hb, 1);
    pruefe (eingereiht
                && hb.find (R"("intervention_state_unknown":true)") != std::string::npos,
            "WN-01/R2: sein erster Heartbeat traegt ausdrueckliches `true` - kein "
            "`false`, das `resync_bestaetigen` ausloest",
            juce::String (hb.substr (0, 110)));

    // Danach der wahre Zustand: Replay voran, dann das `end`.
    s.p->senderAnhaltenFuerTest (false);
    s.bloecke (6);
    s.p->interventionenSendenFuerTest();
    const auto ernte = s.ernte();
    juce::String offeneId;
    bool endeOhneBegin = false;
    for (const auto& e : ernte)
    {
        const auto typ = e.getProperty ("type", {}).toString();
        const auto id  = e.getProperty ("intervention_id", {}).toString();
        if (typ == "audible_intervention_begin") offeneId = id;
        else if (typ == "audible_intervention_end" && id != offeneId) endeOhneBegin = true;
    }
    pruefe (! endeOhneBegin && zaehle (ernte, "audible_intervention_end") == 1,
            "WN-01: auf G+1 steht das Replay-Begin VOR dem `end` - der Broker "
            "sieht das Paar, nicht ein `end` ohne Anfang",
            juce::String ((int) ernte.size()) + " Nachrichten");

    // Und erst nach dem Nachlauf schliesst der Bericht - das einmalige `false`
    // reist nie in einen laufenden Tail.
    s.p->interventionenSendenFuerTest();
    pruefe (s.p->berichtOffenFuerTest() == g2,
            "WN-01/N-05: der Bericht bleibt offen, solange der Nachlauf des gerade "
            "zugestellten `end` laeuft",
            juce::String ((int) s.p->berichtOffenFuerTest()));
    const auto bis = std::chrono::steady_clock::now() + std::chrono::seconds (8);
    while (s.p->berichtOffenFuerTest() != 0
           && std::chrono::steady_clock::now() < bis)
    {
        s.bloecke (2);
        s.p->interventionenSendenFuerTest();
    }
    pruefe (s.p->berichtOffenFuerTest() == 0,
            "WN-01/N-05: und danach schliesst er - ueber den REGULAEREN Pfad, ohne "
            "neuen Link",
            juce::String ((int) s.p->berichtOffenFuerTest()));
}

/// WN-02/N-27 · Der Zustellrueckruf meldet die Generation des TATSAECHLICHEN
/// Wire-Commits, nicht die des Einreihens - gemessen am ECHTEN Writer.
///
/// 🔑 NAK-180 Nacharbeit 2 (WN-02): ein Interventionsereignis ueberlebt den
/// Reconnect - es wird zurueckgelegt und auf dem NAECHSTEN Link geschrieben.
/// Sein Queueeintrag trug aber weiter die alte Zahl. Meldete der Rueckruf sie,
/// stand ein auf G+1 geschriebenes Begin beim Prozessor als „auf G
/// zugestellt": `replayNoetig` erzeugte beim `end` ein ZWEITES Begin mit
/// derselben `intervention_id` und derselben Sequenz, und `sequenz_annehmen`
/// setzte die Sitzung `unknown` - genau was N-27 hier ausschliesst.
///
/// 🔑 Nacharbeit 3 (WA-01): bis hierher stellte der Fall ueber
/// `zustelleAllesFuerTest()` zu. Der Haken IST ein Wire-Commit, aber er waehlt
/// seine Zustellgeneration SELBST (`ControlClient.cpp:1516-1522`) - die
/// Ruecknahme der Produktzeile `ControlClient.cpp:2129` ALLEIN konnte den Test
/// deshalb nicht fallen lassen. Er mass die Generationswahl des Testhakens,
/// nicht die der Schreibschleife.
///
/// Jetzt faehrt der ECHTE ControlClient gegen einen Probe-Server, wie
/// `r01Resync` seit WN-08: das Begin wird auf G EINGEREIHT, der Server
/// verschwindet vor dem Write, der Eintrag wird zurueckgelegt und auf G+1 von
/// der Schreibschleife GESCHRIEBEN. Gemessen wird, was der Server empfing.
///
/// Rotbeweis: in der Schreibschleife wieder `p0Eintrag.generation` melden.
void wn02ZustellungMeldetDieWireGeneration()
{
    abschnitt ("NAK-180 WN-02/N-27  zustellung_meldet_die_generation_des_wire_commits");
    Pruefstand s;

    const auto pipe = testPipeName ("sonde013.wn02");
    pruefe (nakama::ipc::istProbePipename (pipe),
            "WN-02: der Testserver liegt im PROBE-Namensraum - nie auf der "
            "Produktionspipe (Paragraph 48.3)",
            juce::String (pipe));
    pruefe (s.p->v3ProbeGegenstelleFuerTest (pipe, testExeErwartung()),
            "WN-02/WA-01: der ECHTE ControlClient zeigt auf den Probe-Server");

    auto warteAuf = [] (int fristMs, auto&& bedingung)
    {
        const auto bis = std::chrono::steady_clock::now()
                       + std::chrono::milliseconds (fristMs);
        while (std::chrono::steady_clock::now() < bis)
        {
            if (bedingung())
                return true;
            std::this_thread::sleep_for (std::chrono::milliseconds (5));
        }
        return bedingung();
    };
    auto verbunden = [&s] ()
    {
        return s.p->controlV3Snapshot().status
               == nakama::ipc::ControlClient::Status::verbunden;
    };

    // ── G: der erste Link steht wirklich ─────────────────────────────────
    auto server = std::make_unique<TestServer> (pipe);
    pruefe (server->starten(), "WN-02: der erste Probe-Server steht");
    s.p->v3StartFuerTest();
    pruefe (warteAuf (8000, verbunden),
            "WN-02/WA-01: Hello und Welcome sind gelaufen - der Handschlag ist "
            "echt, kein Testhaken");
    const auto g1 = s.p->wireGenerationFuerTest();

    // Der Sender wird BESTAETIGT angehalten: WANN das Begin die Queue erreicht,
    // ist Teil der Messung und darf nicht am Workertakt haengen.
    s.p->senderAnhaltenFuerTest (true);
    pruefe (s.p->warteAufSenderPauseFuerTest(),
            "WN-02: der Sender ist bestaetigt angehalten - das Einreihen faehrt "
            "der Fall selbst");

    s.p->markierungEinreichen (s.auftrag);
    s.bloecke (40);
    pruefe (s.p->markierungHoerbar(),
            "WN-02: der Marker klingt - der Aufbau von G+1 urteilt darum nicht neutral");

    // ── Der Link stirbt VOR dem Write ────────────────────────────────────
    server->stoppen();
    pruefe (warteAuf (8000, [&] { return ! verbunden(); }),
            "WN-02/WA-01: der Link ist wirklich weg - ab hier kann kein Write "
            "mehr gelingen");

    // ERST JETZT wird das Begin eingereiht: auf G, ohne Draht. Die Generation
    // steht still, bis der naechste Aufbauzug sie erhoeht.
    s.p->interventionenSendenFuerTest();
    pruefe (s.p->wireGenerationFuerTest() == g1,
            "WN-02: das Begin ist auf G EINGEREIHT - der Aufbauzug hat noch "
            "nicht gezogen",
            "G" + juce::String ((int) g1));

    // ── G+1: derselbe Eintrag geht vom ECHTEN Writer auf den Draht ───────
    server = std::make_unique<TestServer> (pipe);
    pruefe (server->starten(), "WN-02: der zweite Probe-Server steht");
    s.p->v3ReconnectFuerTest();
    pruefe (warteAuf (12000, verbunden),
            "WN-02/WA-01: der Client steht auf dem NEUEN Link");
    const auto g2 = s.p->wireGenerationFuerTest();
    pruefe (g2 > g1, "WN-02: der Link ist gewechselt",
            "G" + juce::String ((int) g1) + " -> G" + juce::String ((int) g2));

    // Der Wire-Commit des Begins ist gelaufen, wenn der Zustellrueckruf gelaufen
    // ist - `interventionenGesendet` zaehlt GENAU dort (`mitschnittZustellen`).
    // Der Serverempfang allein waere die schwaechere Kante: die Bytes koennen
    // vor dem Rueckruf gelesen sein.
    // 🔑 Erst messen, DANN berichten (wie in `r01Resync`): die Argumente von
    // `pruefe` werden in unbestimmter Reihenfolge ausgewertet, ein Zaehler im
    // Detailtext koennte sonst VOR der Wartebedingung gelesen werden.
    const bool beginZugestellt =
        warteAuf (10000, [&] { return s.p->interventionenGesendetFuerTest() >= 1; });
    const int nachBegin = (int) s.p->interventionenGesendetFuerTest();
    pruefe (beginZugestellt,
            "WN-02/WA-01: das eingereihte Begin ueberlebt den Reconnect und wird "
            "von der SCHREIBSCHLEIFE auf G+1 zugestellt",
            juce::String (nachBegin) + " zugestellt");

    // ── Das `end` folgt. Sein Begin ist auf DIESEM Link zugestellt; ein
    //    Replay waere die doppelte `intervention_id` aus N-27. ────────────
    s.p->markierungAus();
    s.bloecke (140);
    s.p->interventionenSendenFuerTest();
    const bool endeZugestellt =
        warteAuf (10000, [&] { return s.p->interventionenGesendetFuerTest() >= 2; });
    const int nachEnde = (int) s.p->interventionenGesendetFuerTest();
    pruefe (endeZugestellt,
            "WN-02: auch das `end` ist zugestellt",
            juce::String (nachEnde) + " zugestellt");

    // Gemessen wird, was der SERVER empfing - nicht der Mitschnitt daneben.
    auto p0Texte = [&server] ()
    {
        std::lock_guard<std::mutex> l (server->textMutex);
        return server->p0Texte;
    };
    auto amDraht = [&p0Texte] (const char* typ)
    {
        int n = 0;
        for (const auto& t : p0Texte())
            if (t.find (std::string ("\"type\":\"") + typ + "\"") != std::string::npos)
                ++n;
        return n;
    };
    // 🔑 Die Ordnung ist der DRAHT, keine Pause: die Pipe ist FIFO, und ein
    // Replay-Begin ginge dem `end` immer voran (es entsteht im selben Sendezug
    // davor). Wer das `end` beim Server sieht, sieht damit auch alles, was
    // vorher gereist ist - ein `sleep` als einzige Ordnung waere genau das,
    // was WN-06 andernorts abgeschafft hat.
    pruefe (warteAuf (10000, [&] { return amDraht ("audible_intervention_end") >= 1; }),
            "WN-02/WA-01: das `end` ist beim Server angekommen - und mit ihm alles, "
            "was ihm auf dem Draht vorausging");

    std::map<juce::String, int> beginsJeId;
    std::set<juce::int64> sequenzen;
    bool doppelteSequenz = false;
    int begins = 0, enden = 0;
    juce::String folge;
    for (const auto& text : p0Texte())
    {
        const auto e = juce::JSON::parse (juce::String (text));
        const auto typ = e.getProperty ("type", {}).toString();
        if (! typ.startsWith ("audible_intervention_"))
            continue;
        const auto seq = (juce::int64) e.getProperty ("event_sequence", -1);
        if (! sequenzen.insert (seq).second)
            doppelteSequenz = true;
        folge += typ.substring (21) + "/" + juce::String ((int) seq) + " ";
        if (typ == "audible_intervention_begin")
        {
            ++begins;
            ++beginsJeId[e.getProperty ("intervention_id", {}).toString()];
        }
        else if (typ == "audible_intervention_end")
            ++enden;
    }
    int maxBegins = 0;
    for (const auto& kv : beginsJeId)
        maxBegins = std::max (maxBegins, kv.second);

    pruefe (begins >= 1 && enden >= 1,
            "WN-02/WA-01: Begin UND End sind wirklich beim Server angekommen - "
            "gemessen am Draht, nicht am Testhaken",
            juce::String (begins) + " Begin, " + juce::String (enden) + " End");
    pruefe (maxBegins == 1,
            "WN-02/N-27: das auf G+1 zugestellte Begin gilt als auf G+1 zugestellt - "
            "das `end` braucht KEIN Replay, und keine `intervention_id` reist zweimal",
            juce::String (maxBegins) + " Begin je intervention_id");
    pruefe (! doppelteSequenz,
            "WN-02/N-27: und keine Sequenznummer reist doppelt - genau daran setzt "
            "`sequenz_annehmen` die Sitzung sonst `unknown`",
            folge);
    pruefe (enden == 1,
            "WN-02: das `end` reist genau einmal", folge);

    s.p->senderAnhaltenFuerTest (false);
    s.p->v3StopFuerTest();
    server->stoppen();
}

/// WN-03/N-05/M-58 · Die lokale Nachlauffrist ist ein MAXIMUM ohne Deckel.
///
/// 🔑 NAK-180 Nacharbeit 2 (WN-03): `nachlaufFristSetzen` deckelte auf 3600
/// Sekunden und ueberschrieb eine noch offene laengere Frist mit einer
/// kuerzeren. Der Broker haelt in derselben Lage das MAXIMUM
/// (`tail_samples_offen.max(tail_samples)`) und zaehlt ungekappt herunter. In
/// beiden Faellen verbrauchte das Plugin sein einmaliges `false`, waehrend
/// `tail_samples_offen > 0` galt - E4 verwirft es, niemand wiederholt es, und
/// die Sitzung bleibt gesperrt.
///
/// Rotbeweis: Deckel und bedingungsloses `store` zurueckdrehen.
void wn03NachlaufFristIstEinMaximum()
{
    abschnitt ("NAK-180 WN-03/N-05/M-58  nachlauffrist_ist_ein_maximum_ohne_deckel");
    Pruefstand s;

    auto jetztNs = []
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds> (
                   std::chrono::steady_clock::now().time_since_epoch()).count();
    };
    constexpr std::int64_t kStundeNs = 3600LL * 1000000000LL;

    // (1) Ein Tail JENSEITS der alten Stunde wird nicht gekappt. 7200 s bei
    //     48 kHz sind 345 600 000 Samples.
    s.p->nachlaufFristSetzenFuerTest (7200ull * 48000ull);
    const auto lang = s.p->nachlaufFristNsFuerTest();
    // Gemessen wird der ANGEFORDERTE Nachlauf, nicht "mehr als eine Stunde":
    // der alte Deckel lieferte exakt 3600 s plus Marge und haette einen
    // Vergleich gegen eine Stunde bestanden.
    pruefe (lang - jetztNs() > 7000LL * 1000000000LL,
            "WN-03: ein Nachlauf von zwei Stunden reist ungekuerzt in die Frist - "
            "kein 3600-s-Deckel; der Broker zaehlt `tail_samples_offen` ungekappt "
            "herunter",
            juce::String ((double) (lang - jetztNs()) / 1e9, 1) + " s Restfrist");

    // (2) Ein spaeteres, KUERZERES `end` verkuerzt die offene Frist nicht.
    s.p->nachlaufFristSetzenFuerTest (4800);        // 0,1 s bei 48 kHz
    pruefe (s.p->nachlaufFristNsFuerTest() == lang && ! s.p->nachlaufAbgelaufenFuerTest(),
            "WN-03: ein kuerzeres `end` danach verkuerzt sie NICHT - die lokale "
            "Frist ist nie kuerzer als die des Brokers, der das Maximum haelt",
            juce::String ((double) (s.p->nachlaufFristNsFuerTest() - jetztNs()) / 1e9, 1)
                + " s Restfrist");

    // (3) Der Zahlenrand saettigt, statt zu ueberlaufen. `tailSamples` kann bis
    //     an den u64-Rand gehen (N-18); die Saettigung liegt jetzt DORT und
    //     nicht an einer erfundenen Stunde.
    s.p->nachlaufFristSetzenFuerTest (std::numeric_limits<std::uint64_t>::max());
    const auto rand = s.p->nachlaufFristNsFuerTest();
    pruefe (rand - jetztNs() > 24LL * kStundeNs && rand > 0
                && ! s.p->nachlaufAbgelaufenFuerTest(),
            "WN-03: `tailSamples` am u64-Rand saettigt in `int64`-Nanosekunden, "
            "statt zu ueberlaufen - eine negative Frist waere sofort abgelaufen",
            juce::String ((double) rand / 1e9, 0) + " s seit Systemstart");

    // (4) Und die WIRKUNG: solange die laengere Frist steht, schliesst der
    //     Bericht nicht. Ohne die Regel stuende hier die kurze Frist, der
    //     Bericht schloesse nach gut einer Sekunde, und das `false` traefe den
    //     laufenden Nachlauf des Brokers.
    Pruefstand t;
    t.p->markierungEinreichen (t.auftrag);
    t.bloecke (40);
    pruefe (t.p->markierungHoerbar(), "WN-03: der Marker klingt");
    t.p->v3LinkFuerTest (true);
    const auto g = t.p->wireGenerationFuerTest();
    pruefe (t.p->berichtOffenFuerTest() == g, "WN-03: der Bericht ist offen",
            juce::String ((int) g));
    t.p->markierungAus();
    t.bloecke (140);
    (void) t.ernte();                 // das `end` geht auf den Draht

    // Ein LANGES `end` (2 h), danach ein kurzes - genau die Reihenfolge, die
    // die Frist verlor.
    t.p->nachlaufFristSetzenFuerTest (7200ull * 48000ull);
    t.p->nachlaufFristSetzenFuerTest (4800);
    const auto endeDerGeduld = std::chrono::steady_clock::now()
                             + std::chrono::milliseconds (3000);
    while (t.p->berichtOffenFuerTest() != 0
           && std::chrono::steady_clock::now() < endeDerGeduld)
    {
        t.bloecke (2);
        t.p->interventionenSendenFuerTest();
    }
    pruefe (t.p->berichtOffenFuerTest() == g,
            "WN-03/M-58: der Bericht bleibt ueber die kurze Frist hinaus offen - das "
            "einmalige `false` reist nicht in einen laufenden Nachlauf",
            juce::String ((int) t.p->berichtOffenFuerTest()));
}

/// N-37 · Die zwei Generationszustaende DES PROZESSORS - nicht ein isoliertes
/// Atomic daneben (NAK-180 Nacharbeit 1, EP-15).
///
/// Gemessen wird an `berichtOffen` und `replayFaellig` selbst: sie tragen die
/// Generation ihres Links, der negative Callback loescht per CAS auf SEINE
/// sterbende Generation, und ein ueberholter Callback nimmt die Wirkung des
/// naechsten Links nicht mit.
void n37ProzessorZustaendeTragenIhreGeneration()
{
    abschnitt ("NAK-180 N-37  prozessorzustaende_tragen_ihre_generation");
    Pruefstand s;

    // Ein hoerbarer Marker: der Aufbau urteilt NICHT neutral und setzt beide
    // Zustaende.
    s.p->markierungEinreichen (s.auftrag);
    s.bloecke (40);
    pruefe (s.p->markierungHoerbar(), "N-37: der Marker klingt", {});

    s.p->v3LinkFuerTest (true);
    const auto g1 = s.p->wireGenerationFuerTest();
    pruefe (s.p->berichtOffenFuerTest() == g1 && s.p->replayFaelligFuerTest() == g1,
            "N-37: berichtOffen und replayFaellig tragen die Generation IHRES Links",
            juce::String ((int) s.p->berichtOffenFuerTest()) + "/"
                + juce::String ((int) s.p->replayFaelligFuerTest())
                + " bei G" + juce::String ((int) g1));

    // Der negative Callback loescht per CAS auf seine sterbende Generation.
    s.p->v3LinkFuerTest (false);
    pruefe (s.p->berichtOffenFuerTest() == 0 && s.p->replayFaelligFuerTest() == 0,
            "N-37: der Ende-Callback raeumt SEINE Generation auf");

    // Der naechste Link urteilt frisch - und seine Wirkung traegt G2.
    s.p->v3LinkFuerTest (true);
    const auto g2 = s.p->wireGenerationFuerTest();
    pruefe (g2 == g1 + 1 && s.p->berichtOffenFuerTest() == g2
                && s.p->replayFaelligFuerTest() == g2,
            "N-37: und der naechste Link schreibt SEINE Generation",
            juce::String ((int) g1) + " -> " + juce::String ((int) g2));

    // 🔑 Die Zusage: ein Ende-Callback, dessen sterbende Generation ALT ist,
    // nimmt die Wirkung von G2 NICHT mit. Der Client hinterlegt die Zahl beim
    // Statuswechsel; ein zweiter `v3LinkFuerTest(false)` ohne vorherigen
    // Aufbau findet deshalb `0` - und `0` loescht nichts.
    s.p->v3LinkFuerTest (false);
    const auto nachErstem = s.p->berichtOffenFuerTest();
    s.p->v3LinkFuerTest (false);            // ein zweiter, verspaeteter Ende-Callback
    pruefe (nachErstem == 0 && s.p->berichtOffenFuerTest() == 0,
            "N-37: ein zweiter Ende-Callback ohne gemeldeten Link loescht nichts - "
            "die sterbende Generation ist verbraucht, nicht die aktuelle");

    // Und ein Aufbau danach steht wieder allein da.
    s.p->v3LinkFuerTest (true);
    const auto g3 = s.p->wireGenerationFuerTest();
    pruefe (s.p->berichtOffenFuerTest() == g3,
            "N-37: der frische Link schreibt ungestoert",
            juce::String ((int) s.p->berichtOffenFuerTest()));

    // 🔑 WN-04/EP-04/R13: der UEBERHOLTE positive Callback schreibt zuletzt -
    // und darf die Wirkung des neueren Links nicht mitnehmen.
    //
    // 🔑 Nacharbeit 2 (WN-04): festgehalten wird der ECHTE Callback, VOR
    // seinem CAS. Bis hierher haengte sich der Fall HINTER
    // `v3ControlLink(true)` und schrieb ueber `berichtOffenFuerTestSetzen` -
    // eine zweite Zeile neben der Produktzeile, deren echter Schreibzugriff
    // laengst gelaufen war. `gAlt` bekam seinen Wert zudem erst nach der
    // Rueckkehr des Callbacks und war im Callback stets 0: der Rotbeweis fiel
    // mit `[0 (G4 gegen G5)]`, also aus dem falschen Grund, und eine Rueckkehr
    // zu blinden `store`s waere gruen geblieben.
    //
    // Beide Aufbauten urteilen NICHT neutral (der Marker klingt), damit sie
    // denselben Zustand beschreiben; sonst kollidierten sie gar nicht.
    {
        // 🔑 Nacharbeit 3 (WA-02/N-37): der Sender ist der DRITTE Teilnehmer.
        // Die Schranken unten ordnen die beiden Callbacks; der Analyseworker
        // lief bis hierher weiter und konnte `replayFaellig` regulaer per CAS
        // verbrauchen (`PluginProcessor.cpp:1585-1592`). Dann stuende bei den
        // Assertions `0` statt G+1, und der Fall fiele bei KORREKTEM Produkt.
        // Die Pause wird BESTAETIGT, nicht angenommen: ein blosses Bit sagt
        // nichts darueber, ob der Worker gerade schon im Zug steht.
        s.p->senderAnhaltenFuerTest (true);
        pruefe (s.p->warteAufSenderPauseFuerTest(),
                "WA-02: der Sender ist BESTAETIGT angehalten - erst damit ordnen "
                "die Schranken alle drei Teilnehmer, nicht nur die zwei Callbacks");

        std::mutex m;
        std::condition_variable cv;
        bool steht = false, darfSchreiben = false;
        std::uint64_t gAlt = 0;
        std::atomic<int> aufrufe { 0 };

        // Der Haken faengt NUR den ersten Aufbau. Der zweite - G+1 - laeuft
        // ungehindert durch und schreibt, waehrend der erste noch steht.
        s.p->setzeLinkAufbauHakenFuerTest ([&] (std::uint64_t g)
        {
            if (aufrufe.fetch_add (1) != 0)
                return;
            {
                std::lock_guard<std::mutex> l (m);
                gAlt = g;              // die Generation DIESES Aufbaus (R10)
                steht = true;
            }
            cv.notify_all();
            std::unique_lock<std::mutex> l (m);
            cv.wait (l, [&] { return darfSchreiben; });
            // Beim Verlassen des Hakens folgt der ECHTE Schreibzugriff des
            // Produkts - mit SEINER, inzwischen alten Generation.
        });

        std::thread haltend ([&] { s.p->v3LinkFuerTest (true); });
        {
            std::unique_lock<std::mutex> l (m);
            cv.wait (l, [&] { return steht; });
        }

        // Der naechste Link baut vollstaendig auf und schreibt seine Zahl -
        // waehrend der alte Callback noch VOR seinem CAS steht.
        s.p->v3LinkFuerTest (true);
        const auto gNeu = s.p->wireGenerationFuerTest();
        pruefe (gNeu > gAlt && s.p->berichtOffenFuerTest() == gNeu
                    && s.p->replayFaelligFuerTest() == gNeu,
                "WN-04: G+1 hat geschrieben, waehrend der alte Callback vor seinem "
                "CAS steht - das ist die Lage, die der Test braucht",
                "G" + juce::String ((int) gAlt) + " haelt, G"
                    + juce::String ((int) gNeu) + " schreibt");

        {
            std::lock_guard<std::mutex> l (m);
            darfSchreiben = true;
        }
        cv.notify_all();
        haltend.join();
        s.p->setzeLinkAufbauHakenFuerTest ({});

        pruefe (s.p->berichtOffenFuerTest() == gNeu
                    && s.p->replayFaelligFuerTest() == gNeu,
                "WN-04/N-37: cas_statt_store - der ueberholte Callback nimmt die "
                "Wirkung des neueren Links NICHT mit; ein blindes `store` haette "
                "beide Zustaende durch seine alte Zahl ersetzt, und Bericht wie "
                "Replay von G+1 waeren weg",
                "Bericht " + juce::String ((int) s.p->berichtOffenFuerTest())
                    + ", Replay " + juce::String ((int) s.p->replayFaelligFuerTest())
                    + " (G" + juce::String ((int) gAlt) + " gegen G"
                    + juce::String ((int) gNeu) + ")");

        s.p->senderAnhaltenFuerTest (false);   // erst NACH den Assertions
    }

    // 🔑 WN-05/EP-15/N-37 Fall 2: der ALTE negative Callback ueberlappt einen
    // NEUEREN Aufbau.
    //
    // 🔑 Nacharbeit 2 (WN-05): bis hierher liefen zwei `v3LinkFuerTest(false)`
    // NACHEINANDER. Der erste beendete seine Generation synchron, der als
    // verspaetet bezeichnete zweite fand `0` - und `0` loescht nichts. Kein
    // alter negativer Callback ueberlappte je einen neueren Aufbau, und eine
    // Rueckkehr zu `wireGenerationJetzt()` waere gruen geblieben. Der Haken
    // haelt den Callback jetzt VOR seinen Loeschungen fest.
    {
        // 🔑 Nacharbeit 3 (WA-02/N-37): dieselbe Lage, derselbe dritte
        // Teilnehmer - die Pause gilt hier bis nach den Assertions.
        s.p->senderAnhaltenFuerTest (true);
        pruefe (s.p->warteAufSenderPauseFuerTest(),
                "WA-02: auch fuer den WN-05-Block ist der Sender BESTAETIGT "
                "angehalten");

        s.p->v3LinkFuerTest (true);            // ein lebender Link zum Sterben
        std::mutex m;
        std::condition_variable cv;
        bool steht = false, darfLoeschen = false;
        std::uint64_t sterbend = 0;

        s.p->setzeLinkEndeHakenFuerTest ([&] (std::uint64_t g)
        {
            {
                std::lock_guard<std::mutex> l (m);
                sterbend = g;
                steht = true;
            }
            cv.notify_all();
            std::unique_lock<std::mutex> l (m);
            cv.wait (l, [&] { return darfLoeschen; });
        });

        std::thread sterbender ([&] { s.p->v3LinkFuerTest (false); });
        {
            std::unique_lock<std::mutex> l (m);
            cv.wait (l, [&] { return steht; });
        }

        // G+1 baut auf und schreibt - WAEHREND der alte Ende-Callback vor
        // seinen Loeschungen steht.
        s.p->v3LinkFuerTest (true);
        const auto gNeu = s.p->wireGenerationFuerTest();
        pruefe (gNeu > sterbend && s.p->berichtOffenFuerTest() == gNeu
                    && s.p->replayFaelligFuerTest() == gNeu,
                "WN-05: G+1 steht vollstaendig, waehrend der alte negative Callback "
                "noch vor seinen Loeschungen wartet",
                "sterbend G" + juce::String ((int) sterbend) + ", neu G"
                    + juce::String ((int) gNeu));

        {
            std::lock_guard<std::mutex> l (m);
            darfLoeschen = true;
        }
        cv.notify_all();
        sterbender.join();
        s.p->setzeLinkEndeHakenFuerTest ({});

        pruefe (s.p->berichtOffenFuerTest() == gNeu
                    && s.p->replayFaelligFuerTest() == gNeu,
                "WN-05/N-37 Fall 2: der verspaetete negative Callback loescht NUR "
                "seine sterbende Generation - mit `wireGenerationJetzt()` haette er "
                "genau die Zustaende des NEUEN Links mitgenommen",
                "Bericht " + juce::String ((int) s.p->berichtOffenFuerTest())
                    + ", Replay " + juce::String ((int) s.p->replayFaelligFuerTest()));

        s.p->senderAnhaltenFuerTest (false);   // erst NACH den Assertions
    }
}

/// N-04 · Ueberlauf NACH dem Resync setzt das Sticky neu.
void n04UeberlaufNachResync()
{
    abschnitt ("NAK-180 N-04  ueberlauf_nach_resync_setzt_neu");
    auto p = mainProzessor();
    p->prepareToPlay (kFs, kBlock);
    p->v3LinkFuerTest (true);
    pruefe (! p->v3StatusFuerTest().interventionStateUnknown,
            "N-04: nach dem Resync ist der Zustand bekannt");
    const int gefuellt = p->interventionsRingFuellenFuerTest();
    pruefe (gefuellt > 0 && p->v3StatusFuerTest().interventionStateUnknown,
            "N-04: ein Ueberlauf DANACH setzt das Sticky neu - der Resync ist "
            "verbraucht und wiederholt sich nicht von selbst",
            juce::String (gefuellt));
    p->interventionsRingLeerenFuerTest();
    p->releaseResources();
}

} // namespace nak180

void c1Sequenzhandschlag()
{
    abschnitt ("C1  erste_intervention_nach_resync_traegt_sequenz_eins");

    const auto datei = findeDatei ("eq-copilot/fixtures/v3/handschlag-v1.json");
    const auto vertrag = juce::JSON::parse (datei.loadFileAsString());
    const auto basis = (juce::int64) vertrag.getProperty ("resync_sequenzbasis", -1);
    const auto erste = (juce::int64) vertrag.getProperty ("erste_intervention_nach_resync", -1);
    pruefe (basis == 0 && erste == 1,
            "C1: der gemeinsame Handschlag steht in fixtures/v3/handschlag-v1.json",
            "Basis " + juce::String (basis) + ", erste " + juce::String (erste));

    auto p = mainProzessorMitBindung();
    p->setPlayConfigDetails (2, 2, kFs, kBlock);
    p->prepareToPlay (kFs, kBlock);
    p->testForciereEchtzeit (true);
    p->setzeEditorOffen (true);
    TestPlayHead kopf;
    p->setPlayHead (&kopf);
    eqcop::hostbruecke::Bruecke bruecke;
    bruecke.verbinde (p.get());

    juce::AudioBuffer<float> puffer (2, kBlock);
    juce::MidiBuffer midi;
    auto block = [&] (double phase0) -> double
    {
        double phase = phase0;
        const double d = 2.0 * juce::MathConstants<double>::pi * 200.0 / kFs;
        for (int i = 0; i < kBlock; ++i)
        {
            const float v = 0.5f * (float) std::sin (phase);
            phase += d;
            puffer.setSample (0, i, v);
            puffer.setSample (1, i, v);
        }
        bruecheBlock (bruecke, kopf, kBlock);
        p->processBlock (puffer, midi);
        kopf.pos += kBlock;
        return phase;
    };

    // 1. Ein erster Eingriff VOR dem Resync treibt den Zaehler hoch. Ohne ihn
    //    waere die 1 danach zufaellig richtig.
    double ph = 0.0;
    MarkierungsWunsch w;
    w.modus = MarkierungsModus::solo;
    w.istResonanz = false;
    w.fVon = 120.0; w.fBis = 300.0; w.fSchwerpunkt = 200.0;
    w.fs = kFs;
    MarkierungsAuftrag auftrag;
    baueMarkierungsAuftrag (auftrag, w);

    p->markierungEinreichen (auftrag);
    for (int i = 0; i < 40; ++i) ph = block (ph);
    p->markierungAus();
    for (int i = 0; i < 80; ++i) ph = block (ph);
    const int vorher = p->interventionsRingLeerenFuerTest();
    pruefe (vorher >= 2,
            "C1: vor dem Resync sind Begin und Ende wirklich entstanden",
            juce::String (vorher));

    // 2. Der bestaetigte Resync - ueber den ECHTEN Weg des ControlClients.
    //
    // 🔑 NAK-180 Nacharbeit 1 (EP-18/R3b): bis hier endete der Handschlag
    // beim Link-Callback. Der Callback bildet aber keinen Heartbeat, reiht
    // nichts ein und stellt nichts zu - gemessen war damit ein lokales Flag,
    // nicht der Handschlag. Jetzt laeuft die ganze Kette: Aufbauzug und
    // Callback (`v3LinkFuerTest`), DER Heartbeat-Schritt der Sendeschleife
    // (`v3HeartbeatSchrittFuerTest`, dieselbe Methode wie im Produkt) und der
    // Wire-Commit (`zustelleAllesFuerTest`).
    pruefe (! p->markierungHoerbar(), "C1: der Marker ist vor dem Resync still");
    p->v3LinkFuerTest (true);
    std::string handschlag;
    const bool heartbeatEingereiht = p->v3HeartbeatSchrittFuerTest (handschlag, 1);
    pruefe (heartbeatEingereiht,
            "C1/EP-18: der Aufbau-Heartbeat ist eingereiht - der echte Schritt der "
            "Sendeschleife, nicht der Callback allein");
    pruefe (handschlag.find ("\"intervention_state_unknown\":false") != std::string::npos,
            "C1/EP-18: und er traegt die ausdrueckliche Neutralitaetsaussage - genau "
            "das loest beim Broker `resync_bestaetigen`",
            juce::String (handschlag.substr (0, 90)));
    pruefe (p->zustelleAllesFuerTest() >= 1,
            "C1/EP-18: und er erreicht den DRAHT - erst der Wire-Commit ist "
            "'gesendet' (R7)");

    // 3. Der ERSTE Eingriff danach. Gemessen wird die Zahl, die WIRKLICH auf
    //    die Leitung geht — der gebaute Wiretext, nicht ein lokales Flag.
    p->markierungEinreichen (auftrag);
    juce::String wire;
    for (int i = 0; i < 60 && wire.isEmpty(); ++i)
    {
        ph = block (ph);
        // 🔑 EP-13/R7: erst der Wire-Commit macht aus einem eingereihten
        // Eintrag ein gesendetes Ereignis; ohne Pipe faehrt ihn das Bein.
        (void) p->zustelleAllesFuerTest();
        wire = p->naechstesInterventionsJsonFuerTest();
    }
    pruefe (wire.isNotEmpty(), "C1: der erste Eingriff nach dem Resync steht auf der Leitung");
    const auto nachricht = juce::JSON::parse (wire);
    const auto sequenz = (juce::int64) nachricht.getProperty ("event_sequence", -1);
    pruefe (sequenz == erste,
            "C1: erste_intervention_nach_resync_traegt_sequenz_eins - genau die "
            "Zahl, die der Broker nach `resync_bestaetigen(link, 0)` erwartet",
            "gesendet " + juce::String (sequenz) + ", erwartet " + juce::String (erste));
    pruefe (nachricht.getProperty ("type", {}).toString() == "audible_intervention_begin",
            "C1: und es ist wirklich ein Begin, nicht irgendeine Nachricht");

    p->markierungAus();
    for (int i = 0; i < 80; ++i) ph = block (ph);
    bruecke.verbinde (nullptr);
    p->setPlayHead (nullptr);
    p->releaseResources();
}

} // namespace

int main()
{
    std::cout << "== Nakama SONDE-013 - manuelle Passage als Projektintent (§33.5) =="
              << std::endl;
    m25();
    m69Roundtrip();
    m69Unbekannt();
    raenderApi();
    raenderLeser();
    zahlenraender();
    r03Passagenfenster();
    r01Resync();
    r06Vergleichspegel();
    // Nacharbeit 3 nach Wiederpruefung 2 (2026-09-04).
    c2Markierungsepoche();
    c3ZweiPassagen();
    c4PegelNurImFenster();
    c7EngineZug();
    c8ZweiVersucheNacheinander();
    c5ZaehlerImWirezustand();
    c1Sequenzhandschlag();
    // NAK-180 Messlauf (2026-09-05)
    nak180::mitschnittIstVollstaendig();
    nak180::n04UeberlaufNachResync();
    nak180::n05ReplayBeiHoerbaremMarker();
    nak180::n08BacklogOhneReplay();
    nak180::n10PrepareToPlayPaar();
    nak180::n28BeginNachSendefehler();
    nak180::n29OrdnungsregelUndZug();
    nak180::n35SendezugGegenAufbauzug();
    nak180::n36EndeUeberlebtDenLinkwechsel();
    nak180::n05NachlaufWirdAbgewartet();
    // NAK-180 Etappe 2, Nacharbeit 2 (2026-09-05): WN-01 bis WN-03.
    nak180::wn01WartendesEndeVerbietetNeutralitaet();
    nak180::wn02ZustellungMeldetDieWireGeneration();
    nak180::wn03NachlaufFristIstEinMaximum();
    nak180::n37ProzessorZustaendeTragenIhreGeneration();
    nak180::n11KeineBehauptungOhneMain();
    nak180::n12AussageKommtZurueck();
    std::cout << std::endl << bestanden << " bestanden, " << fehler << " gescheitert"
              << std::endl;
    return fehler == 0 ? 0 : 1;
}
