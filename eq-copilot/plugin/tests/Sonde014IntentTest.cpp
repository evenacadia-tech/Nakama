/*  EqCopSonde014IntentTest — der musikalische Intent im Main-State
    (SONDE-014 Etappe A, Matrixzeilen M-01 bis M-09 und M-11).

    ── WAS DIESES BEIN MISST UND WARUM ES EIN EIGENES IST ───────────────────

    Entwurf §37.1 gibt dem `SourceIntent` sechs Teile; Antwort U22 vom
    06.09.2026 legt die Produktzusage darueber: eine Quelle kann GENAU FUENF
    musikalische Rollen bekommen. Damit ist der Intent ein neues persistentes
    Feld im Host-State — die teuerste Stelle im ganzen Plugin — UND ein
    Regelwerk mit drei Kanten, die im Nachhinein teuer sind:

      * die Abbildung Rolle -> §37.1-Belegung muss TOTAL und INJEKTIV sein,
        sonst gibt es zwei Wahrheiten (§33.5);
      * die Konfliktregeln sind eine GEORDNETE LISTE, kein Score — sonst
        schlaegt eine Summe niedriger Stufen eine hohe;
      * ein gerichteter Zyklus darf die Persistenz NIE unmarkiert erreichen
        (§37.4: "Zyklische Entmaskierungsprioritaeten koennen nicht angewendet
        werden").

    ── ZWEI EBENEN, UND DER ROTBEWEIS FAELLT AN DER ZWEITEN ─────────────────

    Risiko R1 des Bauplans, woertlich aus der Wiederpruefung 1 der Etappe 2
    von SONDE-013: "die Runde 1 hat Funktionen gebaut und Tests geschrieben,
    die das Modul direkt aufrufen — der Produktpfad war an vielen Stellen
    nicht geschlossen." Dieses Bein faehrt deshalb jede Zusage ZWEIMAL:

      1. MODUL   — die reinen Funktionen aus `NakamaState.cpp`;
      2. VERDRAHTUNG — derselbe Handgriff ueber den ECHTEN
         `EqCopilotProcessor`, also ueber `setzeQuellenrolle` und Geschwister,
         mit `setStateInformation`/`getStateInformation` als Rand.

    ── UND JEDE GRENZE VON BEIDEN SEITEN ────────────────────────────────────

    Eine Produkt-API, die einen Stand schreibt, den der eigene Leser als
    read-only zurueckgibt, ist stiller Datenverlust beim naechsten Oeffnen
    (der Fehler vom 23.08. am `parameters`-Feld). Der Abschnitt "Die Raender"
    faehrt deshalb jede Grenze von BEIDEN Seiten: die API lehnt ab, UND ein
    von Hand gebauter Baum mit demselben Wert wird read-only.

    Engines liegen in diesem Bein auf dem HEAP (`std::unique_ptr`), nie im
    Funktionsrahmen — NAK-175, MSVC-Standardstack 1 MiB.
*/

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>

#include "NakamaState.h"
#include "PluginProcessor.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <vector>

using namespace eqcop;
namespace state = nakama::state;

