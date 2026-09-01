// SONDE-012 B3a — Probeeq Audioqueue -> Analyseworker -> LUFS-I -> P2.
// Der Audiopuffer bleibt dabei bitgleich; nur der Worker serialisiert/allokiert.

#include "SondeProcessor.h"
#include "TelemetryClient.h"
#include "generiert/nakama_telemetry_v1_generated.h"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <thread>
#include <vector>

namespace
{
thread_local bool zaehleAudiothread = false;
thread_local std::uint64_t audioAllokationen = 0;
}

void* operator new (std::size_t groesse)
{
    if (zaehleAudiothread) ++audioAllokationen;
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete (void* p) noexcept { std::free (p); }
void operator delete (void* p, std::size_t) noexcept { std::free (p); }
void* operator new[] (std::size_t groesse)
{
    if (zaehleAudiothread) ++audioAllokationen;
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete[] (void* p) noexcept { std::free (p); }
void operator delete[] (void* p, std::size_t) noexcept { std::free (p); }

namespace
{
constexpr double fs = 48000.0;
constexpr int block = 512;
constexpr double pi = 3.14159265358979323846;
int bestanden = 0;
int fehler = 0;

void fall (const char* name, bool ok, const std::string& detail = {})
{
    std::cout << (ok ? "[ok]  " : "[ROT] ") << name;
    if (! detail.empty()) std::cout << " [" << detail << "]";
    std::cout << std::endl;
    ok ? ++bestanden : ++fehler;
}

void binde (nakama::sonde::SondeProcessor& p)
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

struct Quelle
{
    Quelle()
        : puffer (2, block)
    {
        binde (prozessor);
        prozessor.setPlayConfigDetails (2, 2, fs, block);
        prozessor.prepareToPlay (fs, block);
    }

    void sende (int samples = block, bool stille = false, bool kontext = true,
                bool schleife = false, bool zeitFortschreiben = true)
    {
        if (puffer.getNumSamples() != samples)
            puffer.setSize (2, samples, false, false, true);
        if (stille)
            puffer.clear();
        else
        {
            for (int i = 0; i < samples; ++i)
            {
                const float v = 0.1f * std::sin ((float) phase);
                phase += 2.0 * pi * 997.0 / fs;
                for (int k = 0; k < 2; ++k)
                    puffer.setSample (k, i, v);
            }
        }

        eqcop::hostbruecke::Blockbefund befund;
        if (kontext)
        {
            befund.kontext.processContextPresent = true;
            befund.kontext.projectTimeSamples.setze (projektzeit);
            befund.kontext.continuousTimeSamples.setze (kontinuierlich);
            befund.kontext.playing.setze (true);
            befund.kontext.recording.setze (false);
            befund.kontext.sampleRate.setze (fs);
            befund.kontext.tempo.setze (120.0);
            befund.kontext.ppqPosition.setze ((double) projektzeit / fs * 2.0);
            befund.kontext.cycle.aktiv = schleife;
            befund.kontext.cycle.gueltig = schleife;
            befund.kontext.cycle.startPpq = 0.0;
            befund.kontext.cycle.endePpq = 4.0;
        }
        befund.blockGroesse = (std::uint32_t) samples;

        zaehleAudiothread = audioZaehlen;
        prozessor.nakamaBlockEmpfangen (befund);
        prozessor.processBlock (puffer, midi);
        zaehleAudiothread = false;

        kontinuierlich += samples;
        if (zeitFortschreiben)
            projektzeit += samples;
    }

    void sendeViele (int anzahl, bool stille = false, bool schleife = false)
    {
        for (int i = 0; i < anzahl; ++i)
            sende (block, stille, true, schleife);
    }

    nakama::sonde::SondeProcessor prozessor;
    juce::AudioBuffer<float> puffer;
    juce::MidiBuffer midi;
    std::int64_t projektzeit = 0;
    std::int64_t kontinuierlich = 0;
    double phase = 0.0;
    bool audioZaehlen = false;
};

bool warteAufPublikation (Quelle& q, std::uint64_t mindestens,
                          nakama::analyse::FeatureFrame& frame,
                          int fristMs = 5000)
{
    const auto ende = std::chrono::steady_clock::now()
                    + std::chrono::milliseconds (fristMs);
    while (std::chrono::steady_clock::now() < ende)
    {
        if (q.prozessor.producerPublikationenFuerTest() >= mindestens
            && q.prozessor.letzterProducerFrameFuerTest (frame))
            return true;
        std::this_thread::sleep_for (std::chrono::milliseconds (5));
    }
    return false;
}

bool warteAufPaar (Quelle& q, nakama::analyse::FeatureFrame& frame,
                   int fristMs = 5000)
{
    const auto ende = std::chrono::steady_clock::now()
                    + std::chrono::milliseconds (fristMs);
    while (std::chrono::steady_clock::now() < ende)
    {
        if (q.prozessor.letzterProducerFrameFuerTest (frame)
            && frame.lufsIGesetzt && frame.lufsIUnsicherheitGesetzt
            && ! frame.lufsIStatusGesetzt)
            return true;
        std::this_thread::sleep_for (std::chrono::milliseconds (5));
    }
    return false;
}

bool warteAufStatus (Quelle& q, std::uint8_t status,
                     nakama::analyse::FeatureFrame& frame,
                     int fristMs = 5000)
{
    const auto ende = std::chrono::steady_clock::now()
                    + std::chrono::milliseconds (fristMs);
    while (std::chrono::steady_clock::now() < ende)
    {
        if (q.prozessor.letzterProducerFrameFuerTest (frame)
            && ! frame.lufsIGesetzt && ! frame.lufsIUnsicherheitGesetzt
            && frame.lufsIStatusGesetzt && frame.lufsIStatus == status)
            return true;
        std::this_thread::sleep_for (std::chrono::milliseconds (5));
    }
    return false;
}

void warteBisStabil (Quelle& q)
{
    auto letzter = q.prozessor.producerPublikationenFuerTest();
    int stabil = 0;
    for (int i = 0; i < 200 && stabil < 12; ++i)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (10));
        const auto jetzt = q.prozessor.producerPublikationenFuerTest();
        if (jetzt == letzter) ++stabil;
        else { letzter = jetzt; stabil = 0; }
    }
}

bool wirePaarIstAtomar (Quelle& q, const nakama::analyse::FeatureFrame& frame)
{
    std::vector<std::uint8_t> bytes;
    if (! nakama::ipc::featureFrameAlsFlatbuffer (
            frame, q.prozessor.v3HelloFuerTest().adresse, bytes))
        return false;
    flatbuffers::Verifier verifier (bytes.data(), bytes.size());
    if (! nakama::v3::VerifyFeatureBatchBuffer (verifier))
        return false;
    const auto* batch = nakama::v3::GetFeatureBatch (bytes.data());
    if (batch == nullptr || batch->eintraege() == nullptr
        || batch->eintraege()->size() != 1)
        return false;
    const auto* wire = batch->eintraege()->Get (0)->frame();
    return wire != nullptr && wire->lufs_i().has_value()
        && wire->lufs_i_unsicherheit_lu().has_value()
        && ! wire->lufs_i_status().has_value()
        && std::isfinite (*wire->lufs_i())
        && std::isfinite (*wire->lufs_i_unsicherheit_lu());
}

bool neueLeereSerieNachGrenze (const char* art)
{
    Quelle q;
    const bool istLoop = std::string (art) == "loop";
    q.sendeViele (110, false, istLoop);
    nakama::analyse::FeatureFrame frame;
    if (! warteAufPaar (q, frame))
        return false;
    warteBisStabil (q);
    const auto vorher = q.prozessor.producerPublikationenFuerTest();
    q.audioZaehlen = true;

    if (std::string (art) == "seek")
    {
        q.projektzeit += 48000;
        q.sende();
    }
    else if (istLoop)
    {
        q.projektzeit = 0;
        q.sende (block, false, true, true);
    }
    else if (std::string (art) == "drop")
    {
        q.sende (nakama::echtzeit::GenStrom::maxBlockFrames + 1);
        q.sende();
    }
    else
    {
        q.sende (block, false, false);
        q.sende();
    }
    q.sendeViele (22, false, istLoop);
    q.audioZaehlen = false;

    if (! warteAufPublikation (q, vorher + 1, frame))
        return false;
    warteBisStabil (q);
    if (! q.prozessor.letzterProducerFrameFuerTest (frame))
        return false;
    const bool dropSichtbar = std::string (art) != "drop"
        || q.prozessor.analyseDropsOversizeFuerTest() >= 1;
    return dropSichtbar && ! frame.lufsIGesetzt
        && ! frame.lufsIUnsicherheitGesetzt
        && frame.lufsIStatusGesetzt && frame.lufsIStatus == 1;
}
} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI juceStart;
    nakama::analyse::FeatureFrame gueltigerFrame;

    {
        Quelle q;
        q.sendeViele (110);
        const bool angekommen = warteAufPaar (q, gueltigerFrame);
        const bool paar = angekommen && gueltigerFrame.lufsIGesetzt
            && gueltigerFrame.lufsIUnsicherheitGesetzt
            && ! gueltigerFrame.lufsIStatusGesetzt
            && std::isfinite (gueltigerFrame.lufsI)
            && std::isfinite (gueltigerFrame.lufsIUnsicherheit)
            && wirePaarIstAtomar (q, gueltigerFrame);
        fall ("producer_emits_integrated_value_and_finite_uncertainty_atomically", paar);

        auto halb = gueltigerFrame;
        halb.lufsIUnsicherheitGesetzt = false;
        std::vector<std::uint8_t> bytes;
        const bool halbAbgelehnt = ! nakama::ipc::featureFrameAlsFlatbuffer (
            halb, q.prozessor.v3HelloFuerTest().adresse, bytes);
        auto nichtEndlich = gueltigerFrame;
        nichtEndlich.lufsIUnsicherheit = std::numeric_limits<float>::infinity();
        const bool infAbgelehnt = ! nakama::ipc::featureFrameAlsFlatbuffer (
            nichtEndlich, q.prozessor.v3HelloFuerTest().adresse, bytes);
        auto mitStatus = gueltigerFrame;
        mitStatus.lufsIStatusGesetzt = true;
        mitStatus.lufsIStatus = 1;
        const bool statusAbgelehnt = ! nakama::ipc::featureFrameAlsFlatbuffer (
            mitStatus, q.prozessor.v3HelloFuerTest().adresse, bytes);
        fall ("value_without_confidence_is_not_serialized",
              halbAbgelehnt && infAbgelehnt && statusAbgelehnt);
    }

    {
        Quelle sammeln;
        sammeln.sendeViele (14);
        nakama::analyse::FeatureFrame frame;
        const bool collecting = warteAufPublikation (sammeln, 1, frame)
            && ! frame.lufsIGesetzt && ! frame.lufsIUnsicherheitGesetzt
            && frame.lufsIStatusGesetzt && frame.lufsIStatus == 1;

        Quelle stille;
        stille.sendeViele (55, true);
        nakama::analyse::FeatureFrame gatedFrame;
        const bool gated = warteAufStatus (stille, 2, gatedFrame);
        fall ("no_baseline_emits_no_pair", collecting && gated);
    }

    audioAllokationen = 0;
    const bool grenzen = neueLeereSerieNachGrenze ("seek")
        && neueLeereSerieNachGrenze ("loop")
        && neueLeereSerieNachGrenze ("drop")
        && neueLeereSerieNachGrenze ("continuity");
    fall ("seek_loop_drop_start_new_empty_series",
          grenzen && audioAllokationen == 0,
          std::to_string (audioAllokationen) + " Audiothread-Allokationen");

    std::cout << "SONDE-012 LOUDNESS SOURCE: " << bestanden << " bestanden, "
              << fehler << " fehlgeschlagen" << std::endl;
    return fehler == 0 ? 0 : 1;
}
