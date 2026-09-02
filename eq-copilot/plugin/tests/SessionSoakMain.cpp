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
//   TOTZEIT_ERFASST <i>             K-S1/K-S5 sind WAEHREND der Totzeit belegt
//   NEUSTART_VOLLSTAENDIG <i> <ms>  Zeile S11
//   {json}                          Schlussbericht
// stdin (Pruefer -> dieses Programm):
//   KILL_VORBEREITEN <i> <art>  Vorbedingung K-S1 festhalten -> KILL_BEREIT <i>
//   BARRIERE <art>         Rueckstau (K-S1) und lange Verzoegerung (K-S4) an
//   BARRIERE_AUS           beide wieder aus
//   MAIN_RECONNECT         erzwingt ein frisches subscribe_session (K-S2)
//   KILL_ERFOLGT <i> <ms>  der Brokerprozess ist JETZT tot; erst hier werden
//                          K-S2 und K-S4 gegen den Killzeitpunkt belegt
//   TOTZEIT_ENDE <i>       letzter Augenblick VOR dem neuen Broker; hier
//                          werden K-S1 und K-S5 erfasst -> TOTZEIT_ERFASST
//   BEREIT <i> <totzeit>   der neue Broker horcht; ab hier laeuft die Frist
//   ENDE                   Messzeit vorbei; Nachlauffenster, dann Bericht
//
// WARUM DIESE REIHENFOLGE
// -----------------------
// Jede Zusage wird an IHREM Ereignis belegt, nicht an einer Abtastung daneben.
// Die Vorfassung erfasste K-S2/K-S4 vor `KILL_BEREIT` (also vor dem Kill) und
// K-S5 nach `BEREIT` (also gegen den NEUEN Broker); beide Belege sagten damit
// etwas ueber einen anderen Zeitpunkt aus als das Urteil behauptete
// (Codex-Abschlusspruefung 02.09.2026, Thread 01a0626a, Befunde 11 und 12).

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
/// Nachlauffenster vor dem Abbau (Manifest S14).
///
/// HERGELEITET, nicht geraten: ein Heartbeat wird mit `kHeartbeatMs` Kadenz
/// eingereiht und darf nach Z4 hoechstens `kP0SchrankeMs` auf sein ACK warten.
/// Der letzte vor `ENDE` eingereihte Heartbeat braucht also im schlimmsten
/// Fall eine volle Kadenz plus eine volle Schranke, bevor ein fehlendes ACK
/// wirklich Verlust heisst. Ohne dieses Fenster machte ein unmittelbar zuvor
/// erfolgreich eingereihter Heartbeat einen gesunden Lauf rot.
constexpr int kNachlaufMs = kHeartbeatMs + static_cast<int> (kP0SchrankeMs);
/// S06: Ueberproduktion der LANGSAMEN Sonden je Livetakt.
///
/// Ein langsamer Lesecallback bremst den Control-Lesepfad, nicht die
/// Telemetrieschleuse. Ohne eigene Ueberproduktion blieb `ersetzt` bei den
/// langsamen Sonden 0, und die frueher unter `langsam` berichteten Zahlen
/// stammten in Wahrheit vom K-S1-Fluter (Codex-Befund 5). Die Schleuse fasst
/// zwei Frames (§53.9); drei im selben Takt erzwingen mindestens eine
/// Ersetzung. Die Drahtlast steigt dadurch NICHT — ein ersetzter Frame geht
/// nie auf den Draht.
constexpr int kLangsamBurst = 3;
/// Audio: Blockgroesse und Samplerate des Sondenpfads.
constexpr double kFs = 48000.0;
constexpr int kBlock = 512;
constexpr double kPi = 3.14159265358979323846;

/// S10: das Ziel MUSS ohne die eigenen v3-Clients des `SondeProcessor` gebaut
/// sein. Faellt das Define weg, oeffnen die konstruierten Sondeninstanzen die
/// PRODUKTIONS-Pipe, und ein Zaehler, der nie gesetzt werden kann, merkt davon
/// nichts (Codex-Befund 9). Deshalb ist es hier eine Laufzeitwache mit Exit 3
/// statt eines stillen `#if`.
#if defined (NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
constexpr bool kOhneProduktV3 = true;
#else
constexpr bool kOhneProduktV3 = false;
#endif

