#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "EqCopilotIds.h"
#include "Diagnose.h"
#include "WorkerCadence.h"
#include <chrono>
#include <cmath>
#include <limits>

namespace eqcop
{

namespace
{
bool projektEnde (juce::int64 start, int samples, juce::int64& aus) noexcept
{
    if (samples < 0 || start > std::numeric_limits<juce::int64>::max() - (juce::int64) samples)
        return false;
    aus = start + (juce::int64) samples;
    return true;
}

bool projektAbstandGroesserAls64 (juce::int64 a, juce::int64 b) noexcept
{
    // Vorzeichenbit kippen bildet int64 streng monoton auf uint64 ab. Die
    // anschliessende Differenz ist auch zwischen INT64_MIN/MAX definiert.
    constexpr std::uint64_t bias = std::uint64_t { 1 } << 63u;
    const auto ua = static_cast<std::uint64_t> (a) ^ bias;
    const auto ub = static_cast<std::uint64_t> (b) ^ bias;
    return (ua >= ub ? ua - ub : ub - ua) > 64u;
}
} // namespace

EqCopilotProcessor::EqCopilotProcessor()
    : juce::AudioProcessor (BusesProperties()
          .withInput ("Eingang", juce::AudioChannelSet::stereo(), true)
          .withOutput ("Ausgang", juce::AudioChannelSet::stereo(), true)),
      pipe ([this] {
                HelloInfo h;
                {
                    std::lock_guard<std::mutex> l (bindungMutex);
                    h.sensorId = zustand.common.instanceId;
                    h.role     = nakama::state::v2Rolle (zustand.common);
                    h.label    = zustand.common.label;
                    h.pairId   = zustand.common.pairId;
                }
                h.instanceNonce = instanceNonce;
                h.samplerate = samplerateAtomic.load();
                h.blockSize  = blockSizeAtomic.load();
                h.channels   = kanaeleAtomic.load();
                return h;
            },
            [this] { return statsSnapshot(); },
            [this] { return messKompakt(); })
{
    // Frische Instanz (nie restauriert): legacy + insert, v2-Rolle "sensor".
    // Der Lebenslauf-Automat startet auf `unclassified` (§53.5) und bekommt
    // hier KEIN Ereignis - genau das ist der Grund, warum "ein Scannerlauf
    // klassifiziert nicht" keine Sonderbehandlung braucht: ein Scanner
    // instanziiert, fragt Busse und Parameter ab und zerstoert wieder. Er
    // ruft nie `setStateInformation` und oeffnet nie einen Editor.
    zustand = nakama::state::frisch (juce::Uuid().toString());
    instanceNonce = juce::Uuid().toString();
    // SONDE-008: der gesamte Backing-Store beider Ringe entsteht HIER - vor dem
    // Start des Workers und lange vor dem ersten Audioblock. `prepareToPlay`
    // fasst danach keinen Speicher mehr an, es meldet nur einen Neuanlauf.
    queue.vorbereiten();

    workerLaeuft.store (true);
    worker = std::thread ([this] { workerLauf(); });
    pipe.start();
}

EqCopilotProcessor::~EqCopilotProcessor()
{
    pipe.stop();
    workerLaeuft.store (false);
    {
        std::lock_guard<std::mutex> l (workerWarteMutex);
        workerWarte.notify_all();
    }
    if (worker.joinable())
        worker.join();
}

void EqCopilotProcessor::prepareToPlay (double samplerate, int maxBlock)
{
    const double sichereSamplerate = std::isfinite (samplerate)
                                  && samplerate > 0.0 && samplerate <= 768000.0
        ? samplerate : 0.0;
    // Jeder Prepare-Aufruf ist eine Queue-Generation. Der Audiothread setzt
    // sein Projektfenster exakt dann zurueck, wenn `veroeffentliche()` diese
    // Generation wirklich uebernimmt - auch bei unveraenderter Samplerate.
    {
        auto l = externerAnalyseSteuerZug();
        samplerateAtomic.store (sichereSamplerate);
        // Samplerate und Generation werden unter EINER Steuerkante sichtbar.
        // Der Worker kann daher nie alte Bloecke mit der neuen Binzuordnung
        // auswerten.
        queue.neustartAnfordern();
    }
    blockSizeAtomic.store (maxBlock);
    kanaeleAtomic.store (getTotalNumInputChannels());
    // SONDE-008: KEIN Reset von hier aus. Bis 23.08. rief diese Zeile
    // `fifo.reset()` — der Nachrichtenthread verstellte damit beide Enden eines
    // SPSC-Rings mitten in einen laufenden Leser hinein. Stattdessen ein
    // Wunsch, den der Audiothread als Einziger einlöst; der Worker erkennt die
    // Reste des alten Anlaufs an ihrer kleineren `startFolge`.
    // Hör-Markierung: Puffer/Zustände neu, Echtzeit-Beweis verfällt — nach
    // jedem prepareToPlay (auch Render-Vorlauf) gilt wieder „neutral, bis
    // Echtzeit bewiesen" (Konzept v2 §4).
    markierung.setzeSamplerate (sichereSamplerate);
    markierung.vorbereiten (maxBlock);
    echtzeitOk.store (false);
    lzBestanden = 0;
    lzLetzterNs = 0;
    lzBucketStartNs = 0;
    lzBucketSamples = 0;
}

bool EqCopilotProcessor::isBusesLayoutSupported (const BusesLayout& layout) const
{
    // V1: Mono und Stereo, Eingang == Ausgang. Anderes wird nicht still
    // heruntergemischt (Plan §9.4) — der Host bekommt ein klares Nein.
    const auto ein = layout.getMainInputChannelSet();
    const auto aus = layout.getMainOutputChannelSet();
    if (ein != aus)
        return false;
    return ein == juce::AudioChannelSet::mono() || ein == juce::AudioChannelSet::stereo();
}

void EqCopilotProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals rein;

    const int kanaele = juce::jmin (buffer.getNumChannels(), 2);
    const int n = buffer.getNumSamples();
    if (n == 0 || kanaele == 0)
        return;

    // Der Puffer bleibt bis zum Blockende unangetastet; erst die Hör-Markierung
    // am Ende darf färben — NACH RMS-Scan und Analyse-Abgriff, damit Messung
    // und Meter immer den echten Mix sehen (Konzept v2 §5). Ohne aktive,
    // erlaubte Markierung ist Passthrough weiterhin das Nichtstun.

    bool nan = false;
    for (int k = 0; k < kanaele; ++k)
    {
        const float* d = buffer.getReadPointer (k);
        float summe = 0.0f;
        for (int i = 0; i < n; ++i)
        {
            const float v = d[i];
            if (! std::isfinite (v))
                nan = true;
            summe += v * v;
        }
        const float rms = std::sqrt (summe / (float) n);
        (k == 0 ? rmsL : rmsR).store (std::isfinite (rms) ? rms : 0.0f);
    }
    if (kanaele == 1)
        rmsR.store (rmsL.load());
    if (nan)
        nanSeen.store (true);

