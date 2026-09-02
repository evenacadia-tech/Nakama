// G3-SOAK — die C++-Haelfte des Dauerlaufbeins A24 (Kanon A24).
//
// Vorbilder: A22 `tests/IpcLastMain.cpp` (Sondenpaare, P0-Latenz, Rueckstau),
// A23 `tests/Sonde012SourcesLatencyTest.cpp` (Main + subscribe_session +
// echtes SourcesModel), B12 `tests/Sonde012LoudnessSourceTest.cpp` (echter
// SondeProcessor am Ganzblockpfad).
//
// Kein Kanon-Bein fuer sich: `tools/eq-copilot/pruefe_session_soak.py` besitzt
// Prozesslebenszyklus, Pipenamen-Riegel, Brokerneustart und Speicherkurve.
// Dieses Programm besitzt die Topologie, die Zaehler und das Urteil ueber die
// Zusagen Z1, Z2, Z4, Z5, Z6, Z7 und die Killpunkte K-S1 bis K-S5.
//
// Verhaltensmatrix und Killmatrix stehen in `docs/beweise/G3-SOAK.md`; jeder
// Pruefpunkt `A24:<name>` dort hat hier eine Zeile in `urteile()`.
//
// PROTOKOLL mit dem Pruefer
// -------------------------
// stdout (dieses Programm -> Pruefer):
//   TOPOLOGIE_STEHT <ms>            Zeile S01
//   MAIN_SUBSCRIBE_GESENDET         Barriere K-S2
//   LANGSAM_IN_VERZOEGERUNG         Barriere K-S4
//   WARMUP_FERTIG                   Zeile S02
//   NEUSTART_VOLLSTAENDIG <i> <ms>  Zeile S11
//   {json}                          Schlussbericht
// stdin (Pruefer -> dieses Programm):
//   KILL_VORBEREITEN <i>   Vorzustand je Client festhalten -> KILL_BEREIT <i>
//   BARRIERE <art>         Rueckstau (K-S1) und lange Verzoegerung (K-S4) an
//   BARRIERE_AUS           beide wieder aus
//   MAIN_RECONNECT         erzwingt ein frisches subscribe_session (K-S2)
//   KILL_GESCHEHEN <i>     der Brokerprozess ist tot, Neustartfenster laeuft
//   BEREIT <i> <totzeit>   der neue Broker horcht; ab hier laeuft die Frist
//   ENDE                   Messzeit vorbei, Bericht drucken

#include "ControlClient.h"
#include "PipeToken.h"
#include "SondeProcessor.h"
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
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace nakama::ipc;
using Uhr = std::chrono::steady_clock;

namespace
{
// ── Konstanten, alle aus der Quelle abgeleitet (Manifest §3) ─────────────
/// Obere Schranke je Clientpaar aus Manifest §3.1: Control-Runde 22.000 +
/// Kopplung 51 + Telemetrie-Runde 22.000 + Heartbeat-Kadenz 1.000.
constexpr long long kSchrankeMs = 45051;
/// Festgelegte Frist Z2 (Manifest §3.1): Schranke plus benannter Zuschlag.
constexpr long long kFristMs = 60000;
/// P0-Antwortlatenz, uebernommen von A22 (`IpcLastMain.cpp:42`).
constexpr long long kP0SchrankeMs = 1000;
/// Vertragskadenz des Heartbeats (`broker/src/coordinator.rs:28`).
constexpr int kHeartbeatMs = 1000;
/// Liveframes je Sonde (Manifest §2.1).
constexpr int kLiveTaktMs = 100;
/// Grossfenster je Sonde.
constexpr int kGrossTaktMs = 1000;
constexpr std::uint32_t kLiveSamples = 2048;
constexpr std::uint32_t kGrossSamples = 16384;
/// Kuenstliche Verzoegerung eines langsamen Lesers in seinem Callback.
int gLangsamMs = 120;   ///< ueber die Kommandozeile steuerbar (Diagnose)
/// Eigener Sequenzraum fuer die MESSENDEN Heartbeats dieses Beins.
///
/// Der produktive `ControlClient` sendet mit gesetztem statusProvider seine
/// EIGENEN 1-Hz-Heartbeats, deren `sequence` bei 0 beginnt
/// (`core/ipc/ControlClient.cpp:1386-1387`). Ohne getrennten Zahlenraum
/// ordnete dieses Bein fremde `heartbeat_ack` seinen eigenen Eintraegen zu
/// und mass gegen falsche Sendezeitpunkte — selbst gemessen im Rauchtest am
/// 02.09.2026: p95 ueber 2.000 ms, unabhaengig von der Leserverzoegerung.
constexpr std::uint64_t kTestSeqBasis = 1000000;
/// Dasselbe im Barrierefenster von K-S4 — ein Fenster, in das der Pruefer
/// zuverlaessig hineinkillt.
constexpr int kLangsamBarriereMs = 3000;
/// Audio: Blockgroesse und Samplerate des Sondenpfads.
constexpr double kFs = 48000.0;
constexpr int kBlock = 512;
constexpr double kPi = 3.14159265358979323846;

std::mutex ausgabeSchloss;

void melde (const std::string& zeile)
{
    std::lock_guard<std::mutex> l (ausgabeSchloss);
    std::cout << zeile << std::endl;
}

std::string hex32 (unsigned long long n)
{
    std::ostringstream s;
    s << std::hex << std::nouppercase << std::setfill ('0') << std::setw (32) << n;
    return s.str();
}

Adresse adresse (unsigned instanz, unsigned nonce)
{
    return { "S-1-5-21-1-2-3-1001", hex32 (1), hex32 (2), hex32 (instanz), hex32 (nonce) };
}

ControlStatus laufStatus()
{
    ControlStatus s;
    s.runtime.gemeldet = true;
    s.runtime.messpunkt = "insert";
    s.runtime.betrieb = "active";
    return s;
}

nakama::analyse::FeatureFrame messframe (std::uint64_t sequence, std::uint32_t samples)
{
    nakama::analyse::FeatureFrame f;
    f.transport.transport_epoch = 1;
    f.transport.continuity_segment = 1;
    f.transport.sequence = sequence;
    f.transport.zeitbasis = nakama::analyse::Zeitbasis::local_monotonic;
    f.transport.sample_count = samples;
    f.transport.sample_rate = kFs;
    f.transport.process_context_present_gesetzt = true;
    f.transport.process_context_present = false;
    f.live.gitter = nakama::analyse::GitterId::nakama_log64_v1;
    f.live.encoding = nakama::analyse::BandEncoding::q_db_0p1_i16;
    for (int i = 0; i < nakama::analyse::Gitter::liveBaender; ++i)
    {
        f.live.werte[i] = -180;
        f.live.bitmap[i / 8] =
            static_cast<std::uint8_t> (f.live.bitmap[i / 8] | (1u << (i % 8)));
    }
    f.metricsVersion = 1;
    f.lufsIStatusGesetzt = true;
    f.lufsIStatus = 1;
    return f;
}

std::string jsonString (const std::string& s)
{
    std::string aus = "\"";
    for (char c : s)
    {
        if (c == '"' || c == '\\') { aus += '\\'; aus += c; }
        else if (static_cast<unsigned char> (c) < 0x20) aus += ' ';
        else aus += c;
    }
    return aus + "\"";
}

/// Die fuenf vertragsinternen Texte, die `TelemetryClient` im verbundenen Lauf
/// setzt, ohne dass die Pipe brach. K-S1 schliesst sie aus (Manifest §6).
bool istVertragstext (const std::string& f)
{
    static const char* kTexte[] = {
        "P2-Frame zu gross",
        "Kopplung gewechselt",
        "Envelope abgelehnt",
        "P0/P1 auf der Telemetrieverbindung",
        "Nachrichtenratengrenze",
    };
    for (const char* t : kTexte)
        if (f.find (t) != std::string::npos) return true;
    return false;
}

double perzentil (std::vector<double> werte, double p)
{
    if (werte.empty()) return 0.0;
    std::sort (werte.begin(), werte.end());
    const auto rang = static_cast<std::size_t> (
        std::ceil (p * static_cast<double> (werte.size()))) - 1;
    return werte[std::min (rang, werte.size() - 1)];
}

// ── Eine Sonde: Control + Telemetrie + echter Audiopfad ──────────────────
struct Sonde
{
    Adresse adr;
    bool langsam = false;
    bool fluter = false;                 ///< Sonde 0: haelt die Schleuse im Rueckstau (K-S1)
    std::unique_ptr<ControlClient>   control;
    std::unique_ptr<TelemetryClient> telemetrie;
    std::unique_ptr<nakama::sonde::SondeProcessor> prozessor;

