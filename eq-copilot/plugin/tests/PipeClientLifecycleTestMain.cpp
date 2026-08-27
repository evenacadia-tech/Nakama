// Adversarialer Gegenpfad fuer den echten C++-PipeClient. Jede Pipe traegt
// PID+Zaehler; dieser Test beruehrt niemals den Produktionsnamen.
#include "PipeClient.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <limits>
#include <mutex>
#include <string>
#include <thread>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace
{
int fehler = 0;
std::atomic<unsigned> nameFolge { 0 };

void pruefe (bool ok, const char* text, const juce::String& detail = {})
{
    std::cout << (ok ? "  ok      " : "  FEHLER  ") << text;
    if (detail.isNotEmpty())
        std::cout << "  [" << detail.toRawUTF8() << "]";
    std::cout << std::endl;
    if (! ok)
        ++fehler;
}

template <typename Bedingung>
bool warteAuf (int millisekunden, Bedingung&& bedingung)
{
    for (int i = 0; i < millisekunden / 10; ++i)
    {
        if (bedingung())
            return true;
        std::this_thread::sleep_for (std::chrono::milliseconds (10));
    }
    return bedingung();
}

juce::String testName (const char* fall)
{
    return "\\\\.\\pipe\\evenacadia.eq-copilot.client-test."
         + juce::String ((int) GetCurrentProcessId()) + "."
         + juce::String ((int) nameFolge.fetch_add (1)) + "." + fall;
}

HANDLE pipeAnlegen (const juce::String& name, bool erste = true)
{
    return CreateNamedPipeW (
        name.toWideCharPointer(),
        PIPE_ACCESS_DUPLEX | (erste ? FILE_FLAG_FIRST_PIPE_INSTANCE : 0),
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
        1, 65536, 65536, 0, nullptr);
}

bool verbinden (HANDLE h)
{
    if (ConnectNamedPipe (h, nullptr) != FALSE)
        return true;
    return GetLastError() == ERROR_PIPE_CONNECTED;
}

bool liesGenau (HANDLE h, void* ziel, DWORD bytes)
{
    auto* p = static_cast<unsigned char*> (ziel);
    while (bytes > 0)
    {
        DWORD gelesen = 0;
        if (ReadFile (h, p, bytes, &gelesen, nullptr) == FALSE || gelesen == 0)
            return false;
        p += gelesen;
        bytes -= gelesen;
    }
    return true;
}

bool schreibeGenau (HANDLE h, const void* quelle, DWORD bytes)
{
    const auto* p = static_cast<const unsigned char*> (quelle);
    while (bytes > 0)
    {
        DWORD geschrieben = 0;
        if (WriteFile (h, p, bytes, &geschrieben, nullptr) == FALSE || geschrieben == 0)
            return false;
        p += geschrieben;
        bytes -= geschrieben;
    }
    return true;
}

bool liesFrame (HANDLE h, std::string& aus)
{
    unsigned char kopf[4] {};
    if (! liesGenau (h, kopf, 4))
        return false;
    const auto n = static_cast<DWORD> (kopf[0])
                 | (static_cast<DWORD> (kopf[1]) << 8)
                 | (static_cast<DWORD> (kopf[2]) << 16)
                 | (static_cast<DWORD> (kopf[3]) << 24);
    if (n == 0 || n > 262144)
        return false;
    aus.resize (n);
    return liesGenau (h, aus.data(), n);
}

bool schreibeFrame (HANDLE h, const std::string& text)
{
    const auto n = static_cast<DWORD> (text.size());
    const unsigned char kopf[4] = {
        static_cast<unsigned char> (n & 0xff),
        static_cast<unsigned char> ((n >> 8) & 0xff),
        static_cast<unsigned char> ((n >> 16) & 0xff),
        static_cast<unsigned char> ((n >> 24) & 0xff)
    };
    return schreibeGenau (h, kopf, 4) && schreibeGenau (h, text.data(), n);
}

bool schreibeRohframe (HANDLE h, const unsigned char* bytes, DWORD n)
{
    const unsigned char kopf[4] = {
        static_cast<unsigned char> (n & 0xff),
        static_cast<unsigned char> ((n >> 8) & 0xff),
        static_cast<unsigned char> ((n >> 16) & 0xff),
        static_cast<unsigned char> ((n >> 24) & 0xff)
    };
    return schreibeGenau (h, kopf, 4) && schreibeGenau (h, bytes, n);
}

eqcop::HelloInfo hello (const juce::String& id)
{
    eqcop::HelloInfo h;
    h.sensorId = id;
    h.instanceNonce = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    h.role = "sensor";
    h.label = "PipeClient-Gegenprobe";
    h.samplerate = 48000.0;
    h.blockSize = 512;
    h.channels = 2;
    return h;
}

std::unique_ptr<eqcop::PipeClient> client (
    const juce::String& name, std::function<eqcop::HelloInfo()> provider,
    std::chrono::milliseconds timeout = std::chrono::milliseconds { 5000 })
{
    return std::make_unique<eqcop::PipeClient> (
        std::move (provider), [] { return eqcop::StatsSnapshot {}; },
        std::function<eqcop::MessKompakt()> {}, name, timeout);
}

void pipeSchliessen (HANDLE h)
{
    DisconnectNamedPipe (h);
    CloseHandle (h);
}

void stoppFall (bool bisAck)
{
    const auto name = testName (bisAck ? "stop-ack" : "stop-welcome");
    const auto server = pipeAnlegen (name);
    pruefe (server != INVALID_HANDLE_VALUE, bisAck ? "Fake-Pipe fuer ACK-Stopp angelegt"
                                                   : "Fake-Pipe fuer Welcome-Stopp angelegt");
    if (server == INVALID_HANDLE_VALUE)
        return;

    std::atomic<bool> blockiert { false }, freigeben { false };
    std::atomic<int> impersonation { -1 };
    std::thread peer ([&]
    {
        std::string frame;
        if (verbinden (server) && liesFrame (server, frame))
        {
            if (! bisAck)
            {
                if (ImpersonateNamedPipeClient (server) != FALSE)
                {
                    HANDLE token = nullptr;
                    if (OpenThreadToken (GetCurrentThread(), TOKEN_QUERY, TRUE, &token) != FALSE)
                    {
                        SECURITY_IMPERSONATION_LEVEL stufe {};
                        DWORD n = 0;
                        if (GetTokenInformation (token, TokenImpersonationLevel,
                                                 &stufe, sizeof (stufe), &n) != FALSE)
                            impersonation.store (static_cast<int> (stufe));
                        CloseHandle (token);
                    }
                    RevertToSelf();
                }
            }
            else
            {
                schreibeFrame (server,
                    R"({"type":"welcome","protocol_version":2,"broker_version":"test","session_token":"tok"})");
                liesFrame (server, frame); // erster Heartbeat; ACK absichtlich nie senden
            }
            blockiert.store (true);
            while (! freigeben.load())
                std::this_thread::sleep_for (std::chrono::milliseconds (5));
        }
        pipeSchliessen (server);
    });

    auto c = client (name, [] { return hello ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"); });
    c->start();
    const bool kamAn = warteAuf (2000, [&] { return blockiert.load(); });
    const auto anfang = std::chrono::steady_clock::now();
    c->stop();
    const auto dauer = std::chrono::duration_cast<std::chrono::milliseconds> (
        std::chrono::steady_clock::now() - anfang).count();
    pruefe (kamAn && dauer < 1000,
            bisAck ? "stop() bricht blockiertes ACK-Lesen begrenzt ab"
                   : "stop() bricht blockiertes Welcome-Lesen begrenzt ab",
            juce::String ((juce::int64) dauer) + " ms");
    if (! bisAck)
        pruefe (impersonation.load() == static_cast<int> (SecurityIdentification),
                "SQOS begrenzt den Peer auf SecurityIdentification",
                juce::String (impersonation.load()));
    freigeben.store (true);
    peer.join();
}

void reconnectGeneration()
{
    const auto name = testName ("reconnect");
    const auto erster = pipeAnlegen (name);
    pruefe (erster != INVALID_HANDLE_VALUE, "Reconnect-Fake-Pipe angelegt");
    if (erster == INVALID_HANDLE_VALUE)
        return;

    std::atomic<bool> helloA { false }, umschalten { false }, helloB { false };
    std::string zweitesHello;
    std::thread peer ([&]
    {
        std::string frame;
        if (verbinden (erster) && liesFrame (erster, frame))
        {
            helloA.store (frame.find ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa") != std::string::npos);
            while (! umschalten.load())
                std::this_thread::sleep_for (std::chrono::milliseconds (5));
        }
        pipeSchliessen (erster);

        const auto zweiter = pipeAnlegen (name, false);
        if (zweiter != INVALID_HANDLE_VALUE && verbinden (zweiter) && liesFrame (zweiter, zweitesHello))
        {
            helloB.store (zweitesHello.find ("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb") != std::string::npos);
            schreibeFrame (zweiter,
                R"({"type":"welcome","protocol_version":2,"broker_version":"test","session_token":"tok"})");
            if (liesFrame (zweiter, frame))
                schreibeFrame (zweiter, R"({"type":"heartbeat_ack","seq":0,"konflikt":false})");
        }
        if (zweiter != INVALID_HANDLE_VALUE)
            pipeSchliessen (zweiter);
    });

    std::mutex infoMutex;
    auto info = hello ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    auto c = client (name, [&]
    {
        std::lock_guard<std::mutex> l (infoMutex);
        return info;
    });
    c->start();
    const bool ersterKam = warteAuf (2000, [&] { return helloA.load(); });
    {
        std::lock_guard<std::mutex> l (infoMutex);
        info.sensorId = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    }
    c->reconnect();
    umschalten.store (true);
    const bool zweiterKam = warteAuf (3000, [&] { return helloB.load(); });
    pruefe (ersterKam && zweiterKam,
            "Reconnect waehrend Welcome verliert keine Konfigurationsgeneration");
    c->stop();
    peer.join();
}

void ungueltigePeerBytes (bool nul)
{
    const auto name = testName (nul ? "nul" : "utf8");
    const auto server = pipeAnlegen (name);
    if (server == INVALID_HANDLE_VALUE)
    {
        pruefe (false, "Malformed-Fake-Pipe angelegt");
        return;
    }
    std::atomic<bool> gesendet { false }, freigeben { false };
    std::thread peer ([&]
    {
        std::string frame;
        if (verbinden (server) && liesFrame (server, frame))
        {
            const unsigned char utf8[] = { 0xc3, 0x28 };
            const unsigned char rohNul[] = { '{', '}', 0, 'x' };
            if (nul) schreibeRohframe (server, rohNul, sizeof (rohNul));
            else     schreibeRohframe (server, utf8, sizeof (utf8));
            gesendet.store (true);
            while (! freigeben.load())
                std::this_thread::sleep_for (std::chrono::milliseconds (5));
        }
        pipeSchliessen (server);
    });

    auto c = client (name, [] { return hello ("cccccccccccccccccccccccccccccccc"); });
    c->start();
    const bool abgelehnt = warteAuf (1000, [&]
    {
        const auto s = c->snapshot();
        return gesendet.load() && s.status == eqcop::PipeClient::Status::getrennt
            && s.letzterFehler.contains (nul ? "NUL" : "UTF-8");
    });
    pruefe (abgelehnt,
            nul ? "rohes NUL wird vor der JUCE-Stringkonstruktion abgelehnt"
                : "ungueltiges UTF-8 wird vor der JUCE-Stringkonstruktion abgelehnt");
    c->stop();
    freigeben.store (true);
    peer.join();
}

void falschesAck()
{
    const auto name = testName ("ack-seq");
    const auto server = pipeAnlegen (name);
    if (server == INVALID_HANDLE_VALUE)
    {
        pruefe (false, "ACK-Fake-Pipe angelegt");
        return;
    }
    std::atomic<bool> gesendet { false }, freigeben { false };
    std::thread peer ([&]
    {
        std::string frame;
        if (verbinden (server) && liesFrame (server, frame))
        {
            schreibeFrame (server,
                R"({"type":"welcome","protocol_version":2,"broker_version":"test","session_token":"tok"})");
            if (liesFrame (server, frame))
            {
                schreibeFrame (server, R"({"type":"heartbeat_ack","seq":1,"konflikt":false})");
                gesendet.store (true);
            }
            while (! freigeben.load())
                std::this_thread::sleep_for (std::chrono::milliseconds (5));
        }
        pipeSchliessen (server);
    });

    auto c = client (name, [] { return hello ("dddddddddddddddddddddddddddddddd"); });
    c->start();
    const bool fiel = warteAuf (1000, [&]
    {
        const auto s = c->snapshot();
        return gesendet.load() && s.status == eqcop::PipeClient::Status::getrennt
            && s.letzterFehler.contains ("heartbeat_ack")
            && s.heartbeatsBestaetigt == 0;
    });
    pruefe (fiel, "ACK mit falscher Sequenz wird nicht als Bestaetigung gezaehlt");
    c->stop();
    freigeben.store (true);
    peer.join();
}

void schemafremdesZusatzfeld (bool imWelcome)
{
    const auto name = testName (imWelcome ? "welcome-extra" : "ack-extra");
    const auto server = pipeAnlegen (name);
    if (server == INVALID_HANDLE_VALUE)
    {
        pruefe (false, "Zusatzfeld-Fake-Pipe angelegt");
        return;
    }
    std::atomic<bool> gesendet { false }, freigeben { false };
    std::thread peer ([&]
    {
        std::string frame;
        if (verbinden (server) && liesFrame (server, frame))
        {
            if (imWelcome)
            {
                schreibeFrame (server,
                    R"({"type":"welcome","protocol_version":2,"broker_version":"test","session_token":"tok","extra":true})");
                gesendet.store (true);
            }
            else
            {
                schreibeFrame (server,
                    R"({"type":"welcome","protocol_version":2,"broker_version":"test","session_token":"tok"})");
                if (liesFrame (server, frame))
                {
                    schreibeFrame (server,
                        R"({"type":"heartbeat_ack","seq":0,"konflikt":false,"extra":true})");
                    gesendet.store (true);
                }
            }
            while (! freigeben.load())
                std::this_thread::sleep_for (std::chrono::milliseconds (5));
        }
        pipeSchliessen (server);
    });

    auto c = client (name, [] { return hello ("ffffffffffffffffffffffffffffffff"); });
    c->start();
    const bool fiel = warteAuf (1000, [&]
    {
        const auto s = c->snapshot();
        return gesendet.load() && s.status == eqcop::PipeClient::Status::getrennt
            && s.letzterFehler.contains (imWelcome ? "Antwort auf hello" : "heartbeat_ack");
    });
    pruefe (fiel,
            imWelcome ? "Welcome mit schemafremdem Zusatzfeld wird abgelehnt"
                      : "ACK mit schemafremdem Zusatzfeld wird abgelehnt");
    c->stop();
    freigeben.store (true);
    peer.join();
}

void eineDeadlineFuerDenGanzenFrame()
{
    const auto name = testName ("frame-deadline");
    const auto server = pipeAnlegen (name);
    if (server == INVALID_HANDLE_VALUE)
    {
        pruefe (false, "Deadline-Fake-Pipe angelegt");
        return;
    }

    std::atomic<bool> kopfVollstaendig { false };
    std::thread peer ([&]
    {
        std::string helloFrame;
        if (verbinden (server) && liesFrame (server, helloFrame))
        {
            const std::string welcome =
                R"({"type":"welcome","protocol_version":2,"broker_version":"test","session_token":"tok"})";
            const auto n = static_cast<DWORD> (welcome.size());
            const unsigned char kopf[4] = {
                static_cast<unsigned char> (n & 0xff),
                static_cast<unsigned char> ((n >> 8) & 0xff),
                static_cast<unsigned char> ((n >> 16) & 0xff),
                static_cast<unsigned char> ((n >> 24) & 0xff)
            };

            // Jeder einzelne Transfer bleibt deutlich unter 320 ms. Erst die
            // Summe aus vier Kopf-Teilstuecken und Body ueberschreitet das
            // Framebudget: ein pro Read/Phase neu gestartetes Budget bestaende.
            bool ok = true;
            for (const auto byte : kopf)
            {
                std::this_thread::sleep_for (std::chrono::milliseconds (60));
                ok = ok && schreibeGenau (server, &byte, 1);
                if (! ok)
                    break;
            }
            kopfVollstaendig.store (ok);
            if (ok)
            {
                std::this_thread::sleep_for (std::chrono::milliseconds (120));
                schreibeGenau (server, welcome.data(), n);
            }
        }
        pipeSchliessen (server);
    });

    auto c = client (name, [] { return hello ("99999999999999999999999999999999"); },
                     std::chrono::milliseconds { 320 });
    const auto anfang = std::chrono::steady_clock::now();
    c->start();
    const bool abgelaufen = warteAuf (900, [&]
    {
        const auto s = c->snapshot();
        return kopfVollstaendig.load()
            && s.status == eqcop::PipeClient::Status::getrennt
            && s.letzterFehler.contains ("Zeitlimit");
    });
    const auto dauer = std::chrono::duration_cast<std::chrono::milliseconds> (
        std::chrono::steady_clock::now() - anfang).count();
    c->stop();
    peer.join();

    pruefe (abgelaufen && dauer < 700,
            "ein absolutes Zeitbudget gilt fuer Kopf, Body und Teiltransfers",
            juce::String ((juce::int64) dauer) + " ms");
}

void dropzaehlerBleibtWireI64()
{
    const auto name = testName ("drop-i64");
    const auto server = pipeAnlegen (name);
    if (server == INVALID_HANDLE_VALUE)
    {
        pruefe (false, "Dropzaehler-Fake-Pipe angelegt");
        return;
    }

    std::atomic<bool> korrekt { false };
    std::thread peer ([&]
    {
        std::string frame;
        if (verbinden (server) && liesFrame (server, frame)
            && schreibeFrame (server,
                R"({"type":"welcome","protocol_version":2,"broker_version":"test","session_token":"tok"})")
            && liesFrame (server, frame))
        {
            const auto json = juce::JSON::parse (
                juce::String::fromUTF8 (frame.data(), static_cast<int> (frame.size())));
            const auto wert = json.getProperty ("stats", {})
                                  .getProperty ("frames_dropped", {});
            korrekt.store (wert.isInt64()
                && static_cast<juce::int64> (wert)
                    == std::numeric_limits<juce::int64>::max());
            schreibeFrame (server, R"({"type":"heartbeat_ack","seq":0,"konflikt":false})");
        }
        pipeSchliessen (server);
    });

    auto stats = []
    {
        eqcop::StatsSnapshot s;
        s.framesDropped = std::numeric_limits<juce::uint64>::max();
        return s;
    };
    eqcop::PipeClient c ([] { return hello ("88888888888888888888888888888888"); },
                         stats, {}, name);
    c.start();
    const bool kamAn = warteAuf (1500, [&] { return korrekt.load(); });
    c.stop();
    peer.join();
    pruefe (kamAn,
            "u64-Dropzaehler saettigt auf die gemeinsame i64-Wire-Grenze");
}

void startStopSerialisiert()
{
    const auto name = testName ("api-race"); // absichtlich ohne Server
    auto c = client (name, [] { return hello ("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"); });
    for (int i = 0; i < 200; ++i)
    {
        std::thread starter ([&] { c->start(); });
        std::thread stopper ([&] { c->stop(); });
        starter.join();
        stopper.join();
        c->stop();
    }
    pruefe (true, "200 konkurrierende start/stop-Paare ohne joinable-Leak");
}
} // namespace

int main()
{
    std::cout << "PIPECLIENT-LIFECYCLE-TEST" << std::endl;
    stoppFall (false);
    stoppFall (true);
    reconnectGeneration();
    ungueltigePeerBytes (false);
    ungueltigePeerBytes (true);
    falschesAck();
    schemafremdesZusatzfeld (true);
    schemafremdesZusatzfeld (false);
    eineDeadlineFuerDenGanzenFrame();
    dropzaehlerBleibtWireI64();
    startStopSerialisiert();
    std::cout << (fehler == 0 ? "PIPECLIENT-LIFECYCLE-TEST OK - "
                              : "PIPECLIENT-LIFECYCLE-TEST FEHLGESCHLAGEN - ")
              << fehler << " Fehler" << std::endl;
    return fehler == 0 ? 0 : 1;
}