    // ── Zeitstempel: woher die Zeit kommt, und wie sicher (SONDE-008) ──────
    // Zwei Quellen, in dieser Reihenfolge, und die Reihenfolge ist der Punkt:
    //  1. die HOSTBRÜCKE (SONDE-003). Nur sie kann „Context fehlt" überhaupt
    //     ausdrücken — JUCEs VST3-Playhead liefert nie `nullopt`, weil der
    //     Wrapper seinen internen Context nullt und daraus ein gefülltes
    //     PositionInfo baut (NakamaHostBridge.h, Kopf; NAK-24).
    //  2. der Playhead als Rückfallweg für Ziele OHNE gepatchten Wrapper
    //     (Konsolentests, andere Formate). Dort ist „PositionInfo da" ehrlich
    //     das Beste, was zu wissen ist — und ohne Playhead ist Transport
    //     ausdrücklich UNBEKANNT, nicht „gestoppt" und nicht „läuft".
    nakama::echtzeit::Stempel stempel;
    if (brueckeStand.frisch)
    {
        // Der Stand IST der Stempel (SONDE-009) — nichts wird umkopiert, also
        // kann auch nichts beim Umkopieren vergessen werden.
        stempel = brueckeStand.stempel;
        brueckeStand.frisch = false;
    }
    else if (auto* kopf = getPlayHead())
    {
        if (const auto pos = kopf->getPosition())
        {
            stempel.kontextAnwesend = true;
            stempel.spieltGueltig   = true;
            stempel.spielt          = pos->getIsPlaying();
            if (const auto zeit = pos->getTimeInSamples())
            {
                stempel.zeitGueltig        = true;
                stempel.projectSampleStart = *zeit;
            }
            // SONDE-009: was der öffentliche Playhead über den Rest sagen kann.
            // Weniger als die Brücke, und das ist ehrlich so: JUCE hat kein
            // Gültigkeitsbit für `isRecording`, weshalb hier bewusst KEINS
            // gesetzt wird — „PositionInfo da" beweist Transport, nicht
            // Aufnahmezustand. Was die Brücke unterscheiden kann, unterscheidet
            // nur die Brücke.
            if (const auto ppq = pos->getPpqPosition())
            {
                stempel.ppqGueltig  = true;
                stempel.ppqPosition = *ppq;
            }
            if (const auto bpm = pos->getBpm())
            {
                stempel.tempoGueltig = true;
                stempel.tempo        = *bpm;
            }
            stempel.cycleAktiv = pos->getIsLooping();
            if (const auto schleife = pos->getLoopPoints())
            {
                stempel.cycleGrenzenGueltig = true;
                stempel.cycleStartPpq = schleife->ppqStart;
                stempel.cycleEndePpq  = schleife->ppqEnd;
            }
        }
    }
    // `isNonRealtime()` fragt den PROZESSOR, nicht den Hostkontext — es steht
    // deshalb hinter beiden Zweigen und überschreibt keinen Brückenwert.
    stempel.nichtEchtzeit = isNonRealtime();
    // Die Samplerate ist im Playhead-Zweig nicht erfragbar; die des Prozessors
    // ist der einzige Wert, den wir hier ehrlich behaupten können, und er
    // stammt aus `prepareToPlay`.
    if (! stempel.sampleRateGueltig)
    {
        const double fs = samplerateAtomic.load();
        if (fs > 0.0)
        {
            stempel.sampleRateGueltig = true;
            stempel.sampleRate        = fs;
        }
    }

    // Nach außen sichtbarer Transportstand (Editor, Heartbeat). `hatTransport`
    // heißt ab hier „Transport ist BEKANNT" statt „irgendein PositionInfo kam" —
    // mit der Brücke ist das erstmals unterscheidbar (NAK-24).
    hatTransport.store (stempel.spieltGueltig);
    transportSpielt.store (stempel.spieltGueltig && stempel.spielt);

    // Hostzeit (M0-Prüfpunkt §9.3) — Projektzeit-Fenster der Messung (Plan
    // §5.7): nur während Play akkumulieren, der stehende Playhead ist kein
    // Fenster.
    // NAK-24 zweite Hälfte: die Projektzeit trägt ihr eigenes Gültigkeitsbit.
    // Bis 23.08. blieb `projektZeitSamples` stehen, wenn der Context wegfiel —
    // ein alter Wert sah aus wie eine aktuelle Position.
    projektZeitGueltig.store (stempel.zeitGueltig);
    if (stempel.zeitGueltig)
        projektZeitSamples.store (stempel.projectSampleStart);

    // Analyseweg: der GANZE Block in die zeitgestempelte Queue — oder gar
    // nicht. Der Out-Parameter ist die einzige Wahrheit fuer die tatsaechlich
    // im Produzenten uebernommene Generation. Ein Resetwunsch kann waehrend
    // dieses Callbacks eintreffen; ein separates, frueher gelesenes Atomic
    // koennte dann einen neuen Block noch ins alte Projektfenster schreiben.
    Queue::TapQuelle abgriff;
    abgriff.links  = buffer.getReadPointer (0);
    abgriff.rechts = kanaele > 1 ? buffer.getReadPointer (1) : nullptr;
    bool neustartUebernommen = false;
    bool lueckeVorBlock = false;
    const bool queueAngenommen = queue.veroeffentliche (
        &abgriff, 1, kanaele, n, stempel, &neustartUebernommen, &lueckeVorBlock);

    // Nur der Audiothread schreibt das Projektfenster. Generation und
    // Kontinuitaetskante kommen direkt aus demselben Queue-Zug, der diesen
    // Block gestempelt hat: kein Lock, keine Allokation und keine
    // atomuebergreifende Sichtbarkeitsannahme. Ein abgewiesener Block gehoert
    // nicht zur M1-Messung; erst der naechste angenommene Block beginnt mit der
    // dort wirklich veroeffentlichten Luecke ein neues Projektfenster.
    if (neustartUebernommen || lueckeVorBlock)
    {
        fensterAktiv.store (false);
        fensterSpruenge.store (0);
        fensterErwartetGueltig = false;
    }

    const bool analyseRateGueltig = samplerateAtomic.load() > 0.0;
    if (queueAngenommen && analyseRateGueltig
        && stempel.zeitGueltig && stempel.spieltGueltig && stempel.spielt)
    {
        const juce::int64 t = stempel.projectSampleStart;
        juce::int64 ende = 0;
        if (! projektEnde (t, n, ende))
        {
            // Ein nicht darstellbares Hostintervall ist keine echte
            // Projektposition. Vorheriges Fenster nicht damit vermischen.
            fensterAktiv.store (false);
            fensterErwartetGueltig = false;
            fensterSpruenge.fetch_add (1);
        }
        else if (! fensterAktiv.load())
        {
            // `fensterAktiv` ist zugleich das Publikationsbit fuer Leser:
            // Grenzen zuerst schreiben, sonst koennte ein Heartbeat die neue
            // Generation schon als gueltig mit den alten Grenzen beobachten.
            fensterVon.store (t);
            fensterBis.store (ende);
            fensterErwartet = ende;
            fensterErwartetGueltig = true;
            fensterAktiv.store (true);
        }
        else
        {
            // Sprung = Loop/Seek/Stop-Rücksprung. Toleranz 64 Samples für
            // Rundungen des Hosts. Auch über eine Pause hinweg gemessen:
            // Resume an anderer Stelle IST eine Lücke im Fenster.
            if (fensterErwartetGueltig && projektAbstandGroesserAls64 (t, fensterErwartet))
                fensterSpruenge.fetch_add (1);
            if (t < fensterVon.load())
                fensterVon.store (t);
            if (ende > fensterBis.load())
                fensterBis.store (ende);
            fensterErwartet = ende;
            fensterErwartetGueltig = true;
        }
    }

