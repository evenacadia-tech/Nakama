/*  EqCopSonde014AssistentTest — der `AssistantStep` im Main-State
    (SONDE-014 Etappe G, Matrixzeilen M-55 bis M-62).

    ── WAS DIESES BEIN MISST UND WARUM ES IM MAIN LIEGT ─────────────────────

    Entscheid E-08 (§4.8): „Zustandsmaschine des `AssistantStep`: Uebergaenge,
    Abbruch, Zurueck, Ueberspringen, Resume, Rekonstruktion — MAIN, persistent
    im `MainProjectState`." `assistent.rs` im Broker ist Spiegel und
    Vertragsvalidierung, KEINE zweite Zustandsmaschine. Dieses Bein misst
    deshalb die Maschine, nicht ihren Spiegel.

    ── DIE SCHAERFSTE ZEILE: PREVIEW (E-07, M-55) ───────────────────────────

    Der Zustand `preview` BLEIBT in der Vertragsmenge — ihn zu streichen
    hiesse, ihn in P6 neu erfinden zu muessen. Die P5-Uebergangstabelle fuehrt
    aber keine Kante dorthin, und ein GESPEICHERTER Schritt mit `preview` ist
    in P5 ein LESEFEHLER, kein stiller Sprung auf `proposal` oder `remeasure`.
    Beide Haelften fallen hier einzeln.

    ── ZWEI EBENEN, UND DER ROTBEWEIS FAELLT AN DER ZWEITEN ─────────────────

    Wie in Etappe A: erst die reinen Funktionen aus `NakamaState.cpp`, dann
    derselbe Handgriff ueber den ECHTEN `EqCopilotProcessor` mit
    `setStateInformation`/`getStateInformation` als Rand. Prozessoren liegen
    auf dem HEAP (NAK-175, MSVC-Standardstack 1 MiB).
*/

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>

#include "NakamaState.h"
#include "PluginProcessor.h"

#include <algorithm>
#include <initializer_list>
#include <iostream>
#include <string_view>
#include <memory>
#include <set>
#include <string>
#include <vector>

using namespace eqcop;
namespace state = nakama::state;

