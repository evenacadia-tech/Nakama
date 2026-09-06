#pragma once

/*  State-Schema 2 `NakamaState` - Leser, Schreiber, Schema-1-Migrator
    (SONDE-006; Vertrag: eq-copilot/schemas/state/nakama-state-v2.md).

    Drei Regeln, die hier Code sind:

      1. Kein Teilstate. Eine unzulaessige Kombination (Root-Major, Kind-
         Major, Kind-Matrix, Klasse ausserhalb des Bundles) macht den GANZEN
         State read-only; die Originalbytes reisen unveraendert zurueck zum
         Host (§53.8).
      2. Reine Migration. Schema 1 -> 2 haengt nur von den Eingabebytes ab -
         Goldens in fixtures/state/schema2/ beweisen das bytegleich.
      3. Unbekanntes Bekanntes ueberlebt. Eine zusaetzliche Eigenschaft in
         einem bekannten Kind desselben Majors wird gehalten und zurueck-
         geschrieben (§33.5 "additive bekannte Felder bleiben erhalten").

    Keine `JucePlugin_*`-Konstante, nur juce_core + juce_data_structures -
    S8 (gemeinsamer Kern) kann das unveraendert uebernehmen. Nicht fuer den
    Audiothread.
*/

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>

#include <vector>

#include "NakamaParameter.h"

namespace nakama::state
{

enum class Klasse       { main, passive_probe, active_probe, legacy };
enum class Messposition { insert, pre, post, post_fader_contribution };

/** Woher der gehaltene Zustand stammt - der Editor zeigt es, Tests messen es. */
enum class Herkunft     { frisch, schema1Migriert, schema2Geladen, nurLesen };

enum class LadeErgebnis { geladen, migriert, nurLesen, ignoriert };

const char* wort (Klasse k);
const char* wort (Messposition p);
bool klasseAusWort   (const juce::String& w, Klasse& aus);
bool positionAusWort (const juce::String& w, Messposition& aus);

/** §2.2 des Vertrags: welche Messposition eine Klasse tragen darf. */
bool positionErlaubt (Klasse k, Messposition p);

/** Common schema=1 - die Identitaet jeder Instanz (§32.1/32.2). */
struct Common
{
    juce::String instanceId;                         ///< bytegleich aus sensor_id; frisch: juce::Uuid
    Klasse       klasse   = Klasse::legacy;
    Messposition position = Messposition::insert;
    juce::String label;                              ///< User-Wort, nie interpretiert
    juce::String pairId;                             ///< leer = kein Paar
    juce::String projectBindingId;                   ///< leer = nicht gebunden (nie erfunden)

    bool operator== (const Common& a) const noexcept;
    bool operator!= (const Common& a) const noexcept { return ! (*this == a); }
};

/** Persistente MainProject-Wahrheit einer bestaetigten Quelle (SONDE-012).

    Nur stabile Identitaet und das vom User vergebene, untrusted Label reisen
    im Host-State. Control-Liveness, Runtime-Nonce, Descriptor und Messframes
    bleiben absichtlich ausserhalb dieses Typs. */
struct MainProjectMitglied
{
    juce::String instanceId;  ///< effektive, stabile hex32-Quellidentitaet
    juce::String label;       ///< User-Wort, hoechstens 120 Codepoints

    bool operator== (const MainProjectMitglied& a) const noexcept
    { return instanceId == a.instanceId && label == a.label; }
};

inline constexpr int maxMainProjectMitglieder = 64;

/** Eine vom User markierte Passage als PROJEKTINTENT (SONDE-013 M-25, M-69).

    §33.5 teilt die Wahrheit auf: der Store ist autoritativ fuer die Passage
    als Evidenzobjekt, `MainProjectState` haelt sie als Intent, der mit der
    FL-Projektdatei reist. Dieser Typ ist deshalb absichtlich klein.

    Was hier NICHT steht, und warum: Fingerprint, aktives Quellenset,
    Abdeckung und Transportepoche sind MESSERGEBNISSE. Sie im Host-State zu
    fuehren hiesse, dass ein Projekt nach dem Loeschen der Datenbank weiter
    behauptet, es gebe Evidenz - M-32 verlangt genau das Gegenteil: "Fehlt
    oder wird die SQLite-Datenbank geloescht, bleiben Projekt-Recall und DSP
    vollstaendig - nur historische Passagen und Experimente fehlen."

    Dieselbe Trennung, die `MainProjectMitglied` schon vormacht: stabile
    Identitaet und User-Wort ja, Liveness und Messframes nein. */
struct ManuellePassage
{
    juce::String passageId;      ///< stabile hex32-Identitaet, nie erfunden
    juce::String label;          ///< User-Wort, hoechstens 120 Codepoints, nie interpretiert
    juce::int64  projektStart = 0;  ///< Projektsamples, halboffen `[start, ende)`
    juce::int64  projektEnde  = 0;