/// Monotone Zeit als Zahl — fuer atomare Ereignisstempel.
long long jetztNs()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds> (
        Uhr::now().time_since_epoch()).count();
}

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
    /// S06: JEDER Rueckgabewert von `veroeffentlichen()` wird gezaehlt. Erst
    /// dadurch misst `abgelehnt == zuGross + beanspruchtVerworfen` wirklich
    /// eine nicht zugeordnete Ablehnung statt einer Tautologie.
    std::atomic<std::uint64_t> veroeffentlichungen { 0 };
    std::atomic<std::uint64_t> veroeffentlichungAbgelehnt { 0 };
    std::atomic<bool> inVerzoegerung { false };   ///< K-S4-Barriere
    /// K-S4: Beginn und Ende der kuenstlichen Verzoegerung als Ereignisstempel.
    /// Ein blosses Flag sagt nur etwas ueber den Abfragezeitpunkt aus; mit den
    /// beiden Stempeln laesst sich fragen, ob die Sonde ZUM KILLZEITPUNKT
    /// darin steckte.
    std::atomic<long long> verzoegerungBeginn { 0 }, verzoegerungEnde { 0 };

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

    /// S06: die beiden Ueberladungen von `veroeffentlichen()` gehen nur hier
    /// durch, damit KEIN Rueckgabewert verlorengeht.
    void publiziere (const nakama::analyse::FeatureFrame& f)
    {
        veroeffentlichungen.fetch_add (1);
        if (! telemetrie->veroeffentlichen (f, adr))
            veroeffentlichungAbgelehnt.fetch_add (1);
    }

    void publiziereRoh (const std::uint8_t* daten, std::size_t laenge)
    {
        veroeffentlichungen.fetch_add (1);
        if (! telemetrie->veroeffentlichen (daten, laenge))
            veroeffentlichungAbgelehnt.fetch_add (1);
    }
};

struct Neustart
{
    int index = 0;
    std::string epochAlt, epochNeu;
    long long bereitBisVollstaendigMs = -1;
    long long totzeitMs = 0;
    long long killDauerMs = 0;
    /// S11: eine Dauer JE CLIENTPAAR — Index 0 ist das Main-Paar, 1..N sind
    /// die Sonden. Die Vorfassung trug N Kopien desselben Topologieendes ein;
    /// `min`, `p95` und `max` waren dann keine Verteilung (Codex-Befund 10).
    std::vector<double> reconnectMs;
    std::uint64_t alteEpocheGesehen = 0;
    /// S09 je Neustartfenster: wie oft der Main OHNE aktive Subscription und
    /// wie oft mindestens eine Quelle als `Control::getrennt` gesehen wurde.
    /// Beides muss je Kill > 0 sein.
    std::uint64_t s09SubscriptionWeg = 0;
    std::uint64_t s09QuellenGetrennt = 0;
    std::uint64_t s09Sichten = 0;
    // Killpunkt-Belege
    std::uint64_t telemetrieHandleFehler = 0;    // K-S1, in der Totzeit erfasst
    bool snapshotVorKill = false;                // K-S2, am Killzeitpunkt
    bool ks2Barriere = false;
    std::uint64_t flagZumKillzeitpunkt = 0;      // K-S4, am Killzeitpunkt
    bool ks4Barriere = false;
    int backoffDeckelErreicht = 0;               // K-S5, in der Totzeit erfasst
    bool ks5Totzeit = false;
    bool killErfolgtGesehen = false;
    bool totzeitErfasst = false;
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

    /// S03 BINDEND: jede Snapshot-Uebernahme wird SOFORT geprueft — der
    /// Rueckgabewert und die unmittelbar entstandene Sicht. Ein falscher
    /// Snapshot, der vor dem naechsten 250-ms-Tick korrigiert wird, blieb in
    /// der Vorfassung unsichtbar (Codex-Befund 6).
    std::atomic<std::uint64_t> snapshotPruefungen { 0 }, snapshotVollstaendig { 0 };
    std::atomic<std::uint64_t> snapshotUebernahmen { 0 }, snapshotUngueltig { 0 };
    /// Die 250-ms-Abtastung bleibt ZUSAETZLICH: sie sieht auch Drift, die ohne
    /// neuen Snapshot entsteht (Liveness-Tick).
    std::atomic<std::uint64_t> abtastPruefungen { 0 }, abtastVollstaendig { 0 };
    std::atomic<std::uint64_t> fremdeAdresse { 0 }, fuehrungFalsch { 0 };
    // Zaehler S13
    std::atomic<std::uint64_t> staleAusserhalb { 0 }, staleImFenster { 0 };
    std::atomic<std::uint64_t> evictedAusserhalb { 0 };
    /// S10: jeder Pipename, den dieses Programm einem Client uebergibt, laeuft
    /// durch `clientPipe()` und wird dort gegen die Erlaubnisliste geprueft.
    std::atomic<std::uint64_t> fremderPipename { 0 };
    std::atomic<std::uint64_t> clientPipenamenGeprueft { 0 };
    /// S09 gesamt: wie oft der Main im Neustartfenster OHNE aktive Subscription
    /// gesehen wurde. Das JE-KILL-Urteil steht in `Neustart::s09*`.
    std::atomic<std::uint64_t> subscriptionWegImFenster { 0 };
    /// K-S2: Zeitpunkt des letzten abgeschickten `subscribe_session` und der
    /// letzten UEBERNOMMENEN Snapshot-Ankunft.
    std::atomic<long long> subscribeNs { 0 }, letzterSnapshotNs { 0 };
    /// S14: im Nachlauffenster werden keine neuen Heartbeats mehr eingereiht,
    /// aber alle Clients lesen weiter.
    std::atomic<bool> nachlauf { false };

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