namespace
{
int bestanden = 0;
int fehler    = 0;

/// NAK-283 Etappe 2: `detail` ist neu und optional - der gemessene Wert neben
/// der Behauptung, wie in B14. Bestehende Aufrufe bleiben unveraendert.
void pruefe (bool ok, const juce::String& was, const juce::String& detail = {})
{
    std::cout << (ok ? "  ok      " : "  FEHLER  ") << was;
    if (detail.isNotEmpty())
        std::cout << "  [" << detail << "]";
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

const auto kQuelleA  = hex32 (0xA1);
const auto kQuelleB  = hex32 (0xB2);
const auto kQuelleC  = hex32 (0xC3);
const auto kPassage1 = hex32 (0x0501);
const auto kPassage2 = hex32 (0x0502);

/** Ein gueltiger Schema-2-Baum der Klasse `main`, damit der Intent einen
    Platz hat. */
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

/** Setzt eine MainProject-Eigenschaft von Hand — so entstehen Staende, die
    die Produkt-API gar nicht erzeugen kann. Genau die misst der Leser. */
juce::MemoryBlock baumMitEigenschaft (const juce::Identifier& name, const juce::var& wert,
                                      bool mitRevision = true)
{
    auto v = mainBaum();
    auto mp = v.getChildWithName ("MainProject");
    mp.setProperty (name, wert, nullptr);
    if (mitRevision)
        mp.setProperty ("intent_revision_v1", juce::var ((juce::int64) 1), nullptr);
    return alsBlock (v);
}

juce::Array<juce::var> liste (std::initializer_list<juce::var> werte)
{
    juce::Array<juce::var> f;
    for (const auto& w : werte) f.add (w);
    return f;
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

/** Laedt Bytes im Modul, ohne Prozessor. */
state::LadeErgebnis lade (const juce::MemoryBlock& b, state::Zustand& aus)
{
    return state::lade (b.getData(), b.getSize(), state::Bundle::eqcp(), aus);
}

/** Die fuenf Rollen als Feld — EINE Quelle fuer jede Schleife dieses Beins.
    Zwei handgepflegte Kopien derselben Liste sind in diesem Projekt schon
    auseinandergelaufen. */
constexpr state::Rolle kRollen[] = {
    state::Rolle::fuehrt, state::Rolle::traegt, state::Rolle::begleitet,
    state::Rolle::geschuetzt, state::Rolle::verschmolzen
};

// ═════════════════════════════════════════════════════════════════════════
// M-01 · rollenmenge_ist_geschlossen_und_fuenf
// ═════════════════════════════════════════════════════════════════════════
//
// Die Zusage hat zwei Haelften, und beide fallen einzeln: die MENGE hat genau
// fuenf Werte, und ein sechster Wert wird ABGEWIESEN statt auf einen
// bekannten Zweig abgebildet. Die zweite Haelfte ist die scharfe: ein Leser
// mit `else { aus = Rolle::traegt; return true; }` erfuellte die erste und
// verletzte die Zusage vollstaendig.
void m01()
{
    abschnitt ("M-01  rollenmenge_ist_geschlossen_und_fuenf");

    pruefe (state::rollenAnzahl == 5, "die Produktzusage aus U22 ist die Zahl 5");
    pruefe ((int) (sizeof (kRollen) / sizeof (kRollen[0])) == state::rollenAnzahl,
            "das Rollenfeld dieses Beins hat genau so viele Eintraege");

    // Jedes Wort ist eigen, keines leer.
    std::set<std::string> woerter;
    for (const auto r : kRollen)
    {
        const std::string w = state::wort (r);
        pruefe (! w.empty(), "Rolle traegt ein nicht leeres Wort: " + juce::String (w));
        woerter.insert (w);
    }
    pruefe ((int) woerter.size() == state::rollenAnzahl,
            "die fuenf Rollenwoerter sind paarweise verschieden");

    // Roundtrip Wort -> Rolle -> Wort.
    for (const auto r : kRollen)
    {
        state::Rolle zurueck {};
        const bool ok = state::rolleAusWort (state::wort (r), zurueck) && zurueck == r;
        pruefe (ok, juce::String ("Wort und Rolle sind umkehrbar: ") + state::wort (r));
    }

    // Der sechste Wert. Keine stille Naeherung auf einen bekannten Zweig.
    for (const auto* fremd : { "impuls", "raum", "lead", "Fuehrt", "", "fuehrt " })
    {
        state::Rolle aus = state::Rolle::traegt;
        pruefe (! state::rolleAusWort (fremd, aus),
                juce::String ("ein sechster Rollenwert wird abgewiesen: '") + fremd + "'");
    }

    // Und derselbe Riegel im LESER: ein von Hand gebauter Baum mit einem
    // sechsten Rollenwort wird read-only, nicht still korrigiert.
    const auto bytes = baumMitEigenschaft ("source_intents_v1",
        juce::var (liste ({ kQuelleA, juce::String(), "impuls", juce::var ((juce::int64) 1),
                            "user", juce::var (1.0) })));
    state::Zustand z;
    const auto erg = lade (bytes, z);
    pruefe (erg == state::LadeErgebnis::nurLesen && z.nurLesen,
            "der Leser weist einen sechsten Rollenwert als read-only ab");
    pruefe (z.grund.contains ("unknown role"),
            "und nennt den Grund: " + z.grund);
}

// ═════════════════════════════════════════════════════════════════════════
// M-02 · rolle_und_belegung_sind_umkehrbar
//        zwei_belegungen_lesen_nie_dieselbe_rolle_zurueck
// ═════════════════════════════════════════════════════════════════════════
//
// E-01a hat den Vorschlagstext der Etappe 1 korrigiert: fuer `geschuetzt` und
// `verschmolzen` ist `prominence` NICHT "unveraendert", sondern der feste
// Neutralwert `middle` mit leerem Funktionstag und einem Veto-Kennzeichen ALS
// TEIL der Belegung. Nur so ist die Abbildung total UND injektiv.
void m02()
{
    abschnitt ("M-02  rolle_und_belegung_sind_umkehrbar");

    // Die abgeleitete Belegung, Wert fuer Wert aus dem Entscheid.
    struct Erwartung { state::Rolle r; state::Prominenz p; state::Funktionstag t; state::Veto v; };
    const Erwartung tafel[] = {
        { state::Rolle::fuehrt,       state::Prominenz::foreground, state::Funktionstag::lead,       state::Veto::keins },
        { state::Rolle::traegt,       state::Prominenz::middle,     state::Funktionstag::foundation, state::Veto::keins },
        { state::Rolle::begleitet,    state::Prominenz::background, state::Funktionstag::texture,    state::Veto::keins },
        { state::Rolle::geschuetzt,   state::Prominenz::middle,     state::Funktionstag::keiner,     state::Veto::schutz },
        { state::Rolle::verschmolzen, state::Prominenz::middle,     state::Funktionstag::keiner,     state::Veto::verschmolzen },
    };
    for (const auto& e : tafel)
    {
        const auto b = state::belegung (e.r);
        pruefe (b.prominenz == e.p && b.tag == e.t && b.veto == e.v,
                juce::String ("Belegung von ") + state::wort (e.r) + " ist ("
                    + state::wort (b.prominenz) + ", '" + state::wort (b.tag) + "', '"
                    + state::wort (b.veto) + "')");
    }

    // TOTAL: jede der fuenf Rollen kommt zurueck.
    for (const auto r : kRollen)
    {
        state::Rolle zurueck {};
        pruefe (state::rolleAus (state::belegung (r), zurueck) && zurueck == r,
                juce::String ("Roundtrip Rolle -> Belegung -> Rolle: ") + state::wort (r));
    }

    // INJEKTIV: keine zwei Rollen teilen sich eine Belegung. Genau das war die
    // Luecke des Vorschlagstexts — `geschuetzt` und `verschmolzen` lagen dort
    // beide auf "prominence unveraendert".
    abschnitt ("M-02  zwei_belegungen_lesen_nie_dieselbe_rolle_zurueck");
    int paare = 0, kollisionen = 0;
    for (const auto a : kRollen)
        for (const auto b : kRollen)
        {
            if (a == b) continue;
            ++paare;
            if (state::belegung (a) == state::belegung (b)) ++kollisionen;
        }
    pruefe (paare == 20, "alle zwanzig geordneten Paare der fuenf Rollen gefahren");
    pruefe (kollisionen == 0, "keine zwei verschiedenen Rollen teilen sich eine Belegung");

    // Und die Gegenprobe: eine Belegung, die es nicht gibt, gehoert zu KEINER
    // Rolle. Ohne sie saehe ein `rolleAus`, das immer `traegt` zurueckgibt,
    // wie eine erfuellte Zusage aus.
    state::Rolle egal {};
    pruefe (! state::rolleAus ({ state::Prominenz::foreground, state::Funktionstag::texture,
                                 state::Veto::schutz }, egal),
            "eine fremde Belegung gehoert zu keiner Rolle");
    pruefe (! state::rolleAus ({ state::Prominenz::background, state::Funktionstag::keiner,
                                 state::Veto::keins }, egal),
            "und eine zweite fremde ebenfalls nicht");
}

// ═════════════════════════════════════════════════════════════════════════
// M-03 · attack_schutz_ist_unabhaengig_von_der_rolle
//        rollenwechsel_loescht_keinen_schutzbereich
// ═════════════════════════════════════════════════════════════════════════
//
// U22 Folge Punkt 2: Impuls ist KEINE Rolle. Kurze Anschlaege werden ueber
// die Schutzangabe an der Quelle geschuetzt. E-01a macht daraus eine
// pruefbare Zusage: der Schutz ist ORTHOGONAL zur Rolle.
void m03()
{
    abschnitt ("M-03  attack_schutz_ist_unabhaengig_von_der_rolle");

    // MODUL: Schutz ohne jede Rolle.
    {
        state::Zustand z = state::frisch (hex32 (1));
        z.common.klasse = state::Klasse::main;
        bool v = false; juce::String g;
        pruefe (state::setzeSchutzangabe (z, kQuelleA, state::Schutzeigenschaft::attack,
                                          -1, -1, v, g) && v,
                "eine Schutzangabe entsteht OHNE dass die Quelle eine Rolle traegt");
        pruefe (state::findeIntent (z, kQuelleA, {}) == nullptr,
                "und sie erzeugt keine Rolle als Nebenwirkung");
    }

    // VERDRAHTUNG: derselbe Handgriff ueber den echten Prozessor, dazu die
    // Kombination, die U22 ausdruecklich benennt — Rolle `begleitet` MIT
    // geschuetztem Attack.
    {
        auto p = mainProzessor();
        pruefe (p->schuetzeQuelle (kQuelleA, state::Schutzeigenschaft::attack, -1, -1),
                "Produktpfad: Schutz ohne Rolle wird angenommen");
        pruefe (p->intentSchutzangaben().size() == 1 && p->sourceIntents().empty(),
                "der Bestand traegt genau die Schutzangabe und keinen Intent");

        pruefe (p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::begleitet,
                                      state::IntentHerkunft::user, 1.0),
                "und danach die Rolle 'begleitet' an derselben Quelle");
        pruefe (p->intentSchutzangaben().size() == 1,
                "eine Quelle mit Rolle 'begleitet' kann Attack geschuetzt haben");

        abschnitt ("M-03  rollenwechsel_loescht_keinen_schutzbereich");
        // Alle fuenf Rollen nacheinander; nach jedem Wechsel muss der Schutz
        // unveraendert stehen. Ein Wechsel, der ihn loescht, faellt hier.
        for (const auto r : kRollen)
        {
            pruefe (p->setzeQuellenrolle (kQuelleA, {}, r, state::IntentHerkunft::user, 1.0),
                    juce::String ("Rollenwechsel auf ") + state::wort (r));
            const auto schutz = p->intentSchutzangaben();
            pruefe (schutz.size() == 1 && schutz[0].quelleId == kQuelleA
                        && schutz[0].eigenschaft == state::Schutzeigenschaft::attack,
                    juce::String ("der Schutzbereich ueberlebt den Wechsel auf ") + state::wort (r));
        }

        // Und der Gegenpfad: der Schutz laesst sich einzeln aufheben, ohne
        // die Rolle anzufassen.
        pruefe (p->hebeQuellenschutzAuf (kQuelleA, state::Schutzeigenschaft::attack, -1, -1),
                "Schutz laesst sich einzeln aufheben");
        pruefe (p->intentSchutzangaben().empty() && p->sourceIntents().size() == 1,
                "und die Rolle bleibt dabei stehen");
    }

    // Ein Bandintervall am Rand: Band 0 ist gueltig, `bis` ist halboffen und
    // darf 221 sein, ein leeres Intervall nicht.
    {
        auto p = mainProzessor();
        pruefe (p->schuetzeQuelle (kQuelleB, state::Schutzeigenschaft::band, 0,
                                   state::bandAnzahlEvidenzgitter),
                "Bandschutz [0, 221) am vollen Rand wird angenommen");
        pruefe (! p->schuetzeQuelle (kQuelleB, state::Schutzeigenschaft::band, 0,
                                     state::bandAnzahlEvidenzgitter + 1),
                "ein Band ueber dem Gitterrand wird abgewiesen");
        pruefe (! p->schuetzeQuelle (kQuelleB, state::Schutzeigenschaft::band, 5, 5),
                "ein leeres Bandintervall wird abgewiesen");
        pruefe (! p->schuetzeQuelle (kQuelleB, state::Schutzeigenschaft::band, -1, -1),
                "eine Bandeigenschaft ohne Intervall wird abgewiesen");
        pruefe (! p->schuetzeQuelle (kQuelleB, state::Schutzeigenschaft::attack, 0, 4),
                "und eine Nicht-Bandeigenschaft MIT Intervall ebenfalls");
    }
}

// ═════════════════════════════════════════════════════════════════════════
// M-04 · nur_rolle · nur_beziehung · beides_im_widerspruch
// ═════════════════════════════════════════════════════════════════════════
//
// E-02: die Rolle `bewusst verschmolzen` ist ein GLOBALES Veto gegen jede
// Entmaskierungsempfehlung mit Beteiligung dieser Quelle; die paarweise
// gerichtete Beziehung ist das SPEZIFISCHERE Werkzeug und hebt das Veto NUR
// FUER DIESES PAAR auf.
void m04()
{
    abschnitt ("M-04  nur_rolle");
    {
        auto p = mainProzessor();
        pruefe (p->entmaskierungErlaubt (kQuelleA, kQuelleB, {}),
                "ohne jeden Intent ist Entmaskierung erlaubt");
        pruefe (p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::verschmolzen,
                                      state::IntentHerkunft::user, 1.0),
                "Quelle A bekommt die Rolle 'bewusst verschmolzen'");
        pruefe (! p->entmaskierungErlaubt (kQuelleA, kQuelleB, {}),
                "das globale Veto sperrt A gegen B");
        pruefe (! p->entmaskierungErlaubt (kQuelleC, kQuelleA, {}),
                "und ebenso A gegen jede dritte Quelle - es ist GLOBAL");
        pruefe (p->entmaskierungErlaubt (kQuelleB, kQuelleC, {}),
                "ein Paar ohne A bleibt unberuehrt");
    }

    abschnitt ("M-04  nur_beziehung");
    {
        auto p = mainProzessor();
        pruefe (p->setzeQuellenbeziehung (kQuelleA, kQuelleB, state::Beziehungsart::fuehrtVor),
                "eine gerichtete Beziehung 'A fuehrt vor B' ohne jede Rolle");
        pruefe (p->entmaskierungErlaubt (kQuelleA, kQuelleB, {}),
                "ohne Veto aendert sie an der Erlaubnis nichts");
    }

    abschnitt ("M-04  beides_im_widerspruch");
    {
        auto p = mainProzessor();
        pruefe (p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::verschmolzen,
                                      state::IntentHerkunft::user, 1.0),
                "globales Veto an A");
        pruefe (p->setzeQuellenbeziehung (kQuelleA, kQuelleB, state::Beziehungsart::fuehrtVor),
                "und die ausdrueckliche Beziehung 'A fuehrt vor B'");
        pruefe (p->entmaskierungErlaubt (kQuelleA, kQuelleB, {}),
                "das SPEZIFISCHERE gewinnt: fuer dieses Paar ist Entmaskierung erlaubt");
        // Der zweite Rotbeweis dieser Zeile: die Beziehung darf das Veto NICHT
        // fuer ein anderes Paar mit aufheben.
        pruefe (! p->entmaskierungErlaubt (kQuelleA, kQuelleC, {}),
                "fuer das Paar A/C bleibt das Veto stehen");
        pruefe (! p->entmaskierungErlaubt (kQuelleC, kQuelleA, {}),
                "auch in der anderen Leserichtung");

        // `darf_verschmelzen` ist KEIN Aufheber: es sagt dasselbe wie das
        // Veto, nur paarweise. Ein Werkzeug, das beide Richtungen bediente,
        // haette keine Aussage.
        auto q = mainProzessor();
        pruefe (q->setzeQuellenrolle (kQuelleA, {}, state::Rolle::verschmolzen,
                                      state::IntentHerkunft::user, 1.0), "Veto an A");
        pruefe (q->setzeQuellenbeziehung (kQuelleA, kQuelleB,
                                          state::Beziehungsart::darfVerschmelzen),
                "und eine Beziehung 'A/B duerfen verschmelzen'");
        pruefe (! q->entmaskierungErlaubt (kQuelleA, kQuelleB, {}),
                "sie hebt das Veto NICHT auf - sie bestaetigt es");
    }
}