    bool operator== (const ManuellePassage& a) const noexcept
    {
        return passageId == a.passageId && label == a.label
            && projektStart == a.projektStart && projektEnde == a.projektEnde;
    }
};

/*  Dieselbe feste Obergrenze wie fuer die bestaetigten Quellen. Beide reisen
    im selben Host-State, und §48.1 verlangt feste Obergrenzen statt einer
    Liste, die mit dem Projekt waechst. */
inline constexpr int maxManuellePassagen = 64;

/*  ── SONDE-014 Etappe A: der musikalische Intent einer Quelle ──────────────

    Entwurf §37.1 gibt dem `SourceIntent` sechs Teile; Antwort U22 vom
    06.09.2026 legt darueber die Produktzusage: eine Quelle kann GENAU FUENF
    musikalische Rollen bekommen. Die Abbildung der fuenf Rollen auf die
    Belegung des Entwurfs ist Technik dieses Tickets (Entscheid E-01/E-01a,
    Matrixzeile M-02) und steht als `belegung()` weiter unten.

    Was hier NICHT gespeichert wird, und warum: `prominence`, Funktionstag und
    Veto-Kennzeichen sind ABGELEITET. Sie getrennt zu persistieren hiesse,
    zwei Wahrheiten zu fuehren - genau das verbietet §33.5. Der Rueckweg
    `rolleAus (Belegung)` ist deshalb eindeutig, und der Test misst genau das:
    Roundtrip ueber alle fuenf plus die Probe, dass zwei verschiedene
    Belegungen nie dieselbe Rolle zurueckgeben. */

/** Die geschlossene Rollenmenge (U22, 06.09.2026). Genau fuenf Werte. */
enum class Rolle { fuehrt, traegt, begleitet, geschuetzt, verschmolzen };

/** §37.1 `prominence`. Abgeleitet aus der Rolle, nie getrennt gesetzt. */
enum class Prominenz { foreground, middle, background };

/** §37.1 Funktionstag. Abgeleitet; `keiner` ist der leere Tag. */
enum class Funktionstag { keiner, lead, foundation, texture };

/** Das Veto-Kennzeichen der Belegung (E-01a). Ohne es waere die Abbildung
    weder total noch injektiv: `geschuetzt` und `verschmolzen` liegen beide
    auf (`middle`, leerer Tag) und liessen sich nicht auseinanderhalten. */
enum class Veto { keins, schutz, verschmolzen };

/** Die abgeleitete §37.1-Belegung einer Rolle. */
struct Belegung
{
    Prominenz    prominenz = Prominenz::middle;
    Funktionstag tag       = Funktionstag::keiner;
    Veto         veto      = Veto::keins;

