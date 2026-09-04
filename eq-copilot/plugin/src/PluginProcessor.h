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
#include "../core/ipc/InterventionsRing.h"
#include "NakamaState.h"
#include "NakamaLebenslauf.h"
#include "NakamaHostBridge.h"
#include "StampedAudioQueue.h"
#include "analysis/FeatureEngine.h"
#include "../core/analysis/Vergleichspegel.h"
#include "../core/analysis/Blindvergleich.h"
#include "BrokerLifecycle.h"
#include "ControlClient.h"
#include "TelemetryClient.h"
#include "SourcesModel.h"

#include <map>
#include <vector>

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

// S10-11/SONDE-008: der Prozessor ist eine `Senke` der Hostbruecke. Bis hierher
// war die Bruecke aus SONDE-003 im Produkt zwar uebersetzt, aber UNBENUTZT
// (`plugin-wissen.md` §2.1: "im Produkt kompiliert, aber unbenutzt … Verbraucher
// SONDE-008/009"). Sie ist die einzige Quelle, die "Context fehlt" ueberhaupt
// ausdruecken kann - und ohne diese Unterscheidung waere jeder Zeitstempel der
// StampedAudioQueue geraten statt bewiesen (§32.3, NAK-24).
class EqCopilotProcessor : public juce::AudioProcessor,
                           public eqcop::hostbruecke::Senke
{
public:
    EqCopilotProcessor();
    ~EqCopilotProcessor() override;

    /** Audiothread, unmittelbar VOR `processBlock` (gepatchter VST3-Wrapper).
        Nimmt nur die sechs Felder ab, die der Zeitstempel braucht - eine Kopie
        des ganzen `HostBlockContext` waere 256 Byte Buslatenztabelle je Block,
        die hier niemand liest. */
    void nakamaBlockEmpfangen (const eqcop::hostbruecke::Blockbefund&) noexcept override;

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

    // ── SONDE-008: Telemetrie des Analyseweges (Entwurf §53.7 „Droptelemetrie") ──
    // Ein Drop ist hier IMMER ein ganzer Block; `verloreneFrames` sagt, wieviel
    // Zeit das war. Beide Zahlen misst `EqCopQueueStressTest`.
    juce::uint64  analyseDropsUeberlauf() const   { return queue.dropsUeberlauf(); }
    juce::uint64  analyseDropsOversize() const    { return queue.dropsOversize(); }
    juce::uint64  analyseBloeckeAngenommen() const { return queue.bloeckeAngenommen(); }
    juce::uint32  analyseGroessterBlock() const   { return queue.groessterBlock(); }
    juce::uint64  analyseVersiegelt() const       { return quarantaene.versiegelteBloecke(); }
    juce::uint64  analyseQuarantaeneVerworfen() const { return quarantaene.verworfeneBloecke(); }
    juce::uint64  analyseKontinuitaetsbrueche() const { return quarantaene.kontinuitaetsbrueche(); }
    juce::uint64  analyseVeraltet() const         { return veralteteBloecke.load(); }
    // Test-/Diagnosezaehler: echte Aufrufe der teuren Gating-/Kandidatenrunde.
    // Er ist absichtlich monoton ueber Resets; Tests vergleichen Differenzen.
    juce::uint64  analyseSchwereAuswertungen() const { return schwereAuswertungen.load(); }
    static constexpr int analyseMaxBlockFrames()  { return Strom::maxBlockFrames; }

    // ── SONDE-009: Fensterbuchhaltung der FeatureEngine v2 ──────────────────
    // Der Gate-Text („Drop/Seek/Loop trennt jedes offene Fenster") wird hier
    // auslesbar. NAK-57 gilt unverändert: eine ANZEIGE bekommen diese Zahlen in
    // diesem Ticket nicht — die Oberflächen kommen aus Figma. `EqCopAnalysis-
    // GoldenTest` liest sie, sonst niemand.
    juce::uint64 merkmaleGetrennteFenster() const   { auto l = externerAnalyseSteuerZug(); return merkmale.getrennteFenster(); }
    juce::uint64 merkmaleEpochenwechsel() const     { auto l = externerAnalyseSteuerZug(); return merkmale.epochenwechsel(); }
    juce::uint64 merkmaleSegmentwechsel() const     { auto l = externerAnalyseSteuerZug(); return merkmale.segmentwechsel(); }
    juce::uint64 merkmaleStraddleVerworfen() const  { auto l = externerAnalyseSteuerZug(); return merkmale.straddleVerworfen(); }
    juce::uint64 merkmaleNak29Abgelehnt() const     { auto l = externerAnalyseSteuerZug(); return merkmale.nak29Abgelehnt(); }
    juce::uint64 merkmaleBloecke() const            { auto l = externerAnalyseSteuerZug(); return merkmale.bloeckeGesehen(); }
    juce::uint64 merkmaleFrames() const             { return merkmalFrames.load(); }
    // Kopie unter derselben Sperre wie der Single-Writer. Damit ist dieser
    // bereits oeffentliche Pfad auch fuer den ersten Live-Consumer sicher.
    nakama::analyse::FeatureFrame merkmalFrame() const
    {
        auto l = externerAnalyseSteuerZug();
        return merkmale.frame();
    }

    // ── Live-Status für Editor/Heartbeat ──
    StatsSnapshot statsSnapshot() const;
    PipeClient::Snapshot pipeSnapshot() const             { return pipe.snapshot(); }
    nakama::ipc::ControlClient::Snapshot controlV3Snapshot() const
    { return controlV3.snapshot(); }
    nakama::ipc::BrokerLifecycle::Snapshot brokerLifecycleSnapshot() const
    { return brokerLifecycle.snapshot(); }
    SourcesModel::Sicht sourcesSicht() const             { return sourcesModel.sicht(); }
    void sourcesTick();
    bool waehleSourcesHauptziel (const std::string& instanceId)
    { return sourcesModel.waehleHauptziel (instanceId); }
    bool bindeSourcesHauptziel (const std::string& erwarteteInstanceId);
    bool benenneSourcesHauptziel (const std::string& erwarteteInstanceId,
                                  const juce::String& label);
    bool entferneSourcesHauptziel (const std::string& erwarteteInstanceId);
    void reconnectSources();

    /*  Manuelle Passagen als Projektintent (SONDE-013 M-25, M-69).

        Der Store ist autoritativ fuer die Passage als Evidenzobjekt; hier
        reist nur der Intent mit der FL-Projektdatei. Beide Handgriffe pruefen
        GENAU dieselben Bedingungen wie der Leser in `NakamaState.cpp` - eine
        API, die einen Stand schreibt, den der eigene Leser als read-only
        zurueckgibt, waere ein stiller Datenverlust beim naechsten Oeffnen. */
    bool merkeManuellePassage (const juce::String& passageId, const juce::String& label,
                               juce::int64 projektStart, juce::int64 projektEnde);
    bool vergissManuellePassage (const juce::String& passageId);
    std::vector<nakama::state::ManuellePassage> manuellePassagen() const;

    // ── SONDE-013 M-40 bis M-51: der Experimentpfad des Plugins ────────────
    //
    // 🔑 Nacharbeit 2 (Befund R06): `Vergleichspegel` und `Blindvergleich`
    // waren uebersetzt und im Produkt UNBENUTZT; `nichtEndlicheSamples()`
    // hatte ausserhalb der C++-Tests keinen Leser. Ein nichtendliches Sample
    // im Vergleichsmaterial verriegelte damit zwar lokal den Gain, blieb im
    // Produkt aber ungezaehlt — genau die stille Beschoenigung, die M-07 und
    // CLAUDE.md („verriegelt UND gezaehlt") ausschliessen.
    //
    // Der Pfad ist ausdruecklich MODELLSCHICHT: er sendet Nachrichten und
    // haelt Zustand, er baut kein sichtbares Element (§4.2, P-01 bis P-06
    // gehoeren dem User).

    /** Fuehrt die Engine gerade das Fenster der markierten Passage (M-03/M-25)?

        Der ehrliche Statusbericht des Weges aus Befund R03: `false` heisst
        „die Engine misst ueber die Transportepoche", nicht „es gibt keine
        Passage". Ein Bein misst damit, dass `merkeManuellePassage` die
        Grenzen wirklich bis zur Engine bringt. */
    bool passagenfensterInEngine (std::int64_t& start, std::int64_t& ende) const;

    /** Beginnt einen `manual_external`-Versuch ueber der markierten Passage.

        Reihenfolge aus M-40: Passage bestimmen, Vergleichspegel EINFRIEREN,
        dann die Nachricht mit den unveraenderlichen Referenzen senden. Ohne
        eingefrorenen Pegel entsteht kein Versuch — §15: „Eine Klangwertung
        ohne vorherigen Lautheitsabgleich ist unzulaessig." */
    bool beginneVersuch (const juce::String& passageId);

    /** Erfasst den Kandidaten und bindet die Blindreihenfolge (M-41/M-44).

        Beides in einem Schritt, weil es ein Moment ist: der User hat die
        Fremdaenderung vorgenommen, und die Reihenfolge steht fest, BEVOR er
        hoert. */
    bool erfasseKandidat (bool kandidatZuerst);

    /** Meldet das Hoerurteil (M-49). `hoerurteil` ist eines der vier Worte
        des Vertrags. */
    bool urteileVersuch (const juce::String& hoerurteil,
                         const juce::String& notiz,
                         const juce::String& werkzeug);

    /** Bricht den laufenden Versuch ab (M-47 — der haeufigste Realfall). */
    bool brichVersuchAb();

    /** Die ID des laufenden Versuchs, oder leer. */
    juce::String laufenderVersuch() const;
    /** Ob der Vergleichspegel eingefroren UND gesetzt ist (M-43). */
    bool versuchLautheitAbgeglichen() const;
    /** Der eingefrorene Match-Gain in dB. Nur mit `versuchLautheitAbgeglichen`. */
    double versuchMatchGainDb() const;
    /** 🔑 DER Produktleser von `Vergleichspegel::nichtEndlicheSamples()`
        (Befund R06/M-07). `> 0` heisst: im Vergleichsmaterial standen
        nichtendliche Samples, der Pegel ist verriegelt, und die Zahl reist im
        Wirezustand mit. */
    juce::uint64 versuchNichtEndlicheSamples() const;

#if defined(NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
    nakama::ipc::ControlHello v3HelloFuerTest() const { return v3Hello(); }
    nakama::ipc::ControlStatus v3StatusFuerTest() const { return v3Status(); }
    bool v3ProduktstatusVerdrahtetFuerTest() const
    {
        return controlV3.statusProviderGesetzt();
    }
    std::string v3SubscribeFuerTest() const { return v3SubscribeJson(); }
    void v3LinkFuerTest (bool verbunden) { v3ControlLink (verbunden); }
    void v3AntwortFuerTest (const std::string& json,
                            std::uint8_t schemaMinor = nakama::ipc::kJsonSchemaMinor)
    { v3Antwort (json, schemaMinor); }
    void setzeSourcesFixtureFuerTest (SourcesModel::Sicht fixture)
    { sourcesModel.setzeFixtureFuerTest (std::move (fixture)); }
    void setzeControlTransportFuerTest (
        const nakama::ipc::ControlClient::Snapshot& transport)
    { sourcesModel.setzeControlTransport (transport); }
    std::string ausstehenderSourcesCommandFuerTest() const;
    /** Nur Tests (SONDE-013 M-39): schreibt in den ECHTEN RT-Control-Ring,
        bis er voll ist. Das Sticky-Bit setzt dabei der Ring selbst, nicht
        dieser Aufruf — gemessen wird der Weg von dort nach `v3Status()`.
        Rueckgabe: wie viele Ereignisse Platz hatten. */
    int interventionsRingFuellenFuerTest()
    {
        nakama::ipc::Interventionsereignis e;
        int n = 0;
        while (interventionsRing.schreibe (e))
            ++n;
        return n;
    }

    /** Leert den Ring, ohne das Sticky anzufassen.

        Das Gegenstueck zu `interventionsRingFuellenFuerTest`: ein Bein misst
        damit die Kante aus Befund R01 — bei GEFUELLTEM Ring behauptet der
        Neuaufbau keine Neutralitaet, bei leerem schon. Es leert wie der
        Sender, nicht wie `zuruecksetzen()`; das Sticky bleibt also stehen. */
    int interventionsRingLeerenFuerTest()
    {
        nakama::ipc::Interventionsereignis weg;
        int n = 0;
        while (interventionsRing.lies (weg))
            ++n;
        return n;
    }

    std::uint64_t v3StateRevisionFuerTest() const noexcept
    { return v3StateRevision.load(); }
#endif
    double holeSamplerate() const                          { return samplerateAtomic.load(); }
    int    holeBlockSize() const                           { return blockSizeAtomic.load(); }

    // ── M1-Messung (Engine gehört dem Worker; siehe Single-Writer-Kontrakt) ──
    MessSnapshot messSnapshot() const                      { return engine.snapshot(); }
    // Reset der Messung setzt auch das Projektzeit-Fenster zurück — das
    // Fenster BESCHREIBT die akkumulierte Messung (Plan §5.7).
    void fordereMessResetAn()
    {
        {
            auto l = externerAnalyseSteuerZug();
            // Die Queue-Generation IST die Fensterkante. Ihr Produzent meldet
            // dem Audiothread beim tatsaechlichen Uebernehmen den Reset; ein
            // separates Fensterbit waere zwischen zwei Atomics wieder racy.
            queue.neustartAnfordern();
            messResetWunsch.store (true);
        }
        workerWarte.notify_all();
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
    nakama::ipc::ControlHello v3Hello() const;
    /// SONDE-013 M-37/M-38: leert den Interventionsring und sendet jedes
    /// Ereignis als P0. Laeuft im Worker, nie im Audiothread.
    void interventionenSenden();
    nakama::ipc::ControlStatus v3Status() const;
    nakama::ipc::TelemetryHello v3TelemetryHello() const;
    std::string v3SubscribeJson() const;
    void v3ControlLink (bool verbunden);
    void v3Antwort (const std::string& json, std::uint8_t schemaMinor);
    void v3Frame (const std::uint8_t*, std::size_t, std::uint8_t schemaMinor);
    enum class SourcesCommandArt { confirmJoin, unbindProbe };
    struct SourcesCommand
    {
        SourcesCommandArt art = SourcesCommandArt::confirmJoin;
        std::string commandId, instanceId, projectBindingId, sessionEpoch, json;
        juce::String label;
    };
    bool sendeSourcesCommand (SourcesCommandArt, const std::string& erwarteteInstanceId);
    void wendeBestaetigteSourcesCommandsAn();
    // Lebenszeichen (Konzept v2 §4): „neutral, bis Echtzeit bewiesen" — nur
    // der Audiothread schreibt den Zustand; Ergebnis wandert als Atomic raus.
    void lebenszeichen (int samples, bool spielt);

    // Bindung — vom UI-Thread geschrieben, vom Pipe-Thread beim Hello gelesen.
    // Seit SONDE-006 ein State-Schema-2-Zustand (NakamaState); das Bundle
    // Eqcp darf die Klassen main und legacy laden.
    mutable std::mutex bindungMutex;
    nakama::state::Zustand zustand;
    SourcesModel sourcesModel;
    mutable std::mutex sourcesCommandMutex;
    std::map<std::string, SourcesCommand> ausstehendeSourcesCommands;
    std::vector<SourcesCommand> bestaetigteSourcesCommands;
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
    std::atomic<bool>   nanSeen { false };
    // `hatTransport` heißt seit SONDE-008 „Transport ist BEKANNT" — mit der
    // Hostbrücke ist das erstmals von „irgendein PositionInfo kam"
    // unterscheidbar (NAK-24).
    std::atomic<bool>   hatTransport { false };
    std::atomic<bool>   transportSpielt { false };
    std::atomic<bool>   aufnahmeGueltig { false };
    std::atomic<bool>   aufnahmeAktiv { false };
    std::atomic<bool>   projektZeitGueltig { false };
    std::atomic<juce::int64> projektZeitSamples { 0 };
    std::atomic<std::uint64_t> v3StateRevision { 0 };

    // Projektzeit-Fenster der akkumulierten Messung (Plan §5.7): nur der
    // Audiothread schreibt (Single-Writer), nur während Transport läuft.
    // spruenge zählt Loop-/Seek-Diskontinuitäten (>64 Samples Abweichung).
    std::atomic<bool>         fensterAktiv { false };
    std::atomic<juce::int64>  fensterVon { 0 }, fensterBis { 0 };
    std::atomic<juce::uint32> fensterSpruenge { 0 };
    // Nur-Audiothread-Zustand der Sprungerkennung (kein Atomic nötig).
    juce::int64 fensterErwartet = 0;
    bool        fensterErwartetGueltig = false;

    // ── Analyseweg: zeitgestempelte Ganzblock-Queue (SONDE-008, §53.7) ──────
    // Bis 23.08. stand hier ein `juce::AbstractFifo` über 65 536 interleavten
    // Frames. Sein Vertrag war „nimm, was passt": bei Platzmangel schrieb der
    // Audiothread einen TEILBLOCK und zählte den Rest — der Worker sah danach
    // einen lückenlosen Samplestrom, dem in der Mitte Zeit fehlte, und konnte
    // das nicht mehr sehen. Entwurf §48.1 verlangt „ganz oder gar nicht".
    using Strom = nakama::echtzeit::GenStrom;          // ein Stereo-Tap (Insert)
    using Queue = nakama::echtzeit::StampedAudioQueue<Strom>;
    Queue queue;
    // Die Quarantäne gehört ALLEIN dem Worker (§53.7 „Worker hält den jüngsten
    // vollständigen Block") — sie taucht deshalb in keinem anderen Pfad auf.
    nakama::echtzeit::Blockquarantaene<Strom>  quarantaene;
    std::atomic<juce::uint64> samplesAnalysiert { 0 };
    std::atomic<juce::uint64> schwereAuswertungen { 0 };
    // Blöcke, die aus einem früheren `prepareToPlay`-Anlauf im Ring lagen. Der
    // alte Weg warf sie per `fifo.reset()` vom Nachrichtenthread aus weg —
    // mitten in einen laufenden Leser hinein.
    std::atomic<juce::uint64> veralteteBloecke { 0 };

    // ── Hostbrücke → Audiothread (SONDE-003 endlich verdrahtet) ─────────────
    // `nakamaBlockEmpfangen` läuft auf DEMSELBEN Thread wie `processBlock` und
    // unmittelbar davor (gepatchter Wrapper). Deshalb reichen einfache Member:
    // ein Atomic würde hier eine Threadgrenze behaupten, die es nicht gibt.
    //
    // SONDE-009: der Stand IST der Stempel. Bis S12-13 stand hier eine eigene
    // Struktur mit sechs Feldern, die `processBlock` einzeln in den `Stempel`
    // umkopierte. Mit den zehn Transportfeldern des v3-Vertrags waeren daraus
    // zwei Strukturen mit je sechzehn Feldern geworden, die dasselbe sagen —
    // und die irgendwann auseinanderlaufen, weil jemand ein Feld nur in einer
    // von beiden ergaenzt. `nichtEchtzeit` bleibt das einzige Feld, das NICHT
    // von der Bruecke kommt: `isNonRealtime()` ist eine Frage an den
    // Prozessor, nicht an den Hostkontext.
    struct BrueckeStand
    {
        bool                      frisch { false };
        nakama::echtzeit::Stempel stempel {};
    };
    BrueckeStand brueckeStand;

    std::thread worker;
    std::atomic<bool> workerLaeuft { false };
    std::mutex workerWarteMutex;
    std::condition_variable workerWarte;

    // Nie vom Audiothread genommen. Koppelt Samplerate/Queue-Generation,
    // Reset und den exklusiven Engine-Zug zu einer atomaren Steueroperation.
    mutable std::mutex analyseSteuerMutex;
    // `std::mutex` verspricht keine Fairness. Ein externer Leser/Steuerer
    // meldet sich deshalb VOR dem Lock an; der Worker beginnt keinen weiteren
    // 8er-Zug, solange jemand wartet. Nach Lock-Erwerb wird die Anmeldung
    // geloescht - der Mutex selbst schuetzt dann bis zum Zugende.
    mutable std::atomic<unsigned> analyseSteuerWartende { 0 };
    std::unique_lock<std::mutex> externerAnalyseSteuerZug() const
    {
        struct WarteMarke
        {
            explicit WarteMarke (std::atomic<unsigned>& z) : zaehler (z)
            {
                zaehler.fetch_add (1);
            }
            ~WarteMarke() { zaehler.fetch_sub (1); }
            std::atomic<unsigned>& zaehler;
        } wartet (analyseSteuerWartende);

        std::unique_lock<std::mutex> zug (analyseSteuerMutex);
        return zug;
    }

    // M1: der Worker besitzt die Engine exklusiv; UI/Host stellen nur Wünsche.
    AnalyseEngine engine;
    std::atomic<bool> messResetWunsch { false };

    // ── SONDE-013 Nacharbeit 2 (Befund R03): der Weg von der GESPEICHERTEN
    //    Passage zur Engine ────────────────────────────────────────────────
    //
    // 🔑 `FeatureEngine::setzePassagenfenster` hatte ausserhalb der Tests
    // keinen Aufrufer: `merkeManuellePassage` schrieb nur Plugin-State, und
    // weder Main noch Sonde uebertrugen die Grenzen an eine Engine. Die
    // Passagenmetriken liefen deshalb weiter seit der letzten Transportgrenze
    // — M-03/M-25 verlangen aber genau die Bindung an das markierte Fenster.
    //
    // Die Uebergabe laeuft ueber DENSELBEN Weg wie `messResetWunsch`: ein
    // Wunsch als Atomic, den der Analyseworker unter `analyseSteuerMutex`
    // einloest. Der Nachrichtenthread fasst die Engine nie an, und der
    // Audiothread erst recht nicht.
    std::atomic<bool>          passagenfensterWunsch    { false };
    std::atomic<bool>          passagenfensterLoeschen  { false };
    std::atomic<std::int64_t>  passagenfensterStart     { 0 };
    std::atomic<std::int64_t>  passagenfensterEnde      { 0 };

    // ── SONDE-013 Nacharbeit 2 (Befund R06): der Experimentpfad ────────────
    //
    // Der Vergleichspegel misst das Verhaeltnis der beiden Signale, die der
    // User im A/B WIRKLICH gegeneinander hoert: das ungefaerbte Monitorsignal
    // und dasselbe Signal nach der Hoermarkierung. §38.3 verlangt genau diese
    // Groesse vorab gemessen und fuer die Dauer des Versuchs eingefroren.
    //
    // Gespeist wird er im Audiothread — allokationsfrei und ohne Sperre, wie
    // die Markierung daneben — und NUR, solange ein Versuch vorbereitet wird.
    // Ausserhalb kostet er nichts: `versuchspegelSpeist` ist dann falsch, und
    // die Trockenkopie unterbleibt.
    /// Baut die unveraenderlichen Referenzen eines Versuchs (Paragraph 43.1).
    std::string versuchReferenzJson() const;
    /// Baut den Steuerkopf eines persistenzpflichtigen P0-Befehls.
    std::string versuchKopfJson (const juce::String& commandId) const;

    nakama::analyse::Vergleichspegel vergleichspegel;
    nakama::analyse::Blindvergleich  blindvergleich;
    std::atomic<bool>          versuchspegelSpeist { false };
    std::atomic<std::uint64_t> versuchNichtEndlich { 0 };
    /// Vorallokierte Trockenkopie des Blocks; nie im Audiothread vergroessert.
    std::vector<float>         versuchTrocken;
    mutable std::mutex         versuchMutex;
    juce::String               versuchIdAktiv;
    juce::String               versuchPassageId;
    juce::String               versuchCommandBasis;

    // ── SONDE-009: FeatureEngine v2, neben M1 statt an ihrer Stelle ─────────
    // Beide bekommen denselben versiegelten Blockstrom, und das ist Absicht:
    // sie messen dieselbe Physik auf VERSCHIEDENEN Achsen (M1 auf der bei 30 Hz
    // verankerten `analyze-track`-Achse, v2 auf dem bei 1000 Hz verankerten
    // IEC-61260-Gitter des v3-Vertrags — 1,2 % Versatz, siehe BandGrid.h).
    // M1 abzulösen hieße, die Golden-Kreuzvalidierung aufzugeben; das ist kein
    // Nebenbei-Schritt und gehört nicht in dieses Ticket.
    // Derselbe Single-Writer-Kontrakt wie bei `engine`: nur der Worker.
    nakama::analyse::FeatureEngine merkmale;
    std::atomic<juce::uint64> merkmalFrames { 0 };

    // ── Hör-Markierung: DSP + Erlaubnis-Zustand ──
    HoerMarkierungDsp markierung;

    /** SONDE-013 M-37 bis M-39: der RT→Control-Ring fuer hoerbare Eingriffe.

        Er ist ein FELD und kein Zeiger — der Audiothread darf ihn nicht erst
        anlegen muessen. Die Sequenz vergibt der Audiothread, damit eine
        Luecke im Ring am Empfaenger als Luecke ankommt: ein Ereignis, das den
        Ring nie erreicht hat, hat seine Nummer trotzdem verbraucht (§34.2). */
    nakama::ipc::InterventionsRing interventionsRing;
    std::atomic<std::uint64_t> interventionsSequenz { 0 };
    /// Laufende Nummer des EINGRIFFS (nicht des Ereignisses). Begin und End
    /// teilen sie sich; daraus entsteht die `intervention_id`.
    std::atomic<std::uint64_t> interventionsNummer { 0 };
    /// Gespiegelt aus dem Ring, damit der Worker den Ueberlauf auch dann
    /// sieht, wenn er den Ring gerade geleert hat.
    std::atomic<bool> interventionsRingUeberlauf { false };
    /// Wie viele Begin/End-Paare der Worker wirklich gesendet hat — die
    /// Gegenzahl zu `verworfeneEreignisse()` des Rings.
    std::atomic<std::uint64_t> interventionenGesendet { 0 };
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

    // v3 verbindet immer zuerst. Nur der getrennte Lifecycle-Worker darf nach
    // einem nachgewiesenen Fehlversuch und dem vorhandenen Lebenslauf-Gate den
    // installierten Broker starten. Keiner dieser Member wird aus
    // `processBlock` beruehrt.
    const std::string v3LogonSid;
    const std::string v3PipeName;
    const std::string v3SessionEpoch;
    PipeClient pipe;
    nakama::ipc::ControlClient controlV3;
    nakama::ipc::TelemetryClient telemetryV3;
    nakama::ipc::BrokerLifecycle brokerLifecycle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqCopilotProcessor)
};

} // namespace eqcop