// ═════════════════════════════════════════════════════════════════════════
// M-05 · konfliktregeln_sind_eine_ordnung_kein_score
//        innerhalb_stufe_zwei_gewinnt_das_spezifischere
// ═════════════════════════════════════════════════════════════════════════
void m05()
{
    abschnitt ("M-05  konfliktregeln_sind_eine_ordnung_kein_score");

    constexpr state::Konfliktstufe stufen[] = {
        state::Konfliktstufe::schutzgrenze,
        state::Konfliktstufe::userintent,
        state::Konfliktstufe::passagespezifisch,
        state::Konfliktstufe::vorlage,
        state::Konfliktstufe::vermutung
    };
    pruefe ((int) (sizeof (stufen) / sizeof (stufen[0])) == state::konfliktstufenAnzahl,
            "§37.2 fuehrt genau fuenf Stufen");

    // ALLE ZEHN Paarungen der fuenf Stufen, in beiden Richtungen gefahren.
    int paare = 0;
    for (int i = 0; i < state::konfliktstufenAnzahl; ++i)
        for (int j = i + 1; j < state::konfliktstufenAnzahl; ++j)
        {
            ++paare;
            const state::Anspruch hoch { stufen[i], false };
            const state::Anspruch tief { stufen[j], false };
            const bool ok = state::vergleicheAnsprueche (hoch, tief) < 0
                         && state::vergleicheAnsprueche (tief, hoch) > 0;
            pruefe (ok, juce::String ("Stufe ") + juce::String ((int) stufen[i])
                        + " schlaegt Stufe " + juce::String ((int) stufen[j])
                        + " in beiden Leserichtungen");
        }
    pruefe (paare == 10, "alle zehn Paarungen der fuenf Stufen gefahren");

    // Die eigentliche Zusage: KEIN Score. Eine hohe Stufe wird von keiner
    // SUMME niedrigerer geschlagen. Der Vergleich kennt nur die Stufenzahl,
    // also gibt es strukturell keinen Additionsweg - und genau das wird hier
    // gemessen statt behauptet: hundert Vergleiche einer Stufe-1-Grenze gegen
    // Stufe 5 aendern das Ergebnis nie.
    bool summeSchlaegtNie = true;
    for (int n = 1; n <= 100; ++n)
    {
        const state::Anspruch grenze { state::Konfliktstufe::schutzgrenze, false };
        const state::Anspruch vermutung { state::Konfliktstufe::vermutung, n % 2 == 0 };
        if (state::vergleicheAnsprueche (grenze, vermutung) >= 0)
            summeSchlaegtNie = false;
    }
    pruefe (summeSchlaegtNie,
            "hundert Vermutungen schlagen die Schutzgrenze nie - eine Ordnung, kein Score");

    // Und die Gegenprobe, damit die Ordnung nicht einfach "immer -1" ist.
    pruefe (state::vergleicheAnsprueche ({ state::Konfliktstufe::vermutung, false },
                                         { state::Konfliktstufe::vermutung, false }) == 0,
            "zwei gleiche Anspruechen sind gleichrangig - die Funktion ist nicht konstant");

    abschnitt ("M-05  innerhalb_stufe_zwei_gewinnt_das_spezifischere");
    const state::Anspruch spezifisch { state::Konfliktstufe::userintent, true };
    const state::Anspruch global     { state::Konfliktstufe::userintent, false };
    pruefe (state::vergleicheAnsprueche (spezifisch, global) < 0,
            "auf Stufe 2 gewinnt die paarweise Beziehung vor dem globalen Veto");
    pruefe (state::vergleicheAnsprueche (global, spezifisch) > 0,
            "und umgekehrt ebenso");
    // Aber die Stufe steht ueber der Spezifitaet: ein spezifischer Anspruch
    // der Stufe 3 schlaegt keinen globalen der Stufe 2.
    pruefe (state::vergleicheAnsprueche ({ state::Konfliktstufe::passagespezifisch, true },
                                         global) > 0,
            "Spezifitaet hebt keine Stufe an");

    // §37.2 Stufe 3 im Produktpfad: passagespezifisch vor global.
    auto p = mainProzessor();
    pruefe (p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::traegt,
                                  state::IntentHerkunft::user, 1.0), "globale Rolle 'traegt'");
    pruefe (p->setzeQuellenrolle (kQuelleA, kPassage1, state::Rolle::fuehrt,
                                  state::IntentHerkunft::user, 1.0),
            "und in Passage 1 die Rolle 'fuehrt'");
    state::Rolle wirkt {};
    pruefe (p->wirkendeQuellenrolle (kQuelleA, kPassage1, wirkt) && wirkt == state::Rolle::fuehrt,
            "in Passage 1 wirkt der passagespezifische Wert");
    pruefe (p->wirkendeQuellenrolle (kQuelleA, kPassage2, wirkt) && wirkt == state::Rolle::traegt,
            "in Passage 2 ohne eigenen Wert faellt er auf den globalen zurueck");
    pruefe (p->wirkendeQuellenrolle (kQuelleA, {}, wirkt) && wirkt == state::Rolle::traegt,
            "und global gilt der globale");
    pruefe (! p->wirkendeQuellenrolle (kQuelleB, kPassage1, wirkt),
            "eine Quelle ohne jeden Intent hat keine wirkende Rolle - kein Vorgabewert");
}

// ═════════════════════════════════════════════════════════════════════════
// M-06 · zyklus_wird_beim_speichern_erkannt
//        zyklus_wird_als_gleichrangigkeit_gespeichert
// ═════════════════════════════════════════════════════════════════════════
void m06()
{
    abschnitt ("M-06  zyklus_wird_beim_speichern_erkannt");

    // MODUL: der reine Riegel.
    pruefe (! state::hatZyklus ({}), "eine leere Kantenmenge hat keinen Zyklus");
    pruefe (! state::hatZyklus ({ { kQuelleA, kQuelleB, state::Beziehungsart::fuehrtVor },
                                  { kQuelleB, kQuelleC, state::Beziehungsart::fuehrtVor } }),
            "eine Kette A->B->C hat keinen Zyklus");
    pruefe (state::hatZyklus ({ { kQuelleA, kQuelleB, state::Beziehungsart::fuehrtVor },
                                { kQuelleB, kQuelleC, state::Beziehungsart::fuehrtVor },
                                { kQuelleC, kQuelleA, state::Beziehungsart::fuehrtVor } }),
            "ein Dreierzyklus A->B->C->A wird erkannt");
    pruefe (! state::hatZyklus ({ { kQuelleA, kQuelleB, state::Beziehungsart::fuehrtVor },
                                  { kQuelleB, kQuelleC, state::Beziehungsart::fuehrtVor },
                                  { kQuelleC, kQuelleA, state::Beziehungsart::gleichrangig } }),
            "dieselbe Runde mit einer gleichrangigen Kante ist KEIN Zyklus");
    pruefe (! state::hatZyklus ({ { kQuelleA, kQuelleB, state::Beziehungsart::darfVerschmelzen },
                                  { kQuelleB, kQuelleA, state::Beziehungsart::darfVerschmelzen } }),
            "`darf_verschmelzen` ist keine Prioritaetskante und bildet keinen Zyklus");

    // VERDRAHTUNG: die Pruefung laeuft BEIM SPEICHERN, nicht beim Anwenden.
    auto p = mainProzessor();
    pruefe (p->setzeQuellenbeziehung (kQuelleA, kQuelleB, state::Beziehungsart::fuehrtVor),
            "A fuehrt vor B");
    pruefe (p->setzeQuellenbeziehung (kQuelleB, kQuelleC, state::Beziehungsart::fuehrtVor),
            "B fuehrt vor C");
    const auto revVorher = p->intentBestandRevision();
    pruefe (! p->setzeQuellenbeziehung (kQuelleC, kQuelleA, state::Beziehungsart::fuehrtVor),
            "C fuehrt vor A wird ABGEWIESEN - der Zyklus erreicht die Persistenz nicht");
    pruefe (p->intentBeziehungen().size() == 2,
            "der Bestand traegt weiterhin genau zwei Kanten");
    pruefe (p->intentBestandRevision() == revVorher,
            "und ein abgewiesener Wert hebt die Bestandsrevision nicht");

    // Der Leser haelt denselben Riegel: ein von Hand gebauter Baum mit einem
    // Zyklus wird read-only. §37.4: "Zyklische Entmaskierungsprioritaeten
    // koennen nicht angewendet werden" — ein angenommener Zyklus waere genau
    // das.
    {
        const auto bytes = baumMitEigenschaft ("intent_relations_v1",
            juce::var (liste ({ kQuelleA, kQuelleB, "fuehrt_vor",
                                kQuelleB, kQuelleC, "fuehrt_vor",
                                kQuelleC, kQuelleA, "fuehrt_vor" })));
        state::Zustand z;
        const auto erg = lade (bytes, z);
        pruefe (erg == state::LadeErgebnis::nurLesen && z.grund.contains ("cycle"),
                "der Leser weist einen gespeicherten Zyklus als read-only ab: " + z.grund);
    }

    abschnitt ("M-06  zyklus_wird_als_gleichrangigkeit_gespeichert");
    pruefe (p->speichereQuellenGleichrangigkeit (kQuelleC, kQuelleA),
            "derselbe Schluss als nicht steuerbare Gleichrangigkeit wird angenommen");
    const auto kanten = p->intentBeziehungen();
    pruefe (kanten.size() == 3, "der Bestand traegt jetzt drei Kanten");
    const auto neue = std::find_if (kanten.begin(), kanten.end(),
        [] (const auto& k) { return k.quelleA == kQuelleC && k.quelleB == kQuelleA; });
    pruefe (neue != kanten.end() && neue->art == state::Beziehungsart::gleichrangig,
            "und die dritte ist als 'gleichrangig' MARKIERT, nicht als gerichtet");

    // Save/Load: die markierte Runde ueberlebt und wird nie wieder als
    // gerichtet gelesen.
    juce::MemoryBlock bytes;
    p->getStateInformation (bytes);
    auto q = std::make_unique<EqCopilotProcessor>();
    q->setStateInformation (bytes.getData(), (int) bytes.getSize());
    pruefe (! q->stateNurLesen(), "der Stand mit markierter Gleichrangigkeit laedt normal");
    const auto zurueck = q->intentBeziehungen();
    const auto wieder = std::find_if (zurueck.begin(), zurueck.end(),
        [] (const auto& k) { return k.quelleA == kQuelleC && k.quelleB == kQuelleA; });
    pruefe (wieder != zurueck.end() && wieder->art == state::Beziehungsart::gleichrangig,
            "und kommt als 'gleichrangig' zurueck");
}

