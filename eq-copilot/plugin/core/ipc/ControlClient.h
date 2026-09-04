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
#include "WireEnvelope.h"

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

/// Bildet die gespeicherte `instance_id` auf das hex32-Feld der v3-Adresse
/// ab (SONDE-011/NAK-40). Gueltiges hex32 bleibt bytegleich; jeder andere
/// nichtleere Wert wird ohne Normalisierung domain-separiert und
/// laengengebunden mit SHA-256/128 abgebildet. Ein leerer Wert bleibt leer,
/// damit ausschliesslich der bestehende State-Lader den frischen UUID-Pfad
/// waehlt. Nicht fuer den Audiothread: allokiert und hasht.
std::string instanceAdresseAusState (const std::string& instanceId);

/** Dieselbe Adresse, wie sie auf die Leitung geht (NAK-40).

    Der Control-Bootstrap bildete den Wirealias frueher an EINER Stelle
    (`ControlClient.cpp`), und jeder weitere Sender, der `v3Hello()` direkt
    benutzte, schickte statt dessen die rohe persistente Instance-ID. Fuer
    eine hex32-ID faellt das nie auf — sie ist ihr eigener Alias —, fuer eine
    unterstuetzte Legacy-ID verschwanden Marker und Evidenz dagegen
    kommentarlos, weil `adresseGueltig` sie zurueckwies.

    Diese Funktion IST der eine Weg. Sie ist idempotent: eine bereits
    aliasierte Adresse geht bytegleich durch. */
Adresse wireAdresseAusState (Adresse adresse);

/// Rueckweg der nicht umkehrbaren Abbildung: der Zielclient berechnet den
/// Alias aus seiner lokalen Original-ID neu. Ein unbekannter Wirewert ist
/// fail-closed; es gibt keine Dekodierung und keinen Alias im Host-State.
bool instanceAliasZielPasst (const std::string& lokaleInstanceId,
                             const std::string& wireInstanceId);

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

/** Optionaler, vollstaendiger `heartbeat.runtime`-Block (SONDE-012 E-M01).
    `gemeldet == false` laesst den ganzen Block weg; einzelne Hostfelder sind
    davon getrennt optional. Der Mixerindex bleibt leer, solange der lokale
    Hostwrapper ihn nicht beobachtet. */
struct ControlRuntime
{
    bool          gemeldet = false;
    std::string   messpunkt;
    std::string   betrieb;
    bool          labelGemeldet = false;
    std::string   label;
    bool          hostBusNameGemeldet = false;
    std::string   hostBusName;
    bool          hostMixerIndexGemeldet = false;
    std::uint64_t hostMixerIndex = 0;
};

/// Laufender, vom Produkt verantworteter Zustand fuer `heartbeat` und
/// `state_report`. Der Provider wird ausschliesslich auf dem Control-Thread
/// aufgerufen. Ein leerer `stateHash` bedeutet vertragsgemaess JSON-null; der
/// Client erfindet weder einen DSP-Stand noch einen Aufnahmezustand.
struct ControlStatus
{
    std::uint32_t dspSchemaVersion = 1;
    std::uint64_t stateRevision = 0;
    std::string   stateHash;
    bool          recordStateValid = false;
    bool          recording = false;
    std::uint64_t framesDropped = 0;
    std::uint64_t parseErrors = 0;
    std::uint64_t queueOverflows = 0;
    /// SONDE-013 M-39: sticky `intervention_state_unknown`. Gesetzt nach
    /// Ringueberlauf, Control-Disconnect oder Sequenzluecke; es reist im
    /// Heartbeat, damit ein VERLORENES letztes Ereignis den Empfaenger
    /// trotzdem erreicht — die Sequenzluecke allein reicht dafuer nicht.
    bool          interventionStateUnknown = false;
    ControlRuntime runtime;
};

/** Der produktive Heartbeat-Writer, zugleich direkter C++-Beweispunkt fuer
    die optionale Runtime-Praesenz. Ein unbekannter Messpunkt/Betrieb laesst
    den Block fail-closed weg; es entsteht nie ein halber Runtime-Block. */
std::string heartbeatAlsJson (const Adresse&, std::uint64_t sequence,
                              const ControlStatus&);

struct GelesenesCommandAck
{
    std::string commandId;
    bool erfolgreich = false;
};

/// Strikter handgeschriebener Leser fuer das gemeinsame `command_ack`-
/// Muster. `true` bedeutet ein vollstaendiges finales ACK; `erfolgreich`
/// unterscheidet angewandt/idempotent von allen fail-closed Endzustaenden.
bool commandAckHaeltVertrag (const std::string&, GelesenesCommandAck&);

class ControlClient
{
public:
    enum class Status { getrennt, verbindet, verbunden };

