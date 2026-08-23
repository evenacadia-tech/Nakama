// Named-Pipe-Client des Sammler-VST3 (Plan §9.2).
//
// Läuft in einem eigenen std::thread, nie im Audiothread. Ablauf pro Runde:
// verbinden → hello → welcome lesen → 1-Hz-Heartbeats mit Live-Statistik →
// bei Fehler trennen und mit exponentiellem Backoff neu versuchen. stop()
// bricht auch blockierende Pipe-I/O ab (CancelSynchronousIo) — der
// Plugin-Destruktor darf nie am Broker hängen.
//
// Die Klasse ist bewusst hostfrei (nur juce_core + Win32), damit die Konsolen-
// Probe EqCopPipeProbe denselben Code gegen den Broker fährt wie das Plugin.
#pragma once

#include <juce_core/juce_core.h>
#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>

namespace eqcop
{

struct HelloInfo
{
    juce::String sensorId;      // persistente UUID aus dem Plugin-State
    juce::String instanceNonce; // flüchtig pro Plugin-Instanz (v2, Plan §8.2)
    juce::String role;          // "sensor" | "hub" | "pre" | "post"
    juce::String label;         // User-Label — untrusted data
    juce::String pairId;        // leer ⇒ null im JSON
    double       samplerate = 0.0;
    int          blockSize  = 0;
    int          channels   = 0;
};

struct StatsSnapshot
{
    float    rmsL = 0.0f, rmsR = 0.0f;
    juce::uint64 framesDropped = 0;
    bool     nanSeen = false;
    bool     hasTransport = false;
    bool     transportPlaying = false;
    // SONDE-008: eigenes Gueltigkeitsbit fuer die Projektzeit. Sie hing bisher
    // am Transportbit mit - zwei verschiedene Aussagen des Hosts an einem
    // Schalter (§53.7 fuehrt sie als getrennte ValidValue). Ohne Beweis geht
    // `project_time_samples` als null hinaus, nie als 0 (NAK-24).
    bool     projectTimeValid = false;
    juce::int64 projectTimeSamples = 0;
};

// Kompakter Messstand für den v2-Heartbeat (measurement in eq-ipc.schema.json).
// Ungültige Werte werden über die *Gueltig-Flags zu JSON-null — nie zu 0.
struct MessKompakt
{
    bool         vorhanden = false;   // false ⇒ kein measurement-Block senden
    juce::String zustand;             // "keine_daten" | "sammelt" | "messbereit"
    // Hör-Markierung hörbar (Konzept v2 §5): nachgelagerte Sensoren messen
    // gerade gefärbtes Signal — der Broker soll fremde Aggregate pausieren.
    bool         hoermarkierung = false;
    juce::String metricsVersion;
    double       aktivS = 0.0, gesamtS = 0.0;
    bool lufsGueltig = false;      double lufsI = 0.0;
    bool lufsShortGueltig = false; double lufsS = 0.0;
    bool truePeakGueltig = false;  double truePeakDb = 0.0;
    bool crestGueltig = false;     double crestDb = 0.0;
    bool spektralGueltig = false;  double centroidHz = 0.0, lowFrac = 0.0;
    bool stereoGueltig = false;    double width = 0.0, corr = 0.0;
    // LTAS-Komposit auf dem 1/24-Okt-Raster; NaN ⇒ null im JSON.
    std::vector<double> ltasKompositDb;
    // Projektzeit-Fenster der akkumulierten Messung (Plan §5.7); spruenge
    // zählt Loop-/Seek-Diskontinuitäten.
    bool         fensterGueltig = false;
    juce::int64  fensterVon = 0, fensterBis = 0;
    juce::uint32 fensterSpruenge = 0;
};

class PipeClient
{
public:
    enum class Status { getrennt, verbindet, verbunden };

    struct Snapshot
    {
        Status       status = Status::getrennt;
        juce::String brokerVersion;
        juce::String sessionToken;
        juce::String letzterFehler;
        int          verbindungsVersuche = 0;
        juce::int64  heartbeatsGesendet  = 0;
        // Vom Broker ausgehandelte Protokollversion (welcome); 0 = getrennt.
        int          protokollVersion = 0;
        // Letztes heartbeat_ack meldet: eine WEITERE lebende Verbindung nutzt
        // dieselbe Sensor-ID (FL-Duplikation). Der Editor zeigt den Hinweis;
        // die Entscheidung »neue Kennung« fällt sichtbar beim User (Plan §8.4).
        bool         konflikt = false;
        // Roh-JSON des letzten heartbeat_ack — nur Diagnose (Pipe-Probe).
        juce::String letztesAck;
    };

    // helloProvider wird bei jedem Verbindungsaufbau frisch gefragt (Rolle/SR
    // können sich geändert haben); statsProvider und messProvider vor jedem
    // Heartbeat. messProvider darf blockierungsarm sein — er läuft im
    // Pipe-Thread, nie im Audiothread.
    PipeClient (std::function<HelloInfo()> helloProvider,
                std::function<StatsSnapshot()> statsProvider,
                std::function<MessKompakt()> messProvider = {},
                const juce::String& pipeName = {});
    ~PipeClient();

    void start();
    void stop();

    // Trennt die aktuelle Verbindung, damit ein frisches hello (neue Rolle,
    // neues Label) gesendet wird. Kehrt sofort zurück.
    void reconnect();

    Snapshot snapshot() const;

private:
    void threadLauf();
    bool eineVerbindung();                       // true = geordnet beendet (stop)
    bool sende (void* handle, const juce::String& json);
    bool empfange (void* handle, juce::String& jsonOut);

    std::function<HelloInfo()>     helloProvider;
    std::function<StatsSnapshot()> statsProvider;
    std::function<MessKompakt()>   messProvider;
    juce::String                   pipeName;

    std::thread             thread;
    std::atomic<bool>       laeuft { false };
    std::atomic<bool>       neuVerbinden { false };
    std::mutex              wartemutex;
    std::condition_variable warte;

    mutable std::mutex      zustandMutex;
    Snapshot                zustand;

    std::atomic<void*>      aktivesHandle { nullptr };  // für CancelSynchronousIo
    std::atomic<void*>      threadHandle  { nullptr };
};

} // namespace eqcop
