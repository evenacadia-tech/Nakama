/*  NAK-312 Etappe 2 (R-312-7) - das Sonde-Lebenslaufbein EqCopSondeLebenslaufTest.

    Bis hierher baute kein Bein eine Sonde mit LAUFENDEN v3-Clients: der
    Produktzweig mit `controlV3.start()` laeuft unter dem Testdefine nicht, und
    die Sonde hatte keinen Testkonstruktor (T3-04-01). Dieses Bein ist das
    Messgeraet fuer die Besitzschleuse der Etappe 4. Es baut die Sonde nach dem
    Muster Gen (NAK-246 D2) ueber den Testkonstruktor mit Probe-Pipe und
    Servererwartung, startet beide Clients ueber die Starthaken gegen den
    geteilten v3-Testserver (`V3TestServer.h`) und baut sie mit laufenden
    Clients wieder ab.

    Gemessen seit Etappe 2 (Manifest NAK-312 §6.1):
      312/M-05  der Testkonstruktor ist fail-closed
      312/M-06  die Starthaken verbinden ueber die Probe-Pipe; ohne sie bleiben
                beide Clients getrennt
      312/M-07  Aufbau und Abbau einer Sonde mit laufenden Clients
      312/M-08  jeder Name, mit dem ein Client startet, liegt im
                Probe-Namensraum
    Seit Etappe 4 (Manifest NAK-312 §6.4, R-312-7, R-312-12) die
    Besitzschleuse der Sonde, gemessen an einem gehaltenen Statusprovider:
      312/M-30  nach dem Ende des Eigentuemers erreicht kein Provider den
                Besitzerzugriff (gezaehlte Marke, nie ein Absturz)
      312/M-31  zwanzig Gegenzyklen; Handles und Threads steigen nicht
      312/M-32  die 2-s-Stoppfrist loest ab, der Destruktor wartet nur den Rest
      312/M-33  ein Reconnect ruehrt die Schleuse nicht an
      312/M-34  beide stop() vor dem Schliessen, danach erst die Mitglieder
      312/M-39  geordnet ueber Schleusenzustaende, drei zulaessige Ausgaenge
    Dazu aus dem Register NAK-345 Punkt 2:
      312/NAK-345  der Produktkonstruktor verdrahtet pipeNameV3 (aktuelleLogonSid())

    PRODUKTIONS-PIPE: nie. Jeder Name, mit dem ein Client starten soll, geht
    VOR dem Aufbau des Servers und VOR jedem Start durch `istProbePipename`;
    faellt die Wache, bricht der Fall ab, bevor irgendetwas eine Pipe oeffnet.
    Der fail-closed-Fall von 312/M-05 uebergibt absichtlich einen Namen
    ausserhalb des Namensraums und startet keinen Client. Der Fall
    312/NAK-345 baut eine Sonde aus dem Produktkonstruktor, deren Clients den
    Produktnamen tragen, und startet keinen: im Testbau laeuft der Startzweig
    des Produktkonstruktors nicht.

    LANDMINE NAK-175: Prozessoren liegen auf dem HEAP (`std::unique_ptr`), nie
    im Rahmen.

    Ein optionales Argument waehlt die Faelle, deren Kennung es enthaelt (der
    Rotlauf einer einzelnen Zeile); der Kanon faehrt ohne Argument alles.

    Exit 0 nur bei "SONDE-LEBENSLAUF OK".
*/

#include "SondeProcessor.h"
#include "IpcVerbindung.h"
#include "PipeToken.h"
#include "controlclient/Schleuse.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
 #define NOMINMAX
#endif
#include <windows.h>
#include <tlhelp32.h>

