// SONDE-008 (S10-11) — zeitgestempelte Ganzblock-Warteschlange, Entwurf §53.7,
// §48.1, §32.3.
//
// WOZU. Der heutige Analyseweg ist ein juce::AbstractFifo ueber interleavten
// Samples (PluginProcessor.cpp, Stand vor diesem Ticket): `prepareToWrite (n,
// s1, n1, s2, n2)` liefert zurueck, wieviel GERADE passt - bei Platzmangel
// schreibt der Audiothread einen TEILBLOCK und zaehlt den Rest als
// `framesDropped`. Fuer jede zeitabhaengige Analyse ist das die schlimmste
// Verlustform, die es gibt: der Worker sieht danach einen lueckenlosen
// Samplestrom, dem in der Mitte Zeit fehlt, und kann das nicht mehr sehen.
// Entwurf §48.1 sagt darum woertlich: die Blockqueue veroeffentlicht
// `{project_sample_start, sample_count, flags, audio}` "ausschliesslich ganz
// oder gar nicht", und "Reduktion erfolgt ueber Kadenz und Ganzblockdrop, nie
// ueber Teilblockkopien" (§53.7).
//
// DIE FORM: ZWEI feste SPSC-Ringe (§53.7).
//   * Sample-Ring    - Startbudget `budgetFrames` je im Layout deklariertem
//                      Stereo-Tap, EINMAL alloziert.
//   * Deskriptor-Ring - `deskriptorPlaetze` vollstaendige `StampedBlock`.
// Ein Ring allein koennte "ganz oder gar nicht" gar nicht ausdruecken: erst der
// Deskriptor, der NACH den Samples mit Release-Semantik erscheint, macht einen
// Block fuer den Consumer sichtbar. Reicht EINER der beiden Ringe nicht, wird
// der komplette Analyseblock verworfen, ein Zaehler steigt, und der naechste
// angenommene Deskriptor beginnt ein neues `continuity_segment`.
//
// GRUNDGESETZ. Der Produzentenpfad laeuft auf dem Audiothread: keine
// Allokation, kein Lock, kein I/O, kein Logging, keine Ausnahmen. Der gesamte
// Backing-Store entsteht in `vorbereiten()` (Nachrichten-/Hostthread, aus
// prepareToPlay). Ueberlast verwirft ANALYSEFRAMES, nie Audio - diese Klasse
// bekommt den Audiopuffer nur lesend zu sehen.
//
// EHRLICHKEIT STATT SANIERUNG (dieselbe Regel wie in NakamaHostBridge.h):
//   * Der lokale Strom (`stromVon`) zaehlt JEDEN Hostblock mit, auch den
//     verworfenen. Ein Zaehler, der nur die angenommenen Frames zaehlt, koennte
//     "hier fehlt Zeit" nicht mehr ausdruecken - genau der Fehler, den der
//     AbstractFifo-Weg macht.
//   * `projectSampleStart` traegt ein eigenes Gueltigkeitsbit. Ohne Hostkontext
//     ist Projektzeit unbewiesen (§32.3) - dann gilt nur der lokale monotone
//     Verlauf (§53.6, Verhalten ohne Beweis von `project_time_samples`).
//   * Ein Hostblock ueber `maxBlockFrames` wird als GANZES nur fuer die Analyse
//     verworfen (`oversizeDrops`) - `maximumExpectedSamplesPerBlock` ist bei
//     JUCE nur ein Hinweis (§48.1), FL zerteilt Puffer sogar bis auf 1 Sample
//     (Capabilityreport S4). Der Audiopfad beruehrt das nicht.
//
// WARUM `maxBlockFrames` eine FESTE Konstante des Layouts ist und nicht aus
// `maximumExpectedSamplesPerBlock` abgeleitet wird: der Entwurf nennt den Hinweis
// selbst unzuverlaessig. Eine Slotkapazitaet, die an einem unzuverlaessigen Wert
// haengt, waere zur Laufzeit nicht mehr vorhersagbar - und die Oversize-Regel
// waere nicht deterministisch pruefbar. Die feste Grenze steht stattdessen in
// der QA-Matrix (§48.1 letzter Satz: "Slotkapazitaet, groesste getestete
// Hostblockgroesse und Chunkgroesse gehoeren zur Capability-/QA-Matrix").
#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

