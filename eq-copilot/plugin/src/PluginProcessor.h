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
#include <chrono>
#include <deque>
#include <thread>
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
    /// Nacharbeit 3 (Befund C2): die Transportepoche, unter der die Engine
    /// GERADE misst. Ein Bein braucht sie, um eine Markierung VOR und eine
    /// NACH einem Seek auseinanderzuhalten.
    juce::uint64 merkmaleTransportEpoche() const    { auto l = externerAnalyseSteuerZug(); return merkmale.transportEpocheJetzt(); }
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

    /** Fuehrt die Engine gerade das Fenster GENAU dieser Passage?

        Nacharbeit 3 (Befund C3): `passagenfensterInEngine` sagt nur, DASS ein
        Fenster laeuft. Seit der State bis zu 64 Passagen haelt und Begin per
        ID adressiert, ist die zweite Frage die wichtigere — sonst liesse sich
        „A ist gebunden" nicht von „B ist gebunden" unterscheiden, und genau
        daran hing der Fehler. */
    bool passagenfensterFuehrt (const juce::String& passageId) const;

    /** Wie viele Bloecke der Vergleichspegel wirklich aufgenommen hat.

        Befund C4: die Zahl unterscheidet „ausserhalb der Passage gespielt,
        also nichts aufgenommen" von „aufgenommen und zufaellig gleich laut".
        Ohne sie ist ein leerer Pegel nicht von einem gemessenen zu trennen. */
    juce::uint64 versuchAufgenommeneBloecke() const;

    /** Wie oft ein EXTERNER Leser den Analyse-Steuerzug genommen hat.

        Befund C7: der Experimentpfad las Fingerprint, Frame und Passagenepoche
        ohne diesen Zug, waehrend der Analyseworker dieselbe Engine mutierte.
        Ein Bein misst an dieser Zahl, dass `beginneVersuch` und
        `erfasseKandidat` den Zug WIRKLICH nehmen — eine Behauptung ueber
        Threadsicherheit, die kein Kommentar tragen kann. */
    juce::uint64 analyseSteuerZuege() const { return analyseSteuerZuegeGesamt.load(); }

    /** Der Fensterwunsch mit AUSDRUECKLICH mitgegebener Transportepoche.

        Das ist der Produktweg von `merkeManuellePassage`, nur mit der einen
        Eingabe, die das Rennen variiert: der Epoche, die beim MARKIEREN galt.
        Vergeht zwischen Markierung und Workerlauf ein Seek, ist sie aelter als
        die der Engine — und genau dann muss das Fenster ausbleiben (Befund
        C2). Ein Bein kann diesen Verlauf sonst nicht deterministisch fahren:
        Epochenlesung und Wunschablage liegen im Produkt unmittelbar
        nebeneinander. */
    bool passagenfensterWunschFuerTest (const juce::String& passageId,
                                        std::int64_t projektStart,
                                        std::int64_t projektEnde,
                                        std::uint64_t transportEpoche);

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
    /** Der Link-Callback wie im Produkt (Test).

        🔑 NAK-180: der POSITIVE Fall faehrt den Aufbauzug 2+3 mit - genau
        wie `eineVerbindung` es tut. Ohne ihn bliebe `wireGeneration` auf 0,
        und `0` heisst "keine Aussage": `replayFaellig` und `berichtOffen`
        wuerden auf 0 gesetzt, der Sender uebersaehe sie, und jeder Test des
        R2-Wegs waere gruen oder rot aus einem Grund, den das Produkt nicht
        kennt. */
    /// NAK-180 (Test): der Wire-Commit ohne Draht. Ohne ihn bliebe jedes
    /// eingereihte Ereignis in Zustand 1, und die Faelle, die ZUSTELLUNG
    /// voraussetzen (Replay nach Linkwechsel), waeren unerreichbar.
    std::size_t zustelleAllesFuerTest() { return controlV3.zustelleAllesFuerTest(); }
    std::size_t fuelleP0QueueFuerTest() { return controlV3.fuelleP0QueueFuerTest(); }
    void leereP0QueueFuerTest() { controlV3.leereP0QueueFuerTest(); }

    /** NAK-180 Nacharbeit 1 (EP-20/N-28): der positive Aufbau mit einem
        Fenster INNERHALB des Callbacks.

        `imCallback` laeuft, nachdem `v3ControlLink(true)` seine Aufbau-Flags
        und `replayFaellig` gesetzt hat, und BEVOR der Callback zurueckkehrt -
        also genau in der Lage, die N-28 verlangt: der geweckte Worker reiht
        sein Replay ein, waehrend der Link-Callback noch steht. Ohne dieses
        Fenster liefen Aufbau und Replay nacheinander, und der Test bestuende
        auch dann, wenn das Replay erst nach der Rueckkehr entsteht. */
    std::uint64_t v3LinkAufbauFuerTest (const std::function<void()>& imCallback)
    {
        return controlV3.linkAufbauFuerTest ([this, &imCallback]
        {
            v3ControlLink (true);
            if (imCallback)
                imCallback();
        });
    }

    /** NAK-180 Nacharbeit 1 (EP-19/N-08): den Sender ANHALTEN.

        Nur so ist die Lage aus N-08 erzwingbar - ein nicht leerer Ring beim
        Linkaufbau. Laeuft der Worker frei, entscheidet sein Takt, ob Begin und
        End schon verarbeitet sind, und der Fall misst eine Zufallslage statt
        des festgelegten Backlog-Zustands. */
    void senderAnhaltenFuerTest (bool an)
    {
        senderPauseFuerTest.store (an, std::memory_order_relaxed);
        if (! an)
            weckeWorkerFuerTest();
    }
    void weckeWorkerFuerTest()
    {
        std::lock_guard<std::mutex> l (workerWarteMutex);
        workerWarte.notify_all();
    }
    /** Der Sendezug SYNCHRON, auf dem Aufruferthread (Test).

        Fuer Faelle, in denen der Zeitpunkt des Zugs Teil der Messung ist und
        ein zweiter Thread ihn nicht liefern kann. */
    void interventionenSendenFuerTest() { interventionenSenden(); }

    /** NAK-180 Nacharbeit 1 (EP-16/EP-20): Schranken IM Sendezug.

        `phase` ist 0 (Zug begonnen, Sperre und Generation stehen), 1 (der
        Zustellstand ist geprueft, das `end` ist NOCH NICHT eingereiht) und 2
        (Zug beendet). Ein Bein haengt sich hier ein und laesst einen zweiten
        Faden genau dort zum Zug kommen - deterministisch ueber Latch oder
        Bedingungsvariable, nie ueber `sleep` als einzige Ordnung.

        ⚠️ Der Haken laeuft unter `sendeMutex` UND `sendeZustandMutex`. Wer
        hier wartet, blockiert beide - genau das ist bei N-35 der Zweck. */
    void setzeZugHakenFuerTest (std::function<void (int phase)> haken)
    { zugHakenFuerTest = std::move (haken); }

    /** NAK-180 Nacharbeit 2 (WN-06/N-35): dieselbe Schranke am EINTRITT des
        Aufbauzugs des ControlClients. Phase 0 = betreten (Sperre wird gleich
        angefordert), Phase 1 = Sperre uebernommen. */
    void setzeAufbauZugHakenFuerTest (std::function<void (int phase)> haken)
    { controlV3.setzeAufbauZugHakenFuerTest (std::move (haken)); }

    /// Wie viele Ereignisse JETZT im RT-Ring warten - ohne sie zu entnehmen.
    /// Der Riegel, mit dem N-08 seinen Backlog-Zustand BEWEIST statt ihn
    /// anzunehmen (EP-19).
    int interventionsRingFuellstandFuerTest() const
    { return interventionsRing.fuellstand(); }

    /** NAK-180 Nacharbeit 1 (EP-18/R3b): DER Heartbeat-Schritt der
        Sendeschleife, gefahren vom Bein - mit dem echten Hello und dem echten
        Status des Prozessors.

        Er verbraucht die Aufbau-Aussage per CAS, vergibt die Marke, bildet den
        Wiretext und REIHT ihn ein; bei Abweisung stellt er die Aussage
        zurueck. `zustelleAllesFuerTest()` fuegt den Wire-Commit an. Der
        frueher benutzte Weg ueber `v3LinkFuerTest` beruehrte nichts davon. */
    bool v3HeartbeatSchrittFuerTest (std::string& textAus, std::uint64_t sequence = 1)
    {
        auto h = v3Hello();
        h.adresse = nakama::ipc::wireAdresseAusState (h.adresse);
        return controlV3.heartbeatSchrittFuerTest (h, sequence, v3Status(), textAus);
    }

    /** NAK-180 Nacharbeit 2 (WN-01/N-36): wartet ein `end` auf seinen
        Wire-Commit? Der vierte Term des Neutralitaetsurteils, LESBAR. */
    bool abschlussOffenFuerTest() const
    { return abschlussOffen.load (std::memory_order_relaxed); }

    // ── NAK-180 Nacharbeit 2 (WN-03/N-05/M-58): die Nachlauffrist ─────────
    // DIE Produktfunktionen, nicht eine Nachbildung daneben. Die Frist haengt
    // an `tail_samples` eines `end`; ueber den Audioweg allein sind weder der
    // Zahlenrand noch die Reihenfolge lang-dann-kurz erzwingbar.
    void nachlaufFristSetzenFuerTest (std::uint64_t tailSamples)
    {
        std::lock_guard<std::mutex> l (sendeZustandMutex);
        nachlaufFristSetzen (tailSamples);
    }
    bool nachlaufAbgelaufenFuerTest() const
    {
        std::lock_guard<std::mutex> l (sendeZustandMutex);
        return nachlaufAbgelaufen();
    }
    std::int64_t nachlaufFristNsFuerTest() const
    { return nachlaufFristNs.load (std::memory_order_relaxed); }

    /** NAK-180 Nacharbeit 2 (WN-04/EP-04/R13): der Haken IM positiven
        Link-Callback, VOR seinem CAS.

        Der alte Fall haengte sich hinter `v3ControlLink(true)` und schrieb
        danach ueber `berichtOffenFuerTestSetzen` - eine zweite Zeile neben
        der Produktzeile. Der echte Schreibzugriff des Callbacks war zu dem
        Zeitpunkt laengst gelaufen, und `gAlt` war im Callback noch 0: der
        Rotbeweis fiel mit `[0 (G4 gegen G5)]`, also aus dem falschen Grund,
        und eine Rueckkehr zu blinden `store`s waere gruen geblieben.

        Der Haken bekommt die Generation DIESES Aufbaus (nach R10 dort bereits
        vergeben) und laeuft, bevor der Callback `berichtOffen` und
        `replayFaellig` per CAS setzt. Ein Bein haelt ihn fest, baut G+1
        vollstaendig auf und laesst ihn dann los. */
    void setzeLinkAufbauHakenFuerTest (std::function<void (std::uint64_t)> haken)
    { linkAufbauHakenFuerTest = std::move (haken); }

    /** NAK-180 Nacharbeit 2 (WN-05/EP-15/N-37 Fall 2): der Haken IM negativen
        Link-Callback, VOR seinen Loeschungen.

        Ohne ihn konnte kein alter negativer Callback einen NEUEREN Aufbau
        ueberlappen: der erste `v3LinkFuerTest(false)` beendete seine
        Generation synchron, und der als verspaetet bezeichnete zweite fand
        `0` - `0` loescht nichts, und der Fall war folgenlos gruen. Der Haken
        bekommt die STERBENDE Generation und haelt den Callback genau dort an,
        wo seine Loeschungen gleich laufen. */
    void setzeLinkEndeHakenFuerTest (std::function<void (std::uint64_t)> haken)
    { linkEndeHakenFuerTest = std::move (haken); }

    std::uint64_t berichtOffenFuerTest() const { return berichtOffen.load(); }
    /** NAK-180 Nacharbeit 1 (EP-04/R13): schreibt `berichtOffen` GENAU SO, wie
        `v3ControlLink(true)` es tut - per CAS, nie blind.

        Ein Bein misst damit den Fall, den ein `store` verliert: der ueberholte
        Callback von G schreibt NACH dem von G+1. Der Haken fuehrt dieselbe
        Zeile, nicht eine Nachbildung daneben. */
    void berichtOffenFuerTestSetzen (std::uint64_t g)
    {
        auto gesehen = berichtOffen.load();
        while (gesehen < g)
            if (berichtOffen.compare_exchange_weak (gesehen, g))
                return;
    }
    std::uint64_t replayFaelligFuerTest() const { return replayFaellig.load(); }
    std::uint64_t wireGenerationFuerTest() const
    { return controlV3.wireGenerationJetzt(); }
    void v3ReconnectFuerTest() { controlV3.reconnect(); }
    void v3StopFuerTest() { controlV3.stop(); }
    void v3StartFuerTest() { controlV3.start(); }

    void v3LinkFuerTest (bool verbunden)
    {
        // 🔑 NAK-180 Nacharbeit 1 (EP-05): BEIDE Richtungen fahren den
        // Produktweg. Der Ende-Callback braucht die hinterlegte sterbende
        // Generation; ohne sie loeschte er nichts, und ein Bein maesse einen
        // Pfad, den das Produkt nicht hat.
        if (verbunden)
            controlV3.linkAufbauFuerTest ([this] { v3ControlLink (true); });
        else
            controlV3.linkEndeFuerTest ([this] { v3ControlLink (false); });
    }
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

    /** Der zuletzt GESENDETE Versuchsbefehl, roh (Befund C5). */
    std::string letzterVersuchP0FuerTest() const
    {
        std::lock_guard<std::mutex> l (versuchWireMutex);
        return letzterVersuchP0;
    }

    /** Der Wiretext, den der naechste Eingriff im Ring WIRKLICH bekaeme.

        Befund C1: er entnimmt wie der Sender und baut wie der Sender — ein
        Bein misst damit die Zahl, die auf die Leitung geht, statt ein lokales
        Flag. Leer heisst „der Ring ist leer". */
    /** Der Wiretext, den der Sender WIRKLICH auf die Leitung gegeben hat.

        🔑 NAK-180 Messlauf: dieser Leser zog frueher selbst am
        `interventionsRing` — und war damit ein ZWEITER Konsument an einem
        Ring, dessen Kopfkommentar Single-Producer-Single-Consumer zusagt.
        Der Produktkonsument ist `interventionenSenden()` im Worker, und der
        Worker startet unbedingt im Konstruktor. Beide riefen `lies()`, beide
        schrieben `schwanz`: ein Datenrennen, und praktisch stahlen sie
        einander die Ereignisse. Ein Test sah deshalb in jedem zweiten
        Marker-Zyklus kein `audible_intervention_end` — nicht weil keines
        entstand, sondern weil der Worker es zuerst hatte (Manifest §6.6).

        Statt am Ring zu ziehen liest dieser Helfer jetzt den MITSCHNITT des
        echten Senders. Das ist zugleich die staerkere Aussage: gemessen wird,
        was gesendet WURDE, nicht was ein Testpfad daneben gebaut haette.

        Leer heisst „der Sender hat (noch) nichts abgesetzt"; der Aufrufer
        fahre weitere Bloecke. */
    std::string naechstesInterventionsJsonFuerTest (int fristMs = 250)
    {
        // Der Sender ist der Worker; er zieht spaetestens alle 50 ms. Diese
        // Frist wartet auf IHN, statt selbst am Ring zu ziehen — sie ist der
        // Preis dafuer, dass der Ring genau einen Konsumenten behaelt.
        const auto bis = std::chrono::steady_clock::now()
                       + std::chrono::milliseconds (std::max (0, fristMs));
        for (;;)
        {
            {
                std::lock_guard<std::mutex> l (sendeZustandMutex);
                if (! gesendeteInterventionen.empty())
                {
                    auto text = std::move (gesendeteInterventionen.front());
                    gesendeteInterventionen.pop_front();
                    return text;
                }
            }
            if (std::chrono::steady_clock::now() >= bis)
                return {};
            std::this_thread::sleep_for (std::chrono::milliseconds (2));
        }
    }

    /// Wie viele Wiretexte der Sender seit dem Start abgesetzt hat (Test).
    std::uint64_t interventionenGesendetFuerTest() const
    {
        return interventionenGesendet.load (std::memory_order_relaxed);
    }
