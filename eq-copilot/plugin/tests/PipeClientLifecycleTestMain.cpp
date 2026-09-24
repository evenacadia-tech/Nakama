// Adversarialer Gegenpfad fuer den echten C++-PipeClient. Jede Pipe traegt
// PID+Zaehler; dieser Test beruehrt niemals den Produktionsnamen.
#include "PipeClient.h"
#include "EqCopilotIds.h"
#include "ProbePipeRegel.h"
#include "../vertrag/NakamaUtf8.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
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

// ═══════════════════════════════════════════════════════════════════════════
// NAK-313 Etappe 4 (R-313-6, M-42): der strenge Lauf im v2-Client an der
// Tabelle `PRODUKTEINGAENGE-FAELLE.json`
// ═══════════════════════════════════════════════════════════════════════════

/// Die Tabelle - Testeingabe, kein Produkteingang, deshalb mit JUCE gelesen.
juce::var produkteingaenge (bool& ok)
{
    const juce::String relativ ("eq-copilot/fixtures/v3/PRODUKTEINGAENGE-FAELLE.json");
    auto datei = juce::File::getCurrentWorkingDirectory().getChildFile (relativ);
    auto ordner = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();
    for (int i = 0; i < 10 && ! datei.existsAsFile() && ordner.exists(); ++i)
    {
        datei = ordner.getChildFile (relativ);
        ordner = ordner.getParentDirectory();
    }
    juce::var kopf;
    ok = datei.existsAsFile()
      && juce::JSON::parse (datei.loadFileAsString(), kopf).wasOk()
      && kopf.getProperty ("faelle", {}).isArray();
    return kopf;
}

/// Die Stufe am Ausgang des v2-Clients, an `letzterFehler` (Manifest §7.2):
/// „eingehender Pipe-Frame: " + Grund des strengen Laufs ist `parser`, mit
/// „doppelter Schluessel" `duplikat`; die Byteprüfungen von `empfange`
/// (Grenze, NUL, BOM, UTF-8) sind `textriegel`; alles danach `feldregel`.
juce::String stufeDesV2Clients (const juce::String& meldung)
{
    if (meldung.startsWith ("eingehender Pipe-Frame: "))
        return meldung.contains ("doppelter Schluessel") ? "duplikat" : "parser";
    if (meldung.startsWith ("eingehender Pipe-Frame "))
        return "textriegel";
    return "feldregel";
}