namespace nakama::echtzeit
{

//==============================================================================
// Flags eines Blocks. Bewusst ein kleines, dokumentiertes Bitfeld - jede
// Bedeutung steht hier und nirgends sonst.
inline constexpr std::uint32_t kFlagKontextAnwesend = 1u << 0;  // ProcessData.processContext lag an
inline constexpr std::uint32_t kFlagZeitGueltig     = 1u << 1;  // projectSampleStart ist bewiesen
inline constexpr std::uint32_t kFlagSpieltGueltig   = 1u << 2;  // "spielt" ist bewiesen (nicht geraten)
inline constexpr std::uint32_t kFlagSpielt          = 1u << 3;  // Transport laeuft
inline constexpr std::uint32_t kFlagLueckeDavor     = 1u << 4;  // vor diesem Block ging Analysezeit verloren
inline constexpr std::uint32_t kFlagNichtEchtzeit   = 1u << 5;  // Offline-Render (isNonRealtime)

// ── SONDE-009: was der Zeit- und Validity-Vertrag zusaetzlich braucht ───────
// Bis S12-13 trug der Stempel nur, was die Ein-Block-Quarantaene fuer ihre
// Grenzentscheidung braucht. Der Transportstempel des v3-Vertrags (§32.3,
// `nakama_telemetry_v1.fbs` Tabelle `Transportstempel`) verlangt mehr, und
// zwar mit je EIGENEM Gueltigkeitsbit - allen voran die Schleifengrenzen: ohne
// sie kann die Fensterbuchhaltung einen Loop-Wrap nicht von einem Seek
// unterscheiden, und "Loop trennt jedes offene Fenster" waere unbelegbar.
//
// ⚠️ ALLE NEUEN BITS SIND ADDITIV UND STEHEN BEWUSST NICHT IN DER `bruchMaske`
// von `schliesstAn()`. Diese Klasse liefert die Grenze; die Fensterbuchhaltung
// wertet sie aus (§4.3 des SONDE-008-Manifests: "Nicht hier, sondern in
// SONDE-009"). Waeren sie in der Maske, aenderte ein blosser Loop-Einschalter
// das Verhalten eines T2-geprueften Riegels - und B4 mit ihm.
inline constexpr std::uint32_t kFlagRecordingGueltig  = 1u << 6;
inline constexpr std::uint32_t kFlagRecording         = 1u << 7;
inline constexpr std::uint32_t kFlagContinuousGueltig = 1u << 8;
inline constexpr std::uint32_t kFlagCycleAktiv        = 1u << 9;   // Schleife laeuft gerade
inline constexpr std::uint32_t kFlagCycleGrenzenGueltig = 1u << 10; // rohe PPQ-Grenzen liegen an
inline constexpr std::uint32_t kFlagTempoGueltig      = 1u << 11;
inline constexpr std::uint32_t kFlagPpqGueltig        = 1u << 12;
inline constexpr std::uint32_t kFlagSampleRateGueltig = 1u << 13;
inline constexpr std::uint32_t kFlagEingangLatenzGemeldet = 1u << 14;
inline constexpr std::uint32_t kFlagAusgangLatenzGemeldet = 1u << 15;

/** Was der Audiothread ueber den Block weiss, bevor er ihn einstellt. Alles mit
    eigenem Gueltigkeitsbit - "nicht gesagt" ist nicht "0". */
struct Stempel
{
    bool         kontextAnwesend  { false };
    bool         zeitGueltig      { false };
    std::int64_t projectSampleStart { 0 };
    bool         spieltGueltig    { false };
    bool         spielt           { false };
    bool         nichtEchtzeit    { false };

    // ── SONDE-009 ──
    bool         recordingGueltig  { false };
    bool         recording         { false };
    bool         continuousGueltig { false };
    std::int64_t continuousTimeSamples { 0 };
    bool         cycleAktiv        { false };
    bool         cycleGrenzenGueltig { false };
    double       cycleStartPpq     { 0.0 };
    double       cycleEndePpq      { 0.0 };
    bool         tempoGueltig      { false };
    double       tempo             { 0.0 };
    bool         ppqGueltig        { false };
    double       ppqPosition       { 0.0 };
    bool         sampleRateGueltig { false };
    double       sampleRate        { 0.0 };
    bool          eingangLatenzGemeldet { false };
    std::uint32_t eingangLatenzSamples  { 0 };
    bool          ausgangLatenzGemeldet { false };
    std::uint32_t ausgangLatenzSamples  { 0 };

    std::uint32_t flags() const noexcept
    {
        std::uint32_t f = 0;
        if (kontextAnwesend) f |= kFlagKontextAnwesend;
        if (zeitGueltig)     f |= kFlagZeitGueltig;
        if (spieltGueltig)   f |= kFlagSpieltGueltig;
        if (spielt)          f |= kFlagSpielt;
        if (nichtEchtzeit)   f |= kFlagNichtEchtzeit;
        if (recordingGueltig)      f |= kFlagRecordingGueltig;
        if (recording)             f |= kFlagRecording;
        if (continuousGueltig)     f |= kFlagContinuousGueltig;
        if (cycleAktiv)            f |= kFlagCycleAktiv;
        if (cycleGrenzenGueltig)   f |= kFlagCycleGrenzenGueltig;
        if (tempoGueltig)          f |= kFlagTempoGueltig;
        if (ppqGueltig)            f |= kFlagPpqGueltig;
        if (sampleRateGueltig)     f |= kFlagSampleRateGueltig;
        if (eingangLatenzGemeldet) f |= kFlagEingangLatenzGemeldet;
        if (ausgangLatenzGemeldet) f |= kFlagAusgangLatenzGemeldet;
        return f;
    }
};

/** Der versiegelte Deskriptor eines GANZEN Hostblocks (§48.1:
    `{project_sample_start, sample_count, flags, audio}`). `audio` steckt nicht
    hier drin, sondern im Sample-Ring an `ringVon` - der Deskriptor ist der
    Schluessel dazu. */
struct StampedBlock
{
    /** Hostzeit. NUR gueltig, wenn `flags & kFlagZeitGueltig`. */
    std::int64_t  projectSampleStart { 0 };
    /** Lokaler, monotoner Strom seit `zuruecksetzen()`. Zaehlt AUCH die Frames
        verworfener Bloecke - sonst koennte "hier fehlt Zeit" nicht ausgedrueckt
        werden. Immer gueltig, auch ohne Hostkontext. */
    std::uint64_t stromVon         { 0 };
    /** Absolute Frameposition im Sample-Ring (nicht maskiert). */
    std::uint64_t ringVon          { 0 };
    /** Fortlaufende Nummer des Kontinuitaets-Segments. Steigt beim ersten
        angenommenen Block nach einem Verlust. */
    std::uint64_t segment          { 0 };
    /** Zaehlt die Neustarts (prepareToPlay). Bloecke aus einem aelteren Anlauf
        traegt der Consument NICHT in die Analyse - dort kann die Samplerate
        eine andere gewesen sein. Ersetzt das `fifo.reset()`, das bisher der
        Nachrichtenthread in einen laufenden Leser hinein rief. */
    std::uint32_t startFolge       { 0 };
    std::uint32_t sampleCount      { 0 };
    std::uint32_t flags            { 0 };
    /** 1 oder 2. Bei 1 traegt der Ring das L-Duplikat auf dem R-Kanal - so wie
        der bisherige FIFO-Vertrag es tat (AnalyseEngine::verarbeite). */
    std::uint32_t kanaele          { 0 };
    /** Welche Taps in diesem Block belegt sind (Bit i = Tap i). Ein
        Laufzeit-Bitset verhindert Kopien fuer inaktive Taps, ohne Speicher
        nachzuallozieren (§53.7). */
    std::uint32_t tapMaske         { 0 };