    std::mutex mutex;
    std::vector<Uhr::time_point> gesendet;   ///< Index = Sequenz
    std::vector<bool> beantwortetFlag;
    /// Im STOERFENSTER abgeschickt: Barrierefenster (kuenstliche Bremse) oder
    /// Neustartfenster. Z4 gilt ausserhalb davon.
    std::vector<bool> imFensterGesendet;
    std::vector<std::pair<double, bool>> latenzen;   ///< Latenz + Stoerfenster
    std::atomic<std::uint64_t> p0Gesendet { 0 }, p0Beantwortet { 0 };
    std::atomic<std::uint64_t> sequence { 0 };
    std::atomic<bool> inVerzoegerung { false };   ///< K-S4-Barriere
    std::atomic<bool> verzoegerungGemeldet { false };

    // Vorzustand fuer K-S1, festgehalten bei KILL_VORBEREITEN.
    bool vorKillVerbunden = false;
    bool vorKillRueckstau = false;
    int  vorKillVersuche = 0;
    int  controlVersucheVorKill = 0;
    std::uint64_t vorKillErsetzt = 0;
    std::uint64_t letzterErsetztStand = 0;

    // Audio
    std::thread audioThread;
    juce::AudioBuffer<float> puffer;
    juce::MidiBuffer midi;
    std::int64_t projektzeit = 0, kontinuierlich = 0;
    double phase = 0.0;
    std::atomic<std::uint64_t> bloecke { 0 };
    std::atomic<std::uint64_t> segmentwechsel { 0 };
    std::uint64_t letztesSegment = 0;
    bool segmentGesehen = false;
};

struct Neustart
{
    int index = 0;
    std::string epochAlt, epochNeu;
    long long bereitBisVollstaendigMs = -1;
    long long totzeitMs = 0;
    std::vector<double> reconnectMs;         ///< je Clientpaar
    std::uint64_t alteEpocheGesehen = 0;
    // Killpunkt-Belege
    std::uint64_t telemetrieHandleFehler = 0;    // K-S1
    bool snapshotVorKill = false;                // K-S2
    bool ks2Barriere = false;
    std::uint64_t p0OhneAckImFenster = 0;        // K-S3
    std::uint64_t flagZumKillzeitpunkt = 0;      // K-S4
    bool ks4Barriere = false;
    int backoffDeckelErreicht = 0;               // K-S5
    bool ks5Totzeit = false;
};

// ── Der ganze Lauf ───────────────────────────────────────────────────────
struct Soak
{
    Soak (std::string p, int n, int langsamAnzahl, ServerErwartung e)
        : pipe (std::move (p)), anzahl (n), langsam (langsamAnzahl),
          erwartung (std::move (e)) {}

    std::string pipe;
    int anzahl;
    int langsam;
    ServerErwartung erwartung;

    // Main
    eqcop::SourcesModel model;
    Adresse mainAdr = adresse (10, 100);
    std::unique_ptr<ControlClient>   mainControl;
    std::unique_ptr<TelemetryClient> mainTelemetrie;
    std::atomic<bool> subscribeGesendet { false };
    std::atomic<bool> snapshotSeitSubscribe { false };
    std::mutex epochSchloss;
    std::string aktuelleEpoche;
    std::set<std::string> geseheneEpochen;
    std::set<std::string> alteEpochen;
    std::atomic<std::uint64_t> alteEpocheGesehen { 0 };

    std::vector<std::unique_ptr<Sonde>> sonden;
    std::set<std::string> erwarteteIds;

    // Zaehler S02/S03
    std::atomic<std::uint64_t> snapshotPruefungen { 0 }, snapshotVollstaendig { 0 };
    std::atomic<std::uint64_t> fremdeAdresse { 0 }, fuehrungFalsch { 0 };
    // Zaehler S13
    std::atomic<std::uint64_t> staleAusserhalb { 0 }, staleImFenster { 0 };
    std::atomic<std::uint64_t> evictedAusserhalb { 0 };
    // Zaehler S10
    std::atomic<std::uint64_t> fremderPipename { 0 };
    /// S09: wie oft der Main im Neustartfenster OHNE aktive Subscription
    /// gesehen wurde. Muss > 0 sein — sonst hat der Kill die Subscription
    /// nicht sichtbar beendet und das Modell zeigte weiter die alte Sicht.
    std::atomic<std::uint64_t> subscriptionWegImFenster { 0 };

    std::atomic<bool> laeuft { true };
    std::atomic<bool> warmupVorbei { false };
    std::atomic<bool> imNeustartfenster { false };
    /// Der Fluter laeuft NUR im Barrierefenster vor einem Kill. Dauerhaftes
    /// Fluten widerspraeche der zugesagten realistischen Kadenz und drueckte
    /// die P0-Latenz (selbst gemessen im Rauchtest 02.09.2026: p95 1.670 ms
    /// bei 12.338 ersetzten Frames). K-S1 braucht den Rueckstau ohnehin nur
    /// zum Killzeitpunkt.
    std::atomic<bool> flutAn { false };
    /// K-S4: im Barrierefenster schlaeft der langsame Leser lang genug,
    /// dass der Pruefer sicher hineinkillt.
    std::atomic<bool> langsamBarriere { false };
    std::atomic<int>  aktuellerNeustart { -1 };