    // ── Hör-Markierung (Konzept v2): Erlaubnis prüfen, dann färben ─────────
    // Reihenfolge ist Vertrag: RMS + Analyse-Abgriff liegen OBEN — die Messung
    // sieht nie das gefärbte Signal (Beweis: Markierungstest T4).
    const bool spielt = stempel.spieltGueltig && stempel.spielt;
    lebenszeichen (n, spielt);
    // §53.5 Satz 1 (S9/SONDE-007b Abschnitt 3): bis zur positiven
    // Klassifikation ist der Entry AUDIO-NEUTRAL. Die Hoer-Markierung ist die
    // einzige Audio-Ausnahme des Grundgesetzes und faellt damit unter diesen
    // Satz; `legacy` bleibt zu, weil §53.5 es "immer passiv" nennt. Gelesen
    // wird die Atomic-Spiegelung, nie der Automat selbst - "Klassifikation,
    // Spawn und Pipe-I/O liegen nie im Audiocallback".
    // Der Term steht ZUERST, damit beim Lesen sofort klar ist: ohne Main
    // faerbt hier nichts, egal was die uebrigen Bedingungen sagen.
    //
    // ⚠️ TRANSPORT-TERM, geändert mit SONDE-008 (User-Entscheid 22.08., Hub
    // `U10`: „Nein, nur mit Signal"). Bis 23.08. stand hier
    // `(spielt ∨ ¬hatTransport)` — ein fail-open: wo kein Transport gemeldet
    // wurde, färbte die Markierung. Verlangt ist ein GÜLTIGES „spielt".
    // Der Term war nicht früher zu schließen, weil „Transport unbekannt" bis
    // zur Verdrahtung der Hostbrücke gar nicht ausdrückbar war (Entwurf §0.1:
    // „Der eigentliche Mangel ist, dass `hatTransport` ‚Transport unbekannt'
    // gar nicht ausdrücken kann"); genau diese Verdrahtung bringt SONDE-008
    // für den Zeitstempel mit. NAK-35/NAK-24.
    // In FL ändert sich dadurch nichts: dort lag `hatTransport` ab dem ersten
    // Block auf true, der fail-open-Zweig war ein toter Zweig (Prüfbericht 1.2).
    // Ohne Playhead und ohne Brücke — also headless — färbt jetzt nichts mehr.
    //
    // `testForciereEchtzeit` umgeht diesen Term ABSICHTLICH NICHT: der Schalter
    // umgeht, was an der Wanduhr hängt (Lebenszeichen, Editor). Transport hängt
    // an nichts dergleichen; ein Test, der ihn mit umginge, prüfte einen Pfad,
    // den das Produkt nicht hat (dieselbe Begründung wie beim §53.5-Term).
    const bool erlaubt = istMainKlassifiziert.load (std::memory_order_relaxed)
                      && (echtzeitOk.load (std::memory_order_relaxed)
                          || testEchtzeit.load (std::memory_order_relaxed))
                      && spielt
                      && ! isNonRealtime()
                      && (editorOffen.load (std::memory_order_relaxed)
                          || testEchtzeit.load (std::memory_order_relaxed));
    markierung.verarbeite (buffer, kanaele, erlaubt);
}

//==============================================================================
// SONDE-008: die Gegenseite der Hostbrücke. Läuft auf DEMSELBEN Thread wie
// `processBlock` und unmittelbar davor — der gepatchte Wrapper ruft
// `uebergib()` zwischen `kontextAus()` und dem Prozessoraufruf.
//
// ⚠️ Der Befund ist NICHT 1:1 mit `processBlock` gepaart (NakamaHostBridge.h,
// `uebergib()`): ein Parameter-Flush (`blockGroesse == 0`) und der
// Wavelab-Riegel liefern einen Befund, ohne dass danach ein Block verarbeitet
// wird. Deshalb ein `frisch`-Bit statt einer Annahme: `processBlock`
// VERBRAUCHT es, und ein Befund ohne Folgeblock wird schlicht vom nächsten
// überschrieben.
void EqCopilotProcessor::nakamaBlockEmpfangen (const eqcop::hostbruecke::Blockbefund& befund) noexcept
{
    const auto& k = befund.kontext;
    auto& s = brueckeStand.stempel;
    s = nakama::echtzeit::Stempel {};     // kein Feld erbt den Vorblock

    s.kontextAnwesend = k.processContextPresent;
    // `projectTimeSamples` ist laut VST3-Doku gültig, SOBALD ein Context
    // existiert — die Brücke setzt das Gültigkeitsbit deshalb genau dann.
    s.zeitGueltig        = k.processContextPresent && k.projectTimeSamples.gueltig;
    s.projectSampleStart = (std::int64_t) k.projectTimeSamples.oder (0);
    s.spieltGueltig      = k.processContextPresent && k.playing.gueltig;
    s.spielt             = k.playing.oder (false);

    // ── SONDE-009: der Rest des Transportstempels (§32.3) ──────────────────
    // Jedes Feld hängt an `processContextPresent` UND an seinem eigenen Bit.
    // Der Kontext ist die Voraussetzung dafür, dass die Zahl überhaupt etwas
    // bedeutet; das Einzelbit sagt, ob der Host sie in DIESEM Block gemeldet
    // hat. Beide Fragen sind verschieden, deshalb beide Konjunktionen.
    s.recordingGueltig      = k.processContextPresent && k.recording.gueltig;
    s.recording             = k.recording.oder (false);
    s.continuousGueltig     = k.processContextPresent && k.continuousTimeSamples.gueltig;
    s.continuousTimeSamples = (std::int64_t) k.continuousTimeSamples.oder (0);
    s.tempoGueltig          = k.processContextPresent && k.tempo.gueltig;
    s.tempo                 = k.tempo.oder (0.0);
    s.ppqGueltig            = k.processContextPresent && k.ppqPosition.gueltig;
    s.ppqPosition           = k.ppqPosition.oder (0.0);
    s.sampleRateGueltig     = k.processContextPresent && k.sampleRate.gueltig;
    s.sampleRate            = k.sampleRate.oder (0.0);

    // Schleife: `aktiv` und `gueltig` sind ZWEI Aussagen. VST3 kann melden
    // „die Schleife läuft" und trotzdem keine brauchbaren Grenzen liefern
    // (`kCycleValid` fehlt) — dann ist der Loop-Wrap bekannt, seine Lage aber
    // nicht. §32.3 nennt genau diesen Fall: „Liegen nur PPQ-Bounds vor oder
    // fehlen die Bounds, wird der mögliche Straddle als ungültig markiert."
    s.cycleAktiv          = k.processContextPresent && k.cycle.aktiv;
    s.cycleGrenzenGueltig = k.processContextPresent && k.cycle.gueltig;
    s.cycleStartPpq       = k.cycle.startPpq;
    s.cycleEndePpq        = k.cycle.endePpq;

    // Presentation-Latency: Bus 0 je Richtung — der Main-Bus. Die weiteren
    // Busse gehören zu Probeeqs Aux-Wegen (P6) und haben heute keinen
    // Verbraucher. `gemeldet` trennt „der Host hat 0 gesagt" von „der Host hat
    // nie etwas gesagt"; ein Latenzwert 0 kann beides heißen (§32.3).
    const auto& ein = k.presentationLatency.hole (true, 0);
    const auto& aus = k.presentationLatency.hole (false, 0);
    s.eingangLatenzGemeldet = ein.gemeldet;
    s.eingangLatenzSamples  = ein.samples;
    s.ausgangLatenzGemeldet = aus.gemeldet;
    s.ausgangLatenzSamples  = aus.samples;

    brueckeStand.frisch = true;
}

