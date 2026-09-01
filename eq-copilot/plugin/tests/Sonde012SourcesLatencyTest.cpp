// SONDE-012 L06 — synthetisches Messfenster -> echter Rust-Coordinator auf
// Probe-Pipe -> echtes Main-SourcesModel -> revisionsbasierte Anzeige-Wache.

#include "ControlClient.h"
#include "NakamaTelemetrie.h"
#include "PipeToken.h"
#include "SourcesModel.h"
#include "TelemetryClient.h"
#include "analysis/FeatureEngine.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

using namespace nakama::ipc;
using Uhr = std::chrono::steady_clock;

namespace
{
std::string hex32 (unsigned long long n)
{
    std::ostringstream s;
    s << std::hex << std::nouppercase << std::setfill ('0') << std::setw (32) << n;
    return s.str();
}

Adresse adresse (unsigned instanz, unsigned nonce)
{
    return { "S-1-5-21-1-2-3-1001", hex32 (1), hex32 (2),
             hex32 (instanz), hex32 (nonce) };
}

ControlStatus status()
{
    ControlStatus s;
    s.runtime.gemeldet = true;
    s.runtime.messpunkt = "insert";
    s.runtime.betrieb = "active";
    return s;
}

nakama::analyse::FeatureFrame frame (std::uint64_t sequence, std::uint32_t samples)
{
    nakama::analyse::FeatureFrame f;
    f.transport.transport_epoch = 1;
    f.transport.continuity_segment = 1;
    f.transport.sequence = sequence;
    f.transport.zeitbasis = nakama::analyse::Zeitbasis::local_monotonic;
    f.transport.sample_count = samples;
    f.transport.sample_rate = 48000.0;
    f.transport.process_context_present_gesetzt = true;
    f.transport.process_context_present = false;
    f.live.gitter = nakama::analyse::GitterId::nakama_log64_v1;
    f.live.encoding = nakama::analyse::BandEncoding::q_db_0p1_i16;
    for (int i = 0; i < nakama::analyse::Gitter::liveBaender; ++i)
    {
        f.live.werte[i] = -180;
        f.live.bitmap[i / 8] = static_cast<std::uint8_t> (
            f.live.bitmap[i / 8] | (1u << (i % 8)));
    }
    f.metricsVersion = 1;
    f.lufsIStatusGesetzt = true;
    f.lufsIStatus = 1;
    return f;
}

struct Quelle
{
    Adresse adresse;
    std::unique_ptr<ControlClient> control;
    std::unique_ptr<TelemetryClient> telemetry;
    std::uint64_t sequence = 0;
};

struct Messlauf
{
    explicit Messlauf (std::string p, int anzahl, ServerErwartung erwartung)
        : pipe (std::move (p)), n (anzahl), serverErwartung (std::move (erwartung)) {}

    std::string pipe;
    int n;
    ServerErwartung serverErwartung;
    eqcop::SourcesModel model;
    std::unique_ptr<ControlClient> mainControl;
    std::unique_ptr<TelemetryClient> mainTelemetry;
    std::vector<std::unique_ptr<Quelle>> quellen;
    std::mutex messMutex;
    std::map<std::pair<std::string, std::uint64_t>, Uhr::time_point> ersteSamples;
    std::vector<double> latenzMs;
    std::atomic<std::uint64_t> anzeigeInvalidierungen { 0 };
    std::mutex diagnoseMutex;
    juce::String letzterSnapshotFehler;