    std::vector<Neustart> neustarts;
    std::mutex neustartSchloss;

    long long topologieMs = -1;
    std::vector<std::thread> faeden;

    // ───────────────────────────────────────────────── Aufbau
    bool aufbauen()
    {
        const auto t0 = Uhr::now();

        mainControl = std::make_unique<ControlClient> (
            [this] {
                ControlHello h;
                h.adresse = mainAdr;
                h.pluginKind = "main";
                h.hostAngeben = true;
                h.hostPid = 4242;
                return h;
            },
            pipe,
            [this] (const std::string& json) { mainText (json); },
            [] { return laufStatus(); },
            [this] (bool verbunden) { mainLinkStatus (verbunden); },
            std::function<void (const std::string&, std::uint8_t)> {},
            erwartung);

        mainTelemetrie = std::make_unique<TelemetryClient> (
            [this] {
                TelemetryHello h;
                h.adresse = mainAdr;
                mainControl->kopplung (h.linkId, h.challenge);
                return h;
            },
            pipe,
            [this] (const std::uint8_t* daten, std::size_t laenge, std::uint8_t minor) {
                juce::String grund;
                model.uebernehmeP2 (daten, laenge, minor, Uhr::now(), grund);
            },
            erwartung);

        for (int i = 0; i < anzahl; ++i)
        {
            auto s = std::make_unique<Sonde>();
            s->adr = adresse (100 + static_cast<unsigned> (i), 1000 + static_cast<unsigned> (i));
            // Die langsamen Leser sind die LETZTEN `langsam` Sonden; Sonde 0
            // bleibt der Fluter fuer K-S1, damit sich beide Barrieren nicht
            // gegenseitig aufheben.
            s->langsam = (i >= anzahl - langsam);
            s->fluter  = (i == 0);
            erwarteteIds.insert (s->adr.instanceId);
            Sonde* roh = s.get();

            s->control = std::make_unique<ControlClient> (
                [roh] {
                    ControlHello h;
                    h.adresse = roh->adr;
                    h.pluginKind = "active_probe";
                    h.hostAngeben = true;
                    h.hostPid = 4242;
                    return h;
                },
                pipe,
                [this, roh] (const std::string& json) { sondeText (*roh, json); },
                [] { return laufStatus(); },
                std::function<void (bool)> {},
                std::function<void (const std::string&, std::uint8_t)> {},
                erwartung);

            s->telemetrie = std::make_unique<TelemetryClient> (
                [roh] {
                    TelemetryHello h;
                    h.adresse = roh->adr;
                    roh->control->kopplung (h.linkId, h.challenge);
                    return h;
                },
                pipe,
                std::function<void (const std::uint8_t*, std::size_t, std::uint8_t)> {},
                erwartung);

            // Der echte Sondenpfad. Unter NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3
            // startet er seine eigenen v3-Clients NICHT und beruehrt damit die
            // Produktionspipe nie (Manifest §2.2, §8).
            s->prozessor = std::make_unique<nakama::sonde::SondeProcessor>();
            bindeSonde (*s->prozessor);
            s->prozessor->setPlayConfigDetails (2, 2, kFs, kBlock);
            s->prozessor->prepareToPlay (kFs, kBlock);
            s->puffer.setSize (2, kBlock);

            sonden.push_back (std::move (s));
        }

        mainControl->start();
        mainTelemetrie->start();
        for (auto& s : sonden) s->control->start();
        for (auto& s : sonden) s->telemetrie->start();

        // S01: das VOLLSTAENDIGE Clientpaar zaehlt, nicht nur Control.
        const auto frist = Uhr::now() + std::chrono::milliseconds (kFristMs);
        while (Uhr::now() < frist)
        {
            if (topologieSteht()) break;
            std::this_thread::sleep_for (std::chrono::milliseconds (20));
        }
        if (! topologieSteht())
        {
            diagnose();
            return false;
        }
        topologieMs = std::chrono::duration_cast<std::chrono::milliseconds> (
            Uhr::now() - t0).count();
        melde ("TOPOLOGIE_STEHT " + std::to_string (topologieMs));
        return true;
    }

    static void bindeSonde (nakama::sonde::SondeProcessor& p)
    {
        auto z = nakama::state::frisch ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        z.common.klasse = nakama::state::Klasse::active_probe;
        z.common.position = nakama::state::Messposition::insert;
        z.common.projectBindingId = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
        z.hatParameters = true;
        juce::MemoryBlock bytes;
        nakama::state::speichere (z, bytes);
        p.setStateInformation (bytes.getData(), (int) bytes.getSize());
    }

    bool topologieSteht() const
    {
        if (mainControl->snapshot().status != ControlClient::Status::verbunden) return false;
        if (mainTelemetrie->snapshot().status != TelemetryClient::Status::verbunden) return false;
        for (const auto& s : sonden)
        {
            if (s->control->snapshot().status != ControlClient::Status::verbunden) return false;
            if (s->telemetrie->snapshot().status != TelemetryClient::Status::verbunden) return false;
        }
        const auto sicht = model.sicht();
        if (sicht.quellen.size() != static_cast<std::size_t> (anzahl)) return false;
        return std::all_of (sicht.quellen.begin(), sicht.quellen.end(),
                            [] (const auto& q) { return q.descriptorVorhanden; });
    }

    void diagnose() const
    {
        int c = 0, t = 0;
        for (const auto& s : sonden)
        {
            c += s->control->snapshot().status == ControlClient::Status::verbunden;
            t += s->telemetrie->snapshot().status == TelemetryClient::Status::verbunden;
        }
        const auto sicht = model.sicht();
        std::cerr << "Diagnose: quellen=" << sicht.quellen.size() << "/" << anzahl
                  << " subscribed=" << sicht.subscriptionAktiv
                  << " sonde-control=" << c << "/" << anzahl
                  << " sonde-telemetrie=" << t << "/" << anzahl
                  << " main-control=" << static_cast<int> (mainControl->snapshot().status)
                  << " main-telemetrie=" << static_cast<int> (mainTelemetrie->snapshot().status)
                  << std::endl;
    }

