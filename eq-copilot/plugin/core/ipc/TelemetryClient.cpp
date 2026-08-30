// K1 (S8/SONDE-007a): der Kompilier-Riegel steht am Anfang UND am Ende jeder
// Kern-Uebersetzungseinheit. Diese hier sieht zwar gar kein JUCE, aber die
// Regel gilt fuer die MENGE, nicht fuer den Einzelfall - eine Ausnahme waere
// die Stelle, an der die naechste Quelle sie stillschweigend erbt.
#include "NakamaKernRiegel.h"

#include "TelemetryClient.h"
#include "WireEnvelope.h"

#include <algorithm>
#include <chrono>

namespace nakama::ipc
{
namespace
{
/// Frist des Leerlauf-LESEVORGANGS, wenn die Schleuse leer ist. Er hat den
/// Schlaf ersetzt: dieselbe Wartezeit, aber er sieht, wenn der Broker die
/// Pipe schliesst. Bei 10 Hz Livekadenz (§33.2) ist die Frist reichlich;
/// laenger wuerde die Latenz eines frischen Frames unnoetig strecken.
constexpr int kLeerlaufMs = 5;

/// Frist, die `stop()` einem LAUFENDEN Callback noch laesst (Matrix
/// `B-TC-07`) — derselbe Wert wie im `ControlClient` und wie `SENKE_FRIST` im
/// Rust-Listener.
constexpr int kStopFristMs = 2000;

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

//== Die geteilte Laufzeit ===================================================
struct TelemetryClient::Laufzeit
{
    Laufzeit (std::function<TelemetryHello()> hp, std::string pn)
        : helloProvider (std::move (hp)), pipeName (std::move (pn)) {}

    void threadLauf (std::uint64_t meinLauf, std::shared_ptr<IpcVerbindung> meine);
    bool eineVerbindung (std::uint64_t generation, std::uint64_t meinLauf,
                         const TelemetryHello& hello, IpcVerbindung& verbindung);
    bool leerlaufLesen (StromLeser& leser, Ratengrenze& rate,
                        std::chrono::steady_clock::time_point rateBeginn,
                        std::uint64_t generation, IpcVerbindung& verbindung);
    bool sollAbbrechen (std::uint64_t generation) const noexcept;
    /// Wie im `ControlClient`: ein abgeloester Lauf schreibt keinen
    /// gemeinsamen Zustand mehr (`B-TC-07`, NAK-104).
    bool abgeloest (std::uint64_t meinLauf) const noexcept
    { return lebenslauf.load() != meinLauf; }
    Snapshot snapshotIntern() const;

    std::function<TelemetryHello()> helloProvider;
    std::string pipeName;

    /// Die Verbindung des LAUFENDEN Laufs — wortgleich zum `ControlClient`
    /// (`B-TC-07`, NAK-104): ein abgeloester Lauf darf die Pipe eines
    /// spaeteren `start()` nicht schliessen, und `lebenslauf` allein waere nur
    /// ein Check-then-use. Jeder Lauf bekommt deshalb seine eigene.
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

    P2Schleuse<8192> schleuse;

    std::atomic<bool> laeuft { false };
    std::atomic<bool> fertig { false };
    std::atomic<std::thread::id> threadId {};
    /// Welcher Lauf ist das? Ein nach `kStopFristMs` ABGELOESTER Thread lebt
    /// weiter, bis sein Callback zurueckkommt. Startet der Client bis dahin
    /// erneut, saehe der alte Thread `laeuft == true` und liefe auf DERSELBEN
    /// Laufzeit weiter — zwei Threads auf einer Pipe. Jeder Lauf traegt
    /// deshalb seine Nummer und endet, sobald sie nicht mehr die aktuelle ist.
    std::atomic<std::uint64_t> lebenslauf { 0 };
    std::atomic<std::uint64_t> verbindungsGeneration { 0 };
    std::mutex   wartemutex;
    std::condition_variable warte;

