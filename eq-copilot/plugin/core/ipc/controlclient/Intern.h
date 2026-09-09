// Interner Kopf des ControlClient — die Helfer, die alle Teile brauchen.
//
// NAK-225/S25d (09.09.2026): Beim Aufteilen von core/ipc/ControlClient.cpp nach
// Fachbereich musste der anonyme Namensraum der Wurzel (Zeilen 19-399 des
// Urstands) sichtbar bleiben - gemessen wurde, dass alle vier Teile aus ihm
// lesen: kStopFristMs in der Wurzel und in Laufzeit, CommandAckArt,
// commandIdAusAuftrag und commandAckArtLesen in Vertrag und Nachrichten,
// kSchluesselStateReport, kLeseTaktMs, kJsonSafeModulus und stateReportJson in
// der Verbindung.
//
// Ein anonymer Namensraum in einem Kopf haette je Uebersetzungseinheit eine
// eigene Kopie erzeugt. Deshalb steht hier alles inline in einem eigenen,
// benannten Namensraum: eine Entitaet je Helfer, und der Name sagt, dass sie
// niemandem ausserhalb des ControlClient gehoeren. Die Teile holen sie mit
// using namespace controlclient_intern innerhalb von nakama::ipc - die
// Aufrufstellen bleiben damit woertlich wie im Urstand.
//
// Was hier NICHT steht: die Vertragsfunktionen, die ControlClient.h deklariert
// (helloAlsJson, heartbeatAlsJson, welcomeHaeltVertrag, ...). Sie gehoeren dem
// Vertrag und stehen in controlclient/Vertrag.cpp.
//
// Dieser Kopf ist ein interner Teil des ControlClient und wird nur von seinen
// Uebersetzungseinheiten eingebunden.

#pragma once