// „Neutral, bis Echtzeit bewiesen" (Konzept v2 §4): zwei Fenster mit
// Audiozeit/Echtzeit ≈ 1 schalten frei; Transportkante oder Verarbeitungs-
// lücke setzt den Beweis zurück; Freilauf (Verhältnis > 1,5) löscht ihn und
// meldet den Latch-Fall an den Editor. Ein Offline-Render besteht den Beweis
// nie — Fenster schließen dort nach Audiozeit, nicht nach Wanduhr.
void EqCopilotProcessor::lebenszeichen (int samples, bool spielt)
{
    using namespace std::chrono;
    const juce::int64 jetzt =
        duration_cast<nanoseconds> (steady_clock::now().time_since_epoch()).count();
    const double fs = samplerateAtomic.load();
    if (fs <= 0.0)
        return;

    bool zuruecksetzen = false;
    if (lzLetzterNs > 0 && jetzt - lzLetzterNs > 250'000'000LL)
        zuruecksetzen = true;                    // Verarbeitungslücke (Suspend/Bypass)
    if (spielt != lzPrevSpielt)
    {
        zuruecksetzen = true;                    // Transportkante (Start UND Stopp)
        lzPrevSpielt = spielt;
    }
    lzLetzterNs = jetzt;
    if (zuruecksetzen)
    {
        echtzeitOk.store (false);
        lzBestanden = 0;
        lzBucketStartNs = jetzt;
        lzBucketSamples = 0;
        return;
    }
    if (lzBucketStartNs == 0)
    {
        lzBucketStartNs = jetzt;
        lzBucketSamples = 0;
    }
    lzBucketSamples += samples;

    const double wandS  = (double) (jetzt - lzBucketStartNs) * 1.0e-9;
    const double audioS = (double) lzBucketSamples / fs;
    // Beweisen langsam (500 ms), kippen schnell (250 ms Audiozeit): im
    // Freilauf schließt das Fenster nach Audiozeit fast sofort.
    const bool fensterZu = wandS >= 0.5
                        || audioS >= (echtzeitOk.load() ? 0.25 : 0.5);
    if (! fensterZu)
        return;

    const double verhaeltnis = audioS / std::max (wandS, 1.0e-6);
    if (verhaeltnis >= 0.5 && verhaeltnis <= 1.2)
    {
        if (++lzBestanden >= 2)
            echtzeitOk.store (true);
    }
    else
    {
        lzBestanden = 0;
        if (verhaeltnis > 1.5)
        {
            echtzeitOk.store (false);
            if (markierung.zielGesetzt())
                freilaufKill.store (true);       // Editor löst den Latch sichtbar
        }
        // 1,2 < Verhältnis ≤ 1,5: Host-Burst — Beweis bleibt, zählt nur nicht.
    }
    lzBucketStartNs = jetzt;
    lzBucketSamples = 0;
}

