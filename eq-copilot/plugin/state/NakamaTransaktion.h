#pragma once

/*  SONDE-015 Etappe 4a - der lokale Transaktionskern der aktiven Sonde.

    Die Referenz ist die geschlossene Protokollspezifikation im Manifest
    `docs/beweise/SONDE-015.md` §5.11.4: Teil 1 (Arithmetik mit `r0`), Teil 2
    (die Stufenfolge S0 bis S8), Teil 3 (die Invarianten I1 bis I6) und Teil 4
    (die Falltabelle T1 bis T17). Dieser Code rechnet keine eigene Zahl aus;
    wo er von einer abhaengt, steht der Verweis daneben.

    DREI ZUSTAENDE, GETRENNT (Entwurf §44.3):
      - der BESTAETIGTE Zustand: ein `parameter::DspSatz` (Entscheid E-1, kein
        zweiter Zustandstyp), dazu `r`, `r0`, sein Hash, der Undo-Ring und
        dessen Cursor. Nur er wird gespeichert und gehasht.
      - der `PreviewOverlay`: fluechtig, nie serialisiert (M-83), nie Teil
        einer Revision.
      - der `AutomationOverlay`: Hostwerte ueber dem bestaetigten Zustand,
        fluechtig, mit `automation_epoch`; er zaehlt keine Revision (M-81).

    IDENTITAETSFREI: kein Prozessor, keine JucePlugin_*-Konstante, kein
    Fernweg. Was ein Commit ausserhalb des Kerns bewirkt - Programm bauen und
    publizieren, Host-Dirty, `state_report` -, ruft er ueber `Ausfuehrung`.

    NICHT FUER DEN AUDIOTHREAD, und EIN Schreiber: der Kern sperrt nicht
    selbst. Wer ihn benutzt, serialisiert jeden Aufruf (Probeeq: das
    Zustandsschloss des Prozessors). Nur so haelt die Zusage aus S5 bis S8,
    dass zwischen Programmbau und Publikation niemand eine Bank belegt.
*/

#include <juce_core/juce_core.h>

#include "NakamaParameter.h"
#include "NakamaState.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <limits>
#include <vector>

namespace nakama::dsp { class DspKern; }

