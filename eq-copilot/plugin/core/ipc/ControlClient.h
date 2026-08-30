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
#include <memory>
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

/// Halten die Audiofelder den v3-Vertrag (`audio_lage` im Schema)? NaN, ±Inf
/// und alles ausserhalb der Grenzen wird verriegelt, BEVOR irgendetwas davon
/// in eine Ganzzahl gewandelt oder gesendet wird.
bool audioGueltig (double samplerate, int blockSize, int channels) noexcept;

/// Haelt ein `welcome` den VOLLSTAENDIGEN Vertrag aus
/// `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`: exakte Feldmenge
/// (`additionalProperties:false`), jeder Typ, jede Laenge?
///
/// BEIDE Clients rufen dieselbe Funktion. Zwei getrennte Fassungen waren die
/// Ursache dafuer, dass die Strenge auseinanderlief (T2-Befund 3 vom
/// 2026-08-29): der Telemetriepfad prueft die Kopplungswerte zusaetzlich, der
/// VERTRAG ist aber derselbe.
bool welcomeHaeltVertrag (const std::vector<JsonFeld>& felder,
                          std::string& linkId, std::string& challenge,
                          std::string& brokerEpoch, std::string& brokerVersion);

/// Dasselbe fuer `reject`: `required [type, code, reason]`,
/// `additionalProperties:false`, `reason` hoechstens 500 Zeichen.
bool rejectHaeltVertrag (const std::vector<JsonFeld>& felder, std::string& grund);

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
        /// Frames einer auf DIESER Verbindung unzulaessigen Familie. Control
        /// traegt ausschliesslich P0/P1 (§33.1); ein P2-Frame darf hier nicht
        /// beim Aufrufer landen, sondern beendet die Verbindung.
        std::uint64_t familieAbweisungen = 0;
        /// Wie oft die Nachrichtenratengrenze (§33.1) die Verbindung beendet
        /// hat.
        std::uint64_t rateAbweisungen = 0;
        /// An der TUER abgewiesene Nachrichten: groesser, als ein v3-Frame
        /// tragen kann. Sie kommen gar nicht erst in die Queue — eine
        /// eingereihte Nachricht, die nie auf den Draht passt, wuerde die
        /// Verbindung endlos schliessen und neu aufbauen.
        std::uint64_t zuGross = 0;
        /// Wie oft `stop()` den Clientthread ABGELOEST hat, statt ihn zu
        /// joinen, weil ein Callback laenger als `kStopFristMs` stand
        /// (Matrix `B-CC-12`). `stop()` kehrt trotzdem zurueck; die Zahl ist
        /// der sichtbare Preis dafuer.
        std::uint64_t stopFristUeberschritten = 0;
    };

    /// `beiAntwort` wird auf dem Client-Thread gerufen, nie im Audiothread.
    ControlClient (std::function<ControlHello()> helloProvider,
                   std::string pipeName,
                   std::function<void (const std::string&)> beiAntwort = {});
    ~ControlClient();

    ControlClient (const ControlClient&) = delete;
    ControlClient& operator= (const ControlClient&) = delete;

    void start();

    /// Kehrt IMMER zurueck (Matrix `B-CC-10`…`B-CC-12`, Regel 6):
    ///
    ///   * aus einem Callback dieses Clients heraus OHNE Self-Join — der
    ///     Thread endet nach Rueckkehr des Callbacks von selbst, den `join`
    ///     holt der naechste `stop()` oder der Destruktor von aussen nach;
    ///   * von aussen mit der Frist `kStopFristMs` auf einen laufenden
    ///     Callback; laeuft sie ab, wird der Thread ABGELOEST statt gejoint
    ///     und `Snapshot::stopFristUeberschritten` waechst;
    ///   * nach der Rueckkehr wird kein Callback mehr gerufen.
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
    /// Alles, was der Clientthread anfasst, liegt GETEILT hinter einem
    /// `shared_ptr` — nicht im Client selbst.
    ///
    /// Der Grund ist Regel 6: `stop()` darf einen Thread, der in einem
    /// blockierenden Callback steht, nach `kStopFristMs` ABLOESEN statt ihn zu
    /// joinen. Ein abgeloester Thread, der danach noch Member des Clients
    /// laese, waere undefiniertes Verhalten, sobald das Objekt zerstoert wird.
    /// Mit der geteilten Laufzeit haelt der Thread seine eigene Referenz und
    /// beruehrt den Client nie — dasselbe Muster wie `Senkenruf` und
    /// `join_mit_frist` im Rust-Listener.
    struct Laufzeit;
    std::shared_ptr<Laufzeit> k;

    std::mutex   lebenslaufMutex;
    std::thread  thread;
};

} // namespace nakama::ipc
