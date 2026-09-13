#pragma once

/*  S9 / SONDE-007b - die geteilte Schale der Sonden.

    NACHTRAG S9b / SONDE-007c (28.08.2026): Das passive Ziel Nakama Suna ist
    STILLGELEGT (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md).
    Kein Bauziel setzt NAKAMA_SONDE_PASSIV mehr; gebaut wird aus dieser Quelle
    heute nur noch Nakama Probeeq. Der Zweig der passiven Klasse bleibt hier
    absichtlich stehen: er ist die zweite Haelfte des `#error`-Riegels weiter
    unten - ohne ihn koennte ein Ziel wieder OHNE Produktklasse uebersetzen
    und still den Vertrag des anderen erben. Der Text darunter beschreibt
    deshalb weiter beide Klassen; als PRODUKT gibt es Suna nicht mehr.

    EINE QUELLE, GETRENNTE IDENTITAET JE ZIEL:

      Nakama Suna    (NkPr) - passive Sonde. Beriet nur. Stillgelegt 28.08.2026.
      Nakama Probeeq (NkAc) - aktive Sonde. Laut User ein vollwertiger EQ
                              ("die active Probe fester Name : Nakama Probeeq
                              ist ein vollwertiger hochwertiger EQ", 21.08.);
                              seit SONDE-015 Etappe 4a traegt dieses Bundle
                              den aktiven DSP-Kern (`dsp::DspKern`). Nur mit
                              `eq_enabled` aus ist es der Passthrough von
                              bisher (GRUNDGESETZ unten); Buslayout,
                              Lebenszyklus, Analysezufuehrung und float-Kante
                              begruenden sich am rechnenden Kern (NAK-283
                              M-43).

    WARUM DAS HIER LIEGT UND NICHT IM KERN (Entwurf §53.4, S8-Riegel K1):
    Eine AudioProcessor-Ableitung braucht juce_audio_processors; der
    gemeinsame Kern uebersetzt gegen juce_core/events/data_structures/
    cryptography und darf KEINE JucePlugin_*-Konstante sehen. Diese Datei
    sieht sie sehr wohl - sie IST Target-Schicht. Sie wird darum wie
    plugin/src/ je Ziel uebersetzt (zwei Uebersetzungen), nicht in NakamaKern
    gelinkt. Der Umzug hinter die §53.4-Verzeichnisgrenzen bleibt
    inkrementell.

    GRUNDGESETZ (CLAUDE.md, Wahrheitskern): nichts Ungefragtes. Solange
    `v2.global.eq_enabled` aus ist - der Default -, ist Probeeq der
    Passthrough von bisher: sampleidentisch, 0 Samples Latenz, kein Tail,
    keine Bank (SONDE-015 R2, Bein A16). `processBlock` haelt keine Sperre,
    allokiert nicht, protokolliert nicht und fasst keine Datei an: er ruft den
    DSP-Kern (`dsp::DspKern::verarbeite`) und kopiert den Tap `post_committed`
    in die vorallokierte Analysequeue. Programmbau, Transaktionen,
    Auswertung und I/O bleiben ausserhalb des Audiothreads.

    HOSTPARAMETER (SONDE-015 R1): Probeeq meldet die 112 Host-Parameter des
    Layouts v2 in Vertragsreihenfolge - die 109 v1-Kennungen, danach
    `eq_enabled`, `mix` und `auto_gain`; `occupied` ist keiner (R5). Die Liste
    kommt aus `nakama::parameter::tabelle()`, nie aus einer zweiten Liste.
    Hostwerte wirken als fluechtiger AutomationOverlay ohne Revision (§44.3);
    gespeichert wird ausschliesslich der bestaetigte Zustand des
    Transaktionskerns (`state/NakamaTransaktion.h`).

    KEINE ERFUNDENE OBERFLAECHE: `hasEditor()` meldet false. Die Gestaltung
    kommt aus dem Figma-Stand des Users ueber design/ (CLAUDE.md: "Claude
    erfindet keine Richtung"), und die Suna-Kachel ist dort nicht begonnen.
    Eine selbstgebaute Zwischen-UI waere genau die Stilsuche, die das
    Arbeitsmodell ausschliesst.
*/

#include <juce_audio_processors/juce_audio_processors.h>