namespace nakama::transaktion
{

/** Kapazitaet des Wiederholungsregisters R = Fensterspanne = Tiefe des
    Undo-Rings (§5.11.4 Teil 1 und I3: eine Zahl, zwei Verwendungen). */
inline constexpr int kRegisterPlaetze = parameter::kUndoTiefe;
static_assert (kRegisterPlaetze == 32, "§5.11.4 I3: Kapazitaet = Fensterspanne = Undo-Tiefe = 32");

/** Die groesste Revision, die der State tragen kann: `state_revision` reist
    als int64 >= 0 (`nakama-state-v2.md`, Kind `Dsp`). Eine Transaktion auf
    diesem Stand scheitert in S4 - hinter dem Commit-Punkt gibt es keinen
    Ueberlauf, der fehlschlagen koennte. */
inline constexpr std::uint64_t kHoechsteRevision = (std::uint64_t) std::numeric_limits<juce::int64>::max();

/** Die Ruhegrenze der Hostautomation (§44.3: der Epochzaehler wechselt
    "einmal beim Beginn und Ende einer Hostgeste beziehungsweise nach einer
    definierten Ruhegrenze"). Technikkonstante mit Test; gezaehlt in
    verarbeiteten AUDIOSAMPLES, nicht an der Wanduhr, damit Realtime- und
    Offline-Render dieselbe Epochenfolge erzeugen (M-120). */
inline constexpr double kAutomationsRuheSekunden = 0.25;

/** Die Stufen aus §5.11.4 Teil 2. `keine` heisst: keine Stufe hat entschieden. */
enum class Stufe { s0 = 0, s1, s2, s3, s4, s5, s6, s7, s8, keine };

/** Die sieben Transaktionen aus R11 und zwei Formen, die sie tragen:
    `gestus` ist der abgeschlossene manuelle Parametergestus (§44.3, M-82),
    `bandBelegen` ein Apply, das den ersten freien Slot waehlt (M-62). */
enum class Art { apply, revert, neutralisieren, remove, undo, redo, presetLaden, gestus, bandBelegen };

/** Die Ausgaenge der Falltabelle (§5.11.4 Teil 4). */
enum class Ausgang { commit, gespeichertesErgebnis, konflikt, fehler, userSchutz, busyRetry };

const char* wort (Stufe s) noexcept;
const char* wort (Art a) noexcept;
const char* wort (Ausgang a) noexcept;

/** Ganzzustands-Wiederherstellung (§5.6.3 Feinheit 3, Ausnahme; T16):
    Undo, Redo und Preset-Laden durchlaufen den Zonenriegel S3 nicht. */
bool istGanzzustand (Art a) noexcept;

/** Transaktions-ID, vom Aufrufer vergeben, je logischem Auftrag genau eine. */
struct Tid
{
    std::uint64_t hoch = 0, tief = 0;
    bool operator== (const Tid& a) const noexcept { return hoch == a.hoch && tief == a.tief; }
    bool operator!= (const Tid& a) const noexcept { return ! (*this == a); }
};

/** Ein `state_hash` als 64 Hexzeichen OHNE Heap: das Register und der
    Commit-Punkt tragen ihn, und dort wird nicht alloziert. */
using HashText = std::array<char, 64>;

juce::String alsText (const HashText& h);
bool ausHex (const juce::String& hex, HashText& aus) noexcept;

/** Eine Eingabe `(tid, base_revision, Nutzlast)` (§5.11.4 Teil 4). */
struct Auftrag
{
    Tid                tid;
    std::uint64_t      baseRevision = 0;
    Art                art = Art::apply;
    parameter::DspSatz satz;              ///< apply, gestus: der vollstaendige Zielzustand
    std::array<parameter::Zelle, (size_t) parameter::kJeSlot> band {};   ///< bandBelegen: die 13 v1-Werte in Vorlagenreihenfolge
    int                slot = -1;         ///< remove: der Slot 0..7
    std::uint64_t      zielRevision = 0;  ///< revert: die exakt referenzierte Revision (Entwurf §61, inverse Pfade)
    juce::MemoryBlock  preset;            ///< presetLaden: die Presetbytes (R12)
};

/** Genau ein Ausgang je Eingabe (I6). */
struct Ergebnis
{
    Ausgang       ausgang    = Ausgang::fehler;
    Stufe         stufe      = Stufe::keine;   ///< die Stufe, die entschieden hat
    std::uint64_t revision   = 0;              ///< Commit: e; gespeichertes Ergebnis: e aus R; sonst r
    HashText      hash {};                     ///< der Hash zu `revision`
    bool          memoisiert = false;          ///< liegt nach dieser Eingabe ein Eintrag fuer die tid in R?
    int           slot       = -1;             ///< bandBelegen: der belegte Slot
    juce::String  grund;                       ///< nur bei `fehler` gesetzt (leer alloziert nicht)
};

/** Die Seite, auf der ein Commit wirkt. S5 und S8 rufen sie; die Preview
    publiziert ueber dieselbe Seite. */
class Ausfuehrung
{
public:
    virtual ~Ausfuehrung() = default;

    /** S5: baut das Programm des Kandidaten OHNE Bank. false genau dann,
        wenn der Kandidat bankpflichtig ist (`eq_enabled = true`, §5.11.4
        Teil 2) und keine Bank frei ist - das ist `busy_retry`. */
    virtual bool baueVor (const parameter::DspSatz& kandidat) = 0;

    /** S8: publiziert das Programm aus `baueVor`. Kann nicht scheitern und
        alloziert nicht. */
    virtual void publiziereVorbau() noexcept = 0;

    /** S8: Host-Dirty und `state_report` sind faellig. Nur eine Markierung;
        den Hostaufruf macht der Besitzer nach dem Loslassen seines Schlosses. */
    virtual void commitGemeldet (std::uint64_t revision) noexcept = 0;

    /** Preview ueber den Candidate-Pfad (§44.2). false = `busy_retry`. */
    virtual bool publizierePreview (const parameter::DspSatz& satz) = 0;
    virtual void beendePreview() noexcept = 0;
};

/** Der fluechtige Automationszustand (§44.3): je Host-Parameter ein Wert, der
    vom bestaetigten abweicht, und die Epoche. */
struct AutomationOverlay
{
    std::array<bool, (size_t) parameter::kHostParameter>             gesetzt {};
    std::array<parameter::Zelle, (size_t) parameter::kHostParameter> werte {};
    std::uint64_t epoche = 0;       ///< `automation_epoch`
    bool          laeuft = false;   ///< zwischen Beginn und Ende einer Hostgeste

