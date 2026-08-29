// SONDE-010 — die gemeinsame Pipe-Mechanik der beiden v3-Clients.
//
// Warum eigen und nicht `src/PipeClient` wiederverwendet: PipeClient ist der
// v2-PRODUKTPFAD. Er bleibt in diesem Ticket unangetastet — der heutige
// Heartbeat ist weiterhin das, was Gen und Probeeq wirklich sprechen. Die
// v3-Clients sind ein zweiter, danebenliegender Weg; sie teilen die LEHREN
// von PipeClient, nicht seinen Code:
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

    /// Verbindet zu `pipeName`. `false` ⇒ `fehler` traegt den Grund.
    bool oeffnen (const std::string& pipeName, std::string& fehler);

    /// Schliesst und bricht laufende I/O ab. Idempotent.
    void schliessen();

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