// ═════════════════════════════════════════════════════════════════════════
// M-07 · abgeleitet_ueberschreibt_user_nie
// ═════════════════════════════════════════════════════════════════════════
void m07()
{
    abschnitt ("M-07  abgeleitet_ueberschreibt_user_nie");

    auto p = mainProzessor();
    pruefe (p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::fuehrt,
                                  state::IntentHerkunft::user, 1.0),
            "der User setzt 'fuehrt'");
    const auto revNachUser = p->intentBestandRevision();

    pruefe (p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::begleitet,
                                  state::IntentHerkunft::abgeleitet, 0.4),
            "eine abgeleitete Vermutung wird angenommen - das ist kein Fehler");
    auto intents = p->sourceIntents();
    pruefe (intents.size() == 1 && intents[0].rolle == state::Rolle::fuehrt
                && intents[0].herkunft == state::IntentHerkunft::user,
            "aber sie ERSETZT den Userwert nicht");
    pruefe (p->intentBestandRevision() == revNachUser,
            "und hebt die Bestandsrevision nicht - es hat sich nichts geaendert");

    // Der umgekehrte Weg gilt: ein Userwert darf eine Vermutung ersetzen.
    auto q = mainProzessor();
    pruefe (q->setzeQuellenrolle (kQuelleA, {}, state::Rolle::begleitet,
                                  state::IntentHerkunft::abgeleitet, 0.4),
            "zuerst eine abgeleitete Vermutung");
    pruefe (q->setzeQuellenrolle (kQuelleA, {}, state::Rolle::fuehrt,
                                  state::IntentHerkunft::user, 1.0),
            "dann ein Userwert");
    intents = q->sourceIntents();
    pruefe (intents.size() == 1 && intents[0].rolle == state::Rolle::fuehrt
                && intents[0].herkunft == state::IntentHerkunft::user,
            "der Userwert gewinnt - die Regel ist gerichtet, nicht symmetrisch");
    pruefe (intents[0].revision == 2, "und die Objektrevision ist gestiegen");

    // Die Herkunft geht beim Speichern nicht verloren.
    juce::MemoryBlock bytes;
    q->getStateInformation (bytes);
    auto r = std::make_unique<EqCopilotProcessor>();
    r->setStateInformation (bytes.getData(), (int) bytes.getSize());
    const auto zurueck = r->sourceIntents();
    pruefe (zurueck.size() == 1 && zurueck[0].herkunft == state::IntentHerkunft::user
                && zurueck[0].revision == 2 && zurueck[0].konfidenz == 1.0,
            "Herkunft, Revision und Konfidenz ueberleben Save/Load");

    // Eine Vermutung bleibt als solche erkennbar (§37.4).
    auto s = mainProzessor();
    pruefe (s->setzeQuellenrolle (kQuelleB, {}, state::Rolle::traegt,
                                  state::IntentHerkunft::abgeleitet, 0.25),
            "eine reine Vermutung wird gespeichert");
    juce::MemoryBlock sBytes;
    s->getStateInformation (sBytes);
    auto t = std::make_unique<EqCopilotProcessor>();
    t->setStateInformation (sBytes.getData(), (int) sBytes.getSize());
    const auto vermutung = t->sourceIntents();
    pruefe (vermutung.size() == 1 && vermutung[0].herkunft == state::IntentHerkunft::abgeleitet
                && vermutung[0].konfidenz == 0.25,
            "und bleibt nach dem Laden als abgeleitet erkennbar");
}

// ═════════════════════════════════════════════════════════════════════════
// M-08 · zwei_rollen_je_bus_in_zwei_passagen
// ═════════════════════════════════════════════════════════════════════════
//
// §37.4 woertlich: "Derselbe Bus kann ohne Datenverlust verschiedene Rollen in
// zwei Passagen besitzen." Der Chor im Refrain vorne, in der Strophe hinter
// dem Klavier (§37.1).
void m08()
{
    abschnitt ("M-08  zwei_rollen_je_bus_in_zwei_passagen");

    auto p = mainProzessor();
    pruefe (p->setzeQuellenrolle (kQuelleA, kPassage1, state::Rolle::fuehrt,
                                  state::IntentHerkunft::user, 1.0),
            "Passage 1: die Quelle fuehrt");
    pruefe (p->setzeQuellenrolle (kQuelleA, kPassage2, state::Rolle::begleitet,
                                  state::IntentHerkunft::user, 1.0),
            "Passage 2: dieselbe Quelle begleitet");

    auto intents = p->sourceIntents();
    pruefe (intents.size() == 2,
            "beide Werte stehen nebeneinander - der zweite ueberschreibt den ersten nicht");

    state::Rolle r {};
    pruefe (p->wirkendeQuellenrolle (kQuelleA, kPassage1, r) && r == state::Rolle::fuehrt,
            "in Passage 1 wirkt 'fuehrt'");
    pruefe (p->wirkendeQuellenrolle (kQuelleA, kPassage2, r) && r == state::Rolle::begleitet,
            "in Passage 2 wirkt 'begleitet'");

    // Und beide ueberleben Save/Load bytegleich.
    juce::MemoryBlock erst;
    p->getStateInformation (erst);
    auto q = std::make_unique<EqCopilotProcessor>();
    q->setStateInformation (erst.getData(), (int) erst.getSize());
    pruefe (q->sourceIntents().size() == 2, "nach dem Laden stehen weiterhin beide");
    juce::MemoryBlock zweit;
    q->getStateInformation (zweit);
    pruefe (gleich (erst, zweit), "und Save nach Load ist BYTEGLEICH");

    // Ein dritter Scope derselben Quelle - global - kommt dazu, ohne die
    // beiden Passagenwerte anzufassen.
    pruefe (q->setzeQuellenrolle (kQuelleA, {}, state::Rolle::traegt,
                                  state::IntentHerkunft::user, 1.0),
            "ein globaler Wert kommt dazu");
    pruefe (q->sourceIntents().size() == 3, "jetzt sind es drei Scopes derselben Quelle");
    pruefe (q->wirkendeQuellenrolle (kQuelleA, kPassage1, r) && r == state::Rolle::fuehrt,
            "und der passagespezifische Wert gilt weiter (§37.2 Stufe 3)");

    // Der Gegenpfad: einen Scope entfernen laesst die anderen stehen.
    pruefe (q->entferneQuellenrolle (kQuelleA, kPassage1), "Passage 1 wird entfernt");
    pruefe (q->sourceIntents().size() == 2, "zwei Scopes bleiben");
    pruefe (q->wirkendeQuellenrolle (kQuelleA, kPassage1, r) && r == state::Rolle::traegt,
            "und Passage 1 faellt jetzt auf den globalen Wert zurueck");
}

// ═════════════════════════════════════════════════════════════════════════
// M-09 · userwerte_ueberleben_projekt_recall
// ═════════════════════════════════════════════════════════════════════════
//
// Die Brokerhaelfte (Neustart) misst A4 in Etappe B/I. Hier liegt die
// Plugin-Haelfte: der Intent reist im `MainProject`-Kind mit der
// FL-Projektdatei, nicht im SQLite-Spiegel.
void m09()
{
    abschnitt ("M-09  userwerte_ueberleben_projekt_recall");

    auto p = mainProzessor();
    p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::fuehrt, state::IntentHerkunft::user, 1.0);
    p->setzeQuellenrolle (kQuelleB, kPassage1, state::Rolle::verschmolzen,
                          state::IntentHerkunft::abgeleitet, 0.7);
    p->schuetzeQuelle (kQuelleA, state::Schutzeigenschaft::ausklang, -1, -1);
    p->schuetzeQuelle (kQuelleB, state::Schutzeigenschaft::band, 12, 40);
    p->setzeQuellenbeziehung (kQuelleA, kQuelleB, state::Beziehungsart::fuehrtVor);
    const auto revision = p->intentBestandRevision();
    pruefe (revision == 5, "fuenf echte Aenderungen ergeben Bestandsrevision 5");

    juce::MemoryBlock erst;
    p->getStateInformation (erst);

    auto q = std::make_unique<EqCopilotProcessor>();
    q->setStateInformation (erst.getData(), (int) erst.getSize());
    pruefe (! q->stateNurLesen(), "der Recall laedt normal");
    pruefe (q->sourceIntents().size() == 2 && q->intentSchutzangaben().size() == 2
                && q->intentBeziehungen().size() == 1,
            "alle drei Bestandteile kommen vollstaendig zurueck");
    pruefe (q->intentBestandRevision() == revision,
            "und die Bestandsrevision ueberlebt - sonst haette die "
            "Vollstaendigkeitsmarke aus M-86 keine Zahl");

    const auto zurueck = q->sourceIntents();
    int userWerte = 0, abgeleiteteWerte = 0;
    for (const auto& s : zurueck)
    {
        if (s.herkunft == state::IntentHerkunft::user) ++userWerte;
        if (s.herkunft == state::IntentHerkunft::abgeleitet) ++abgeleiteteWerte;
    }
    pruefe (userWerte == 1 && abgeleiteteWerte == 1,
            "Userwerte ueberleben, abgeleitete bleiben als solche erkennbar (§37.4)");

    juce::MemoryBlock zweit;
    q->getStateInformation (zweit);
    pruefe (gleich (erst, zweit), "Save nach Recall ist bytegleich");

    // Zweite Runde: der Roundtrip ist stabil, nicht nur einmalig.
    auto r = std::make_unique<EqCopilotProcessor>();
    r->setStateInformation (zweit.getData(), (int) zweit.getSize());
    juce::MemoryBlock dritt;
    r->getStateInformation (dritt);
    pruefe (gleich (zweit, dritt), "und ueber zwei Runden ebenfalls");
}

