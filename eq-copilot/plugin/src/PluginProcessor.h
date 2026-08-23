// Transparentes Sammler-VST3 — M0 (Plan §9.1, §11 M0).
//
// Audiothread-Vertrag: Eingang unverändert an Ausgang, 0 Latenz, kein Tail,
// keine Allokation/Sperre/Datei/Netz. Analyse-Samples wandern über einen
// vorallokierten Lock-free-FIFO an einen Worker-Thread; Überlast verwirft
// Analyseframes, nie Audio. NaN/Inf im Eingang wird GEZÄHLT und unverändert
// weitergereicht — erst die AnalyseEngine ersetzt nicht-endliche Samples in
// ihrer Kopie durch Stille (nanErsetzt im MessSnapshot), das Audio nie.
//
// EINZIGE Ausnahme von „unverändert" (Konzept HOER-MARKIERUNG v2): die vom
// User gehaltene Hör-Markierung färbt das Monitorsignal NACH dem Analyse-
// Abgriff — und nur, wenn Echtzeit-Wiedergabe BEWIESEN ist (Lebenszeichen:
// Verhältnis Audiozeit/Echtzeit ≈ 1 über zwei Fenster). Ein Offline-Render
// besteht den Beweis nie ⇒ null gefärbte Render-Samples; die Messung sieht
// stets den ungefärbten Eingang.
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PipeClient.h"
#include "AnalyseEngine.h"
#include "HoerMarkierung.h"
#include "NakamaState.h"
#include "NakamaLebenslauf.h"

// ── S9/SONDE-007b Abschnitt 3: welches Bundle uebersetzt hier? ─────────────
// Die duenne Target-Schicht sagt es (plugin/CMakeLists.txt), nicht dieser
// geteilte Quelltext - dieselbe Form wie NAKAMA_SONDE_PASSIV/AKTIV bei den
// beiden neuen Zielen. Ohne diesen Riegel erbte ein kuenftiges Ziel, das src/
// mituebersetzt, still den Vertrag des Main-Bundles.
#if ! defined (NAKAMA_BUNDLE_MAIN)
 #error "S9/SONDE-007b: NAKAMA_BUNDLE_MAIN ist nicht gesetzt. Welchen Bundlevertrag ein Ziel laedt, kommt aus der Target-Schicht in plugin/CMakeLists.txt - nicht aus src/."
#endif

namespace eqcop
{

/** Welche Produktklassen dieses Bundle laden darf (State-Vertrag §2.3).
    `Eqcp` ⇒ {main, legacy}. Der Wert steht im Kern; hier steht nur, WELCHER
    der drei Vertraege gilt - und das sagt die Target-Schicht. */
inline nakama::state::Bundle bundleVertrag()
{
    return nakama::state::Bundle::eqcp();
}

class EqCopilotProcessor : public juce::AudioProcessor
{
public:
    EqCopilotProcessor();
    ~EqCopilotProcessor() override;

    // ── AudioProcessor ──
    void prepareToPlay (double samplerate, int blockSize) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layout) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                       { return true; }

    const juce::String getName() const override           { return "EQ-Copilot"; }
    bool acceptsMidi() const override                     { return false; }
    bool producesMidi() const override                    { return false; }
    double getTailLengthSeconds() const override          { return 0.0; }

