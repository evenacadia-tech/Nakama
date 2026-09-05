// S14-15 / SONDE-010 — das C++-Bein des v3-Nachrichtenwegs (Kanon B10).
//
// Zwei Ebenen in EINEM Ziel, wie bei B4 und B8:
//
//   1. Envelope, Pipetoken und die drei Backpressure-Politiken PUR — dort sind
//      Grenzen, CRC-Fehler und Ueberlauf deterministisch herbeifuehrbar.
//   2. Die ECHTEN Clients (`ControlClient`, `TelemetryClient`) gegen einen
//      selbst gehosteten v3-Server auf einer Testpipe: Bootstrap, Kopplung,
//      Reconnect nach Serverneustart, Envelopeverstoss und P0-Ueberlauf.
//
// Ohne Ebene 2 waere es ein huebscher Parser, der im Produkt nichts verbindet.
// Die Testpipe traegt IMMER PID und Zaehler im Namen; dieses Bein beruehrt
// weder die Produktions- noch die v3-SID-Pipe.
#include "ControlClient.h"
#include "BrokerLifecycle.h"
#include "IpcQueues.h"
#include "IpcVerbindung.h"
#include "PipeToken.h"
#include "TelemetryClient.h"
#include "WireEnvelope.h"
#include "../core/analysis/FeatureEngine.h"
#include "../vertrag/NakamaVertrag.h"

#include <juce_core/juce_core.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
 #define NOMINMAX
#endif
#include <windows.h>

using namespace nakama::ipc;

//==============================================================================
namespace
{
    thread_local bool          zaehleAllokationen = false;
    thread_local std::uint64_t allokationen       = 0;
}