// ═════════════════════════════════════════════════════════════════════════
// Die fuenf Teile der State-Invariante (CLAUDE.md)
// ═════════════════════════════════════════════════════════════════════════
void stateInvariante()
{
    abschnitt ("State-Invariante  versionieren / Altstand / unbekanntes Feld / Dirty");

    // 1. Versionieren: jeder Eigenschaftsname traegt seine Fassung.
    {
        auto p = mainProzessor();
        p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::traegt, state::IntentHerkunft::user, 1.0);
        p->schuetzeQuelle (kQuelleA, state::Schutzeigenschaft::attack, -1, -1);
        p->setzeQuellenbeziehung (kQuelleA, kQuelleB, state::Beziehungsart::fuehrtVor);
        juce::MemoryBlock bytes;
        p->getStateInformation (bytes);
        const auto baum = juce::ValueTree::readFromData (bytes.getData(), bytes.getSize());
        const auto mp = baum.getChildWithName ("MainProject");
        for (const auto* name : { "source_intents_v1", "intent_protections_v1",
                                  "intent_relations_v1", "intent_revision_v1" })
            pruefe (mp.hasProperty (name),
                    juce::String ("der Eigenschaftsname traegt seine Fassung: ") + name);
    }

    // 2. Altstaende: ein Stand ohne die Felder bleibt gueltig.
    {
        const auto ohne = alsBlock (mainBaum());
        auto p = std::make_unique<EqCopilotProcessor>();
        p->setStateInformation (ohne.getData(), (int) ohne.getSize());
        pruefe (! p->stateNurLesen(), "ein Altstand ohne Intent-Felder laedt normal");
        pruefe (p->sourceIntents().empty() && p->intentBestandRevision() == 0,
                "und traegt einen leeren Bestand mit Revision 0 - kein erfundener Vorgabewert");
        juce::MemoryBlock heraus;
        p->getStateInformation (heraus);
        pruefe (gleich (ohne, heraus),
                "sein Save ist bytegleich - eine leere Liste reist NICHT als leeres Array");
    }

    // 3. Unbekannte Felder aus einer spaeteren Fassung ueberleben.
    {
        auto v = mainBaum();
        auto mp = v.getChildWithName ("MainProject");
        mp.setProperty ("intent_zukunft_v9", "etwas, das dieser Build nicht kennt", nullptr);
        const auto mitZukunft = alsBlock (v);
        auto p = std::make_unique<EqCopilotProcessor>();
        p->setStateInformation (mitZukunft.getData(), (int) mitZukunft.getSize());
        pruefe (! p->stateNurLesen(),
                "eine unbekannte MainProject-Eigenschaft zerstoert den Leser nicht");
        p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::traegt, state::IntentHerkunft::user, 1.0);
        juce::MemoryBlock heraus;
        p->getStateInformation (heraus);
        const auto zurueck = juce::ValueTree::readFromData (heraus.getData(), heraus.getSize());
        pruefe (zurueck.getChildWithName ("MainProject").hasProperty ("intent_zukunft_v9"),
                "und sie steht unveraendert im naechsten Save");
    }

    // 4. Save/Load bytegleich ueber zwei Runden — schon in M-08 und M-09.

    // 5. Host-Dirty: jede persistente Aenderung meldet GENAU einmal, ein
    //    No-op und ein abgewiesener Wert melden nichts (M-13).
    {
        auto p = mainProzessor();
        DirtyZaehler dirty;
        p->addListener (&dirty);

        pruefe (p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::traegt,
                                      state::IntentHerkunft::user, 1.0) && dirty.nonParam == 1,
                "eine neue Rolle meldet genau einmal Host-Dirty");
        pruefe (p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::traegt,
                                      state::IntentHerkunft::user, 1.0) && dirty.nonParam == 1,
                "derselbe Wert noch einmal ist ein No-op und meldet nichts");
        pruefe (p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::fuehrt,
                                      state::IntentHerkunft::user, 1.0) && dirty.nonParam == 2,
                "ein anderer Wert meldet wieder");
        pruefe (! p->setzeQuellenrolle ("keine-hex32", {}, state::Rolle::fuehrt,
                                        state::IntentHerkunft::user, 1.0) && dirty.nonParam == 2,
                "ein abgewiesener Wert meldet NICHT");
        pruefe (! p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::fuehrt,
                                        state::IntentHerkunft::user,
                                        std::numeric_limits<double>::quiet_NaN())
                    && dirty.nonParam == 2,
                "eine NaN-Konfidenz wird abgewiesen und meldet nicht");
        pruefe (p->schuetzeQuelle (kQuelleA, state::Schutzeigenschaft::attack, -1, -1)
                    && dirty.nonParam == 3,
                "eine neue Schutzangabe meldet");
        pruefe (p->schuetzeQuelle (kQuelleA, state::Schutzeigenschaft::attack, -1, -1)
                    && dirty.nonParam == 3,
                "dieselbe noch einmal nicht");
        pruefe (p->setzeQuellenbeziehung (kQuelleA, kQuelleB, state::Beziehungsart::fuehrtVor)
                    && dirty.nonParam == 4,
                "eine neue Beziehung meldet");
        pruefe (p->entferneQuellenrolle (kQuelleA, {}) && dirty.nonParam == 5,
                "das Entfernen meldet wie das Setzen - ein Loeschen ohne Dirty kaeme zurueck");
        pruefe (p->entferneQuellenrolle (kQuelleA, {}) && dirty.nonParam == 5,
                "ein zweites Entfernen ist ein No-op");
        pruefe (p->hebeQuellenschutzAuf (kQuelleA, state::Schutzeigenschaft::attack, -1, -1)
                    && dirty.nonParam == 6,
                "das Aufheben eines Schutzes meldet");
        pruefe (p->entferneQuellenbeziehung (kQuelleA, kQuelleB) && dirty.nonParam == 7,
                "und das Entfernen einer Beziehung ebenfalls");

        // Laden schweigt.
        juce::MemoryBlock bytes;
        p->getStateInformation (bytes);
        const auto vorLaden = dirty.nonParam;
        p->setStateInformation (bytes.getData(), (int) bytes.getSize());
        pruefe (dirty.nonParam == vorLaden, "Laden meldet kein Dirty");
        p->removeListener (&dirty);
    }

    // read-only verweigert jeden Handgriff und meldet nichts.
    {
        auto v = mainBaum();
        v.setProperty ("schema", 9, nullptr);      // fremdes Root-Major
        const auto fremd = alsBlock (v);
        auto p = std::make_unique<EqCopilotProcessor>();
        DirtyZaehler dirty;
        p->addListener (&dirty);
        p->setStateInformation (fremd.getData(), (int) fremd.getSize());
        pruefe (p->stateNurLesen(), "ein fremdes Root-Major laedt read-only");
        pruefe (! p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::traegt,
                                        state::IntentHerkunft::user, 1.0),
                "read-only verweigert setzeQuellenrolle");
        pruefe (! p->schuetzeQuelle (kQuelleA, state::Schutzeigenschaft::attack, -1, -1),
                "read-only verweigert schuetzeQuelle");
        pruefe (! p->setzeQuellenbeziehung (kQuelleA, kQuelleB, state::Beziehungsart::fuehrtVor),
                "read-only verweigert setzeQuellenbeziehung");
        pruefe (dirty.nonParam == 0, "und meldet dabei nichts");
        juce::MemoryBlock zurueck;
        p->getStateInformation (zurueck);
        pruefe (gleich (fremd, zurueck), "die Originalbytes gehen unveraendert zurueck");
        p->removeListener (&dirty);
    }
}

