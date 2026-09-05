// SONDE-010 — die gemeinsame Pipe-Mechanik der beiden v3-Clients.
//
// Warum eigen und nicht `src/PipeClient` wiederverwendet: PipeClient ist der
// v2-PRODUKTPFAD, die v3-Clients sind ein zweiter, danebenliegender Weg. Beide
// behalten ihre Framing-/I/O-Mechanik; seit NAK-123 teilen alle drei nur den
// Serverbeweis am jeweils konkret geoeffneten Handle:
//
//   * `CreateFileW` mit SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION —
//     NAK-49/ACCESS-001 wird nicht wiederholt: ohne diese Flags darf ein
//     Pipe-Namensbesetzer den DAW-Token staerker impersonieren als noetig;
//   * overlapped I/O mit ABSOLUTER Frist ueber den ganzen Schreibvorgang,
//     nicht je Teiloperation (wissen/engineering 2026-08-27: "nur Timeout pro
//     Teil-Lesevorgang ⇒ Drip-Feed haelt Kopf oder Body beliebig offen");
//   * Schliessen und `CancelIoEx` unter DEMSELBEN Mutex, damit kein
//     Check-then-use auf einem wiederverwendeten HANDLE entsteht;
//   * ein Zeitlimit beim Lesen ist KEIN Fehler, sondern ein eigener Ausgang —
//     sonst waere jede stille Sekunde ein Verbindungsabbruch.
//
// JUCE-frei wie der uebrige `core/`-Baum.
#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <initializer_list>
#include <mutex>
#include <string>
#include <vector>

namespace nakama::ipc
{

/// Backoff wie im v2-Client (EqCopilotIds.h): 500 ms, verdoppelt bis 8000 ms.
inline constexpr int kBackoffStartMs = 500;
inline constexpr int kBackoffMaxMs   = 8000;

/// Absolute Frist je Schreibvorgang und je Bootstrap-Antwort.
inline constexpr int kIoFristMs = 5000;

/// Ergebnis der Serverpruefung am KONKRETEN, noch unveroeffentlichten
/// `CreateFileW`-Handle.
///
/// Drei Ausgaenge, drei Bedeutungen (NAK-134/R1) — sie duerfen nie wieder
/// zusammenfallen:
///
///   * `nichtDa` — der Name existiert nicht. NUR dieser Wert darf den
///     Broker-Startpfad oeffnen.
///   * `belegtNichtErreicht` — der Name existiert, alle Instanzen sind belegt
///     und der Server wurde NIE erreicht (`ERROR_PIPE_BUSY` nach der
///     erschoepften Warteschleife). Das ist ein LIVENESSfehler: normaler
///     Backoff, kein Parken, kein Startpfad, keine Serverpruefung.
///   * `belegtAberUnverifiziert` — ein SICHERHEITSfehler. Entweder wurde ein
///     Handle geoeffnet und der Identitaetsbeweis nicht bestanden, oder der
///     Oeffnungsfehler deutet auf einen fremden Besitzer des Namens
///     (`ERROR_ACCESS_DENIED` und jeder unbekannte Fehler). Hier gilt die
///     NAK-123-Zusage unveraendert: kein wiederholtes Anklopfen.
///
/// Warum das eigene Werte sein muessen: bis NAK-134 trug
/// `belegtAberUnverifiziert` beide letzten Bedeutungen. Nach einem
/// Brokerneustart mit vielen Sonden verlor ein Teil der Clients das Rennen um
/// die freien Pipeinstanzen, bekam `ERROR_PIPE_BUSY` — und wurde als
/// Sicherheitsfall dauerhaft geparkt. Sie kamen bis zum Neuladen der Instanz
/// nie zurueck (Gate-Lauf G3, `docs/beweise/NAK-134.md`).
enum class ServerPruefStatus
{
    nichtGeprueft,
    nichtDa,
    verifiziert,
    belegtNichtErreicht,
    belegtAberUnverifiziert,
};

enum class ServerPruefFehler
{
    keiner,
    pipeFehlt,
    /// NAK-134/R1: alle Instanzen belegt, Server nie erreicht. Gehoert immer
    /// zu `ServerPruefStatus::belegtNichtErreicht`.
    pipeBelegt,
    pipeOeffnen,
    erwartungUngueltig,
    serverPidNichtErmittelbar,
    serverPidFalsch,
    serverprozessNichtOeffnen,
    serverTokenNichtOeffnen,
    pluginTokenNichtOeffnen,
    serverTokenUserNichtLesbar,
    pluginTokenUserNichtLesbar,
    serverSidUngueltig,
    pluginSidUngueltig,
    serverSidFalsch,
    prozessbildNichtErmittelbar,
    prozessbildNichtOeffnen,
    erwarteteDateiNichtOeffnen,
    dateiidentitaetFalsch,
    hashFalsch,
    signaturFehltOderUngueltig,
    signerFalsch,
};

/// Manifestgebundene Erwartung fuer einen Brokerprozess. `erwarteterPid == 0`
/// bedeutet im Produkt: PID am Pipehandle ermitteln, aber nicht vorab pinnen.
/// Selbstgehostete Tests setzen ihn auf die eigene PID und bleiben dadurch
/// ebenfalls fail-closed, ohne eine Ausnahme im Produktpfad zu brauchen.
struct ServerErwartung
{
    std::wstring absoluterBrokerPfad;
    std::string sha256;
    std::string authenticodeThumbprint;
    std::uint32_t erwarteterPid = 0;