void EqCopilotProcessor::workerLauf()
{
    // FP-Modi sind threadlokal: der ScopedNoDenormals im Audiocallback
    // schuetzt diesen Worker und seine rekursiven K-Filter nicht.
    juce::ScopedNoDenormals keineDenormals;
    // Leert die Analysequeue in begrenzten Zuegen; ohne Rueckstau schlaeft der
    // Worker bis zu 50 ms, bei Rueckstau arbeitet er direkt weiter. Leichte und
    // schwere Publikation bleiben trotzdem an 50-/250-ms-Wanduhrdeadlines.
    // Die Engine gehoert exklusiv diesem Thread; Reset/Samplerate kommen als
    // Atomics herein. Kein Realtime-Anspruch — Überlast verwirft ganze Bloecke.
    quarantaene.vorbereiten();     // einmalige Allokation, im Worker, vor dem ersten Zug
    juce::uint64 unverarbeitet = 0;   // Samples seit der letzten Schwer-Auswertung
    auto workerAnlauf = queue.aktuellerAnlauf();
    detail::WorkerKadenz kadenz;
    while (workerLaeuft.load())
    {
        // Explizite Uebergabe statt Fairness-Hoffnung: sobald Prepare, Reset
        // oder ein Frame-Leser wartet, konkurriert der Worker nicht um den
        // naechsten Zug. Der laufende Zug bleibt durch acht Bloecke begrenzt.
        if (analyseSteuerWartende.load() != 0)
        {
            std::this_thread::yield();
            continue;
        }

        bool queueHatRest = false;
        {
            std::unique_lock<std::mutex> steuerung (analyseSteuerMutex);
            // Schliesst die Luecke zwischen Vorpruefung und Lock-Erwerb: hat
            // sich dort jemand angemeldet, geben wir sofort wieder frei.
            if (analyseSteuerWartende.load() != 0)
            {
                steuerung.unlock();
                std::this_thread::yield();
                continue;
            }

            const double srWunsch = samplerateAtomic.load();
            const bool analyseRateGueltig = srWunsch > 0.0;
            if (analyseRateGueltig)
            {
                engine.vorbereiten (srWunsch);          // no-op bei gleicher Rate
            }
            // Auch die ungueltige Rate muss die FeatureEngine sehen: sie
            // deaktiviert damit eine eventuell alte, gueltige Binzuordnung.
            merkmale.vorbereiten (srWunsch);

            const auto aktuellerAnlauf = queue.aktuellerAnlauf();
            if (aktuellerAnlauf != workerAnlauf)
            {
                // Auch ein same-rate-prepare ist eine Messgrenze. M1 besitzt
                // keinen Deskriptor und muss sie hier explizit bekommen.
                engine.zuruecksetzen();
                quarantaene.zuruecksetzen();
                unverarbeitet = 0;
                workerAnlauf = aktuellerAnlauf;
                kadenz.zuruecksetzen (detail::WorkerKadenz::Uhr::now());
            }

            if (messResetWunsch.exchange (false))
            {
                engine.zuruecksetzen();
                merkmale.zuruecksetzen();
                // Gegenpfad: was in Quarantäne liegt, gehört zur alten Messung.
                quarantaene.zuruecksetzen();
                unverarbeitet = 0;
                kadenz.zuruecksetzen (detail::WorkerKadenz::Uhr::now());
            }

            // SONDE-008: Block für Block durch die Ein-Block-Quarantäne.
            // Die Steuer-Sperre bleibt bewusst auf einen kleinen Zug begrenzt.
            // Ein dauerhaft voller Producer darf Prepare/Reset/Snapshot nicht
            // hinter einem unendlichen Drain verhungern lassen.
            constexpr int kMaxBloeckeJeSteuerzug = 8;
            int bloeckeInDiesemZug = 0;
            while (bloeckeInDiesemZug < kMaxBloeckeJeSteuerzug)
            {
                // Kommt waehrend des Zugs ein Steuerer hinzu, endet der Zug
                // nach hoechstens dem gerade laufenden Block statt erst bei 8.
                if (analyseSteuerWartende.load() != 0)
                    break;
                const auto* roh = queue.spitze();
                if (roh == nullptr)
                    break;
                ++bloeckeInDiesemZug;
                if (roh->startFolge != queue.aktuellerAnlauf())
                {
                    veralteteBloecke.fetch_add (1);
                    quarantaene.zuruecksetzen();
                    queue.freigeben();
                    continue;
                }

                const auto bruecheVorher = quarantaene.kontinuitaetsbrueche();
                const auto frei = quarantaene.schiebe (queue, *roh);
                queue.freigeben();
                if (quarantaene.kontinuitaetsbrueche() != bruecheVorher)
                {
                    // FeatureEngine erkennt die Grenze am naechsten freigegebenen
                    // Deskriptor selbst. M1 sieht nur Samples und braucht den
                    // expliziten Gegenpfad fuer FFT-, K- und Loudness-Zustaende.
                    engine.zuruecksetzen();
                    unverarbeitet = 0;
                    kadenz.zuruecksetzen (detail::WorkerKadenz::Uhr::now());
                }
                if (frei)
                {
                    // Fail-closed: ein Hostblock ohne gueltige aktuelle Rate
                    // darf weder die alte FeatureEngine noch M1 fuettern. Die
                    // Queue/Quarantaene werden weiterhin begrenzt geleert;
                    // beim naechsten Prepare trennt die Generation den Rest.
                    if (! analyseRateGueltig)
                        continue;

                    const auto grenzenVorher = merkmale.getrennteFenster();
                    const auto straddlesVorher = merkmale.straddleVerworfen();
                    if (merkmale.nimmBlock (*frei.block, frei.audio))
                        merkmalFrames.fetch_add (1);

                    const bool featureGrenze = merkmale.getrennteFenster() != grenzenVorher;
                    const bool blockVerworfen = merkmale.straddleVerworfen() != straddlesVorher;
                    if (featureGrenze)
                    {
                        engine.zuruecksetzen();
                        unverarbeitet = 0;
                        kadenz.zuruecksetzen (detail::WorkerKadenz::Uhr::now());
                    }
                    if (! blockVerworfen)
                    {
                        engine.verarbeite (frei.audio, (int) frei.block->sampleCount,
                                           (int) frei.block->kanaele);
                        samplesAnalysiert.fetch_add ((juce::uint64) frei.block->sampleCount);
                        unverarbeitet += (juce::uint64) frei.block->sampleCount;
                    }
                }
            }
            queueHatRest = queue.spitze() != nullptr;

            // Eine volle Queue fuehrt sofort zum naechsten begrenzten Zug.
            // Daher ist nur die monotone Deadline ein Zeitbeweis; eine Anzahl
            // abgearbeiteter Zuege wuerde Gating/Kandidaten unter Rueckstau
            // bis auf CPU-Geschwindigkeit beschleunigen.
            // Eine bereits sichtbare Steueranfrage geht auch vor einer jetzt
            // faelligen Auswertung. Die Deadline wird dann nicht verbraucht,
            // sondern nach der Uebergabe im naechsten Workerzug bedient.
            const auto faellig = analyseRateGueltig && analyseSteuerWartende.load() == 0
                ? kadenz.faellig (detail::WorkerKadenz::Uhr::now())
                : detail::WorkerKadenz::Faelligkeit {};
            if (faellig.schwer)
            {
                if (unverarbeitet > 0)
                {
                    unverarbeitet = 0;
                    schwereAuswertungen.fetch_add (1);
                    engine.auswerten();
                }
            }
            else if (faellig.leicht)
            {
                engine.auswertenLeicht();
            }
        }

        if (queueHatRest)
        {
            std::this_thread::yield();
            continue;
        }

        std::unique_lock<std::mutex> l (workerWarteMutex);
        workerWarte.wait_for (l, std::chrono::milliseconds (50),
                              [this] { return ! workerLaeuft.load(); });
    }
}

StatsSnapshot EqCopilotProcessor::statsSnapshot() const
{
    StatsSnapshot s;
    s.rmsL = rmsL.load();
    s.rmsR = rmsR.load();
    // SONDE-008: EINE Wahrheit. Der Zähler lebt in der Queue, die den Verlust
    // verursacht; ein zweiter Atomic daneben könnte nur auseinanderlaufen.
    // Einheit unverändert: verlorene Analyse-FRAMES.
    s.framesDropped = queue.verloreneFrames();
    s.nanSeen = nanSeen.load();
    s.hasTransport = hatTransport.load();
    s.transportPlaying = transportSpielt.load();
    s.projectTimeValid = projektZeitGueltig.load();
    s.projectTimeSamples = projektZeitSamples.load();
    return s;
}

