// K1 (S8/SONDE-007a): der Kompilier-Riegel steht am Anfang UND am Ende jeder
// Kern-Uebersetzungseinheit. Diese hier sieht zwar gar kein JUCE, aber die
// Regel gilt fuer die MENGE, nicht fuer den Einzelfall - eine Ausnahme waere
// die Stelle, an der die naechste Quelle sie stillschweigend erbt.
#include "NakamaKernRiegel.h"

#include "TelemetryClient.h"
#include "WireEnvelope.h"

#include <algorithm>

namespace nakama::ipc
{
namespace
{
/// Wie lange der Thread schlaeft, wenn die Schleuse leer ist. Bei 10 Hz
/// Livekadenz (§33.2) ist das reichlich; laenger wuerde die Latenz eines
/// frischen Frames unnoetig strecken.
constexpr int kLeerlaufMs = 5;

std::string jsonStringSicher (const std::string& roh)
{
    std::string aus = "\"";
    for (char c : roh)
        if (static_cast<unsigned char> (c) >= 0x20 && c != '"' && c != '\\')
            aus.push_back (c);
    aus.push_back ('"');
    return aus;
}
} // namespace

TelemetryClient::TelemetryClient (std::function<TelemetryHello()> helloProviderIn,
                                  std::string pipeNameIn)
    : helloProvider (std::move (helloProviderIn)),
      pipeName (std::move (pipeNameIn))
{
}

TelemetryClient::~TelemetryClient()
{
    stop();
}

void TelemetryClient::start()
{
    std::lock_guard<std::mutex> l (lebenslaufMutex);
    if (laeuft.load())
        return;
    laeuft.store (true);
    thread = std::thread ([this] { threadLauf(); });
}

void TelemetryClient::stop()
{
    std::lock_guard<std::mutex> l (lebenslaufMutex);
    if (! laeuft.load() && ! thread.joinable())
        return;
    laeuft.store (false);
    verbindungsGeneration.fetch_add (1);
    verbindung.ioAbbrechen();
    warte.notify_all();
    if (thread.joinable())
        thread.join();
    verbindung.schliessen();
    std::lock_guard<std::mutex> z (zustandMutex);
    zustand.status = Status::getrennt;
}

void TelemetryClient::reconnect()
{
    verbindungsGeneration.fetch_add (1);
    verbindung.ioAbbrechen();
    warte.notify_all();
}

bool TelemetryClient::sollAbbrechen (std::uint64_t generation) const noexcept
{
    return ! laeuft.load() || verbindungsGeneration.load() != generation;
}

bool TelemetryClient::veroeffentlichen (const std::uint8_t* daten, std::size_t laenge) noexcept
{
    return schleuse.veroeffentlichen (daten, laenge);
}

TelemetryClient::Snapshot TelemetryClient::snapshot() const
{
    std::lock_guard<std::mutex> l (zustandMutex);
    Snapshot s = zustand;
    s.ersetzt = schleuse.ersetzteFrames();
    s.zuGross = schleuse.zuGrosseFrames();
    return s;
}

void TelemetryClient::threadLauf()
{
    int backoffMs = kBackoffStartMs;
    while (laeuft.load())
    {
        const auto generation = verbindungsGeneration.load();
        const TelemetryHello hello = helloProvider ? helloProvider() : TelemetryHello();

        if (! istHex32 (hello.linkId) || ! istHex32 (hello.challenge))
        {
            // Die Kopplung steht noch nicht. Warten, NICHT verbinden: ein
            // ungekoppelter Telemetry-Connect wird geschlossen und kostet nur
            // einen Verbindungsslot.
            {
                std::lock_guard<std::mutex> z (zustandMutex);
                zustand.status = Status::wartetAufKopplung;
            }
            std::unique_lock<std::mutex> l (wartemutex);
            warte.wait_for (l, std::chrono::milliseconds (kBackoffStartMs / 10 + 1),
                            [this, generation] {
                                return ! laeuft.load()
                                    || verbindungsGeneration.load() != generation;
                            });
            continue;
        }

        const bool stand = eineVerbindung (generation, hello);
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
            backoffMs = kBackoffStartMs;
            continue;
        }
        backoffMs = std::min (backoffMs * 2, kBackoffMaxMs);
    }
}