namespace
{
int bestanden = 0;
int fehler    = 0;

void pruefe (bool ok, const juce::String& was)
{
    std::cout << (ok ? "  ok      " : "  FEHLER  ") << was << std::endl;
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

const auto kSchritt = hex32 (0x5741);
const auto kBefund  = hex32 (0xF1D6);

/// Die acht Zustaende in Vertragsreihenfolge.
const std::vector<state::Assistentenschritt> kAlle {
    state::Assistentenschritt::coverage,  state::Assistentenschritt::finding,
    state::Assistentenschritt::evidence,  state::Assistentenschritt::listen,
    state::Assistentenschritt::proposal,  state::Assistentenschritt::preview,
    state::Assistentenschritt::remeasure, state::Assistentenschritt::verdict,
};

/// Die P5-Folge OHNE `preview`.
const std::vector<state::Assistentenschritt> kP5Folge {
    state::Assistentenschritt::coverage,  state::Assistentenschritt::finding,
    state::Assistentenschritt::evidence,  state::Assistentenschritt::listen,
    state::Assistentenschritt::proposal,  state::Assistentenschritt::remeasure,
    state::Assistentenschritt::verdict,
};

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

/// Ein von Hand gebauter Stand mit einem Assistentenschritt — so entstehen
/// Staende, die die Produkt-API gar nicht erzeugen kann. Genau die misst der
/// Leser.
juce::MemoryBlock baumMitSchritt (const juce::var& liste)
{
    auto v = mainBaum();
    auto mp = v.getChildWithName ("MainProject");
    mp.setProperty ("assistant_step_v1", liste, nullptr);
    return alsBlock (v);
}

juce::var schrittListe (const juce::String& id, const char* schritt, juce::int64 revision,
                        bool offen, const char* ergebnis = "schritt")
{
    juce::Array<juce::var> flach;
    flach.add (id);
    flach.add (juce::String (schritt));
    flach.add (juce::var (revision));
    flach.add (juce::var (offen));
    flach.add (juce::String());
    flach.add (juce::String());
    flach.add (juce::String());
    flach.add (juce::String (ergebnis));
    return juce::var (flach);
}

state::Zustand frischerZustand()
{
    state::Zustand z;
    const auto block = alsBlock (mainBaum());
    const auto ergebnis = state::lade (block.getData(), block.getSize(),
                                       state::Bundle::eqcp(), z);
    jassert (ergebnis == state::LadeErgebnis::geladen);
    juce::ignoreUnused (ergebnis);
    return z;
}

/// Laedt einen von Hand gebauten Stand und sagt, ob er READ-ONLY wurde.
bool wirdReadOnly (const juce::MemoryBlock& block, juce::String& grund)
{
    state::Zustand z;
    const auto ergebnis = state::lade (block.getData(), block.getSize(),
                                       state::Bundle::eqcp(), z);
    grund = z.grund;
    return ergebnis != state::LadeErgebnis::geladen || z.nurLesen;
}

/// Laedt einen Stand, der GELADEN werden soll.
bool laedtNormal (const juce::MemoryBlock& block, state::Zustand& aus, juce::String& grund)
{
    const auto ergebnis = state::lade (block.getData(), block.getSize(),
                                       state::Bundle::eqcp(), aus);
    grund = aus.grund;
    return ergebnis == state::LadeErgebnis::geladen && ! aus.nurLesen;
}

std::unique_ptr<EqCopilotProcessor> prozessor()
{
    // HEAP, nicht Rahmen: NAK-175, der MSVC-Standardstack ist 1 MiB.
    auto p = std::make_unique<EqCopilotProcessor>();
    const auto block = alsBlock (mainBaum());
    p->setStateInformation (block.getData(), static_cast<int> (block.getSize()));
    return p;
}

/// Wie `prozessor()`, aber MIT gueltiger Wire-Adresse.
///
/// NR-08 misst die SENDESEITE; ohne Klassifizierung und Bindung ist
/// `v3AssistantStepJson()` strukturell leer, und der Fall maesse nichts.
std::unique_ptr<EqCopilotProcessor> prozessorAmDraht()
{
    auto p = prozessor();
    p->setzeEditorOffen (true);
    p->setzeBindung ("hub", {}, {});
    return p;
}
} // namespace

int main()
{
    std::cout.setf (std::ios::unitbuf);
    std::cout << "SONDE-014 Etappe G - AssistantStep im Main (M-55 bis M-62)\n";

    // ═══════════════════════════════════════════════════════════════════
    // M-55 · p5_uebergangstabelle_hat_keine_kante_nach_preview
    // ═══════════════════════════════════════════════════════════════════
    abschnitt ("M-55: die Zustandsmenge, die Folge und der Sonderfall preview");
    {
        // Die Menge hat ACHT Werte, und jeder kommt ueber seinen eigenen
        // Rueckweg zurueck.
        std::set<std::string> woerter;
        for (auto s : kAlle)
        {
            const auto w = std::string (state::wort (s));
            woerter.insert (w);
            state::Assistentenschritt zurueck {};
            pruefe (state::assistentenschrittAus (juce::String (w), zurueck) && zurueck == s,
                    juce::String ("M-55: ") + w.c_str() + " kommt ueber seinen Rueckweg zurueck");
        }
        pruefe (woerter.size() == 8, "M-55: acht verschiedene Zustaende");
        state::Assistentenschritt fremd {};
        pruefe (! state::assistentenschrittAus ("neunter", fremd),
                "M-55: ein neunter Zustand faellt");
        pruefe (! state::assistentenschrittAus ("", fremd),
                "M-55: und ein leeres Wort ebenso");

        // Die Folge: keine Kante nach `preview`, in KEINE Richtung.
        for (auto s : kAlle)
        {
            pruefe (! state::p5UebergangErlaubt (s, state::Assistentenschritt::preview),
                    juce::String ("M-55: keine Kante von ") + state::wort (s) + " nach preview");
            pruefe (! state::p5UebergangErlaubt (state::Assistentenschritt::preview, s),
                    juce::String ("M-55: und keine von preview nach ") + state::wort (s));
        }
        // Die P5-Folge selbst ist vollstaendig verkettet.
        for (size_t i = 0; i + 1 < kP5Folge.size(); ++i)
            pruefe (state::p5UebergangErlaubt (kP5Folge[i], kP5Folge[i + 1]),
                    juce::String ("M-55: ") + state::wort (kP5Folge[i]) + " -> "
                        + state::wort (kP5Folge[i + 1]));
        state::Assistentenschritt weiter {};
        pruefe (! state::p5Naechster (state::Assistentenschritt::verdict, weiter),
                "M-55: `verdict` ist terminal");
        pruefe (! state::p5Naechster (state::Assistentenschritt::preview, weiter),
                "M-55: und `preview` fuehrt nirgendwohin");
        // Ein Sprung ueber einen Zustand ist keine Kante.
        pruefe (! state::p5UebergangErlaubt (state::Assistentenschritt::coverage,
                                             state::Assistentenschritt::listen),
                "M-55: ein Sprung ueber zwei Zustaende ist keine Kante");
    }

    // ═══════════════════════════════════════════════════════════════════
    // M-55 · gespeicherter_preview_schritt_ist_ein_lesefehler
    // ═══════════════════════════════════════════════════════════════════
    abschnitt ("M-55: ein gespeicherter preview-Schritt ist ein Lesefehler");
    {
        juce::String grund;
        const auto block = baumMitSchritt (schrittListe (kSchritt, "preview", 3, true));
        pruefe (wirdReadOnly (block, grund),
                "M-55: ein gespeicherter preview-Schritt wird NICHT normal geladen");
        pruefe (grund.contains ("preview"),
                juce::String ("M-55: und der Grund nennt ihn beim Namen: ") + grund);
        // ⚠️ Die Gegenprobe: derselbe Stand mit einem P5-Zustand laedt.
        state::Zustand z2;
        juce::String grund2;
        const auto ok = baumMitSchritt (schrittListe (kSchritt, "proposal", 3, true));
        pruefe (laedtNormal (ok, z2, grund2) && z2.assistent.gesetzt
                    && z2.assistent.schritt == state::Assistentenschritt::proposal,
                juce::String ("M-55: derselbe Stand mit `proposal` laedt normal: ") + grund2);
        // Und die Produkt-API kann `preview` gar nicht erst setzen.
        auto z3 = frischerZustand();
        bool veraendert = false;
        juce::String g3;
        pruefe (! state::setzeAssistentenschritt (z3, kSchritt,
                                                  state::Assistentenschritt::preview,
                                                  veraendert, g3)
                    && ! veraendert,
                "M-55: die API setzt `preview` nicht");
    }

    // ═══════════════════════════════════════════════════════════════════
    // M-56 · jeder_zustand_traegt_fuenf_angaben
    // ═══════════════════════════════════════════════════════════════════
    abschnitt ("M-56: fuenf Angaben je Zustand, keine optional");
    {
        for (auto s : kAlle)
        {
            const auto v = state::schrittvertrag (s);
            pruefe (v.schritt == s && v.vollstaendig(),
                    juce::String ("M-56: ") + state::wort (s) + " traegt alle fuenf Angaben");
            pruefe (v.timeoutMs > 0,
                    juce::String ("M-56: ") + state::wort (s) + " hat ein Timeout > 0");
            // Die Rueckkante ist IMMER ein Zustand — der erste zeigt auf sich
            // selbst, und das ist die sichere Rueckkante „bleib, wo du bist".
            pruefe (std::find (kAlle.begin(), kAlle.end(), v.rueckkante) != kAlle.end(),
                    juce::String ("M-56: ") + state::wort (s) + " hat eine Rueckkante");
        }
        // ⚠️ Auch `preview` traegt seinen vollstaendigen Vertrag: der Zustand
        // existiert, P5 fuehrt nur keine Kante dorthin. Ihn leer zu lassen
        // hiesse, ihn halb zu streichen.
        pruefe (state::schrittvertrag (state::Assistentenschritt::preview).vollstaendig(),
                "M-56: auch `preview` traegt seinen vollstaendigen Vertrag");
        // Und die Rueckkante des ersten Zustands zeigt auf ihn selbst.
        pruefe (state::schrittvertrag (state::Assistentenschritt::coverage).rueckkante
                    == state::Assistentenschritt::coverage,
                "M-56: der erste Zustand ist seine eigene sichere Rueckkante");
    }

    // ═══════════════════════════════════════════════════════════════════
    // M-57 · hoechstens_ein_klanglicher_versuch
    // ═══════════════════════════════════════════════════════════════════
    abschnitt ("M-57: EIN Slot, strukturell");
    {
        auto z = frischerZustand();
        bool veraendert = false;
        juce::String grund;
        pruefe (state::setzeAssistentenschritt (z, kSchritt,
                                                state::Assistentenschritt::coverage,
                                                veraendert, grund)
                    && veraendert && z.assistent.gesetzt && z.assistent.offen,
                juce::String ("M-57: der erste Schritt beginnt: ") + grund);
        // Ein ZWEITER Startversuch bei offenem Schritt wird ABGEWIESEN, nicht
        // eingereiht.
        bool v2 = false;
        juce::String g2;
        pruefe (! state::setzeAssistentenschritt (z, hex32 (0x5742),
                                                  state::Assistentenschritt::coverage,
                                                  v2, g2)
                    && ! v2,
                "M-57: ein zweiter Startversuch wird abgewiesen, nicht eingereiht");
        pruefe (z.assistent.stepId == kSchritt,
                "M-57: und der erste Schritt steht unveraendert");
        // Nach dem Abbruch ist der Slot frei — und der erste Schritt bleibt
        // als Historie stehen.
        bool v3 = false;
        juce::String g3;
        pruefe (state::assistentAbbrechen (z, v3, g3) && v3 && ! z.assistent.offen,
                "M-57: der Abbruch macht den Schritt terminal");
        bool v4 = false;
        juce::String g4;
        pruefe (state::setzeAssistentenschritt (z, hex32 (0x5742),
                                                state::Assistentenschritt::coverage,
                                                v4, g4)
                    && v4 && z.assistent.stepId == hex32 (0x5742),
                juce::String ("M-57: danach beginnt ein neuer Schritt: ") + g4);
    }

    // ═══════════════════════════════════════════════════════════════════
    // M-58 · vier_gegenpfade
    // ═══════════════════════════════════════════════════════════════════
    abschnitt ("M-58: Abbruch, Zurueck, Ueberspringen, Resume");
    {
        auto z = frischerZustand();
        bool v = false;
        juce::String g;
        state::setzeAssistentenschritt (z, kSchritt, state::Assistentenschritt::coverage, v, g);
        state::setzeAssistentenschritt (z, kSchritt, state::Assistentenschritt::finding, v, g);
        state::setzeAssistentenschritt (z, kSchritt, state::Assistentenschritt::evidence, v, g);
        const auto revisionVorher = z.assistent.revision;

        // ZURUECK geht auf die sichere Rueckkante.
        pruefe (state::assistentZurueck (z, v, g) && v
                    && z.assistent.schritt == state::Assistentenschritt::finding,
                juce::String ("M-58: Zurueck geht auf die Rueckkante: ") + g);
        pruefe (z.assistent.revision == revisionVorher + 1,
                "M-58: und hebt die Revision genau einmal");

        // UEBERSPRINGEN geht auf den naechsten Zustand.
        pruefe (state::assistentUeberspringen (z, v, g) && v
                    && z.assistent.schritt == state::Assistentenschritt::evidence,
                "M-58: Ueberspringen geht auf den naechsten Zustand");

        // RESUME an derselben belegten Stelle.
        state::Assistentenzustand fortsetzung {};
        pruefe (state::assistentResume (z, fortsetzung)
                    && fortsetzung.stepId == kSchritt
                    && fortsetzung.schritt == state::Assistentenschritt::evidence,
                "M-58: Resume setzt an derselben Stelle fort");
        const auto revisionNachResume = z.assistent.revision;
        state::Assistentenzustand nochmal {};
        state::assistentResume (z, nochmal);
        pruefe (z.assistent.revision == revisionNachResume,
                "M-58: Resume ist eine FRAGE - es aendert nichts und hebt keine Revision");

        // ABBRUCH ist terminal, und der Schritt bleibt stehen.
        pruefe (state::assistentAbbrechen (z, v, g) && ! z.assistent.offen
                    && z.assistent.gesetzt && z.assistent.stepId == kSchritt,
                "M-58: Verwerfen ist ein terminales Ereignis, kein Loeschen der Historie");
        state::Assistentenzustand nachTerminal {};
        pruefe (! state::assistentResume (z, nachTerminal),
                "M-58: ein terminaler Schritt wird nicht fortgesetzt");
        // Und ein zweiter Abbruch ist kein Abbruch mehr.
        bool v2 = false;
        pruefe (! state::assistentAbbrechen (z, v2, g) && ! v2,
                "M-58: ein zweiter Abbruch faellt");
    }

    // ═══════════════════════════════════════════════════════════════════
    // M-61 · drei_benannte_ergebnisse
    // ═══════════════════════════════════════════════════════════════════
    abschnitt ("M-61: drei benannte Ergebnisse, keine Leerzustaende");
    {
        auto z = frischerZustand();
        bool v = false;
        juce::String g;
        state::setzeAssistentenschritt (z, kSchritt, state::Assistentenschritt::coverage, v, g);
        for (auto e : { state::Assistentenergebnis::passageMessen,
                        state::Assistentenergebnis::routingBestaetigen,
                        state::Assistentenergebnis::keineAenderungEmpfohlen })
        {
            const auto vorher = z.assistent.revision;
            bool ve = false;
            pruefe (state::setzeAssistentenergebnis (z, e, ve, g) && ve
                        && z.assistent.ergebnis == e
                        && z.assistent.revision == vorher + 1,
                    juce::String ("M-61: ") + state::wort (e)
                        + " ist ein eigenes Ergebnis mit Objekt");
            // Der SCHRITT bleibt stehen — das Ergebnis ist kein Leerzustand.
            pruefe (z.assistent.gesetzt && z.assistent.offen,
                    juce::String ("M-61: und der Schritt bleibt offen"));
        }
        // Dasselbe Ergebnis zweimal ist ein No-op.
        const auto vorher = z.assistent.revision;
        bool ve = false;
        pruefe (state::setzeAssistentenergebnis (
                    z, state::Assistentenergebnis::keineAenderungEmpfohlen, ve, g)
                    && ! ve && z.assistent.revision == vorher,
                "M-61: dasselbe Ergebnis zweimal hebt keine Revision");
    }

    // ═══════════════════════════════════════════════════════════════════
    // M-60 · harte_gates_greifen_vor_der_gewichtung
    // ═══════════════════════════════════════════════════════════════════
    abschnitt ("M-60: harte Gates vor der Gewichtung");
    {
        const auto perfekt = [] (const char* id, bool vergleichbar, bool sicher,
                                 bool erfolglos)
        {
            state::Schrittkandidat k;
            k.findingId = juce::String (id);
            k.schritt = state::Assistentenschritt::finding;
            k.erwarteterNutzen = 1.0;
            k.intentRelevanz = 1.0;
            k.konfidenz = 1.0;
            k.reversibilitaet = 1.0;
            k.messkosten = 0.0;
            k.vergleichbar = vergleichbar;
            k.sicher = sicher;
            k.bereitsErfolglos = erfolglos;
            return k;
        };
        // Drei Kandidaten mit PERFEKTEM Nutzen, jeder mit genau einem
        // gerissenen Gate — und ein bescheidener, der alle drei haelt.
        std::vector<state::Schrittkandidat> liste {
            perfekt ("aaa", false, true, false),
            perfekt ("bbb", true, false, false),
            perfekt ("ccc", true, true, true),
        };
        auto bescheiden = perfekt ("ddd", true, true, false);
        bescheiden.erwarteterNutzen = 0.2;
        bescheiden.intentRelevanz = 0.2;
        bescheiden.konfidenz = 0.2;
        bescheiden.reversibilitaet = 0.2;
        bescheiden.messkosten = 0.8;
        liste.push_back (bescheiden);

        const auto geordnet = state::ordneSchritte (liste);
        pruefe (geordnet.size() == 1 && geordnet[0].findingId == "ddd",
                "M-60: die drei Gates entfernen ihre Kandidaten VOR der Gewichtung");
        pruefe (state::schrittrang (liste[0]) > state::schrittrang (bescheiden),
                "M-60: und zwar, obwohl ihr Rang der hoechste waere");

        // Bei Gleichstand die kleinere Kennung — eine stabile Wahl.
        auto a = perfekt ("bbb", true, true, false);
        auto b = perfekt ("aaa", true, true, false);
        const auto stabil = state::ordneSchritte ({ a, b });
        pruefe (stabil.size() == 2 && stabil[0].findingId == "aaa",
                "M-60: bei Gleichstand gewinnt die kleinere Kennung");
        // Und der Rang selbst ist eine Zahl in [0,1], auch bei Unsinn.
        auto unsinn = perfekt ("eee", true, true, false);
        unsinn.konfidenz = std::numeric_limits<double>::quiet_NaN();
        unsinn.messkosten = 5.0;
        const auto rang = state::schrittrang (unsinn);
        pruefe (rang >= 0.0 && rang <= 1.0,
                "M-60: NaN und Ausreisser vergiften den Rang nicht");
    }

    // ═══════════════════════════════════════════════════════════════════
    // Die Verdrahtung: derselbe Handgriff ueber den echten Prozessor
    // ═══════════════════════════════════════════════════════════════════
    abschnitt ("Verdrahtung: der echte EqCopilotProcessor");
    {
        auto p = prozessor();
        DirtyZaehler z;
        p->addListener (&z);

        pruefe (p->assistentStarten (kSchritt), "Verdrahtung: der Schritt beginnt");
        pruefe (z.nonParam == 1, "Verdrahtung: und meldet GENAU einmal Host-Dirty");
        pruefe (! p->assistentStarten (hex32 (0x5742)),
                "M-57 verdrahtet: ein zweiter Start wird abgewiesen");
        pruefe (z.nonParam == 1, "und ein abgewiesener Start meldet nichts");

        pruefe (p->assistentWeiter (state::Assistentenschritt::finding),
                "Verdrahtung: der Uebergang laeuft");
        pruefe (! p->assistentWeiter (state::Assistentenschritt::preview),
                "M-55 verdrahtet: `preview` ist auch hier unerreichbar");
        pruefe (! p->assistentWeiter (state::Assistentenschritt::verdict),
                "Verdrahtung: ein Sprung ueber vier Zustaende faellt");

        // M-61 verdrahtet.
        pruefe (p->assistentAntwort (state::Assistentenergebnis::routingBestaetigen),
                "M-61 verdrahtet: das benannte Ergebnis wird gesetzt");
        pruefe (p->assistentAusState().ergebnis
                    == state::Assistentenergebnis::routingBestaetigen,
                "M-61 verdrahtet: und steht im Main-State");

        // M-59: Save/Load und Rekonstruktion.
        juce::MemoryBlock gespeichert;
        p->getStateInformation (gespeichert);
        auto zweiter = std::make_unique<EqCopilotProcessor>();
        zweiter->setStateInformation (gespeichert.getData(),
                                      static_cast<int> (gespeichert.getSize()));
        const auto wieder = zweiter->assistentAusState();
        pruefe (wieder.gesetzt && wieder.stepId == kSchritt
                    && wieder.schritt == state::Assistentenschritt::finding
                    && wieder.ergebnis == state::Assistentenergebnis::routingBestaetigen,
                "M-59: der Schritt wird aus dem gespeicherten MainProject rekonstruiert");
        state::Assistentenzustand fortsetzung {};
        pruefe (zweiter->assistentFortsetzen (fortsetzung)
                    && fortsetzung.schritt == state::Assistentenschritt::finding,
                "M-59: und laesst sich an derselben Stelle fortsetzen");

        // Save/Load ist bytegleich ueber zwei Runden.
        juce::MemoryBlock zweitesMal;
        zweiter->getStateInformation (zweitesMal);
        pruefe (zweitesMal == gespeichert, "M-59: Save/Load ist bytegleich");

        // Der Abbruch: terminal, und danach kein Resume mehr.
        pruefe (p->assistentAbbrechen(), "M-58 verdrahtet: der Abbruch laeuft");
        state::Assistentenzustand nachTerminal {};
        pruefe (! p->assistentFortsetzen (nachTerminal),
                "M-58 verdrahtet: ein terminaler Schritt wird nicht fortgesetzt");
        pruefe (p->assistentAusState().gesetzt && ! p->assistentAusState().offen,
                "M-58 verdrahtet: und bleibt als Historie stehen");
        p->removeListener (&z);
    }

    // ═══════════════════════════════════════════════════════════════════
    // M-62 · der Assistent ruft den BESTEHENDEN Experimentweg
    // ═══════════════════════════════════════════════════════════════════
    //
    // „Der Assistent erzeugt KEINE neue Experimentfamilie; er ruft die
    // vorhandenen." Gemessen daran, dass er ohne den bestehenden Weg gar
    // nichts erreicht: ohne offenen Schritt faellt er, und mit offenem
    // Schritt haengt er vollstaendig an `beginneVersuch` — schlaegt das fehl
    // (kein eingefrorener Vergleichspegel, §15), traegt der Schritt KEINE
    // Versuchskennung. Ein Assistent mit eigenem Weg haette hier eine.
    abschnitt ("M-62: der Assistent hat keinen eigenen Experimentweg");
    {
        auto p = prozessor();
        pruefe (! p->assistentVersuchStarten (kBefund),
                "M-62: ohne offenen Schritt startet der Assistent nichts");
        pruefe (p->assistentStarten (kSchritt), "M-62: Vorbedingung - ein Schritt laeuft");
        // `beginneVersuch` verlangt einen eingefrorenen Vergleichspegel und
        // ein gesetztes Passagenfenster; beides gibt es hier nicht.
        pruefe (! p->assistentVersuchStarten (kBefund),
                "M-62: und ohne den bestehenden Weg auch mit offenem Schritt nicht");
        pruefe (p->assistentAusState().experimentId.isEmpty(),
                "M-62: der Schritt traegt KEINE Versuchskennung - der Assistent "
                "erzeugt keine eigene Experimentfamilie");
    }

    // ═══════════════════════════════════════════════════════════════════
    // Die Raender: jede Grenze von BEIDEN Seiten
    // ═══════════════════════════════════════════════════════════════════
    abschnitt ("Die Raender");
    {
        struct Fall { juce::var liste; const char* was; };
        juce::Array<juce::var> zuKurz;
        zuKurz.add (kSchritt);
        const Fall faelle[] = {
            { juce::var (zuKurz),                                    "eine Liste mit sieben fehlenden Werten" },
            { schrittListe ("nicht-hex", "coverage", 1, true),       "eine step_id, die keine hex32 ist" },
            { schrittListe (kSchritt, "neunter", 1, true),           "ein unbekannter Zustand" },
            { schrittListe (kSchritt, "coverage", 0, true),          "Revision 0" },
            { schrittListe (kSchritt, "coverage", -1, true),         "eine negative Revision" },
            { schrittListe (kSchritt, "coverage", 1, true, "sonst"), "ein unbekanntes Ergebnis" },
            { schrittListe (kSchritt, "preview", 1, true),           "ein gespeicherter preview-Schritt" },
        };
        for (const auto& f : faelle)
        {
            juce::String grund;
            pruefe (wirdReadOnly (baumMitSchritt (f.liste), grund),
                    juce::String ("Rand: ") + f.was + " wird read-only");
        }
        // Die GEGENPROBE: der gueltige Stand laedt.
        state::Zustand ok;
        juce::String grundOk;
        const auto gut = baumMitSchritt (schrittListe (kSchritt, "listen", 4, false,
                                                       "keine_aenderung_empfohlen"));
        pruefe (laedtNormal (gut, ok, grundOk) && ok.assistent.gesetzt
                    && ok.assistent.schritt == state::Assistentenschritt::listen
                    && ok.assistent.revision == 4 && ! ok.assistent.offen
                    && ok.assistent.ergebnis
                           == state::Assistentenergebnis::keineAenderungEmpfohlen,
                juce::String ("Rand: der gueltige Stand laedt: ") + grundOk);

        // Ein Stand OHNE die Eigenschaft laedt und traegt keinen Schritt —
        // „noch nie einen Assistenten benutzt" ist etwas anderes als „ein
        // Schritt mit leeren Feldern".
        state::Zustand leer;
        juce::String grundLeer;
        const auto ohne = alsBlock (mainBaum());
        pruefe (laedtNormal (ohne, leer, grundLeer) && ! leer.assistent.gesetzt,
                "Rand: ein Altstand ohne die Eigenschaft laedt normal und traegt keinen Schritt");
        // Und er schreibt sie auch nicht.
        juce::MemoryBlock zurueck;
        state::speichere (leer, zurueck);
        juce::MemoryInputStream ein (zurueck, false);
        const auto baum = juce::ValueTree::readFromStream (ein);
        pruefe (! baum.getChildWithName ("MainProject").hasProperty ("assistant_step_v1"),
                "Rand: ein nicht gesetzter Schritt reist gar nicht");
    }

    // ═══════════════════════════════════════════════════════════════════
    // SONDE-014 ETAPPE I · M-71 und M-81 am gespeicherten Stand
    // ═══════════════════════════════════════════════════════════════════

    abschnitt ("M-71: der Main-State haelt von Store-Objekten NUR IDs");
    {
        // §33.5 woertlich: `Evidenz, Findings, Proposals,
        // Transaktionsereignisse, Experimente und Urteile` gehoeren in den
        // lokalen SQLite-Experimentstore, und der Main-State haelt davon
        // `nur kompakte aktuelle IDs`. Der Assistentenschritt zeigt auf drei
        // solche Objekte - Befund, Vorschlag, Versuch -, und genau deshalb
        // faellt die Zeile hier: ein Schritt, der die OBJEKTE mitnaehme,
        // waere die zweite Wahrheit, die §33.5 verbietet.
        auto p = prozessor();
        const auto schrittId = juce::String ("00000000000000000000000000000a01");
        const auto quelleId  = juce::String ("000000000000000000000000000000a1");
        // Der PERSISTENTE Weg, nicht der Wire-Weg: `assistentStarten` faehrt
        // die Zustandsmaschine unter dem Bindungsschloss und schreibt in
        // `MainProject`. `setzeAssistentSchritt` daneben ist der Spiegelweg
        // zum Broker und persistiert nichts - ein Fall, der IHN naehme,
        // maesse einen leeren Stand.
        pruefe (p->assistentStarten (schrittId),
                "M-71: ein Schritt liegt im Main-State");
        pruefe (p->setzeQuellenrolle (quelleId, {}, state::Rolle::fuehrt,
                                      state::IntentHerkunft::user, 1.0),
                "M-71: und die Sitzung traegt einen Intent");

        juce::MemoryBlock gespeichert;
        p->getStateInformation (gespeichert);
        // ⚠️ ROHE Bytes, kein `juce::String`. Der serialisierte `ValueTree`
        // traegt Laengenpraefixe und NUL-Bytes; eine Zeichenkette daraus
        // endete am ersten NUL, und jede Suche darin waere gruen, weil sie
        // nach dem ersten Kilobyte nichts mehr sieht.
        const std::string_view bytes (static_cast<const char*> (gespeichert.getData()),
                                      static_cast<size_t> (gespeichert.getSize()));
        const auto enthaelt = [&bytes] (const juce::String& wort)
        {
            return bytes.find (wort.toRawUTF8()) != std::string_view::npos;
        };

        // Die Gegenprobe ZUERST: die drei Kennungen stehen wirklich drin.
        // Ohne sie waere der Riegel darunter auch an einem leeren Stand gruen.
        pruefe (enthaelt (schrittId) && enthaelt (quelleId),
                "M-71: beide Kennungen stehen im gespeicherten Stand");

        // Und KEIN Feld eines Store-Objekts. Die Woerter sind die
        // Pflichtfelder aus `$defs/session_finding` und `$defs/proposal` -
        // taeuchte eines davon auf, laege ein ganzes Objekt im Main-State.
        const char* fremdfelder[] = {
            "claim_class", "ursachenklasse", "likely_cause", "smallest_test",
            "evidence_ids", "band_hz", "allowed_bounds", "expected_effect",
            "stop_if", "protected_traits", "beobachtung", "confidence"
        };
        bool sauber = true;
        juce::String getroffen;
        for (const auto* w : fremdfelder)
            if (enthaelt (juce::String (w))) { sauber = false; getroffen = w; }
        pruefe (sauber,
                "M-71: kein Feld eines Store-Objekts liegt im Main-State - "
                + (getroffen.isEmpty() ? juce::String ("keins gefunden") : getroffen));

        // Und der Baum traegt keine Kinder fuer Store-Objekte.
        juce::MemoryInputStream ein (gespeichert, false);
        const auto baum = juce::ValueTree::readFromStream (ein);
        const auto mp = baum.getChildWithName ("MainProject");
        bool eigeneKinder = true;
        for (const char* k : { "Findings", "Proposals", "Evidence", "Experiments" })
            eigeneKinder = eigeneKinder && ! mp.getChildWithName (k).isValid();
        pruefe (eigeneKinder && mp.isValid(),
                "M-71: `MainProject` hat kein Kind fuer ein Store-Objekt");
    }

    abschnitt ("M-81: State bleibt verlustfrei, auch mit den neuen Eigenschaften");
    {
        // Eine unbekannte EIGENSCHAFT in einem bekannten Kind desselben
        // Majors ist additiv und wird erhalten - neben den beiden neuen
        // Eigenschaften dieses Tickets, nicht statt ihrer. Genau diese
        // Kombination gab es bis Etappe I nirgends: B27 misst sie mit dem
        // Intent, B29 mit dem Schritt, keiner mit beiden.
        auto v = mainBaum();
        auto mp = v.getChildWithName ("MainProject");
        mp.setProperty ("eine_spaetere_fassung_v9", "unbekannt, aber meins", nullptr);
        mp.setProperty ("assistant_step_v1",
                        schrittListe ("00000000000000000000000000000b01", "listen", 4, true),
                        nullptr);
        const auto block = alsBlock (v);

        state::Zustand z;
        juce::String grund;
        pruefe (laedtNormal (block, z, grund),
                "M-81: ein Stand mit unbekannter Eigenschaft UND beiden neuen laedt normal - "
                + grund);
        pruefe (z.assistent.gesetzt
                    && z.assistent.schritt == state::Assistentenschritt::listen,
                "M-81: der Schritt kommt an");

        juce::MemoryBlock zurueck;
        state::speichere (z, zurueck);
        juce::MemoryInputStream ein (zurueck, false);
        const auto neu = juce::ValueTree::readFromStream (ein);
        const auto neuMp = neu.getChildWithName ("MainProject");
        pruefe (neuMp.getProperty ("eine_spaetere_fassung_v9").toString()
                    == "unbekannt, aber meins",
                "M-81: und die unbekannte Eigenschaft ueberlebt das Speichern");
        pruefe (neuMp.hasProperty ("assistant_step_v1"),
                "M-81: der Schritt daneben ebenso");

        // Ein unbekanntes KIND ist etwas anderes: read-only mit
        // Originalbytes. Die Trennung ist die Zusage.
        auto v2 = mainBaum();
        v2.appendChild (juce::ValueTree ("EinGanzNeuesKind"), nullptr);
        juce::String grund2;
        pruefe (wirdReadOnly (alsBlock (v2), grund2),
                "M-81: ein unbekanntes KIND laedt read-only - " + grund2);
    }

    // ===================================================================
    // NACHARBEIT 1 (07.09.2026) - NR-08: EINE Wahrheit des Schritts
    // ===================================================================
    //
    // Der Sender las bis hierher einen eigenen Schatten mit EIGENER
    // Revisionszaehlung, waehrend `setStateInformation()` ausschliesslich
    // `zustand.assistent` restauriert. Der bestehende Recall-Test prueft
    // Main-State und Speicherbytes - nicht diese Sendeseite.
    abschnitt ("NR-08: der Sender liest den RESTAURIERTEN Main-State");
    {
        auto p = prozessorAmDraht();
        const auto schrittId = juce::String ("00000000000000000000000000000b01");
        pruefe (p->assistentStarten (schrittId), "NR-08: ein Schritt liegt im Main");
        pruefe (p->assistentWeiter (state::Assistentenschritt::finding),
                "NR-08: und geht einen Uebergang");
        const auto vorher = p->v3AssistantStepFuerTest();
        pruefe (vorher.find (schrittId.toStdString()) != std::string::npos,
                "NR-08: die Wire-Nachricht traegt den Schritt");

        juce::MemoryBlock gespeichert;
        p->getStateInformation (gespeichert);

        // Der NEUE Prozessor - genau die Reprofolge aus EP-08.
        auto q = std::make_unique<EqCopilotProcessor>();
        q->setStateInformation (gespeichert.getData(),
                                static_cast<int> (gespeichert.getSize()));
        q->setzeEditorOffen (true);
        q->setzeBindung ("hub", {}, {});
        const auto nachher = q->v3AssistantStepFuerTest();
        pruefe (! nachher.empty(),
                "NR-08: nach dem Reload ist die Wire-Nachricht NICHT leer");
        pruefe (nachher.find (schrittId.toStdString()) != std::string::npos,
                "NR-08: sie traegt die restaurierte step_id");
        pruefe (nachher.find ("\"schritt\":\"finding\"") != std::string::npos,
                "NR-08: und den restaurierten Schritt, nicht den Anfangszustand");
        // Die Revision stammt aus dem MainProject, nicht aus einer zweiten
        // Zaehlung: sie ist dieselbe wie vor dem Speichern.
        const auto revisionAusState = q->assistentAusState().revision;
        pruefe (nachher.find ("\"revision\":" + std::to_string ((long long) revisionAusState))
                    != std::string::npos,
                "NR-08: die Revision kommt aus dem MainProject - "
                + juce::String (revisionAusState));
        pruefe (revisionAusState == p->assistentAusState().revision,
                "NR-08: und sie ist dieselbe wie vor dem Speichern");

        // Zweiter Abschnitt: ein Reload im SELBEN Prozessor sendet nie den
        // Vorgaengerschritt. Der Schatten haette hier den alten getragen.
        const auto zweiteId = juce::String ("00000000000000000000000000000b02");
        auto r = prozessorAmDraht();
        pruefe (r->assistentStarten (zweiteId), "NR-08: der Vorgaengerschritt laeuft");
        const auto leer = alsBlock (mainBaum());
        r->setStateInformation (leer.getData(), static_cast<int> (leer.getSize()));
        const auto nachLeerem = r->v3AssistantStepFuerTest();
        pruefe (nachLeerem.find (zweiteId.toStdString()) == std::string::npos,
                "NR-08: nach dem Reload eines Standes OHNE Schritt reist der "
                "Vorgaenger nicht mehr");
        pruefe (nachLeerem.empty(),
                "NR-08: der Sender schweigt, statt einen Schritt zu erfinden");
    }

    // ===================================================================
    // NACHARBEIT 1 - NR-10: das Userurteil auf dem Draht (M-73, E-09)
    // ===================================================================
    abschnitt ("NR-10: `assistentAntwort` erzeugt genau EIN `user_verdict`");
    {
        auto p = prozessorAmDraht();
        const auto schrittId = juce::String ("00000000000000000000000000000c01");
        const auto findingId = juce::String ("00000000000000000000000000000c0f");
        pruefe (p->assistentStarten (schrittId), "NR-10: ein Schritt laeuft");

        // OHNE Befund entsteht kein Urteil: ein Urteil ohne Gegenstand waere
        // ein Objekt ohne Bezug, und ein erfundener Bezug waere schlimmer.
        pruefe (p->v3UserVerdictFuerTest (state::Userurteil::angenommen).empty(),
                "NR-10: ohne `finding_id` entsteht kein Urteil");
        pruefe (p->v3UserVerdictFuerTest (state::Userurteil::angenommen, "kein hex32").empty(),
                "NR-10: und eine Kennung ausserhalb hex32 ebenso wenig");

        // MIT Befund: das Objekt traegt Kopf, Kennung, Befund und Urteil.
        const auto urteil = p->v3UserVerdictFuerTest (state::Userurteil::angenommen,
                                                      findingId, "klingt offener");
        pruefe (! urteil.empty(), "NR-10: mit `finding_id` entsteht ein Objekt");
        pruefe (urteil.find ("\"type\":\"user_verdict\"") != std::string::npos,
                "NR-10: es ist ein `user_verdict`");
        pruefe (urteil.find ("\"finding_id\":\"" + findingId.toStdString() + "\"")
                    != std::string::npos,
                "NR-10: und bindet an DEN Befund, ueber den geurteilt wurde");
        pruefe (urteil.find ("\"urteil\":\"angenommen\"") != std::string::npos,
                "NR-10: das Urteil steht als Vertragswort darin");
        pruefe (urteil.find ("\"notiz\":") != std::string::npos,
                "NR-10: die Notiz reist unveraendert mit (User-Wort, nie interpretiert)");
        pruefe (urteil.find ("\"command_id\"") != std::string::npos,
                "NR-10: es traegt einen Steuerkopf - es ist ein P0-Befehl (E-09)");
        // Zwei Urteile tragen VERSCHIEDENE Kennungen: sie koalesziert nichts,
        // und zwei Aussagen des Users sind zwei Objekte.
        const auto zweites = p->v3UserVerdictFuerTest (state::Userurteil::abgelehnt,
                                                       findingId);
        pruefe (! zweites.empty() && zweites != urteil,
                "NR-10: ein zweites Urteil ist ein EIGENES Objekt");
        // Ohne Notiz entsteht kein Feld: Abwesenheit heisst „keine Notiz",
        // nie `null`.
        pruefe (zweites.find ("\"notiz\":") == std::string::npos,
                "NR-10: ohne Notiz reist das Feld gar nicht");

        // Die geschlossene Menge der vier Urteile (M-77): jedes Wort steht
        // genau einmal, und ein fuenftes gibt es nicht.
        const char* worte[] = { "angenommen", "abgelehnt", "spaeter", "enthaltung" };
        const state::Userurteil werte[] = {
            state::Userurteil::angenommen, state::Userurteil::abgelehnt,
            state::Userurteil::spaeter, state::Userurteil::enthaltung
        };
        bool alleWorte = true;
        for (size_t i = 0; i < 4; ++i)
            alleWorte = alleWorte && juce::String (state::wort (werte[i])) == worte[i];
        pruefe (alleWorte, "NR-10: die vier Urteile tragen ihre Vertragswoerter");
    }

    // ===================================================================
    // NACHARBEIT 2 - WN-01: der Kopf traegt die GEMELDETE Revision
    // ===================================================================
    //
    // `assistentAenderungMelden` erhoeht `v3StateRevision`, und der Kopf des
    // unmittelbar danach gesendeten `user_verdict` uebernahm die NEUE Zahl.
    // Der Broker kennt bis zum naechsten `state_report` aber noch die alte:
    // das Urteil reist als P0 und ueberholt den Bericht (P1) strukturell,
    // `befehl.rs` antwortet `revision_conflict`, und das unpersistierte
    // Urteil war fort. Der Kopf nimmt seither die zuletzt GEMELDETE Revision.
    abschnitt ("WN-01: `base_revision` kommt aus dem Draht, nicht aus dem Zaehler");
    {
        auto p = prozessorAmDraht();
        const auto schrittId = juce::String ("00000000000000000000000000000d01");
        const auto findingId = juce::String ("00000000000000000000000000000d0f");
        pruefe (p->assistentStarten (schrittId), "WN-01: ein Schritt laeuft");
        // Der Schritt hat den lokalen Zaehler bewegt - ohne Verbindung ist
        // aber NICHTS gemeldet worden.
        const auto lokal = p->v3StateRevisionFuerTest();
        pruefe (lokal > 0, "WN-01: der lokale Zaehler ist gestiegen");
        const auto urteil = p->v3UserVerdictFuerTest (state::Userurteil::angenommen, findingId);
        pruefe (! urteil.empty(), "WN-01: das Urteil entsteht");
        pruefe (urteil.find ("\"base_revision\":0") != std::string::npos,
                "WN-01: der Kopf traegt die GEMELDETE Revision (0), nicht den Zaehler");
        pruefe (urteil.find ("\"base_revision\":" + std::to_string (lokal)) == std::string::npos,
                "WN-01: und ausdruecklich NICHT den soeben erhoehten lokalen Stand");

        // Der Weg zurueck aus einem `konflikt`-ACK: derselbe Auftrag, frischer
        // Kopf, DIESELBE `command_id` (idempotent, NR-10).
        const auto anfang = urteil.find ("\"command_id\":\"");
        const auto commandId = anfang == std::string::npos
            ? std::string {}
            : urteil.substr (anfang + 14, 32);
        pruefe (commandId.size() == 32, "WN-01: das Urteil traegt seine Kennung");
        const auto wiederholt = p->urteilMitFrischemKopfFuerTest (juce::String (commandId), 7);
        pruefe (! wiederholt.empty(),
                "WN-01: ein `konflikt` ist kein Verlust - der Auftrag entsteht neu");
        pruefe (wiederholt.find ("\"command_id\":\"" + commandId + "\"") != std::string::npos,
                "WN-01: unter DERSELBEN `command_id`");
        pruefe (wiederholt.find ("\"base_revision\":7") != std::string::npos,
                "WN-01: und mit der Revision, die der Broker genannt hat");
        pruefe (wiederholt.find ("\"finding_id\":\"" + findingId.toStdString() + "\"")
                    != std::string::npos,
                "WN-01: der Gegenstand bleibt derselbe Befund");
        // Eine fremde Kennung kennt der Mitschnitt nicht - er erfindet nichts.
        pruefe (p->urteilMitFrischemKopfFuerTest ("00000000000000000000000000000fff", 7).empty(),
                "WN-01: eine fremde `command_id` erzeugt kein Urteil");
    }

    // ===================================================================
    // NACHARBEIT 2 - WN-05: eine abgewiesene Einreihung meldet FALSE
    // ===================================================================
    //
    // Bei voller 64er-P0-Queue liefert `sendePersistenzP0` false und die
    // Verbindung wird verworfen (M-73). `assistentAntwort` ignorierte den
    // Rueckgabewert und meldete weiter Erfolg - das Urteil war weder
    // persistiert noch wiederholt (WP1-5).
    abschnitt ("WN-05: der Rueckgabewert der Queue-Politik wird ausgewertet");
    {
        auto p = prozessorAmDraht();
        const auto schrittId = juce::String ("00000000000000000000000000000e01");
        const auto findingId = juce::String ("00000000000000000000000000000e0f");
        pruefe (p->assistentStarten (schrittId), "WN-05: ein Schritt laeuft");
        const auto urteil = state::Userurteil::angenommen;

        // Die GEGENPROBE zuerst: mit Platz in der Queue meldet die Methode
        // Erfolg. Ohne sie waere das `false` unten nicht von "hier geht
        // ohnehin nichts" zu unterscheiden.
        p->leereP0QueueFuerTest();
        pruefe (p->assistentAntwort (state::Assistentenergebnis::schritt,
                                     &urteil, findingId),
                "WN-05: mit Platz in der Queue meldet `assistentAntwort` Erfolg");

        // Und jetzt voll: die Einreihung wird abgewiesen, und die Methode
        // sagt es.
        const auto gefuellt = p->fuelleP0QueueFuerTest();
        pruefe (gefuellt > 0, "WN-05: die P0-Queue ist wirklich voll");
        pruefe (! p->assistentAntwort (state::Assistentenergebnis::passageMessen,
                                       &urteil, findingId),
                "WN-05: bei voller Queue meldet `assistentAntwort` KEINEN Erfolg");
        p->leereP0QueueFuerTest();
    }

    // ===================================================================
    // NACHARBEIT 3 - WN3-01: der Mitschnitt haengt an der `command_id`
    // ===================================================================
    //
    // Die Runde 2 hielt GENAU EINEN Mitschnitt, mit der Begruendung „zwei
    // gleichzeitig offene gibt es nicht". Erzwungen war das nirgends: schon
    // zwei Urteile vor dem ersten ACK ueberschrieben den ersten Auftrag, und
    // sein `konflikt`-ACK lief danach in einen leeren Text - der
    // ControlClient entfernte das unpersistierte Urteil endgueltig (WP2-1,
    // Bruch von M-73). Dieser Abschnitt misst den ECHTEN Weg:
    // `urteilMitFrischemKopf` ist der Rumpf des Hooks, den der Prozessor im
    // Konstruktor an `setzeKonfliktWiederholungHook` gibt.
    abschnitt ("WN3-01: zwei ausstehende Urteile, zwei Mitschnitte");
    {
        auto p = prozessorAmDraht();
        pruefe (p->assistentStarten ("00000000000000000000000000000f01"),
                "WN3-01: ein Schritt laeuft");

        const auto ersterBefund  = juce::String ("00000000000000000000000000000fa1");
        const auto zweiterBefund = juce::String ("00000000000000000000000000000fb2");
        const auto kennung = [] (const std::string& urteil) -> std::string
        {
            const auto anfang = urteil.find ("\"command_id\":\"");
            return anfang == std::string::npos ? std::string {}
                                               : urteil.substr (anfang + 14, 32);
        };

        // ZWEI Urteile, keines quittiert - genau die Lage aus WP2-1.
        const auto ersteres = p->v3UserVerdictFuerTest (state::Userurteil::angenommen,
                                                        ersterBefund, "erstes");
        const auto zweiteres = p->v3UserVerdictFuerTest (state::Userurteil::abgelehnt,
                                                         zweiterBefund, "zweites");
        const auto id1 = kennung (ersteres);
        const auto id2 = kennung (zweiteres);
        pruefe (id1.size() == 32 && id2.size() == 32 && id1 != id2,
                "WN3-01: beide Urteile tragen ihre eigene Kennung");
        pruefe (p->urteilMitschnitteFuerTest() == 2,
                "WN3-01: und beide stehen im Register");

        // 🔑 Die Zusage: der `konflikt`-ACK auf das ERSTE Urteil wiederholt
        // das ERSTE - unter seiner eigenen Kennung und mit seinem eigenen
        // Befund. Vor WN3-01 war der Mitschnitt hier vom zweiten
        // ueberschrieben, und diese Zeile lieferte einen leeren Text.
        const auto wiederholt1 = p->urteilMitFrischemKopfFuerTest (juce::String (id1), 11);
        pruefe (! wiederholt1.empty(),
                "WN3-01: das ERSTE Urteil ist nach dem zweiten noch wiederholbar");
        pruefe (wiederholt1.find ("\"command_id\":\"" + id1 + "\"") != std::string::npos,
                "WN3-01: unter DERSELBEN `command_id`");
        pruefe (wiederholt1.find ("\"finding_id\":\"" + ersterBefund.toStdString() + "\"")
                    != std::string::npos,
                "WN3-01: und mit SEINEM Befund, nicht dem des zweiten");
        pruefe (wiederholt1.find ("\"base_revision\":11") != std::string::npos,
                "WN3-01: mit der Revision aus dem ACK");
        pruefe (wiederholt1.find ("\"urteil\":\"angenommen\"") != std::string::npos
                    && wiederholt1.find ("\"urteil\":\"abgelehnt\"") == std::string::npos,
                "WN3-01: und mit SEINEM Urteil, nicht dem des zweiten");

        // Das zweite bleibt davon unberuehrt - eine Wiederholung ist kein
        // Verbrauch.
        const auto wiederholt2 = p->urteilMitFrischemKopfFuerTest (juce::String (id2), 12);
        pruefe (! wiederholt2.empty() && wiederholt2.find (zweiterBefund.toStdString())
                                             != std::string::npos,
                "WN3-01: das zweite Urteil bleibt unberuehrt");
        pruefe (p->urteilMitschnitteFuerTest() == 2,
                "WN3-01: und das Register haelt weiter genau zwei");

        // ── Die GEGENPROBE: erst der Abschluss gibt frei ─────────────────
        //
        // `angewandt`, `idempotent_wiederholt` oder endgueltiger Fehlschlag -
        // der ControlClient ruft dann `setzeAuftragAbgeschlossenHook`, und
        // genau dieser Weg laeuft hier ohne Draht.
        p->urteilAbgeschlossenFuerTest (juce::String (id1));
        pruefe (p->urteilMitschnitteFuerTest() == 1,
                "WN3-01: der Abschluss gibt den Mitschnitt frei");
        pruefe (p->urteilMitFrischemKopfFuerTest (juce::String (id1), 13).empty(),
                "WN3-01: ein abgeschlossener Auftrag wird nicht mehr wiederholt");
        pruefe (! p->urteilMitFrischemKopfFuerTest (juce::String (id2), 13).empty(),
                "WN3-01: und der andere ist davon unberuehrt");
        p->urteilAbgeschlossenFuerTest ("00000000000000000000000000000fff");
        pruefe (p->urteilMitschnitteFuerTest() == 1,
                "WN3-01: eine fremde Kennung raeumt nichts weg - derselbe Hook "
                "gilt fuer JEDEN persistenzpflichtigen P0");

        // ── Der DECKEL: so viele Plaetze wie die P0-Queue ────────────────
        //
        // Mehr ausstehende persistenzpflichtige Auftraege kann es nicht
        // geben; faellt der Deckel doch, geht der AELTESTE - der, dessen ACK
        // am laengsten aussteht.
        std::string aeltester;
        for (std::size_t i = 0; i < nakama::ipc::kCapP0 + 4; ++i)
        {
            const auto text = p->v3UserVerdictFuerTest (state::Userurteil::angenommen,
                                                        ersterBefund);
            if (aeltester.empty())
                aeltester = kennung (text);
        }
        pruefe (p->urteilMitschnitteFuerTest() == nakama::ipc::kCapP0,
                "WN3-01: das Register waechst nie ueber die P0-Kapazitaet");
        pruefe (p->urteilMitFrischemKopfFuerTest (juce::String (aeltester), 14).empty(),
                "WN3-01: verdraengt wird der AELTESTE");
    }

    std::cout << std::endl << "SONDE-014 AssistantStep: " << bestanden << "/"
              << (bestanden + fehler) << " gruen" << std::endl;
    return fehler == 0 ? 0 : 1;
}
