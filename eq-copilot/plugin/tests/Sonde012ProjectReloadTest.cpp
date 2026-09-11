// SONDE-012 B3b — MainProject Save/Load und Host-Dirty.
// Das echte Processor-State-Gespann wird benutzt; die Produkt-v3-Threads sind
// fuer dieses Ziel abgeschaltet, daher wird keine Produktionspipe beruehrt.

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "WireEnvelope.h"
// NAK-246 D2 (M-06, M-07, M-09): der geteilte v3-Probe-Server und die
// Stoppfrist des ControlClients - die Lebensdauerfaelle fahren den ECHTEN
// Prozessor mit seiner Produktverdrahtung gegen einen Testserver.
#include "IpcVerbindung.h"
#include "PipeToken.h"
#include "controlclient/Intern.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <new>
#include <string>
#include <thread>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
 #define NOMINMAX
#endif
#include <windows.h>

namespace
{
// Der Probe-Server spricht den v3-Bootstrap unqualifiziert (wie in B10/B23).
using namespace nakama::ipc;
#include "V3TestServer.h"

int fehler = 0;
int bestanden = 0;

void pruefe (bool ok, const char* name, const juce::String& detail = {})
{
    std::cout << (ok ? "  ok      " : "  FEHLER  ") << name;
    if (detail.isNotEmpty())
        std::cout << "  [" << detail.toRawUTF8() << "]";
    std::cout << '\n';
    ok ? ++bestanden : ++fehler;
}

template <typename Bedingung>
bool warteAuf (int millisekunden, Bedingung&& bedingung)
{
    for (int i = 0; i < millisekunden / 5; ++i)
    {
        if (bedingung())
            return true;
        std::this_thread::sleep_for (std::chrono::milliseconds (5));
    }
    return bedingung();
}

struct DirtyZaehler final : juce::AudioProcessorListener
{
    /// NAK-246 D3: seit dem Worker-Drain meldet auch der Analyse-Workerzug
    /// Host-Dirty (M-11) - der Zaehler wird von zwei Threads geschrieben.
    std::atomic<int> nonParam { 0 };
    void audioProcessorParameterChanged (juce::AudioProcessor*, int, float) override {}
    void audioProcessorChanged (juce::AudioProcessor*, const ChangeDetails& d) override
    {
        if (d.nonParameterStateChanged) ++nonParam;
    }
};

std::string id (char c)
{
    return std::string (32, c);
}

eqcop::SourcesModel::Sicht lebendeQuelle (const std::string& instance)
{
    eqcop::SourcesModel::Sicht s;
    s.subscriptionAktiv = true;
    s.fuehrendesMain = id ('f');
    s.mainDarfSchreiben = true;
    eqcop::SourcesModel::Zeile q;
    q.instanceId = instance;
    q.runtimeNonce = id ('b');
    q.pluginKind = "active_probe";
    q.mitgliedschaft = eqcop::SourcesModel::Mitgliedschaft::unclassified;
    q.control = eqcop::SourcesModel::Control::verbunden;
    q.messung = eqcop::SourcesModel::Messung::fresh;
    q.betrieb = eqcop::SourcesModel::Betrieb::active;
    q.lautheit = eqcop::SourcesModel::Lautheit::gueltig;
    q.messpunkt = eqcop::SourcesModel::Messpunkt::insert;
    q.descriptorVorhanden = true;
    q.sichtbarerName = "Host Piano";
    q.userLabel = "Reported fallback";
    q.hostBusName = "Host Piano";
    q.namensherkunft = eqcop::SourcesModel::Namensherkunft::host;
    q.lufsPaarVorhanden = true;
    q.lufsI = -18.0f;
    q.lufsIUnsicherheitLu = 0.3f;
    q.hauptziel = true;
    s.quellen.push_back (q);
    return s;
}

std::string commandId (const std::string& command)
{
    juce::var root;
    if (juce::JSON::parse (command, root).failed()) return {};
    const auto* o = root.getDynamicObject();
    return o != nullptr && o->getProperty ("command_id").isString()
             ? o->getProperty ("command_id").toString().toStdString() : std::string();
}

std::string ack (const std::string& id, bool erfolg)
{
    if (erfolg)
        return std::string (R"({"type":"command_ack","command_id":")") + id
             + R"(","ergebnis":"angewandt","state_revision":1,"state_hash":")"
             + std::string (64, 'e') + R"("})";
    return std::string (R"({"type":"command_ack","command_id":")") + id
         + R"(","ergebnis":"abgelehnt","state_revision":0,"code":"unauthorized"})";
}

std::string leererSnapshot (const nakama::ipc::ControlHello& h)
{
    const auto mainId = nakama::ipc::instanceAdresseAusState (h.adresse.instanceId);
    return std::string (R"({"type":"session_snapshot","session_epoch":")")
         + h.adresse.sessionEpoch
         + R"(","broker_epoch":"88888888888888888888888888888888","fuehrendes_main":")"
         + mainId
         + R"(","beitritt_bestaetigung_noetig":false,"mitglieder":[]})";
}

void gefaelschtes_command_ack_vor_serverauth_mutiert_keinen_persistenten_projektzustand()
{
    const auto quelle = id ('a');
    eqcop::EqCopilotProcessor processor;
    // Ohne eine vorbereitete Audiolage traegt `v3Hello()` samplerate/blockSize/
    // channels = 0. Der ControlClient weist sein eigenes Hello dann schon vor
    // `CreateFileW` ab ("Audiolage haelt den v3-Vertrag nicht"), oeffnet die
    // Testpipe nie und misst den C-10-Angriff ueberhaupt nicht — der Peer
    // haengt statt dessen in `ConnectNamedPipe`.
    processor.prepareToPlay (48000.0, 512);
    processor.setzeEditorOffen (true);
    // NAK-246 D3 (M-14): das Speichern unten ist der EINZIGE Drain - die
    // Zeile misst, dass er nur anwendet, was ein echter Produkt-Callback
    // bestaetigt hat, nicht den Takt des Workers.
    processor.setzeWorkerDrainFuerTest (false);
    const bool initialisiert = processor.setzeBindung ("hub", "Gen", "");
    processor.setzeSourcesFixtureFuerTest (lebendeQuelle (quelle));
    DirtyZaehler dirty;
    processor.addListener (&dirty);
    // Die Bezugsgroessen VOR dem Senden des Befehls (NAK-246 D3, M-14): ein
    // nur ausstehender, nie bestaetigter Befehl darf weder das Speichern noch
    // den Tick veraendern. Stuende die Bezugsgroesse erst hinter dem Senden,
    // saehe ein Speicher-Drain, der auch Unbestaetigtes anwendet, in beiden
    // Saves dasselbe - und die Wache bliebe gruen.
    juce::MemoryBlock stateVorher;
    processor.getStateInformation (stateVorher);
    const auto mitgliederVorher = processor.holeZustandKopie().mainProjectMitglieder;
    const auto dirtyVorher = dirty.nonParam.load();
    const auto revisionVorher = processor.v3StateRevisionFuerTest();
    const bool eingereiht = processor.bindeSourcesHauptziel (quelle);
    const auto ausstehendVorher = processor.ausstehenderSourcesCommandFuerTest();
    const auto command = commandId (ausstehendVorher);

    const std::string pipe = std::string ("\\\\.\\pipe\\evenacadia.eq-copilot.probe.nak123.c10.")
                           + std::to_string (GetCurrentProcessId());
    const std::wstring pipeW (pipe.begin(), pipe.end());
    HANDLE server = CreateNamedPipeW (
        pipeW.c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
        1, 65536, 65536, 0, nullptr);
    if (server == INVALID_HANDLE_VALUE)
    {
        processor.removeListener (&dirty);
        pruefe (false,
                "gefaelschtes_command_ack_vor_serverauth_mutiert_keinen_persistenten_projektzustand",
                "Testpipe konnte nicht angelegt werden");
        return;
    }

    std::atomic<bool> ackSchreibversuch { false };
    std::atomic<bool> ackVollstaendigGeschrieben { false };
    std::atomic<DWORD> clientBytes { 0 };
    const auto gefaelschtesAck = ack (command, true);
    std::vector<std::uint8_t> ackFrame;
    const bool frameGueltig = nakama::ipc::envelopeSchreiben (
        nakama::ipc::Familie::p0, nakama::ipc::kJsonSchemaMinor,
        reinterpret_cast<const std::uint8_t*> (gefaelschtesAck.data()),
        gefaelschtesAck.size(), ackFrame);
    std::thread peer ([&]
    {
        const bool verbunden = ConnectNamedPipe (server, nullptr) != FALSE
                            || GetLastError() == ERROR_PIPE_CONNECTED;
        if (verbunden && frameGueltig)
        {
            ackSchreibversuch.store (true);
            DWORD geschrieben = 0;
            ackVollstaendigGeschrieben.store (
                WriteFile (server, ackFrame.data(), static_cast<DWORD> (ackFrame.size()),
                           &geschrieben, nullptr) != FALSE
                && geschrieben == static_cast<DWORD> (ackFrame.size()));
            unsigned char byte = 0;
            DWORD gelesen = 0;
            ReadFile (server, &byte, 1, &gelesen, nullptr);
            clientBytes.store (gelesen);
        }
        DisconnectNamedPipe (server);
        CloseHandle (server);
    });

    std::atomic<int> callbacks { 0 };
    std::atomic<bool> authVorFehler { false }, authFreigeben { false };
    auto erwartung = nakama::ipc::serverErwartungFuerEigenprozessTest();
    erwartung.testFehler = nakama::ipc::ServerPruefFehler::hashFalsch;
    erwartung.testVorFehlerErreicht = &authVorFehler;
    erwartung.testFehlerFreigeben = &authFreigeben;
    nakama::ipc::ControlClient angreifer (
        [&] { return processor.v3HelloFuerTest(); }, pipe,
        [&] (const std::string& json) {
            ++callbacks;
            processor.v3AntwortFuerTest (json);
        }, {}, {}, {}, erwartung);
    angreifer.start();
    const bool ackVorAblehnung = warteAuf (4000, [&]
    {
        return authVorFehler.load() && ackVollstaendigGeschrieben.load();
    });
    authFreigeben.store (true);
    const bool authFiel = warteAuf (4000, [&]
    {
        const auto s = angreifer.snapshot();
        return s.serverPruefstatus
                    == nakama::ipc::ServerPruefStatus::belegtAberUnverifiziert
            && s.serverPrueffehler == nakama::ipc::ServerPruefFehler::hashFalsch
            && s.serverPruefungen == 1;
    });
    const auto authZustand = angreifer.snapshot();
    angreifer.stop();
    // Ein ROTER Lauf darf den Kanon nicht blockieren: kommt der Client nicht
    // an, steht der Peer noch in `ConnectNamedPipe` und `join()` kaeme nie
    // zurueck (Prueflistenzeile E-5).
    {
        HANDLE weck = CreateFileW (pipeW.c_str(), GENERIC_READ, 0, nullptr,
                                   OPEN_EXISTING,
                                   SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION,
                                   nullptr);
        if (weck != INVALID_HANDLE_VALUE)
            CloseHandle (weck);
    }
    peer.join();

    // 🔑 NAK-246 D3 (M-14, Regressionswache): ein Speichern OHNE Tick. Der
    // Speicher-Drain (`getStateInformation`) wendet nur an, was ein echter
    // Produkt-Callback bestaetigt hat - das gefaelschte ACK hat keinen
    // erreicht. State-Bytes, Mitglieder, Dirty und Revision sind vor und
    // nach dem Speichern identisch.
    juce::MemoryBlock stateOhneTick;
    processor.getStateInformation (stateOhneTick);
    const bool ohneTickBytesGleich = stateVorher.getSize() == stateOhneTick.getSize()
        && (stateVorher.getSize() == 0
            || std::memcmp (stateVorher.getData(), stateOhneTick.getData(),
                            stateVorher.getSize()) == 0);
    const bool ohneTickUnveraendert = ohneTickBytesGleich
        && processor.holeZustandKopie().mainProjectMitglieder == mitgliederVorher
        && dirty.nonParam == dirtyVorher
        && processor.v3StateRevisionFuerTest() == revisionVorher;
    pruefe (ohneTickUnveraendert,
            "M-14: gefaelschtes_command_ack_vor_serverauth - ein Speichern OHNE Tick "
            "laesst State-Bytes, Mitglieder, Dirty und Revision unveraendert: der "
            "Speicher-Drain wendet nur an, was ein echter Produkt-Callback bestaetigt hat",
            juce::String ((juce::int64) stateOhneTick.getSize()) + " Bytes, Dirty "
                + juce::String (dirty.nonParam.load()) + ", Revision "
                + juce::String ((juce::int64) processor.v3StateRevisionFuerTest()));

    processor.setzeControlTransportFuerTest (authZustand);
    processor.sourcesTick();
    juce::MemoryBlock stateNachher;
    processor.getStateInformation (stateNachher);
    const auto zustandNachher = processor.holeZustandKopie();
    const bool stateBytesGleich = stateVorher.getSize() == stateNachher.getSize()
        && (stateVorher.getSize() == 0
            || std::memcmp (stateVorher.getData(), stateNachher.getData(),
                            stateVorher.getSize()) == 0);
    const bool unverbraucht = processor.ausstehenderSourcesCommandFuerTest()
                           == ausstehendVorher;
    const bool ehrlich = authZustand.status == nakama::ipc::ControlClient::Status::getrennt
        && ! authZustand.brokerPipeFehlt
        && authZustand.serverPruefstatus
                == nakama::ipc::ServerPruefStatus::belegtAberUnverifiziert
        && authZustand.letzterFehler.find ("Server nicht verifiziert")
                != std::string::npos
        && authZustand.p0Gesendet == 0 && authZustand.p1Gesendet == 0
        && authZustand.empfangen == 0
        && processor.sourcesSicht().diagnose
                == eqcop::SourcesModel::Diagnose::serverUnverified
        && ! processor.sourcesSicht().diagnoseHatHandgriff;
    const bool unveraendert = callbacks.load() == 0 && unverbraucht
        && zustandNachher.mainProjectMitglieder == mitgliederVorher
        && dirty.nonParam == dirtyVorher
        && processor.v3StateRevisionFuerTest() == revisionVorher
        && stateBytesGleich;
    processor.removeListener (&dirty);
    pruefe (initialisiert && eingereiht && ! command.empty()
                && frameGueltig && ackSchreibversuch.load()
                && ackVorAblehnung && ackVollstaendigGeschrieben.load()
                && clientBytes.load() == 0
                && authFiel && ehrlich && unveraendert,
            "gefaelschtes_command_ack_vor_serverauth_mutiert_keinen_persistenten_projektzustand");
}

