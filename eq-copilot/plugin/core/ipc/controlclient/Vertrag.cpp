// K1 (S8/SONDE-007a): der Kompilier-Riegel steht am Anfang UND am Ende jeder
// Kern-Uebersetzungseinheit. Diese hier sieht zwar gar kein JUCE, aber die
// Regel gilt fuer die MENGE, nicht fuer den Einzelfall - eine Ausnahme waere
// die Stelle, an der die naechste Quelle sie stillschweigend erbt.
#include "NakamaKernRiegel.h"

// ControlClient — Vertrag und Fehlercodes: was ueber den Draht geht.
//
// NAK-225/S25d (09.09.2026), herausgeloest aus core/ipc/ControlClient.cpp.
// Inhalt sind die freien Funktionen des Namensraums nakama::ipc, die
// ControlClient.h deklariert:
//
//   helloAlsJson, heartbeatAlsJson, adresseAlsJson, wireAdresseAusState
//                        Die Wire-Form der eigenen Aussagen.
//   welcomeHaeltVertrag, rejectHaeltVertrag, commandAckHaeltVertrag
//                        Die Vertragspruefung der Gegenaussagen. Ein Frame der
//                        falschen Familie wird abgewiesen, nicht geraten.
//   auftragMitBasisRevision
//                        Die Basisrevision in einen Auftrag setzen.
//   audioGueltig, adresseGueltig, istHex32, instanceAliasZielPasst
//                        Die Grenzen, an denen ein Wert abgelehnt wird.
//   instanceAdresseAusState
//                        Die Ableitung einer Wire-Adresse aus einer State-Id.
//
// Zwei Regeln binden diese Datei:
//
//   Vertraege und Laengen (Pruefliste C): jedes Textfeld mit Schema-Laenge wird
//   gegen genau diese Laenge geprueft, Zeichen gegen Bytes benannt;
//   Discriminator und Familie werden VOR dem Inhalt geprueft.
//
//   Behauptung <= Messung: ein Fehlercode, den der Vertrag nicht kennt, wird
//   nicht auf einen bekannten abgebildet, sondern abgewiesen.

#include "ControlClient.h"
#include "NakamaKanon.h"
#include "WireEnvelope.h"
#include "controlclient/Intern.h"
#include <cmath>
#include <cctype>

