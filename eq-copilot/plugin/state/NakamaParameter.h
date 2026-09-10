#pragma once

/*  Fester Parameterbestand der aktiven Sonde (SONDE-006, Entwurf §53.8;
    Layout v2 aus SONDE-015 R1-R6).

    Die Wahrheit ist eq-copilot/schemas/state/nakama-parameter-v2.json; diese
    Tabelle MUSS ihr deckungsgleich sein - EqCopStateMigrationTest misst das
    (Anzahl, IDs, Typen, Grenzen, Defaults, Enumwoerter, Host-Attribut). Warum
    die Tabelle trotzdem in C++ steht: der Audiothread und die
    APVTS-Konstruktion (SONDE-015) brauchen sie zur Bauzeit, nicht als Datei
    zur Laufzeit.

    Layout v2 ist ADDITIV. Die 109 v1-Kennungen behalten ID, Typ, Grenzen,
    Default und Wechselart; die elf neuen tragen ausnahmslos das Praefix
    `v2.*` und stehen HINTER dem v1-Bestand. Die Vertragsreihenfolge ist
    deshalb: 109 v1, dann `eq_enabled`, `mix`, `auto_gain` (zusammen die 112
    HOST-Parameter), dann acht `occupied` (persistenter Zustand im State-Kind
    `Dsp`, kein Host-Parameter).

    Heute (Eqcp = Gen/Legacy) gibt es KEINE Hostparameter - §53.8: "der
    bestehende Eqcp-Eintrag aendert seine Parameterliste nicht". Die Tabelle
    beschreibt das Layout, das Probeeq (NkAc) ab SONDE-015 traegt, und sie ist
    der Vertrag fuer `Parameters` im State und fuer das DSP-DTO samt
    `state_hash`.

    Keine `JucePlugin_*`-Konstante, nur juce_core - S8-tauglich.
*/

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>

#include <array>
#include <vector>