/// M-42 und die Zaehlpruefung aus M-49: jeder Eintrag von `cpp_v2_client` als
/// eigener Fall. Ein Peer auf einem Probenamen antwortet auf das Hello mit den
/// Bytes des Eintrags - bei `heartbeat_ack` zuerst mit dem Welcome der
/// Einspeisung und dann auf den ersten Heartbeat (seq 0). Gelesen wird im
/// Wartefenster nach dem Fehlschlag (Status getrennt, Rueckzug ab 500 ms).
void nak313V2ParserLehntAb()
{
    bool ok = false;
    const auto kopf = produkteingaenge (ok);
    pruefe (ok, "313/M-49 PRODUKTEINGAENGE-FAELLE.json liegt im Korpus");
    if (! ok)
        return;
    const auto welcomeVorAck = kopf.getProperty ("eingaenge", {}).getProperty ("cpp_v2_client", {})
                                   .getProperty ("einspeisung", {})
                                   .getProperty ("welcome_vor_ack", {}).toString().toStdString();
    int gefahren = 0;
    for (const auto& fall : *kopf.getProperty ("faelle", {}).getArray())
    {
        if (fall.getProperty ("eingang", {}).toString() != "cpp_v2_client")
            continue;
        ++gefahren;
        const auto fallName = "313/M-42 v2_parser_lehnt_ab " + fall.getProperty ("id", {}).toString();
        if (! fall.getProperty ("wert", {}).isVoid())
        {
            pruefe (false, (fallName + " (Wert)").toRawUTF8(), "dieses Bein vergleicht noch keine Werte");
            continue;
        }
        juce::MemoryBlock roh;
        roh.loadFromHexString (fall.getProperty ("bytes_hex", {}).toString());
        const bool ack = fall.getProperty ("nachricht", {}).toString() == "heartbeat_ack";

        const auto name = testName ("nak313-v2");
        const auto server = pipeAnlegen (name);
        if (server == INVALID_HANDLE_VALUE)
        {
            pruefe (false, (fallName + " (Fake-Pipe angelegt)").toRawUTF8());
            continue;
        }
        std::atomic<bool> gesendet { false }, freigeben { false };
        std::thread peer ([&]
        {
            std::string frame;
            if (verbinden (server) && liesFrame (server, frame))
            {
                const bool bereit = ! ack || (schreibeFrame (server, welcomeVorAck)
                                              && liesFrame (server, frame));
                if (bereit && schreibeRohframe (server, static_cast<const unsigned char*> (roh.getData()),
                                                static_cast<DWORD> (roh.getSize())))
                    gesendet.store (true);
                while (! freigeben.load())
                    std::this_thread::sleep_for (std::chrono::milliseconds (5));
            }
            pipeSchliessen (server);
        });

        auto c = client (name, [] { return hello ("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"); });
        c->start();
        eqcop::PipeClient::Snapshot s;
        warteAuf (1500, [&]
        {
            s = c->snapshot();
            return gesendet.load() && s.status == eqcop::PipeClient::Status::getrennt
                && s.letzterFehler.isNotEmpty();
        });
        c->stop();
        freigeben.store (true);
        peer.join();

        const bool getrennt = s.status == eqcop::PipeClient::Status::getrennt;
        const juce::String urteil = ack ? (s.heartbeatsBestaetigt == 0 && ! s.konflikt ? "ungueltig" : "gueltig")
                                        : (getrennt && s.heartbeatsGesendet == 0 ? "ungueltig" : "gueltig");
        const auto stufe = urteil == "ungueltig" ? stufeDesV2Clients (s.letzterFehler) : juce::String();
        const auto soll = fall.getProperty ("produkt", {});
        const auto sollUrteil = soll.getProperty ("urteil", {}).toString();
        const auto sollStufe = soll.getProperty ("stufe", {}).isString()
                                 ? soll.getProperty ("stufe", {}).toString() : juce::String();
        const auto zustand = "Status " + juce::String (getrennt ? "getrennt" : "nicht getrennt")
                           + ", Heartbeats " + juce::String ((juce::int64) s.heartbeatsGesendet)
                           + "/" + juce::String ((juce::int64) s.heartbeatsBestaetigt)
                           + ", letzterFehler '" + s.letzterFehler + "'";
        pruefe (gesendet.load() && urteil == sollUrteil && stufe == sollStufe,
                (fallName + " (Urteil, Stufe)").toRawUTF8(),
                "ist " + urteil + "/" + stufe + ", soll " + sollUrteil + "/" + sollStufe + " - " + zustand);

        bool gehalten = true;
        juce::StringArray worte;
        for (const auto& w : *fall.getProperty ("wirkung", {}).getArray())
        {
            const auto wort = w.toString();
            worte.add (wort);
            if (wort == "ablehnung")
                gehalten = gehalten && urteil == "ungueltig" && getrennt;
            else if (wort == "keine_teilmutation")
                gehalten = gehalten && s.heartbeatsGesendet == 0 && s.sessionToken.isEmpty()
                        && s.protokollVersion == 0;
            else if (wort == "kein_ack")
                gehalten = gehalten && s.heartbeatsBestaetigt == 0 && ! s.konflikt;
            else
                gehalten = false;   // fremde Wirkung: rot
        }
        pruefe (gehalten, (fallName + " (Wirkung)").toRawUTF8(), worte.joinIntoString (", ") + " - " + zustand);
    }
    const int soll = kopf.getProperty ("anzahl_je_eingang", {}).getProperty ("cpp_v2_client", {});
    pruefe (gefahren == soll && gefahren > 0, "313/M-49 Zaehlpruefung A4b: cpp_v2_client",
            juce::String (gefahren) + " gefahren, Kopf " + juce::String (soll));
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

        // a2) **NAK-197 — auf den ABSCHLUSS des dritten Versuchs warten.**
        //    `versuchsStempel` faellt, sobald `verbindungsVersuche` 3 zeigt.
        //    `PipeClient::eineVerbindung` zaehlt aber VOR dem Oeffnen
        //    (`PipeClient.cpp:347`, `CreateFileW` erst :375). Wer die
        //    Sperrursache direkt nach dem Stempel anlegt, trifft unter Last
        //    noch den DRITTEN Versuch: der parkt dann bei Versuch 3 statt der
        //    zugesagten >= 4, und die Zeile faellt falsch rot (Kanonlauf
        //    `768737f`, Bein A4b, Fall `authfehler`).
        //
        //    Beobachtet wird deshalb `serverPruefstatus`, nicht der Zaehler:
        //    der Eintritt setzt ihn im SELBEN Mutexblock wie den Zaehler auf
        //    `nichtGeprueft` zurueck (:353), und erst der Ausgang des
        //    Oeffnungsversuchs belegt ihn mit dem echten Urteil — gegen den
        //    noch freien Namen `nichtDa` (:405). Ein Snapshot, der den
        //    Zaehler auf 3 sieht, kann also kein Urteil des zweiten Versuchs
        //    mehr zeigen. Der v3-Zwilling in `IpcTestMain.cpp` wartet auf
        //    dasselbe Feld aus demselben Grund.
        const bool dritterFertig = warteAuf (12000, [&] {
            return c->snapshot().serverPruefstatus
                != nakama::ipc::ServerPruefStatus::nichtGeprueft;
        });

        // b) Erst JETZT die Sperrursache — der dritte Versuch ist durch, die
        //    2.000-ms-Wartezeit vor dem vierten laeuft. Die Race-Richtung ist
        //    einseitig: zu FRUEH angelegt hiesse Parken bei Versuch 3 und
        //    damit ein falsches Rot, deshalb a2. Zu SPAET angelegt kostet
        //    nichts — dann faellt auch Versuch 4 ins Leere, `backoffMs`
        //    verdoppelt auf 8.000 und Versuch 5 parkt: immer noch >= 4 und
        //    >= 4.000 ms, die Zusage bleibt diskriminierend.
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
        pruefe (aufbau && gestiegen && dritterFertig && geparkt
                    && versucheBeimParken >= 4
                    && weitere == 0 && sofort && zweiter
                    && abstand >= 350 && abstand <= 1200,
                "pipeclient_parken_uebergeht_den_backoff",
                juce::String (ueberAuth ? "authfehler" : "access_denied")
                    + ": Aufstieg " + (alleDa
                        ? juce::String ((juce::int64) (aufstieg[1] - aufstieg[0])) + " "
                            + juce::String ((juce::int64) (aufstieg[2] - aufstieg[1]))
                        : juce::String ("unvollstaendig"))
                    + " ms, dritter Versuch fertig "
                    + juce::String ((int) dritterFertig)
                    + ", geparkt " + juce::String ((int) geparkt)
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

// NAK-289 Etappe 1: das Probewerkzeug eqcop-pipe-probe.exe
// (probe/PipeProbeMain.cpp) faehrt kein Bein. Diese Wachen lesen deshalb
// seinen QUELLTEXT und halten die Zusagen fest, die NAK-289 dort eingefuehrt
// hat. Der Pfad kommt aus __FILE__, wie in DspGoldenTestMain.cpp.
std::string probeQuelltext()
{
    const auto datei = std::filesystem::path (__FILE__).parent_path().parent_path()
                     / "probe" / "PipeProbeMain.cpp";
    std::ifstream ein (datei, std::ios::binary);
    return std::string ((std::istreambuf_iterator<char> (ein)), std::istreambuf_iterator<char>());
}

std::size_t zaehleVorkommen (const std::string& text, const std::string& muster)
{
    std::size_t n = 0;
    for (auto pos = text.find (muster); pos != std::string::npos; pos = text.find (muster, pos + muster.size()))
        ++n;
    return n;
}

// bugprone-exception-escape in main: ein Funktions-try-Block, ein Handler fuer
// std::exception und ein catch-all, beide mit Meldung auf stderr und dem
// Exitcode 70 (kExitAusnahme), der kein Stufenergebnis (0, 1, 2) ist.
void probeAusnahmegrenze()
{
    const auto text = probeQuelltext();
    const std::string signatur = "int main (int argc, char** argv)";
    const auto kopf = text.find (signatur);
    const auto danach = kopf == std::string::npos
                            ? std::string::npos
                            : text.find_first_not_of (" \t\r\n", kopf + signatur.size());
    const bool tryBlock  = danach != std::string::npos && text.compare (danach, 3, "try") == 0;
    const auto handlerAb = kopf == std::string::npos ? std::string::npos : text.find ("catch (", kopf);
    const std::string handler = handlerAb == std::string::npos ? std::string() : text.substr (handlerAb);
    const bool ausnahme  = zaehleVorkommen (handler, "catch (const std::exception& e)") == 1;
    const bool alles     = zaehleVorkommen (handler, "catch (...)") == 1;
    const bool exitcode  = zaehleVorkommen (text, "static constexpr int kExitAusnahme = 70;") == 1
                        && zaehleVorkommen (handler, "return kExitAusnahme;") == 2;
    const bool stderrBeide = zaehleVorkommen (handler, "stderr") >= 4;
    pruefe (text.size() > 0 && tryBlock && ausnahme && alles && exitcode && stderrBeide,
            "nak289_probe_main_hat_ausnahmegrenze_mit_exit_70",
            juce::String ("Quelltext ") + juce::String ((int) text.size()) + " Bytes, try-Block "
                + (tryBlock ? "ja" : "nein") + ", std::exception " + (ausnahme ? "ja" : "nein")
                + ", catch-all " + (alles ? "ja" : "nein") + ", Exit 70 zweimal " + (exitcode ? "ja" : "nein")
                + ", stderr in beiden Handlern " + (stderrBeide ? "ja" : "nein"));
}

// performance-avoid-endl: jede Ausgabezeile endet mit Umbruch UND flush.
// std::endl ist nach [ostream.manip] put(widen('\n')) und dann flush(); die
// Form << '\n' << std::flush schreibt dieselben Bytes und flusht an derselben
// Stelle. Gezaehlt wird: kein std::endl, jedes << '\n' direkt gefolgt von
// << std::flush, und so viele std::cout-Anweisungen wie Enden mit Umbruch und
// flush.
void probeZeilenendeMitFlush()
{
    const auto text = probeQuelltext();
    const std::string umbruch = "<< '\\n'";
    std::size_t ohneFlush = 0, ersteZeile = 0;
    for (auto pos = text.find (umbruch); pos != std::string::npos; pos = text.find (umbruch, pos + umbruch.size()))
    {
        const auto weiter = text.find_first_not_of (" \t\r\n", pos + umbruch.size());
        const bool mitFlush = weiter != std::string::npos && text.compare (weiter, 13, "<< std::flush") == 0;
        if (! mitFlush && ohneFlush++ == 0)
        {
            ersteZeile = 1;
            for (std::size_t k = 0; k < pos; ++k)
                if (text[k] == '\n')
                    ++ersteZeile;
        }
    }
    const auto endl     = zaehleVorkommen (text, "std::endl");
    const auto ausgaben = zaehleVorkommen (text, "std::cout <<");
    const auto enden    = zaehleVorkommen (text, "<< '\\n' << std::flush;");
    pruefe (text.size() > 0 && endl == 0 && ohneFlush == 0 && ausgaben > 0 && enden == ausgaben,
            "nak289_probe_jede_ausgabezeile_endet_mit_umbruch_und_flush",
            juce::String ("std::endl ") + juce::String ((int) endl) + ", Umbrueche ohne flush "
                + juce::String ((int) ohneFlush)
                + (ohneFlush > 0 ? " (erster in Zeile " + juce::String ((int) ersteZeile) + ")" : juce::String())
                + ", std::cout-Anweisungen " + juce::String ((int) ausgaben)
                + ", Enden mit Umbruch und flush " + juce::String ((int) enden));
}
// ═════════════════════════════════════════════════════════════════════════
// NAK-309 Etappe 3 (R-309-5, R-309-3): die Regeln des Probewerkzeugs aus
// src/ProbePipeRegel.h (M-18 bis M-28). Gemessen werden die reinen Funktionen
// und ihre Verdrahtung in main per Quelltextwache (Muster NAK-289 oben). Kein
// Fall startet einen Client mit einem anderen als einem leeren Namen - auch
// unter einer Rotmutation erreicht dieses Bein die Produktions-Pipe nie
// (Gate S25h).

juce::String produktionsName()
{
    return juce::String (juce::CharPointer_UTF16 (eqcop::kPipeName));
}

std::string pluginQuelltext (const std::filesystem::path& relativ)
{
    const auto datei = std::filesystem::path (__FILE__).parent_path().parent_path() / relativ;
    std::ifstream ein (datei, std::ios::binary);
    return std::string ((std::istreambuf_iterator<char> (ein)), std::istreambuf_iterator<char>());
}

/// Der Quelltext von main in PipeProbeMain.cpp, ab der Signatur.
std::string probeMainText()
{
    const auto text = probeQuelltext();
    const auto kopf = text.find ("int main (int argc, char** argv)");
    return kopf == std::string::npos ? std::string() : text.substr (kopf);
}

const char* urteilText (eqcop::probe::PipeUrteil u)
{
    return eqcop::probe::urteilWort (u);
}

// M-18: ohne erstes Argument der v2-Probename, nie kPipeName; mit Argument
// genau das Argument, auch leer (das entscheidet dann das Urteil, M-21).
void probe_pipe_ohne_argument_waehlt_probe_namen()
{
    const char* nurProgramm[] = { "eqcop-pipe-probe.exe", nullptr };
    const auto name = eqcop::probe::probePipeName (1, nurProgramm);
    const bool probe = name == "\\\\.\\pipe\\evenacadia.eq-copilot.m2probe";
    const bool nichtProduktion = ! name.equalsIgnoreCase (produktionsName());
    const auto urteil = eqcop::probe::probePipeUrteil (name);
    pruefe (probe && nichtProduktion && urteil == eqcop::probe::PipeUrteil::zugelassen,
            "probe_pipe_ohne_argument_waehlt_probe_namen",
            name + " (Urteil " + urteilText (urteil) + ")");
    const char* mitArgument[] = { "eqcop-pipe-probe.exe", "\\\\.\\pipe\\evenacadia.eq-copilot.m2probe.rot", nullptr };
    const char* leeresArgument[] = { "eqcop-pipe-probe.exe", "", nullptr };
    const auto genannt = eqcop::probe::probePipeName (2, mitArgument);
    const auto leer = eqcop::probe::probePipeName (2, leeresArgument);
    pruefe (genannt == "\\\\.\\pipe\\evenacadia.eq-copilot.m2probe.rot" && leer.isEmpty(),
            "probe_pipe_ohne_argument_waehlt_probe_namen/mit_argument_gilt_das_argument",
            genannt + " | leer: '" + leer + "'");
}

struct NamensFall
{
    const char* bezeichnung;
    const char* name;
    eqcop::probe::PipeUrteil erwartet;
};

void pruefeNamensfaelle (const char* test, const NamensFall* faelle, std::size_t anzahl)
{
    for (std::size_t i = 0; i < anzahl; ++i)
    {
        const auto& f = faelle[i];
        const auto ist = eqcop::probe::probePipeUrteil (juce::String (f.name));
        pruefe (ist == f.erwartet, (std::string (test) + "/" + f.bezeichnung).c_str(),
                juce::String ("'") + f.name + "': erwartet " + urteilText (f.erwartet)
                    + ", Regel sagt " + urteilText (ist));
    }
}

// M-19 (Produktionsname in beiden Schreibweisen), M-21 (leer), M-22 (fremd:
// Erlaubnisliste statt Sperrliste). Die Rueckkehr mit Exit 64 vor dem ersten
// Client haelt die Quelltextwache probe_main_prueft_pipenamen_vor_dem_client.
void probe_pipe_lehnt_produktionsnamen_ab()
{
    using U = eqcop::probe::PipeUrteil;
    static const NamensFall faelle[] = {
        { "produktion_genau",                "\\\\.\\pipe\\evenacadia.eq-copilot.v1",                       U::produktion },
        { "produktion_grossgeschrieben",     "\\\\.\\PIPE\\EVENACADIA.EQ-COPILOT.V1",                       U::produktion },
        { "produktion_gemischt",             "\\\\.\\Pipe\\Evenacadia.EQ-Copilot.v1",                       U::produktion },
        { "m21_leer",                        "",                                                           U::leer },
        { "m22_v3_produktionsform",          "\\\\.\\pipe\\evenacadia.nakama.v3.ABCDEFGHIJKLMNOPQRSTUVWXYZ", U::fremd },
        { "m22_traversal_v2",                "\\\\.\\pipe\\evenacadia.eq-copilot.m2probe\\..\\evenacadia.eq-copilot.v1", U::fremd },
        { "m22_traversal_v3",                "\\\\.\\pipe\\evenacadia.nakama.v3.probe.x\\..\\evenacadia.eq-copilot.v1", U::fremd },
        { "m22_traversal_schraegstrich",     "\\\\.\\pipe\\evenacadia.eq-copilot.m2probe/../evenacadia.eq-copilot.v1", U::fremd },
        { "m22_produktion_mit_punkt",        "\\\\.\\pipe\\evenacadia.eq-copilot.v1.",                      U::fremd },
        { "m22_beliebig",                    "\\\\.\\pipe\\irgendein.name",                                 U::fremd },
        { "m22_kein_pipepfad",               "C:\\temp\\evenacadia.eq-copilot.m2probe",                     U::fremd },
        { "m22_probe_ohne_suffixzeichen",    "\\\\.\\pipe\\evenacadia.eq-copilot.m2probe.",                 U::fremd },
        { "m22_probe_suffix_mit_punkt",      "\\\\.\\pipe\\evenacadia.eq-copilot.m2probe.a.b",              U::fremd },
        { "m22_probe_suffix_mit_leerzeichen","\\\\.\\pipe\\evenacadia.eq-copilot.m2probe.a b",              U::fremd },
    };
    pruefeNamensfaelle ("probe_pipe_lehnt_produktionsnamen_ab", faelle, std::size (faelle));
}

// M-20: die Probenamen bleiben zugelassen - v2 genau, v2 mit Suffix (darunter
// der Name aus den NAK-123-Laeufen und der Testbau-Name aus M-74) und der
// v3-Probenamensraum ueber das unveraenderte istProbePipename.
void probe_pipe_laesst_probe_namen_zu()
{
    using U = eqcop::probe::PipeUrteil;
    static const NamensFall faelle[] = {
        { "v2_genau",           "\\\\.\\pipe\\evenacadia.eq-copilot.m2probe",           U::zugelassen },
        { "v2_suffix_rot",      "\\\\.\\pipe\\evenacadia.eq-copilot.m2probe.rot",       U::zugelassen },
        { "v2_suffix_nak123r1", "\\\\.\\pipe\\evenacadia.eq-copilot.m2probe.nak123r1",  U::zugelassen },
        { "v2_suffix_testbau",  "\\\\.\\pipe\\evenacadia.eq-copilot.m2probe.testbau",   U::zugelassen },
        { "v2_suffix_zeichen",  "\\\\.\\pipe\\evenacadia.eq-copilot.m2probe.A-z_09",    U::zugelassen },
        { "v3_probe",           "\\\\.\\pipe\\evenacadia.nakama.v3.probe.a4b-4242",     U::zugelassen },
    };
    pruefeNamensfaelle ("probe_pipe_laesst_probe_namen_zu", faelle, std::size (faelle));
}

// Quelltextwache M-18/M-19: main bestimmt den Namen ueber probePipeName, prueft
// ihn mit probePipeUrteil und kehrt bei Ablehnung mit Exit 64 zurueck - vor der
// Servererwartung und vor dem ersten baueClient; main nennt kPipeName nicht.
void probe_main_prueft_pipenamen_vor_dem_client()
{
    const auto text = probeQuelltext();
    const auto hauptteil = probeMainText();
    constexpr auto npos = std::string::npos;
    const auto name      = hauptteil.find ("eqcop::probe::probePipeName (argc, argv)");
    const auto urteil    = hauptteil.find ("eqcop::probe::probePipeUrteil (pipeName)");
    const auto bedingung = hauptteil.find ("if (urteil != eqcop::probe::PipeUrteil::zugelassen)");
    const auto ablehnen  = hauptteil.find ("return kExitPipename;");
    const auto erwartung = hauptteil.find ("serverErwartungFuer (serverBinary)");
    const auto client    = hauptteil.find ("baueClient (");
    const bool gefunden = name != npos && urteil != npos && bedingung != npos && ablehnen != npos
                       && erwartung != npos && client != npos;
    const bool reihenfolge = gefunden && name < urteil && urteil < bedingung && bedingung < ablehnen
                          && ablehnen < erwartung && erwartung < client;
    const bool exit64 = zaehleVorkommen (text, "static constexpr int kExitPipename = 64;") == 1
                     && zaehleVorkommen (hauptteil, "return kExitPipename;") == 1;
    const bool meldung = zaehleVorkommen (hauptteil, "\"PROBE ABGELEHNT (Pipename \"") == 1;
    const bool ohneProduktion = hauptteil.find ("kPipeName") == npos;
    pruefe (hauptteil.size() > 0 && reihenfolge && exit64 && meldung && ohneProduktion,
            "probe_main_prueft_pipenamen_vor_dem_client",
            juce::String ("Anker gefunden ") + (gefunden ? "ja" : "nein") + ", Reihenfolge Name < Urteil < "
                + "Bedingung < Exit 64 < Servererwartung < baueClient " + (reihenfolge ? "ja" : "nein")
                + ", Exit 64 genau einmal " + (exit64 ? "ja" : "nein") + ", Meldung " + (meldung ? "ja" : "nein")
                + ", kein kPipeName in main " + (ohneProduktion ? "ja" : "nein"));
}

// M-23: ein leerer Name bleibt leer (kein Rueckfall auf kPipeName), und start()
// versucht keine Verbindung. Gestartet wird der Client NUR, wenn sein Name
// leer ist - so erreicht auch die Rotmutation (Rueckfall zurueck) nie die
// Produktions-Pipe. "kein Pipename" setzt nur der Leerzweig von start(); ohne
// ihn fehlt die Meldung unmittelbar nach start(), ohne Wettlauf mit einem Thread.
void pipeclient_ohne_namen_verbindet_nicht()
{
    auto c = client ({}, [] { return hello ("99999999999999999999999999999999"); });
    const auto name = c->pipeNameFuerTest();
    pruefe (name.isEmpty(), "pipeclient_ohne_namen_verbindet_nicht/name_bleibt_leer",
            "Name '" + name + "'");
    if (name.isNotEmpty())
        return;
    c->start();
    const auto s = c->snapshot();
    pruefe (s.verbindungsVersuche == 0 && s.status == eqcop::PipeClient::Status::getrennt
                && s.letzterFehler == "kein Pipename",
            "pipeclient_ohne_namen_verbindet_nicht/start_versucht_nichts",
            juce::String ("Versuche ") + juce::String (s.verbindungsVersuche) + ", Fehler '"
                + s.letzterFehler + "'");
    c->stop();
    const auto nachStopp = c->snapshot();
    pruefe (nachStopp.verbindungsVersuche == 0,
            "pipeclient_ohne_namen_verbindet_nicht/nach_stop_kein_versuch",
            juce::String ("Versuche ") + juce::String (nachStopp.verbindungsVersuche));
}

// Quelltextwache M-24: das Produkt nennt die v2-Pipe ausdruecklich. Die
// Namenswahl v2PipeNameDesBaus() liefert ausserhalb des Testbaus kPipeName,
// produktVerdrahtung() setzt sie, und der v2-Client bekommt den Namen aus der
// Verdrahtung statt `{}` (sonst verloere Gen mit M-23 still seine v2-Verbindung).
void produkt_nennt_v2_pipe_ausdruecklich()
{
    const auto text = pluginQuelltext ("src/PluginProcessor.cpp");
    constexpr auto npos = std::string::npos;
    const auto funktion = text.find ("juce::String v2PipeNameDesBaus()");
    const auto testzweig = funktion == npos ? npos : text.find ("#if defined(NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)", funktion);
    const auto sonst = testzweig == npos ? npos : text.find ("#else", testzweig);
    const auto produkt = sonst == npos ? npos
                                       : text.find ("return juce::String (juce::CharPointer_UTF16 (kPipeName));", sonst);
    const auto ende = produkt == npos ? npos : text.find ("#endif", produkt);
    const bool namenswahl = ende != npos
        && text.substr (sonst, produkt - sonst).find ("return") == npos;
    const auto verdrahtung = text.find ("EqCopilotProcessor::V3Verdrahtung EqCopilotProcessor::produktVerdrahtung()");
    const auto zuweisung = verdrahtung == npos ? npos : text.find ("v.v2PipeName = v2PipeNameDesBaus();", verdrahtung);
    const auto rueckgabe = verdrahtung == npos ? npos : text.find ("return v;", verdrahtung);
    const bool gesetzt = zuweisung != npos && rueckgabe != npos && zuweisung < rueckgabe;
    const auto mess = text.find ("[this] { return messKompakt(); },");
    const auto frist = mess == npos ? npos : text.find ("std::chrono::milliseconds { 5000 }, brokerServerErwartung())", mess);
    const auto argument = (mess == npos || frist == npos) ? std::string() : text.substr (mess, frist - mess);
    const bool client = argument.find ("verdrahtung.v2PipeName,") != npos && argument.find ("{}") == npos;
    pruefe (namenswahl && gesetzt && client, "produkt_nennt_v2_pipe_ausdruecklich",
            juce::String ("Namenswahl mit kPipeName im Produktzweig ") + (namenswahl ? "ja" : "nein")
                + ", produktVerdrahtung setzt sie " + (gesetzt ? "ja" : "nein")
                + ", v2-Client bekommt verdrahtung.v2PipeName statt {} " + (client ? "ja" : "nein"));
}

eqcop::PipeClient::Snapshot ackStand (eqcop::PipeClient::Status status, int protokoll, int versuche,
                                      juce::int64 bestaetigt, bool konflikt)
{
    eqcop::PipeClient::Snapshot s;
    s.status = status;
    s.protokollVersion = protokoll;
    s.verbindungsVersuche = versuche;
    s.heartbeatsBestaetigt = bestaetigt;
    s.konflikt = konflikt;
    return s;
}

juce::String standText (const eqcop::PipeClient::Snapshot& s)
{
    return juce::String (s.status == eqcop::PipeClient::Status::verbunden ? "verbunden"
                         : s.status == eqcop::PipeClient::Status::verbindet ? "verbindet" : "getrennt")
         + "/v" + juce::String (s.protokollVersion) + "/Versuche " + juce::String (s.verbindungsVersuche)
         + "/bestaetigt " + juce::String (s.heartbeatsBestaetigt) + "/konflikt " + (s.konflikt ? "ja" : "nein");
}

void pruefeKonfliktende (const char* test, const eqcop::PipeClient::Snapshot& vorher,
                         const eqcop::PipeClient::Snapshot& jetzt, bool erwartet)
{
    const bool ist = eqcop::probe::konfliktEndeBestaetigt (vorher, jetzt);
    pruefe (ist == erwartet, test,
            standText (vorher) + " -> " + standText (jetzt) + ": erwartet "
                + (erwartet ? "Konfliktende" : "kein Konfliktende") + ", Praedikat sagt "
                + (ist ? "Konfliktende" : "kein Konfliktende"));
}

// M-25: A verbunden, ein neues ACK ohne Konflikt nach dem Stopp von B.
void konfliktende_mit_neuem_ack_bestaetigt()
{
    using S = eqcop::PipeClient::Status;
    const auto vorher = ackStand (S::verbunden, 2, 1, 5, true);
    pruefeKonfliktende ("konfliktende_mit_neuem_ack_bestaetigt", vorher, ackStand (S::verbunden, 2, 1, 6, false), true);
    pruefeKonfliktende ("konfliktende_mit_neuem_ack_bestaetigt/ohne_neues_ack", vorher,
                        ackStand (S::verbunden, 2, 1, 5, false), false);
}

// M-26: A verliert nach b->stop() die Verbindung. Der Abbau in PipeClient
// setzt getrennt, Protokoll 0 und konflikt falsch; die Zaehler bleiben. Der
// zweite Stand trennt die Statusbedingung von der Protokollbedingung.
void konfliktende_nicht_bei_abbruch()
{
    using S = eqcop::PipeClient::Status;
    const auto vorher = ackStand (S::verbunden, 2, 1, 5, true);
    pruefeKonfliktende ("konfliktende_nicht_bei_abbruch/abbau", vorher, ackStand (S::getrennt, 0, 1, 6, false), false);
    pruefeKonfliktende ("konfliktende_nicht_bei_abbruch/nur_status", vorher, ackStand (S::getrennt, 2, 1, 6, false), false);
}

// M-27: Neuaufbau - Versuche 1 -> 2, der neu ab 0 gezaehlte ACK-Zaehler (5)
// ueberholt den alten Wert (3).
void konfliktende_nicht_nach_neuaufbau()
{
    using S = eqcop::PipeClient::Status;
    pruefeKonfliktende ("konfliktende_nicht_nach_neuaufbau", ackStand (S::verbunden, 2, 1, 3, true),
                        ackStand (S::verbunden, 2, 2, 5, false), false);
}

// M-28: das erste ACK nach dem Stopp traegt noch konflikt (der Broker hat B
// noch nicht abgemeldet), erst das folgende nicht.
void konfliktende_wartet_auf_ack_ohne_konflikt()
{
    using S = eqcop::PipeClient::Status;
    const auto vorher = ackStand (S::verbunden, 2, 1, 5, true);
    pruefeKonfliktende ("konfliktende_wartet_auf_ack_ohne_konflikt/erstes_ack_mit_konflikt", vorher,
                        ackStand (S::verbunden, 2, 1, 6, true), false);
    pruefeKonfliktende ("konfliktende_wartet_auf_ack_ohne_konflikt/folgendes_ack_ohne_konflikt", vorher,
                        ackStand (S::verbunden, 2, 1, 7, false), true);
}

// Quelltextwache M-25: main zieht den Stand direkt nach b->stop() und wartet
// auf das Praedikat statt auf das blosse Fallen des Flags; bei Fristende
// meldet es "Konflikt-Ende ohne bestaetigtes ACK" und endet mit Exit 1.
void probe_main_wartet_auf_konfliktende_praedikat()
{
    const auto hauptteil = probeMainText();
    constexpr auto npos = std::string::npos;
    const auto vorher  = hauptteil.find ("const auto vorher = a->snapshot();");
    const auto stopp   = vorher == npos ? npos : hauptteil.rfind ("b->stop();", vorher);
    const auto warten  = hauptteil.find ("warteAuf (100, [&] { return eqcop::probe::konfliktEndeBestaetigt (vorher, a->snapshot()); })");
    const auto meldung = hauptteil.find ("PROBE FEHLGESCHLAGEN (Konflikt-Ende ohne bestaetigtes ACK)");
    const auto exit1   = meldung == npos ? npos : hauptteil.find ("return 1;", meldung);
    const auto ok      = hauptteil.find ("KONFLIKT-ENDE OK");
    const bool reihenfolge = stopp != npos && warten != npos && meldung != npos && exit1 != npos && ok != npos
                          && stopp < vorher && vorher < warten && warten < meldung && meldung < exit1 && exit1 < ok;
    const bool direkt = reihenfolge
        && hauptteil.substr (stopp + std::string ("b->stop();").size(), vorher - stopp - std::string ("b->stop();").size())
               .find (';') == npos;
    const bool altesPraedikatWeg = hauptteil.find ("! a->snapshot().konflikt") == npos;
    pruefe (reihenfolge && direkt && altesPraedikatWeg, "probe_main_wartet_auf_konfliktende_praedikat",
            juce::String ("Reihenfolge Stopp < Stand < Praedikat < Meldung < Exit 1 < OK ")
                + (reihenfolge ? "ja" : "nein") + ", Stand direkt nach dem Stopp " + (direkt ? "ja" : "nein")
                + ", altes Flagpraedikat entfernt " + (altesPraedikatWeg ? "ja" : "nein"));
}

// NAK-289 Etappe 1 (bugprone-inc-dec-in-conditions, vertrag/NakamaUtf8.h): der
// Zwei-Byte-Zweig C2..DF des UTF-8-Riegels, den PipeClient vor jedem Frame
// faehrt. Die Faelle, die die umgestellte Zeile unterscheidet: kein Folgebyte
// mehr (die Laenge zaehlt, nicht ein Terminator), ungueltiges Folgebyte,
// gueltiges Folgebyte an beiden Raendern und die Position hinter dem Folgebyte.
void utf8ZweiByteRiegel()
{
    struct Fall
    {
        const char* name;
        std::vector<unsigned char> bytes;
        std::size_t laenge;
        bool gueltig;
    };
    const Fall faelle[] = {
        { "c3_ohne_folgebyte",                    { 0xc3 },             1, false },
        { "c3_a4_mit_laenge_1",                   { 0xc3, 0xa4 },       1, false },
        { "c3_28_ungueltiges_folgebyte",          { 0xc3, 0x28 },       2, false },
        { "c3_c0_folgebyte_ueber_bf",             { 0xc3, 0xc0 },       2, false },
        { "c2_80_untere_grenze",                  { 0xc2, 0x80 },       2, true  },
        { "df_bf_obere_grenze",                   { 0xdf, 0xbf },       2, true  },
        { "c3_a4_gueltig",                        { 0xc3, 0xa4 },       2, true  },
        { "c3_a4_41_weiter_hinter_dem_folgebyte", { 0xc3, 0xa4, 0x41 }, 3, true  },
        { "c3_a4_80_naechstes_byte_ist_leitbyte", { 0xc3, 0xa4, 0x80 }, 3, false },
    };
    for (const auto& f : faelle)
    {
        const bool ist = nakama::utf8::istGueltig (f.bytes.data(), f.laenge);
        pruefe (ist == f.gueltig, (std::string ("nak289_utf8_zwei_byte/") + f.name).c_str(),
                juce::String ("erwartet ") + (f.gueltig ? "gueltig" : "ungueltig")
                    + ", Riegel sagt " + (ist ? "gueltig" : "ungueltig"));
    }
}
} // namespace

int main()
{
    std::cout << "PIPECLIENT-LIFECYCLE-TEST" << std::endl;
    // NAK-289 Etappe 1 - UTF-8-Riegel, Zwei-Byte-Zweig.
    utf8ZweiByteRiegel();
    stoppFall (false);
    stoppFall (true);
    reconnectGeneration();
    pipeclient_sendet_vor_serverauth_keine_bytes();
    ungueltigePeerBytes (false);
    ungueltigePeerBytes (true);
    falschesAck();
    // NAK-313 Etappe 4 (R-313-6) - der strenge Lauf vor jedem JSON-Parse.
    nak313V2ParserLehntAb();
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
    // NAK-289 Etappe 1 - Quelltextwachen des Probewerkzeugs.
    probeAusnahmegrenze();
    probeZeilenendeMitFlush();
    // NAK-309 Etappe 3 - Regeln des Probewerkzeugs (R-309-5, R-309-3), der
    // leere Name des PipeClient und der v2-Name des Produkts (M-18 bis M-28).
    probe_pipe_ohne_argument_waehlt_probe_namen();
    probe_pipe_lehnt_produktionsnamen_ab();
    probe_pipe_laesst_probe_namen_zu();
    probe_main_prueft_pipenamen_vor_dem_client();
    pipeclient_ohne_namen_verbindet_nicht();
    produkt_nennt_v2_pipe_ausdruecklich();
    konfliktende_mit_neuem_ack_bestaetigt();
    konfliktende_nicht_bei_abbruch();
    konfliktende_nicht_nach_neuaufbau();
    konfliktende_wartet_auf_ack_ohne_konflikt();
    probe_main_wartet_auf_konfliktende_praedikat();
    std::cout << (fehler == 0 ? "PIPECLIENT-LIFECYCLE-TEST OK - "
                              : "PIPECLIENT-LIFECYCLE-TEST FEHLGESCHLAGEN - ")
              << fehler << " Fehler" << std::endl;
    return fehler == 0 ? 0 : 1;
}
