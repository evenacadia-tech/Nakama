// EqCopHostProbe — WEGWERF-MESSGERAET fuer User-Termin B (Session S3b).
//
// Zweck: in FL Studio ablesbar machen, was der Host bei Live, Stop, Seek,
// Loop-Straddle, Offline-Render und Smart Disable wirklich liefert — und ob
// samplegenaue Automation ankommt. Das ist mit dem Auge nicht messbar: VST3
// setzt Validity-Bits, die keine Oberflaeche zeigt.
//
// Es ist das erste Ziel, das die Hostbruecke aus SONDE-003 BENUTZT: der
// Processor ist eine `eqcop::hostbruecke::Senke` und bekommt die Rohdaten
// unmittelbar vor jedem processBlock. Ohne den Bridge-Patch gaebe es hier
// nichts zu messen — JUCEs oeffentlicher Pfad kann „Context vorhanden" gar
// nicht beantworten.
//
// GRUNDGESETZ: Es misst und zeigt, es greift NIE ins Audio ein. Der Ausgang
// bleibt bitgleich zum Eingang, 0 Samples gemeldete Latenz, kein Tail. Auf dem
// Audiothread: keine Allokation, kein Lock, kein I/O, kein Logging.
//
// Ausdruecklich KEIN Produktcode. Eigener Plugin-Code `NkHp` — ausserhalb der
// eingefrorenen Identitaet (Entwurf §31.2 reserviert `Eqcp`, `NkPr`, `NkAc`)
// und ausserhalb des Aux-Spikes (`NkSp`). Wird nach dem Capabilityreport (S4)
// nicht weitergepflegt.
#pragma once

#include "NakamaHostBridge.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <cstring>

namespace eqcop::hostprobe
{

/** Fassungsvermoegen des Ereignisprotokolls. Ereignisse sind Wechsel, keine
    Bloecke — eine halbe Stunde Arbeit erzeugt Dutzende, nicht Tausende. */
inline constexpr int kMaxEreignisse = 512;

/** Was ein Ereignis ist. Jede Art beantwortet genau eine Termin-B-Frage. */
enum class Art
{
    ErsterBlock,
    KontextDa,
    KontextWeg,
    TransportAn,
    TransportAus,
    AufnahmeAn,
    AufnahmeAus,
    SchleifeAn,
    SchleifeAus,
    ZeitsprungVor,       ///< Projektzeit springt vorwaerts (Seek vorwaerts ODER Smart-Disable-Luecke)
    ZeitsprungZurueck,   ///< Projektzeit springt rueckwaerts (Seek zurueck ODER Loop-Wechsel)
    OfflineAn,
    OfflineAus,
    GenauigkeitFloat,
    GenauigkeitDouble,
    LatenzGemeldet,
    AutomationPunkt,       ///< erster Automationspunkt ueberhaupt
    AutomationMehrpunkt,   ///< mehr als EIN Punkt in einem Block — der Kernbeweis
    AutomationUnplausibel,
    BlockOhneVerarbeitung, ///< Senke gerufen, processBlock danach nicht (Flush/Hostriegel)
};

inline constexpr int kArtAnzahl = (int) Art::BlockOhneVerarbeitung + 1;

/** Ein Ereignis. POD, damit es ohne Allokation in den Ring passt. */
struct Ereignis
{
    Art          art          { Art::ErsterBlock };
    juce::int64  block        { 0 };
    juce::int64  projektZeit  { -1 };   ///< -1 = nicht bekannt (NICHT 0)
    juce::int64  vorherige    { -1 };
    double       ppq          { 0.0 };
    double       zusatz       { 0.0 };  ///< je nach Art: Sprungweite, Latenz, Punktzahl
    int          ganzzahl     { -1 };   ///< je nach Art: Busindex, Blockgroesse
};

/** Welche Hostangaben je gueltig waren. Ein Bit je Feld — so laesst sich
    „hat FL das je gesagt?" von „hat FL 0 gesagt?" trennen. */
struct Gueltigkeiten
{
    bool projektZeit  { false };
    bool fortlaufend  { false };   ///< continousTimeSamples (SDK-Schreibweise)
    bool tempo        { false };
    bool ppq          { false };
    bool taktbeginn   { false };
    bool schleife     { false };
    bool samplerate   { false };
};

/** Der Messstand. POD, damit ihn ein Seqlock am Stueck kopieren kann. */
struct Messstand
{
    juce::int64 bloecke              { 0 };
    juce::int64 senkeAufrufe         { 0 };
    juce::int64 verarbeiteteBloecke  { 0 };
    juce::int64 bloeckeMitKontext    { 0 };
    juce::int64 bloeckeOhneKontext   { 0 };
    juce::int64 bloeckeOffline       { 0 };
    juce::int64 bloeckeEchtzeit      { 0 };
    juce::int64 bloeckeFloat         { 0 };
    juce::int64 bloeckeDouble        { 0 };

    Gueltigkeiten jeGueltig  {};   ///< mindestens einmal gueltig gewesen
    Gueltigkeiten immerGueltig {}; ///< in JEDEM Kontextblock gueltig