    /// S10: der EINZIGE Weg, auf dem ein Pipename in einen Client gelangt.
    /// Jede Uebergabe wird gegen die Erlaubnisliste `istProbePipename` geprueft
    /// (`core/ipc/PipeToken.h`) und gezaehlt. Ein Zaehler, den kein Aufruf
    /// erreichen kann, ist keine Wache — deshalb liegt die Pruefung hier an
    /// jeder der 2·(N+1) Konstruktionen und nicht einmalig in `main()`.
    const std::string& clientPipe()
    {
        clientPipenamenGeprueft.fetch_add (1);
        if (! istProbePipename (pipe))
            fremderPipename.fetch_add (1);
        return pipe;
    }

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
            clientPipe(),
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
            clientPipe(),
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
                clientPipe(),
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
                clientPipe(),
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
        // Barriere K-S2: eingereiht, noch kein Snapshot da. Der Stempel ist der
        // Bezugspunkt fuer `snapshot_vor_kill` — ein Snapshot zaehlt nur, wenn
        // er NACH diesem Zeitpunkt und VOR dem Kill ankam.
        subscribeNs.store (jetztNs());
        melde ("MAIN_SUBSCRIBE_GESENDET");
    }

    /// S02/S03 an EINER Sicht. Zaehlt in die uebergebenen Zaehler und liefert,
    /// ob die Sicht vollstaendig und richtig war.
    bool pruefeSicht (const eqcop::SourcesModel::Sicht& sicht,
                      std::atomic<std::uint64_t>& pruefungen,
                      std::atomic<std::uint64_t>& vollstaendigZaehler)
    {
        pruefungen.fetch_add (1);
        bool vollstaendig = sicht.quellen.size() == static_cast<std::size_t> (anzahl);
        std::set<std::string> gesehen;
        for (const auto& q : sicht.quellen)
        {
            gesehen.insert (q.instanceId);
            if (erwarteteIds.count (q.instanceId) == 0)
            {
                fremdeAdresse.fetch_add (1);
                vollstaendig = false;
            }
        }
        // Die MENGE der instanceId muss gleich der erwarteten Menge sein — eine
        // blosse Zeilenzahl liesse ein Duplikat durch (Matrix S02).
        if (gesehen != erwarteteIds)
            vollstaendig = false;
        if (sicht.fuehrendesMain != mainAdr.instanceId)
        {
            fuehrungFalsch.fetch_add (1);
            vollstaendig = false;
        }
        if (vollstaendig) vollstaendigZaehler.fetch_add (1);
        return vollstaendig;
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
            std::lock_guard<std::mutex> l (epochSchloss);
            for (const auto& alt : alteEpochen)
                if (! alt.empty() && json.find (alt) != std::string::npos)
                    alteEpocheGesehen.fetch_add (1);
        }
        juce::String grund;
        // S03: der RUECKGABEWERT wird ausgewertet, nicht verworfen. `ignoriert`
        // ist der Normalfall fuer jede Nicht-Snapshot-Nachricht auf derselben
        // Controlverbindung (`SourcesModel.cpp:459-464`); `ungueltig` ist ein
        // Befund und wird gezaehlt.
        const auto ergebnis = model.uebernehmeSessionSnapshot (json, Uhr::now(), grund);
        if (ergebnis == eqcop::SourcesModel::SnapshotErgebnis::ungueltig)
            snapshotUngueltig.fetch_add (1);
        if (ergebnis != eqcop::SourcesModel::SnapshotErgebnis::uebernommen)
            return;

        snapshotUebernahmen.fetch_add (1);
        letzterSnapshotNs.store (jetztNs());
        snapshotSeitSubscribe.store (true);
        // S03 bindend: die UNMITTELBAR entstandene Sicht, nicht die naechste
        // Abtastung. Innerhalb eines Neustartfensters ist eine unvollstaendige
        // Sicht erlaubt (Matrix S03), davor auch das Warmup.
        if (warmupVorbei.load() && ! imNeustartfenster.load())
            pruefeSicht (model.sicht(), snapshotPruefungen, snapshotVollstaendig);
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
            // Beginn VOR dem Flag setzen und Ende NACH dem Loeschen: dann gilt
            // "beginn <= t && ende < beginn" genau dann, wenn die Sonde zum
            // Zeitpunkt t in der Verzoegerung steckte (K-S4).
            s.verzoegerungBeginn.store (jetztNs());
            s.inVerzoegerung.store (true);
            if (barriere) melde ("LANGSAM_IN_VERZOEGERUNG");
            std::this_thread::sleep_for (std::chrono::milliseconds (
                barriere ? kLangsamBarriereMs : gLangsamMs));
            s.inVerzoegerung.store (false);
            s.verzoegerungEnde.store (jetztNs());
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
            // S14-Nachlauffenster: nichts Neues mehr einreihen, aber alle
            // Clients lesen weiter, damit ausstehende ACKs noch ankommen.
            if (nachlauf.load())
            {
                naechster += std::chrono::milliseconds (kHeartbeatMs);
                std::this_thread::sleep_until (naechster);
                continue;
            }
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
                s->publiziere (messframe (s->sequence.fetch_add (1) + 1,
                                          gross ? kGrossSamples : kLiveSamples));
                // S06: der langsame Leser haelt SEINE EIGENE Schleuse im
                // Rueckstau. Zwei weitere Frames im selben Takt uebersteigen
                // den Cap 2 sicher; ohne das gaebe es unter `langsam` nichts
                // zu messen und die Zahlen kaemen vom Fluter (Befund 5).
                if (s->langsam)
                    for (int i = 1; i < kLangsamBurst; ++i)
                        s->publiziere (messframe (s->sequence.fetch_add (1) + 1,
                                                  kLiveSamples));
                // K-S1: Sonde 0 haelt ihre Schleuse (Cap 2) im Rueckstau,
                // damit der verbundene Lauf im Schreibzweig steht und nicht
                // im Leerlauf-Lesezweig (Manifest §6, K-S1 Punkt 2).
                if (s->fluter && flutAn.load())
                    for (int i = 0; i < 12; ++i)
                        s->publiziereRoh (flut.data(), flut.size());
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
                pruefeSicht (sicht, abtastPruefungen, abtastVollstaendig);
                for (const auto& q : sicht.quellen)
                    if (q.control == eqcop::SourcesModel::Control::stale)
                        staleAusserhalb.fetch_add (1);
                if (sicht.quellen.size() < static_cast<std::size_t> (anzahl))
                    evictedAusserhalb.fetch_add (1);
            }
            else if (imNeustartfenster.load())
            {
                const auto sicht = model.sicht();
                const bool ohneSubscription = ! sicht.subscriptionAktiv;
                bool quelleGetrennt = false;
                for (const auto& q : sicht.quellen)
                {
                    if (q.control == eqcop::SourcesModel::Control::stale)
                        staleImFenster.fetch_add (1);
                    // S09: `controlEnde()` setzt fluechtige Zeilen auf
                    // `getrennt` (`SourcesModel.cpp:381-387`). Eine bloss
                    // inaktive Subscription belegt das noch nicht.
                    if (q.control == eqcop::SourcesModel::Control::getrennt)
                        quelleGetrennt = true;
                }
                if (ohneSubscription) subscriptionWegImFenster.fetch_add (1);
                // S09 JE KILL: der Gesamtzaehler liesse einen Kill ohne
                // sichtbares `disconnected` durchgehen, solange ein anderer
                // Kill zaehlte (Befund 8).
                std::lock_guard<std::mutex> l (neustartSchloss);
                const int i = aktuellerNeustart.load();
                if (i >= 0 && i < static_cast<int> (neustarts.size()))
                {
                    ++neustarts[static_cast<std::size_t> (i)].s09Sichten;
                    if (ohneSubscription)
                        ++neustarts[static_cast<std::size_t> (i)].s09SubscriptionWeg;
                    if (quelleGetrennt)
                        ++neustarts[static_cast<std::size_t> (i)].s09QuellenGetrennt;
                }
            }
            // 100 ms statt 250 ms: das Neustartfenster ist im Kanon rund 2,5 s
            // lang; bei 250 ms blieben darin zu wenige Sichten, um `getrennt`
            // je Kill sicher zu treffen.
            std::this_thread::sleep_for (std::chrono::milliseconds (100));
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
    /// VOR dem Kill: nur die VORBEDINGUNG von K-S1 (war die Sonde verbunden und
    /// im Rueckstau?). Alles, was den ZUSTAND ZUM KILLZEITPUNKT behauptet,
    /// gehoert nach `killErfolgt()` — hier lebt der Broker noch.
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
        }
        {
            std::lock_guard<std::mutex> l (neustartSchloss);
            neustarts.push_back (n);
            aktuellerNeustart.store (static_cast<int> (neustarts.size()) - 1);
        }
        melde ("KILL_BEREIT " + std::to_string (index));
    }

    /// Der Brokerprozess ist JETZT tot. Erst hier werden K-S2 und K-S4 belegt.
    ///
    /// Der Zeitpunkt wird beim Empfang dieser Zeile gestempelt. Zwischen dem
    /// Ruecksprung aus `Popen.kill()` im Pruefer und diesem Stempel liegen ein
    /// Pipe-Write und ein `getline` — Bruchteile einer Millisekunde gegen ein
    /// 3.000-ms-Barrierefenster. Der Rest wirkt konservativ: ein zu spaeter
    /// Stempel meldet hoechstens `nicht_getroffen`, nie einen falschen Treffer.
    void killErfolgt (int, long long killDauerMs)
    {
        const auto kz = jetztNs();
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

        std::lock_guard<std::mutex> l (neustartSchloss);
        if (neustarts.empty()) return;
        auto& n = neustarts.back();
        n.killErfolgtGesehen = true;
        n.killDauerMs = killDauerMs;
        // K-S2: kam zwischen dem abgeschickten `subscribe_session` und dem Kill
        // ein uebernommener Snapshot an? Dann war das Fenster nicht offen.
        const auto sub = subscribeNs.load();
        const auto snap = letzterSnapshotNs.load();
        n.snapshotVorKill = sub > 0 && snap > sub && snap <= kz;
        // K-S4: steckte die Sonde ZUM KILLZEITPUNKT in ihrer Verzoegerung?
        for (auto& s : sonden)
        {
            if (! s->langsam) continue;
            const auto beginn = s->verzoegerungBeginn.load();
            const auto ende = s->verzoegerungEnde.load();
            if (beginn > 0 && beginn <= kz && ende < beginn)
                ++n.flagZumKillzeitpunkt;
        }
    }

    /// Letzter Augenblick VOR dem neuen Broker. K-S1 und K-S5 werden hier
    /// erfasst, weil beide Aussagen ueber die TOTZEIT sind: der Fehler auf dem
    /// offenen P2-Handle des alten Brokers und der erreichte Backoff-Deckel.
    /// Nach `BEREIT` gemessen, konnte schon ein erfolgreicher Versuch gegen den
    /// NEUEN Broker den Deckelzaehler fuellen (Befund 12).
    void totzeitEnde (int)
    {
        std::lock_guard<std::mutex> l (neustartSchloss);
        if (neustarts.empty()) return;
        auto& n = neustarts.back();
        for (auto& s : sonden)
        {
            const auto ts = s->telemetrie->snapshot();
            if (s->vorKillVerbunden && s->vorKillRueckstau
                && ! ts.letzterFehler.empty() && ! istVertragstext (ts.letzterFehler)
                && ts.verbindungsVersuche == s->vorKillVersuche)
                ++n.telemetrieHandleFehler;
        }
        n.backoffDeckelErreicht = backoffDeckel();
        n.totzeitErfasst = true;
        melde ("TOTZEIT_ERFASST " + std::to_string (n.index));
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
        n->ks5Totzeit = totzeitMs >= 20000;

        // S11: die Dauer JE CLIENTPAAR — Index 0 ist das Main-Paar, 1..N sind
        // die Sonden. Gemessen wird vom BEREIT bis zum EIGENEN Wieder-Mitglied-
        // Zeitpunkt: beide Verbindungen des Paares stehen UND das Paar ist in
        // der Sicht des Main wieder da. Die Vorfassung trug N Kopien desselben
        // Topologieendes ein (Befund 10).
        std::vector<double> proPaar (static_cast<std::size_t> (anzahl) + 1, -1.0);
        const auto frist = bereit + std::chrono::milliseconds (kFristMs);
        while (laeuft.load())
        {
            const auto sicht = model.sicht();
            std::set<std::string> mitglied;
            for (const auto& q : sicht.quellen)
                if (q.descriptorVorhanden) mitglied.insert (q.instanceId);
            const auto ms = std::chrono::duration<double, std::milli> (
                Uhr::now() - bereit).count();

            if (proPaar[0] < 0.0
                && mainControl->snapshot().status == ControlClient::Status::verbunden
                && mainTelemetrie->snapshot().status == TelemetryClient::Status::verbunden
                && sicht.subscriptionAktiv
                && sicht.fuehrendesMain == mainAdr.instanceId)
                proPaar[0] = ms;

            for (int i = 0; i < anzahl; ++i)
            {
                auto& p = proPaar[static_cast<std::size_t> (i) + 1];
                if (p >= 0.0) continue;
                const auto& s = *sonden[static_cast<std::size_t> (i)];
                if (s.control->snapshot().status == ControlClient::Status::verbunden
                    && s.telemetrie->snapshot().status == TelemetryClient::Status::verbunden
                    && mitglied.count (s.adr.instanceId) > 0)
                    p = ms;
            }

            if (std::none_of (proPaar.begin(), proPaar.end(),
                              [] (double v) { return v < 0.0; }))
                break;
            if (Uhr::now() >= frist) break;
            std::this_thread::sleep_for (std::chrono::milliseconds (25));
        }

        // Ein Paar, das die Frist verfehlt hat, traegt Frist + 1 ms — es faellt
        // damit in `max` auf und wird nie stillschweigend ausgelassen.
        const bool alle = std::none_of (proPaar.begin(), proPaar.end(),
                                        [] (double v) { return v < 0.0; });
        for (auto& v : proPaar)
            if (v < 0.0) v = static_cast<double> (kFristMs) + 1.0;
        n->reconnectMs = proPaar;
        n->bereitBisVollstaendigMs = alle
            ? static_cast<long long> (
                  *std::max_element (proPaar.begin(), proPaar.end()))
            : -1;
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
        // S06: die Cap-Zaehler NUR ueber die langsamen Sonden. Der K-S1-Fluter
        // wird getrennt gefuehrt und zaehlt nicht fuer S06 (Befund 5).
        std::uint64_t ersetzt = 0, neuesteVerworfen = 0, zuGross = 0, kollision = 0;
        std::uint64_t veroeffentlicht = 0, abgelehnt = 0;
        std::uint64_t flErsetzt = 0, flNeuesteVerworfen = 0, flZuGross = 0;
        std::uint64_t flKollision = 0, flVeroeffentlicht = 0, flAbgelehnt = 0;
        std::uint64_t bloecke = 0, dropsUeberlauf = 0, dropsOversize = 0;
        std::uint64_t brueche = 0, publikationen = 0;
        std::uint64_t publikationenMin = 0, bloeckeMin = 0;
        bool ersteSonde = true;
        bool langsameImmerMitglied = true;

        const auto sicht = model.sicht();
        std::set<std::string> sichtbar;
        for (const auto& q : sicht.quellen) sichtbar.insert (q.instanceId);

        for (const auto& s : sonden)
        {
            p0Ges += s->p0Gesendet.load();
            p0Ack += s->p0Beantwortet.load();
            const auto ts = s->telemetrie->snapshot();
            if (s->langsam)
            {
                ersetzt += ts.ersetzt;
                neuesteVerworfen += ts.beanspruchtVerworfen;
                zuGross += ts.zuGross;
                kollision += ts.kollisionsLoecher;
                veroeffentlicht += s->veroeffentlichungen.load();
                abgelehnt += s->veroeffentlichungAbgelehnt.load();
            }
            if (s->fluter)
            {
                flErsetzt += ts.ersetzt;
                flNeuesteVerworfen += ts.beanspruchtVerworfen;
                flZuGross += ts.zuGross;
                flKollision += ts.kollisionsLoecher;
                flVeroeffentlicht += s->veroeffentlichungen.load();
                flAbgelehnt += s->veroeffentlichungAbgelehnt.load();
            }
            const auto sBloecke = s->bloecke.load();
            const auto sPubl = s->prozessor->producerPublikationenFuerTest();
            bloecke += sBloecke;
            dropsUeberlauf += s->prozessor->analyseDropsUeberlaufFuerTest();
            dropsOversize += s->prozessor->analyseDropsOversizeFuerTest();
            publikationen += sPubl;
            // S08: das MINIMUM je Sonde. Eine Summe verbirgt einen einzelnen
            // stehengebliebenen Analyseworker hinter fuenfzehn laufenden
            // (Befund 7).
            if (ersteSonde) { publikationenMin = sPubl; bloeckeMin = sBloecke; ersteSonde = false; }
            else
            {
                publikationenMin = std::min (publikationenMin, sPubl);
                bloeckeMin = std::min (bloeckeMin, sBloecke);
            }
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
          << ",\"uebernahmen\":" << snapshotUebernahmen.load()
          << ",\"ungueltig\":" << snapshotUngueltig.load()
          << ",\"abtast_pruefungen\":" << abtastPruefungen.load()
          << ",\"abtast_vollstaendig\":" << abtastVollstaendig.load()
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
              << ",\"reconnect_paare\":" << n.reconnectMs.size()
              << ",\"bereit_bis_vollstaendig_ms\":" << n.bereitBisVollstaendigMs
              << ",\"ueber_schranke\":"
              << ((n.bereitBisVollstaendigMs > kSchrankeMs) ? "true" : "false")
              << ",\"alte_epoche_nach_neustart_gesehen\":" << n.alteEpocheGesehen
              << ",\"s09_sichten\":" << n.s09Sichten
              << ",\"s09_subscription_weg\":" << n.s09SubscriptionWeg
              << ",\"s09_quellen_getrennt\":" << n.s09QuellenGetrennt
              << ",\"kill_erfolgt_gesehen\":" << (n.killErfolgtGesehen ? "true" : "false")
              << ",\"kill_dauer_ms\":" << n.killDauerMs
              << ",\"totzeit_erfasst\":" << (n.totzeitErfasst ? "true" : "false")
              << ",\"backoff_deckel_in_totzeit\":" << n.backoffDeckelErreicht
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
          << ",\"schranke_ms\":" << kP0SchrankeMs
          << ",\"nachlauf_ms\":" << kNachlaufMs << "}"
          << ",\"liveness\":{\"stale_ausserhalb_neustart\":" << staleAusserhalb.load()
          << ",\"evicted_ausserhalb_neustart\":" << evictedAusserhalb.load()
          << ",\"stale_im_neustartfenster\":" << staleImFenster.load() << "}"
          << ",\"langsam\":{\"sonden\":" << langsam
          << ",\"immer_mitglied\":" << (langsameImmerMitglied ? "true" : "false")
          << ",\"ersetzte_liveframes\":" << ersetzt
          << ",\"neueste_verworfen\":" << neuesteVerworfen
          // ABGELEHNT kommt aus den Rueckgabewerten von `veroeffentlichen()`,
          // nicht aus `zuGross + beanspruchtVerworfen`. Erst dadurch kann die
          // Pruefung `abgelehnt == zu_gross + neueste_verworfen` ueberhaupt
          // eine nicht zugeordnete Ablehnung finden (Befund 4).
          << ",\"abgelehnt\":" << abgelehnt
          << ",\"veroeffentlichungen\":" << veroeffentlicht
          << ",\"zu_gross\":" << zuGross
          << ",\"kollisionsloecher\":" << kollision
          << ",\"schnelle_p95_ms\":" << perzentil (schnelleLatenzen, 0.95)
          << ",\"fluter_ueberlappt\":"
          << (sonden.empty() ? "false" : (sonden[0]->langsam ? "true" : "false"))
          << ",\"blockiert_andere_nicht\":"
          << (anzahl == langsam ? "\"nicht_anwendbar\""
              : (perzentil (schnelleLatenzen, 0.95) <= kP0SchrankeMs ? "true" : "false"))
          << "}"
          // Der K-S1-Fluter (Sonde 0) getrennt. Seine Flut belegt K-S1, nie S06.
          << ",\"fluter\":{\"ersetzte_liveframes\":" << flErsetzt
          << ",\"neueste_verworfen\":" << flNeuesteVerworfen
          << ",\"abgelehnt\":" << flAbgelehnt
          << ",\"veroeffentlichungen\":" << flVeroeffentlicht
          << ",\"zu_gross\":" << flZuGross
          << ",\"kollisionsloecher\":" << flKollision << "}"
          << ",\"audio\":{\"bloecke\":" << bloecke
          << ",\"ganzblockdrops_ueberlauf\":" << dropsUeberlauf
          << ",\"ganzblockdrops_oversize\":" << dropsOversize
          << ",\"kontinuitaetsbrueche\":" << brueche
          << ",\"publikationen\":" << publikationen
          << ",\"publikationen_je_sonde_min\":" << publikationenMin
          << ",\"bloecke_je_sonde_min\":" << bloeckeMin
          << ",\"samplerate\":" << kFs
          << ",\"blockgroesse\":" << kBlock << "}"
          << ",\"pipe\":{\"fremder_name_versucht\":" << fremderPipename.load()
          << ",\"clientnamen_geprueft\":" << clientPipenamenGeprueft.load()
          << ",\"ohne_produkt_v3\":" << (kOhneProduktV3 ? "true" : "false") << "}"
          << ",\"subscription\":{\"weg_im_neustartfenster\":"
          << subscriptionWegImFenster.load() << "}"
          << ",\"kill\":" << killBericht (verlorenFenster)
          << "}";
        return o.str();
    }

    /// Jedes Urteil folgt allein aus seinem Belegfeld (§7). `gefahren` steht
    /// als eigenes Feld daneben, damit der Pruefer das Urteil NACHRECHNEN kann,
    /// statt es zu uebernehmen (Befund 14).
    ///
    /// `verlorenFenster` kommt aus dem Schlussbericht, also NACH dem
    /// Nachlauffenster: nur ein dort noch offener ACK ist endgueltig verloren.
    /// Die Vorfassung zaehlte in `bereitWieder()` alles, was zu diesem fruehen
    /// Zeitpunkt unbeantwortet war — spaeter eintreffende ACKs widerlegten den
    /// Treffer (Befund 1).
    std::string killBericht (std::uint64_t verlorenFenster) const
    {
        auto urteil = [] (bool gefahren, bool getroffen) {
            return ! gefahren ? "\"nicht_gefahren\""
                              : (getroffen ? "\"getroffen\"" : "\"nicht_getroffen\"");
        };
        std::uint64_t ks1 = 0, ks4 = 0;
        bool ks2Gefahren = false, ks2SnapshotVorKill = false;
        bool ks5Gefahren = false; int ks5Deckel = 0;
        for (const auto& n : neustarts)
        {
            ks1 += n.telemetrieHandleFehler;
            ks4 += n.flagZumKillzeitpunkt;
            if (n.ks2Barriere)
            {
                ks2Gefahren = true;
                ks2SnapshotVorKill = n.snapshotVorKill;
            }
            if (n.ks5Totzeit)
            {
                ks5Gefahren = true;
                ks5Deckel = n.backoffDeckelErreicht;
            }
        }
        const bool ks4Gefahren = std::any_of (neustarts.begin(), neustarts.end(),
                                              [] (const Neustart& n) { return n.ks4Barriere; });
        // K-S1 und K-S3 brauchen einen Kill, dessen Beleg auch erfasst wurde.
        const bool killGefahren = std::any_of (
            neustarts.begin(), neustarts.end(),
            [] (const Neustart& n) { return n.killErfolgtGesehen; });
        const bool totzeitGefahren = std::any_of (
            neustarts.begin(), neustarts.end(),
            [] (const Neustart& n) { return n.totzeitErfasst; });
        std::ostringstream o;
        o << "{"
          << "\"k_s1\":{\"urteil\":" << urteil (totzeitGefahren, ks1 > 0)
          << ",\"gefahren\":" << (totzeitGefahren ? "true" : "false")
          << ",\"telemetrie_handle_fehler\":" << ks1 << "}"
          << ",\"k_s2\":{\"urteil\":" << urteil (ks2Gefahren, ! ks2SnapshotVorKill)
          << ",\"gefahren\":" << (ks2Gefahren ? "true" : "false")
          << ",\"snapshot_vor_kill\":" << (ks2SnapshotVorKill ? "true" : "false") << "}"
          << ",\"k_s3\":{\"urteil\":" << urteil (killGefahren, verlorenFenster > 0)
          << ",\"gefahren\":" << (killGefahren ? "true" : "false")
          << ",\"p0_ohne_ack_im_fenster\":" << verlorenFenster << "}"
          << ",\"k_s4\":{\"urteil\":" << urteil (ks4Gefahren, ks4 > 0)
          << ",\"gefahren\":" << (ks4Gefahren ? "true" : "false")
          << ",\"flag_zum_killzeitpunkt\":" << ks4 << "}"
          << ",\"k_s5\":{\"urteil\":" << urteil (ks5Gefahren, ks5Deckel == anzahl + 1)
          << ",\"gefahren\":" << (ks5Gefahren ? "true" : "false")
          << ",\"backoff_deckel_erreicht\":" << ks5Deckel
          << ",\"erwartet\":" << (anzahl + 1) << "}"
          << "}";
        return o.str();
    }
};
} // namespace

