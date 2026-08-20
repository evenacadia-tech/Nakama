// SONDE-003 — Nakama-Hostbruecke (Entwurf §31.2, §44.3, §53.7).
//
// WOZU. Der gepinnte JUCE-8.0.9-VST3-Wrapper bildet drei Hostinformationen NICHT
// auf die oeffentliche AudioProcessor-API ab (an der Quelle nachgemessen, nicht
// abgeschrieben — juce_audio_plugin_client_VST3.cpp, JUCE 8.0.9):
//
//   1. Parameterpunkte: Zeile ~3636 reicht normalen Pluginparametern nur
//      getPointFromQueue (queue, numPoints - 1) weiter — den LETZTEN Punkt des
//      Blocks, ohne Sample-Offset. Samplegenaue Automation ist damit unmoeglich.
//   2. Context-Anwesenheit: process() macht bei data.processContext == nullptr
//      ein zerostruct (processContext); VST3PlayHead::getPosition() liefert
//      danach TROTZDEM ein gefuelltes PositionInfo. "Context vorhanden" ist im
//      oeffentlichen Pfad also nicht von "Context fehlt" unterscheidbar — und
//      getTimeInSeconds() rechnet dort 0.0/0.0 = NaN.
//   3. Buslatenz: Vst::IAudioPresentationLatency ist im Wrapper GAR NICHT
//      implementiert (nur IProcessContextRequirements). Presentation-Latency je
//      Bus erreicht das Plugin heute nie.
//
// DIESER HEADER ist die Gegenseite des Wrapper-Patches
// (third_party/patches/juce-8.0.9-nakama-vst3-bridge.patch). Er ist bewusst
// JUCE- UND VST3-SDK-frei: dadurch liegt die riskante Abbildungslogik in
// UNSEREM Repo und ist headless pruefbar (EqCopHostContextTest), waehrend der
// Patch am Fremdcode nur duenner Klebstoff bleibt und bei jedem JUCE-Update
// billig neu zu beweisen ist.
//
// GRUNDGESETZ. Nichts hier veraendert Audio. Der ganze Schreibpfad laeuft auf
// dem Audiothread und ist deshalb: keine Allokation, kein Lock, kein I/O, kein
// Logging, keine Ausnahmen. Alle Puffer sind vorallokiert und fest dimensioniert.
//
// EHRLICHKEIT STATT SANIERUNG. Fehlt eine Information, wird sie NICHT erfunden:
//   * timeInSeconds wird grundsaetzlich NICHT uebernommen (waere im genullten
//     Context NaN — verwerfen, nicht sanitisieren; Pruefbericht 1.2).
//   * Ein Latenzwert 0 heisst laut VST3-SDK "keine ODER unbekannt" — deshalb
//     traegt jeder Eintrag ein eigenes Bit "hat der Host das je gemeldet".
//   * Ueberlauf verwirft keine unbekannte Teilmenge: der GANZE Block verliert
//     sampleAccurateAutomation, meldet einen Zaehler, und der dokumentierte
//     letzte Blockwert bleibt gueltig (Entwurf §53.7). Dieser Rueckfallwert
//     wird DESHALB getrennt vom Ereignisring gefuehrt — ein Ring, der beim
//     Ueberlauf hinten abschneidet, wuerfe sonst ausgerechnet den Wert weg,
//     den der Vertrag ueberleben laesst (T2-Befund 21.08.2026: gemessen kam
//     0.511 statt 0.777 heraus, und zwar ununterscheidbar von einem gueltigen
//     Wert).
#pragma once

