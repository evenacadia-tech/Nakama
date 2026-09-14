/*  NAK-286 Etappe 2 - die Antwort des Diagnose-Briefkastens (F-4 bis F-6).

    ZIELSCHICHT, nicht Kern: wird je Ziel uebersetzt (Gen, Probeeq, Beine).
    Hier steht, was die beiden Prozessoren zu einer Antwort beitragen:

      * die Wurzel des Briefkastens unter %LOCALAPPDATA% - sie traegt den
        eingefrorenen Herstellernamen und darf deshalb nicht im Kern stehen
        (Manifest §20 P-11, §7.3 Punkt 9; Muster PipeToken);
      * das Snapshot-v3-Objekt, herausgeloest aus `schreibeSnapshotDatei` OHNE
        Aenderung der Feldreihenfolge (F-5, M-39): der Knopfweg und die
        Antwort bauen es mit derselben Funktion;
      * der Rahmenauszug `frame` mit der Gruppe Materialzeit (F-6) und die
        Zaehler, die der Worker dafuer fuehrt (§16.2 P-10);
      * der Umschlag nach nakama.diagnose.antwort.v1 (F-4).

    Kein Aufruf hier nimmt eine Sperre: der Prozessor kopiert unter seiner
    bestehenden Sperre und reicht die Kopie herein (F-15).
*/

#pragma once

#include "AnalyseEngine.h"
#include "StampedAudioQueue.h"
#include "analysis/FeatureEngine.h"

#include <juce_core/juce_core.h>

#include <cstdint>
#include <string>

namespace nakama::diagnose
{

/** Die drei Ebenen unter %LOCALAPPDATA% (F-1). Der Antwortordner `antwort\`
    und `anfrage.json` darunter nennt der Kern selbst. */
inline constexpr const wchar_t* kWurzelRelativ = L"evenacadia\\nakama\\diagnose";
inline constexpr const char*    kAntwortFormat = "nakama.diagnose.antwort.v1";

//== Snapshot-v3-Objekt (F-5) ===================================================

struct SnapshotSensor
{
    juce::String sensorId;
    juce::String rolle;
    juce::String label;
    juce::String paarId;
    int          kanaele = 0;
};

/** Die Befundkarten: nur Gen. Probeeq schreibt weder `befunde` noch
    `diagnose_version` (T-8). */
struct Befundteil
{
    juce::String            diagnoseVersion;
    juce::Array<juce::var>  befunde;
};

/** Das Objekt nach eq-snapshot.schema.json, in der Feldreihenfolge des
    Knopfwegs. Nur fuer `m.zustand != keineDaten`. */
juce::var snapshotObjekt (const eqcop::MessSnapshot& m, const SnapshotSensor& sensor,
                          const juce::String& createdUtc, const Befundteil* befundteil);

//== Materialzeit (F-6, §16.2 P-10) ===========================================

struct MaterialStand
{
    bool          endeGueltig = false;
    std::int64_t  ende = 0;              ///< material_ende_projektsample
    bool          fortlaufendGueltig = false;
    std::int64_t  fortlaufend = 0;       ///< hostzeit_fortlaufend_samples
    bool          stillstandGueltig = false;
    std::uint64_t stillstand = 0;        ///< hostzeit_stillstand_bloecke
};

/** Gefuehrt im Worker, unter der bestehenden Sperre, je in die Engine
    gegebenem Block; zurueckgesetzt mit jedem Ruecksetzen der Engine;
    festgehalten unmittelbar vor der Publikation. Das Plugin rechnet die
    Fortlaufbedingung nie - das tut der Runner (F-28). */
class MaterialZaehler
{
public:
    void zuruecksetzen() noexcept;
    void blockGegeben (const nakama::echtzeit::StampedBlock& block) noexcept;
    void festhalten() noexcept;

    MaterialStand festgehalten() const noexcept { return fest; }
    /** Groesster `sampleCount` seit Instanzstart (kumulativ, roh). */
    std::uint64_t bloeckeMax() const noexcept   { return groessterBlock; }

private:
    bool          habeBlock = false;
    bool          zeitDurchgehend = true;
    bool          uebergelaufen = false;
    bool          letzteZeitGueltig = false;
    std::int64_t  letzterStart = 0;
    std::uint32_t letzteLaenge = 0;
    std::int64_t  summe = 0;
    std::uint64_t stillstand = 0;
    std::uint64_t groessterBlock = 0;
    MaterialStand fest {};
};

//== Rahmenauszug (F-6) =========================================================

struct RahmenAuszug
{
    nakama::analyse::FeatureFrame rahmen {};
    std::uint64_t framesGebaut = 0;
    std::uint64_t summeGesamt = 0;
    std::uint64_t summeAktiv = 0;
    std::uint64_t offenGesamt = 0;
    std::uint64_t offenAktiv = 0;
    MaterialStand material {};
    std::uint64_t bloeckeMax = 0;
    std::uint64_t schwerSamples = 0;   ///< MessSnapshot::schwerVerarbeiteteSamples (P-8)
    double        samplerate = 0.0;    ///< des Snapshots
};

juce::var rahmenObjekt (const RahmenAuszug& auszug);

//== Umschlag (F-4) =============================================================

struct Umschlag
{
    std::string            anfrageId;
    juce::String           rolle;
    juce::String           instanzId;
    std::string            laufzeitId;
    std::uint32_t          pid = 0;
    std::string            erzeugtUtc;
    juce::String           version;
    juce::var              snapshot;
    juce::var              frame;
    juce::Array<juce::var> gruende;
};

/** Setzt `snapshot`, `frame` und `gruende` nach F-4 aus einer Kopie. `snapshot`
    ist das fertige Objekt (Gen mit, Probeeq ohne Befundkarten); ohne Messdaten
    steht es null. `frame` ist null nur, wenn dazu kein Rahmen gebaut ist;
    `aggregat` ist fuer Plugins immer null. Jedes null nennt seinen Grund. */
void fuelleTeile (const eqcop::MessSnapshot& m, const RahmenAuszug& auszug,
                  const juce::var& snapshot, Umschlag& u);

/** Der Umschlag als UTF-8-JSON ohne BOM. */
std::string umschlagText (const Umschlag& u);

} // namespace nakama::diagnose
