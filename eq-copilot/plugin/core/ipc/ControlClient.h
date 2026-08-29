// SONDE-010 — v3-Control-Client (P0/P1), Entwurf §33.1, §53.9.
//
// Eine Instanz oeffnet ZWEI logisch gekoppelte Verbindungen: diese hier traegt
// Steuerung und Zustand (P0/P1), der `TelemetryClient` die Featureframes (P2).
// Der Control-Client wird ZUERST geoeffnet; erst sein `welcome` liefert
// `link_id` und `challenge`, mit denen sich die Telemetrieverbindung koppelt.
//
// Was dieser Client NICHT tut:
//
//   * Er startet NIE einen Prozess (§48.3 Connect-without-spawn). Ist der
//     Broker nicht da, wartet er mit Backoff — mehr nicht. Scanner, Render und
//     Sonden duerfen nichts starten, und dieser Code kennt den Unterschied
//     gar nicht erst.
//   * Er laeuft NIE im Audiothread. Alle I/O liegt auf seinem eigenen Thread;
//     `sendeP0`/`sendeP1` reihen nur ein.
//   * Er ersetzt in diesem Ticket NICHT den v2-Produktpfad. Gen und Probeeq
//     sprechen weiterhin `src/PipeClient`; die Produktverdrahtung folgt mit
//     Coordinator und Landkarte (SONDE-011/012).
#pragma once

#include "IpcQueues.h"
#include "IpcVerbindung.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

namespace nakama::ipc
{

/// Effektive Steueradresse (§32.1). Alle hex32-Felder sind 32 Kleinbuchstaben-
/// Hexziffern; ein Feld ausserhalb dieser Form wird gar nicht erst gesendet.
struct Adresse
{
    std::string logonSid;
    std::string projectBindingId;
    std::string sessionEpoch;
    std::string instanceId;
    std::string runtimeNonce;
};

bool istHex32 (const std::string& s) noexcept;

/// `adresse`-Objekt des v3-Vertrags als JSON. Auch der TelemetryClient sendet
/// sie — beide muessen DIESELBE Adresse tragen, sonst faellt die Kopplung.
std::string adresseAlsJson (const Adresse& a);

/// Haelt die Adresse den Vertrag ein? Eine ungueltige wird gar nicht erst
/// gesendet: der Broker wuerde sie ablehnen, und ein Client, der wissentlich
/// Muell sendet, verschleiert nur die Ursache.
bool adresseGueltig (const Adresse& a);

struct ControlHello
{
    Adresse      adresse;
    std::string  pluginVersion = "0.3.0";
    /// "main" | "passive_probe" | "active_probe" | "legacy"
    std::string  pluginKind    = "active_probe";
    double       samplerate    = 48000.0;
    int          blockSize     = 512;
    int          channels      = 2;
    bool         hostAngeben   = false;
    std::uint32_t hostPid      = 0;
};

class ControlClient
{
public:
    enum class Status { getrennt, verbindet, verbunden };

    struct Snapshot
    {
        Status        status = Status::getrennt;
        std::string   linkId, challenge, brokerEpoch, brokerVersion, letzterFehler;
        int           verbindungsVersuche = 0;
        std::uint64_t p0Gesendet = 0;
        std::uint64_t p1Gesendet = 0;
        std::uint64_t empfangen = 0;
        std::uint64_t p0Ueberlaeufe = 0;
        std::uint64_t p1Wiederholungen = 0;
        std::uint64_t envelopeAbweisungen = 0;
    };

    /// `beiAntwort` wird auf dem Client-Thread gerufen, nie im Audiothread.
    ControlClient (std::function<ControlHello()> helloProvider,
                   std::string pipeName,
                   std::function<void (const std::string&)> beiAntwort = {});
    ~ControlClient();

    ControlClient (const ControlClient&) = delete;
    ControlClient& operator= (const ControlClient&) = delete;

    void start();
    void stop();
    /// Trennt die aktuelle Verbindung; die naechste Runde sendet ein frisches
    /// Hello. Kehrt sofort zurueck.
    void reconnect();

    /// P0 einreihen. `false` = Ueberlauf der 64er-Queue ⇒ die Verbindung wird
    /// geschlossen (§53.9 "nichts verwerfen; Verbindung schliessen").
    bool sendeP0 (const std::string& json);

    /// P1 einreihen. Leerer `schluessel` = Ereignis; ein nicht leerer
    /// Schluessel koalesziert Snapshots desselben Objekts.
    P1Ergebnis sendeP1 (const std::string& schluessel, const std::string& json);

    Snapshot snapshot() const;

    /// Kopplungsdaten fuer den TelemetryClient. `false`, solange kein
    /// `welcome` angekommen ist — ein Telemetry-Connect ohne diese Werte wird
    /// vom Broker geschlossen, und dieser Client erfindet sie nicht.
    bool kopplung (std::string& linkId, std::string& challenge) const;

private:
    void threadLauf();
    bool eineVerbindung (std::uint64_t generation);
    bool sollAbbrechen (std::uint64_t generation) const noexcept;

    std::function<ControlHello()> helloProvider;
    std::function<void (const std::string&)> beiAntwort;
    std::string pipeName;

    IpcVerbindung verbindung;

    std::mutex   lebenslaufMutex;
    std::thread  thread;
    std::atomic<bool> laeuft { false };
    std::atomic<std::uint64_t> verbindungsGeneration { 0 };
    std::mutex   wartemutex;
    std::condition_variable warte;

    mutable std::mutex zustandMutex;
    Snapshot zustand;

    std::mutex sendeMutex;
    P0Warteschlange p0;
    P1Warteschlange p1;
    /// Monoton wachsender Zaehler der P0-Ueberlaeufe. Die laufende Verbindung
    /// merkt sich seinen Stand beim Verbinden und schliesst, sobald er waechst
    /// (§53.9 "nichts verwerfen; Verbindung schliessen"). Ein Ueberlauf, der
    /// VOR der Verbindung passiert ist, schliesst dagegen nichts — es gibt
    /// nichts zu schliessen, und der Aufrufer hat sein `false` bereits.
    std::atomic<std::uint64_t> p0UeberlaufZaehler { 0 };
};

} // namespace nakama::ipc
