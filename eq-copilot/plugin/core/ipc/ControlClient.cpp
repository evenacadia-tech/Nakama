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
#include <algorithm>
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

thread_local std::uint64_t tlAufbauGeneration = 0;

std::uint64_t ControlClient::meldeAufbauUrteil (bool neutral,
                                               std::uint64_t fuerGeneration)
{
    // 🔑 NAK-180 Nacharbeit 1 (EP-03/R12): ein Urteil, das AUSDRUECKLICH einer
    // Generation gilt, wird nur angewendet, solange sie die laufende ist.
    //
    // Der Worker stellt die Neutralitaet unter `sendeMutex` fuer Generation G
    // fest und ruft danach - ausserhalb der Sperre, weil im Zug nicht gesendet
    // wird. Baut G+1 in diesem Fenster auf, traefe das alte `false` dessen
    // frisches `true` und ersetzte es: der Bericht des neuen Links waere nie
    // eroeffnet, waehrend sein Marker klingt. Vergleich und Wirkung bleiben
    // damit eine Einheit, auch ueber die Sperrengrenze hinweg.
    if (fuerGeneration != 0 && fuerGeneration != k->wireGeneration.load())
        return 0;
    const auto generation = fuerGeneration != 0
                              ? fuerGeneration
                              : (tlAufbauGeneration != 0 ? tlAufbauGeneration
                                                         : k->wireGeneration.load());
    // 🔑 NAK-180 R13: das jeweils ANDERE Flag faellt per CAS, nie blind.
    //
    // Ein blindes `store(0)` nahm die Aussage eines NEUEREN Links mit: der
    // ueberholte Callback von G1 loeschte das `true`, das G2 gerade gesetzt
    // hatte, und der erste Heartbeat von G2 trug gar keine Aussage - die
    // Sitzung blieb gesperrt, ohne dass sie je jemand aufloesen konnte. Nur
    // die eigene Generation darf fallen.
    auto loescheEigene = [generation] (std::atomic<std::uint64_t>& flag)
    {
        auto meine = generation;
        flag.compare_exchange_strong (meine, 0);
    };
    // Und die eigene Aussage wird nur gesetzt, solange kein NEUERER Link den
    // Platz belegt hat: `0` (frei) oder eine kleinere/gleiche Generation
    // duerfen weichen, eine hoehere nie.
    auto setzeEigene = [generation] (std::atomic<std::uint64_t>& flag)
    {
        auto gesehen = flag.load();
        while (gesehen <= generation)
            if (flag.compare_exchange_weak (gesehen, generation))
                return;
    };
    if (neutral)
    {
        loescheEigene (k->nichtNeutralerNeuaufbau);
        setzeEigene (k->neutralerNeuaufbau);
    }
    else
    {
        loescheEigene (k->neutralerNeuaufbau);
        setzeEigene (k->nichtNeutralerNeuaufbau);
    }
    return generation;
}

void ControlClient::loescheAufbauUrteil (std::uint64_t generation)
{
    if (generation == 0)
        return;
    auto a = generation;
    k->neutralerNeuaufbau.compare_exchange_strong (a, 0);
    auto b = generation;
    k->nichtNeutralerNeuaufbau.compare_exchange_strong (b, 0);
}

std::uint64_t ControlClient::wireGenerationJetzt() const noexcept
{
    return k->wireGeneration.load();
}

/** 🔑 NAK-180 Nacharbeit 1 (EP-05): die Generation des sterbenden Links,
    hinterlegt fuer die Dauer des negativen Callbacks.

    `thread_local` aus demselben Grund wie `tlAufbauGeneration`: der negative
    Callback laeuft SYNCHRON auf dem Aufruferthread von `reconnect()`/`stop()`
    (oder auf dem Clientthread bei `eineVerbindung`), und zwei Clients in einem
    Prozess teilten sonst dieselbe Zahl. Der Wert wird um den Aufruf herum
    gesichert und wiederhergestellt, weil der negative Callback reentrant aus
    dem positiven erreichbar ist (`PluginProcessor.cpp` ruft `reconnect()` aus
    `v3ControlLink(true)`). */

thread_local std::uint64_t tlLinkEndeGeneration = 0;

std::uint64_t ControlClient::sterbendeGenerationJetzt() const noexcept
{
    return tlLinkEndeGeneration;
}

bool ControlClient::kopplung (std::string& linkId, std::string& challenge) const
{
    return k->kopplung (linkId, challenge);
}

bool ControlClient::Laufzeit::sollAbbrechen (std::uint64_t generation) const noexcept
{
    return ! laeuft.load() || verbindungsGeneration.load() != generation;
}

bool ControlClient::Laufzeit::kopplung (std::string& linkId, std::string& challenge) const
{
    std::lock_guard<std::mutex> l (zustandMutex);
    if (zustand.status != Status::verbunden || zustand.linkId.empty())
        return false;
    linkId = zustand.linkId;
    challenge = zustand.challenge;
    return true;
}

/** NAK-180 R1/R13: ein verworfener Aufbau-Heartbeat gibt seine Aussage zurueck.

    Ohne das waere die Aussage weg: das Flag ist beim Bilden des Heartbeats
    per CAS verbraucht worden, und wenn dieser Heartbeat den Draht nie
    erreicht, faende der Broker nie ein `false` - die Sitzung bliebe fuer
    immer gesperrt (D-01 in neuer Form). Zurueckgestellt wird nur, wenn der
    Platz noch LEER ist: hat ein neuerer Callback dort schon geschrieben,
    gehoert er ihm, und seine Aussage ist die juengere. */