    bool operator== (const Belegung& a) const noexcept
    { return prominenz == a.prominenz && tag == a.tag && veto == a.veto; }
    bool operator!= (const Belegung& a) const noexcept { return ! (*this == a); }
};

/** §37.1: Herkunft eines Intentwerts. Eine abgeleitete Vermutung
    ueberschreibt einen Userwert NIE (M-07). */
enum class IntentHerkunft { user, vorlage, abgeleitet };

/** §37.1: geschuetzte Eigenschaften einer Quelle. Orthogonal zur Rolle
    (E-01a, M-03): eine Quelle mit Rolle `begleitet` kann Attack geschuetzt
    haben, und ein Rollenwechsel loescht keinen Schutzbereich.

    `band` traegt zusaetzlich ein halboffenes Bandintervall des bestehenden
    221er-Gitters; die uebrigen drei tragen keines und fuehren `bandVon` und
    `bandBis` auf -1. Es entsteht keine zweite Frequenzachse. */
enum class Schutzeigenschaft { attack, breite, ausklang, band };

/** §37.1: gerichtete Beziehungen zwischen zwei Quellen.

    `gleichrangig` ist kein Entwurfsbegriff, sondern der von §37.2 woertlich
    verlangte Speicherplatz: "ein Zyklus muss aufgeloest oder als nicht
    steuerbare Gleichrangigkeit gespeichert werden". Ohne einen eigenen Wert
    haette ein aufgeloester Zyklus keinen Ort und wuerde beim naechsten Laden
    wieder als gerichtet gelesen (M-06). */
enum class Beziehungsart { fuehrtVor, darfVerschmelzen, gleichrangig };

/** §37.2, Reihenfolge der Konfliktregeln. Eine GEORDNETE LISTE, kein Score:
    eine hoehere Stufe wird von keiner Summe niedrigerer geschlagen (M-05).
    Der kleinere Zahlenwert gewinnt. */
enum class Konfliktstufe
{
    schutzgrenze      = 1,   ///< 1. Schutz-/Sicherheitsgrenze
    userintent        = 2,   ///< 2. expliziter Userintent
    passagespezifisch = 3,   ///< 3. passagespezifischer vor globalem Intent
    vorlage           = 4,   ///< 4. bestaetigte Vorlage
    vermutung         = 5    ///< 5. abgeleitete Vermutung
};

inline constexpr int konfliktstufenAnzahl = 5;

/** Ein Anspruch, wie ihn die Konfliktaufloesung vergleicht. `spezifisch`
    trennt innerhalb der Stufe 2 die paarweise Beziehung (spezifisch) vom
    globalen Rollenveto (E-02, M-04/M-05). */
struct Anspruch
{
    Konfliktstufe stufe      = Konfliktstufe::vermutung;
    bool          spezifisch = false;
};

/** §37.1: ein `SourceIntent`. Genau ein Objekt je Quelle UND Scope (E-01);
    `passageId` leer heisst globaler Scope. */
struct SourceIntent
{
    juce::String   quelleId;               ///< effektive, stabile hex32-Quellidentitaet
    juce::String   passageId;              ///< leer = global, sonst hex32 (§37.1 Passage-Scope)
    Rolle          rolle = Rolle::traegt;
    juce::int64    revision = 1;           ///< je Objekt, streng steigend (§37.3)
    IntentHerkunft herkunft = IntentHerkunft::user;
    double         konfidenz = 1.0;        ///< [0,1], endlich

    bool operator== (const SourceIntent& a) const noexcept
    {
        return quelleId == a.quelleId && passageId == a.passageId
            && rolle == a.rolle && revision == a.revision
            && herkunft == a.herkunft && konfidenz == a.konfidenz;
    }
};

/** §37.1: ein geschuetzter Bereich beziehungsweise eine geschuetzte
    Eigenschaft. Orthogonal zur Rolle (M-03). */
struct Schutzangabe
{
    juce::String      quelleId;
    Schutzeigenschaft eigenschaft = Schutzeigenschaft::attack;
    int               bandVon = -1;   ///< nur bei `band`: Index im 221er-Gitter, sonst -1
    int               bandBis = -1;   ///< halboffen `[von, bis)`

    bool operator== (const Schutzangabe& a) const noexcept
    {
        return quelleId == a.quelleId && eigenschaft == a.eigenschaft
            && bandVon == a.bandVon && bandBis == a.bandBis;
    }
};

/** §37.1: eine gerichtete Beziehung zwischen zwei Quellen. */
struct IntentBeziehung
{
    juce::String  quelleA;
    juce::String  quelleB;
    Beziehungsart art = Beziehungsart::fuehrtVor;