    // ── SONDE-009: Traeger des v3-Transportstempels (§32.3) ────────────────
    // Jedes Feld gilt NUR mit seinem Flag. Ohne Flag steht hier der
    // Nullwert - nie ein fortgeschriebener alter Wert, denn genau das war
    // NAK-24 (ein alter `projektZeitSamples` sah aus wie eine aktuelle
    // Position).
    /** NUR gueltig mit `kFlagContinuousGueltig`. */
    std::int64_t  continuousTimeSamples { 0 };
    /** NUR gueltig mit `kFlagCycleGrenzenGueltig`. Rohe PPQ, keine Samples. */
    double        cycleStartPpq   { 0.0 };
    double        cycleEndePpq    { 0.0 };
    /** NUR gueltig mit `kFlagTempoGueltig` bzw. `kFlagPpqGueltig`. Beide
        zusammen sind die Voraussetzung dafuer, aus PPQ-Grenzen ueberhaupt
        Samplegrenzen ABLEITEN zu duerfen (§32.3). */
    double        tempo           { 0.0 };
    double        ppqPosition     { 0.0 };
    /** Kontext-Samplerate des Hosts, roh. NUR mit `kFlagSampleRateGueltig`. */
    double        sampleRate      { 0.0 };
    /** NUR gueltig mit dem jeweiligen `…LatenzGemeldet`-Flag. 0 ohne Flag heisst
        "nie gesagt", 0 mit Flag heisst "der Host hat 0 gesagt" - genau die
        Unterscheidung, die §32.3 verlangt. */
    std::uint32_t eingangLatenzSamples { 0 };
    std::uint32_t ausgangLatenzSamples { 0 };
};

//==============================================================================
/** Compile-time-Layout eines Produktstroms (§53.7 `StampedAudioQueue<StreamLayout>`).

    Alle vier Zahlen sind Startwerte, keine ABI (§53.7 Schlussabsatz: "Die
    Startgroessen sind keine ABI. P2 misst High-Water-Marks und schreibt
    gewaehlte Caps in die Capability-/QA-Matrix"). */
template <int TapsN, int BudgetFramesN, int DeskriptorPlaetzeN, int MaxBlockFramesN>
struct StreamLayout
{
    static_assert (TapsN >= 1 && TapsN <= 32,
                   "tapMaske ist 32 Bit breit.");
    static_assert (BudgetFramesN > 0 && (BudgetFramesN & (BudgetFramesN - 1)) == 0,
                   "Sample-Ring braucht eine Zweierpotenz: der Umlauf ist eine Maske, keine Division.");
    static_assert (DeskriptorPlaetzeN > 1 && (DeskriptorPlaetzeN & (DeskriptorPlaetzeN - 1)) == 0,
                   "Deskriptor-Ring braucht eine Zweierpotenz.");
    static_assert (MaxBlockFramesN > 0 && MaxBlockFramesN <= BudgetFramesN,
                   "Ein Block, der nie in den Ring passt, waere ein Drop, den kein Betrieb je aufloest.");

    static constexpr int taps              = TapsN;
    static constexpr int budgetFrames      = BudgetFramesN;
    static constexpr int deskriptorPlaetze = DeskriptorPlaetzeN;
    static constexpr int maxBlockFrames    = MaxBlockFramesN;

    static constexpr std::uint64_t rahmenMaske = (std::uint64_t) BudgetFramesN - 1u;
    static constexpr std::uint64_t deskMaske   = (std::uint64_t) DeskriptorPlaetzeN - 1u;
};

/** Der Strom des heutigen Gen/Main-Bundles: EIN Stereo-Tap (der Insert-Abgriff
    von `processBlock`). Probeeq bekommt laut §53.7 hoechstens `pre_nakama`,
    `post_committed`, `post_candidate`, `priority_sidechain` und `compare_pre` -
    die entstehen mit ihrem DSP in P6, nicht hier. Die Klasse ist deshalb ein
    Template und kein Einzweckring: die Tapzahl ist ein Parameter, keine
    Kopie der Implementierung. */
using GenStrom = StreamLayout<1, 131072, 2048, 16384>;

//==============================================================================
/** Ganz-oder-gar-nicht-SPSC-Queue zwischen Audiothread (ein Produzent) und
    Worker (ein Consument).

    NEBENLAEUFIGKEITSVERTRAG, und er ist knapp gehalten, damit er pruefbar ist:
      * `veroeffentliche()` ruft NUR der Audiothread.
      * `spitze()`, `lies()`, `freigeben()` ruft NUR der Worker.
      * `vorbereiten()`/`zuruecksetzen()` ruft NUR der Nachrichtenthread, und
        nur wenn der Audiothread nicht laeuft (aus `prepareToPlay`, wie der
        alte `fifo.reset()`).
      * Die Ordnung haengt an EINEM Paar: der Produzent legt `deskSchreib` mit
        `release` ab, der Consument liest es mit `acquire`. Alles, was der
        Produzent VORHER geschrieben hat (Samples, Deskriptorfelder), ist damit
        sichtbar. Ein zweites Ordnungspaar gaebe es nicht umsonst - es gaebe nur
        eine zweite Stelle, an der die Ordnung falsch sein kann. */
template <typename Layout>
class StampedAudioQueue
{
public:
    using LayoutTyp = Layout;

    /** Eine Tapquelle des Blocks. `rechts == nullptr` heisst Mono: der Ring
        bekommt L auf beiden Seiten (FIFO-Vertrag der AnalyseEngine). */
    struct TapQuelle
    {
        const float* links  { nullptr };
        const float* rechts { nullptr };
    };