// Kompakter Messstand für den v2-Heartbeat (Vertrag: measurement in
// eq-ipc.schema.json). Läuft 1×/s im Pipe-Thread; engine.snapshot() ist die
// threadsichere Kopie. LTAS wird auf 0,1 dB gerundet — die volle Auflösung
// bleibt der lokalen Snapshot-Datei vorbehalten.
MessKompakt EqCopilotProcessor::messKompakt() const
{
    const auto m = engine.snapshot();
    MessKompakt k;
    k.vorhanden = true;   // auch »keine_daten« ist Information für die Übersicht
    k.zustand = m.zustand == MessZustand::messbereit ? "messbereit"
              : m.zustand == MessZustand::sammelt    ? "sammelt"
                                                     : "keine_daten";
    k.metricsVersion = kMetricsVersion;
    k.aktivS  = m.aktivSekunden;
    k.gesamtS = m.gesamtSekunden;
    k.lufsGueltig = m.lufsGueltig;           k.lufsI = m.lufsIntegriert;
    k.lufsShortGueltig = m.lufsShortGueltig; k.lufsS = m.lufsShort;
    k.truePeakGueltig = m.zustand != MessZustand::keineDaten && m.truePeakDb > -199.0;
    k.truePeakDb = m.truePeakDb;
    k.crestGueltig = m.crestGueltig;         k.crestDb = m.crestDb;
    k.spektralGueltig = m.spektralGueltig;
    k.centroidHz = m.centroidMagHz;          k.lowFrac = m.lowFrac;
    k.stereoGueltig = m.stereoGueltig;
    k.width = m.width;                       k.corr = m.corr;
    if (m.ltasGueltig)
    {
        k.ltasKompositDb.resize ((size_t) kLtasBaender);
        for (int b = 0; b < kLtasBaender; ++b)
        {
            const double v = m.ltasKompositDb[(size_t) b];
            k.ltasKompositDb[(size_t) b] = std::isfinite (v)
                ? std::round (v * 10.0) / 10.0
                : std::numeric_limits<double>::quiet_NaN();   // ⇒ null im JSON
        }
    }
    k.fensterSpruenge = fensterSpruenge.load();
    if (fensterAktiv.load())
    {
        k.fensterGueltig  = true;
        k.fensterVon      = fensterVon.load();
        k.fensterBis      = fensterBis.load();
    }
    // Hör-Markierung aktiv ⇒ nachgelagerte Sensoren hören gefärbtes Signal —
    // der Broker markiert den Messstand und pausiert fremde Aggregate
    // (Konzept v2 §5; Broker-Seite folgt nach dem Harness-Lauf).
    k.hoermarkierung = markierung.hoerbar();
    return k;
}

// Sichtbare Antwort auf einen Kennungs-Konflikt (Plan §8.4): DIESE Instanz
// bekommt eine frische persistente ID und meldet sich neu an. Der Host
// speichert sie mit dem nächsten Projekt-Save (getStateInformation) — dafür
// MUSS er die Änderung kennen: Host-Dirty (Vertrag nakama-state-v2.md §6).
bool EqCopilotProcessor::neueSensorId()
{
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen)
            return false;
        zustand.common.instanceId = juce::Uuid().toString();
    }
    meldeHostDirty();
    pipe.reconnect();
    return true;
}

void EqCopilotProcessor::meldeHostDirty()
{
    updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withNonParameterStateChanged (true));
}

// ── State: Schema 2 `NakamaState` (SONDE-006) ─────────────────────────────
// Vertrag: eq-copilot/schemas/state/nakama-state-v2.md. Schema 1 wird rein
// und deterministisch migriert; ein Stand, den dieser Build nicht
// interpretieren darf, wird read-only gehalten und bytegleich zurueckgegeben.
void EqCopilotProcessor::getStateInformation (juce::MemoryBlock& ziel)
{
    std::lock_guard<std::mutex> l (bindungMutex);
    nakama::state::speichere (zustand, ziel);
}

void EqCopilotProcessor::setStateInformation (const void* daten, int groesse)
{
    if (daten == nullptr || groesse <= 0)
        return;

    nakama::state::Zustand geladen;
    const auto ergebnis = nakama::state::lade (daten, (size_t) groesse, bundleVertrag(), geladen);
    if (ergebnis == nakama::state::LadeErgebnis::ignoriert)
        return;   // fremder Baumtyp / Muell: Zustand bleibt (wie seit 0.1)

    if (ergebnis == nakama::state::LadeErgebnis::nurLesen)
    {
        // Keine vertrauenswuerdige Identitaet ⇒ keine Anmeldung beim Broker.
        // ERST stoppen, DANN tauschen: ein gerade laufender (Re-)Connect liest
        // den Zustand im hello-Lambda - nach dem Tausch waere das ein hello
        // mit leerer instance_id (T2-Befund SONDE-006).
        pipe.stop();
        std::lock_guard<std::mutex> l (bindungMutex);
        zustand = geladen;
        // §53.5: read-only ist kein vollstaendiger State-Restore. Zurueck auf
        // neutral - auch aus einer frueheren positiven Klassifikation.
        lebenslauf.stateRestauriert (ergebnis, geladen);
        spiegleKlassifikation();
        return;
    }

    {
        std::lock_guard<std::mutex> l (bindungMutex);
        zustand = geladen;
        // §53.5: JETZT, nach vollstaendigem Restore, darf klassifiziert
        // werden - Schema-1 `sensor|pre|post` ist zu `legacy` migriert,
        // Schema-1 `hub` und ein bestaetigter Schema-2-Main-State zu `main`.
        lebenslauf.stateRestauriert (ergebnis, geladen);
        spiegleKlassifikation();
    }
    pipe.start();       // No-Op, wenn sie laeuft; hebt einen frueheren read-only-Stopp auf
    pipe.reconnect();   // frisches hello mit der geladenen Bindung
    // Kein Host-Dirty: Laden und Migration sind keine Aenderung des Users.
}

juce::String EqCopilotProcessor::holeSensorId() const { std::lock_guard<std::mutex> l (bindungMutex); return zustand.common.instanceId; }
juce::String EqCopilotProcessor::holeRolle() const    { std::lock_guard<std::mutex> l (bindungMutex); return nakama::state::v2Rolle (zustand.common); }
juce::String EqCopilotProcessor::holeLabel() const    { std::lock_guard<std::mutex> l (bindungMutex); return zustand.common.label; }
juce::String EqCopilotProcessor::holePaarId() const   { std::lock_guard<std::mutex> l (bindungMutex); return zustand.common.pairId; }

bool EqCopilotProcessor::stateNurLesen() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.nurLesen;
}

nakama::state::Herkunft EqCopilotProcessor::holeStateHerkunft() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.herkunft;
}

juce::String EqCopilotProcessor::holeStateGrund() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.grund;
}

int EqCopilotProcessor::holeStateFremdesMajor() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.fremdesMajor;
}

nakama::state::Zustand EqCopilotProcessor::holeZustandKopie() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand;
}

// ── Lifecycle-Klassifikation (§53.5) ───────────────────────────────────────

void EqCopilotProcessor::spiegleKlassifikation()
{
    // Aufrufer haelt `bindungMutex`. Der Audiothread liest ausschliesslich
    // diese Atomic; er befragt den Automaten nie (§53.5: "Klassifikation,
    // Spawn und Pipe-I/O liegen nie im Audiocallback").
    // Der Store bleibt bewusst seq_cst (Vorgabe) wie `editorOffen` und
    // `echtzeitOk` daneben: er laeuft nie im Audiothread, die Ordnung kostet
    // hier nichts, und eine dritte Ordnungsregel im selben Zustandsblock waere
    // eine Frage, die ein Leser jedes Mal neu beantworten muesste. Gelesen
    // wird im processBlock relaxed - dort haengt kein anderer Wert daran.
    istMainKlassifiziert.store (lebenslauf.audioAusnahmeErlaubt());
}

nakama::state::Klassifikation EqCopilotProcessor::holeKlassifikation() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return lebenslauf.klassifikation();
}

