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

#include "NakamaState.h"
#include "PluginProcessor.h"

#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

using namespace eqcop;
namespace state = nakama::state;

namespace
{
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
    std::cout << std::endl << bestanden << " bestanden, " << fehler << " gescheitert"
              << std::endl;
    return fehler == 0 ? 0 : 1;
}