    // ─────────────────────────────────────────── Main-Callbacks (§4.1)
    void mainLinkStatus (bool verbunden)
    {
        if (! verbunden)
        {
            model.controlEnde();
            subscribeGesendet.store (false);
            return;
        }
        // Reihenfolge bindend: erst das Subscription-Fenster im Modell, dann
        // der P1 auf den Draht (`Sonde012SourcesLatencyTest.cpp:131-140`).
        model.beginneSubscription (mainAdr.projectBindingId, mainAdr.sessionEpoch,
                                   mainAdr.instanceId);
        const auto text = std::string ("{\"type\":\"subscribe_session\",\"adresse\":")
                        + adresseAlsJson (mainAdr) + ",\"session_epoch\":\""
                        + mainAdr.sessionEpoch + "\"}";
        snapshotSeitSubscribe.store (false);
        const auto ergebnis = mainControl->sendeP1 ("subscribe_session", text);
        if (ergebnis == P1Ergebnis::abgewiesen || ergebnis == P1Ergebnis::zuGross)
        {
            model.controlEnde();
            return;
        }
        subscribeGesendet.store (true);
        // Barriere K-S2: eingereiht, noch kein Snapshot da.
        melde ("MAIN_SUBSCRIBE_GESENDET");
    }

    void mainText (const std::string& json)
    {
        // Der `session_snapshot` traegt das `mitglieder`-ARRAY, ist also kein
        // flaches Objekt; `flachesJsonObjekt` lehnt ihn ab (selbst gemessen im
        // Rauchtest 02.09.2026: epoch_alt/epoch_neu blieben leer). Zwei Wege
        // statt eines eigenen JSON-Parsers:
        //   * Identitaet der laufenden Epoche: `ControlClient::Snapshot::
        //     brokerEpoch` aus dem Welcome (`core/ipc/ControlClient.h:165`,
        //     gesetzt in `ControlClient.cpp:1248`, geleert in `:1051`);
        //   * Wache gegen die ALTE Epoche: Textsuche nach ihrer 32-stelligen
        //     Hexfolge im rohen Snapshot. Keine Vertragspruefung, sondern eine
        //     Wache — eine 128-Bit-Hexfolge kollidiert nicht zufaellig.
        if (json.find ("session_snapshot") != std::string::npos)
        {
            snapshotSeitSubscribe.store (true);
            std::lock_guard<std::mutex> l (epochSchloss);
            for (const auto& alt : alteEpochen)
                if (! alt.empty() && json.find (alt) != std::string::npos)
                    alteEpocheGesehen.fetch_add (1);
        }
        juce::String grund;
        model.uebernehmeSessionSnapshot (json, Uhr::now(), grund);
    }

    /// Die Epoche des LAUFENDEN Brokers, aus dem Control-Welcome.
    std::string epocheJetzt() const { return mainControl->snapshot().brokerEpoch; }

    // ─────────────────────────────────────────── Sonden-Callback (S05)
    void sondeText (Sonde& s, const std::string& json)
    {
        // Der Empfangszeitpunkt wird VOR jeder kuenstlichen Verzoegerung
        // genommen; sonst maesse das Bein die eigene Bremse (Manifest S05).
        const auto jetzt = Uhr::now();
        std::vector<JsonFeld> felder;
        std::string typ, seqText;
        if (flachesJsonObjekt (json, felder)
            && jsonText (felder, "type", typ) && typ == "heartbeat_ack"
            && jsonLiteral (felder, "sequence", seqText))
        {
            // Ein ACK unterhalb der Basis gehoert dem produktiven Heartbeat
            // des ControlClient. Es wird uebergangen, aber es BREMST den
            // langsamen Leser trotzdem — er liest ja auch dieses Frame.
            const auto wire = std::strtoull (seqText.c_str(), nullptr, 10);
            if (wire >= kTestSeqBasis)
            {
                const auto seq = static_cast<std::size_t> (wire - kTestSeqBasis);
                std::lock_guard<std::mutex> l (s.mutex);
                if (seq < s.gesendet.size() && ! s.beantwortetFlag[seq])
                {
                    s.beantwortetFlag[seq] = true;
                    s.latenzen.push_back (
                        { std::chrono::duration<double, std::milli> (
                              jetzt - s.gesendet[seq]).count(),
                          s.imFensterGesendet[seq] });
                    s.p0Beantwortet.fetch_add (1);
                }
            }
        }
        if (s.langsam)
        {
            // Im Barrierefenster wird die Verzoegerung lang genug, dass der
            // Pruefer zwischen Meldung und Kill nicht ins Leere laeuft; die
            // Meldung kommt bei JEDEM Eintritt, nicht nur beim ersten
            // (Rauchtest 02.09.2026: einmalige Meldung + 120 ms Fenster ergab
            // flag_zum_killzeitpunkt = 0).
            const bool barriere = langsamBarriere.load();
            s.inVerzoegerung.store (true);
            if (barriere) melde ("LANGSAM_IN_VERZOEGERUNG");
            std::this_thread::sleep_for (std::chrono::milliseconds (
                barriere ? kLangsamBarriereMs : gLangsamMs));
            s.inVerzoegerung.store (false);
        }
    }

    // ─────────────────────────────────────────── Faeden
    void starteFaeden()
    {
        faeden.emplace_back ([this] { heartbeatLauf(); });
        faeden.emplace_back ([this] { liveLauf(); });
        faeden.emplace_back ([this] { wacheLauf(); });
        faeden.emplace_back ([this] { audioLauf(); });
    }

    void heartbeatLauf()
    {
        auto naechster = Uhr::now();
        while (laeuft.load())
        {
            for (auto& s : sonden)
            {
                std::size_t seq;
                {
                    std::lock_guard<std::mutex> l (s->mutex);
                    seq = s->gesendet.size();
                    s->gesendet.push_back (Uhr::now());
                    s->beantwortetFlag.push_back (false);
                    // Das Stoerfenster umfasst Barriere UND Neustart: die
                    // 3-s-Bremse aus K-S4 und die Totzeit aus K-S5 sind
                    // Testeingriffe, keine Produktlatenz.
                    s->imFensterGesendet.push_back (
                        imNeustartfenster.load() || flutAn.load());
                }
                // VERTRAGSGEMAESS, nicht handgestrickt: der echte Coordinator
                // verlangt im Heartbeat die `adresse` und prueft sie gegen die
                // Wire-Adresse des Links (`broker/src/coordinator.rs:2944-2955`).
                // Ein Heartbeat ohne Adresse wird nie beantwortet — A22 kommt
                // damit nur durch, weil sein Probe-Broker die zaehlende
                // Standardsenke aus `broker/src/transport/server_v3.rs:297`
                // nutzt. Hier laeuft der echte Coordinator.
                const std::string json =
                    heartbeatAlsJson (s->adr, kTestSeqBasis + seq, laufStatus());
                if (s->control->sendeP0 (json))
                    s->p0Gesendet.fetch_add (1);
                else
                {
                    // Nie eingereiht ⇒ nie erwartet (`IpcLastMain.cpp:229-235`).
                    std::lock_guard<std::mutex> l (s->mutex);
                    s->gesendet.pop_back();
                    s->beantwortetFlag.pop_back();
                    s->imFensterGesendet.pop_back();
                }
            }
            naechster += std::chrono::milliseconds (kHeartbeatMs);
            std::this_thread::sleep_until (naechster);
        }
    }