bool EqCopilotProcessor::darfBrokerStarten() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return lebenslauf.darfBrokerStarten();
}

void EqCopilotProcessor::setzeEditorOffen (bool offen)
{
    // Zwei Verbraucher, ein Ereignis: der Audiothread-Term der Markierungs-
    // Verriegelung und die Editor-Haelfte der Brokerstart-Bedingung.
    editorOffen.store (offen);
    std::lock_guard<std::mutex> l (bindungMutex);
    lebenslauf.editorOffen (offen);
    // Kein spiegleKlassifikation(): der Editor allein klassifiziert nichts
    // (§53.5 verlangt Editor UND explizite Initialisierung). Die
    // Markierungs-Verriegelung traegt `editorOffen` ohnehin als eigenen Term.
}

bool EqCopilotProcessor::setzeBindung (const juce::String& r, const juce::String& lbl, const juce::String& p)
{
    // Dieselben Grenzen wie der einzige UI-Aufrufer. Sie gehoeren auch an die
    // API-Kante: der State-Leser beweist seinen Writer-Headroom gegen genau
    // diese Grenzen; ein kuenftiger Caller darf sie nicht umgehen.
    if (lbl.length() > 120 || p.length() > 60)
        return false;

    nakama::state::Klasse klasse;
    nakama::state::Messposition position;
    if (! nakama::state::ausV2Rolle (r, klasse, position))
        return false;

    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen)
            return false;
        auto neu = zustand.common;
        neu.klasse = klasse;
        neu.position = position;
        neu.label = lbl;
        neu.pairId = p;
        if (neu == zustand.common)
            return false;   // keine Aenderung: kein Dirty, kein Reconnect-Geflacker
        zustand.common = neu;

        // §53.5, dritter Punkt: "leerer, nie gespeicherter Altstate → Main
        // erst nach geoeffnetem Editor UND expliziter Initialisierung". Genau
        // hier ist dieser Akt - der einzige Aufrufer von `setzeBindung` ist
        // die Rollenwahl im Editor (PluginEditor.cpp), und der Automat
        // verlangt zusaetzlich selbst einen offenen Editor. Ein Scannerlauf
        // kann ihn nicht ausloesen: er bedient nichts und oeffnet nichts.
        //
        // Der Weg gilt in BEIDE Richtungen: stellt der User `hub` zurueck auf
        // `sensor`, faellt die Klassifikation auf `legacy`. Sonst behauptete
        // der Automat etwas anderes als der Stand, den dieselbe Instanz im
        // naechsten Projekt-Save schreibt.
        lebenslauf.expliziteInitialisierung (zustand);
        spiegleKlassifikation();
    }
    meldeHostDirty();
    pipe.reconnect();
    return true;
}

