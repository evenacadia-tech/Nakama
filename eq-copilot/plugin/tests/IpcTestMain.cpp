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
    std::mutex textMutex;
    std::string letztesControlHello, letztesTelemetryHello, letzterAbweisungsgrund;
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
            const bool passt = helloJson.find ("\"link_id\":\"" + linkId + "\"") != std::string::npos
                            && helloJson.find ("\"challenge\":\"" + challenge + "\"") != std::string::npos;
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
        const std::string welcome =
            std::string ("{\"type\":\"welcome\",\"protocol\":3,\"broker_version\":\"test\",")
            + "\"broker_epoch\":\"" + hex32 ('c') + "\",\"link_id\":\"" + linkId
            + "\",\"challenge\":\"" + challenge + "\"}";
        std::vector<std::uint8_t> aus;
        envelopeSchreiben (Familie::p0, 0,
                           reinterpret_cast<const std::uint8_t*> (welcome.data()),
                           welcome.size(), aus);
        if (sendeKaputtenFrame.load())
            aus[4] = 9;  // encoding unbekannt — der Client muss schliessen
        if (! schreiben (h, aus.data(), aus.size()))
        {
            schliessen (h);
            return;
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
                    }
                }
                else if (e.kopf.familie == Familie::p1) ++p1;
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
    return "\\\\.\\pipe\\evenacadia.nakama.v3.test."
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
        }
        std::string weg;
        pruefe (reihenfolge && ! p0.entnehmen (weg),
                "alle 64 kommen unveraendert und in Reihenfolge zurueck");

        P1Warteschlange p1 (4, 4);
        pruefe (p1.einreihen ("a", "a1") == P1Ergebnis::eingereiht, "P1 nimmt Snapshot a");
        pruefe (p1.einreihen ("", "ereignis") == P1Ergebnis::eingereiht, "P1 nimmt Ereignis");
        pruefe (p1.einreihen ("b", "b1") == P1Ergebnis::eingereiht, "P1 nimmt Snapshot b");
        pruefe (p1.einreihen ("a", "a2") == P1Ergebnis::koalesziert
                    && p1.groesse() == 3,
                "ein zweiter Snapshot desselben Objekts koalesziert, ohne zu wachsen");
        std::string s1, s2, s3;
        p1.entnehmen (s1); p1.entnehmen (s2); p1.entnehmen (s3);
        pruefe (s1 == "a2" && s2 == "ereignis" && s3 == "b1",
                "Koaleszieren behaelt die Position, tauscht nur den Inhalt");

        P1Warteschlange p1b (2, 2);
        p1b.einreihen ("", "1"); p1b.einreihen ("", "2");
        pruefe (p1b.einreihen ("", "3") == P1Ergebnis::zurWiederholung
                    && p1b.einreihen ("", "4") == P1Ergebnis::zurWiederholung,
                "nicht koaleszierbare Ereignisse gehen in den Wiederholpuffer");
        pruefe (p1b.einreihen ("", "5") == P1Ergebnis::wiederholungVerdraengt
                    && p1b.verdraengte() == 1,
                "ein voller Wiederholpuffer verdraengt gezaehlt, nie still");
        std::string x;
        p1b.entnehmen (x); p1b.entnehmen (x);
        pruefe (p1b.nachReconnectWiederholen() == 2, "Reconnect holt beide zurueck");
        p1b.entnehmen (s1); p1b.entnehmen (s2);
        pruefe (s1 == "4" && s2 == "5" && p1b.wiederholungen() == 0,
                "und zwar in der urspruenglichen Reihenfolge");
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

        // Nebenlaeufig: ein Erzeuger flutet, ein Verbraucher holt ab. Kein
        // abgeholter Frame darf zerrissen sein.
        auto s2 = std::make_unique<P2Schleuse<256>>();
        std::atomic<bool> stopp { false };
        std::atomic<int> zerrissen { 0 }, geholt { 0 };
        std::thread erzeuger ([&] {
            std::uint8_t f[128];
            for (int i = 0; i < 200000 && ! stopp.load(); ++i)
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
                for (std::size_t i = 1; i < n; ++i)
                    if (z[i] != z[0]) { ++zerrissen; break; }
            }
        });
        erzeuger.join();
        stopp.store (true);
        verbraucher.join();
        pruefe (zerrissen.load() == 0 && geholt.load() > 0,
                "unter Flut ist kein abgeholter Frame zerrissen",
                std::to_string (geholt.load()) + " geholt, "
                    + std::to_string (zerrissen.load()) + " zerrissen, "
                    + std::to_string (s2->ersetzteFrames()) + " ersetzt");
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

    // ── H · Bootstrapgrenze und der strenge kleine JSON-Leser ─────────────
    abschnitt ("H · Bootstrapgrenze und JSON-Riegel");
    {
        std::vector<std::uint8_t> rahmen;
        pruefe (bootstrapRahmen (std::string (kMaxBootstrapBytes, 'x'), rahmen)
                    && rahmen.size() == 4 + kMaxBootstrapBytes,
                "16 KiB sind noch ein Bootstrap-Hello");
        pruefe (! bootstrapRahmen (std::string (kMaxBootstrapBytes + 1, 'x'), rahmen),
                "ein Byte darueber nicht mehr");

        std::vector<std::pair<std::string, std::string>> felder;
        pruefe (flachesJsonObjekt ("{\"a\":\"b\",\"c\":3}", felder) && felder.size() == 2,
                "flaches Objekt wird gelesen");
        std::string w;
        pruefe (jsonFeld (felder, "c", w) && w == "3", "Zahlen kommen als Text zurueck");
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