void ControlClient::Laufzeit::aufbauAussageZurueckstellen (std::uint64_t marke)
{
    if (marke == 0 || aufbauHeartbeatMarke.load() != marke)
        return;
    const auto generation = aufbauHeartbeatGeneration.load();
    if (generation == 0)
        return;
    // 🔑 Nacharbeit 1 (EP-06/N-12): BEIDE verbrauchten Aussagen kommen
    // zurueck. Der Heartbeat verbraucht beide Flags; welche gesetzt waren,
    // steht in den zwei Merkbits. Eine halb zurueckgestellte Aussage waere
    // dieselbe verlorene Aussage in kleiner.
    auto zurueck = [generation] (std::atomic<std::uint64_t>& flag)
    {
        std::uint64_t leer = 0;
        flag.compare_exchange_strong (leer, generation);
    };
    if (aufbauHeartbeatWarNeutral.load())
        zurueck (neutralerNeuaufbau);
    if (aufbauHeartbeatWarUnbekannt.load())
        zurueck (nichtNeutralerNeuaufbau);
    aufbauHeartbeatMarke.store (0);
}

/** Der Aufbauzug 2+3 als EINE benannte Operation (NAK-180 R10/R12).

    Er steht als Methode und nicht mehr inline in `eineVerbindung`, damit
    Produkt UND Bein denselben Zug fahren. Ein Test, der `beiLinkStatus` ohne
    echte Pipe ausloest, liesse `wireGeneration` sonst auf 0 - und `0` heisst
    "keine Aussage": jede generationsgebundene Wirkung liefe ins Leere, und
    der Test waere gruen oder rot aus einem Grund, den das Produkt nicht hat.

    Getrennt waeren Vergabe und Filter ausserdem ein Fenster, in dem ein
    Einreiher die neue Generation schon liest, der Filter aber noch nicht
    durch ist. Unter EINER Sperre gibt es dieses Fenster nicht. */
/** Der Heartbeat-Schritt der Sendeschleife (NAK-180 R1/R13) - als EINE
    benannte Operation.

    🔑 Nacharbeit 1 (EP-18): er steht als Methode und nicht mehr inline in
    `eineVerbindung`, damit ein Bein DENSELBEN Schritt fahren kann. Ein Test,
    der den Handschlag nur ueber `v3LinkFuerTest` fuhr, beruehrte weder
    Textbildung noch P0-Enqueue, Wire-Commit oder Rueckstellung - er mass den
    Callback und nannte es Handschlag.

    Die Aussage des Aufbaus reist GENAU EINMAL: verbraucht wird per CAS auf die
    eigene Generation - schlaegt er fehl, hat ein NEUERER Callback den Platz
    belegt, und die Aussage gehoert ihm. Ein blindes `exchange` haette dessen
    Wirkung geloescht (MP4-2). Eine FREMDE Generation wird ebenfalls nur per
    CAS aufgeraeumt, damit sie nicht ein gerade geschriebenes G+1 mitnimmt.

    Rueckgabe: wurde der Heartbeat eingereiht? `textAus` bekommt den gebildeten
    Wiretext, wenn der Aufrufer ihn braucht. */

bool ControlClient::Laufzeit::heartbeatSchritt (const ControlHello& hello,
                                               std::uint64_t sequence,
                                               const ControlStatus& status,
                                               std::uint64_t dieseWireGeneration,
                                               std::string* textAus)
{
    auto verbrauche = [this, dieseWireGeneration] (std::atomic<std::uint64_t>& flag)
    {
        auto gesehen = flag.load();
        if (gesehen == 0)
            return false;
        if (gesehen == dieseWireGeneration)
            return flag.compare_exchange_strong (gesehen, 0);
        // Fremde Generation: nicht werten, nur aufraeumen - und auch das nur,
        // wenn sie noch dasteht.
        flag.compare_exchange_strong (gesehen, 0);
        return false;
    };
    const bool bestaetigtNeutral = verbrauche (neutralerNeuaufbau);
    const bool bestaetigtUnbekannt = verbrauche (nichtNeutralerNeuaufbau);

    auto statusFuerDraht = status;
    if (bestaetigtUnbekannt)
        statusFuerDraht.interventionStateUnknown = true;

    // Marke: nur der Aufbau-Heartbeat braucht eine - er ist der einzige,
    // dessen Verlust eine Aussage kostet.
    std::uint64_t marke = 0;
    if (bestaetigtNeutral || bestaetigtUnbekannt)
    {
        marke = p0MarkenFolge.fetch_add (1) + 1;
        aufbauHeartbeatMarke.store (marke);
        aufbauHeartbeatGeneration.store (dieseWireGeneration);
        aufbauHeartbeatWarNeutral.store (bestaetigtNeutral);
        aufbauHeartbeatWarUnbekannt.store (bestaetigtUnbekannt);
    }
    auto text = heartbeatAlsJson (hello.adresse, sequence, statusFuerDraht,
                                  bestaetigtNeutral);
    if (textAus)
        *textAus = text;
    // 🔑 NAK-180 Nacharbeit 1 (EP-06/N-12): der Rueckgabewert wird
    // AUSGEWERTET.
    //
    // Weist die P0-Queue ab (voll oder zu gross), existiert gar kein Eintrag,
    // dessen spaeterer Verwurf die Aussage zurueckstellen koennte - beide
    // Flags sind aber schon per CAS verbraucht. Die Aussage entstuende nur
    // zufaellig durch einen weiteren Linkaufbau neu; bis dahin bliebe die
    // Sitzung gesperrt, ohne dass sie je jemand aufloesen kann (D-01 in neuer
    // Form).
    if (sendeP0 (std::move (text), P0Klasse::bericht, marke))
        return true;
    if (marke != 0)
        aufbauAussageZurueckstellen (marke);
    return false;
}