    StampedAudioQueue() = default;
    StampedAudioQueue (const StampedAudioQueue&) = delete;
    StampedAudioQueue& operator= (const StampedAudioQueue&) = delete;

    //== Einrichtung ==========================================================

    /** Legt den GESAMTEN Backing-Store an (§53.7: "der gesamte Backing-Store
        entsteht in prepareToPlay"). NUR aufrufen, solange weder Produzent noch
        Consument laufen - im Prozessor also im Konstruktor, vor dem Start des
        Workerthreads. Ein zweiter Aufruf alloziert nicht erneut. */
    void vorbereiten()
    {
        const std::size_t noetig = (std::size_t) Layout::taps
                                 * (std::size_t) Layout::budgetFrames * 2u;
        if (ring.size() != noetig)
            ring.assign (noetig, 0.0f);
        if (deskriptoren.size() != (std::size_t) Layout::deskriptorPlaetze)
            deskriptoren.assign ((std::size_t) Layout::deskriptorPlaetze, StampedBlock {});

        deskSchreib.store (0, std::memory_order_relaxed);
        deskLese.store (0, std::memory_order_relaxed);
        lesePos.store (0, std::memory_order_relaxed);
        ringSchreib = 0;
        stromPos    = 0;
        segment     = 0;
        startFolge  = 0;
        lueckeOffen = false;
        aktuelleStartFolge.store (0, std::memory_order_relaxed);
        neustartWunsch.store (false, std::memory_order_relaxed);
    }

    /** Meldet einen Neuanlauf an (aus `prepareToPlay`, beliebiger Thread).

        WARUM KEIN `zuruecksetzen()`: der bisherige Weg rief `fifo.reset()` vom
        Nachrichtenthread aus mitten in einen laufenden Leser hinein - beide
        Enden eines SPSC-Rings von einem dritten Thread aus zu verstellen, ist
        genau die Stelle, an der ein Ring still Muell liefert. Stattdessen
        NIMMT DER PRODUZENT den Wunsch entgegen: er uebernimmt die neue
        `startFolge` und oeffnet eine Luecke. Der Consument erkennt Bloecke aus
        dem alten Anlauf an ihrer kleineren `startFolge` und traegt sie nicht in
        die Analyse - dasselbe Ergebnis wie der alte Reset, ohne den
        Fremdzugriff.

        ⚠️ DER ANLAUF STEIGT HIER, NICHT ERST BEIM NAECHSTEN AUDIOBLOCK (T2-3,
        23.08.2026). Die erste Fassung setzte nur den Wunsch und liess den
        Produzenten `startFolge` erhoehen - aber `prepareToPlay` ruft hier
        typischerweise hinein, WAEHREND der Audiocallback steht. Bis zum
        naechsten Block blieb `aktuellerAnlauf()` also auf dem alten Wert, und
        die Bloecke, die schon im Ring lagen, sahen aus wie aktuelle: gemessen
        0 von 3 erkennbar. Weil der Worker am Kopf derselben Runde
        `engine.vorbereiten(neueSr)` ruft, wurden sie mit der NEUEN Samplerate
        analysiert - genau das, was diese Nummer verhindern soll. §32.3 fuehrt
        den Sampleratewechsel ausdruecklich als Epochengrenze.

        🔑 Die Reihenfolge der zwei Zeilen ist tragend, nicht Geschmack: erst
        den Anlauf erhoehen, DANN den Wunsch setzen. Umgekehrt koennte der
        Produzent den Wunsch verbrauchen und sich dabei die noch UNVERAENDERTE
        Nummer holen - der Neuanlauf waere spurlos verloren. In der gewaehlten
        Reihenfolge ist der schlimmste Fall, dass ein Block aus dem Augenblick
        dazwischen als veraltet verworfen wird; das ist die sichere Seite und
        genau das, was der alte `fifo.reset()` ohnehin tat. */
    void neustartAnfordern() noexcept
    {
        aktuelleStartFolge.fetch_add (1, std::memory_order_relaxed);
        neustartWunsch.store (true, std::memory_order_relaxed);
    }

    /** Der Anlauf, der gerade gilt (Consument: alles Kleinere ist veraltet). */
    std::uint32_t aktuellerAnlauf() const noexcept
    {
        return aktuelleStartFolge.load (std::memory_order_relaxed);
    }

    //== Produzent (Audiothread) ==============================================