    void liveLauf()
    {
        auto naechster = Uhr::now();
        int takt = 0;
        std::vector<std::uint8_t> flut (512, 0x5A);
        while (laeuft.load())
        {
            const bool gross = (++takt % (kGrossTaktMs / kLiveTaktMs)) == 0;
            for (auto& s : sonden)
            {
                const auto f = messframe (s->sequence.fetch_add (1) + 1,
                                          gross ? kGrossSamples : kLiveSamples);
                s->telemetrie->veroeffentlichen (f, s->adr);
                // K-S1: Sonde 0 haelt ihre Schleuse (Cap 2) im Rueckstau,
                // damit der verbundene Lauf im Schreibzweig steht und nicht
                // im Leerlauf-Lesezweig (Manifest §6, K-S1 Punkt 2).
                if (s->fluter && flutAn.load())
                    for (int i = 0; i < 12; ++i)
                        s->telemetrie->veroeffentlichen (flut.data(), flut.size());
            }
            naechster += std::chrono::milliseconds (kLiveTaktMs);
            std::this_thread::sleep_until (naechster);
        }
    }

    /// Beobachtet den Main-Snapshot dauernd (S02, S03, S12) und die Liveness
    /// (S13). Ausserhalb der Neustartfenster ist jede Abweichung ein Befund.
    void wacheLauf()
    {
        while (laeuft.load())
        {
            if (warmupVorbei.load() && ! imNeustartfenster.load())
            {
                model.tick (Uhr::now());
                const auto sicht = model.sicht();
                snapshotPruefungen.fetch_add (1);

                bool vollstaendig = sicht.quellen.size() == static_cast<std::size_t> (anzahl);
                for (const auto& q : sicht.quellen)
                {
                    if (erwarteteIds.count (q.instanceId) == 0)
                    {
                        fremdeAdresse.fetch_add (1);
                        vollstaendig = false;
                    }
                    if (q.control == eqcop::SourcesModel::Control::stale)
                        staleAusserhalb.fetch_add (1);
                }
                if (sicht.quellen.size() < static_cast<std::size_t> (anzahl))
                    evictedAusserhalb.fetch_add (1);
                if (sicht.fuehrendesMain != mainAdr.instanceId)
                {
                    fuehrungFalsch.fetch_add (1);
                    vollstaendig = false;
                }
                if (vollstaendig) snapshotVollstaendig.fetch_add (1);
            }
            else if (imNeustartfenster.load())
            {
                const auto sicht = model.sicht();
                if (! sicht.subscriptionAktiv)
                    subscriptionWegImFenster.fetch_add (1);
                for (const auto& q : sicht.quellen)
                    if (q.control == eqcop::SourcesModel::Control::stale)
                        staleImFenster.fetch_add (1);
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (250));
        }
    }

    /// Echter Sondenpfad in Echtzeitkadenz (Z7).
    ///
    /// EIN Thread bedient ALLE Sonden im selben Blocktakt — so, wie FLs
    /// Audiocallback alle Plugins eines Puffers nacheinander bedient. Die
    /// erste Fassung gab jeder Sonde einen eigenen Thread; das ist kein Host,
    /// sondern N-facher Blocktakt (bei 16 Sonden rund 1.500 statt 94 Bloecke
    /// je Sekunde) und drueckte im Rauchtest am 02.09.2026 die P0-ACK-p95 der
    /// SCHNELLEN Sonden auf 1.189 ms — ein Werkzeugfehler, kein Produktbefund.
    void audioLauf()
    {
        const auto blockDauer = std::chrono::duration_cast<Uhr::duration> (
            std::chrono::duration<double> (static_cast<double> (kBlock) / kFs));
        auto naechster = Uhr::now();
        while (laeuft.load())
        {
            for (auto& sp : sonden)
            {
                Sonde& s = *sp;
                for (int i = 0; i < kBlock; ++i)
                {
                    const float v = 0.1f * std::sin (static_cast<float> (s.phase));
                    s.phase += 2.0 * kPi * 997.0 / kFs;
                    for (int k = 0; k < 2; ++k) s.puffer.setSample (k, i, v);
                }
                eqcop::hostbruecke::Blockbefund befund;
                befund.kontext.processContextPresent = true;
                befund.kontext.projectTimeSamples.setze (s.projektzeit);
                befund.kontext.continuousTimeSamples.setze (s.kontinuierlich);
                befund.kontext.playing.setze (true);
                befund.kontext.recording.setze (false);
                befund.kontext.sampleRate.setze (kFs);
                befund.kontext.tempo.setze (120.0);
                befund.kontext.ppqPosition.setze (
                    static_cast<double> (s.projektzeit) / kFs * 2.0);
                befund.blockGroesse = static_cast<std::uint32_t> (kBlock);

                s.prozessor->nakamaBlockEmpfangen (befund);
                s.prozessor->processBlock (s.puffer, s.midi);
                s.bloecke.fetch_add (1);
                s.projektzeit += kBlock;
                s.kontinuierlich += kBlock;

                // Kontinuitaet ueber den Producer-Frame: `SondeProcessor`
                // veroeffentlicht keinen Bruchzaehler (Manifest S08).
                nakama::analyse::FeatureFrame f;
                if (s.prozessor->letzterProducerFrameFuerTest (f))
                {
                    if (! s.segmentGesehen)
                    {
                        s.segmentGesehen = true;
                        s.letztesSegment = f.transport.continuity_segment;
                    }
                    else if (f.transport.continuity_segment != s.letztesSegment)
                    {
                        s.segmentwechsel.fetch_add (1);
                        s.letztesSegment = f.transport.continuity_segment;
                    }
                }
            }
            naechster += blockDauer;
            std::this_thread::sleep_until (naechster);
        }
    }

    // ─────────────────────────────────────────── Kill-Ablauf
    void killVorbereiten (int index)
    {
        Neustart n;
        n.index = index;
        {
            std::lock_guard<std::mutex> l (epochSchloss);
            aktuelleEpoche = epocheJetzt();
            n.epochAlt = aktuelleEpoche;
        }
        for (auto& s : sonden)
        {
            const auto ts = s->telemetrie->snapshot();
            s->vorKillVerbunden = ts.status == TelemetryClient::Status::verbunden;
            s->vorKillErsetzt = ts.ersetzt;
            s->vorKillRueckstau = ts.ersetzt > s->letzterErsetztStand;
            s->vorKillVersuche = ts.verbindungsVersuche;
            if (s->inVerzoegerung.load()) ++n.flagZumKillzeitpunkt;
        }
        n.snapshotVorKill = snapshotSeitSubscribe.load();
        {
            std::lock_guard<std::mutex> l (neustartSchloss);
            neustarts.push_back (n);
            aktuellerNeustart.store (static_cast<int> (neustarts.size()) - 1);
        }
        melde ("KILL_BEREIT " + std::to_string (index));
    }