bool TelemetryClient::eineVerbindung (std::uint64_t generation, const TelemetryHello& hello)
{
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::verbindet;
        ++zustand.verbindungsVersuche;
    }

    if (! adresseGueltig (hello.adresse))
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::getrennt;
        zustand.letzterFehler = "Adresse haelt den v3-Vertrag nicht (hex32/SID)";
        return false;
    }

    std::string fehler;
    if (! verbindung.oeffnen (pipeName, fehler))
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::getrennt;
        if (! sollAbbrechen (generation))
            zustand.letzterFehler = fehler;
        return false;
    }

    const std::string helloJson =
        std::string ("{\"type\":\"hello\",\"connection_kind\":\"telemetry\",\"protocol\":3,")
        + "\"plugin_version\":" + jsonStringSicher (hello.pluginVersion)
        + ",\"adresse\":" + adresseAlsJson (hello.adresse)
        + ",\"link_id\":" + jsonStringSicher (hello.linkId)
        + ",\"challenge\":" + jsonStringSicher (hello.challenge) + "}";

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

    // Auch die Antwort auf das Telemetry-Hello ist v3-gerahmt (§53.9).
    StromLeser leser;
    std::uint8_t puffer[4096];
    const auto welcomeFrist = IpcVerbindung::fristIn (kIoFristMs);
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
            // Dieselbe Strenge wie im ControlClient. Die alte Fassung verlangte
            // nur flaches JSON mit `type == "welcome"` — damit haette ein
            // Testserver die Kopplung mit einem P2-Envelope und dem Payload
            // {"type":"welcome"} bestaetigen koennen, ohne link_id, challenge,
            // protocol oder Broker-Epoch je zu nennen (T2-Befund 10 vom
            // 2026-08-29). Das `welcome` ist per Vertrag ein P0-Frame
            // (§53.9, `eq-ipc-v3.schema.json`).
            if (e.kopf.familie != Familie::p0)
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler = "welcome kam nicht als P0";
                break;
            }
            const std::string text (reinterpret_cast<const char*> (e.payload), e.payloadLaenge);
            std::vector<std::pair<std::string, std::string>> felder;
            std::string typ, protokoll, linkId, challenge, brokerEpoch, brokerVersion;
            if (! flachesJsonObjekt (text, felder) || ! jsonFeld (felder, "type", typ))
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler = "welcome: kein flaches JSON-Objekt";
                break;
            }
            if (typ == "reject")
            {
                std::string grund;
                jsonFeld (felder, "reason", grund);
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler = "Broker lehnt ab: " + grund;
                break;
            }
            // Die KOPPLUNGSWERTE muessen die eigenen sein: ein welcome mit
            // fremder link_id bestaetigt die Kopplung einer anderen Instanz.
            if (typ != "welcome"
                || ! jsonFeld (felder, "protocol", protokoll) || protokoll != "3"
                || ! jsonFeld (felder, "link_id", linkId) || linkId != hello.linkId
                || ! jsonFeld (felder, "challenge", challenge) || challenge != hello.challenge
                || ! jsonFeld (felder, "broker_epoch", brokerEpoch) || ! istHex32 (brokerEpoch)
                || ! jsonFeld (felder, "broker_version", brokerVersion) || brokerVersion.empty())
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler = "unerwartete Antwort auf das Telemetry-Hello";
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
        if (! sollAbbrechen (generation))
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.letzterFehler = (ausgang == LeseAusgang::zeitlimit)
                                      ? "Kopplung nicht bestaetigt (kein welcome)"
                                      : (fehler.empty() ? "Verbindung vor dem welcome beendet"
                                                        : fehler);
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
        zustand.letzterFehler.clear();
    }

    std::vector<std::uint8_t> ausgang;
    std::vector<std::uint8_t> frame (schleuse.slotGroesse());
    while (! sollAbbrechen (generation))
    {
        const std::size_t n = schleuse.abholen (frame.data(), frame.size());
        if (n == 0)
        {
            std::unique_lock<std::mutex> l (wartemutex);
            warte.wait_for (l, std::chrono::milliseconds (kLeerlaufMs), [this, generation] {
                return ! laeuft.load() || verbindungsGeneration.load() != generation;
            });
            continue;
        }
        if (! envelopeSchreiben (Familie::p2, 0, frame.data(), n, ausgang)
            || ! verbindung.schreibenGenau (ausgang.data(), ausgang.size(),
                                            IpcVerbindung::fristIn (kIoFristMs), fehler))
        {
            if (! sollAbbrechen (generation))
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler = fehler.empty() ? "P2-Frame zu gross" : fehler;
            }
            break;
        }
        std::lock_guard<std::mutex> l (zustandMutex);
        ++zustand.gesendet;
    }

    verbindung.schliessen();
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