    /** Stellt EINEN Hostblock vollstaendig ein - oder gar nicht.

        Rueckgabe `false` heisst: dieser Block ist fuer die Analyse verloren,
        der Zaehler dazu steht bereit, und der naechste angenommene Block traegt
        `kFlagLueckeDavor`. Audio ist davon nie betroffen; der Aufrufer
        uebergibt den Puffer nur lesend.

        `frames <= 0` ist KEIN Verlust: VST3 erlaubt `numSamples == 0`
        (Parameter-Flush, siehe NakamaHostBridge.h). Ein Flush traegt keine
        Audiozeit, also darf er auch keine Luecke erzeugen. */
    bool veroeffentliche (const TapQuelle* quellen, int anzahlQuellen,
                          int kanaele, int frames, const Stempel& stempel) noexcept
    {
        if (frames <= 0 || quellen == nullptr)
            return false;

        // Neuanlauf zuerst: er gilt ab DIESEM Block, nicht ab dem naechsten.
        // Die Nummer steht schon (s. `neustartAnfordern`) - hier wird sie nur
        // uebernommen, damit ab jetzt gestempelte Bloecke als aktuell gelten.
        if (neustartWunsch.exchange (false, std::memory_order_relaxed))
        {
            startFolge = aktuelleStartFolge.load (std::memory_order_relaxed);
            lueckeOffen = true;
        }

        // Der lokale Strom laeuft IMMER weiter - Annahme oder Verlust, das Audio
        // hat stattgefunden. Nur so kann `kFlagLueckeDavor` spaeter die Wahrheit
        // sagen.
        const std::uint64_t von = stromPos;
        stromPos += (std::uint64_t) frames;

        const auto verlust = [this, frames] (std::atomic<std::uint64_t>& zaehler) noexcept
        {
            zaehler.fetch_add (1, std::memory_order_relaxed);
            verworfeneFrames.fetch_add ((std::uint64_t) frames, std::memory_order_relaxed);
            lueckeOffen = true;
            return false;
        };

        if (anzahlQuellen != Layout::taps || kanaele < 1 || kanaele > 2)
            return verlust (ueberlaufDrops);

        // Oversize: §48.1. Der ganze Block faellt fuer die ANALYSE weg.
        if (frames > Layout::maxBlockFrames)
            return verlust (oversizeDrops);

        // Beide Ringe muessen reichen - erst reservieren, dann kopieren.
        const std::uint64_t belegt = ringSchreib - lesePos.load (std::memory_order_acquire);
        if ((std::uint64_t) Layout::budgetFrames - belegt < (std::uint64_t) frames)
            return verlust (ueberlaufDrops);

        const std::uint64_t schreib = deskSchreib.load (std::memory_order_relaxed);
        if (schreib - deskLese.load (std::memory_order_acquire) >= (std::uint64_t) Layout::deskriptorPlaetze)
            return verlust (ueberlaufDrops);

        // Kein einziger aktiver Tap: es gibt nichts zu analysieren. Die
        // Audiozeit ist trotzdem vergangen, also ist das ein VERLUST und kein
        // stillschweigend leerer Block - sonst reichte die Queue einen
        // Deskriptor weiter, zu dem `lies()` nichts liefern kann.
        std::uint32_t maske = 0;
        for (int t = 0; t < Layout::taps; ++t)
            if (quellen[t].links != nullptr)
                maske |= (std::uint32_t) 1u << t;
        if (maske == 0)
            return verlust (ueberlaufDrops);

        // --- ab hier ist der Block angenommen -------------------------------
        const std::uint32_t n = (std::uint32_t) frames;
        for (int t = 0; t < Layout::taps; ++t)
            if ((maske & ((std::uint32_t) 1u << t)) != 0)
                kopiereTap (t, quellen[t], kanaele, n);   // inaktiv ⇒ keine Kopie

        StampedBlock& b = deskriptoren[(std::size_t) (schreib & Layout::deskMaske)];
        b.projectSampleStart = stempel.zeitGueltig ? stempel.projectSampleStart : 0;
        // SONDE-009: dieselbe Regel fuer jedes neue Feld - ohne sein
        // Gueltigkeitsbit steht der Nullwert da, nie ein alter Wert. Der
        // Deskriptor liegt in einem RING und traegt beim Wiederverwenden noch
        // die Werte seines Vorgaengers; ohne diese Zuweisungen laege in einem
        // Block ohne Kontext die Schleifengrenze des Blocks von vor 2048
        // Runden - ununterscheidbar von einer aktuellen.
        b.continuousTimeSamples = stempel.continuousGueltig ? stempel.continuousTimeSamples : 0;
        b.cycleStartPpq = stempel.cycleGrenzenGueltig ? stempel.cycleStartPpq : 0.0;
        b.cycleEndePpq  = stempel.cycleGrenzenGueltig ? stempel.cycleEndePpq  : 0.0;
        b.tempo         = stempel.tempoGueltig ? stempel.tempo : 0.0;
        b.ppqPosition   = stempel.ppqGueltig   ? stempel.ppqPosition : 0.0;
        b.sampleRate    = stempel.sampleRateGueltig ? stempel.sampleRate : 0.0;
        b.eingangLatenzSamples = stempel.eingangLatenzGemeldet ? stempel.eingangLatenzSamples : 0;
        b.ausgangLatenzSamples = stempel.ausgangLatenzGemeldet ? stempel.ausgangLatenzSamples : 0;
        b.stromVon    = von;
        b.ringVon     = ringSchreib;
        b.sampleCount = n;
        b.startFolge  = startFolge;
        b.kanaele     = (std::uint32_t) kanaele;
        b.tapMaske    = maske;
        b.flags       = stempel.flags();
        if (lueckeOffen)
        {
            b.flags |= kFlagLueckeDavor;
            ++segment;
            lueckeOffen = false;
        }
        b.segment = segment;

        ringSchreib += n;
        angenommeneBloecke.fetch_add (1, std::memory_order_relaxed);
        if (n > groesstenBlock.load (std::memory_order_relaxed))
            groesstenBlock.store (n, std::memory_order_relaxed);

        // DER Veroeffentlichungspunkt. Alles darueber ist vorher geschrieben.
        deskSchreib.store (schreib + 1, std::memory_order_release);
        return true;
    }

    //== Consument (Worker) ===================================================

    /** Aeltester noch nicht freigegebener Block, oder `nullptr`. Der Zeiger ist
        gueltig, bis `freigeben()` gerufen wird. */
    const StampedBlock* spitze() const noexcept
    {
        const std::uint64_t lese = deskLese.load (std::memory_order_relaxed);
        if (lese == deskSchreib.load (std::memory_order_acquire))
            return nullptr;
        return &deskriptoren[(std::size_t) (lese & Layout::deskMaske)];
    }