    bool operator== (const IntentBeziehung& a) const noexcept
    { return quelleA == a.quelleA && quelleB == a.quelleB && art == a.art; }
};

/*  Feste Obergrenzen (§48.1). Sie sind aus dem Bestand gerechnet, nicht
    geraten: 64 bestaetigte Quellen mal (global + drei Passagenscopes) ist die
    Groesse, die eine Sitzung mit dem heutigen Passagendeckel realistisch
    erreicht; Schutzangaben und Beziehungen liegen in derselben Klasse. Wer
    mehr braucht, hebt die Zahl bewusst - keine Liste, die mit dem Projekt
    waechst. */
inline constexpr int maxSourceIntents     = 256;
inline constexpr int maxSchutzangaben     = 256;
inline constexpr int maxIntentBeziehungen = 256;

/** Der groesste Bandindex des Evidenzgitters `nakama_1_24_oct_30_18k_v1`
    (221 Baender). Die Zahl steht hier NICHT als zweites Gitter, sondern als
    Grenze: ein Schutzintervall, das darueber hinausgeht, benennt kein Band
    dieses Vertrags. */
inline constexpr int bandAnzahlEvidenzgitter = 221;

/** Die Zahl der Rollen ist eine PRODUKTZUSAGE (U22), keine Zaehlung. */
inline constexpr int rollenAnzahl = 5;

const char* wort (Rolle r);
const char* wort (Prominenz p);
const char* wort (Funktionstag t);
const char* wort (Veto v);
const char* wort (IntentHerkunft h);
const char* wort (Schutzeigenschaft e);
const char* wort (Beziehungsart a);

bool rolleAusWort             (const juce::String& w, Rolle& aus);
bool intentHerkunftAusWort    (const juce::String& w, IntentHerkunft& aus);
bool schutzeigenschaftAusWort (const juce::String& w, Schutzeigenschaft& aus);
bool beziehungsartAusWort     (const juce::String& w, Beziehungsart& aus);

/** E-01a, M-02: die ABGELEITETE §37.1-Belegung. Total und injektiv. */
Belegung belegung (Rolle r);

/** Der eindeutige Rueckweg. `false`, wenn die Belegung zu keiner Rolle
    gehoert - es gibt keine stille Naeherung auf einen bekannten Zweig. */
bool rolleAus (const Belegung& b, Rolle& aus);

/** §37.2, M-05: welcher von zwei Anspruechen gewinnt.
    Rueckgabe -1 = a gewinnt, +1 = b gewinnt, 0 = gleichrangig.
    Die Stufe entscheidet zuerst; innerhalb derselben Stufe gewinnt der
    spezifischere Anspruch (E-02). */
int vergleicheAnsprueche (const Anspruch& a, const Anspruch& b);

/** Welche Klassen ein Bundle laden darf (§2.3 des Vertrags). */
struct Bundle
{
    std::vector<Klasse> klassen;
    bool erlaubt (Klasse k) const noexcept;
    static Bundle eqcp()   { return { { Klasse::main, Klasse::legacy } }; }
    static Bundle nkpr()   { return { { Klasse::passive_probe } }; }
    static Bundle nkac()   { return { { Klasse::active_probe } }; }
};

// ═══════════════════════════════════════════════════════════════════════════
// SONDE-014 Etappe G: der `AssistantStep` (§46.1, M-55 bis M-62)
// ═══════════════════════════════════════════════════════════════════════════
//
// ── WARUM DIE ZUSTANDSMASCHINE HIER LIEGT ──────────────────────────────────
//
// Entscheid E-08 (§4.8): „Zustandsmaschine des `AssistantStep`: Uebergaenge,
// Abbruch, Zurueck, Ueberspringen, Resume, Rekonstruktion — **Main**,
// persistent im `MainProjectState`." `assistent.rs` im Broker ist Spiegel und
// Vertragsvalidierung, KEINE zweite Zustandsmaschine.
//
// ── PREVIEW BLEIBT, DIE KANTE NICHT (E-07, M-55) ───────────────────────────
//
// Die Zustandsmenge des Vertrags hat ACHT Werte, und `preview` ist einer
// davon. Die P5-Uebergangstabelle fuehrt aber KEINE Kante dorthin, und ein
// GESPEICHERTER Schritt mit `preview` ist in P5 ein **Lesefehler** — kein
// stiller Sprung auf `proposal` oder `remeasure`. Ihn aus der Menge zu
// streichen hiesse, ihn in P6 neu erfinden zu muessen; ihn still abzubilden
// hiesse, einen Zustand zu behaupten, den der User nie hatte.

/** §46.1: die acht Zustaende der Assistentenfolge. */
enum class Assistentenschritt
{
    coverage, finding, evidence, listen, proposal, preview, remeasure, verdict
};

/** Die Wireform eines Schritts, wortgleich mit `$defs/assistant_schritt`. */
const char* wort (Assistentenschritt s) noexcept;
/** Fail-closed: ein unbekanntes Wort ist `false`, nie ein Zweig „sonst". */
bool assistentenschrittAus (const juce::String& wort, Assistentenschritt& aus) noexcept;

/** Die FUENF Angaben eines Zustands (M-56). Keine ist optional.

    Ein Zustand ohne Rueckkante ist ein Vertragsbruch — deshalb ist
    `rueckkante` kein `optional`, sondern immer ein Zustand: der erste seiner
    Folge zeigt auf sich selbst, und genau das ist die sichere Rueckkante
    „bleib, wo du bist". */
struct Schrittvertrag
{
    Assistentenschritt schritt = Assistentenschritt::coverage;
    /** Eintrittsbedingung, benannt statt beschrieben. */
    const char*        eintritt = "";
    /** Ob dieser Zustand Evidenz-IDs traegt. */
    bool               traegtEvidenz = false;
    /** Die Useraktion, die ihn verlaesst. */
    const char*        useraktion = "";
    /** Timeout in Millisekunden. > 0, immer. */
    int                timeoutMs = 0;
    /** Die sichere Rueckkante. */
    Assistentenschritt rueckkante = Assistentenschritt::coverage;