#include "ControlClient.h"
#include "WireZahl.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace nakama::ipc::controlclient_intern
{

/// SONDE-014 WN-01: der Koaleszierungsschluessel des produktiven
/// `state_report`. Er steht an EINER Stelle, weil zwei Schreibweisen
/// desselben Schluessels zwei Wahrheiten ueber dieselbe Koaleszierung waeren.
inline constexpr const char* kSchluesselStateReport = "produkt-state-report";

/// Wie lange der Lesevorgang je Runde wartet, bevor die Sendequeues wieder
/// drankommen. Kurz genug, dass ein P0-Befehl nicht hinter Stille wartet.

inline constexpr int kLeseTaktMs = 20;

// Der Takt selbst steht im Header (`nakama::ipc::kHeartbeatTaktMs`): ein
// erster Heartbeat wird unmittelbar nach dem welcome eingereiht, damit die
// 2,5-s-Stalegrenze nicht von einem zufaelligen Phasenversatz abhaengt.

inline constexpr std::uint64_t kJsonSafeModulus = 9007199254740992ULL; // 2^53

/// Frist, die `stop()` einem LAUFENDEN Callback noch laesst (Matrix
/// `B-CC-12`). Derselbe Wert wie `SENKE_FRIST` im Rust-Listener: beide Seiten
/// geben fremdem Code dieselbe Gnadenfrist. Lang genug fuer einen normalen
/// Callback (Mikro- bis Millisekunden), kurz genug, dass das Schliessen eines
/// Plugins im Host nicht spuerbar haengt.

inline constexpr int kStopFristMs = 2000;

enum class CommandAckArt
{
    keinAck,
    angewandt,
    abgelehnt,
    konflikt,
    abgelaufen,
    idempotentWiederholt
};

inline bool istHex64 (const std::string& s) noexcept
{
    if (s.size() != 64)
        return false;
    for (char c : s)
        if (! ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')))
            return false;
    return true;
}

inline bool nichtnegativeJsonGanzzahl (const std::string& s) noexcept
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

/// Die achtzehn Fehlercodes des Vertrags — `$defs/fehlercode` in
/// `eq-copilot/schemas/v3/eq-ipc-v3.schema.json` (Enum Zeilen 817-834):
/// zwoelf der Fassung 1 und sechs der Fassung 2 (SONDE-013 Nacharbeit 1
/// und 2). Das ist die EINZIGE Stelle, an der der C++-Leser sie fuehrt;
/// `IpcTestMain.cpp` haelt sie als Menge gegen die Enum-Liste des Schemas,
/// damit die naechste Vertragsfassung im TEST faellt und nicht im Feld
/// (NAK-230, Riegel R3).
inline constexpr std::string_view kFehlercodes[] = {
    // Fassung 1
    "protocol_mismatch", "unknown_message", "schema_violation",
    "unauthorized", "unknown_target", "revision_conflict",
    "capability_missing", "record_state_unknown", "recording_active",
    "lease_expired", "rate_limited", "internal",
    // Fassung 2 - die Produktregeln der Experimentfamilien
    "abdeckung_zu_gering", "schon_terminal", "ohne_lautheitsabgleich",
    "ohne_resultatmessung", "blindreihenfolge_widerspruch",
    "reihenfolge_nicht_gebunden"
};

/// Kennt der Leser diesen Code?
///
/// 🔑 NAK-230 (09.09.2026): das Wort ist BEKANNT, nicht "gueltig". Bis zu
/// diesem Ticket hiess die Funktion `fehlercodeHaeltVertrag` und war ein
/// Gueltigkeitsriegel - ein `command_ack` mit einem anderen Code fiel ganz.
/// Der Vertrag sagt aber woertlich "ein unbekannter Code wird als
/// generischer Fehler behandelt, nie als Erfolg", nicht "die Nachricht wird
/// verworfen". Ein Leser, der sie verwirft, ist STRENGER als sein Vertrag:
/// er laesst den Auftrag im In-Flight-Register stehen, obwohl der Broker ihn
/// final beantwortet hat. Die Antwort dieser Funktion entscheidet deshalb
/// nur noch die DEUTUNG des Codes, nie die Gueltigkeit der Nachricht.
inline bool fehlercodeIstBekannt (const std::string& code) noexcept
{
    const std::string_view sicht (code);
    for (const auto bekannt : kFehlercodes)
        if (sicht == bekannt)
            return true;
    return false;
}

// Liest einen JSON-String nur so weit, wie es fuer die eindeutige
// `command_id`-Suche noetig ist. Escapes werden uebersprungen, aber nicht
// interpretiert: weder ein Schluessel noch hex32 darf laut Vertrag ein Escape
// benoetigen. Die vollstaendige Nachricht prueft weiterhin der Schemaweg.

inline bool jsonStringToken (const std::string& text, std::size_t& position,
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

inline bool commandIdAusAuftrag (const std::string& text, std::string& commandId)
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

inline CommandAckArt commandAckArtLesen (const std::string& text, std::string& commandId,
                                  std::uint64_t* revisionAus = nullptr)
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

    // SONDE-014 WN-01: der Broker nennt im ACK die Revision, die er KENNT.
    // Genau sie traegt der frische Kopf einer Wiederholung - keine geratene
    // und keine lokal hochgezaehlte.
    if (revisionAus != nullptr)
    {
        try
        {
            *revisionAus = std::stoull (revision);
        }
        catch (...)
        {
            *revisionAus = 0;
        }
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

    // 🔑 NAK-230: der Leser ist genau so streng wie der Vertrag.
    //
    // Ein `code`, der KEINE Zeichenkette ist, bleibt ein Strukturfehler der
    // Nachricht - der Vertrag deutet unbekannte Codes, er erlaubt keine
    // falschen Typen.
    if (code != nullptr && ! code->istString)
        return CommandAckArt::keinAck;

    // Ein Zeichenketten-`code` macht die Nachricht dagegen NIE ungueltig:
    // "ein unbekannter Code wird als generischer Fehler behandelt, nie als
    // Erfolg" (`eq-ipc-v3.schema.json`, `$defs/fehlercode`). Bekannte Codes
    // laufen unveraendert weiter; ein unbekannter waere allein mit einem
    // Erfolgsergebnis widerspruechlich - dann wuerde sein `state_hash` als
    // bestaetigter Wirkungsschnitt gebucht, obwohl der Leser den mitgelieferten
    // Grund nicht versteht. Der Ausgang ist deshalb ein ENDGUELTIGER FEHLER:
    // der Auftrag ist beendet und das Register frei, aber nie erfolgreich.
    // Der Wortlaut des Codes bleibt unangetastet - er reist ohnehin im
    // Rohtext der Antwort an Anzeige und Log (`Verbindung.cpp`, `beiAntwort`).
    if (code != nullptr && erfolg && ! fehlercodeIstBekannt (code->wert))
        return CommandAckArt::abgelehnt;
    return art;
}

inline std::string jsonString (const std::string& roh)
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

inline std::string jsonStringUntrusted (const std::string& roh)
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

inline bool utf8CodepointsBis (const std::string& text, std::size_t maximum) noexcept
{
    std::size_t codepoints = 0;
    for (std::size_t i = 0; i < text.size();)
    {
        const auto erster = static_cast<unsigned char> (text[i]);
        std::size_t bytes = 0;
        std::uint32_t wert = 0;
        if (erster <= 0x7f)      { bytes = 1; wert = erster; }
        else if (erster >= 0xc2 && erster <= 0xdf)
                                { bytes = 2; wert = erster & 0x1f; }
        else if (erster >= 0xe0 && erster <= 0xef)
                                { bytes = 3; wert = erster & 0x0f; }
        else if (erster >= 0xf0 && erster <= 0xf4)
                                { bytes = 4; wert = erster & 0x07; }
        else return false;
        if (i + bytes > text.size()) return false;
        for (std::size_t j = 1; j < bytes; ++j)
        {
            const auto folge = static_cast<unsigned char> (text[i + j]);
            if ((folge & 0xc0) != 0x80) return false;
            wert = (wert << 6) | (folge & 0x3f);
        }
        if ((bytes == 3 && wert < 0x800)
            || (bytes == 4 && wert < 0x10000)
            || (wert >= 0xd800 && wert <= 0xdfff)
            || wert > 0x10ffff)
            return false;
        i += bytes;
        if (++codepoints > maximum) return false;
    }
    return true;
}

inline std::uint64_t jsonSafe (std::uint64_t wert) noexcept
{
    return std::min (wert, kJsonSafeModulus - 1);
}

inline const char* boolJson (bool wert) noexcept { return wert ? "true" : "false"; }

inline std::string capabilitiesJson()
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

inline std::string stateHashJson (const std::string& hash)
{
    // Ein kaputter lokaler Hash darf nicht als scheinbar gueltiger Stand auf
    // den Draht. null ist der schemafeste, fail-closed Wert.
    return istHex64 (hash) ? jsonString (hash) : "null";
}

inline std::string runtimeJson (const ControlRuntime& runtime)
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
    if (runtime.labelGemeldet && utf8CodepointsBis (runtime.label, 120))
        aus += ",\"label\":" + jsonStringUntrusted (runtime.label);
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

inline std::string stateReportJson (const Adresse& adresse, const ControlStatus& status)
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

/// Zahl aus einem Audiofeld.
///
/// 🔑 NAK-181 R4 (G4-Befund V05): derselbe locale-freie Bauer wie in
/// `NakamaEvidenz.cpp`, mit den Grenzen des v3-Textriegels. Bis hierher stand
/// hier `std::to_string(double)` — unter MSVC LC_NUMERIC-abhaengig.
///
/// `"null"` bleibt unerreichbar: `audioGueltig` laesst keinen Handschlag zu,
/// dessen Samplerate der Draht nicht traegt (T2-Befund 9 vom 2026-08-29,
/// NAK-181 R4d).

inline std::string zahl (double w)
{
    std::string text;
    if (! nakama::wire::wireZahl (w, text))
        return "null";
    return text;
}

} // namespace nakama::ipc::controlclient_intern
