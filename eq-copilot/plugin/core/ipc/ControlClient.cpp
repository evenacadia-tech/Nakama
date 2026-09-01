// K1 (S8/SONDE-007a): der Kompilier-Riegel steht am Anfang UND am Ende jeder
// Kern-Uebersetzungseinheit. Diese hier sieht zwar gar kein JUCE, aber die
// Regel gilt fuer die MENGE, nicht fuer den Einzelfall - eine Ausnahme waere
// die Stelle, an der die naechste Quelle sie stillschweigend erbt.
#include "NakamaKernRiegel.h"

#include "ControlClient.h"
#include "NakamaKanon.h"
#include "WireEnvelope.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cctype>

namespace nakama::ipc
{
namespace
{
/// Wie lange der Lesevorgang je Runde wartet, bevor die Sendequeues wieder
/// drankommen. Kurz genug, dass ein P0-Befehl nicht hinter Stille wartet.
constexpr int kLeseTaktMs = 20;

/// Der Vertrag beschreibt Heartbeats mit 1 Hz. Ein erster wird unmittelbar
/// nach dem welcome eingereiht, damit die 2,5-s-Stalegrenze nicht von einem
/// zufaelligen Phasenversatz abhaengt.
constexpr int kHeartbeatTaktMs = 1000;
constexpr std::uint64_t kJsonSafeModulus = 9007199254740992ULL; // 2^53

/// Frist, die `stop()` einem LAUFENDEN Callback noch laesst (Matrix
/// `B-CC-12`). Derselbe Wert wie `SENKE_FRIST` im Rust-Listener: beide Seiten
/// geben fremdem Code dieselbe Gnadenfrist. Lang genug fuer einen normalen
/// Callback (Mikro- bis Millisekunden), kurz genug, dass das Schliessen eines
/// Plugins im Host nicht spuerbar haengt.
constexpr int kStopFristMs = 2000;

enum class CommandAckArt
{
    keinAck,
    angewandt,
    abgelehnt,
    konflikt,
    abgelaufen,
    idempotentWiederholt
};

bool istHex64 (const std::string& s) noexcept
{
    if (s.size() != 64)
        return false;
    for (char c : s)
        if (! ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')))
            return false;
    return true;
}

bool nichtnegativeJsonGanzzahl (const std::string& s) noexcept
{
    if (s == "0")
        return true;
    if (s.empty() || s.front() < '1' || s.front() > '9')
        return false;
    for (char c : s)
        if (c < '0' || c > '9')
            return false;
    return true;
}

bool fehlercodeHaeltVertrag (const std::string& code) noexcept
{
    return code == "protocol_mismatch" || code == "unknown_message"
        || code == "schema_violation" || code == "unauthorized"
        || code == "unknown_target" || code == "revision_conflict"
        || code == "capability_missing" || code == "record_state_unknown"
        || code == "recording_active" || code == "lease_expired"
        || code == "rate_limited" || code == "internal";
}

bool brokerPipeFehltNachOeffnen (const std::string& fehler) noexcept
{
    // `IpcVerbindung::oeffnen` bildet den Win32-Code verlustfrei in genau
    // diese interne Meldung ab. Nur ERROR_FILE_NOT_FOUND (2) bedeutet bei
    // einer Named Pipe, dass kein Broker horcht. ERROR_PIPE_BUSY (231),
    // Zugriff verweigert und jeder lokale Vertragsfehler sind ausdruecklich
    // keine Startberechtigung.
    return fehler == "Broker nicht erreichbar (Win32 2)";
}

// Liest einen JSON-String nur so weit, wie es fuer die eindeutige
// `command_id`-Suche noetig ist. Escapes werden uebersprungen, aber nicht
// interpretiert: weder ein Schluessel noch hex32 darf laut Vertrag ein Escape
// benoetigen. Die vollstaendige Nachricht prueft weiterhin der Schemaweg.
bool jsonStringToken (const std::string& text, std::size_t& position,
                      std::string& wert, bool& hatteEscape)
{
    while (position < text.size()
           && std::isspace (static_cast<unsigned char> (text[position])) != 0)
        ++position;
    if (position >= text.size() || text[position] != '"')
        return false;
    ++position;
    wert.clear();
    hatteEscape = false;
    while (position < text.size())
    {
        const char c = text[position++];
        if (c == '"')
            return true;
        if (c == '\\')
        {
            hatteEscape = true;
            if (position >= text.size())
                return false;
            ++position;
            continue;
        }
        if (static_cast<unsigned char> (c) < 0x20)
            return false;
        wert.push_back (c);
    }
    return false;
}

bool commandIdAusAuftrag (const std::string& text, std::string& commandId)
{
    std::size_t position = 0;
    unsigned gefunden = 0;
    while (position < text.size())
    {
        if (text[position] != '"')
        {
            ++position;
            continue;
        }

        std::string token;
        bool escape = false;
        if (! jsonStringToken (text, position, token, escape))
            return false;
        std::size_t nachToken = position;
        while (nachToken < text.size()
               && std::isspace (static_cast<unsigned char> (text[nachToken])) != 0)
            ++nachToken;
        if (escape || token != "command_id"
            || nachToken >= text.size() || text[nachToken] != ':')
            continue;

        position = nachToken + 1;
        std::string wert;
        bool wertEscape = false;
        if (! jsonStringToken (text, position, wert, wertEscape)
            || wertEscape || ! istHex32 (wert))
            return false;
        commandId = std::move (wert);
        ++gefunden;
        if (gefunden > 1)
            return false;
    }
    return gefunden == 1;
}

CommandAckArt commandAckLesen (const std::string& text, std::string& commandId)
{
    std::vector<JsonFeld> felder;
    std::string typ, ergebnis, revision;
    if (! flachesJsonObjekt (text, felder)
        || ! jsonText (felder, "type", typ) || typ != "command_ack"
        || ! jsonText (felder, "command_id", commandId) || ! istHex32 (commandId)
        || ! jsonText (felder, "ergebnis", ergebnis)
        || ! jsonLiteral (felder, "state_revision", revision)
        || ! nichtnegativeJsonGanzzahl (revision))
        return CommandAckArt::keinAck;

    // `event_uuid` ist intern. Auch ein ansonsten plausibles ACK darf das
    // In-Flight-Register nicht ueber eine erfundene Wireform freigeben.
    const JsonFeld* stateHash = nullptr;
    const JsonFeld* code = nullptr;
    for (const auto& feld : felder)
    {
        if (feld.name == "event_uuid")
            return CommandAckArt::keinAck;
        if (feld.name != "type" && feld.name != "command_id"
            && feld.name != "ergebnis" && feld.name != "state_revision"
            && feld.name != "state_hash" && feld.name != "code")
            return CommandAckArt::keinAck;
        if (feld.name == "state_hash")
            stateHash = &feld;
        else if (feld.name == "code")
            code = &feld;
    }

    CommandAckArt art = CommandAckArt::keinAck;
    if (ergebnis == "angewandt")                  art = CommandAckArt::angewandt;
    else if (ergebnis == "abgelehnt")             art = CommandAckArt::abgelehnt;
    else if (ergebnis == "konflikt")              art = CommandAckArt::konflikt;
    else if (ergebnis == "abgelaufen")            art = CommandAckArt::abgelaufen;
    else if (ergebnis == "idempotent_wiederholt") art = CommandAckArt::idempotentWiederholt;
    if (art == CommandAckArt::keinAck)
        return art;

    const bool erfolg = art == CommandAckArt::angewandt
                     || art == CommandAckArt::idempotentWiederholt;
    if (erfolg)
    {
        if (stateHash == nullptr || ! stateHash->istString
            || ! istHex64 (stateHash->wert))
            return CommandAckArt::keinAck;
    }
    else if (stateHash != nullptr
             && ! ((stateHash->istString && istHex64 (stateHash->wert))
                   || (! stateHash->istString && stateHash->wert == "null")))
    {
        return CommandAckArt::keinAck;
    }

    if (code != nullptr
        && (! code->istString || ! fehlercodeHaeltVertrag (code->wert)))
        return CommandAckArt::keinAck;
    return art;
}

std::string jsonString (const std::string& roh)
{
    // Der Client erzeugt nur Werte aus dem eigenen Vertrag (hex32, SID,
    // Versionsstring, Produktklasse). Alles, was ein Escape braeuchte, ist
    // hier ein Fehler und wird nicht kodiert, sondern verworfen — der
    // Aufrufer bekommt die Ablehnung beim Verbinden zu sehen.
    std::string aus = "\"";
    for (char c : roh)
        if (static_cast<unsigned char> (c) >= 0x20 && c != '"' && c != '\\')
            aus.push_back (c);
    aus.push_back ('"');
    return aus;
}

std::string jsonStringUntrusted (const std::string& roh)
{
    static constexpr char hex[] = "0123456789abcdef";
    std::string aus = "\"";
    aus.reserve (roh.size() + 2);
    for (const unsigned char c : roh)
    {
        switch (c)
        {
            case '"':  aus += "\\\""; break;
            case '\\': aus += "\\\\"; break;
            case '\b': aus += "\\b";  break;
            case '\f': aus += "\\f";  break;
            case '\n': aus += "\\n";  break;
            case '\r': aus += "\\r";  break;
            case '\t': aus += "\\t";  break;
            default:
                if (c < 0x20)
                {
                    aus += "\\u00";
                    aus.push_back (hex[c >> 4]);
                    aus.push_back (hex[c & 0x0f]);
                }
                else
                {
                    aus.push_back (static_cast<char> (c));
                }
                break;
        }
    }
    aus.push_back ('"');
    return aus;
}

std::uint64_t jsonSafe (std::uint64_t wert) noexcept
{
    return std::min (wert, kJsonSafeModulus - 1);
}

const char* boolJson (bool wert) noexcept { return wert ? "true" : "false"; }

std::string capabilitiesJson()
{
    // Maschinenlesbare Wahrheit aus host-capabilities-fl-v1.json: zwei in FL
    // gemessene Faehigkeiten, acht feste Fallbacks. Diese Funktion wird nur
    // auf dem Clientthread gerufen.
    return "{\"host_context_presence\":\"supported\","
           "\"project_time_samples\":\"supported\","
           "\"sample_accurate_automation\":\"unsupported\","
           "\"presentation_latency\":\"unsupported\","
           "\"aux_compare_pre\":\"unsupported\","
           "\"aux_priority_sidechain\":\"unsupported\","
           "\"contribution_aux\":\"unsupported\","
           "\"float64_processing\":\"unsupported\","
           "\"binary_telemetry\":\"unsupported\","
           "\"remote_control\":\"unsupported\"}";
}

std::string stateHashJson (const std::string& hash)
{
    // Ein kaputter lokaler Hash darf nicht als scheinbar gueltiger Stand auf
    // den Draht. null ist der schemafeste, fail-closed Wert.
    return istHex64 (hash) ? jsonString (hash) : "null";
}

std::string runtimeJson (const ControlRuntime& runtime)
{
    if (! runtime.gemeldet
        || (runtime.messpunkt != "insert" && runtime.messpunkt != "pre"
            && runtime.messpunkt != "post")
        || (runtime.betrieb != "active" && runtime.betrieb != "suspended"
            && runtime.betrieb != "offline"))
        return {};

    std::string aus = ",\"runtime\":{\"messpunkt\":"
                    + jsonString (runtime.messpunkt)
                    + ",\"betrieb\":" + jsonString (runtime.betrieb);
    // Ein ungueltiger optionaler Hostwert ist semantisch "nicht geliefert":
    // der vollstaendige Messpunkt-/Betriebsblock darf deshalb weiter reisen.
    if (runtime.hostBusNameGemeldet && ! runtime.hostBusName.empty())
        aus += ",\"host_bus_name\":" + jsonStringUntrusted (runtime.hostBusName);
    if (runtime.hostMixerIndexGemeldet && runtime.hostMixerIndex >= 1
        && runtime.hostMixerIndex <= 9007199254740991ULL)
        aus += ",\"host_mixer_index\":" + std::to_string (runtime.hostMixerIndex);
    aus += "}";
    return aus;
}

std::string stateReportJson (const Adresse& adresse, const ControlStatus& status)
{
    const auto schema = std::max<std::uint32_t> (1, status.dspSchemaVersion);
    return std::string ("{\"type\":\"state_report\",\"adresse\":")
         + adresseAlsJson (adresse)
         + ",\"dsp_schema_version\":" + std::to_string (schema)
         + ",\"state_revision\":" + std::to_string (jsonSafe (status.stateRevision))
         + ",\"state_hash\":" + stateHashJson (status.stateHash)
         + ",\"record_state\":{\"valid\":" + boolJson (status.recordStateValid)
         + ",\"recording\":" + boolJson (status.recording) + "}}";
}

/// Zahl aus einem Audiofeld. Sie wird NUR gerufen, nachdem `audioGueltig`
/// zugestimmt hat — der Riegel steht trotzdem hier: eine Wandlung nach
/// `long long` ist fuer NaN, ±Inf und alles ausserhalb des darstellbaren
/// Bereichs undefiniertes Verhalten, und undefiniertes Verhalten passiert VOR
/// jeder Pruefung, die danach kaeme (T2-Befund 9 vom 2026-08-29).
std::string zahl (double w)
{
    if (! std::isfinite (w))
        return "null";
    if (w >= -9.007199254740992e15 && w <= 9.007199254740992e15
        && w == static_cast<double> (static_cast<long long> (w)))
        return std::to_string (static_cast<long long> (w));
    return std::to_string (w);
}
} // namespace

std::string heartbeatAlsJson (const Adresse& adresse, std::uint64_t sequence,
                              const ControlStatus& status)
{
    return std::string ("{\"type\":\"heartbeat\",\"adresse\":")
         + adresseAlsJson (adresse)
         + ",\"sequence\":" + std::to_string (jsonSafe (sequence))
         + ",\"state_revision\":" + std::to_string (jsonSafe (status.stateRevision))
         + ",\"capabilities\":" + capabilitiesJson()
         + ",\"zaehler\":{\"frames_dropped\":"
         + std::to_string (jsonSafe (status.framesDropped))
         + ",\"parse_errors\":" + std::to_string (jsonSafe (status.parseErrors))
         + ",\"queue_overflows\":" + std::to_string (jsonSafe (status.queueOverflows))
         + "}" + runtimeJson (status.runtime) + "}";
}

/// Haelt ein `welcome` den VOLLSTAENDIGEN Vertrag aus
/// `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`?
///
/// Die alte Fassung las nur die Pflichtfelder und nahm jeden Wert als Text.
/// `"broker_version":null` kam damit als nichtleerer Text `null` durch, und
/// ein Zusatzfeld wie `"extra":1` fiel gar nicht auf, obwohl der Vertrag
/// `additionalProperties:false` sagt (T2-Befund 3 vom 2026-08-29). Geprueft
/// werden deshalb DREI Dinge: die Feldmenge exakt, jeder Typ, jede Laenge.
bool welcomeHaeltVertrag (const std::vector<JsonFeld>& felder,
                          std::string& linkId, std::string& challenge,
                          std::string& brokerEpoch, std::string& brokerVersion)
{
    if (! feldmengeGenau (felder, { "type", "protocol", "broker_version",
                                    "broker_epoch", "link_id", "challenge" }))
        return false;

    std::string typ, protokoll;
    if (! jsonText (felder, "type", typ) || typ != "welcome")
        return false;
    // `protocol` ist im Schema eine ZAHL mit dem Wert 3, kein String "3".
    if (! jsonLiteral (felder, "protocol", protokoll) || protokoll != "3")
        return false;
    if (! jsonText (felder, "broker_version", brokerVersion)
        || brokerVersion.empty() || brokerVersion.size() > 64)
        return false;
    return jsonText (felder, "link_id", linkId) && istHex32 (linkId)
        && jsonText (felder, "challenge", challenge) && istHex32 (challenge)
        && jsonText (felder, "broker_epoch", brokerEpoch) && istHex32 (brokerEpoch);
}

/// Dasselbe fuer `reject`: `required [type, code, reason]`,
/// `additionalProperties:false`, `reason` hoechstens 500 Zeichen.
bool rejectHaeltVertrag (const std::vector<JsonFeld>& felder, std::string& grund)
{
    if (! feldmengeGenau (felder, { "type", "code", "reason" }))
        return false;
    std::string code;
    return jsonText (felder, "code", code) && ! code.empty()
        && jsonText (felder, "reason", grund) && grund.size() <= 500;
}

bool audioGueltig (double samplerate, int blockSize, int channels) noexcept
{
    // Dieselben Grenzen wie im Broker (`bootstrap.rs`, audio ausserhalb des
    // Vertrags) und im Schema `audio_lage`. Ein Client, der wissentlich
    // Nicht-Zahlen sendet, verschleiert nur die Ursache — er verbindet gar
    // nicht erst (CLAUDE.md, NaN-Ehrlichkeit).
    return std::isfinite (samplerate) && samplerate > 0.0 && samplerate <= 768000.0
        && blockSize >= 1 && blockSize <= 65536
        && channels >= 0 && channels <= 64;
}

bool istHex32 (const std::string& s) noexcept
{
    if (s.size() != 32)
        return false;
    for (char c : s)
        if (! ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')))
            return false;
    return true;
}

std::string adresseAlsJson (const Adresse& a)
{
    return std::string ("{\"logon_sid\":") + jsonString (a.logonSid)
         + ",\"project_binding_id\":" + jsonString (a.projectBindingId)
         + ",\"session_epoch\":" + jsonString (a.sessionEpoch)
         + ",\"instance_id\":" + jsonString (a.instanceId)
         + ",\"runtime_nonce\":" + jsonString (a.runtimeNonce) + "}";
}

bool adresseGueltig (const Adresse& a)
{
    return ! a.logonSid.empty() && a.logonSid.size() <= 184
        && istHex32 (a.projectBindingId) && istHex32 (a.sessionEpoch)
        && istHex32 (a.instanceId) && istHex32 (a.runtimeNonce);
}

std::string instanceAdresseAusState (const std::string& instanceId)
{
    if (instanceId.empty() || istHex32 (instanceId))
        return instanceId;

    static constexpr char domain[] = "nakama.v3.instance-address.v1";
    std::vector<std::uint8_t> eingang;
    eingang.reserve (sizeof (domain) - 1 + 1 + 8 + instanceId.size());
    eingang.insert (eingang.end(), domain, domain + sizeof (domain) - 1);
    eingang.push_back (0);
    const auto laenge = static_cast<std::uint64_t> (instanceId.size());
    for (int verschiebung = 56; verschiebung >= 0; verschiebung -= 8)
        eingang.push_back (static_cast<std::uint8_t> (laenge >> verschiebung));
    eingang.insert (eingang.end(), instanceId.begin(), instanceId.end());

    const auto hash = nakama::kanon::sha256Hex (eingang.data(), eingang.size());
    return hash.substring (0, 32).toStdString();
}

bool instanceAliasZielPasst (const std::string& lokaleInstanceId,
                             const std::string& wireInstanceId)
{
    return ! lokaleInstanceId.empty()
        && istHex32 (wireInstanceId)
        && instanceAdresseAusState (lokaleInstanceId) == wireInstanceId;
}

//== Die geteilte Laufzeit ===================================================
//
// Alles, was der Clientthread anfasst. Der Client selbst haelt nur noch den
// `shared_ptr` darauf, seinen Thread und den Lebenslaufmutex — die Begruendung
// steht im Header (Regel 6, `B-CC-12`).
struct ControlClient::Laufzeit
{
    Laufzeit (std::function<ControlHello()> hp,
              std::string pn,
              std::function<void (const std::string&)> ba,
              std::function<ControlStatus()> sp)
        : helloProvider (std::move (hp)), beiAntwort (std::move (ba)),
          statusProvider (std::move (sp)), pipeName (std::move (pn)) {}

    void threadLauf (std::uint64_t meinLauf, std::shared_ptr<IpcVerbindung> meine);
    bool eineVerbindung (std::uint64_t generation, std::uint64_t meinLauf,
                         IpcVerbindung& verbindung);
    bool sollAbbrechen (std::uint64_t generation) const noexcept;
    /// Ist dieser Lauf abgeloest? Ein abgeloester Lauf schreibt KEINEN
    /// gemeinsamen Zustand mehr: sein `getrennt` waere sonst eine Aussage
    /// ueber einen Lauf, den es nicht mehr gibt — und der neue steht
    /// womoeglich gerade verbunden da (`B-CC-12`, NAK-104).
    bool abgeloest (std::uint64_t meinLauf) const noexcept
    { return lebenslauf.load() != meinLauf; }
    bool sendeP0 (const std::string& json);
    bool sendePersistenzP0 (const std::string& json);
    P1Ergebnis sendeP1 (const std::string& schluessel, const std::string& json);
    Snapshot snapshotIntern() const;
    bool kopplung (std::string& linkId, std::string& challenge) const;

    std::function<ControlHello()> helloProvider;
    std::function<void (const std::string&)> beiAntwort;
    std::function<ControlStatus()> statusProvider;
    std::string pipeName;
    std::atomic<std::uint64_t> heartbeatFolge { 0 };

    /// Die Verbindung des LAUFENDEN Laufs. Jeder `start()` legt eine eigene an.
    ///
    /// Eine gemeinsame Verbindung je Laufzeit reichte nicht: ein nach
    /// `kStopFristMs` ABGELOESTER Thread lebt weiter, bis sein Callback
    /// zurueckkommt. Startet der Client bis dahin erneut, endet der alte Lauf
    /// zwar (`lebenslauf`) — er erreicht aber noch das unbedingte
    /// `verbindung.schliessen()` am Ende von `eineVerbindung` und trennte damit
    /// die Pipe des NEUEN Laufs (NAK-104, Pruefbefund vom 2026-08-30). Und die
    /// Pruefung `lebenslauf` allein waere ohnehin nur ein Check-then-use: der
    /// alte Lauf haelt zwischen Pruefung und Zugriff Lese- und Schreibwege auf
    /// derselben Pipe.
    ///
    /// Mit einer eigenen Verbindung je Lauf beruehrt ein abgeloester Lauf
    /// ausschliesslich SEINE — der neue bleibt unangetastet. Der Zeiger wird
    /// nie null: `stop()` vor dem ersten `start()` findet eine geschlossene,
    /// gueltige Verbindung vor.
    mutable std::mutex verbindungMutex;
    /// NICHT direkt benutzen: `eineVerbindung` bekommt die Verbindung SEINES
    /// Laufs als Parameter. Der eigene Name macht sichtbar, dass beides nicht
    /// dasselbe ist.
    std::shared_ptr<IpcVerbindung> laufendeVerbindung = std::make_shared<IpcVerbindung>();

    std::shared_ptr<IpcVerbindung> aktuelleVerbindung() const
    {
        std::lock_guard<std::mutex> l (verbindungMutex);
        return laufendeVerbindung;
    }

    std::shared_ptr<IpcVerbindung> neueVerbindung()
    {
        auto frisch = std::make_shared<IpcVerbindung>();
        std::lock_guard<std::mutex> l (verbindungMutex);
        laufendeVerbindung = frisch;
        return frisch;
    }

    std::atomic<bool> laeuft { false };
    /// Der Thread hat `threadLauf()` verlassen. `stop()` wartet darauf, statt
    /// blind zu joinen — nur so kann es nach der Frist abloesen.
    std::atomic<bool> fertig { false };
    /// Wer ist der Clientthread? `stop()` aus einem Callback heraus liefe
    /// sonst in einen Self-Join (`B-CC-11`).
    std::atomic<std::thread::id> threadId {};
    /// Welcher Lauf ist das? Ein nach `kStopFristMs` ABGELOESTER Thread lebt
    /// weiter, bis sein Callback zurueckkommt. Startet der Client bis dahin
    /// erneut, saehe der alte Thread `laeuft == true` und liefe auf DERSELBEN
    /// Laufzeit weiter — zwei Threads auf einer Pipe. Jeder Lauf traegt
    /// deshalb seine Nummer und endet, sobald sie nicht mehr die aktuelle ist.
    std::atomic<std::uint64_t> lebenslauf { 0 };
    std::atomic<std::uint64_t> verbindungsGeneration { 0 };
    /// Anders als `verbindungsGeneration` (Abbruchsignal) waechst diese Zahl
    /// bei JEDEM erfolgreichen Pipe-Neuaufbau. Ein ungeplanter Broker-Kill
    /// aendert das Abbruchsignal nicht, muss In-Flight-Auftraege aber trotzdem
    /// erneut senden.
    std::atomic<std::uint64_t> wireGeneration { 0 };
    std::mutex   wartemutex;
    std::condition_variable warte;

    mutable std::mutex zustandMutex;
    Snapshot zustand;

    std::mutex sendeMutex;
    P0Warteschlange p0;
    P1Warteschlange p1;
    struct InFlightEintrag
    {
        std::string commandId;
        std::string json;
        std::uint64_t gesendetInGeneration = 0;
        bool inQueue = true;
    };
    std::vector<InFlightEintrag> inFlight;

    void inFlightNachReconnect (std::uint64_t generation);
    void inFlightNachWireWrite (const std::string& json, std::uint64_t generation);
    void inFlightAck (const std::string& json);
    /// Monoton wachsender Zaehler der P0-Ueberlaeufe. Die laufende Verbindung
    /// merkt sich seinen Stand beim Verbinden und schliesst, sobald er waechst
    /// (§53.9 "nichts verwerfen; Verbindung schliessen"). Ein Ueberlauf, der
    /// VOR der Verbindung passiert ist, schliesst dagegen nichts — es gibt
    /// nichts zu schliessen, und der Aufrufer hat sein `false` bereits.
    std::atomic<std::uint64_t> p0UeberlaufZaehler { 0 };
};

ControlClient::ControlClient (std::function<ControlHello()> helloProviderIn,
                              std::string pipeNameIn,
                              std::function<void (const std::string&)> beiAntwortIn,
                              std::function<ControlStatus()> statusProviderIn)
    : k (std::make_shared<Laufzeit> (std::move (helloProviderIn),
                                     std::move (pipeNameIn),
                                     std::move (beiAntwortIn),
                                     std::move (statusProviderIn)))
{
}

ControlClient::~ControlClient()
{
    stop();
}

void ControlClient::start()
{
    std::lock_guard<std::mutex> l (lebenslaufMutex);
    if (k->laeuft.load())
        return;
    k->laeuft.store (true);
    k->fertig.store (false);
    auto kern = k;
    const auto meinLauf = kern->lebenslauf.fetch_add (1) + 1;
    // Eigene Verbindung je Lauf (`B-CC-12`): der Thread bekommt sie direkt in
    // die Hand, statt sie spaeter aus der Laufzeit zu holen.
    auto meine = kern->neueVerbindung();
    thread = std::thread ([kern, meinLauf, meine] {
        kern->threadLauf (meinLauf, std::move (meine));
    });
}

void ControlClient::stop()
{
    // `B-CC-11`: aus einem Callback dieses Clients heraus wird nur markiert.
    // Der Thread endet nach Rueckkehr des Callbacks von selbst; sich hier
    // selbst zu joinen waere `std::system_error` und danach `std::terminate`.
    const bool ausDemClientthread = (std::this_thread::get_id() == k->threadId.load());

    // Genau die Verbindung, die beim Aufruf die aktuelle war. Ein spaeterer
    // `start()` legt eine neue an; diese hier zu schliessen darf jene nicht
    // treffen.
    auto verbindung = k->aktuelleVerbindung();

    k->laeuft.store (false);
    k->verbindungsGeneration.fetch_add (1);
    verbindung->ioAbbrechen();
    k->warte.notify_all();
    if (ausDemClientthread)
        return;

    std::lock_guard<std::mutex> l (lebenslaufMutex);
    if (! thread.joinable())
    {
        verbindung->schliessen();
        std::lock_guard<std::mutex> z (k->zustandMutex);
        k->zustand.status = Status::getrennt;
        return;
    }

    // `B-CC-12`: auf einen laufenden Callback wird hoechstens `kStopFristMs`
    // gewartet. Danach wird der Thread ABGELOEST — er haelt die Laufzeit ueber
    // seinen eigenen `shared_ptr` am Leben und beruehrt den Client nie.
    const auto bis = std::chrono::steady_clock::now()
                   + std::chrono::milliseconds (kStopFristMs);
    while (! k->fertig.load())
    {
        if (std::chrono::steady_clock::now() >= bis)
        {
            {
                std::lock_guard<std::mutex> z (k->zustandMutex);
                ++k->zustand.stopFristUeberschritten;
                k->zustand.status = Status::getrennt;
            }
            thread.detach();
            return;
        }
        std::this_thread::sleep_for (std::chrono::milliseconds (1));
    }
    thread.join();
    verbindung->schliessen();
    std::lock_guard<std::mutex> z (k->zustandMutex);
    k->zustand.status = Status::getrennt;
}

void ControlClient::reconnect()
{
    {
        std::lock_guard<std::mutex> l (k->zustandMutex);
        k->zustand.brokerPipeFehlt = false;
    }
    k->verbindungsGeneration.fetch_add (1);
    k->aktuelleVerbindung()->ioAbbrechen();
    k->warte.notify_all();
}

ControlClient::Snapshot ControlClient::snapshot() const { return k->snapshotIntern(); }

bool ControlClient::statusProviderGesetzt() const noexcept
{
    return static_cast<bool> (k->statusProvider);
}

bool ControlClient::sendeP0 (const std::string& json) { return k->sendeP0 (json); }

bool ControlClient::sendePersistenzP0 (const std::string& json)
{
    return k->sendePersistenzP0 (json);
}

P1Ergebnis ControlClient::sendeP1 (const std::string& schluessel, const std::string& json)
{
    return k->sendeP1 (schluessel, json);
}

bool ControlClient::kopplung (std::string& linkId, std::string& challenge) const
{
    return k->kopplung (linkId, challenge);
}

bool ControlClient::Laufzeit::sollAbbrechen (std::uint64_t generation) const noexcept
{
    return ! laeuft.load() || verbindungsGeneration.load() != generation;
}

bool ControlClient::Laufzeit::sendeP0 (const std::string& json)
{
    // An der TUER, nicht am Draht. Eine eingereihte Nachricht ueber der
    // Paketgrenze koennte NIE gesendet werden — sie bliebe dank der
    // Reservierung fuer immer vorn in der Queue und liesse jede neue
    // Verbindung an derselben Stelle scheitern.
    if (json.size() > kMaxPayloadBytes)
    {
        std::lock_guard<std::mutex> z (zustandMutex);
        ++zustand.zuGross;
        return false;
    }

    bool ueberlauf = false;
    {
        std::lock_guard<std::mutex> l (sendeMutex);
        if (! p0.einreihen (json))
        {
            ueberlauf = true;
            p0UeberlaufZaehler.fetch_add (1);
            std::lock_guard<std::mutex> z (zustandMutex);
            zustand.p0Ueberlaeufe = p0.ueberlauf();
        }
    }
    if (ueberlauf)
    {
        // Der Verbindungsthread haengt womoeglich in einem blockierten Write
        // (genau deshalb ist die Queue ja voll). Ohne diesen Abbruch merkte er
        // den Ueberlauf erst nach seiner Frist — und "Verbindung schliessen"
        // waere eine Zusage mit fuenf Sekunden Verspaetung.
        aktuelleVerbindung()->ioAbbrechen();
        return false;
    }
    return true;
}

bool ControlClient::Laufzeit::sendePersistenzP0 (const std::string& json)
{
    std::string commandId;
    if (json.size() > kMaxPayloadBytes || ! commandIdAusAuftrag (json, commandId))
    {
        std::lock_guard<std::mutex> z (zustandMutex);
        if (json.size() > kMaxPayloadBytes)
            ++zustand.zuGross;
        else
            zustand.letzterFehler =
                "Persistenzauftrag braucht genau eine gueltige command_id";
        return false;
    }

    bool ueberlauf = false;
    {
        std::lock_guard<std::mutex> l (sendeMutex);
        const auto bekannt = std::find_if (inFlight.begin(), inFlight.end(),
            [&] (const InFlightEintrag& e) { return e.commandId == commandId; });
        if (bekannt != inFlight.end())
        {
            // Derselbe logische Auftrag darf vom Aufrufer erneut angeboten
            // werden, aber dieselbe ID darf nie zwei verschiedene Inhalte
            // bedeuten. Die bereits gehaltene Fassung bleibt die Wahrheit.
            return bekannt->json == json;
        }
        inFlight.push_back (InFlightEintrag { commandId, json, 0, true });
        if (! p0.einreihen (json))
        {
            inFlight.pop_back();
            ueberlauf = true;
            p0UeberlaufZaehler.fetch_add (1);
        }

        std::lock_guard<std::mutex> z (zustandMutex);
        zustand.p0Ueberlaeufe = p0.ueberlauf();
        zustand.inFlight = static_cast<std::uint64_t> (inFlight.size());
    }
    if (ueberlauf)
    {
        aktuelleVerbindung()->ioAbbrechen();
        return false;
    }
    return true;
}

void ControlClient::Laufzeit::inFlightNachReconnect (std::uint64_t generation)
{
    std::uint64_t wiederholt = 0;
    std::uint64_t anzahl = 0;
    {
        std::lock_guard<std::mutex> l (sendeMutex);
        for (auto& e : inFlight)
        {
            if (e.inQueue || e.gesendetInGeneration == generation)
                continue;
            if (! p0.einreihen (e.json))
                break;
            e.inQueue = true;
            ++wiederholt;
        }
        anzahl = static_cast<std::uint64_t> (inFlight.size());
    }
    if (wiederholt != 0)
    {
        std::lock_guard<std::mutex> z (zustandMutex);
        zustand.inFlight = anzahl;
        zustand.inFlightWiederholungen += wiederholt;
    }
}

void ControlClient::Laufzeit::inFlightNachWireWrite (const std::string& json,
                                                      std::uint64_t generation)
{
    std::lock_guard<std::mutex> l (sendeMutex);
    const auto eintrag = std::find_if (inFlight.begin(), inFlight.end(),
        [&] (const InFlightEintrag& e) { return e.json == json && e.inQueue; });
    if (eintrag != inFlight.end())
    {
        eintrag->inQueue = false;
        eintrag->gesendetInGeneration = generation;
    }
}

void ControlClient::Laufzeit::inFlightAck (const std::string& json)
{
    std::string commandId;
    const auto art = commandAckLesen (json, commandId);
    if (art == CommandAckArt::keinAck)
        return;

    bool gefunden = false;
    std::uint64_t anzahl = 0;
    {
        std::lock_guard<std::mutex> l (sendeMutex);
        const auto eintrag = std::find_if (inFlight.begin(), inFlight.end(),
            [&] (const InFlightEintrag& e) { return e.commandId == commandId; });
        if (eintrag != inFlight.end())
        {
            inFlight.erase (eintrag);
            gefunden = true;
        }
        anzahl = static_cast<std::uint64_t> (inFlight.size());
    }
    if (! gefunden)
        return;

    std::lock_guard<std::mutex> z (zustandMutex);
    zustand.inFlight = anzahl;
    if (art == CommandAckArt::angewandt
        || art == CommandAckArt::idempotentWiederholt)
        ++zustand.inFlightErfolg;
    else
        ++zustand.inFlightEndgueltigOhneErfolg;
}

P1Ergebnis ControlClient::Laufzeit::sendeP1 (const std::string& schluessel,
                                            const std::string& json)
{
    if (json.size() > kMaxPayloadBytes)
    {
        std::lock_guard<std::mutex> z (zustandMutex);
        ++zustand.zuGross;
        return P1Ergebnis::zuGross;
    }

    std::lock_guard<std::mutex> l (sendeMutex);
    const auto e = p1.einreihen (schluessel, json);
    std::lock_guard<std::mutex> z (zustandMutex);
    zustand.p1Wiederholungen = p1.wiederholungen();
    return e;
}

ControlClient::Snapshot ControlClient::Laufzeit::snapshotIntern() const
{
    std::lock_guard<std::mutex> l (zustandMutex);
    return zustand;
}

bool ControlClient::Laufzeit::kopplung (std::string& linkId, std::string& challenge) const
{
    std::lock_guard<std::mutex> l (zustandMutex);
    if (zustand.status != Status::verbunden || zustand.linkId.empty())
        return false;
    linkId = zustand.linkId;
    challenge = zustand.challenge;
    return true;
}

void ControlClient::Laufzeit::threadLauf (std::uint64_t meinLauf,
                                         std::shared_ptr<IpcVerbindung> meine)
{
    threadId.store (std::this_thread::get_id());
    int backoffMs = kBackoffStartMs;
    while (laeuft.load() && lebenslauf.load() == meinLauf)
    {
        const auto generation = verbindungsGeneration.load();
        const bool stand = eineVerbindung (generation, meinLauf, *meine);
        if (! laeuft.load())
            break;
        if (stand)
            backoffMs = kBackoffStartMs;

        {
            std::unique_lock<std::mutex> l (wartemutex);
            warte.wait_for (l, std::chrono::milliseconds (backoffMs), [this, generation] {
                return ! laeuft.load() || verbindungsGeneration.load() != generation;
            });
        }
        if (verbindungsGeneration.load() != generation)
        {
            backoffMs = kBackoffStartMs;  // bewusster Reconnect wartet nicht laenger
            continue;
        }
        backoffMs = std::min (backoffMs * 2, kBackoffMaxMs);
    }
    // Erst JETZT ist der Thread fertig — `stop()` wartet auf genau dieses
    // Zeichen und darf danach joinen (`B-CC-10`).
    // Nur der AKTUELLE Lauf meldet sich fertig: ein abgeloester Vorgaenger
    // wuerde sonst den `join` des neuen Laufs freigeben.
    if (lebenslauf.load() == meinLauf)
        fertig.store (true);
}

bool ControlClient::Laufzeit::eineVerbindung (std::uint64_t generation,
                                             std::uint64_t meinLauf,
                                             IpcVerbindung& verbindung)
{
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::verbindet;
        zustand.brokerPipeFehlt = false;
        ++zustand.verbindungsVersuche;
        zustand.linkId.clear();
        zustand.challenge.clear();
        zustand.brokerEpoch.clear();
        zustand.brokerVersion.clear();
    }
    ControlHello hello = helloProvider ? helloProvider() : ControlHello();
    // NAK-40: Der Provider liefert die persistente Original-ID. Erst an der
    // v3-Grenze entsteht der Wirealias; in den Host-State fliesst er nie
    // zurueck. Leere bleibt leer und faellt weiter fail-closed — nur der
    // State-Lader darf dafuer den bestehenden Frisch-UUID-Pfad waehlen.
    hello.adresse.instanceId = instanceAdresseAusState (hello.adresse.instanceId);
    // Der Provider ist FREMDER Code und darf beliebig lange stehen. In dieser
    // Zeit kann `stop()` diesen Lauf abgeloest und ein neuer `start()` laengst
    // verbunden haben. Dann wird hier NICHT mehr geoeffnet: ein abgeloester
    // Lauf macht keine neue Pipe auf und meldet auch keinen Zustand mehr
    // (`B-CC-12`/`B-CC-16`, NAK-104).
    if (sollAbbrechen (generation))
        return false;
    if (! adresseGueltig (hello.adresse))
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::getrennt;
        zustand.letzterFehler = "Adresse haelt den v3-Vertrag nicht (hex32/SID)";
        return false;
    }
    // Audiofelder VOR der Serialisierung verriegeln — und vor dem Oeffnen der
    // Pipe: ein Hello mit NaN-Samplerate haette der Broker ohnehin abgelehnt,
    // aber die Wandlung dorthin waere schon vorher undefiniertes Verhalten
    // gewesen (T2-Befund 9 vom 2026-08-29).
    if (! audioGueltig (hello.samplerate, hello.blockSize, hello.channels))
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::getrennt;
        zustand.letzterFehler =
            "Audiolage haelt den v3-Vertrag nicht (samplerate/block_size/channels)";
        return false;
    }