    int anzahl() const noexcept;
};

/** Die fluechtige Vorschau (§44.3, §44.4). */
struct PreviewOverlay
{
    bool               aktiv = false;
    parameter::DspSatz satz;
};

/*  ── Schutz-Zonen (R6, §5.6) ─────────────────────────────────────────────── */

/** Liegt `freqHz` in der Zone? Raender inklusive. Ist die Samplerate bekannt
    (> 0), sind Frequenz UND Zonenraender auf 0,45*fs gekappt - dieselbe
    Kappung, mit der das Band klingt (M-12, M-66); kein persistenter Wert
    wird dabei veraendert. */
bool frequenzInZone (double freqHz, const parameter::Schutzzone& z, double samplerate) noexcept;

/** P(Slot, Z): belegt UND eingeschaltet UND `freq_hz` in der EINGESCHALTETEN
    Zone Z (§5.6.3 Feinheit 3). */
bool liegtInZone (const parameter::DspSatz& s, int slot, const parameter::Schutzzone& z, double samplerate) noexcept;

/** Der Zonenriegel einer Bedienaenderung (§5.6.3 Feinheit 3, B-03): der erste
    Slot, fuer den P mit IRGENDEINER eingeschalteten Zone von falsch auf wahr
    kippt, waehrend der Uebergang `occupied`, `enabled` oder `freq_hz` dieses
    Slots aendert - sonst -1. Je Zone einzeln, kein Sammelzustand. */
int zoneneintritt (const parameter::DspSatz& vorher, const parameter::DspSatz& nachher, double samplerate) noexcept;

/** Die verletzten Baender (R6, M-68, M-70): Slot i, fuer den P mit einer
    eingeschalteten Zone gilt. */
std::array<bool, (size_t) parameter::kSlots> verletzteBaender (const parameter::DspSatz& s, double samplerate) noexcept;

/** Die Kernsicht: freie Slots tragen ihre Vertragsdefaults. Ein freier Slot
    verarbeitet nichts (M-58); mit dieser Sicht aendert Automation auf seinen
    Werten auch das Programm nicht (M-63). Normalisiert wird eine KOPIE - die
    persistenten Werte bleiben, was sie sind. */
void setzeFreieSlotsNeutral (parameter::Satz& werte) noexcept;

/*  ── Der Kern ─────────────────────────────────────────────────────────────── */

class Transaktionskern
{
public:
    explicit Transaktionskern (Ausfuehrung& ausfuehrung);

    /** Ladestart (§5.11.4 Teil 1, I5, T17). KEINE Transaktion: keine tid,
        keine Revision, keine Stufe. Danach gilt `r0 = r = revision`, das
        Register ist leer, beide Overlays sind leer. false mit `grund`, wenn
        der Zustand nicht validiert, sich nicht hashen laesst, die Revision
        ueber `kHoechsteRevision` liegt oder Ring und Cursor unzulaessig sind -
        dann bleibt der Kern, wie er war. */
    bool ladestart (const parameter::DspSatz& satz, std::uint64_t revision,
                    const std::vector<state::UndoEintrag>& undoRing, int undoCursor,
                    juce::String& grund);

    /** Eine Eingabe durch die Stufen S0 bis S8 (§5.11.4 Teil 2). */
    Ergebnis fuehreAus (const Auftrag& auftrag);

    //== Lesen ================================================================
    const parameter::DspSatz& bestaetigt() const noexcept { return committed; }
    std::uint64_t revision() const noexcept      { return r; }
    std::uint64_t ladeRevision() const noexcept  { return r0; }
    const HashText& hash() const noexcept        { return committedHash; }
    const std::vector<state::UndoEintrag>& undoRing() const noexcept { return ring; }
    int undoCursor() const noexcept              { return cursor; }

    /** Schreibt, was persistiert wird, in einen State: die 120 Werte, die
        Zonen, `state_revision`, den Undo-Ring und seinen Cursor. NIE ein
        Overlay und NIE das Register (I5, M-83). */
    void schreibeIn (state::Zustand& z) const;