    int getNumPrograms() override                         { return 1; }
    int getCurrentProgram() override                      { return 0; }
    void setCurrentProgram (int) override                 {}
    const juce::String getProgramName (int) override      { return "Standard"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // ── Sensorbindung (UI-Thread) — State-Schema 2 (SONDE-006) ──
    // Der Zustand ist `nakama::state::Zustand` (Common: instance_id, Klasse,
    // Messposition, Label, Paar). Die v2-Rolle (hub|sensor|pre|post) ist bis
    // SONDE-010 die Bruecke zum heutigen Broker-hello.
    juce::String holeSensorId() const;          // = Common.instance_id
    juce::String holeRolle() const;             // v2-Rolle aus Klasse + Messposition
    juce::String holeLabel() const;
    juce::String holePaarId() const;
    juce::String holeRuntimeNonce() const        { return instanceNonce; }
    // Setzt die Bindung; true = echte Aenderung (dann Host-Dirty + Reconnect).
    // false: keine Aenderung, unbekannte Rolle oder read-only.
    bool setzeBindung (const juce::String& rolle, const juce::String& label, const juce::String& paarId);
    // M2, Plan §8.4: sichtbare Antwort auf einen Kennungs-Konflikt — DIESE
    // Instanz bekommt eine frische Sensor-ID und meldet sich neu an (Host-Dirty).
    // false = read-only.
    bool neueSensorId();
    // read-only (Vertrag nakama-state-v2.md §5): ein State, den dieser Build
    // nicht interpretieren darf. Originalbytes reisen unveraendert zurueck,
    // keine Pipe-Anmeldung, der Editor zeigt es.
    bool                    stateNurLesen() const;
    nakama::state::Herkunft holeStateHerkunft() const;
    juce::String            holeStateGrund() const;
    int                     holeStateFremdesMajor() const;
    nakama::state::Zustand  holeZustandKopie() const;

    // ── Lifecycle-Klassifikation (§53.5, S9/SONDE-007b Abschnitt 3) ──
    // `unclassified` beim Laden und audio-neutral; `legacy` bzw. `main` erst
    // nach vollstaendigem State-Restore; ein nie gespeicherter Stand wird
    // `main` erst nach geoeffnetem Editor UND expliziter Initialisierung
    // (= der User setzt die Rolle im Editor). Ein Scannerlauf klassifiziert
    // nicht - er ruft `setStateInformation` nie.
    nakama::state::Klassifikation holeKlassifikation() const;
    // §53.5: "Ausschliesslich ein positiv klassifiziertes Main mit geoeffnetem
    // Editor darf den installierten Broker starten." Heute gibt es keinen
    // Spawn-Pfad; SONDE-010 haengt ihn HIER an, statt eine neue Bedingung zu
    // erfinden. Verbinden duerfen alle Instanzen - auch die neutralen.
    bool darfBrokerStarten() const;
    // true, solange der Broker per heartbeat_ack einen Kennungs-Konflikt meldet.
    bool konfliktGemeldet() const                          { return pipe.snapshot().konflikt; }

    // ── Live-Status für Editor/Heartbeat ──
    StatsSnapshot statsSnapshot() const;
    PipeClient::Snapshot pipeSnapshot() const             { return pipe.snapshot(); }
    double holeSamplerate() const                          { return samplerateAtomic.load(); }
    int    holeBlockSize() const                           { return blockSizeAtomic.load(); }

    // ── M1-Messung (Engine gehört dem Worker; siehe Single-Writer-Kontrakt) ──
    MessSnapshot messSnapshot() const                      { return engine.snapshot(); }
    // Reset der Messung setzt auch das Projektzeit-Fenster zurück — das
    // Fenster BESCHREIBT die akkumulierte Messung (Plan §5.7).
    void fordereMessResetAn()
    {
        messResetWunsch.store (true);
        fensterResetWunsch.store (true);
    }
    // Kompakter Messstand für den v2-Heartbeat (läuft im Pipe-Thread).
    MessKompakt messKompakt() const;

    // ── Hör-Markierung (Konzept v2 §3–§5) — Editor reicht ein, Audiothread färbt ──
    void  markierungEinreichen (const MarkierungsAuftrag& a) { markierung.reicheEin (a); }
    void  markierungAus()                                    { markierung.reicheAus(); }
    bool  markierungHoerbar() const                          { return markierung.hoerbar(); }
    float markierungPhase() const                            { return markierung.phase(); }
    bool  markierungEchtzeitOk() const                       { return echtzeitOk.load(); }
    // Liest UND löscht das Freilauf-Signal (Editor-Poll: Latch fällt sichtbar).
    bool  markierungKillGemeldet()                           { return freilaufKill.exchange (false); }
    // Setzt zugleich den Editor-Term der Markierungs-Verriegelung UND die
    // Editor-Haelfte der Brokerstart-Bedingung (§53.5) - eine Wahrheit, ein
    // Aufruf.
    void  setzeEditorOffen (bool offen);
    // Nur für Headless-Tests: erzwingt Echtzeit- UND Editor-Erlaubnis, um
    // DSP-Verhalten ohne Wanduhr-Taktung und ohne Editor deterministisch zu
    // prüfen. Kein Aufrufer im Produktpfad; Transport- und isNonRealtime-Gates
    // bleiben auch damit wirksam.
    void  testForciereEchtzeit (bool an)                     { testEchtzeit.store (an); }
    // Schreibt den aktuellen Messstand als JSON (ohne Roh-Audio) nach
    // %LOCALAPPDATA%\evenacadia\EQ-Copilot\snapshots\. true = geschrieben.
    bool schreibeSnapshotDatei (juce::String& pfadOderFehler);

private:
    void workerLauf();
    // Lebenszeichen (Konzept v2 §4): „neutral, bis Echtzeit bewiesen" — nur
    // der Audiothread schreibt den Zustand; Ergebnis wandert als Atomic raus.
    void lebenszeichen (int samples, bool spielt);

    // Bindung — vom UI-Thread geschrieben, vom Pipe-Thread beim Hello gelesen.
    // Seit SONDE-006 ein State-Schema-2-Zustand (NakamaState); das Bundle
    // Eqcp darf die Klassen main und legacy laden.
    mutable std::mutex bindungMutex;
    nakama::state::Zustand zustand;
    // §53.5-Automat. Er wird ausschliesslich unter `bindungMutex` gefuehrt
    // (Nachrichten-/Hostthread); der Audiothread liest nie ihn, sondern die
    // Atomic-Spiegelung `istMainKlassifiziert` darunter.
    nakama::state::Lebenslauf lebenslauf;
    // Setzt die Spiegelung nach jeder Zustandsaenderung. Ruft der Aufrufer
    // unter gehaltenem `bindungMutex`.
    void spiegleKlassifikation();
    // Meldet dem Host eine gespeicherte Aenderung (withNonParameterStateChanged)
    // — der VST3-Wrapper setzt daraus IComponentHandler2::setDirty.
    void meldeHostDirty();
    // Flüchtige Verbindungs-ID pro Prozessor-Lebenszeit (v2, Plan §8.2;
    // Entwurf §32.1 `runtime_nonce`) — nach dem Konstruktor unveränderlich,
    // daher ohne Mutex lesbar. Nie Teil des States.
    juce::String instanceNonce;

    // Audiothread → Rest der Welt: nur Atomics.
    std::atomic<double> samplerateAtomic { 0.0 };
    std::atomic<int>    blockSizeAtomic  { 0 };
    std::atomic<int>    kanaeleAtomic    { 0 };
    std::atomic<float>  rmsL { 0.0f }, rmsR { 0.0f };
    std::atomic<juce::uint64> framesDropped { 0 };
    std::atomic<bool>   nanSeen { false };
    std::atomic<bool>   hatTransport { false };
    std::atomic<bool>   transportSpielt { false };
    std::atomic<juce::int64> projektZeitSamples { 0 };

    // Projektzeit-Fenster der akkumulierten Messung (Plan §5.7): nur der
    // Audiothread schreibt (Single-Writer), nur während Transport läuft.
    // spruenge zählt Loop-/Seek-Diskontinuitäten (>64 Samples Abweichung).
    std::atomic<bool>         fensterAktiv { false };
    std::atomic<juce::int64>  fensterVon { 0 }, fensterBis { 0 };
    std::atomic<juce::uint32> fensterSpruenge { 0 };
    std::atomic<bool>         fensterResetWunsch { false };
    // Nur-Audiothread-Zustand der Sprungerkennung (kein Atomic nötig).
    juce::int64 fensterErwartet = 0;
    bool        fensterErwartetGueltig = false;

    // Lock-free Analyseweg (M0: Worker leert und zählt; M1 rechnet hier LTAS).
    static constexpr int kFifoKapazitaet = 1 << 16;   // Samples, interleaved L/R
    juce::AbstractFifo  fifo { kFifoKapazitaet };
    juce::HeapBlock<float> fifoPuffer;                 // [kFifoKapazitaet * 2]
    std::atomic<juce::uint64> samplesAnalysiert { 0 };

    std::thread worker;
    std::atomic<bool> workerLaeuft { false };
    std::mutex workerWarteMutex;
    std::condition_variable workerWarte;

    // M1: der Worker besitzt die Engine exklusiv; UI/Host stellen nur Wünsche.
    AnalyseEngine engine;
    std::atomic<bool> messResetWunsch { false };

    // ── Hör-Markierung: DSP + Erlaubnis-Zustand ──
    HoerMarkierungDsp markierung;
    std::atomic<bool> editorOffen { false };
    std::atomic<bool> testEchtzeit { false };     // nur Tests, s. testForciereEchtzeit
    // §53.5 Satz 1 ("unclassified und audio-neutral") als Atomic fuer den
    // Audiothread. Geschrieben nur von spiegleKlassifikation(), gelesen nur
    // in processBlock - der Automat selbst wird nie aus dem Audiocallback
    // befragt ("Klassifikation, Spawn und Pipe-I/O liegen nie im Audiocallback").
    std::atomic<bool> istMainKlassifiziert { false };
    std::atomic<bool> echtzeitOk { false };       // „Echtzeit bewiesen"
    std::atomic<bool> freilaufKill { false };     // Freilauf bei gesetztem Ziel → Editor
    // Lebenszeichen-Zustand — ausschließlich Audiothread.
    juce::int64 lzLetzterNs = 0;
    juce::int64 lzBucketStartNs = 0;
    juce::int64 lzBucketSamples = 0;
    int  lzBestanden = 0;
    bool lzPrevSpielt = false;

    PipeClient pipe;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqCopilotProcessor)
};

} // namespace eqcop