    void killGeschehen (int)
    {
        imNeustartfenster.store (true);
        // Jeder zum Killzeitpunkt noch unbeantwortete Heartbeat verliert sein
        // ACK mit dem Brokerprozess. Er gehoert ins Neustartfenster, auch wenn
        // er davor abgeschickt wurde — sonst zaehlte Z4 einen Verlust, den der
        // Kill verursacht hat (selbst gemessen im Rauchtest 02.09.2026: 42
        // angeblich "ausserhalb" verlorene P0).
        for (auto& s : sonden)
        {
            std::lock_guard<std::mutex> l (s->mutex);
            for (std::size_t i = 0; i < s->gesendet.size(); ++i)
                if (! s->beantwortetFlag[i]) s->imFensterGesendet[i] = true;
        }
        {
            std::lock_guard<std::mutex> l (epochSchloss);
            if (! aktuelleEpoche.empty()) alteEpochen.insert (aktuelleEpoche);
        }
        alteEpocheGesehen.store (0);
    }

    /// Der neue Broker horcht. Ab hier laeuft die Frist aus §3.1.
    void bereitWieder (int index, long long totzeitMs)
    {
        const auto bereit = Uhr::now();
        std::size_t pos = 0;
        {
            std::lock_guard<std::mutex> l (neustartSchloss);
            if (neustarts.empty()) return;
            pos = neustarts.size() - 1;
        }
        // Kein roher Zeiger in den Vektor: ein spaeteres push_back realloziert
        // und der Zeiger baumelte. Der Index bleibt gueltig, weil hier nur
        // angehaengt wird.
        Neustart* n = &neustarts[pos];
        n->totzeitMs = totzeitMs;

        // K-S1 und K-S5: Belege einsammeln, solange die Clients noch im
        // Reconnect stehen.
        for (auto& s : sonden)
        {
            const auto ts = s->telemetrie->snapshot();
            if (s->vorKillVerbunden && s->vorKillRueckstau
                && ! ts.letzterFehler.empty() && ! istVertragstext (ts.letzterFehler)
                && ts.verbindungsVersuche == s->vorKillVersuche)
                ++n->telemetrieHandleFehler;
        }
        n->backoffDeckelErreicht = backoffDeckel();
        n->ks5Totzeit = totzeitMs >= 20000;

        // K-S3: P0, die im Fenster abgeschickt wurden und nie ein ACK bekamen.
        std::uint64_t ohneAck = 0;
        for (auto& s : sonden)
        {
            std::lock_guard<std::mutex> l (s->mutex);
            for (std::size_t i = 0; i < s->gesendet.size(); ++i)
                if (s->imFensterGesendet[i] && ! s->beantwortetFlag[i]) ++ohneAck;
        }
        n->p0OhneAckImFenster = ohneAck;

        // Warten, bis der Snapshot wieder vollstaendig und richtig ist (S11).
        const auto frist = bereit + std::chrono::milliseconds (kFristMs);
        while (Uhr::now() < frist && laeuft.load())
        {
            if (topologieSteht() && fuehrungRichtig()) break;
            std::this_thread::sleep_for (std::chrono::milliseconds (25));
        }
        const bool ok = topologieSteht() && fuehrungRichtig();
        n->bereitBisVollstaendigMs = ok
            ? std::chrono::duration_cast<std::chrono::milliseconds> (Uhr::now() - bereit).count()
            : -1;
        for (auto& s : sonden)
        {
            const auto cs = s->control->snapshot();
            const auto ts = s->telemetrie->snapshot();
            const bool paarSteht = cs.status == ControlClient::Status::verbunden
                                && ts.status == TelemetryClient::Status::verbunden;
            n->reconnectMs.push_back (paarSteht
                ? static_cast<double> (n->bereitBisVollstaendigMs < 0 ? kFristMs
                                                                      : n->bereitBisVollstaendigMs)
                : static_cast<double> (kFristMs) + 1.0);
        }
        {
            std::lock_guard<std::mutex> l (epochSchloss);
            aktuelleEpoche = epocheJetzt();
            n->epochNeu = aktuelleEpoche;
        }
        n->alteEpocheGesehen = alteEpocheGesehen.load();
        imNeustartfenster.store (false);
        for (auto& s : sonden)
            s->letzterErsetztStand = s->telemetrie->snapshot().ersetzt;
        melde ("NEUSTART_VOLLSTAENDIG " + std::to_string (index) + " "
               + std::to_string (n->bereitBisVollstaendigMs));
    }

    bool fuehrungRichtig() const
    {
        const auto sicht = model.sicht();
        return sicht.fuehrendesMain == mainAdr.instanceId;
    }

    /// K-S5: `verbindungsVersuche` steigt bei JEDEM Versuch
    /// (`core/ipc/ControlClient.cpp:1048`). Fuenf Versuche in der Totzeit
    /// heissen 500+1.000+2.000+4.000+8.000 ms durchlaufen — der Deckel steht.
    int backoffDeckel() const
    {
        int erreicht = 0;
        if (mainControl->snapshot().verbindungsVersuche >= mainVersucheVorKill + 5) ++erreicht;
        for (const auto& s : sonden)
            if (s->control->snapshot().verbindungsVersuche >= s->controlVersucheVorKill + 5)
                ++erreicht;
        return erreicht;
    }

    int mainVersucheVorKill = 0;

    void versucheFesthalten()
    {
        mainVersucheVorKill = mainControl->snapshot().verbindungsVersuche;
        for (auto& s : sonden)
            s->controlVersucheVorKill = s->control->snapshot().verbindungsVersuche;
    }

    // ─────────────────────────────────────────── Abbau (S14)
    void abbauen()
    {
        laeuft.store (false);
        for (auto& f : faeden) if (f.joinable()) f.join();
        faeden.clear();
        // Reihenfolge: Telemetrie vor Control (`IpcLastMain.cpp:280`), danach
        // erst die Prozessoren.
        for (auto& s : sonden) { s->telemetrie->stop(); s->control->stop(); }
        if (mainTelemetrie) mainTelemetrie->stop();
        if (mainControl) mainControl->stop();
        // Die SondeProcessor bleiben stehen: `bericht()` liest danach ihre
        // Zaehler. Sie sterben mit dem Soak-Objekt, nach dem Bericht.
    }