#include "NakamaLebenslauf.h"
#include "NakamaState.h"
#include "NakamaTransaktion.h"
#include "DspKern.h"
#include "AnalyseEngine.h"
#include "ControlClient.h"
#include "NakamaHostBridge.h"
#include "StampedAudioQueue.h"
#include "TelemetryClient.h"
#include "analysis/FeatureEngine.h"

#include <array>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

// Genau EINE Produktklasse je Ziel - gesetzt von der duennen Target-Schicht
// in plugin/CMakeLists.txt. Der Riegel ist kein Zierrat: ohne ihn uebersetzte
// eine vergessene Zeile stillschweigend das falsche Bundle.
#if defined (NAKAMA_SONDE_PASSIV) && defined (NAKAMA_SONDE_AKTIV)
 #error "S9/SONDE-007b: NAKAMA_SONDE_PASSIV und NAKAMA_SONDE_AKTIV sind beide gesetzt - ein Ziel hat genau EINE Produktklasse."
#endif
#if ! defined (NAKAMA_SONDE_PASSIV) && ! defined (NAKAMA_SONDE_AKTIV)
 #error "S9/SONDE-007b: Weder NAKAMA_SONDE_PASSIV noch NAKAMA_SONDE_AKTIV gesetzt. Die Produktklasse kommt aus der duennen Target-Schicht (plugin/CMakeLists.txt), nicht aus dem geteilten Code."
#endif