std::uint64_t ControlClient::Laufzeit::aufbauZug()
{
    const auto neueGeneration = wireGeneration.load() + 1;
    std::vector<std::uint64_t> verworfeneMarken;
    // 🔑 NAK-180 Nacharbeit 2 (WN-06/N-35): Phase 0 - der Zug ist BETRETEN,
    // die Sperre wird als naechstes angefordert. Ein Bein weiss damit
    // deterministisch, dass sein zweiter Faden wirklich hier steht; vorher
    // musste es das aus einer Pause schliessen.
    if (aufbauZugHakenFuerTest)
        aufbauZugHakenFuerTest (0);
    {
        std::lock_guard<std::mutex> l (sendeMutex);
        // Phase 1 - die Sperre ist UEBERNOMMEN. Faellt sie, waehrend ein
        // Sendezug sie noch haelt, ist es nicht dieselbe Sperre: genau das
        // misst der Rotbeweis mit aufgeteiltem Mutex.
        if (aufbauZugHakenFuerTest)
            aufbauZugHakenFuerTest (1);
        wireGeneration.store (neueGeneration);
        // Berichte des alten Links fallen - ihre Aussage galt ihm allein.
        p0.berichteAelterAls (neueGeneration,
                              [&verworfeneMarken] (std::uint64_t m)
                              { if (m != 0) verworfeneMarken.push_back (m); });
        // Zustellpruefung: liegt noch ein INTERVENTIONSEREIGNIS aelterer
        // Generation da, kann darunter ein `end` sein, dessen Begin auf dem
        // alten Link zugestellt wurde. Es reist nie ohne sein Begin - der
        // Prozessor liefert den Wiretext, wir stellen ihn voran.
        //
        // 🔑 Nacharbeit 1 (EP-09): die Frage gilt NUR Interventionen (ein
        // persistenter Befehl loeste sonst ein Replay aus, das niemand
        // angefordert hat), und der Eintrag bekommt eine ECHTE Marke aus dem
        // gemeinsamen Raum. Ohne sie blieb der Zustellstand des Prozessors auf
        // „nicht eingereiht", und der geweckte Worker reihte dasselbe Begin
        // ein zweites Mal ein - die doppelte `intervention_id` aus N-27.
        if (hookReplayBegin && p0.hatInterventionsereignisAelterAls (neueGeneration))
        {
            const auto marke = p0MarkenFolge.fetch_add (1) + 1;
            const auto replay = hookReplayBegin (neueGeneration, marke);
            if (! replay.empty()
                && ! p0.voranstellen (P0Eintrag { replay, P0Klasse::bericht,
                                                  neueGeneration, marke }))
            {
                // Der Hook hat das Begin bereits als eingereiht gebucht; ohne
                // Eintrag kaeme nie ein Rueckruf. Die Marke geht deshalb
                // sofort in die Verwurfmeldung - der Zustellstand faellt damit
                // auf „nicht eingereiht" zurueck, und der naechste Zug replayt.
                verworfeneMarken.push_back (marke);
            }
        }
    }
    for (const auto m : verworfeneMarken)
    {
        aufbauAussageZurueckstellen (m);
        if (beiP0Verworfen)
            beiP0Verworfen (m);
    }
    return neueGeneration;
}

std::uint64_t ControlClient::aufbauZug() { return k->aufbauZug(); }

std::uint64_t ControlClient::linkAufbauFuerTest (const std::function<void()>& imCallback)
{
    const auto generation = k->aufbauZug();
    tlAufbauGeneration = generation;
    // 🔑 Nacharbeit 1 (EP-05): dasselbe Buch wie `meldeLinkStatus(true)`. Ohne
    // die hinterlegte Zahl faende ein spaeterer Ende-Callback des Beins die
    // sterbende Generation nicht und loeschte nichts - der Test maesse einen
    // Pfad, den das Produkt nicht hat.
    k->linkAlsVerbundenGemeldet.store (true);
    k->gemeldeteLinkGeneration.store (generation);
    if (imCallback)
        imCallback();
    tlAufbauGeneration = 0;
    return generation;
}

std::uint64_t ControlClient::linkEndeFuerTest (const std::function<void()>& imCallback)
{
    // Die Gegenrichtung von `linkAufbauFuerTest`, Zeile fuer Zeile wie
    // `meldeLinkStatus(false)`: die gemeldete Generation wird EINMAL
    // verbraucht und fuer die Dauer des Callbacks hinterlegt.
    k->linkAlsVerbundenGemeldet.store (false);
    const auto sterbend = k->gemeldeteLinkGeneration.exchange (0);
    const auto vorheriger = tlLinkEndeGeneration;
    tlLinkEndeGeneration = sterbend;
    if (imCallback)
        imCallback();
    tlLinkEndeGeneration = vorheriger;
    return sterbend;
}