    /** M-56: sind alle fuenf Angaben belegt? */
    bool vollstaendig() const noexcept;
};

/** Der Vertrag EINES Zustands. Tabelle, keine Rechnung — sie steht an einer
    Stelle, damit Anzeige, Automat und Leser dieselbe lesen. */
Schrittvertrag schrittvertrag (Assistentenschritt s) noexcept;

/** M-55: fuehrt die P5-Uebergangstabelle eine Kante von `von` nach `nach`?

    ⚠️ Die Folge ist `coverage → finding → evidence → listen → proposal →
    remeasure → verdict`. Es gibt KEINE Kante nach `preview` — weder hin noch
    zurueck. */
bool p5UebergangErlaubt (Assistentenschritt von, Assistentenschritt nach) noexcept;

/** Der naechste Zustand der P5-Folge. `false`, wenn `von` terminal ist. */
bool p5Naechster (Assistentenschritt von, Assistentenschritt& aus) noexcept;

/** §46.2: die drei benannten Ergebnisse, die der Assistent ausdruecklich
    sagen darf (M-61). Sie sind eigene Ergebnisse mit Objekt, keine
    Leerzustaende — deshalb ein eigener Typ und kein „kein Schritt". */
enum class Assistentenergebnis { schritt, passageMessen, routingBestaetigen, keineAenderungEmpfohlen };

const char* wort (Assistentenergebnis e) noexcept;

/** Der gehaltene Schritt. EIN Slot — der Deckel aus M-57 ist strukturell und
    kein Zaehler, der auch 2 tragen koennte. */
struct Assistentenzustand
{
    bool               gesetzt = false;
    juce::String       stepId;              ///< hex32
    Assistentenschritt schritt = Assistentenschritt::coverage;
    juce::int64        revision = 0;        ///< streng steigend, >= 1 wenn gesetzt
    bool               offen = true;        ///< false = terminal (§46.1)
    juce::String       findingId, proposalId, experimentId;
    /** §46.2: was der Assistent sagt, wenn kein Eingriff ansteht. */
    Assistentenergebnis ergebnis = Assistentenergebnis::schritt;
};

struct Zustand
{
    /** Der gehaltene Baum (NakamaState). Traegt auch Eigenschaften, die dieser
        Build nicht kennt - sie werden beim Speichern zurueckgeschrieben. */
    juce::ValueTree baum;
    Common common;
    std::vector<MainProjectMitglied> mainProjectMitglieder;
    std::vector<ManuellePassage>     manuellePassagen;

    /*  SONDE-014 Etappe A: der musikalische Intent (§37.1, §33.5). Drei
        additive Eigenschaften in einem BEKANNTEN Kind desselben Majors -
        kein neues Kind, keine Root-Versionierung (M-81). */
    std::vector<SourceIntent>    sourceIntents;
    std::vector<Schutzangabe>    schutzangaben;
    std::vector<IntentBeziehung> intentBeziehungen;