    //== Register (fuer die Wachen I1, I2, I4) =================================
    int registerBelegung() const noexcept;
    /** Platz 0..31: belegt? Dann `tid`, `e` und Hash. */
    bool registerPlatz (int platz, Tid& tid, std::uint64_t& e, HashText& h) const noexcept;

    //== PreviewOverlay =======================================================
    /** Validiert und publiziert eine Vorschau auf dem Candidate-Pfad. Keine
        Transaktion, keine Revision, nie gespeichert. false: `grund` traegt ein
        Validatorwort oder `busy_retry`. */
    bool setzePreview (const parameter::DspSatz& satz, juce::String& grund);
    void beendePreview() noexcept;
    const PreviewOverlay& preview() const noexcept { return vorschau; }

    //== AutomationOverlay ====================================================
    /** Ein Hostwert fuer den Host-Parameter `index` (0..111). Gleicht er dem
        bestaetigten Wert, faellt der Eintrag weg. Beginnt damit eine
        Hostgeste, wechselt die Epoche. Keine Revision (M-81). */
    void automationSchreiben (int index, const parameter::Zelle& wert) noexcept;

    /** Die Ruhegrenze ist erreicht: die laufende Hostgeste endet, die Epoche
        wechselt ein zweites Mal. Ohne laufende Geste: nichts. */
    void automationRuht() noexcept;

    /** Nimmt den Eintrag eines Host-Parameters aus dem Overlay, ohne die
        Epoche zu beruehren - fuer einen abgewiesenen Gestus, dessen Regler zum
        bestaetigten Wert zurueckkehrt. */
    void automationLoesen (int index) noexcept;

    const AutomationOverlay& automation() const noexcept { return automationOverlay; }

    /** Der WIRKSAME Zustand: bestaetigt, ueberlagert von der Automation.
        Topologische Parameter werden nur bei samplegenauer Automation
        ueberlagert (§44.3 letzter Absatz: sonst "Topologieautomation wird
        deaktiviert", M-119). */
    parameter::DspSatz wirksam() const;

    //== Umgebung =============================================================
    void   setzeSamplerate (double samplerate) noexcept;
    double samplerate() const noexcept { return fs; }
    void   setzeSamplegenaueAutomation (bool ja) noexcept { samplegenau = ja; }
    bool   samplegenaueAutomation() const noexcept { return samplegenau; }

    //== Testzugaenge (B7; im Produkt nie gesetzt) =============================
    /** Laesst die genannte Stufe mit IHREM Fehlerausgang scheitern (M-75). */
    void setzeEinspritzung (Stufe s) noexcept { einspritzung = s; }

    /** Am Beginn jeder Stufe gerufen - fuer den Allokationszaehler in S0 und
        hinter dem Commit-Punkt. */
    void setzeStufenHaken (void (*h) (Stufe, void*), void* kontext) noexcept { haken = h; hakenKontext = kontext; }

private:
    struct RegisterEintrag
    {
        bool          belegt = false;
        Tid           tid;
        std::uint64_t e = 0;
        HashText      hash {};
    };

    Ergebnis ohneCommit (Ausgang a, Stufe s) const noexcept;
    void     melde (Stufe s) noexcept { if (haken != nullptr) haken (s, hakenKontext); }
    int      ersterFreierSlot() const noexcept;
    const parameter::DspSatz* revertZiel (std::uint64_t ziel) const noexcept;
    /** Der Zustand, den eine Transaktion herstellen will (fuer S3), oder nullptr. */
    const parameter::DspSatz* zonenziel (const Auftrag& a) const noexcept;

    Ausfuehrung& aus;

    parameter::DspSatz              committed;
    std::uint64_t                   r  = 0;
    std::uint64_t                   r0 = 0;
    HashText                        committedHash {};
    std::vector<state::UndoEintrag> ring;
    int                             cursor = 0;
    std::array<RegisterEintrag, (size_t) kRegisterPlaetze> reg {};

    PreviewOverlay    vorschau;
    AutomationOverlay automationOverlay;
    double            fs = 0.0;
    bool              samplegenau = false;

    // Arbeitsplatz der Stufen S2 bis S7 - vor dem Commit-Punkt beschrieben,
    // in S8 nur noch getauscht oder verschoben.
    parameter::DspSatz kandidat;
    HashText           kandidatHash {};
    state::UndoEintrag eintrag;
    int                belegterSlot = -1;