bool ControlClient::heartbeatSchrittFuerTest (const ControlHello& hello,
                                             std::uint64_t sequence,
                                             const ControlStatus& status,
                                             std::string& textAus)
{
    return k->heartbeatSchritt (hello, sequence, status,
                                k->wireGeneration.load(), &textAus);
}

std::string ControlClient::heartbeatTextFuerTest (const Adresse& adresse,
                                                 std::uint64_t sequence,
                                                 const ControlStatus& status)
{
    // Dieselbe Verbrauchsregel wie in der Sendeschleife (R1/R13): CAS auf die
    // eigene Generation, fremde nur aufraeumen.
    const auto meine = k->wireGeneration.load();
    auto verbrauche = [meine] (std::atomic<std::uint64_t>& flag)
    {
        auto gesehen = flag.load();
        if (gesehen == 0)
            return false;
        if (gesehen == meine)
            return flag.compare_exchange_strong (gesehen, 0);
        flag.compare_exchange_strong (gesehen, 0);
        return false;
    };
    const bool neutral   = verbrauche (k->neutralerNeuaufbau);
    const bool unbekannt = verbrauche (k->nichtNeutralerNeuaufbau);
    auto st = status;
    if (unbekannt)
        st.interventionStateUnknown = true;
    return heartbeatAlsJson (adresse, sequence, st, neutral);
}

void ControlClient::Laufzeit::meldeLinkStatus (bool verbunden)
{
    // Ein Reconnect/stop darf die Subscription sofort entwerten, auch wenn
    // der blockierte Read erst danach zurueckkehrt. `exchange` verhindert den
    // spaeteren doppelten Ende-Callback aus `eineVerbindung`.
    const bool vorher = linkAlsVerbundenGemeldet.exchange (verbunden);
    if (vorher == verbunden || ! beiLinkStatus)
    {
        // Kein Wechsel: aber die Zahl des gerade gemeldeten Links darf nicht
        // von einem spaeteren `true` ohne Wechsel ueberschrieben werden.
        return;
    }
    if (verbunden)
    {
        // 🔑 EP-05: die Generation DIESES Aufbaus wird hinterlegt, bevor der
        // Callback laeuft. `tlAufbauGeneration` ist gesetzt, wenn der Aufruf
        // aus `eineVerbindung` oder `linkAufbauFuerTest` kommt; sonst ist die
        // laufende Zahl die richtige.
        gemeldeteLinkGeneration.store (tlAufbauGeneration != 0
                                           ? tlAufbauGeneration
                                           : wireGeneration.load());
        beiLinkStatus (true);
        return;
    }
    // 🔑 EP-05/N-37 Fall 2: der negative Callback bekommt die STERBENDE
    // Generation. `exchange` gibt sie genau einem Aufrufer; ein spaeterer
    // Aufruf ohne Statuswechsel kommt hier gar nicht an. Der alte Wert wird
    // gesichert und wiederhergestellt, weil dieser Callback reentrant aus dem
    // positiven erreichbar ist.
    const auto sterbend = gemeldeteLinkGeneration.exchange (0);
    const auto vorheriger = tlLinkEndeGeneration;
    tlLinkEndeGeneration = sterbend;
    beiLinkStatus (false);
    tlLinkEndeGeneration = vorheriger;
}

void ControlClient::Laufzeit::threadLauf (std::uint64_t meinLauf,
                                         std::shared_ptr<IpcVerbindung> meine)
{
    threadId.store (std::this_thread::get_id());
    int backoffMs = kBackoffStartMs;
    while (laeuft.load() && lebenslauf.load() == meinLauf)
    {
        const auto generation = verbindungsGeneration.load();
        const bool stand = eineVerbindung (generation, meinLauf, *meine);
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
            // Kein automatischer Fallback und kein wiederholtes Anklopfen an
            // einen belegten, aber unverifizierten Namen. Nur ein bewusster
            // reconnect()/stop() aendert die Generation und loest die Sperre.
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
            backoffMs = kBackoffStartMs;  // bewusster Reconnect wartet nicht laenger
            continue;
        }
        backoffMs = std::min (backoffMs * 2, kBackoffMaxMs);
    }
    // Erst JETZT ist der Thread fertig — `stop()` wartet auf genau dieses
    // Zeichen und darf danach joinen (`B-CC-10`).
    // Nur der AKTUELLE Lauf meldet sich fertig: ein abgeloester Vorgaenger
    // wuerde sonst den `join` des neuen Laufs freigeben.
    if (lebenslauf.load() == meinLauf)
        fertig.store (true);
}