    mutable std::mutex zustandMutex;
    Snapshot zustand;
};

TelemetryClient::TelemetryClient (std::function<TelemetryHello()> helloProviderIn,
                                  std::string pipeNameIn)
    : k (std::make_shared<Laufzeit> (std::move (helloProviderIn), std::move (pipeNameIn)))
{
}

TelemetryClient::~TelemetryClient()
{
    stop();
}

void TelemetryClient::start()
{
    std::lock_guard<std::mutex> l (lebenslaufMutex);
    if (k->laeuft.load())
        return;
    k->laeuft.store (true);
    k->fertig.store (false);
    auto kern = k;
    const auto meinLauf = kern->lebenslauf.fetch_add (1) + 1;
    auto meine = kern->neueVerbindung();
    thread = std::thread ([kern, meinLauf, meine] {
        kern->threadLauf (meinLauf, std::move (meine));
    });
}

void TelemetryClient::stop()
{
    // Wortgleich zum `ControlClient` (`B-TC-07`): Reentranz ohne Self-Join,
    // sonst Frist und Abloesen. Ein blockierender P2-Write faellt ueber
    // `ioAbbrechen` sofort, nicht erst nach `kIoFristMs`.
    const bool ausDemClientthread = (std::this_thread::get_id() == k->threadId.load());

    // Genau die Verbindung, die beim Aufruf die aktuelle war.
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

void TelemetryClient::reconnect()
{
    k->verbindungsGeneration.fetch_add (1);
    k->aktuelleVerbindung()->ioAbbrechen();
    k->warte.notify_all();
}

bool TelemetryClient::Laufzeit::sollAbbrechen (std::uint64_t generation) const noexcept
{
    return ! laeuft.load() || verbindungsGeneration.load() != generation;
}

bool TelemetryClient::veroeffentlichen (const std::uint8_t* daten, std::size_t laenge) noexcept
{
    return k->schleuse.veroeffentlichen (daten, laenge);
}

TelemetryClient::Snapshot TelemetryClient::snapshot() const { return k->snapshotIntern(); }

TelemetryClient::Snapshot TelemetryClient::Laufzeit::snapshotIntern() const
{
    std::lock_guard<std::mutex> l (zustandMutex);
    Snapshot s = zustand;
    s.ersetzt = schleuse.ersetzteFrames();
    s.zuGross = schleuse.zuGrosseFrames();
    s.kollisionsLoecher = schleuse.kollisionsLoecher();
    s.beanspruchtVerworfen = schleuse.beanspruchtVerworfen();
    return s;
}

void TelemetryClient::Laufzeit::threadLauf (std::uint64_t meinLauf,
                                           std::shared_ptr<IpcVerbindung> meine)
{
    threadId.store (std::this_thread::get_id());
    int backoffMs = kBackoffStartMs;
    while (laeuft.load() && lebenslauf.load() == meinLauf)
    {
        const auto generation = verbindungsGeneration.load();
        const TelemetryHello hello = helloProvider ? helloProvider() : TelemetryHello();
        // Der Provider ist fremder Code und darf beliebig lange stehen. Ist
        // dieser Lauf in der Zeit abgeloest worden, wird NICHT mehr verbunden —
        // sonst risse ein abgeloester Lauf die Pipe des neuen auf und mit ihr
        // dessen Verbindung (`B-TC-07`, NAK-104).
        if (! laeuft.load() || lebenslauf.load() != meinLauf)
            break;

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

        const bool stand = eineVerbindung (generation, meinLauf, hello, *meine);
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
    // Nur der AKTUELLE Lauf meldet sich fertig: ein abgeloester Vorgaenger
    // wuerde sonst den `join` des neuen Laufs freigeben.
    if (lebenslauf.load() == meinLauf)
        fertig.store (true);
}

bool TelemetryClient::Laufzeit::leerlaufLesen (StromLeser& leser, Ratengrenze& rate,
                                              std::chrono::steady_clock::time_point rateBeginn,
                                              std::uint64_t generation,
                                              IpcVerbindung& verbindung)
{
    std::uint8_t puffer[4096];
    std::size_t gelesen = 0;
    std::string fehler;
    const auto ausgang = verbindung.lesen (puffer, sizeof (puffer), gelesen,
                                           IpcVerbindung::fristIn (kLeerlaufMs), fehler);
    if (ausgang == LeseAusgang::ende || ausgang == LeseAusgang::fehler)
    {
        if (! sollAbbrechen (generation))
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.letzterFehler =
                fehler.empty() ? "Telemetriepipe vom Broker geschlossen" : fehler;
        }
        return false;
    }
    if (ausgang == LeseAusgang::daten && gelesen > 0)
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

        // Die Telemetrieverbindung traegt ausschliesslich P2 (§33.1). Ein P0-
        // oder P1-Frame hier ist derselbe Vertragsbruch wie ein P2-Frame auf
        // der Control-Verbindung — der Broker weist ihn in der Gegenrichtung
        // genauso ab (`server_v3.rs`, `geschlossen_familie`).
        if (e.kopf.familie != Familie::p2)
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.letzterFehler =
                "P0/P1 auf der Telemetrieverbindung — wird geschlossen";
            ++zustand.familieAbweisungen;
            return false;
        }

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

        // Broker→Main-Liveupdates (§33.1) sind auf DIESER Verbindung
        // vertragsgemaess, haben in diesem Ticket aber noch keinen Verbraucher:
        // die Landkarte, die sie verteilt, ist SONDE-012. Sie werden deshalb
        // gezaehlt und verworfen — sichtbar, nicht still.
        std::lock_guard<std::mutex> l (zustandMutex);
        ++zustand.empfangen;
    }
}

bool TelemetryClient::Laufzeit::eineVerbindung (std::uint64_t generation,
                                                std::uint64_t meinLauf,
                                                const TelemetryHello& hello,
                                                IpcVerbindung& verbindung)
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
            std::vector<JsonFeld> felder;
            std::string typ, linkId, challenge, brokerEpoch, brokerVersion;
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
            // Derselbe Vertragspruefer wie im ControlClient — Typ, Laenge und
            // exakte Feldmenge (T2-Befund 3 vom 2026-08-29). Zusaetzlich
            // muessen die KOPPLUNGSWERTE die eigenen sein: ein welcome mit
            // fremder link_id bestaetigt die Kopplung einer anderen Instanz.
            if (! welcomeHaeltVertrag (felder, linkId, challenge, brokerEpoch, brokerVersion)
                || linkId != hello.linkId || challenge != hello.challenge)
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