//==============================================================================
// NAK-246 D2 · Besitz laufender Callbacks am ECHTEN Prozessor (Regel R-D2;
// Manifest docs/beweise/NAK-246.md Paragraph 3.2 M-06, M-07, M-09; 5.2)
//==============================================================================
//
// Die drei Faelle fahren den Prozessor mit seiner PRODUKTVERDRAHTUNG - den
// zehn Lambdas aus PluginProcessor.cpp - gegen den geteilten Testserver
// (Aufbau wie B23 `r01Resync`: Probe-Pipe, `v3StartFuerTest`, Handschlag,
// Heartbeat, P0-Wire-Commit). Neu ist der Lebensdauertest: ein
// Produkt-Callback wird an einem Testhaken festgehalten, der Prozessor wird
// ueber die Stoppfrist hinaus zerstoert, der Callback freigegeben. Der
// Prozessor liegt dafuer per Placement-new in einem eigenen, ausgerichteten
// Heap-Puffer; nach der Zerstoerung legt das Bein ein Bytemuster darueber und
// prueft jedes Byte (Paragraph 5.2 Feinheit 6, "Kanarienvogel statt ASan").
//
// Rotlauf gegen den Basis-SHA: dieselben Faelle mit der Alt-Verdrahtung
// (rohes `this`, kein Schliessen) enden mit veraendertem Muster oder einer
// Zugriffsverletzung; die Rohdatei nennt, welches von beiden
// (docs/beweise/roh/NAK-246-rot-M-06.txt, -M-07.txt, -M-09.txt).

namespace nak246
{
using Uhr = std::chrono::steady_clock;
constexpr int kStopFristMs = nakama::ipc::controlclient_intern::kStopFristMs;
/// Freigabe des gehaltenen Callbacks NACH Ablauf der Stoppfrist (Matrix
/// M-07: "Freigabe aus einem dritten Thread nach 500 ms" - gerechnet ab dem
/// Ablauf der Frist, damit `stop()` abloest statt joint).
constexpr int kFreigabeNachFristMs = 500;
constexpr double kFs = 48000.0;
constexpr int kBlock = 512;

long long msSeit (Uhr::time_point t) noexcept
{
    return std::chrono::duration_cast<std::chrono::milliseconds> (Uhr::now() - t).count();
}
long long jetztNs() noexcept
{
    return std::chrono::duration_cast<std::chrono::nanoseconds> (
        Uhr::now().time_since_epoch()).count();
}

/// Eine Kondvar-Schranke (Muster `PushProbe`, `HakenSchleuse` in A3, B10).
struct Schranke
{
    std::mutex m;
    std::condition_variable cv;
    bool erreicht = false;
    bool frei = false;

    void halten()
    {
        std::unique_lock<std::mutex> l (m);
        erreicht = true;
        cv.notify_all();
        cv.wait (l, [this] { return frei; });
    }
    bool warteBisErreicht (int fristMs)
    {
        std::unique_lock<std::mutex> l (m);
        return cv.wait_for (l, std::chrono::milliseconds (fristMs),
                            [this] { return erreicht; });
    }
    void freigeben()
    {
        {
            std::lock_guard<std::mutex> l (m);
            frei = true;
        }
        cv.notify_all();
    }
};

/// Der Prozessor in einem eigenen Bytepuffer. Der Speicher bleibt nach der
/// Zerstoerung reserviert und traegt das Muster - ein Callback, der ihn noch
/// anfasst, hinterlaesst Spuren, statt zufaellig in frischen Speicher zu
/// schreiben. Groesse und Ausrichtung kommen vom Typ selbst.
struct ProzessorPuffer
{
    static constexpr std::uint8_t kMuster = 0xA5;
    static constexpr std::size_t kGroesse = sizeof (eqcop::EqCopilotProcessor);
    static constexpr std::align_val_t kAusrichtung { alignof (eqcop::EqCopilotProcessor) };

    void* roh = ::operator new (kGroesse, kAusrichtung);
    eqcop::EqCopilotProcessor* p = nullptr;