namespace nakama::sonde
{

/** Die Produktklasse dieses Bundles - fest, nicht aus dem State geraten.

    §53.5 unterscheidet zwei Dinge, die leicht verwechselt werden:
    die PRODUKTKLASSE (fest am Bundle, hier) und die KLASSIFIKATION
    (`unclassified` bis zu einem gueltigen State - siehe `lebenslauf` unten,
    gebaut in S9 Abschnitt 3). Fuer Gen sind beide verschieden, weil sein
    Bundle `main` ODER `legacy` sein kann; hier fallen sie nach gueltigem
    State zusammen - aber erst dann.
*/
constexpr nakama::state::Klasse kProduktklasse =
   #if defined (NAKAMA_SONDE_PASSIV)
    nakama::state::Klasse::passive_probe;
   #else
    nakama::state::Klasse::active_probe;
   #endif

// Der Lebenslauf-Automat nimmt fuer die feste Produktklasse ausdruecklich nur
// Sondenklassen an: waere `main` moeglich, koennte sich ein Sondenbundle zum
// Main erklaeren. Die Garantie steht hier, wo die Konstante entsteht.
static_assert (kProduktklasse == nakama::state::Klasse::passive_probe
            || kProduktklasse == nakama::state::Klasse::active_probe,
               "S9/SONDE-007b: ein Sondenbundle traegt passive_probe oder active_probe - nichts sonst.");

/** Welche Klassen dieses Bundle laden darf (Vertrag §2.3, SONDE-006). */
inline nakama::state::Bundle bundleVertrag()
{
   #if defined (NAKAMA_SONDE_PASSIV)
    return nakama::state::Bundle::nkpr();
   #else
    return nakama::state::Bundle::nkac();
   #endif
}

class SondeProcessor final : public juce::AudioProcessor,
                             public eqcop::hostbruecke::Senke,
                             private juce::AudioProcessorParameter::Listener
{
public:
    SondeProcessor();
    ~SondeProcessor() override;

    void prepareToPlay (double samplerate, int maxBlock) override;
    void releaseResources() override;

    /** NAK-283 F05 (R-283-3): der dritte Hosteintritt. Der VST3-Wrapper ruft
        ihn bei `setProcessing (false)` - ohne `releaseResources` und ohne
        `prepareToPlay`. Er beendet die Audiohistorie des Kerns und laesst
        Programm, Parameter und Zustand stehen; getTailLengthSeconds() bleibt
        0,0 (SONDE-015 M-51). */
    void reset() override;

    bool isBusesLayoutSupported (const BusesLayout& layout) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void nakamaBlockEmpfangen (const eqcop::hostbruecke::Blockbefund&) noexcept override;

    /** JUCE/VST3 ChannelContext. Laut JUCE-Vertrag nur Message-Thread; ein
        lokaler Verstoss wird fail-closed ignoriert statt in den Runtime-
        Zustand hineinzuracen. */
    void updateTrackProperties (const TrackProperties&) override;

    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override                     { return false; }

    // Der Produktname kommt aus JucePlugin_Name - also aus der Target-Schicht,
    // die ihn ihrerseits aus plugin-identities-v1.json hat. Keine zweite
    // Wahrheit. Die Konsolen-Beweise (EqCopSondeNullTest) bauen dieselbe
    // Quelle OHNE Plugin-Wrapper, dort gibt es die Konstante nicht; sie messen
    // Audio, nie den Namen.
   #if defined (JucePlugin_Name)
    const juce::String getName() const override         { return JucePlugin_Name; }
   #else
    const juce::String getName() const override         { return "nakama-sonde-testschale"; }
   #endif
    bool acceptsMidi() const override                   { return false; }
    bool producesMidi() const override                  { return false; }
    bool isMidiEffect() const override                  { return false; }
    double getTailLengthSeconds() const override        { return 0.0; }

    /** Offline-Render laeuft mit dem bestaetigten Zustand, nie mit einer
        Vorschau (§44.4, §49.2 Gate 3, M-120): der Wechsel in den
        Offline-Betrieb beendet die Preview und stellt die Hoermatrix auf
        Processed. */
    void setNonRealtime (bool offline) noexcept override;

    int getNumPrograms() override                       { return 1; }
    int getCurrentProgram() override                    { return 0; }
    void setCurrentProgram (int) override               {}
    const juce::String getProgramName (int) override    { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& ziel) override;
    void setStateInformation (const void* daten, int groesse) override;

    /** Fuer Tests: genau der Stand, den `getStateInformation` schreibt. Kein
        Hostweg. Die Dsp-Haelfte kommt beim Lesen aus dem Transaktionskern -
        es gibt keine zweite Kopie, die nachlaeuft. */
    nakama::state::Zustand zustandLesen() const;

    //== SONDE-015 Etappe 4a: der lokale Transaktionskern ======================
    //
    // Nicht fuer den Audiothread. Jeder Aufruf nimmt das Zustandsschloss; ein
    // Hostaufruf (Parameterabgleich, Host-Dirty) folgt erst nach dem Loslassen.

    /** Eine Transaktion durch S0 bis S8 (Manifest SONDE-015 §5.11.4). Ein
        read-only gehaltener Stand nimmt keine an (Ausgang `fehler`, Grund
        `schreibgeschuetzt`): beim Speichern gingen ohnehin die Originalbytes
        zurueck. */
    nakama::transaktion::Ergebnis fuehreTransaktionAus (const nakama::transaktion::Auftrag& auftrag);

    /** Eine neue, in dieser Instanz eindeutige Transaktions-ID. */
    nakama::transaktion::Tid neueTid() noexcept;

    bool setzePreview (const nakama::parameter::DspSatz& satz, juce::String& grund);
    void beendePreview();

    nakama::parameter::DspSatz bestaetigterZustand() const;
    nakama::parameter::DspSatz wirksamerZustand() const;
    std::uint64_t stateRevision() const;
    juce::String  stateHashText() const;
    std::uint64_t automationEpoche() const;
    bool          previewAktiv() const;

    /** Die lokale Nutzlast von `state_report.dsp` (R13); der Sender ist 4b. */
    bool dspBericht (nakama::transaktion::DspBericht& aus, juce::String& grund) const;

    /** Die transiente Hoermatrix (R10): kein Parameter, nichts im Zustand. */
    void setzeHoermatrix (nakama::dsp::Hoermatrix h) noexcept { dspKern->setzeHoermatrix (h); }
    nakama::dsp::Hoermatrix gewuenschteHoermatrix() const noexcept { return dspKern->gewuenschteHoermatrix(); }

    /** §53.5, letzter Aufzaehlungspunkt: "die beiden neuen Bundles haben eine
        feste Produktklasse, bleiben aber bis gueltigem State neutral."

        Der Automat sagt, welche Haelfte gerade gilt. Audio haengt hier an
        nichts davon - `processBlock` fragt die Klassifikation nie -, aber der
        Brokerstart tut es: `darfBrokerStarten()` ist fuer eine Sonde IMMER
        falsch, weil sie nie `main` wird. Das ist die Sonden-Haelfte von
        "Scanner/Probe/Render spawnen nie Broker".

        Beide lesen unter `zustandSchloss` - demselben Schloss, unter dem
        `setStateInformation` den Automaten schreibt. Ohne das waeren Schreiber
        und Leser desselben Zustands verschieden verriegelt, und genau hier
        haengt SONDE-010 den Brokerstart an: dann liest ein anderer Thread,
        waehrend der Host einen Stand nachreicht. Das Main-Bundle beantwortet
        dieselbe Frage schon so (`PluginProcessor.cpp:531-540`, `bindungMutex`).
        T2-Befund 23.08. */
    nakama::state::Klassifikation klassifikation() const noexcept
    {
        const juce::ScopedLock l (zustandSchloss);
        return lebenslauf.klassifikation();
    }
    bool darfBrokerStarten() const noexcept
    {
        const juce::ScopedLock l (zustandSchloss);
        return lebenslauf.darfBrokerStarten();
    }

#if defined (NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
    nakama::ipc::ControlHello v3HelloFuerTest() const { return v3Hello(); }
    nakama::ipc::ControlStatus v3StatusFuerTest() const { return v3Status(); }

    /** SONDE-015 4a: der DSP-Kern fuer Pool- und Zaehlerbeobachtung (B7, A16). */
    nakama::dsp::DspKern& dspKernFuerTest() noexcept { return *dspKern; }

    /** Ein Takt des Control-Workers, synchron - damit ein Bein nicht auf den
        Worker wartet. Derselbe Code wie im Worker. */
    void kontrollTaktFuerTest() { dspKontrollTakt(); }
    nakama::ipc::ControlClient::Snapshot controlV3FuerTest() const
    {
        return controlV3.snapshot();
    }
    bool v3ProduktstatusVerdrahtetFuerTest() const
    {
        return controlV3.statusProviderGesetzt();
    }
    nakama::ipc::TelemetryClient::Snapshot telemetryV3FuerTest() const
    {
        return telemetryV3.snapshot();
    }
    bool letzterProducerFrameFuerTest (nakama::analyse::FeatureFrame&) const;
    std::uint64_t producerPublikationenFuerTest() const noexcept
    {
        return producerPublikationen.load();
    }
    std::uint64_t evidenzSnapshotsFuerTest() const noexcept
    {
        return evidenzSnapshots.load();
    }
    std::uint64_t evidenzNichtGesendetFuerTest() const noexcept
    {
        return evidenzNichtGesendet.load();
    }
    std::uint64_t evidenzKadenzReduktionenFuerTest() const noexcept
    {
        return evidenzKadenzReduktionen.load();
    }
    double evidenzIntervallFuerTest() const
    {
        std::lock_guard<std::mutex> l (analyseSchloss);
        return merkmale.evidenzIntervallJetzt();
    }
    std::uint64_t analyseDropsUeberlaufFuerTest() const noexcept
    {
        return analyseQueue.dropsUeberlauf();
    }
    std::uint64_t analyseDropsOversizeFuerTest() const noexcept
    {
        return analyseQueue.dropsOversize();
    }
    /** NAK-283 F09 (M-36): Bloecke, deren Zeit die Queue ohne Audio verbucht
        hat (`verwirfOhneAudio`), und alle verlorenen Analyseframes. */
    std::uint64_t analyseDropsOhneAudioFuerTest() const noexcept
    {
        return analyseQueue.dropsOhneAudio();
    }
    std::uint64_t analyseVerloreneFramesFuerTest() const noexcept
    {
        return analyseQueue.verloreneFrames();
    }
    /** NAK-283 M-35 bis M-37, M-41: haelt den Analyseworker an, solange `f`
        laeuft, und reicht `f` die Queue als EINZIGEM Consumenten. Der Worker
        liest die Queue nur unter `analyseSchloss`, der Audiopfad nimmt es
        nie - `f` darf deshalb `processBlock` rufen, aber weder
        `prepareToPlay` noch einen Testzugang, der dasselbe Schloss nimmt. */
    template <typename Funktion>
    void mitAngehaltenerAnalyseFuerTest (Funktion&& f)
    {
        std::lock_guard<std::mutex> l (analyseSchloss);
        f (analyseQueue);
    }
    bool hostCallbackAufMessageThreadFuerTest() const noexcept
    {
        return hostCallbackAufMessageThread.load();
    }

    /** Die Grenzzaehler der FeatureEngine (NAK-181 Nacharbeit 1, EP-07/NR-07).

        🔑 N-31 sagt nicht nur einen Stempel zu, sondern auch, dass KEIN
        Fenster die Grenze ueberbrueckt und der Grund `lokaleLuecke` heisst.
        Ohne diese zwei Zahlen misst ein Bein nur, dass irgendein Frame da ist
        — und bleibt gruen, wenn der alte volle Reset zurueckkehrt. Gelesen
        unter derselben Sperre wie `evidenzIntervallFuerTest` daneben. */
    std::uint64_t getrennteFensterFuerTest() const
    {
        std::lock_guard<std::mutex> l (analyseSchloss);
        return merkmale.getrennteFenster();
    }
    std::uint64_t grenzenMitGrundFuerTest (nakama::analyse::Grenzgrund g) const
    {
        std::lock_guard<std::mutex> l (analyseSchloss);
        return merkmale.grenzenMitGrund (g);
    }
    /** Wie oft die Quarantaene einen Block ohne Nachfolgebeweis verworfen hat.

        Der Zaehler, an dem "ein echter Queue-Drop ist passiert" haengt: ohne
        ihn koennte ein Bein nicht von "der Drop kam gar nicht zustande"
        unterscheiden. */
    std::uint64_t kontinuitaetsbruecheFuerTest() const noexcept
    {
        return quarantaene.kontinuitaetsbrueche();
    }
#endif

private:
    void workerLauf();
    void producerStandLeeren() noexcept;
    nakama::ipc::ControlHello v3Hello() const;
    nakama::ipc::ControlStatus v3Status() const;
    nakama::ipc::TelemetryHello v3TelemetryHello() const;

    // ── SONDE-015 Etappe 4a ──────────────────────────────────────────────
    void parameterValueChanged (int parameterIndex, float neuNormiert) override;
    void parameterGestureChanged (int parameterIndex, bool beginnt) override;

    /** Ein Takt des Control-Workers: ACKs ernten, Hostereignisse in den
        AutomationOverlay, Ruhegrenze, wirksamen Zustand publizieren. */
    void dspKontrollTakt();

    /** Setzt die Hostparameter auf `werte` - mit Herkunftstag, damit der
        eigene Listener den Abgleich nicht als Automation liest. */
    void hostParameterAbgleichen (const nakama::parameter::Satz& werte);

    /** Hostwert (normiert) -> Vertragszelle. Gleicht er dem bestaetigten Wert
        in Hostgenauigkeit oder ist er nicht endlich, ist es GENAU der bestaetigte Wert. Unter Schloss. */
    nakama::parameter::Zelle zelleAusHost (int index, float normiert) const;

    void gestusAbschliessen();
    bool committedRuhtImPassthrough() const noexcept;
    /** Unter dem Zustandsschloss: `zustand` mit der Dsp-Haelfte aus dem Kern. */
    nakama::state::Zustand gehaltenerStand() const;

    nakama::state::Zustand zustand;
    nakama::state::Lebenslauf lebenslauf { kProduktklasse };
    juce::CriticalSection zustandSchloss;   ///< nur Nachrichten-/Hostthread, nie processBlock

    /** Hostname ist Message-Thread-Zustand, wird aber vom Control-Thread
        gelesen. Er bleibt strikt getrennt vom persistenten User-Label. */
    mutable juce::CriticalSection hostKontextSchloss;
    bool hostBusNameGemeldet = false;
    std::string hostBusName;

    // Audiothread -> Analyseworker: derselbe vorallokierte Ganzblockweg wie
    // Gen. Der Audiothread beruehrt weder Engine noch TelemetryClient.
    using Strom = nakama::echtzeit::GenStrom;
    using AnalyseQueue = nakama::echtzeit::StampedAudioQueue<Strom>;
    AnalyseQueue analyseQueue;
    nakama::echtzeit::Blockquarantaene<Strom> quarantaene;
    eqcop::AnalyseEngine analyseEngine;
    nakama::analyse::FeatureEngine merkmale;
    mutable std::mutex analyseSchloss;
    std::mutex workerWarteSchloss;
    std::condition_variable workerWarte;
    std::atomic<bool> workerLaeuft { false };
    std::thread worker;
    nakama::analyse::FeatureFrame letzterProducerFrame;
    bool letzterProducerFrameVorhanden = false;
    std::atomic<std::uint64_t> producerPublikationen { 0 };

    // ── SONDE-013 M-05: Evidenzpfad ──────────────────────────────────────
    //
    // Alles hier gehoert dem WORKER und laeuft unter `analyseSchloss`; nur
    // die drei Zaehler sind atomar, weil Tests sie von aussen lesen. Der
    // Audiothread beruehrt nichts davon.
    /// Ab welcher P1-Tiefe (Hauptqueue plus Wiederholpuffer) gar kein
    /// Evidenzsnapshot mehr entsteht.
    ///
    /// Klein mit Absicht: ein Snapshot ist rund 10 KiB, und die Queue ist in
    /// EINTRAEGEN gedeckelt. Vier ausstehende sind 40 KiB je Sonde — bei 16
    /// Sonden 640 KiB statt 40 MiB. Die Zahl ist eine Ressourcengrenze, keine
    /// Messaussage; sie steht deshalb nicht in `kFeatureMetricsVersion`.
    static constexpr std::size_t kEvidenzP1Schwelle = 4;

    void evidenzSnapshotSenden (const nakama::analyse::FeatureFrame& frame);
    /// Laufgebundener Verlustzaehler der Engine beim letzten Snapshot. Die
    /// DIFFERENZ ist der Verlust dieses Fensters — der Engine-Zaehler bleibt
    /// laufgebunden, weil B5 ihn so misst.
    std::uint64_t letzteEreignisverluste { 0 };
    std::atomic<std::uint64_t> evidenzSnapshots { 0 };
    std::atomic<std::uint64_t> evidenzNichtGesendet { 0 };
    std::atomic<std::uint64_t> evidenzKadenzReduktionen { 0 };

    // Der gepatchte Wrapper liefert diesen Stand unmittelbar vor demselben
    // Audioblock. Deshalb kein Atomic und kein Lock.
    struct BrueckeStand
    {
        nakama::echtzeit::Stempel stempel;
        bool frisch = false;
    } brueckeStand;

#if defined (NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
    std::atomic<bool> hostCallbackAufMessageThread { false };
#endif

    const std::string v3LogonSid;
    const std::string v3PipeName;
    const std::string v3RuntimeNonce;
    std::atomic<double> v3Samplerate { 0.0 };
    std::atomic<int> v3BlockSize { 0 };
    std::atomic<int> v3Channels { 0 };
    nakama::ipc::ControlClient controlV3;
    nakama::ipc::TelemetryClient telemetryV3;

    // ── SONDE-015 Etappe 4a: Parameter, DSP-Kern, Transaktionskern ─────────
    //
    // Die 112 Host-Parameter leben in der APVTS; gespeichert wird aber nicht
    // die APVTS, sondern der bestaetigte Zustand des Transaktionskerns (Kinder
    // `Parameters` und `Dsp`). DSP-Kern und Transaktionskern liegen auf dem
    // Heap, weil Konsolenbeine Prozessoren im Rahmen anlegen und der
    // MSVC-Standardstack 1 MiB fasst (NAK-175).
    juce::AudioProcessorValueTreeState parameterBaum;
    std::array<juce::RangedAudioParameter*, (size_t) nakama::parameter::kHostParameter> hostParameter {};
    std::unique_ptr<nakama::dsp::DspKern>                    dspKern;
    std::unique_ptr<nakama::transaktion::DspKernAusfuehrung> dspAusfuehrung;
    std::unique_ptr<nakama::transaktion::Transaktionskern>   transaktion;

    // Hostereignisse. Der Listener laeuft auch im Audiothread (VST3-Wrapper,
    // `processParameterChanges`) und schreibt deshalb nur Atomics.
    std::array<std::atomic<float>, (size_t) nakama::parameter::kHostParameter>         hostWert {};
    std::array<std::atomic<std::uint32_t>, (size_t) nakama::parameter::kHostParameter> hostEreignis {};
    std::array<std::uint32_t, (size_t) nakama::parameter::kHostParameter>              hostEreignisGesehen {};   ///< unter Zustandsschloss
    std::atomic<bool>          hostEreignisOffen { false };
    std::atomic<std::uint64_t> verarbeiteteSamples { 0 };   ///< Audiothread zaehlt, Worker liest (Ruhegrenze)
    std::uint64_t samplesBeiLetzterAutomation = 0;          ///< unter Zustandsschloss
    bool          publikationOffen = false;                 ///< unter Zustandsschloss

    // Gesten der eigenen Oberflaeche (Message-Thread), unter Zustandsschloss.
    std::array<bool, (size_t) nakama::parameter::kHostParameter> gesteOffen {};
    std::array<bool, (size_t) nakama::parameter::kHostParameter> gesteBeteiligt {};

    // Der Analysetap `post_committed` als float, vorallokiert in prepareToPlay.
    std::vector<float> analyseL, analyseR;

    std::uint64_t              tidHoch = 0;
    std::atomic<std::uint64_t> tidZaehler { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SondeProcessor)
};

} // namespace nakama::sonde