    /*  Die Revision des GANZEN Intent-Bestands. Sie steigt bei jeder
        persistenten Aenderung an einem der drei Bestandteile und ist die
        Zahl, die die Vollstaendigkeitsmarke aus E-10/M-86 traegt. Ohne sie
        koennte der Broker einen vollstaendigen leeren Bestand nicht von
        "noch nichts gehoert" unterscheiden. */
    juce::int64 intentBestandRevision = 0;

    /*  SONDE-014 Etappe G: der AKTUELLE Assistentenschritt (§46.1, E-08).
        EIN Slot — der Deckel aus M-57 ist strukturell und kein Zaehler, der
        auch 2 tragen koennte. Ein zweiter Startversuch wird abgewiesen, nicht
        eingereiht. */
    Assistentenzustand assistent;

    bool hatParameters = false;

    /*  Der NEUTRALE Satz, nicht Nullen. `Satz` ist ein std::array; ein
        `{}` haette jeden Parameter auf 0 gesetzt - und 0 Hz liegt fuer
        `band.0.freq_hz` ausserhalb seines Bereichs. Wer `hatParameters`
        setzt, ohne die Werte zu fuellen, schriebe damit einen Stand, den
        der eigene Leser als read-only zurueckgibt.
        Gefunden am 23.08. beim Bau von Nakama Probeeq (S9): dessen Klasse
        verlangt laut Kind-Matrix §2.1 genau ein Parameters-Kind, und der
        Passthrough-Beweis fiel beim Gegenpfad speichern<->laden. Kein
        gespeicherter Stand aendert sich dadurch: `lade()` fuellt die Werte
        ohnehin aus dem Baum, und fuer `main`/`legacy` bleibt
        `hatParameters` falsch. */
    parameter::Satz parameters = parameter::standardSatz();
    Herkunft herkunft = Herkunft::frisch;