    // ─────────────────────────────────────────── Bericht
    std::string bericht (int minuten, int warmupS) const
    {
        std::vector<double> alleLatenzen, schnelleLatenzen, fensterLatenzen;
        std::uint64_t p0Ges = 0, p0Ack = 0, verlorenAussen = 0, verlorenFenster = 0;
        std::uint64_t ersetzt = 0, neuesteVerworfen = 0, zuGross = 0, kollision = 0;
        std::uint64_t bloecke = 0, dropsUeberlauf = 0, dropsOversize = 0;
        std::uint64_t brueche = 0, publikationen = 0;
        bool langsameImmerMitglied = true;

        const auto sicht = model.sicht();
        std::set<std::string> sichtbar;
        for (const auto& q : sicht.quellen) sichtbar.insert (q.instanceId);

        for (const auto& s : sonden)
        {
            p0Ges += s->p0Gesendet.load();
            p0Ack += s->p0Beantwortet.load();
            const auto ts = s->telemetrie->snapshot();
            ersetzt += ts.ersetzt;
            neuesteVerworfen += ts.beanspruchtVerworfen;
            zuGross += ts.zuGross;
            kollision += ts.kollisionsLoecher;
            bloecke += s->bloecke.load();
            dropsUeberlauf += s->prozessor->analyseDropsUeberlaufFuerTest();
            dropsOversize += s->prozessor->analyseDropsOversizeFuerTest();
            publikationen += s->prozessor->producerPublikationenFuerTest();
            brueche += s->segmentwechsel.load();
            if (s->langsam && sichtbar.count (s->adr.instanceId) == 0)
                langsameImmerMitglied = false;
            std::lock_guard<std::mutex> l (s->mutex);
            for (std::size_t i = 0; i < s->gesendet.size(); ++i)
                if (! s->beantwortetFlag[i])
                    (s->imFensterGesendet[i] ? verlorenFenster : verlorenAussen) += 1;
            for (const auto& [w, imFenster] : s->latenzen)
            {
                if (imFenster) { fensterLatenzen.push_back (w); continue; }
                alleLatenzen.push_back (w);
                if (! s->langsam) schnelleLatenzen.push_back (w);
            }
        }

        std::ostringstream o;
        o << std::fixed << std::setprecision (1);
        o << "{\"sonden\":" << anzahl
          << ",\"minuten\":" << minuten
          << ",\"langsam_anzahl\":" << langsam
          << ",\"warmup_s\":" << warmupS
          << ",\"clientpaare\":" << (anzahl + 1)
          << ",\"topologie_ms\":" << topologieMs
          << ",\"mitgliedschaft\":{\"snapshot_pruefungen\":" << snapshotPruefungen.load()
          << ",\"vollstaendig\":" << snapshotVollstaendig.load()
          << ",\"fremde_adresse\":" << fremdeAdresse.load()
          << ",\"fuehrendes_main_falsch\":" << fuehrungFalsch.load() << "}"
          << ",\"neustart\":[";
        for (std::size_t i = 0; i < neustarts.size(); ++i)
        {
            const auto& n = neustarts[i];
            const double mn = n.reconnectMs.empty() ? 0.0
                : *std::min_element (n.reconnectMs.begin(), n.reconnectMs.end());
            const double mx = n.reconnectMs.empty() ? 0.0
                : *std::max_element (n.reconnectMs.begin(), n.reconnectMs.end());
            if (i) o << ",";
            o << "{\"index\":" << n.index
              << ",\"epoch_alt\":" << jsonString (n.epochAlt)
              << ",\"epoch_neu\":" << jsonString (n.epochNeu)
              << ",\"frist_ms\":" << kFristMs
              << ",\"schranke_ms\":" << kSchrankeMs
              << ",\"reconnect_ms\":{\"min\":" << mn
              << ",\"p95\":" << perzentil (n.reconnectMs, 0.95)
              << ",\"max\":" << mx << "}"
              << ",\"bereit_bis_vollstaendig_ms\":" << n.bereitBisVollstaendigMs
              << ",\"ueber_schranke\":"
              << ((n.bereitBisVollstaendigMs > kSchrankeMs) ? "true" : "false")
              << ",\"alte_epoche_nach_neustart_gesehen\":" << n.alteEpocheGesehen
              << ",\"totzeit_ms\":" << n.totzeitMs << "}";
        }
        o << "]"
          << ",\"p0\":{\"gesendet\":" << p0Ges
          << ",\"beantwortet\":" << p0Ack
          << ",\"verloren_ausserhalb_neustart\":" << verlorenAussen
          << ",\"verloren_im_neustartfenster\":" << verlorenFenster
          << ",\"latenz_p95_ms\":" << perzentil (alleLatenzen, 0.95)
          << ",\"latenz_max_ms\":"
          << (alleLatenzen.empty() ? 0.0
              : *std::max_element (alleLatenzen.begin(), alleLatenzen.end()))
          << ",\"gemessen_ausserhalb\":" << alleLatenzen.size()
          << ",\"gemessen_im_stoerfenster\":" << fensterLatenzen.size()
          << ",\"latenz_max_im_stoerfenster_ms\":"
          << (fensterLatenzen.empty() ? 0.0
              : *std::max_element (fensterLatenzen.begin(), fensterLatenzen.end()))
          << ",\"schranke_ms\":" << kP0SchrankeMs << "}"
          << ",\"liveness\":{\"stale_ausserhalb_neustart\":" << staleAusserhalb.load()
          << ",\"evicted_ausserhalb_neustart\":" << evictedAusserhalb.load()
          << ",\"stale_im_neustartfenster\":" << staleImFenster.load() << "}"
          << ",\"langsam\":{\"sonden\":" << langsam
          << ",\"immer_mitglied\":" << (langsameImmerMitglied ? "true" : "false")
          << ",\"ersetzte_liveframes\":" << ersetzt
          << ",\"neueste_verworfen\":" << neuesteVerworfen
          << ",\"abgelehnt\":" << (zuGross + neuesteVerworfen)
          << ",\"zu_gross\":" << zuGross
          << ",\"kollisionsloecher\":" << kollision
          << ",\"schnelle_p95_ms\":" << perzentil (schnelleLatenzen, 0.95)
          << ",\"blockiert_andere_nicht\":"
          << (anzahl == langsam ? "\"nicht_anwendbar\""
              : (perzentil (schnelleLatenzen, 0.95) <= kP0SchrankeMs ? "true" : "false"))
          << "}"
          << ",\"audio\":{\"bloecke\":" << bloecke
          << ",\"ganzblockdrops_ueberlauf\":" << dropsUeberlauf
          << ",\"ganzblockdrops_oversize\":" << dropsOversize
          << ",\"kontinuitaetsbrueche\":" << brueche
          << ",\"publikationen\":" << publikationen << "}"
          << ",\"pipe\":{\"fremder_name_versucht\":" << fremderPipename.load() << "}"
          << ",\"subscription\":{\"weg_im_neustartfenster\":"
          << subscriptionWegImFenster.load() << "}"
          << ",\"kill\":" << killBericht()
          << "}";
        return o.str();
    }

