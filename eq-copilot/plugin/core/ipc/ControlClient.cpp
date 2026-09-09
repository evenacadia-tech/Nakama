// ControlClient — Wurzel: Aufbau, Abbau und die Haken.
//
// Diese Datei behaelt Namen und Pfad, damit die Historie des Clients auf ihr
// bleibt (NAK-225/S25d, 09.09.2026). Sie traegt:
//
//   Konstruktor, Destruktor   Der Client selbst.
//   start, stop, reconnect    Der Lebenszyklus des Clientthreads. stop() meldet
//                             zuerst ab und faehrt danach Fristen und Join -
//                             nichts bleibt registriert, waehrend auf einen
//                             Verbraucher gewartet wird.
//   snapshot, statusProviderGesetzt
//                             Die schmale Sicht nach aussen.
//   setzeKonfliktWiederholungHook, setzeAuftragAbgeschlossenHook,
//   setzeP0Rueckmeldung, setzeReplayBeginHook, setzeAufbauZugHakenFuerTest,
//   setzeProbeGegenstelleFuerTest
//                             Die Haken, die ein Verbraucher oder ein Bein
//                             setzt, bevor start() laeuft.
//
// Die uebrigen Fachbereiche liegen unter core/ipc/controlclient/:
//
//   Vertrag.cpp      Wire-Form, Vertragspruefung, Fehlercodes.
//   Nachrichten.cpp  senden, zustellen, quittieren.
//   Verbindung.cpp   aufbauen, halten, abbauen, Generationen, Heartbeat.
//   Laufzeit.h       die geteilte Laufzeit (Pimpl-Rumpf).
//   Intern.h         die Helfer, die alle Teile brauchen.
//
// Probe-Pipe ist nie Produktions-Pipe: welchen Pipenamen eine Instanz waehlt,
// bekommt der Client uebergeben; er rechnet ihn nie selbst aus.

// K1 (S8/SONDE-007a): der Kompilier-Riegel steht am Anfang UND am Ende jeder
// Kern-Uebersetzungseinheit. Diese hier sieht zwar gar kein JUCE, aber die
// Regel gilt fuer die MENGE, nicht fuer den Einzelfall - eine Ausnahme waere
// die Stelle, an der die naechste Quelle sie stillschweigend erbt.
#include "NakamaKernRiegel.h"

#include "ControlClient.h"
#include "NakamaKanon.h"
#include "WireEnvelope.h"
#include "controlclient/Intern.h"
#include "controlclient/Laufzeit.h"
#include <chrono>