    juce::int64 zeitspruengeVor      { 0 };
    juce::int64 zeitspruengeZurueck  { 0 };
    juce::int64 groessterSprungVor   { 0 };
    juce::int64 groessterSprungZur   { 0 };

    juce::int64 automationPunkte     { 0 };
    int         maxPunkteProBlock    { 0 };
    juce::int64 bloeckeMitMehrpunkt  { 0 };
    int         kleinsterOffset      { -1 };
    int         groesterOffset       { -1 };
    juce::int64 automationUeberlaeufe { 0 };
    juce::int64 automationUnplausibel { 0 };

    bool        latenzJeGemeldet     { false };
    int         latenzEingangBus     { -1 };
    juce::uint32 latenzEingangWert   { 0 };
    int         latenzAusgangBus     { -1 };
    juce::uint32 latenzAusgangWert   { 0 };

    double      samplerate           { 0.0 };
    int         kleinsterBlock       { -1 };
    int         groessterBlock       { -1 };

    juce::int64 letzteProjektZeit    { -1 };
    double      letztesPpq           { 0.0 };
    double      letztesTempo         { 0.0 };
    bool        spieltGerade         { false };
    bool        schleifeAktiv        { false };
    bool        kontextZuletztDa     { false };

    juce::int64 ereignisseGesamt     { 0 };   ///< inkl. der nicht mehr gespeicherten
    juce::int64 artZaehler[kArtAnzahl] {};    ///< je Art, ueberlebt den Ringueberlauf
};

//==============================================================================
class HostProbeProcessor final : public juce::AudioProcessor,
                                 public eqcop::hostbruecke::Senke
{
public:
    HostProbeProcessor();
    ~HostProbeProcessor() override = default;

    //== AudioProcessor ======================================================
    const juce::String getName() const override { return "EqCop-Host-Probe"; }

    void prepareToPlay (double samplerate, int blockgroesse) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;

    void processBlock (juce::AudioBuffer<float>&,  juce::MidiBuffer&) override;
    void processBlock (juce::AudioBuffer<double>&, juce::MidiBuffer&) override;
    bool supportsDoublePrecisionProcessing() const override { return true; }

    double getTailLengthSeconds() const override { return 0.0; }
    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Messung"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}

    //== Senke (Audiothread) =================================================
    void nakamaBlockEmpfangen (const eqcop::hostbruecke::Blockbefund&) noexcept override;

    //== Nachrichtenthread ===================================================
    /** Konsistenter Schnappschuss ueber einen Seqlock — kein Lock fuer den
        Audiothread, der Leser wiederholt notfalls. */
    Messstand messstand() const noexcept;

    /** Kopiert das Ereignisprotokoll. Setzt waehrenddessen die Aufzeichnung
        aus, damit kein Eintrag unter dem Leser weggeschrieben wird; danach
        laeuft sie weiter. Der Audiothread WARTET dabei nie — er ueberspringt
        nur das Anhaengen; die Zaehler laufen ungestoert weiter. */
    int ereignisseLesen (Ereignis* ziel, int platz) const noexcept;

    void messungZuruecksetzen() noexcept;

    juce::String berichtAlsJson() const;
    static juce::File berichtsOrdner();
    juce::File berichtSchreiben() const;

    /** true, sobald die Hostbruecke mindestens einen Block geliefert hat.
        Ist das dauerhaft false, laeuft ein Bau OHNE Bridge-Patch. */
    bool brueckeLiefert() const noexcept { return brueckeGesehen.load (std::memory_order_relaxed); }

    static const char* artName (Art) noexcept;

private:
    void merke (Art, juce::int64 projektZeit, juce::int64 vorherige,
                double ppq, double zusatz, int ganzzahl) noexcept;
    void zaehleBlock (bool doppeltePraezision, int samples, bool offline) noexcept;

    // Seqlock: gerade Generation = stabil, ungerade = Schreibvorgang laeuft.
    mutable std::atomic<juce::uint32> generation { 0 };
    Messstand stand {};

    // mutable, weil das Protokoll-Lesen den Messstand nicht veraendert — es
    // pausiert nur kurz das Anhaengen. Ohne mutable braeuchte es einen
    // const_cast, und der verdeckt genau diese Absicht.
    mutable Ereignis         protokoll[kMaxEreignisse] {};
    mutable std::atomic<int>  schreibIndex { 0 };
    mutable std::atomic<bool> aufzeichnung { true };
    std::atomic<bool> brueckeGesehen { false };
    std::atomic<bool> zuruecksetzen { false };

    // Nur vom Audiothread benutzt — kein Atomic noetig, keins vorgetaeuscht.
    bool        ersterBlockGesehen    { false };
    bool        senkeSeitLetztemBlock { false };
    bool        spielteVorher         { false };
    bool        aufnahmeVorher        { false };
    /** Nach Transportwechsel, Kontextverlust und Reset ist JEDER Zeitwert
        legitim — sonst meldete der erste Block nach Play einen Riesensprung. */
    bool        zeitBasisNeu          { true };
    juce::int64 letzteBlockGroesse    { 0 };

    juce::AudioParameterFloat* testwert { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HostProbeProcessor)
};

} // namespace eqcop::hostprobe