    ~ProzessorPuffer()
    {
        if (p != nullptr)
            zerstoere();
        ::operator delete (roh, kAusrichtung);
    }
    eqcop::EqCopilotProcessor* anlegen (const std::string& pipe,
                                        nakama::ipc::ServerErwartung erwartung)
    {
        p = new (roh) eqcop::EqCopilotProcessor (pipe, std::move (erwartung));
        return p;
    }
    void zerstoere()
    {
        p->~EqCopilotProcessor();
        p = nullptr;
    }
    void musterLegen() { std::memset (roh, kMuster, kGroesse); }
    std::size_t abweichendeBytes() const
    {
        const auto* b = static_cast<const std::uint8_t*> (roh);
        std::size_t n = 0;
        for (std::size_t i = 0; i < kGroesse; ++i)
            if (b[i] != kMuster)
                ++n;
        return n;
    }
};

/// Ein Main mit Bindung und Audiolage - sonst weist der Client sein eigenes
/// Hello ab ("Audiolage haelt den v3-Vertrag nicht"), und der Sender baut
/// ohne gueltige Adresse keinen Wiretext.
eqcop::EqCopilotProcessor* mainAnlegen (ProzessorPuffer& puffer, const std::string& pipe,
                                        const char* fall)
{
    pruefe (nakama::ipc::istProbePipename (pipe),
            (std::string (fall) + ": der Testserver liegt im PROBE-Namensraum").c_str(),
            juce::String (pipe));
    auto* p = puffer.anlegen (pipe, testExeErwartung());
    pruefe (p->v3PipeNameFuerTest() == pipe,
            (std::string (fall) + ": beide v3-Clients zeigen auf die Probe-Pipe "
             "(Testkonstruktor, Paragraph 5.2 Feinheit 5)").c_str());
    p->prepareToPlay (kFs, kBlock);
    pruefe (p->setzeBindung ("hub", "Gen", ""),
            (std::string (fall) + ": der Prozessor ist ein Main mit Bindung").c_str());
    return p;
}

/// M-07 · `v3Antwort` wird im Haken gehalten, der Prozessor ueber die
/// Stoppfrist hinaus zerstoert, der Callback aus einem dritten Thread
/// freigegeben. Der Destruktor muss den Callback zu Ende warten und die
/// Wartezeit messen.
void prozessorabbau_ueber_die_frist_wartet_den_produkt_callback_zu_ende()
{
    std::cout << "== NAK-246 M-07 prozessorabbau_ueber_die_frist_wartet_den_produkt_callback_zu_ende ==\n";
    TestServer server (testPipeName ("nak246-m07"));
    pruefe (server.starten(), "M-07: der Testserver steht");
    ProzessorPuffer puffer;
    auto* p = mainAnlegen (puffer, server.pipeName(), "M-07");
    auto schleuse = p->callbackSchleuseFuerTest();

    Schranke gate;
    std::atomic<int> antworten { 0 };
    // Die ERSTE Antwort (heartbeat_ack des Servers) wird gehalten; die
    // Schranke gehoert dem Bein und ueberlebt den Prozessor.
    p->setzeV3AntwortHakenFuerTest ([&gate, &antworten] (const std::string&)
    {
        if (antworten.fetch_add (1) == 0)
            gate.halten();
    });
    p->v3StartFuerTest();
    pruefe (warteAuf (8000, [&] {
                return p->controlV3Snapshot().status
                       == nakama::ipc::ControlClient::Status::verbunden;
            }),
            "M-07: der echte Client ist ueber die Probe-Pipe verbunden");
    pruefe (gate.warteBisErreicht (8000),
            "M-07: der Produkt-Callback `v3Antwort` steht im Haken - im Zug der "
            "Schleuse, vor dem ersten Zustandszugriff");
    const auto imZug = schleuse->stand();
    pruefe (imZug.aktiv >= 1 && ! imZug.geschlossen,
            "M-07: die Schleuse fuehrt ihn als laufend",
            juce::String ((int) imZug.aktiv) + " aktiv");

    std::atomic<long long> freigabeNs { 0 };
    const auto t0 = Uhr::now();
    std::thread dritter ([&]
    {
        std::this_thread::sleep_until (
            t0 + std::chrono::milliseconds (kStopFristMs + kFreigabeNachFristMs));
        freigabeNs.store (jetztNs());
        gate.freigeben();
    });
    std::cout << "  M-07: Destruktor beginnt; Freigabe nach "
              << (kStopFristMs + kFreigabeNachFristMs) << " ms\n";
    puffer.zerstoere();                 // stop() -> Frist -> Abloesung -> Schliessen wartet
    const auto endeNs = jetztNs();
    const auto dauerMs = msSeit (t0);
    puffer.musterLegen();
    dritter.join();
    std::cout << "  M-07: Destruktor zurueck nach " << dauerMs
              << " ms, Muster gelegt, 300 ms warten\n";
    std::this_thread::sleep_for (std::chrono::milliseconds (300));
    const auto stand = schleuse->stand();
    const auto abweichend = puffer.abweichendeBytes();
    const auto nachFreigabeMs = (endeNs - freigabeNs.load()) / 1000000LL;

    pruefe (dauerMs >= kStopFristMs + kFreigabeNachFristMs,
            "M-07: prozessorabbau_ueber_die_frist_wartet_den_produkt_callback_zu_ende - "
            "der Destruktor kehrt erst zurueck, nachdem der gehaltene Callback "
            "freigegeben wurde; stop() hatte die Frist laengst abgewartet und abgeloest",
            juce::String ((juce::int64) dauerMs) + " ms Destruktor, Frist "
                + juce::String (kStopFristMs) + " ms, Freigabe nach "
                + juce::String (kStopFristMs + kFreigabeNachFristMs) + " ms");
    pruefe (stand.gewartetMs > 0
                && stand.gewartetMs <= static_cast<std::uint64_t> (dauerMs) + 1,
            "M-07: die Wartezeit ist an der Schleuse GEMESSEN - gewartetMs > 0 und "
            "nicht groesser als der Destruktor selbst",
            "gewartetMs=" + juce::String ((juce::int64) stand.gewartetMs));
    pruefe (nachFreigabeMs >= 0 && nachFreigabeMs <= 100,
            "M-07: die Wartezeit ist durch die Callback-Dauer begrenzt - der Destruktor "
            "endet binnen 100 ms nach der Freigabe",
            juce::String ((juce::int64) nachFreigabeMs) + " ms nach der Freigabe");
    pruefe (abweichend == 0,
            "M-07: das Muster ueber dem zerstoerten Prozessor ist in jedem Byte "
            "unveraendert - der Zustandszugriff des Callbacks lag VOR der Zerstoerung",
            juce::String ((juce::int64) abweichend) + " von "
                + juce::String ((juce::int64) ProzessorPuffer::kGroesse) + " Bytes veraendert");
    pruefe (stand.aktiv == 0 && stand.geschlossen,
            "M-07: nach dem Destruktor laeuft kein Callback mehr, die Schleuse ist zu",
            "aktiv=" + juce::String ((int) stand.aktiv)
                + " betreten=" + juce::String ((juce::int64) stand.betreten)
                + " abgewiesen=" + juce::String ((juce::int64) stand.abgewiesen));
    server.stoppen();
}

/// M-06 · ein nachregistrierter Callback (`hookReplayBegin`) wird im
/// Aufbauzug des echten Verbindungsaufbaus gehalten; nach Zerstoerung und
/// Freigabe ruft der abgeloeste Thread im selben Zug `beiP0Verworfen` (fuer
/// den verworfenen Aufbau-Heartbeat) und danach `beiLinkStatus` - alle
/// beginnen NACH dem Schliessen und muessen abgewiesen werden.
void abgeloester_produkt_callback_beruehrt_den_zerstoerten_prozessor_nicht()
{
    std::cout << "== NAK-246 M-06 abgeloester_produkt_callback_beruehrt_den_zerstoerten_prozessor_nicht ==\n";
    TestServer server (testPipeName ("nak246-m06"));
    pruefe (server.starten(), "M-06: der Testserver steht");
    ProzessorPuffer puffer;
    auto* p = mainAnlegen (puffer, server.pipeName(), "M-06");
    auto schleuse = p->callbackSchleuseFuerTest();

    // Der Sender wird BESTAETIGT angehalten (WA-04), dann laufen die
    // Ringereignisse SYNCHRON als P0-Interventionsereignisse der Generation 0
    // in die Queue - genau die Lage, in der der Aufbauzug des ersten Links die
    // Zustellpruefung faehrt und `hookReplayBegin` ruft.
    p->senderAnhaltenFuerTest (true);
    pruefe (p->warteAufSenderPauseFuerTest(), "M-06: der Sender ist bestaetigt angehalten");
    const int imRing = p->interventionsRingFuellenFuerTest();
    pruefe (imRing > 0, "M-06: der Ring nimmt Ereignisse auf", juce::String (imRing));
    p->interventionenSendenFuerTest();
    pruefe (p->interventionsRingFuellstandFuerTest() == 0,
            "M-06: die Ereignisse liegen als P0 in der Queue (Generation 0)");

    // Der zweite Ruf des Hooks - der aus dem ECHTEN Aufbauzug - wird gehalten.
    Schranke gate;
    std::atomic<int> replayRufe { 0 };
    p->setzeReplayBeginHakenFuerTest ([&gate, &replayRufe] (std::uint64_t, std::uint64_t)
    {
        if (replayRufe.fetch_add (1) == 1)
            gate.halten();
    });
    // Ein Testlink (Generation 1): sein Aufbauzug faehrt die Zustellpruefung
    // (Ruf 1) und der positive Link-Callback hinterlegt die Aufbau-Aussage
    // dieser Generation. Der Heartbeat-Schritt verbraucht sie und reiht einen
    // Bericht MIT Marke ein - den verwirft der echte Aufbauzug (Generation 2)
    // und meldet die Marke ueber `beiP0Verworfen`.
    p->v3LinkFuerTest (true);
    pruefe (replayRufe.load() == 1,
            "M-06: der Aufbauzug des Testlinks hat die Zustellpruefung gefahren (Ruf 1)");
    std::string heartbeat;
    pruefe (p->v3HeartbeatSchrittFuerTest (heartbeat, 1) && ! heartbeat.empty(),
            "M-06: ein Aufbau-Heartbeat mit Marke liegt in der Queue (Generation 1)");

    p->v3StartFuerTest();
    pruefe (gate.warteBisErreicht (8000),
            "M-06: der echte Aufbauzug (Generation 2) ruft `hookReplayBegin`, und der "
            "Produkt-Callback steht im Haken - im Zug, unter sendeMutex, vor dem "
            "Zustandszugriff");

    const auto t0 = Uhr::now();
    std::thread dritter ([&]
    {
        std::this_thread::sleep_until (
            t0 + std::chrono::milliseconds (kStopFristMs + kFreigabeNachFristMs));
        gate.freigeben();
    });
    std::cout << "  M-06: Destruktor beginnt; Freigabe nach "
              << (kStopFristMs + kFreigabeNachFristMs) << " ms\n";
    puffer.zerstoere();                 // stop() -> Frist -> Abloesung -> Schliessen wartet
    const auto dauerMs = msSeit (t0);
    puffer.musterLegen();
    dritter.join();
    std::cout << "  M-06: Destruktor zurueck nach " << dauerMs
              << " ms, Muster gelegt, 600 ms warten (der abgeloeste Thread laeuft "
                 "seinen Aufbauzug zu Ende)\n";
    std::this_thread::sleep_for (std::chrono::milliseconds (600));
    const auto stand = schleuse->stand();
    const auto abweichend = puffer.abweichendeBytes();

    pruefe (dauerMs >= kStopFristMs,
            "M-06: stop() hat die volle Frist gewartet und den Clientthread abgeloest "
            "(B-CC-12) - der gehaltene Callback lebte ueber die Frist hinaus",
            juce::String ((juce::int64) dauerMs) + " ms Destruktor bei Frist "
                + juce::String (kStopFristMs) + " ms");
    pruefe (stand.gewartetMs > 0,
            "M-06: der Destruktor hat den gehaltenen Callback zu Ende gewartet",
            "gewartetMs=" + juce::String ((juce::int64) stand.gewartetMs));
    pruefe (abweichend == 0,
            "M-06: abgeloester_produkt_callback_beruehrt_den_zerstoerten_prozessor_nicht - "
            "`beiP0Verworfen` und `beiLinkStatus` aus dem abgeloesten Thread beruehren "
            "den zerstoerten Prozessor NICHT: das Muster ist in jedem Byte unveraendert",
            juce::String ((juce::int64) abweichend) + " von "
                + juce::String ((juce::int64) ProzessorPuffer::kGroesse) + " Bytes veraendert");
    pruefe (stand.abgewiesen >= 1,
            "M-06: und jeder Versuch nach dem Schliessen ist GEZAEHLT - abgewiesen >= 1",
            "abgewiesen=" + juce::String ((juce::int64) stand.abgewiesen)
                + " betreten=" + juce::String ((juce::int64) stand.betreten)
                + " aktiv=" + juce::String ((int) stand.aktiv));
    server.stoppen();
}

/// M-09 · Live-Verbindung: Hello, Status-Heartbeat, Link auf/ab, Antwort,
/// Telemetrie-Frame, P0-Wire-Commit (`beiP0Zugestellt`) und Zustellpruefung
/// (`hookReplayBegin`) laufen ALLE durch die Schleuse - `betreten` ist nach
/// dem Lauf mindestens die Summe der deterministisch ausgeloesten Ereignisse.
void alle_produkt_callbacks_laufen_durch_die_schleuse()
{
    std::cout << "== NAK-246 M-09 alle_produkt_callbacks_laufen_durch_die_schleuse ==\n";
    TestServer server (testPipeName ("nak246-m09"));
    // Nach dem Telemetrie-Welcome schickt der Server einen Schwung
    // vertragsgemaesser P2-Frames - jeder erreicht `beiFrame`.
    server.frameFlutTelemetrieP2.store (1);
    pruefe (server.starten(), "M-09: der Testserver steht");
    ProzessorPuffer puffer;
    auto* p = mainAnlegen (puffer, server.pipeName(), "M-09");
    auto schleuse = p->callbackSchleuseFuerTest();

    p->senderAnhaltenFuerTest (true);
    pruefe (p->warteAufSenderPauseFuerTest(), "M-09: der Sender ist bestaetigt angehalten");
    const int imRing = p->interventionsRingFuellenFuerTest();
    p->interventionenSendenFuerTest();
    pruefe (imRing > 0 && p->interventionsRingFuellstandFuerTest() == 0,
            "M-09: Interventionsereignisse der Generation 0 liegen als P0 in der Queue - "
            "der Aufbauzug faehrt damit die Zustellpruefung, und jeder Wire-Commit "
            "ruft `beiP0Zugestellt`",
            juce::String (imRing));

    std::atomic<int> replayRufe { 0 };
    std::atomic<int> antworten { 0 };
    p->setzeReplayBeginHakenFuerTest ([&replayRufe] (std::uint64_t, std::uint64_t)
                                      { ++replayRufe; });
    p->setzeV3AntwortHakenFuerTest ([&antworten] (const std::string&) { ++antworten; });

    p->v3StartFuerTest();
    p->v3TelemetrieStartFuerTest();
    const bool verbunden = warteAuf (8000, [&] {
        return p->controlV3Snapshot().status == nakama::ipc::ControlClient::Status::verbunden;
    });
    pruefe (verbunden, "M-09: Control ist verbunden (Hello, Welcome, Link auf)");
    const bool teleVerbunden = warteAuf (8000, [&] {
        return p->telemetryV3Snapshot().status == nakama::ipc::TelemetryClient::Status::verbunden;
    });
    pruefe (teleVerbunden, "M-09: Telemetrie ist gekoppelt und verbunden (Telemetrie-Hello)");
    const bool frameKam = warteAuf (8000, [&] { return p->telemetryV3Snapshot().empfangen >= 1; });
    pruefe (frameKam, "M-09: mindestens ein P2-Frame hat `beiFrame` erreicht",
            juce::String ((juce::int64) p->telemetryV3Snapshot().empfangen));
    const bool antwortKam = warteAuf (8000, [&] { return antworten.load() >= 1; });
    pruefe (antwortKam, "M-09: mindestens ein heartbeat_ack hat `v3Antwort` erreicht",
            juce::String (antworten.load()));
    const bool zugestellt = warteAuf (8000, [&] { return p->interventionenGesendetFuerTest() >= 1; });
    pruefe (zugestellt, "M-09: mindestens ein P0-Wire-Commit hat `beiP0Zugestellt` erreicht",
            juce::String ((juce::int64) p->interventionenGesendetFuerTest()));
    bool heartbeatBeimServer = false;
    {
        std::lock_guard<std::mutex> l (server.textMutex);
        for (const auto& t : server.p0Texte)
            heartbeatBeimServer = heartbeatBeimServer
                || t.find ("\"type\":\"heartbeat\"") != std::string::npos;
    }
    pruefe (heartbeatBeimServer, "M-09: der Server hat einen Heartbeat empfangen (`v3Status`)");
    bool helloBeimServer = false;
    {
        std::lock_guard<std::mutex> l (server.textMutex);
        helloBeimServer = ! server.letztesControlHello.empty();
    }
    pruefe (helloBeimServer, "M-09: der Server hat das Control-Hello empfangen (`v3Hello`)");
    pruefe (replayRufe.load() >= 1, "M-09: die Zustellpruefung hat `hookReplayBegin` gerufen",
            juce::String (replayRufe.load()));

    const auto vorZerstoerung = schleuse->stand();
    const auto t0 = Uhr::now();
    puffer.zerstoere();                 // Link ab laeuft synchron VOR dem Schliessen
    const auto dauerMs = msSeit (t0);
    const auto stand = schleuse->stand();

    // Die untere Schranke aus M-09: Hello >= 1, Status >= 1, Link >= 2 (auf und
    // ab), Antwort >= 1, Frame >= 1, beiP0Zugestellt >= 1, hookReplayBegin >= 1.
    const std::uint64_t summe = (helloBeimServer ? 1u : 0u) + (heartbeatBeimServer ? 1u : 0u)
                              + (verbunden ? 2u : 0u) + (antwortKam ? 1u : 0u)
                              + (frameKam ? 1u : 0u) + (zugestellt ? 1u : 0u)
                              + (replayRufe.load() >= 1 ? 1u : 0u);
    pruefe (summe == 8 && stand.betreten >= summe,
            "M-09: alle_produkt_callbacks_laufen_durch_die_schleuse - `betreten` ist "
            "mindestens die Summe der deterministisch ausgeloesten Ereignisse (8): kein "
            "Callback umgeht die Schleuse",
            "betreten=" + juce::String ((juce::int64) stand.betreten)
                + " Summe=" + juce::String ((juce::int64) summe)
                + " (vor der Zerstoerung " + juce::String ((juce::int64) vorZerstoerung.betreten) + ")");
    pruefe (stand.betreten > vorZerstoerung.betreten,
            "M-09: der negative Link-Callback aus stop() lief synchron durch die noch "
            "OFFENE Schleuse - kein Selbstblock (Abweichung 1, Paragraph 5.10)",
            juce::String ((juce::int64) (stand.betreten - vorZerstoerung.betreten)) + " Eintritte im Destruktor");
    pruefe (dauerMs < kStopFristMs && stand.gewartetMs == 0 && stand.abgewiesen == 0,
            "M-09: ohne gehaltenen Callback blockiert der Destruktor nicht - keine Frist, "
            "kein Warten, kein abgewiesener Callback",
            juce::String ((juce::int64) dauerMs) + " ms, gewartetMs="
                + juce::String ((juce::int64) stand.gewartetMs) + ", abgewiesen="
                + juce::String ((juce::int64) stand.abgewiesen));
    server.stoppen();
}

/// `--nur <name>` faehrt genau einen der drei Faelle (Rotlaeufe, deren
/// Ausgang der Prozess nicht ueberlebt).
bool nak246Fall (const std::string& name)
{
    if (name == "m06") { abgeloester_produkt_callback_beruehrt_den_zerstoerten_prozessor_nicht(); return true; }
    if (name == "m07") { prozessorabbau_ueber_die_frist_wartet_den_produkt_callback_zu_ende(); return true; }
    if (name == "m09") { alle_produkt_callbacks_laufen_durch_die_schleuse(); return true; }
    return false;
}
} // namespace nak246