// ═════════════════════════════════════════════════════════════════════════
// Die Raender — jede Grenze von BEIDEN Seiten
// ═════════════════════════════════════════════════════════════════════════
void raender()
{
    abschnitt ("Raender  was die API ablehnt, lehnt auch der Leser ab");

    struct Fall
    {
        const char* name;
        juce::Identifier eigenschaft;
        juce::Array<juce::var> werte;
        const char* grundstueck;
    };

    const Fall faelle[] = {
        { "Quelle ist keine hex32", "source_intents_v1",
          liste ({ "nicht-hex", juce::String(), "traegt", juce::var ((juce::int64) 1), "user", juce::var (1.0) }),
          "invalid source id" },
        { "Passagenscope ist weder leer noch hex32", "source_intents_v1",
          liste ({ kQuelleA, "kurz", "traegt", juce::var ((juce::int64) 1), "user", juce::var (1.0) }),
          "invalid passage scope" },
        { "Revision 0 gibt es nicht", "source_intents_v1",
          liste ({ kQuelleA, juce::String(), "traegt", juce::var ((juce::int64) 0), "user", juce::var (1.0) }),
          "at least 1" },
        { "unbekannte Herkunft", "source_intents_v1",
          liste ({ kQuelleA, juce::String(), "traegt", juce::var ((juce::int64) 1), "geraten", juce::var (1.0) }),
          "unknown origin" },
        { "Konfidenz ueber 1", "source_intents_v1",
          liste ({ kQuelleA, juce::String(), "traegt", juce::var ((juce::int64) 1), "user", juce::var (1.5) }),
          "within [0,1]" },
        { "Konfidenz unter 0", "source_intents_v1",
          liste ({ kQuelleA, juce::String(), "traegt", juce::var ((juce::int64) 1), "user", juce::var (-0.001) }),
          "within [0,1]" },
        { "Konfidenz NaN", "source_intents_v1",
          liste ({ kQuelleA, juce::String(), "traegt", juce::var ((juce::int64) 1), "user",
                   juce::var (std::numeric_limits<double>::quiet_NaN()) }),
          "finite" },
        { "Konfidenz Inf", "source_intents_v1",
          liste ({ kQuelleA, juce::String(), "traegt", juce::var ((juce::int64) 1), "user",
                   juce::var (std::numeric_limits<double>::infinity()) }),
          "finite" },
        { "zwei Intents fuer denselben Scope", "source_intents_v1",
          liste ({ kQuelleA, juce::String(), "traegt", juce::var ((juce::int64) 1), "user", juce::var (1.0),
                   kQuelleA, juce::String(), "fuehrt", juce::var ((juce::int64) 1), "user", juce::var (1.0) }),
          "same source and scope" },
        { "unvollstaendiges Sextupel", "source_intents_v1",
          liste ({ kQuelleA, juce::String(), "traegt" }),
          "sextuples" },
        { "unbekannte Schutzeigenschaft", "intent_protections_v1",
          liste ({ kQuelleA, "hall", juce::var (-1), juce::var (-1) }),
          "unknown trait" },
        { "Bandschutz ueber dem Gitterrand", "intent_protections_v1",
          liste ({ kQuelleA, "band", juce::var (0), juce::var (222) }),
          "evidence grid" },
        { "leeres Bandintervall", "intent_protections_v1",
          liste ({ kQuelleA, "band", juce::var (7), juce::var (7) }),
          "evidence grid" },
        { "Nicht-Bandeigenschaft mit Intervall", "intent_protections_v1",
          liste ({ kQuelleA, "attack", juce::var (0), juce::var (4) }),
          "evidence grid" },
        { "Beziehung auf sich selbst", "intent_relations_v1",
          liste ({ kQuelleA, kQuelleA, "fuehrt_vor" }),
          "self relation" },
        { "unbekannte Beziehungsart", "intent_relations_v1",
          liste ({ kQuelleA, kQuelleB, "haengt_ab" }),
          "unknown relation" },
        { "zwei Beziehungen fuer dasselbe geordnete Paar", "intent_relations_v1",
          liste ({ kQuelleA, kQuelleB, "fuehrt_vor", kQuelleA, kQuelleB, "gleichrangig" }),
          "same ordered pair" },
    };

    for (const auto& f : faelle)
    {
        const auto bytes = baumMitEigenschaft (f.eigenschaft, juce::var (f.werte));
        state::Zustand z;
        const auto erg = lade (bytes, z);
        pruefe (erg == state::LadeErgebnis::nurLesen && z.grund.contains (f.grundstueck),
                juce::String ("Leser lehnt ab - ") + f.name + ": " + z.grund);
    }

    // Die Gegenprobe: der Leser ist nicht einfach kaputt. Ein gueltiger Stand
    // mit allen drei Bestandteilen laedt normal.
    {
        auto v = mainBaum();
        auto mp = v.getChildWithName ("MainProject");
        mp.setProperty ("source_intents_v1", juce::var (liste (
            { kQuelleA, juce::String(), "geschuetzt", juce::var ((juce::int64) 3), "template", juce::var (0.5) })), nullptr);
        mp.setProperty ("intent_protections_v1", juce::var (liste (
            { kQuelleA, "band", juce::var (0), juce::var (state::bandAnzahlEvidenzgitter) })), nullptr);
        mp.setProperty ("intent_relations_v1", juce::var (liste (
            { kQuelleA, kQuelleB, "gleichrangig" })), nullptr);
        mp.setProperty ("intent_revision_v1", juce::var ((juce::int64) 7), nullptr);
        state::Zustand z;
        const auto erg = lade (alsBlock (v), z);
        pruefe (erg == state::LadeErgebnis::geladen && ! z.nurLesen,
                "ein gueltiger Stand mit allen drei Bestandteilen laedt normal");
        pruefe (z.sourceIntents.size() == 1 && z.sourceIntents[0].rolle == state::Rolle::geschuetzt
                    && z.sourceIntents[0].revision == 3
                    && z.sourceIntents[0].herkunft == state::IntentHerkunft::vorlage,
                "und traegt die Werte, die dort stehen");
        pruefe (z.intentBestandRevision == 7, "samt Bestandsrevision 7");
    }

    // Der Rand, der beide Haelften verbindet: Inhalt ohne Bestandsrevision ist
    // kein gueltiger Stand - sonst haette die Vollstaendigkeitsmarke aus M-86
    // keine Zahl.
    {
        const auto bytes = baumMitEigenschaft ("source_intents_v1",
            juce::var (liste ({ kQuelleA, juce::String(), "traegt",
                                juce::var ((juce::int64) 1), "user", juce::var (1.0) })),
            /*mitRevision*/ false);
        state::Zustand z;
        const auto erg = lade (bytes, z);
        pruefe (erg == state::LadeErgebnis::nurLesen && z.grund.contains ("without intent_revision_v1"),
                "Intent-Inhalt ohne Bestandsrevision wird abgewiesen: " + z.grund);
    }

    // Und die API-Seite derselben Grenzen.
    {
        auto p = mainProzessor();
        pruefe (! p->setzeQuellenrolle ("nicht-hex", {}, state::Rolle::traegt,
                                        state::IntentHerkunft::user, 1.0),
                "API lehnt ab - Quelle ist keine hex32");
        pruefe (! p->setzeQuellenrolle (kQuelleA, "kurz", state::Rolle::traegt,
                                        state::IntentHerkunft::user, 1.0),
                "API lehnt ab - Passagenscope ist weder leer noch hex32");
        pruefe (! p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::traegt,
                                        state::IntentHerkunft::user, 1.5),
                "API lehnt ab - Konfidenz ueber 1");
        pruefe (! p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::traegt,
                                        state::IntentHerkunft::user,
                                        std::numeric_limits<double>::infinity()),
                "API lehnt ab - Konfidenz Inf");
        pruefe (! p->setzeQuellenbeziehung (kQuelleA, kQuelleA, state::Beziehungsart::fuehrtVor),
                "API lehnt ab - Beziehung auf sich selbst");
        pruefe (! p->schuetzeQuelle (kQuelleA, state::Schutzeigenschaft::band, 0, 222),
                "API lehnt ab - Bandschutz ueber dem Gitterrand");
        pruefe (p->sourceIntents().empty() && p->intentSchutzangaben().empty()
                    && p->intentBeziehungen().empty() && p->intentBestandRevision() == 0,
                "und nach sechs Ablehnungen ist der Bestand unveraendert leer");
    }

    // Der Deckel: die Produkt-API erzeugt keinen Stand, den der eigene Leser
    // ablehnt. Volle Liste schreiben, speichern, laden.
    {
        auto p = mainProzessor();
        for (int i = 0; i < state::maxSourceIntents; ++i)
            p->setzeQuellenrolle (hex32 (i + 1), {}, kRollen[i % state::rollenAnzahl],
                                  state::IntentHerkunft::user, 1.0);
        pruefe ((int) p->sourceIntents().size() == state::maxSourceIntents,
                "der Deckel von 256 Intents wird erreicht");
        pruefe (! p->setzeQuellenrolle (hex32 (0xFFFF), {}, state::Rolle::traegt,
                                        state::IntentHerkunft::user, 1.0),
                "und der 257. wird abgewiesen statt still verworfen");
        juce::MemoryBlock voll;
        p->getStateInformation (voll);
        auto q = std::make_unique<EqCopilotProcessor>();
        q->setStateInformation (voll.getData(), (int) voll.getSize());
        pruefe (! q->stateNurLesen() && (int) q->sourceIntents().size() == state::maxSourceIntents,
                "der volle Stand laedt - die API schreibt nichts, was der Leser verweigert");
        juce::MemoryBlock nochmal;
        q->getStateInformation (nochmal);
        pruefe (gleich (voll, nochmal), "und ist bytegleich reproduzierbar");
    }
}

// ═════════════════════════════════════════════════════════════════════════
// M-11 · intent_beruehrt_keine_messwerte
// ═════════════════════════════════════════════════════════════════════════
//
// §37.3 woertlich: "Intent veraendert nur Ranking, Schutz und erlaubte Tests.
// Er veraendert keine Messwerte." Gemessen wird das am PRODUKTPFAD: zwei
// echte Prozessoren bekommen denselben Strom, einer mit vollem Intent, einer
// ohne — und die Bandsaetze, Perzentile und Zaehler ihrer Messframes muessen
// bitgleich sein.
//
// Warum ein SINUS und keine Konstante: ein Gleichanteil traegt in keinem Band
// des Evidenzgitters einen Wert, und ein Vergleich zweier leerer Bandsaetze
// waere trivial gruen. Der 1000-Hz-Sinus wird aus dem Stromsample gerechnet,
// ist also ueber Blockgrenzen phasenstetig und in beiden Laeufen SAMPLEGLEICH;
// die Evidenzkadenz zaehlt Samples, keine Wanduhr, also faellt der erste
// frische Evidenzframe in beiden Laeufen an derselben Stelle. Ein Intent, der
// in die Messung leckte, veraenderte die Bandwerte selbst und faellt hier.
//
// Die Gegenprobe am Ende ist deshalb Pflicht: sie misst, dass wirklich Baender
// einen Wert tragen — sonst saehe ein zerstoerter Messpfad wie eine erfuellte
// Zusage aus.
void m11()
{
    abschnitt ("M-11  intent_beruehrt_keine_messwerte");

    constexpr double kFs = 48000.0;
    constexpr int    kBlock = 512;

    struct TestPlayHead : juce::AudioPlayHead
    {
        juce::int64 pos = 0;
        juce::Optional<PositionInfo> getPosition() const override
        {
            PositionInfo p;
            p.setIsPlaying (true);
            p.setTimeInSamples (pos);
            return p;
        }
    };

    // Der Prozessor liegt auf dem HEAP (NAK-175): er traegt eine
    // FeatureEngine von rund 0,5 MB, und der MSVC-Standardstack ist 1 MiB.
    const auto messe = [&] (bool mitIntent, nakama::analyse::FeatureFrame& aus) -> bool
    {
        auto p = mainProzessor();
        if (mitIntent)
        {
            // Ein VOLLER Intent, damit ein Leck an jeder der drei Stellen
            // sichtbar wuerde: Rolle, Schutzbereich und Beziehung.
            for (int i = 0; i < 8; ++i)
                p->setzeQuellenrolle (hex32 (i + 1), {}, kRollen[i % state::rollenAnzahl],
                                      state::IntentHerkunft::user, 1.0);
            p->schuetzeQuelle (hex32 (1), state::Schutzeigenschaft::band, 0,
                               state::bandAnzahlEvidenzgitter);
            p->schuetzeQuelle (hex32 (2), state::Schutzeigenschaft::attack, -1, -1);
            p->setzeQuellenbeziehung (hex32 (1), hex32 (2), state::Beziehungsart::fuehrtVor);
        }
        p->prepareToPlay (kFs, kBlock);
        TestPlayHead kopf;
        p->setPlayHead (&kopf);
        juce::AudioBuffer<float> puffer (2, kBlock);
        juce::MidiBuffer midi;
        bool bekommen = false;
        juce::int64 strom = 0;
        for (int i = 0; i < 4000 && ! bekommen; ++i)
        {
            for (int s = 0; s < puffer.getNumSamples(); ++s)
            {
                const auto wert = (float) (0.5 * std::sin (2.0 * juce::MathConstants<double>::pi
                                                           * 1000.0 * (double) (strom + s) / kFs));
                for (int c = 0; c < puffer.getNumChannels(); ++c)
                    puffer.setSample (c, s, wert);
            }
            p->processBlock (puffer, midi);
            strom += kBlock;
            kopf.pos += kBlock;
            const auto f = p->merkmalFrame();
            if (f.evidenzFrisch)
            {
                aus = f;
                bekommen = true;
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (1));
        }
        p->setPlayHead (nullptr);
        return bekommen;
    };

    nakama::analyse::FeatureFrame ohne {}, mit {};
    const bool a = messe (false, ohne);
    const bool b = messe (true, mit);
    pruefe (a && b, "beide Laeufe liefern einen Evidenzframe");

    if (a && b)
    {
        pruefe (ohne.evidenz == mit.evidenz,
                "die 221 Evidenzbaender sind bitgleich (Werte, Bitmap, Gitter, Encoding)");
        pruefe (ohne.evidenzP10 == mit.evidenzP10 && ohne.evidenzP50 == mit.evidenzP50
                    && ohne.evidenzP95 == mit.evidenzP95,
                "die drei Perzentile sind bitgleich");
        pruefe (ohne.live == mit.live, "die 64 Livebaender sind bitgleich");
        pruefe (ohne.metricsVersion == mit.metricsVersion,
                "die metrics_version ist dieselbe");
        pruefe (ohne.nichtEndlichEvidenz == mit.nichtEndlichEvidenz
                    && ohne.nichtEndlichRahmen == mit.nichtEndlichRahmen,
                "und die NaN-Zaehler ebenfalls");

        // Gegenprobe: der Vergleich ist nicht trivial gruen, weil beide
        // Bandsaetze leer waeren. Mindestens ein Band traegt sein Bit.
        int mitBit = 0;
        for (int i = 0; i < nakama::analyse::Gitter::evidenzBaender; ++i)
            if ((ohne.evidenz.bitmap[i / 8] >> (i % 8)) & 1) ++mitBit;
        pruefe (mitBit > 0,
                juce::String ("Gegenprobe: ") + juce::String (mitBit)
                    + " Evidenzbaender tragen wirklich einen Wert");
    }
}