    /** Kopiert Tap `tap` des Blocks INTERLEAVED (immer 2 Werte je Frame) nach
        `ziel`. `ziel` traegt mindestens `2 * block.sampleCount` floats.
        Liefert false, wenn der Tap in diesem Block nicht belegt war. */
    bool lies (const StampedBlock& block, int tap, float* ziel) const noexcept
    {
        if (ziel == nullptr || tap < 0 || tap >= Layout::taps)
            return false;
        if ((block.tapMaske & ((std::uint32_t) 1u << tap)) == 0)
            return false;

        const std::size_t basis = (std::size_t) tap * (std::size_t) Layout::budgetFrames * 2u;
        const std::uint64_t start = block.ringVon & Layout::rahmenMaske;
        const std::uint64_t bisEnde = (std::uint64_t) Layout::budgetFrames - start;
        const std::uint32_t ersteFrames = (std::uint32_t) (bisEnde < block.sampleCount ? bisEnde : block.sampleCount);

        std::memcpy (ziel, ring.data() + basis + (std::size_t) start * 2u,
                     (std::size_t) ersteFrames * 2u * sizeof (float));
        if (ersteFrames < block.sampleCount)
            std::memcpy (ziel + (std::size_t) ersteFrames * 2u, ring.data() + basis,
                         (std::size_t) (block.sampleCount - ersteFrames) * 2u * sizeof (float));
        return true;
    }

    /** Gibt den aeltesten Block samt seiner Ringframes frei. */
    void freigeben() noexcept
    {
        const std::uint64_t lese = deskLese.load (std::memory_order_relaxed);
        if (lese == deskSchreib.load (std::memory_order_acquire))
            return;
        const StampedBlock& b = deskriptoren[(std::size_t) (lese & Layout::deskMaske)];
        // Sample-Ring zuerst: erst danach darf der Produzent die Frames
        // ueberschreiben, und erst der Deskriptor-Fortschritt gibt den Slot frei.
        lesePos.store (b.ringVon + b.sampleCount, std::memory_order_release);
        deskLese.store (lese + 1, std::memory_order_release);
    }

    //== Telemetrie (beliebiger Thread) =======================================

    std::uint64_t dropsUeberlauf() const noexcept  { return ueberlaufDrops.load (std::memory_order_relaxed); }
    std::uint64_t dropsOversize() const noexcept   { return oversizeDrops.load (std::memory_order_relaxed); }
    std::uint64_t dropsGesamt() const noexcept     { return dropsUeberlauf() + dropsOversize(); }
    /** Analyseframes, die nie in den Ring kamen. Einheit: FRAMES, nicht Bloecke -
        derselbe Wert, den der alte FIFO-Weg als `framesDropped` meldete. */
    std::uint64_t verloreneFrames() const noexcept { return verworfeneFrames.load (std::memory_order_relaxed); }
    std::uint64_t bloeckeAngenommen() const noexcept { return angenommeneBloecke.load (std::memory_order_relaxed); }
    /** High-Water-Mark der Hostblockgroesse (§53.7: "P2 misst High-Water-Marks").
        Zaehlt nur ANGENOMMENE Bloecke - ein Oversize-Block hat den Ring nie
        gesehen und steht in `dropsOversize()`. */
    std::uint32_t groessterBlock() const noexcept  { return groesstenBlock.load (std::memory_order_relaxed); }

    static constexpr int maxBlockFrames() noexcept { return Layout::maxBlockFrames; }
    static constexpr int taps() noexcept           { return Layout::taps; }

private:
    void kopiereTap (int tap, const TapQuelle& q, int kanaele, std::uint32_t n) noexcept
    {
        const std::size_t basis = (std::size_t) tap * (std::size_t) Layout::budgetFrames * 2u;
        const float* l = q.links;
        const float* r = (kanaele > 1 && q.rechts != nullptr) ? q.rechts : q.links;

        std::uint64_t pos = ringSchreib & Layout::rahmenMaske;
        std::uint32_t rest = n;
        std::uint32_t gelesen = 0;
        while (rest > 0)
        {
            const std::uint64_t bisEnde = (std::uint64_t) Layout::budgetFrames - pos;
            const std::uint32_t stueck = (std::uint32_t) (bisEnde < rest ? bisEnde : rest);
            float* ziel = ring.data() + basis + (std::size_t) pos * 2u;
            for (std::uint32_t i = 0; i < stueck; ++i)
            {
                ziel[(std::size_t) i * 2u]      = l[gelesen + i];
                ziel[(std::size_t) i * 2u + 1u] = r[gelesen + i];
            }
            gelesen += stueck;
            rest    -= stueck;
            pos = (pos + stueck) & Layout::rahmenMaske;
        }
    }

    std::vector<float>        ring;            // taps * budgetFrames * 2, einmal alloziert
    std::vector<StampedBlock> deskriptoren;    // deskriptorPlaetze, einmal alloziert

    // Produzentenzustand - nur der Audiothread fasst ihn an.
    std::uint64_t ringSchreib { 0 };
    std::uint64_t stromPos    { 0 };
    std::uint64_t segment     { 0 };
    std::uint32_t startFolge  { 0 };
    bool          lueckeOffen { false };

    // Nachrichtenthread -> Produzent. Der Produzent ist der einzige, der daraus
    // eine Zustandsaenderung macht.
    std::atomic<bool>          neustartWunsch     { false };
    std::atomic<std::uint32_t> aktuelleStartFolge { 0 };

    // Die einzige Ordnungskante zwischen den Threads.
    std::atomic<std::uint64_t> deskSchreib { 0 };
    std::atomic<std::uint64_t> deskLese    { 0 };
    std::atomic<std::uint64_t> lesePos     { 0 };