void* operator new (std::size_t groesse)
{
    if (zaehleAllokationen) ++allokationen;
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete (void* p) noexcept { std::free (p); }
void operator delete (void* p, std::size_t) noexcept { std::free (p); }
void* operator new[] (std::size_t groesse)
{
    if (zaehleAllokationen) ++allokationen;
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete[] (void* p) noexcept { std::free (p); }
void operator delete[] (void* p, std::size_t) noexcept { std::free (p); }

//==============================================================================
namespace
{
int fehler = 0, geprueft = 0;

void pruefe (bool ok, const std::string& was, const std::string& detail = {})
{
    ++geprueft;
    std::cout << (ok ? "  ok      " : "  FEHLER  ") << was;
    if (! detail.empty())
        std::cout << "  [" << detail << "]";
    std::cout << std::endl;
    if (! ok)
        ++fehler;
}

void abschnitt (const char* text)
{
    std::cout << "\n== " << text << " ==" << std::endl;
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

std::string hex32 (char fuellzeichen) { return std::string (32, fuellzeichen); }

std::string commandIdAusJson (const std::string& text)
{
    constexpr const char* marker = "\"command_id\":\"";
    const auto anfang = text.find (marker);
    if (anfang == std::string::npos)
        return {};
    const auto wert = anfang + std::char_traits<char>::length (marker);
    if (wert + 32 >= text.size() || text[wert + 32] != '"')
        return {};
    const auto id = text.substr (wert, 32);
    return istHex32 (id) ? id : std::string {};
}

/*  SONDE-013 E-02: das Ergebnis eines `manual_external`-Versuchs als
    steuernde P0-Nachricht.

    Sie traegt denselben `steuerkopf` wie jede andere - genau darum braucht
    der C++-Client fuer die drei neuen Familien KEINE Zeile Sonderbehandlung:
    das In-Flight-Register haengt an der `command_id`, nicht am Familiennamen.
    Der Test unten misst das, statt es anzunehmen.  */
std::string experimentErgebnisBefehl (const std::string& commandId)
{
    return "{\"type\":\"experiment_manual_result\",\"kopf\":{\"command_id\":\""
         + commandId
         + "\",\"ziel\":{\"logon_sid\":\"S-1-5-21-1-2-3-1001\","
           "\"project_binding_id\":\"00000000000000000000000000000000\","
           "\"session_epoch\":\"11111111111111111111111111111111\","
           "\"instance_id\":\"22222222222222222222222222222222\","
           "\"runtime_nonce\":\"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\"},"
           "\"base_revision\":0,\"ttl_ms\":10000,\"schema_major\":3,"
           "\"schema_minor\":2},"
           "\"experiment_id\":\"abababababababababababababababab\","
           "\"hoerurteil\":\"kandidat\","
           "\"blindreihenfolge\":\"baseline_zuerst\","
           "\"notiz\":\"Saettigung leicht erhoeht\","
           "\"werkzeug\":null}";
}

std::string persistenzBefehl (const std::string& commandId)
{
    return "{\"type\":\"preview_begin\",\"kopf\":{\"command_id\":\""
         + commandId
         + "\",\"ziel\":{\"logon_sid\":\"S-1-5-21-1-2-3-1001\","
           "\"project_binding_id\":\"00000000000000000000000000000000\","
           "\"session_epoch\":\"11111111111111111111111111111111\","
           "\"instance_id\":\"22222222222222222222222222222222\","
           "\"runtime_nonce\":\"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\"},"
           "\"base_revision\":0,\"ttl_ms\":10000,\"schema_major\":3,"
           "\"schema_minor\":0},\"lease_duration_ms\":400,"
           "\"renew_id\":\"33333333333333333333333333333333\"}";
}

Adresse testAdresse (const std::string& nonce)
{
    Adresse a;
    a.logonSid = "S-1-5-21-1-2-3-1001";
    a.projectBindingId = hex32 ('0');
    a.sessionEpoch = hex32 ('1');
    a.instanceId = hex32 ('2');
    a.runtimeNonce = nonce;
    return a;
}

//== Deterministischer Zufall (xorshift64*), wie im Rust-Fuzzbein ============
struct Zufall
{
    std::uint64_t z;
    explicit Zufall (std::uint64_t saat) : z (saat | 1u) {}
    std::uint64_t naechste()
    {
        z ^= z >> 12; z ^= z << 25; z ^= z >> 27;
        return z * 0x2545F4914F6CDD1Dull;
    }
    std::uint64_t bis (std::uint64_t n) { return n == 0 ? 0 : naechste() % n; }
};

//== Verstossmenge als sortierte Namensliste ================================
std::vector<std::string> namen (VerstossMenge m)
{
    std::vector<std::string> aus;
    for (int i = 0; i < kVerstossAnzahl; ++i)
        if (enthaelt (m, static_cast<Verstoss> (i)))
            aus.emplace_back (verstossName (static_cast<Verstoss> (i)));
    return aus;
}

std::string alsText (const std::vector<std::string>& v)
{
    std::string s;
    for (const auto& e : v) { if (! s.empty()) s += ","; s += e; }
    return s.empty() ? "(leer)" : s;
}

//==============================================================================
// Ein v3-Server auf einer eigenen Testpipe. Er spricht genau den Bootstrap aus
// §53.9 und ist absichtlich klein: er ist der GEGENUEBER des Clients, nicht
// eine zweite Brokerimplementierung.
class TestServer
{
public:
    std::atomic<int> verbindungen { 0 };
    std::atomic<int> p0 { 0 }, p1 { 0 }, p2 { 0 };
    std::atomic<int> abgewiesen { 0 };
    std::atomic<bool> sendeKaputtenFrame { false };
    /// Der Server macht den Bootstrap, schickt sein welcome und liest DANACH
    /// nie wieder. So laeuft der Sendeweg des Clients garantiert voll — ohne
    /// diesen Gegenspieler waere ein P0-Ueberlauf im Betrieb ein Rennen.
    std::atomic<bool> nichtLesen { false };
    /// NAK-134/D-K04, D-K05: nach bestandener Serverpruefung und gelesenem
    /// Hello GAR KEIN welcome schicken. `eineVerbindung` laeuft in seine
    /// Lesefrist und gibt `false` — der Auth-Erfolg allein setzt den Backoff
    /// also nicht zurueck (D-P02).
    std::atomic<bool> welcomeAusbleiben { false };
    /// Welcome-Varianten fuer die Strenge des TelemetryClient (T2-Befund 10):
    /// als P2-Familie statt P0, mit fremder link_id, ohne `protocol`.
    std::atomic<bool> welcomeAlsP2 { false };
    std::atomic<bool> welcomeFremdeLinkId { false };
    std::atomic<bool> welcomeOhneProtokoll { false };
    /// Welcome-Varianten fuer den VERTRAG (T2-Befund 3): typfalsches
    /// `broker_version` (`null`), ein Zusatzfeld trotz
    /// `additionalProperties:false`, und eine Version ueber der Laengengrenze.
    std::atomic<bool> welcomeTypfalsch { false };
    std::atomic<bool> welcomeZusatzfeld { false };
    std::atomic<bool> welcomeLangeVersion { false };
    /// Nach dem welcome einen korrekt gerahmten P2-Frame ueber die CONTROL-
    /// Verbindung schicken (T2-Befund 4).
    std::atomic<bool> sendeP2AufControl { false };
    /// So viele P0-Frames nach dem welcome so schnell wie moeglich
    /// hinterherschicken (T2-Befund 5).
    std::atomic<int> frameFlut { 0 };
    /// Dasselbe auf der TELEMETRIE-Verbindung, aber mit P2-Frames: dort ist
    /// P2 die vertragsgemaesse Familie, und geprueft wird die Rate.
    std::atomic<int> frameFlutTelemetrieP2 { 0 };
    /// Ein einzelner P0-Frame auf der Telemetrieverbindung — dort
    /// vertragswidrig (§33.1), der Client muss schliessen.
    std::atomic<bool> sendeP0AufTelemetrie { false };
    /// Die naechste Telemetrieverbindung einmalig schliessen — der Fall, den
    /// der Leerlauf ohne Lesen nie bemerkte (T2-Befund 2).
    std::atomic<bool> telemetrieSchliessen { false };
    /// Nach der ERSTEN P0-Antwort nicht mehr lesen (Matrix `B-CC-06`): der
    /// ACK liegt dann beim Client, waehrend dessen P1-Weg vollaeuft. Genau in
    /// dieser Lage uebersprang die alte Fassung den Lesepfad.
    std::atomic<bool> nachErsterP0AntwortNichtLesen { false };
    /// Zusammen mit `nichtLesen`: so viele Millisekunden nach dem welcome
    /// einen UNGEFRAGTEN P0-ACK schicken (Matrix `B-CC-07`, NAK-104). Zu
    /// diesem Zeitpunkt steht der Client bereits in seinem Write, weil der
    /// Peer nicht liest — der ACK liegt also beim Client, WAEHREND dessen
    /// Schreibfrist laeuft. Nur so wird der Fehlerpfad nach dem Zeitlimit
    /// wirklich gefahren; `nachErsterP0AntwortNichtLesen` liefert den ACK
    /// dagegen nach einem ERFOLGREICHEN Write.
    std::atomic<int> ackNachNichtLesenMs { 0 };
    /// Phase-B-Antworten auf persistenzpflichtige Befehle. 0 = keine,
    /// 1..5 entsprechen der schemafesten Reihenfolge angewandt, abgelehnt,
    /// konflikt, abgelaufen, idempotent_wiederholt.
    std::atomic<int> commandAckArt { 0 };
    std::atomic<int> commandAckVerzoegerungMs { 0 };
    std::atomic<bool> commandVorAckSchliessen { false };
    std::atomic<bool> commandAckMitEventUuid { false };
    /// 1 = Revision mit fuehrender Null, 2 = numerischer Fehler-state_hash,
    /// 3 = unbekannter Fehlercode, 4 = Erfolgs-ACK ohne Pflicht-Hash.
    std::atomic<int> commandAckVertragsbruch { 0 };
    /// Minor fuer nach dem Welcome erzeugte P0-Antworten. Default 0 haelt
    /// die Rueckwaertskompatibilitaet sichtbar; einzelne Tests setzen 1 bzw.
    /// einen unbekannten Wert.
    std::atomic<int> controlAntwortMinor { 0 };
    std::mutex textMutex;
    std::string letztesControlHello, letztesTelemetryHello, letzterAbweisungsgrund;
    /// Jeder empfangene P0-/P1-Payload, woertlich. Damit laesst sich pruefen,
    /// ob eine bestimmte Nachricht WIRKLICH angekommen ist — eine Zahl allein
    /// sagt nichts darueber, WELCHE fehlt.
    std::vector<std::string> p0Texte, p1Texte;
    std::vector<std::uint8_t> p0Minors, p1Minors;
    /// Vollstaendiger zuletzt empfangener Telemetrie-Wireframe inklusive
    /// u32-Laengenpraefix. Damit misst der Sender-Test schema_minor an Offset
    /// 7 des tatsaechlich geschriebenen Rahmens.
    std::vector<std::uint8_t> letzterTelemetryWire;

    /// Kopplungswerte. Sie sind veraenderlich: eine neu aufgebaute
    /// Control-Verbindung bekommt beim echten Broker eine frische `link_id`,
    /// und genau das muss der TelemetryClient bemerken (T2-Befund 2).
    std::string kopplungLinkId()
    {
        std::lock_guard<std::mutex> l (kopplungMutex);
        return linkId;
    }
    std::string kopplungChallenge()
    {
        std::lock_guard<std::mutex> l (kopplungMutex);
        return challenge;
    }
    void neueKopplung (std::string l2, std::string c2)
    {
        std::lock_guard<std::mutex> l (kopplungMutex);
        linkId = std::move (l2);
        challenge = std::move (c2);
    }

    std::mutex kopplungMutex;
    std::string linkId = hex32 ('a'), challenge = hex32 ('b');

    /// Startsynchronisation: `pipeSteht` wird erst gesetzt, wenn der Acceptor
    /// seine erste Instanz angelegt hat; `startGescheitert`, wenn er es nicht
    /// konnte (Name schon belegt).
    std::atomic<bool> pipeSteht { false }, startGescheitert { false };

    explicit TestServer (std::string pipeName) : name (std::move (pipeName)) {}
    ~TestServer() { stoppen(); }

    /// Kehrt erst zurueck, wenn die erste Pipeinstanz WIRKLICH existiert.
    /// Vorher war das ein Rennen: `acceptorLauf` legt sie im Thread an,
    /// waehrend der Aufrufer schon `namedPipeErreichbar` fragt. Fuer die
    /// Tests, die ueber einen `ControlClient` mit Warteschleife verbinden,
    /// blieb das folgenlos; die NAK-123-C-07-Tests ticken den Lifecycle
    /// dagegen sofort einmal an und sahen den Namen mal, mal nicht.
    bool starten()
    {
        laeuft.store (true);
        pipeSteht.store (false);
        startGescheitert.store (false);
        acceptor = std::thread ([this] { acceptorLauf(); });
        for (int i = 0; i < 1000; ++i)
        {
            if (pipeSteht.load() || startGescheitert.load())
                break;
            std::this_thread::sleep_for (std::chrono::milliseconds (2));
        }
        return pipeSteht.load();
    }

    void stoppen()
    {
        if (! laeuft.exchange (false))
        {
            if (acceptor.joinable()) acceptor.join();
            return;
        }
        wecken();
        if (acceptor.joinable())
            acceptor.join();
        {
            std::lock_guard<std::mutex> l (threadMutex);
            for (auto& t : verbindungsThreads)
                if (t.joinable())
                    t.join();
            verbindungsThreads.clear();
        }
    }

    const std::string& pipeName() const { return name; }

private:
    void wecken()
    {
        std::wstring breit (name.begin(), name.end());
        HANDLE h = CreateFileW (breit.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                                SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION, nullptr);
        if (h != INVALID_HANDLE_VALUE)
            CloseHandle (h);
    }

    void acceptorLauf()
    {
        std::wstring breit (name.begin(), name.end());
        bool erste = true;
        while (laeuft.load())
        {
            HANDLE h = CreateNamedPipeW (
                breit.c_str(),
                PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED | (erste ? FILE_FLAG_FIRST_PIPE_INSTANCE : 0),
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
                8, 65536, 65536, 0, nullptr);
            erste = false;
            if (h == INVALID_HANDLE_VALUE)
            {
                startGescheitert.store (true);
                return;
            }
            pipeSteht.store (true);

            OVERLAPPED ov {};
            ov.hEvent = CreateEventW (nullptr, TRUE, FALSE, nullptr);
            BOOL ok = ConnectNamedPipe (h, &ov);
            DWORD f = GetLastError();
            if (! ok && f == ERROR_IO_PENDING)
            {
                WaitForSingleObject (ov.hEvent, INFINITE);
                DWORD dummy = 0;
                ok = GetOverlappedResult (h, &ov, &dummy, FALSE);
                f = ok ? 0 : GetLastError();
            }
            CloseHandle (ov.hEvent);
            if (! ok && f != ERROR_PIPE_CONNECTED)
            {
                CloseHandle (h);
                continue;
            }
            if (! laeuft.load())
            {
                DisconnectNamedPipe (h);
                CloseHandle (h);
                return;
            }
            ++verbindungen;
            std::lock_guard<std::mutex> l (threadMutex);
            verbindungsThreads.emplace_back ([this, h] { verbindungLauf (h); });
        }
    }

    /// Ein Lesevorgang mit Frist auf einem overlapped Serverhandle.
    static int lesen (HANDLE h, std::uint8_t* ziel, DWORD n, int fristMs)
    {
        OVERLAPPED ov {};
        ov.hEvent = CreateEventW (nullptr, TRUE, FALSE, nullptr);
        DWORD gelesen = 0;
        int ergebnis = -1;
        if (ReadFile (h, ziel, n, nullptr, &ov))
        {
            GetOverlappedResult (h, &ov, &gelesen, FALSE);
            ergebnis = static_cast<int> (gelesen);
        }
        else if (GetLastError() == ERROR_IO_PENDING)
        {
            if (WaitForSingleObject (ov.hEvent, static_cast<DWORD> (fristMs)) == WAIT_OBJECT_0
                && GetOverlappedResult (h, &ov, &gelesen, FALSE))
                ergebnis = static_cast<int> (gelesen);
            else
            {
                CancelIoEx (h, &ov);
                DWORD verworfen = 0;
                GetOverlappedResult (h, &ov, &verworfen, TRUE);
                ergebnis = verworfen > 0 ? static_cast<int> (verworfen) : 0;  // 0 = nichts
            }
        }
        CloseHandle (ov.hEvent);
        return ergebnis;
    }

    static bool schreiben (HANDLE h, const std::uint8_t* daten, std::size_t n)
    {
        OVERLAPPED ov {};
        ov.hEvent = CreateEventW (nullptr, TRUE, FALSE, nullptr);
        DWORD geschrieben = 0;
        bool ok = false;
        if (WriteFile (h, daten, static_cast<DWORD> (n), nullptr, &ov))
            ok = GetOverlappedResult (h, &ov, &geschrieben, FALSE) != 0;
        else if (GetLastError() == ERROR_IO_PENDING)
            ok = WaitForSingleObject (ov.hEvent, 5000) == WAIT_OBJECT_0
              && GetOverlappedResult (h, &ov, &geschrieben, FALSE) != 0;
        CloseHandle (ov.hEvent);
        return ok && geschrieben == n;
    }

    void verbindungLauf (HANDLE h)
    {
        std::vector<std::uint8_t> roh;
        std::uint8_t puffer[4096];

        // ── Bootstrap: u32-Praefix + JSON, hoechstens 16 KiB ──────────────
        std::string helloJson;
        while (laeuft.load())
        {
            if (roh.size() >= 4)
            {
                const std::uint32_t n = static_cast<std::uint32_t> (roh[0])
                                      | (static_cast<std::uint32_t> (roh[1]) << 8)
                                      | (static_cast<std::uint32_t> (roh[2]) << 16)
                                      | (static_cast<std::uint32_t> (roh[3]) << 24);
                if (n == 0 || n > kMaxBootstrapBytes)
                {
                    ++abgewiesen;
                    schliessen (h);
                    return;
                }
                if (roh.size() >= 4 + n)
                {
                    helloJson.assign (reinterpret_cast<const char*> (roh.data() + 4), n);
                    roh.erase (roh.begin(), roh.begin() + 4 + static_cast<std::ptrdiff_t> (n));
                    break;
                }
            }
            const int gelesen = lesen (h, puffer, sizeof (puffer), 3000);
            if (gelesen <= 0)
            {
                schliessen (h);
                return;
            }
            roh.insert (roh.end(), puffer, puffer + gelesen);
        }

        const bool istTelemetry = helloJson.find ("\"telemetry\"") != std::string::npos;
        {
            std::lock_guard<std::mutex> l (textMutex);
            (istTelemetry ? letztesTelemetryHello : letztesControlHello) = helloJson;
        }

        if (istTelemetry)
        {
            // Kopplung: link_id UND challenge muessen aus dem eigenen welcome
            // stammen. Fehlt eines, wird geschlossen (§53.9).
            const bool passt =
                helloJson.find ("\"link_id\":\"" + kopplungLinkId() + "\"") != std::string::npos
                && helloJson.find ("\"challenge\":\"" + kopplungChallenge() + "\"")
                       != std::string::npos;
            if (! passt)
            {
                ++abgewiesen;
                {
                    std::lock_guard<std::mutex> l (textMutex);
                    letzterAbweisungsgrund = "telemetry ungekoppelt";
                }
                schliessen (h);
                return;
            }
        }

        // NAK-134/D-K04, D-K05: die Serverpruefung des Clients ist an dieser
        // Stelle bestanden und sein Hello gelesen — aber es kommt KEIN
        // welcome. Die Verbindung bleibt offen, damit der Client wirklich in
        // seine Lesefrist laeuft statt ein Ende zu sehen.
        if (welcomeAusbleiben.load())
        {
            while (laeuft.load() && welcomeAusbleiben.load())
                std::this_thread::sleep_for (std::chrono::milliseconds (20));
            schliessen (h);
            return;
        }

        // ── welcome, bereits v3-gerahmt ───────────────────────────────────
        const std::string ausgegebeneLink =
            welcomeFremdeLinkId.load() ? hex32 ('9') : kopplungLinkId();
        std::string welcome = std::string ("{\"type\":\"welcome\"");
        if (! welcomeOhneProtokoll.load())
            welcome += ",\"protocol\":3";
        welcome += ",\"broker_version\":";
        if (welcomeTypfalsch.load())
            welcome += "null";                                  // Typ statt Text
        else if (welcomeLangeVersion.load())
            welcome += "\"" + std::string (65, 'v') + "\"";      // maxLength 64
        else
            welcome += "\"test\"";
        welcome += std::string (",\"broker_epoch\":\"") + hex32 ('c')
                 + "\",\"link_id\":\"" + ausgegebeneLink
                 + "\",\"challenge\":\"" + kopplungChallenge() + "\"";
        if (welcomeZusatzfeld.load())
            welcome += ",\"extra\":1";                          // additionalProperties:false
        welcome += "}";
        std::vector<std::uint8_t> aus;
        envelopeSchreiben (welcomeAlsP2.load() ? Familie::p2 : Familie::p0, 0,
                           reinterpret_cast<const std::uint8_t*> (welcome.data()),
                           welcome.size(), aus);
        if (sendeKaputtenFrame.load())
            aus[4] = 9;  // encoding unbekannt — der Client muss schliessen
        if (! schreiben (h, aus.data(), aus.size()))
        {
            schliessen (h);
            return;
        }

        // Ein korrekt gerahmter P2-Frame auf der CONTROL-Verbindung: der
        // Client darf ihn NICHT an `beiAntwort` weiterreichen (T2-Befund 4).
        if (sendeP2AufControl.load() && ! istTelemetry)
        {
            std::uint8_t nutz[32];
            std::memset (nutz, 0x11, sizeof (nutz));
            std::vector<std::uint8_t> p2rahmen;
            envelopeSchreiben (Familie::p2, 0, nutz, sizeof (nutz), p2rahmen);
            if (! schreiben (h, p2rahmen.data(), p2rahmen.size()))
            {
                schliessen (h);
                return;
            }
        }

        // Ein P0-Frame auf der Telemetrieverbindung: dort traegt der Vertrag
        // ausschliesslich P2 (T2-Befund 4, Telemetrie-Haelfte).
        if (sendeP0AufTelemetrie.load() && istTelemetry)
        {
            const std::string beliebig = "{\"type\":\"heartbeat_ack\"}";
            std::vector<std::uint8_t> p0rahmen;
            envelopeSchreiben (Familie::p0, 0,
                               reinterpret_cast<const std::uint8_t*> (beliebig.data()),
                               beliebig.size(), p0rahmen);
            if (! schreiben (h, p0rahmen.data(), p0rahmen.size()))
            {
                schliessen (h);
                return;
            }
        }

        // P2-Flut auf der Telemetrieverbindung: vertragsgemaesse Familie,
        // aber ueber der Rate (T2-Befund 5, Telemetrie-Haelfte).
        if (const int flutT = frameFlutTelemetrieP2.load(); flutT > 0 && istTelemetry)
        {
            std::uint8_t nutz[16];
            std::memset (nutz, 0x33, sizeof (nutz));
            std::vector<std::uint8_t> einer;
            envelopeSchreiben (Familie::p2, 0, nutz, sizeof (nutz), einer);
            std::vector<std::uint8_t> haeppchen;
            for (int i = 0; i < 64; ++i)
                haeppchen.insert (haeppchen.end(), einer.begin(), einer.end());
            for (int gesendet = 0; gesendet < flutT && laeuft.load(); gesendet += 64)
                if (! schreiben (h, haeppchen.data(), haeppchen.size()))
                    break;
        }

        // Mehr Frames, als die Nachrichtenrate erlaubt (T2-Befund 5). In
        // Haeppchen, damit der Server nicht 5 s in einem einzigen Write steht,
        // wenn der Client die Verbindung — richtigerweise — schliesst.
        if (const int flut = frameFlut.load(); flut > 0 && ! istTelemetry)
        {
            const std::string ack =
                "{\"type\":\"heartbeat_ack\",\"sequence\":0,\"duplicate_instance_id\":false}";
            std::vector<std::uint8_t> einer;
            envelopeSchreiben (Familie::p0, 0,
                               reinterpret_cast<const std::uint8_t*> (ack.data()),
                               ack.size(), einer);
            std::vector<std::uint8_t> haeppchen;
            for (int i = 0; i < 64; ++i)
                haeppchen.insert (haeppchen.end(), einer.begin(), einer.end());
            for (int gesendet = 0; gesendet < flut && laeuft.load(); gesendet += 64)
                if (! schreiben (h, haeppchen.data(), haeppchen.size()))
                    break;   // der Client hat geschlossen — genau das ist das Ziel
        }

        if (nichtLesen.load())
        {
            // `B-CC-07`: der ACK trifft ein, waehrend der Client schon
            // schreibt. Erst warten, damit sein Sendeweg sicher steht.
            if (const int nachMs = ackNachNichtLesenMs.load();
                nachMs > 0 && ! istTelemetry)
            {
                std::this_thread::sleep_for (std::chrono::milliseconds (nachMs));
                const std::string ack =
                    "{\"type\":\"heartbeat_ack\",\"sequence\":7,\"duplicate_instance_id\":false}";
                std::vector<std::uint8_t> rahmen;
                envelopeSchreiben (Familie::p0, 0,
                                   reinterpret_cast<const std::uint8_t*> (ack.data()),
                                   ack.size(), rahmen);
                schreiben (h, rahmen.data(), rahmen.size());
            }
            while (laeuft.load())
                std::this_thread::sleep_for (std::chrono::milliseconds (20));
            schliessen (h);
            return;
        }

        // ── ab hier nur noch v3-Frames ────────────────────────────────────
        StromLeser leser;
        std::vector<std::uint8_t> wirePuffer;
        auto wireMerken = [&] (const std::uint8_t* daten, std::size_t n)
        {
            if (! istTelemetry || n == 0)
                return;
            wirePuffer.insert (wirePuffer.end(), daten, daten + n);
            while (wirePuffer.size() >= 4)
            {
                const std::uint32_t frameLen = static_cast<std::uint32_t> (wirePuffer[0])
                    | (static_cast<std::uint32_t> (wirePuffer[1]) << 8)
                    | (static_cast<std::uint32_t> (wirePuffer[2]) << 16)
                    | (static_cast<std::uint32_t> (wirePuffer[3]) << 24);
                if (frameLen < kKopfBytes || frameLen > kMaxFrameBytes)
                    return;
                const auto gesamt = static_cast<std::size_t> (4u + frameLen);
                if (wirePuffer.size() < gesamt)
                    return;
                {
                    std::lock_guard<std::mutex> l (textMutex);
                    letzterTelemetryWire.assign (wirePuffer.begin(),
                                                   wirePuffer.begin() + gesamt);
                }
                wirePuffer.erase (wirePuffer.begin(), wirePuffer.begin() + gesamt);
            }
        };
        wireMerken (roh.data(), roh.size());
        leser.fuettern (roh.data(), roh.size());
        while (laeuft.load())
        {
            // Einmalig: die Telemetriepipe schliessen. Der echte Broker tut
            // das, wenn die zugehoerige Control-Verbindung endet.
            if (istTelemetry && telemetrieSchliessen.exchange (false))
            {
                schliessen (h);
                return;
            }
            bool weiter = true;
            while (weiter)
            {
                const auto e = leser.naechster();
                if (e.art == StromLeser::Art::unvollstaendig) { weiter = false; break; }
                if (e.art == StromLeser::Art::verstoss)
                {
                    ++abgewiesen;
                    schliessen (h);
                    return;
                }
                if (e.kopf.familie == Familie::p0)
                {
                    ++p0;
                    // heartbeat -> heartbeat_ack, sonst nichts.
                    const std::string text (reinterpret_cast<const char*> (e.payload),
                                            e.payloadLaenge);
                    {
                        std::lock_guard<std::mutex> l (textMutex);
                        p0Texte.push_back (text);
                        p0Minors.push_back (e.kopf.schemaMinor);
                    }
                    if (text.find ("\"heartbeat\"") != std::string::npos)
                    {
                        const std::string ack =
                            "{\"type\":\"heartbeat_ack\",\"sequence\":0,\"duplicate_instance_id\":false}";
                        std::vector<std::uint8_t> antwort;
                        envelopeSchreiben (Familie::p0,
                                           static_cast<std::uint8_t> (controlAntwortMinor.load()),
                                           reinterpret_cast<const std::uint8_t*> (ack.data()),
                                           ack.size(), antwort);
                        if (! schreiben (h, antwort.data(), antwort.size()))
                        {
                            schliessen (h);
                            return;
                        }
                        if (nachErsterP0AntwortNichtLesen.load())
                        {
                            while (laeuft.load())
                                std::this_thread::sleep_for (
                                    std::chrono::milliseconds (20));
                            schliessen (h);
                            return;
                        }
                    }
                    else if (const auto commandId = commandIdAusJson (text);
                             ! commandId.empty())
                    {
                        if (commandVorAckSchliessen.exchange (false))
                        {
                            schliessen (h);
                            return;
                        }

                        const int art = commandAckArt.load();
                        if (art >= 1 && art <= 5)
                        {
                            if (const int pause = commandAckVerzoegerungMs.load(); pause > 0)
                                std::this_thread::sleep_for (
                                    std::chrono::milliseconds (pause));
                            static constexpr const char* ergebnisse[] = {
                                "", "angewandt", "abgelehnt", "konflikt",
                                "abgelaufen", "idempotent_wiederholt"
                            };
                            const int bruch = commandAckVertragsbruch.load();
                            std::string ack = "{\"type\":\"command_ack\",\"command_id\":\""
                                + commandId + "\",\"ergebnis\":\"" + ergebnisse[art]
                                + "\",\"state_revision\":"
                                + (bruch == 1 ? "01" : "7");
                            if ((art == 1 || art == 5) && bruch != 4)
                                ack += ",\"state_hash\":\"" + std::string (64, 'd') + "\"";
                            if (bruch == 2)
                                ack += ",\"state_hash\":17";
                            if (bruch == 3)
                                ack += ",\"code\":\"nicht_im_schema\"";
                            if (commandAckMitEventUuid.load())
                                ack += ",\"event_uuid\":\"" + hex32 ('e') + "\"";
                            ack += "}";

                            std::vector<std::uint8_t> antwort;
                            envelopeSchreiben (Familie::p0, 0,
                                reinterpret_cast<const std::uint8_t*> (ack.data()),
                                ack.size(), antwort);
                            if (! schreiben (h, antwort.data(), antwort.size()))
                            {
                                schliessen (h);
                                return;
                            }
                        }
                    }
                }
                else if (e.kopf.familie == Familie::p1)
                {
                    ++p1;
                    std::lock_guard<std::mutex> l (textMutex);
                    p1Minors.push_back (e.kopf.schemaMinor);
                    p1Texte.emplace_back (reinterpret_cast<const char*> (e.payload),
                                          e.payloadLaenge);
                }
                else                                    ++p2;
            }

            const int gelesen = lesen (h, puffer, sizeof (puffer), 100);
            if (gelesen < 0)
                break;
            if (gelesen > 0)
            {
                wireMerken (puffer, static_cast<std::size_t> (gelesen));
                leser.fuettern (puffer, static_cast<std::size_t> (gelesen));
            }
        }
        schliessen (h);
    }

    static void schliessen (HANDLE h)
    {
        FlushFileBuffers (h);
        DisconnectNamedPipe (h);
        CloseHandle (h);
    }

    std::string name;
    std::atomic<bool> laeuft { false };
    std::thread acceptor;
    std::mutex threadMutex;
    std::vector<std::thread> verbindungsThreads;
};

std::atomic<unsigned> namensFolge { 0 };

std::string testPipeName (const char* fall)
{
    // Im PROBE-Namensraum, demselben, den die beiden Probeprogramme als
    // einzigen zulassen (§48.3 / T2-Befund 7 vom 2026-08-29).
    return std::string (kPipePraefixProbe) + "test."
         + std::to_string ((int) GetCurrentProcessId()) + "."
         + std::to_string ((int) namensFolge.fetch_add (1)) + "." + fall;
}

ServerErwartung testExeErwartung (std::uint32_t pid = 0)
{
    const auto exe = juce::File::getSpecialLocation (
        juce::File::currentExecutableFile).getFullPathName();
    return serverErwartungFuerTestdatei (std::wstring (exe.toWideCharPointer()), pid);
}

bool serverHatKeinHello (TestServer& server)
{
    std::lock_guard<std::mutex> l (server.textMutex);
    return server.letztesControlHello.empty() && server.letztesTelemetryHello.empty();
}

ControlHello nak123ControlHello()
{
    ControlHello h;
    h.adresse = testAdresse (hex32 ('a'));
    return h;
}

/// Derselbe Testserver in einem zweiten Prozess. Dadurch koennen C-05/C-09
/// einen echten Besitzer-/PID-Wechsel herstellen, ohne eine fremde Binary
/// oder die Produktionspipe zu verwenden.
class ChildTestServer
{
public:
    explicit ChildTestServer (std::string pipe) : pipeName (std::move (pipe)) {}
    ~ChildTestServer() { stoppen(); }

    ChildTestServer (const ChildTestServer&) = delete;
    ChildTestServer& operator= (const ChildTestServer&) = delete;

    bool starten()
    {
        if (prozess != nullptr)
            return false;
        SECURITY_ATTRIBUTES vererbbar {};
        vererbbar.nLength = sizeof (vererbbar);
        vererbbar.bInheritHandle = TRUE;
        HANDLE kindStdin = nullptr;
        if (CreatePipe (&kindStdin, &stdinSchreiben, &vererbbar, 0) == FALSE)
            return false;
        SetHandleInformation (stdinSchreiben, HANDLE_FLAG_INHERIT, 0);

        const auto exeText = juce::File::getSpecialLocation (
            juce::File::currentExecutableFile).getFullPathName();
        const std::wstring exe (exeText.toWideCharPointer());
        const std::wstring pipeW (pipeName.begin(), pipeName.end());
        std::wstring befehl = L"\"" + exe + L"\" --nak123-test-server \""
                            + pipeW + L"\"";
        STARTUPINFOW start {};
        start.cb = sizeof (start);
        start.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        start.wShowWindow = SW_HIDE;
        start.hStdInput = kindStdin;
        start.hStdOutput = GetStdHandle (STD_OUTPUT_HANDLE);
        start.hStdError = GetStdHandle (STD_ERROR_HANDLE);
        PROCESS_INFORMATION info {};
        const BOOL ok = CreateProcessW (
            exe.c_str(), befehl.data(), nullptr, nullptr, TRUE,
            CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT,
            nullptr, nullptr, &start, &info);
        CloseHandle (kindStdin);
        if (ok == FALSE)
        {
            CloseHandle (stdinSchreiben);
            stdinSchreiben = nullptr;
            return false;
        }
        CloseHandle (info.hThread);
        prozess = info.hProcess;
        prozessId = info.dwProcessId;
        return warteAuf (5000, [&] { return namedPipeErreichbar (pipeName); });
    }

    void stoppen()
    {
        if (prozess == nullptr)
            return;
        if (stdinSchreiben != nullptr)
        {
            DWORD geschrieben = 0;
            const char stop[] = "STOP\n";
            WriteFile (stdinSchreiben, stop, sizeof (stop) - 1, &geschrieben, nullptr);
            CloseHandle (stdinSchreiben);
            stdinSchreiben = nullptr;
        }
        if (WaitForSingleObject (prozess, 5000) == WAIT_TIMEOUT)
        {
            TerminateProcess (prozess, 91);
            WaitForSingleObject (prozess, 5000);
        }
        CloseHandle (prozess);
        prozess = nullptr;
        prozessId = 0;
    }

    void hartBeenden()
    {
        if (prozess == nullptr)
            return;
        if (stdinSchreiben != nullptr)
        {
            CloseHandle (stdinSchreiben);
            stdinSchreiben = nullptr;
        }
        TerminateProcess (prozess, 92);
        WaitForSingleObject (prozess, 5000);
        CloseHandle (prozess);
        prozess = nullptr;
        prozessId = 0;
    }

    std::uint32_t pid() const noexcept { return prozessId; }

private:
    std::string pipeName;
    HANDLE prozess = nullptr;
    HANDLE stdinSchreiben = nullptr;
    std::uint32_t prozessId = 0;
};

int nak123TestServerMain (const std::string& pipe)
{
    TestServer server (pipe);
    if (! server.starten() || ! warteAuf (5000, [&] { return namedPipeErreichbar (pipe); }))
        return 31;
    std::string zeile;
    std::getline (std::cin, zeile);
    server.stoppen();
    return zeile == "STOP" ? 0 : 32;
}

void fehlender_broker_ist_einzige_startbedingung_auch_mit_serverauth()
{
    const auto pipe = testPipeName ("c01-fehlt");
    ControlClient client ([] { return nak123ControlHello(); }, pipe);
    client.start();
    const bool ok = warteAuf (3000, [&] {
        const auto s = client.snapshot();
        return s.status == ControlClient::Status::getrennt
            && s.brokerPipeFehlt
            && s.serverPruefstatus == ServerPruefStatus::nichtDa
            && s.serverPrueffehler == ServerPruefFehler::pipeFehlt
            && s.serverPruefungen == 0;
    });
    client.stop();
    pruefe (ok, "fehlender_broker_ist_einzige_startbedingung_auch_mit_serverauth");
}

void serverauth_gueltiger_pid_sid_datei_hash_und_signer_vor_hello()
{
    const auto pipe = testPipeName ("c02-ok");
    TestServer server (pipe);
    server.starten();
    std::atomic<bool> vorHash { false }, hashFreigeben { false };
    auto erwartung = testExeErwartung (GetCurrentProcessId());
    erwartung.testVorFehlerErreicht = &vorHash;
    erwartung.testFehlerFreigeben = &hashFreigeben;
    ControlClient client ([] { return nak123ControlHello(); }, pipe,
                          {}, {}, {}, {}, erwartung);
    client.start();
    const bool vorHelloGehalten = warteAuf (4000, [&] { return vorHash.load(); });
    const auto waehrendPruefung = client.snapshot();
    const bool nullBytesVorVerifikation = serverHatKeinHello (server)
        && waehrendPruefung.status == ControlClient::Status::verbindet
        && waehrendPruefung.serverPruefstatus == ServerPruefStatus::nichtGeprueft
        && waehrendPruefung.serverPruefungen == 0;
    hashFreigeben.store (true);
    const bool ok = warteAuf (5000, [&] {
        const auto s = client.snapshot();
        std::lock_guard<std::mutex> l (server.textMutex);
        return s.status == ControlClient::Status::verbunden
            && s.serverPruefstatus == ServerPruefStatus::verifiziert
            && s.serverPrueffehler == ServerPruefFehler::keiner
            && s.serverPid == GetCurrentProcessId()
            && s.serverPruefungen == 1
            && ! server.letztesControlHello.empty();
    });
    client.stop();
    server.stoppen();
    pruefe (vorHelloGehalten && nullBytesVorVerifikation && ok,
            "serverauth_gueltiger_pid_sid_datei_hash_und_signer_vor_hello");
}

void serverauth_fremde_sid_und_tokenabfragefehler_senden_null_bytes()
{
    const std::vector<ServerPruefFehler> faelle {
        ServerPruefFehler::serverprozessNichtOeffnen,
        ServerPruefFehler::serverTokenNichtOeffnen,
        ServerPruefFehler::pluginTokenNichtOeffnen,
        ServerPruefFehler::serverTokenUserNichtLesbar,
        ServerPruefFehler::pluginTokenUserNichtLesbar,
        ServerPruefFehler::serverSidUngueltig,
        ServerPruefFehler::pluginSidUngueltig,
        ServerPruefFehler::serverSidFalsch,
    };
    bool alle = true;
    for (const auto fehlerkante : faelle)
    {
        const auto pipe = testPipeName (serverPruefFehlerName (fehlerkante));
        TestServer server (pipe);
        server.starten();
        auto erwartung = testExeErwartung (GetCurrentProcessId());
        erwartung.testFehler = fehlerkante;
        ControlClient client ([] { return nak123ControlHello(); }, pipe,
                              {}, {}, {}, {}, erwartung);
        client.start();
        const bool fiel = warteAuf (3000, [&] {
            const auto s = client.snapshot();
            return s.status == ControlClient::Status::getrennt
                && ! s.brokerPipeFehlt
                && s.serverPruefstatus == ServerPruefStatus::belegtAberUnverifiziert
                && s.serverPrueffehler == fehlerkante
                && s.serverPruefungen == 1;
        });
        const auto versucheNachFehler = client.snapshot().verbindungsVersuche;
        std::this_thread::sleep_for (std::chrono::milliseconds (600));
        const auto danach = client.snapshot();
        alle = alle && fiel && serverHatKeinHello (server)
             && danach.verbindungsVersuche == versucheNachFehler
             && danach.p0Gesendet == 0 && danach.p1Gesendet == 0;
        client.stop();
        server.stoppen();
    }
    pruefe (alle, "serverauth_fremde_sid_und_tokenabfragefehler_senden_null_bytes");
}

void serverauth_bild_dateiid_hash_signatur_und_signer_fail_closed()
{
    const std::vector<ServerPruefFehler> faelle {
        ServerPruefFehler::prozessbildNichtErmittelbar,
        ServerPruefFehler::prozessbildNichtOeffnen,
        ServerPruefFehler::erwarteteDateiNichtOeffnen,
        ServerPruefFehler::dateiidentitaetFalsch,
        ServerPruefFehler::hashFalsch,
        ServerPruefFehler::signaturFehltOderUngueltig,
        ServerPruefFehler::signerFalsch,
    };
    bool alle = true;
    for (const auto fehlerkante : faelle)
    {
        const auto pipe = testPipeName (serverPruefFehlerName (fehlerkante));
        TestServer server (pipe);
        server.starten();
        auto erwartung = testExeErwartung (GetCurrentProcessId());
        erwartung.testFehler = fehlerkante;
        ControlClient client ([] { return nak123ControlHello(); }, pipe,
                              {}, {}, {}, {}, erwartung);
        client.start();
        const bool fiel = warteAuf (3000, [&] {
            const auto s = client.snapshot();
            return s.serverPruefstatus == ServerPruefStatus::belegtAberUnverifiziert
                && s.serverPrueffehler == fehlerkante
                && s.serverPruefungen == 1;
        });
        alle = alle && fiel && serverHatKeinHello (server);
        client.stop();
        server.stoppen();
    }
    pruefe (alle, "serverauth_bild_dateiid_hash_signatur_und_signer_fail_closed");
}

void waitnamedpipe_toctou_wird_am_verbundenen_handle_erkannt()
{
    const auto pipe = testPipeName ("c05-toctou");
    ChildTestServer erster (pipe);
    const bool ersterDa = erster.starten() && namedPipeErreichbar (pipe);
    const auto alterPid = erster.pid();
    erster.stoppen();

    TestServer neuer (pipe);
    neuer.starten();
    ControlClient client ([] { return nak123ControlHello(); }, pipe,
                          {}, {}, {}, {}, testExeErwartung (alterPid));
    client.start();
    const bool erkannt = warteAuf (4000, [&] {
        const auto s = client.snapshot();
        return s.serverPrueffehler == ServerPruefFehler::serverPidFalsch
            && s.serverPid == GetCurrentProcessId()
            && s.serverPruefungen == 1;
    });
    const bool keinHello = serverHatKeinHello (neuer);
    client.stop();
    neuer.stoppen();
    pruefe (ersterDa && alterPid != GetCurrentProcessId() && erkannt && keinHello,
            "waitnamedpipe_toctou_wird_am_verbundenen_handle_erkannt");
}

void control_und_telemetry_authentisieren_jedes_handle()
{
    const auto pipe = testPipeName ("c06-beide");
    TestServer server (pipe);
    server.starten();
    ControlClient control ([] { return nak123ControlHello(); }, pipe);
    control.start();
    const bool controlDa = warteAuf (5000, [&] {
        return control.snapshot().status == ControlClient::Status::verbunden;
    });
    TelemetryClient telemetry ([&] {
        TelemetryHello h;
        h.adresse = testAdresse (hex32 ('a'));
        const auto c = control.snapshot();
        h.linkId = c.linkId;
        h.challenge = c.challenge;
        return h;
    }, pipe);
    telemetry.start();
    const bool telemetryDa = warteAuf (5000, [&] {
        return telemetry.snapshot().status == TelemetryClient::Status::verbunden;
    });
    const auto c = control.snapshot();
    const auto t = telemetry.snapshot();
    bool beideHello = false;
    {
        std::lock_guard<std::mutex> l (server.textMutex);
        beideHello = ! server.letztesControlHello.empty()
                 && ! server.letztesTelemetryHello.empty();
    }
    telemetry.stop();
    control.stop();
    server.stoppen();
    pruefe (controlDa && telemetryDa && beideHello
                && c.serverPruefstatus == ServerPruefStatus::verifiziert
                && t.serverPruefstatus == ServerPruefStatus::verifiziert
                && c.serverPruefungen == 1 && t.serverPruefungen == 1,
            "control_und_telemetry_authentisieren_jedes_handle");
}

void vorhandene_pipe_wird_nur_mit_dem_authentisierten_handle_uebernommen()
{
    const auto pipe = testPipeName ("c07-tristate");
    TestServer server (pipe);
    const bool serverSteht = server.starten();
    std::atomic<ServerPruefStatus> status { ServerPruefStatus::nichtDa };
    std::atomic<bool> staleVerbunden { false };
    int reconnects = 0, spawns = 0;
    BrokerLifecycleHooks hooks;
    hooks.verbunden = [&] { return staleVerbunden.load(); };
    hooks.connectFehlgeschlagen = [] { return true; };
    hooks.serverPruefstatus = [&] { return status.load(); };
    hooks.darfStarten = [] { return true; };
    hooks.reconnect = [&] { ++reconnects; };
    hooks.pruefen = [] { return BrokerPruefBericht {}; };
    hooks.spawn = [&] { ++spawns; return true; };
    hooks.mutexName = L"Local\\Nakama.NAK123.C07.ok."
                    + std::to_wstring (GetCurrentProcessId());
    hooks.pipeName = pipe;
    BrokerLifecycle lifecycle (std::move (hooks));
    lifecycle.tickFuerTest (0);
    const auto wartend = lifecycle.snapshot();
    status.store (ServerPruefStatus::verifiziert);
    lifecycle.tickFuerTest (1);
    const auto fertig = lifecycle.snapshot();
    const bool ok = serverSteht && reconnects == 1 && spawns == 0
                 && wartend.wartetAufServerpruefung
                 && ! fertig.wartetAufServerpruefung
                 && ! fertig.serverNichtVerifiziert
                 && fertig.letzterServerPruefstatus == ServerPruefStatus::verifiziert;
    server.stoppen();
    pruefe (ok, "vorhandene_pipe_wird_nur_mit_dem_authentisierten_handle_uebernommen");
}

void unverifizierte_belegte_pipe_spawnt_und_reconnectet_nicht()
{
    const auto pipe = testPipeName ("c07-block");
    TestServer server (pipe);
    const bool serverSteht = server.starten();
    std::atomic<ServerPruefStatus> status { ServerPruefStatus::nichtDa };
    std::atomic<bool> staleVerbunden { false };
    int reconnects = 0, spawns = 0;
    BrokerLifecycleHooks hooks;
    hooks.verbunden = [&] { return staleVerbunden.load(); };
    hooks.connectFehlgeschlagen = [] { return true; };
    hooks.serverPruefstatus = [&] { return status.load(); };
    hooks.darfStarten = [] { return true; };
    hooks.reconnect = [&] { ++reconnects; };
    hooks.pruefen = [] { return BrokerPruefBericht {}; };
    hooks.spawn = [&] { ++spawns; return true; };
    hooks.mutexName = L"Local\\Nakama.NAK123.C07.block."
                    + std::to_wstring (GetCurrentProcessId());
    hooks.pipeName = pipe;
    BrokerLifecycle lifecycle (std::move (hooks));
    lifecycle.tickFuerTest (0);
    staleVerbunden.store (true);
    status.store (ServerPruefStatus::belegtAberUnverifiziert);
    lifecycle.tickFuerTest (1);
    lifecycle.tickFuerTest (1000);
    const auto s = lifecycle.snapshot();
    const bool ok = serverSteht && reconnects == 1 && spawns == 0
                 && s.serverNichtVerifiziert && ! s.wartetAufBereit
                 && ! s.wartetAufServerpruefung
                 && s.letzterServerPruefstatus
                        == ServerPruefStatus::belegtAberUnverifiziert;
    server.stoppen();
    pruefe (ok, "unverifizierte_belegte_pipe_spawnt_und_reconnectet_nicht");
    pruefe (ok, "vorhandene_fremdpipe_wird_nicht_adoptiert");
}

void spawn_pid_ersetzt_serverauth_nicht_und_fremdpipe_im_bereitfenster_faellt()
{
    const auto pipe = testPipeName ("c08-spawn");
    ControlClient control ([] { return nak123ControlHello(); }, pipe);
    control.start();
    const bool fehlt = warteAuf (3000, [&] {
        return control.snapshot().serverPruefstatus == ServerPruefStatus::nichtDa;
    });
    std::unique_ptr<ChildTestServer> kind;
    int spawns = 0;
    BrokerLifecycleHooks hooks;
    hooks.verbunden = [&] {
        return control.snapshot().status == ControlClient::Status::verbunden;
    };
    hooks.connectFehlgeschlagen = [&] { return control.snapshot().brokerPipeFehlt; };
    hooks.serverPruefstatus = [&] { return control.snapshot().serverPruefstatus; };
    hooks.darfStarten = [] { return true; };
    hooks.reconnect = [&] { control.reconnect(); };
    hooks.pruefen = [] { return BrokerPruefBericht {}; };
    hooks.spawn = [&] {
        ++spawns;
        kind = std::make_unique<ChildTestServer> (pipe);
        return kind->starten();
    };
    hooks.mutexName = L"Local\\Nakama.NAK123.C08."
                    + std::to_wstring (GetCurrentProcessId());
    hooks.pipeName = pipe;
    BrokerLifecycle lifecycle (std::move (hooks));
    lifecycle.tickFuerTest (0);
    lifecycle.tickFuerTest (SPAWN_CONNECT_BACKOFF_START_MS);
    const bool fiel = warteAuf (5000, [&] {
        const auto s = control.snapshot();
        return s.serverPruefstatus == ServerPruefStatus::belegtAberUnverifiziert
            && s.serverPrueffehler == ServerPruefFehler::serverPidFalsch
            && s.serverPruefungen == 1;
    });
    lifecycle.tickFuerTest (SPAWN_CONNECT_BACKOFF_START_MS + 1);
    const auto ls = lifecycle.snapshot();
    const auto cs = control.snapshot();
    const bool ok = fehlt && spawns == 1 && fiel
                 && ls.serverNichtVerifiziert && ! ls.wartetAufBereit
                 && cs.status == ControlClient::Status::getrennt
                 && cs.p0Gesendet == 0 && cs.p1Gesendet == 0;
    control.stop();
    if (kind != nullptr)
        kind->stoppen();
    pruefe (ok, "spawn_pid_ersetzt_serverauth_nicht_und_fremdpipe_im_bereitfenster_faellt");
}

void reconnect_prueft_pid_sid_und_bild_ohne_altfreigabe()
{
    const auto pipe = testPipeName ("c09-reconnect");
    ChildTestServer erster (pipe);
    const bool startA = erster.starten();
    ControlClient control ([] { return nak123ControlHello(); }, pipe,
                           {}, {}, {}, {}, testExeErwartung());
    control.start();
    const bool a = warteAuf (5000, [&] {
        const auto s = control.snapshot();
        return s.status == ControlClient::Status::verbunden
            && s.serverPid == erster.pid() && s.serverPruefungen == 1;
    });
    const auto pidA = erster.pid();
    erster.stoppen();
    ChildTestServer zweiter (pipe);
    const bool startB = zweiter.starten();
    control.reconnect();
    const bool b = warteAuf (5000, [&] {
        const auto s = control.snapshot();
        return s.status == ControlClient::Status::verbunden
            && s.serverPid == zweiter.pid() && s.serverPruefungen >= 2;
    });
    const auto nachher = control.snapshot();
    control.stop();
    zweiter.stoppen();
    const bool pidUndBildNeu = startA && startB && a && b
        && pidA != nachher.serverPid
        && nachher.serverPruefstatus == ServerPruefStatus::verifiziert;

    // Das PID-Bein oben beweist den neuen Prozess. Diese zweite, am Hash-
    // Abschluss arretierte Verbindung misst die Byte-Reihenfolge selbst:
    // Auch beim Reconnect sieht der neue Server vor seiner eigenen C-02-
    // Freigabe kein Hello und damit auch keinen wartenden P0/P1-Frame.
    const auto ordnungsPipe = testPipeName ("c09-reconnect-order");
    std::atomic<bool> vorHash { false }, hashFreigeben { true };
    auto erwartung = testExeErwartung (GetCurrentProcessId());
    erwartung.testVorFehlerErreicht = &vorHash;
    erwartung.testFehlerFreigeben = &hashFreigeben;
    TestServer ordnungA (ordnungsPipe);
    ordnungA.starten();
    ControlClient ordnungsClient ([] { return nak123ControlHello(); }, ordnungsPipe,
                                  {}, {}, {}, {}, erwartung);
    ordnungsClient.start();
    const bool ordnungAStand = warteAuf (5000, [&] {
        return ordnungsClient.snapshot().status == ControlClient::Status::verbunden;
    });
    hashFreigeben.store (false);
    vorHash.store (false);
    ordnungA.stoppen();
    ordnungsClient.reconnect();
    TestServer ordnungB (ordnungsPipe);
    ordnungB.starten();
    const bool reconnectInPruefung = warteAuf (5000, [&] { return vorHash.load(); });
    const auto vorFreigabe = ordnungsClient.snapshot();
    const bool reconnectNullBytes = serverHatKeinHello (ordnungB)
        && vorFreigabe.status == ControlClient::Status::verbindet
        && vorFreigabe.serverPruefstatus == ServerPruefStatus::nichtGeprueft;
    hashFreigeben.store (true);
    const bool ordnungBStand = warteAuf (5000, [&] {
        const auto s = ordnungsClient.snapshot();
        std::lock_guard<std::mutex> l (ordnungB.textMutex);
        return s.status == ControlClient::Status::verbunden
            && s.serverPruefstatus == ServerPruefStatus::verifiziert
            && s.serverPruefungen >= 2
            && ! ordnungB.letztesControlHello.empty();
    });
    ordnungsClient.stop();
    ordnungB.stoppen();

    // Reconnect waehrend der Pruefung: Das Ergebnis der alten Generation
    // darf weder gezaehlt noch als Sicherheitsstatus veroeffentlicht werden.
    // Die zweite Generation prueft denselben Handletyp erneut und liefert
    // genau EINE sichtbare (hier absichtlich negative) Entscheidung.
    const auto stalePipe = testPipeName ("c09-stale-auth");
    std::atomic<bool> staleVorHash { false }, staleFreigeben { false };
    auto staleErwartung = testExeErwartung (GetCurrentProcessId());
    staleErwartung.testFehler = ServerPruefFehler::hashFalsch;
    staleErwartung.testVorFehlerErreicht = &staleVorHash;
    staleErwartung.testFehlerFreigeben = &staleFreigeben;
    TestServer staleServer (stalePipe);
    staleServer.starten();
    ControlClient staleClient ([] { return nak123ControlHello(); }, stalePipe,
                               {}, {}, {}, {}, staleErwartung);
    staleClient.start();
    const bool altePruefungHaelt = warteAuf (5000, [&] { return staleVorHash.load(); });
    staleClient.reconnect();
    staleFreigeben.store (true);
    const bool nurNeueGenerationSichtbar = warteAuf (5000, [&] {
        const auto s = staleClient.snapshot();
        return s.serverPruefstatus == ServerPruefStatus::belegtAberUnverifiziert
            && s.serverPrueffehler == ServerPruefFehler::hashFalsch
            && s.serverPruefungen == 1 && s.verbindungsVersuche >= 2;
    });
    const bool staleNullBytes = serverHatKeinHello (staleServer);
    staleClient.stop();
    staleServer.stoppen();

    const bool ok = pidUndBildNeu && ordnungAStand && reconnectInPruefung
                 && reconnectNullBytes && ordnungBStand && altePruefungHaelt
                 && nurNeueGenerationSichtbar && staleNullBytes;
    pruefe (ok, "reconnect_prueft_pid_sid_und_bild_ohne_altfreigabe");
    pruefe (ok, "reconnect_sendet_erst_nach_neuer_serverauth");
}

bool gepinnte_serverauth_verweigert_besitzer_nach_ende (const char* fall,
                                                        bool harterCrash)
{
    const auto pipe = testPipeName (fall);
    ChildTestServer erster (pipe);
    const bool startA = erster.starten();
    const auto pidA = erster.pid();
    ControlClient control ([] { return nak123ControlHello(); }, pipe,
                           {}, {}, {}, {}, testExeErwartung (pidA));
    control.start();
    const bool a = warteAuf (5000, [&] {
        return control.snapshot().status == ControlClient::Status::verbunden;
    });
    if (harterCrash)
        erster.hartBeenden();
    else
        erster.stoppen();
    ChildTestServer zweiter (pipe);
    const bool startB = zweiter.starten();
    const auto pidB = zweiter.pid();
    control.reconnect();
    const bool verweigert = warteAuf (5000, [&] {
        const auto s = control.snapshot();
        return s.serverPruefstatus == ServerPruefStatus::belegtAberUnverifiziert
            && s.serverPrueffehler == ServerPruefFehler::serverPidFalsch
            && s.serverPid == zweiter.pid() && s.serverPruefungen >= 2;
    });
    control.stop();
    zweiter.stoppen();
    return startA && startB && a && pidA != pidB && verweigert;
}

void serverauth_nach_brokercrash_verweigert_pidwechsel_und_fremdbild()
{
    const bool pidWechsel = gepinnte_serverauth_verweigert_besitzer_nach_ende (
        "a07-crash-pid", true);
    const auto pipe = testPipeName ("a07-fremdbild");
    TestServer fremdbild (pipe);
    fremdbild.starten();
    auto erwartung = testExeErwartung (GetCurrentProcessId());
    erwartung.testFehler = ServerPruefFehler::dateiidentitaetFalsch;
    ControlClient control ([] { return nak123ControlHello(); }, pipe,
                           {}, {}, {}, {}, erwartung);
    control.start();
    const bool bildVerweigert = warteAuf (4000, [&] {
        const auto s = control.snapshot();
        return s.serverPrueffehler == ServerPruefFehler::dateiidentitaetFalsch
            && s.serverPruefstatus == ServerPruefStatus::belegtAberUnverifiziert;
    }) && serverHatKeinHello (fremdbild);
    control.stop();
    fremdbild.stoppen();
    pruefe (pidWechsel && bildVerweigert,
            "serverauth_nach_brokercrash_verweigert_pidwechsel_und_fremdbild");
}

void serverauth_nach_idle_exit_verweigert_fremde_pipe()
{
    pruefe (gepinnte_serverauth_verweigert_besitzer_nach_ende (
                "a06-idle-pid", false),
            "serverauth_nach_idle_exit_verweigert_fremde_pipe");
}

//==============================================================================
juce::File wurzel()
{
    auto d = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    while (d.exists() && ! d.getChildFile ("eq-copilot").isDirectory())
        d = d.getParentDirectory();
    return d;
}

//==============================================================================
/// Testhaken der P2-Schleuse fuer `A-P2-04` (NAK-104): ein Verbraucher, der
/// zwischen zwei Anspruchsversuchen des Erzeugers WEITERRUECKT.
///
/// Er tut nichts, was `abholen` nicht auch tut: den gehaltenen Platz
/// freigeben, seinen Fortschritt melden, den naechsten beanspruchen — und er
/// hoert auf, sobald seine Position `schreib` erreicht, denn darueber gibt es
/// nichts zu holen. Genau daran scheitert eine feste Versuchsschranke.
struct WandernderVerbraucher
{
    bool          aktiv    = false;
    bool          haelt    = false;
    std::uint64_t position = 0;
    std::uint64_t grenze   = 0;
    int           spruenge = 0;

    template <class Schleuse>
    void vorAnspruch (Schleuse& s, std::uint64_t) noexcept
    {
        if (! aktiv)
            return;
        if (haelt)
        {
            s.testSlotFreigeben (position);
            ++position;
            s.testVerbrauchtMelden (position);
            haelt = false;
        }
        if (position >= grenze)
            return;
        haelt = s.testSlotBeanspruchen (position);
        if (haelt)
            ++spruenge;
    }
};

int phaseBCommandClientMain (const std::string& pipeName,
                             const std::string& commandId,
                             const std::string& erwartetesErgebnis,
                             const std::string& serverBildUtf8)
{
    if (! istHex32 (commandId))
        return 20;
    const juce::String serverBildText = juce::String::fromUTF8 (serverBildUtf8.c_str());
    const std::wstring serverBild (serverBildText.toWideCharPointer());
    const auto serverErwartung = serverErwartungFuerTestdatei (serverBild);
    if (serverErwartung.absoluterBrokerPfad.empty())
        return 23;
    std::mutex ackMutex;
    std::string letztesAck;
    ControlClient control ([&] {
        ControlHello h;
        h.adresse = testAdresse (hex32 ('a'));
        return h;
    }, pipeName, [&] (const std::string& antwort) {
        if (antwort.find ("\"type\":\"command_ack\"") != std::string::npos)
        {
            std::lock_guard<std::mutex> l (ackMutex);
            letztesAck = antwort;
        }
    }, {}, {}, {}, serverErwartung);
    control.start();
    const bool verbunden = warteAuf (10000, [&] {
        return control.snapshot().status == ControlClient::Status::verbunden;
    });
    const bool eingereiht = verbunden
                         && control.sendePersistenzP0 (persistenzBefehl (commandId));
    const bool beantwortet = eingereiht && warteAuf (20000, [&] {
        const auto s = control.snapshot();
        std::lock_guard<std::mutex> l (ackMutex);
        return s.inFlight == 0 && s.inFlightErfolg == 1
            && letztesAck.find ("\"command_id\":\"" + commandId + "\"")
                   != std::string::npos
            && letztesAck.find ("\"ergebnis\":\"" + erwartetesErgebnis + "\"")
                   != std::string::npos;
    });
    control.stop();
    std::lock_guard<std::mutex> l (ackMutex);
    const bool wireSauber = letztesAck.find ("event_uuid") == std::string::npos;
    std::cout << "phase_b_command_client result=" << erwartetesErgebnis
              << " connected=" << verbunden << " queued=" << eingereiht
              << " answered=" << beantwortet << " wire_uuid=" << ! wireSauber
              << std::endl;
    return beantwortet && wireSauber ? 0 : 21;
}

const char* brokerPruefFehlerName (BrokerPruefFehler zustand) noexcept
{
    switch (zustand)
    {
        case BrokerPruefFehler::keiner: return "ok";
        case BrokerPruefFehler::pfadNichtAbsolut: return "pfadNichtAbsolut";
        case BrokerPruefFehler::dateiNichtLesbar: return "dateiNichtLesbar";
        case BrokerPruefFehler::erwarteterHashUngueltig: return "erwarteterHashUngueltig";
        case BrokerPruefFehler::hashFalsch: return "hashFalsch";
        case BrokerPruefFehler::thumbprintUngueltig: return "thumbprintUngueltig";
        case BrokerPruefFehler::signaturFehltOderUngueltig:
            return "signaturFehltOderUngueltig";
        case BrokerPruefFehler::signerFalsch: return "signerFalsch";
    }
    return "unbekannt";
}

// ═════════════════════════════════════════════════════════════════════════
// NAK-134 — `ERROR_PIPE_BUSY` ist Liveness, kein Sicherheitsurteil
//
// Die drei Werkzeuge dieses Abschnitts stellen die Oeffnungsausgaenge
// DETERMINISTISCH her, ohne Fehlerinjektion im Produktpfad. Welchen Win32-Code
// jede Lage liefert, ist an dieser Maschine gemessen und nicht geraten:
//
//   * genau eine Instanz, von uns selbst besetzt  ⇒ 231 ERROR_PIPE_BUSY
//   * `PIPE_ACCESS_INBOUND`, Client will RW       ⇒   5 ERROR_ACCESS_DENIED
//   * Name existiert nicht                        ⇒   2 ERROR_FILE_NOT_FOUND
//   * `<name>\..\..`                              ⇒ 123 ERROR_INVALID_NAME
//
// Der letzte Fall ist der einzige an `CreateFileW` erreichbare "sonstige"
// Fehler; Sternchen, Fragezeichen, ueberlange und geschachtelte Namen liefern
// alle wieder 2 (gemessen, Phase 2).

/// Haelt genau EINE Pipeinstanz und besetzt sie selbst. Jeder weitere
/// `CreateFileW` bekommt `ERROR_PIPE_BUSY`, und `WaitNamedPipeW` laeuft in
/// sein Zeitlimit, weil nie eine Instanz frei wird — die Lage aus dem
/// Gate-Lauf G3, nur ohne 32 Sonden.
struct BelegtePipe
{
    HANDLE server = INVALID_HANDLE_VALUE;
    HANDLE besetzer = INVALID_HANDLE_VALUE;

    bool anlegen (const std::string& name)
    {
        const std::wstring breit (name.begin(), name.end());
        server = CreateNamedPipeW (
            breit.c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
            1, 4096, 4096, 0, nullptr);
        if (server == INVALID_HANDLE_VALUE)
            return false;
        besetzer = CreateFileW (breit.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                                OPEN_EXISTING,
                                SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION, nullptr);
        return besetzer != INVALID_HANDLE_VALUE;
    }
    ~BelegtePipe()
    {
        if (besetzer != INVALID_HANDLE_VALUE) CloseHandle (besetzer);
        if (server != INVALID_HANDLE_VALUE)
        {
            DisconnectNamedPipe (server);
            CloseHandle (server);
        }
    }
};

/// Eine nur EINGEHENDE Pipe. Der Client oeffnet mit GENERIC_READ|GENERIC_WRITE
/// und bekommt `ERROR_ACCESS_DENIED` — der Fall "der Name gehoert jemand
/// anderem" ohne einen zweiten Benutzer im Test.
struct FremdePipe
{
    HANDLE server = INVALID_HANDLE_VALUE;

    bool anlegen (const std::string& name)
    {
        const std::wstring breit (name.begin(), name.end());
        server = CreateNamedPipeW (
            breit.c_str(), PIPE_ACCESS_INBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
            1, 4096, 4096, 0, nullptr);
        return server != INVALID_HANDLE_VALUE;
    }
    /// Gibt den Namen WIEDER FREI. Gebraucht wird das vom Park-Reset-Test:
    /// nach dem Loesen der Sperre muss der naechste Versuch schlicht
    /// scheitern duerfen — bliebe der Name belegt, parkte der Thread sofort
    /// erneut und der Backoff, um den es geht, kaeme gar nicht zum Einsatz.
    void schliessen()
    {
        if (server != INVALID_HANDLE_VALUE)
            CloseHandle (server);
        server = INVALID_HANDLE_VALUE;
    }
    ~FremdePipe() { schliessen(); }
};

/// Ein Name, den Win32 als ungueltig zurueckweist (123) — der einzige an
/// `CreateFileW` erreichbare "sonstige" Fehler. Er bleibt im Probe-Namensraum.
std::string ungueltigerName (const char* fall)
{
    return testPipeName (fall) + "\\..\\..";
}

/// W-H3: `backoffMs` ist eine lokale Variable in `threadLauf` und von aussen
/// nicht lesbar. Belegbar ist nur, WANN ein neuer Oeffnungsversuch beginnt —
/// `verbindungsVersuche` steigt in der Eintrittssperre von `eineVerbindung`.
int weitereVersucheIn (const std::function<int()>& versuche, int fensterMs)
{
    const int vorher = versuche();
    std::this_thread::sleep_for (std::chrono::milliseconds (fensterMs));
    return versuche() - vorher;
}

/// Die Zeitpunkte der naechsten `anzahl` Oeffnungsversuche, relativ zum
/// Aufruf. Ein Wert < 0 heisst: der Versuch kam innerhalb `deckelMs` nicht.
std::vector<long long> versuchsStempel (const std::function<int()>& versuche,
                                        int anzahl, int deckelMs)
{
    std::vector<long long> stempel;
    const auto start = std::chrono::steady_clock::now();
    int stand = versuche();
    while (static_cast<int> (stempel.size()) < anzahl)
    {
        const auto jetzt = std::chrono::steady_clock::now();
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds> (
            jetzt - start).count();
        if (ms > deckelMs)
        {
            while (static_cast<int> (stempel.size()) < anzahl) stempel.push_back (-1);
            break;
        }
        const int neu = versuche();
        if (neu > stand)
        {
            for (int i = 0; i < neu - stand && static_cast<int> (stempel.size()) < anzahl; ++i)
                stempel.push_back (ms);
            stand = neu;
            continue;
        }
        std::this_thread::sleep_for (std::chrono::milliseconds (10));
    }
    return stempel;
}

TelemetryHello nak134TelemetryHello()
{
    TelemetryHello h;
    h.adresse = testAdresse (hex32 ('a'));
    h.linkId = hex32 ('c');
    h.challenge = hex32 ('d');
    return h;
}

/// Ein v3-Client hinter EINER Schnittstelle.
///
/// `ControlClient` und `TelemetryClient` fahren dieselbe `threadLauf`-Schleife
/// und tragen dieselben Snapshotfelder, haben aber keine gemeinsame
/// Basisklasse. Die Matrix verlangt jede D-P-Zeile trotzdem auf BEIDEN Pfaden
/// (§5.2) — ohne diesen Wrapper stuende jede Probe zweimal fast gleich da, und
/// genau daran ist in Runde 1 der Telemetriepfad durchgefallen (Defekt 3).
struct V3Pfad
{
    std::unique_ptr<ControlClient>   control;
    std::unique_ptr<TelemetryClient> telemetrie;
    const bool istTelemetrie;

    V3Pfad (bool telemetriePfad, const std::string& pipe,
            ServerErwartung erwartung = serverErwartungFuerEigenprozessTest(),
            std::function<TelemetryHello()> telemetrieHello = {})
        : istTelemetrie (telemetriePfad)
    {
        if (istTelemetrie)
            telemetrie.reset (new TelemetryClient (
                telemetrieHello ? telemetrieHello
                                : std::function<TelemetryHello()> (
                                      [] { return nak134TelemetryHello(); }),
                pipe, {}, std::move (erwartung)));
        else
            control.reset (new ControlClient (
                [] { return nak123ControlHello(); }, pipe, {}, {}, {}, {},
                std::move (erwartung)));
    }

    void start()     { if (istTelemetrie) telemetrie->start();     else control->start(); }
    void stop()      { if (istTelemetrie) telemetrie->stop();      else control->stop(); }
    void reconnect() { if (istTelemetrie) telemetrie->reconnect(); else control->reconnect(); }

    int versuche() const
    {
        return istTelemetrie ? telemetrie->snapshot().verbindungsVersuche
                             : control->snapshot().verbindungsVersuche;
    }
    bool verbunden() const
    {
        return istTelemetrie
            ? telemetrie->snapshot().status == TelemetryClient::Status::verbunden
            : control->snapshot().status == ControlClient::Status::verbunden;
    }
    std::uint64_t pruefungen() const
    {
        return istTelemetrie ? telemetrie->snapshot().serverPruefungen
                             : control->snapshot().serverPruefungen;
    }
    ServerPruefStatus pruefstatus() const
    {
        return istTelemetrie ? telemetrie->snapshot().serverPruefstatus
                             : control->snapshot().serverPruefstatus;
    }
    const char* name() const { return istTelemetrie ? "telemetrie" : "control"; }
};

/// D-K10 bis D-K26 auf den beiden v3-Pfaden, je Ausgang ein eigener
/// Pruefpunkt. Gemessen werden Status, Fehler, `brokerPipeFehlt`,
/// `serverPruefungen` UND die Phasenwirkung: laeuft der Backoff weiter oder
/// parkt der Thread?
void oeffnungsausgaenge_sind_liveness_oder_sicherheit()
{
    struct Fall
    {
        const char* name;
        ServerPruefStatus status;
        ServerPruefFehler fehler;
        bool brokerFehlt;
        std::uint64_t pruefungenErsterVersuch;
        bool parkt;
    };
    const std::vector<Fall> faelle {
        // D-K18/D-K19 — der Defekt. Vor R1: belegtAberUnverifiziert + Parken.
        { "pipe_busy", ServerPruefStatus::belegtNichtErreicht,
          ServerPruefFehler::pipeBelegt, false, 0, false },
        // D-K16 — unveraendert.
        { "file_not_found", ServerPruefStatus::nichtDa,
          ServerPruefFehler::pipeFehlt, true, 0, false },
        // D-K21/D-K22 — die NAK-123-Zusage bleibt.
        { "access_denied", ServerPruefStatus::belegtAberUnverifiziert,
          ServerPruefFehler::pipeOeffnen, false, 1, true },
        // D-K24/D-K25 — jeder UNBEKANNTE Fehler bleibt fail-closed.
        { "sonstiger_fehler", ServerPruefStatus::belegtAberUnverifiziert,
          ServerPruefFehler::pipeOeffnen, false, 1, true },
    };

    for (const auto& f : faelle)
    {
        const std::string fall (f.name);
        // ── Control ──────────────────────────────────────────────────────
        {
            BelegtePipe belegt;
            FremdePipe fremd;
            std::string pipe;
            bool aufbau = true;
            if (fall == "pipe_busy")       { pipe = testPipeName ("k-busy-c");  aufbau = belegt.anlegen (pipe); }
            else if (fall == "access_denied") { pipe = testPipeName ("k-denied-c"); aufbau = fremd.anlegen (pipe); }
            else if (fall == "sonstiger_fehler") pipe = ungueltigerName ("k-invalid-c");
            else                              pipe = testPipeName ("k-fehlt-c");

            const auto t0 = std::chrono::steady_clock::now();
            ControlClient client ([] { return nak123ControlHello(); }, pipe);
            client.start();
            const bool klassifiziert = warteAuf (9000, [&] {
                const auto s = client.snapshot();
                return s.status == ControlClient::Status::getrennt
                    && s.serverPruefstatus == f.status
                    && s.serverPrueffehler == f.fehler;
            });
            const auto dauerMs = std::chrono::duration_cast<std::chrono::milliseconds> (
                std::chrono::steady_clock::now() - t0).count();
            const auto nachFehler = client.snapshot();
            // Der Backoff beginnt bei 500 ms. Ein Fenster von 1.500 ms trennt
            // "parkt" (0 weitere Versuche) von "Backoff laeuft" (mindestens
            // einer) — ohne eine Obergrenze zu behaupten.
            const int weitere = weitereVersucheIn (
                [&] { return client.snapshot().verbindungsVersuche; }, 1500);
            client.stop();

            const bool phase = f.parkt ? weitere == 0 : weitere >= 1;
            // Nur `pipe_busy` faehrt die volle Warteschleife; die untere
            // Schranke aus §7 gilt deshalb nur dort.
            const bool frist = fall != "pipe_busy" || dauerMs >= 4000;
            pruefe (aufbau && klassifiziert && phase && frist
                        && nachFehler.brokerPipeFehlt == f.brokerFehlt
                        && nachFehler.serverPruefungen == f.pruefungenErsterVersuch,
                    ("oeffnungsausgaenge_sind_liveness_oder_sicherheit/control/" + fall).c_str(),
                    "klass " + std::to_string (klassifiziert) + ", " + std::to_string (dauerMs)
                        + " ms, weitere Versuche " + std::to_string (weitere)
                        + " (erwartet " + (f.parkt ? "0" : ">=1") + "), pruefungen "
                        + std::to_string (nachFehler.serverPruefungen) + " (erwartet "
                        + std::to_string (f.pruefungenErsterVersuch) + ")");
        }
        // ── Telemetrie ───────────────────────────────────────────────────
        {
            BelegtePipe belegt;
            FremdePipe fremd;
            std::string pipe;
            bool aufbau = true;
            if (fall == "pipe_busy")       { pipe = testPipeName ("k-busy-t");  aufbau = belegt.anlegen (pipe); }
            else if (fall == "access_denied") { pipe = testPipeName ("k-denied-t"); aufbau = fremd.anlegen (pipe); }
            else if (fall == "sonstiger_fehler") pipe = ungueltigerName ("k-invalid-t");
            else                              pipe = testPipeName ("k-fehlt-t");

            TelemetryClient client ([] { return nak134TelemetryHello(); }, pipe);
            client.start();
            const bool klassifiziert = warteAuf (9000, [&] {
                const auto s = client.snapshot();
                return s.status == TelemetryClient::Status::getrennt
                    && s.serverPruefstatus == f.status
                    && s.serverPrueffehler == f.fehler;
            });
            const auto nachFehler = client.snapshot();
            const int weitere = weitereVersucheIn (
                [&] { return client.snapshot().verbindungsVersuche; }, 1500);
            client.stop();

            const bool phase = f.parkt ? weitere == 0 : weitere >= 1;
            pruefe (aufbau && klassifiziert && phase
                        && nachFehler.serverPruefungen == f.pruefungenErsterVersuch,
                    ("oeffnungsausgaenge_sind_liveness_oder_sicherheit/telemetrie/" + fall).c_str(),
                    "klass " + std::to_string (klassifiziert)
                        + ", weitere Versuche " + std::to_string (weitere)
                        + " (erwartet " + (f.parkt ? "0" : ">=1") + "), pruefungen "
                        + std::to_string (nachFehler.serverPruefungen));
        }
    }
}

/// D-K10, D-K11 — die zwei Ausgaenge, die in Phase 1 der Matrix ganz fehlten.
/// Beide entstehen in `namedPipeServerAuthentisieren` und bleiben
/// Sicherheitsfaelle: `authFehler` setzt IMMER `belegtAberUnverifiziert`.
///
/// **Nacharbeit Runde 1, Defekt 3:** die Matrixzeilen nennen `V3C, V3T, V2`,
/// gemessen war nur Control. Der Telemetriepfad laeuft jetzt in derselben
/// Fallschleife mit; der v2-Pfad liegt in
/// `PipeClientLifecycleTestMain.cpp/pipeclient_authausgaenge_und_welcome`.
void authausgaenge_erwartung_und_serverpid_bleiben_sicherheit()
{
    struct Fall { const char* name; ServerPruefFehler fehler; bool ueberErwartung; };
    const std::vector<Fall> faelle {
        { "erwartung_ungueltig", ServerPruefFehler::erwartungUngueltig, true },
        { "server_pid_nicht_ermittelbar", ServerPruefFehler::serverPidNichtErmittelbar, false },
    };
    for (const auto& f : faelle)
    for (const bool ueberTelemetrie : { false, true })
    {
        const std::string pfad (ueberTelemetrie ? "telemetrie" : "control");
        const auto pipe = testPipeName ((std::string (f.name) + "-" + pfad).c_str());
        TestServer server (pipe);
        const bool steht = server.starten();
        ServerErwartung erwartung;
        if (f.ueberErwartung)
        {
            // Kein `testFehler`-Weg noetig: eine leere Erwartung IST der Fall.
            erwartung = ServerErwartung {};
        }
        else
        {
            erwartung = testExeErwartung (GetCurrentProcessId());
            erwartung.testFehler = f.fehler;
        }

        std::unique_ptr<ControlClient> control;
        std::unique_ptr<TelemetryClient> telemetrie;
        if (ueberTelemetrie)
            telemetrie.reset (new TelemetryClient (
                [&server] {
                    TelemetryHello h;
                    h.adresse = testAdresse (hex32 ('a'));
                    h.linkId = server.kopplungLinkId();
                    h.challenge = server.kopplungChallenge();
                    return h;
                }, pipe, {}, erwartung));
        else
            control.reset (new ControlClient (
                [] { return nak123ControlHello(); }, pipe, {}, {}, {}, {}, erwartung));

        auto pruefstatus = [&] {
            return ueberTelemetrie ? telemetrie->snapshot().serverPruefstatus
                                   : control->snapshot().serverPruefstatus;
        };
        auto prueffehler = [&] {
            return ueberTelemetrie ? telemetrie->snapshot().serverPrueffehler
                                   : control->snapshot().serverPrueffehler;
        };
        auto pruefungen = [&] {
            return ueberTelemetrie ? telemetrie->snapshot().serverPruefungen
                                   : control->snapshot().serverPruefungen;
        };
        auto versuche = [&] {
            return ueberTelemetrie ? telemetrie->snapshot().verbindungsVersuche
                                   : control->snapshot().verbindungsVersuche;
        };
        if (ueberTelemetrie) telemetrie->start(); else control->start();
        const bool fiel = warteAuf (4000, [&] {
            return pruefstatus() == ServerPruefStatus::belegtAberUnverifiziert
                && prueffehler() == f.fehler
                && pruefungen() == 1;
        });
        const int weitere = weitereVersucheIn (versuche, 1500);
        if (ueberTelemetrie) telemetrie->stop(); else control->stop();
        server.stoppen();
        pruefe (steht && fiel && weitere == 0 && serverHatKeinHello (server),
                ("authausgaenge_erwartung_und_serverpid_bleiben_sicherheit/" + pfad
                 + "/" + f.name).c_str(),
                "fiel " + std::to_string (fiel) + ", weitere Versuche "
                    + std::to_string (weitere) + ", Fehler "
                    + std::to_string (static_cast<int> (prueffehler())));
    }
}

/// D-K08 — die Telemetrie parkt bei einem Authfehler genauso wie Control.
/// Der bisher zitierte `control_und_telemetry_authentisieren_jedes_handle`
/// misst nur den ERFOLGSfall.
void telemetrie_authfehler_parkt_wie_control()
{
    const auto pipe = testPipeName ("k-tele-auth");
    TestServer server (pipe);
    const bool steht = server.starten();
    auto erwartung = testExeErwartung (GetCurrentProcessId());
    erwartung.testFehler = ServerPruefFehler::hashFalsch;
    TelemetryClient client ([] { return nak134TelemetryHello(); }, pipe, {}, erwartung);
    client.start();
    const bool fiel = warteAuf (4000, [&] {
        const auto s = client.snapshot();
        return s.status == TelemetryClient::Status::getrennt
            && s.serverPruefstatus == ServerPruefStatus::belegtAberUnverifiziert
            && s.serverPrueffehler == ServerPruefFehler::hashFalsch
            && s.serverPruefungen == 1;
    });
    const int weitere = weitereVersucheIn (
        [&] { return client.snapshot().verbindungsVersuche; }, 1500);
    client.stop();
    server.stoppen();
    pruefe (steht && fiel && weitere == 0,
            "telemetrie_authfehler_parkt_wie_control",
            "fiel " + std::to_string (fiel) + ", weitere Versuche "
                + std::to_string (weitere));
}

/// D-P01, D-P02, D-P05, D-P06 und W-H3 auf beiden v3-Pfaden: die beobachtbare
/// Wartefolge 500 → 1.000 → 2.000 → 4.000 → 8.000 → 8.000. Gemessen werden die
/// ABSTAENDE zwischen Oeffnungsversuchen, nicht eine interne Variable (E9).
///
/// Die volle Folge bis zum Deckel kostet 23,5 s reine Wartezeit. Sie wird
/// deshalb EINMAL gefahren, auf dem Control-Pfad mit `file_not_found`; die
/// uebrigen Faelle messen die Verdopplung ueber die ersten drei Abstaende.
///
/// **Nacharbeit Runde 1, Defekt 3.** Vorher fuhr diese Funktion ausschliesslich
/// Control, obwohl alle vier Zeilen `V3C, V3T, V2` nennen; und der
/// Erfolgs-Reset D-P01 fehlte ganz — die Zeile war als „gemessen" gefuehrt,
/// ohne dass irgendein Fall je eine STEHENDE Verbindung hergestellt haette.
/// Beides ist hier nachgezogen. `stehende_verbindung_setzt_zurueck` beginnt
/// ausdruecklich aus einer ERHOEHTEN Backoff-Stufe: nur so kann ein fehlender
/// Reset ueberhaupt sichtbar werden.
void backoff_folge_und_deckel_sind_beobachtbar()
{
    // Toleranz nach OBEN, nicht nach unten: `wait_for` kehrt nie frueher als
    // die Wartezeit zurueck, Scheduling und der Oeffnungsversuch kommen oben
    // drauf. Nach unten genuegen 10 % fuer die 10-ms-Granularitaet der
    // Beobachtung. Eine untere Schranke von 3/5 liesse einen auf 5.000 ms
    // verkuerzten Deckel als 8.000 durchgehen.
    auto imFenster = [] (long long ist, long long soll) {
        return ist >= soll * 9 / 10 && ist <= soll * 7 / 5 + 200;
    };

    // ── Fall `file_not_found`, Control: volle Folge inklusive Deckel ──────
    {
        const auto pipe = testPipeName ("k-backoff-voll");
        ControlClient client ([] { return nak123ControlHello(); }, pipe);
        client.start();
        auto versuche = [&] { return client.snapshot().verbindungsVersuche; };
        warteAuf (2000, [&] { return versuche() >= 1; });
        // Deckel 45 s, nicht 30 s: die Folge braucht nominal
        // 500+1.000+2.000+4.000+8.000+8.000 = 23,5 s, und die Toleranz oben
        // laesst je Abstand bis zu 40 % mehr zu. Ein Deckel von 30 s koennte
        // damit einen voellig gesunden Lauf abschneiden und den Kanon auf
        // einer ausgelasteten Maschine flackern lassen.
        const auto s = versuchsStempel (versuche, 6, 45000);
        client.stop();
        bool alleDa = true;
        for (auto v : s) alleDa = alleDa && v >= 0;
        const bool folge = alleDa
            && imFenster (s[0], 500) && imFenster (s[1] - s[0], 1000)
            && imFenster (s[2] - s[1], 2000) && imFenster (s[3] - s[2], 4000)
            && imFenster (s[4] - s[3], 8000) && imFenster (s[5] - s[4], 8000);
        std::string detail;
        for (std::size_t i = 0; i < s.size(); ++i)
            detail += (i ? " " : "") + std::to_string (i ? s[i] - s[i - 1] : s[i]);
        pruefe (folge, "backoff_folge_und_deckel_sind_beobachtbar/control/file_not_found",
                "Abstaende " + detail + " ms (erwartet 500 1000 2000 4000 8000 8000)");
    }

    // ── Fall `file_not_found`, Telemetrie: die ersten drei Abstaende ──────
    //    Der Deckel ist an beiden Stellen derselbe Ausdruck
    //    `min (backoffMs * 2, kBackoffMaxMs)`; die volle Folge kostet 23,5 s
    //    reine Wartezeit und laeuft deshalb nur auf dem Control-Pfad.
    {
        const auto pipe = testPipeName ("k-backoff-voll-t");
        V3Pfad pfad (true, pipe);
        pfad.start();
        auto versuche = [&] { return pfad.versuche(); };
        warteAuf (2000, [&] { return versuche() >= 1; });
        const auto s = versuchsStempel (versuche, 3, 15000);
        pfad.stop();
        bool alleDa = true;
        for (auto v : s) alleDa = alleDa && v >= 0;
        const bool folge = alleDa && imFenster (s[0], 500)
                        && imFenster (s[1] - s[0], 1000) && imFenster (s[2] - s[1], 2000);
        pruefe (folge, "backoff_folge_und_deckel_sind_beobachtbar/telemetrie/file_not_found",
                "Abstaende " + (alleDa ? std::to_string (s[0]) + " "
                                             + std::to_string (s[1] - s[0]) + " "
                                             + std::to_string (s[2] - s[1])
                                       : std::string ("unvollstaendig"))
                    + " ms (erwartet 500 1000 2000)");
    }

    // ── Fall `pipe_busy`: der BELEG. Vor R1 gibt es hier gar keine Folge ──
    for (const bool telemetriePfad : { false, true })
    {
        const auto pipe = testPipeName (telemetriePfad ? "k-backoff-busy-t"
                                                       : "k-backoff-busy");
        BelegtePipe belegt;
        const bool aufbau = belegt.anlegen (pipe);
        V3Pfad pfad (telemetriePfad, pipe);
        pfad.start();
        auto versuche = [&] { return pfad.versuche(); };
        // Jeder Versuch kostet hier zusaetzlich die 4-s-Warteschleife.
        const auto s = versuchsStempel (versuche, 3, 30000);
        pfad.stop();
        bool alleDa = true;
        for (auto v : s) alleDa = alleDa && v >= 0;
        // Zwei aufeinanderfolgende Versuche muessen mindestens die
        // Warteschleife plus den wachsenden Backoff auseinanderliegen —
        // gemessen wird die untere Schranke, nie eine Obergrenze.
        const bool folge = alleDa && (s[1] - s[0]) >= 4000 && (s[2] - s[1]) >= 4000
                        && (s[2] - s[1]) > (s[1] - s[0]);
        std::string detail;
        for (std::size_t i = 0; i < s.size(); ++i)
            detail += (i ? " " : "") + std::to_string (s[i]);
        pruefe (aufbau && folge,
                (std::string ("backoff_folge_und_deckel_sind_beobachtbar/")
                 + pfad.name() + "/pipe_busy").c_str(),
                "Stempel " + detail + " ms — vor R1 parkt der Thread nach dem ersten");
    }

    // ── D-K04/D-K05, D-P02: verifiziert, aber Welcome bleibt aus ⇒ KEINE
    //    Ruecksetzung. Je Pfad ein Fall; die Telemetriehaelfte fehlte.
    for (const bool telemetriePfad : { false, true })
    {
        const auto pipe = testPipeName (telemetriePfad ? "k-backoff-welcome-t"
                                                       : "k-backoff-welcome");
        // Ein Server, der annimmt und das Hello liest, aber nie ein welcome
        // schickt: `eineVerbindung` gibt `false`, der Backoff verdoppelt.
        TestServer stumm (pipe);
        stumm.welcomeAusbleiben.store (true);
        const bool aufbau = stumm.starten();
        V3Pfad pfad (telemetriePfad, pipe, testExeErwartung (GetCurrentProcessId()),
                     [&stumm] {
                         TelemetryHello h;
                         h.adresse = testAdresse (hex32 ('a'));
                         h.linkId = stumm.kopplungLinkId();
                         h.challenge = stumm.kopplungChallenge();
                         return h;
                     });
        pfad.start();
        auto versuche = [&] { return pfad.versuche(); };
        const auto s = versuchsStempel (versuche, 3, 40000);
        const auto pruefungen = pfad.pruefungen();
        pfad.stop();
        stumm.stoppen();
        bool alleDa = true;
        for (auto v : s) alleDa = alleDa && v >= 0;
        // Waere die Ruecksetzung an den Auth-Erfolg gebunden statt an die
        // Rueckgabe, blieben die Abstaende bei 500 ms stehen.
        const bool waechst = alleDa && (s[2] - s[1]) > (s[1] - s[0]);
        pruefe (aufbau && waechst && pruefungen >= 2,
                (std::string ("verifiziert_ohne_welcome_setzt_backoff_nicht_zurueck/")
                 + pfad.name()).c_str(),
                "Stempel " + (alleDa ? std::to_string (s[0]) + " " + std::to_string (s[1])
                                           + " " + std::to_string (s[2])
                                     : std::string ("unvollstaendig"))
                    + " ms, pruefungen " + std::to_string (pruefungen));
    }

    // ── D-P01: eine Verbindung STAND und endete regulaer ⇒ der Backoff faellt
    //    aus einer ERHOEHTEN Stufe auf 500 ms zurueck.
    //
    //    Der Aufbau ist der Kern der Zeile: erst ohne Server zwei Verdopplungen
    //    fahren (500 → 1.000 → 2.000), dann den Server starten, damit die
    //    naechste Runde WIRKLICH verbindet, dann den Server schliessen. Ein
    //    fehlender Reset wuerde die naechste Runde erst nach >= 2.000 ms
    //    beginnen. Ohne die erhoehte Vorstufe waere die Messung wertlos: bei
    //    500 ms Ausgangsbackoff sind „zurueckgesetzt" und „nicht
    //    zurueckgesetzt" nicht unterscheidbar.
    for (const bool telemetriePfad : { false, true })
    {
        const auto pipe = testPipeName (telemetriePfad ? "k-backoff-reset-t"
                                                       : "k-backoff-reset");
        TestServer server (pipe);          // absichtlich NOCH NICHT gestartet
        V3Pfad pfad (telemetriePfad, pipe, testExeErwartung (GetCurrentProcessId()),
                     [&server] {
                         TelemetryHello h;
                         h.adresse = testAdresse (hex32 ('a'));
                         h.linkId = server.kopplungLinkId();
                         h.challenge = server.kopplungChallenge();
                         return h;
                     });
        pfad.start();
        auto versuche = [&] { return pfad.versuche(); };
        // 1) Aufstieg: drei Versuche, Abstaende 500 und 1.000 ⇒ `backoffMs`
        //    steht danach auf 2.000.
        const auto aufstieg = versuchsStempel (versuche, 3, 12000);
        bool alleDa = true;
        for (auto v : aufstieg) alleDa = alleDa && v >= 0;
        const bool gestiegen = alleDa && (aufstieg[2] - aufstieg[1]) >= 900;

        // 2) Server an — die naechste Runde verbindet wirklich.
        const bool serverSteht = server.starten();
        const bool stand = warteAuf (20000, [&] { return pfad.verbunden(); });

        // 3) Verbindung regulaer beenden und messen, WANN die naechste Runde
        //    beginnt. 500 ms nominal; ein stehengebliebener Backoff koennte
        //    1.200 ms nicht unterbieten, ein faelschlich genullter nicht
        //    350 ms ueberschreiten.
        //
        //    Der Anker ist der Moment, in dem der Client das Ende SIEHT, nicht
        //    der, in dem der Server schliesst — sonst mischt sich die
        //    Erkennungslatenz in die Messung. Auf dem v2-Zwilling sind das
        //    volle 1.000 ms (`kHeartbeatMs`), hier nur der Lesetakt; derselbe
        //    Anker haelt beide Proben vergleichbar.
        const int vorEnde = versuche();
        server.stoppen();
        const bool endeGesehen = warteAuf (5000, [&] { return ! pfad.verbunden(); });
        const auto t0 = std::chrono::steady_clock::now();
        const bool neueRunde = endeGesehen
                            && warteAuf (2500, [&] { return versuche() > vorEnde; });
        const auto verzug = std::chrono::duration_cast<std::chrono::milliseconds> (
            std::chrono::steady_clock::now() - t0).count();
        pfad.stop();

        pruefe (gestiegen && serverSteht && stand && endeGesehen && neueRunde
                    && verzug >= 350 && verzug <= 1200,
                (std::string ("backoff_folge_und_deckel_sind_beobachtbar/")
                 + pfad.name() + "/stehende_verbindung_setzt_zurueck").c_str(),
                "Aufstieg " + (alleDa ? std::to_string (aufstieg[1] - aufstieg[0]) + " "
                                            + std::to_string (aufstieg[2] - aufstieg[1])
                                      : std::string ("unvollstaendig"))
                    + " ms, Verbindung stand " + std::to_string (stand)
                    + ", naechste Runde nach " + std::to_string (verzug)
                    + " ms (erwartet 350..1200, ohne Reset >= 2000)");
    }
}

/// D-P03, D-P07 — die Phasenaussage des Parkens. Sie kann kein Backoff-Test
/// messen: geparkt gibt es GAR KEINE Folge. Belegt werden drei Dinge:
///   1. nach dem Ausgang folgt kein Versuch mehr (Backoff uebergangen);
///   2. nach `reconnect()` folgt der naechste Versuch SOFORT, ohne Wartezeit;
///   3. der DARAUF FOLGENDE Versuch kommt nach 500 ms — NICHT nach der
///      erhoehten Stufe, aus der heraus geparkt wurde.
///
/// Punkt 3 ist der eigentliche Beleg fuer die Ruecksetzung von `backoffMs`
/// (`ControlClient.cpp:1010`, `TelemetryClient.cpp:420`). Punkt 2 ist es
/// NICHT: das `continue` im `authBlockiert`-Zweig ist bedingungslos
/// (`ControlClient.cpp:1011`, `TelemetryClient.cpp:421`), der erste Versuch
/// nach dem Loesen folgt also ohne Wartezeit, welchen Wert `backoffMs` auch
/// traegt. Erst die Runde DANACH benutzt ihn, im `wait_for (backoffMs)`
/// unterhalb des Zweigs.
///
/// Daraus folgt der Aufbau, und er ist der Kern der Zeile:
///   a) OHNE Sperrursache aufsteigen — drei Versuche mit Abstaenden 500 und
///      1.000 ms, danach steht `backoffMs` auf 4.000. Ohne diese Vorstufe
///      sind „zurueckgesetzt" und „stehengeblieben" beide 500 ms und damit
///      nicht unterscheidbar; genau daran ist die Zeile in Runde 1
///      durchgefallen;
///   b) Sperrursache anlegen ⇒ der naechste Versuch parkt aus der erhoehten
///      Stufe;
///   c) Sperrursache WIEDER ENTFERNEN — bliebe sie stehen, parkte der Thread
///      nach dem Loesen sofort erneut und erreichte das `wait_for` nie;
///   d) `reconnect()`, dann den Abstand vom ersten zum ZWEITEN Versuch messen.
///
/// **Nacharbeit Runde 1, Defekt 3:** D-P03 und D-P07 nennen `V3C, V3T, V2`,
/// gemessen war nur Control. Der Telemetriepfad laeuft jetzt mit.
/// **Nacharbeit Runde 2:** der Test parkte aus `backoffMs == 500` und mass nur
/// den sofortigen Versuch. Ein entfernter Reset blieb dabei gruen — die Zusage
/// „erhoehter Backoff faellt beim Loesen auf 500" war nicht falsifizierbar.
/// Punkt 3 und die Schritte a bis c schliessen das, auf BEIDEN Pfaden: der
/// `authBlockiert`-Zweig ist in Control und Telemetrie derselbe Ausdruck.
void parken_uebergeht_den_backoff()
{
    struct Fall { const char* name; bool ueberAuth; };
    for (const auto& f : std::vector<Fall> { { "authfehler", true },
                                             { "access_denied", false } })
    for (const bool ueberTelemetrie : { false, true })
    {
        const std::string fall (f.name);
        const std::string pfad (ueberTelemetrie ? "telemetrie" : "control");
        const auto pipe = testPipeName (("k-parken-" + fall + "-" + pfad).c_str());
        TestServer server (pipe);
        FremdePipe fremd;
        ServerErwartung erwartung;
        // Die Erwartung geht in den Konstruktor und muss deshalb JETZT stehen.
        // Die Sperrursache selbst entsteht erst nach dem Aufstieg (Schritt b).
        if (f.ueberAuth)
        {
            erwartung = testExeErwartung (GetCurrentProcessId());
            erwartung.testFehler = ServerPruefFehler::signerFalsch;
        }

        std::unique_ptr<ControlClient> control;
        std::unique_ptr<TelemetryClient> telemetrie;
        if (ueberTelemetrie)
            telemetrie.reset (new TelemetryClient (
                [&server] {
                    TelemetryHello h;
                    h.adresse = testAdresse (hex32 ('a'));
                    h.linkId = server.kopplungLinkId();
                    h.challenge = server.kopplungChallenge();
                    return h;
                }, pipe, {}, erwartung));
        else
            control.reset (new ControlClient (
                [] { return nak123ControlHello(); }, pipe, {}, {}, {}, {}, erwartung));

        auto versuche = [&] {
            return ueberTelemetrie ? telemetrie->snapshot().verbindungsVersuche
                                   : control->snapshot().verbindungsVersuche;
        };
        auto pruefstatus = [&] {
            return ueberTelemetrie ? telemetrie->snapshot().serverPruefstatus
                                   : control->snapshot().serverPruefstatus;
        };
        if (ueberTelemetrie) telemetrie->start(); else control->start();

        // a) Aufstieg gegen den noch NICHT belegten Namen. Drei Versuche,
        //    Abstaende 500 und 1.000 ms ⇒ `backoffMs` steht danach auf 4.000.
        const auto aufstieg = versuchsStempel (versuche, 3, 12000);
        bool alleDa = true;
        for (auto v : aufstieg) alleDa = alleDa && v >= 0;
        const bool gestiegen = alleDa && (aufstieg[2] - aufstieg[1]) >= 900;

        // b) Erst JETZT die Sperrursache. Das Fenster bis zum naechsten
        //    Versuch ist 2.000 ms breit; verpasst der Aufbau es doch, parkt
        //    der Pfad eine Stufe spaeter — auch das bleibt >= 2.000 ms und
        //    damit diskriminierend, deshalb wartet die Parkprobe grosszuegig.
        const bool aufbau = f.ueberAuth ? server.starten() : fremd.anlegen (pipe);
        const bool geparkt = warteAuf (20000, [&] {
            return pruefstatus() == ServerPruefStatus::belegtAberUnverifiziert;
        });
        // Bei welchem Versuch wurde geparkt? Nach k gescheiterten Versuchen
        // steht `backoffMs` auf `500 * 2^k`; der Parkversuch selbst aendert
        // ihn nicht (das `continue` ueberspringt die Verdopplung). Ab dem
        // vierten Versuch ist der stale Wert also >= 4.000 ms — genau das
        // macht Punkt 3 unten ueberhaupt erst zu einer Aussage.
        const int versucheBeimParken = versuche();
        // 1. kein weiterer Versuch — deutlich laenger als der Startbackoff.
        const int weitere = weitereVersucheIn (versuche, 1500);

        // c) Sperrursache loesen. Der naechste Versuch soll schlicht
        //    scheitern (`nichtDa`), nicht erneut in den Parkzweig laufen —
        //    sonst bliebe `backoffMs` ungenutzt und unbeobachtbar.
        if (f.ueberAuth) server.stoppen(); else fremd.schliessen();

        // 2. reconnect() loest die Sperre; der naechste Versuch kommt OHNE
        //    Wartezeit. 250 ms sind die halbe Startwartezeit.
        const int vorReconnect = versuche();
        const auto t0 = std::chrono::steady_clock::now();
        if (ueberTelemetrie) telemetrie->reconnect(); else control->reconnect();
        const bool sofort = warteAuf (250, [&] { return versuche() > vorReconnect; });
        const auto verzug = std::chrono::duration_cast<std::chrono::milliseconds> (
            std::chrono::steady_clock::now() - t0).count();

        // 3. d) DER Reset-Beweis: der Versuch DANACH ist der erste, der
        //    `backoffMs` wirklich benutzt. 500 ms nominal, Fenster wie in
        //    D-P01. Ein stehengebliebener Backoff braeuchte >= 4.000 ms, ein
        //    faelschlich genullter kaeme unter 350 ms. Das Wartefenster ist
        //    absichtlich breiter als der Deckel, damit der Rotlauf den
        //    stalen Wert MISST statt nur ein Ausbleiben zu melden.
        const int nachSofort = versuche();
        const auto t1 = std::chrono::steady_clock::now();
        const bool zweiter = warteAuf (9000, [&] { return versuche() > nachSofort; });
        const auto abstand = std::chrono::duration_cast<std::chrono::milliseconds> (
            std::chrono::steady_clock::now() - t1).count();

        if (ueberTelemetrie) telemetrie->stop(); else control->stop();
        server.stoppen();
        pruefe (aufbau && gestiegen && geparkt && versucheBeimParken >= 4
                    && weitere == 0 && sofort && zweiter
                    && abstand >= 350 && abstand <= 1200,
                ("parken_uebergeht_den_backoff/" + pfad + "/" + fall).c_str(),
                "Aufstieg " + (alleDa ? std::to_string (aufstieg[1] - aufstieg[0]) + " "
                                            + std::to_string (aufstieg[2] - aufstieg[1])
                                      : std::string ("unvollstaendig"))
                    + " ms, geparkt " + std::to_string (geparkt) + " bei Versuch "
                    + std::to_string (versucheBeimParken) + ", weitere Versuche "
                    + std::to_string (weitere) + ", Neustart nach "
                    + std::to_string (verzug) + " ms, Versuch DANACH nach "
                    + std::to_string (abstand)
                    + " ms (erwartet 350..1200, ohne Reset >= 4000)");
    }
}

/// E-Z01, E-Z02, E-Z04 — die zwei Zaehlervertraege der v3-Clients.
/// `verbindungsVersuche` steigt bei JEDEM Eintritt; `serverPruefungen` nur,
/// wenn der Status in der Positivliste `{verifiziert, belegtAberUnverifiziert}`
/// liegt. R1 aendert keine der beiden Regeln — es faellt nur `PIPE_BUSY` aus
/// der Positivliste, weil sein Status ein anderer wird.
void zaehlervertrag_verbindungsversuche_und_serverpruefungen()
{
    // Fall `pipe_busy`: der Wert SINKT gegenueber heute — das ist der Beleg.
    {
        const auto pipe = testPipeName ("k-zaehler-busy");
        BelegtePipe belegt;
        const bool aufbau = belegt.anlegen (pipe);
        ControlClient client ([] { return nak123ControlHello(); }, pipe);
        client.start();
        auto versuche = [&] { return client.snapshot().verbindungsVersuche; };
        const bool zweiVersuche = warteAuf (20000, [&] { return versuche() >= 2; });
        const auto s = client.snapshot();
        client.stop();
        pruefe (aufbau && zweiVersuche && s.serverPruefungen == 0,
                "zaehlervertrag_verbindungsversuche_und_serverpruefungen/pipe_busy",
                std::to_string (s.verbindungsVersuche) + " Versuche, "
                    + std::to_string (s.serverPruefungen)
                    + " Serverpruefungen (erwartet 0)");
    }
    // Fall `access_denied`: der Wert BLEIBT — ein Oeffnungsfehler ohne Handle
    // zaehlt auf dem v3-Pfad weiterhin als Serverpruefung (offener Punkt,
    // ausserhalb von R1).
    {
        const auto pipe = testPipeName ("k-zaehler-denied");
        FremdePipe fremd;
        const bool aufbau = fremd.anlegen (pipe);
        ControlClient client ([] { return nak123ControlHello(); }, pipe);
        client.start();
        const bool gefallen = warteAuf (4000, [&] {
            return client.snapshot().serverPruefstatus
                == ServerPruefStatus::belegtAberUnverifiziert;
        });
        const auto s = client.snapshot();
        client.stop();
        pruefe (aufbau && gefallen && s.serverPruefungen == 1
                    && s.verbindungsVersuche == 1,
                "zaehlervertrag_verbindungsversuche_und_serverpruefungen/access_denied",
                std::to_string (s.verbindungsVersuche) + " Versuche, "
                    + std::to_string (s.serverPruefungen) + " Serverpruefungen");
    }
    // Telemetrie, Fall `pipe_busy`: derselbe Vertrag.
    {
        const auto pipe = testPipeName ("k-zaehler-busy-t");
        BelegtePipe belegt;
        const bool aufbau = belegt.anlegen (pipe);
        TelemetryClient client ([] { return nak134TelemetryHello(); }, pipe);
        client.start();
        auto versuche = [&] { return client.snapshot().verbindungsVersuche; };
        const bool zweiVersuche = warteAuf (20000, [&] { return versuche() >= 2; });
        const auto s = client.snapshot();
        client.stop();
        pruefe (aufbau && zweiVersuche && s.serverPruefungen == 0,
                "zaehlervertrag_verbindungsversuche_und_serverpruefungen/telemetrie_pipe_busy",
                std::to_string (s.verbindungsVersuche) + " Versuche, "
                    + std::to_string (s.serverPruefungen) + " Serverpruefungen");
    }
}

/// D-P08 — die Kopplungswarte der Telemetrie laeuft VOR `eineVerbindung` und
/// faelscht `backoffMs` nicht.
///
/// **Nacharbeit Runde 1, Defekt 3.** Die alte Fassung begann bei
/// `backoffMs == kBackoffStartMs` und mass nur, dass ohne gueltige Kopplung
/// gar nicht geoeffnet wird. Damit konnte sie einen FEHLERHAFTEN Reset nicht
/// erkennen: bei 500 ms Ausgangswert sind „zurueckgesetzt" und „unberuehrt"
/// derselbe Wert. Der Fall beginnt jetzt aus einer erhoehten Stufe und misst
/// drei Dinge:
///   1. waehrend der Kopplungswarte steigt `verbindungsVersuche` NICHT
///      (es wird nicht geoeffnet);
///   2. nach dem Wiedereinsetzen der Kopplung folgt der naechste Versuch
///      SOFORT — die Kopplungswarte `continue`t und ueberspringt das
///      `wait_for (backoffMs)` (`TelemetryClient.cpp:383-398`);
///   3. der Abstand DANACH traegt weiterhin die erhoehte Stufe: weder auf 500
///      zurueckgesetzt noch zusaetzlich verdoppelt.
///
/// Welche Stufe das ist, folgt aus der Schleife und ist nicht geschaetzt: die
/// Verdopplung steht NACH dem `wait_for`, drei beobachtete Versuche heissen
/// also drei abgeschlossene Wartezeiten (500, 1.000, 2.000) und danach
/// `backoffMs == 4.000`. Genau diese 4.000 ms traegt die Kopplungswarte
/// unveraendert weiter.
void kopplungswarte_faelscht_den_backoff_nicht()
{
    const auto pipe = testPipeName ("k-kopplung");
    std::atomic<bool> gekoppelt { true };
    TelemetryClient client ([&gekoppelt] {
        TelemetryHello h;
        h.adresse = testAdresse (hex32 ('a'));
        if (gekoppelt.load())
        {
            h.linkId = hex32 ('c');
            h.challenge = hex32 ('d');
        }
        else
        {
            h.linkId = "keinhex";          // absichtlich kein hex32
            h.challenge = "auchnichthex";
        }
        return h;
    }, pipe);
    client.start();
    auto versuche = [&] { return client.snapshot().verbindungsVersuche; };

    // 1) Aufstieg auf eine ERHOEHTE Stufe: drei Versuche, Abstaende 500 und
    //    1.000; die dritte Wartezeit (2.000 ms) laeuft danach noch, und erst
    //    hinter ihr steht `backoffMs` auf 4.000. Die Kopplung wird waehrend
    //    dieser Wartezeit weggenommen — der Client betritt die Kopplungswarte
    //    also mit genau dieser Stufe.
    const auto aufstieg = versuchsStempel (versuche, 3, 12000);
    bool alleDa = true;
    for (auto v : aufstieg) alleDa = alleDa && v >= 0;
    const bool gestiegen = alleDa && (aufstieg[2] - aufstieg[1]) >= 900;

    // 2) Kopplung wegnehmen. Der Client geht in `wartetAufKopplung` und
    //    oeffnet nicht mehr — 800 ms sind rund 15 Kopplungswarten
    //    (`kBackoffStartMs / 10 + 1` = 51 ms).
    gekoppelt.store (false);
    const bool wartet = warteAuf (4000, [&] {
        return client.snapshot().status == TelemetryClient::Status::wartetAufKopplung;
    });
    const int inWarte = weitereVersucheIn (versuche, 800);

    // 3) Kopplung zurueck. Der naechste Versuch kommt SOFORT (die
    //    Kopplungswarte `continue`t vor dem Backoff-`wait_for`), und der
    //    Abstand DANACH traegt die unveraenderte Stufe 4.000 ms.
    const int vorRueckkehr = versuche();
    const auto t0 = std::chrono::steady_clock::now();
    gekoppelt.store (true);
    const bool sofort = warteAuf (400, [&] { return versuche() > vorRueckkehr; });
    const auto verzug = std::chrono::duration_cast<std::chrono::milliseconds> (
        std::chrono::steady_clock::now() - t0).count();
    const auto danach = versuchsStempel (versuche, 1, 8000);
    // ⚠️ SONDE-013 Nacharbeit 1: auf den PRUEFSTATUS warten, statt ihn
    // anzunehmen.
    //
    // Der Fall las den Snapshot unmittelbar nach dem letzten Versuchsstempel
    // und erwartete `nichtDa`. Der Statuswechsel passiert aber IM Clientthread
    // und ist mit dem Stempel nicht synchronisiert: unter Last steht dort noch
    // `nichtGeprueft` (0), und der Fall fiel — einmal im Kanonlauf vom
    // 04.09.2026 auf `8777410`, waehrend derselbe Lauf ihn standalone zweimal
    // gruen sah (Status 1). Das ist eine Zeitannahme im TEST, keine Aussage
    // ueber das Produkt; die Wartezeit macht sie zu einer Messung. Faellt sie
    // ab, bleibt der Status stehen und die Pruefung darunter faellt weiterhin.
    warteAuf (4000, [&] {
        return client.snapshot().serverPruefstatus != ServerPruefStatus::nichtGeprueft;
    });
    const auto s = client.snapshot();
    client.stop();

    // 4.000 ms nominal (Herleitung im Kopf). Untergrenze 3.600 faengt einen
    // faelschlichen Reset auf 500 ab, Obergrenze 5.800 eine faelschliche
    // Verdopplung auf den Deckel 8.000.
    const bool stufeGehalten = ! danach.empty() && danach[0] >= 3600 && danach[0] <= 5800;
    // Der Name existiert nicht, also ist der Status nach dem Aufstieg ehrlich
    // `nichtDa`/`pipeFehlt` und `serverPruefungen` bleibt 0 (kein Handle, keine
    // Pruefung). Die alte Fassung erwartete hier `nichtGeprueft` — richtig,
    // solange gar nicht geoeffnet wurde, und genau deshalb mass sie nichts.
    pruefe (gestiegen && wartet && inWarte == 0 && sofort && stufeGehalten
                && s.serverPruefungen == 0
                && s.serverPruefstatus == ServerPruefStatus::nichtDa
                && s.serverPrueffehler == ServerPruefFehler::pipeFehlt,
            "kopplungswarte_faelscht_den_backoff_nicht",
            "Aufstieg " + (alleDa ? std::to_string (aufstieg[1] - aufstieg[0]) + " "
                                        + std::to_string (aufstieg[2] - aufstieg[1])
                                  : std::string ("unvollstaendig"))
                + " ms, Versuche in der Kopplungswarte " + std::to_string (inWarte)
                + ", Rueckkehr nach " + std::to_string (verzug)
                + " ms, Abstand danach "
                + (danach.empty() ? std::string ("-") : std::to_string (danach[0]))
                + " ms (erwartet 3600..5800 — die Stufe vor der Warte), Status "
                + std::to_string (static_cast<int> (s.serverPruefstatus))
                + ", Pruefungen " + std::to_string (s.serverPruefungen));
}

/// E-V01, E-V02 — `BrokerLifecycle` sieht den neuen Status und tut NICHTS:
/// kein Spawn (den darf nur `nichtDa` oeffnen) und keine Sperre (die verlangt
/// `belegtAberUnverifiziert`). Der Wert faellt in beiden Zweigen durch.
///
/// **Nacharbeit Runde 1, Defekt 4.** Der Fall `ausServerpruefung` richtete
/// keinen erreichbaren `pipeName` ein. `vorhandenePipeUebernehmen`
/// (`BrokerLifecycle.cpp:783-802`) verlangt genau den; ohne ihn blieb der
/// Lifecycle in `wartetAufConnect` und erreichte die behauptete Phase
/// `wartetAufServerpruefung` nie. Die Behauptung war zusaetzlich
/// `letzterServerPruefstatus != nichtDa` — das besteht schon der Default
/// `nichtGeprueft`, misst also nichts. Beides ist hier ersetzt: eine echte
/// Pipe fuehrt in die Phase (sichtbar an `wartetAufServerpruefung == true`),
/// und geprueft wird der KONKRETE neue Wert samt Phasenwechsel. Getickt wird
/// deterministisch ueber `tickFuerTest`, nicht ueber den 25-ms-Thread.
void belegt_nicht_erreicht_spawnt_nicht_und_blockiert_nicht()
{
    // ── E-V01: Phase `wartetAufConnect`, der neue Wert faellt durch ───────
    {
        std::atomic<int> spawns { 0 }, reconnects { 0 };
        std::atomic<ServerPruefStatus> status { ServerPruefStatus::belegtNichtErreicht };
        BrokerLifecycleHooks h;
        h.serverPruefstatus = [&] { return status.load(); };
        h.darfStarten = [] { return true; };
        h.reconnect = [&] { ++reconnects; };
        h.pruefen = [] { return BrokerPruefBericht {}; };
        h.spawn = [&] { ++spawns; return true; };
        h.mutexName = L"Local\\Nakama.NAK134.EV01."
                    + std::to_wstring (GetCurrentProcessId());
        BrokerLifecycle lifecycle (std::move (h));
        lifecycle.tickFuerTest (0);
        lifecycle.tickFuerTest (1000);
        const auto z = lifecycle.snapshot();
        lifecycle.stop();
        // Der Wert trifft weder den Spawn- noch den Sperrzweig und faellt in
        // den `return`: `letzterServerPruefstatus` wird auf diesem Weg gar
        // nicht geschrieben und bleibt deshalb der Default. Das ist ein
        // KONKRETER Wert, keine Ungleichung.
        pruefe (spawns.load() == 0 && reconnects.load() == 0
                    && ! z.serverNichtVerifiziert && ! z.imCooldown
                    && z.cooldowns == 0 && z.mutexVerloren == 0
                    && ! z.wartetAufServerpruefung
                    && z.letzterServerPruefstatus == ServerPruefStatus::nichtGeprueft,
                "belegt_nicht_erreicht_spawnt_nicht_und_blockiert_nicht/aus_connect",
                std::to_string (spawns.load()) + " Spawns, serverNichtVerifiziert "
                    + std::to_string (z.serverNichtVerifiziert) + ", cooldowns "
                    + std::to_string (z.cooldowns) + ", letzterStatus "
                    + std::to_string (static_cast<int> (z.letzterServerPruefstatus)));
    }

    // ── E-V02: WIRKLICH in `wartetAufServerpruefung`, dann der neue Wert ──
    {
        const auto pipe = testPipeName ("ev02-serverpruefung");
        TestServer server (pipe);
        const bool serverSteht = server.starten();
        std::atomic<int> spawns { 0 }, reconnects { 0 };
        // `nichtDa` oeffnet den Startweg — und genau dort uebernimmt
        // `vorhandenePipeUebernehmen` die vorhandene Pipe und stellt die Phase
        // `wartetAufServerpruefung` her, ohne zu spawnen.
        std::atomic<ServerPruefStatus> status { ServerPruefStatus::nichtDa };
        BrokerLifecycleHooks h;
        h.serverPruefstatus = [&] { return status.load(); };
        h.darfStarten = [] { return true; };
        h.reconnect = [&] { ++reconnects; };
        h.pruefen = [] { return BrokerPruefBericht {}; };
        h.spawn = [&] { ++spawns; return true; };
        h.mutexName = L"Local\\Nakama.NAK134.EV02."
                    + std::to_wstring (GetCurrentProcessId());
        h.pipeName = pipe;
        BrokerLifecycle lifecycle (std::move (h));
        lifecycle.tickFuerTest (0);
        const auto inPhase = lifecycle.snapshot();
        // Erst JETZT, in der erreichten Phase, meldet Control den neuen Wert.
        status.store (ServerPruefStatus::belegtNichtErreicht);
        lifecycle.tickFuerTest (1);
        const auto danach = lifecycle.snapshot();
        lifecycle.stop();
        server.stoppen();
        pruefe (serverSteht
                    // die behauptete Phase ist wirklich erreicht
                    && inPhase.wartetAufServerpruefung
                    && reconnects.load() == 1
                    // ... und der neue Wert loest den Phasenwechsel nach
                    // `wartetAufConnect` aus, mit dem KONKRETEN Status
                    && ! danach.wartetAufServerpruefung
                    && danach.letzterServerPruefstatus
                           == ServerPruefStatus::belegtNichtErreicht
                    // ... ohne Spawn und ohne Sperre
                    && spawns.load() == 0 && ! danach.serverNichtVerifiziert
                    && ! danach.imCooldown && danach.cooldowns == 0
                    && danach.mutexVerloren == 0,
                "belegt_nicht_erreicht_spawnt_nicht_und_blockiert_nicht/aus_serverpruefung",
                "Phase erreicht " + std::to_string (inPhase.wartetAufServerpruefung)
                    + ", danach " + std::to_string (danach.wartetAufServerpruefung)
                    + ", letzterStatus "
                    + std::to_string (static_cast<int> (danach.letzterServerPruefstatus))
                    + " (erwartet "
                    + std::to_string (static_cast<int> (ServerPruefStatus::belegtNichtErreicht))
                    + "), " + std::to_string (spawns.load()) + " Spawns");
    }
}

/// E-V04, E-V05, E-V06 — der Gegenpfad parken↔loesen AM CLIENT. Bisher hat
/// ihn kein Test beruehrt: der frueher zitierte
/// `unverifizierte_belegte_pipe_spawnt_und_reconnectet_nicht` misst den
/// `BrokerLifecycle`-Hook, nie die Condition-Variable des Clients.
void parken_loest_ueber_reconnect_und_stop()
{
    for (const bool ueberStop : { false, true })
    {
        const auto pipe = testPipeName (ueberStop ? "k-loesen-stop" : "k-loesen-reconnect");
        FremdePipe fremd;
        const bool aufbau = fremd.anlegen (pipe);
        ControlClient client ([] { return nak123ControlHello(); }, pipe);
        client.start();
        const bool geparkt = warteAuf (4000, [&] {
            return client.snapshot().serverPruefstatus
                == ServerPruefStatus::belegtAberUnverifiziert;
        });
        auto versuche = [&] { return client.snapshot().verbindungsVersuche; };
        const int vorher = versuche();
        bool geloest = false;
        long long dauerMs = 0;
        const auto t0 = std::chrono::steady_clock::now();
        if (ueberStop)
        {
            client.stop();
            dauerMs = std::chrono::duration_cast<std::chrono::milliseconds> (
                std::chrono::steady_clock::now() - t0).count();
            // Geloest heisst hier: `stop()` kehrt zurueck, ohne die
            // Stopfrist zu reissen. Ein Thread, der auf der Sperre haengen
            // bliebe, wuerde abgeloest und der Zaehler stiege.
            geloest = client.snapshot().stopFristUeberschritten == 0;
        }
        else
        {
            client.reconnect();
            geloest = warteAuf (250, [&] { return versuche() > vorher; });
            dauerMs = std::chrono::duration_cast<std::chrono::milliseconds> (
                std::chrono::steady_clock::now() - t0).count();
            client.stop();
        }
        pruefe (aufbau && geparkt && geloest,
                ueberStop ? "parken_loest_ueber_reconnect_und_stop/stop"
                          : "parken_loest_ueber_reconnect_und_stop/reconnect",
                "geparkt " + std::to_string (geparkt) + ", geloest nach "
                    + std::to_string (dauerMs) + " ms");
    }
}

/// Obere Schranke der R5-Zusage in den Proben dieser Datei: Restzeit einer
/// laufenden `WaitNamedPipeW(200 ms)` plus hoechstens eine Runde (ein
/// `CreateFileW`, dazu nur bei `PIPE_BUSY` ein weiteres `WaitNamedPipeW(200)`).
/// Nominal sind das rund 400 ms; 1.500 ms lassen Scheduling zu und liegen
/// trotzdem weit unter den >= 4.000 ms der vollen Warteschleife — die Schranke
/// TRENNT also "Abbruch hat gegriffen" von "Schleife lief zu Ende".
constexpr int kR5FristMs = 1500;

/// NAK-134 Nacharbeit Runde 1, Defekt 1 — R5 / D-A01 bis D-A06.
///
/// Das Abbruchsignal gehoert der VERBINDUNGSGENERATION, nicht dem
/// Oeffnungsaufruf. Bis zur Nacharbeit rief `oeffnen()` zuerst `schliessen()`
/// und setzte danach `abbruch = false`. Ein `ioAbbrechen()`, das NACH der
/// aeusseren Generationspruefung des Clients und VOR dem Eintritt in `oeffnen()`
/// eintraf, wurde damit geloescht — auf einer belegten Pipe liefen anschliessend
/// alle 20 `WaitNamedPipeW(200 ms)`-Runden, also rund vier Sekunden statt der
/// R5-Frist.
///
/// Gemessen wird in vier Faellen:
///   * `ipcverbindung`         BELEG, deterministisch: das Signal steht VOR dem
///                             Eintritt, die Pipe ist belegt. Vor dem Fix
///                             >= 4.000 ms, danach innerhalb der R5-Frist.
///   * `gegenpfad_neue_generation`  die andere Richtung: nach
///                             `neueGenerationBeginnen()` oeffnet DIESELBE
///                             Verbindung wieder normal. Ohne diesen Fall
///                             koennte ein Fix das Signal einfach nie mehr
///                             loesen und der Beleg bliebe trotzdem gruen.
///   * `control`, `telemetrie` die Aufrufreihenfolge im Client:
///                             `neueGenerationBeginnen()`, dann ERNEUT die
///                             Generation lesen, dann `oeffnen()`. Das ist die
///                             Stelle, an der das Rennen geschlossen wird —
///                             `stop()`/`reconnect()` erhoehen die Generation
///                             VOR `ioAbbrechen()`, also hat jeder Abbruch, den
///                             das Loesen verschlucken koennte, die Generation
///                             vorher schon erhoeht. Das verbleibende Fenster
///                             zwischen dieser zweiten Pruefung und `oeffnen()`
///                             ist ohne einen Testhaken IM Oeffnungspfad nicht
///                             anfahrbar; die beiden Faelle sind deshalb
///                             ehrlich **Regressionswache durch Lesen**, wie
///                             `ressourcenfehler_nach_authbeweis_ist_liveness`.
void abbruchsignal_gehoert_der_generation()
{
    // ── BELEG: Signal vor dem Eintritt, belegte Pipe ─────────────────────
    {
        const auto pipe = testPipeName ("k-latch-belegt");
        BelegtePipe belegt;
        const bool aufbau = belegt.anlegen (pipe);
        IpcVerbindung verbindung;
        ServerPruefBericht bericht;
        std::string fehlertext;
        verbindung.ioAbbrechen();       // genau die Lage aus Defekt 1
        const auto t0 = std::chrono::steady_clock::now();
        const bool geoeffnet = verbindung.oeffnen (pipe, ServerErwartung {},
                                                   bericht, fehlertext);
        const auto dauerMs = std::chrono::duration_cast<std::chrono::milliseconds> (
            std::chrono::steady_clock::now() - t0).count();
        verbindung.schliessen();
        pruefe (aufbau && ! geoeffnet && dauerMs < kR5FristMs
                    && bericht.status == ServerPruefStatus::nichtGeprueft
                    && bericht.fehler == ServerPruefFehler::keiner,
                "abbruchsignal_gehoert_der_generation/ipcverbindung",
                std::to_string (dauerMs) + " ms (R5-Frist " + std::to_string (kR5FristMs)
                    + " ms; volle Warteschleife >= 4000 ms), Bericht "
                    + std::to_string (static_cast<int> (bericht.status)));
    }

    // ── Gegenpfad: die neue Generation LOEST das Signal wieder ───────────
    //    Ohne diesen Fall koennte ein Fix das Signal einfach nie mehr loesen —
    //    der Beleg oben bliebe gruen und keine Verbindung kaeme je zustande.
    {
        const auto pipe = testPipeName ("k-latch-loesen");
        TestServer server (pipe);
        const bool steht = server.starten();
        IpcVerbindung verbindung;
        ServerPruefBericht bericht;
        std::string fehlertext;
        verbindung.ioAbbrechen();
        verbindung.neueGenerationBeginnen();
        const bool geoeffnet = verbindung.oeffnen (
            pipe, testExeErwartung (GetCurrentProcessId()), bericht, fehlertext);
        verbindung.schliessen();
        server.stoppen();
        pruefe (steht && geoeffnet && bericht.status == ServerPruefStatus::verifiziert,
                "abbruchsignal_gehoert_der_generation/gegenpfad_neue_generation",
                "geoeffnet " + std::to_string (geoeffnet) + ", Bericht "
                    + std::to_string (static_cast<int> (bericht.status)) + ", "
                    + (fehlertext.empty() ? std::string ("kein Fehler") : fehlertext));
    }

    // ── Aufrufreihenfolge in den beiden v3-Clients ───────────────────────
    {
        const auto ipc = wurzel().getChildFile (
            "eq-copilot/plugin/core/ipc/IpcVerbindung.cpp").loadFileAsString();
        const auto rumpf = ipc.fromFirstOccurrenceOf ("bool IpcVerbindung::oeffnen",
                                                      false, false)
                              .upToFirstOccurrenceOf ("void IpcVerbindung::schliessen",
                                                      false, false);
        // `oeffnen` liest das Signal, loescht es aber nie und schliesst auch
        // nicht mehr selbst (das waere ein `abbruch = true` ohne Loesung).
        const bool oeffnenLoeschtNicht = rumpf.isNotEmpty()
                                      && rumpf.contains ("abbruch.load()")
                                      && ! rumpf.contains ("abbruch.store")
                                      && ! rumpf.contains ("schliessen();");
        pruefe (oeffnenLoeschtNicht,
                "abbruchsignal_gehoert_der_generation/oeffnen_loescht_nie",
                "Codepruefung an IpcVerbindung.cpp");

        for (const char* datei : { "ControlClient.cpp", "TelemetryClient.cpp" })
        {
            const auto quelle = wurzel().getChildFile (
                std::string ("eq-copilot/plugin/core/ipc/") + datei).loadFileAsString();
            // Zwischen dem Loesen und dem Oeffnungsversuch MUSS die Generation
            // erneut gelesen werden — sonst bliebe genau das Rennen offen, das
            // Defekt 1 beschreibt.
            const auto zone = quelle.fromFirstOccurrenceOf (
                                        "verbindung.neueGenerationBeginnen()", false, false)
                                    .upToFirstOccurrenceOf ("verbindung.oeffnen (", false, false);
            pruefe (zone.isNotEmpty() && zone.contains ("sollAbbrechen (generation)")
                        && ! zone.contains ("oeffnen ("),
                    (std::string ("abbruchsignal_gehoert_der_generation/")
                     + (std::string (datei) == "ControlClient.cpp" ? "control" : "telemetrie")).c_str(),
                    "Regressionswache durch Lesen: loesen, Generation erneut pruefen, "
                    "dann oeffnen");
        }
    }
}

/// D-A01 bis D-A11 — Abbruch vor und nach einem erfolgreichen `CreateFileW`,
/// auf BEIDEN v3-Pfaden.
///
/// R5 ist eine SPAETESTENS-Zusage: gemessen wird der AUSGANG (kein Hello, kein
/// Byte, kein veroeffentlichtes Urteil) UND die obere Schranke `kR5FristMs` —
/// ausdruecklich keine Mindestdauer (§7).
///
/// **Nacharbeit Runde 1, Defekt 2.** Die Vor-Open-Faelle liefen bis dahin gegen
/// einen NICHT EXISTIERENDEN Namen: `CreateFileW` kehrte in Runde 1 mit
/// `FILE_NOT_FOUND` zurueck, die zugesagte Wartephase auf einer belegten Pipe
/// wurde also nie beruehrt, und ein Abbruch traf auf eine Schleife, die
/// ohnehin schon fertig war. Beides ist hier nachgezogen:
///   * die Vor-Open-Lage ist eine **belegte** Pipe (§4.1 der Matrix), der
///     Abbruch trifft MITTEN in die 20 x 200 ms-Warteschleife;
///   * die Telemetrie hat jetzt jeden der vier Faelle (D-A02, D-A05, D-A08,
///     D-A11), nicht nur `stop_vor_open`.
void abbruch_vor_und_nach_createfile()
{
    // ── vor dem Oeffnungserfolg: BELEGTE Pipe, Abbruch in der Wartephase ──
    //    D-A01/D-A04 (Control) und D-A02/D-A05 (Telemetrie).
    for (const bool ueberStop : { true, false })
    for (const bool ueberTelemetrie : { false, true })
    {
        const std::string vorgang (ueberStop ? "stop_vor_open" : "reconnect_vor_open");
        const std::string pfad (ueberTelemetrie ? "telemetrie" : "control");
        const auto pipe = testPipeName (("k-abbruch-vor-" + pfad).c_str());
        BelegtePipe belegt;
        const bool aufbau = belegt.anlegen (pipe);

        std::unique_ptr<ControlClient> control;
        std::unique_ptr<TelemetryClient> telemetrie;
        if (ueberTelemetrie)
            telemetrie.reset (new TelemetryClient (
                [] { return nak134TelemetryHello(); }, pipe));
        else
            control.reset (new ControlClient (
                [] { return nak123ControlHello(); }, pipe));

        auto versuche = [&] {
            return ueberTelemetrie ? telemetrie->snapshot().verbindungsVersuche
                                   : control->snapshot().verbindungsVersuche;
        };
        if (ueberTelemetrie) telemetrie->start(); else control->start();

        // Erst wirklich in die Warteschleife laufen lassen. Der Versuch ist
        // gezaehlt, sobald `eineVerbindung` betreten wurde; die 300 ms danach
        // setzen den Abbruch sicher MITTEN in die 4-s-Schleife und nicht in
        // die Luecke davor.
        const bool inSchleife = warteAuf (3000, [&] { return versuche() >= 1; });
        std::this_thread::sleep_for (std::chrono::milliseconds (300));

        bool ok = false;
        long long dauerMs = 0;
        std::string zustand;
        if (ueberStop)
        {
            const auto t0 = std::chrono::steady_clock::now();
            if (ueberTelemetrie) telemetrie->stop(); else control->stop();
            dauerMs = std::chrono::duration_cast<std::chrono::milliseconds> (
                std::chrono::steady_clock::now() - t0).count();
            // D-A01/D-A02: nach dem Join getrennt / nichtGeprueft / keiner.
            if (ueberTelemetrie)
            {
                const auto s = telemetrie->snapshot();
                ok = s.status == TelemetryClient::Status::getrennt
                  && s.serverPruefstatus == ServerPruefStatus::nichtGeprueft
                  && s.serverPrueffehler == ServerPruefFehler::keiner
                  && s.serverPid == 0 && s.serverPruefungen == 0
                  && s.stopFristUeberschritten == 0;
                zustand = "Status " + std::to_string (static_cast<int> (s.status));
            }
            else
            {
                const auto s = control->snapshot();
                ok = s.status == ControlClient::Status::getrennt
                  && s.serverPruefstatus == ServerPruefStatus::nichtGeprueft
                  && s.serverPrueffehler == ServerPruefFehler::keiner
                  && s.serverPid == 0 && s.serverPruefungen == 0
                  && s.stopFristUeberschritten == 0;
                zustand = "Status " + std::to_string (static_cast<int> (s.status));
            }
        }
        else
        {
            const int vorher = versuche();
            const auto t0 = std::chrono::steady_clock::now();
            if (ueberTelemetrie) telemetrie->reconnect(); else control->reconnect();
            // D-A04/D-A05: neue Generation, Reset, neue Runde OHNE Wartezeit.
            // Der naechste Versuchsstempel ist der von aussen sichtbare
            // Zeitpunkt, an dem der Abbruch gegriffen HAT.
            const bool neu = warteAuf (kR5FristMs, [&] { return versuche() > vorher; });
            dauerMs = std::chrono::duration_cast<std::chrono::milliseconds> (
                std::chrono::steady_clock::now() - t0).count();
            const std::uint64_t pruefungen =
                ueberTelemetrie ? telemetrie->snapshot().serverPruefungen
                                : control->snapshot().serverPruefungen;
            ok = neu && pruefungen == 0;
            zustand = "Versuche +" + std::to_string (versuche() - vorher);
            if (ueberTelemetrie) telemetrie->stop(); else control->stop();
        }

        pruefe (aufbau && inSchleife && ok && dauerMs < kR5FristMs,
                ("abbruch_vor_und_nach_createfile/" + pfad + "/" + vorgang).c_str(),
                std::to_string (dauerMs) + " ms (R5-Frist " + std::to_string (kR5FristMs)
                    + " ms; die belegte Warteschleife allein kostet >= 4000 ms), "
                    + zustand);
    }

    // ── nach dem Oeffnungserfolg: der Abbruch trifft waehrend der
    //    Authentisierung ein. Kein Hello darf fliessen.
    //    D-A07/D-A10 (Control) und D-A08/D-A11 (Telemetrie).
    for (const bool ueberStop : { true, false })
    for (const bool ueberTelemetrie : { false, true })
    {
        const std::string vorgang (ueberStop ? "stop_nach_open" : "reconnect_nach_open");
        const std::string pfad (ueberTelemetrie ? "telemetrie" : "control");
        const auto pipe = testPipeName (("k-abbruch-nach-" + pfad).c_str());
        TestServer server (pipe);
        const bool steht = server.starten();
        std::atomic<bool> vorHash { false }, freigeben { false };
        auto erwartung = testExeErwartung (GetCurrentProcessId());
        erwartung.testVorFehlerErreicht = &vorHash;
        erwartung.testFehlerFreigeben = &freigeben;

        std::unique_ptr<ControlClient> control;
        std::unique_ptr<TelemetryClient> telemetrie;
        if (ueberTelemetrie)
            telemetrie.reset (new TelemetryClient (
                [&server] {
                    TelemetryHello h;
                    h.adresse = testAdresse (hex32 ('a'));
                    h.linkId = server.kopplungLinkId();
                    h.challenge = server.kopplungChallenge();
                    return h;
                }, pipe, {}, erwartung));
        else
            control.reset (new ControlClient (
                [] { return nak123ControlHello(); }, pipe, {}, {}, {}, {}, erwartung));

        auto versuche = [&] {
            return ueberTelemetrie ? telemetrie->snapshot().verbindungsVersuche
                                   : control->snapshot().verbindungsVersuche;
        };
        auto pruefungen = [&] {
            return ueberTelemetrie ? telemetrie->snapshot().serverPruefungen
                                   : control->snapshot().serverPruefungen;
        };
        if (ueberTelemetrie) telemetrie->start(); else control->start();

        // Jetzt steht das Handle offen und die Authentisierung haelt an der
        // Testbarriere — genau der Zeitpunkt "nach `CreateFileW`-Erfolg".
        const bool ander = warteAuf (8000, [&] { return vorHash.load(); });
        const int versucheVor = versuche();
        const std::uint64_t pruefungenVor = pruefungen();
        bool ok = false;
        if (ueberStop)
        {
            std::thread abbrecher ([&] {
                if (ueberTelemetrie) telemetrie->stop(); else control->stop(); });
            std::this_thread::sleep_for (std::chrono::milliseconds (50));
            freigeben.store (true);
            abbrecher.join();
            // Das Urteil der veralteten Generation wird verworfen: der Zaehler
            // steht still und kein Hello ist geflossen.
            ok = pruefungen() == pruefungenVor;
        }
        else
        {
            if (ueberTelemetrie) telemetrie->reconnect(); else control->reconnect();
            std::this_thread::sleep_for (std::chrono::milliseconds (50));
            // Den Server VOR der Freigabe schliessen. Sonst faenge die neue
            // Runde eine zweite Verbindung an und ihr — voellig regulaeres —
            // Hello wuerde die Aussage ueber die ABGEBROCHENE Runde faelschen.
            server.stoppen();
            freigeben.store (true);
            ok = warteAuf (5000, [&] { return versuche() > versucheVor; })
              && pruefungen() == pruefungenVor;
            if (ueberTelemetrie) telemetrie->stop(); else control->stop();
        }
        const bool keinHello = serverHatKeinHello (server);
        server.stoppen();
        pruefe (steht && ander && ok && keinHello,
                ("abbruch_vor_und_nach_createfile/" + pfad + "/" + vorgang).c_str(),
                "an der Barriere " + std::to_string (ander) + ", kein Hello "
                    + std::to_string (keinHello) + ", Pruefungen "
                    + std::to_string (pruefungen()) + " (unveraendert "
                    + std::to_string (pruefungenVor) + ")");
    }
}

/// D-K12, D-K13, R4 — ein lokaler Ressourcenfehler NACH bestandenem
/// Identitaetsbeweis ist ein Livenessausgang.
///
/// `CreateEventW` ist im Produktpfad nicht injizierbar, ohne einen Testhaken
/// in den Oeffnungspfad zu legen. Die Zeile ist deshalb ehrlich eine
/// **Regressionswache durch Lesen**: sie prueft, dass die vier Zusagen aus dem
/// Code folgen, und behauptet keinen Lauf, den es nicht gibt.
void ressourcenfehler_nach_authbeweis_ist_liveness()
{
    // Was gelesen wurde (`IpcVerbindung.cpp`, Zweig `CreateEventW == nullptr`):
    //   * `CloseHandle (h)` — der Handle wird geschlossen;
    //   * `bericht.status = nichtGeprueft` — der Status faellt aus der
    //     Positivliste von `ControlClient.cpp:1101-1103`, also KEINE
    //     zusaetzliche Serverpruefung;
    //   * `authBlockiert` prueft auf `belegtAberUnverifiziert` und bleibt
    //     `false` — also normaler Backoff, kein Parken;
    //   * der Spawnzweig verlangt `nichtDa` — also kein Startpfad.
    // Die fuenfte Zusage (der Win32-Fehler steht im Bericht) ist Phase 2 und
    // wird hier als Codeeigenschaft gemessen, nicht als Lauf.
    const auto quelle = wurzel().getChildFile (
        "eq-copilot/plugin/core/ipc/IpcVerbindung.cpp").loadFileAsString();
    const auto zweig = quelle.fromFirstOccurrenceOf ("HANDLE e = CreateEventW", false, false)
                             .upToFirstOccurrenceOf ("std::lock_guard", false, false);
    // Auf STRUKTUR pruefen, nicht auf Schreibweise: dass der Zweig einen
    // frischen `GetLastError()` holt UND ihn in den Bericht schreibt. Eine
    // Pruefung auf den Wortlaut einer einzelnen Zeile faellt beim naechsten
    // Umbenennen einer lokalen Variablen, ohne dass sich die Zusage aendert.
    const bool win32ImBericht = zweig.contains ("GetLastError()")
                             && zweig.contains ("bericht.win32Fehler =");
    pruefe (zweig.isNotEmpty() && win32ImBericht
                && zweig.contains ("bericht.status = ServerPruefStatus::nichtGeprueft")
                && zweig.contains ("CloseHandle (h)")
                && ! zweig.contains ("belegtAberUnverifiziert")
                && ! zweig.contains ("ServerPruefStatus::nichtDa"),
            "ressourcenfehler_nach_authbeweis_ist_liveness",
            "Codepruefung an IpcVerbindung.cpp — CreateEventW ist ohne Testhaken "
            "im Oeffnungspfad nicht injizierbar (Regressionswache durch Lesen)");
}

int phaseBBinaryVerifyMain (const std::string& pfadUtf8,
                            const std::string& erwarteterHash,
                            const std::string& thumbprint,
                            const std::string& erwartetesUrteil)
{
    const juce::String pfadText = juce::String::fromUTF8 (pfadUtf8.c_str());
    const std::wstring pfad (pfadText.toWideCharPointer());
    const auto bericht = brokerBinaryPruefen (pfad, erwarteterHash, thumbprint);
    const std::string urteil = brokerPruefFehlerName (bericht.fehler);
    const bool signaturErwartet = erwartetesUrteil == "ok"
                               || erwartetesUrteil == "signerFalsch"
                               || erwartetesUrteil == "signaturFehltOderUngueltig";
    const bool ok = urteil == erwartetesUrteil && bericht.hashGeprueft
                 && (! signaturErwartet || bericht.signaturGeprueft);
    std::cout << "phase_b_binary_verify result=" << urteil
              << " expected=" << erwartetesUrteil
              << " hash_checked=" << bericht.hashGeprueft
              << " signature_checked=" << bericht.signaturGeprueft
              << " signer=" << bericht.signerThumbprint << std::endl;
    return ok ? 0 : 22;
}
} // namespace

//==============================================================================
int main (int argc, char** argv)
{
    if (argc == 3 && std::string (argv[1]) == "--nak123-test-server")
        return nak123TestServerMain (argv[2]);
    if (argc == 6 && std::string (argv[1]) == "--phase-b-command-client")
        return phaseBCommandClientMain (argv[2], argv[3], argv[4], argv[5]);
    if (argc == 6 && std::string (argv[1]) == "--phase-b-verify-binary")
        return phaseBBinaryVerifyMain (argv[2], argv[3], argv[4], argv[5]);

    std::cout << "SONDE-010 | v3-Envelope, Pipetoken, Backpressure und die zwei Clients"
              << std::endl;

    // ── A · Envelope-Korpus gegen das handgeschriebene MANIFEST ────────────
    abschnitt ("A · Envelope-Fixture-Korpus (dieselbe dritte Partei wie Rust)");
    {
        const auto basis = wurzel().getChildFile ("eq-copilot/fixtures/v3/envelope");
        const auto manifestDatei = basis.getChildFile ("MANIFEST.json");
        pruefe (manifestDatei.existsAsFile(), "MANIFEST.json gefunden",
                manifestDatei.getFullPathName().toStdString());
        const auto manifest = juce::JSON::parse (manifestDatei);

        // Die Regelliste des Manifests IST die geschlossene Menge des Lesers.
        const auto* regeln = manifest.getProperty ("regeln", {}).getArray();
        bool regelnGleich = regeln != nullptr && regeln->size() == kVerstossAnzahl;
        if (regelnGleich)
            for (int i = 0; i < kVerstossAnzahl; ++i)
                regelnGleich = regelnGleich
                            && (*regeln)[i].toString().toStdString()
                                   == verstossName (static_cast<Verstoss> (i));
        pruefe (regelnGleich, "Regelmenge und ihre Reihenfolge stimmen mit dem Manifest",
                regeln != nullptr ? std::to_string (regeln->size()) + " Regeln" : "keine Liste");

        // Innerhalb der SAMMELNDEN Stufe 3 ist die Deklarationsreihenfolge
        // zugleich die kanonische Namenssortierung — nur deshalb darf der
        // C++-Leser ohne Sortierschritt auskommen und trotzdem dieselbe Menge
        // in derselben Ordnung liefern wie Rust (das dort sortiert).
        bool stufeDreiSortiert = true;
        for (int i = static_cast<int> (Verstoss::crcBeiJsonNichtNull) + 1;
             i < kVerstossAnzahl; ++i)
            if (std::string (verstossName (static_cast<Verstoss> (i - 1)))
                >= std::string (verstossName (static_cast<Verstoss> (i))))
                stufeDreiSortiert = false;
        pruefe (stufeDreiSortiert,
                "die SAMMELNDE Stufe 3 ist in Deklarationsreihenfolge namenssortiert");

        const auto* fixtures = manifest.getProperty ("fixtures", {}).getArray();
        int gueltig = 0, ungueltig = 0, abweichungen = 0;
        std::string ersteAbweichung;
        if (fixtures != nullptr)
            for (const auto& eintrag : *fixtures)
            {
                const auto datei = eintrag.getProperty ("datei", {}).toString();
                const auto pfad = basis.getChildFile (datei);
                juce::MemoryBlock bytes;
                if (! pfad.loadFileAsData (bytes))
                {
                    ++abweichungen;
                    if (ersteAbweichung.empty())
                        ersteAbweichung = datei.toStdString() + ": nicht lesbar";
                    continue;
                }
                const bool sollGueltig =
                    eintrag.getProperty ("urteil", {}).toString() == "gueltig";
                std::vector<std::string> soll;
                if (auto* v = eintrag.getProperty ("verstoesse", {}).getArray())
                    for (const auto& e : *v)
                        soll.emplace_back (e.toString().toStdString());

                const auto u = envelopePruefen (
                    static_cast<const std::uint8_t*> (bytes.getData()), bytes.getSize());
                const auto ist = namen (u.verstoesse);

                if (u.gueltig != sollGueltig || ist != soll)
                {
                    ++abweichungen;
                    if (ersteAbweichung.empty())
                        ersteAbweichung = datei.toStdString() + ": soll " + alsText (soll)
                                        + ", ist " + alsText (ist);
                }
                else if (u.gueltig && u.drahtlaenge != bytes.getSize())
                {
                    ++abweichungen;
                    if (ersteAbweichung.empty())
                        ersteAbweichung = datei.toStdString() + ": Drahtlaenge passt nicht";
                }
                (sollGueltig ? gueltig : ungueltig)++;
            }

        pruefe (abweichungen == 0,
                "alle Fixtures klassifiziert wie das Manifest (Urteil UND Verstossmenge)",
                std::to_string (gueltig) + " gueltig / " + std::to_string (ungueltig)
                    + " ungueltig" + (ersteAbweichung.empty() ? "" : "; " + ersteAbweichung));
        pruefe (gueltig + ungueltig >= 30, "Korpus ist substanziell",
                std::to_string (gueltig + ungueltig) + " Fixtures");

        // Jede Regel hat mindestens ein Negativfixture — sonst stuende sie nur
        // im Code (Lehre aus SONDE-005a).
        VerstossMenge gedeckt = 0;
        if (fixtures != nullptr)
            for (const auto& eintrag : *fixtures)
                if (auto* v = eintrag.getProperty ("verstoesse", {}).getArray())
                    for (const auto& e : *v)
                    {
                        Verstoss w;
                        if (verstossAusName (e.toString().toStdString(), w))
                            gedeckt |= alsBit (w);
                    }
        pruefe (namen (gedeckt).size() == static_cast<std::size_t> (kVerstossAnzahl),
                "jede der 14 Regeln hat mindestens ein Negativfixture",
                std::to_string (namen (gedeckt).size()) + "/14");
    }

    // ── B · CRC32C und Rundlauf ───────────────────────────────────────────
    abschnitt ("B · CRC32C und Envelope-Rundlauf");
    {
        auto c = [] (const char* s) {
            return crc32c (reinterpret_cast<const std::uint8_t*> (s), std::strlen (s));
        };
        pruefe (c ("") == 0x00000000u, "CRC32C(\"\") == 0");
        pruefe (c ("123456789") == 0xE3069283u, "CRC32C(\"123456789\") == 0xE3069283 (RFC 3720)");
        std::uint8_t nullen[32] = {};
        std::uint8_t einsen[32];
        std::memset (einsen, 0xFF, sizeof (einsen));
        pruefe (crc32c (nullen, 32) == 0x8A9136AAu, "CRC32C(32x 0x00) == 0x8A9136AA");
        pruefe (crc32c (einsen, 32) == 0x62A8AB43u, "CRC32C(32x 0xFF) == 0x62A8AB43");

        std::vector<std::uint8_t> frame;
        const std::string p = "{\"type\":\"welcome\"}";
        pruefe (envelopeSchreiben (Familie::p0, 0,
                                   reinterpret_cast<const std::uint8_t*> (p.data()),
                                   p.size(), frame),
                "P0-Frame geschrieben");
        auto u = envelopePruefen (frame.data(), frame.size());
        pruefe (u.gueltig && u.kopf.pruefsumme == 0 && u.kopf.encoding == Kodierung::json,
                "P0 ist JSON und traegt CRC exakt 0");

        std::vector<std::uint8_t> nutz (64);
        for (std::size_t i = 0; i < nutz.size(); ++i) nutz[i] = static_cast<std::uint8_t> (i);
        envelopeSchreiben (Familie::p2, 0, nutz.data(), nutz.size(), frame);
        u = envelopePruefen (frame.data(), frame.size());
        pruefe (u.gueltig && u.kopf.pruefsumme == crc32c (nutz.data(), nutz.size()),
                "P2 traegt die Pflicht-CRC ueber genau die Payloadbytes");
        frame.back() ^= 0x01;
        u = envelopePruefen (frame.data(), frame.size());
        pruefe (! u.gueltig && namen (u.verstoesse) == std::vector<std::string> { "crc_falsch" },
                "ein gekipptes Payloadbit faellt an der CRC");

        // Payload an und ueber der Paketgrenze.
        std::vector<std::uint8_t> gross (kMaxPayloadBytes, 0);
        pruefe (envelopeSchreiben (Familie::p2, 0, gross.data(), gross.size(), frame)
                    && envelopePruefen (frame.data(), frame.size()).gueltig,
                "Payload an der Paketgrenze wird angenommen");
        gross.push_back (0);
        pruefe (! envelopeSchreiben (Familie::p2, 0, gross.data(), gross.size(), frame),
                "ein Byte darueber wird nicht einmal geschrieben");
    }

    // ── C · Fuzz: dieselben Invarianten wie das Rust-Bein ─────────────────
    abschnitt ("C · Fuzz (deterministisch): Invarianten des angenommenen Frames");
    {
        Zufall z (0x0000001050170010ull);
        std::size_t angenommen = 0, abgelehnt = 0;
        bool invarianteHaelt = true;
        std::vector<std::uint8_t> roh;
        for (int runde = 0; runde < 40000; ++runde)
        {
            const std::size_t n = static_cast<std::size_t> (z.bis (40));
            roh.resize (n);
            for (std::size_t i = 0; i < n; ++i)
                roh[i] = static_cast<std::uint8_t> (z.naechste() & 0xFF);
            const auto u = envelopePruefen (roh.data(), roh.size());
            if (! u.gueltig) { ++abgelehnt; continue; }
            ++angenommen;
            const std::uint32_t frameLen = static_cast<std::uint32_t> (roh[0])
                                         | (static_cast<std::uint32_t> (roh[1]) << 8)
                                         | (static_cast<std::uint32_t> (roh[2]) << 16)
                                         | (static_cast<std::uint32_t> (roh[3]) << 24);
            if (! (frameLen >= kKopfBytes && frameLen <= kMaxFrameBytes
                   && static_cast<std::uint64_t> (frameLen)
                          == static_cast<std::uint64_t> (kKopfBytes) + u.kopf.payloadLen
                   && u.kopf.flags == 0 && u.kopf.schemaMajor == kSchemaMajor
                   && u.kopf.encoding == erwarteteKodierung (u.kopf.familie)
                   && u.drahtlaenge == 4u + frameLen))
                invarianteHaelt = false;
        }
        pruefe (invarianteHaelt, "kein angenommener Zufallsframe verletzt eine Kopfregel",
                std::to_string (angenommen) + " angenommen / "
                    + std::to_string (abgelehnt) + " abgelehnt");

        // Reine Zufallsbytes werden praktisch immer abgelehnt (gemessen: 0 von
        // 40 000 angenommen) - die Invariante liefe damit ins Leere. Deshalb
        // dieselbe Pruefung noch einmal an EINEM gekippten Bit eines GUELTIGEN
        // Frames: dort kommen genug durch, um die Aussage zu tragen. Ohne
        // diesen zweiten Lauf waere der Satz oben eine Behauptung ueber eine
        // leere Menge.
        std::size_t angenommenMutiert = 0, abgelehntMutiert = 0;
        bool invarianteMutiert = true;
        for (int runde = 0; runde < 20000; ++runde)
        {
            std::vector<std::uint8_t> gut2;
            const auto fam = static_cast<Familie> (z.bis (3));
            const std::size_t n = static_cast<std::size_t> (z.bis (60));
            std::vector<std::uint8_t> nutz (n, static_cast<std::uint8_t> ('a'));
            envelopeSchreiben (fam, static_cast<std::uint8_t> (z.bis (256)),
                               nutz.data(), n, gut2);
            const std::size_t pos = static_cast<std::size_t> (z.bis (gut2.size()));
            gut2[pos] ^= static_cast<std::uint8_t> (1u << z.bis (8));
            const auto u = envelopePruefen (gut2.data(), gut2.size());
            if (! u.gueltig) { ++abgelehntMutiert; continue; }
            ++angenommenMutiert;
            const std::uint32_t frameLen = static_cast<std::uint32_t> (gut2[0])
                                         | (static_cast<std::uint32_t> (gut2[1]) << 8)
                                         | (static_cast<std::uint32_t> (gut2[2]) << 16)
                                         | (static_cast<std::uint32_t> (gut2[3]) << 24);
            if (! (frameLen >= kKopfBytes && frameLen <= kMaxFrameBytes
                   && static_cast<std::uint64_t> (frameLen)
                          == static_cast<std::uint64_t> (kKopfBytes) + u.kopf.payloadLen
                   && u.kopf.flags == 0 && u.kopf.schemaMajor == kSchemaMajor
                   && u.kopf.encoding == erwarteteKodierung (u.kopf.familie)
                   && u.drahtlaenge == 4u + frameLen))
                invarianteMutiert = false;
        }
        pruefe (invarianteMutiert && angenommenMutiert > 0 && abgelehntMutiert > 0,
                "ein gekipptes Bit in einem GUELTIGEN Frame: was durchkommt, "
                "haelt jede Kopfregel",
                std::to_string (angenommenMutiert) + " angenommen / "
                    + std::to_string (abgelehntMutiert) + " abgelehnt");

        // Ein gekipptes Bit im P2-Payload faellt IMMER.
        bool immerGefangen = true;
        for (int runde = 0; runde < 3000; ++runde)
        {
            const std::size_t n = 1 + static_cast<std::size_t> (z.bis (120));
            std::vector<std::uint8_t> nutz (n);
            for (auto& b : nutz) b = static_cast<std::uint8_t> (z.naechste() & 0xFF);
            std::vector<std::uint8_t> frame;
            envelopeSchreiben (Familie::p2, 0, nutz.data(), n, frame);
            const std::size_t pos = 4 + kKopfBytes + static_cast<std::size_t> (z.bis (n));
            frame[pos] ^= static_cast<std::uint8_t> (1u << z.bis (8));
            const auto u = envelopePruefen (frame.data(), frame.size());
            if (u.gueltig || namen (u.verstoesse) != std::vector<std::string> { "crc_falsch" })
                immerGefangen = false;
        }
        pruefe (immerGefangen, "3000 gekippte P2-Payloadbits: jedes einzelne faellt an der CRC");

        // Fragmentierung: byteweise Zustellung liefert dieselben Frames.
        std::vector<std::vector<std::uint8_t>> quellen;
        std::vector<std::uint8_t> strom;
        for (int i = 0; i < 40; ++i)
        {
            const auto fam = static_cast<Familie> (z.bis (3));
            const std::size_t n = static_cast<std::size_t> (z.bis (200));
            std::vector<std::uint8_t> nutz (n, static_cast<std::uint8_t> ('x'));
            std::vector<std::uint8_t> frame;
            envelopeSchreiben (fam, 0, nutz.data(), n, frame);
            quellen.push_back (nutz);
            strom.insert (strom.end(), frame.begin(), frame.end());
        }
        StromLeser leser;
        std::size_t gefunden = 0;
        bool gleich = true;
        for (std::size_t i = 0; i < strom.size(); ++i)
        {
            leser.fuettern (&strom[i], 1);
            for (;;)
            {
                const auto e = leser.naechster();
                if (e.art == StromLeser::Art::unvollstaendig) break;
                if (e.art == StromLeser::Art::verstoss) { gleich = false; break; }
                if (gefunden >= quellen.size()
                    || e.payloadLaenge != quellen[gefunden].size()
                    || (e.payloadLaenge > 0
                        && std::memcmp (e.payload, quellen[gefunden].data(), e.payloadLaenge) != 0))
                    gleich = false;
                ++gefunden;
            }
        }
        pruefe (gefunden == quellen.size() && gleich && leser.offen() == 0,
                "byteweise Zustellung liefert exakt dieselben 40 Frames",
                std::to_string (gefunden) + " Frames, Rest " + std::to_string (leser.offen()));

        // Ein kaputter Frame beendet den Strom — es wird nicht resynchronisiert.
        std::vector<std::uint8_t> gut, kaputt;
        const char* leer = "{}";
        envelopeSchreiben (Familie::p0, 0, reinterpret_cast<const std::uint8_t*> (leer), 2, gut);
        kaputt = gut;
        kaputt[4] = 9;
        StromLeser l2;
        l2.fuettern (kaputt.data(), kaputt.size());
        l2.fuettern (gut.data(), gut.size());
        const auto e = l2.naechster();
        pruefe (e.art == StromLeser::Art::verstoss
                    && namen (e.verstoesse) == std::vector<std::string> { "encoding_unbekannt" },
                "ein kaputter Frame im Strom wird gemeldet, nicht uebersprungen");
    }

    // ── D · Pipetoken (§48.3) ─────────────────────────────────────────────
    abschnitt ("D · v3-Pipename als reine Funktion");
    {
        pruefe (pipeToken ("S-1-5-21-111111111-222222222-333333333-1001")
                    == "BNSM62JZZCCXIDV3PJZAEHMZPA",
                "Golden aus Entwurf §48.3");
        pruefe (pipeNameV3 ("S-1-5-21-111111111-222222222-333333333-1001")
                    == "\\\\.\\pipe\\evenacadia.nakama.v3.BNSM62JZZCCXIDV3PJZAEHMZPA",
                "vollstaendiger Pipename");
        pruefe (pipeToken ("s-1-5-21-111111111-222222222-333333333-1001")
                    == pipeToken ("S-1-5-21-111111111-222222222-333333333-1001"),
                "Schreibweise der SID ist egal (Grossschreibung vor dem Hashen)");
        pruefe (pipeToken ("S-1-5-21-1-2-3-1001") != pipeToken ("S-1-5-21-1-2-3-1002"),
                "verschiedene SIDs, verschiedene Token");

        // Der Riegel der Probeprogramme. Er ist eine ERLAUBNIS, keine
        // Sperrliste — die alte Sperrliste kannte nur den v1-Namen und liess
        // ausgerechnet den GOLDEN-Namen aus §48.3 durch.
        pruefe (! istProbePipename (pipeNameV3 ("S-1-5-21-111111111-222222222-333333333-1001")),
                "der Golden-Pipename wird als Probe-Name VERWEIGERT");
        pruefe (! istProbePipename ("\\\\.\\pipe\\evenacadia.eq-copilot.v1"),
                "die v1-Produktions-Pipe ebenso");
        pruefe (! istProbePipename ("\\\\.\\pipe\\evenacadia.nakama.v3.last.4711"),
                "und ein v3-Name ausserhalb des Probe-Namensraums");
        pruefe (! istProbePipename (kPipePraefixProbe),
                "`probe.` allein ist keine Pipe, sondern nur der Namensraum");
        pruefe (istProbePipename (testPipeName ("riegel")),
                "der eigene Testname liegt im Probe-Namensraum");

        // SHA-256 gegen zwei bekannte Vektoren (FIPS 180-4 Anhang B).
        auto hexDigest = [] (const std::string& s) {
            std::uint8_t d[32];
            sha256 (reinterpret_cast<const std::uint8_t*> (s.data()), s.size(), d);
            static const char* zi = "0123456789abcdef";
            std::string aus;
            for (auto b : d) { aus += zi[b >> 4]; aus += zi[b & 15]; }
            return aus;
        };
        pruefe (hexDigest ("abc")
                    == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
                "SHA-256(\"abc\") wie FIPS 180-4");
        pruefe (hexDigest ("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq")
                    == "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1",
                "SHA-256 des 56-Byte-Vektors (Zweiblockfall)");

        auto b32 = [] (const char* s) {
            return base32 (reinterpret_cast<const std::uint8_t*> (s), std::strlen (s));
        };
        pruefe (b32 ("f") == "MY" && b32 ("fo") == "MZXQ" && b32 ("foo") == "MZXW6"
                    && b32 ("foob") == "MZXW6YQ" && b32 ("fooba") == "MZXW6YTB"
                    && b32 ("foobar") == "MZXW6YTBOI",
                "Base32 gegen die Testvektoren aus RFC 4648 §10");

        const auto eigene = aktuelleLogonSid();
        pruefe (eigene.rfind ("S-1-", 0) == 0, "eigene Logon-SID ist wohlgeformt", eigene);
    }

    // ── E · Backpressure ──────────────────────────────────────────────────
    abschnitt ("E · Backpressure: P0 · P1 · P2");
    {
        P0Warteschlange p0;
        bool alleDurch = true;
        for (int i = 0; i < 64; ++i)
            alleDurch = alleDurch && p0.einreihen ("n" + std::to_string (i));
        pruefe (alleDurch && p0.kapazitaet() == 64, "P0 nimmt 64 Nachrichten");
        pruefe (! p0.einreihen ("zuviel"),
                "die 65. meldet Ueberlauf (nichts verwerfen ⇒ Verbindung schliessen)");
        bool reihenfolge = true;
        for (int i = 0; i < 64; ++i)
        {
            std::string s;
            reihenfolge = reihenfolge && p0.entnehmen (s) && s == "n" + std::to_string (i);
            p0.bestaetigen();   // "auf dem Draht" — erst das gibt den Platz frei
        }
        std::string weg;
        pruefe (reihenfolge && ! p0.entnehmen (weg) && p0.inFlug() == 0,
                "alle 64 kommen unveraendert und in Reihenfolge zurueck");

        // ── T2-Befund 1: der Platz des unterwegs befindlichen Eintrags ────
        //
        // Solange ein Befehl zwischen Queue und Pipe steht, darf sein Platz
        // NICHT neu vergeben werden. Sonst hat das Zuruecklegen nach einem
        // gescheiterten Write keinen Platz mehr — und der Befehl ist weg,
        // ohne dass es der Ueberlaufzaehler meldet.
        P0Warteschlange p0c (64);
        for (int i = 0; i < 64; ++i)
            p0c.einreihen ("n" + std::to_string (i));
        std::string unterwegs;
        pruefe (p0c.entnehmen (unterwegs) && unterwegs == "n0" && p0c.inFlug() == 1,
                "der Sender entnimmt den ersten Befehl und reserviert seinen Platz");
        int angenommen = 0;
        for (int i = 0; i < 64; ++i)
            if (p0c.einreihen ("neu" + std::to_string (i)))
                ++angenommen;
        pruefe (angenommen == 0 && p0c.ueberlauf() == 64,
                "64 neue Befehle waehrend des Writes finden KEINEN Platz — ehrlich gezaehlt",
                std::to_string (angenommen) + " angenommen, "
                    + std::to_string (p0c.ueberlauf()) + " Ueberlaeufe");
        p0c.zuruecklegen (unterwegs);
        pruefe (p0c.groesse() == 64 && p0c.inFlug() == 0,
                "der gescheiterte Write legt ihn zurueck; die Queue traegt wieder genau 64",
                std::to_string (p0c.groesse()) + " Eintraege");
        std::string wiederVorn;
        pruefe (p0c.entnehmen (wiederVorn) && wiederVorn == "n0",
                "und zwar VORN — kein P0-Befehl ist verlorengegangen");

        P1Warteschlange p1 (4, 4);
        pruefe (p1.einreihen ("a", "a1") == P1Ergebnis::eingereiht, "P1 nimmt Snapshot a");
        pruefe (p1.einreihen ("", "ereignis") == P1Ergebnis::eingereiht, "P1 nimmt Ereignis");
        pruefe (p1.einreihen ("b", "b1") == P1Ergebnis::eingereiht, "P1 nimmt Snapshot b");
        pruefe (p1.einreihen ("a", "a2") == P1Ergebnis::koalesziert
                    && p1.groesse() == 3,
                "ein zweiter Snapshot desselben Objekts koalesziert, ohne zu wachsen");
        std::string s1, s2, s3;
        p1.entnehmen (s1); p1.bestaetigen();
        p1.entnehmen (s2); p1.bestaetigen();
        p1.entnehmen (s3); p1.bestaetigen();
        pruefe (s1 == "a2" && s2 == "ereignis" && s3 == "b1",
                "Koaleszieren behaelt die Position, tauscht nur den Inhalt");

        P1Warteschlange p1b (2, 2);
        p1b.einreihen ("", "1"); p1b.einreihen ("", "2");
        pruefe (p1b.einreihen ("", "3") == P1Ergebnis::zurWiederholung
                    && p1b.einreihen ("", "4") == P1Ergebnis::zurWiederholung,
                "nicht koaleszierbare Ereignisse gehen in den Wiederholpuffer");
        // Der volle Wiederholpuffer weist den NEUZUGANG ab. Die alte Fassung
        // machte mit `pop_front()` Platz und loeschte damit ein bereits
        // ANGENOMMENES Ereignis: bei 2/2 und den Ereignissen 1…5 verschwand
        // genau die Nr. 3 (T2-Befund 1 Runde 3). §53.9 verlangt Wiederholung,
        // nicht Verdraengung.
        // Erst rechnen, dann pruefen: die Auswertungsreihenfolge der
        // Argumente von `pruefe` ist unbestimmt — stuende der Aufruf im
        // Bedingungsargument, koennte der Detailtext den Stand VOR ihm zeigen.
        const auto fuenfte = p1b.einreihen ("", "5");
        const auto abgewiesen5 = p1b.abgewiesene();
        const auto vorgehalten5 = p1b.wiederholungen();
        pruefe (fuenfte == P1Ergebnis::abgewiesen
                    && abgewiesen5 == 1 && vorgehalten5 == 2,
                "ein voller Wiederholpuffer weist das NEUE Ereignis ab, gezaehlt, nie still",
                std::to_string (vorgehalten5) + " vorgehalten, "
                    + std::to_string (abgewiesen5) + " abgewiesen");
        // `A-P1-06` (Regel 1): der Wiederholpuffer fliesst ab, sobald Platz
        // frei wird — OHNE Reconnect. Vorher war `nachReconnectWiederholen()`
        // sein einziger Abfluss; bei nur voruebergehendem Rueckstau blieben
        // akzeptierte Ereignisse unbegrenzt liegen (NAK-95, Befund 1).
        std::string x;
        p1b.entnehmen (x); p1b.bestaetigen();
        const auto nachErstemSenden = p1b.wiederholungen();
        pruefe (nachErstemSenden == 1 && p1b.groesse() == 2,
                "ein einziges Senden zieht eine Wiederholung nach — ohne Reconnect",
                std::to_string (nachErstemSenden) + " noch vorgehalten");
        p1b.entnehmen (x); p1b.bestaetigen();
        pruefe (p1b.wiederholungen() == 0 && p1b.nachReconnectWiederholen() == 0,
                "beim Reconnect ist nichts mehr nachzuholen — es floss schon ab");
        p1b.entnehmen (s1); p1b.bestaetigen();
        p1b.entnehmen (s2); p1b.bestaetigen();
        pruefe (s1 == "3" && s2 == "4" && p1b.wiederholungen() == 0,
                "und zwar JEDES angenommene Ereignis, in der urspruenglichen Reihenfolge",
                s1 + "," + s2);

        // `A-P1-07` (Regel 1): ein Neuzugang ueberholt nie eine bereits
        // angenommene Wiederholung.
        {
            P1Warteschlange q (1, 4);
            q.einreihen ("", "1");
            q.einreihen ("", "2");          // in den Wiederholpuffer
            std::string weg;
            q.entnehmen (weg); q.bestaetigen();   // Platz frei, "2" rueckt nach
            pruefe (weg == "1" && q.einreihen ("", "3") == P1Ergebnis::zurWiederholung,
                    "ein Neuzugang findet keinen Platz, solange eine Wiederholung wartet");
            std::string a2, a3;
            q.entnehmen (a2); q.bestaetigen();
            q.entnehmen (a3); q.bestaetigen();
            pruefe (a2 == "2" && a3 == "3",
                    "und er ueberholt sie nicht — Annahmereihenfolge ueber beide Puffer",
                    a2 + "," + a3);
        }

        // `A-P1-03` (Regel 2): der Objektschluessel ueberlebt den
        // Wiederholpuffer, und Koaleszierung gilt auch dort. Vorher verlor der
        // Snapshot dort seinen Schluessel; ein neuerer wurde abgewiesen und der
        // AELTERE kam als Ereignis zurueck (NAK-95, Befund 2).
        {
            P1Warteschlange q (1, 2);
            q.einreihen ("", "fremd");                       // haelt die Queue belegt
            const auto alt = q.einreihen ("k", "alt");
            const auto neu = q.einreihen ("k", "neu");
            pruefe (alt == P1Ergebnis::zurWiederholung && neu == P1Ergebnis::koalesziert
                        && q.wiederholungen() == 1 && q.abgewiesene() == 0,
                    "ein Snapshot im Wiederholpuffer behaelt seinen Schluessel und "
                    "koalesziert dort",
                    std::to_string (q.wiederholungen()) + " vorgehalten, "
                        + std::to_string (q.abgewiesene()) + " abgewiesen");
            std::string e1, e2;
            q.entnehmen (e1); q.bestaetigen();
            q.entnehmen (e2); q.bestaetigen();
            pruefe (e1 == "fremd" && e2 == "neu",
                    "und der NEUERE geht raus, nicht der aeltere", e1 + "," + e2);
        }

        // Entnommen, aber nicht geschrieben: der Eintrag muss zurueck. Ohne
        // diesen Weg verschwand ein P1-Ereignis endgueltig, wenn der Write
        // nach der Entnahme scheiterte (T2-Befund 4).
        P1Warteschlange p1c (4, 4);
        p1c.einreihen ("", "e1");
        p1c.einreihen ("k", "s1");
        std::string sk, sn;
        pruefe (p1c.entnehmen (sk, sn) && sk.empty() && sn == "e1",
                "entnehmen liefert Objektschluessel UND Nachricht");
        pruefe (p1c.zuruecklegen (sk, sn) == P1Ergebnis::eingereiht && p1c.groesse() == 2,
                "ein gescheiterter Write legt das Ereignis an seinen Platz zurueck");
        std::string wieder;
        pruefe (p1c.entnehmen (wieder) && wieder == "e1",
                "und es steht wieder VORN, nicht hinten");

        // Ein Snapshot dagegen darf nicht zurueckkehren, wenn schon ein
        // neuerer desselben Objekts wartet — der neuere ist die Wahrheit.
        P1Warteschlange p1d (4, 4);
        p1d.einreihen ("k", "alt");
        std::string dk, dn;
        p1d.entnehmen (dk, dn);
        p1d.einreihen ("k", "neu");
        pruefe (p1d.zuruecklegen (dk, dn) == P1Ergebnis::koalesziert && p1d.groesse() == 1,
                "ein zurueckgelegter Snapshot weicht dem neueren, statt ihn zu verdraengen");
        std::string dz;
        p1d.entnehmen (dz);
        pruefe (dz == "neu", "und der neuere geht raus");

        // Auch bei P1 haelt die Reservierung den Platz des unterwegs
        // befindlichen Eintrags frei: ein waehrenddessen eingereihtes Ereignis
        // findet KEINEN Platz und wartet auf den Reconnect, statt den Platz zu
        // belegen, an den der unterwegs befindliche zurueckmuss.
        P1Warteschlange p1e (1, 2);
        p1e.einreihen ("", "erst");
        std::string ek, en;
        p1e.entnehmen (ek, en);
        pruefe (p1e.inFlug() == 1,
                "der entnommene P1-Eintrag zaehlt weiter gegen die Kapazitaet");
        pruefe (p1e.einreihen ("", "zweit") == P1Ergebnis::zurWiederholung
                    && p1e.wiederholungen() == 1,
                "ein neues Ereignis findet deshalb keinen Platz und wartet auf den Reconnect");
        pruefe (p1e.zuruecklegen (ek, en) == P1Ergebnis::eingereiht
                    && p1e.groesse() == 1 && p1e.inFlug() == 0,
                "und der unterwegs gewesene Eintrag kommt an SEINEN Platz zurueck");

        // P0 kennt dieselbe Zusage, nur strenger: nichts verwerfen.
        P0Warteschlange p0b (2);
        p0b.einreihen ("a"); p0b.einreihen ("b");
        std::string p0raus;
        p0b.entnehmen (p0raus);
        p0b.zuruecklegen (p0raus);
        pruefe (p0b.groesse() == 2,
                "ein nicht geschriebener P0-Befehl geht ebenfalls zurueck");
        std::string p0wieder;
        pruefe (p0b.entnehmen (p0wieder) && p0wieder == "a",
                "und behaelt seine Reihenfolge");
    }

    abschnitt ("E2 · P2-Schleuse: vorallokiert, ohne Allokation, replace-oldest");
    {
        auto schleuse = std::make_unique<P2Schleuse<1024>>();
        std::uint8_t frame[64];
        std::uint8_t ziel[1024];

        for (int i = 0; i < 3; ++i)
        {
            std::memset (frame, i + 1, sizeof (frame));
            schleuse->veroeffentlichen (frame, sizeof (frame));
        }
        pruefe (schleuse->ersetzteFrames() == 1,
                "der dritte Frame ersetzt den aeltesten ungesendeten (Cap 2)");
        const auto n1 = schleuse->abholen (ziel, sizeof (ziel));
        pruefe (n1 == 64 && ziel[0] == 2, "abgeholt wird der zweite, nicht der erste",
                std::to_string ((int) ziel[0]));
        const auto n2 = schleuse->abholen (ziel, sizeof (ziel));
        pruefe (n2 == 64 && ziel[0] == 3, "danach der dritte");
        pruefe (schleuse->abholen (ziel, sizeof (ziel)) == 0, "danach ist die Schleuse leer");

        pruefe (! schleuse->veroeffentlichen (frame, 2048) && schleuse->zuGrosseFrames() == 1,
                "ein Frame ueber der Slotgroesse wird gezaehlt verworfen, nie halb geschrieben");

        // Der eigentliche Punkt: 100 000 Veroeffentlichungen ohne EINE Allokation.
        zaehleAllokationen = true;
        allokationen = 0;
        for (int i = 0; i < 100000; ++i)
        {
            frame[0] = static_cast<std::uint8_t> (i & 0xFF);
            schleuse->veroeffentlichen (frame, sizeof (frame));
            if ((i & 3) == 0)
                schleuse->abholen (ziel, sizeof (ziel));
        }
        const auto gezaehlt = allokationen;
        zaehleAllokationen = false;
        pruefe (gezaehlt == 0, "100 000 Uebergaben mit 0 Allokationen",
                std::to_string (gezaehlt));

        // Gegenprobe: der Zaehler zaehlt wirklich. Ohne sie waere die Null
        // oben genauso gut das Ergebnis eines abgeschalteten Zaehlers.
        zaehleAllokationen = true;
        allokationen = 0;
        { std::vector<std::uint8_t> gegenprobe (4096); gegenprobe[0] = 1; }
        const auto gegen = allokationen;
        zaehleAllokationen = false;
        pruefe (gegen > 0, "Gegenprobe: derselbe Zaehler sieht eine echte Allokation",
                std::to_string (gegen));

        // ── Das Rennen selbst ─────────────────────────────────────────────
        //
        // EHRLICH VORWEG: auf dieser Maschine gibt es keinen Race Detector.
        // ThreadSanitizer unterstuetzt Windows/MSVC nicht, und /fsanitize=
        // address findet keine Datenrennen. Der Beweis kann deshalb nicht
        // lauten "ein Werkzeug hat kein Rennen gesehen". Er lautet:
        //
        //   1. der Kollisionsfall TRITT EIN — der Erzeuger laeuft unter Flut
        //      wirklich auf den Platz, den der Verbraucher gerade beansprucht
        //      hat (`kollisionsLoecher() > 0`);
        //   2. und er endet jedes Mal OHNE Schreibzugriff auf DIESEN Platz,
        //      weil ein einziges Atomic den Besitz entscheidet — der Frame
        //      selbst faellt dabei NICHT, er geht in den naechsten Platz
        //      (`beanspruchtVerworfen() == 0`, T2-Befund 2 Runde 3);
        //   3. dazu, als aeussere Probe, ueber Millionen Frames kein einziger
        //      zerrissener Frame, keine falsche Laenge und keine ruecklaeufige
        //      Folgenummer — die Loecher brechen die Reihenfolge nicht.
        //
        // Die grossen Frames sind Absicht: ein 8000-Byte-memcpy haelt das
        // Zeitfenster des Verbrauchers lange genug offen, dass der
        // Kollisionsfall nicht vom Zufall der Taktung abhaengt.
        {
            auto s2 = std::make_unique<P2Schleuse<256>>();
            std::atomic<bool> stopp { false };
            std::atomic<long long> zerrissen { 0 }, geholt { 0 };
            std::thread erzeuger ([&] {
                std::uint8_t f[128];
                for (int i = 0; i < 2000000 && ! stopp.load(); ++i)
                {
                    std::memset (f, static_cast<int> (i & 0xFF), sizeof (f));
                    s2->veroeffentlichen (f, sizeof (f));
                }
            });
            std::thread verbraucher ([&] {
                std::uint8_t z[256];
                while (! stopp.load())
                {
                    const auto n = s2->abholen (z, sizeof (z));
                    if (n == 0) continue;
                    ++geholt;
                    if (n != 128) { ++zerrissen; continue; }
                    for (std::size_t i = 1; i < n; ++i)
                        if (z[i] != z[0]) { ++zerrissen; break; }
                }
            });
            erzeuger.join();
            stopp.store (true);
            verbraucher.join();
            pruefe (zerrissen.load() == 0 && geholt.load() > 0,
                    "2 000 000 kleine Frames unter Flut: kein zerrissener Frame",
                    std::to_string (geholt.load()) + " geholt, "
                        + std::to_string (zerrissen.load()) + " zerrissen, "
                        + std::to_string (s2->ersetzteFrames()) + " ersetzt");
        }
        {
            auto s3 = std::make_unique<P2Schleuse<8192>>();
            constexpr std::size_t kGross = 8000;
            std::atomic<bool> stopp { false };
            std::atomic<long long> zerrissen { 0 }, geholt { 0 }, ruecklaeufig { 0 };
            // Die ersten vier Bytes tragen die Folgenummer im Klartext. Ohne
            // sie sagt der Test nur "nicht zerrissen" — nichts darueber, ob
            // die uebersprungenen Positionen (die Loecher der Kollision) die
            // Reihenfolge zerreissen oder einen Frame doppelt liefern.
            std::thread erzeuger ([&] {
                std::vector<std::uint8_t> f (kGross);
                for (std::uint32_t i = 0; i < 300000 && ! stopp.load(); ++i)
                {
                    std::memset (f.data() + 4, static_cast<int> (i & 0xFF), f.size() - 4);
                    std::memcpy (f.data(), &i, sizeof (i));
                    s3->veroeffentlichen (f.data(), f.size());
                }
            });
            std::thread verbraucher ([&] {
                std::vector<std::uint8_t> z (8192);
                long long letzte = -1;
                while (! stopp.load())
                {
                    const auto n = s3->abholen (z.data(), z.size());
                    if (n == 0) continue;
                    ++geholt;
                    if (n != kGross) { ++zerrissen; continue; }
                    std::uint32_t folge = 0;
                    std::memcpy (&folge, z.data(), sizeof (folge));
                    const auto fuellung = static_cast<std::uint8_t> (folge & 0xFF);
                    for (std::size_t i = 4; i < n; ++i)
                        if (z[i] != fuellung) { ++zerrissen; break; }
                    if (static_cast<long long> (folge) <= letzte) ++ruecklaeufig;
                    letzte = static_cast<long long> (folge);
                }
            });
            erzeuger.join();
            stopp.store (true);
            verbraucher.join();
            pruefe (zerrissen.load() == 0 && geholt.load() > 0,
                    "300 000 grosse Frames (8000 B) ebenso: kein zerrissener Frame",
                    std::to_string (geholt.load()) + " geholt, "
                        + std::to_string (zerrissen.load()) + " zerrissen");
            pruefe (ruecklaeufig.load() == 0,
                    "und keine ruecklaeufige oder doppelte Folgenummer — die Loecher "
                    "der Kollision brechen die Reihenfolge nicht",
                    std::to_string (ruecklaeufig.load()) + " ruecklaeufig");
            pruefe (s3->kollisionsLoecher() > 0,
                    "der Erzeuger traf den beanspruchten Slot WIRKLICH — und hat ihn "
                    "nicht beschrieben",
                    std::to_string (s3->kollisionsLoecher())
                        + " uebersprungene Positionen");
            pruefe (s3->beanspruchtVerworfen() == 0,
                    "der NEUESTE Frame faellt dabei NIE — es weicht der aelteste "
                    "wartende (replace-oldest, §53.9)",
                    std::to_string (s3->beanspruchtVerworfen())
                        + " neueste wegen fremden Anspruchs verworfen");
        }

        // ── Dieselbe Zusage, DETERMINISTISCH erzwungen (`A-P2-04`) ────────
        //
        // Die Lastprobe oben trifft die Kollision nur, wenn die Taktung
        // mitspielt — genau deshalb fiel NAK-98 unter Baulast auf und war
        // einzeln fuenfmal gruen. Hier werden die Plaetze mit dem Testhaken
        // beansprucht: derselbe Fall, ohne Zufall. Beansprucht sind die BEIDEN
        // Plaetze, auf die der Erzeuger zuerst laeuft — das ist die Lage, in
        // der die alte Fassung (zwei Versuche) den neuesten Frame opferte.
        {
            auto s4 = std::make_unique<P2Schleuse<256>>();
            const auto stand = s4->testSchreibstand();
            const bool belegt = s4->testSlotBeanspruchen (stand)
                             && s4->testSlotBeanspruchen (stand + 1);
            pruefe (belegt, "Testhaken: die zwei naechsten Plaetze sind beansprucht");

            std::uint8_t f[64];
            std::memset (f, 0xAB, sizeof (f));
            const bool uebernommen = s4->veroeffentlichen (f, sizeof (f));
            pruefe (uebernommen && s4->beanspruchtVerworfen() == 0,
                    "erzwungene Slot-Kollision: der neueste Frame findet immer einen Platz",
                    std::to_string (s4->beanspruchtVerworfen())
                        + " verworfen, " + std::to_string (s4->kollisionsLoecher())
                        + " Loecher");
            pruefe (s4->kollisionsLoecher() == 2,
                    "und die beiden beanspruchten Positionen sind LOECHER, nicht Verluste",
                    std::to_string (s4->kollisionsLoecher()));

            s4->testSlotFreigeben (stand);
            s4->testSlotFreigeben (stand + 1);
            std::uint8_t ziel[256];
            const auto n = s4->abholen (ziel, sizeof (ziel));
            pruefe (n == sizeof (f) && ziel[0] == 0xAB,
                    "der Verbraucher ueberspringt die Loecher und bekommt genau diesen Frame",
                    std::to_string (n) + " Bytes");
        }

        // ── Der WANDERNDE Verbraucher (`A-P2-04`, NAK-104) ────────────────
        //
        // Die Probe oben haelt zwei Plaetze gleichzeitig und laesst den dritten
        // statisch frei. Sie zeigt nur, dass ZWEI Versuche zu wenig sind — das
        // Wandern eines EINZELNEN Verbrauchers bildet sie nicht ab.
        //
        // Hier wandert er: er gibt zwischen zwei Anspruchsversuchen des
        // Erzeugers seinen Platz frei, meldet seinen Fortschritt und
        // beansprucht den naechsten Kandidaten. Genau das darf ein echter
        // Verbraucher, und genau daran scheiterte die feste Schranke
        // `kSlots * 2`: liegt er sechs Positionen zurueck, verbraucht er alle
        // sechs Versuche, und der NEUESTE Frame fiel.
        {
            using Schleuse5 = P2Schleuse<256, WandernderVerbraucher>;
            auto s5 = std::make_unique<Schleuse5>();
            std::uint8_t f[64];
            std::memset (f, 0xCD, sizeof (f));

            // 1) Den Rueckstand aufbauen — mit demselben Mittel, das ihn im
            //    Betrieb aufbaut: je ein Kollisionsloch je Veroeffentlichung
            //    laesst `schreib` um zwei und `boden` nur um eins wachsen.
            constexpr std::uint64_t kRueckstand = Schleuse5::kSlots * 2;  // die alte feste Schranke
            int runden = 0;
            while (s5->testSchreibstand() - s5->testBodenstand() < kRueckstand && runden < 64)
            {
                const auto stand = s5->testSchreibstand();
                if (! s5->testSlotBeanspruchen (stand))
                    break;
                s5->veroeffentlichen (f, sizeof (f));
                s5->testSlotFreigeben (stand);
                ++runden;
            }
            const auto boden5   = s5->testBodenstand();
            const auto schreib5 = s5->testSchreibstand();
            const auto fenster  = schreib5 - boden5;
            pruefe (fenster == kRueckstand && (fenster % Schleuse5::kSlots) == 0,
                    "Kollisionsloecher lassen den Verbraucher wirklich zurueckfallen "
                    "(schreib - boden waechst)",
                    std::to_string (fenster) + " Positionen Rueckstand nach "
                        + std::to_string (runden) + " Runden");

            // 2) Jetzt WANDERT er: vor jedem Versuch des Erzeugers gibt er den
            //    alten Platz frei und beansprucht den naechsten Kandidaten.
            s5->haken.position = boden5;
            s5->haken.grenze   = schreib5;
            s5->haken.aktiv    = true;
            const auto loecherVorher = s5->kollisionsLoecher();
            // Eigene Fuellung: sonst waere die Abholprobe unten auch dann
            // gruen, wenn ein Frame aus dem Aufbau geliefert wird.
            std::uint8_t neuester[64];
            std::memset (neuester, 0xE7, sizeof (neuester));
            const bool uebernommen5 = s5->veroeffentlichen (neuester, sizeof (neuester));
            s5->haken.aktiv = false;

            pruefe (s5->haken.spruenge == static_cast<int> (kRueckstand),
                    "der Verbraucher ist zwischen JEDEM Versuch weitergerueckt — "
                    "kein statisch freier dritter Platz",
                    std::to_string (s5->haken.spruenge) + " Spruenge");
            pruefe (uebernommen5 && s5->beanspruchtVerworfen() == 0,
                    "und der Erzeuger bekommt TROTZDEM einen Platz — garantiert, "
                    "nicht begrenzt versucht",
                    std::to_string (s5->beanspruchtVerworfen()) + " verworfen, "
                        + std::to_string (s5->kollisionsLoecher() - loecherVorher)
                        + " Loecher in dieser Veroeffentlichung");
            std::uint8_t ziel5[256];
            const auto n5 = s5->abholen (ziel5, sizeof (ziel5));
            pruefe (n5 == sizeof (neuester) && ziel5[0] == 0xE7,
                    "der neueste Frame liegt danach wirklich da, hinter allen Loechern",
                    std::to_string (n5) + " Bytes");
        }
    }

    abschnitt ("F · Ratengrenze");
    {
        Ratengrenze r (1000, 1000);
        int durch = 0;
        for (int i = 0; i < 5000; ++i)
            if (r.erlaubt (0)) ++durch;
        pruefe (durch == 1000, "5000 Nachrichten in derselben Millisekunde: 1000 gehen durch",
                std::to_string (durch));
        int durch2 = 0;
        for (int i = 0; i < 5000; ++i)
            if (r.erlaubt (1000)) ++durch2;
        pruefe (durch2 == 1000, "ein volles Fenster spaeter wieder 1000");
        Ratengrenze r2 (10, 100);
        bool immer = true;
        for (std::uint64_t t = 0; t < 10000; ++t)
            immer = immer && r2.erlaubt (t * 10);
        pruefe (immer, "gleitend: 1 Nachricht je 10 ms laeuft unbegrenzt");
    }

    // ── G · Die echten Clients gegen einen v3-Server ──────────────────────
    abschnitt ("G · ControlClient und TelemetryClient auf einer Testpipe");
    {
        TestServer server (testPipeName ("kopplung"));
        server.starten();

        auto adresse = testAdresse (hex32 ('7'));
        const std::string legacyInstanceId = "11111111-2222-3333-4444-555555555555";
        const std::string wireInstanceId = "239953e95f122e18abb34ba1add46f9e";
        adresse.instanceId = legacyInstanceId;
        ControlClient control ([&] {
            ControlHello h;
            h.adresse = adresse;
            h.pluginKind = "active_probe";
            return h;
        }, server.pipeName());

        control.start();
        pruefe (warteAuf (4000, [&] {
                    return control.snapshot().status == ControlClient::Status::verbunden;
                }),
                "Control verbindet und bekommt ein v3-gerahmtes welcome",
                control.snapshot().letzterFehler);

        std::string linkId, challenge;
        pruefe (control.kopplung (linkId, challenge)
                    && linkId == hex32 ('a') && challenge == hex32 ('b'),
                "welcome liefert link_id und challenge");

        TelemetryClient telemetrie ([&] {
            TelemetryHello t;
            t.adresse = adresse;
            std::string l, c;
            control.kopplung (l, c);
            t.linkId = l;
            t.challenge = c;
            return t;
        }, server.pipeName());
        telemetrie.start();
        pruefe (warteAuf (4000, [&] {
                    return telemetrie.snapshot().status == TelemetryClient::Status::verbunden;
                }),
                "Telemetry koppelt mit link_id + challenge + derselben runtime_nonce",
                telemetrie.snapshot().letzterFehler);

        nakama::analyse::FeatureFrame feature {};
        feature.metricsVersion = 1;
        feature.transport.transport_epoch = 1;
        feature.transport.continuity_segment = 1;
        feature.transport.sequence = 1;
        feature.transport.zeitbasis = nakama::analyse::Zeitbasis::project_samples;
        feature.transport.project_sample_start_gesetzt = true;
        feature.transport.sample_count = 512;
        feature.transport.sample_rate = 48000.0;
        feature.transport.gueltigkeit = nakama::analyse::kGProjectTime;
        feature.transport.process_context_present_gesetzt = true;
        feature.transport.process_context_present = true;
        feature.live.gitter = nakama::analyse::GitterId::nakama_log64_v1;
        feature.live.encoding = nakama::analyse::BandEncoding::q_db_0p1_i16;
        std::fill (std::begin (feature.live.bitmap), std::end (feature.live.bitmap),
                   std::uint8_t { 0xff });
        feature.liveBreite[0] = 0.5f;
        feature.liveBreiteBitmap[0] = 0x01;

        auto projektOhneBit = feature;
        projektOhneBit.transport.gueltigkeit &= ~nakama::analyse::kGProjectTime;
        pruefe (nakama::analyse::nak29Verstoss (projektOhneBit.transport) == 1
                    && ! telemetrie.veroeffentlichen (projektOhneBit, adresse),
                "nak29_sender_project_samples_ohne_project_time_bit");

        auto lokalMitProjektstart = feature;
        lokalMitProjektstart.transport.zeitbasis =
            nakama::analyse::Zeitbasis::local_monotonic;
        lokalMitProjektstart.transport.gueltigkeit &= ~nakama::analyse::kGProjectTime;
        pruefe (nakama::analyse::nak29Verstoss (lokalMitProjektstart.transport) == 2
                    && ! telemetrie.veroeffentlichen (lokalMitProjektstart, adresse),
                "nak29_sender_local_monotonic_mit_project_sample_start");

        const bool featureAngenommen = telemetrie.veroeffentlichen (feature, adresse);
        const bool featureGesendet = warteAuf (3000, [&] { return server.p2.load() >= 1; });
        bool minorBeobachtet = false;
        {
            std::lock_guard<std::mutex> l (server.textMutex);
            minorBeobachtet = server.letzterTelemetryWire.size() > 7
                && server.letzterTelemetryWire[7] == kFeatureBatchSchemaMinor;
        }
        pruefe (featureAngenommen && featureGesendet && minorBeobachtet,
                "band_stereo_sender_emittiert_erhoehten_schema_minor");

        // P0-Rundlauf.
        control.sendeP0 ("{\"type\":\"heartbeat\",\"sequence\":1}");
        pruefe (warteAuf (3000, [&] { return server.p0.load() >= 1; }),
                "P0 kommt beim Server als P0-Familie an");
        pruefe (warteAuf (3000, [&] { return control.snapshot().empfangen >= 1; }),
                "und die Antwort erreicht den Client");

        control.sendeP1 ("sonde-1", "{\"type\":\"state_report\"}");
        pruefe (warteAuf (3000, [&] { return server.p1.load() >= 1; }),
                "P1 kommt als P1-Familie an");
        {
            std::lock_guard<std::mutex> l (server.textMutex);
            pruefe (! server.p0Minors.empty() && ! server.p1Minors.empty()
                        && std::all_of (server.p0Minors.begin(), server.p0Minors.end(),
                                       [] (auto m) { return m == nakama::ipc::kJsonSchemaMinor; })
                        && std::all_of (server.p1Minors.begin(), server.p1Minors.end(),
                                       [] (auto m) { return m == nakama::ipc::kJsonSchemaMinor; }),
                    // Gegen die KONSTANTE, nicht gegen eine 1: genau dieses
                    // Literal wurde beim Fassungsschritt der SONDE-013 still
                    // falsch, waehrend der Test weiter "aktiver JSON-Minor"
                    // hiess (04.09.2026).
                    "ControlClient sendet P0 und P1 mit aktivem JSON-Minor");
        }

        std::uint8_t p2Frame[128];
        std::memset (p2Frame, 0x5A, sizeof (p2Frame));
        for (int i = 0; i < 20; ++i)
        {
            telemetrie.veroeffentlichen (p2Frame, sizeof (p2Frame));
            std::this_thread::sleep_for (std::chrono::milliseconds (10));
        }
        pruefe (warteAuf (3000, [&] { return server.p2.load() >= 5; }),
                "P2 fliesst ueber die zweite Verbindung",
                std::to_string (server.p2.load()) + " Frames");

        // Beide Hellos tragen dieselbe runtime_nonce — das ist die Kopplung.
        {
            std::lock_guard<std::mutex> l (server.textMutex);
            const std::string nonce = "\"runtime_nonce\":\"" + hex32 ('7') + "\"";
            pruefe (server.letztesControlHello.find (nonce) != std::string::npos
                        && server.letztesTelemetryHello.find (nonce) != std::string::npos,
                    "beide Bootstrap-Hellos tragen dieselbe runtime_nonce");
            const std::string alias = "\"instance_id\":\"" + wireInstanceId + "\"";
            pruefe (server.letztesControlHello.find (alias) != std::string::npos
                        && server.letztesTelemetryHello.find (alias) != std::string::npos
                        && adresse.instanceId == legacyInstanceId,
                    "instance_address_alias_is_on_actual_wire");
            pruefe (server.letztesControlHello.find ("\"protocol\":3") != std::string::npos,
                    "das Bootstrap-Hello ist v3 und nicht v2");
        }

        telemetrie.stop();
        control.stop();
        pruefe (control.snapshot().status == ControlClient::Status::getrennt
                    && telemetrie.snapshot().status == TelemetryClient::Status::getrennt,
                "stop() trennt beide Verbindungen und kehrt zurueck");
        server.stoppen();
    }

    abschnitt ("G1a · JSON-Minor wird bis zum C++-Empfaenger getragen");
    {
        TestServer server (testPipeName ("jsonminor"));
        server.controlAntwortMinor.store (nakama::ipc::kJsonSchemaMinor);
        server.starten();
        std::atomic<int> empfangenerMinor { -1 };
        ControlClient control (
            [&] {
                ControlHello h;
                h.adresse = testAdresse (hex32 ('6'));
                h.pluginKind = "main";
                return h;
            },
            server.pipeName(), {}, {}, {},
            [&] (const std::string&, std::uint8_t minor) {
                empfangenerMinor.store (minor);
            });
        control.start();
        const bool verbunden = warteAuf (4000, [&] {
            return control.snapshot().status == ControlClient::Status::verbunden;
        });
        control.sendeP0 ("{\"type\":\"heartbeat\",\"sequence\":1}");
        pruefe (verbunden && warteAuf (3000, [&] {
                    return empfangenerMinor.load() == nakama::ipc::kJsonSchemaMinor;
                }),
                "ControlClient reicht den empfangenen aktiven Minor an den Vertragsleser weiter");

        // Einen ueber dem aktiven: was es NICHT gibt, wird abgewiesen. Auch
        // hier stand eine 2 als Literal und meinte "unbekannt" - seit
        // SONDE-013 ist 2 die aktive Fassung.
        server.controlAntwortMinor.store (
            static_cast<std::uint8_t> (nakama::ipc::kJsonSchemaMinor + 1));
        control.sendeP0 ("{\"type\":\"heartbeat\",\"sequence\":2}");
        pruefe (warteAuf (3000, [&] {
                    return control.snapshot().envelopeAbweisungen >= 1;
                }),
                "ein hoeherer unbekannter JSON-Minor schliesst wie ein Envelopeverstoss");
        control.stop();
        server.stoppen();
    }

    abschnitt ("G1b · Produktstatus sendet Heartbeat und State-Report");
    {
        TestServer server (testPipeName ("produktstatus"));
        server.starten();
        std::atomic<std::uint64_t> revision { 7 };
        ControlClient control (
            [&] {
                ControlHello h;
                h.adresse = testAdresse (hex32 ('4'));
                h.pluginKind = "main";
                return h;
            },
            server.pipeName(), {},
            [&] {
                ControlStatus s;
                s.dspSchemaVersion = 1;
                s.stateRevision = revision.load();
                s.stateHash = std::string (64, revision.load() == 7 ? 'a' : 'b');
                s.recordStateValid = true;
                s.recording = false;
                s.framesDropped = 3;
                s.parseErrors = 4;
                s.queueOverflows = 5;
                return s;
            });
        control.start();
        const bool ersteBeide = warteAuf (4000, [&] {
            std::lock_guard<std::mutex> l (server.textMutex);
            return ! server.p0Texte.empty() && ! server.p1Texte.empty();
        });
        bool heartbeatVoll = false, reportVoll = false;
        int reportZahl = 0;
        {
            std::lock_guard<std::mutex> l (server.textMutex);
            reportZahl = static_cast<int> (server.p1Texte.size());
            for (const auto& t : server.p0Texte)
                heartbeatVoll = heartbeatVoll
                    || (t.find ("\"type\":\"heartbeat\"") != std::string::npos
                        && t.find ("\"state_revision\":7") != std::string::npos
                        && t.find ("\"host_context_presence\":\"supported\"") != std::string::npos
                        && t.find ("\"remote_control\":\"unsupported\"") != std::string::npos
                        && t.find ("\"frames_dropped\":3") != std::string::npos
                        && t.find ("\"parse_errors\":4") != std::string::npos
                        && t.find ("\"queue_overflows\":5") != std::string::npos);
            for (const auto& t : server.p1Texte)
                reportVoll = reportVoll
                    || (t.find ("\"type\":\"state_report\"") != std::string::npos
                        && t.find ("\"dsp_schema_version\":1") != std::string::npos
                        && t.find ("\"state_hash\":\"" + std::string (64, 'a') + "\"")
                               != std::string::npos
                        && t.find ("\"record_state\":{\"valid\":true,\"recording\":false}")
                               != std::string::npos);
        }
        pruefe (ersteBeide && heartbeatVoll && reportVoll,
                "produkt_client_sendet_vollen_heartbeat_und_state_report");

        const bool heartbeatWiederholt = warteAuf (2500, [&] {
            return server.p0.load() >= 2;
        });
        int vorAenderung = 0;
        {
            std::lock_guard<std::mutex> l (server.textMutex);
            vorAenderung = static_cast<int> (server.p1Texte.size());
        }
        pruefe (heartbeatWiederholt && vorAenderung == reportZahl,
                "heartbeat_laeuft_1hz_state_report_bleibt_koalesziert");

        revision.store (8);
        const bool geaendert = warteAuf (2500, [&] {
            std::lock_guard<std::mutex> l (server.textMutex);
            return static_cast<int> (server.p1Texte.size()) > vorAenderung;
        });
        pruefe (geaendert,
                "geaenderter_produktzustand_sendet_neuen_state_report");
        control.stop();
        server.stoppen();
    }

    abschnitt ("G2 · ungekoppelter Telemetry-Connect wird geschlossen");
    {
        TestServer server (testPipeName ("ungekoppelt"));
        server.starten();
        TelemetryClient telemetrie ([&] {
            TelemetryHello t;
            t.adresse = testAdresse (hex32 ('8'));
            t.linkId = hex32 ('f');       // nie ausgegeben
            t.challenge = hex32 ('e');
            return t;
        }, server.pipeName());
        telemetrie.start();
        pruefe (warteAuf (4000, [&] { return server.abgewiesen.load() >= 1; }),
                "der Server weist die ungekoppelte Telemetrieverbindung ab");
        pruefe (warteAuf (2000, [&] {
                    return telemetrie.snapshot().status != TelemetryClient::Status::verbunden;
                }),
                "der Client gilt nicht als verbunden");
        telemetrie.stop();
        server.stoppen();
    }

    abschnitt ("G3 · Reconnect nach Serverneustart");
    {
        const auto name = testPipeName ("reconnect");
        auto server = std::make_unique<TestServer> (name);
        server->starten();

        const auto adresse = testAdresse (hex32 ('9'));
        ControlClient control ([&] { ControlHello h; h.adresse = adresse; return h; }, name);
        control.start();
        pruefe (warteAuf (4000, [&] {
                    return control.snapshot().status == ControlClient::Status::verbunden;
                }),
                "erste Verbindung steht");
        const int versucheVorher = control.snapshot().verbindungsVersuche;

        server->stoppen();
        server.reset();
        pruefe (warteAuf (4000, [&] {
                    return control.snapshot().status != ControlClient::Status::verbunden;
                }),
                "nach dem Serverende faellt der Client auf getrennt");

        server = std::make_unique<TestServer> (name);
        server->starten();
        pruefe (warteAuf (15000, [&] {
                    return control.snapshot().status == ControlClient::Status::verbunden;
                }),
                "der Client verbindet von selbst wieder (Backoff 500..8000 ms)",
                control.snapshot().letzterFehler);
        pruefe (control.snapshot().verbindungsVersuche > versucheVorher,
                "und zaehlt die Versuche ehrlich mit",
                std::to_string (control.snapshot().verbindungsVersuche));

        // Ereignisse, die waehrend der Trennung entstanden, gehen erneut raus.
        control.stop();
        server->stoppen();
    }

    abschnitt ("G4 · ein kaputter Envelope vom Server schliesst die Verbindung");
    {
        TestServer server (testPipeName ("kaputt"));
        server.sendeKaputtenFrame.store (true);
        server.starten();
        ControlClient control ([&] {
            ControlHello h;
            h.adresse = testAdresse (hex32 ('3'));
            return h;
        }, server.pipeName());
        control.start();
        pruefe (warteAuf (5000, [&] { return control.snapshot().envelopeAbweisungen >= 1; }),
                "der Client weist den Frame ab und verbindet nicht",
                control.snapshot().letzterFehler);
        pruefe (control.snapshot().status != ControlClient::Status::verbunden,
                "Status bleibt ungleich 'verbunden'");
        control.stop();
        server.stoppen();
    }

    abschnitt ("G5 · P0-Ueberlauf schliesst die Verbindung, statt zu verwerfen");
    {
        // Ohne Verbindung ist ein Ueberlauf nur eine Absage an den Aufrufer —
        // es gibt nichts zu schliessen.
        TestServer leer (testPipeName ("p0zaehlung"));
        ControlClient ungestartet ([&] {
            ControlHello h;
            h.adresse = testAdresse (hex32 ('4'));
            return h;
        }, leer.pipeName());
        bool alleDurch = true;
        for (int i = 0; i < 64; ++i)
            alleDurch = alleDurch && ungestartet.sendeP0 ("{\"type\":\"heartbeat\",\"sequence\":0}");
        pruefe (alleDurch, "64 P0-Nachrichten passen");
        pruefe (! ungestartet.sendeP0 ("{\"type\":\"heartbeat\",\"sequence\":0}"),
                "die 65. meldet Ueberlauf an den Aufrufer");
        pruefe (ungestartet.snapshot().p0Ueberlaeufe >= 1, "der Ueberlauf ist gezaehlt");

        // Und jetzt derselbe Ueberlauf WAEHREND einer stehenden Verbindung.
        // Der Server macht den Bootstrap und liest danach nicht mehr; ein
        // 200-KiB-Befehl fuellt den 64-KiB-Pipepuffer und blockiert den
        // Sendeweg sicher — erst dann laeuft die Queue wirklich voll.
        TestServer server (testPipeName ("p0ueberlauf"));
        server.nichtLesen.store (true);
        server.starten();
        ControlClient control ([&] {
            ControlHello h;
            h.adresse = testAdresse (hex32 ('5'));
            return h;
        }, server.pipeName());
        control.start();
        pruefe (warteAuf (4000, [&] {
                    return control.snapshot().status == ControlClient::Status::verbunden;
                }),
                "Verbindung steht (der Server liest nur nicht mehr)",
                control.snapshot().letzterFehler);

        std::string gross = "{\"type\":\"heartbeat\",\"sequence\":0,\"fuell\":\"";
        gross.append (200 * 1024, 'x');
        gross += "\"}";
        pruefe (control.sendeP0 (gross), "ein 200-KiB-Befehl wird eingereiht");
        std::this_thread::sleep_for (std::chrono::milliseconds (300));

        bool ueberlaufKam = false;
        for (int i = 0; i < 200 && ! ueberlaufKam; ++i)
            ueberlaufKam = ! control.sendeP0 ("{\"type\":\"heartbeat\",\"sequence\":0}");
        pruefe (ueberlaufKam, "die Queue laeuft ueber, weil der Peer nicht liest");
        pruefe (warteAuf (5000, [&] {
                    return control.snapshot().letzterFehler.find ("P0-Ueberlauf")
                           != std::string::npos;
                }),
                "die Verbindung wird deswegen geschlossen, nicht stillschweigend gekuerzt",
                control.snapshot().letzterFehler);
        control.stop();
        server.stoppen();
    }

    abschnitt ("G6 · ein P1-Ereignis ueberlebt einen gescheiterten Write");
    {
        // Der Aufbau erzwingt genau den Fall aus T2-Befund 4: der Sender hat
        // den Eintrag ENTNOMMEN, und dann bricht der Write. Dafuer liest der
        // Server nach dem Bootstrap nicht mehr (der 64-KiB-Pipepuffer laeuft
        // voll, der Schreibvorgang blockiert) und wird dann beendet — der
        // blockierte Write faellt sofort.
        const auto name = testPipeName ("p1verlust");
        auto server1 = std::make_unique<TestServer> (name);
        server1->nichtLesen.store (true);
        server1->starten();

        const auto adresse = testAdresse (hex32 ('6'));
        ControlClient control ([&] { ControlHello h; h.adresse = adresse; return h; }, name);
        control.start();
        pruefe (warteAuf (5000, [&] {
                    return control.snapshot().status == ControlClient::Status::verbunden;
                }),
                "Verbindung steht (der Server liest nur nicht mehr)",
                control.snapshot().letzterFehler);

        auto ereignis = [] (int i) {
            return "{\"type\":\"probe_event\",\"id\":" + std::to_string (i)
                 + ",\"fuell\":\"" + std::string (800, 'x') + "\"}";
        };
        for (int i = 0; i < 200; ++i)
            control.sendeP1 ("", ereignis (i));   // leerer Schluessel = Ereignis

        std::this_thread::sleep_for (std::chrono::milliseconds (400));
        server1->stoppen();
        server1.reset();
        pruefe (warteAuf (8000, [&] {
                    return control.snapshot().status != ControlClient::Status::verbunden;
                }),
                "der Write bricht, sobald der Server weg ist");

        // Ohne Server kann der Client nicht weitersenden — der Stand ist
        // damit stabil und `p1Gesendet` nennt genau den Eintrag, der unterwegs
        // war.
        const auto unterwegs = static_cast<int> (control.snapshot().p1Gesendet);
        pruefe (unterwegs > 0 && unterwegs < 128,
                "einige Ereignisse gingen raus, eines blieb im Sender haengen",
                "p1Gesendet = " + std::to_string (unterwegs));

        auto server2 = std::make_unique<TestServer> (name);
        server2->starten();
        const std::string gesucht = "\"id\":" + std::to_string (unterwegs) + ",";
        const bool kam = warteAuf (20000, [&] {
            std::lock_guard<std::mutex> l (server2->textMutex);
            for (const auto& t : server2->p1Texte)
                if (t.find (gesucht) != std::string::npos)
                    return true;
            return false;
        });
        std::size_t angekommen = 0;
        {
            std::lock_guard<std::mutex> l (server2->textMutex);
            angekommen = server2->p1Texte.size();
        }
        pruefe (kam,
                "das Ereignis, dessen Write scheiterte, kommt nach dem Reconnect an",
                "gesucht " + gesucht + " · " + std::to_string (angekommen)
                    + " P1 nach dem Reconnect empfangen");

        control.stop();
        server2->stoppen();
    }

    abschnitt ("G7 · der TelemetryClient prueft sein welcome vollstaendig");
    {
        auto lauf = [&] (const char* fall, bool alsP2, bool fremdeLink,
                         bool ohneProtokoll, const char* was) {
            TestServer server (testPipeName (fall));
            server.welcomeAlsP2.store (alsP2);
            server.welcomeFremdeLinkId.store (fremdeLink);
            server.welcomeOhneProtokoll.store (ohneProtokoll);
            server.starten();
            TelemetryClient telemetrie ([&] {
                TelemetryHello t;
                t.adresse = testAdresse (hex32 ('7'));
                t.linkId = hex32 ('a');       // dieselben Werte wie im Server
                t.challenge = hex32 ('b');
                return t;
            }, server.pipeName());
            telemetrie.start();
            const bool nieVerbunden = ! warteAuf (2500, [&] {
                return telemetrie.snapshot().status == TelemetryClient::Status::verbunden;
            });
            const auto fehlerText = telemetrie.snapshot().letzterFehler;
            telemetrie.stop();
            server.stoppen();
            pruefe (nieVerbunden, was, fehlerText);
        };

        lauf ("welcome-p2", true, false, false,
              "ein welcome in der P2-Familie gilt NICHT als Kopplung");
        lauf ("welcome-fremd", false, true, false,
              "ein welcome mit fremder link_id gilt nicht als Kopplung");
        lauf ("welcome-ohne-protokoll", false, false, true,
              "ein welcome ohne `protocol` gilt nicht als Kopplung");

        // Gegenprobe: dasselbe Geruest mit korrektem welcome koppelt sehr wohl.
        TestServer gut (testPipeName ("welcome-gut"));
        gut.starten();
        TelemetryClient telemetrie ([&] {
            TelemetryHello t;
            t.adresse = testAdresse (hex32 ('7'));
            t.linkId = hex32 ('a');
            t.challenge = hex32 ('b');
            return t;
        }, gut.pipeName());
        telemetrie.start();
        pruefe (warteAuf (4000, [&] {
                    return telemetrie.snapshot().status == TelemetryClient::Status::verbunden;
                }),
                "Gegenprobe: mit vollstaendigem welcome koppelt derselbe Client",
                telemetrie.snapshot().letzterFehler);
        telemetrie.stop();
        gut.stoppen();
    }

    abschnitt ("G8 · nicht endliche Audiofelder werden VOR der Wandlung verriegelt");
    {
        pruefe (audioGueltig (48000.0, 512, 2), "48 kHz / 512 / 2 halten den Vertrag");
        pruefe (! audioGueltig (std::numeric_limits<double>::quiet_NaN(), 512, 2)
                    && ! audioGueltig (std::numeric_limits<double>::infinity(), 512, 2)
                    && ! audioGueltig (-std::numeric_limits<double>::infinity(), 512, 2)
                    && ! audioGueltig (1e300, 512, 2)
                    && ! audioGueltig (48000.0, 0, 2)
                    && ! audioGueltig (48000.0, 512, 65),
                "NaN, ±Inf, 1e300 und Grenzverstoesse fallen");

        TestServer server (testPipeName ("nanrate"));
        server.starten();
        std::atomic<int> welche { 0 };
        const double raten[4] = { std::numeric_limits<double>::quiet_NaN(),
                                  std::numeric_limits<double>::infinity(),
                                  -std::numeric_limits<double>::infinity(),
                                  1e300 };
        ControlClient control ([&] {
            ControlHello h;
            h.adresse = testAdresse (hex32 ('2'));
            h.samplerate = raten[welche.load() & 3];
            return h;
        }, server.pipeName());
        control.start();
        const bool nieVerbunden = ! warteAuf (2500, [&] {
            welche.fetch_add (1);
            return control.snapshot().status == ControlClient::Status::verbunden;
        });
        pruefe (nieVerbunden, "mit NaN/±Inf/1e300 verbindet der Client gar nicht erst",
                control.snapshot().letzterFehler);
        pruefe (control.snapshot().letzterFehler.find ("Audiolage") != std::string::npos,
                "und nennt den Grund ehrlich beim Namen",
                control.snapshot().letzterFehler);
        {
            std::lock_guard<std::mutex> l (server.textMutex);
            pruefe (server.letztesControlHello.empty(),
                    "kein Hello mit einer Nicht-Zahl hat den Server je erreicht");
        }
        control.stop();
        server.stoppen();
    }

    abschnitt ("G9 · ein P0-Befehl ueberlebt einen gescheiterten Write bei voller Queue");
    {
        // T2-Befund 1 vom 2026-08-29: der Sender hat den Befehl ENTNOMMEN, der
        // Write blockiert, und waehrenddessen laufen neue Befehle ein. Ohne
        // die Reservierung war der entnommene danach nicht mehr unterzubringen
        // — und weg, ohne dass es der oeffentliche Zaehler meldete.
        const auto name = testPipeName ("p0verlust");
        auto server1 = std::make_unique<TestServer> (name);
        server1->nichtLesen.store (true);
        server1->starten();

        const auto adresse = testAdresse (hex32 ('b'));
        ControlClient control ([&] { ControlHello h; h.adresse = adresse; return h; }, name);
        control.start();
        pruefe (warteAuf (5000, [&] {
                    return control.snapshot().status == ControlClient::Status::verbunden;
                }),
                "Verbindung steht (der Server liest nur nicht mehr)",
                control.snapshot().letzterFehler);

        // Der erste Befehl fuellt den 64-KiB-Pipepuffer: der Write blockiert
        // sicher, statt zufaellig.
        std::string gross = "{\"type\":\"apply\",\"id\":0,\"fuell\":\"";
        gross.append (200 * 1024, 'x');
        gross += "\"}";
        pruefe (control.sendeP0 (gross), "der erste, grosse Befehl wird eingereiht");
        std::this_thread::sleep_for (std::chrono::milliseconds (300));

        // Jetzt die Queue randvoll fahren. Der abgewiesene Aufruf bricht den
        // blockierten Write ab — genau der Fall aus dem Befund.
        std::vector<int> angenommen { 0 };
        int abgelehnt = 0;
        for (int i = 1; i <= 200; ++i)
        {
            if (control.sendeP0 ("{\"type\":\"apply\",\"id\":" + std::to_string (i) + "}"))
                angenommen.push_back (i);
            else
            {
                ++abgelehnt;
                break;
            }
        }
        pruefe (abgelehnt == 1 && angenommen.size() == 64,
                "genau 64 passen — der unterwegs befindliche belegt seinen Platz weiter",
                std::to_string (angenommen.size()) + " angenommen");
        pruefe (control.snapshot().p0Ueberlaeufe >= 1,
                "und der Ueberlauf ist oeffentlich gezaehlt",
                std::to_string (control.snapshot().p0Ueberlaeufe));
        pruefe (warteAuf (5000, [&] {
                    return control.snapshot().letzterFehler.find ("P0-Ueberlauf")
                           != std::string::npos;
                }),
                "die Verbindung wird deswegen geschlossen",
                control.snapshot().letzterFehler);

        server1->stoppen();
        server1.reset();

        // Ein lesender Server: jetzt muss JEDER angenommene Befehl ankommen.
        auto server2 = std::make_unique<TestServer> (name);
        server2->starten();
        auto fehltNoch = [&] () -> int {
            std::lock_guard<std::mutex> l (server2->textMutex);
            for (int id : angenommen)
            {
                const std::string gesucht = "\"id\":" + std::to_string (id)
                                          + (id == 0 ? "," : "}");
                bool gefunden = false;
                for (const auto& t : server2->p0Texte)
                    if (t.find (gesucht) != std::string::npos)
                    {
                        gefunden = true;
                        break;
                    }
                if (! gefunden)
                    return id;
            }
            return -1;
        };
        const bool alleDa = warteAuf (30000, [&] { return fehltNoch() < 0; });
        pruefe (alleDa,
                "nach dem Reconnect kommt JEDER angenommene Befehl an — auch der, "
                "dessen Write scheiterte",
                alleDa ? std::string() : "erster fehlender: id " + std::to_string (fehltNoch()));

        control.stop();
        server2->stoppen();
    }

    abschnitt ("G10 · beide Clients pruefen das welcome gegen den VOLLSTAENDIGEN Vertrag");
    {
        // T2-Befund 3 vom 2026-08-29: `"broker_version":null` kam als
        // nichtleerer Text `null` durch, und ein Zusatzfeld fiel gar nicht auf,
        // obwohl `eq-ipc-v3.schema.json` `additionalProperties:false` sagt.
        auto telemetrieProbe = [&] (const char* fall, bool typfalsch, bool zusatz,
                                    bool langeVersion, const char* was) {
            TestServer server (testPipeName (fall));
            server.welcomeTypfalsch.store (typfalsch);
            server.welcomeZusatzfeld.store (zusatz);
            server.welcomeLangeVersion.store (langeVersion);
            server.starten();
            TelemetryClient telemetrie ([&] {
                TelemetryHello t;
                t.adresse = testAdresse (hex32 ('7'));
                t.linkId = hex32 ('a');
                t.challenge = hex32 ('b');
                return t;
            }, server.pipeName());
            telemetrie.start();
            const bool nieVerbunden = ! warteAuf (2500, [&] {
                return telemetrie.snapshot().status == TelemetryClient::Status::verbunden;
            });
            const auto text = telemetrie.snapshot().letzterFehler;
            telemetrie.stop();
            server.stoppen();
            pruefe (nieVerbunden, was, text);
        };

        telemetrieProbe ("wc-typ", true, false, false,
                         "Telemetry: `broker_version` als `null` ist kein gueltiges welcome");
        telemetrieProbe ("wc-extra", false, true, false,
                         "Telemetry: ein Zusatzfeld verletzt additionalProperties:false");
        telemetrieProbe ("wc-lang", false, false, true,
                         "Telemetry: `broker_version` ueber 64 Zeichen faellt an der Laenge");

        auto controlProbe = [&] (const char* fall, bool typfalsch, bool zusatz,
                                 bool langeVersion, const char* was) {
            TestServer server (testPipeName (fall));
            server.welcomeTypfalsch.store (typfalsch);
            server.welcomeZusatzfeld.store (zusatz);
            server.welcomeLangeVersion.store (langeVersion);
            server.starten();
            ControlClient control ([&] {
                ControlHello h;
                h.adresse = testAdresse (hex32 ('c'));
                return h;
            }, server.pipeName());
            control.start();
            const bool nieVerbunden = ! warteAuf (2500, [&] {
                return control.snapshot().status == ControlClient::Status::verbunden;
            });
            const auto text = control.snapshot().letzterFehler;
            control.stop();
            server.stoppen();
            pruefe (nieVerbunden, was, text);
        };

        controlProbe ("wcc-typ", true, false, false,
                      "Control: dieselbe Strenge — `null` statt String verbindet nicht");
        controlProbe ("wcc-extra", false, true, false,
                      "Control: ein Zusatzfeld verbindet nicht");
        controlProbe ("wcc-lang", false, false, true,
                      "Control: eine zu lange `broker_version` verbindet nicht");
    }

    abschnitt ("G11 · ein P2-Frame auf der Control-Verbindung wird abgewiesen");
    {
        // T2-Befund 4 vom 2026-08-29: der Pfad zaehlte ihn als empfangen und
        // reichte die BINAERPAYLOAD an `beiAntwort` weiter, das JSON erwartet.
        TestServer server (testPipeName ("p2aufcontrol"));
        server.sendeP2AufControl.store (true);
        server.starten();
        std::atomic<int> rueckrufe { 0 };
        ControlClient control ([&] {
            ControlHello h;
            h.adresse = testAdresse (hex32 ('d'));
            return h;
        }, server.pipeName(), [&] (const std::string&) { rueckrufe.fetch_add (1); });
        control.start();
        pruefe (warteAuf (5000, [&] {
                    return control.snapshot().familieAbweisungen >= 1;
                }),
                "der Client weist den P2-Frame ab und schliesst die Verbindung",
                control.snapshot().letzterFehler);
        pruefe (rueckrufe.load() == 0,
                "und er hat die Binaerpayload NIE an den Aufrufer weitergereicht",
                std::to_string (rueckrufe.load()) + " Rueckrufe");
        control.stop();
        server.stoppen();
    }

    abschnitt ("G12 · die Nachrichtenratengrenze gilt auch im Client");
    {
        // T2-Befund 5 vom 2026-08-29: die C++-`Ratengrenze` existierte, wurde
        // aber nur im Test benutzt. §33.1 verlangt sie auf JEDER Parserseite.
        TestServer server (testPipeName ("rateflut"));
        server.frameFlut.store (8000);   // doppelt so viele wie erlaubt
        server.starten();
        std::atomic<int> rueckrufe { 0 };
        ControlClient control ([&] {
            ControlHello h;
            h.adresse = testAdresse (hex32 ('e'));
            return h;
        }, server.pipeName(), [&] (const std::string&) { rueckrufe.fetch_add (1); });
        control.start();
        pruefe (warteAuf (10000, [&] { return control.snapshot().rateAbweisungen >= 1; }),
                "ein Peer, der schneller pipelined als die Rate erlaubt, wird getrennt",
                control.snapshot().letzterFehler);
        pruefe (rueckrufe.load() <= static_cast<int> (kRateProSekunde),
                "und hoechstens die erlaubte Zahl Frames hat den Aufrufer erreicht",
                std::to_string (rueckrufe.load()) + " von 8000");
        control.stop();
        server.stoppen();
    }

    abschnitt ("G13 · die Telemetrie merkt im Leerlauf, dass ihre Kopplung fort ist");
    {
        // T2-Befund 2 vom 2026-08-29: bei leerer P2-Schleuse las der
        // Leerlaufzweig weder von der Pipe noch verglich er die aktuellen
        // Kopplungswerte. Der Client blieb unbegrenzt als `verbunden` sichtbar.
        TestServer server (testPipeName ("kopplungneu"));
        server.starten();

        const auto adresse = testAdresse (hex32 ('f'));
        ControlClient control ([&] { ControlHello h; h.adresse = adresse; return h; },
                               server.pipeName());
        control.start();
        pruefe (warteAuf (5000, [&] {
                    return control.snapshot().status == ControlClient::Status::verbunden;
                }),
                "Control steht", control.snapshot().letzterFehler);

        TelemetryClient telemetrie ([&] {
            TelemetryHello t;
            t.adresse = adresse;
            std::string l, c;
            control.kopplung (l, c);
            t.linkId = l;
            t.challenge = c;
            return t;
        }, server.pipeName());
        telemetrie.start();
        pruefe (warteAuf (5000, [&] {
                    return telemetrie.snapshot().status == TelemetryClient::Status::verbunden;
                }),
                "Telemetry koppelt", telemetrie.snapshot().letzterFehler);

        // ── a) Der Broker schliesst die Telemetriepipe. KEIN P2-Verkehr. ──
        const int versucheVorher = telemetrie.snapshot().verbindungsVersuche;
        server.telemetrieSchliessen.store (true);
        pruefe (warteAuf (5000, [&] {
                    return telemetrie.snapshot().verbindungsVersuche > versucheVorher;
                }),
                "ohne eine einzige Veroeffentlichung bemerkt der Client den Pipe-Abschluss",
                telemetrie.snapshot().letzterFehler);
        pruefe (warteAuf (8000, [&] {
                    return telemetrie.snapshot().status == TelemetryClient::Status::verbunden;
                }),
                "und koppelt binnen Frist wieder",
                telemetrie.snapshot().letzterFehler);

        // ── b) Die Control-Verbindung bekommt NEUE Kopplungswerte. ────────
        server.neueKopplung (hex32 ('4'), hex32 ('5'));
        control.reconnect();
        pruefe (warteAuf (8000, [&] {
                    std::string l, c;
                    return control.kopplung (l, c) && l == hex32 ('4');
                }),
                "Control koppelt neu und traegt eine frische link_id",
                control.snapshot().letzterFehler);
        pruefe (warteAuf (8000, [&] {
                    return telemetrie.snapshot().kopplungswechsel >= 1;
                }),
                "die Telemetrie bemerkt die neuen Kopplungswerte im Leerlauf",
                telemetrie.snapshot().letzterFehler);
        pruefe (warteAuf (10000, [&] {
                    return telemetrie.snapshot().status == TelemetryClient::Status::verbunden;
                }),
                "und koppelt sich mit ihnen neu — ohne dass je ein P2-Frame floss",
                telemetrie.snapshot().letzterFehler);
        pruefe (telemetrie.snapshot().gesendet == 0,
                "Gegenprobe: es wurde in diesem Abschnitt wirklich nichts veroeffentlicht",
                std::to_string (telemetrie.snapshot().gesendet) + " gesendet");

        telemetrie.stop();
        control.stop();
        server.stoppen();
    }

    abschnitt ("G14 · was der TelemetryClient auf SEINER Verbindung annimmt");
    {
        // Die Gegenstuecke zu G11 und G12 auf der zweiten Verbindung. Ohne sie
        // waeren die Familien- und Ratensperre dort behauptet, nicht belegt.
        auto koppeln = [] (TestServer& server) {
            return [&server] {
                TelemetryHello t;
                t.adresse = testAdresse (hex32 ('7'));
                t.linkId = hex32 ('a');
                t.challenge = hex32 ('b');
                return t;
            };
        };

        {
            TestServer server (testPipeName ("p0auftele"));
            server.sendeP0AufTelemetrie.store (true);
            server.starten();
            TelemetryClient telemetrie (koppeln (server), server.pipeName());
            telemetrie.start();
            pruefe (warteAuf (5000, [&] {
                        return telemetrie.snapshot().familieAbweisungen >= 1;
                    }),
                    "ein P0-Frame auf der Telemetrieverbindung schliesst sie",
                    telemetrie.snapshot().letzterFehler);
            telemetrie.stop();
            server.stoppen();
        }

        {
            TestServer server (testPipeName ("p2flut"));
            server.frameFlutTelemetrieP2.store (8000);
            server.starten();
            TelemetryClient telemetrie (koppeln (server), server.pipeName());
            telemetrie.start();
            pruefe (warteAuf (10000, [&] {
                        return telemetrie.snapshot().rateAbweisungen >= 1;
                    }),
                    "auch hier gilt die Nachrichtenratengrenze",
                    telemetrie.snapshot().letzterFehler);
            const auto s = telemetrie.snapshot();
            pruefe (s.empfangen > 0 && s.empfangen <= kRateProSekunde,
                    "die vertragsgemaessen P2-Frames sind gezaehlt, nicht still verworfen",
                    std::to_string (s.empfangen) + " von 8000 gezaehlt");
            telemetrie.stop();
            server.stoppen();
        }
    }

    abschnitt ("G15 · zu grosse Nachrichten werden an der TUER abgewiesen");
    {
        // Kehrseite der Reservierung (T2-Befund 1): eine Nachricht ueber der
        // Paketgrenze bliebe sonst fuer immer vorn in der Queue und liesse
        // jede neue Verbindung an derselben Stelle scheitern.
        TestServer leer (testPipeName ("zugross"));
        ControlClient control ([&] {
            ControlHello h;
            h.adresse = testAdresse (hex32 ('1'));
            return h;
        }, leer.pipeName());

        const std::string riesig (kMaxPayloadBytes + 1, 'x');
        pruefe (! control.sendeP0 (riesig) && control.snapshot().zuGross == 1,
                "ein P0 ueber der Paketgrenze wird gar nicht erst eingereiht",
                std::to_string (control.snapshot().zuGross));
        pruefe (control.sendeP1 ("", riesig) == P1Ergebnis::zuGross
                    && control.snapshot().zuGross == 2,
                "und ein P1 ebenso — der Aufrufer erfaehrt es sofort");

        // Gegenprobe: genau auf der Grenze geht es durch. Sonst spraeche der
        // Riegel ueber eine falsche Zahl.
        const std::string genauNoch (kMaxPayloadBytes, 'x');
        pruefe (control.sendeP0 (genauNoch) && control.snapshot().zuGross == 2,
                "genau auf der Grenze wird eingereiht, nicht abgewiesen");
    }

    // ── H · Bootstrapgrenze und der strenge kleine JSON-Leser ─────────────
    abschnitt ("G16 · ein P0-ACK kommt an, waehrend P1 rueckstaut");
    {
        // Matrix `B-CC-06`/`B-CC-07`, Regel 4 und der Gate-Satz „ohne
        // P0-Starvation": der Peer beantwortet den Heartbeat und liest danach
        // NICHT mehr. Der P1-Weg des Clients laeuft damit voll, und der ACK
        // liegt bereits da. Die alte Fassung sprang mit `continue` an den
        // Lesepfad vorbei, solange irgendetwas zu senden war — der ACK wurde
        // nie verarbeitet, die Verbindung lief in ihre Schreibfrist und fiel.
        TestServer server (testPipeName ("p0trotzp1"));
        server.nachErsterP0AntwortNichtLesen.store (true);
        server.starten();

        std::atomic<int> acks { 0 };
        ControlClient control ([&] {
            ControlHello h;
            h.adresse = testAdresse (hex32 ('4'));
            return h;
        }, server.pipeName(),
           [&acks] (const std::string& t) {
               if (t.find ("heartbeat_ack") != std::string::npos)
                   ++acks;
           });
        control.start();
        pruefe (warteAuf (5000, [&] {
                    return control.snapshot().status == ControlClient::Status::verbunden;
                }),
                "Verbindung steht");

        // Erst der Rueckstau, dann der Heartbeat: P0 hat Vorrang und geht
        // trotzdem zuerst raus, der Peer antwortet und verstummt.
        for (int i = 0; i < 100; ++i)
            control.sendeP1 ("", std::string (60000, 'x'));
        pruefe (control.sendeP0 ("{\"type\":\"heartbeat\",\"sequence\":1}"),
                "der Heartbeat wird eingereiht");

        const bool kam = warteAuf (3000, [&] { return acks.load() >= 1; });
        pruefe (kam, "der ACK erreicht beiAntwort, obwohl P1 rueckstaut",
                std::to_string (acks.load()) + " ACKs, "
                    + std::to_string (control.snapshot().p1Gesendet) + " P1 gesendet, "
                    + control.snapshot().letzterFehler);
        control.stop();
        server.stoppen();
    }

    abschnitt ("G16b · der ACK trifft ein, WAEHREND der Write in seine Frist laeuft");
    {
        // Matrix `B-CC-07` (NAK-104). G16 oben liefert den ACK nach einem
        // ERFOLGREICHEN Write und erzwingt den Fehlerpfad damit nicht. Hier
        // liest der Peer von Anfang an NICHT: der P1-Write des Clients laeuft
        // in `kIoFristMs` (5000 ms), und der ACK kommt mitten hinein.
        //
        // Genau dort stand die Falle: das Zeitlimit des Schreibens setzte ueber
        // `ioAbbrechen()` das Abbruchflag der GANZEN Verbindung, und der
        // nachgelagerte Lesevorgang bekam sofort `LeseAusgang::fehler`. Der
        // ACK, den `B-CC-07` noch melden will, war unerreichbar — und der
        // Client verbindet danach neu und faellt in dieselbe Lage.
        TestServer server (testPipeName ("ackimwrite"));
        server.nichtLesen.store (true);
        server.ackNachNichtLesenMs.store (400);
        server.starten();

        std::atomic<int> acks { 0 };
        ControlClient control ([&] {
            ControlHello h;
            h.adresse = testAdresse (hex32 ('4'));
            return h;
        }, server.pipeName(),
           [&acks] (const std::string& t) {
               if (t.find ("heartbeat_ack") != std::string::npos)
                   ++acks;
           });
        control.start();
        pruefe (warteAuf (5000, [&] {
                    return control.snapshot().status == ControlClient::Status::verbunden;
                }),
                "Verbindung steht, und der Peer liest kein einziges Byte");

        // Rueckstau, der den Sendepuffer sicher ueberschreitet: der zweite
        // Write bleibt stehen, bis seine Frist ablaeuft.
        for (int i = 0; i < 100; ++i)
            control.sendeP1 ("", std::string (60000, 'x'));

        const bool kam = warteAuf (9000, [&] { return acks.load() >= 1; });
        pruefe (kam,
                "ein vor dem Verbindungsende empfangener ACK geht nicht verloren — "
                "auch nach einem Write-Zeitlimit",
                std::to_string (acks.load()) + " ACKs, empfangen="
                    + std::to_string (control.snapshot().empfangen) + ", "
                    + control.snapshot().letzterFehler);
        control.stop();
        server.stoppen();
    }

    abschnitt ("G17 · stop() kehrt in JEDEM Zustand zurueck");
    {
        // Matrix `B-CC-10`…`B-CC-12`, `B-TC-07`, `B-TC-09` (Regel 6).
        using Uhr = std::chrono::steady_clock;

        // (1) stop() aus `beiAntwort` heraus: kein Self-Join.
        {
            TestServer server (testPipeName ("stopinnen"));
            server.starten();
            std::atomic<bool> gestoppt { false };
            std::unique_ptr<ControlClient> control;
            control = std::make_unique<ControlClient> ([&] {
                ControlHello h;
                h.adresse = testAdresse (hex32 ('5'));
                return h;
            }, server.pipeName(),
               [&] (const std::string&) {
                   if (! gestoppt.exchange (true))
                       control->stop();     // aus dem Clientthread heraus
               });
            control->start();
            pruefe (warteAuf (5000, [&] {
                        return control->snapshot().status == ControlClient::Status::verbunden;
                    }),
                    "Verbindung steht");
            control->sendeP0 ("{\"type\":\"heartbeat\",\"sequence\":1}");
            pruefe (warteAuf (5000, [&] { return gestoppt.load(); }),
                    "stop() aus beiAntwort kehrt zurueck — kein Self-Join, kein terminate");
            control->stop();   // von aussen, holt den join nach
            pruefe (control->snapshot().status == ControlClient::Status::getrennt,
                    "und danach ist der Client getrennt");
            control.reset();
            server.stoppen();
        }

        // (2) Ein blockierender Callback haelt stop() hoechstens die Frist auf.
        {
            TestServer server (testPipeName ("stopfrist"));
            server.starten();
            auto blockiert = std::make_shared<std::atomic<bool>> (true);
            auto imCallback = std::make_shared<std::atomic<bool>> (false);
            long long dauerMs = 0;
            std::uint64_t abgeloest = 0;
            {
                ControlClient control ([&] {
                    ControlHello h;
                    h.adresse = testAdresse (hex32 ('6'));
                    return h;
                }, server.pipeName(),
                   [blockiert, imCallback] (const std::string&) {
                       imCallback->store (true);
                       const auto bis = Uhr::now() + std::chrono::seconds (20);
                       while (blockiert->load() && Uhr::now() < bis)
                           std::this_thread::sleep_for (std::chrono::milliseconds (5));
                   });
                control.start();
                pruefe (warteAuf (5000, [&] {
                            return control.snapshot().status == ControlClient::Status::verbunden;
                        }),
                        "Verbindung steht");
                control.sendeP0 ("{\"type\":\"heartbeat\",\"sequence\":1}");
                pruefe (warteAuf (5000, [&] { return imCallback->load(); }),
                        "der Callback blockiert wirklich");

                const auto t0 = Uhr::now();
                control.stop();
                dauerMs = std::chrono::duration_cast<std::chrono::milliseconds> (
                              Uhr::now() - t0).count();
                abgeloest = control.snapshot().stopFristUeberschritten;
            }
            // Der Client ist zerstoert, der abgeloeste Thread laeuft noch: er
            // haelt seine Laufzeit selbst und beruehrt den Client nicht mehr.
            blockiert->store (false);
            std::this_thread::sleep_for (std::chrono::milliseconds (300));
            pruefe (dauerMs < 3500,
                    "stop() kehrt trotz blockierendem Callback binnen Frist zurueck",
                    std::to_string (dauerMs) + " ms");
            pruefe (abgeloest >= 1,
                    "und die Fristueberschreitung ist sichtbar gezaehlt",
                    std::to_string (abgeloest));
            server.stoppen();
        }

        // (2b) Nach einem abgeloesten stop() darf ein neues start() NICHT auf
        //      denselben Thread treffen. Der Vorgaenger lebt noch, bis sein
        //      Callback zurueckkommt; ohne Laufnummer saehe er `laeuft` wieder
        //      auf true und liefe auf derselben Pipe weiter — zwei Threads auf
        //      einer Verbindung. (Selbstaudit dieser Runde, ausserhalb der
        //      Matrixzeilen, aber Folge von `B-CC-12`.)
        {
            TestServer server (testPipeName ("stopneustart"));
            server.starten();
            auto blockiert = std::make_shared<std::atomic<bool>> (true);
            auto imCallback = std::make_shared<std::atomic<bool>> (false);
            auto rufe = std::make_shared<std::atomic<int>> (0);
            ControlClient control ([&] {
                ControlHello h;
                h.adresse = testAdresse (hex32 ('a'));
                return h;
            }, server.pipeName(),
               [blockiert, imCallback, rufe] (const std::string&) {
                   rufe->fetch_add (1);
                   imCallback->store (true);
                   const auto bis = Uhr::now() + std::chrono::seconds (20);
                   while (blockiert->load() && Uhr::now() < bis)
                       std::this_thread::sleep_for (std::chrono::milliseconds (5));
               });
            control.start();
            pruefe (warteAuf (5000, [&] {
                        return control.snapshot().status == ControlClient::Status::verbunden;
                    }),
                    "Verbindung steht");
            control.sendeP0 ("{\"type\":\"heartbeat\",\"sequence\":1}");
            pruefe (warteAuf (5000, [&] { return imCallback->load(); }),
                    "der Callback blockiert wirklich");
            control.stop();          // loest ab
            const auto versucheNachStop = control.snapshot().verbindungsVersuche;

            control.start();         // neuer Lauf auf derselben Laufzeit
            const bool wiederDa = warteAuf (8000, [&] {
                return control.snapshot().verbindungsVersuche > versucheNachStop;
            });
            blockiert->store (false);
            std::this_thread::sleep_for (std::chrono::milliseconds (300));
            const auto stand = control.snapshot();
            control.stop();
            pruefe (wiederDa, "nach dem Abloesen verbindet ein neuer start() wieder",
                    std::to_string (stand.verbindungsVersuche) + " Versuche");
            pruefe (stand.verbindungsVersuche == versucheNachStop + 1
                        && stand.stopFristUeberschritten == 1,
                    "und zwar GENAU EIN neuer Lauf — der abgeloeste Vorgaenger "
                    "faehrt nicht daneben weiter",
                    std::to_string (stand.verbindungsVersuche) + " Versuche, "
                        + std::to_string (stand.stopFristUeberschritten)
                        + " Fristueberschreitungen");
            server.stoppen();
        }

        // (2c) `B-CC-12` (NAK-104): der abgeloeste Lauf beruehrt die Verbindung
        //      eines SPAETEREN Laufs nicht. (2b) gibt den alten Callback frei,
        //      sobald `verbindungsVersuche` gewachsen ist — also schon, BEVOR
        //      die neue Verbindung wirklich steht. Genau der Rueckfall danach
        //      war der Befund: der alte Lauf erreichte sein unbedingtes
        //      `verbindung.schliessen()` und trennte die frische Pipe.
        //
        //      Hier wird erst das neue `welcome` abgewartet (Status
        //      `verbunden` gibt es nur nach vollstaendig geprueftem welcome)
        //      und ERST DANN der alte Callback freigegeben.
        {
            TestServer server (testPipeName ("altlaufneu"));
            server.starten();
            auto blockiert  = std::make_shared<std::atomic<bool>> (true);
            auto imCallback = std::make_shared<std::atomic<bool>> (false);
            auto ersterRuf  = std::make_shared<std::atomic<bool>> (true);
            auto acksNeu    = std::make_shared<std::atomic<int>> (0);
            ControlClient control ([&] {
                ControlHello h;
                h.adresse = testAdresse (hex32 ('b'));
                return h;
            }, server.pipeName(),
               [blockiert, imCallback, ersterRuf, acksNeu] (const std::string&) {
                   if (! ersterRuf->exchange (false))
                   {
                       acksNeu->fetch_add (1);   // Antworten des NEUEN Laufs
                       return;
                   }
                   imCallback->store (true);
                   const auto bis = Uhr::now() + std::chrono::seconds (20);
                   while (blockiert->load() && Uhr::now() < bis)
                       std::this_thread::sleep_for (std::chrono::milliseconds (5));
               });
            control.start();
            pruefe (warteAuf (5000, [&] {
                        return control.snapshot().status == ControlClient::Status::verbunden;
                    }),
                    "Verbindung steht");
            control.sendeP0 ("{\"type\":\"heartbeat\",\"sequence\":1}");
            pruefe (warteAuf (5000, [&] { return imCallback->load(); }),
                    "der Callback blockiert wirklich");
            control.stop();                    // loest den alten Lauf ab
            control.start();                   // neuer Lauf, neue Verbindung
            const bool neuVerbunden = warteAuf (8000, [&] {
                return control.snapshot().status == ControlClient::Status::verbunden;
            });
            pruefe (neuVerbunden,
                    "der neue Lauf steht NACHGEWIESEN — welcome geprueft, nicht nur "
                    "ein Versuch gezaehlt");

            const auto vorFreigabe      = control.snapshot();
            const int  verbindungenVor  = server.verbindungen.load();
            blockiert->store (false);          // JETZT kehrt der alte Callback zurueck
            std::this_thread::sleep_for (std::chrono::milliseconds (1200));
            const auto nachFreigabe = control.snapshot();

            control.sendeP0 ("{\"type\":\"heartbeat\",\"sequence\":2}");
            const bool antwortet = warteAuf (4000, [&] { return acksNeu->load() >= 1; });
            control.stop();

            pruefe (nachFreigabe.status == ControlClient::Status::verbunden
                        && nachFreigabe.verbindungsVersuche == vorFreigabe.verbindungsVersuche
                        && server.verbindungen.load() == verbindungenVor,
                    "der zurueckkehrende alte Lauf trennt die neue Verbindung NICHT",
                    std::to_string (nachFreigabe.verbindungsVersuche) + " Versuche (vorher "
                        + std::to_string (vorFreigabe.verbindungsVersuche) + "), "
                        + std::to_string (server.verbindungen.load()) + " Serververbindungen (vorher "
                        + std::to_string (verbindungenVor) + "), Status "
                        + (nachFreigabe.status == ControlClient::Status::verbunden
                               ? "verbunden" : "NICHT verbunden"));
            pruefe (antwortet,
                    "und der neue Lauf beantwortet danach weiter P0",
                    std::to_string (acksNeu->load()) + " ACKs");
            server.stoppen();
        }

        // (2d) Dieselbe Zusage fuer den TelemetryClient (`B-TC-07`, NAK-104).
        //      Sein blockierender Callback ist der `helloProvider`; er steht
        //      VOR dem Oeffnen. Ein abgeloester Lauf, der danach weiterlaeuft,
        //      riefe `oeffnen()` — und das schliesst zuerst. Auf einer
        //      gemeinsamen Verbindung waere das die Pipe des neuen Laufs.
        {
            TestServer server (testPipeName ("telealtlauf"));
            server.starten();
            auto freigabe   = std::make_shared<std::atomic<bool>> (false);
            auto imProvider = std::make_shared<std::atomic<bool>> (false);
            auto rufNr      = std::make_shared<std::atomic<int>> (0);
            TelemetryHello vorlage;
            vorlage.adresse   = testAdresse (hex32 ('8'));
            vorlage.linkId    = server.kopplungLinkId();
            vorlage.challenge = server.kopplungChallenge();

            TelemetryClient tele ([vorlage, freigabe, imProvider, rufNr] {
                // Genau der ZWEITE Aufruf blockiert: der erste baut die
                // Verbindung auf, der dritte gehoert dem neuen Lauf.
                if (rufNr->fetch_add (1) == 1)
                {
                    imProvider->store (true);
                    const auto bis = Uhr::now() + std::chrono::seconds (20);
                    while (! freigabe->load() && Uhr::now() < bis)
                        std::this_thread::sleep_for (std::chrono::milliseconds (5));
                }
                return vorlage;
            }, server.pipeName());

            tele.start();
            pruefe (warteAuf (5000, [&] {
                        return tele.snapshot().status == TelemetryClient::Status::verbunden;
                    }),
                    "die Telemetrie steht");
            tele.reconnect();                  // zwingt den zweiten Provideraufruf
            pruefe (warteAuf (5000, [&] { return imProvider->load(); }),
                    "der helloProvider blockiert wirklich");
            tele.stop();                       // loest den alten Lauf ab
            tele.start();                      // neuer Lauf, neue Verbindung
            const bool neuVerbunden = warteAuf (8000, [&] {
                return tele.snapshot().status == TelemetryClient::Status::verbunden;
            });
            pruefe (neuVerbunden, "der neue Telemetrielauf steht nachgewiesen");

            const auto vorFreigabe     = tele.snapshot();
            const int  verbindungenVor = server.verbindungen.load();
            freigabe->store (true);            // der alte Provider kehrt zurueck
            std::this_thread::sleep_for (std::chrono::milliseconds (1200));
            const auto nachFreigabe = tele.snapshot();
            tele.stop();

            pruefe (nachFreigabe.status == TelemetryClient::Status::verbunden
                        && nachFreigabe.verbindungsVersuche == vorFreigabe.verbindungsVersuche
                        && server.verbindungen.load() == verbindungenVor,
                    "dieselbe Zusage gilt fuer den TelemetryClient",
                    std::to_string (nachFreigabe.verbindungsVersuche) + " Versuche (vorher "
                        + std::to_string (vorFreigabe.verbindungsVersuche) + "), "
                        + std::to_string (server.verbindungen.load()) + " Serververbindungen (vorher "
                        + std::to_string (verbindungenVor) + "), Status "
                        + (nachFreigabe.status == TelemetryClient::Status::verbunden
                               ? "verbunden" : "NICHT verbunden"));
            server.stoppen();
        }

        // (3) TelemetryClient: stop() vor der Kopplung und waehrend eines
        //     blockierenden P2-Writes (`B-TC-07`, `B-TC-09`).
        {
            TestServer server (testPipeName ("stoptele"));
            server.nichtLesen.store (true);
            server.starten();

            {
                TelemetryClient wartend ([] { return TelemetryHello(); },
                                         server.pipeName());
                wartend.start();
                pruefe (warteAuf (3000, [&] {
                            return wartend.snapshot().status
                                   == TelemetryClient::Status::wartetAufKopplung;
                        }),
                        "die Telemetrie wartet auf ihre Kopplung");
                const auto t0 = Uhr::now();
                wartend.stop();
                const auto ms = std::chrono::duration_cast<std::chrono::milliseconds> (
                                    Uhr::now() - t0).count();
                pruefe (ms < 2500, "stop() im Zustand wartetAufKopplung kehrt zurueck",
                        std::to_string (ms) + " ms");
            }

            {
                TelemetryHello vorlage;
                vorlage.adresse = testAdresse (hex32 ('7'));
                vorlage.linkId = server.kopplungLinkId();
                vorlage.challenge = server.kopplungChallenge();
                TelemetryClient tele ([&] { return vorlage; }, server.pipeName());
                tele.start();
                pruefe (warteAuf (5000, [&] {
                            return tele.snapshot().status
                                   == TelemetryClient::Status::verbunden;
                        }),
                        "die Telemetrie steht");
                // Der Peer liest nicht: die Writes laufen in ihre Frist.
                std::vector<std::uint8_t> gross (8000, 0x5A);
                for (int i = 0; i < 200; ++i)
                    tele.veroeffentlichen (gross.data(), gross.size());
                std::this_thread::sleep_for (std::chrono::milliseconds (200));
                const auto t0 = Uhr::now();
                tele.stop();
                const auto ms = std::chrono::duration_cast<std::chrono::milliseconds> (
                                    Uhr::now() - t0).count();
                pruefe (ms < 3500,
                        "stop() waehrend eines blockierenden P2-Writes kehrt binnen "
                        "Frist zurueck, nicht erst nach kIoFristMs",
                        std::to_string (ms) + " ms");
            }
            server.stoppen();
        }
    }

    abschnitt ("G18 · die Telemetrie verliert die Verbindung mitten im Write");
    {
        // Matrix `B-TC-10`: der Frame in Arbeit gilt als verloren (P2 ist
        // verlusttolerant, §33.1), die Schleuse behaelt ihren Inhalt, die
        // Verbindung endet ueber `ioAbbrechen` statt nach `kIoFristMs`, und
        // der Reconnect laeuft mit den AKTUELLEN Kopplungswerten.
        auto server = std::make_unique<TestServer> (testPipeName ("telewriteweg"));
        server->nichtLesen.store (true);
        server->starten();

        TelemetryHello vorlage;
        vorlage.adresse = testAdresse (hex32 ('8'));
        vorlage.linkId = server->kopplungLinkId();
        vorlage.challenge = server->kopplungChallenge();
        TelemetryClient tele ([&] { return vorlage; }, server->pipeName());
        tele.start();
        pruefe (warteAuf (5000, [&] {
                    return tele.snapshot().status == TelemetryClient::Status::verbunden;
                }),
                "die Telemetrie steht");
        const int versucheVorher = tele.snapshot().verbindungsVersuche;

        std::vector<std::uint8_t> gross (8000, 0x77);
        for (int i = 0; i < 200; ++i)
            tele.veroeffentlichen (gross.data(), gross.size());
        std::this_thread::sleep_for (std::chrono::milliseconds (200));

        const auto t0 = std::chrono::steady_clock::now();
        server->stoppen();                 // Peer weg — mitten im Write
        const bool bemerkt = warteAuf (4000, [&] {
            return tele.snapshot().status != TelemetryClient::Status::verbunden;
        });
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds> (
                            std::chrono::steady_clock::now() - t0).count();
        pruefe (bemerkt, "der Verbindungsverlust im Write wird bemerkt",
                std::to_string (ms) + " ms");

        pruefe (warteAuf (12000, [&] {
                    return tele.snapshot().verbindungsVersuche > versucheVorher;
                }),
                "und der Client zaehlt einen neuen Versuch — er koppelt von selbst neu",
                std::to_string (tele.snapshot().verbindungsVersuche) + " Versuche");
        pruefe (tele.veroeffentlichen (gross.data(), gross.size()),
                "die Schleuse nimmt weiter an — der Erzeuger merkt vom Abbruch nichts");
        tele.stop();
        server.reset();
    }

    abschnitt ("H · Bootstrapgrenze und JSON-Riegel");
    {
        std::vector<std::uint8_t> rahmen;
        pruefe (bootstrapRahmen (std::string (kMaxBootstrapBytes, 'x'), rahmen)
                    && rahmen.size() == 4 + kMaxBootstrapBytes,
                "16 KiB sind noch ein Bootstrap-Hello");
        pruefe (! bootstrapRahmen (std::string (kMaxBootstrapBytes + 1, 'x'), rahmen),
                "ein Byte darueber nicht mehr");

        std::vector<JsonFeld> felder;
        pruefe (flachesJsonObjekt ("{\"a\":\"b\",\"c\":3}", felder) && felder.size() == 2,
                "flaches Objekt wird gelesen");
        std::string w;
        pruefe (jsonLiteral (felder, "c", w) && w == "3", "Zahlen kommen als Text zurueck");

        // T2-Befund 3: der Leser HATTE den Typ und warf ihn am Rueckgabewert
        // weg. `null` und "null" waren danach dasselbe.
        pruefe (jsonText (felder, "a", w) && w == "b", "ein String kommt als String");
        pruefe (! jsonText (felder, "c", w), "eine Zahl ist KEIN String");
        pruefe (! jsonLiteral (felder, "a", w), "und ein String ist kein Literal");
        pruefe (flachesJsonObjekt ("{\"v\":null}", felder)
                    && ! jsonText (felder, "v", w)
                    && jsonLiteral (felder, "v", w) && w == "null",
                "`null` ist ein Literal und wird nie als Text `null` durchgereicht");
        pruefe (flachesJsonObjekt ("{\"v\":\"null\"}", felder)
                    && jsonText (felder, "v", w) && w == "null",
                "der STRING \"null\" dagegen ist einer — beide sind unterscheidbar");

        // additionalProperties:false braucht einen Vergleich der FELDMENGE.
        pruefe (flachesJsonObjekt ("{\"a\":1,\"b\":2}", felder)
                    && feldmengeGenau (felder, { "a", "b" }),
                "die exakte Feldmenge wird erkannt");
        pruefe (! feldmengeGenau (felder, { "a" }),
                "ein Zusatzfeld faellt auf (additionalProperties:false)");
        pruefe (! feldmengeGenau (felder, { "a", "b", "c" }),
                "und ein fehlendes Pflichtfeld ebenso");
        pruefe (! flachesJsonObjekt ("{\"a\":{\"b\":1}}", felder),
                "Verschachtelung wird ABGELEHNT, nicht geraten");
        pruefe (! flachesJsonObjekt ("{\"a\":[1]}", felder), "Arrays ebenso");
        pruefe (! flachesJsonObjekt ("{\"a\":\"b\\u0041\"}", felder),
                "ein Escape wird abgelehnt statt interpretiert");
        pruefe (! flachesJsonObjekt ("{\"a\":1,\"a\":2}", felder),
                "doppelter Schluessel ist eine Ablehnung, keine 'letzter gewinnt'-Regel");
        pruefe (! flachesJsonObjekt ("{\"a\":1} nachspann", felder),
                "Text hinter dem Objekt wird abgelehnt");
        pruefe (flachesJsonObjekt ("  { }  ", felder) && felder.empty(),
                "das leere Objekt ist gueltig");
    }

    abschnitt ("I · Phase B: semantisches command_ack und In-Flight-Register");
    {
        bool alleAckWerte = true;
        for (const auto [art, name] : std::vector<std::pair<int, const char*>> {
                 { 1, "angewandt" }, { 5, "idempotent_wiederholt" } })
        {
            TestServer server (testPipeName (name));
            server.commandAckArt.store (art);
            server.starten();
            const auto commandId = hex32 (art == 1 ? '1' : '5');
            ControlClient control ([&] {
                ControlHello h;
                h.adresse = testAdresse (hex32 ('a'));
                return h;
            }, server.pipeName());
            control.start();
            const bool verbunden = warteAuf (5000, [&] {
                return control.snapshot().status == ControlClient::Status::verbunden;
            });
            const bool angenommen = verbunden
                && control.sendePersistenzP0 (persistenzBefehl (commandId));
            const bool frei = angenommen && warteAuf (3000, [&] {
                const auto s = control.snapshot();
                return s.inFlight == 0 && s.inFlightErfolg == 1;
            });
            pruefe (frei,
                    "command_ack_angewandt_und_idempotent_wiederholt_geben_inflight_als_erfolg_frei",
                    name);
            alleAckWerte = alleAckWerte && frei;
            control.stop();
            server.stoppen();
        }

        bool alleEndgueltig = true;
        for (const auto [art, name] : std::vector<std::pair<int, const char*>> {
                 { 2, "abgelehnt" }, { 3, "konflikt" }, { 4, "abgelaufen" } })
        {
            TestServer server (testPipeName (name));
            server.commandAckArt.store (art);
            server.starten();
            ControlClient control ([&] {
                ControlHello h;
                h.adresse = testAdresse (hex32 ('b'));
                return h;
            }, server.pipeName());
            control.start();
            const bool verbunden = warteAuf (5000, [&] {
                return control.snapshot().status == ControlClient::Status::verbunden;
            });
            const bool gesendet = verbunden && control.sendePersistenzP0 (
                persistenzBefehl (hex32 (static_cast<char> ('5' + art))));
            const bool frei = gesendet && warteAuf (3000, [&] {
                const auto s = control.snapshot();
                return s.inFlight == 0 && s.inFlightErfolg == 0
                    && s.inFlightEndgueltigOhneErfolg == 1;
            });
            alleEndgueltig = alleEndgueltig && frei;
            alleAckWerte = alleAckWerte && frei;
            control.stop();
            server.stoppen();
        }
        pruefe (alleEndgueltig,
                "command_ack_endgueltige_fehler_geben_ohne_erfolg_frei");
        pruefe (alleAckWerte,
                "alle_command_ack_ergebnisse_beenden_inflight_korrekt");

        {
            TestServer server (testPipeName ("queue-vs-inflight"));
            server.commandAckArt.store (1);
            server.commandAckVerzoegerungMs.store (300);
            server.starten();
            ControlClient control ([&] {
                ControlHello h;
                h.adresse = testAdresse (hex32 ('9'));
                return h;
            }, server.pipeName());
            control.start();
            const bool verbunden = warteAuf (5000, [&] {
                return control.snapshot().status == ControlClient::Status::verbunden;
            });
            const bool gesendet = verbunden && control.sendePersistenzP0 (
                persistenzBefehl (hex32 ('9')));
            const bool queueFreiSemantikOffen = gesendet && warteAuf (1000, [&] {
                const auto s = control.snapshot();
                return server.p0.load() >= 1 && s.p0Gesendet >= 1 && s.inFlight == 1;
            });
            const bool danachFrei = warteAuf (3000, [&] {
                const auto s = control.snapshot();
                return s.inFlight == 0 && s.inFlightErfolg == 1;
            });
            pruefe (queueFreiSemantikOffen && danachFrei,
                    "queueplatz_frei_aber_semantischer_auftrag_bleibt_inflight");
            control.stop();
            server.stoppen();
        }

        {
            const auto name = testPipeName ("killvorack");
            auto server1 = std::make_unique<TestServer> (name);
            server1->commandVorAckSchliessen.store (true);
            server1->starten();
            const auto commandId = hex32 ('c');
            const auto befehl = persistenzBefehl (commandId);
            ControlClient control ([&] {
                ControlHello h;
                h.adresse = testAdresse (hex32 ('c'));
                return h;
            }, name);
            control.start();
            const bool verbunden = warteAuf (5000, [&] {
                return control.snapshot().status == ControlClient::Status::verbunden;
            });
            const bool gesendet = verbunden && control.sendePersistenzP0 (befehl);
            const bool ersterVersuch = gesendet && warteAuf (3000, [&] {
                return server1->p0.load() >= 1
                    && control.snapshot().status != ControlClient::Status::verbunden;
            });
            server1->stoppen();
            server1.reset();

            auto server2 = std::make_unique<TestServer> (name);
            server2->commandAckArt.store (1);
            server2->starten();
            const bool wiederholt = ersterVersuch && warteAuf (12000, [&] {
                const auto s = control.snapshot();
                return s.inFlight == 0 && s.inFlightErfolg == 1
                    && s.inFlightWiederholungen >= 1;
            });
            bool gleicherText = false;
            {
                std::lock_guard<std::mutex> l (server2->textMutex);
                gleicherText = std::find (server2->p0Texte.begin(), server2->p0Texte.end(),
                                           befehl) != server2->p0Texte.end();
            }
            pruefe (wiederholt && gleicherText,
                    "brokerkill_vor_ack_reiht_dieselbe_command_id_wieder_ein");
            control.stop();
            server2->stoppen();
        }

        {
            TestServer server (testPipeName ("ackuuidfern"));
            server.commandAckArt.store (1);
            server.commandAckMitEventUuid.store (true);
            server.starten();
            ControlClient control ([&] {
                ControlHello h;
                h.adresse = testAdresse (hex32 ('d'));
                return h;
            }, server.pipeName());
            control.start();
            const bool verbunden = warteAuf (5000, [&] {
                return control.snapshot().status == ControlClient::Status::verbunden;
            });
            const bool gesendet = verbunden && control.sendePersistenzP0 (
                persistenzBefehl (hex32 ('d')));
            const bool ungueltigBleibt = gesendet && warteAuf (3000, [&] {
                const auto s = control.snapshot();
                return s.empfangen >= 1 && s.inFlight == 1 && s.inFlightErfolg == 0;
            });
            server.commandAckMitEventUuid.store (false);
            server.commandAckArt.store (5);
            control.reconnect();
            const bool ohneUuidFrei = warteAuf (12000, [&] {
                const auto s = control.snapshot();
                return s.inFlight == 0 && s.inFlightErfolg == 1;
            });
            pruefe (ungueltigBleibt && ohneUuidFrei,
                    "command_ack_traegt_keine_event_uuid");
            control.stop();
            server.stoppen();
        }

        {
            struct VertragsbruchFall
            {
                int bruch;
                int ackArt;
                char commandZeichen;
                const char* name;
            };
            const VertragsbruchFall faelle[] = {
                { 1, 1, 'e', "revision-mit-fuehrender-null" },
                { 2, 2, 'f', "state-hash-mit-falschem-typ" },
                { 3, 2, 'a', "unbekannter-fehlercode" },
                { 4, 1, 'b', "erfolg-ohne-pflicht-hash" },
            };
            bool alleAbgewiesen = true;
            for (const auto& fall : faelle)
            {
                TestServer server (testPipeName (fall.name));
                server.commandAckArt.store (fall.ackArt);
                server.commandAckVertragsbruch.store (fall.bruch);
                server.starten();
                ControlClient control ([&] {
                    ControlHello h;
                    h.adresse = testAdresse (hex32 (fall.commandZeichen));
                    return h;
                }, server.pipeName());
                control.start();
                const bool verbunden = warteAuf (5000, [&] {
                    return control.snapshot().status == ControlClient::Status::verbunden;
                });
                const bool gesendet = verbunden && control.sendePersistenzP0 (
                    persistenzBefehl (hex32 (fall.commandZeichen)));
                const bool bliebOffen = gesendet && warteAuf (3000, [&] {
                    const auto s = control.snapshot();
                    return s.empfangen >= 1 && s.inFlight == 1
                        && s.inFlightErfolg == 0
                        && s.inFlightEndgueltigOhneErfolg == 0;
                });

                server.commandAckVertragsbruch.store (0);
                control.reconnect();
                const bool danachFrei = bliebOffen && warteAuf (12000, [&] {
                    const auto s = control.snapshot();
                    return s.inFlight == 0
                        && s.inFlightErfolg + s.inFlightEndgueltigOhneErfolg == 1;
                });
                alleAbgewiesen = alleAbgewiesen && bliebOffen && danachFrei;
                control.stop();
                server.stoppen();
            }
            pruefe (alleAbgewiesen,
                    "nur_schemafestes_command_ack_gibt_inflight_frei");
        }
    }

    abschnitt ("J · Phase B: Broker-Autostart und Signaturkette");
    {
        fehlender_broker_ist_einzige_startbedingung_auch_mit_serverauth();
        serverauth_gueltiger_pid_sid_datei_hash_und_signer_vor_hello();
        serverauth_fremde_sid_und_tokenabfragefehler_senden_null_bytes();
        serverauth_bild_dateiid_hash_signatur_und_signer_fail_closed();
        waitnamedpipe_toctou_wird_am_verbundenen_handle_erkannt();
        control_und_telemetry_authentisieren_jedes_handle();
        vorhandene_pipe_wird_nur_mit_dem_authentisierten_handle_uebernommen();
        unverifizierte_belegte_pipe_spawnt_und_reconnectet_nicht();
        spawn_pid_ersetzt_serverauth_nicht_und_fremdpipe_im_bereitfenster_faellt();
        reconnect_prueft_pid_sid_und_bild_ohne_altfreigabe();
        serverauth_nach_brokercrash_verweigert_pidwechsel_und_fremdbild();
        serverauth_nach_idle_exit_verweigert_fremde_pipe();

        pruefe (! spawnRetryFaellig (SPAWN_CONNECT_BACKOFF_START_MS - 1)
                    && spawnRetryFaellig (SPAWN_CONNECT_BACKOFF_START_MS)
                    && ! spawnBereitTimeoutAbgelaufen (SPAWN_BEREIT_TIMEOUT_MS - 1)
                    && spawnBereitTimeoutAbgelaufen (SPAWN_BEREIT_TIMEOUT_MS)
                    && ! spawnCooldownAbgelaufen (SPAWN_COOLDOWN_MS - 1)
                    && spawnCooldownAbgelaufen (SPAWN_COOLDOWN_MS),
                "spawn_retry_bereit_timeout_und_cooldown_grenzen");
        pruefe (kBackoffStartMs == 500 && kBackoffMaxMs == 8000,
                "normaler_reconnect_backoff_start_und_max");
        pruefe (! brokerIdleEndeErreicht (BROKER_IDLE_ENDE_MS - 1, 0)
                    && brokerIdleEndeErreicht (BROKER_IDLE_ENDE_MS, 0),
                "letzter_client_idle_stop_an_grenze");
        pruefe (! brokerIdleEndeErreicht (BROKER_IDLE_ENDE_MS, BROKER_PRO_USER_MAX),
                "aktiver_fremdclient_verhindert_stop");
        pruefe (! brokerIdleEndeErreicht (0, 0)
                    && ! brokerIdleEndeErreicht (BROKER_IDLE_ENDE_MS - 1, 0)
                    && brokerIdleEndeErreicht (BROKER_IDLE_ENDE_MS, 0),
                "client_resetet_idlefrist");

        {
            const auto name = testPipeName ("broker-fehlt-signal");
            ControlClient fehlt ([&] {
                ControlHello h;
                h.adresse = testAdresse (hex32 ('6'));
                return h;
            }, name);
            fehlt.start();
            const bool echteFehlendePipe = warteAuf (3000, [&] {
                const auto s = fehlt.snapshot();
                return s.status == ControlClient::Status::getrennt
                    && s.brokerPipeFehlt;
            });
            fehlt.stop();

            ControlClient lokalerVertragsfehler ([&] {
                ControlHello h;
                h.adresse = testAdresse (hex32 ('7'));
                h.samplerate = 0.0;
                return h;
            }, name);
            lokalerVertragsfehler.start();
            const bool keinFalschesFehltsignal = warteAuf (3000, [&] {
                const auto s = lokalerVertragsfehler.snapshot();
                return s.verbindungsVersuche > 0
                    && s.status == ControlClient::Status::getrennt
                    && ! s.brokerPipeFehlt;
            });
            lokalerVertragsfehler.stop();
            pruefe (echteFehlendePipe && keinFalschesFehltsignal,
                    "fehlender_broker_ist_notwendige_startbedingung");
        }

        std::vector<std::string> reihenfolge;
        BrokerLifecycleHooks ordnung;
        ordnung.verbunden = [&] { reihenfolge.emplace_back ("connect"); return false; };
        ordnung.connectFehlgeschlagen = [&] {
            reihenfolge.emplace_back ("connect_fehlgeschlagen"); return true;
        };
        ordnung.darfStarten = [&] { reihenfolge.emplace_back ("gate"); return true; };
        ordnung.pruefen = [&] {
            reihenfolge.emplace_back ("hash_signatur"); return BrokerPruefBericht {};
        };
        ordnung.spawn = [&] { reihenfolge.emplace_back ("spawn"); return true; };
        ordnung.mutexName = L"Local\\Nakama.PhaseB.IpcTest.Reihenfolge."
                          + std::to_wstring (GetCurrentProcessId());
        BrokerLifecycle geordnet (std::move (ordnung));
        geordnet.tickFuerTest (0);
        const auto spawnPos = std::find (reihenfolge.begin(), reihenfolge.end(), "spawn");
        const auto pruefPos = std::find (reihenfolge.begin(), reihenfolge.end(), "hash_signatur");
        pruefe (! reihenfolge.empty() && reihenfolge.front() == "connect",
                "connect_without_spawn_kommt_zuerst");
        pruefe (pruefPos != reihenfolge.end() && spawnPos != reihenfolge.end()
                    && pruefPos < spawnPos,
                "autostart_hash_vor_signatur_vor_spawn");

        {
            std::atomic<bool> gate { true };
            int spawns = 0;
            BrokerLifecycleHooks h;
            h.verbunden = [] { return false; };
            h.connectFehlgeschlagen = [] { return true; };
            h.darfStarten = [&] { return gate.load(); };
            h.pruefen = [&] {
                // Das bildet Editor-zu/Rollenwechsel waehrend der langsamen
                // Artefaktpruefung exakt nach.
                gate.store (false);
                return BrokerPruefBericht {};
            };
            h.spawn = [&] { ++spawns; return true; };
            h.mutexName = L"Local\\Nakama.PhaseB.IpcTest.GateVorSpawn."
                        + std::to_wstring (GetCurrentProcessId());
            BrokerLifecycle lifecycle (std::move (h));
            lifecycle.tickFuerTest (0);
            pruefe (spawns == 0 && lifecycle.snapshot().spawnVersuche == 0
                        && ! lifecycle.snapshot().imCooldown,
                    "lifecycle_gate_wird_unmittelbar_vor_spawn_neu_geprueft");
        }

        {
            int retries = 0, spawns = 0;
            BrokerLifecycleHooks h;
            h.verbunden = [] { return false; };
            h.connectFehlgeschlagen = [] { return true; };
            h.darfStarten = [] { return true; };
            h.reconnect = [&] { ++retries; };
            h.pruefen = [] { return BrokerPruefBericht {}; };
            h.spawn = [&] { ++spawns; return true; };
            h.mutexName = L"Local\\Nakama.PhaseB.IpcTest.Grenzen."
                        + std::to_wstring (GetCurrentProcessId());
            BrokerLifecycle lifecycle (std::move (h));
            lifecycle.tickFuerTest (0);
            lifecycle.tickFuerTest (SPAWN_CONNECT_BACKOFF_START_MS - 1);
            const bool vorRetry = retries == 0;
            lifecycle.tickFuerTest (SPAWN_CONNECT_BACKOFF_START_MS);
            const bool retryAnGrenze = retries == 1;
            lifecycle.tickFuerTest (SPAWN_BEREIT_TIMEOUT_MS - 1);
            const bool vorTimeout = lifecycle.snapshot().wartetAufBereit;
            lifecycle.tickFuerTest (SPAWN_BEREIT_TIMEOUT_MS);
            const bool timeout = lifecycle.snapshot().imCooldown;
            lifecycle.tickFuerTest (SPAWN_BEREIT_TIMEOUT_MS + SPAWN_COOLDOWN_MS - 1);
            const bool vorCooldown = spawns == 1;
            lifecycle.tickFuerTest (SPAWN_BEREIT_TIMEOUT_MS + SPAWN_COOLDOWN_MS);
            lifecycle.tickFuerTest (SPAWN_BEREIT_TIMEOUT_MS + SPAWN_COOLDOWN_MS + 1);
            pruefe (vorRetry && retryAnGrenze && retries >= 1 && vorTimeout && timeout
                        && vorCooldown && spawns == 2,
                    "spawn_retry_bereit_timeout_und_cooldown_grenzen_zustandsmaschine");
        }

        {
            const auto mutexName = L"Local\\Nakama.PhaseB.IpcTest.Parallel."
                                 + std::to_wstring (GetCurrentProcessId());
            std::atomic<int> spawns { 0 };
            std::atomic<int> reconnectZwei { 0 };
            std::atomic<bool> einsVerbunden { false };
            const auto pipe = testPipeName ("autostart-real-existing");
            std::unique_ptr<TestServer> vorhandenerBroker;
            BrokerLifecycleHooks h1;
            h1.verbunden = [&] { return einsVerbunden.load(); };
            h1.connectFehlgeschlagen = [] { return true; };
            h1.darfStarten = [] { return true; };
            h1.pruefen = [] { return BrokerPruefBericht {}; };
            h1.spawn = [&] {
                ++spawns;
                vorhandenerBroker = std::make_unique<TestServer> (pipe);
                return vorhandenerBroker->starten();
            };
            h1.mutexName = mutexName;
            h1.pipeName = pipe;

            BrokerLifecycleHooks h2;
            // Absichtlich stale: der zweite ControlClient hat den neuen
            // Broker noch nicht in seinem Cache gesehen.
            h2.verbunden = [] { return false; };
            h2.connectFehlgeschlagen = [] { return true; };
            h2.darfStarten = [] { return true; };
            h2.reconnect = [&] { ++reconnectZwei; };
            h2.pruefen = [] { return BrokerPruefBericht {}; };
            h2.spawn = [&] { ++spawns; return true; };
            h2.mutexName = mutexName;
            h2.pipeName = pipe;

            BrokerLifecycle eins (std::move (h1)), zwei (std::move (h2));
            std::atomic<bool> einsHaelt { false }, einsFreigeben { false };
            std::thread besitzer ([&] {
                eins.tickFuerTest (0);   // startet Broker und haelt Mutex
                einsHaelt.store (true);
                while (! einsFreigeben.load())
                    std::this_thread::sleep_for (std::chrono::milliseconds (1));
                eins.tickFuerTest (1);   // gibt als verbundener Sieger frei
            });
            const bool ersterHaelt = warteAuf (1000, [&] {
                return einsHaelt.load();
            });
            zwei.tickFuerTest (0);
            const bool pipeReal = warteAuf (3000, [&] {
                return namedPipeErreichbar (pipe);
            });
            einsVerbunden.store (true);
            einsFreigeben.store (true);
            besitzer.join();
            zwei.tickFuerTest (1);
            const bool genauEiner = ersterHaelt && pipeReal
                        && spawns.load() == static_cast<int> (BROKER_PRO_USER_MAX)
                        && zwei.snapshot().mutexVerloren == 1
                        && reconnectZwei.load() == 1
                        && zwei.snapshot().pruefungen == 0
                        && zwei.snapshot().spawnVersuche == 0;
            pruefe (genauEiner,
                    "autostart_parallel_startet_genau_einen_prozess",
                    "haelt=" + std::to_string (ersterHaelt)
                        + " pipe=" + std::to_string (pipeReal)
                        + " spawns=" + std::to_string (spawns.load())
                        + " mutexVerloren="
                        + std::to_string (zwei.snapshot().mutexVerloren)
                        + " reconnect=" + std::to_string (reconnectZwei.load())
                        + " pruefungen="
                        + std::to_string (zwei.snapshot().pruefungen)
                        + " spawnVersuche="
                        + std::to_string (zwei.snapshot().spawnVersuche));
            pruefe (genauEiner && spawns.load() == 1,
                    "autostart_mehrere_mains_ein_broker");
            pruefe (pipeReal && reconnectZwei.load() == 1
                        && zwei.snapshot().pruefungen == 0
                        && zwei.snapshot().spawnVersuche == 0,
                    "autostart_mutex_verlierer_prueft_reale_pipe_trotz_stalem_cache");
            if (vorhandenerBroker != nullptr)
                vorhandenerBroker->stoppen();
        }

        const auto testExeText = juce::File::getSpecialLocation (
            juce::File::currentExecutableFile).getFullPathName();
        const std::wstring testExe (testExeText.toWideCharPointer());
        const auto testHashProbe = brokerBinaryPruefen (
            testExe, std::string (64, '0'), "");
        const auto hashVorSignatur = brokerBinaryPruefen (
            testExe, std::string (64, '0'), std::string (40, '0'));
        const auto hashNull = brokerBinaryPruefen (
            testExe, testHashProbe.dateiSha256, "");
        const auto signaturFehlt = brokerBinaryPruefen (
            testExe, testHashProbe.dateiSha256, std::string (40, '0'));
        pruefe (testHashProbe.hashGeprueft
                    && testHashProbe.fehler == BrokerPruefFehler::hashFalsch
                    && hashVorSignatur.fehler == BrokerPruefFehler::hashFalsch
                    && ! hashVorSignatur.signaturGeprueft
                    && hashNull.ok() && hashNull.hashGeprueft && ! hashNull.signaturGeprueft,
                "thumbprint_null_erlaubt_nur_passenden_hash");
        pruefe (signaturFehlt.fehler == BrokerPruefFehler::signaturFehltOderUngueltig,
                "thumbprint_gesetzt_verlangt_winverifytrust_und_exakten_signer_fehlend");

        wchar_t systemVerzeichnis[MAX_PATH] {};
        const UINT systemLaenge = GetSystemDirectoryW (systemVerzeichnis, MAX_PATH);
        const std::wstring signierteDatei = systemLaenge > 0 && systemLaenge < MAX_PATH
            ? std::wstring (systemVerzeichnis, systemLaenge) + L"\\notepad.exe"
            : std::wstring {};
        const auto signiertHashProbe = brokerBinaryPruefen (
            signierteDatei, std::string (64, '0'), "");
        const auto falscherSigner = brokerBinaryPruefen (
            signierteDatei, signiertHashProbe.dateiSha256, std::string (40, '0'));
        const auto richtigerSigner = falscherSigner.signerThumbprint.empty()
            ? BrokerPruefBericht { BrokerPruefFehler::signaturFehltOderUngueltig }
            : brokerBinaryPruefen (signierteDatei, signiertHashProbe.dateiSha256,
                                   falscherSigner.signerThumbprint);
        pruefe (falscherSigner.fehler == BrokerPruefFehler::signerFalsch,
                "thumbprint_gesetzt_verlangt_winverifytrust_und_exakten_signer_falsch");
        pruefe (richtigerSigner.ok() && richtigerSigner.hashGeprueft
                    && richtigerSigner.signaturGeprueft,
                "thumbprint_gesetzt_verlangt_winverifytrust_und_exakten_signer_gueltig");
        pruefe (signaturFehlt.fehler == BrokerPruefFehler::signaturFehltOderUngueltig
                    && falscherSigner.fehler == BrokerPruefFehler::signerFalsch
                    && richtigerSigner.ok() && richtigerSigner.hashGeprueft
                    && richtigerSigner.signaturGeprueft,
                "thumbprint_gesetzt_verlangt_winverifytrust_und_exakten_signer");

        {
            wchar_t systemPfad[MAX_PATH] {};
            const UINT n = GetSystemDirectoryW (systemPfad, MAX_PATH);
            const std::wstring helfer = n > 0 && n < MAX_PATH
                ? std::wstring (systemPfad, n) + L"\\where.exe"
                : std::wstring {};
            pruefe (! brokerVerborgenStarten (L"where.exe")
                        && ! helfer.empty() && brokerVerborgenStarten (helfer),
                    "spawn_per_createprocessw_nur_absolut_und_verborgen");
        }

        {
            std::atomic<bool> spawnBetreten { false };
            BrokerLifecycleHooks h;
            h.verbunden = [] { return false; };
            h.connectFehlgeschlagen = [] { return true; };
            h.darfStarten = [] { return true; };
            h.pruefen = [] { return BrokerPruefBericht {}; };
            h.spawn = [&] {
                spawnBetreten.store (true);
                std::this_thread::sleep_for (std::chrono::milliseconds (100));
                return true;
            };
            h.mutexName = L"Local\\Nakama.PhaseB.IpcTest.StopSpawn."
                        + std::to_wstring (GetCurrentProcessId());
            BrokerLifecycle lifecycle (std::move (h));
            lifecycle.start();
            const bool betreten = warteAuf (1000, [&] { return spawnBetreten.load(); });
            const auto t0 = std::chrono::steady_clock::now();
            lifecycle.stop();
            const auto stopMs = std::chrono::duration_cast<std::chrono::milliseconds> (
                std::chrono::steady_clock::now() - t0).count();
            pruefe (betreten && stopMs < 1000,
                    "stop_waehrend_spawn");
        }

        {
            const auto brokerDatei = wurzel().getChildFile (
                "broker/target/debug/eqcop-broker-v3probe.exe");
            const std::wstring exe (brokerDatei.getFullPathName().toWideCharPointer());
            const auto pipe = testPipeName ("foreign-broker-owner");
            const std::wstring pipeW (pipe.begin(), pipe.end());
            std::wstring befehl = L"\"" + exe + L"\" \"" + pipeW + L"\" 10";
            SECURITY_ATTRIBUTES vererbbar {};
            vererbbar.nLength = sizeof (vererbbar);
            vererbbar.bInheritHandle = TRUE;
            HANDLE stdinLesen = nullptr, stdinSchreiben = nullptr;
            const BOOL stdinErzeugt = CreatePipe (
                &stdinLesen, &stdinSchreiben, &vererbbar, 0);
            if (stdinErzeugt != FALSE)
                SetHandleInformation (stdinSchreiben, HANDLE_FLAG_INHERIT, 0);
            STARTUPINFOW start {};
            start.cb = sizeof (start);
            start.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
            start.wShowWindow = SW_HIDE;
            start.hStdInput = stdinLesen;
            start.hStdOutput = GetStdHandle (STD_OUTPUT_HANDLE);
            start.hStdError = GetStdHandle (STD_ERROR_HANDLE);
            PROCESS_INFORMATION fremd {};
            const BOOL gestartet = stdinErzeugt != FALSE && brokerDatei.existsAsFile()
                ? CreateProcessW (
                exe.c_str(), befehl.data(), nullptr, nullptr, TRUE,
                CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT,
                nullptr, nullptr, &start, &fremd) : FALSE;
            if (stdinLesen != nullptr)
                CloseHandle (stdinLesen);
            bool nachPluginStopLebendig = false;
            bool realVerbunden = false;
            if (gestartet != FALSE)
            {
                CloseHandle (fremd.hThread);
                ControlClient client ([&] {
                    ControlHello h;
                    h.adresse = testAdresse (hex32 ('a'));
                    return h;
                }, pipe, {}, {}, {}, {},
                serverErwartungFuerTestdatei (exe, fremd.dwProcessId));
                client.start();
                realVerbunden = warteAuf (6000, [&] {
                    return client.snapshot().status == ControlClient::Status::verbunden;
                });
                BrokerLifecycleHooks h;
                h.verbunden = [&] {
                    return client.snapshot().status == ControlClient::Status::verbunden;
                };
                BrokerLifecycle lifecycle (std::move (h));
                lifecycle.start();
                std::this_thread::sleep_for (std::chrono::milliseconds (50));
                lifecycle.stop();
                nachPluginStopLebendig = WaitForSingleObject (fremd.hProcess, 0) == WAIT_TIMEOUT;
                client.stop();
                DWORD geschrieben = 0;
                const char stop[] = "STOP\n";
                WriteFile (stdinSchreiben, stop, sizeof (stop) - 1, &geschrieben, nullptr);
                if (WaitForSingleObject (fremd.hProcess, 3000) == WAIT_TIMEOUT)
                {
                    TerminateProcess (fremd.hProcess, 1);
                    WaitForSingleObject (fremd.hProcess, 3000);
                }
                CloseHandle (fremd.hProcess);
            }
            if (stdinSchreiben != nullptr)
                CloseHandle (stdinSchreiben);
            pruefe (gestartet != FALSE && realVerbunden && nachPluginStopLebendig,
                    "plugin_stoppt_keinen_fremden_brokerprozess");
        }

        pruefe (AUTOSTART_ARTEFAKTE_PHASE_B == 0,
                "kein_installer_oder_boot_autostartartefakt");
    }

    abschnitt ("K · NAK-134: ERROR_PIPE_BUSY ist Liveness, kein Sicherheitsurteil");
    {
        oeffnungsausgaenge_sind_liveness_oder_sicherheit();
        authausgaenge_erwartung_und_serverpid_bleiben_sicherheit();
        telemetrie_authfehler_parkt_wie_control();
        ressourcenfehler_nach_authbeweis_ist_liveness();
        zaehlervertrag_verbindungsversuche_und_serverpruefungen();
        kopplungswarte_faelscht_den_backoff_nicht();
        parken_uebergeht_den_backoff();
        parken_loest_ueber_reconnect_und_stop();
        belegt_nicht_erreicht_spawnt_nicht_und_blockiert_nicht();
        abbruchsignal_gehoert_der_generation();
        abbruch_vor_und_nach_createfile();
        backoff_folge_und_deckel_sind_beobachtbar();
    }

    {
        {
            const auto repo = wurzel();
            const auto install = repo.getChildFile ("eq-copilot/install");
            const auto manifestDatei = install.getChildFile ("nakama-installer-v1.json");
            const auto manifest = juce::JSON::parse (manifestDatei);
            const auto* artefakte = manifest.getProperty ("artefakte", {}).getArray();
            std::set<std::string> arten;
            if (artefakte != nullptr)
                for (const auto& artefakt : *artefakte)
                    arten.insert (artefakt.getProperty ("art", "").toString().toStdString());

            juce::Array<juce::File> dateien;
            install.findChildFiles (dateien, juce::File::findFiles, true);
            bool keineStartdatei = true;
            bool keineStartanweisung = true;
            for (const auto& datei : dateien)
            {
                const auto name = datei.getFileName().toLowerCase();
                keineStartdatei = keineStartdatei
                    && ! name.endsWith (".lnk") && ! name.endsWith (".url")
                    && ! name.endsWith (".job") && ! name.contains ("startup");
                if (datei.hasFileExtension ("ps1;cmd;bat"))
                {
                    const auto text = datei.loadFileAsString().toLowerCase();
                    keineStartanweisung = keineStartanweisung
                        && ! text.contains ("new-service")
                        && ! text.contains ("schtasks")
                        && ! text.contains ("currentversion\\run")
                        && ! text.contains ("startup\\");
                }
            }
            const auto runner = repo.getChildFile ("tools/beweise.ps1").loadFileAsString();
            const auto installer = install.getChildFile ("Install-Nakama.ps1").loadFileAsString();
            const bool rueckwegGebunden = runner.contains ("Kuerzel='A17'")
                                      && runner.contains ("Kuerzel='A18'")
                                      && installer.contains ("[switch]$Rueckweg")
                                      && ! manifest.getProperty ("rueckweg", {}).isVoid();
            pruefe (artefakte != nullptr && artefakte->size() == 3
                        && arten == std::set<std::string> { "broker", "vst3" }
                        && keineStartdatei && keineStartanweisung
                        && rueckwegGebunden,
                    "installer_boot_autostartinventar_ist_null_und_rueckweg_haengt_an_a17_a18");
        }
    }

    // ── SONDE-013 · die drei Experimentfamilien auf der C++-Haelfte ────────
    //
    // Zwei Ebenen, weil eine allein nichts sagt:
    //
    //   1. der C++-Vertragsleser nimmt die committeten Fixtures der drei
    //      Familien an - dieselben Dateien, die Rust und das Referenzbein
    //      lesen. Ein Fixture, das nur eine Sprache annimmt, ist ein Befund;
    //   2. eine dieser Nachrichten laeuft als PERSISTENZAUFTRAG durch den
    //      echten ControlClient. Sie traegt denselben `steuerkopf` wie jede
    //      andere steuernde Nachricht, also darf das In-Flight-Register keine
    //      Sonderbehandlung brauchen. Genau das wird hier gemessen und nicht
    //      angenommen.
    abschnitt ("SONDE-013 · experiment_begin/_abort/_manual_result (C++-Haelfte)");
    {
        const auto schemaDatei = wurzel()
            .getChildFile ("eq-copilot/schemas/v3/eq-ipc-v3.schema.json");
        nakama::vertrag::Schema schema;
        juce::String ladefehler;
        const bool geladen = nakama::vertrag::Schema::laden (
            juce::JSON::parse (schemaDatei), schema, ladefehler);
        pruefe (geladen, "v3-Schema laedt in die C++-Engine", ladefehler.toStdString());

        const auto basis = wurzel().getChildFile ("eq-copilot/fixtures/v3/gueltig");
        bool alleAngenommen = geladen;
        std::string erstesAbweichende;
        // Nacharbeit 2 (Befunde R16/R21): `experiment_candidate` ist die
        // vierte Familie dieses Tickets - der Schritt zwischen Begin und
        // Ergebnis, der den Kandidaten erfasst und die Blindreihenfolge bindet.
        for (const char* name : { "experiment_begin", "experiment_candidate",
                                  "experiment_abort", "experiment_manual_result" })
        {
            const auto datei = basis.getChildFile (juce::String (name) + ".json");
            const auto daten = juce::JSON::parse (datei);
            const auto verletzungen = geladen ? schema.pruefe (daten)
                                              : juce::Array<nakama::vertrag::Verletzung> {};
            if (! datei.existsAsFile() || ! verletzungen.isEmpty())
            {
                alleAngenommen = false;
                if (erstesAbweichende.empty())
                    erstesAbweichende = name;
            }
        }
        pruefe (alleAngenommen,
                "experiment_familien_werden_von_der_cpp_engine_angenommen",
                erstesAbweichende);

        // Gegenprobe: der neue Discriminator ist WIRKLICH geschlossen. Ohne
        // sie sagt die Zeile darueber nur, dass die Engine ueberhaupt etwas
        // annimmt.
        auto fremd = juce::JSON::parse (basis.getChildFile ("experiment_begin.json"));
        if (auto* obj = fremd.getDynamicObject())
            obj->setProperty ("execution_mode", "active_probe");
        pruefe (geladen && ! schema.pruefe (fremd).isEmpty(),
                "fremder_execution_mode_faellt_an_derselben_engine");

        TestServer server (testPipeName ("sonde013-experiment"));
        server.commandAckArt.store (1);
        server.starten();
        ControlClient control ([&] {
            ControlHello h;
            h.adresse = testAdresse (hex32 ('e'));
            return h;
        }, server.pipeName());
        control.start();
        const bool verbunden = warteAuf (5000, [&] {
            return control.snapshot().status == ControlClient::Status::verbunden;
        });
        const bool angenommen = verbunden && control.sendePersistenzP0 (
            experimentErgebnisBefehl (hex32 ('7')));
        const bool frei = angenommen && warteAuf (3000, [&] {
            const auto s = control.snapshot();
            return s.inFlight == 0 && s.inFlightErfolg == 1;
        });
        pruefe (frei,
                "experiment_manual_result_ist_ein_persistenzpflichtiger_p0_befehl");
        control.stop();
        server.stoppen();
    }

    // ── NAK-180 R7/R10/R12/R13: die P0-Queue trägt Klasse, Generation und
    //    Marke (Matrix N-25 bis N-27, N-35 bis N-37) ─────────────────────────
    //
    // Gemessen wird die Queue selbst, nicht der ControlClient darum herum:
    // jede der vier Prüfungsrunden fand ihren Defekt genau hier — im
    // Verhältnis zwischen einem Eintrag und dem Link, für den er gilt.
    {
        using nakama::ipc::P0Warteschlange;
        using nakama::ipc::P0Eintrag;
        using nakama::ipc::P0Klasse;

        // -- N-26 (a): Berichte älterer Generation fallen beim Aufbau -------
        {
            P0Warteschlange q;
            q.einreihen (P0Eintrag { "{\"alt\":1}", P0Klasse::bericht,  1, 11 });
            q.einreihen (P0Eintrag { "{\"ev\":1}",  P0Klasse::ereignis, 1, 12 });
            q.einreihen (P0Eintrag { "{\"neu\":1}", P0Klasse::bericht,  2, 13 });

            std::vector<std::uint64_t> verworfen;
            const auto gefallen = q.berichteAelterAls (
                2, [&verworfen] (std::uint64_t m) { verworfen.push_back (m); });

            pruefe (gefallen == 1 && verworfen.size() == 1 && verworfen[0] == 11,
                    "NAK-180 R7: aufbaufilter_verwirft_nur_alte_berichte - genau der "
                    "Bericht der alten Generation faellt, gemeldet mit seiner Marke",
                    std::to_string (gefallen) + " gefallen");
            pruefe (q.groesse() == 2,
                    "NAK-180 R7: das EREIGNIS bleibt - fuer P0-Ereignisse gilt "
                    "'nichts verwerfen' unveraendert (Paragraph 53.9)",
                    std::to_string (q.groesse()));
            pruefe (q.verworfen() == 1,
                    "NAK-180 R7: und der Verwurf ist GEZAEHLT, nie stillschweigend",
                    std::to_string ((unsigned long long) q.verworfen()));
        }

        // -- N-26 (b): ein Bericht der NEUEN Generation ueberlebt -----------
        //
        // Das ist die Haelfte, die MP2-1 offenliess: wird waehrend des
        // Link-Callbacks eingereiht - also nachdem die Generation vergeben
        // ist -, traegt der Eintrag die neue Zahl und darf nicht fallen.
        {
            P0Warteschlange q;
            q.einreihen (P0Eintrag { "{\"replay\":1}", P0Klasse::bericht, 2, 21 });
            std::vector<std::uint64_t> verworfen;
            q.berichteAelterAls (2, [&verworfen] (std::uint64_t m) { verworfen.push_back (m); });
            pruefe (q.groesse() == 1 && verworfen.empty(),
                    "NAK-180 R10: ein waehrend des Callbacks eingereihtes Replay traegt "
                    "die NEUE Generation und wird ZUGESTELLT, nicht verworfen");
        }

        // -- N-36: das `end` in der Queue beim Aufbau von G+1 ---------------
        //
        // Dieser Bruch braucht kein Rennen: er entsteht allein daraus, dass
        // die Queue den Linkwechsel ueberlebt. Ohne die Zustellpruefung
        // traefe das `end` beim Broker auf kein Begin.
        {
            P0Warteschlange q;
            q.einreihen (P0Eintrag { "{\"end\":1}", P0Klasse::intervention, 1, 31 });
            pruefe (q.hatInterventionsereignisAelterAls (2),
                    "NAK-180 R12: der Aufbauzug SIEHT das Interventionsereignis "
                    "aelterer Generation");
            // 🔑 Nacharbeit 1 (EP-09): und NUR dieses. Ein persistenter
            // P0-Befehl ist ebenfalls ein Ereignis und ueberlebt den Wechsel -
            // er sagt aber nichts ueber Interventionen. Loeste er die
            // Zustellpruefung aus, entstuende ein Replay, das niemand
            // angefordert hat, und mit `replayFaellig` daneben die doppelte
            // `intervention_id` aus N-27.
            {
                P0Warteschlange nurBefehl;
                nurBefehl.einreihen (P0Eintrag { "{\"cmd\":1}", P0Klasse::ereignis, 1, 91 });
                pruefe (! nurBefehl.hatInterventionsereignisAelterAls (2),
                        "NAK-180 EP-09: ein persistenter P0-Befehl loest die "
                        "Zustellpruefung NICHT aus - nur Interventionen tun das");
            }

            const bool vorn = q.voranstellen (
                P0Eintrag { "{\"replaybegin\":1}", P0Klasse::bericht, 2, 32 });
            P0Eintrag erster;
            pruefe (vorn && q.entnehmen (erster) && erster.json == "{\"replaybegin\":1}",
                    "NAK-180 R12: zustellpruefung_stellt_das_replay_voran - auf dem neuen "
                    "Link reist Begin VOR End, ohne dass ein Rennen noetig waere",
                    erster.json);
            q.bestaetigen();
            P0Eintrag zweiter;
            pruefe (q.entnehmen (zweiter) && zweiter.json == "{\"end\":1}",
                    "NAK-180 R12: und das `end` folgt unmittelbar", zweiter.json);
            q.bestaetigen();
        }

        // -- N-25: Abbruch zwischen Enqueue und Wire-Commit -----------------
        //
        // Ein BERICHT wird nicht zurueckgelegt: seine Aussage galt dem Link,
        // der gerade stirbt. Zurueckgelegt reiste er auf dem naechsten und
        // behauptete dort etwas Falsches (MP1-1).
        {
            P0Warteschlange q;
            q.einreihen (P0Eintrag { "{\"hb\":1}", P0Klasse::bericht, 1, 41 });
            P0Eintrag raus;
            pruefe (q.entnehmen (raus) && raus.klasse == P0Klasse::bericht,
                    "NAK-180 R7: der Bericht ist entnommen und reserviert");
            q.fallenLassen();
            pruefe (q.groesse() == 0 && q.inFlug() == 0 && q.verworfen() == 1,
                    "NAK-180 R7: bericht_wird_beim_write_fehler_fallen_gelassen - er geht "
                    "NICHT zurueck in die Queue, und der Platz wird frei",
                    std::to_string (q.groesse()) + "/" + std::to_string (q.inFlug()));
        }

        // -- N-27: ein EREIGNIS geht dagegen unveraendert zurueck -----------
        {
            P0Warteschlange q;
            q.einreihen (P0Eintrag { "{\"begin\":1}", P0Klasse::ereignis, 1, 51 });
            P0Eintrag raus;
            q.entnehmen (raus);
            q.zuruecklegen (std::move (raus));
            P0Eintrag wieder;
            pruefe (q.groesse() == 1 && q.entnehmen (wieder)
                        && wieder.json == "{\"begin\":1}"
                        && wieder.klasse == P0Klasse::ereignis
                        && wieder.generation == 1 && wieder.marke == 51,
                    "NAK-180 R7: ein Ereignis geht mit Klasse, Generation und Marke "
                    "UNVERAENDERT zurueck - es reist auf dem neuen Link von selbst, "
                    "und genau deshalb darf der Prozessor es nicht replayen (N-27)");
            q.bestaetigen();
        }

        // -- N-37: CAS statt blindem store - kein Lost-Update ---------------
        //
        // Der Verbraucher hat G beobachtet, der positive Callback von G+1
        // schreibt dazwischen. Ein blindes `store(0)` nahm dessen Wirkung
        // mit; ein verlorenes `neutralerNeuaufbau` ist D-01 in neuer Form.
        {
            std::atomic<std::uint64_t> flag { 1 };
            auto gesehen = flag.load();          // Verbraucher sieht G = 1
            flag.store (2);                      // Callback von G+1 schreibt
            const bool geloescht = flag.compare_exchange_strong (gesehen, 0);
            pruefe (! geloescht && flag.load() == 2,
                    "NAK-180 R13: aufraeumen_per_cas_verliert_kein_update - das CAS "
                    "schlaegt fehl, und die Wirkung von G+1 bleibt stehen",
                    std::to_string ((unsigned long long) flag.load()));

            // Gegenprobe: steht der beobachtete Wert noch, raeumt das CAS auf.
            std::atomic<std::uint64_t> flag2 { 1 };
            auto g2 = flag2.load();
            pruefe (flag2.compare_exchange_strong (g2, 0) && flag2.load() == 0,
                    "NAK-180 R13: und wenn niemand dazwischenschrieb, raeumt es auf");
        }
    }

    // ── NAK-180 R11/R12/R13: erzwungenes true/false-Interleaving
    //    (Matrix N-23, N-24, N-34, N-35) ────────────────────────────────────
    //
    // Vier Pruefungsrunden fanden ihren Defekt in derselben Luecke: eine
    // Aussage ueber "diesen Link", deren Zeitpunkt gegenueber dem
    // Generationswechsel offen blieb. Gemessen wird deshalb das
    // ZUSAMMENTREFFEN, nicht die Absicht - mit einem zweiten Thread und einer
    // deterministischen Schranke, nie mit `sleep` als einziger Ordnung.
    {
        nakama::ipc::ControlClient client ([] { return ControlHello {}; },
                                          testPipeName ("nak180-interleave"));

        // (N-34, Fall 1) Der positive Callback von G1 wird FESTGEHALTEN.
        // Waehrenddessen laeuft der naechste Aufbau (G2) vollstaendig durch -
        // genau die Lage, die `reconnect()` erzeugt: sein negativer Callback
        // laeuft synchron auf dem Aufruferthread, waehrend der positive noch
        // steht.
        std::mutex m;
        std::condition_variable cv;
        bool g1DarfSchreiben = false;
        bool g1Steht = false;
        std::uint64_t g1 = 0, g2 = 0;

        std::thread haltend ([&]
        {
            g1 = client.linkAufbauFuerTest ([&]
            {
                {
                    std::lock_guard<std::mutex> l (m);
                    g1Steht = true;
                }
                cv.notify_all();
                // Der Callback von G1 haelt hier, bis G2 komplett durch ist.
                std::unique_lock<std::mutex> l (m);
                cv.wait (l, [&] { return g1DarfSchreiben; });
                // ERST JETZT schreibt der ueberholte Callback seine Aussage.
                client.meldeAufbauUrteil (true);
            });
        });

        {
            std::unique_lock<std::mutex> l (m);
            cv.wait (l, [&] { return g1Steht; });
        }

        // G2 baut auf und urteilt NICHT neutral (Marker laeuft).
        g2 = client.linkAufbauFuerTest ([&] { client.meldeAufbauUrteil (false); });

        {
            std::lock_guard<std::mutex> l (m);
            g1DarfSchreiben = true;
        }
        cv.notify_all();
        haltend.join();                     // erst danach steht `g1`
        pruefe (g2 == g1 + 1,
                "NAK-180 N-34: der zweite Aufbau vergibt die naechste Generation",
                std::to_string (g1) + " -> " + std::to_string (g2));

        // Die Aussage des ueberholten Callbacks traegt G1 und ist fuer G2
        // inert. Gemessen am Heartbeat: er darf NICHT `false` tragen.
        nakama::ipc::Adresse a;
        a.logonSid = "S-1-5-21-1"; a.projectBindingId = std::string (32, 'a');
        a.sessionEpoch = std::string (32, 'b'); a.instanceId = std::string (32, 'c');
        a.runtimeNonce = std::string (32, 'd');
        nakama::ipc::ControlStatus st;
        const auto text = client.heartbeatTextFuerTest (a, 1, st);
        pruefe (text.find ("\"intervention_state_unknown\":true") != std::string::npos,
                "NAK-180 N-34: ueberholter_callback_praegt_keinen_spaeteren_link - der "
                "erste Heartbeat von G2 traegt dessen eigenes Urteil (`true`), nicht "
                "das `false`, das der Callback von G1 danach schrieb",
                text.substr (text.size() > 120 ? text.size() - 120 : 0));
        pruefe (text.find ("\"intervention_state_unknown\":false") == std::string::npos,
                "NAK-180 N-23: und die Aussage des toten Links reist nicht mit");
    }

    // ── NAK-180 R6: die Wartefrist gehoert UNS, nicht dem Pipe-Besitzer
    //    (Matrix N-19, N-21, N-22) ──────────────────────────────────────────
    {
        // Ein fremder lokaler Prozess darf jeden `\\.\pipe\`-Namen anlegen.
        // Genau das wird hier nachgestellt: eine Pipe mit
        // `nDefaultTimeOut = 0xFFFFFFFE` und OHNE `ConnectNamedPipe`, also
        // ohne horchende Instanz. Mit dem alten `WaitNamedPipeW(name, 0)`
        // uebernahm der Aufrufer diese Frist und stand bis zu ~49,7 Tage —
        // im Lebenslaufthread, der das Win32-Startmutex haelt, und damit
        // spaeter im Destruktor auf dem Message-Thread des Hosts (§48.4).
        //
        // Der Name liegt im PROBE-Namensraum; ein produktiver v3-Name wird
        // hier nie belegt (N-22).
        const auto pipe = testPipeName ("nak180-frist");
        pruefe (istProbePipename (pipe),
                "NAK-180 R6: der Testname liegt im Probe-Namensraum - ein produktiver "
                "v3-Name wird nie belegt");

        std::wstring breit;
        for (char c : pipe)
            breit.push_back (static_cast<wchar_t> (static_cast<unsigned char> (c)));
        HANDLE feindlich = CreateNamedPipeW (
            breit.c_str(), PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            1,                       // nMaxInstances: genau eine, und die ist belegt
            4096, 4096,
            0xFFFFFFFE,              // nDefaultTimeOut: die Frist des ANGREIFERS
            nullptr);
        pruefe (feindlich != INVALID_HANDLE_VALUE,
                "NAK-180 R6: die feindliche Pipe steht (jedes lokale Konto darf das)");

        if (feindlich != INVALID_HANDLE_VALUE)
        {
            const auto start = std::chrono::steady_clock::now();
            const bool erreichbar = namedPipeErreichbar (pipe);
            const auto dauerMs = (long long) std::chrono::duration_cast<std::chrono::milliseconds> (
                std::chrono::steady_clock::now() - start).count();

            pruefe (dauerMs < 1000,
                    "NAK-180 R6: wartefrist_ist_unsere_nicht_die_des_servers - der Aufruf "
                    "kehrt unter 1 s zurueck, obwohl der Pipe-Besitzer 0xFFFFFFFE als "
                    "Default-Timeout gesetzt hat",
                    std::to_string (dauerMs) + " ms");
            // Die Semantik bleibt: eine belegte Instanz heisst "da, nur gerade
            // keine frei" — sonst haelte der Lebenslauf einen gerade
            // startenden eigenen Broker faelschlich fuer abwesend.
            pruefe (erreichbar,
                    "NAK-180 R6: und eine belegte Instanz zaehlt weiter als erreichbar",
                    erreichbar ? "true" : "false");
            CloseHandle (feindlich);
        }

        // N-21: ein Name, den es gar nicht gibt, bleibt "nicht erreichbar" —
        // sonst oeffnete der Lebenslauf den Spawnweg nie.
        const auto leer = testPipeName ("nak180-leer");
        const auto start2 = std::chrono::steady_clock::now();
        const bool nichtDa = namedPipeErreichbar (leer);
        const auto dauer2 = (long long) std::chrono::duration_cast<std::chrono::milliseconds> (
            std::chrono::steady_clock::now() - start2).count();
        pruefe (! nichtDa && dauer2 < 1000,
                "NAK-180 R6: ein nicht existierender Name bleibt nicht erreichbar und "
                "kehrt sofort zurueck",
                std::to_string (dauer2) + " ms");
    }

    std::cout << "\n" << (fehler == 0 ? "ALLE PRUEFUNGEN GRUEN" : "FEHLER")
              << " — " << geprueft << " Pruefungen, " << fehler << " Fehler" << std::endl;
    return fehler == 0 ? 0 : 1;
}