    bool starten()
    {
        const auto mainAdresse = adresse (10, 100);
        mainControl = std::make_unique<ControlClient> (
            [mainAdresse] {
                ControlHello h;
                h.adresse = mainAdresse;
                h.pluginKind = "main";
                h.hostAngeben = true;
                h.hostPid = 4242;
                return h;
            }, pipe,
            [this] (const std::string& json) {
                juce::String grund;
                const auto ergebnis = model.uebernehmeSessionSnapshot (json, Uhr::now(), grund);
                if (ergebnis == eqcop::SourcesModel::SnapshotErgebnis::ungueltig)
                {
                    std::lock_guard<std::mutex> l (diagnoseMutex);
                    letzterSnapshotFehler = grund;
                }
            }, [] { return status(); },
            [this, mainAdresse] (bool verbunden) {
                if (! verbunden)
                {
                    model.controlEnde();
                    return;
                }
                model.beginneSubscription (mainAdresse.projectBindingId,
                                            mainAdresse.sessionEpoch,
                                            mainAdresse.instanceId);
                const auto subscribe = std::string ("{\"type\":\"subscribe_session\",\"adresse\":")
                    + adresseAlsJson (mainAdresse) + ",\"session_epoch\":\""
                    + mainAdresse.sessionEpoch + "\"}";
                const auto ergebnis = mainControl->sendeP1 ("subscribe_session", subscribe);
                if (ergebnis == P1Ergebnis::abgewiesen || ergebnis == P1Ergebnis::zuGross)
                    model.controlEnde();
            },
            std::function<void (const std::string&, std::uint8_t)> {},
            serverErwartung);
        mainTelemetry = std::make_unique<TelemetryClient> (
            [this, mainAdresse] {
                TelemetryHello h;
                h.adresse = mainAdresse;
                mainControl->kopplung (h.linkId, h.challenge);
                return h;
            }, pipe,
            [this] (const std::uint8_t* daten, std::size_t laenge, std::uint8_t minor) {
                std::vector<nakama::telemetrie::Empfangsframe> gelesen;
                juce::Array<nakama::telemetrie::Verstoss> verstoesse;
                if (! nakama::telemetrie::lese (daten, laenge, gelesen, verstoesse)) return;
                const auto vorher = model.sicht().revision;
                juce::String grund;
                if (! model.uebernehmeP2 (daten, laenge, minor, Uhr::now(), grund)) return;
                const auto nachher = model.sicht().revision;
                if (nachher <= vorher) return;
                anzeigeInvalidierungen.fetch_add (1);
                const auto jetzt = Uhr::now();
                std::lock_guard<std::mutex> l (messMutex);
                for (const auto& f : gelesen)
                {
                    const auto it = ersteSamples.find ({ f.instanceId, f.sequence });
                    if (it == ersteSamples.end()) continue;
                    latenzMs.push_back (std::chrono::duration<double, std::milli> (
                        jetzt - it->second).count());
                    ersteSamples.erase (it);
                }
            }, serverErwartung);

        mainControl->start();
        mainTelemetry->start();
        for (int i = 0; i < n; ++i)
        {
            auto q = std::make_unique<Quelle>();
            q->adresse = adresse (100 + static_cast<unsigned> (i),
                                  1000 + static_cast<unsigned> (i));
            auto* roh = q.get();
            q->control = std::make_unique<ControlClient> (
                [roh] {
                    ControlHello h;
                    h.adresse = roh->adresse;
                    h.pluginKind = "active_probe";
                    h.hostAngeben = true;
                    h.hostPid = 4242;
                    return h;
                }, pipe, std::function<void (const std::string&)>{}, [] { return status(); },
                std::function<void (bool)> {},
                std::function<void (const std::string&, std::uint8_t)> {},
                serverErwartung);
            q->telemetry = std::make_unique<TelemetryClient> (
                [roh] {
                    TelemetryHello h;
                    h.adresse = roh->adresse;
                    roh->control->kopplung (h.linkId, h.challenge);
                    return h;
                }, pipe,
                std::function<void (const std::uint8_t*, std::size_t, std::uint8_t)> {},
                serverErwartung);
            q->control->start();
            q->telemetry->start();
            quellen.push_back (std::move (q));
        }

        const auto frist = Uhr::now() + std::chrono::seconds (25);
        while (Uhr::now() < frist)
        {
            bool clients = mainControl->snapshot().status == ControlClient::Status::verbunden
                && mainTelemetry->snapshot().status == TelemetryClient::Status::verbunden;
            for (const auto& q : quellen)
                clients = clients
                    && q->control->snapshot().status == ControlClient::Status::verbunden
                    && q->telemetry->snapshot().status == TelemetryClient::Status::verbunden;
            const auto sicht = model.sicht();
            const bool descriptors = sicht.quellen.size() == static_cast<std::size_t> (n)
                && std::all_of (sicht.quellen.begin(), sicht.quellen.end(),
                               [] (const auto& q) { return q.descriptorVorhanden; });
            if (clients && descriptors) return true;
            std::this_thread::sleep_for (std::chrono::milliseconds (10));
        }
        int control = 0, telemetrie = 0;
        for (const auto& q : quellen)
        {
            control += q->control->snapshot().status == ControlClient::Status::verbunden;
            telemetrie += q->telemetry->snapshot().status == TelemetryClient::Status::verbunden;
        }
        std::lock_guard<std::mutex> l (diagnoseMutex);
        const auto mainSicht = mainControl->snapshot();
        const auto sourceSicht = quellen.front()->control->snapshot();
        const auto modellSicht = model.sicht();
        std::cerr << "Diagnose: sources=" << model.sicht().quellen.size()
                  << " subscribed=" << modellSicht.subscriptionAktiv
                  << " source-control=" << control << "/" << n
                  << " source-telemetry=" << telemetrie << "/" << n
                  << " main-control=" << static_cast<int> (mainSicht.status)
                  << " main-telemetry=" << static_cast<int> (mainTelemetry->snapshot().status)
                  << " main-sent=" << mainSicht.p0Gesendet
                  << " main-recv=" << mainSicht.empfangen
                  << " source-sent=" << sourceSicht.p0Gesendet
                  << " source-recv=" << sourceSicht.empfangen
                  << " snapshot='" << letzterSnapshotFehler << "'\n";
        return false;
    }

