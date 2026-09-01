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

/** Welche Klassen ein Bundle laden darf (§2.3 des Vertrags). */
struct Bundle
{
    std::vector<Klasse> klassen;
    bool erlaubt (Klasse k) const noexcept;
    static Bundle eqcp()   { return { { Klasse::main, Klasse::legacy } }; }
    static Bundle nkpr()   { return { { Klasse::passive_probe } }; }
    static Bundle nkac()   { return { { Klasse::active_probe } }; }
};

struct Zustand
{
    /** Der gehaltene Baum (NakamaState). Traegt auch Eigenschaften, die dieser
        Build nicht kennt - sie werden beim Speichern zurueckgeschrieben. */
    juce::ValueTree baum;
    Common common;
    std::vector<MainProjectMitglied> mainProjectMitglieder;
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

/** Lesbarer Text fuer Manifeste und Tests (XML-Form des Baums). */
juce::String alsText (const Zustand& z);

} // namespace nakama::state
