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
#include "IpcQueues.h"
#include "IpcVerbindung.h"
#include "PipeToken.h"
#include "TelemetryClient.h"
#include "WireEnvelope.h"

#include <juce_core/juce_core.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
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
    std::mutex textMutex;
    std::string letztesControlHello, letztesTelemetryHello, letzterAbweisungsgrund;
    /// Jeder empfangene P0-/P1-Payload, woertlich. Damit laesst sich pruefen,
    /// ob eine bestimmte Nachricht WIRKLICH angekommen ist — eine Zahl allein
    /// sagt nichts darueber, WELCHE fehlt.
    std::vector<std::string> p0Texte, p1Texte;

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

    explicit TestServer (std::string pipeName) : name (std::move (pipeName)) {}
    ~TestServer() { stoppen(); }

    bool starten()
    {
        laeuft.store (true);
        acceptor = std::thread ([this] { acceptorLauf(); });
        return true;
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
                return;

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
            while (laeuft.load())
                std::this_thread::sleep_for (std::chrono::milliseconds (20));
            schliessen (h);
            return;
        }

        // ── ab hier nur noch v3-Frames ────────────────────────────────────
        StromLeser leser;
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
                    }
                    if (text.find ("\"heartbeat\"") != std::string::npos)
                    {
                        const std::string ack =
                            "{\"type\":\"heartbeat_ack\",\"sequence\":0,\"duplicate_instance_id\":false}";
                        std::vector<std::uint8_t> antwort;
                        envelopeSchreiben (Familie::p0, 0,
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
                }
                else if (e.kopf.familie == Familie::p1)
                {
                    ++p1;
                    std::lock_guard<std::mutex> l (textMutex);
                    p1Texte.emplace_back (reinterpret_cast<const char*> (e.payload),
                                          e.payloadLaenge);
                }
                else                                    ++p2;
            }

            const int gelesen = lesen (h, puffer, sizeof (puffer), 100);
            if (gelesen < 0)
                break;
            if (gelesen > 0)
                leser.fuettern (puffer, static_cast<std::size_t> (gelesen));
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

//==============================================================================
juce::File wurzel()
{
    auto d = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    while (d.exists() && ! d.getChildFile ("eq-copilot").isDirectory())
        d = d.getParentDirectory();
    return d;
}
} // namespace

//==============================================================================
int main()
{
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

        const auto adresse = testAdresse (hex32 ('7'));
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

        // P0-Rundlauf.
        control.sendeP0 ("{\"type\":\"heartbeat\",\"sequence\":1}");
        pruefe (warteAuf (3000, [&] { return server.p0.load() >= 1; }),
                "P0 kommt beim Server als P0-Familie an");
        pruefe (warteAuf (3000, [&] { return control.snapshot().empfangen >= 1; }),
                "und die Antwort erreicht den Client");

        control.sendeP1 ("sonde-1", "{\"type\":\"state_report\"}");
        pruefe (warteAuf (3000, [&] { return server.p1.load() >= 1; }),
                "P1 kommt als P1-Familie an");

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

    std::cout << "\n" << (fehler == 0 ? "ALLE PRUEFUNGEN GRUEN" : "FEHLER")
              << " — " << geprueft << " Pruefungen, " << fehler << " Fehler" << std::endl;
    return fehler == 0 ? 0 : 1;
}