    struct Snapshot
    {
        Status        status = Status::getrennt;
        std::string   linkId, challenge, brokerEpoch, brokerVersion, letzterFehler;
        /// Nur ein frischer `CreateFileW`-Fehler ERROR_FILE_NOT_FOUND setzt
        /// dieses Bit. Adress-/Audiofehler, Rejects, kaputte Welcomes und eine
        /// belegte Pipe duerfen den Lifecycle nie als "Broker fehlt" oeffnen.
        bool          brokerPipeFehlt = false;
        ServerPruefStatus serverPruefstatus = ServerPruefStatus::nichtGeprueft;
        ServerPruefFehler serverPrueffehler = ServerPruefFehler::keiner;
        std::uint32_t serverPid = 0;
        /// Vollstaendige Serverpruefungen an tatsaechlich geoeffneten
        /// Pipehandles. Ein Reconnect muss diesen Zaehler erneut erhoehen.
        std::uint64_t serverPruefungen = 0;
        int           verbindungsVersuche = 0;
        std::uint64_t p0Gesendet = 0;
        std::uint64_t p1Gesendet = 0;
        std::uint64_t empfangen = 0;
        std::uint64_t p0Ueberlaeufe = 0;
        std::uint64_t p1Wiederholungen = 0;
        /// Aktuelle Fuellstaende der P1-Wege: Hauptqueue und Wiederholpuffer.
        ///
        /// 🔑 Sie stehen hier, seit SONDE-013 den `evidence_snapshot` sendet.
        /// Die P1-Queue ist in EINTRAEGEN gedeckelt (128 + 128), nicht in
        /// Bytes — bei `state_report`-Nachrichten von rund 300 Byte ist das
        /// ein Puffer von 80 KiB, bei einem Evidenzsnapshot von rund 10 KiB
        /// sind es 2,5 MiB JE SONDE. Gemessen am 04.09.2026: 16 Sonden liessen
        /// den Working Set des Clients im G3-Soak um 23,4 MiB wachsen, Budget
        /// 16,8 MiB. Ein Sender, der seine eigene Last kennen soll, braucht
        /// den Fuellstand — und nicht erst das Urteil, wenn die Nachricht
        /// schon eingereiht ist.
        std::size_t p1Tiefe = 0;
        std::size_t p1WiederholTiefe = 0;
        /// Persistenzpflichtige logische Auftraege oberhalb der P0-Queue.
        /// Ein erfolgreicher Draht-Write veraendert diese Zahl nicht.
        std::uint64_t inFlight = 0;
        std::uint64_t inFlightErfolg = 0;
        std::uint64_t inFlightEndgueltigOhneErfolg = 0;
        std::uint64_t inFlightWiederholungen = 0;
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

    /// `beiAntwort`, `statusProvider` und der positive `beiLinkStatus` werden
    /// auf dem Client-Thread gerufen. Der negative Linkstatus kommt bei einem
    /// expliziten `reconnect()`/`stop()` synchron auf dessen Aufruferthread,
    /// damit die Subscription vor der Rueckkehr bereits ungueltig ist; beim
    /// unerwarteten Ende kommt er vom Client-Thread. Keiner dieser Aufrufe
    /// stammt aus dem Audiothread. Ist kein Statusprovider gesetzt, bleibt der
    /// Client ein manuell gespeister Transport wie vor Phase B.
    ControlClient (std::function<ControlHello()> helloProvider,
                   std::string pipeName,
                   std::function<void (const std::string&)> beiAntwort = {},
                   std::function<ControlStatus()> statusProvider = {},
                   std::function<void (bool verbunden)> beiLinkStatus = {},
                   std::function<void (const std::string&, std::uint8_t schemaMinor)>
                       beiVersionierterAntwort = {},
                   ServerErwartung serverErwartung = serverErwartungFuerEigenprozessTest());
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

    /// Persistenzpflichtiger P0-Auftrag. Der JSON-Text muss genau eine
    /// gueltige `command_id` tragen. Sein Queueplatz wird nach dem Wire-Write
    /// frei; logisch erledigt ist er erst durch ein schemafestes
    /// `command_ack`. Bei Verbindungsverlust wird derselbe Text und damit
    /// dieselbe ID erneut eingereiht.
    bool sendePersistenzP0 (const std::string& json);

    /// P1 einreihen. Leerer `schluessel` = Ereignis; ein nicht leerer
    /// Schluessel koalesziert Snapshots desselben Objekts.
    P1Ergebnis sendeP1 (const std::string& schluessel, const std::string& json);

    Snapshot snapshot() const;

    /// Diagnose-/Testaussage ueber die produktive 1-Hz-Quelle. Der Provider
    /// ist nach dem Konstruktor unveraenderlich; die Abfrage startet nichts.
    bool statusProviderGesetzt() const noexcept;

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