#endif
    double holeSamplerate() const                          { return samplerateAtomic.load(); }
    /** NAK-180 R5: die letzte als gueltig befundene Rate - die Zahl, mit der
        `tailSamplesFuer` den Quarantaene-Nachlauf rechnet. Ein Bein misst
        damit die EINSPEISUNG; die Rechnung selbst misst es an der reinen
        Funktion in `InterventionsRing.h`. */
    double letzteGueltigeSamplerateFuerTest() const
    {
        return letzteGueltigeSamplerate.load (std::memory_order_relaxed);
    }
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
    /// Der Wiretext EINES Eingriffs (Befund C1) — getrennt vom Senden, damit
    /// die Zahl auf der Leitung messbar ist.
    std::string interventionsWireJson (const nakama::ipc::Interventionsereignis& e,
                                       const std::string& adresseJson) const;
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
    // NAK-180 R5: die letzte Rate, die `prepareToPlay` als GUELTIG befunden
    // hat. `samplerateAtomic` faellt bei einer nicht-endlichen Hostrate auf
    // 0.0 — richtig fuer die Analyse, aber falsch fuer den Quarantaene-Tail:
    // dort waere 0 ein Nachlauf von einem Sample, und §34.2 nennt "zu kurz"
    // den teuren Fehler. Vorgabe 48000 ist dieselbe Zahl, die der Broker bei
    // fehlender Rate ansetzt (`intervention.rs`), damit beide Seiten dieselbe
    // Groesse meinen.
    std::atomic<double> letzteGueltigeSamplerate { 48000.0 };
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
    /// Befund C7: gezaehlte externe Zuege. Monoton, nur Diagnose.
    mutable std::atomic<std::uint64_t> analyseSteuerZuegeGesamt { 0 };
    std::unique_lock<std::mutex> externerAnalyseSteuerZug() const
    {
        analyseSteuerZuegeGesamt.fetch_add (1, std::memory_order_relaxed);
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
    //
    // 🔑 Nacharbeit 3 (Befund C2/C3, M-25/G4): der Wunsch traegt seit dieser
    // Runde ID, Grenzen UND die beim MARKIEREN gelesene Transportepoche.
    //
    // Bis dahin las der Worker die Epoche beim Abarbeiten aus der Engine —
    // also aus derselben Quelle, gegen die `setzePassagenfenster` sie
    // vergleicht. Der Vergleich war damit tautologisch erfuellt, und ein Seek
    // ZWISCHEN Markierung und Workerlauf liess die alten Grenzen unter der
    // neuen Epoche durch. Die Epoche muss aus dem Moment der Markierung
    // stammen, sonst sagt sie nichts.
    std::atomic<bool>          passagenfensterWunsch    { false };
    std::atomic<bool>          passagenfensterLoeschen  { false };
    std::atomic<std::int64_t>  passagenfensterStart     { 0 };
    std::atomic<std::int64_t>  passagenfensterEnde      { 0 };
    std::atomic<std::uint64_t> passagenfensterEpocheWunsch { 0 };
    /// Zaehlt JEDE Bindung und JEDES Loesen. Der Worker veroeffentlicht das
    /// Fenster fuer den Audiothread nur unter der Generation, die er wirklich
    /// eingeloest hat; eine spaetere Bindung entwertet damit sofort, was er
    /// gerade sagen wollte. Ohne diesen Zaehler koennte er zwischen
    /// Wunschannahme und Veroeffentlichung ueberholt werden und das Fenster
    /// der VORIGEN Passage als aktiv melden.
    std::atomic<std::uint64_t> passagenfensterGeneration { 0 };

    /// Welche Passage das Fenster gerade BINDET. Nur der Nachrichtenthread
    /// fasst sie an; der Audiothread braucht die ID nicht, sondern die
    /// Grenzen daneben (Befund C3).
    mutable std::mutex passagenBindungMutex;
    juce::String       gebundenePassageId;
    std::int64_t       gebundenerStart { 0 };
    std::int64_t       gebundenesEnde  { 0 };
    std::uint64_t      gebundeneEpoche { 0 };

    /// Setzt den Wunsch fuer `id` und bindet sie. `false`, wenn die Engine
    /// gerade keine Epoche liefern kann.
    bool bindePassagenfenster (const juce::String& passageId,
                               std::int64_t projektStart, std::int64_t projektEnde);
    bool bindePassagenfensterMitEpoche (const juce::String& passageId,
                                        std::int64_t projektStart,
                                        std::int64_t projektEnde,
                                        std::uint64_t transportEpoche);
    /// Loest die Bindung — nur, wenn `passageId` die gebundene ist.
    bool loesePassagenfenster (const juce::String& passageId);

    // ── Was der AUDIOTHREAD ueber das Fenster wissen muss (Befund C4) ──────
    //
    // Der Audiothread darf die Engine nicht anfassen und kennt die
    // Transportepoche nicht. Der Analyseworker veroeffentlicht deshalb, was
    // er gerade FUEHRT: die Grenzen und ob das Fenster gesetzt UND unversehrt
    // ist. `pegelFensterAktiv` ist das Publikationsbit — Grenzen zuerst,
    // Bit danach, wie beim Messfenster daneben.
    std::atomic<bool>          pegelFensterAktiv { false };
    std::atomic<std::int64_t>  pegelFensterStart { 0 };
    std::atomic<std::int64_t>  pegelFensterEnde  { 0 };

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
    /** Alles, was der Experimentpfad aus der FeatureEngine braucht — in EINEM
        Zug gelesen (Befund C7).

        `versuchReferenzJson` und `beginneVersuch` lasen Fingerprint, Frame und
        Passagenepoche bis zur Runde 2 OHNE `externerAnalyseSteuerZug`,
        waehrend der Analyseworker dieselbe Engine unter `analyseSteuerMutex`
        mutierte. Das ist ein C++-Datenrennen; ausserdem konnten die drei Werte
        aus drei verschiedenen Engine-Staenden stammen und damit eine Referenz
        beschreiben, die es nie gab. Ein Zug, ein Stand. */
    struct Engineabzug
    {
        nakama::analyse::Fingerprint fingerprint {};
        std::uint64_t passagenEpoche { 0 };
        bool          fensterGesetzt { false };
        double        abdeckung { 0.0 };
        bool          abdeckungGesetzt { false };
    };
    Engineabzug engineabzugLesen() const;

    /// Baut die unveraenderlichen Referenzen eines Versuchs (Paragraph 43.1).
    std::string versuchReferenzJson (const Engineabzug& abzug) const;
    /// Baut den Steuerkopf eines persistenzpflichtigen P0-Befehls.
    std::string versuchKopfJson (const juce::String& commandId) const;

    /** Reicht einen Versuchsbefehl weiter und MERKT sich, was gesendet wurde.

        Befund C5: der Nichtendlich-Zaehler soll „im Wirezustand reisen". Bis
        zur Runde 2 war das eine Behauptung ueber eine Zeile, die es nicht gab
        — und der Test las statt der Leitung einen lokalen Getter. Ein Bein
        braucht deshalb Zugriff auf den TEXT, der wirklich gesendet wurde. */
    bool sendeVersuchP0 (const std::string& json);
    mutable std::mutex versuchWireMutex;
    std::string        letzterVersuchP0;

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

    // ── NAK-180: der Sendezustand des Interventionspfads ──────────────────
    //
    // Er beantwortet die eine Frage, die der Ring nicht beantworten kann:
    // WAS WEISS DER BROKER? Der Ring kennt nur, was noch nicht entnommen ist;
    // ein bereits gesendetes Begin ist dort verschwunden, obwohl sein Marker
    // weiterklingt. Genau daran fiel C2 (Reconnect bei hoerbarem Marker).
    struct OffenesBegin
    {
        nakama::ipc::Interventionsereignis ereignis {};
        bool gueltig = false;
        /// `prepareToPlay` hat den Marker hart abgeschaltet; sein `end` kommt
        /// vom Audiothread nie (HoerMarkierung `vorbereiten` meldet keinen
        /// Uebergang). `prepareToPlay` bildet es deshalb SOFORT - samt
        /// Sequenznummer, die es sich aus derselben Folge zieht wie der
        /// Audiothread.
        ///
        /// Warum sofort und nicht erst beim Senden: der Marker faehrt nach
        /// `zielGesetzt()` im naechsten Block WIEDER HOCH und schreibt ein
        /// neues Begin mit der naechsten Nummer in den Ring. Wuerde das
        /// synthetische Ende erst danach gebildet, laege seine Sequenz HINTER
        /// der des neuen Begins - und auf dem Draht stuende Begin(1),
        /// Begin(3), End(2): beim Broker eine Luecke.
        bool tot = false;
        nakama::ipc::Interventionsereignis totesEnde {};
        /// Dreiwertiger Zustellstand (Matrix E6):
        ///   0 = nicht eingereiht (sendeP0 wies ab) → Replay noetig
        ///   1 = eingereiht, nicht auf dem Draht     → KEIN Replay, es reist selbst
        ///   2 = zugestellt                          → Replay noetig, wenn der
        ///                                             Link seither wechselte
        int zustand = 0;
        /// Die `wireGeneration`, auf der zugestellt wurde (Zustand 2). Ein
        /// Vergleich statt einer Umschreibung beim Linkende (E10).
        std::uint64_t zustellGeneration = 0;
        /// Rueckmeldemarke des zuletzt eingereihten Begin.
        std::uint64_t marke = 0;
    };

    /** NAK-180 Nacharbeit 1 (EP-01/N-36): ein Begin, dessen `end` eingereiht,
        aber noch NICHT auf dem Draht ist.

        Der Sender loeschte das offene Begin bisher, sobald das `end` in der
        Queue stand. Stirbt der Link davor, ueberlebt das `end` als Ereignis —
        aber der Aufbauzug des naechsten Links faende kein Begin mehr, stellte
        kein Replay voran, und das `end` traefe beim Broker auf nichts: die
        Sitzung nullt nie (N-36). Das Begin bleibt deshalb hier stehen, bis der
        Zustellrueckruf seiner `endeMarke` kommt.

        Genau EINE solche Aufzeichnung kann noetig sein: sie entsteht nur,
        wenn das Begin bereits ZUGESTELLT war (also nicht selbst in der Queue
        liegt), und der naechste Kandidat kann erst zugestellt werden, wenn
        dieses `end` den Draht verlassen hat — die Queue ist FIFO. */
    struct AbschlussBegin
    {
        nakama::ipc::Interventionsereignis ereignis {};
        bool gueltig = false;
        /// Marke des eingereihten `end`. Sein Wire-Commit schliesst das Paar.
        std::uint64_t endeMarke = 0;
    };

    /** NAK-180 Nacharbeit 1 (EP-08/N-10): ein faelliger Markerabbruch, dessen
        Begin beim `prepareToPlay` noch im RT-Ring lag.

        `vorbereiten()`/`setzeSamplerate()` melden den Uebergang seit dieser
        Runde ZURUECK, statt ihn zu verschlucken. War das Begin zu diesem
        Zeitpunkt noch nicht aus dem Ring entnommen, kann `prepareToPlay` das
        `end` nicht bilden — es haette kein Ereignis, an das es anknuepft. Der
        Uebergang wartet deshalb hier, bis der Sender den Ring geleert hat, und
        wird genau vor dem ersten Ringereignis mit HOEHERER Sequenz eingereiht:
        die Folge auf dem Draht bleibt lueckenlos. */
    struct TotUebergang
    {
        bool gueltig = false;
        std::uint64_t sequenz = 0;
        std::uint64_t dauerSamples = 0;
        std::uint64_t tailSamples = 0;
    };
    /// ⚠️ Sperrenordnung (E11): `sendeMutex` des ControlClients wird VOR
    /// diesem genommen, nie umgekehrt. Der Audiothread fasst ihn nie an.
    mutable std::mutex sendeZustandMutex;
    OffenesBegin offenesBegin;
    AbschlussBegin abschlussBegin;
    TotUebergang ausstehenderTotUebergang;
    /** Marke und Nachlauf JEDES eingereihten, noch nicht zugestellten `end`
        (EP-02, berichtigt in Nacharbeit 2 fuer WN-03).

        Bis hierher stand nur das ZULETZT eingereihte `end` in zwei Feldern.
        Liegen zwei Enden gleichzeitig in der Queue - ein langes, dann ein
        kurzes -, verlor das lange seine Marke, sein Wire-Commit stellte gar
        keine Frist, und die des kurzen galt allein. Der Broker haelt in
        derselben Lage das MAXIMUM beider Nachlaeufe
        (`intervention.rs`: `tail_samples_offen.max(tail_samples)`), das
        Plugin lief also mit der kuerzeren Frist gegen die laengere des
        Brokers - und sein einmaliges `false` reiste in einen laufenden
        Nachlauf (N-05/M-58).

        Strukturell gedeckelt: jeder Eintrag entspricht genau einem
        eingereihten P0-Platz, und `kCapP0` ist 64. */
    std::deque<std::pair<std::uint64_t, std::uint64_t>> ausstehendeEnden;

    // ── NAK-180 Nacharbeit 1: Helfer des Sendezustands ────────────────────
    // Alle drei setzen `sendeZustandMutex` als GEHALTEN voraus; sie werden
    // ausschliesslich aus den P0-Rueckwegen und dem Sendezug gerufen.
    /** EP-02/N-05: Frist bis zum Ablauf des Nachlaufs, gestellt am
        Wire-Commit des `end`. Marge: eine Heartbeat-Periode. */
    void nachlaufFristSetzen (std::uint64_t tailSamples);
    /** EP-02: ist die Frist abgelaufen (oder gab es gar keine)? */
    bool nachlaufAbgelaufen() const;
    /** EP-13/R7: der Wiretext dieser Marke ist auf dem Draht. */
    void mitschnittZustellen (std::uint64_t marke);
    /** EP-13/R7: er ist es nicht und wird es nie. */
    void mitschnittVerwerfen (std::uint64_t marke);
    /// Lock-freie Spiegelung fuer `v3ControlLink`, damit der Callback die
    /// Sperre nicht nehmen muss.
    std::atomic<bool> sendeBeginOffen { false };
    /** NAK-180 Nacharbeit 2 (WN-01/N-36, N-05/M-58): ein `end` wartet auf
        seinen Wire-Commit.

        Das Neutralitaetsurteil des Aufbaus las bis hierher nur
        `sendeBeginOffen` - und das loescht `sendeEnde()`, sobald das `end`
        EINGEREIHT ist. Stirbt der Link in genau diesem Fenster, urteilte
        G+1 „neutral": sein erster Heartbeat trug `false`, der Broker fuehrte
        den R1-Resync aus und loeschte den Tail, den das gerade zugestellte
        `end` erst gestartet hatte. Solange ein `end` auf den Draht wartet, ist
        der Prozessor NICHT neutral - der Broker hat das Paar noch nicht
        gesehen.

        Spiegel von `! ausstehendeEnden.empty()`, unter `sendeZustandMutex`
        geschrieben und vom Callback lock-frei gelesen. */
    std::atomic<bool> abschlussOffen { false };
    /** NAK-180 Nacharbeit 1 (EP-02/N-05): fruehestens ab dieser Zeit darf das
        einmalige `false` des Nachberichts reisen.

        Der Nachlauf des letzten gesendeten `end` laeuft beim Broker in
        ECHTZEIT ab (`tail_samples_offen`). Kam das `false` davor, verwarf der
        Broker es — und niemand wiederholte es, weil `berichtOffen` und die
        Aufbauaussage bereits verbraucht waren; die Sitzung blieb fuer immer
        gesperrt (M-58). Die Frist wird am WIRE-COMMIT des `end` gestellt, denn
        erst dort beginnt der Broker zu zaehlen, und traegt eine Heartbeat-
        Periode Marge.

        Steady-Clock-Nanosekunden; `0` = keine Frist offen. */
    std::atomic<std::int64_t> nachlaufFristNs { 0 };
    /// NAK-180 R10/R13: generationsgebunden, jeder Zugriff ein CAS.
    std::atomic<std::uint64_t> replayFaellig { 0 };
    std::atomic<std::uint64_t> berichtOffen { 0 };
    /// NAK-180 Messlauf: Mitschnitt der tatsaechlich abgesetzten Wiretexte.
    /// Er lebt unter `sendeZustandMutex` — derselben Sperre wie der
    /// Sendezustand, damit kein dritter Mutex entsteht. Gedeckelt, weil ein
    /// unbegrenzter Mitschnitt in einer langen Sitzung waechst.
    std::deque<std::string> gesendeteInterventionen;
    static constexpr std::size_t kMitschnittDeckel = 256;
    /** NAK-180 Nacharbeit 1 (EP-13/R7): Wiretexte, die EINGEREIHT, aber noch
        nicht auf dem Draht sind.

        Mitschnitt und Zaehler entstanden bisher unmittelbar nach dem Enqueue.
        Die Passage-Tests ernteten ihn dann ohne `zustelleAllesFuerTest` und
        blieben gruen, obwohl ein Write scheiterte oder der Aufbaufilter den
        Eintrag verwarf. R7 definiert „gesendet" als Wire-Commit; der Text
        wartet deshalb hier auf seine Zustellmeldung. */
    std::deque<std::pair<std::uint64_t, std::string>> ausstehendeMitschnitte;
    static constexpr std::size_t kAusstehendDeckel = 256;
    /// Wie viele Begin/End-Paare der Worker wirklich gesendet hat — die
    /// Gegenzahl zu `verworfeneEreignisse()` des Rings.
    std::atomic<std::uint64_t> interventionenGesendet { 0 };
    /// NAK-180 Nacharbeit 1 (EP-19/EP-20): nur Tests. Der Sender ueberspringt
    /// seinen Zug, solange das Bit steht - der einzige Weg, die Lage aus N-08
    /// (Ring nicht leer beim Aufbau) zu ERZWINGEN statt sie zu hoffen.
    std::atomic<bool> senderPauseFuerTest { false };
    /// NAK-180 Nacharbeit 1 (EP-16/EP-20): Schranke im Sendezug, nur Tests.
    /// Vor dem ersten `start()` gesetzt und danach unveraendert - wie der
    /// Statusprovider des ControlClients.
    std::function<void (int)> zugHakenFuerTest;
    /// NAK-180 Nacharbeit 2 (WN-04/WN-05): Test-Einhaengepunkte IN den beiden
    /// Link-Callbacks, jeweils VOR der generationsgebundenen Wirkung. Beide
    /// laufen auf dem Nachrichten- bzw. Clientthread, nie im Audiothread, und
    /// halten dabei keine Sperre des Sendezustands.
    std::function<void (std::uint64_t)> linkAufbauHakenFuerTest;
    std::function<void (std::uint64_t)> linkEndeHakenFuerTest;
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