namespace nakama::ipc
{

// Die Helfer des internen Kopfes; die Aufrufstellen bleiben damit
// woertlich wie im Urstand.
using namespace controlclient_intern;

ControlClient::ControlClient (std::function<ControlHello()> helloProviderIn,
                              std::string pipeNameIn,
                              std::function<void (const std::string&)> beiAntwortIn,
                              std::function<ControlStatus()> statusProviderIn,
                               std::function<void (bool)> beiLinkStatusIn,
                               std::function<void (const std::string&, std::uint8_t)>
                                   beiVersionierterAntwortIn,
                               ServerErwartung serverErwartungIn)
    : k (std::make_shared<Laufzeit> (std::move (helloProviderIn),
                                     std::move (pipeNameIn),
                                     std::move (beiAntwortIn),
                                     std::move (statusProviderIn),
                                     std::move (beiLinkStatusIn),
                                     std::move (beiVersionierterAntwortIn),
                                     std::move (serverErwartungIn)))
{
}

ControlClient::~ControlClient()
{
    stop();
}

void ControlClient::start()
{
    std::lock_guard<std::mutex> l (lebenslaufMutex);
    if (k->laeuft.load())
        return;
    k->laeuft.store (true);
    k->fertig.store (false);
    auto kern = k;
    const auto meinLauf = kern->lebenslauf.fetch_add (1) + 1;
    // Eigene Verbindung je Lauf (`B-CC-12`): der Thread bekommt sie direkt in
    // die Hand, statt sie spaeter aus der Laufzeit zu holen.
    auto meine = kern->neueVerbindung();
    thread = std::thread ([kern, meinLauf, meine] {
        kern->threadLauf (meinLauf, std::move (meine));
    });
}

void ControlClient::stop()
{
    // `B-CC-11`: aus einem Callback dieses Clients heraus wird nur markiert.
    // Der Thread endet nach Rueckkehr des Callbacks von selbst; sich hier
    // selbst zu joinen waere `std::system_error` und danach `std::terminate`.
    const bool ausDemClientthread = (std::this_thread::get_id() == k->threadId.load());

    // Genau die Verbindung, die beim Aufruf die aktuelle war. Ein spaeterer
    // `start()` legt eine neue an; diese hier zu schliessen darf jene nicht
    // treffen.
    auto verbindung = k->aktuelleVerbindung();

    k->laeuft.store (false);
    k->meldeLinkStatus (false);
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

    // `B-CC-12`: auf einen laufenden Callback wird hoechstens `kStopFristMs`
    // gewartet. Danach wird der Thread ABGELOEST — er haelt die Laufzeit ueber
    // seinen eigenen `shared_ptr` am Leben und beruehrt den Client nie.
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
    k->zustand.serverPruefstatus = ServerPruefStatus::nichtGeprueft;
    k->zustand.serverPrueffehler = ServerPruefFehler::keiner;
    k->zustand.serverPid = 0;
}

void ControlClient::reconnect()
{
    {
        std::lock_guard<std::mutex> l (k->zustandMutex);
        // Generation und sichtbarer Auth-Zustand bilden eine atomare
        // Zustandsgrenze. So kann ein alter Prueflauf sein Urteil nicht nach
        // diesem Reset in die neue Verbindungsgeneration schreiben.
        k->verbindungsGeneration.fetch_add (1);
        k->zustand.brokerPipeFehlt = false;
        k->zustand.serverPruefstatus = ServerPruefStatus::nichtGeprueft;
        k->zustand.serverPrueffehler = ServerPruefFehler::keiner;
        k->zustand.serverPid = 0;
    }
    k->aktuelleVerbindung()->ioAbbrechen();
    k->warte.notify_all();
    k->meldeLinkStatus (false);
}

ControlClient::Snapshot ControlClient::snapshot() const { return k->snapshotIntern(); }

bool ControlClient::statusProviderGesetzt() const noexcept
{
    return static_cast<bool> (k->statusProvider);
}

void ControlClient::setzeKonfliktWiederholungHook (
    std::function<std::string (const std::string&, const std::string&, std::uint64_t)> hook)
{
    // Wie die uebrigen Rueckwege: vor dem `start()` gesetzt und danach
    // unveraendert.
    k->hookKonfliktWiederholung = std::move (hook);
}

void ControlClient::setzeAuftragAbgeschlossenHook (
    std::function<void (const std::string&)> hook)
{
    // Wie die uebrigen Rueckwege: vor dem `start()` gesetzt und danach
    // unveraendert.
    k->hookAuftragAbgeschlossen = std::move (hook);
}

void ControlClient::setzeP0Rueckmeldung (
    std::function<void (std::uint64_t, std::uint64_t)> zugestellt,
    std::function<void (std::uint64_t)> verworfen)
{
    // Vor dem `start()` gesetzt und danach unveraendert - wie der
    // `statusProvider`. Ein Wechsel unter laufender Verbindung waere ein
    // Datenrennen auf einem `std::function`, das die Sendeschleife gerade
    // ruft.
    k->beiP0Zugestellt = std::move (zugestellt);
    k->beiP0Verworfen  = std::move (verworfen);
}

void ControlClient::setzeReplayBeginHook (
    std::function<std::string (std::uint64_t, std::uint64_t)> hook)
{
    k->hookReplayBegin = std::move (hook);
}

void ControlClient::setzeAufbauZugHakenFuerTest (std::function<void (int)> haken)
{
    k->aufbauZugHakenFuerTest = std::move (haken);
}

bool ControlClient::setzeProbeGegenstelleFuerTest (const std::string& pipename,
                                                   ServerErwartung erwartung)
{
    // Fail-closed: nur solange der Client STEHT, und nie mit leerem Namen.
    //
    // Den NAMENSRAUM prueft der Aufrufer - `PipeToken.h` liegt bewusst
    // ausserhalb von NakamaKern (CMakeLists §Kernquellen: sein
    // Hersteller-Namensraum ist Zielidentitaet, keine geteilte Kernwahrheit).
    // Der einzige erreichbare Aufrufer ist `EqCopilotProcessor::
    // v3ProbeGegenstelleFuerTest`, und der laesst ausschliesslich
    // `istProbePipename` durch.
    if (pipename.empty() || k->laeuft.load())
        return false;
    k->pipeName = pipename;
    k->serverErwartung = std::move (erwartung);
    return true;
}

/** 🔑 NAK-180 R11: die Generation, FUER DIE der laufende Link-Callback laeuft.

    `meldeAufbauUrteil` las bis hier `wireGeneration.load()` - die AKTUELLE
    Zahl. Ein ueberholter positiver Callback (sein Link ist tot, ein neuer
    steht schon) haette damit die NEUE Generation gestempelt, und seine
    Aussage waere fuer den naechsten Link faelschlich gueltig gewesen: genau
    MP3-1, nur eine Ebene tiefer. Die Generationsbindung traegt nur, wenn die
    Wirkung die Zahl ihres EIGENEN Aufbaus bekommt.
    
    `thread_local`, weil der positive Callback synchron auf dem Client-Thread
    laeuft und zwei Clients in einem Prozess sonst dieselbe Zahl teilten. Der
    Worker ruft `meldeAufbauUrteil` beim Abschluss (E3.3) ohne dieses Fenster
    - dort ist die aktuelle Generation die richtige. */

} // namespace nakama::ipc

// K1 erneut: die zweite Auswertung sieht Makros, die erst ein spaeter
// eingebundener Header definiert haben koennte.
#include "NakamaKernRiegel.h"