//==============================================================================
// NAK-246 D3 · editorunabhaengiger Persistenzabschluss (Regel R-D3; Manifest
// docs/beweise/NAK-246.md Paragraph 3.3 M-10 bis M-13; 5.3)
//==============================================================================
//
// Der Editor-Timer war der EINZIGE Drain der vom Broker bestaetigten
// Sources-Befehle: ohne offenen Editor wurde ein bestaetigter Join oder
// Unbind nie in den State uebernommen, und das Speichern nahm ihn nicht mit
// (Auditbefund D3). Die vier Faelle messen die drei Drains und ihren Riegel:
//   M-10  das Speichern wendet an, bevor es serialisiert (neue Instanz laedt
//         das gejointe Mitglied und nicht mehr das geloeste; kein Tick);
//   M-11  der Analyse-Workerzug wendet ohne Editor an (Latenz gemessen);
//   M-12  zwei bestaetigte Befehle desselben Mitglieds, zwei Drains, ein
//         Testhaken zwischen Swap und Anwendung: genau einmal je Befehl, in
//         ACK-Reihenfolge - der Riegel `sourcesDrainMutex` klammert beides;
//   M-13  der Reload-Riegel gilt auch im Speicher-Drain (Wache).
// M-14 (Wache) steht oben im Fall `gefaelschtes_command_ack_vor_serverauth`.
// Rotlauf gegen den Basis-SHA: docs/beweise/roh/NAK-246-rot-M-10.txt bis -M-14.txt.