    void stoppen()
    {
        for (auto& q : quellen) { q->telemetry->stop(); q->control->stop(); }
        if (mainTelemetry) mainTelemetry->stop();
        if (mainControl) mainControl->stop();
    }

    double messen (std::uint32_t samples)
    {
        constexpr int runden = 4;
        const auto startAnzahl = [&] {
            std::lock_guard<std::mutex> l (messMutex);
            return latenzMs.size();
        }();
        for (int r = 0; r < runden; ++r)
        {
            const auto t0 = Uhr::now();
            {
                std::lock_guard<std::mutex> l (messMutex);
                for (auto& q : quellen)
                    ersteSamples[{ q->adresse.instanceId, ++q->sequence }] = t0;
            }
            const auto fenster = std::chrono::duration<double> (
                static_cast<double> (samples) / 48000.0);
            std::this_thread::sleep_for (fenster);
            for (auto& q : quellen)
            {
                const auto f = frame (q->sequence, samples);
                if (! q->telemetry->veroeffentlichen (f, q->adresse)) return 1.0e9;
            }
            const auto soll = startAnzahl + static_cast<std::size_t> ((r + 1) * n);
            const auto frist = Uhr::now() + std::chrono::seconds (5);
            while (Uhr::now() < frist)
            {
                bool fertig = false;
                {
                    std::lock_guard<std::mutex> l (messMutex);
                    fertig = latenzMs.size() >= soll;
                }
                if (fertig) break;
                std::this_thread::sleep_for (std::chrono::milliseconds (1));
            }
        }
        std::vector<double> werte;
        {
            std::lock_guard<std::mutex> l (messMutex);
            werte.assign (latenzMs.begin() + static_cast<std::ptrdiff_t> (startAnzahl),
                          latenzMs.end());
        }
        if (werte.size() != static_cast<std::size_t> (runden * n)) return 1.0e9;
        std::sort (werte.begin(), werte.end());
        const auto rang = static_cast<std::size_t> (std::ceil (0.95 * werte.size())) - 1;
        return werte[rang];
    }
};
} // namespace

int main (int argc, char** argv)
{
    if (argc != 2 || ! istProbePipename (argv[1]))
    {
        std::cerr << "VERWEIGERT: genau ein Pipename aus dem Probe-Namensraum ist Pflicht\n";
        return 3;
    }
    const auto brokerPfad = std::filesystem::absolute (
        std::filesystem::path (L"broker/target/release/eqcop-broker-sonde012-probe.exe"));
    const auto serverErwartung = serverErwartungFuerTestdatei (brokerPfad.wstring());
    if (serverErwartung.absoluterBrokerPfad.empty())
    {
        std::cerr << "VORAUSSETZUNG FEHLT: SONDE-012-Probe-Broker nicht hashbar\n";
        return 3;
    }
    int fehler = 0;
    std::map<int, std::map<int, double>> ergebnisse;
    for (const int n : { 16, 32 })
    {
        Messlauf lauf (argv[1], n, serverErwartung);
        if (! lauf.starten())
        {
            lauf.stoppen();
            std::cerr << "FEHLER: " << n << " Quellen wurden nicht vollstaendig sichtbar\n";
            return 1;
        }
        for (const int samples : { 2048, 4096, 16384 })
        {
            const auto p95 = lauf.messen (static_cast<std::uint32_t> (samples));
            ergebnisse[n][samples] = p95;
            const double grenze = samples == 16384 ? 750.0 : 300.0;
            const bool ok = p95 <= grenze;
            std::cout << (ok ? "  ok      " : "  FEHLER  ") << n << " Quellen / "
                      << samples << " Samples p95 " << p95 << " ms <= " << grenze
                      << " ms\n";
            if (! ok) ++fehler;
        }
        const auto erwartet = static_cast<std::uint64_t> (n * 4 * 3);
        if (lauf.anzeigeInvalidierungen.load() < erwartet)
        {
            std::cerr << "FEHLER: Anzeige-Revisionen " << lauf.anzeigeInvalidierungen.load()
                      << "/" << erwartet << "\n";
            ++fehler;
        }
        lauf.stoppen();
    }
    std::cout << "{\"p95_16_2048_ms\":" << ergebnisse[16][2048]
              << ",\"p95_16_4096_ms\":" << ergebnisse[16][4096]
              << ",\"p95_16_16384_ms\":" << ergebnisse[16][16384]
              << ",\"p95_32_2048_ms\":" << ergebnisse[32][2048]
              << ",\"p95_32_4096_ms\":" << ergebnisse[32][4096]
              << ",\"p95_32_16384_ms\":" << ergebnisse[32][16384] << "}\n";
    return fehler == 0 ? 0 : 1;
}