// ═════════════════════════════════════════════════════════════════════════
// NAK-283 Etappe 2 · F11 — die Revisionsraender
// (Manifest docs/beweise/NAK-283.md Paragraph 5.1, M-07 bis M-12)
// ═════════════════════════════════════════════════════════════════════════
//
// Zwei Befunde in einem: (1) in den drei Entfern-Handgriffen stand das `erase`
// VOR dem Riegel - an der Revisionsobergrenze war der Eintrag entfernt, der
// Handgriff meldete `false`, `veraendert` blieb `false`, also KEIN Host-Dirty
// und keine Revision. Eine persistente Aenderung ohne Marke verletzt
// `nakama-state-v2.md:139` und "State bleibt verlustfrei". (2) die
// Assistentenrevision hatte keine obere Schranke - `a.revision += 1` an
// `int64max` ist signed-integer-UB, und der erzeugte negative Wert wird vom
// EIGENEN Reader abgewiesen ("revision must be at least 1").
//
// Der Zustand am Rand ist ueber den LADEWEG erreichbar: der Reader hat keine
// obere Schranke, und der eigene Headroomkandidat erzeugt `int64max`
// ausdruecklich. Jeder Fall hier baut ihn deshalb als DEKLARIERTEN MUTANTEN
// eines Writer-Standes mit genau EINER Abweichung (Pruefliste E) und laedt ihn,
// statt den `Zustand` von Hand zu setzen.
// Rotlauf: docs/beweise/roh/NAK-283-rot-M-07-etappe-2.txt bis -M-12-etappe-2.txt.

namespace nak283
{
constexpr auto kMax = std::numeric_limits<juce::int64>::max();

/// Ein deklarierter Mutant eines WRITER-Standes: genau eine MainProject-
/// Eigenschaft wird ersetzt.
juce::MemoryBlock mitMainFeld (const juce::MemoryBlock& writer,
                               const juce::Identifier& feld, const juce::var& wert)
{
    auto baum = juce::ValueTree::readFromData (writer.getData(), writer.getSize());
    baum.getChildWithName ("MainProject").setProperty (feld, wert, nullptr);
    return alsBlock (baum);
}

/// Derselbe Mutant fuer EINEN Platz der flachen Assistentenliste (Index 2 ist
/// die Revision, Index 3 das Offen-Flag).
juce::MemoryBlock mitAssistentenPlatz (const juce::MemoryBlock& writer,
                                       int platz, const juce::var& wert)
{
    auto baum = juce::ValueTree::readFromData (writer.getData(), writer.getSize());
    auto mp = baum.getChildWithName ("MainProject");
    const auto* alt = mp.getProperty ("assistant_step_v1").getArray();
    juce::Array<juce::var> liste;
    if (alt != nullptr) liste = *alt;
    if (platz < liste.size()) liste.set (platz, wert);
    mp.setProperty ("assistant_step_v1", juce::var (liste), nullptr);
    return alsBlock (baum);
}

/// Ein Writer-Stand mit allen drei Bestandteilen des Intents.
juce::MemoryBlock writerMitBestand()
{
    auto p = mainProzessor();
    p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::fuehrt, state::IntentHerkunft::user, 1.0);
    p->schuetzeQuelle (kQuelleA, state::Schutzeigenschaft::attack, -1, -1);
    p->setzeQuellenbeziehung (kQuelleA, kQuelleB, state::Beziehungsart::fuehrtVor);
    juce::MemoryBlock b;
    p->getStateInformation (b);
    return b;
}

/// Ein Writer-Stand mit einem OFFENEN Assistentenschritt.
juce::MemoryBlock writerMitAssistent()
{
    auto p = mainProzessor();
    p->assistentStarten (hex32 (0x5711));
    juce::MemoryBlock b;
    p->getStateInformation (b);
    return b;
}

/// Laedt einen Stand und prueft, dass er NORMAL (nicht read-only) ankommt.
state::Zustand geladen (const juce::MemoryBlock& bytes, const char* was)
{
    state::Zustand z;
    const auto erg = lade (bytes, z);
    pruefe (erg == state::LadeErgebnis::geladen && ! z.nurLesen,
            juce::String (was) + ": der Mutant laedt normal (der eigene Reader nimmt "
            "int64max an, er hat keine obere Schranke) - " + z.grund);
    return z;
}

/// Die Bytes eines Zustands - die schaerfste Form von "unveraendert".
juce::MemoryBlock bytesVon (const state::Zustand& z)
{
    juce::MemoryBlock b;
    state::speichere (z, b);
    return b;
}

// ─────────────────────────────────────────────────────────────────────────
// M-07 bis M-09 · Ablehnung ohne Mutation in den drei Entfern-Handgriffen
// ─────────────────────────────────────────────────────────────────────────
void m283_07_bis_09()
{
    abschnitt ("NAK-283 M-07 bis M-09  entfernen_an_der_revisionsobergrenze_mutiert_nichts");

    const auto writer = writerMitBestand();
    const auto amRand = mitMainFeld (writer, "intent_revision_v1", juce::var (kMax));

    // ── M-07 · entferneIntent ──────────────────────────────────────────────
    {
        auto z = geladen (amRand, "M-07");
        pruefe (z.intentBestandRevision == kMax && z.sourceIntents.size() == 1,
                "M-07: Vorbedingung - Bestandsrevision am int64-Maximum, ein Intent-Eintrag");
        const auto vorher = bytesVon (z);
        const auto intentsVorher = z.sourceIntents;

        bool veraendert = true;
        juce::String grund;
        const bool ok = state::entferneIntent (z, kQuelleA, {}, veraendert, grund);

        pruefe (! ok && ! veraendert && grund == "intent revision would overflow",
                "M-07: der Handgriff liefert `false` mit gesetztem Grund und "
                "`veraendert == false` - " + grund);
        pruefe (z.sourceIntents == intentsVorher && z.sourceIntents.size() == 1,
                "M-07: entferne_intent_an_der_revisionsobergrenze_mutiert_nichts - "
                "`z.sourceIntents` ist unveraendert, der Eintrag steht noch");
        pruefe (gleich (bytesVon (z), vorher),
                "M-07: der GANZE Zustand ist byteweise unveraendert");
        pruefe (z.intentBestandRevision == kMax,
                "M-07: und die Bestandsrevision steht weiterhin am Maximum");
    }

    // ── M-08 · entferneSchutzangabe ────────────────────────────────────────
    {
        auto z = geladen (amRand, "M-08");
        pruefe (z.schutzangaben.size() == 1, "M-08: Vorbedingung - eine Schutzangabe");
        const auto vorher = bytesVon (z);

        bool veraendert = true;
        juce::String grund;
        const bool ok = state::entferneSchutzangabe (
            z, kQuelleA, state::Schutzeigenschaft::attack, -1, -1, veraendert, grund);

        pruefe (! ok && ! veraendert && grund == "intent revision would overflow",
                "M-08: der Handgriff liefert `false` mit gesetztem Grund - " + grund);
        pruefe (z.schutzangaben.size() == 1,
                "M-08: entferne_schutzangabe_an_der_revisionsobergrenze_mutiert_nichts - "
                "`z.schutzangaben` ist unveraendert");
        pruefe (gleich (bytesVon (z), vorher),
                "M-08: der GANZE Zustand ist byteweise unveraendert");
    }

    // ── M-09 · entferneBeziehung ───────────────────────────────────────────
    {
        auto z = geladen (amRand, "M-09");
        pruefe (z.intentBeziehungen.size() == 1, "M-09: Vorbedingung - eine Beziehung");
        const auto vorher = bytesVon (z);

        bool veraendert = true;
        juce::String grund;
        const bool ok = state::entferneBeziehung (z, kQuelleA, kQuelleB, veraendert, grund);

        pruefe (! ok && ! veraendert && grund == "intent revision would overflow",
                "M-09: der Handgriff liefert `false` mit gesetztem Grund - " + grund);
        pruefe (z.intentBeziehungen.size() == 1,
                "M-09: entferne_beziehung_an_der_revisionsobergrenze_mutiert_nichts - "
                "`z.intentBeziehungen` ist unveraendert");
        pruefe (gleich (bytesVon (z), vorher),
                "M-09: der GANZE Zustand ist byteweise unveraendert");
    }

    // ── Gegenprobe: NICHTS ZU ENTFERNEN bleibt ein No-op ───────────────────
    //
    // Der Fix haette den No-op-Pfad verschieben koennen: waere die Revision
    // jetzt auch dann gehoben, wenn gar kein Eintrag passt, meldete ein
    // folgenloser Aufruf Host-Dirty. Diese Probe haelt fest, dass das nicht
    // passiert (M-12, `nakama-state-v2.md:139`).
    {
        state::Zustand z;
        const auto erg = lade (writer, z);
        pruefe (erg == state::LadeErgebnis::geladen, "Gegenprobe: der Writer-Stand laedt");
        const auto revVorher = z.intentBestandRevision;
        bool veraendert = true;
        juce::String grund;
        const bool ok = state::entferneIntent (z, kQuelleC, {}, veraendert, grund);
        pruefe (ok && ! veraendert && grund.isEmpty()
                    && z.intentBestandRevision == revVorher,
                "Gegenprobe: nichts zu entfernen bleibt ein No-op - `true`, keine "
                "Revision, kein Grund; der Riegel hat den No-op-Pfad nicht verschoben");
    }
}