namespace nakama::ipc
{

// Die Helfer des internen Kopfes; die Aufrufstellen bleiben damit
// woertlich wie im Urstand.
using namespace controlclient_intern;

bool commandAckHaeltVertrag (const std::string& text, GelesenesCommandAck& gelesen)
{
    gelesen = {};
    std::string commandId;
    const auto art = commandAckArtLesen (text, commandId);
    if (art == CommandAckArt::keinAck)
        return false;
    gelesen.commandId = std::move (commandId);
    gelesen.erfolgreich = art == CommandAckArt::angewandt
                       || art == CommandAckArt::idempotentWiederholt;
    return true;
}

/*  SONDE-014 KR-01 (Entscheid E-15, Konvergenzrunde 07.09.2026).

    Der Text wird NICHT geparst und neu geschrieben, sondern an genau einer
    Stelle ersetzt: alles andere bleibt bytegleich. Ein Neuaufbau haette die
    Zusage "derselbe Auftrag" nur behauptet - hier traegt sie der Speicher.

    Fail-closed nach demselben Muster wie `commandIdAusAuftrag`: Strings
    werden als Token konsumiert, damit ein `base_revision` INNERHALB einer
    Notiz nie getroffen wird; genau EIN `kopf` und genau EINE
    `base_revision` duerfen vorkommen, und die Zahl muss hinter dem Kopf
    stehen. Alles andere liefert leer - dann wiederholt der Client nicht. */
std::string auftragMitBasisRevision (const std::string& auftragJson,
                                     std::uint64_t basisRevision)
{
    std::size_t position = 0;
    std::size_t kopfStelle = std::string::npos;
    std::size_t zahlVon = std::string::npos, zahlBis = std::string::npos;
    unsigned kopfGefunden = 0, revisionGefunden = 0;

    while (position < auftragJson.size())
    {
        if (auftragJson[position] != '"')
        {
            ++position;
            continue;
        }

        const std::size_t tokenBeginn = position;
        std::string token;
        bool escape = false;
        if (! jsonStringToken (auftragJson, position, token, escape))
            return {};
        std::size_t nachToken = position;
        while (nachToken < auftragJson.size()
               && std::isspace (static_cast<unsigned char> (auftragJson[nachToken])) != 0)
            ++nachToken;
        // Ohne folgenden Doppelpunkt ist das ein WERT, kein Feldname.
        if (escape || nachToken >= auftragJson.size() || auftragJson[nachToken] != ':')
            continue;

        if (token == "kopf")
        {
            if (++kopfGefunden > 1)
                return {};
            kopfStelle = tokenBeginn;
            continue;
        }
        if (token != "base_revision")
            continue;
        if (++revisionGefunden > 1)
            return {};

        std::size_t ziffer = nachToken + 1;
        while (ziffer < auftragJson.size()
               && std::isspace (static_cast<unsigned char> (auftragJson[ziffer])) != 0)
            ++ziffer;
        zahlVon = ziffer;
        while (ziffer < auftragJson.size()
               && auftragJson[ziffer] >= '0' && auftragJson[ziffer] <= '9')
            ++ziffer;
        zahlBis = ziffer;
        position = ziffer;
    }

    if (kopfGefunden != 1 || revisionGefunden != 1
        || zahlBis <= zahlVon || zahlVon < kopfStelle)
        return {};

    std::string aus = auftragJson;
    aus.replace (zahlVon, zahlBis - zahlVon, std::to_string (basisRevision));
    return aus;
}

/*  Der Bootstrap-`hello`-Text (NAK-181 Nacharbeit 1, EP-09/NR-09).

    🔑 Er stand bis zu dieser Runde inline im Verbindungsaufbau — hinter
    Pipe, Serverpruefung und Handschlag — und war damit ohne Draht nicht
    messbar. `samplerate` laeuft hier durch `zahl()`, also durch dieselbe
    locale-unabhaengige Zahlform wie jede andere v3-Zahl; `block_size` und
    `channels` sind Ganzzahlen und duerfen `std::to_string` behalten. */
std::string helloAlsJson (const ControlHello& hello)
{
    std::string aus =
        std::string ("{\"type\":\"hello\",\"connection_kind\":\"control\",\"protocol\":3,")
        + "\"plugin_version\":" + jsonString (hello.pluginVersion)
        + ",\"plugin_kind\":" + jsonString (hello.pluginKind)
        + ",\"adresse\":" + adresseAlsJson (hello.adresse);
    if (hello.hostAngeben)
        aus += ",\"host\":{\"pid\":" + std::to_string (hello.hostPid) + "}";
    aus += ",\"audio\":{\"samplerate\":" + zahl (hello.samplerate)
         + ",\"block_size\":" + std::to_string (hello.blockSize)
         + ",\"channels\":" + std::to_string (hello.channels) + "}}";
    return aus;
}

std::string heartbeatAlsJson (const Adresse& adresse, std::uint64_t sequence,
                              const ControlStatus& status, bool bestaetigtNeutral)
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
         + "}"
         // SONDE-013 M-39: nur GESETZT reist es. Ein `false` in jedem
         // Heartbeat waere die Behauptung "Zustand bekannt" — und das Feld ist
         // im Schema optional, damit genau diese Behauptung nicht bei jedem
         // Takt mitfaehrt.
         //
         // 🔑 NAK-180 R1: die Regel ist DREIWERTIG. Das Feld traegt eine
         // Aussage in genau zwei Faellen - gemeldeter Ueberlauf (`true`) und
         // ausdruecklich bestaetigter Neuaufbau (`false`); sonst fehlt es
         // ganz. M-39 bleibt damit fuer den Dauerbetrieb woertlich erhalten:
         // der Steady-State schweigt weiter. Das `false` ist der einzige
         // Ausloeser von `resync_bestaetigen` im Broker - vorher hatte der
         // dort gebaute Riegel im Produkt gar keinen Aufrufer (D-01).
         + (status.interventionStateUnknown
                ? std::string (",\"intervention_state_unknown\":true")
                : (bestaetigtNeutral
                       ? std::string (",\"intervention_state_unknown\":false")
                       : std::string()))
         + runtimeJson (status.runtime) + "}";
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
    // 🔑 NAK-181 R4d (MP3-1): die vierte Bedingung an der Samplerate. `> 0`
    // ist keine Schranke gegen `1e-308` — der Wert ist endlich, positiv und
    // unter 768000, und der v3-Textriegel lehnt ihn trotzdem ab. Ohne diese
    // Zeile reiste `"samplerate":null` in einem Pflicht-`number` des Hellos,
    // und der Gegenleser verwuerfe den Handschlag. Ein Client, der wissentlich
    // Unzustellbares sendet, verbindet lieber gar nicht erst.
    std::string verworfen;
    return std::isfinite (samplerate) && samplerate > 0.0 && samplerate <= 768000.0
        && nakama::wire::wireZahl (samplerate, verworfen)
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

Adresse wireAdresseAusState (Adresse adresse)
{
    adresse.instanceId = instanceAdresseAusState (adresse.instanceId);
    return adresse;
}

bool instanceAliasZielPasst (const std::string& lokaleInstanceId,
                             const std::string& wireInstanceId)
{
    return ! lokaleInstanceId.empty()
        && istHex32 (wireInstanceId)
        && instanceAdresseAusState (lokaleInstanceId) == wireInstanceId;
}

} // namespace nakama::ipc

// K1 erneut: die zweite Auswertung sieht Makros, die erst ein spaeter
// eingebundener Header definiert haben koennte.
#include "NakamaKernRiegel.h"
