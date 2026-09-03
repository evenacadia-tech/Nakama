// K1 (S8/SONDE-007a): der Kompilier-Riegel steht am Anfang UND am Ende jeder
// Kern-Uebersetzungseinheit. Diese hier sieht zwar gar kein JUCE, aber die
// Regel gilt fuer die MENGE, nicht fuer den Einzelfall - eine Ausnahme waere
// die Stelle, an der die naechste Quelle sie stillschweigend erbt.
#include "NakamaKernRiegel.h"

#include "TelemetryClient.h"
#include "WireEnvelope.h"
#include "../analysis/FeatureEngine.h"
#include "../../vertrag/generiert/nakama_telemetry_v1_generated.h"

#include <algorithm>
#include <chrono>
#include <cmath>

namespace nakama::ipc
{
namespace
{
namespace fb = ::nakama::v3;
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

bool featureFrameAlsFlatbuffer (const analyse::FeatureFrame& frame,
                                const Adresse& quelle,
                                std::vector<std::uint8_t>& ausgabe)
{
    ausgabe.clear();
    Adresse wireQuelle = quelle;
    wireQuelle.instanceId = instanceAdresseAusState (quelle.instanceId);
    if (! adresseGueltig (wireQuelle) || frame.metricsVersion < 1u
        || analyse::nak29Verstoss (frame.transport) != 0
        || ! frame.transport.process_context_present_gesetzt)
        return false;

    bool stereoVorhanden = false;
    for (int i = 0; i < analyse::Gitter::liveBaender; ++i)
    {
        if ((frame.liveBreiteBitmap[i / 8] & (1u << (i % 8))) == 0)
            continue;
        stereoVorhanden = true;
        if (! std::isfinite (frame.liveBreite[i])
            || frame.liveBreite[i] < 0.0f || frame.liveBreite[i] > 1.0f)
            return false;
    }

    const bool lufsIHalb = frame.lufsIGesetzt != frame.lufsIUnsicherheitGesetzt;
    const bool lufsIPaar = frame.lufsIGesetzt && frame.lufsIUnsicherheitGesetzt;
    if (lufsIHalb
        || (lufsIPaar && (! std::isfinite (frame.lufsI)
                          || ! std::isfinite (frame.lufsIUnsicherheit)))
        || (frame.lufsIStatusGesetzt
            && (frame.lufsIStatus < 1 || frame.lufsIStatus > 2 || lufsIPaar)))
        return false;

    flatbuffers::FlatBufferBuilder b (4096);
    const auto sid = b.CreateString (wireQuelle.logonSid);
    const auto projekt = b.CreateString (wireQuelle.projectBindingId);
    const auto epoche = b.CreateString (wireQuelle.sessionEpoch);
    const auto instanz = b.CreateString (wireQuelle.instanceId);
    const auto nonce = b.CreateString (wireQuelle.runtimeNonce);
    const auto adresse = fb::CreateAdresse (b, sid, projekt, epoche, instanz, nonce);

    flatbuffers::Offset<fb::Schleife> schleife;
    const auto& t = frame.transport;
    const bool schleifeVorhanden = t.cycle_active || t.cycle_bounds_valid
        || t.cycle_start_ppq_gesetzt || t.cycle_end_ppq_gesetzt
        || t.cycle_derivation != analyse::Herleitung::unbekannt;
    if (schleifeVorhanden)
    {
        flatbuffers::Optional<double> start = flatbuffers::nullopt;
        flatbuffers::Optional<double> ende = flatbuffers::nullopt;
        if (t.cycle_start_ppq_gesetzt) start = t.cycle_start_ppq;
        if (t.cycle_end_ppq_gesetzt) ende = t.cycle_end_ppq;
        schleife = fb::CreateSchleife (b, t.cycle_active, t.cycle_bounds_valid,
                                      start, ende, 0);
    }

    flatbuffers::Optional<std::int64_t> projektStart = flatbuffers::nullopt;
    flatbuffers::Optional<std::int64_t> kontinuierlich = flatbuffers::nullopt;
    flatbuffers::Optional<std::uint32_t> eingangLatenz = flatbuffers::nullopt;
    flatbuffers::Optional<std::uint32_t> ausgangLatenz = flatbuffers::nullopt;
    if (t.project_sample_start_gesetzt) projektStart = t.project_sample_start;
    if (t.continuous_time_samples_gesetzt) kontinuierlich = t.continuous_time_samples;
    if (t.input_presentation_latency_gesetzt) eingangLatenz = t.input_presentation_latency;
    if (t.output_presentation_latency_gesetzt) ausgangLatenz = t.output_presentation_latency;
    const auto transport = fb::CreateTransportstempel (
        b, t.transport_epoch, t.continuity_segment, t.sequence,
        static_cast<fb::Zeitbasis> (t.zeitbasis), projektStart,
        t.sample_count, t.sample_rate, t.playing, t.recording, kontinuierlich,
        schleife, eingangLatenz, ausgangLatenz,
        static_cast<fb::Gueltigkeit> (t.gueltigkeit), t.process_context_present);

    const auto liveWerte = b.CreateVector (frame.live.werte,
                                           analyse::Gitter::liveBaender);
    const auto liveBitmap = b.CreateVector (frame.live.bitmap,
                                             sizeof frame.live.bitmap);
    const auto live = fb::CreateBandwerte (
        b, static_cast<fb::Bandgitter> (frame.live.gitter),
        static_cast<fb::BandEncoding> (frame.live.encoding),
        liveWerte, 0, liveBitmap, frame.live.saturated);

    flatbuffers::Offset<fb::Bandwerte> stereo;
    if (stereoVorhanden)
    {
        const auto werte = b.CreateVector (frame.liveBreite,
                                           analyse::Gitter::liveBaender);
        const auto bitmap = b.CreateVector (frame.liveBreiteBitmap,
                                             sizeof frame.liveBreiteBitmap);
        stereo = fb::CreateBandwerte (b, fb::Bandgitter::nakama_log64_v1,
                                      fb::BandEncoding::float32, 0, werte,
                                      bitmap, false);
    }

    auto optional = [] (bool gesetzt, float wert) -> flatbuffers::Optional<float>
    { return gesetzt ? flatbuffers::Optional<float> (wert) : flatbuffers::nullopt; };
    const auto lufsIStatus = frame.lufsIStatusGesetzt
        ? flatbuffers::Optional<std::uint8_t> (frame.lufsIStatus)
        : flatbuffers::nullopt;
    // NAK-68: nur ein GESETZTES Feld reist. Ein ungesetzter Rahmen laesst es
    // weg, statt 0 zu senden - 0 hiesse "ueber nichts integriert" und faellt
    // bei beiden Lesern.
    const auto integration = frame.integrationGesetzt
        ? flatbuffers::Optional<std::uint32_t> (frame.integrationSamples)
        : flatbuffers::nullopt;
    const auto frameFb = fb::CreateFrame (
        b, transport, live, frame.metricsVersion,
        optional (frame.aktivitaetGesetzt, frame.aktivitaet),
        optional (frame.lufsSGesetzt, frame.lufsS),
        optional (frame.peakGesetzt, frame.peakDb),
        optional (frame.crestGesetzt, frame.crestDb),
        optional (frame.psrGesetzt, frame.psrDb),
        optional (frame.breiteGesetzt, frame.breite),
        optional (frame.korrelationGesetzt, frame.korrelation), stereo,
        optional (frame.lufsIGesetzt, frame.lufsI),
        optional (frame.lufsIUnsicherheitGesetzt, frame.lufsIUnsicherheit),
        lufsIStatus, integration);
    const auto eintrag = fb::CreateQuellenEintrag (b, adresse, frameFb);
    const auto eintraege = b.CreateVector (&eintrag, 1);
    const auto batch = fb::CreateFeatureBatch (b, eintraege);
    fb::FinishFeatureBatchBuffer (b, batch);
    ausgabe.assign (b.GetBufferPointer(), b.GetBufferPointer() + b.GetSize());
    return true;
}

//== Die geteilte Laufzeit ===================================================
struct TelemetryClient::Laufzeit
{
    Laufzeit (std::function<TelemetryHello()> hp, std::string pn,
               std::function<void (const std::uint8_t*, std::size_t,
                                   std::uint8_t)> bf,
               ServerErwartung se)
        : helloProvider (std::move (hp)), beiFrame (std::move (bf)),
          pipeName (std::move (pn)), serverErwartung (std::move (se)) {}

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
    std::function<void (const std::uint8_t*, std::size_t, std::uint8_t)> beiFrame;
    std::string pipeName;
    ServerErwartung serverErwartung;

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
                                  std::string pipeNameIn,
                                  std::function<void (const std::uint8_t*, std::size_t,
                                                      std::uint8_t)> beiFrameIn,
                                  ServerErwartung serverErwartungIn)
    : k (std::make_shared<Laufzeit> (std::move (helloProviderIn),
                                     std::move (pipeNameIn),
                                     std::move (beiFrameIn),
                                     std::move (serverErwartungIn)))
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
        k->zustand.serverPruefstatus = ServerPruefStatus::nichtGeprueft;
        k->zustand.serverPrueffehler = ServerPruefFehler::keiner;
        k->zustand.serverPid = 0;
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
                k->zustand.serverPruefstatus = ServerPruefStatus::nichtGeprueft;
                k->zustand.serverPrueffehler = ServerPruefFehler::keiner;
                k->zustand.serverPid = 0;
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
    k->zustand.serverPruefstatus = ServerPruefStatus::nichtGeprueft;
    k->zustand.serverPrueffehler = ServerPruefFehler::keiner;
    k->zustand.serverPid = 0;
}

void TelemetryClient::reconnect()
{
    {
        std::lock_guard<std::mutex> z (k->zustandMutex);
        // Die Generation wechselt unter derselben Sperre wie der sichtbare
        // Auth-Zustand; ein alter Prueflauf kann den Reset damit nicht spaeter
        // wieder ueberschreiben.
        k->verbindungsGeneration.fetch_add (1);
        k->zustand.serverPruefstatus = ServerPruefStatus::nichtGeprueft;
        k->zustand.serverPrueffehler = ServerPruefFehler::keiner;
        k->zustand.serverPid = 0;
    }
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

bool TelemetryClient::veroeffentlichen (const analyse::FeatureFrame& frame,
                                        const Adresse& quelle)
{
    std::vector<std::uint8_t> puffer;
    return featureFrameAlsFlatbuffer (frame, quelle, puffer)
        && veroeffentlichen (puffer.data(), puffer.size());
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
        TelemetryHello hello = helloProvider ? helloProvider() : TelemetryHello();
        hello.adresse.instanceId = instanceAdresseAusState (hello.adresse.instanceId);
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

        bool authBlockiert = false;
        {
            std::lock_guard<std::mutex> z (zustandMutex);
            authBlockiert = zustand.serverPruefstatus
                         == ServerPruefStatus::belegtAberUnverifiziert;
        }
        if (authBlockiert)
        {
            std::unique_lock<std::mutex> l (wartemutex);
            warte.wait (l, [this, generation] {
                return ! laeuft.load()
                    || verbindungsGeneration.load() != generation;
            });
            backoffMs = kBackoffStartMs;
            continue;
        }

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

        // Der Payloadzeiger gilt ausschliesslich waehrend des Callbacks. Das
        // Main-Modell kopiert die typisierten Felder; generierte
        // FlatBuffers-Zeiger verlassen den Pipe-Thread nie.
        if (beiFrame && ! sollAbbrechen (generation))
            beiFrame (e.payload, e.payloadLaenge, e.kopf.schemaMinor);
        if (sollAbbrechen (generation))
            return false;
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            ++zustand.empfangen;
        }
    }
}