// ─────────────────────────────────────────────────────────────────────────
// M-10 · die Assistentenrevision laeuft nicht ueber - an allen drei
// Inkrementstellen
// ─────────────────────────────────────────────────────────────────────────
void m283_10()
{
    abschnitt ("NAK-283 M-10  assistentenrevision_laeuft_nicht_ueber");

    const auto writer = writerMitAssistent();
    const auto offenAmRand = mitAssistentenPlatz (writer, 2, juce::var (kMax));

    // ── Weg 1: `schrittAendern` ueber `assistentUeberspringen` ─────────────
    {
        auto z = geladen (offenAmRand, "M-10 (schrittAendern)");
        pruefe (z.assistent.gesetzt && z.assistent.offen && z.assistent.revision == kMax,
                "M-10: Vorbedingung - ein OFFENER Schritt mit `revision == int64max`");
        const auto schrittVorher = z.assistent.schritt;
        bool veraendert = true;
        juce::String grund;
        const bool ok = state::assistentUeberspringen (z, veraendert, grund);
        pruefe (! ok && ! veraendert && grund == "assistant revision would overflow",
                "M-10: `schrittAendern` weist ab statt zu inkrementieren - " + grund);
        pruefe (z.assistent.revision == kMax && z.assistent.schritt == schrittVorher
                    && z.assistent.offen,
                "M-10: `a.revision` bleibt int64max, und weder `a.schritt` noch `a.offen` "
                "sind angefasst - die Schranke steht VOR der Zuweisung",
                juce::String (z.assistent.revision));
    }

    // ── Weg 2: der neue Schritt (zweite Inkrementstelle) ───────────────────
    {
        // Derselbe Stand, aber der Schritt ist GESCHLOSSEN - dann nimmt
        // `setzeAssistentenschritt` den Zweig "neuer Schritt" mit seinem
        // eigenen `a.revision += 1`.
        const auto geschlossenAmRand = mitAssistentenPlatz (offenAmRand, 3, juce::var (false));
        auto z = geladen (geschlossenAmRand, "M-10 (neuer Schritt)");
        pruefe (z.assistent.gesetzt && ! z.assistent.offen && z.assistent.revision == kMax,
                "M-10: Vorbedingung - ein GESCHLOSSENER Schritt mit `revision == int64max`");
        const auto stepIdVorher = z.assistent.stepId;
        bool veraendert = true;
        juce::String grund;
        const bool ok = state::setzeAssistentenschritt (
            z, hex32 (0x5712), state::Assistentenschritt::coverage, veraendert, grund);
        pruefe (! ok && ! veraendert && grund == "assistant revision would overflow",
                "M-10: auch der NEUE Schritt weist ab - " + grund);
        pruefe (z.assistent.revision == kMax && z.assistent.stepId == stepIdVorher,
                "M-10: `a.revision` bleibt int64max, und die Schritt-Kennung ist "
                "unveraendert");
    }

    // ── Weg 3: das Ergebnis (dritte Inkrementstelle) ───────────────────────
    {
        auto z = geladen (offenAmRand, "M-10 (Ergebnis)");
        const auto ergebnisVorher = z.assistent.ergebnis;
        bool veraendert = true;
        juce::String grund;
        const bool ok = state::setzeAssistentenergebnis (
            z, state::Assistentenergebnis::passageMessen, veraendert, grund);
        pruefe (! ok && ! veraendert && grund == "assistant revision would overflow",
                "M-10: auch das Ergebnis weist ab - " + grund);
        pruefe (z.assistent.revision == kMax && z.assistent.ergebnis == ergebnisVorher,
                "M-10: assistentenrevision_laeuft_nicht_ueber - `a.revision += 1` wird an "
                "keiner der drei Stellen auf int64max ausgefuehrt; kein negativer "
                "Folgewert, kein signed-integer-UB",
                juce::String (z.assistent.revision));
    }
}

// ─────────────────────────────────────────────────────────────────────────
// M-12 · Regressionswache: unter der Grenze aendert sich nichts
// ─────────────────────────────────────────────────────────────────────────
//
// Die Vorpruefung darf den NORMALFALL nicht verschieben: bei `int64max - 1`
// wird der Eintrag entfernt, die Revision steigt um GENAU 1, und jeder der
// vier Handgriffe verhaelt sich wie vor der Etappe. Heute gruen; absichtlich
// gebrochen mit einer Schranke bei `int64max - 1`.
void m283_12()
{
    abschnitt ("NAK-283 M-12 (Wache)  unter_der_grenze_aendert_sich_nichts");

    const auto writer = writerMitBestand();
    const auto knappDrunter = mitMainFeld (writer, "intent_revision_v1", juce::var (kMax - 1));

    {
        auto z = geladen (knappDrunter, "M-12 (Intent)");
        pruefe (z.intentBestandRevision == kMax - 1,
                "M-12: Vorbedingung - Bestandsrevision bei int64max - 1");
        bool veraendert = false;
        juce::String grund;
        const bool ok = state::entferneIntent (z, kQuelleA, {}, veraendert, grund);
        pruefe (ok && veraendert && grund.isEmpty() && z.sourceIntents.empty()
                    && z.intentBestandRevision == kMax,
                "M-12: der Intent wird entfernt, und die Revision steigt um GENAU 1",
                juce::String (z.intentBestandRevision));
    }
    {
        auto z = geladen (knappDrunter, "M-12 (Schutz)");
        bool veraendert = false;
        juce::String grund;
        const bool ok = state::entferneSchutzangabe (
            z, kQuelleA, state::Schutzeigenschaft::attack, -1, -1, veraendert, grund);
        pruefe (ok && veraendert && z.schutzangaben.empty()
                    && z.intentBestandRevision == kMax,
                "M-12: die Schutzangabe wird entfernt, Revision +1");
    }
    {
        auto z = geladen (knappDrunter, "M-12 (Beziehung)");
        bool veraendert = false;
        juce::String grund;
        const bool ok = state::entferneBeziehung (z, kQuelleA, kQuelleB, veraendert, grund);
        pruefe (ok && veraendert && z.intentBeziehungen.empty()
                    && z.intentBestandRevision == kMax,
                "M-12: die Beziehung wird entfernt, Revision +1");
    }
    {
        const auto assistentDrunter =
            mitAssistentenPlatz (writerMitAssistent(), 2, juce::var (kMax - 1));
        auto z = geladen (assistentDrunter, "M-12 (Assistent)");
        pruefe (z.assistent.revision == kMax - 1,
                "M-12: Vorbedingung - Assistentenrevision bei int64max - 1");
        bool veraendert = false;
        juce::String grund;
        const bool ok = state::assistentUeberspringen (z, veraendert, grund);
        pruefe (ok && veraendert && z.assistent.revision == kMax,
                "M-12: unter_der_grenze_aendert_sich_nichts - der Assistentenschritt "
                "wechselt, und die Revision steigt um GENAU 1",
                juce::String (z.assistent.revision));
    }

    // Und der Weg ueber den PRODUKTPFAD: ein echter Handgriff meldet genau
    // einmal Host-Dirty (`nakama-state-v2.md:139`).
    {
        auto p = mainProzessor();
        DirtyZaehler dirty;
        p->addListener (&dirty);
        pruefe (p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::fuehrt,
                                      state::IntentHerkunft::user, 1.0),
                "M-12 Produktpfad: ein Intent wird gesetzt");
        const auto dirtyNachSetzen = dirty.nonParam;
        const auto revNachSetzen = p->intentBestandRevision();
        pruefe (p->entferneQuellenrolle (kQuelleA, {}),
                "M-12 Produktpfad: derselbe Intent wird entfernt");
        pruefe (dirty.nonParam == dirtyNachSetzen + 1
                    && p->intentBestandRevision() == revNachSetzen + 1
                    && p->sourceIntents().empty(),
                "M-12 Produktpfad: GENAU eine Dirty-Meldung und GENAU eine Revision je "
                "persistenter Aenderung",
                juce::String (dirty.nonParam - dirtyNachSetzen));
        // Ein folgenloser Aufruf schweigt weiterhin.
        pruefe (p->entferneQuellenrolle (kQuelleA, {}),
                "M-12 Produktpfad: der folgenlose Aufruf meldet Erfolg");
        pruefe (dirty.nonParam == dirtyNachSetzen + 1
                    && p->intentBestandRevision() == revNachSetzen + 1,
                "M-12 Produktpfad: und er schweigt - kein Dirty, keine Revision",
                juce::String (dirty.nonParam - dirtyNachSetzen));
        p->removeListener (&dirty);
    }
}
} // namespace nak283

} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    std::cout << "EqCopSonde014IntentTest - der musikalische Intent im Main-State" << std::endl;
    std::cout << "SONDE-014 Etappe A, M-01 bis M-09 und M-11" << std::endl;

    m01();
    m02();
    m03();
    m04();
    m05();
    m06();
    m07();
    m08();
    m09();
    stateInvariante();
    raender();
    m11();
    // NAK-283 Etappe 2 (F11): die Revisionsraender - Ablehnung ohne Mutation
    // (M-07 bis M-09), die obere Schranke der Assistentenrevision (M-10) und
    // die Wache, dass der Normalfall unter der Grenze unveraendert bleibt (M-12).
    nak283::m283_07_bis_09();
    nak283::m283_10();
    nak283::m283_12();

    std::cout << std::endl;
    if (fehler == 0)
        std::cout << "SONDE-014-INTENT-TEST OK - " << bestanden << " Pruefungen ok, 0 Fehler"
                  << std::endl;
    else
        std::cout << "SONDE-014-INTENT-TEST FEHLGESCHLAGEN - " << fehler << " Fehler, "
                  << bestanden << " ok" << std::endl;
    return fehler == 0 ? 0 : 1;
}