// ── Lokaler Mess-Snapshot als Datei (M1 §11: "lokale Snapshot-Erfassung") ──
// Kein Roh-Audio, keine Historie im Plugin-State — nur der Messstand.
// NaN/±inf werden als JSON-null geschrieben (juce::JSON kennt kein NaN).
bool EqCopilotProcessor::schreibeSnapshotDatei (juce::String& pfadOderFehler)
{
    const auto m = engine.snapshot();
    if (m.zustand == MessZustand::keineDaten)
    {
        pfadOderFehler = "noch keine Messdaten";
        return false;
    }

    auto zahl = [] (double v, bool gueltig = true)
    {
        return (gueltig && std::isfinite (v)) ? juce::var (v) : juce::var();
    };
    auto* wurzel = new juce::DynamicObject();
    // v3 (M3a): zusätzlich Band-Perzentile, Zonen-Zeitverlauf und Konvergenz —
    // v2 brachte die Befundkarten. Ältere Leser ignorieren neue Felder.
    wurzel->setProperty ("snapshot_version", 3);
    wurzel->setProperty ("metrics_version", kMetricsVersion);
    wurzel->setProperty ("diagnose_version", kDiagnoseVersion);
    wurzel->setProperty ("created_utc", juce::Time::getCurrentTime().toISO8601 (true));

    {
        auto* sensor = new juce::DynamicObject();
        std::lock_guard<std::mutex> l (bindungMutex);
        const auto& c = zustand.common;
        sensor->setProperty ("sensor_id", c.instanceId);
        sensor->setProperty ("role", nakama::state::v2Rolle (c));
        sensor->setProperty ("label", c.label);
        sensor->setProperty ("pair_id", c.pairId.isEmpty() ? juce::var() : juce::var (c.pairId));
        sensor->setProperty ("samplerate", m.samplerate);
        sensor->setProperty ("channels", kanaeleAtomic.load());
        wurzel->setProperty ("sensor", juce::var (sensor));
    }

    wurzel->setProperty ("zustand", m.zustand == MessZustand::messbereit ? "messbereit" : "sammelt");
    wurzel->setProperty ("aktiv_sekunden", m.aktivSekunden);
    wurzel->setProperty ("gesamt_sekunden", m.gesamtSekunden);
    // Ehrlichkeits-Ausweis (Paket C): so viele nicht-endliche Eingangssamples
    // hat die Analyse seit dem Messstart durch Stille ersetzt.
    wurzel->setProperty ("nan_ersetzt_samples", (juce::int64) m.nanErsetzt);

    {
        auto* loud = new juce::DynamicObject();
        loud->setProperty ("lufs_integriert", zahl (m.lufsIntegriert, m.lufsGueltig));
        loud->setProperty ("lufs_short", zahl (m.lufsShort, m.lufsShortGueltig));
        loud->setProperty ("true_peak_dbtp", zahl (m.truePeakDb));
        loud->setProperty ("crest_db", zahl (m.crestDb, m.crestGueltig));
        wurzel->setProperty ("loudness", juce::var (loud));
    }
    {
        auto* sp = new juce::DynamicObject();
        sp->setProperty ("centroid_mag_hz", zahl (m.centroidMagHz, m.spektralGueltig));
        sp->setProperty ("rolloff_hz", zahl (m.rolloffHz, m.spektralGueltig));
        sp->setProperty ("low_frac", zahl (m.lowFrac, m.spektralGueltig));
        sp->setProperty ("flatness", zahl (m.flatness, m.spektralGueltig));
        juce::Array<juce::var> band;
        for (const double v : m.band8Prozent)
            band.add (zahl (v, m.spektralGueltig));
        sp->setProperty ("band_pct", band);
        wurzel->setProperty ("spektral", juce::var (sp));
    }
    {
        auto* st = new juce::DynamicObject();
        st->setProperty ("width", zahl (m.width, m.stereoGueltig));
        st->setProperty ("corr", zahl (m.corr, m.stereoGueltig));
        st->setProperty ("echt_stereo", m.stereoGueltig);
        wurzel->setProperty ("stereo", juce::var (st));
    }
    {
        auto* ltas = new juce::DynamicObject();
        juce::Array<juce::var> zentren, komposit, referenzKurve;
        for (int b = 0; b < kLtasBaender; ++b)
        {
            zentren.add (m.ltasZentrenHz[(size_t) b]);
            komposit.add (zahl (m.ltasKompositDb[(size_t) b], m.ltasGueltig));
            referenzKurve.add (zahl (m.ltasReferenzDb[(size_t) b], m.ltasGueltig));
        }
        ltas->setProperty ("zentren_hz", zentren);
        ltas->setProperty ("komposit_db", komposit);
        ltas->setProperty ("referenz_8192_db", referenzKurve);
        wurzel->setProperty ("ltas", juce::var (ltas));
    }
    {
        juce::Array<juce::var> ab;
        for (const auto& a : m.abdeckung)
        {
            auto* o = new juce::DynamicObject();
            o->setProperty ("zentrum_hz", a.zentrumHz);
            o->setProperty ("anteil", a.anteil);
            o->setProperty ("klasse", a.klasse == AbdeckungsKlasse::belastbar ? "belastbar"
                                    : a.klasse == AbdeckungsKlasse::eingeschraenkt ? "eingeschraenkt"
                                                                                   : "nicht_messbar");
            ab.add (juce::var (o));
        }
        wurzel->setProperty ("abdeckung", ab);
    }
    {
        // M3a: Band-Perzentile (1-dB-Quantisierung, Zuständigkeits-Stufe).
        auto* pz = new juce::DynamicObject();
        juce::Array<juce::var> p10, p50, p95;
        for (int b = 0; b < kLtasBaender; ++b)
        {
            p10.add (zahl (m.perzentilP10[(size_t) b], m.perzentileGueltig));
            p50.add (zahl (m.perzentilP50[(size_t) b], m.perzentileGueltig));
            p95.add (zahl (m.perzentilP95[(size_t) b], m.perzentileGueltig));
        }
        pz->setProperty ("p10_db", p10);
        pz->setProperty ("p50_db", p50);
        pz->setProperty ("p95_db", p95);
        wurzel->setProperty ("perzentile", juce::var (pz));
    }
    {
        // M3a: Zonen-Zeitverlauf (Anteil aktiver 1-s-Ticks jenseits der
        // Regel-Schwelle; Reihenfolge = ZonenRegeln.h).
        static const char* zonenNamen[kZonenAnzahl] = { "mitten_loch", "mulm", "haerte", "hoehen_hype" };
        juce::Array<juce::var> zz;
        for (int z = 0; z < kZonenAnzahl; ++z)
        {
            const auto& w = m.zonenZeit[(size_t) z];
            auto* o = new juce::DynamicObject();
            o->setProperty ("zone", zonenNamen[z]);
            o->setProperty ("gueltig", w.gueltig);
            o->setProperty ("anteil_jenseits", w.anteil);
            o->setProperty ("ticks", (int) w.ticks);
            zz.add (juce::var (o));
        }
        wurzel->setProperty ("zonen_zeit", zz);
    }
    {
        // M3a: Konvergenz „Kurve steht" (rein informativ).
        auto* ko = new juce::DynamicObject();
        ko->setProperty ("gueltig", m.konvergenzGueltig);
        ko->setProperty ("mean_db", zahl (m.konvergenzMeanDb, m.konvergenzGueltig));
        ko->setProperty ("max_db", zahl (m.konvergenzMaxDb, m.konvergenzGueltig));
        wurzel->setProperty ("konvergenz", juce::var (ko));
    }
    {
        juce::Array<juce::var> res;
        for (const auto& r : m.resonanzen)
        {
            auto* o = new juce::DynamicObject();
            o->setProperty ("freq_hz", r.freqHz);
            o->setProperty ("db_over", r.dbOver);
            o->setProperty ("persistenz", r.persistenzAnteil);
            o->setProperty ("breite_okt", r.breiteOktaven);
            o->setProperty ("klasse", r.klasse == ResonanzKlasse::dauerhaft ? "dauerhaft" : "zeitweise");
            res.add (juce::var (o));
        }
        wurzel->setProperty ("resonanzen", res);
    }
    {
        // M3-Kern: die Befundkarten (deterministisch aus GENAU diesem
        // Messstand — dieselbe Funktion wie Hinweis-Knopf und Golden-Test).
        const auto befunde = diagnose (m, holeRolle());
        juce::Array<juce::var> bf;
        for (const auto& b : befunde)
        {
            auto* o = new juce::DynamicObject();
            o->setProperty ("klasse", b.klasse == BefundKlasse::resonanz ? "resonanz"
                                    : b.klasse == BefundKlasse::mittenLoch ? "mitten_loch"
                                    : b.klasse == BefundKlasse::mulm ? "mulm"
                                    : b.klasse == BefundKlasse::haerte ? "haerte"
                                                                       : "hoehen_hype");
            o->setProperty ("f_von_hz", b.fVon);
            o->setProperty ("f_bis_hz", b.fBis);
            o->setProperty ("f_schwerpunkt_hz", b.fSchwerpunkt);
            o->setProperty ("staerke_db", b.staerkeDb);
            o->setProperty ("titel", b.titel);
            o->setProperty ("gemessen", b.gemessen);
            o->setProperty ("wirkung", b.wirkung);
            o->setProperty ("tu", b.tu);
            o->setProperty ("warum", b.warum);
            o->setProperty ("hoeren", b.hoeren);
            o->setProperty ("konfidenz", b.konfidenz == Konfidenz::hoch ? "hoch"
                                       : b.konfidenz == Konfidenz::mittel ? "mittel" : "niedrig");
            o->setProperty ("konfidenz_grund", b.konfidenzGrund);
            o->setProperty ("top_rang", b.topRang);
            bf.add (juce::var (o));
        }
        wurzel->setProperty ("befunde", bf);
    }
    wurzel->setProperty ("raw_audio", juce::var());   // per Vertrag immer null

    auto ordner = juce::File::getSpecialLocation (juce::File::windowsLocalAppData)
                      .getChildFile ("evenacadia").getChildFile ("EQ-Copilot")
                      .getChildFile ("snapshots");
    if (! ordner.createDirectory())
    {
        pfadOderFehler = "Ordner nicht anlegbar: " + ordner.getFullPathName();
        return false;
    }
    juce::String labelTeil;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        for (const auto z : zustand.common.label)
            if (juce::CharacterFunctions::isLetterOrDigit (z) || z == '-')
                labelTeil += z;
    }
    const auto zeit = juce::Time::getCurrentTime().formatted ("%Y%m%d-%H%M%S");
    const auto datei = ordner.getChildFile ("snapshot-" + zeit
                                            + (labelTeil.isEmpty() ? "" : "-" + labelTeil.substring (0, 40))
                                            + ".json");
    if (! datei.replaceWithText (juce::JSON::toString (juce::var (wurzel), false), false, false, "\n"))
    {
        pfadOderFehler = "Schreiben fehlgeschlagen: " + datei.getFullPathName();
        return false;
    }
    pfadOderFehler = datei.getFullPathName();
    return true;
}

juce::AudioProcessorEditor* EqCopilotProcessor::createEditor()
{
    return new EqCopilotEditor (*this);
}

} // namespace eqcop