    std::atomic<std::uint64_t> ueberlaufDrops     { 0 };
    std::atomic<std::uint64_t> oversizeDrops      { 0 };
    std::atomic<std::uint64_t> verworfeneFrames   { 0 };
    std::atomic<std::uint64_t> angenommeneBloecke { 0 };
    std::atomic<std::uint32_t> groesstenBlock     { 0 };
};

// Der Audiothread darf hier nicht warten: waeren diese Atomics nicht lock-free,
// versteckte sich ein Mutex im Blockpfad (dieselbe Pruefung wie in der
// Hostbruecke).
static_assert (std::atomic<std::uint64_t>::is_always_lock_free,
               "StampedAudioQueue braucht lock-freie 64-Bit-Atomics (Audiothread).");

//==============================================================================
/** Ein-Block-Quarantaene (§53.7, §32.3).

    "Worker haelt den juengsten vollstaendigen Block in Ein-Block-Quarantaene.
    Erst der zeitlich konsistente Folgeblock versiegelt ihn. Drop, Seek oder
    Loop-Wrap verwerfen alle noch offenen Fenster an der Grenze" (§53.7) -
    "Stoppt der Callback vorher, bleiben sie unvollstaendig" (§32.3).

    GELESEN ALS: ein Block wird an die Analyse gereicht, wenn und NUR wenn sein
    Nachfolger beweist, dass er ihn fortsetzt. Beweist er es nicht, faellt der
    gehaltene Block - er ist dann der letzte vor einer Grenze, deren Lage erst
    nachtraeglich sichtbar wurde, und genau diese Bloecke sind es, die eine
    bereits veroeffentlichte Evidenz kontaminieren koennten. Der Preis ist EIN
    Block je Diskontinuitaet (bei 512 Frames = 10,7 ms), gezaehlt und sichtbar;
    der Gegenwert ist, dass kein Fenster ueber eine Epochengrenze reicht.

    WAS "zeitlich konsistent" HEISST - und was hier bewusst NICHT geprueft wird:
      * Der LOKALE Strom muss lueckenlos anschliessen. Das gilt immer, auch ohne
        Hostkontext (§53.6: ohne `project_time_samples` "nur lokaler monotoner
        Verlauf").
      * Die HOSTZEIT muss anschliessen - aber nur, wenn beide Bloecke sie haben
        UND der Transport in beiden nachweislich LAEUFT. Bei gestopptem
        Transport steht `projectTimeSamples` still, waehrend Audio weiterlaeuft
        (Vorhoeren am Instrument): waere die stehende Projektzeit ein Bruch,
        stuerbe jede Analyse ausserhalb der Wiedergabe. Sie sagt dort schlicht
        nichts.
      * ... aber eine Zeit, die bei bekannt gestopptem Transport SPRINGT, sagt
        sehr wohl etwas: das ist ein Seek, und §32.3 fuehrt "einen Sprung" als
        Epochengrenze. STEHEN und SPRINGEN sind an den vorhandenen Bits
        unterscheidbar, und seit T2-4 (23.08.2026) werden sie es auch.
      * Eine Transportkante (`spielt` kippt) ist ein Bruch - §32.3 fuehrt sie
        ausdruecklich als Epochengrenze.
      * Epochen-, Segment- und Fensterbuchhaltung der FEATURES gehoert nicht
        hierher, sondern zu `SONDE-009` ("Drop/Seek/Loop trennt jedes offene
        Fenster", Entwurf §65). Diese Klasse liefert nur die Grenze. */
template <typename Layout>
class Blockquarantaene
{
public:
    struct Freigabe
    {
        const StampedBlock* block { nullptr };
        /** Interleaved L/R des Tap 0. Gueltig BIS ZUM NAECHSTEN `schiebe()`
            oder `zuruecksetzen()` - die Puffer laufen als Ping-Pong. */
        const float*        audio { nullptr };

        explicit operator bool() const noexcept { return block != nullptr; }
    };

    /** Legt beide Haltepuffer an (Workerthread). Idempotent. */
    void vorbereiten()
    {
        const std::size_t noetig = (std::size_t) Layout::maxBlockFrames * 2u;
        for (auto& p : puffer)
            if (p.size() != noetig)
                p.assign (noetig, 0.0f);
        zuruecksetzen();
    }

    /** Verwirft, was gehalten wird ("stoppt der Callback vorher, bleiben sie
        unvollstaendig"). Zaehlt den Verlust mit. */
    void zuruecksetzen() noexcept
    {
        if (belegt)
            hoch (verworfen);
        belegt = false;
        aktiv = 0;
    }

    /** Nimmt den naechsten gelesenen Block auf und liefert den versiegelten
        Vorgaenger - oder nichts. */
    Freigabe schiebe (const StampedAudioQueue<Layout>& queue, const StampedBlock& neu) noexcept
    {
        const int ziel = 1 - aktiv;
        if (! queue.lies (neu, 0, puffer[(std::size_t) ziel].data()))
        {
            // Tap 0 war nicht belegt: nichts zu analysieren. Der gehaltene Block
            // verliert damit seinen Nachfolgebeweis.
            zuruecksetzen();
            return {};
        }

        Freigabe heraus;
        if (belegt)
        {
            if (schliesstAn (gehalten, neu))
            {
                // Der Deskriptor wandert in ein EIGENES Feld. `gehalten` wird
                // zwei Zeilen weiter ueberschrieben - ein Zeiger darauf haette
                // dem Aufrufer den NACHFOLGER untergeschoben, und zwar
                // ununterscheidbar von einem gueltigen Wert.
                versiegelterBlock = gehalten;
                heraus.block = &versiegelterBlock;
                heraus.audio = puffer[(std::size_t) aktiv].data();
                hoch (versiegelt);
            }
            else
            {
                hoch (verworfen);
                hoch (brueche);
            }
        }

        gehalten = neu;
        belegt = true;
        aktiv = ziel;
        return heraus;
    }