    // read-only (§5 des Vertrags): gesetzt, wenn der State nicht interpretiert
    // werden darf. `originalBytes` gehen unveraendert zurueck an den Host.
    bool             nurLesen = false;
    juce::MemoryBlock originalBytes;
    juce::String     grund;
    int              fremdesMajor = 0;   ///< Root-`schema` des fremden Standes, wenn lesbar
};

/** Frische Instanz (nie restauriert): legacy + insert, leeres Label. */
Zustand frisch (const juce::String& instanceId);

/** Laedt Host-Bytes. `geladen`/`migriert`: `aus` traegt den Zustand.
    `nurLesen`: `aus` traegt die Originalbytes und den Grund. `ignoriert`
    (fremder Baumtyp, Muell): `aus` bleibt unveraendert. */
LadeErgebnis lade (const void* daten, size_t laenge, const Bundle& bundle, Zustand& aus);

/** Reine Funktion: EqCopilotState{schema=1} -> NakamaState{schema=2}.
    Deterministisch fuer jede nicht-leere sensor_id. false mit `grund`, wenn
    der Altstand kein Schema 1 ist oder ein unbekanntes Rollenwort traegt. */
bool migriereSchema1 (const juce::ValueTree& alt, juce::ValueTree& neu, juce::String& grund);

/** Serialisiert deterministisch (feste Kind- und Eigenschaftsreihenfolge).
    read-only: die Originalbytes, unveraendert. */
void speichere (const Zustand& z, juce::MemoryBlock& aus);

/** Bruecke zum heutigen v2-`hello` (bis SONDE-010): hub | sensor | pre | post. */
juce::String v2Rolle (const Common& c);
bool ausV2Rolle (const juce::String& rolle, Klasse& klasse, Messposition& position);

/*  ── SONDE-014 Etappe A: die Produkt-API des Intents ───────────────────────

    E-08 weist die AUTORITAET hier zu: Rollenmenge, Zyklenpruefung und
    Konfliktaufloesung beim Setzen laufen im Main, also in diesem C++-Code.
    `intent.rs` im Broker ist Spiegel und Vertragsvalidierung beim Empfang -
    keine zweite Zustandsmaschine und keine zweite Wahrheit (M-71).

    Jede dieser Funktionen ist ein GEGENPFAD-PAAR mit ihrem Leser: was die
    API ablehnt, lehnt `lade()` ebenso ab - fail-closed als read-only, nie
    still korrigiert. Ein Schreiber, der einen Stand erzeugt, den der eigene
    Leser verweigert, ist stiller Datenverlust beim naechsten Oeffnen. */

/** Setzt oder ersetzt den Intent einer Quelle in genau EINEM Scope.

    Reihenfolge (M-06, M-13): Validierung -> Zyklenpruefung -> Schreiben ->
    (der Aufrufer meldet Host-Dirty, wenn `true` UND `veraendert` true ist).

    - `herkunft == abgeleitet` ersetzt einen bestehenden `user`-Wert NIE
      (M-07); die Funktion gibt dann `true` mit `veraendert = false` zurueck,
      weil das kein Fehler ist, sondern die Regel.
    - Ein unveraenderter Wert erhoeht keine Revision und meldet kein Dirty.
    - `false` mit `grund`, wenn die Quelle keine hex32 ist, die Passage keine
      hex32, die Konfidenz nicht endlich oder ausserhalb [0,1] liegt oder der
      Deckel `maxSourceIntents` erreicht ist. */
/** Beginnt einen Schritt oder geht einen erlaubten Uebergang.

    Reihenfolge und Riegel:
      * ein TERMINALER Schritt wird nicht fortgesetzt — er wird neu begonnen;
      * ein Uebergang, den die P5-Tabelle nicht fuehrt, wird abgewiesen;
      * `preview` ist nie erreichbar (E-07);
      * die Revision steigt bei jeder angenommenen Aenderung. */
bool setzeAssistentenschritt (Zustand& z, const juce::String& stepId,
                              Assistentenschritt schritt, bool& veraendert,
                              juce::String& grund);

/** M-58: Abbruch. TERMINAL — „Verwerfen ist ein terminales Ereignis, kein
    Loeschen der Historie" (§46.1). Der Schritt bleibt stehen und traegt
    `offen = false`. */
bool assistentAbbrechen (Zustand& z, bool& veraendert, juce::String& grund);

/** M-58: Zurueck. Geht auf die sichere Rueckkante des aktuellen Zustands. */
bool assistentZurueck (Zustand& z, bool& veraendert, juce::String& grund);

/** M-58: Ueberspringen. Geht auf den naechsten Zustand der Folge, ohne die
    Useraktion des aktuellen. */
bool assistentUeberspringen (Zustand& z, bool& veraendert, juce::String& grund);

/** M-58/M-59: Resume. `true`, wenn ein OFFENER Schritt an derselben Stelle
    fortgesetzt werden kann; `false` bei terminalem oder fehlendem Schritt.
    Aendert nichts — Resume ist eine Frage, keine Aenderung. */
bool assistentResume (const Zustand& z, Assistentenzustand& aus);

/** §46.2: setzt eines der drei benannten Ergebnisse (M-61). */
bool setzeAssistentenergebnis (Zustand& z, Assistentenergebnis ergebnis,
                               bool& veraendert, juce::String& grund);

/** Ein Kandidat der Assistentenpriorisierung (§46.2, M-60). */
struct Schrittkandidat
{
    juce::String       findingId;
    Assistentenschritt schritt = Assistentenschritt::finding;
    double erwarteterNutzen = 0.0;   ///< [0,1]
    double intentRelevanz  = 0.0;    ///< [0,1]
    double konfidenz       = 0.0;    ///< [0,1]
    double reversibilitaet = 0.0;    ///< [0,1]
    double messkosten      = 0.0;    ///< [0,1], hoeher = teurer
    bool   vergleichbar    = false;  ///< HARTES Gate
    bool   sicher          = false;  ///< HARTES Gate
    bool   bereitsErfolglos = false; ///< HARTES Gate
};

/** M-60: das deterministische Ranking.

    ⚠️ Die HARTEN GATES greifen VOR der Gewichtung: nicht vergleichbare,
    unsichere und bereits erfolglose Schritte verlassen die Liste, bevor
    irgendein Gewicht gerechnet wird. Ein Kandidat mit perfektem Nutzen und
    gerissener Vergleichbarkeit erreicht die Gewichtung nie.

    Rueckgabe: die ueberlebenden Kandidaten, absteigend nach Rang; bei
    Gleichstand aufsteigend nach `findingId` — eine stabile Wahl, keine
    zufaellige. */
std::vector<Schrittkandidat> ordneSchritte (const std::vector<Schrittkandidat>& kandidaten);

/** Der Rang eines Kandidaten in [0,1]. Gleichgewichtet ueber die fuenf
    Groessen; `messkosten` gehen invers ein („der kleinste hochrelevante,
    reversible Test zuerst"). */
double schrittrang (const Schrittkandidat& k) noexcept;

bool setzeIntent (Zustand& z, const juce::String& quelleId, const juce::String& passageId,
                  Rolle rolle, IntentHerkunft herkunft, double konfidenz,
                  bool& veraendert, juce::String& grund);

/** Entfernt den Intent einer Quelle in genau einem Scope. `veraendert` ist
    false, wenn es keinen gab (No-op meldet kein Dirty). */
bool entferneIntent (Zustand& z, const juce::String& quelleId, const juce::String& passageId,
                     bool& veraendert, juce::String& grund);

/** Findet den Intent einer Quelle in einem Scope. */
const SourceIntent* findeIntent (const Zustand& z, const juce::String& quelleId,
                                 const juce::String& passageId);

/** §37.2 Stufe 3: der passagespezifische Intent gewinnt vor dem globalen.
    Gibt den WIRKENDEN Intent einer Quelle in einer Passage zurueck; ohne
    Passagenwert faellt er auf den globalen zurueck, ohne beide auf nullptr. */
const SourceIntent* wirkenderIntent (const Zustand& z, const juce::String& quelleId,
                                     const juce::String& passageId);

/** Setzt eine Schutzangabe. Orthogonal zur Rolle (M-03): sie braucht keine
    und veraendert keine. Doppelte Angaben sind ein No-op. */
bool setzeSchutzangabe (Zustand& z, const juce::String& quelleId, Schutzeigenschaft eigenschaft,
                        int bandVon, int bandBis, bool& veraendert, juce::String& grund);

bool entferneSchutzangabe (Zustand& z, const juce::String& quelleId, Schutzeigenschaft eigenschaft,
                           int bandVon, int bandBis, bool& veraendert, juce::String& grund);

/** Setzt eine gerichtete Beziehung.

    M-06: die Zyklenpruefung laeuft BEIM SPEICHERN dieses Werts, nicht beim
    Anwenden. Erzeugte die Kante einen Zyklus im `fuehrtVor`-Teilgraphen,
    gibt die Funktion `false` mit Grund zurueck - der Zyklus erreicht die
    Persistenz nie unmarkiert. Der Aufrufer loest ihn auf oder ruft
    `speichereAlsGleichrangigkeit`. */
bool setzeBeziehung (Zustand& z, const juce::String& quelleA, const juce::String& quelleB,
                     Beziehungsart art, bool& veraendert, juce::String& grund);

/** §37.2, zweite Haelfte: speichert die Kante als NICHT STEUERBARE
    Gleichrangigkeit. Das ist der einzige Weg, auf dem ein Zyklus in den
    Bestand gelangt - und er ist dann als `gleichrangig` markiert und wird
    beim Laden nie wieder als gerichtet gelesen. */
bool speichereAlsGleichrangigkeit (Zustand& z, const juce::String& quelleA,
                                   const juce::String& quelleB,
                                   bool& veraendert, juce::String& grund);

bool entferneBeziehung (Zustand& z, const juce::String& quelleA, const juce::String& quelleB,
                        bool& veraendert, juce::String& grund);

/** Prueft den `fuehrtVor`-Teilgraphen einer Kantenmenge auf Zyklen.
    `true` = es gibt einen Zyklus. Reine Funktion, damit Leser und Schreiber
    denselben Riegel benutzen. */
bool hatZyklus (const std::vector<IntentBeziehung>& kanten);

/** E-02, M-04: darf fuer dieses Paar eine Entmaskierung empfohlen werden?

    Die Rolle `verschmolzen` an einer der beiden Quellen ist ein GLOBALES
    VETO; eine ausdrueckliche gerichtete Beziehung `A fuehrt vor B` ist das
    SPEZIFISCHERE Werkzeug und hebt das Veto NUR fuer dieses Paar auf. Beide
    liegen auf Stufe 2, spezifisch vor global. */
bool entmaskierungErlaubt (const Zustand& z, const juce::String& quelleA,
                           const juce::String& quelleB, const juce::String& passageId);

/** Lesbarer Text fuer Manifeste und Tests (XML-Form des Baums). */
juce::String alsText (const Zustand& z);

} // namespace nakama::state
