/*  EqCopSonde013StereoGoldenTest — bandweise Stereoevidenz
    (SONDE-013 M-08, M-10, M-11, M-12).

    §40.1 sagt, warum es dieses Bein gibt: zwei globale Skalare reichen nicht.
    `breite` und `korrelation` im 10-Hz-Liveframe beschreiben die ganze
    Bandbreite mit je einer Zahl, und ein Signal, das unten mono und oben
    breit ist, sieht darin genauso aus wie eines mit gleichmaessiger Breite.

    ── DIE FUENF KLASSEN AUS §40.3 ──────────────────────────────────────────

    Mono, identisches Stereo, Polaritaetsinvertierung, bekannte Laufzeit und
    unkorrelierte Kanaele. Jede hat eine ANALYTISCH bekannte Antwort, und
    genau daran wird gemessen — nicht an einer zweiten eigenen Rechnung:

    - Mono (L bitgleich R): Korrelation exakt 1, Seitenanteil bei -inf,
      Mono-Folddown 0 dB.
    - Polaritaet (R = -L): Korrelation exakt -1, und der Mono-Folddown ist
      totale Ausloeschung.
    - Laufzeit (R = L um n Samples verzoegert): die Interchannel-PHASE ist in
      jedem Band linear in der Frequenz, phi = -2*pi*f*tau. Das ist die
      schaerfste Probe des ganzen Beins, weil sie die Phase gegen eine Formel
      haelt statt gegen ein Vorzeichen.
    - Unkorreliert: Kohaerenz nahe null bei genug Frames.

    ── DIE ZWEI FAIL-CLOSED-STUFEN ──────────────────────────────────────────

    §40.1 woertlich: "Bei zu wenig Energie oder Frames ist Kohaerenz `null`"
    und "Interchannel-Phase wird nur in ausreichend kohaerenten Baendern
    interpretiert". Dass die Phasenbits eine TEILMENGE der Kohaerenzbits sind,
    kann das JSON-Schema nicht ausdruecken (es steht dort als Kommentar); hier
    wird es gemessen.
*/

#include <juce_core/juce_core.h>

#include "../core/analysis/FeatureEngine.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace rt = nakama::echtzeit;
using nakama::analyse::FeatureEngine;
using nakama::analyse::FeatureFrame;
using nakama::analyse::Gitter;
using nakama::analyse::StereoBandwert;

namespace
{
int bestanden = 0;
int fehler = 0;

void pruefe (bool ok, const juce::String& was, const juce::String& zusatz = {})
{
    std::cout << (ok ? "[ok]   " : "[ROT]  ") << was.toRawUTF8();
    if (zusatz.isNotEmpty())
        std::cout << "  [" << zusatz.toRawUTF8() << "]";
    std::cout << std::endl;
    if (ok) ++bestanden; else ++fehler;
}

void abschnitt (const char* text)
{
    std::cout << "\n== " << text << " ==" << std::endl;
}

constexpr double kZweiPi = 6.283185307179586476925286766559;

/** Speist Bloecke mit GETRENNTEM Audio je Kanal — der Unterschied zu den
    Speisern der Nachbarbeine, und fuer Stereo der ganze Punkt. */
struct Speiser
{
    FeatureEngine& engine;
    double sr { 48000.0 };
    int    frames { 512 };
    std::uint64_t strom { 0 };
    std::int64_t  projekt { 0 };
    std::vector<float> audio;

    explicit Speiser (FeatureEngine& e) : engine (e) {}

    rt::StampedBlock bauen() const
    {
        rt::StampedBlock b;
        b.stromVon = strom;
        b.sampleCount = (std::uint32_t) frames;
        b.segment = 0;
        b.startFolge = 0;
        b.kanaele = 2;
        b.tapMaske = 1;
        b.projectSampleStart = projekt;
        b.sampleRate = sr;
        b.flags = rt::kFlagKontextAnwesend | rt::kFlagSpieltGueltig
                | rt::kFlagSampleRateGueltig | rt::kFlagSpielt | rt::kFlagZeitGueltig;
        return b;
    }

    /** `f(n, l, r)` schreibt beide Kanaele fuer die absolute Stromposition n. */
    bool sende (const std::function<void (std::uint64_t, float&, float&)>& f)
    {
        const auto b = bauen();
        audio.resize ((std::size_t) b.sampleCount * 2u);
        for (std::uint32_t i = 0; i < b.sampleCount; ++i)
        {
            float l = 0.0f, r = 0.0f;
            f (strom + i, l, r);
            audio[(std::size_t) i * 2u]      = l;
            audio[(std::size_t) i * 2u + 1u] = r;
        }
        const bool ok = engine.nimmBlock (b, audio.data());
        strom   += b.sampleCount;
        projekt += (std::int64_t) b.sampleCount;
        return ok;
    }

    /** Faehrt, bis ein Evidenzframe entsteht, und laesst die Stereoevidenz
        des zugehoerigen Fensters stehen. `false`, wenn keiner kam. */
    bool bisEvidenz (const std::function<void (std::uint64_t, float&, float&)>& f,
                     int maxBloecke = 900)
    {
        for (int i = 0; i < maxBloecke; ++i)
            if (sende (f) && engine.frame().evidenzFrisch)
                return true;
        return false;
    }
};

/** Das Band, in dem eine Frequenz liegt — ausgerechnet aus dem Gitter, nicht
    abgeschrieben. Eine feste Bandnummer im Test waere eine zweite Wahrheit
    ueber das Gitter. */
int bandFuer (double hz)
{
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
        if (hz >= Gitter::evidenzKante (b) && hz < Gitter::evidenzKante (b + 1))
            return b;
    return -1;
}

/** Wie viele Baender ueberhaupt eine Basis tragen. */
int baenderMitBasis (const FeatureEngine& e)
{
    int n = 0;
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
        if (e.stereoBand (b).basisGesetzt)
            ++n;
    return n;
}
} // namespace