    /** Beweist `neu`, dass er `gehalten` fortsetzt? */
    static bool schliesstAn (const StampedBlock& gehalten, const StampedBlock& neu) noexcept
    {
        if ((neu.flags & kFlagLueckeDavor) != 0)          return false;
        if (neu.startFolge != gehalten.startFolge)        return false;
        if (neu.segment  != gehalten.segment)             return false;
        if (neu.kanaele  != gehalten.kanaele)             return false;
        if (neu.tapMaske != gehalten.tapMaske)            return false;
        if (neu.stromVon != gehalten.stromVon + gehalten.sampleCount) return false;

        // Wechselt die Beweislage selbst (Kontext kommt/geht, Offline beginnt),
        // ist das eine Grenze - danach bedeutet dieselbe Zahl etwas anderes.
        constexpr std::uint32_t bruchMaske = kFlagKontextAnwesend | kFlagZeitGueltig
                                           | kFlagSpieltGueltig | kFlagSpielt
                                           | kFlagNichtEchtzeit;
        if (((gehalten.flags ^ neu.flags) & bruchMaske) != 0) return false;

        // Die Flagmaske oben hat schon geprueft, dass beide Bloecke DIESELBE
        // Beweislage haben - hier genuegt daher ein Blick auf `neu`.
        const bool transportBekannt = (neu.flags & kFlagSpieltGueltig) != 0;
        const bool beideSpielen = transportBekannt && (neu.flags & kFlagSpielt) != 0;
        const bool beideStehen  = transportBekannt && (neu.flags & kFlagSpielt) == 0;
        const bool beideZeit    = (neu.flags & kFlagZeitGueltig) != 0;

        // Die Hostzeit muss anschliessen, sobald beide Bloecke sie haben UND der
        // Transport BEKANNT ist - laufend oder stehend. Ist er unbekannt, bleibt
        // die Zeit unbewertet: dort koennte eine wandernde Zeit ganz normale
        // Wiedergabe sein, und dieselbe Regel waere ein Bruch je Block.
        //
        // T2-4 (23.08.2026): bis dahin lief diese Pruefung nur mit `spielt`. Ein
        // Seek bei gestopptem Transport war damit keine Grenze (gemessen: Sprung
        // um 10 s => 0 Brueche) - obwohl §32.3 "einen Sprung" ausdruecklich als
        // Epochengrenze fuehrt und der Fall alltaeglich ist: der User zieht den
        // Playhead, waehrend er am Instrument vorhoert.
        //
        // 🔑 Das ist die KEHRSEITE der Teilpuffer-Regel unten, nicht ihr
        // Widerspruch. Eine Zeit, die STEHT, sagt nichts - sie faellt durch das
        // `!=` und wird gar nicht geprueft. Eine Zeit, die sich BEWEGT, sagt
        // etwas, und dann muss sie es lueckenlos tun: bei Stopp genauso wie bei
        // Wiedergabe. Alles andere ist ein Sprung.
        //
        // ⚠️ WARUM NICHT "bei Stopp ist JEDE Bewegung ein Bruch", obwohl das
        // naeher am Wortlaut laege - das war die erste Fassung dieser Zeilen und
        // ist im Selbstaudit derselben Session gefallen: ein Host, der
        // "gestoppt" meldet und die Projektzeit trotzdem lueckenlos
        // fortschreibt, verloere damit JEDEN Block, die Analyse stuerbe dort
        // ganz. Eine lueckenlose Fortsetzung ist eine Fortsetzung, egal was das
        // Transportbit dazu sagt; der Befund war ein SPRUNG, kein Fortschritt.
        if (beideZeit && (beideSpielen || beideStehen)
            && neu.projectSampleStart != gehalten.projectSampleStart)   // s. u.
        {
            // Numerischer Rand: ein Host, der `projectTimeSamples` nahe
            // INT64_MAX meldet, brauchte hier einen Ueberlauf. Der ist keine
            // Fortsetzung, sondern eine Grenze.
            if (gehalten.projectSampleStart
                    > std::numeric_limits<std::int64_t>::max() - (std::int64_t) gehalten.sampleCount)
                return false;
            if (neu.projectSampleStart
                    != gehalten.projectSampleStart + (std::int64_t) gehalten.sampleCount)
                return false;
        }
        return true;
        // ⚠️ WARUM `neu.start == gehalten.start` KEIN Bruch ist, obwohl es nach
        // stehender Zeit aussieht: FL zerteilt Puffer bis auf 1 Sample, um
        // Automationspunkte zu setzen (Termin B, Capabilityreport S4:
        // "Automation nie >1 Punkt je Block bei Offset 0 - FL zerteilt Puffer
        // bis 1 Sample"). Die Teilstuecke tragen dieselbe Projektzeit und sind
        // trotzdem LUECKENLOSE Fortsetzung. Eine stehende Zeit sagt nichts -
        // sie ist keine Aussage ueber einen Sprung. Ein echter Seek oder
        // Loop-Wrap BEWEGT die Zeit und faellt weiter oben durch. Ohne diese
        // Zeile verloere die Analyse in automationsreichen Projekten an jeder
        // Teilungsgrenze einen Block, und zwar still.
    }

    bool          haeltBlock() const noexcept    { return belegt; }
    // Die drei Zaehler sind atomar, weil Editor und Tests sie lesen, waehrend
    // der Worker sie fortschreibt. Relaxed genuegt: es sind Zaehlstaende, keine
    // Ordnungskanten.
    std::uint64_t versiegelteBloecke() const noexcept { return versiegelt.load (std::memory_order_relaxed); }
    /** Bloecke, die die Quarantaene nie verlassen haben (Grenze oder Stopp). */
    std::uint64_t verworfeneBloecke() const noexcept  { return verworfen.load (std::memory_order_relaxed); }
    /** Wie oft ein Nachfolger die Fortsetzung NICHT bewiesen hat. */
    std::uint64_t kontinuitaetsbrueche() const noexcept { return brueche.load (std::memory_order_relaxed); }

private:
    static void hoch (std::atomic<std::uint64_t>& z) noexcept
    {
        z.store (z.load (std::memory_order_relaxed) + 1, std::memory_order_relaxed);
    }

    std::vector<float> puffer[2];
    StampedBlock  gehalten {};
    StampedBlock  versiegelterBlock {};
    bool          belegt   { false };
    int           aktiv    { 0 };
    std::atomic<std::uint64_t> versiegelt { 0 };
    std::atomic<std::uint64_t> verworfen  { 0 };
    std::atomic<std::uint64_t> brueche    { 0 };
};

} // namespace nakama::echtzeit