namespace nak246d3
{
using nak246::Schranke;
using nak246::Uhr;
using nak246::msSeit;

std::vector<nakama::state::MainProjectMitglied> mitglieder (const eqcop::EqCopilotProcessor& p)
{
    return p.holeZustandKopie().mainProjectMitglieder;
}

bool genau (const std::vector<nakama::state::MainProjectMitglied>& m,
            std::initializer_list<std::string> ids)
{
    if (m.size() != ids.size())
        return false;
    std::size_t i = 0;
    for (const auto& erwartet : ids)
        if (m[i++].instanceId != juce::String (erwartet))
            return false;
    return true;
}

/// Ein Main mit Bindung; `editorOffen` nur fuer die explizite Initialisierung
/// (Paragraph 53.5), danach wie vom Fall gewuenscht.
std::unique_ptr<eqcop::EqCopilotProcessor> mainAnlegen (bool workerDrain, bool editorBleibtOffen)
{
    // HEAP, nicht Rahmen (NAK-175).
    auto p = std::make_unique<eqcop::EqCopilotProcessor>();
    p->setzeWorkerDrainFuerTest (workerDrain);
    p->setzeEditorOffen (true);
    pruefe (p->setzeBindung ("hub", "Gen", ""), "NAK-246 D3: der Prozessor ist ein Main mit Bindung");
    if (! editorBleibtOffen)
        p->setzeEditorOffen (false);
    return p;
}

/// M-10 · Join A bestaetigt, danach Unbind B bestaetigt, KEIN Tick; der Host
/// speichert; eine neue Instanz laedt genau A. Der Worker-Drain ist fuer
/// diesen Fall abgeschaltet, damit die Zeile am SPEICHERN faellt.
void bestaetigte_join_und_unbind_landen_ohne_tick_im_gespeicherten_state()
{
    std::cout << "== NAK-246 M-10 bestaetigte_join_und_unbind_landen_ohne_tick_im_gespeicherten_state ==\n";
    const auto a = id ('a');
    const auto b = id ('b');
    auto vor = mainAnlegen (false, true);
    DirtyZaehler dirty;
    vor->addListener (&dirty);

    // Aufbau MIT Tick: B ist Mitglied - der Zustand, den ein Wire-Unbind
    // voraussetzt (`sendeSourcesCommand` verlangt `hat`).
    vor->setzeSourcesFixtureFuerTest (lebendeQuelle (b));
    pruefe (vor->bindeSourcesHauptziel (b), "M-10 Aufbau: Join B gesendet");
    vor->v3AntwortFuerTest (ack (commandId (vor->ausstehenderSourcesCommandFuerTest()), true));
    vor->sourcesTick();
    pruefe (genau (mitglieder (*vor), { b }) && dirty.nonParam == 1,
            "M-10 Aufbau: B ist Mitglied (ein Tick, ein Dirty)");

    // Ab hier KEIN Tick mehr. A ist Hauptziel und unklassifiziert; B steht als
    // bestaetigtes Mitglied mit Runtime-Nonce in der Sicht (Wire-Unbind).
    auto sicht = lebendeQuelle (a);
    auto zeileB = lebendeQuelle (b).quellen.front();
    zeileB.mitgliedschaft = eqcop::SourcesModel::Mitgliedschaft::bestaetigt;
    zeileB.hauptziel = false;
    zeileB.runtimeNonce = id ('d');
    sicht.quellen.push_back (zeileB);
    vor->setzeSourcesFixtureFuerTest (std::move (sicht));
    pruefe (vor->bindeSourcesHauptziel (a), "M-10: Join A gesendet");
    const auto joinId = commandId (vor->ausstehenderSourcesCommandFuerTest());
    vor->v3AntwortFuerTest (ack (joinId, true));
    pruefe (vor->waehleSourcesHauptziel (b), "M-10: B wird Hauptziel");
    pruefe (vor->entferneSourcesHauptziel (b), "M-10: Unbind B gesendet (Wire, mit Runtime-Nonce)");
    const auto unbindId = commandId (vor->ausstehenderSourcesCommandFuerTest());
    pruefe (! joinId.empty() && ! unbindId.empty() && joinId != unbindId,
            "M-10: zwei Befehle, zwei Kennungen");
    vor->v3AntwortFuerTest (ack (unbindId, true));

    // Kein Tick: der State traegt noch den alten Stand, nichts ist gemeldet.
    pruefe (genau (mitglieder (*vor), { b }) && dirty.nonParam == 1,
            "M-10: vor dem Speichern ist nichts angewandt - kein Tick lief");
    const auto revisionVor = vor->v3StateRevisionFuerTest();
    const auto dirtyVor = dirty.nonParam.load();

    juce::MemoryBlock state;
    vor->getStateInformation (state);

    pruefe (genau (mitglieder (*vor), { a }),
            "M-10: das Speichern hat die bestaetigten Befehle angewandt - A ist Mitglied, B nicht mehr");
    pruefe (dirty.nonParam == dirtyVor + 2,
            "M-10: Dirty ist je geaendertem Befehl EINMAL gemeldet (+2)",
            juce::String (dirty.nonParam.load()));
    pruefe (vor->v3StateRevisionFuerTest() == revisionVor + 2,
            "M-10: die Revision ist je Befehl um 1 gestiegen (+2)",
            juce::String ((juce::int64) (vor->v3StateRevisionFuerTest() - revisionVor)));
    {
        const auto s = vor->sourcesSicht();
        bool aBestaetigt = false;
        for (const auto& q : s.quellen)
            if (q.instanceId == a)
                aBestaetigt = q.mitgliedschaft == eqcop::SourcesModel::Mitgliedschaft::bestaetigt;
        pruefe (aBestaetigt, "M-10: das Modell ist nachgefuehrt - A gilt als bestaetigt");
    }
    vor->removeListener (&dirty);

    auto nach = std::make_unique<eqcop::EqCopilotProcessor>();
    DirtyZaehler dirtyNach;
    nach->addListener (&dirtyNach);
    nach->setStateInformation (state.getData(), (int) state.getSize());
    const auto geladen = mitglieder (*nach);
    pruefe (genau (geladen, { a })
                && geladen.front().label == "Reported fallback",
            "M-10: bestaetigte_join_und_unbind_landen_ohne_tick_im_gespeicherten_state - "
            "die neue Instanz laedt das gejointe Mitglied und nicht mehr das geloeste",
            juce::String ((int) geladen.size()) + " Mitglied(er)");
    pruefe (dirtyNach.nonParam == 0, "M-10: das Laden meldet kein Dirty");
    nach->removeListener (&dirtyNach);
}

/// M-11 · Join bestaetigt, KEIN Tick, KEIN Speichern, Editor geschlossen: der
/// prozessoreigene Takt (Analyse-Workerzug) wendet an. Latenz gemessen.
void bestaetigtes_ack_wird_ohne_editor_vom_prozessortakt_angewandt()
{
    std::cout << "== NAK-246 M-11 bestaetigtes_ack_wird_ohne_editor_vom_prozessortakt_angewandt ==\n";
    const auto a = id ('a');
    auto p = mainAnlegen (true, false);
    DirtyZaehler dirty;
    p->addListener (&dirty);
    p->setzeSourcesFixtureFuerTest (lebendeQuelle (a));
    pruefe (p->bindeSourcesHauptziel (a), "M-11: Join A gesendet");
    const auto joinId = commandId (p->ausstehenderSourcesCommandFuerTest());
    const auto revisionVor = p->v3StateRevisionFuerTest();

    const auto t0 = Uhr::now();
    p->v3AntwortFuerTest (ack (joinId, true));
    const bool angewandt = warteAuf (2000, [&] { return genau (mitglieder (*p), { a }); });
    const auto latenzMs = msSeit (t0);
    pruefe (angewandt,
            "M-11: bestaetigtes_ack_wird_ohne_editor_vom_prozessortakt_angewandt - ohne "
            "Editor und ohne Tick steht das Mitglied binnen Frist im State (Workerzug "
            "spaetestens alle 50 ms)",
            juce::String ((juce::int64) latenzMs) + " ms von ACK bis Anwendung");
    const bool gemeldet = warteAuf (1000, [&] {
        return dirty.nonParam.load() == 1 && p->v3StateRevisionFuerTest() == revisionVor + 1;
    });
    pruefe (gemeldet, "M-11: Dirty ist einmal gemeldet, die Revision um 1 gestiegen",
            "Dirty " + juce::String (dirty.nonParam.load()) + ", Revision +"
                + juce::String ((juce::int64) (p->v3StateRevisionFuerTest() - revisionVor)));
    // Und GENAU einmal: weitere Zuege melden nichts nach.
    std::this_thread::sleep_for (std::chrono::milliseconds (150));
    pruefe (dirty.nonParam == 1 && p->v3StateRevisionFuerTest() == revisionVor + 1,
            "M-11: nach weiteren Workerzuegen bleibt es bei einem Dirty und einer Revision");
    {
        const auto s = p->sourcesSicht();
        bool aBestaetigt = false;
        for (const auto& q : s.quellen)
            if (q.instanceId == a)
                aBestaetigt = q.mitgliedschaft == eqcop::SourcesModel::Mitgliedschaft::bestaetigt;
        pruefe (aBestaetigt, "M-11: das Modell ist nachgefuehrt - A gilt als bestaetigt");
    }
    p->removeListener (&dirty);
}

/// M-12 · zwei bestaetigte Befehle desselben Mitglieds in ACK-Reihenfolge
/// (confirm_join, dann unbind_probe); Drain A (Workerzug) haelt am Haken
/// zwischen Swap und Anwendung, Drain B (Editor-Tick) laeuft aus einem
/// eigenen Faden. Mit `sourcesDrainMutex` wartet B, bis A fertig ist; ohne
/// ihn ueberholt B (No-op auf ein Mitglied, das noch nicht da ist), und A
/// wendet danach den Join an - das Mitglied bleibt, obwohl es fort sein muss.
/// Die Ordnung haengt an Ereignissen, nicht an Zeit (Nacharbeit 1, R-E4-1):
/// A haelt am Haken -> B hat den Rahmen betreten (Zaehler vor dem Riegel) ->
/// A wird freigegeben, sobald B am gehaltenen Riegel steht oder - ohne
/// Riegel - bis nach seiner Anwendung durchgelaufen ist.
void dirty_und_revision_genau_einmal_je_bestaetigtem_befehl()
{
    std::cout << "== NAK-246 M-12 dirty_und_revision_genau_einmal_je_bestaetigtem_befehl ==\n";
    const auto a = id ('a');
    // Schranke und Zaehler VOR dem Prozessor: der Haken lebt im Prozessor und
    // kann bis zu dessen Destruktor aus dem Workerzug gerufen werden - was er
    // faengt, muss ihn ueberleben.
    Schranke gate;
    std::atomic<int> hakenRufe { 0 };
    auto p = mainAnlegen (true, true);
    DirtyZaehler dirty;
    p->addListener (&dirty);
    const auto revisionVor = p->v3StateRevisionFuerTest();

    p->setzeSourcesDrainHakenFuerTest ([&gate, &hakenRufe] (std::size_t)
    {
        if (hakenRufe.fetch_add (1) == 0)
            gate.halten();
    });

    // Befehl 1 (eingeschleust, dann ECHTER ACK-Weg): confirm_join A.
    const auto joinId = p->merkeSourcesCommandFuerTest (
        eqcop::EqCopilotProcessor::SourcesCommandArt::confirmJoin, a);
    p->v3AntwortFuerTest (ack (joinId, true));
    pruefe (gate.warteBisErreicht (2000),
            "M-12: Drain A (Workerzug) steht am Haken - zwischen Swap [confirm_join] und Anwendung");
    pruefe (mitglieder (*p).empty() && dirty.nonParam == 0,
            "M-12: waehrend A haelt, ist nichts angewandt und nichts gemeldet");

    // Befehl 2: unbind_probe A, bestaetigt - er liegt jetzt in der Liste, die
    // A NICHT mitgenommen hat.
    const auto unbindId = p->merkeSourcesCommandFuerTest (
        eqcop::EqCopilotProcessor::SourcesCommandArt::unbindProbe, a);
    p->v3AntwortFuerTest (ack (unbindId, true));

    // Nacharbeit 1 (R-E4-1): die Ordnung haengt an EREIGNISSEN, nie an einem
    // Schlafintervall. Waehrend A haelt, betritt kein anderer Drain den
    // Rahmen - der Workerzug IST A, gespeichert wird nicht, einen Editor gibt
    // es nicht -, jeder weitere Eintritt ist also B.
    const auto eintritteVorB = p->sourcesDrainEintritteFuerTest();

    // Drain B: der Editor-Tick aus einem eigenen Faden.
    std::atomic<bool> bFertig { false };
    std::thread tickB ([&]
    {
        p->sourcesTick();
        bFertig.store (true);
    });
    // Ereignis 1: B hat den Rahmen betreten - gezaehlt VOR dem Riegel.
    const bool bEingetreten = warteAuf (5000, [&] {
        return p->sourcesDrainEintritteFuerTest() > eintritteVorB;
    });
    // Ereignis 2: haelt A den Riegel, steht B an ihm - vorbei kommt er erst,
    // wenn A freigibt -, und A wird JETZT freigegeben. Haelt kein Drain einen
    // Riegel (Rotbau ohne `sourcesDrainMutex`), laeuft B bis nach seiner
    // Anwendung durch, BEVOR A freigegeben wird: dieselbe Ordnung, kein Rennen.
    const bool riegelGehalten = bEingetreten && p->sourcesDrainRiegelGehaltenFuerTest();
    const bool bAmRiegel = riegelGehalten && ! bFertig.load();
    const bool bVorAFertig = ! riegelGehalten && warteAuf (5000, [&] { return bFertig.load(); });
    const auto waehrend = mitglieder (*p);
    gate.freigeben();
    tickB.join();
    // Ruhe, ebenfalls als Ereignis: B ist zurueck (join); der NAECHSTE
    // Eintritt des Workerzugs beweist, dass A samt Nachfuehrung (Dirty,
    // Revision) zurueck ist - der Zug betritt den Rahmen erst wieder, nachdem
    // sein voriger Aufruf zurueckgekehrt ist.
    const auto eintritteNachB = p->sourcesDrainEintritteFuerTest();
    const bool ruhe = warteAuf (5000, [&] {
        return p->sourcesDrainEintritteFuerTest() > eintritteNachB;
    });
    pruefe (bEingetreten && ruhe,
            "M-12: Drain B hat den Rahmen betreten, und der Workerzug hat ihn nach der Freigabe "
            "erneut betreten - beide Drains sind samt Nachfuehrung zurueck",
            juce::String ((juce::int64) eintritteVorB) + " -> "
                + juce::String ((juce::int64) eintritteNachB) + " -> "
                + juce::String ((juce::int64) p->sourcesDrainEintritteFuerTest()) + " Eintritte");

    pruefe (mitglieder (*p).empty(),
            "M-12: dirty_und_revision_genau_einmal_je_bestaetigtem_befehl - Join, dann "
            "Unbind desselben Mitglieds in ACK-Reihenfolge endet OHNE Mitglied",
            juce::String ((int) mitglieder (*p).size()) + " Mitglied(er)");
    pruefe (dirty.nonParam == 2,
            "M-12: Dirty-Zaehler == Zahl der State-aendernden Befehle (2)",
            juce::String (dirty.nonParam.load()));
    pruefe (p->v3StateRevisionFuerTest() == revisionVor + 2,
            "M-12: Revision-Delta == 2",
            juce::String ((juce::int64) (p->v3StateRevisionFuerTest() - revisionVor)));
    pruefe (bAmRiegel && waehrend.empty(),
            "M-12: der zweite Drain stand am Riegel, waehrend der erste hielt - "
            "`sourcesDrainMutex` klammert Swap UND Anwendung",
            juce::String ("Riegel gehalten: ") + (riegelGehalten ? "ja" : "nein")
                + ", B vor der Freigabe von A durchgelaufen: " + (bVorAFertig ? "ja" : "nein")
                + ", " + juce::String ((int) waehrend.size()) + " Mitglied(er) waehrend A hielt");
    pruefe (hakenRufe.load() >= 2,
            "M-12: beide Swaps liefen durch den Haken",
            juce::String (hakenRufe.load()));
    // Der Haken bleibt bis zum Destruktor gesetzt (kein Umsetzen, waehrend der
    // Workerzug ihn lesen koennte); Schranke und Zaehler ueberleben ihn.
    p->removeListener (&dirty);
}

/// M-13 · Wache: der Reload-Riegel gilt auch fuer den Speicher-Drain.
/// (1) Produktweg: ein ACK nach dem Reload findet keine Zuordnung mehr;
/// (2) Riegel: ein bestaetigter Befehl mit fremder Bindung oder fremder
///     Epoche in der Liste wird vom Speicher-Drain nie angewandt.
void ack_eines_alten_laufs_mutiert_den_neuen_state_auch_im_speicherdrain_nicht()
{
    std::cout << "== NAK-246 M-13 ack_eines_alten_laufs_mutiert_den_neuen_state_auch_im_speicherdrain_nicht ==\n";
    const auto a = id ('a');
    auto p = mainAnlegen (false, true);
    DirtyZaehler dirty;
    p->addListener (&dirty);
    const auto alteBindung = p->holeZustandKopie().common.projectBindingId.toStdString();

    // Ein anderes Projekt, dessen State geladen wird.
    juce::MemoryBlock fremdState;
    {
        auto fremd = std::make_unique<eqcop::EqCopilotProcessor>();
        fremd->setzeEditorOffen (true);
        pruefe (fremd->setzeBindung ("hub", "Anderes", ""), "M-13: ein zweites Projekt");
        fremd->getStateInformation (fremdState);
    }

    // (1) Join A gesendet, dann Reload, dann der ACK des alten Laufs.
    p->setzeSourcesFixtureFuerTest (lebendeQuelle (a));
    pruefe (p->bindeSourcesHauptziel (a), "M-13: Join A gesendet (alter Lauf)");
    const auto joinId = commandId (p->ausstehenderSourcesCommandFuerTest());
    p->setStateInformation (fremdState.getData(), (int) fremdState.getSize());
    const auto neueBindung = p->holeZustandKopie().common.projectBindingId.toStdString();
    pruefe (! neueBindung.empty() && neueBindung != alteBindung,
            "M-13: das Projekt ist gewechselt (andere project_binding_id)");
    const auto dirtyNachReload = dirty.nonParam.load();
    const auto revisionNachReload = p->v3StateRevisionFuerTest();
    p->v3AntwortFuerTest (ack (joinId, true));
    juce::MemoryBlock state1;
    p->getStateInformation (state1);
    pruefe (mitglieder (*p).empty() && dirty.nonParam == dirtyNachReload
                && p->v3StateRevisionFuerTest() == revisionNachReload,
            "M-13: der ACK des alten Laufs findet nach dem Reload keine Zuordnung - "
            "Speichern ohne Tick aendert nichts");

    // (2) Der Riegel im Speicher-Drain selbst: bestaetigte Befehle mit fremder
    // Bindung bzw. fremder Epoche liegen in der Liste.
    const auto fremdeBindung = p->merkeSourcesCommandFuerTest (
        eqcop::EqCopilotProcessor::SourcesCommandArt::confirmJoin, a, alteBindung);
    p->v3AntwortFuerTest (ack (fremdeBindung, true));
    const auto fremdeEpoche = p->merkeSourcesCommandFuerTest (
        eqcop::EqCopilotProcessor::SourcesCommandArt::confirmJoin, a, {}, id ('9'));
    p->v3AntwortFuerTest (ack (fremdeEpoche, true));
    juce::MemoryBlock state2;
    p->getStateInformation (state2);
    const bool bytesGleich = state1.getSize() == state2.getSize()
        && std::memcmp (state1.getData(), state2.getData(), state1.getSize()) == 0;
    pruefe (mitglieder (*p).empty() && bytesGleich && dirty.nonParam == dirtyNachReload
                && p->v3StateRevisionFuerTest() == revisionNachReload,
            "M-13: ack_eines_alten_laufs_mutiert_den_neuen_state_auch_im_speicherdrain_nicht - "
            "bestaetigte Befehle mit fremder Bindung oder Epoche wendet der Speicher-Drain "
            "nie an (Bytes gleich, kein Mitglied, kein Dirty, keine Revision)");
    p->removeListener (&dirty);

    auto nach = std::make_unique<eqcop::EqCopilotProcessor>();
    nach->setStateInformation (state2.getData(), (int) state2.getSize());
    pruefe (mitglieder (*nach).empty(),
            "M-13: eine neue Instanz laedt kein Mitglied des alten Laufs");

    // Gegenprobe: derselbe Befehl mit der AKTUELLEN Bindung und Epoche wird
    // vom Speicher-Drain angewandt - der Riegel ist selektiv, nicht blind.
    const auto passend = p->merkeSourcesCommandFuerTest (
        eqcop::EqCopilotProcessor::SourcesCommandArt::confirmJoin, a);
    p->v3AntwortFuerTest (ack (passend, true));
    juce::MemoryBlock state3;
    p->getStateInformation (state3);
    pruefe (genau (mitglieder (*p), { a }),
            "M-13 Gegenprobe: mit passender Bindung und Epoche wendet der Speicher-Drain an");
}

/// M-16 · die KETTE (D3 und D4 zusammen): volle Queue -> Quellen-Join ->
/// Reconnect -> ACK -> Speichern ohne Tick -> Laden in neuer Instanz, am
/// echten Prozessor mit Produkt-`controlV3` gegen den Testserver
/// (Testkonstruktor aus Etappe 3, `fuelleP0QueueFuerTest`, Server mit
/// `commandAckArt = angewandt`). Der Server geht vor dem Fuellen und kommt
/// danach wieder: so laeuft die Queue deterministisch nirgends ab, und der
/// Reconnect ist ein echter Neuaufbau mit Replay unter derselben command_id.
void volle_queue_join_reconnect_ack_save_load_als_eine_kette()
{
    std::cout << "== NAK-246 M-16 volle_queue_join_reconnect_ack_save_load_als_eine_kette ==\n";
    const auto a = id ('a');
    const auto pipe = testPipeName ("nak246-m16");
    auto server1 = std::make_unique<TestServer> (pipe);
    server1->commandAckArt.store (1);            // angewandt
    pruefe (server1->starten(), "M-16: der Testserver steht");
    // HEAP (NAK-175); Testkonstruktor mit Probe-Pipe und Servererwartung.
    auto p = std::make_unique<eqcop::EqCopilotProcessor> (pipe, testExeErwartung());
    pruefe (p->v3PipeNameFuerTest() == pipe,
            "M-16: beide v3-Clients zeigen auf die Probe-Pipe (Testkonstruktor)");
    p->setzeWorkerDrainFuerTest (false);         // das Speichern ist der Drain der Kette
    p->prepareToPlay (nak246::kFs, nak246::kBlock);
    p->setzeEditorOffen (true);
    pruefe (p->setzeBindung ("hub", "Gen", ""), "M-16: der Prozessor ist ein Main mit Bindung");
    DirtyZaehler dirty;
    p->addListener (&dirty);
    p->v3StartFuerTest();
    pruefe (warteAuf (8000, [&] {
                return p->controlV3Snapshot().status == nakama::ipc::ControlClient::Status::verbunden;
            }),
            "M-16: der echte Client ist ueber die Probe-Pipe verbunden");
    // Schranke 1: der Aufbau-Callback hat das Modell uebernommen. `verbunden`
    // steht im Client VOR `meldeLinkStatus (true)` (`Verbindung.cpp`); erst der
    // Callback ruft `beginneSubscription` und setzt `authenticating` (dieser
    // Testserver sendet keinen `session_snapshot`, der es aendern koennte).
    // Die Schranke macht die Voraussetzung von Schranke 2 gemessen statt
    // angenommen.
    pruefe (warteAuf (8000, [&] {
                return p->sourcesSicht().diagnose == eqcop::SourcesModel::Diagnose::authenticating;
            }),
            "M-16: der Link-Aufbau-Callback ist durch - das Modell meldet authenticating");

    // Der Server geht: ab jetzt laeuft die Queue nirgends ab.
    server1->stoppen();
    server1.reset();
    pruefe (warteAuf (8000, [&] {
                return p->controlV3Snapshot().status != nakama::ipc::ControlClient::Status::verbunden;
            }),
            "M-16: die Verbindung ist weg");
    // Schranke 2: die Quellensicht kommt erst, wenn der Ende-Callback DURCH
    // ist. Der Client setzt `getrennt` VOR `meldeLinkStatus (false)`; der
    // Callback nimmt dem Modell ueber `controlEnde` die Subscription - eine
    // frueher gesetzte Fixture verloere `mainDarfSchreiben`, und
    // `sendeSourcesCommand` kehrte vor dem Senden um. Nach Schranke 1 setzt in
    // diesem Fall nur noch `controlEnde` `brokerUnavailable` (kein Editor-Tick,
    // kein Reload); ein zweiter Ende-Callback kommt vor dem naechsten Aufbau
    // nicht (`exchange` in `meldeLinkStatus`). Kein Schlaf als
    // Reihenfolgegarantie.
    pruefe (warteAuf (8000, [&] {
                return p->sourcesSicht().diagnose == eqcop::SourcesModel::Diagnose::brokerUnavailable;
            }),
            "M-16: der Link-Ende-Callback ist durch - das Modell meldet brokerUnavailable");
    p->setzeSourcesFixtureFuerTest (lebendeQuelle (a));
    {
        const auto s = p->sourcesSicht();
        pruefe (s.mainDarfSchreiben && s.quellen.size() == 1 && s.quellen.front().hauptziel,
                "M-16 Aufbau: die Quelle A ist Hauptziel, dieses Main darf schreiben");
    }
    const auto gefuellt = p->fuelleP0QueueFuerTest();
    pruefe (gefuellt > 0, "M-16: die P0-Queue ist voll", juce::String ((int) gefuellt));
    const auto vorJoin = p->controlV3Snapshot();

    // Der Quellen-Join bei voller Queue.
    const bool angenommen = p->bindeSourcesHauptziel (a);
    const auto zuordnung = p->ausstehenderSourcesCommandFuerTest();
    pruefe (angenommen && ! zuordnung.empty()
                && zuordnung.find ("\"command\":\"confirm_join\"") != std::string::npos,
            "M-16: volle_queue_join_reconnect_ack_save_load_als_eine_kette - der Join ist bei "
            "voller Queue ZUR WIEDERHOLUNG angenommen, und der Aufrufer BEHAELT die "
            "command_id-Zuordnung (Basis-SHA: bool false, Zuordnung geloescht)",
            juce::String (angenommen ? "angenommen" : "abgewiesen") + ", Zuordnung "
                + (zuordnung.empty() ? "leer" : "vorhanden"));
    // Die Kennung, die der Aufrufer zum Join haelt (leer, wenn er die
    // Zuordnung geloescht hat).
    const auto joinId = commandId (zuordnung);
    const auto nachJoin = p->controlV3Snapshot();
    pruefe (nachJoin.inFlight >= 1 && nachJoin.p0Ueberlaeufe > vorJoin.p0Ueberlaeufe,
            "M-16: der Join steht im Register, der Ueberlauf ist gezaehlt (M-73)",
            "inFlight " + juce::String ((juce::int64) nachJoin.inFlight));

    // Die Testfuellung geht; der Server kommt wieder; der Client verbindet neu
    // und spielt den Join unter derselben command_id nach; der Server wendet an.
    p->leereP0QueueFuerTest();
    auto server2 = std::make_unique<TestServer> (pipe);
    server2->commandAckArt.store (1);
    pruefe (server2->starten(), "M-16: der Testserver steht wieder (Reconnect)");
    const bool bestaetigt = warteAuf (20000, [&] {
        const auto s = p->controlV3Snapshot();
        return s.inFlight == 0 && s.inFlightErfolg >= 1;
    });
    const auto nachAck = p->controlV3Snapshot();
    pruefe (bestaetigt,
            "M-16: nach dem Reconnect ist der Join nachgespielt und vom Server angewandt (ACK)",
            "inFlightErfolg " + juce::String ((juce::int64) nachAck.inFlightErfolg)
                + ", Wiederholungen " + juce::String ((juce::int64) nachAck.inFlightWiederholungen));
    // Was der zweite Server empfangen hat, UNABHAENGIG von der Zuordnung des
    // Aufrufers: die command_id jedes confirm_join. Das Register spielt nach,
    // auch wenn der Aufrufer die Zuordnung verloren hat (M-73).
    std::vector<std::string> replayKennungen;
    {
        std::lock_guard<std::mutex> l (server2->textMutex);
        for (const auto& t : server2->p0Texte)
            if (t.find ("\"command\":\"confirm_join\"") != std::string::npos)
                replayKennungen.push_back (commandId (t));
    }
    pruefe (! replayKennungen.empty(),
            "M-16: der zweite Server hat den Join empfangen (Replay nach dem Reconnect, M-73)",
            juce::String ((int) replayKennungen.size()) + " confirm_join"
                + (replayKennungen.empty() ? juce::String()
                                           : ", " + juce::String (replayKennungen.front())));
    // Die Zuordnung gefunden heisst: jeder Replay traegt DIE Kennung, die der
    // Aufrufer haelt, und der ACK hat den Befehl aus den ausstehenden in die
    // bestaetigten gelegt (`v3Antwort`). Am Basis-SHA war die Zuordnung vor
    // dem ACK geloescht - "nicht mehr ausstehend" gilt dann auch, bestaetigt
    // ist aber nichts.
    const bool vorgemerkt = warteAuf (2000, [&] {
        return p->ausstehenderSourcesCommandFuerTest().empty()
            && p->bestaetigteSourcesCommandsFuerTest() == 1;
    });
    const bool dieselbeKennung = ! joinId.empty() && ! replayKennungen.empty()
        && std::all_of (replayKennungen.begin(), replayKennungen.end(),
                        [&] (const std::string& k) { return k == joinId; });
    pruefe (vorgemerkt && dieselbeKennung,
            "M-16: der ACK hat die Zuordnung gefunden - der Replay traegt DIESELBE command_id, die der "
            "Aufrufer haelt, und der Befehl ist bestaetigt vorgemerkt",
            juce::String ((int) p->bestaetigteSourcesCommandsFuerTest()) + " bestaetigt, Aufrufer "
                + (joinId.empty() ? juce::String ("ohne Kennung") : juce::String (joinId)));
    pruefe (mitglieder (*p).empty() && dirty.nonParam == 0,
            "M-16: kein Tick lief - nichts ist angewandt, kein Dirty");

    // Speichern OHNE Tick: der Persistenzabschluss (M-10) fuehrt die
    // Mitgliedschaft nach.
    juce::MemoryBlock state;
    p->getStateInformation (state);
    pruefe (genau (mitglieder (*p), { a }) && dirty.nonParam == 1,
            "M-16: das Speichern ohne Tick wendet den bestaetigten Join an, Dirty einmal",
            juce::String ((int) mitglieder (*p).size()) + " Mitglied(er), Dirty "
                + juce::String (dirty.nonParam.load()));
    p->removeListener (&dirty);
    p.reset();                                   // stoppt die Clients vor dem Server
    server2->stoppen();

    auto nach = std::make_unique<eqcop::EqCopilotProcessor>();
    nach->setStateInformation (state.getData(), (int) state.getSize());
    const auto geladen = mitglieder (*nach);
    pruefe (genau (geladen, { a }) && geladen.front().label == "Reported fallback",
            "M-16: die neue Instanz laedt das Mitglied - die Kette ist geschlossen",
            juce::String ((int) geladen.size()) + " Mitglied(er)");
}

bool nak246d3Fall (const std::string& name)
{
    if (name == "m10") { bestaetigte_join_und_unbind_landen_ohne_tick_im_gespeicherten_state(); return true; }
    if (name == "m11") { bestaetigtes_ack_wird_ohne_editor_vom_prozessortakt_angewandt(); return true; }
    if (name == "m12") { dirty_und_revision_genau_einmal_je_bestaetigtem_befehl(); return true; }
    if (name == "m13") { ack_eines_alten_laufs_mutiert_den_neuen_state_auch_im_speicherdrain_nicht(); return true; }
    if (name == "m14") { gefaelschtes_command_ack_vor_serverauth_mutiert_keinen_persistenten_projektzustand(); return true; }
    if (name == "m16") { volle_queue_join_reconnect_ack_save_load_als_eine_kette(); return true; }
    return false;
}
} // namespace nak246d3
} // namespace

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;
    if (argc == 3 && std::string (argv[1]) == "--nur")
    {
        if (! nak246::nak246Fall (argv[2]) && ! nak246d3::nak246d3Fall (argv[2]))
        {
            std::cout << "unbekannter Fall: " << argv[2] << " (m06 | m07 | m09 | m10 | m11 | m12 | m13 | m14 | m16)\n";
            return 2;
        }
        std::cout << "SONDE-012 ProjectReload (nur " << argv[2] << "): " << bestanden << "/"
                  << (bestanden + fehler) << " gruen\n";
        return fehler == 0 ? 0 : 1;
    }
    gefaelschtes_command_ack_vor_serverauth_mutiert_keinen_persistenten_projektzustand();
    // NAK-246 D2: die Lebensdauerfaelle am echten Prozessor (M-06, M-07, M-09).
    nak246::prozessorabbau_ueber_die_frist_wartet_den_produkt_callback_zu_ende();
    nak246::abgeloester_produkt_callback_beruehrt_den_zerstoerten_prozessor_nicht();
    nak246::alle_produkt_callbacks_laufen_durch_die_schleuse();
    // NAK-246 D3: der editorunabhaengige Persistenzabschluss (M-10 bis M-13).
    nak246d3::bestaetigte_join_und_unbind_landen_ohne_tick_im_gespeicherten_state();
    nak246d3::bestaetigtes_ack_wird_ohne_editor_vom_prozessortakt_angewandt();
    nak246d3::dirty_und_revision_genau_einmal_je_bestaetigtem_befehl();
    nak246d3::ack_eines_alten_laufs_mutiert_den_neuen_state_auch_im_speicherdrain_nicht();
    // NAK-246 D3 + D4: die Kette M-16 am echten Prozessor gegen den Testserver.
    nak246d3::volle_queue_join_reconnect_ack_save_load_als_eine_kette();
    const auto quelle = id ('a');

    eqcop::EqCopilotProcessor vor;
    vor.setzeEditorOffen (true);
    pruefe (vor.setzeBindung ("hub", "Gen", ""),
            "main_project_is_explicitly_initialised");
    auto live = lebendeQuelle (quelle);
    auto nurFluechtig = live.quellen.front();
    nurFluechtig.instanceId = id ('c');
    nurFluechtig.runtimeNonce = id ('d');
    nurFluechtig.hauptziel = false;
    live.quellen.push_back (nurFluechtig);
    vor.setzeSourcesFixtureFuerTest (std::move (live));

    DirtyZaehler dirtyVor;
    vor.addListener (&dirtyVor);
    const bool fremdEingereiht = vor.bindeSourcesHauptziel (quelle);
    const auto fremdCommand = vor.ausstehenderSourcesCommandFuerTest();
    vor.v3AntwortFuerTest (ack (commandId (fremdCommand), false));
    vor.sourcesTick();
    const bool fremdAbgelehnt = vor.holeZustandKopie().mainProjectMitglieder.empty()
                             && dirtyVor.nonParam == 0;
    const bool gebundenEingereiht = vor.bindeSourcesHauptziel (quelle);
    const auto bindCommand = vor.ausstehenderSourcesCommandFuerTest();
    const auto bindId = commandId (bindCommand);
    const bool vorAckNichtPersistiert = vor.holeZustandKopie().mainProjectMitglieder.empty()
                                     && dirtyVor.nonParam == 0;
    vor.v3AntwortFuerTest (ack (bindId, true));
    vor.sourcesTick();
    const auto nachBind = vor.holeZustandKopie();
    const bool berichtetesLabelPersistiert = nachBind.mainProjectMitglieder.size() == 1
        && nachBind.mainProjectMitglieder.front().label == "Reported fallback";
    const bool benannt = vor.benenneSourcesHauptziel (quelle, "Stored Piano");
    const bool noOp = ! vor.benenneSourcesHauptziel (quelle, "Stored Piano");
    // NAK-246 Etappe 4 Nacharbeit 1 (R-E4-2): seit D3 drainiert hier auch der
    // Analyse-Workerzug. Wendet ER den Join an, gibt er den Riegel frei, BEVOR
    // er Dirty meldet (`wendeBestaetigteSourcesCommandsAn`): der Tick oben
    // findet dann eine leere Liste - das Mitglied steht schon im State, der
    // Riegel garantiert es -, die Dirty-Meldung kann aber noch ausstehen.
    // Gemessen wird deshalb nach dem VOLLSTAENDIGEN Nachlauf; die Zusage
    // bleibt: Join und Name melden je genau einmal Dirty.
    (void) warteAuf (2000, [&] {
        return dirtyVor.nonParam.load() == 2
            && vor.holeZustandKopie().mainProjectMitglieder.size() == 1;
    });
    pruefe (fremdEingereiht && fremdAbgelehnt && gebundenEingereiht
            && bindCommand.find ("\"command\":\"confirm_join\"")
                != std::string::npos && ! bindId.empty() && vorAckNichtPersistiert
            && berichtetesLabelPersistiert && benannt && noOp && dirtyVor.nonParam == 2,
            "confirmed_join_ack_and_name_each_mark_host_dirty",
            juce::String (dirtyVor.nonParam.load()));

    juce::MemoryBlock state;
    vor.getStateInformation (state);
    pruefe (dirtyVor.nonParam == 2 && state.getSize() > 0,
            "save_is_not_a_persistent_mutation");
    vor.setStateInformation (state.getData(), (int) state.getSize());
    const auto reloadGleichesObjekt = vor.sourcesSicht();
    const auto* reloadQuelle = reloadGleichesObjekt.quellen.size() == 1
                                 ? &reloadGleichesObjekt.quellen.front() : nullptr;
    pruefe (reloadQuelle != nullptr && reloadQuelle->instanceId == quelle
            && reloadQuelle->control == eqcop::SourcesModel::Control::getrennt
            && reloadQuelle->messung == eqcop::SourcesModel::Messung::missing
            && ! reloadQuelle->lufsPaarVorhanden && reloadQuelle->runtimeNonce.empty()
            && dirtyVor.nonParam == 2,
            "same_instance_reload_discards_transient_snapshot_and_measurement");
    vor.removeListener (&dirtyVor);

    eqcop::EqCopilotProcessor nach;
    DirtyZaehler dirtyNach;
    nach.addListener (&dirtyNach);
    nach.setStateInformation (state.getData(), (int) state.getSize());
    const auto z = nach.holeZustandKopie();
    const auto sicht = nach.sourcesSicht();
    const auto* q = sicht.quellen.size() == 1 ? &sicht.quellen.front() : nullptr;
    const bool stateWahr = z.common.klasse == nakama::state::Klasse::main
        && z.common.projectBindingId.isNotEmpty()
        && z.mainProjectMitglieder.size() == 1
        && z.mainProjectMitglieder.front().instanceId == juce::String (quelle)
        && z.mainProjectMitglieder.front().label == "Stored Piano";
    const bool keineLiveWahrheit = q != nullptr
        && q->mitgliedschaft == eqcop::SourcesModel::Mitgliedschaft::bestaetigt
        && q->sichtbarerName == "Stored Piano"
        && q->control == eqcop::SourcesModel::Control::getrennt
        && q->messung != eqcop::SourcesModel::Messung::fresh
        && ! q->lufsPaarVorhanden
        && q->runtimeNonce.empty();
    pruefe (stateWahr && keineLiveWahrheit && dirtyNach.nonParam == 0,
            "reload_preserves_identity_label_membership_but_not_live_truth",
            juce::String (dirtyNach.nonParam.load()));

    const auto subscribe = nach.v3SubscribeFuerTest();
    pruefe (subscribe.find ("\"type\":\"subscribe_session\"") != std::string::npos
            && subscribe.find (z.common.projectBindingId.toStdString()) != std::string::npos,
            "project_reload_reconnect_path_builds_a_fresh_subscribe");

    const auto hello = nach.v3HelloFuerTest();
    nach.v3LinkFuerTest (true); // derselbe Callback wie direkt nach welcome
    const bool wartetSnapshot = ! nach.sourcesSicht().subscriptionAktiv;
    nach.v3AntwortFuerTest (leererSnapshot (hello));
    const bool ersterAktiv = nach.sourcesSicht().subscriptionAktiv;
    nach.v3LinkFuerTest (false);
    const bool endeInvalidiert = ! nach.sourcesSicht().subscriptionAktiv;
    nach.v3LinkFuerTest (true);
    const bool reconnectWartet = ! nach.sourcesSicht().subscriptionAktiv;
    nach.v3AntwortFuerTest (leererSnapshot (hello));
    pruefe (wartetSnapshot && ersterAktiv && endeInvalidiert
            && reconnectWartet && nach.sourcesSicht().subscriptionAktiv,
            "main_client_subscribes_after_welcome_and_reconnect_resubscribes");

    auto referenz = nach.sourcesSicht();
    referenz.mainDarfSchreiben = false;
    referenz.fuehrendesMain = id ('e');
    nach.setzeSourcesFixtureFuerTest (std::move (referenz));
    {
        eqcop::EqCopilotEditor editor (nach);
        editor.setSize (760, 430);
        juce::Timer::callPendingTimersSynchronously();
        const bool labelDeaktiviert = ! editor.sourcesLabelAktivFuerTest()
            && editor.sourcesBedienstatusFuerTest().containsIgnoreCase ("leading Main");
        editor.sourcesLabelSchreibversuchFuerTest ("Must not write");
        const bool fehlschlagSichtbar = editor.sourcesBedienstatusFuerTest()
            .containsIgnoreCase ("failed");
        pruefe (labelDeaktiviert && fehlschlagSichtbar
                && nach.holeZustandKopie().mainProjectMitglieder.front().label == "Stored Piano"
                && dirtyNach.nonParam == 0,
                "nonleading_main_disables_label_and_reports_failed_write");
    }

    nach.v3LinkFuerTest (true);
    nach.v3AntwortFuerTest (leererSnapshot (hello));
    const auto nurPersistent = nach.sourcesSicht();
    const bool istNurPersistent = nurPersistent.mainDarfSchreiben
        && nurPersistent.quellen.size() == 1
        && nurPersistent.quellen.front().mitgliedschaft
            == eqcop::SourcesModel::Mitgliedschaft::bestaetigt
        && nurPersistent.quellen.front().runtimeNonce.empty();
    const bool entferntLokal = nach.entferneSourcesHauptziel (quelle);
    const auto nachUnbind = nach.sourcesSicht();
    const bool keineWireBehauptung = nach.ausstehenderSourcesCommandFuerTest().empty();
    const bool entferntSichtbar = nach.holeZustandKopie().mainProjectMitglieder.empty()
        && nachUnbind.quellen.empty();
    const bool zweitesNoOp = ! nach.entferneSourcesHauptziel (quelle);
    pruefe (istNurPersistent && entferntLokal && keineWireBehauptung
            && entferntSichtbar && zweitesNoOp && dirtyNach.nonParam == 1,
            "persistent_only_member_remove_needs_no_runtime_nonce_and_marks_host_dirty_once",
            juce::String (dirtyNach.nonParam.load()));
    nach.removeListener (&dirtyNach);

    /*  ── SONDE-014 M-13 und M-09: der Intent im selben Recall-Pfad ─────────

        Dieselbe Zusage wie fuer die Mitgliedschaft, eine Ebene weiter: jede
        persistente Intent-Aenderung meldet GENAU einmal Host-Dirty, ein
        No-op und ein abgewiesener Wert melden nichts, Speichern und Laden
        melden nichts - und der Bestand ueberlebt den Recall in eine FRISCHE
        Instanz.

        Warum hier und nicht nur im eigenen Bein B27: M-13 nennt ausdruecklich
        `DirtyZaehler.nonParam` dieses Beins, und der Recall-Pfad ist der Ort,
        an dem der Zaehler seit SONDE-012 gemessen wird. */
    {
        eqcop::EqCopilotProcessor intentMain;
        DirtyZaehler dirtyIntent;
        intentMain.addListener (&dirtyIntent);
        const bool alsMain = intentMain.setzeBindung ("hub", "Gen", "");
        const auto dirtyNachBindung = dirtyIntent.nonParam.load();

        const auto qa = juce::String (id ('a'));
        const auto qb = juce::String (id ('b'));
        const auto pas = juce::String (id ('f'));

        const bool gesetzt = intentMain.setzeQuellenrolle (
            qa, {}, nakama::state::Rolle::fuehrt, nakama::state::IntentHerkunft::user, 1.0);
        const bool meldetEinmal = dirtyIntent.nonParam == dirtyNachBindung + 1;
        const bool noOpIntent = intentMain.setzeQuellenrolle (
            qa, {}, nakama::state::Rolle::fuehrt, nakama::state::IntentHerkunft::user, 1.0);
        const bool noOpSchweigt = dirtyIntent.nonParam == dirtyNachBindung + 1;
        const bool abgewiesen = ! intentMain.setzeQuellenrolle (
            "kurz", {}, nakama::state::Rolle::fuehrt, nakama::state::IntentHerkunft::user, 1.0);
        const bool abweisungSchweigt = dirtyIntent.nonParam == dirtyNachBindung + 1;
        const bool zweiterScope = intentMain.setzeQuellenrolle (
            qa, pas, nakama::state::Rolle::begleitet, nakama::state::IntentHerkunft::user, 1.0);
        const bool schutz = intentMain.schuetzeQuelle (
            qa, nakama::state::Schutzeigenschaft::attack, -1, -1);
        const bool kante = intentMain.setzeQuellenbeziehung (
            qa, qb, nakama::state::Beziehungsart::fuehrtVor);
        const bool vierWeitereMeldungen = dirtyIntent.nonParam == dirtyNachBindung + 4;
        pruefe (alsMain && gesetzt && meldetEinmal && noOpIntent && noOpSchweigt
                && abgewiesen && abweisungSchweigt && zweiterScope && schutz && kante
                && vierWeitereMeldungen,
                "intent_change_marks_host_dirty_once_and_noop_or_rejected_stays_silent",
                juce::String (dirtyIntent.nonParam.load()));

        juce::MemoryBlock intentState;
        intentMain.getStateInformation (intentState);
        const bool saveSchweigt = dirtyIntent.nonParam == dirtyNachBindung + 4;
        intentMain.removeListener (&dirtyIntent);

        eqcop::EqCopilotProcessor intentRecall;
        DirtyZaehler dirtyRecall;
        intentRecall.addListener (&dirtyRecall);
        intentRecall.setStateInformation (intentState.getData(), (int) intentState.getSize());
        const auto intents = intentRecall.sourceIntents();
        nakama::state::Rolle wirkt {};
        const bool passageGewinnt = intentRecall.wirkendeQuellenrolle (qa, pas, wirkt)
                                 && wirkt == nakama::state::Rolle::begleitet;
        const bool globalBleibt = intentRecall.wirkendeQuellenrolle (qa, {}, wirkt)
                               && wirkt == nakama::state::Rolle::fuehrt;
        juce::MemoryBlock nochmal;
        intentRecall.getStateInformation (nochmal);
        const bool bytegleich = nochmal.getSize() == intentState.getSize()
            && std::memcmp (nochmal.getData(), intentState.getData(), nochmal.getSize()) == 0;
        pruefe (saveSchweigt && intents.size() == 2
                && intentRecall.intentSchutzangaben().size() == 1
                && intentRecall.intentBeziehungen().size() == 1
                && intentRecall.intentBestandRevision() == 4
                && passageGewinnt && globalBleibt && bytegleich
                && dirtyRecall.nonParam == 0,
                "intent_survives_project_recall_and_load_marks_no_dirty",
                juce::String (dirtyRecall.nonParam.load()));
        intentRecall.removeListener (&dirtyRecall);
    }

    std::cout << "SONDE-012 ProjectReload: " << bestanden << "/"
              << (bestanden + fehler) << " gruen\n";
    return fehler == 0 ? 0 : 1;
}