#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace eqcop::hostbruecke
{

// Feste Obergrenzen. Beide sind Vertrag, nicht Geschmack: sie bestimmen die
// Groesse der vorallokierten Puffer und damit die Allokationsfreiheit.
inline constexpr std::uint32_t kMaxParameterEreignisse = 512;
inline constexpr std::uint32_t kMaxLetztwerte          = 128;  // Parameter je Block
inline constexpr std::int32_t  kMaxBusse               = 16;   // je Richtung

// P0 kennt noch keine stabile Parameter-Identitaet; hier steht die rohe
// VST3-ParamID. Die Abbildung auf StableParameterId gehoert in SONDE-006/P1.
using StableParameterId = std::uint32_t;

//==============================================================================
/** Wert mit eigenem Gueltigkeitsbit. Ungueltig heisst: der Host hat es nicht
    gesagt — NICHT "es ist 0". */
template <typename T>
struct ValidValue
{
    T    wert     {};
    bool gueltig  { false };

    void setze (T neu) noexcept        { wert = neu; gueltig = true; }
    void loesche() noexcept            { wert = T{}; gueltig = false; }
    T    oder (T ersatz) const noexcept { return gueltig ? wert : ersatz; }
};

/** Cycle-Grenzen sind in VST3 PPQ-Werte (Viertelnoten), keine Samples.
    Samplegrenzen waeren ein ABGELEITETES Feld und brauchen eine durch
    FL-Goldens validierte Abbildung (Entwurf §32) — hier bewusst nicht. */
struct ValidCyclePpq
{
    double startPpq { 0.0 };
    double endePpq  { 0.0 };
    bool   gueltig  { false };
    bool   aktiv    { false };   // kCycleActive: Schleife laeuft gerade
};

/** Presentation-Latency je Bus und Richtung.

    "gemeldet" trennt die beiden Bedeutungen der Null: der Host hat 0 gesagt
    (gemeldet=true, samples=0) gegenueber der Host hat nie etwas gesagt
    (gemeldet=false). Ohne diese Trennung waere jede Latenzaussage geraten. */
struct FixedBusLatencyTable
{
    struct Eintrag
    {
        std::uint32_t samples  { 0 };
        bool          gemeldet { false };
    };

    Eintrag eingang[kMaxBusse] {};
    Eintrag ausgang[kMaxBusse] {};

    const Eintrag& hole (bool istEingang, std::int32_t bus) const noexcept
    {
        static constexpr Eintrag leer {};
        if (bus < 0 || bus >= kMaxBusse) return leer;
        return istEingang ? eingang[bus] : ausgang[bus];
    }
};

//==============================================================================
/** Rohe Hostinformation eines Blocks. Reihenfolge und Felder folgen Entwurf
    §53.7; timeInSeconds fehlt absichtlich (siehe Kopf). */
struct HostBlockContext
{
    /** DAS entscheidende Bit: lag ProcessData.processContext wirklich an?
        Im oeffentlichen JUCE-Pfad ist das nicht erfragbar. */
    bool processContextPresent { false };

    ValidValue<std::int64_t> projectTimeSamples;      // im vorhandenen Context immer gueltig
    ValidValue<std::int64_t> continuousTimeSamples;   // nur bei kContTimeValid
    ValidValue<bool>         playing;
    ValidValue<bool>         recording;
    ValidValue<double>       tempo;                   // nur bei kTempoValid
    ValidValue<double>       ppqPosition;             // nur bei kProjectTimeMusicValid
    ValidValue<double>       barPositionPpq;          // nur bei kBarPositionValid
    ValidValue<double>       sampleRate;              // Kontext-Samplerate, roh
    ValidCyclePpq            cycle;
    FixedBusLatencyTable     presentationLatency;

    void leeren() noexcept
    {
        processContextPresent = false;
        projectTimeSamples.loesche();
        continuousTimeSamples.loesche();
        playing.loesche();
        recording.loesche();
        tempo.loesche();
        ppqPosition.loesche();
        barPositionPpq.loesche();
        sampleRate.loesche();
        cycle = ValidCyclePpq {};
        // presentationLatency bleibt stehen: sie ist Setup-Zustand, kein Blockwert.
    }
};

/** Der letzte Wert, den der Host in diesem Block fuer einen Parameter geschickt
    hat — genau das, was JUCEs eigener Weg an den Parameter reicht
    (`getPointFromQueue (queue, numPoints - 1)`, Entwurf §44.3). Ueberlebt den
    Ueberlauf des Ereignisrings, weil der Vertrag ihn als Rueckfallweg benennt. */
struct Letztwert
{
    StableParameterId id   { 0 };
    float             wert { 0.0f };
};

/** Ein Automationspunkt. Nach Offset sortiert, bei gleichem Offset in der
    Reihenfolge, in der der Host ihn geliefert hat (stabil). */
struct ParameterEvent
{
    StableParameterId id               { 0 };
    std::uint32_t     sampleOffset     { 0 };
    float             normalisedValue  { 0.0f };
};

/** Was die Senke pro Block bekommt. Der Zeiger lebt nur waehrend des Aufrufs. */
struct Blockbefund
{
    HostBlockContext      kontext {};
    const ParameterEvent* ereignisse { nullptr };
    std::uint32_t         anzahl     { 0 };
    std::uint32_t         blockGroesse { 0 };

    /** Rueckfallweg: letzter Wert je Parameter, unabhaengig vom Ereignisring. */
    const Letztwert*      letztwerte     { nullptr };
    std::uint32_t         letztwertAnzahl { 0 };

    /** Das Fallbackbit des Tickets. false heisst fuer den Verbraucher:
        NICHT samplegenau rampen, sondern vom vorigen zum letzten Blockwert —
        und Topologieautomation aus. */
    bool sampleAccurateAutomation { false };

    // Zaehler statt stiller Korrekturen — ALLE vier sind hier sichtbar, sonst
    // gilt die Regel nur fuer drei davon (T2-Befund 21.08.2026).
    std::uint64_t ueberlaeufe          { 0 };   // Ereignisse, die nicht mehr in den Ring passten
    std::uint64_t unplausibleOffsets   { 0 };   // Offset < 0 oder >= Blockgroesse
    std::uint64_t unplausibleWerte     { 0 };   // NaN/Inf als Parameterwert
    std::uint64_t verworfeneLetztwerte { 0 };   // mehr als kMaxLetztwerte Parameter im Block
    std::uint64_t verworfeneBusse      { 0 };   // Buslatenz-Meldung mit Index ausserhalb [0, kMaxBusse)

    /** Letzter Wert je Parameter — auch ohne Samplegenauigkeit gueltig; das ist
        der dokumentierte Rueckfallweg (Entwurf §53.7).

        Liest bewusst die Letztwert-Tabelle, NICHT den Ereignisring: der Ring
        schneidet beim Ueberlauf hinten ab und verlöre damit genau diesen Wert.
        false heisst ehrlich „nichts gesehen" — nie „0". */
    bool hatLetztenBlockwert (StableParameterId id, float& heraus) const noexcept
    {
        for (std::uint32_t i = 0; i < letztwertAnzahl; ++i)
            if (letztwerte[i].id == id) { heraus = letztwerte[i].wert; return true; }
        return false;
    }
};

//==============================================================================
/** Empfaenger der Bruecke. Ein AudioProcessor, der das implementiert, bekommt
    die Rohdaten unmittelbar VOR seinem processBlock. Wer es nicht implementiert,
    merkt von der Bruecke nichts — genau das ist der Fallbackweg. */
struct Senke
{
    virtual ~Senke() = default;

    /** Audiothread. Keine Allokation, kein Lock, kein I/O, kein Logging. */
    virtual void nakamaBlockEmpfangen (const Blockbefund&) noexcept = 0;
};

//==============================================================================
/** Der Sammler. Lebt als Member im gepatchten Wrapper, eine Instanz je Plugin.

    Lebenslauf je Block (so ruft der Patch):
        beginneBlock (blockGroesse)
        kontextAus (*data.processContext)   ODER   kontextFehlt()
        punkt (...) je Automationspunkt, in Hostreihenfolge
        uebergib()
*/
class Bruecke
{
public:
    Bruecke() noexcept = default;

    Bruecke (const Bruecke&) = delete;
    Bruecke& operator= (const Bruecke&) = delete;

    //== Einrichtung (Nachrichtenthread) ======================================

    /** EINMAL beim Erzeugen des Wrappers. Der dynamic_cast gehoert dorthin,
        nie in den Audiothread. */
    void verbinde (Senke* neueSenke) noexcept { senke = neueSenke; }
    bool istVerbunden() const noexcept      { return senke != nullptr; }

    //== Buslatenz (BELIEBIGER Thread) ========================================

    /** IAudioPresentationLatency::setAudioPresentationLatencySamples.
        Der Host darf das ausserhalb des Audiothreads rufen, deshalb atomar.
        gemeldet-Bit und Wert stecken in EINEM 64-Bit-Wort — so kann kein
        Leser ein halbes Update sehen. */
    void setzeBuslatenz (bool istEingang, std::int32_t bus, std::uint32_t samples) noexcept
    {
        if (bus < 0 || bus >= kMaxBusse) { verworfeneBusse.fetch_add (1, std::memory_order_relaxed); return; }
        const std::uint64_t wort = (std::uint64_t) 1u << 32 | (std::uint64_t) samples;
        (istEingang ? latenzEingang : latenzAusgang)[bus].store (wort, std::memory_order_relaxed);
    }

    std::uint64_t verworfeneBusmeldungen() const noexcept { return verworfeneBusse.load (std::memory_order_relaxed); }

    //== Blockpfad (Audiothread) ==============================================

    void beginneBlock (std::uint32_t neueBlockGroesse) noexcept
    {
        anzahl = 0;
        letztwertAnzahl = 0;
        ueberlaufImBlock = 0;
        letztwertUeberlauf = 0;
        offsetImBlock = 0;
        wertImBlock = 0;
        blockGroesse = neueBlockGroesse;
        kontext.leeren();
    }

    /** Context fehlt — ausdruecklich, nicht durch Schweigen. */
    void kontextFehlt() noexcept { kontext.processContextPresent = false; }

    /** Uebernimmt einen VST3-ProcessContext, ohne den SDK-Header zu kennen.

        Die Flag-Werte kommen aus dem uebergebenen Typ selbst
        (VstKontext::kTempoValid …) — dadurch kann diese Abbildung die
        SDK-Konstanten nicht falsch abschreiben, und der Test kann sie mit einem
        nachgebauten Kontext fahren.

        SDK-FALLE: das Feld heisst continousTimeSamples (ohne "u" nach contin) —
        ein Tippfehler im offiziellen Steinberg-Header, kein eigener. */
    template <typename VstKontext>
    void kontextAus (const VstKontext& c) noexcept
    {
        const auto zustand = c.state;
        const auto gesetzt = [zustand] (std::uint32_t bit) { return (zustand & bit) != 0; };

        kontext.processContextPresent = true;

        // Immer gueltig, SOBALD der Context existiert (VST3-Doku).
        kontext.projectTimeSamples.setze ((std::int64_t) c.projectTimeSamples);
        kontext.sampleRate.setze ((double) c.sampleRate);
        kontext.playing.setze (gesetzt ((std::uint32_t) VstKontext::kPlaying));
        kontext.recording.setze (gesetzt ((std::uint32_t) VstKontext::kRecording));

        if (gesetzt ((std::uint32_t) VstKontext::kContTimeValid))
            kontext.continuousTimeSamples.setze ((std::int64_t) c.continousTimeSamples);

        if (gesetzt ((std::uint32_t) VstKontext::kTempoValid))
            kontext.tempo.setze ((double) c.tempo);

        if (gesetzt ((std::uint32_t) VstKontext::kProjectTimeMusicValid))
            kontext.ppqPosition.setze ((double) c.projectTimeMusic);

        if (gesetzt ((std::uint32_t) VstKontext::kBarPositionValid))
            kontext.barPositionPpq.setze ((double) c.barPositionMusic);

        kontext.cycle.aktiv = gesetzt ((std::uint32_t) VstKontext::kCycleActive);
        if (gesetzt ((std::uint32_t) VstKontext::kCycleValid))
        {
            kontext.cycle.startPpq = (double) c.cycleStartMusic;
            kontext.cycle.endePpq  = (double) c.cycleEndMusic;
            kontext.cycle.gueltig  = true;
        }

        // timeInSeconds wird bewusst NICHT gebildet (siehe Kopf).
    }

    /** Ein Automationspunkt, in Hostreihenfolge. Wird sofort stabil an seinen
        Platz einsortiert — dadurch ist der Puffer jederzeit gueltig und es gibt
        keinen zweiten Sortierdurchgang. */
    void punkt (StableParameterId id, std::int32_t sampleOffset, double wert) noexcept
    {
        const float roh = (float) wert;

        // ZUERST der Rueckfallweg, VOR jedem Ueberlauf-Ausstieg: der letzte
        // Wert je Parameter ist genau das, was JUCE an den Parameter reicht,
        // und der Vertrag laesst ihn den Ueberlauf ueberleben (Entwurf §53.7).
        merkeLetztwert (id, roh);

        if (anzahl >= kMaxParameterEreignisse) { ++ueberlaufImBlock; return; }

        // Numerische Raender, alle drei gemeldet statt still geglaettet:
        // negativer Offset, Offset jenseits des Blocks, und der Flush-Block
        // mit blockGroesse == 0 (VST3 erlaubt numSamples 0).
        std::uint32_t offset = 0;
        if (sampleOffset < 0)
        {
            ++offsetImBlock;
        }
        else if (blockGroesse == 0)
        {
            if (sampleOffset != 0) ++offsetImBlock;
        }
        else if ((std::uint32_t) sampleOffset >= blockGroesse)
        {
            ++offsetImBlock;
            offset = blockGroesse - 1;
        }
        else
        {
            offset = (std::uint32_t) sampleOffset;
        }

        const float f = roh;
        if (! std::isfinite (f))
            ++wertImBlock;

        // Stabile Einfuegung: hinter ALLE Ereignisse mit kleinerem ODER gleichem
        // Offset. Gleicher Offset behaelt damit die Hostreihenfolge.
        std::uint32_t stelle = anzahl;
        while (stelle > 0 && puffer[stelle - 1].sampleOffset > offset)
        {
            puffer[stelle] = puffer[stelle - 1];
            --stelle;
        }

        puffer[stelle].id              = id;
        puffer[stelle].sampleOffset    = offset;
        puffer[stelle].normalisedValue = f;
        ++anzahl;
    }

    /** Schliesst den Block ab und reicht ihn an die Senke. Ohne Senke passiert
        nichts — kein Aufwand, kein Nebeneffekt.

        VERTRAG, und zwar der ehrliche: die Senke sieht, was der Host in DIESEM
        process()-Aufruf geliefert hat — nicht zwingend einen Block, den der
        Processor auch verarbeitet. Zwei Faelle im JUCE-Wrapper koennen danach
        noch abbrechen, bevor processBlock laeuft: der Parameter-Flush
        (numSamples == 0, am blockGroesse == 0 erkennbar) und der
        Wavelab-Riegel. Wer 1:1-Paarung mit processBlock annimmt, liegt falsch;
        wer Parameterpunkte sammelt, will genau diese Faelle sehen. */
    void uebergib() noexcept
    {
        if (senke == nullptr) return;

        // Latenztabelle erst hier einsammeln: EIN Lesevorgang je Block statt
        // eines Atomzugriffs je Meldung.
        for (std::int32_t b = 0; b < kMaxBusse; ++b)
        {
            entpacke (latenzEingang[b].load (std::memory_order_relaxed), kontext.presentationLatency.eingang[b]);
            entpacke (latenzAusgang[b].load (std::memory_order_relaxed), kontext.presentationLatency.ausgang[b]);
        }

        Blockbefund befund;
        befund.kontext                  = kontext;
        befund.ereignisse               = puffer;
        befund.anzahl                   = anzahl;
        befund.blockGroesse             = blockGroesse;
        befund.letztwerte               = letztwerte;
        befund.letztwertAnzahl          = letztwertAnzahl;
        befund.ueberlaeufe              = ueberlaufImBlock;
        befund.unplausibleOffsets       = offsetImBlock;
        befund.unplausibleWerte         = wertImBlock;
        befund.verworfeneLetztwerte     = letztwertUeberlauf;
        befund.verworfeneBusse          = verworfeneBusse.load (std::memory_order_relaxed);
        befund.sampleAccurateAutomation = (ueberlaufImBlock == 0)
                                       && (offsetImBlock == 0)
                                       && (wertImBlock == 0)
                                       && (letztwertUeberlauf == 0);

        senke->nakamaBlockEmpfangen (befund);
    }

    //== Nur fuer Tests / Diagnose ============================================

    std::uint32_t ereignisAnzahl() const noexcept              { return anzahl; }
    std::uint32_t letztwerteAnzahl() const noexcept            { return letztwertAnzahl; }
    const ParameterEvent& ereignis (std::uint32_t i) const noexcept { return puffer[i]; }
    const HostBlockContext& letzterKontext() const noexcept    { return kontext; }

private:
    /** Letzten Wert je Parameter fortschreiben. Lineare Suche ueber wenige
        Eintraege (ein Block hat in der Praxis eine Handvoll automatisierter
        Parameter); kein Speicher, keine Sortierung. */
    void merkeLetztwert (StableParameterId id, float wert) noexcept
    {
        for (std::uint32_t i = 0; i < letztwertAnzahl; ++i)
            if (letztwerte[i].id == id) { letztwerte[i].wert = wert; return; }

        if (letztwertAnzahl >= kMaxLetztwerte) { ++letztwertUeberlauf; return; }

        letztwerte[letztwertAnzahl].id   = id;
        letztwerte[letztwertAnzahl].wert = wert;
        ++letztwertAnzahl;
    }

    static void entpacke (std::uint64_t wort, FixedBusLatencyTable::Eintrag& ziel) noexcept
    {
        ziel.gemeldet = (wort >> 32) != 0;
        ziel.samples  = (std::uint32_t) (wort & 0xffffffffull);
    }

    Senke* senke { nullptr };

    ParameterEvent   puffer[kMaxParameterEreignisse] {};
    Letztwert        letztwerte[kMaxLetztwerte] {};
    std::uint32_t    letztwertAnzahl { 0 };
    std::uint32_t    anzahl       { 0 };
    std::uint32_t    blockGroesse { 0 };
    HostBlockContext kontext      {};

    std::uint64_t ueberlaufImBlock   { 0 };
    std::uint64_t letztwertUeberlauf { 0 };
    std::uint64_t offsetImBlock      { 0 };
    std::uint64_t wertImBlock        { 0 };

    std::atomic<std::uint64_t> latenzEingang[kMaxBusse] {};
    std::atomic<std::uint64_t> latenzAusgang[kMaxBusse] {};
    std::atomic<std::uint64_t> verworfeneBusse { 0 };
};

// Der Audiothread darf hier nicht warten: waeren diese Atomics nicht
// lock-free, versteckte sich ein Mutex im Blockpfad.
static_assert (std::atomic<std::uint64_t>::is_always_lock_free,
               "Nakama-Hostbruecke braucht lock-freie 64-Bit-Atomics (Audiothread).");

} // namespace eqcop::hostbruecke