namespace nakama::parameter
{

constexpr int kSlots          = 8;
constexpr int kGlobal         = 5;                            ///< v1: bypass, trims, width, mono_bass
constexpr int kJeSlot         = 13;                           ///< v1 je Slot
constexpr int kAnzahlV1       = kGlobal + kSlots * kJeSlot;   // 109
constexpr int kGlobalV2       = 3;                            ///< eq_enabled, mix, auto_gain
constexpr int kJeSlotV2       = 1;                            ///< occupied
constexpr int kHostParameter  = kAnzahlV1 + kGlobalV2;        // 112
constexpr int kAnzahl         = kHostParameter + kSlots * kJeSlotV2;   // 120

constexpr int kDspSchemaVersion   = 2;   ///< Layout v2 (dieser Build)
constexpr int kDspSchemaVersionV1 = 1;   ///< Layout v1 (Verlauf; Staende migrieren)

static_assert (kAnzahlV1      == 109, "§53.8: 5 globale + 8 x 13 Slot-Parameter");
static_assert (kHostParameter == 112, "R1: 109 v1-Kennungen + drei v2-Host-Parameter");
static_assert (kAnzahl        == 120, "R1: 112 Host-Parameter + 8 x occupied");

/*  Feste Indizes der drei v2-Host-Parameter und der acht `occupied`. Sie sind
    aus der Vertragsreihenfolge GERECHNET, nicht abgeschrieben: verschoebe
    jemand die Reihenfolge, faellt der static_assert unten und nicht erst ein
    Wert im Feld. */
constexpr int kIndexEqEnabled = kAnzahlV1;       // 109
constexpr int kIndexMix       = kAnzahlV1 + 1;   // 110
constexpr int kIndexAutoGain  = kAnzahlV1 + 2;   // 111

/** Index von `v2.band.<slot>.occupied` - 112 bis 119. */
constexpr int indexOccupied (int slot) noexcept { return kHostParameter + slot; }

/** Index eines v1-Slotfeldes: `feld` = 0..12 in Vorlagenreihenfolge. */
constexpr int indexBandV1 (int slot, int feld) noexcept { return kGlobal + slot * kJeSlot + feld; }

/*  Die dreizehn v1-Slotfelder in Vorlagenreihenfolge - als Namen, damit die
    Migrationsregel aus R5 ("mindestens ein Wert des Slots weicht ab") nicht
    mit einer Zahl im Code hantiert. */
enum SlotFeld
{
    kEnabled = 0, kType, kFreqHz, kQ, kGainDb, kChannelMode, kDynamicEnabled,
    kDynamicRangeDb, kThresholdDb, kAttackMs, kHoldMs, kReleaseMs, kSidechainSource
};
static_assert (kSidechainSource + 1 == kJeSlot, "dreizehn Slotfelder");

/** Hoechstzahl der Schutz-Zonen (SONDE-015 R6). */
constexpr int kMaxZonen = 8;

/** Tiefe des Undo-Rings (v3 `undo_tiefe`, `maximum: 32`). */
constexpr int kUndoTiefe = 32;

enum class Typ     { boolean, gleitkomma, aufzaehlung };
enum class Wechsel { rampe, blockrand };

struct Beschreibung
{
    juce::String id;            ///< z. B. "v1.band.3.freq_hz" oder "v2.global.mix"
    Typ          typ;
    double       min = 0.0, max = 0.0, standardZahl = 0.0;   ///< gleitkomma
    bool         standardBool = false;                        ///< boolean
    juce::StringArray werte;                                  ///< aufzaehlung (Reihenfolge = Vertrag)
    int          standardIndex = 0;                           ///< aufzaehlung
    Wechsel      wechsel = Wechsel::rampe;
    bool         topologisch = false;
    /*  R1: jede Kennung sagt, ob sie ein APVTS-/FL-Automationsparameter ist
        oder nur persistenter Zustand im Kind `Dsp`. Die 112 Host-Parameter
        sind das PRAEFIX der Vertragsreihenfolge - `occupied` ist bewusst
        nicht automatisierbar, sonst belegte eine Automationskurve einen Slot
        ohne Transaktion, ohne Revision und ohne Undo-Objekt (R5). */
    bool         hostParameter = true;
    int          layout = 1;    ///< 1 = v1-Bestand, 2 = Erweiterung dieses Tickets
};

/** Die 120 Beschreibungen in Vertragsreihenfolge (109 v1, 3 v2 global, 8 occupied). */
const std::array<Beschreibung, kAnzahl>& tabelle();

/** Index 0..119 oder -1. */
int indexVonId (const juce::String& id);

/** Ein Parameterwert - genau EIN Feld ist je nach Typ gueltig. */
struct Zelle
{
    bool   b = false;
    double zahl = 0.0;
    int    enumIndex = 0;
};

/** Alle 120 Werte, Index = Vertragsreihenfolge. */
using Satz = std::array<Zelle, kAnzahl>;

/** Der neutrale Zustand: jeder Parameter auf seinem Default. */
Satz standardSatz();

/** Eine Schutz-Zone (SONDE-015 R6).

    Die `id` gehoert der ZONE, nicht einem Band-Slot: eine an einen Slot
    gebundene Zone verschwaende beim Entfernen dieses Slots, und "Loesen" ist
    ausschliesslich `enabled = false` oder das Entfernen des Eintrags. Zwei
    Wege, eine Zone loszuwerden, waeren zwei Wahrheiten. */
struct Schutzzone
{
    int    id      = 0;        ///< 0..7, im Bestand eindeutig
    double lowHz   = 20.0;
    double highHz  = 20000.0;
    bool   enabled = false;

    bool operator== (const Schutzzone& a) const noexcept
    {
        return id == a.id && lowHz == a.lowHz && highHz == a.highHz && enabled == a.enabled;
    }
    bool operator!= (const Schutzzone& a) const noexcept { return ! (*this == a); }
};

/** Der vollstaendige DSP-DTO-Inhalt: 120 Werte plus die Zonenliste.

    Beides zusammen ist der Gegenstand des `state_hash`. Es gibt bewusst KEINE
    Ueberladung, die nur den `Satz` nimmt: wer den Hash bildet, muss die Zonen
    in der Hand haben, sonst hasht er einen Zustand, den es nicht gibt. */
struct DspSatz
{
    Satz werte = standardSatz();
    std::vector<Schutzzone> zonen;   ///< 0..8 Eintraege, streng aufsteigend nach id

