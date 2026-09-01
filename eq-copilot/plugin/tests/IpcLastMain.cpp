// S14-15 / SONDE-010 — das Ende-zu-Ende-Lastbein (Kanon A22, C++-Haelfte).
//
// Es faehrt die ECHTEN C++-Clients gegen den ECHTEN Rust-Listener ueber eine
// PROBE-Pipe: zwei Sprachen, ein Draht. Der Gate-Text von §65 sagt
// "CRC/Fuzz/Backpressure/Reconnect ohne P0-Starvation" — dieses Programm macht
// aus dem letzten Halbsatz eine ZAHL:
//
//   * N Sonden fluten gleichzeitig Telemetrie (P2), so schnell, dass die
//     Schleuse mit Cap 2 nachweislich Frames ersetzt (= echter Rueckstau);
//   * WAEHRENDDESSEN sendet jede Sonde P0-Heartbeats und misst die Zeit bis
//     zum `heartbeat_ack`;
//   * gemessen wird: kein einziger P0-Frame geht verloren, und die
//     P0-Antwortlatenz bleibt beschraenkt.
//
// Der Aufrufer ist tools/eq-copilot/pruefe_ipc_last.py; der startet den
// Rust-Probe-Broker und fuehrt danach dieses Programm.
#include "ControlClient.h"
#include "IpcVerbindung.h"
#include "PipeToken.h"
#include "TelemetryClient.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace nakama::ipc;

namespace
{
/// Obergrenze der P0-Antwortlatenz unter vollem Rueckstau. Sie ist bewusst
/// grosszuegig: gemessen wird nicht die Schnelligkeit, sondern dass P0
/// UEBERHAUPT durchkommt, waehrend P2 flutet. Eine Starvation waere kein
/// "langsam", sondern ein "nie" — und faellt an dieser Schranke sofort auf.
constexpr long long kMaxP0LatenzMs = 1000;

std::string hex32Aus (unsigned wert, char praefix)
{
    std::string s (1, praefix);
    char puffer[16];
    std::snprintf (puffer, sizeof (puffer), "%07x", wert & 0xFFFFFFFu);
    s += puffer;
    while (s.size() < 32)
        s += '0';
    return s.substr (0, 32);
}

struct Sonde
{
    std::unique_ptr<ControlClient>   control;
    std::unique_ptr<TelemetryClient> telemetrie;
    std::mutex mutex;
    std::vector<std::chrono::steady_clock::time_point> gesendet;  // Index = Sequenz
    std::vector<long long> latenzen;
    std::atomic<std::uint64_t> beantwortet { 0 };
    std::atomic<std::uint64_t> gesendetAnzahl { 0 };
    std::string linkId, challenge;
};
} // namespace

