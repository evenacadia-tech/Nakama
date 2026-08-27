// Ende-zu-Ende-Probe ohne FL: spielt Sensoren gegen den laufenden Tauri-Broker
// — derselbe PipeClient-Code wie im Plugin. M2-Umfang (Protokoll v2):
//   1. v2-Handshake + Heartbeats MIT kompaktem Messstand (LTAS + Projektfenster)
//   2. heartbeat_ack wird gelesen (sonst bräche die Verbindung nach Heartbeat 1)
//   3. Kennungs-Konflikt: zweiter Client mit derselben Sensor-ID ⇒ beide sehen
//      konflikt=true im ACK; nach dessen Ende fällt das Flag wieder.
// Exit 0 nur, wenn alle drei Stufen bestanden sind.
//
//   eqcop-pipe-probe.exe [pipe-name] [sekunden]
#include "PipeClient.h"
#include "EqCopilotIds.h"
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>

static eqcop::MessKompakt probeMessung()
{
    eqcop::MessKompakt m;
    m.vorhanden = true;
    m.zustand = "messbereit";
    m.metricsVersion = "m3-2026-08-14";   // aktueller kMetricsVersion-Stand
    m.aktivS = 12.0;
    m.gesamtS = 14.0;
    m.lufsGueltig = true;  m.lufsI = -18.5;
    m.truePeakGueltig = true; m.truePeakDb = -3.2;
    m.ltasKompositDb.assign (221, -60.0);
    m.ltasKompositDb[3] = std::numeric_limits<double>::quiet_NaN();   // ⇒ null
    m.fensterGueltig = true;
    m.fensterVon = 0; m.fensterBis = 480000; m.fensterSpruenge = 0;
    return m;
}

static std::unique_ptr<eqcop::PipeClient> baueClient (const juce::String& sensorId,
                                                      const juce::String& label,
                                                      const juce::String& pipeName)
{
    eqcop::HelloInfo hello;
    hello.sensorId = sensorId;
    hello.instanceNonce = juce::Uuid().toString();
    hello.role = "sensor";
    hello.label = label;
    hello.samplerate = 48000.0;
    hello.blockSize = 512;
    hello.channels = 2;
    return std::make_unique<eqcop::PipeClient> (
        [hello] { return hello; },
        [] {
            eqcop::StatsSnapshot s;
            s.rmsL = 0.1f; s.rmsR = 0.1f;
            return s;
        },
        [] { return probeMessung(); },
        pipeName);
}

template <typename Bedingung>
static bool warteAuf (int zehntel, Bedingung ok)
{
    for (int i = 0; i < zehntel; ++i)
    {
        if (ok())
            return true;
        juce::Thread::sleep (100);
    }
    return false;
}

int main (int argc, char** argv)
{
    const juce::String pipeName = argc > 1 ? juce::String (argv[1])
                                           : juce::String (juce::CharPointer_UTF16 (eqcop::kPipeName));
    const int sekunden = argc > 2 ? juce::jlimit (1, 60, juce::String (argv[2]).getIntValue()) : 5;

    const auto sensorId = juce::Uuid().toString();
    auto a = baueClient (sensorId, "pipe-probe A", pipeName);
    a->start();

    // Stufe 1+2: v2 verbunden, mindestens zwei streng passende ACKs. Der
    // Sendezähler allein wäre kein Beweis, solange das zweite ACK aussteht.
    if (! warteAuf (sekunden * 10, [&] {
            const auto z = a->snapshot();
            return z.status == eqcop::PipeClient::Status::verbunden
                && z.protokollVersion == 2 && z.heartbeatsBestaetigt >= 2;
        }))
    {
        const auto z = a->snapshot();
        std::cout << "PROBE FEHLGESCHLAGEN (Stufe 1) · Status "
                  << (z.status == eqcop::PipeClient::Status::verbindet ? "verbindet" : "getrennt")
                  << " · Protokoll " << z.protokollVersion
                  << " · Versuche " << z.verbindungsVersuche
                  << " · Fehler: " << z.letzterFehler << std::endl;
        a->stop();
        return 1;
    }
    {
        const auto z = a->snapshot();
        std::cout << "PROBE OK v2 · broker " << z.brokerVersion
                  << " · token " << z.sessionToken.substring (0, 8) << "..."
                  << " · heartbeats " << z.heartbeatsGesendet
                  << "/" << z.heartbeatsBestaetigt << " bestaetigt"
                  << " · konflikt " << (z.konflikt ? "ja" : "nein") << std::endl;
    }

    // Stufe 3: Duplikat — beide Verbindungen melden dieselbe Sensor-ID.
    auto b = baueClient (sensorId, "pipe-probe B (Duplikat)", pipeName);
    b->start();
    if (! warteAuf (100, [&] { return a->snapshot().konflikt && b->snapshot().konflikt; }))
    {
        auto dump = [] (const char* wer, const eqcop::PipeClient::Snapshot& z)
        {
            std::cout << "  " << wer << ": status "
                      << (z.status == eqcop::PipeClient::Status::verbunden ? "verbunden"
                          : z.status == eqcop::PipeClient::Status::verbindet ? "verbindet" : "getrennt")
                      << " · protokoll " << z.protokollVersion
                      << " · heartbeats " << z.heartbeatsGesendet
                      << "/" << z.heartbeatsBestaetigt << " bestaetigt"
                      << " · versuche " << z.verbindungsVersuche
                      << " · konflikt " << (z.konflikt ? "ja" : "nein")
                      << " · letztesAck " << z.letztesAck
                      << " · fehler " << z.letzterFehler << std::endl;
        };
        std::cout << "PROBE FEHLGESCHLAGEN (Konflikt kam nicht an)" << std::endl;
        dump ("A", a->snapshot());
        dump ("B", b->snapshot());
        a->stop(); b->stop();
        return 1;
    }
    std::cout << "KONFLIKT OK · beide Instanzen sehen das Flag" << std::endl;

    b->stop();
    if (! warteAuf (100, [&] { return ! a->snapshot().konflikt; }))
    {
        std::cout << "PROBE FEHLGESCHLAGEN (Konflikt-Ende kam nicht an)" << std::endl;
        a->stop();
        return 1;
    }
    std::cout << "KONFLIKT-ENDE OK · Flag faellt nach Trennung des Duplikats" << std::endl;

    a->stop();
    return 0;
}