    bool operator== (const DspSatz& a) const noexcept;
    bool operator!= (const DspSatz& a) const noexcept { return ! (*this == a); }
};

/** Prueft Endlichkeit, Bereich und Enumindex der 120 Werte. `grund` ist ein
    Wort aus fixtures/state/MANIFEST.json "gruende" (nichtendlich | bereich |
    enum). */
bool validiere (const Satz& s, juce::String& grund, juce::String& woId);

/** Prueft die Zonenliste: Anzahl <= 8, `id` in 0..7, 20 <= low < high <= 20000,
    keine doppelte `id`, streng aufsteigend nach `id`. `grund` ist eines von
    zone_anzahl | bereich | zone_doppelt | zone_sortierung. */
bool validiereZonen (const std::vector<Schutzzone>& z, juce::String& grund, juce::String& woId);

/** Werte UND Zonen. */
bool validiere (const DspSatz& s, juce::String& grund, juce::String& woId);

/** Liest ein DSP-DTO aus UTF-8-Text - die volle Stufenfolge aus
    nakama-state-v2.md §4: v3-Textriegel (Bytes, Text) → doppelte Schluessel →
    Parser → Struktur → dsp_schema_version → unbekannt → fehlend → Typ →
    nichtendlich → Bereich/Enum → Zonenregeln. `grund` traegt bei false das
    Manifestwort des ERSTEN Fehlers. */
bool ausDtoText (const void* utf8, size_t laenge, DspSatz& aus, juce::String& grund, juce::String& detail);

/** Validiert und kanonisiert (RFC 8785) das DTO
    `{"dsp_schema_version":2,"parameters":{...},"schutz_zonen":[...]}`. */
bool dtoKanon (const DspSatz& s, juce::MemoryBlock& utf8, juce::String& grund);

/** `state_hash`: SHA-256-Hex des kanonischen DTO (§53.8). */
bool stateHash (const DspSatz& s, juce::String& hex, juce::String& grund);

/** Prueft die Nutzlast von v3 `state_report.dsp` (SONDE-015 R13, M-105).

    `jcs` ist GENAU die RFC-8785-Zeichenkette, ueber die `state_hash` gebildet
    wurde. Der Empfaenger rechnet `SHA-256(jcs)` nach und vergleicht; weichen
    sie ab, wird der Bericht GANZ abgewiesen (`grund` = "state_hash"). Erst
    danach wird der Text als DTO gelesen - mit dem exakten Zahlenleser des
    DTO-Wegs, nicht mit dem binary64-Leser des Schemawegs, sonst verloere
    gerade der Wert seine letzte Ziffer, fuer den dieser Weg gebaut ist.

    Ein Leser, der den Hash nicht nachrechnet, ist kein Riegel: die
    Zeichenkette kaeme ungeprueft aus dem Netz. */
bool berichtDtoPruefen (const juce::String& jcs, const juce::String& stateHashHex,
                        DspSatz& aus, juce::String& grund, juce::String& detail);

/** Schreibt die 112 HOST-Parameter als Eigenschaften (bool / double /
    Enumwort) in Vertragsreihenfolge, dazu `dsp_schema_version` = 2.
    `occupied` gehoert NICHT hierher, sondern in das Kind `Dsp`.
    Bestehende Eigenschaften werden ueberschrieben, fremde bleiben. */
void schreibeInBaum (const Satz& s, juce::ValueTree& parameters);

/** Liest die Host-Parameter aus `Parameters`.

    Fehlt die Eigenschaft `dsp_schema_version`, ist der Knoten im Layout v1:
    die 109 v1-Werte werden gelesen, die drei v2-Host-Parameter bekommen ihren
    Default, und `layoutV1` wird true. Traegt sie 2, werden alle 112 gelesen.
    Jede andere Zahl ist ein unbekanntes Layout-Major und ergibt false.

    Die acht `occupied` bleiben unberuehrt (Default false); sie kommen aus dem
    Kind `Dsp` bzw. aus der Migration. Fehlt eine erwartete Eigenschaft oder
    hat sie den falschen Typ, ist das KEIN Teilstate (false + `grund`).
    Zusaetzliche Eigenschaften werden ignoriert (additiv). */
bool leseAusBaum (const juce::ValueTree& parameters, Satz& aus, bool& layoutV1, juce::String& grund);

/** Migration v1 → v2 (SONDE-015 R5), auf einem bereits gelesenen Satz:

        occupied[slot] := enabled[slot] ODER mindestens einer der dreizehn
        v1-Werte des Slots weicht BITGENAU vom Vertragsdefault ab.

    Bitgenau, nicht mit Epsilon: ein Epsilonvergleich laese einen minimal
    verstellten Wert als unberuehrt und verloere ein vom User gesetztes Band. */
void setzeOccupiedAusV1 (Satz& s);

/** Weicht der Wert bitgenau vom Vertragsdefault ab? (oeffentlich, weil die
    Migrationsregel damit messbar ist und nicht nur behauptet). */
bool weichtVomDefaultAb (int index, const Zelle& z);

} // namespace nakama::parameter