    // Ratengrenze je Verbindung, dieselbe wie im Broker und im ControlClient
    // (§33.1 "Parser erhalten ... Nachrichtenratenlimits").
    Ratengrenze rate (kRateProSekunde, kRateFensterMs);
    const auto rateBeginn = std::chrono::steady_clock::now();

    std::vector<std::uint8_t> ausgang;
    std::vector<std::uint8_t> frame (schleuse.slotGroesse());
    while (! sollAbbrechen (generation))
    {
        // ── 1) Gehoert diese Pipe noch zur aktuellen Kopplung? ────────────
        //
        // Die Kopplung ist eine Eigenschaft der CONTROL-Verbindung. Wird sie
        // getrennt oder neu aufgebaut, gilt eine neue `link_id`/`challenge`,
        // und der Broker schliesst die alte Telemetriepipe. Bei leerer
        // Schleuse merkte die alte Fassung davon NICHTS: sie las nicht und
        // verglich nicht, blieb unbegrenzt als `verbunden` sichtbar und
        // koppelte erst nach einer spaeteren Veroeffentlichung mit
        // gescheitertem Write neu (T2-Befund 2 vom 2026-08-29).
        if (helloProvider)
        {
            const TelemetryHello jetzt = helloProvider();
            if (jetzt.linkId != hello.linkId || jetzt.challenge != hello.challenge)
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler = "Kopplung gewechselt — Telemetrie koppelt neu";
                ++zustand.kopplungswechsel;
                break;
            }
        }

        // ── 2) Frisches Material senden ───────────────────────────────────
        const std::size_t n = schleuse.abholen (frame.data(), frame.size());
        if (n == 0)
        {
            // Leerlauf: LESEN statt schlafen. Dieselbe Frist, dieselbe
            // Groessenordnung an Syscalls — aber ein Schlaf kann keinen
            // Pipe-Abschluss sehen, ein fristbegrenztes Lesen schon. `stop`
            // und `reconnect` brechen es ueber `ioAbbrechen` sofort ab, genau
            // wie sie vorher die Condvar weckten.
            if (! leerlaufLesen (leser, rate, rateBeginn, generation, verbindung))
                break;
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
