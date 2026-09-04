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
    std::cout << bestanden << " bestanden, " << fehler << " gescheitert" << std::endl;
    return fehler == 0 ? 0 : 1;
}