int main (int argc, char** argv)
{
    // S10: OHNE dieses Define starten die konstruierten `SondeProcessor` ihre
    // eigenen v3-Clients und oeffnen die PRODUKTIONS-Pipe
    // (`sonde/SondeProcessor.cpp:100-105`). Der argv-Riegel sieht diesen
    // internen Weg nicht — deshalb steht hier eine eigene Wache mit Exit 3
    // statt eines Zaehlers, den nichts setzen kann (Befund 9).
    if (! kOhneProduktV3)
    {
        std::cerr << "VORAUSSETZUNG FEHLT: EqCopSessionSoak ist ohne "
                     "NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3 gebaut. Die "
                     "SondeProcessor-Instanzen wuerden die PRODUKTIONS-Pipe "
                     "oeffnen (CLAUDE.md: Probe-Pipe ist nie Produktions-Pipe).\n";
        return 3;
    }
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
        else if (befehl == "KILL_ERFOLGT")
        {
            int i = 0; long long dauer = 0;
            z >> i >> dauer;
            soak.killErfolgt (i, dauer);
        }
        else if (befehl == "TOTZEIT_ENDE")
        {
            int i = 0; z >> i;
            soak.totzeitEnde (i);
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

    // S14-Nachlauffenster VOR dem Abbau: keine neuen Heartbeats mehr, aber
    // alle Clients lesen weiter. Erst danach heisst ein offener ACK Verlust.
    soak.nachlauf.store (true);
    std::this_thread::sleep_for (std::chrono::milliseconds (kNachlaufMs));
    soak.abbauen();
    std::cout << soak.bericht (messS / 60, warmupS) << std::endl;
    return 0;
}