    std::string fehler;
    if (! verbindung.oeffnen (pipeName, fehler))
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::getrennt;
        zustand.brokerPipeFehlt = brokerPipeFehltNachOeffnen (fehler);
        if (! sollAbbrechen (generation))
            zustand.letzterFehler = fehler;
        return false;
    }

    std::string helloJson =
        std::string ("{\"type\":\"hello\",\"connection_kind\":\"control\",\"protocol\":3,")
        + "\"plugin_version\":" + jsonString (hello.pluginVersion)
        + ",\"plugin_kind\":" + jsonString (hello.pluginKind)
        + ",\"adresse\":" + adresseAlsJson (hello.adresse);
    if (hello.hostAngeben)
        helloJson += ",\"host\":{\"pid\":" + std::to_string (hello.hostPid) + "}";
    helloJson += ",\"audio\":{\"samplerate\":" + zahl (hello.samplerate)
               + ",\"block_size\":" + std::to_string (hello.blockSize)
               + ",\"channels\":" + std::to_string (hello.channels) + "}}";

    std::vector<std::uint8_t> rahmen;
    if (! bootstrapRahmen (helloJson, rahmen)
        || ! verbindung.schreibenGenau (rahmen.data(), rahmen.size(),
                                        IpcVerbindung::fristIn (kIoFristMs), fehler))
    {
        verbindung.schliessen();
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::getrennt;
        if (! sollAbbrechen (generation))
            zustand.letzterFehler = fehler.empty() ? "Bootstrap-Hello zu gross" : fehler;
        return false;
    }

    // ── welcome lesen: bereits v3-gerahmt (§53.9) ─────────────────────────
    StromLeser leser;
    std::uint8_t puffer[4096];
    const auto welcomeFrist = IpcVerbindung::fristIn (kIoFristMs);
    std::string linkId, challenge, brokerEpoch, brokerVersion;
    bool welcomeKam = false;

    while (! sollAbbrechen (generation) && ! welcomeKam)
    {
        const auto e = leser.naechster();
        if (e.art == StromLeser::Art::verstoss)
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.letzterFehler = "welcome: Envelope abgelehnt";
            ++zustand.envelopeAbweisungen;
            break;
        }
        if (e.art == StromLeser::Art::frame)
        {
            if (e.kopf.familie != Familie::p0)
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler = "welcome kam nicht als P0";
                break;
            }
            const std::string text (reinterpret_cast<const char*> (e.payload), e.payloadLaenge);
            std::vector<JsonFeld> felder;
            std::string typ;
            if (! flachesJsonObjekt (text, felder) || ! jsonText (felder, "type", typ))
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler = "welcome: kein flaches JSON-Objekt";
                break;
            }
            if (typ == "reject")
            {
                std::string grund;
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler =
                    rejectHaeltVertrag (felder, grund) ? "Broker lehnt ab: " + grund
                                                       : "reject haelt den Vertrag nicht";
                break;
            }
            if (! welcomeHaeltVertrag (felder, linkId, challenge, brokerEpoch, brokerVersion))
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler = "unerwartete Antwort auf hello";
                break;
            }
            welcomeKam = true;
            break;
        }

        std::size_t gelesen = 0;
        const auto ausgang = verbindung.lesen (puffer, sizeof (puffer), gelesen,
                                               welcomeFrist, fehler);
        if (ausgang == LeseAusgang::daten && gelesen > 0)
        {
            leser.fuettern (puffer, gelesen);
            continue;
        }
        if (ausgang == LeseAusgang::zeitlimit)
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.letzterFehler = "kein welcome innerhalb der Frist";
        }
        else if (! sollAbbrechen (generation))
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.letzterFehler = fehler.empty() ? "Verbindung vor dem welcome beendet" : fehler;
        }
        break;
    }

    if (! welcomeKam)
    {
        verbindung.schliessen();
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::getrennt;
        return false;
    }

    {
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::verbunden;
        zustand.linkId = linkId;
        zustand.challenge = challenge;
        zustand.brokerEpoch = brokerEpoch;
        zustand.brokerVersion = brokerVersion;
        zustand.letzterFehler.clear();
    }
    const auto dieseWireGeneration = wireGeneration.fetch_add (1) + 1;

    // Was der letzte Verbindungsabbruch offen liess, geht jetzt zuerst raus
    // (§53.9 "nicht koaleszierbare Events bei Ueberlauf ueber Reconnect
    // wiederholen").
    {
        std::lock_guard<std::mutex> l (sendeMutex);
        p1.nachReconnectWiederholen();
    }
    inFlightNachReconnect (dieseWireGeneration);

    // Stand der P0-Ueberlaeufe beim Verbindungsaufbau. Waechst er waehrend
    // dieser Verbindung, wird sie geschlossen — nichts wird verworfen.
    const std::uint64_t ueberlaufBeimVerbinden = p0UeberlaufZaehler.load();
    auto ueberlaufSeitVerbinden = [this, ueberlaufBeimVerbinden] {
        return p0UeberlaufZaehler.load() != ueberlaufBeimVerbinden;
    };

    // Ratengrenze je Verbindung, DIESELBE wie im Broker (§33.1). Die Uhr ist
    // die des Aufrufers, damit die Klasse selbst keine liest.
    Ratengrenze rate (kRateProSekunde, kRateFensterMs);
    const auto rateBeginn = std::chrono::steady_clock::now();

    // Produktstatus lebt nicht in der allgemeinen Userqueue: der Provider ist
    // explizit und fehlt in Transport-/Fuzztests. Bei gesetztem Provider wird
    // unmittelbar und danach im 1-Hz-Takt ein voller Heartbeat erzeugt. Der
    // State-Report wird auf derselben Taktkante nur bei Aenderung koalesziert.
    auto naechsterHeartbeat = std::chrono::steady_clock::now();
    std::string letzterStateReport;

    // `B-CC-06`/`B-CC-07` (Regel 4): der Empfangsweg als eigener Schritt.
    // `false` heisst: die Verbindung ist zu beenden. Er wird in JEDER Runde
    // gegangen — auch direkt nach einem Send — und einmal zusaetzlich, bevor
    // ein gescheiterter Write die Verbindung schliesst.
    bool leseFehler = false;
    auto empfangenes = [&] (int fristMs) -> bool
    {
        std::size_t gelesen = 0;
        const auto la = verbindung.lesen (puffer, sizeof (puffer), gelesen,
                                          IpcVerbindung::fristIn (fristMs), fehler);
        if (la == LeseAusgang::fehler || la == LeseAusgang::ende)
        {
            leseFehler = true;
            return false;
        }
        if (la == LeseAusgang::daten && gelesen > 0)
            leser.fuettern (puffer, gelesen);

        for (;;)
        {
            const auto e = leser.naechster();
            if (e.art == StromLeser::Art::unvollstaendig)
                return true;
            if (e.art == StromLeser::Art::verstoss)
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler = "Envelope abgelehnt — Verbindung wird geschlossen";
                ++zustand.envelopeAbweisungen;
                return false;
            }
            // Die Familienzuordnung des Vertrags gilt in BEIDE Richtungen:
            // die Control-Verbindung traegt ausschliesslich P0/P1 (§33.1).
            // Ohne diese Sperre reichte ein korrekt gerahmter P2-Frame vom
            // Peer seine Binaerpayload an `beiAntwort` weiter, das JSON
            // erwartet (T2-Befund 4 vom 2026-08-29).
            if (e.kopf.familie == Familie::p2
                || e.kopf.encoding != Kodierung::json)
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler =
                    "P2 oder Nicht-JSON auf der Control-Verbindung — wird geschlossen";
                ++zustand.familieAbweisungen;
                return false;
            }

            // Ratengrenze VOR dem Callback: ein Peer, der hinter dem welcome
            // beliebig viele Frames pipelined, darf den Aufrufer nicht damit
            // fluten (§33.1, T2-Befund 5 vom 2026-08-29).
            const auto jetztMs = static_cast<std::uint64_t> (
                std::chrono::duration_cast<std::chrono::milliseconds> (
                    std::chrono::steady_clock::now() - rateBeginn).count());
            if (! rate.erlaubt (jetztMs))
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler =
                    "Nachrichtenratengrenze ueberschritten — Verbindung wird geschlossen";
                ++zustand.rateAbweisungen;
                return false;
            }

            {
                std::lock_guard<std::mutex> l (zustandMutex);
                ++zustand.empfangen;
            }
            const std::string antwort (reinterpret_cast<const char*> (e.payload),
                                       e.payloadLaenge);
            inFlightAck (antwort);
            // Nach `stop()` wird kein Callback mehr gerufen (`B-CC-10`).
            if (beiAntwort && ! sollAbbrechen (generation))
                beiAntwort (antwort);
        }
    };

    std::vector<std::uint8_t> ausgang;
    while (! sollAbbrechen (generation))
    {
        const auto jetzt = std::chrono::steady_clock::now();
        if (statusProvider && jetzt >= naechsterHeartbeat)
        {
            const auto status = statusProvider();
            if (sollAbbrechen (generation))
                break;

            const auto report = stateReportJson (hello.adresse, status);
            if (report != letzterStateReport)
            {
                sendeP1 ("produkt-state-report", report);
                letzterStateReport = report;
            }

            const auto sequence = heartbeatFolge.fetch_add (1) % kJsonSafeModulus;
            sendeP0 (heartbeatAlsJson (hello.adresse, sequence, status));
            naechsterHeartbeat = jetzt + std::chrono::milliseconds (kHeartbeatTaktMs);
        }

        if (ueberlaufSeitVerbinden())
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.letzterFehler = "P0-Ueberlauf: Verbindung wird geschlossen";
            break;
        }

        // 1) Steuerung zuerst, immer. P1 kommt erst dran, wenn P0 leer ist —
        //    das ist die Client-Haelfte von "kein P0 wartet hinter Daten".
        bool etwasGesendet = false;
        std::string nachricht, schluessel;
        bool istP0 = false;
        {
            std::lock_guard<std::mutex> l (sendeMutex);
            istP0 = p0.entnehmen (nachricht);
            etwasGesendet = istP0 || p1.entnehmen (schluessel, nachricht);
        }
        if (etwasGesendet)
        {
            const auto familie = istP0 ? Familie::p0 : Familie::p1;
            if (! envelopeSchreiben (familie, 0,
                                     reinterpret_cast<const std::uint8_t*> (nachricht.data()),
                                     nachricht.size(), ausgang)
                || ! verbindung.schreibenGenau (ausgang.data(), ausgang.size(),
                                                IpcVerbindung::fristIn (kIoFristMs), fehler))
            {
                // Entnommen, aber NICHT auf dem Draht: der Eintrag geht
                // zurueck. Ohne das verschwand ein nicht koaleszierbares
                // P1-Ereignis endgueltig, wenn der Broker zwischen Entnahme und
                // Write schloss — trotz Reconnect-Vertrag (§53.9). Fuer P0 gilt
                // dieselbe Zusage noch strenger: "nichts verwerfen".
                {
                    std::lock_guard<std::mutex> l (sendeMutex);
                    if (istP0)
                        p0.zuruecklegen (std::move (nachricht));
                    else
                        p1.zuruecklegen (schluessel, std::move (nachricht));
                    std::lock_guard<std::mutex> z (zustandMutex);
                    zustand.p1Wiederholungen = p1.wiederholungen();
                }
                // `B-CC-07`: was schon vollstaendig empfangen wurde, wird
                // noch GEMELDET, bevor die Verbindung endet. Sonst ginge genau
                // der P0-ACK verloren, auf den der Aufrufer wartet — der Write
                // scheiterte ja oft, weil der Peer nur nicht mehr liest.
                empfangenes (0);

                // Der Platz war bis hierher reserviert; `zuruecklegen` hat ihn
                // wieder mit dem Eintrag belegt. Nichts ist verlorengegangen.
                if (! sollAbbrechen (generation))
                {
                    std::lock_guard<std::mutex> l (zustandMutex);
                    // Der Ueberlauf ist die URSACHE, der abgebrochene Write nur
                    // seine Folge. Wer die Folge meldet, verschleiert den Grund.
                    zustand.letzterFehler =
                        ueberlaufSeitVerbinden()
                            ? std::string ("P0-Ueberlauf: Verbindung wird geschlossen")
                            : (fehler.empty() ? std::string ("Nachricht zu gross") : fehler);
                }
                break;
            }
            // Auf dem Draht: erst JETZT gibt die Queue den reservierten Platz
            // frei (§53.9 "nichts verwerfen" gilt bis zum Write-Commit).
            {
                std::lock_guard<std::mutex> l (sendeMutex);
                if (istP0)
                    p0.bestaetigen();
                else
                    p1.bestaetigen();
            }
            if (istP0)
            {
                inFlightNachWireWrite (nachricht, dieseWireGeneration);
                // Ein Reconnect kann mehr semantisch offene Auftraege als
                // P0-Plaetze vorfinden. Jeder frei gewordene Queueplatz zieht
                // deshalb den naechsten alten In-Flight-Eintrag nach; Eintraege
                // dieser Generation werden dabei nie ohne Verbindungsverlust
                // erneut gesendet.
                inFlightNachReconnect (dieseWireGeneration);
            }
            std::lock_guard<std::mutex> l (zustandMutex);
            if (istP0)
                ++zustand.p0Gesendet;
            else
                ++zustand.p1Gesendet;
            // KEIN `continue` mehr. Die alte Fassung sprang hier zurueck an
            // den Anfang und uebersprang den Lesepfad, solange irgendetwas
            // wartete: ein bereits vorliegender P0-ACK wurde nicht verarbeitet,
            // waehrend P1 rueckstaute, und ein blockierender P1-Write hungerte
            // ihn ganz aus. Das bricht den Gate-Satz "ohne P0-Starvation"
            // (NAK-95, Befund 4).
        }

        // 2) Lesen — IN JEDER RUNDE (`B-CC-06`). Direkt nach einem Send nur
        //    kurz pollen, damit der Durchsatz nicht am Lesetakt haengt; ist
        //    nichts zu senden, wird der volle Takt gewartet.
        if (! empfangenes (etwasGesendet ? 0 : kLeseTaktMs))
        {
            if (! sollAbbrechen (generation) && leseFehler)
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                if (ueberlaufSeitVerbinden())
                    zustand.letzterFehler = "P0-Ueberlauf: Verbindung wird geschlossen";
                else if (! fehler.empty())
                    zustand.letzterFehler = fehler;
            }
            break;
        }
    }

    verbindung.schliessen();
    if (! abgeloest (meinLauf))
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::getrennt;
    }
    return true;
}

} // namespace nakama::ipc

// K1 erneut: die zweite Auswertung sieht Makros, die erst ein spaeter
// eingebundener Header definiert haben koennte.
#include "NakamaKernRiegel.h"