namespace
{
// Der Probe-Server spricht den v3-Bootstrap unqualifiziert (wie in B10 und B14).
using namespace nakama::ipc;
#include "V3TestServer.h"

using Sonde = nakama::sonde::SondeProcessor;
using Uhr   = std::chrono::steady_clock;

constexpr double kFs    = 48000.0;
constexpr int    kBlock = 256;
constexpr int    kVerbindenMs = 8000;

int fehler = 0, geprueft = 0;

void pruefe (bool ok, const std::string& was, const std::string& detail = {})
{
    ++geprueft;
    if (! ok) ++fehler;
    std::cout << (ok ? "  ok      " : "  FEHLER  ") << was;
    if (! detail.empty()) std::cout << "  [" << detail << "]";
    std::cout << std::endl;
}

void abschnitt (const std::string& titel)
{
    std::cout << std::endl << "== " << titel << " ==" << std::endl;
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

long long msSeit (Uhr::time_point t) noexcept
{
    return std::chrono::duration_cast<std::chrono::milliseconds> (Uhr::now() - t).count();
}

/** 312/M-08: die Wache des Beins. Jeder Name, mit dem ein Client starten soll,
    liegt im Probe-Namensraum - geprueft VOR dem Aufbau des Servers und VOR
    jedem Start. Faellt sie, bricht der Fall ab, bevor etwas eine Pipe oeffnet. */
bool imProbeNamensraum (const std::string& name, const std::string& wo)
{
    const bool ok = istProbePipename (name);
    pruefe (ok, "312/M-08 kein_bein_oeffnet_die_produktions_pipe (R-312-7): " + wo
                    + " liegt im Probe-Namensraum",
            name.empty() ? std::string ("leer") : name);
    return ok;
}

/** Eine Projektbindung und ein erlaubter Messpunkt: ohne sie weist der Client
    sein eigenes Hello ab ("Adresse haelt den v3-Vertrag nicht"). */
void binde (Sonde& p)
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

const char* wort (ControlClient::Status s)
{
    switch (s)
    {
        case ControlClient::Status::getrennt:  return "getrennt";
        case ControlClient::Status::verbindet: return "verbindet";
        case ControlClient::Status::verbunden: return "verbunden";
    }
    return "?";
}

const char* wort (TelemetryClient::Status s)
{
    switch (s)
    {
        case TelemetryClient::Status::getrennt:          return "getrennt";
        case TelemetryClient::Status::wartetAufKopplung: return "wartetAufKopplung";
        case TelemetryClient::Status::verbindet:         return "verbindet";
        case TelemetryClient::Status::verbunden:         return "verbunden";
    }
    return "?";
}

//==============================================================================
// 312/M-05 - der Testkonstruktor ist fail-closed.

void testkonstruktorIstFailClosed()
{
    abschnitt ("312/M-05 - der Testkonstruktor ist fail-closed (R-312-7)");

    const auto probe = testPipeName ("nak312-m05");
    if (! imProbeNamensraum (probe, "M-05: der Probe-Name des Beins"))
        return;

    // Ein Name ausserhalb des Probe-Namensraums, der auch nicht der
    // Produktionsname ist. Dieser Fall startet keinen Client.
    const std::string fremd = "\\\\.\\pipe\\evenacadia.nakama.nak312.nicht-probe";

    auto mitProbe = std::make_unique<Sonde> (probe, testExeErwartung());
    auto mitFremd = std::make_unique<Sonde> (fremd, testExeErwartung());

    const auto& uebernommen      = mitProbe->v3PipeNameFuerTest();
    const auto& fremdUebernommen = mitFremd->v3PipeNameFuerTest();
    const auto  controlFremd     = mitFremd->controlV3FuerTest();
    const auto  teleFremd        = mitFremd->telemetryV3FuerTest();

    std::string d = "Probe-Name uebernommen " + std::string (uebernommen == probe ? "ja" : "NEIN")
                  + ", fremder Name im Probe-Namensraum " + (istProbePipename (fremd) ? "JA" : "nein")
                  + ", uebernommen '" + fremdUebernommen + "', Control " + wort (controlFremd.status)
                  + " nach " + std::to_string (controlFremd.verbindungsVersuche) + " Versuchen, Telemetrie "
                  + wort (teleFremd.status);
    pruefe (uebernommen == probe && ! istProbePipename (fremd) && fremdUebernommen.empty()
                && controlFremd.status == ControlClient::Status::getrennt && controlFremd.verbindungsVersuche == 0
                && teleFremd.status == TelemetryClient::Status::getrennt,
            "312/M-05 testkonstruktor_ist_fail_closed (R-312-7): v3PipeNameFuerTest() liefert genau den "
            "uebergebenen Probe-Namen; ein Name ausserhalb des Probe-Namensraums wird als LEERER Pipename "
            "uebernommen - beide v3-Clients entstehen aus diesem einen Namen, und keiner hat verbunden",
            d);
}

//==============================================================================
// 312/M-06 und 312/M-07 - Starthaken, Verbindung ueber die Probe-Pipe, Abbau.

void starthakenUndAbbau()
{
    abschnitt ("312/M-06 und 312/M-07 - Starthaken und Abbau mit laufenden Clients (R-312-7)");

    const auto pipe = testPipeName ("nak312-m06");
    if (! imProbeNamensraum (pipe, "M-06: der Name des Testservers"))
        return;
    TestServer server (pipe);
    const bool steht = server.starten();
    pruefe (steht, "M-06: der Testserver steht auf der Probe-Pipe");
    if (! steht)
        return;

    auto p = std::make_unique<Sonde> (pipe, testExeErwartung());
    std::cout << "  sizeof (SondeProcessor) = " << sizeof (Sonde) << " Bytes" << std::endl;
    binde (*p);
    p->setRateAndBufferSizeDetails (kFs, kBlock);
    p->prepareToPlay (kFs, kBlock);

    // Ohne die Haken: Laden und Vorbereiten rufen beide `reconnect()`, und
    // trotzdem hat kein Client je einen Versuch gemacht.
    const auto vorher     = p->controlV3FuerTest();
    const auto teleVorher = p->telemetryV3FuerTest();
    const int  serverVorher = server.verbindungen.load();
    pruefe (vorher.status == ControlClient::Status::getrennt && vorher.verbindungsVersuche == 0
                && teleVorher.status == TelemetryClient::Status::getrennt && teleVorher.verbindungsVersuche == 0
                && serverVorher == 0,
            "312/M-06 ohne_starthaken_bleiben_beide_clients_getrennt (R-312-7): nach Laden und Vorbereiten "
            "hat im Testbau kein Client einen Versuch gemacht - controlV3.start() im Produktzweig laeuft "
            "unter dem Testdefine nicht",
            std::string ("Control ") + wort (vorher.status) + " nach " + std::to_string (vorher.verbindungsVersuche)
                + " Versuchen, Telemetrie " + wort (teleVorher.status) + ", Serververbindungen "
                + std::to_string (serverVorher));

    if (! imProbeNamensraum (p->v3PipeNameFuerTest(), "M-06: der Name, mit dem beide Clients starten"))
        return;

    const auto t0 = Uhr::now();
    p->v3StartFuerTest();
    const bool verbunden = warteAuf (kVerbindenMs, [&] {
        return p->controlV3FuerTest().status == ControlClient::Status::verbunden;
    });
    const auto controlMs = msSeit (t0);

    const auto t1 = Uhr::now();
    p->v3TelemetrieStartFuerTest();
    const bool teleVerbunden = warteAuf (kVerbindenMs, [&] {
        return p->telemetryV3FuerTest().status == TelemetryClient::Status::verbunden;
    });
    const auto teleMs = msSeit (t1);

    std::string helloControl, helloTele;
    {
        std::lock_guard<std::mutex> l (server.textMutex);
        helloControl = server.letztesControlHello;
        helloTele    = server.letztesTelemetryHello;
    }
    const bool sondeMeldetSich = helloControl.find ("\"plugin_kind\":\"active_probe\"") != std::string::npos;
    pruefe (verbunden && teleVerbunden && sondeMeldetSich && ! helloTele.empty(),
            "312/M-06 starthaken_verbinden_ueber_die_probe_pipe (R-312-7): nach v3StartFuerTest() meldet der "
            "ControlClient binnen 8 s verbunden, und die Telemetrie koppelt nach dessen welcome - beide ueber "
            "die Probe-Pipe des Testservers, das Hello traegt plugin_kind active_probe",
            std::string ("Control ") + wort (p->controlV3FuerTest().status) + " nach " + std::to_string (controlMs)
                + " ms, Telemetrie " + wort (p->telemetryV3FuerTest().status) + " nach " + std::to_string (teleMs)
                + " ms, Serververbindungen " + std::to_string (server.verbindungen.load())
                + ", Control-Hello " + std::to_string (helloControl.size()) + " Bytes");

    // 312/M-07: Abbau mit laufenden Clients.
    const bool laufenBeide = p->controlV3FuerTest().status == ControlClient::Status::verbunden
                          && p->telemetryV3FuerTest().status == TelemetryClient::Status::verbunden;
    const auto t2 = Uhr::now();
    p.reset();
    const auto abbauMs = msSeit (t2);
    pruefe (laufenBeide && p == nullptr,
            "312/M-07 sonde_mit_laufenden_clients_wird_abgebaut (R-312-7): eine Sonde auf dem Heap mit "
            "verbundenem Control- und Telemetrieclient wird zerstoert, und der Destruktor kehrt zurueck",
            std::string ("beide Clients vor dem Abbau verbunden ") + (laufenBeide ? "ja" : "NEIN")
                + ", Destruktor " + std::to_string (abbauMs) + " ms");
    server.stoppen();
}

//==============================================================================
// 312/NAK-345 - die Produktverdrahtung (Register NAK-345 Punkt 2, Haertung H-2).

void produktverdrahtungTraegtDenProduktnamen()
{
    abschnitt ("312/NAK-345 - der Produktkonstruktor verdrahtet den Produktnamen (Register NAK-345 Punkt 2)");

    // Der PRODUKTkonstruktor im Testbau: sein Zweig mit `controlV3.start()`
    // laeuft unter dem Testdefine nicht, es oeffnet sich also keine Pipe. Das
    // Bein liest nur, welchen Namen die Verdrahtung beiden Clients gegeben hat,
    // und startet nichts.
    auto p = std::make_unique<Sonde>();
    const auto erwartet = pipeNameV3 (aktuelleLogonSid());
    const auto& verdrahtet = p->v3PipeNameFuerTest();
    const auto control = p->controlV3FuerTest();
    const auto tele    = p->telemetryV3FuerTest();
    const bool gleich  = verdrahtet == erwartet;

    // Der Name selbst traegt die SID des Benutzers und steht deshalb nicht in
    // der Ausgabe - nur Vergleich, Laengen und Clientzustand.
    pruefe (gleich && ! erwartet.empty() && ! istProbePipename (verdrahtet)
                && control.status == ControlClient::Status::getrennt && control.verbindungsVersuche == 0
                && tele.status == TelemetryClient::Status::getrennt && tele.verbindungsVersuche == 0,
            "312/NAK-345 produktverdrahtung_traegt_den_produktnamen (Register NAK-345 Punkt 2): eine Sonde aus dem "
            "PRODUKTkonstruktor traegt im Testbau genau pipeNameV3 (aktuelleLogonSid()) - dieselbe Verdrahtung wie "
            "im Bundle -, und ohne Starthaken hat kein Client einen Versuch gemacht, keine Pipe ist geoeffnet",
            std::string ("gleich ") + (gleich ? "ja" : "NEIN") + ", Laenge verdrahtet "
                + std::to_string (verdrahtet.size()) + " / erwartet " + std::to_string (erwartet.size())
                + ", Probe-Namensraum " + (istProbePipename (verdrahtet) ? "JA" : "nein") + ", Control "
                + wort (control.status) + " nach " + std::to_string (control.verbindungsVersuche)
                + " Versuchen, Telemetrie " + wort (tele.status));
}

//==============================================================================
// NAK-312 Etappe 4 (T3-04-01, R-312-7, R-312-12): die Besitzschleuse der Sonde.
//
// SCHRANKE UND MARKE SIND ZWEI DINGE (Manifest §6.4). Die Schranke haelt den
// Statusprovider an - VOR `betreten()` (312/M-39 Fall 2, 312/M-31) oder
// HINTER `betreten()` (312/M-30, 312/M-39 Fall 1). Die Marke sitzt im Provider
// unmittelbar vor dem ersten Besitzerzugriff (dem Aufruf von `v3Status()`,
// dessen erste Zeile das Zustandsschloss nimmt): sie liest den Kanarienvogel
// und zaehlt bei `lebt == false` einen Zugriff NACH dem Ende des Eigentuemers,
// dann kehrt der Provider zurueck, ohne den Prozessor anzufassen. Der Beleg
// ist die gezaehlte Marke, nie ein Absturz - auch nicht im Lauf gegen den
// Stand ohne Schleuse und nicht unter einer Mutation.
//
// Geordnet wird ueber beobachtbare Zustaende, nie ueber die Wanduhr: die
// Schranke gibt frei, sobald die Schleuse geschlossen ist UND den gehaltenen
// Rueckruf als laufend fuehrt (der Destruktor wartet dann beweisbar auf ihn),
// oder sobald der Abbauende-Haken `lebt` auf false gesetzt hat, oder nach der
// Rueckkehr des Destruktors. Die 2-s-Stoppfrist der Clients laeuft nur, weil
// der Rueckruf ueber sie hinaus gehalten wird; 312/M-32 misst genau sie.

using Stand = CallbackSchleuse::Stand;

/** Der Kanarienvogel (Manifest §6.4): gehoert dem Bein und ueberlebt den
    Prozessor - die Haken halten ihn ueber `shared_ptr`, auch aus einer
    abgeloesten Client-Laufzeit heraus. `lebt` setzt der Abbauende-Haken des
    Destruktors auf false: am ENDE des Rumpfs, nach dem Schliessen der
    Schleuse und vor den Mitgliedern. Ein Zugriff, der waehrend des Wartens
    zulaessig laeuft, zaehlt deshalb nicht als "nach dem Ende". `uhr` ist eine
    logische Uhr fuer die Reihenfolge der Ereignisse. */
struct Kanarienvogel
{
    std::atomic<bool> lebt { true };
    std::atomic<int>  nachEndeErreicht { 0 };
    std::atomic<int>  uhr { 0 };
    std::atomic<int>  markeLebend { 0 };      ///< Stempel: Provider lebend an der Marke vorbei
    std::atomic<int>  lage { 0 };             ///< 0 aus, 1 vor betreten(), 2 hinter betreten()
    std::atomic<bool> gehalten { false };
    std::atomic<bool> frei { false };
    std::atomic<bool> schonGehalten { false };
    std::atomic<DWORD> clientThread { 0 };   ///< der Thread des gehaltenen Providers

    int stempel() noexcept { return uhr.fetch_add (1) + 1; }

    /** Die Schranke: haelt genau den ERSTEN Rueckruf an ihrer Lage, bis das Bein
        freigibt. Nur Atomics und kurzes Schlafen - kein Schloss, das der
        Prozessor kennt. */
    void halteEinmal (int ort)
    {
        if (lage.load() != ort || schonGehalten.exchange (true))
            return;
        clientThread.store (GetCurrentThreadId());
        gehalten.store (true);
        while (! frei.load())
            std::this_thread::sleep_for (std::chrono::milliseconds (1));
    }
    void freigeben() noexcept { frei.store (true); }
};

enum class Schranke { vorEintritt = 1, hinterEintritt = 2 };
enum class Freigabe { beimSchliessen, nachDemDestruktor, vorDemAbbau };

/** Was ein gehaltener Abbau misst. `amEnde` liest der Abbauende-Haken im
    Destruktor (der Prozessor lebt noch), `danach` das Bein nach der Rueckkehr
    und dem Ende der Client-Laufzeit - ueber seinen eigenen `shared_ptr`. */
struct Lauf
{
    bool aufgebaut = false, verbunden = false, gehalten = false, laufzeitWeg = false, threadBeendet = false;
    bool amEndeGelesen = false, schleuseUeberlebt = false;
    int  nachEnde = 0;
    Stand amEnde {}, danach {};
    std::uint64_t stopFristControl = 0, stopFristTelemetrie = 0;
    int stempelGeschlossen = 0, stempelFreigabe = 0, stempelMarke = 0, stempelEnde = 0, stempelRueckkehr = 0;
    long long destruktorUs = 0;

    // 312/M-33: der Reconnect vor dem Abbau
    bool reconnectGefahren = false, reconnectVerbunden = false, gleicheSchleuse = false;
    bool geschlossenVor = true, geschlossenNach = true;
    std::uint64_t betretenVor = 0, betretenNach = 0;
};

std::string standText (const Stand& s)
{
    return "geschlossen " + std::string (s.geschlossen ? "ja" : "nein") + ", aktiv " + std::to_string (s.aktiv)
         + ", betreten " + std::to_string (s.betreten) + ", abgewiesen " + std::to_string (s.abgewiesen)
         + ", gewartetMs " + std::to_string (s.gewartetMs);
}

/** Ein Abbau mit gehaltenem Statusprovider: Aufbau gegen einen eigenen
    Testserver, Verbindung ueber die Starthaken, Schranke scharf, Abbau,
    Freigabe nach `freigabe`, dann warten, bis die Client-Laufzeit den Haken
    losgelassen hat. `reconnectVorher` faehrt vor dem Scharfmachen einen
    Ladestart, der beide Clients neu verbinden laesst (312/M-33). */
Lauf gehaltenerAbbau (const char* fall, Schranke lage, Freigabe freigabe, bool reconnectVorher = false)
{
    Lauf l;
    const auto pipe = testPipeName (fall);
    // 312/M-08: die Wache vor Server und Start, hier still - der Fall meldet
    // einen Verstoss als nicht aufgebauten Lauf, und der Aufrufer prueft.
    if (! istProbePipename (pipe))
        return l;
    TestServer server (pipe);
    if (! server.starten())
        return l;

    auto p = std::make_unique<Sonde> (pipe, testExeErwartung());
    Sonde* const roh = p.get();
    binde (*p);
    p->setRateAndBufferSizeDetails (kFs, kBlock);
    p->prepareToPlay (kFs, kBlock);
    if (! istProbePipename (p->v3PipeNameFuerTest()))
        return l;
    l.aufgebaut = true;

    auto k = std::make_shared<Kanarienvogel>();
    Sonde::ProviderHakenFuerTest haken;
    haken.vorEintritt = [k] { k->halteEinmal ((int) Schranke::vorEintritt); };
    haken.imZug       = [k] { k->halteEinmal ((int) Schranke::hinterEintritt); };
    haken.vorBesitz   = [k]
    {
        if (! k->lebt.load())
        {
            k->nachEndeErreicht.fetch_add (1);
            return false;          // zurueck, ohne den Prozessor anzufassen
        }
        // Gestempelt wird der erste lebende Durchgang NACH der Freigabe - der
        // gehaltene Provider; Heartbeats davor zaehlen nicht.
        if (k->frei.load())
        {
            int erwartet = 0;
            k->markeLebend.compare_exchange_strong (erwartet, k->stempel());
        }
        return true;
    };
    p->setzeProviderHakenFuerTest (std::move (haken));
    p->setzeAbbauEndeHakenFuerTest ([k, roh, &l]
    {
        // Im Destruktor, nach dem Schliessen der Schleuse, vor den Mitgliedern.
        l.stopFristControl    = roh->controlV3FuerTest().stopFristUeberschritten;
        l.stopFristTelemetrie = roh->telemetryV3FuerTest().stopFristUeberschritten;
#if ! defined (NAK312_GEGENPROBE_OHNE_SCHLEUSE)
        l.amEnde = roh->callbackSchleuseFuerTest()->stand();
#endif
        l.amEndeGelesen = true;
        l.stempelEnde = k->stempel();
        k->lebt.store (false);
    });

    p->v3StartFuerTest();
    const bool control = warteAuf (kVerbindenMs, [&] {
        return p->controlV3FuerTest().status == ControlClient::Status::verbunden;
    });
    p->v3TelemetrieStartFuerTest();
    const bool tele = warteAuf (kVerbindenMs, [&] {
        return p->telemetryV3FuerTest().status == TelemetryClient::Status::verbunden;
    });
    l.verbunden = control && tele;

#if ! defined (NAK312_GEGENPROBE_OHNE_SCHLEUSE)
    if (reconnectVorher)
    {
        // 312/M-33: ein Ladestart mit gueltigem Stand ruft am Ende
        // `controlV3.reconnect()` und `telemetryV3.reconnect()`.
        const auto vorher = p->callbackSchleuseFuerTest();
        l.geschlossenVor = vorher->istGeschlossen();
        const int serverVorher = server.verbindungen.load();
        binde (*p);
        l.reconnectGefahren = true;
        l.betretenVor = vorher->stand().betreten;
        l.reconnectVerbunden = warteAuf (kVerbindenMs, [&] {
            return server.verbindungen.load() >= serverVorher + 2
                && p->controlV3FuerTest().status == ControlClient::Status::verbunden
                && p->telemetryV3FuerTest().status == TelemetryClient::Status::verbunden
                && p->callbackSchleuseFuerTest()->stand().betreten > l.betretenVor;
        });
        const auto nachher = p->callbackSchleuseFuerTest();
        l.gleicheSchleuse = nachher.get() == vorher.get();
        l.geschlossenNach = nachher->istGeschlossen();
        l.betretenNach    = nachher->stand().betreten;
    }
    // Die Schleuse, die der Destruktor schliesst - gelesen nach einem
    // etwaigen Reconnect, ueber den eigenen `shared_ptr` des Beins.
    const auto schleuse = p->callbackSchleuseFuerTest();
#endif

    k->lage.store ((int) lage);
    l.gehalten = warteAuf (kVerbindenMs, [&] { return k->gehalten.load(); });
    if (! l.gehalten)
        k->freigeben();

    std::thread freigeber;
    if (l.gehalten && freigabe == Freigabe::vorDemAbbau)
    {
        l.stempelFreigabe = k->stempel();
        k->freigeben();
        warteAuf (kVerbindenMs, [&] { return k->markeLebend.load() != 0; });
    }
    else if (l.gehalten && freigabe == Freigabe::beimSchliessen)
    {
        freigeber = std::thread ([&l, k
#if ! defined (NAK312_GEGENPROBE_OHNE_SCHLEUSE)
                                  , schleuse
#endif
                                 ]
        {
            // Obergrenze nur gegen einen haengenden Lauf, keine Zusage.
            for (int i = 0; i < 20000; ++i)
            {
#if ! defined (NAK312_GEGENPROBE_OHNE_SCHLEUSE)
                const auto s = schleuse->stand();
                if (s.geschlossen && s.aktiv >= 1)
                {
                    l.stempelGeschlossen = k->stempel();
                    break;
                }
#endif
                if (! k->lebt.load())
                    break;
                std::this_thread::sleep_for (std::chrono::milliseconds (1));
            }
            l.stempelFreigabe = k->stempel();
            k->freigeben();
        });
    }

    const auto t0 = Uhr::now();
    if (freigabe == Freigabe::nachDemDestruktor)
    {
        std::thread zerstoerer ([&p] { p.reset(); });
        zerstoerer.join();
        l.destruktorUs = std::chrono::duration_cast<std::chrono::microseconds> (Uhr::now() - t0).count();
        l.stempelRueckkehr = k->stempel();
        l.stempelFreigabe = k->stempel();
        k->freigeben();
    }
    else
    {
        p.reset();
        l.destruktorUs = std::chrono::duration_cast<std::chrono::microseconds> (Uhr::now() - t0).count();
        l.stempelRueckkehr = k->stempel();
    }
    if (freigeber.joinable())
        freigeber.join();
    k->freigeben();

    // Die Client-Laufzeit haelt die Haken ueber ihre `std::function`; erst
    // wenn sie endet, gehoert der Kanarienvogel wieder allein dem Bein.
    l.laufzeitWeg = warteAuf (10000, [&] { return k.use_count() == 1; });
    // Und der abgeloeste Thread selbst ist beendet (nicht nur sein Callable):
    // erst dann zaehlt 312/M-31 Threads und Handles.
    if (const DWORD tid = k->clientThread.load(); tid != 0)
    {
        if (HANDLE h = OpenThread (SYNCHRONIZE, FALSE, tid); h != nullptr)
        {
            l.threadBeendet = WaitForSingleObject (h, 10000) == WAIT_OBJECT_0;
            CloseHandle (h);
        }
        else
        {
            l.threadBeendet = true;   // schon fort
        }
    }
    l.stempelMarke = k->markeLebend.load();
    l.nachEnde = k->nachEndeErreicht.load();
#if ! defined (NAK312_GEGENPROBE_OHNE_SCHLEUSE)
    // Die Schleuse ueberlebt den Prozessor nur, wenn er sie ueber `shared_ptr`
    // haelt; ohne Besitz liest das Bein sie nach dem Abbau nicht mehr.
    l.schleuseUeberlebt = schleuse.use_count() > 0;
    if (l.schleuseUeberlebt)
        l.danach = schleuse->stand();
#endif
    server.stoppen();
    return l;
}

std::string laufText (const Lauf& l)
{
    return "aufgebaut " + std::string (l.aufgebaut ? "ja" : "NEIN") + ", verbunden " + (l.verbunden ? "ja" : "NEIN")
         + ", gehalten " + (l.gehalten ? "ja" : "NEIN") + ", nachEndeErreicht " + std::to_string (l.nachEnde)
         + "; im Destruktor: " + standText (l.amEnde) + "; danach: " + standText (l.danach)
         + "; stopFristUeberschritten Control " + std::to_string (l.stopFristControl) + ", Telemetrie "
         + std::to_string (l.stopFristTelemetrie) + "; Destruktor " + std::to_string (l.destruktorUs / 1000)
         + " ms; Stempel geschlossen " + std::to_string (l.stempelGeschlossen) + ", Freigabe "
         + std::to_string (l.stempelFreigabe) + ", Marke lebend " + std::to_string (l.stempelMarke) + ", Abbauende "
         + std::to_string (l.stempelEnde) + ", Rueckkehr " + std::to_string (l.stempelRueckkehr)
         + "; Laufzeit beendet " + (l.laufzeitWeg ? "ja" : "NEIN");
}

bool vor (int a, int b) noexcept { return a > 0 && b > 0 && a < b; }

//==============================================================================
// 312/M-30, 312/M-32, 312/M-34 und 312/M-39 Fall 1 - EIN Lauf, vier Zusagen.

void providerNachDemSchliessen()
{
    abschnitt ("312/M-30, 312/M-32, 312/M-34 - gehaltener Provider, Abbau ueber die Stoppfrist (R-312-7, R-D2)");

    const auto l = gehaltenerAbbau ("nak312-m30", Schranke::hinterEintritt, Freigabe::beimSchliessen);
    const auto d = laufText (l);

    pruefe (l.aufgebaut && l.verbunden && l.gehalten && l.amEndeGelesen && l.laufzeitWeg && l.nachEnde == 0
                && l.amEnde.gewartetMs >= 1 && l.amEnde.abgewiesen == 0 && l.amEnde.aktiv == 0
                && vor (l.stempelFreigabe, l.stempelMarke) && vor (l.stempelMarke, l.stempelEnde)
                && vor (l.stempelEnde, l.stempelRueckkehr),
            "312/M-30 provider_fasst_nach_dem_schliessen_nichts_mehr_an (R-312-7, R-312-12): der Statusprovider steht "
            "HINTER betreten() in der Schranke, die Sonde wird zerstoert und die Schranke erst freigegeben, als die "
            "Schleuse geschlossen ist - nach dem Ende des Eigentuemers erreicht kein Provider den Besitzerzugriff "
            "(nachEndeErreicht 0), die Schleuse hat gewartet (gewartetMs >= 1) und nichts abgewiesen, und der "
            "Destruktor ist erst nach dem Provider zurueckgekehrt",
            d);

    pruefe (l.aufgebaut && l.gehalten && l.stopFristControl == 1 && l.stopFristTelemetrie == 0
                && l.destruktorUs >= 2000LL * 1000LL
                && (long long) l.amEnde.gewartetMs * 1000LL <= l.destruktorUs - 2000LL * 1000LL + 1000LL,
            "312/M-32 stoppfrist_loest_ab_und_der_destruktor_wartet_nur_den_rest (Teilfall von 312/M-30, R-D2 Satz 4): "
            "stop() des gehaltenen ControlClients kehrt nach der 2-s-Frist zurueck, stopFristUeberschritten steigt "
            "genau um 1 (die Telemetrie um 0), der Thread ist abgeloest; danach wartet der Destruktor hoechstens "
            "die Restdauer des laufenden Providers - gewartetMs liegt innerhalb der Destruktordauer jenseits der Frist",
            d);

    pruefe (l.aufgebaut && l.gehalten && l.stopFristControl == 1 && l.amEnde.geschlossen && l.amEnde.aktiv == 0
                && l.amEnde.gewartetMs >= 1 && l.amEnde.abgewiesen == 0
                && vor (l.stempelGeschlossen, l.stempelMarke) && vor (l.stempelMarke, l.stempelEnde),
            "312/M-34 destruktor_stoppt_vor_dem_schliessen (Teilfall von 312/M-30, CLAUDE.md starten<->stoppen): "
            "beide stop() laufen VOR dem Schliessen - der gehaltene Provider war beim Schliessen schon von stop() "
            "abgeloest (Frist 1) und wurde danach zu Ende GEWARTET statt abgewiesen -, und erst nach dem Schliessen "
            "enden die Mitglieder: der Abbauende-Haken sieht die Schleuse zu und ohne laufenden Rueckruf",
            d);

    pruefe (l.aufgebaut && l.gehalten && l.nachEnde == 0 && l.amEnde.aktiv == 0
                && l.amEnde.abgewiesen == 0 && l.amEnde.gewartetMs >= 1,
            "312/M-39 Fall 1 schleuse_ist_ueber_zustaende_geordnet (E-312-10, R-312-12): Schranke HINTER betreten(), "
            "Freigabe sobald die Schleuse geschlossen ist und den Provider als laufend fuehrt - "
            "(abgewiesen, gewartetMs) = (0, >= 1), nachEndeErreicht 0, und der Destruktor kehrt nicht zurueck, "
            "waehrend aktiv > 0 ist",
            d);
}

//==============================================================================
// 312/M-39 Fall 2 und Fall 3.

void schleuseIstUeberZustaendeGeordnet()
{
    abschnitt ("312/M-39 - die Schleuse ist ueber Zustaende geordnet, nie ueber die Wanduhr (E-312-10, R-312-12)");

    const auto zwei = gehaltenerAbbau ("nak312-m39-2", Schranke::vorEintritt, Freigabe::nachDemDestruktor);
    pruefe (zwei.aufgebaut && zwei.verbunden && zwei.gehalten && zwei.amEndeGelesen && zwei.laufzeitWeg
                && zwei.nachEnde == 0 && zwei.amEnde.aktiv == 0 && zwei.amEnde.gewartetMs == 0
                && zwei.schleuseUeberlebt && zwei.danach.abgewiesen >= 1 && zwei.danach.gewartetMs == 0
                && zwei.danach.aktiv == 0 && vor (zwei.stempelRueckkehr, zwei.stempelFreigabe),
            "312/M-39 Fall 2 (E-312-10, R-312-12): Schranke VOR betreten(), Freigabe nach der Rueckkehr des "
            "Destruktors (Join des zerstoerenden Threads) - der Provider wird abgewiesen: (abgewiesen, gewartetMs) "
            "= (>= 1, 0), nachEndeErreicht 0, der Destruktor kehrt nicht zurueck, waehrend aktiv > 0 ist",
            laufText (zwei));

    const auto drei = gehaltenerAbbau ("nak312-m39-3", Schranke::hinterEintritt, Freigabe::vorDemAbbau);
    pruefe (drei.aufgebaut && drei.verbunden && drei.gehalten && drei.amEndeGelesen && drei.laufzeitWeg
                && drei.nachEnde == 0 && drei.stempelMarke > 0 && drei.amEnde.aktiv == 0
                && drei.amEnde.abgewiesen == 0 && drei.amEnde.gewartetMs == 0
                && drei.schleuseUeberlebt && drei.danach.abgewiesen == 0 && drei.danach.gewartetMs == 0,
            "312/M-39 Fall 3 (E-312-10): Freigabe VOR Beginn der Zerstoerung - der Provider laeuft lebend durch, "
            "(abgewiesen, gewartetMs) = (0, 0) ist korrektes Verhalten, nachEndeErreicht 0",
            laufText (drei));
}

//==============================================================================
// 312/M-33 - ein Reconnect ruehrt die Schleuse nicht an.

void reconnectRuehrtDieSchleuseNichtAn()
{
    abschnitt ("312/M-33 - ein Reconnect ruehrt die Schleuse nicht an (R-312-7, verbinden<->trennen)");

    const auto l = gehaltenerAbbau ("nak312-m33", Schranke::hinterEintritt, Freigabe::beimSchliessen, true);
    pruefe (l.aufgebaut && l.verbunden && l.reconnectGefahren && l.reconnectVerbunden && l.gleicheSchleuse
                && ! l.geschlossenVor && ! l.geschlossenNach && l.betretenNach > l.betretenVor
                && l.gehalten && l.nachEnde == 0 && l.amEnde.gewartetMs >= 1 && l.amEnde.abgewiesen == 0,
            "312/M-33 reconnect_ruehrt_die_schleuse_nicht_an (R-312-7, CLAUDE.md verbinden<->trennen): ein Ladestart "
            "mit gueltigem Stand laesst beide Clients neu verbinden - die Schleuse ist dieselbe, istGeschlossen() "
            "ist vor und nach dem Reconnect falsch, betreten steigt weiter, und der Abbau danach schliesst genau "
            "die Schleuse, durch die die Provider laufen (nachEndeErreicht 0, gewartet statt abgewiesen)",
            "Reconnect verbunden " + std::string (l.reconnectVerbunden ? "ja" : "NEIN") + ", dieselbe Schleuse "
                + (l.gleicheSchleuse ? "ja" : "NEIN") + ", geschlossen vor/nach " + (l.geschlossenVor ? "ja" : "nein")
                + "/" + (l.geschlossenNach ? "ja" : "nein") + ", betreten " + std::to_string (l.betretenVor)
                + " -> " + std::to_string (l.betretenNach) + "; " + laufText (l));
}

//==============================================================================
// 312/M-31 - zwanzig Gegenzyklen.

int threadsDesProzesses()
{
    const DWORD pid = GetCurrentProcessId();
    HANDLE schnapp = CreateToolhelp32Snapshot (TH32CS_SNAPTHREAD, 0);
    if (schnapp == INVALID_HANDLE_VALUE)
        return -1;
    THREADENTRY32 t {};
    t.dwSize = sizeof (t);
    int n = 0;
    if (Thread32First (schnapp, &t))
    {
        do
        {
            if (t.th32OwnerProcessID == pid)
                ++n;
        } while (Thread32Next (schnapp, &t));
    }
    CloseHandle (schnapp);
    return n;
}

long handlesDesProzesses()
{
    DWORD n = 0;
    return GetProcessHandleCount (GetCurrentProcess(), &n) ? (long) n : -1L;
}

void zwanzigGegenzyklen()
{
    abschnitt ("312/M-31 - zwanzig Gegenzyklen aus Aufbau, gehaltenem Provider, Abbau und Freigabe (R-312-7, R-312-12)");

    constexpr int kZyklen = 20;
    int nachEndeSumme = 0, zyklenMitMarke = 0, nichtGemessen = 0;
    int threadsErst = -1, threadsLetzt = -1;
    long handlesErst = -1, handlesLetzt = -1;
    for (int z = 0; z < kZyklen; ++z)
    {
        // Schranke VOR betreten(), Freigabe nach dem Destruktor: der Provider
        // beginnt NACH dem Ende des Eigentuemers - genau der Rueckruf, den eine
        // Schleuse ohne Besitz nicht mehr schuetzt (Manifest, Abweichung A-40).
        const auto l = gehaltenerAbbau ("nak312-m31", Schranke::vorEintritt, Freigabe::nachDemDestruktor);
        if (! (l.aufgebaut && l.verbunden && l.gehalten && l.laufzeitWeg && l.threadBeendet && l.amEndeGelesen))
            ++nichtGemessen;
        nachEndeSumme += l.nachEnde;
        if (l.nachEnde > 0)
            ++zyklenMitMarke;
        if (z == 0)
        {
            threadsErst = threadsDesProzesses();
            handlesErst = handlesDesProzesses();
        }
        if (z == kZyklen - 1)
        {
            threadsLetzt = threadsDesProzesses();
            handlesLetzt = handlesDesProzesses();
        }
    }
    pruefe (nichtGemessen == 0 && nachEndeSumme == 0 && threadsErst > 0 && handlesErst > 0
                && threadsLetzt <= threadsErst && handlesLetzt <= handlesErst,
            "312/M-31 zwanzig_gegenzyklen_ohne_zugriff_nach_dem_ende (Teilfall von 312/M-30, R-312-12): in jedem von "
            "20 Zyklen - je eigene Sonde, eigene Schleuse, eigener Testserver - ist nachEndeErreicht 0, und nach dem "
            "letzten Zyklus stehen Handles und Threads des Prozesses nicht hoeher als nach dem ersten",
            std::to_string (kZyklen) + " Zyklen, nicht vollstaendig gemessen " + std::to_string (nichtGemessen)
                + ", nachEndeErreicht Summe " + std::to_string (nachEndeSumme) + " in " + std::to_string (zyklenMitMarke)
                + " Zyklen; Threads nach Zyklus 1 / 20: " + std::to_string (threadsErst) + " / "
                + std::to_string (threadsLetzt) + ", Handles " + std::to_string (handlesErst) + " / "
                + std::to_string (handlesLetzt));
}

} // namespace

int main (int argc, char* argv[])
{
    std::cout << "== Nakama NAK-312 - Sonde-Lebenslaufbein (EqCopSondeLebenslaufTest) ==" << std::endl;
    std::cout << "Gate: Manifest NAK-312 §6.1 (312/M-05 bis 312/M-08) und §6.4 (312/M-30 bis 312/M-34, "
                 "312/M-39), R-312-7, R-312-12; Register NAK-345 Punkt 2." << std::endl;

    // Ohne Argument alles (Kanon); mit Argument nur die Faelle, deren Kennung
    // es enthaelt.
    const std::string nur = argc > 1 ? std::string (argv[1]) : std::string();
    const auto faehrt = [&nur] (const char* kennung)
    {
        return nur.empty() || std::string (kennung).find (nur) != std::string::npos;
    };
    if (! nur.empty())
        std::cout << "Auswahl: nur Faelle mit '" << nur << "'" << std::endl;

    if (faehrt ("312/M-05"))
        testkonstruktorIstFailClosed();
    if (faehrt ("312/M-06 312/M-07 312/M-08"))
        starthakenUndAbbau();
    if (faehrt ("312/NAK-345"))
        produktverdrahtungTraegtDenProduktnamen();
    if (faehrt ("312/M-30 312/M-32 312/M-34 312/M-39"))
        providerNachDemSchliessen();
    if (faehrt ("312/M-39"))
        schleuseIstUeberZustaendeGeordnet();
    if (faehrt ("312/M-33"))
        reconnectRuehrtDieSchleuseNichtAn();
    if (faehrt ("312/M-31"))
        zwanzigGegenzyklen();

    std::cout << std::endl << geprueft << " geprueft, " << fehler << " Fehler" << std::endl;
    const bool ok = fehler == 0 && geprueft > 0;
    std::cout << (ok ? "SONDE-LEBENSLAUF OK" : "SONDE-LEBENSLAUF FEHLGESCHLAGEN") << std::endl;
    return ok ? 0 : 1;
}