bool ControlClient::Laufzeit::eineVerbindung (std::uint64_t generation,
                                             std::uint64_t meinLauf,
                                             IpcVerbindung& verbindung)
{
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        if (sollAbbrechen (generation))
            return false;
        zustand.status = Status::verbindet;
        zustand.brokerPipeFehlt = false;
        zustand.serverPruefstatus = ServerPruefStatus::nichtGeprueft;
        zustand.serverPrueffehler = ServerPruefFehler::keiner;
        zustand.serverPid = 0;
        ++zustand.verbindungsVersuche;
        zustand.linkId.clear();
        zustand.challenge.clear();
        zustand.brokerEpoch.clear();
        zustand.brokerVersion.clear();
    }
    ControlHello hello = helloProvider ? helloProvider() : ControlHello();
    // NAK-40: Der Provider liefert die persistente Original-ID. Erst an der
    // v3-Grenze entsteht der Wirealias; in den Host-State fliesst er nie
    // zurueck. Leere bleibt leer und faellt weiter fail-closed — nur der
    // State-Lader darf dafuer den bestehenden Frisch-UUID-Pfad waehlen.
    hello.adresse = wireAdresseAusState (hello.adresse);
    // Der Provider ist FREMDER Code und darf beliebig lange stehen. In dieser
    // Zeit kann `stop()` diesen Lauf abgeloest und ein neuer `start()` laengst
    // verbunden haben. Dann wird hier NICHT mehr geoeffnet: ein abgeloester
    // Lauf macht keine neue Pipe auf und meldet auch keinen Zustand mehr
    // (`B-CC-12`/`B-CC-16`, NAK-104).
    if (sollAbbrechen (generation))
        return false;
    if (! adresseGueltig (hello.adresse))
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        if (sollAbbrechen (generation))
            return false;
        zustand.status = Status::getrennt;
        zustand.letzterFehler = "Adresse haelt den v3-Vertrag nicht (hex32/SID)";
        return false;
    }
    // Audiofelder VOR der Serialisierung verriegeln — und vor dem Oeffnen der
    // Pipe: ein Hello mit NaN-Samplerate haette der Broker ohnehin abgelehnt,
    // aber die Wandlung dorthin waere schon vorher undefiniertes Verhalten
    // gewesen (T2-Befund 9 vom 2026-08-29).
    if (! audioGueltig (hello.samplerate, hello.blockSize, hello.channels))
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        if (sollAbbrechen (generation))
            return false;
        zustand.status = Status::getrennt;
        zustand.letzterFehler =
            "Audiolage haelt den v3-Vertrag nicht (samplerate/block_size/channels)";
        return false;
    }

    std::string fehler;
    ServerPruefBericht serverBericht;
    // NAK-134 Nacharbeit Runde 1, R5: Das Abbruchsignal gehoert der
    // Verbindungsgeneration, nicht dem Oeffnungsaufruf. Es wird HIER geloest —
    // und die Generation unmittelbar danach ERNEUT gelesen. `stop()` und
    // `reconnect()` erhoehen sie VOR `ioAbbrechen()`; ein Abbruch, den das
    // Loesen verschlucken koennte, hat die Generation also vorher schon
    // erhoeht und faellt in genau diese zweite Pruefung. Beides gehoert
    // zusammen: ohne das Loesen bliebe das Signal der vorigen Runde stehen und
    // keine Verbindung kaeme je zustande, ohne die zweite Pruefung liefe der
    // Fall aus Defekt 1 weiter (4.009 ms statt der R5-Frist).
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
                zustand.brokerPipeFehlt = serverBericht.status == ServerPruefStatus::nichtDa;
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
        // Ein reconnect() kann waehrend PID-/SID-/Datei-/Hashpruefung die
        // Generation wechseln. Das alte Urteil darf danach weder den neuen
        // Lauf als verifiziert markieren noch dessen Lifecycle-Mutex loesen.
        if (serverGeoeffnet)
            verbindung.schliessen();
        return false;
    }
    if (! serverGeoeffnet)
        return false;

    const std::string helloJson = helloAlsJson (hello);

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

    // ── welcome lesen: bereits v3-gerahmt (§53.9) ─────────────────────────
    StromLeser leser;
    std::uint8_t puffer[4096];
    const auto welcomeFrist = IpcVerbindung::fristIn (kIoFristMs);
    std::string linkId, challenge, brokerEpoch, brokerVersion;
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
            if (e.kopf.familie != Familie::p0)
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler = "welcome kam nicht als P0";
                break;
            }
            const std::string text (reinterpret_cast<const char*> (e.payload), e.payloadLaenge);
            std::vector<JsonFeld> felder;
            std::string typ;
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
            if (! welcomeHaeltVertrag (felder, linkId, challenge, brokerEpoch, brokerVersion))
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler = "unerwartete Antwort auf hello";
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
        if (ausgang == LeseAusgang::zeitlimit)
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.letzterFehler = "kein welcome innerhalb der Frist";
        }
        else if (! sollAbbrechen (generation))
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.letzterFehler = fehler.empty() ? "Verbindung vor dem welcome beendet" : fehler;
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
        zustand.linkId = linkId;
        zustand.challenge = challenge;
        zustand.brokerEpoch = brokerEpoch;
        zustand.brokerVersion = brokerVersion;
        zustand.letzterFehler.clear();
    }
    // 🔑 NAK-180 R10/R12: der Aufbauzug 2+3 laeuft VOR dem Link-Callback.
    //
    // Vorher stand `meldeLinkStatus (true)` hier oben und die Generation
    // darunter. Weil der positive Callback den Worker fuer ein Replay weckt,
    // konnte der sein Replay noch unter der ALTEN Generation einreihen - und
    // der unmittelbar folgende Filter warf es weg. Das `end` traf beim Broker
    // auf kein Begin und setzte `unknown` (MP2-1/MP4-1).
    const auto dieseWireGeneration = aufbauZug();

    // Das Fenster, in dem `meldeAufbauUrteil` die Generation DIESES Aufbaus
    // sieht - nicht die aktuelle. Ein ueberholter Callback stempelt damit
    // seine eigene, alte Zahl, und seine Wirkung ist fuer den naechsten Link
    // inert (R11).
    tlAufbauGeneration = dieseWireGeneration;
    meldeLinkStatus (true);
    tlAufbauGeneration = 0;

    // Was der letzte Verbindungsabbruch offen liess, geht jetzt zuerst raus
    // (§53.9 "nicht koaleszierbare Events bei Ueberlauf ueber Reconnect
    // wiederholen").
    {
        std::lock_guard<std::mutex> l (sendeMutex);
        p1.nachReconnectWiederholen();
    }
    inFlightNachReconnect (dieseWireGeneration);

    // Stand der P0-Ueberlaeufe beim Verbindungsaufbau. Waechst er waehrend
    // dieser Verbindung, wird sie geschlossen — nichts wird verworfen.
    const std::uint64_t ueberlaufBeimVerbinden = p0UeberlaufZaehler.load();
    auto ueberlaufSeitVerbinden = [this, ueberlaufBeimVerbinden] {
        return p0UeberlaufZaehler.load() != ueberlaufBeimVerbinden;
    };

    // Ratengrenze je Verbindung, DIESELBE wie im Broker (§33.1). Die Uhr ist
    // die des Aufrufers, damit die Klasse selbst keine liest.
    Ratengrenze rate (kRateProSekunde, kRateFensterMs);
    const auto rateBeginn = std::chrono::steady_clock::now();

    // Produktstatus lebt nicht in der allgemeinen Userqueue: der Provider ist
    // explizit und fehlt in Transport-/Fuzztests. Bei gesetztem Provider wird
    // unmittelbar und danach im 1-Hz-Takt ein voller Heartbeat erzeugt. Der
    // State-Report wird auf derselben Taktkante nur bei Aenderung koalesziert.
    auto naechsterHeartbeat = std::chrono::steady_clock::now();
    std::string letzterStateReport;

    // `B-CC-06`/`B-CC-07` (Regel 4): der Empfangsweg als eigener Schritt.
    // `false` heisst: die Verbindung ist zu beenden. Er wird in JEDER Runde
    // gegangen — auch direkt nach einem Send — und einmal zusaetzlich, bevor
    // ein gescheiterter Write die Verbindung schliesst.
    bool leseFehler = false;
    auto empfangenes = [&] (int fristMs) -> bool
    {
        std::size_t gelesen = 0;
        const auto la = verbindung.lesen (puffer, sizeof (puffer), gelesen,
                                          IpcVerbindung::fristIn (fristMs), fehler);
        if (la == LeseAusgang::fehler || la == LeseAusgang::ende)
        {
            leseFehler = true;
            return false;
        }
        if (la == LeseAusgang::daten && gelesen > 0)
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
            // Die Familienzuordnung des Vertrags gilt in BEIDE Richtungen:
            // die Control-Verbindung traegt ausschliesslich P0/P1 (§33.1).
            // Ohne diese Sperre reichte ein korrekt gerahmter P2-Frame vom
            // Peer seine Binaerpayload an `beiAntwort` weiter, das JSON
            // erwartet (T2-Befund 4 vom 2026-08-29).
            if (e.kopf.familie == Familie::p2
                || e.kopf.encoding != Kodierung::json)
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler =
                    "P2 oder Nicht-JSON auf der Control-Verbindung — wird geschlossen";
                ++zustand.familieAbweisungen;
                return false;
            }
            if (e.kopf.schemaMinor > kJsonSchemaMinor)
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler =
                    "Envelope schema_minor ist neuer als der JSON-Leser — wird geschlossen";
                ++zustand.envelopeAbweisungen;
                return false;
            }

            // Ratengrenze VOR dem Callback: ein Peer, der hinter dem welcome
            // beliebig viele Frames pipelined, darf den Aufrufer nicht damit
            // fluten (§33.1, T2-Befund 5 vom 2026-08-29).
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

            {
                std::lock_guard<std::mutex> l (zustandMutex);
                ++zustand.empfangen;
            }
            const std::string antwort (reinterpret_cast<const char*> (e.payload),
                                       e.payloadLaenge);
            inFlightAck (antwort);
            // Nach `stop()` wird kein Callback mehr gerufen (`B-CC-10`).
            if (! sollAbbrechen (generation))
            {
                if (beiVersionierterAntwort)
                    beiVersionierterAntwort (antwort, e.kopf.schemaMinor);
                else if (beiAntwort)
                    beiAntwort (antwort);
            }
        }
    };

    std::vector<std::uint8_t> ausgang;
    P0Eintrag p0Eintrag;                 // NAK-180 R7: Klasse und Marke reisen mit
    while (! sollAbbrechen (generation))
    {
        const auto jetzt = std::chrono::steady_clock::now();
        if (statusProvider && jetzt >= naechsterHeartbeat)
        {
            const auto status = statusProvider();
            if (sollAbbrechen (generation))
                break;

            const auto report = stateReportJson (hello.adresse, status);
            if (report != letzterStateReport)
            {
                // 🔑 SONDE-014 WN-01: die Revision, die MIT DIESEM Bericht
                // reist. Sie wird erst am Wire-Commit unten gemeldet - ein
                // eingereihter Bericht steht hinter jedem P0, das vor ihm
                // entnommen wird, und genau diese Ueberholung ist der Kern
                // von WP1-1.
                stateReportRevisionEingereiht.store (status.stateRevision);
                sendeP1 (kSchluesselStateReport, report);
                letzterStateReport = report;
            }

            const auto sequence = heartbeatFolge.fetch_add (1) % kJsonSafeModulus;

            (void) heartbeatSchritt (hello, sequence, status, dieseWireGeneration,
                                     nullptr);
            naechsterHeartbeat = jetzt + std::chrono::milliseconds (kHeartbeatTaktMs);
        }

        if (ueberlaufSeitVerbinden())
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.letzterFehler = "P0-Ueberlauf: Verbindung wird geschlossen";
            break;
        }

        // 1) Steuerung zuerst, immer. P1 kommt erst dran, wenn P0 leer ist —
        //    das ist die Client-Haelfte von "kein P0 wartet hinter Daten".
        bool etwasGesendet = false;
        std::string nachricht, schluessel;
        bool istP0 = false;
        std::size_t p1TiefeJetzt = 0, p1WiederholJetzt = 0;
        {
            std::lock_guard<std::mutex> l (sendeMutex);
            istP0 = p0.entnehmen (p0Eintrag);
            if (istP0)
                nachricht = p0Eintrag.json;
            etwasGesendet = istP0 || p1.entnehmen (schluessel, nachricht);
            // 🔑 Die Fuellstaende werden HIER gelesen, unter `sendeMutex`.
            // `zustandMutex` schuetzt den Snapshot, nicht die Queues; ein
            // `p1.groesse()` dort waere ein Datenrennen auf der Deque, auch
            // wenn das Ergebnis "nur eine Zahl" ist.
            p1TiefeJetzt = p1.groesse();
            p1WiederholJetzt = p1.wiederholungen();
        }
        if (etwasGesendet)
        {
            const auto familie = istP0 ? Familie::p0 : Familie::p1;
            if (! envelopeSchreiben (familie, kJsonSchemaMinor,
                                     reinterpret_cast<const std::uint8_t*> (nachricht.data()),
                                     nachricht.size(), ausgang)
                || ! verbindung.schreibenGenau (ausgang.data(), ausgang.size(),
                                                IpcVerbindung::fristIn (kIoFristMs), fehler))
            {
                // Entnommen, aber NICHT auf dem Draht: der Eintrag geht
                // zurueck. Ohne das verschwand ein nicht koaleszierbares
                // P1-Ereignis endgueltig, wenn der Broker zwischen Entnahme und
                // Write schloss — trotz Reconnect-Vertrag (§53.9). Fuer P0 gilt
                // dieselbe Zusage noch strenger: "nichts verwerfen".
                {
                    std::lock_guard<std::mutex> l (sendeMutex);
                    if (istP0)
                    {
                        // 🔑 NAK-180 R7: ein BERICHT geht nicht zurueck.
                        //
                        // "Nichts verwerfen" gilt Ereignissen (§53.9). Ein
                        // Heartbeat ist periodisch, ein Replay-Begin wird beim
                        // naechsten Aufbau neu gebildet - beide gelten nur fuer
                        // den Link, der gerade stirbt. Zurueckgelegt reisten sie
                        // auf dem NAECHSTEN und behaupteten dort etwas Falsches:
                        // ein altes `intervention_state_unknown: false` loeste
                        // den Resync aus, waehrend der Marker klang.
                        if (p0Eintrag.klasse == P0Klasse::bericht)
                        {
                            p0.fallenLassen();
                            aufbauAussageZurueckstellen (p0Eintrag.marke);
                            if (beiP0Verworfen && p0Eintrag.marke != 0)
                                beiP0Verworfen (p0Eintrag.marke);
                        }
                        else
                        {
                            p0.zuruecklegen (std::move (p0Eintrag));
                        }
                    }
                    else
                        p1.zuruecklegen (schluessel, std::move (nachricht));
                    // Hier ist `sendeMutex` bereits gehalten (Zeile darueber),
                    // die Abfrage der Queues ist also gedeckt.
                    const auto tiefe = p1.groesse();
                    const auto wiederholungen = p1.wiederholungen();
                    std::lock_guard<std::mutex> z (zustandMutex);
                    zustand.p1Wiederholungen = wiederholungen;
                    zustand.p1Tiefe = tiefe;
                    zustand.p1WiederholTiefe = wiederholungen;
                }
                // `B-CC-07`: was schon vollstaendig empfangen wurde, wird
                // noch GEMELDET, bevor die Verbindung endet. Sonst ginge genau
                // der P0-ACK verloren, auf den der Aufrufer wartet — der Write
                // scheiterte ja oft, weil der Peer nur nicht mehr liest.
                empfangenes (0);

                // Der Platz war bis hierher reserviert; `zuruecklegen` hat ihn
                // wieder mit dem Eintrag belegt. Nichts ist verlorengegangen.
                if (! sollAbbrechen (generation))
                {
                    std::lock_guard<std::mutex> l (zustandMutex);
                    // Der Ueberlauf ist die URSACHE, der abgebrochene Write nur
                    // seine Folge. Wer die Folge meldet, verschleiert den Grund.
                    zustand.letzterFehler =
                        ueberlaufSeitVerbinden()
                            ? std::string ("P0-Ueberlauf: Verbindung wird geschlossen")
                            : (fehler.empty() ? std::string ("Nachricht zu gross") : fehler);
                }
                break;
            }
            // Auf dem Draht: erst JETZT gibt die Queue den reservierten Platz
            // frei (§53.9 "nichts verwerfen" gilt bis zum Write-Commit).
            {
                std::lock_guard<std::mutex> l (sendeMutex);
                if (istP0)
                {
                    p0.bestaetigen();
                    // NAK-180 R7: DAS ist der Wire-Commit. Erst hier weiss der
                    // Einreicher, dass seine Nachricht wirklich raus ist - der
                    // Rueckgabewert von `sendeP0` sagt nur "eingereiht".
                    //
                    // 🔑 Nacharbeit 2 (WN-02/N-27): gemeldet wird die
                    // Generation des TATSAECHLICHEN Wire-Commits, nicht die
                    // des Einreihens.
                    //
                    // Ein Intervensionsereignis geht beim Reconnect nicht
                    // verloren - es wird zurueckgelegt (unten) und auf dem
                    // NAECHSTEN Link geschrieben. Sein Eintrag traegt aber
                    // weiter die alte Zahl. Meldete der Rueckruf sie, stuende
                    // ein auf G+1 geschriebenes Begin beim Prozessor als "auf
                    // G zugestellt", und `replayNoetig` erzeugte beim `end`
                    // ein ZWEITES Begin derselben `intervention_id`:
                    // `sequenz_annehmen` sieht die Wiederholung und setzt die
                    // Sitzung `unknown` - genau das, was N-27 hier ausdruecklich
                    // ausschliesst.
                    if (beiP0Zugestellt && p0Eintrag.marke != 0)
                        beiP0Zugestellt (p0Eintrag.marke, dieseWireGeneration);
                }
                else
                {
                    p1.bestaetigen();
                    // 🔑 SONDE-014 WN-01: JETZT kennt der Broker diesen Stand.
                    //
                    // Der Schluessel ist die Identitaet des
                    // Koaleszierungsobjekts, kein Rohtextfund: die P1-Queue
                    // ersetzt einen Bericht an seiner Position, der zuletzt
                    // eingereihte ist also der, der hier hinausgeht.
                    if (schluessel == kSchluesselStateReport)
                        stateReportRevisionGemeldet.store (
                            stateReportRevisionEingereiht.load());
                }
            }
            if (istP0)
            {
                inFlightNachWireWrite (nachricht, dieseWireGeneration);
                // Ein Reconnect kann mehr semantisch offene Auftraege als
                // P0-Plaetze vorfinden. Jeder frei gewordene Queueplatz zieht
                // deshalb den naechsten alten In-Flight-Eintrag nach; Eintraege
                // dieser Generation werden dabei nie ohne Verbindungsverlust
                // erneut gesendet.
                inFlightNachReconnect (dieseWireGeneration);
            }
            std::lock_guard<std::mutex> l (zustandMutex);
            if (istP0)
                ++zustand.p0Gesendet;
            else
                ++zustand.p1Gesendet;
            // Der Fuellstand faellt auch dann, wenn eine Nachricht den Draht
            // VERLAESST — sonst saehe ein Sender, der ihn als Rueckstausignal
            // liest, eine Queue, die nie wieder leer wird. Gelesen wurde er
            // oben unter `sendeMutex`; hier wird er nur GEMELDET.
            zustand.p1Tiefe = p1TiefeJetzt;
            zustand.p1WiederholTiefe = p1WiederholJetzt;
            // KEIN `continue` mehr. Die alte Fassung sprang hier zurueck an
            // den Anfang und uebersprang den Lesepfad, solange irgendetwas
            // wartete: ein bereits vorliegender P0-ACK wurde nicht verarbeitet,
            // waehrend P1 rueckstaute, und ein blockierender P1-Write hungerte
            // ihn ganz aus. Das bricht den Gate-Satz "ohne P0-Starvation"
            // (NAK-95, Befund 4).
        }

        // 2) Lesen — IN JEDER RUNDE (`B-CC-06`). Direkt nach einem Send nur
        //    kurz pollen, damit der Durchsatz nicht am Lesetakt haengt; ist
        //    nichts zu senden, wird der volle Takt gewartet.
        if (! empfangenes (etwasGesendet ? 0 : kLeseTaktMs))
        {
            if (! sollAbbrechen (generation) && leseFehler)
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                if (ueberlaufSeitVerbinden())
                    zustand.letzterFehler = "P0-Ueberlauf: Verbindung wird geschlossen";
                else if (! fehler.empty())
                    zustand.letzterFehler = fehler;
            }
            break;
        }
    }

    verbindung.schliessen();
    if (! abgeloest (meinLauf))
    {
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.status = Status::getrennt;
            zustand.serverPruefstatus = ServerPruefStatus::nichtGeprueft;
            zustand.serverPrueffehler = ServerPruefFehler::keiner;
            zustand.serverPid = 0;
        }
        meldeLinkStatus (false);
    }
    return true;
}

} // namespace nakama::ipc

// K1 erneut: die zweite Auswertung sieht Makros, die erst ein spaeter
// eingebundener Header definiert haben koennte.
#include "NakamaKernRiegel.h"
