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
    Den gehaltenen Provider ueber die Stoppfrist und die Marke nach dem Ende
    des Eigentuemers misst dieses Bein erst mit Etappe 4 (312/M-30 ff.).

    PRODUKTIONS-PIPE: nie. Jeder Name, mit dem ein Client starten soll, geht
    VOR dem Aufbau des Servers und VOR jedem Start durch `istProbePipename`;
    faellt die Wache, bricht der Fall ab, bevor irgendetwas eine Pipe oeffnet.
    Der fail-closed-Fall von 312/M-05 uebergibt absichtlich einen Namen
    ausserhalb des Namensraums und startet keinen Client.

    LANDMINE NAK-175: Prozessoren liegen auf dem HEAP (`std::unique_ptr`), nie
    im Rahmen.

    Exit 0 nur bei "SONDE-LEBENSLAUF OK".
*/

#include "SondeProcessor.h"
#include "IpcVerbindung.h"
#include "PipeToken.h"

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

} // namespace

int main()
{
    std::cout << "== Nakama NAK-312 - Sonde-Lebenslaufbein (EqCopSondeLebenslaufTest) ==" << std::endl;
    std::cout << "Gate: Manifest NAK-312 §6.1 (312/M-05 bis 312/M-08), R-312-7." << std::endl;

    testkonstruktorIstFailClosed();
    starthakenUndAbbau();

    std::cout << std::endl << geprueft << " geprueft, " << fehler << " Fehler" << std::endl;
    const bool ok = fehler == 0 && geprueft > 0;
    std::cout << (ok ? "SONDE-LEBENSLAUF OK" : "SONDE-LEBENSLAUF FEHLGESCHLAGEN") << std::endl;
    return ok ? 0 : 1;
}