    std::string killBericht() const
    {
        auto urteil = [] (bool gefahren, bool getroffen) {
            return ! gefahren ? "\"nicht_gefahren\""
                              : (getroffen ? "\"getroffen\"" : "\"nicht_getroffen\"");
        };
        std::uint64_t ks1 = 0, ks3 = 0, ks4 = 0;
        bool ks2Gefahren = false, ks2Getroffen = false;
        bool ks5Gefahren = false; int ks5Deckel = 0;
        for (const auto& n : neustarts)
        {
            ks1 += n.telemetrieHandleFehler;
            ks3 += n.p0OhneAckImFenster;
            ks4 += n.flagZumKillzeitpunkt;
            if (n.ks2Barriere) { ks2Gefahren = true; ks2Getroffen = ! n.snapshotVorKill; }
            if (n.ks5Totzeit)  { ks5Gefahren = true; ks5Deckel = n.backoffDeckelErreicht; }
        }
        const bool ks4Gefahren = std::any_of (neustarts.begin(), neustarts.end(),
                                              [] (const Neustart& n) { return n.ks4Barriere; });
        std::ostringstream o;
        o << "{"
          << "\"k_s1\":{\"urteil\":" << urteil (! neustarts.empty(), ks1 > 0)
          << ",\"telemetrie_handle_fehler\":" << ks1 << "}"
          << ",\"k_s2\":{\"urteil\":" << urteil (ks2Gefahren, ks2Getroffen)
          << ",\"snapshot_vor_kill\":" << ((ks2Gefahren && ! ks2Getroffen) ? "true" : "false") << "}"
          << ",\"k_s3\":{\"urteil\":" << urteil (! neustarts.empty(), ks3 > 0)
          << ",\"p0_ohne_ack_im_fenster\":" << ks3 << "}"
          << ",\"k_s4\":{\"urteil\":" << urteil (ks4Gefahren, ks4 > 0)
          << ",\"flag_zum_killzeitpunkt\":" << ks4 << "}"
          << ",\"k_s5\":{\"urteil\":" << urteil (ks5Gefahren, ks5Deckel == anzahl + 1)
          << ",\"backoff_deckel_erreicht\":" << ks5Deckel
          << ",\"erwartet\":" << (anzahl + 1) << "}"
          << "}";
        return o.str();
    }
};
} // namespace

int main (int argc, char** argv)
{
    if (argc < 2 || ! istProbePipename (argv[1]))
    {
        std::cerr << "VERWEIGERT: Aufruf EqCopSessionSoak <probe-pipe> [sonden] "
                     "[messsekunden] [warmupsekunden] [langsam-prozent] [langsam-ms]\n"
                     "Nur der Probe-Namensraum " << kPipePraefixProbe << " ist zulaessig.\n";
        return 3;
    }
    const std::string pipe = argv[1];
    const int sonden   = argc > 2 ? std::atoi (argv[2]) : 16;
    const int messS    = argc > 3 ? std::atoi (argv[3]) : 120;
    const int warmupS  = argc > 4 ? std::atoi (argv[4]) : 20;
    const int langsamP = argc > 5 ? std::atoi (argv[5]) : 25;
    if (argc > 6) gLangsamMs = std::max (0, std::atoi (argv[6]));
    if (sonden < 1 || messS < 1 || warmupS < 0)
    {
        std::cerr << "VERWEIGERT: unbrauchbare Parameter\n";
        return 3;
    }
    // max(1, round(N * Anteil)) — Manifest S16.
    const int langsam = std::max (1, static_cast<int> (
        std::lround (static_cast<double> (sonden) * static_cast<double> (langsamP) / 100.0)));

    const auto brokerPfad = std::filesystem::absolute (
        std::filesystem::path (L"broker/target/release/eqcop-broker-sonde012-probe.exe"));
    const auto erwartung = serverErwartungFuerTestdatei (brokerPfad.wstring());
    if (erwartung.absoluterBrokerPfad.empty())
    {
        std::cerr << "VORAUSSETZUNG FEHLT: Probe-Broker nicht hashbar\n";
        return 3;
    }

    Soak soak (pipe, sonden, std::min (langsam, sonden), erwartung);
    if (! soak.aufbauen())
    {
        soak.abbauen();
        std::cerr << "FEHLER: Topologie stand nicht innerhalb " << kFristMs << " ms\n";
        return 1;
    }
    soak.starteFaeden();

    std::this_thread::sleep_for (std::chrono::seconds (warmupS));
    soak.warmupVorbei.store (true);
    melde ("WARMUP_FERTIG");

    // Befehlsschleife: der Pruefer steuert Kill und Neustart.
    std::string zeile;
    while (std::getline (std::cin, zeile))
    {
        while (! zeile.empty() && (zeile.back() == '\r' || zeile.back() == '\n'))
            zeile.pop_back();
        std::istringstream z (zeile);
        std::string befehl;
        z >> befehl;
        if (befehl == "ENDE") break;
        if (befehl == "KILL_VORBEREITEN")
        {
            int i = 0; std::string art;
            z >> i >> art;
            soak.versucheFesthalten();
            soak.killVorbereiten (i);
            std::lock_guard<std::mutex> l (soak.neustartSchloss);
            if (! soak.neustarts.empty())
            {
                soak.neustarts.back().ks2Barriere = (art == "k_s2");
                soak.neustarts.back().ks4Barriere = (art == "k_s4");
            }
        }
        else if (befehl == "KILL_GESCHEHEN")
        {
            int i = 0; z >> i;
            soak.killGeschehen (i);
        }
        else if (befehl == "BARRIERE")
        {
            // Schaltet das Fenster fuer K-S1 (Rueckstau) und K-S4 (lange
            // Verzoegerung) scharf. Der Pruefer wartet danach auf
            // LANGSAM_IN_VERZOEGERUNG und killt erst dann.
            std::string art;
            z >> art;
            soak.flutAn.store (true);
            soak.langsamBarriere.store (art == "k_s4");
        }
        else if (befehl == "BARRIERE_AUS")
        {
            soak.flutAn.store (false);
            soak.langsamBarriere.store (false);
        }
        else if (befehl == "MAIN_RECONNECT")
        {
            // Erzwingt ein FRISCHES subscribe_session, damit K-S2 ueberhaupt
            // ein Fenster hat: nach einem abgeschlossenen Neustart subscribt
            // der Main von sich aus nicht noch einmal. `reconnect()` ist
            // bestehende Client-API (`core/ipc/ControlClient.h:242`), kein
            // neuer Produktpfad.
            soak.mainControl->reconnect();
        }
        else if (befehl == "BEREIT")
        {
            int i = 0; long long totzeit = 0;
            z >> i >> totzeit;
            soak.bereitWieder (i, totzeit);
        }
    }

    soak.abbauen();
    std::cout << soak.bericht (messS / 60, warmupS) << std::endl;
    return 0;
}