int main()
{
    constexpr double fs = 48000.0;
    std::cout << "== Nakama SONDE-013 - bandweise Stereoevidenz (§40.1, §40.3) =="
              << std::endl;

    // ── M-08: mono_identity ───────────────────────────────────────────────
    abschnitt ("M-08  mono_identity");
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        // ⚠️ 1 Hz statt der Vorgabe 4 Hz. Bei 0,25 s Evidenzfenster liegen nur
        // rund fuenf Welch-Frames darin, und die Kohaerenz braucht acht
        // (§40.1). Der Fall waere sonst gruen, ohne die Kohaerenz je zu
        // sehen - gemessen beim Bau.
        e.evidenzIntervallSetzen (1.0);
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
            r = l;                                  // bitgleich, echtes Mono
        });
        pruefe (kam, "ein Evidenzfenster entsteht");

        const int b1k = bandFuer (1000.0);
        pruefe (b1k >= 0, "das 1-kHz-Band ist im Gitter auffindbar",
                "Band " + juce::String (b1k));
        const auto& w = e.stereoBand (b1k);
        pruefe (w.basisGesetzt, "es traegt eine Basis");
        pruefe (w.basisGesetzt && w.korrelationMittel > 0.999f,
                "bei echtem Mono ist die Korrelation exakt 1",
                juce::String (w.korrelationMittel, 6));
        pruefe (w.basisGesetzt && w.seitenanteilDb < -60.0f,
                "und der Seitenanteil liegt am Boden - es gibt keine Seitenenergie",
                juce::String (w.seitenanteilDb, 2) + " dB");
        pruefe (w.kohaerenzGesetzt && w.kohaerenz > 0.99f,
                "die Kohaerenz ist 1 - zwei identische Kanaele sind vollstaendig kohaerent",
                juce::String (w.kohaerenz, 6));
        pruefe (w.phaseGesetzt && std::abs (w.phaseRad) < 0.01f,
                "und die Phase ist 0 - kein Laufzeitunterschied",
                juce::String (w.phaseRad, 6) + " rad");

        const auto& sk = e.stereoSkalare();
        pruefe (sk.folddownGesetzt && std::abs (sk.monoFolddownDb) < 0.25f,
                "der Mono-Folddown ist 0 dB innerhalb 0,25 dB (§40.3) - eine "
                "Monosumme aus zwei gleichen Kanaelen verliert nichts",
                juce::String (sk.monoFolddownDb, 4) + " dB");
        pruefe (sk.balanceGesetzt && std::abs (sk.lrBalanceDb) < 0.01f,
                "und die L/R-Balance ist 0 dB",
                juce::String (sk.lrBalanceDb, 4) + " dB");
    }

    // ── M-08: silent_channel_has_no_bit ──────────────────────────────────
    abschnitt ("M-08  silent_channel_has_no_bit");
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
            r = 0.0f;                               // ein stiller Kanal
        });
        pruefe (kam, "ein Evidenzfenster entsteht");

        const int b1k = bandFuer (1000.0);
        const auto& w = e.stereoBand (b1k);
        // Die Kohaerenz braucht BEIDE Autospektren; mit einem stillen Kanal
        // ist eines davon null, und der Nenner waere es auch. Es entsteht
        // kein Wert - keine 0 und kein NaN.
        pruefe (! w.kohaerenzGesetzt,
                "mit einem stillen Kanal faellt das Kohaerenzbit weg, statt 0 oder NaN "
                "zu senden");
        pruefe (! w.phaseGesetzt,
                "und die Phase erst recht - sie ist eine Teilmenge der Kohaerenzbits");
        pruefe (w.basisGesetzt && w.seitenanteilDb > -6.0f,
                "die Basis steht trotzdem: ein stiller Kanal ist maximal breit "
                "(Mid und Side tragen dieselbe Energie)",
                juce::String (w.seitenanteilDb, 2) + " dB");

        const auto& sk = e.stereoSkalare();
        pruefe (sk.folddownGesetzt && sk.monoFolddownDb < -2.5f
                  && sk.monoFolddownDb > -3.5f,
                "der Mono-Folddown ist rund -3 dB - die halbe Energie ist im "
                "stillen Kanal",
                juce::String (sk.monoFolddownDb, 3) + " dB");
        pruefe (sk.balanceGesetzt && sk.lrBalanceDb > 300.0f,
                "und die Balance kippt vollstaendig nach links - der Wert laeuft an "
                "die Vertragsgrenze, statt zu fehlen: 'ganz links' ist eine Aussage, "
                "kein Schweigen",
                juce::String (sk.lrBalanceDb, 1) + " dB");
    }

    // ── M-08: folddown_matches_real_buffer_within_0p25db ─────────────────
    //
    // §40.3 verlangt, dass der ANGEZEIGTE Verlust dem wirklich gefalteten
    // Audiopuffer entspricht. Der Fall rechnet den Puffer hier noch einmal
    // von Hand - eine Schaetzung aus der Korrelation laege bei diesem Signal
    // um Dezibel daneben.
    abschnitt ("M-08  folddown_matches_real_buffer_within_0p25db");
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };

        double monoEnergie = 0.0, stereoEnergie = 0.0;
        const auto signal = [&] (std::uint64_t n, float& l, float& r)
        {
            const double t = (double) n / 48000.0;
            l = (float) (0.35 * std::sin (kZweiPi * 700.0 * t));
            r = (float) (0.28 * std::sin (kZweiPi * 1100.0 * t + 1.1));
            const double mono = 0.5 * ((double) l + (double) r);
            monoEnergie   += mono * mono;
            stereoEnergie += 0.5 * ((double) l * l + (double) r * r);
        };
        const bool kam = s.bisEvidenz (signal);
        pruefe (kam, "ein Evidenzfenster entsteht");

        // ⚠️ Die Handrechnung laeuft ueber ALLE gesendeten Samples, die
        // Engine ueber ihr Evidenzfenster. Beide sind hier gleich lang,
        // weil das erste Fenster mit dem ersten Block beginnt - deshalb
        // steht die Zusage in einer Toleranz und nicht als Gleichheit.
        const double erwartet = 10.0 * std::log10 (monoEnergie / stereoEnergie);
        const auto& sk = e.stereoSkalare();
        pruefe (sk.folddownGesetzt
                  && std::abs ((double) sk.monoFolddownDb - erwartet) < 0.25,
                "der gemeldete Verlust stimmt mit dem von Hand gefalteten Puffer "
                "innerhalb 0,25 dB ueberein",
                "gemeldet " + juce::String (sk.monoFolddownDb, 3) + " dB, von Hand "
                + juce::String (erwartet, 3) + " dB");
    }

    // ── M-11: bandwise_ms_and_correlation ────────────────────────────────
    //
    // Der eigentliche Grund fuer die bandweise Rechnung: ein Signal, das
    // UNTEN mono und OBEN breit ist. Zwei globale Skalare koennen das nicht
    // ausdruecken - 221 Baender schon.
    abschnitt ("M-11  bandwise_ms_and_correlation");
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            const double t = (double) n / 48000.0;
            const double tief = 0.4 * std::sin (kZweiPi * 100.0 * t);
            // Oben: zwei verschiedene Frequenzen, also dekorreliert.
            const double hochL = 0.25 * std::sin (kZweiPi * 5000.0 * t);
            const double hochR = 0.25 * std::sin (kZweiPi * 5300.0 * t + 0.4);
            l = (float) (tief + hochL);
            r = (float) (tief + hochR);
        });
        pruefe (kam, "ein Evidenzfenster entsteht");

        const int bTief = bandFuer (100.0);
        const int bHoch = bandFuer (5000.0);
        pruefe (bTief >= 0 && bHoch >= 0 && bTief != bHoch,
                "100 Hz und 5 kHz liegen in verschiedenen Baendern",
                "Band " + juce::String (bTief) + " gegen " + juce::String (bHoch));

        const auto& t100 = e.stereoBand (bTief);
        const auto& h5k  = e.stereoBand (bHoch);
        pruefe (t100.basisGesetzt && h5k.basisGesetzt,
                "beide tragen eine Basis");
        pruefe (t100.korrelationMittel > 0.9f,
                "unten ist das Material mono - Korrelation nahe 1",
                juce::String (t100.korrelationMittel, 4));
        pruefe (h5k.korrelationMittel < t100.korrelationMittel - 0.3f,
                "oben ist es dekorreliert - und GENAU DAS koennen zwei globale "
                "Skalare nicht ausdruecken",
                "unten " + juce::String (t100.korrelationMittel, 4) + ", oben "
                + juce::String (h5k.korrelationMittel, 4));
        pruefe (t100.seitenanteilDb < h5k.seitenanteilDb - 6.0f,
                "und der Seitenanteil trennt die beiden Baender ebenso",
                "unten " + juce::String (t100.seitenanteilDb, 2) + " dB, oben "
                + juce::String (h5k.seitenanteilDb, 2) + " dB");
    }

    // ── M-11: coherence_carries_window_and_dof ───────────────────────────
    abschnitt ("M-11  coherence_carries_window_and_dof");
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        // Langsame Kadenz, damit sicher mehr als acht Frames zusammenkommen.
        e.evidenzIntervallSetzen (1.0);
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
            r = l;
        });
        pruefe (kam, "ein Evidenzfenster entsteht");

        const int b1k = bandFuer (1000.0);
        const auto& w = e.stereoBand (b1k);
        pruefe (w.freiheitsgrade >= (std::uint32_t) nakama::analyse::kWelchMindestFrames,
                "das Band traegt seine Freiheitsgrade - die Zahl der gemittelten "
                "gueltigen Welch-Frames (§40.1: 'Fensterdauer und Freiheitsgrade "
                "werden Teil der Evidenz')",
                juce::String ((int) w.freiheitsgrade) + " Frames");
        pruefe (w.fensterDauerMs > 0.0f,
                "und seine Fensterdauer in Millisekunden",
                juce::String (w.fensterDauerMs, 1) + " ms");
        pruefe (w.kohaerenzGesetzt,
                "mit genug Frames traegt die Kohaerenz ein Bit");

        // Die Fensterdauer muss zur Zahl der Frames passen: sie ist ihre
        // Summe ueber den Hop. Eine Dauer, die nicht mitwaechst, waere eine
        // Konstante mit dem Namen einer Messung.
        const double hopMs = 1000.0 * (double) (FeatureEngine::kHauptPunkte / 2) / fs;
        const double erwartet = hopMs * (double) w.freiheitsgrade;
        pruefe (std::abs ((double) w.fensterDauerMs - erwartet) < 1.0,
                "und die Dauer ist die Summe der Hops ueber genau diese Frames",
                juce::String (w.fensterDauerMs, 2) + " ms gegen "
                + juce::String (erwartet, 2) + " ms");
    }

    // ── M-11: coherence_is_null_below_eight_frames ───────────────────────
    //
    // Die schaerfste Stelle des fail-closed. Die Magnitude-Squared Coherence
    // aus EINEM Frame ist identisch 1, egal wie unkorreliert die Kanaele
    // sind - |L·conj(R)|² = |L|²·|R|² gilt fuer jedes Bin exakt. Ein
    // Erzeuger ohne Mindestframezahl meldete also volle Kohaerenz fuer
    // Rauschen.
    abschnitt ("M-11  coherence_is_null_below_eight_frames");
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        // Schnellste Kadenz: 4 Hz, also 0,25 s Evidenzfenster. Bei 4096
        // Punkten und 2048 Hop sind das rund fuenf Frames - weniger als acht.
        e.evidenzIntervallSetzen (0.25);
        std::uint32_t lcg = 0x2468acef;
        const bool kam = s.bisEvidenz ([&lcg] (std::uint64_t, float& l, float& r)
        {
            auto zug = [&lcg]
            {
                lcg = lcg * 1664525u + 1013904223u;
                return (float) (((double) ((lcg >> 8) & 0xffffu) / 32768.0) - 1.0) * 0.3f;
            };
            l = zug();
            r = zug();                              // eigener Zug: unkorreliert
        });
        pruefe (kam, "ein Evidenzfenster entsteht");

        int mitBit = 0, ohneBitTrotzBasis = 0, unterAcht = 0;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            const auto& w = e.stereoBand (b);
            if (! w.basisGesetzt) continue;
            if (w.freiheitsgrade < (std::uint32_t) nakama::analyse::kWelchMindestFrames)
            {
                ++unterAcht;
                if (w.kohaerenzGesetzt) ++mitBit; else ++ohneBitTrotzBasis;
            }
        }
        pruefe (unterAcht > 0,
                "es gibt wirklich Baender mit weniger als acht Frames - der Fall "
                "ist hergestellt, nicht ausgerechnet",
                juce::String (unterAcht) + " von "
                + juce::String (baenderMitBasis (e)) + " Baendern mit Basis");
        pruefe (mitBit == 0,
                "und KEINES davon traegt ein Kohaerenzbit - unter acht Frames ist "
                "die Kohaerenz `null`, nicht 1",
                juce::String (mitBit) + " mit Bit, "
                + juce::String (ohneBitTrotzBasis) + " ohne");
    }

    // ── M-11: phase_only_in_coherent_bands ───────────────────────────────
    abschnitt ("M-11  phase_only_in_coherent_bands");
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);
        // Ein kohaerenter Traeger bei 1 kHz plus unkorreliertes Rauschen je
        // Kanal. Das Traegerband wird kohaerent, die Rauschbaender nicht.
        //
        // ⚠️ Der Traeger liegt bei 1 kHz und nicht bei 120 Hz, und das ist
        // keine Willkuer: unter `kTrennungHz` ist die BASSSTUFE zustaendig,
        // und ihr Hop von 170,7 ms erreicht die acht Welch-Frames im
        // laengsten Evidenzfenster (1 s) nicht. Baender dort tragen deshalb
        // nie eine Kohaerenz - fail-closed und richtig, aber als Testsignal
        // unbrauchbar (gemessen beim Bau: 0 Baender mit Phase). Die Grenze
        // selbst steht bei `kWelchMindestFrames` und im Manifest §10.4.
        std::uint32_t lcg = 0x13572468;
        const bool kam = s.bisEvidenz ([&lcg] (std::uint64_t n, float& l, float& r)
        {
            auto zug = [&lcg]
            {
                lcg = lcg * 1664525u + 1013904223u;
                return (double) (((double) ((lcg >> 8) & 0xffffu) / 32768.0) - 1.0);
            };
            const double traeger = 0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0);
            l = (float) (traeger + 0.12 * zug());
            r = (float) (traeger + 0.12 * zug());
        });
        pruefe (kam, "ein Evidenzfenster entsteht");

        int phaseOhneKohaerenz = 0, phaseUnterSchwelle = 0;
        int mitPhase = 0, mitKohaerenz = 0;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            const auto& w = e.stereoBand (b);
            if (w.kohaerenzGesetzt) ++mitKohaerenz;
            if (w.phaseGesetzt)
            {
                ++mitPhase;
                if (! w.kohaerenzGesetzt) ++phaseOhneKohaerenz;
                else if ((double) w.kohaerenz <= nakama::analyse::kKohaerenzSchwellePhase)
                    ++phaseUnterSchwelle;
            }
        }
        pruefe (mitPhase > 0 && mitKohaerenz > mitPhase,
                "es gibt Baender MIT und OHNE Phase - der Fall trennt wirklich",
                juce::String (mitPhase) + " mit Phase, "
                + juce::String (mitKohaerenz) + " mit Kohaerenz");
        pruefe (phaseOhneKohaerenz == 0,
                "kein Band traegt eine Phase ohne Kohaerenz - die Phasenbits sind "
                "eine TEILMENGE der Kohaerenzbits (das kann das Schema nicht sagen)",
                juce::String (phaseOhneKohaerenz) + " Verletzungen");
        pruefe (phaseUnterSchwelle == 0,
                "und keines unterhalb der benannten Schwelle "
                + juce::String (nakama::analyse::kKohaerenzSchwellePhase, 2),
                juce::String (phaseUnterSchwelle) + " Verletzungen");
    }

    // ── M-11: die Phase gegen eine FORMEL, nicht gegen ein Vorzeichen ────
    //
    // Eine bekannte Laufzeit tau zwischen den Kanaelen erzeugt eine Phase,
    // die linear in der Frequenz ist: phi(f) = -2*pi*f*tau. Das ist die
    // schaerfste Probe des Beins - ein Erzeuger, der irgendeinen Winkel
    // liefert, faellt hier, und einer mit vertauschtem Vorzeichen auch.
    abschnitt ("M-11  bekannte Laufzeit: die Phase folgt -2*pi*f*tau");
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);
        constexpr int kVerzoegerung = 8;            // Samples, R hinter L
        const double tau = (double) kVerzoegerung / fs;

        // Breitbandig, damit viele Baender Energie haben - und deterministisch,
        // damit die Verzoegerung exakt ist.
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            auto welle = [] (double m)
            {
                const double t = m / 48000.0;
                return 0.20 * std::sin (kZweiPi * 300.0 * t)
                     + 0.20 * std::sin (kZweiPi * 900.0 * t + 0.3)
                     + 0.20 * std::sin (kZweiPi * 2000.0 * t + 1.1);
            };
            l = (float) welle ((double) n);
            r = (float) welle ((double) n >= (double) kVerzoegerung
                               ? (double) n - (double) kVerzoegerung : 0.0);
        });
        pruefe (kam, "ein Evidenzfenster entsteht");

        int geprueft = 0, daneben = 0;
        double groessteAbweichung = 0.0;
        for (const double hz : { 300.0, 900.0, 2000.0 })
        {
            const int b = bandFuer (hz);
            if (b < 0) continue;
            const auto& w = e.stereoBand (b);
            if (! w.phaseGesetzt) continue;
            ++geprueft;
            // R ist die VERZOEGERTE Haelfte, also eilt L vor: arg(L·conj(R))
            // ist positiv und gleich +2*pi*f*tau.
            double erwartet = kZweiPi * hz * tau;
            // Auf (-pi, pi] wickeln - dieselbe Wicklung, die der Erzeuger
            // ueber atan2 liefert.
            while (erwartet >  3.14159265358979) erwartet -= kZweiPi;
            while (erwartet <= -3.14159265358979) erwartet += kZweiPi;
            const double ist = (double) w.phaseRad;
            double diff = ist - erwartet;
            while (diff >  3.14159265358979) diff -= kZweiPi;
            while (diff <= -3.14159265358979) diff += kZweiPi;
            groessteAbweichung = std::max (groessteAbweichung, std::abs (diff));
            if (std::abs (diff) > 0.25)
                ++daneben;
        }
        pruefe (geprueft >= 2,
                "mindestens zwei der drei Traegerfrequenzen tragen eine Phase",
                juce::String (geprueft) + " von 3");
        pruefe (daneben == 0,
                "und jede folgt der Formel +2*pi*f*tau - die Phase ist gemessen, "
                "nicht geraten",
                "groesste Abweichung " + juce::String (groessteAbweichung, 4) + " rad");
    }

    // ── M-11: persistence_is_reported ────────────────────────────────────
    abschnitt ("M-11  persistence_is_reported");
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
            r = l;
        });
        pruefe (kam, "ein Evidenzfenster entsteht");
        const auto& w = e.stereoBand (bandFuer (1000.0));
        pruefe (w.basisGesetzt && w.persistenz > 0.5f,
                "ein durchgehend kohaerentes Band meldet hohe Persistenz",
                juce::String (w.persistenz, 3));

        auto halter2 = std::make_unique<FeatureEngine>();
        auto& e2 = *halter2;
        e2.vorbereiten (fs);
        Speiser s2 { e2 };
        e2.evidenzIntervallSetzen (1.0);
        std::uint32_t lcg = 0x0f0f0f0f;
        const bool kam2 = s2.bisEvidenz ([&lcg] (std::uint64_t, float& l, float& r)
        {
            auto zug = [&lcg]
            {
                lcg = lcg * 1664525u + 1013904223u;
                return (float) (((double) ((lcg >> 8) & 0xffffu) / 32768.0) - 1.0) * 0.3f;
            };
            l = zug();
            r = zug();
        });
        const auto& w2 = e2.stereoBand (bandFuer (1000.0));
        pruefe (kam2 && w2.persistenz < 0.5f,
                "ein unkorreliertes Band meldet niedrige - die Zahl misst wirklich "
                "die Stabilitaet des Befundes",
                juce::String (w2.persistenz, 3));
    }

    // ── M-10: band_stereo_metrics_share_the_nyquist_cap ──────────────────
    abschnitt ("M-10  band_stereo_metrics_share_the_nyquist_cap");
    {
        // Bei 22,05 kHz Samplerate liegt die Kappe bei min(18 kHz,
        // 0,95·11,025 kHz) = 10,474 kHz. Jedes Band darueber darf KEINE
        // Stereometrik tragen - es entsteht keine zweite Kappenregel.
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (22050.0);
        Speiser s { e };
        s.sr = 22050.0;
        e.evidenzIntervallSetzen (1.0);
        std::uint32_t lcg = 0x77777777;
        const bool kam = s.bisEvidenz ([&lcg] (std::uint64_t, float& l, float& r)
        {
            auto zug = [&lcg]
            {
                lcg = lcg * 1664525u + 1013904223u;
                return (float) (((double) ((lcg >> 8) & 0xffffu) / 32768.0) - 1.0) * 0.3f;
            };
            l = zug();
            r = l * 0.9f;
        });
        pruefe (kam, "ein Evidenzfenster bei 22,05 kHz entsteht");

        const double kappe = std::min (18000.0, 0.95 * 22050.0 * 0.5);
        int ueberKappeMitWert = 0, ueberKappe = 0;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            if (Gitter::evidenzKante (b) < kappe)
                continue;                           // Band beginnt unter der Kappe
            ++ueberKappe;
            const auto& w = e.stereoBand (b);
            if (w.basisGesetzt || w.kohaerenzGesetzt || w.phaseGesetzt)
                ++ueberKappeMitWert;
        }
        pruefe (ueberKappe > 0,
                "es gibt Baender ueber der Kappe - der Fall ist hergestellt",
                juce::String (ueberKappe) + " Baender ab "
                + juce::String (kappe / 1000.0, 2) + " kHz");
        pruefe (ueberKappeMitWert == 0,
                "und KEINES traegt eine Stereometrik - die neuen Bandmetriken erben "
                "die Nyquist-Kappe, es entsteht keine zweite Regel",
                juce::String (ueberKappeMitWert) + " Verletzungen");
    }

    // ── M-12: width_alone_is_never_a_defect ──────────────────────────────
    //
    // §40.2: "Breite ist kein Qualitaetswert." Das ist eine Zusage ueber das,
    // was die Engine NICHT tut - sie liefert Messwerte und kein Urteil. Der
    // Fall misst es an der Fläche: es gibt kein Feld, das ein Urteil traegt.
    abschnitt ("M-12  width_alone_is_never_a_defect / low_coherence_yields_no_recommendation");
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);
        std::uint32_t lcg = 0x9abcdef0;
        const bool kam = s.bisEvidenz ([&lcg] (std::uint64_t, float& l, float& r)
        {
            auto zug = [&lcg]
            {
                lcg = lcg * 1664525u + 1013904223u;
                return (float) (((double) ((lcg >> 8) & 0xffffu) / 32768.0) - 1.0) * 0.3f;
            };
            l = zug();
            r = zug();                              // sehr breit, sehr inkohaerent
        });
        pruefe (kam, "ein sehr breites, inkohaerentes Evidenzfenster entsteht");

        // Die Zusage in Feldern: ein `StereoBandwert` traegt Messwerte und
        // Gueltigkeitsbits - keinen Befund, keine Empfehlung, keinen Lag.
        // Waere hier je ein Urteilsfeld, faellt dieser Fall beim Kompilieren
        // (der Test kennt die Struktur) oder spaetestens beim Lesen.
        int mitPhase = 0, breiteBaender = 0;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            const auto& w = e.stereoBand (b);
            if (! w.basisGesetzt) continue;
            if (w.seitenanteilDb > -6.0f) ++breiteBaender;
            if (w.phaseGesetzt) ++mitPhase;
        }
        pruefe (breiteBaender > 0,
                "viele Baender sind maximal breit",
                juce::String (breiteBaender) + " von "
                + juce::String (baenderMitBasis (e)));
        pruefe (mitPhase == 0,
                "und trotzdem entsteht in KEINEM eine Phase - bei niedriger "
                "Kohaerenz gibt es keine Lag- oder Polaritaetsaussage (§40.2), "
                "und Breite allein ist kein Befund",
                juce::String (mitPhase) + " Baender mit Phase");
    }

    // ── §40.3: Polaritaetsinvertierung ───────────────────────────────────
    abschnitt ("§40.3  Polaritaetsinvertierung: Korrelation -1, Monosumme loescht aus");
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
            r = -l;
        });
        pruefe (kam, "ein Evidenzfenster entsteht");
        const auto& w = e.stereoBand (bandFuer (1000.0));
        pruefe (w.basisGesetzt && w.korrelationMittel < -0.999f,
                "die Korrelation ist -1",
                juce::String (w.korrelationMittel, 6));
        pruefe (w.kohaerenzGesetzt && w.kohaerenz > 0.99f,
                "die KOHAERENZ ist dabei 1 - sie misst den Zusammenhang, nicht das "
                "Vorzeichen. Wer beide verwechselt, meldet Antiphase als Rauschen",
                juce::String (w.kohaerenz, 6));
        pruefe (w.phaseGesetzt && std::abs (std::abs ((double) w.phaseRad)
                                            - 3.14159265358979) < 0.05,
                "und die Phase ist +/-pi - genau die Aussage 'invertiert'",
                juce::String (w.phaseRad, 4) + " rad");
        const auto& sk = e.stereoSkalare();
        pruefe (sk.folddownGesetzt && sk.monoFolddownDb < -300.0f,
                "die Monosumme loescht sich VOLLSTAENDIG aus - der Wert laeuft an die "
                "Vertragsgrenze, statt zu fehlen. Gemessen am wirklich gefalteten "
                "Puffer, nicht aus der Korrelation geschaetzt: das ist die schaerfste "
                "Stereoaussage, die es gibt, und sie darf nicht schweigen",
                juce::String (sk.monoFolddownDb, 1) + " dB");
    }

    // ── §32.3: keine Stereogroesse ueberbrueckt eine Grenze ──────────────
    //
    // ⚠️ Dieser Abschnitt ist die Wache, die G13 NICHT leisten kann: die
    // Zwillingsprobe vergleicht `FeatureFrame`-Objekte, und die
    // Stereoevidenz liegt bewusst daneben (Begruendung bei
    // `StereoBandwert`). Ohne diesen Fall waere sie der einzige Traeger des
    // Tickets ohne Grenzwache.
    abschnitt ("§32.3  keine Stereogroesse ueberbrueckt eine Transportgrenze");
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);

        // Vorher: stark korreliertes, lautes Material.
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            l = (float) (0.5 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
            r = l;
        });
        pruefe (kam, "vor der Grenze steht ein Evidenzfenster");
        const int b1k = bandFuer (1000.0);
        const auto vorher = e.stereoBand (b1k);
        const auto skVorher = e.stereoSkalare();
        pruefe (vorher.basisGesetzt && vorher.korrelationMittel > 0.99f
                  && skVorher.folddownGesetzt,
                "mit hoher Korrelation und gemessenem Folddown",
                "Korr " + juce::String (vorher.korrelationMittel, 4) + ", Folddown "
                + juce::String (skVorher.monoFolddownDb, 3) + " dB");

        // Die Grenze, danach dekorreliertes Material.
        s.strom += 65536;
        std::uint32_t lcg = 0x5a5a5a5a;
        const bool kam2 = s.bisEvidenz ([&lcg] (std::uint64_t, float& l, float& r)
        {
            auto zug = [&lcg]
            {
                lcg = lcg * 1664525u + 1013904223u;
                return (float) (((double) ((lcg >> 8) & 0xffffu) / 32768.0) - 1.0) * 0.3f;
            };
            l = zug();
            r = zug();
        });
        pruefe (kam2, "nach der Grenze entsteht ein neues Evidenzfenster");
        const auto& nachher = e.stereoBand (b1k);
        pruefe (! nachher.basisGesetzt || nachher.korrelationMittel < 0.8f,
                "und seine Korrelation ist die des NEUEN Materials - kein "
                "Kreuzspektrum hat die Grenze ueberbrueckt",
                nachher.basisGesetzt
                    ? juce::String (nachher.korrelationMittel, 4)
                    : juce::String ("kein Bit"));
        const auto& skNachher = e.stereoSkalare();
        pruefe (! skNachher.folddownGesetzt
                  || std::abs (skNachher.monoFolddownDb - skVorher.monoFolddownDb) > 1.0f,
                "auch der Folddown ist neu gemessen und nicht fortgeschrieben",
                "vorher " + juce::String (skVorher.monoFolddownDb, 3) + " dB, nachher "
                + juce::String (skNachher.monoFolddownDb, 3) + " dB");
    }

    std::cout << "\n-----------------------------------------" << std::endl;

    // ═══════════════════════════════════════════════════════════════════
    // NAK-181 R5 · Praesenzbits fuer die Stereofelder (G4-Befund V06)
    // ═══════════════════════════════════════════════════════════════════
    //
    // ⚠️ Alle bestehenden Faelle dieses Beins fahren `evidenzIntervallSetzen(1.0)`
    // — genau deshalb hat keiner den Defekt gesehen. Bei der VORGABEkadenz
    // 0,25 s schliesst das Kurzfenster nie (12000 Samples / 2048 = 5,86 Hops,
    // Schwelle 8), und `korrelation_kurz` wie `persistenz` reisten mit dem
    // Bit von `basisGesetzt` als gemessene 0,0.
    abschnitt ("NAK-181 N-21  kurzkorrelation_und_persistenz_schweigen_ohne_kurzfenster");
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        // KEIN evidenzIntervallSetzen: die Vorgabe ist 0,25 s.
        pruefe (std::abs (e.evidenzIntervallJetzt() - 0.25) < 1e-9,
                "N-21: der Fall faehrt die VORGABEkadenz",
                juce::String (e.evidenzIntervallJetzt(), 3));
        std::uint32_t lcg = 0x13572468;
        const bool kam = s.bisEvidenz ([&lcg] (std::uint64_t, float& l, float& r)
        {
            auto zug = [&lcg]
            {
                lcg = lcg * 1664525u + 1013904223u;
                return (float) (((double) ((lcg >> 8) & 0xffffu) / 32768.0) - 1.0) * 0.3f;
            };
            l = zug();
            r = l * 0.8f + zug() * 0.2f;      // korreliert, aber nicht identisch
        });
        pruefe (kam, "N-21: ein Evidenzfenster entsteht");

        int mitBasis = 0, mitKurz = 0, mitPersistenz = 0;
        for (int b = 0; b < nakama::analyse::Gitter::evidenzBaender; ++b)
        {
            const auto& w = e.stereoBand (b);
            if (w.basisGesetzt) ++mitBasis;
            if (w.korrelationKurzGesetzt) ++mitKurz;
            if (w.persistenzGesetzt) ++mitPersistenz;
        }
        pruefe (mitBasis > 0, "N-21: Baender mit Basis gibt es", juce::String (mitBasis));
        pruefe (mitKurz == 0,
                "N-21: aber KEIN Band traegt korrelation_kurz - das Kurzfenster "
                "schliesst bei 0,25 s nie",
                juce::String (mitKurz));
        pruefe (mitPersistenz == 0,
                "N-21: und keines persistenz", juce::String (mitPersistenz));
    }

    abschnitt ("NAK-181 N-22  praesenzbits_ueberleben_das_fensterleeren");
    {
        // Bei 1 s schliessen 23 Hops (48 kHz) beziehungsweise 46 (96 kHz)
        // Kurzfenster - die Bits werden gesetzt UND muessen den Leser erreichen.
        // `baueFrame` ruft `evidenzLeeren` -> `stereoLeeren` in sich selbst,
        // BEVOR der Writer `stereoBand()` liest; ein Bit, das dort genullt
        // wuerde, waere in jedem Snapshot schon weg.
        for (const double sr : { 48000.0, 96000.0 })
        {
            auto halter = std::make_unique<FeatureEngine>();
            auto& e = *halter;
            e.vorbereiten (sr);
            Speiser s { e };
            e.evidenzIntervallSetzen (1.0);
            const bool kam = s.bisEvidenz ([sr] (std::uint64_t n, float& l, float& r)
            {
                l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / sr));
                r = l;
            });
            pruefe (kam, juce::String ("N-22: ein Evidenzfenster bei ")
                         + juce::String (sr / 1000.0, 0) + " kHz");
            const auto& w = e.stereoBand (bandFuer (1000.0));
            pruefe (w.korrelationKurzGesetzt && w.persistenzGesetzt,
                    juce::String ("N-22: beide Bits stehen NACH dem Fensterleeren (")
                    + juce::String (sr / 1000.0, 0) + " kHz)",
                    juce::String (w.persistenz, 3));
            pruefe (w.persistenz > 0.5f,
                    "N-22: und die Persistenz traegt ihren Wert",
                    juce::String (w.persistenz, 3));
        }
    }

    abschnitt ("NAK-181 N-23  stiller_kanal_laesst_alle_drei_bits_weg");
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);        // die Kurzfenster SCHLIESSEN
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
            r = 0.0f;                          // ein stiller Kanal
        });
        pruefe (kam, "N-23: ein Evidenzfenster entsteht");
        const auto& w = e.stereoBand (bandFuer (1000.0));
        // Das globale `stereoKurzfenster` ist hier GROESSER null - genau
        // deshalb reicht es als Bedingung nicht: dieses Band hatte in keinem
        // der Fenster einen gueltigen Nenner.
        pruefe (w.basisGesetzt, "N-23: die Basis steht (ein stiller Kanal ist maximal breit)");
        pruefe (! w.korrelationMittelGesetzt,
                "N-23: korrelation_mittel traegt KEIN Bit - der Nenner ist null "
                "(M-08: bei stillem Kanal faellt das Praesenzbit weg)");
        pruefe (! w.korrelationKurzGesetzt,
                "N-23: korrelation_kurz ebenso");
        pruefe (! w.persistenzGesetzt,
                "N-23: und persistenz ebenso - obwohl Kurzfenster geschlossen "
                "haben, hatte DIESES Band in keinem einen gueltigen Nenner");
    }
    // ── NAK-182 M-82: der Blockgroessen-Sweep (R3/E3, N-15 bis N-20) ─────
    //
    // M-82 verlangt die fuenf Klassen aus §40.3 "ueber alle unterstuetzten
    // Blockgroessen".  Die Abschnitte oben messen jede Klasse TIEF, aber bei
    // einer einzigen Blockgroesse (`Speiser::frames` = 512).  Dieser Sweep
    // misst dieselbe analytisch bekannte Antwort ueber die Blockgroessenmenge
    // - flacher, dafuer auf der Achse, die M-82 nennt.
    //
    // DIE MENGE steht genau EINMAL, gleich hier: dieselben 18 Groessen, die
    // B4 `EqCopQueueStressTest` §J fuer den Passthrough faehrt.  Ihre
    // Obergrenze ist die Slotkapazitaet des Layouts (`GenStrom =
    // StreamLayout<1, 131072, 2048, 16384>`, `core/StampedAudioQueue.h`): ein
    // groesserer Hostblock wird als GANZES nur fuer die Analyse verworfen.
    //
    // ⚠️ Sie ist eine AUSWAHL, keine Erschoepfung.  "Alle unterstuetzten"
    // waeren 16384 Werte; 256 zum Beispiel bleibt ungemessen.  Die
    // Bein-Behauptung in `tools/beweise.ps1` nennt deshalb die 18 Werte
    // einzeln und sagt nie "ueber alle" (Manifest §2.5).  Die vier Anker der
    // Auswahl:
    //
    //       1  die kleinste; FL zerteilt Puffer bis auf 1 Sample (S4)
    //     333  NICHTteiler des Hauptfensterhops 2048 und teilerfremd zur
    //          Rahmenkadenz - die Phasenfalle aus `AnalysisGoldenTestMain`
    //     512  der Wert der Abschnitte oben, damit der Sweep sie reproduziert
    //    2048  GENAU der Hauptfensterhop: Block- und Fenstergrenze fallen
    //          zusammen
    //   16384  die groesste; zugleich `kBassPunkte` und die Slotkapazitaet
    //
    // ⚠️ DIE FALLE, an der ein naiver Sweep still gruen waere: `bisEvidenz`
    // zaehlt BLOECKE, nicht Samples.  Bei Blockgroesse 1 waeren 900 Bloecke
    // 900 Samples - kein Evidenzfenster, `kam == false`, und ohne Pruefung
    // von `kam` haette der Fall nichts gemessen und trotzdem bestanden.  Die
    // Schranke wird deshalb auf dieselbe SAMPLEZAHL gerechnet wie bei 512,
    // und `kam` ist je Lauf eine eigene Zusage.
    //
    // ⚠️ Jedes Material ist eine reine Funktion der ABSOLUTEN Stromposition
    // `n`.  Nur so ist der Sweep ueberhaupt eine Aussage ueber Blockgroessen:
    // ein Signal, das aus einem Blockindex entstuende, waere je Blockgroesse
    // ein anderes Signal.
    abschnitt ("NAK-182 M-82  Blockgroessen-Sweep: fuenf Klassen und Folddown");
    {
        static constexpr int kSweepBlockgroessen[] = {
            1, 2, 3, 7, 15, 16, 31, 64, 127, 128, 333, 512,
            1024, 2048, 4096, 8192, 12345, 16384 };
        constexpr int kSweepN = (int) (sizeof (kSweepBlockgroessen)
                                       / sizeof (kSweepBlockgroessen[0]));
        const juce::String mengenText ("1, 2, 3, 7, 15, 16, 31, 64, 127, 128, "
                                       "333, 512, 1024, 2048, 4096, 8192, 12345, 16384");

        // Deterministisches Breitbandrauschen als reine Funktion von (n, Saat).
        //
        // ⚠️ Der erste Versuch nahm `(n * 2654435761) ^ saat` mit einem
        // xorshift32 darueber.  Zwei Saaten ergaben damit KEINE unabhaengigen
        // Stroeme: die Bandkorrelation lag bei 0,93 statt nahe null, und
        // `sweep_uncorrelated_channels` fiel zu Recht.  Die Saat muss VOR der
        // Durchmischung in den Zustand, nicht danach - deshalb der
        // splitmix64-Abschluss.
        const auto rausch = [] (std::uint64_t n, std::uint64_t saat) -> double
        {
            std::uint64_t x = n * 0x9E3779B97F4A7C15ull + saat;
            x ^= x >> 30; x *= 0xBF58476D1CE4E5B9ull;
            x ^= x >> 27; x *= 0x94D049BB133111EBull;
            x ^= x >> 31;
            return (double) (x >> 11) / 4503599627370495.5 - 1.0;
        };

        /*  Ein TRAEGER: das Band, in dem eine Frequenz liegt, mit seinen
            Bandwerten.  NAK-182 Nacharbeit 1 (EP-02/NR-02): der Sweep speiste
            drei Traeger ein und wertete nur einen aus - die Matrixzeile N-18
            verlangt alle drei.  Ein Sample Versatz ergibt bei 2000 Hz
            0,2618 rad; genau der Fall blieb bei 900 Hz gruen. */
        struct Traeger
        {
            double hz { 0.0 };
            bool basis { false }, kohGesetzt { false }, phaseGesetzt { false };
            bool korrGesetzt { false };
            float korrelation { 0.0f }, kohaerenz { 0.0f }, phase { 0.0f };
            std::uint32_t dof { 0 };
        };

        struct Lauf
        {
            bool kam { false };
            bool basis { false }, kohGesetzt { false }, phaseGesetzt { false };
            bool korrGesetzt { false }, folddownGesetzt { false };
            float korrelation { 0.0f }, kohaerenz { 0.0f }, phase { 0.0f };
            float folddown { 0.0f };
            std::uint32_t dof { 0 };
            double handFolddownDb { 0.0 };
            std::vector<Traeger> traeger;
            /*  NAK-182 Nacharbeit 1 (EP-03/NR-03): dieselben zwei Zahlen, die
                der 512er-Abschnitt fuer M-12 erhebt - Baender mit Basis und
                davon welche mit Phase.  "Keine Lag- oder Polaritaetsempfehlung"
                ist genau `phaseGesetzt == false`, und ohne diese Zahl haette
                eine faelschlich vorhandene Phase bei niedriger Korrelation als
                richtige Antwort bestanden. */
            int baenderMitBasis { 0 }, baenderMitPhase { 0 };
        };

        // Ein Lauf je Blockgroesse.  Die Engine liegt auf dem HEAP - der
        // MSVC-Standardstack ist 1 MiB, eine `FeatureEngine` rund 0,5 MB
        // (Register NAK-175).
        const auto fahre = [&] (int frames, const std::vector<double>& hzListe,
                                const std::function<void (std::uint64_t, float&, float&)>& f,
                                bool mitHandfaltung) -> Lauf
        {
            auto halter = std::make_unique<FeatureEngine>();
            auto& e = *halter;
            e.vorbereiten (fs);
            e.evidenzIntervallSetzen (1.0);
            Speiser s { e };
            s.frames = frames;
            const long long noetig = (900LL * 512LL) / (long long) frames;
            const int hoechstens = (int) std::max<long long> (900LL, noetig);

            Lauf L {};
            double monoEnergie = 0.0, stereoEnergie = 0.0;
            L.kam = s.bisEvidenz ([&] (std::uint64_t n, float& l, float& r)
            {
                f (n, l, r);
                if (mitHandfaltung)
                {
                    const double mono = 0.5 * ((double) l + (double) r);
                    monoEnergie   += mono * mono;
                    stereoEnergie += 0.5 * ((double) l * (double) l
                                            + (double) r * (double) r);
                }
            }, hoechstens);
            if (! L.kam)
                return L;

            for (double hz : hzListe)
            {
                Traeger t;
                t.hz = hz;
                const int b = bandFuer (hz);
                if (b >= 0)
                {
                    const auto& w = e.stereoBand (b);
                    t.basis       = w.basisGesetzt;
                    t.korrGesetzt = w.korrelationMittelGesetzt;
                    t.korrelation = w.korrelationMittel;
                    t.kohGesetzt  = w.kohaerenzGesetzt;
                    t.kohaerenz   = w.kohaerenz;
                    t.phaseGesetzt = w.phaseGesetzt;
                    t.phase       = w.phaseRad;
                    t.dof         = w.freiheitsgrade;
                }
                L.traeger.push_back (t);
            }
            if (! L.traeger.empty())
            {
                // Die flachen Felder sind der ERSTE Traeger - die fuenf
                // uebrigen Klassen messen genau ihn, unveraendert.
                const auto& t = L.traeger.front();
                L.basis        = t.basis;
                L.korrGesetzt  = t.korrGesetzt;
                L.korrelation  = t.korrelation;
                L.kohGesetzt   = t.kohGesetzt;
                L.kohaerenz    = t.kohaerenz;
                L.phaseGesetzt = t.phaseGesetzt;
                L.phase        = t.phase;
                L.dof          = t.dof;
            }
            for (int b = 0; b < Gitter::evidenzBaender; ++b)
            {
                const auto& w = e.stereoBand (b);
                if (! w.basisGesetzt) continue;
                ++L.baenderMitBasis;
                if (w.phaseGesetzt) ++L.baenderMitPhase;
            }
            const auto& sk = e.stereoSkalare();
            L.folddownGesetzt = sk.folddownGesetzt;
            L.folddown        = sk.monoFolddownDb;
            if (mitHandfaltung && stereoEnergie > 0.0 && monoEnergie > 0.0)
                L.handFolddownDb = 10.0 * std::log10 (monoEnergie / stereoEnergie);
            return L;
        };

        // Jede Klasse: fuenf Laeufe, dann EINE Zusage ueber alle fuenf.
        const auto klasse = [&] (const char* bezeichner, const std::vector<double>& hzListe,
                                 const std::function<void (std::uint64_t, float&, float&)>& f,
                                 const std::function<bool (const Lauf&)>& antwortStimmt,
                                 const std::function<juce::String (const Lauf&)>& zeigen,
                                 bool mitHandfaltung)
        {
            Lauf laeufe[kSweepN];
            int kamAlle = 0, richtig = 0;
            juce::String bericht;
            for (int i = 0; i < kSweepN; ++i)
            {
                laeufe[i] = fahre (kSweepBlockgroessen[i], hzListe, f, mitHandfaltung);
                if (laeufe[i].kam) ++kamAlle;
                if (laeufe[i].kam && antwortStimmt (laeufe[i])) ++richtig;
                bericht << (i ? ", " : "") << kSweepBlockgroessen[i] << ":"
                        << zeigen (laeufe[i]);
            }
            const juce::String kopf (bezeichner);
            // 1. Jeder Lauf hat wirklich gemessen.
            pruefe (kamAlle == kSweepN,
                    kopf + ": bei jeder Blockgroesse aus {" + mengenText
                        + "} entsteht ein Evidenzfenster",
                    juce::String (kamAlle) + " von " + juce::String (kSweepN));
            // 2. Die analytisch bekannte Antwort steht bei JEDER Blockgroesse.
            pruefe (richtig == kSweepN,
                    kopf + ": und jede traegt dieselbe analytisch bekannte Antwort",
                    bericht);
            // 3. Kein Lauf besteht, weil zu wenig Material floss: die
            //    Freiheitsgrade des Bandes sind ueber die Menge vergleichbar.
            std::uint32_t sortiert[kSweepN];
            for (int i = 0; i < kSweepN; ++i) sortiert[i] = laeufe[i].dof;
            std::sort (sortiert, sortiert + kSweepN);
            const double median = (double) sortiert[kSweepN / 2];
            bool dofOk = median > 0.0;
            juce::String dofText;
            for (int i = 0; i < kSweepN; ++i)
            {
                dofText << (i ? ", " : "") << (int) laeufe[i].dof;
                if (median > 0.0
                    && std::abs ((double) laeufe[i].dof - median) > 0.2 * median)
                    dofOk = false;
            }
            pruefe (dofOk,
                    kopf + ": je Blockgroesse wurden vergleichbar viele Fenster "
                           "gemittelt (Freiheitsgrade innerhalb 20 % des Medians)",
                    dofText + " (Median " + juce::String (median, 0) + ")");
        };

        // ── sweep_mono_identity ──────────────────────────────────────────
        klasse ("sweep_mono_identity", { 1000.0 },
                [] (std::uint64_t n, float& l, float& r)
                {
                    l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
                    r = l;
                },
                [] (const Lauf& L)
                {
                    return L.basis && L.korrGesetzt && L.korrelation > 0.999f
                        && L.kohGesetzt && L.kohaerenz > 0.99f
                        && L.phaseGesetzt && std::abs (L.phase) < 0.01f
                        && L.folddownGesetzt && std::abs (L.folddown) < 0.25f;
                },
                [] (const Lauf& L)
                {
                    return juce::String (L.korrelation, 4) + "/"
                         + juce::String (L.kohaerenz, 4) + "/"
                         + juce::String (L.phase, 4) + "/"
                         + juce::String (L.folddown, 3);
                }, false);

        // ── sweep_identical_stereo ───────────────────────────────────────
        // Breitbandiges Material, auf beiden Kanaelen BITGLEICH: dieselbe
        // Antwort wie Mono, aber aus einem Signal, das ohne die Gleichheit
        // breit waere.
        klasse ("sweep_identical_stereo", { 1000.0 },
                [&] (std::uint64_t n, float& l, float& r)
                {
                    l = (float) (0.35 * rausch (n, 0x51ED2701A17B93C5ull));
                    r = l;
                },
                [] (const Lauf& L)
                {
                    return L.basis && L.korrGesetzt && L.korrelation > 0.999f
                        && L.kohGesetzt && L.kohaerenz > 0.99f
                        && L.phaseGesetzt && std::abs (L.phase) < 0.01f
                        && L.folddownGesetzt && std::abs (L.folddown) < 0.25f;
                },
                [] (const Lauf& L)
                {
                    return juce::String (L.korrelation, 4) + "/"
                         + juce::String (L.kohaerenz, 4) + "/"
                         + juce::String (L.phase, 4) + "/"
                         + juce::String (L.folddown, 3);
                }, false);

        // ── sweep_polarity_inversion ─────────────────────────────────────
        // Korrelation -1 bei Kohaerenz 1, Phase +/-pi, und die Monosumme
        // laeuft an die Vertragsgrenze statt zu schweigen (§40.3).
        klasse ("sweep_polarity_inversion", { 1000.0 },
                [] (std::uint64_t n, float& l, float& r)
                {
                    l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
                    r = -l;
                },
                [] (const Lauf& L)
                {
                    return L.basis && L.korrGesetzt && L.korrelation < -0.999f
                        && L.kohGesetzt && L.kohaerenz > 0.99f
                        && L.phaseGesetzt
                        && std::abs (std::abs ((double) L.phase) - 3.14159265358979) < 0.05
                        && L.folddownGesetzt && L.folddown < -300.0f;
                },
                [] (const Lauf& L)
                {
                    return juce::String (L.korrelation, 4) + "/"
                         + juce::String (L.kohaerenz, 4) + "/"
                         + juce::String (L.phase, 3) + "/"
                         + juce::String (L.folddown, 0);
                }, false);

        // ── sweep_known_delay ────────────────────────────────────────────
        // R ist um 8 Samples verzoegert, also eilt L vor: die Phase folgt
        // +2*pi*f*tau.  Gemessen an DREI Traegern wie im Abschnitt oben, mit
        // derselben Toleranz von 0,25 rad - die Phase ist ein BANDwert, und
        // der Traeger sitzt in einem Band endlicher Breite.
        //
        // 🔑 NAK-182 Nacharbeit 1 (EP-02/NR-02): bis hierher speiste die Zeile
        // drei Traeger ein und wertete NUR 900 Hz aus.  N-18 verlangt drei je
        // Blockgroesse, und der Unterschied ist messbar: ein Sample Versatz
        // sind bei 900 Hz 0,1178 rad und bei 2000 Hz 0,2618 rad - der
        // Rotbeweis M-82 fiel deshalb an dieser Zeile NICHT.  Die eingespeiste
        // Welle und die ausgewerteten Baender kommen ab jetzt aus DERSELBEN
        // Liste; ein vierter Traeger im Signal ohne Auswertung ist damit
        // ausgeschlossen.
        {
            constexpr int kVerzoegerung = 8;
            // EINE Zahl fuer Test und Matrixzeile N-18 (Begruendung: Manifest
            // Paragraph 6.4 Nr. 6 - die Phase ist ein Bandwert).
            constexpr double kPhaseToleranzRad = 0.25;
            const double tau = (double) kVerzoegerung / fs;
            // EINE Quelle fuer Signal und Auswertung: Frequenz und Startphase
            // stehen zusammen, und die ausgewertete Liste wird daraus
            // abgeleitet.  Zwei getrennte Listen koennten auseinanderlaufen -
            // genau das war der Defekt.
            struct Quelle { double hz, startphase; };
            const Quelle quellen[] = { { 300.0, 0.0 }, { 900.0, 0.3 }, { 2000.0, 1.1 } };
            std::vector<double> traeger;
            for (const auto& q : quellen)
                traeger.push_back (q.hz);
            const auto welle = [&quellen] (double m)
            {
                const double t = m / 48000.0;
                double s = 0.0;
                for (const auto& q : quellen)
                    s += 0.20 * std::sin (kZweiPi * q.hz * t + q.startphase);
                return s;
            };
            const auto speise = [&] (std::uint64_t n, float& l, float& r)
            {
                l = (float) welle ((double) n);
                r = (float) welle ((double) n >= (double) kVerzoegerung
                                   ? (double) n - (double) kVerzoegerung : 0.0);
            };
            const auto phasenfehler = [&] (double hz, float gemessen)
            {
                double erwartet = kZweiPi * hz * tau;
                while (erwartet >  3.14159265358979) erwartet -= kZweiPi;
                while (erwartet <= -3.14159265358979) erwartet += kZweiPi;
                double diff = (double) gemessen - erwartet;
                while (diff >  3.14159265358979) diff -= kZweiPi;
                while (diff <= -3.14159265358979) diff += kZweiPi;
                return std::abs (diff);
            };
            klasse ("sweep_known_delay", traeger, speise,
                    [&] (const Lauf& L)
                    {
                        if (L.traeger.size() != traeger.size())
                            return false;
                        for (const auto& t : L.traeger)
                            if (! t.basis || ! t.phaseGesetzt
                                || phasenfehler (t.hz, t.phase) > kPhaseToleranzRad)
                                return false;
                        return true;
                    },
                    [&] (const Lauf& L)
                    {
                        juce::String s;
                        for (const auto& t : L.traeger)
                            s << (s.isEmpty() ? "" : "|") << juce::String (t.hz, 0) << "Hz "
                              << juce::String (t.phase, 4) << "(d="
                              << juce::String (phasenfehler (t.hz, t.phase), 4) << ")";
                        return s;
                    }, false);
        }

        // ── sweep_uncorrelated_channels ──────────────────────────────────
        // Zwei unabhaengige Rauschstroeme: die Korrelation faellt weit unter
        // die Schwelle, und die Kohaerenz traegt keine Empfehlung (M-12).
        //
        // 🔑 NAK-182 Nacharbeit 1 (EP-03/NR-03): N-19 verlangt neben der
        // niedrigen Korrelation ausdruecklich "keine Lag- oder
        // Polaritaetsempfehlung".  Bis hierher las das Praedikat nur Basis,
        // Korrelationsbit und Korrelation - eine faelschlich vorhandene Phase
        // waere bei niedriger Korrelation als richtige Antwort durchgegangen.
        // Gemessen wird ab jetzt mit DENSELBEN Feldern wie der 512er-Abschnitt
        // M-12 (`width_alone_is_never_a_defect`): kein Band mit Basis traegt
        // eine Phase.  Die Phase ist Stufe 2 des fail-closed - ohne sie gibt es
        // weder Lag- noch Polaritaetsaussage (Paragraph 40.2).
        klasse ("sweep_uncorrelated_channels", { 1000.0 },
                [&] (std::uint64_t n, float& l, float& r)
                {
                    l = (float) (0.35 * rausch (n, 0x1234ABCD5678EF01ull));
                    r = (float) (0.35 * rausch (n, 0xC0FFEE1234567890ull));
                },
                [] (const Lauf& L)
                {
                    return L.basis && L.korrGesetzt && L.korrelation < 0.2f
                        && L.baenderMitBasis > 0 && L.baenderMitPhase == 0;
                },
                [] (const Lauf& L)
                {
                    return juce::String (L.korrelation, 4) + "/"
                         + juce::String (L.baenderMitPhase) + " von "
                         + juce::String (L.baenderMitBasis) + " mit Phase";
                },
                false);

        // ── sweep_folddown_within_0p25db ─────────────────────────────────
        // Der gemeldete Monoverlust gegen den WIRKLICH gefalteten Puffer,
        // je Blockgroesse, innerhalb 0,25 dB (§40.3).  Die Handrechnung
        // laeuft ueber dieselben Samples, die die Engine gesehen hat.
        klasse ("sweep_folddown_within_0p25db", { 700.0 },
                [] (std::uint64_t n, float& l, float& r)
                {
                    const double t = (double) n / 48000.0;
                    l = (float) (0.35 * std::sin (kZweiPi * 700.0 * t));
                    r = (float) (0.28 * std::sin (kZweiPi * 1100.0 * t + 1.1));
                },
                [] (const Lauf& L)
                {
                    return L.folddownGesetzt
                        && std::abs ((double) L.folddown - L.handFolddownDb) < 0.25;
                },
                [] (const Lauf& L)
                {
                    return juce::String (L.folddown, 3) + " gegen "
                         + juce::String (L.handFolddownDb, 3);
                }, true);
    }

    std::cout << bestanden << " bestanden, " << fehler << " gescheitert" << std::endl;
    return fehler == 0 ? 0 : 1;
}