    /// Ausschliesslich deterministische Fehlerinjektion in den benannten
    /// security_vectors-Tests. Produktziele lassen den Wert `keiner`.
    ServerPruefFehler testFehler = ServerPruefFehler::keiner;
    /// Optionale Testbarriere unmittelbar vor der abschliessenden Hash-/Signer-
    /// Entscheidung. Sie misst auch im Erfolgsfall, dass vor der vollstaendigen
    /// Verifikation kein Byte fliesst. Beide Zeiger muessen gemeinsam gesetzt
    /// sein und bis zum Ende des Connectversuchs leben; Produktwerte sind null.
    std::atomic<bool>* testVorFehlerErreicht = nullptr;
    std::atomic<bool>* testFehlerFreigeben = nullptr;
};

struct ServerPruefBericht
{
    ServerPruefStatus status = ServerPruefStatus::nichtGeprueft;
    ServerPruefFehler fehler = ServerPruefFehler::keiner;
    std::uint32_t serverPid = 0;
    std::uint32_t win32Fehler = 0;

    bool ok() const noexcept { return status == ServerPruefStatus::verifiziert; }
};

/// Baut fuer selbstgehostete Testserver eine VOR dem Connect feststehende
/// Erwartung aus einer Datei. Die Datei wird gehasht; ein leerer Rueckgabepfad
/// ist damit kein permissiver Modus, sondern fuehrt spaeter fail-closed.
ServerErwartung serverErwartungFuerTestdatei (const std::wstring& absoluterPfad,
                                              std::uint32_t erwarteterPid = 0);
ServerErwartung serverErwartungFuerEigenprozessTest();

/// Gemeinsamer Serverbeweis fuer v3-Control, v3-Telemetry und den v2-Client.
/// Der Aufrufer darf vor einem erfolgreichen Bericht weder den Handle
/// veroeffentlichen noch ein einziges Byte senden.
ServerPruefBericht namedPipeServerAuthentisieren (void* pipeHandle,
                                                  const ServerErwartung& erwartung);
const char* serverPruefFehlerName (ServerPruefFehler fehler) noexcept;

/// Reine Existenzprobe fuer eine lokale Named Pipe. Sie oeffnet keine
/// Verbindung und verbraucht deshalb keinen Server-Slot. `ERROR_PIPE_BUSY`
/// und `ERROR_SEM_TIMEOUT` bedeuten: Broker vorhanden, nur gerade belegt.
///
/// NAK-180 R6: sie wartet mit einer EIGENEN, kleinen Frist (50 ms), nie mit
/// der des Servers. Die alte Fassung uebergab `0` — das ist
/// `NMPWAIT_USE_DEFAULT_WAIT`, also die Frist des Pipe-BESITZERS, und dieser
/// Kopf las sie faelschlich als "nicht warten". Ein fremder lokaler Prozess
/// konnte den Aufruf damit beliebig lange halten und ueber das Startmutex den
/// Message-Thread des Hosts blockieren (Entwurf §48.4).
bool namedPipeErreichbar (const std::string& pipeName);

enum class LeseAusgang
{
    daten,      ///< `gelesen` Bytes liegen vor.
    zeitlimit,  ///< nichts angekommen — normal, kein Fehler.
    ende,       ///< Peer hat sauber geschlossen.
    fehler,     ///< `fehler` traegt den Grund.
};

/// Besitzt genau eine offene Pipe-Verbindung und ihre I/O-Fristen.
class IpcVerbindung
{
public:
    using Frist = std::chrono::steady_clock::time_point;

    IpcVerbindung() = default;
    ~IpcVerbindung();

    IpcVerbindung (const IpcVerbindung&) = delete;
    IpcVerbindung& operator= (const IpcVerbindung&) = delete;

    /// Verbindet zu `pipeName` und authentisiert den Server am tatsaechlich
    /// geoeffneten Handle. `false` ⇒ `bericht` und `fehler` tragen den Grund.
    bool oeffnen (const std::string& pipeName, const ServerErwartung& erwartung,
                  ServerPruefBericht& bericht, std::string& fehler);

    /// Schliesst und bricht laufende I/O ab. Idempotent.
    void schliessen();

