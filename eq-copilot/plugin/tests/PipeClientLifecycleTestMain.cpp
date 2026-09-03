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
#include <vector>

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
    std::chrono::milliseconds timeout = std::chrono::milliseconds { 5000 },
    nakama::ipc::ServerErwartung serverErwartung
        = nakama::ipc::serverErwartungFuerEigenprozessTest())
{
    return std::make_unique<eqcop::PipeClient> (
        std::move (provider), [] { return eqcop::StatsSnapshot {}; },
        std::function<eqcop::MessKompakt()> {}, name, timeout,
        std::move (serverErwartung));
}

void pipeSchliessen (HANDLE h)
{
    DisconnectNamedPipe (h);
    CloseHandle (h);
}

/// Loest ein wartendes `ConnectNamedPipe` von aussen aus. Ohne diesen Weg
/// wuerde ein Peer-Thread, auf dessen Verbindung der Test nicht mehr wartet,
/// den Prozess beim `join()` blockieren.
void wecken (const juce::String& name)
{
    HANDLE h = CreateFileW (name.toWideCharPointer(), GENERIC_READ, 0, nullptr,
                            OPEN_EXISTING,
                            SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION,
                            nullptr);
    if (h != INVALID_HANDLE_VALUE)
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
    const bool zweimalAuthentisiert = c->snapshot().serverPruefungen >= 2;
    pruefe (ersterKam && zweiterKam,
            "Reconnect waehrend Welcome verliert keine Konfigurationsgeneration");
    pruefe (ersterKam && zweiterKam && zweimalAuthentisiert,
            "pipeclient_reconnect_verwirft_serverfreigabe");
    c->stop();
    // Ohne diesen Weckruf haengt der Peer nach einem ROTEN Lauf fuer immer in
    // `ConnectNamedPipe (zweiter)`, weil der gestoppte Client nicht mehr
    // verbindet. Eine Pruefung, die statt rot zu werden den Kanon blockiert,
    // ist keine Pruefung (Prueflistenzeile E-5).
    wecken (name);
    peer.join();
}