    Stufe einspritzung = Stufe::keine;
    void (*haken) (Stufe, void*) = nullptr;
    void* hakenKontext = nullptr;

    JUCE_DECLARE_NON_COPYABLE (Transaktionskern)
};

/*  ── Die Anbindung an den DSP-Kern der Etappe 3 ──────────────────────────── */

/** `Ausfuehrung` ueber `dsp::DspKern`, identitaetsfrei wie beide Seiten. Der
    DSP-Kern bekommt immer die Kernsicht (`setzeFreieSlotsNeutral`) - in einem
    Arbeitssatz ohne Zonenliste, damit S5 nicht alloziert. */
class DspKernAusfuehrung final : public Ausfuehrung
{
public:
    explicit DspKernAusfuehrung (dsp::DspKern& k) noexcept : kern (k) {}

    bool baueVor (const parameter::DspSatz& kandidat) override;
    void publiziereVorbau() noexcept override;
    void commitGemeldet (std::uint64_t revision) noexcept override;
    bool publizierePreview (const parameter::DspSatz& satz) override;
    void beendePreview() noexcept override;

    /** Ausserhalb einer Transaktion (Automation, Vorbereitung): publiziert den
        wirksamen Zustand, wenn seine Kernsicht sich seit der letzten
        Publikation geaendert hat oder `erzwingen` gilt. false = `busy_retry`;
        der Aufrufer versucht es am naechsten Takt erneut. */
    bool publiziereWirksam (const parameter::DspSatz& wirksam, bool erzwingen);

    /** Holt die Markierungen aus S8 ab (true = war gesetzt). */
    bool dirtyAbholen() noexcept   { return dirty.exchange (false); }
    bool berichtAbholen() noexcept { return bericht.exchange (false); }
    std::uint64_t gemeldeteCommits() const noexcept { return commits.load(); }

    /** Die Kernsicht der letzten Committed-Publikation, sofern es eine gab. */
    bool zuletztPubliziert (parameter::Satz& aus) const noexcept;
    void vergissLetztePublikation() noexcept { zuletztGueltig = false; }

private:
    dsp::DspKern&      kern;
    parameter::DspSatz sichtCommit;    ///< Arbeitssatz fuer S5/S8 (Zonen bleiben leer)
    parameter::DspSatz sichtWeg;       ///< Arbeitssatz fuer Preview und Automation
    parameter::Satz    zuletzt = parameter::standardSatz();
    bool               zuletztGueltig = false;
    bool               vorbauGebaut   = false;
    std::atomic<bool>          dirty   { false };
    std::atomic<bool>          bericht { false };
    std::atomic<std::uint64_t> commits { 0 };
};

/*  ── Die lokale Nutzlast von `state_report.dsp` (R13) ────────────────────── */

/** Eine gemeldete Klemmung (v3 `$defs/dsp_klemmung`, R7; M-23) - hoechstens
    eine je Slot (E2-7). */
struct Klemmung
{
    juce::String id, gemeldet, wirksam;
};

/** Was `state_report.dsp` traegt (R13, §5.13). Etappe 4a baut den Inhalt; der
    Sender im ControlClient ist Etappe 4b. */
struct DspBericht
{
    std::uint64_t         revision = 0;
    HashText              hash {};
    juce::String          jcs;               ///< GENAU der kanonische Text, ueber den `hash` gebildet wurde (M-103)
    double                autoGainDb = 0.0;  ///< abgeleitet aus dem Programm des BESTAETIGTEN Zustands (R4)
    std::vector<Klemmung> klemmungen;        ///< aus demselben Programm, in Slotreihenfolge
    std::vector<int>      verletzteBaender;  ///< aus dem WIRKSAMEN Zustand - Automation meldet dieselbe Verletzung (M-70)
    int                   undoTiefe = 0;     ///< jetzt moegliche Undo-Schritte: Ringlaenge minus Cursor
};

/** Baut den Bericht. false mit `grund`, wenn der bestaetigte Zustand sich
    nicht kanonisieren laesst (kann nach einem gueltigen Ladestart nicht
    geschehen). Ohne Samplerate bleiben Programmangaben leer bzw. 0. */
bool baueBericht (const Transaktionskern& tk, DspBericht& aus, juce::String& grund);

} // namespace nakama::transaktion