bool TelemetryClient::Laufzeit::eineVerbindung (std::uint64_t generation,
                                                std::uint64_t meinLauf,
                                                const TelemetryHello& hello,
                                                IpcVerbindung& verbindung)
{
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        if (sollAbbrechen (generation))
            return false;
        zustand.status = Status::verbindet;
        zustand.serverPruefstatus = ServerPruefStatus::nichtGeprueft;
        zustand.serverPrueffehler = ServerPruefFehler::keiner;
        zustand.serverPid = 0;
        ++zustand.verbindungsVersuche;
    }

    if (! adresseGueltig (hello.adresse))
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        if (sollAbbrechen (generation))
            return false;
        zustand.status = Status::getrennt;
        zustand.letzterFehler = "Adresse haelt den v3-Vertrag nicht (hex32/SID)";
        return false;
    }

    std::string fehler;
    ServerPruefBericht serverBericht;
    // NAK-134 Nacharbeit Runde 1, R5 — dieselbe Reihenfolge wie in
    // `ControlClient.cpp`: Abbruchsignal loesen, Generation ERNEUT lesen, dann
    // oeffnen. Begruendung dort; sie gilt hier unveraendert, weil beide
    // v3-Clients denselben `IpcVerbindung::oeffnen` fahren und `stop()` wie
    // `reconnect()` auch hier die Generation vor `ioAbbrechen()` erhoehen.
    verbindung.neueGenerationBeginnen();
    if (sollAbbrechen (generation))
        return false;
    const bool serverGeoeffnet = verbindung.oeffnen (
        pipeName, serverErwartung, serverBericht, fehler);
    bool veralteteGeneration = false;
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        veralteteGeneration = sollAbbrechen (generation);
        if (! veralteteGeneration)
        {
            if (serverBericht.status == ServerPruefStatus::verifiziert
                || serverBericht.status == ServerPruefStatus::belegtAberUnverifiziert)
                ++zustand.serverPruefungen;

            if (! serverGeoeffnet)
            {
                zustand.status = Status::getrennt;
                zustand.serverPruefstatus = serverBericht.status;
                zustand.serverPrueffehler = serverBericht.fehler;
                zustand.serverPid = serverBericht.serverPid;
                zustand.letzterFehler = fehler;
            }
            else
            {
                zustand.serverPruefstatus = ServerPruefStatus::verifiziert;
                zustand.serverPrueffehler = ServerPruefFehler::keiner;
                zustand.serverPid = serverBericht.serverPid;
            }
        }
    }
    if (veralteteGeneration)
    {
        // Wie beim Controlpfad gehoert das Urteil genau dieser Generation.
        // Ein waehrend der Pruefung angeforderter Reconnect verwirft es samt
        // Handle, bevor Status oder Bootstrapbytes sichtbar werden.
        if (serverGeoeffnet)
            verbindung.schliessen();
        return false;
    }
    if (! serverGeoeffnet)
        return false;

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
        zustand.serverPruefstatus = ServerPruefStatus::nichtGeprueft;
        zustand.serverPrueffehler = ServerPruefFehler::keiner;
        zustand.serverPid = 0;
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
        if (! envelopeSchreiben (Familie::p2, kFeatureBatchSchemaMinor,
                                 frame.data(), n, ausgang)
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
        zustand.serverPruefstatus = ServerPruefStatus::nichtGeprueft;
        zustand.serverPrueffehler = ServerPruefFehler::keiner;
        zustand.serverPid = 0;
    }
    return true;
}

} // namespace nakama::ipc

// K1 erneut: die zweite Auswertung sieht Makros, die erst ein spaeter
// eingebundener Header definiert haben koennte.
#include "NakamaKernRiegel.h"