int main (int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Aufruf: EqCopIpcLast <pipe-name> [sonden] [sekunden]" << std::endl;
        return 2;
    }
    const std::string pipe = argv[1];
    // ERLAUBNIS statt Sperrliste: die alte Fassung kannte nur den v1-Namen und
    // liess damit ausgerechnet den produktiven v3-Namensraum aus §48.3 durch
    // (T2-Befund 7 vom 2026-08-29).
    if (! istProbePipename (pipe))
    {
        std::cerr << "VERWEIGERT: " << pipe << " liegt nicht im Probe-Namensraum "
                  << kPipePraefixProbe << std::endl;
        return 3;
    }
    const int sonden   = argc > 2 ? std::atoi (argv[2]) : 32;
    const int sekunden = argc > 3 ? std::atoi (argv[3]) : 8;
    const auto brokerPfad = std::filesystem::absolute (
        std::filesystem::path (L"broker/target/release/eqcop-broker-v3probe.exe"));
    const auto serverErwartung = serverErwartungFuerTestdatei (brokerPfad.wstring());
    if (serverErwartung.absoluterBrokerPfad.empty())
    {
        std::cerr << "VORAUSSETZUNG FEHLT: Broker-Testimage nicht hashbar" << std::endl;
        return 3;
    }

    std::cout << "SONDE-010 | Lastbein: " << sonden << " Sonden, " << sekunden
              << " s, Pipe " << pipe << std::endl;

    std::vector<std::unique_ptr<Sonde>> alle;
    alle.reserve (static_cast<std::size_t> (sonden));

    for (int i = 0; i < sonden; ++i)
    {
        auto s = std::make_unique<Sonde>();
        Sonde* roh = s.get();

        Adresse a;
        a.logonSid = "S-1-5-21-1-2-3-1001";
        a.projectBindingId = hex32Aus (1u, 'a');
        a.sessionEpoch     = hex32Aus (2u, 'b');
        a.instanceId       = hex32Aus (static_cast<unsigned> (i), 'c');
        a.runtimeNonce     = hex32Aus (static_cast<unsigned> (i), 'd');

        s->control = std::make_unique<ControlClient> (
            [a] {
                ControlHello h;
                h.adresse = a;
                h.pluginKind = "active_probe";
                return h;
            },
            pipe,
            [roh] (const std::string& antwort) {
                std::vector<JsonFeld> felder;
                std::string typ, seqText;
                // `type` ist ein String, `sequence` eine ZAHL. Der Leser
                // unterscheidet das seit T2-Befund 3 vom 2026-08-29 wieder.
                if (! flachesJsonObjekt (antwort, felder)
                    || ! jsonText (felder, "type", typ) || typ != "heartbeat_ack"
                    || ! jsonLiteral (felder, "sequence", seqText))
                    return;
                const auto seq = static_cast<std::size_t> (std::strtoull (seqText.c_str(),
                                                                          nullptr, 10));
                const auto jetzt = std::chrono::steady_clock::now();
                std::lock_guard<std::mutex> l (roh->mutex);
                if (seq >= roh->gesendet.size())
                    return;
                roh->latenzen.push_back (
                    std::chrono::duration_cast<std::chrono::milliseconds> (
                        jetzt - roh->gesendet[seq]).count());
                roh->beantwortet.fetch_add (1);
            },
            std::function<ControlStatus()> {},
            std::function<void (bool)> {},
            std::function<void (const std::string&, std::uint8_t)> {},
            serverErwartung);

        s->telemetrie = std::make_unique<TelemetryClient> (
            [roh, a] {
                TelemetryHello t;
                t.adresse = a;
                std::string l, c;
                roh->control->kopplung (l, c);
                t.linkId = l;
                t.challenge = c;
                return t;
            },
            pipe,
            std::function<void (const std::uint8_t*, std::size_t, std::uint8_t)> {},
            serverErwartung);

        alle.push_back (std::move (s));
    }

    for (auto& s : alle) s->control->start();
    for (auto& s : alle) s->telemetrie->start();

    // Warten, bis alle Paare stehen.
    const auto frist = std::chrono::steady_clock::now() + std::chrono::seconds (20);
    int verbundenControl = 0, verbundenTelemetrie = 0;
    while (std::chrono::steady_clock::now() < frist)
    {
        verbundenControl = 0;
        verbundenTelemetrie = 0;
        for (auto& s : alle)
        {
            if (s->control->snapshot().status == ControlClient::Status::verbunden)
                ++verbundenControl;
            if (s->telemetrie->snapshot().status == TelemetryClient::Status::verbunden)
                ++verbundenTelemetrie;
        }
        if (verbundenControl == sonden && verbundenTelemetrie == sonden)
            break;
        std::this_thread::sleep_for (std::chrono::milliseconds (20));
    }
    std::cout << "  verbunden: " << verbundenControl << " Control, "
              << verbundenTelemetrie << " Telemetrie" << std::endl;
    if (verbundenControl != sonden || verbundenTelemetrie != sonden)
    {
        for (auto& s : alle) { s->telemetrie->stop(); s->control->stop(); }
        std::cerr << "FEHLER: nicht alle Paare gekoppelt" << std::endl;
        return 1;
    }

    // ── Flut: ein Erzeuger bedient alle Schleusen der Reihe nach ──────────
    std::atomic<bool> laeuft { true };
    std::atomic<std::uint64_t> veroeffentlicht { 0 }, abgelehnt { 0 };
    std::vector<std::uint8_t> frame (512, 0x5A);
    std::thread fluter ([&] {
        while (laeuft.load())
        {
            for (auto& s : alle)
            {
                // Die RUECKGABE zaehlt mit. Sie ist die einzige Stelle, an der
                // ein nicht uebernommener Frame ueberhaupt sichtbar wird; sie
                // zu verwerfen hiess, den Verlust nicht messen zu koennen
                // (T2-Befund 2 Runde 3, A22-Haelfte).
                if (! s->telemetrie->veroeffentlichen (frame.data(), frame.size()))
                    abgelehnt.fetch_add (1);
                veroeffentlicht.fetch_add (1);
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (1));
        }
    });

    // ── Gleichzeitig P0: jede Sonde alle 25 ms ────────────────────────────
    std::thread befehler ([&] {
        while (laeuft.load())
        {
            for (auto& s : alle)
            {
                std::size_t seq;
                {
                    std::lock_guard<std::mutex> l (s->mutex);
                    seq = s->gesendet.size();
                    s->gesendet.push_back (std::chrono::steady_clock::now());
                }
                const std::string json = "{\"type\":\"heartbeat\",\"sequence\":"
                                       + std::to_string (seq) + "}";
                if (s->control->sendeP0 (json))
                    s->gesendetAnzahl.fetch_add (1);
                else
                {
                    std::lock_guard<std::mutex> l (s->mutex);
                    s->gesendet.pop_back();  // nie eingereiht ⇒ nie erwartet
                }
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (25));
        }
    });

    std::this_thread::sleep_for (std::chrono::seconds (sekunden));
    laeuft.store (false);
    fluter.join();
    befehler.join();

    // Nachlauf: ausstehende ACKs duerfen noch ankommen.
    std::this_thread::sleep_for (std::chrono::milliseconds (1500));

    std::uint64_t p0Gesendet = 0, p0Beantwortet = 0, p2Ersetzt = 0, p2Gesendet = 0;
    std::uint64_t envelopeAbweisungen = 0, p0Ueberlaeufe = 0;
    std::uint64_t p2ZuGross = 0, p2Loecher = 0, p2NeuesteVerworfen = 0;
    long long maxLatenz = 0;
    std::vector<long long> alleLatenzen;
    for (auto& s : alle)
    {
        p0Gesendet    += s->gesendetAnzahl.load();
        p0Beantwortet += s->beantwortet.load();
        const auto cs = s->control->snapshot();
        const auto ts = s->telemetrie->snapshot();
        p2Ersetzt += ts.ersetzt;
        p2Gesendet += ts.gesendet;
        p2ZuGross += ts.zuGross;
        p2Loecher += ts.kollisionsLoecher;
        p2NeuesteVerworfen += ts.beanspruchtVerworfen;
        envelopeAbweisungen += cs.envelopeAbweisungen + ts.envelopeAbweisungen;
        p0Ueberlaeufe += cs.p0Ueberlaeufe;
        std::lock_guard<std::mutex> l (s->mutex);
        for (auto w : s->latenzen)
        {
            alleLatenzen.push_back (w);
            maxLatenz = std::max (maxLatenz, w);
        }
    }
    std::sort (alleLatenzen.begin(), alleLatenzen.end());
    const long long p99 = alleLatenzen.empty()
        ? 0
        : alleLatenzen[static_cast<std::size_t> (
              static_cast<double> (alleLatenzen.size() - 1) * 0.99)];

    for (auto& s : alle) { s->telemetrie->stop(); s->control->stop(); }

    std::cout << "{\"sonden\":" << sonden
              << ",\"sekunden\":" << sekunden
              << ",\"p2_veroeffentlicht\":" << veroeffentlicht.load()
              << ",\"p2_gesendet\":" << p2Gesendet
              << ",\"p2_ersetzt\":" << p2Ersetzt
              << ",\"p2_abgelehnt\":" << abgelehnt.load()
              << ",\"p2_zu_gross\":" << p2ZuGross
              << ",\"p2_kollisionsloecher\":" << p2Loecher
              << ",\"p2_neueste_verworfen\":" << p2NeuesteVerworfen
              << ",\"p0_gesendet\":" << p0Gesendet
              << ",\"p0_beantwortet\":" << p0Beantwortet
              << ",\"p0_latenz_max_ms\":" << maxLatenz
              << ",\"p0_latenz_p99_ms\":" << p99
              << ",\"p0_ueberlaeufe\":" << p0Ueberlaeufe
              << ",\"envelope_abweisungen\":" << envelopeAbweisungen
              << "}" << std::endl;

    int fehler = 0;
    auto pruefe = [&fehler] (bool ok, const char* text, const std::string& detail) {
        std::cout << (ok ? "  ok      " : "  FEHLER  ") << text << "  [" << detail << "]"
                  << std::endl;
        if (! ok) ++fehler;
    };

    pruefe (p0Gesendet > 0, "P0-Befehle wurden ueberhaupt gesendet",
            std::to_string (p0Gesendet));
    pruefe (p0Beantwortet == p0Gesendet, "KEIN P0-Frame geht verloren",
            std::to_string (p0Beantwortet) + "/" + std::to_string (p0Gesendet));
    pruefe (maxLatenz <= kMaxP0LatenzMs, "P0-Antwortlatenz bleibt beschraenkt",
            "max " + std::to_string (maxLatenz) + " ms, p99 " + std::to_string (p99)
                + " ms, Schranke " + std::to_string (kMaxP0LatenzMs) + " ms");
    pruefe (p2Ersetzt > 0, "es lag WIRKLICH Rueckstau an (Cap 2 hat ersetzt)",
            std::to_string (p2Ersetzt) + " ersetzte P2-Frames");
    // Rueckstau heisst "der aelteste weicht" — nie "der neueste faellt".
    // Unter echter Flut ueber eine echte Pipe laeuft der Erzeuger auf den
    // Platz des Telemetriethreads; er ueberspringt die Position, statt den
    // gerade erzeugten Frame zu opfern (T2-Befund 2 Runde 3).
    pruefe (p2NeuesteVerworfen == 0,
            "und dabei faellt NIE der neueste Frame (replace-oldest)",
            std::to_string (p2NeuesteVerworfen) + " neueste verworfen, "
                + std::to_string (p2Loecher) + " Positionen uebersprungen");
    // Jede Ablehnung hat einen benannten Grund. Ohne diese Gleichung koennte
    // ein `false` aus einem Grund kommen, den niemand zaehlt.
    pruefe (abgelehnt.load() == p2ZuGross + p2NeuesteVerworfen,
            "jede abgelehnte Veroeffentlichung hat einen gezaehlten Grund",
            std::to_string (abgelehnt.load()) + " abgelehnt = "
                + std::to_string (p2ZuGross) + " zu gross + "
                + std::to_string (p2NeuesteVerworfen) + " ohne Platz");
    pruefe (envelopeAbweisungen == 0, "kein Envelope wurde abgewiesen",
            std::to_string (envelopeAbweisungen));
    pruefe (p0Ueberlaeufe == 0, "keine P0-Queue lief ueber",
            std::to_string (p0Ueberlaeufe));

    std::cout << (fehler == 0 ? "LASTBEIN GRUEN" : "LASTBEIN ROT") << std::endl;
    return fehler == 0 ? 0 : 1;
}