void pipeclient_sendet_vor_serverauth_keine_bytes()
{
    const auto name = testName ("serverauth-null-bytes");
    const auto server = pipeAnlegen (name);
    if (server == INVALID_HANDLE_VALUE)
    {
        pruefe (false, "pipeclient_sendet_vor_serverauth_keine_bytes");
        return;
    }
    std::atomic<bool> angenommen { false }, helloGelesen { false };
    std::thread peer ([&]
    {
        std::string frame;
        angenommen.store (verbinden (server));
        if (angenommen.load())
            helloGelesen.store (liesFrame (server, frame));
        pipeSchliessen (server);
    });

    auto erwartung = nakama::ipc::serverErwartungFuerEigenprozessTest();
    erwartung.testFehler = nakama::ipc::ServerPruefFehler::hashFalsch;
    auto c = client (name,
                     [] { return hello ("abababababababababababababababab"); },
                     std::chrono::milliseconds { 5000 }, erwartung);
    c->start();
    const bool fiel = warteAuf (3000, [&]
    {
        const auto s = c->snapshot();
        return s.serverPruefstatus
                    == nakama::ipc::ServerPruefStatus::belegtAberUnverifiziert
            && s.serverPrueffehler == nakama::ipc::ServerPruefFehler::hashFalsch
            && s.serverPruefungen == 1
            && s.heartbeatsGesendet == 0;
    });
    c->stop();
    peer.join();
    pruefe (fiel && angenommen.load() && ! helloGelesen.load(),
            "pipeclient_sendet_vor_serverauth_keine_bytes");
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

// ═════════════════════════════════════════════════════════════════════════
// NAK-134 auf dem v2-Pfad. Matrix C-06 verlangt fuer ALLE DREI Connectpfade
// dieselbe Regel; der v2-Client hat denselben Defekt und denselben Parkpfad
// (`PipeClient.cpp:305-318`, `:391-405`).
//
// Die Win32-Codes je Lage sind gemessen, nicht geraten: eine selbst besetzte
// Einzelinstanz gibt 231 (PIPE_BUSY), eine `PIPE_ACCESS_INBOUND`-Pipe gibt 5
// (ACCESS_DENIED), ein fehlender Name gibt 2, und `<name>\..\..` gibt 123 —
// der einzige an `CreateFileW` erreichbare "sonstige" Fehler.

/// Haelt genau EINE Instanz und besetzt sie selbst ⇒ jeder weitere
/// `CreateFileW` bekommt `ERROR_PIPE_BUSY` und `WaitNamedPipeW` laeuft leer.
struct BelegtePipe
{
    HANDLE server = INVALID_HANDLE_VALUE, besetzer = INVALID_HANDLE_VALUE;
    bool anlegen (const juce::String& name)
    {
        server = pipeAnlegen (name);
        if (server == INVALID_HANDLE_VALUE)
            return false;
        besetzer = CreateFileW (name.toWideCharPointer(), GENERIC_READ | GENERIC_WRITE,
                                0, nullptr, OPEN_EXISTING,
                                SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION, nullptr);
        return besetzer != INVALID_HANDLE_VALUE;
    }
    ~BelegtePipe()
    {
        if (besetzer != INVALID_HANDLE_VALUE) CloseHandle (besetzer);
        if (server != INVALID_HANDLE_VALUE) pipeSchliessen (server);
    }
};

/// Nur EINGEHEND ⇒ der Client mit GENERIC_READ|GENERIC_WRITE bekommt
/// `ERROR_ACCESS_DENIED`: der Fall "fremder Besitzer des Namens".
struct FremdePipe
{
    HANDLE server = INVALID_HANDLE_VALUE;
    bool anlegen (const juce::String& name)
    {
        server = CreateNamedPipeW (
            name.toWideCharPointer(), PIPE_ACCESS_INBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE,
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

int weitereVersucheIn (eqcop::PipeClient& c, int fensterMs)
{
    const int vorher = c.snapshot().verbindungsVersuche;
    std::this_thread::sleep_for (std::chrono::milliseconds (fensterMs));
    return c.snapshot().verbindungsVersuche - vorher;
}

std::vector<long long> versuchsStempel (eqcop::PipeClient& c, int anzahl, int deckelMs)
{
    std::vector<long long> stempel;
    const auto start = std::chrono::steady_clock::now();
    int stand = c.snapshot().verbindungsVersuche;
    while ((int) stempel.size() < anzahl)
    {
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds> (
            std::chrono::steady_clock::now() - start).count();
        if (ms > deckelMs)
        {
            while ((int) stempel.size() < anzahl) stempel.push_back (-1);
            break;
        }
        const int neu = c.snapshot().verbindungsVersuche;
        if (neu > stand)
        {
            for (int i = 0; i < neu - stand && (int) stempel.size() < anzahl; ++i)
                stempel.push_back (ms);
            stand = neu;
            continue;
        }
        std::this_thread::sleep_for (std::chrono::milliseconds (10));
    }
    return stempel;
}

/// D-K03, D-K06, D-K17, D-K20, D-K23, D-K26 auf dem v2-Pfad.
void pipeclient_oeffnungsausgaenge_sind_liveness_oder_sicherheit()
{
    struct Fall
    {
        const char* name;
        nakama::ipc::ServerPruefStatus status;
        nakama::ipc::ServerPruefFehler fehler;
        bool parkt;
    };
    using S = nakama::ipc::ServerPruefStatus;
    using F = nakama::ipc::ServerPruefFehler;
    const std::vector<Fall> faelle {
        { "pipe_busy",        S::belegtNichtErreicht,     F::pipeBelegt,   false },
        { "file_not_found",   S::nichtDa,                 F::pipeFehlt,    false },
        { "access_denied",    S::belegtAberUnverifiziert, F::pipeOeffnen,  true  },
        { "sonstiger_fehler", S::belegtAberUnverifiziert, F::pipeOeffnen,  true  },
    };
    for (const auto& f : faelle)
    {
        const juce::String fall (f.name);
        BelegtePipe belegt;
        FremdePipe fremd;
        juce::String name;
        bool aufbau = true;
        if (fall == "pipe_busy")            { name = testName ("v2-busy");   aufbau = belegt.anlegen (name); }
        else if (fall == "access_denied")   { name = testName ("v2-denied"); aufbau = fremd.anlegen (name); }
        else if (fall == "sonstiger_fehler") name = testName ("v2-invalid") + "\\..\\..";
        else                                 name = testName ("v2-fehlt");

        const auto t0 = std::chrono::steady_clock::now();
        auto c = client (name, [] { return hello ("11111111111111111111111111111111"); });
        c->start();
        const bool klassifiziert = warteAuf (9000, [&]
        {
            const auto s = c->snapshot();
            return s.serverPruefstatus == f.status && s.serverPrueffehler == f.fehler;
        });
        const auto dauerMs = std::chrono::duration_cast<std::chrono::milliseconds> (
            std::chrono::steady_clock::now() - t0).count();
        const int weitere = weitereVersucheIn (*c, 1500);
        c->stop();
        const bool phase = f.parkt ? weitere == 0 : weitere >= 1;
        // Nur `pipe_busy` faehrt die volle Warteschleife (20 x 200 ms).
        const bool frist = fall != "pipe_busy" || dauerMs >= 4000;
        pruefe (aufbau && klassifiziert && phase && frist,
                "pipeclient_oeffnungsausgaenge_sind_liveness_oder_sicherheit",
                fall + ": klass " + juce::String ((int) klassifiziert) + ", "
                    + juce::String ((juce::int64) dauerMs) + " ms, weitere Versuche "
                    + juce::String (weitere) + " (erwartet "
                    + (f.parkt ? "0" : ">=1") + ")");
    }
}

/// E-Z03, E-Z05 — die beiden v2-Zaehlervertraege. `verbindungsVersuche` +1 bei
/// JEDEM Eintritt; `serverPruefungen` POSITIONSgesteuert, also nur nach einem
/// erfolgreichen `CreateFileW`. Ein Oeffnungsfehler erreicht die Stelle nie —
/// das ist die Asymmetrie zu v3 (offener Punkt, ausserhalb von R1).
void pipeclient_zaehlervertrag()
{
    // `pipe_busy`: kein Handle ⇒ keine Serverpruefung, aber Versuche steigen.
    {
        const auto name = testName ("v2-zaehler-busy");
        BelegtePipe belegt;
        const bool aufbau = belegt.anlegen (name);
        auto c = client (name, [] { return hello ("22222222222222222222222222222222"); });
        c->start();
        const bool zwei = warteAuf (20000, [&] {
            return c->snapshot().verbindungsVersuche >= 2;
        });
        const auto s = c->snapshot();
        c->stop();
        pruefe (aufbau && zwei && s.serverPruefungen == 0,
                "pipeclient_zaehlervertrag",
                "pipe_busy: " + juce::String (s.verbindungsVersuche) + " Versuche, "
                    + juce::String ((juce::int64) s.serverPruefungen)
                    + " Serverpruefungen (erwartet 0)");
    }
    // `access_denied`: ebenfalls kein Handle — auf dem v2-Pfad zaehlt der
    // Oeffnungsfehler deshalb NICHT als Serverpruefung, anders als bei v3.
    {
        const auto name = testName ("v2-zaehler-denied");
        FremdePipe fremd;
        const bool aufbau = fremd.anlegen (name);
        auto c = client (name, [] { return hello ("33333333333333333333333333333333"); });
        c->start();
        const bool fiel = warteAuf (4000, [&] {
            return c->snapshot().serverPruefstatus
                == nakama::ipc::ServerPruefStatus::belegtAberUnverifiziert;
        });
        const auto s = c->snapshot();
        c->stop();
        pruefe (aufbau && fiel && s.serverPruefungen == 0 && s.verbindungsVersuche == 1,
                "pipeclient_zaehlervertrag",
                "access_denied: " + juce::String (s.verbindungsVersuche) + " Versuche, "
                    + juce::String ((juce::int64) s.serverPruefungen)
                    + " Serverpruefungen (erwartet 0 — positionsgesteuert)");
    }
}

/// Obere Schranke der R5-Zusage: Restzeit einer laufenden
/// `WaitNamedPipeW(200 ms)` plus hoechstens eine Runde. Nominal rund 400 ms;
/// 1.500 ms lassen Scheduling zu und liegen weit unter den >= 4.000 ms der
/// vollen Warteschleife — die Schranke TRENNT also "Abbruch hat gegriffen" von
/// "Schleife lief zu Ende".
constexpr int kR5FristMs = 1500;

/// Ein v2-Peer, der annimmt, das Hello liest und — je nach Schalter — ein
/// welcome schickt oder eben nicht. Er nimmt WIEDERHOLT an, damit sich auch
/// eine Backoff-Folge ueber mehrere Runden messen laesst.
///
/// `welcome == false` ist die Lage D-K06/D-P02 auf dem v2-Pfad: die
/// Serverpruefung ist bestanden und das Hello gelesen, aber es kommt keine
/// Antwort. Die Verbindung bleibt offen, damit der Client wirklich in seine
/// Lesefrist laeuft statt ein Ende zu sehen.
struct V2Peer
{
    HANDLE server = INVALID_HANDLE_VALUE;
    std::thread thread;
    std::atomic<bool> laeuft { true };
    std::atomic<bool> angenommen { false }, helloGelesen { false };
    std::atomic<int>  welcomes { 0 }, verbindungen { 0 };
    juce::String name;

    bool starten (const juce::String& pipeName, bool welcome)
    {
        name = pipeName;
        server = pipeAnlegen (pipeName);
        if (server == INVALID_HANDLE_VALUE)
            return false;
        thread = std::thread ([this, welcome]
        {
            std::string frame;
            while (laeuft.load())
            {
                if (! verbinden (server))
                    break;
                angenommen.store (true);
                ++verbindungen;
                if (liesFrame (server, frame))
                {
                    helloGelesen.store (true);
                    if (welcome)
                    {
                        if (schreibeFrame (server,
                                R"({"type":"welcome","protocol_version":2,"broker_version":"test","session_token":"tok"})"))
                            ++welcomes;
                        while (laeuft.load() && liesFrame (server, frame))
                            schreibeFrame (server,
                                R"({"type":"heartbeat_ack","seq":0,"konflikt":false})");
                    }
                    else
                    {
                        // NICHTS antworten. Der Client laeuft in seine
                        // Lesefrist und schliesst; erst dann faellt `liesFrame`.
                        while (laeuft.load() && liesFrame (server, frame)) {}
                    }
                }
                DisconnectNamedPipe (server);
            }
            pipeSchliessen (server);
        });
        return true;
    }

    /// Beendet den Peer und damit eine stehende Verbindung. Idempotent.
    void stoppen()
    {
        if (! laeuft.exchange (false))
        {
            if (thread.joinable()) thread.join();
            return;
        }
        if (thread.joinable())
        {
            wecken (name);   // loest ein wartendes `ConnectNamedPipe`
            thread.join();
        }
    }
    ~V2Peer() { stoppen(); }
};

/// D-K06, D-K10, D-K11 auf dem v2-Pfad.
///
/// **Nacharbeit Runde 1, Defekt 3.** Die drei Zeilen waren als „gemessen"
/// gefuehrt, ohne dass der v2-Test sie je angefahren haette:
/// `welcome_bleibt_aus` fehlte ganz, und die beiden Authausgaenge
/// `erwartung_ungueltig` und `server_pid_nicht_ermittelbar` gab es nur auf dem
/// Control-Pfad.
void pipeclient_authausgaenge_und_welcome_ausbleiben()
{
    // ── D-K06: verifiziert, Hello gelesen — aber kein welcome ────────────
    {
        const auto name = testName ("v2-welcome-aus");
        V2Peer peer;
        const bool aufbau = peer.starten (name, false);
        auto c = client (name, [] { return hello ("99999999999999999999999999999999"); });
        c->start();
        const bool gelesen = warteAuf (8000, [&] { return peer.helloGelesen.load(); });
        // Das ENDE der Runde abwarten: `status == getrennt` bei bereits
        // gezaehlter Serverpruefung. Erst dort steht der Aufraeumzustand aus
        // `PipeClient.cpp:625-635`, und die naechste Runde hat noch nichts
        // ueberschrieben.
        const bool rundeEnde = warteAuf (12000, [&] {
            const auto z = c->snapshot();
            return z.serverPruefungen >= 1
                && z.status == eqcop::PipeClient::Status::getrennt;
        });
        const auto s = c->snapshot();
        c->stop();
        peer.stoppen();
        pruefe (aufbau && gelesen && rundeEnde && s.protokollVersion == 0
                    && s.serverPruefungen == 1
                    && s.serverPruefstatus == nakama::ipc::ServerPruefStatus::nichtGeprueft
                    && s.serverPrueffehler == nakama::ipc::ServerPruefFehler::keiner,
                "pipeclient_authausgaenge_und_welcome_ausbleiben",
                "welcome_bleibt_aus: Hello gelesen " + juce::String ((int) gelesen)
                    + ", Pruefungen " + juce::String ((juce::int64) s.serverPruefungen)
                    + ", Protokoll " + juce::String (s.protokollVersion)
                    + " (erwartet 0), Status " + juce::String ((int) s.serverPruefstatus)
                    + " (erwartet nichtGeprueft)");
    }

    // ── D-K10, D-K11: beide bleiben Sicherheitsfaelle und parken ──────────
    struct Fall { const char* name; nakama::ipc::ServerPruefFehler fehler; bool ueberErwartung; };
    for (const auto& f : std::vector<Fall> {
             { "erwartung_ungueltig", nakama::ipc::ServerPruefFehler::erwartungUngueltig, true },
             { "server_pid_nicht_ermittelbar",
               nakama::ipc::ServerPruefFehler::serverPidNichtErmittelbar, false } })
    {
        const auto name = testName (f.name);
        V2Peer peer;
        const bool aufbau = peer.starten (name, false);
        // Eine LEERE Erwartung ist der Fall `erwartungUngueltig` selbst; der
        // zweite Ausgang laeuft ueber die vorhandene Injektion.
        auto erwartung = f.ueberErwartung
            ? nakama::ipc::ServerErwartung {}
            : nakama::ipc::serverErwartungFuerEigenprozessTest();
        if (! f.ueberErwartung)
            erwartung.testFehler = f.fehler;
        auto c = client (name, [] { return hello ("aaaaaaaabbbbbbbbccccccccdddddddd"); },
                         std::chrono::milliseconds { 5000 }, erwartung);
        c->start();
        const bool fiel = warteAuf (5000, [&] {
            const auto z = c->snapshot();
            return z.serverPruefstatus
                       == nakama::ipc::ServerPruefStatus::belegtAberUnverifiziert
                && z.serverPrueffehler == f.fehler;
        });
        const int weitere = weitereVersucheIn (*c, 1500);
        const auto s = c->snapshot();
        c->stop();
        peer.stoppen();
        pruefe (aufbau && fiel && weitere == 0 && ! peer.helloGelesen.load()
                    && s.serverPruefungen == 1,
                "pipeclient_authausgaenge_und_welcome_ausbleiben",
                juce::String (f.name) + ": fiel " + juce::String ((int) fiel)
                    + ", weitere Versuche " + juce::String (weitere)
                    + ", Hello gelesen " + juce::String ((int) peer.helloGelesen.load())
                    + " (erwartet 0), Pruefungen "
                    + juce::String ((juce::int64) s.serverPruefungen));
    }
}

/// D-P01, D-P02, D-P05, D-P06 auf dem v2-Pfad, ueber Versuchsstempel (W-H3).
/// Die volle Folge bis zum Deckel kostet 23,5 s reine Wartezeit und wird auf
/// dem v3-Zwilling gefahren; hier werden die ersten Verdopplungen gemessen.
void pipeclient_backoff_folge_und_deckel_sind_beobachtbar()
{
    // `file_not_found`: 500 → 1.000 → 2.000.
    {
        const auto name = testName ("v2-backoff-fehlt");
        auto c = client (name, [] { return hello ("44444444444444444444444444444444"); });
        c->start();
        warteAuf (2000, [&] { return c->snapshot().verbindungsVersuche >= 1; });
        const auto s = versuchsStempel (*c, 3, 15000);
        c->stop();
        bool alleDa = true;
        for (auto v : s) alleDa = alleDa && v >= 0;
        // Toleranz nach oben, nicht nach unten — Begruendung wie im
        // v3-Zwilling: `wait_for` kehrt nie frueher zurueck.
        auto imFenster = [] (long long ist, long long soll) {
            return ist >= soll * 9 / 10 && ist <= soll * 7 / 5 + 200;
        };
        const bool folge = alleDa && imFenster (s[0], 500)
                        && imFenster (s[1] - s[0], 1000) && imFenster (s[2] - s[1], 2000);
        pruefe (folge, "pipeclient_backoff_folge_und_deckel_sind_beobachtbar",
                "file_not_found Abstaende " + (alleDa
                    ? juce::String ((juce::int64) s[0]) + " "
                        + juce::String ((juce::int64) (s[1] - s[0])) + " "
                        + juce::String ((juce::int64) (s[2] - s[1]))
                    : juce::String ("unvollstaendig")) + " ms (erwartet 500 1000 2000)");
    }
    // `pipe_busy`: der BELEG. Vor R1 gibt es hier keinen zweiten Versuch.
    {
        const auto name = testName ("v2-backoff-busy");
        BelegtePipe belegt;
        const bool aufbau = belegt.anlegen (name);
        auto c = client (name, [] { return hello ("55555555555555555555555555555555"); });
        c->start();
        const auto s = versuchsStempel (*c, 3, 30000);
        c->stop();
        bool alleDa = true;
        for (auto v : s) alleDa = alleDa && v >= 0;
        const bool folge = alleDa && (s[1] - s[0]) >= 4000 && (s[2] - s[1]) >= 4000
                        && (s[2] - s[1]) > (s[1] - s[0]);
        pruefe (aufbau && folge, "pipeclient_backoff_folge_und_deckel_sind_beobachtbar",
                "pipe_busy Stempel " + (alleDa
                    ? juce::String ((juce::int64) s[0]) + " "
                        + juce::String ((juce::int64) s[1]) + " "
                        + juce::String ((juce::int64) s[2])
                    : juce::String ("unvollstaendig"))
                    + " ms — vor R1 parkt der Thread nach dem ersten");
    }
    // `verifiziert_ohne_welcome` (D-P02): der Server authentisiert und liest
    // das Hello, antwortet aber nie. `eineVerbindung` gibt `false`
    // (`PipeClient.cpp:637` liefert `welcomeKam`), der Backoff faellt also
    // NICHT zurueck, sondern verdoppelt weiter.
    {
        const auto name = testName ("v2-backoff-welcome");
        V2Peer peer;
        const bool aufbau = peer.starten (name, false);
        auto c = client (name, [] { return hello ("77777777777777777777777777777777"); });
        c->start();
        const auto s = versuchsStempel (*c, 3, 40000);
        const auto letzter = c->snapshot();
        c->stop();
        peer.stoppen();
        bool alleDa = true;
        for (auto v : s) alleDa = alleDa && v >= 0;
        // Waere die Ruecksetzung an den Auth-Erfolg gebunden statt an die
        // Rueckgabe, blieben die Abstaende gleich.
        const bool waechst = alleDa && (s[2] - s[1]) > (s[1] - s[0]);
        pruefe (aufbau && waechst && letzter.serverPruefungen >= 2,
                "pipeclient_backoff_folge_und_deckel_sind_beobachtbar",
                "verifiziert_ohne_welcome Stempel " + (alleDa
                    ? juce::String ((juce::int64) s[0]) + " "
                        + juce::String ((juce::int64) s[1]) + " "
                        + juce::String ((juce::int64) s[2])
                    : juce::String ("unvollstaendig"))
                    + " ms, Pruefungen "
                    + juce::String ((juce::int64) letzter.serverPruefungen));
    }
    // `stehende_verbindung_setzt_zurueck` (D-P01): eine Verbindung STAND und
    // endete regulaer ⇒ der Backoff faellt aus einer ERHOEHTEN Stufe auf
    // 500 ms zurueck.
    //
    // Nacharbeit Runde 1, Defekt 3: diese Zeile war als „gemessen" gefuehrt,
    // ohne dass ein v2-Fall je eine stehende Verbindung hergestellt haette.
    // Der Aufstieg VOR der Verbindung ist der Kern: bei 500 ms Ausgangswert
    // sind „zurueckgesetzt" und „nicht zurueckgesetzt" nicht unterscheidbar.
    {
        const auto name = testName ("v2-backoff-reset");
        auto c = client (name, [] { return hello ("88888888888888888888888888888888"); });
        c->start();
        // 1) Aufstieg ohne Peer: Abstaende 500 und 1.000 ⇒ `backoffMs` steht
        //    danach auf 2.000.
        const auto aufstieg = versuchsStempel (*c, 3, 12000);
        bool alleDa = true;
        for (auto v : aufstieg) alleDa = alleDa && v >= 0;
        const bool gestiegen = alleDa && (aufstieg[2] - aufstieg[1]) >= 900;

        // 2) Peer an — die naechste Runde verbindet wirklich.
        V2Peer peer;
        const bool aufbau = peer.starten (name, true);
        const bool stand = warteAuf (20000, [&] {
            return c->snapshot().status == eqcop::PipeClient::Status::verbunden;
        });

        // 3) Verbindung regulaer beenden und messen, WANN die naechste Runde
        //    beginnt. 500 ms nominal; ein stehengebliebener Backoff koennte
        //    1.200 ms nicht unterbieten.
        //
        //    Der Anker ist der Moment, in dem der Client das Ende SIEHT, nicht
        //    der, in dem der Peer schliesst: der v2-Client merkt einen
        //    Abbruch erst an seinem naechsten Heartbeat-Takt
        //    (`kHeartbeatMs` = 1.000 ms, `EqCopilotIds.h:23`). Ab
        //    `peer.stoppen()` gemessen ergaeben sich 1.500 ms — das waere die
        //    Erkennungslatenz PLUS Backoff und nicht die Groesse, die D-P01
        //    behauptet.
        const int vorEnde = c->snapshot().verbindungsVersuche;
        peer.stoppen();
        const bool endeGesehen = warteAuf (5000, [&] {
            return c->snapshot().status != eqcop::PipeClient::Status::verbunden;
        });
        const auto t0 = std::chrono::steady_clock::now();
        const bool neueRunde = endeGesehen && warteAuf (2500, [&] {
            return c->snapshot().verbindungsVersuche > vorEnde;
        });
        const auto verzug = std::chrono::duration_cast<std::chrono::milliseconds> (
            std::chrono::steady_clock::now() - t0).count();
        c->stop();
        pruefe (aufbau && gestiegen && stand && endeGesehen && neueRunde
                    && verzug >= 350 && verzug <= 1200,
                "pipeclient_backoff_folge_und_deckel_sind_beobachtbar",
                "stehende_verbindung_setzt_zurueck: Aufstieg " + (alleDa
                    ? juce::String ((juce::int64) (aufstieg[1] - aufstieg[0])) + " "
                        + juce::String ((juce::int64) (aufstieg[2] - aufstieg[1]))
                    : juce::String ("unvollstaendig"))
                    + " ms, Verbindung stand " + juce::String ((int) stand)
                    + ", naechste Runde nach " + juce::String ((juce::int64) verzug)
                    + " ms (erwartet 350..1200, ohne Reset >= 2000)");
    }
}

/// D-P03, D-P07 auf dem v2-Pfad: Parken uebergeht den Backoff, und das Loesen
/// setzt einen ERHOEHTEN Backoff auf 500 ms zurueck (`PipeClient.cpp:305-318`).
///
/// **Nacharbeit Runde 2:** wie der v3-Zwilling parkte dieser Fall aus
/// `backoffMs == 500` und mass nur den sofortigen Versuch nach `reconnect()`.
/// Der folgt wegen des bedingungslosen `continue` (`PipeClient.cpp:319`)
/// ohnehin, welchen Wert `backoffMs` auch traegt — ein entfernter Reset in
/// Zeile 318 blieb dabei gruen. Gemessen wird jetzt der Abstand zum ZWEITEN
/// Versuch: erst der benutzt `backoffMs`, im `wait_for` unterhalb des Zweigs.
/// Der Aufbau ist derselbe wie beim v3-Zwilling und aus denselben Gruenden:
/// erst aus einer erhoehten Stufe parken, dann die Sperrursache loesen, damit
/// der Pfad nach dem `reconnect()` nicht sofort wieder parkt.
void pipeclient_parken_uebergeht_den_backoff()
{
    for (const bool ueberAuth : { true, false })
    {
        const auto name = testName (ueberAuth ? "v2-parken-auth" : "v2-parken-denied");
        FremdePipe fremd;
        HANDLE server = INVALID_HANDLE_VALUE;
        std::atomic<bool> laeuftPeer { true };
        std::thread peer;
        bool aufbau = true;
        // Die Erwartung geht in den Konstruktor und muss deshalb JETZT stehen.
        // Die Sperrursache selbst entsteht erst nach dem Aufstieg (Schritt b).
        auto erwartung = nakama::ipc::serverErwartungFuerEigenprozessTest();
        if (ueberAuth)
            erwartung.testFehler = nakama::ipc::ServerPruefFehler::signerFalsch;

        auto c = client (name, [] { return hello ("66666666666666666666666666666666"); },
                         std::chrono::milliseconds { 5000 }, erwartung);
        c->start();

        // a) Aufstieg gegen den noch NICHT belegten Namen. Drei Versuche,
        //    Abstaende 500 und 1.000 ms ⇒ `backoffMs` steht danach auf 4.000.
        const auto aufstieg = versuchsStempel (*c, 3, 12000);
        bool alleDa = true;
        for (auto v : aufstieg) alleDa = alleDa && v >= 0;
        const bool gestiegen = alleDa && (aufstieg[2] - aufstieg[1]) >= 900;

        // b) Erst JETZT die Sperrursache. Verpasst der Aufbau das 2.000-ms-
        //    Fenster, parkt der Pfad eine Stufe spaeter — auch das bleibt
        //    >= 2.000 ms und damit diskriminierend.
        if (ueberAuth)
        {
            server = pipeAnlegen (name);
            aufbau = server != INVALID_HANDLE_VALUE;
            if (aufbau)
                peer = std::thread ([&]
                {
                    std::string frame;
                    if (verbinden (server))
                        liesFrame (server, frame);   // kommt nie: Auth faellt vorher
                    while (laeuftPeer.load())
                        std::this_thread::sleep_for (std::chrono::milliseconds (5));
                    pipeSchliessen (server);
                });
        }
        else
        {
            aufbau = fremd.anlegen (name);
        }
        const bool geparkt = warteAuf (20000, [&] {
            return c->snapshot().serverPruefstatus
                == nakama::ipc::ServerPruefStatus::belegtAberUnverifiziert;
        });
        // Nach k gescheiterten Versuchen steht `backoffMs` auf `500 * 2^k`;
        // der Parkversuch selbst aendert ihn nicht. Ab dem vierten Versuch
        // ist der stale Wert also >= 4.000 ms.
        const int versucheBeimParken = c->snapshot().verbindungsVersuche;
        const int weitere = weitereVersucheIn (*c, 1500);

        // c) Sperrursache loesen. Der naechste Versuch soll schlicht
        //    scheitern, nicht erneut in den Parkzweig laufen.
        if (ueberAuth)
        {
            laeuftPeer.store (false);
            if (peer.joinable()) { wecken (name); peer.join(); }
        }
        else
        {
            fremd.schliessen();
        }

        // 2. reconnect() loest die Sperre; der naechste Versuch kommt OHNE
        //    Wartezeit.
        const int vorReconnect = c->snapshot().verbindungsVersuche;
        const auto t0 = std::chrono::steady_clock::now();
        c->reconnect();
        const bool sofort = warteAuf (250, [&] {
            return c->snapshot().verbindungsVersuche > vorReconnect;
        });
        const auto verzug = std::chrono::duration_cast<std::chrono::milliseconds> (
            std::chrono::steady_clock::now() - t0).count();

        // 3. d) DER Reset-Beweis: der Versuch DANACH ist der erste, der
        //    `backoffMs` wirklich benutzt. 500 ms nominal, Fenster wie in
        //    D-P01. Ein stehengebliebener Backoff braeuchte >= 4.000 ms, ein
        //    faelschlich genullter kaeme unter 350 ms. Das Wartefenster ist
        //    absichtlich breiter als der Deckel, damit der Rotlauf den stalen
        //    Wert MISST statt nur ein Ausbleiben zu melden.
        const int nachSofort = c->snapshot().verbindungsVersuche;
        const auto t1 = std::chrono::steady_clock::now();
        const bool zweiter = warteAuf (9000, [&] {
            return c->snapshot().verbindungsVersuche > nachSofort;
        });
        const auto abstand = std::chrono::duration_cast<std::chrono::milliseconds> (
            std::chrono::steady_clock::now() - t1).count();

        c->stop();
        laeuftPeer.store (false);
        if (peer.joinable()) { wecken (name); peer.join(); }
        pruefe (aufbau && gestiegen && geparkt && versucheBeimParken >= 4
                    && weitere == 0 && sofort && zweiter
                    && abstand >= 350 && abstand <= 1200,
                "pipeclient_parken_uebergeht_den_backoff",
                juce::String (ueberAuth ? "authfehler" : "access_denied")
                    + ": Aufstieg " + (alleDa
                        ? juce::String ((juce::int64) (aufstieg[1] - aufstieg[0])) + " "
                            + juce::String ((juce::int64) (aufstieg[2] - aufstieg[1]))
                        : juce::String ("unvollstaendig"))
                    + " ms, geparkt " + juce::String ((int) geparkt)
                    + " bei Versuch " + juce::String (versucheBeimParken)
                    + ", weitere Versuche " + juce::String (weitere)
                    + ", Neustart nach " + juce::String ((juce::int64) verzug)
                    + " ms, Versuch DANACH nach " + juce::String ((juce::int64) abstand)
                    + " ms (erwartet 350..1200, ohne Reset >= 4000)");
    }
}

/// D-A03, D-A06, D-A09, D-A12 — Abbruch vor und nach einem erfolgreichen
/// `CreateFileW`. R5 ist eine Spaetestens-Zusage: gemessen wird der AUSGANG und
/// die OBERE Schranke `kR5FristMs`, ausdruecklich keine Mindestdauer.
///
/// **Nacharbeit Runde 1, Defekt 2.** Die Vor-Open-Faelle liefen gegen einen
/// nicht existierenden Namen — `CreateFileW` kehrte in Runde 1 mit
/// `FILE_NOT_FOUND` zurueck, die Wartephase wurde nie beruehrt — und der
/// `stop()`-Fall mass `dauer < 9000 ms`, also gar nicht die R5-Frist, sondern
/// nur „nicht ewig". Beides ist hier nachgezogen: belegte Pipe, Abbruch mitten
/// in der 20 x 200 ms-Warteschleife, Schranke `kR5FristMs`.
void pipeclient_abbruch_vor_und_nach_createfile()
{
    // ── vor dem Oeffnungserfolg: BELEGTE Pipe, Abbruch in der Wartephase ──
    for (const bool ueberStop : { true, false })
    {
        const auto name = testName (ueberStop ? "v2-abbruch-vor-stop"
                                              : "v2-abbruch-vor-reconnect");
        BelegtePipe belegt;
        const bool aufbau = belegt.anlegen (name);
        auto c = client (name, [] { return hello ("77777777777777777777777777777777"); });
        c->start();
        // Erst wirklich in die Warteschleife laufen lassen; die 300 ms danach
        // setzen den Abbruch mitten in die 4-s-Schleife.
        const bool inSchleife = warteAuf (3000, [&] {
            return c->snapshot().verbindungsVersuche >= 1;
        });
        std::this_thread::sleep_for (std::chrono::milliseconds (300));
        bool ok = false;
        long long dauer = 0;
        juce::String zustand;
        if (ueberStop)
        {
            const auto t0 = std::chrono::steady_clock::now();
            c->stop();
            dauer = std::chrono::duration_cast<std::chrono::milliseconds> (
                std::chrono::steady_clock::now() - t0).count();
            const auto s = c->snapshot();
            // D-A03: `stop()` joint OHNE Frist und setzt KEINE Zustandsfelder
            // zurueck — der Snapshot bleibt auf `verbindet` stehen. Das ist
            // ein offener Punkt ausserhalb von R1 (NAK-144) und wird hier als
            // heutiger Stand gemessen, nicht geaendert.
            ok = s.serverPruefungen == 0
              && s.status == eqcop::PipeClient::Status::verbindet;
            zustand = "Status " + juce::String ((int) s.status) + " (v2 setzt nicht zurueck)";
        }
        else
        {
            const int vorher = c->snapshot().verbindungsVersuche;
            const auto t0 = std::chrono::steady_clock::now();
            c->reconnect();
            const bool neu = warteAuf (kR5FristMs, [&] {
                return c->snapshot().verbindungsVersuche > vorher;
            });
            dauer = std::chrono::duration_cast<std::chrono::milliseconds> (
                std::chrono::steady_clock::now() - t0).count();
            ok = neu && c->snapshot().serverPruefungen == 0;
            zustand = "Versuche +"
                    + juce::String (c->snapshot().verbindungsVersuche - vorher);
            c->stop();
        }
        pruefe (aufbau && inSchleife && ok && dauer < kR5FristMs,
                "pipeclient_abbruch_vor_und_nach_createfile",
                juce::String (ueberStop ? "stop_vor_open" : "reconnect_vor_open")
                    + ": " + juce::String ((juce::int64) dauer) + " ms (R5-Frist "
                    + juce::String (kR5FristMs)
                    + " ms; die belegte Warteschleife allein kostet >= 4000 ms), "
                    + zustand);
    }

    // ── nach dem Oeffnungserfolg: der Abbruch trifft waehrend der
    //    Authentisierung ein. Kein Hello darf fliessen (D-A09, D-A12).
    for (const bool ueberStop : { true, false })
    {
        const auto name = testName (ueberStop ? "v2-abbruch-nach-stop"
                                              : "v2-abbruch-nach-reconnect");
        const auto server = pipeAnlegen (name);
        if (server == INVALID_HANDLE_VALUE)
        {
            pruefe (false, "pipeclient_abbruch_vor_und_nach_createfile", "Fake-Pipe fehlt");
            continue;
        }
        std::atomic<bool> angenommen { false }, helloGelesen { false }, freigeben { false };
        std::thread peer ([&]
        {
            std::string frame;
            angenommen.store (verbinden (server));
            if (angenommen.load())
                helloGelesen.store (liesFrame (server, frame));
            while (! freigeben.load())
                std::this_thread::sleep_for (std::chrono::milliseconds (5));
            pipeSchliessen (server);
        });

        std::atomic<bool> anBarriere { false }, barriereLoesen { false };
        auto erwartung = nakama::ipc::serverErwartungFuerEigenprozessTest();
        erwartung.testVorFehlerErreicht = &anBarriere;
        erwartung.testFehlerFreigeben = &barriereLoesen;
        auto c = client (name, [] { return hello ("88888888888888888888888888888888"); },
                         std::chrono::milliseconds { 5000 }, erwartung);
        c->start();
        const bool ander = warteAuf (5000, [&] { return anBarriere.load(); });
        const auto vorAbbruch = c->snapshot();
        bool ok = false;
        if (ueberStop)
        {
            std::thread abbrecher ([&] { c->stop(); });
            std::this_thread::sleep_for (std::chrono::milliseconds (50));
            barriereLoesen.store (true);
            abbrecher.join();
            ok = c->snapshot().serverPruefungen == vorAbbruch.serverPruefungen;
        }
        else
        {
            c->reconnect();
            std::this_thread::sleep_for (std::chrono::milliseconds (50));
            barriereLoesen.store (true);
            ok = warteAuf (4000, [&] {
                return c->snapshot().verbindungsVersuche > vorAbbruch.verbindungsVersuche;
            });
            c->stop();
        }
        const bool keinHello = ! helloGelesen.load();
        freigeben.store (true);
        wecken (name);
        peer.join();
        pruefe (ander && ok && keinHello && angenommen.load(),
                "pipeclient_abbruch_vor_und_nach_createfile",
                juce::String (ueberStop ? "stop_nach_open" : "reconnect_nach_open")
                    + ": an der Barriere " + juce::String ((int) ander)
                    + ", kein Hello " + juce::String ((int) keinHello));
    }
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
    pipeclient_sendet_vor_serverauth_keine_bytes();
    ungueltigePeerBytes (false);
    ungueltigePeerBytes (true);
    falschesAck();
    schemafremdesZusatzfeld (true);
    schemafremdesZusatzfeld (false);
    eineDeadlineFuerDenGanzenFrame();
    dropzaehlerBleibtWireI64();
    startStopSerialisiert();
    // NAK-134 — PIPE_BUSY ist Liveness, auch auf dem v2-Pfad (Matrix C-06).
    pipeclient_oeffnungsausgaenge_sind_liveness_oder_sicherheit();
    pipeclient_zaehlervertrag();
    pipeclient_authausgaenge_und_welcome_ausbleiben();
    pipeclient_parken_uebergeht_den_backoff();
    pipeclient_abbruch_vor_und_nach_createfile();
    pipeclient_backoff_folge_und_deckel_sind_beobachtbar();
    std::cout << (fehler == 0 ? "PIPECLIENT-LIFECYCLE-TEST OK - "
                              : "PIPECLIENT-LIFECYCLE-TEST FEHLGESCHLAGEN - ")
              << fehler << " Fehler" << std::endl;
    return fehler == 0 ? 0 : 1;
}