    /// Beginnt eine neue Verbindungsgeneration: schliesst eine noch offene
    /// Verbindung UND loest das Abbruchsignal. Vor JEDEM `oeffnen()` zu rufen.
    ///
    /// NAK-134 Nacharbeit Runde 1, Defekt 1. Diese beiden Zeilen standen bis
    /// dahin am Anfang von `oeffnen()`. Damit gehoerte das Abbruchsignal dem
    /// OEFFNUNGSAUFRUF statt der Generation: ein `ioAbbrechen()`, das nach der
    /// aeusseren Generationspruefung des Clients und vor dem Eintritt in
    /// `oeffnen()` eintraf, wurde geloescht. Auf einer belegten Pipe liefen
    /// danach alle 20 `WaitNamedPipeW(200 ms)`-Runden — gemessen 4.009 ms
    /// statt der R5-Frist (`roh/NAK-134-nacharbeit-1-abbruch-vor-fix.txt`).
    ///
    /// Warum das Loesen beim AUFRUFER richtig liegt und in `oeffnen()` nicht:
    /// `stop()` und `reconnect()` erhoehen die Verbindungsgeneration VOR
    /// `ioAbbrechen()`. Der Aufrufer loest das Signal deshalb hier und liest
    /// die Generation unmittelbar danach ERNEUT — jeder Abbruch, den dieses
    /// Loesen verschlucken koennte, hat die Generation vorher schon erhoeht
    /// und faellt in genau diese zweite Pruefung. Ein Loesen INNERHALB von
    /// `oeffnen()` hat diese zweite Pruefung nicht und kann sie auch nicht
    /// haben: `IpcVerbindung` kennt die Generation des Aufrufers nicht.
    void neueGenerationBeginnen();

    bool offen() const noexcept;

    /// Schreibt ALLE Bytes oder scheitert. `frist` ist absolut.
    bool schreibenGenau (const std::uint8_t* daten, std::size_t laenge,
                         Frist frist, std::string& fehler);

    /// Ein Lesevorgang bis `frist`.
    LeseAusgang lesen (std::uint8_t* ziel, std::size_t laenge, std::size_t& gelesen,
                       Frist frist, std::string& fehler);

    /// Von aussen (Stop/Reconnect): bricht laufende I/O ab, ohne das Handle zu
    /// schliessen — der besitzende Thread raeumt selbst auf.
    void ioAbbrechen();

    static Frist fristIn (int millisekunden)
    {
        return std::chrono::steady_clock::now() + std::chrono::milliseconds (millisekunden);
    }

private:
    mutable std::mutex handleMutex;
    void* handle = nullptr;   // HANDLE
    void* ereignis = nullptr; // HANDLE des Overlapped-Events
    std::atomic<bool> abbruch { false };
};

/// Baut ein `u32`-laengenpraefigiertes Bootstrap-Hello (die EINZIGE Nachricht
/// ohne v3-Kopf, §53.9). Liefert false, wenn der Text ueber 16 KiB liegt —
/// dann waere es kein Hello mehr.
bool bootstrapRahmen (const std::string& json, std::vector<std::uint8_t>& ziel);

/// Ein Feld aus einem flachen JSON-Objekt — MIT seinem Typ.
///
/// Die alte Fassung gab nur `(name, roher Text)` zurueck. Damit war
/// `"broker_version":null` von `"broker_version":"null"` nicht mehr zu
/// unterscheiden: der Leser HAT die Typinformation, warf sie aber am
/// Rueckgabewert weg, und der Client nahm ein typfalsches welcome an
/// (T2-Befund 3 vom 2026-08-29). Der Vertrag
/// (`eq-copilot/schemas/v3/eq-ipc-v3.schema.json`) unterscheidet sehr wohl:
/// `broker_version` ist ein String, `protocol` eine Zahl.
struct JsonFeld
{
    std::string name;
    /// String OHNE Anfuehrungszeichen; Zahl, `true`, `false`, `null` als Text.
    std::string wert;
    bool istString = false;
};

/// Sehr kleiner, STRENGER Leser fuer ein FLACHES JSON-Objekt.
///
/// Er kann absichtlich fast nichts: keine Verschachtelung, keine Arrays, keine
/// Escapes. Genau das ist die Eigenschaft, die hier gebraucht wird — der
/// Client liest nur `welcome` und `reject`, beide flach und beide aus unserem
/// eigenen Vertrag. Alles andere wird ABGELEHNT statt geraten; ein Leser, der
/// raet, waere die groessere Angriffsflaeche als einer, der nein sagt.
bool flachesJsonObjekt (const std::string& text, std::vector<JsonFeld>& felder);

/// Feld als JSON-STRING. `false`, wenn es fehlt ODER kein String ist.
bool jsonText (const std::vector<JsonFeld>& felder, const std::string& name,
               std::string& wert);

/// Feld als Zahl oder Literal (`true`/`false`/`null`), roh als Text. `false`,
/// wenn es fehlt ODER ein String ist.
bool jsonLiteral (const std::vector<JsonFeld>& felder, const std::string& name,
                  std::string& wert);

/// Traegt das Objekt GENAU diese Felder — keines zu wenig, keines zu viel?
///
/// Das ist die C++-Haelfte von `additionalProperties:false`. Ohne sie ist eine
/// Pruefung der Pflichtfelder per Konstruktion blind fuer ein Zusatzfeld: sie
/// liest, was sie kennt, und uebersieht, was sie nicht kennt.
bool feldmengeGenau (const std::vector<JsonFeld>& felder,
                     std::initializer_list<const char*> erwartet);

} // namespace nakama::ipc
