// Hör-Markierung („Einfärben") — Konzept v2, eq-copilot/docs/HOER-MARKIERUNG-KONZEPT.md.
//
// Zwei Abhör-Modi je Befundkarte: SOLO (nur der Problembereich spielt) und
// PULS (voller Mix, der Problembereich schwillt im 40/60-Takt an). Rein
// beratendes Abhören — der Renderweg bleibt transparent: verarbeite() färbt
// nur, wenn der Prozessor Echtzeit BEWIESEN hat (Lebenszeichen, §4/B6), und
// schneidet bei Wegfall der Erlaubnis HART auf den Originalpfad (keine
// Fade-Reste in einem Render).
//
// Threading-Vertrag:
//  · Message-Thread: baueMarkierungsAuftrag() (alle Transzendenten) +
//    reicheEin()/reicheAus() — Ring aus 4 Slots, Publikationszähler.
//  · Audiothread: verarbeite() — kopiert höchstens einen Slot (POD), keine
//    Allokation, keine Sperre, keine Transzendente (B7: Puls-Hüllkurve liegt
//    als Stufenfolge je 32-Sample-Chunk vorberechnet im Auftrag).
//  · Ein Slot-Riss wäre nur möglich, wenn der Editor während EINER
//    Blockkopie vier Aufträge publiziert — menschlich unerreichbar; der
//    Zähler macht jede neue Publikation trotzdem sichtbar.
//
// DSP: TDF2-Biquads, double-Koeffizienten + double-Zustände (Tieffrequenz-
// Robustheit, Mulm-Kante 120 Hz). RBJ-Cookbook-Parametrik mit Bandbreite in
// Oktaven. Ruhephasen-Identität des Puls-Bells: bei 0 dB gilt b == a
// bit-exakt (α·A und α/A sind mit A=1 dieselbe double-Rechnung) — mit
// FTZ/ScopedNoDenormals fallen die Zustände in der Ruhe auf exakt 0 und der
// Pfad ist wieder samplegleich.
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace eqcop
{

enum class MarkierungsModus : int { aus = 0, solo = 1, puls = 2 };

inline constexpr int kPulsStufen = 64;            // ≤ 0,08 dB Gain-Stufung bei 4,5 dB
inline constexpr int kPulsChunk = 32;             // Koeffizientenwechsel-Raster (~0,7 ms)
inline constexpr int kPulsMaxChunks = 2048;       // 250 ms Anstieg bis 262 kHz Samplerate
inline constexpr int kMarkierungMaxSektionen = 4;

// Normalisierte Biquad-Koeffizienten (a0 = 1):
// y = b0·x + b1·x1 + b2·x2 − a1·y1 − a2·y2
struct BiquadKoeff
{
    double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
};

// ── RBJ Audio-EQ-Cookbook, Bandbreite-in-Oktaven-Parametrik ────────────────
inline double rbjAlphaBw (double w0, double bwOkt)
{
    const double s = std::sin (w0);
    return s * std::sinh (0.5 * std::log (2.0) * bwOkt * (w0 / s));
}

inline bool biquadStabil (const BiquadKoeff& k)
{
    // Stabilitätsdreieck: Pole innerhalb des Einheitskreises.
    return std::isfinite (k.b0) && std::isfinite (k.b1) && std::isfinite (k.b2)
        && std::isfinite (k.a1) && std::isfinite (k.a2)
        && std::abs (k.a2) < 1.0 && std::abs (k.a1) < 1.0 + k.a2;
}

inline BiquadKoeff rbjPeaking (double fs, double f0, double bwOkt, double gainDb)
{
    const double A  = std::pow (10.0, gainDb / 40.0);
    const double w0 = 2.0 * juce::MathConstants<double>::pi * f0 / fs;
    const double a  = rbjAlphaBw (w0, bwOkt);
    const double c  = std::cos (w0);
    const double a0 = 1.0 + a / A;
    // Bei gainDb == 0 ist A == 1 ⇒ a·A und a/A sind DIESELBE Rechnung ⇒
    // b == a bit-exakt ⇒ Übertragung exakt 1 (Ruhephasen-Identität, B1).
    BiquadKoeff k;
    k.b0 = (1.0 + a * A) / a0;
    k.b1 = (-2.0 * c) / a0;
    k.b2 = (1.0 - a * A) / a0;
    k.a1 = (-2.0 * c) / a0;
    k.a2 = (1.0 - a / A) / a0;
    return k;
}

inline BiquadKoeff rbjBandpass0dB (double fs, double f0, double bwOkt)
{
    // „constant 0 dB peak gain"-Variante — Solo darf nie lauter machen.
    const double w0 = 2.0 * juce::MathConstants<double>::pi * f0 / fs;
    const double a  = rbjAlphaBw (w0, bwOkt);
    const double c  = std::cos (w0);
    const double a0 = 1.0 + a;
    BiquadKoeff k;
    k.b0 = a / a0;
    k.b1 = 0.0;
    k.b2 = -a / a0;
    k.a1 = (-2.0 * c) / a0;
    k.a2 = (1.0 - a) / a0;
    return k;
}

inline BiquadKoeff rbjHochpass (double fs, double fc, double q)
{
    const double w0 = 2.0 * juce::MathConstants<double>::pi * fc / fs;
    const double a  = std::sin (w0) / (2.0 * q);
    const double c  = std::cos (w0);
    const double a0 = 1.0 + a;
    BiquadKoeff k;
    k.b0 = ((1.0 + c) * 0.5) / a0;
    k.b1 = (-(1.0 + c)) / a0;
    k.b2 = ((1.0 + c) * 0.5) / a0;
    k.a1 = (-2.0 * c) / a0;
    k.a2 = (1.0 - a) / a0;
    return k;
}

inline BiquadKoeff rbjTiefpass (double fs, double fc, double q)
{
    const double w0 = 2.0 * juce::MathConstants<double>::pi * fc / fs;
    const double a  = std::sin (w0) / (2.0 * q);
    const double c  = std::cos (w0);
    const double a0 = 1.0 + a;
    BiquadKoeff k;
    k.b0 = ((1.0 - c) * 0.5) / a0;
    k.b1 = (1.0 - c) / a0;
    k.b2 = ((1.0 - c) * 0.5) / a0;
    k.a1 = (-2.0 * c) / a0;
    k.a2 = (1.0 - a) / a0;
    return k;
}

// ── Der fertige Auftrag: POD, vom Message-Thread gerechnet (B7) ────────────
struct MarkierungsAuftrag
{
    MarkierungsModus modus = MarkierungsModus::aus;
    double fs = 0.0;                 // Riegel: gilt nur für diese Samplerate
    int sektionen = 0;               // Solo: 2 (Resonanz) / 4 (Zone); Puls: 1
    std::array<BiquadKoeff, kMarkierungMaxSektionen> statisch {};
    std::array<BiquadKoeff, kPulsStufen> puls {};        // Stufe 0 = Identität
    std::array<std::uint8_t, kPulsMaxChunks> stufenFolge {};  // je 32-Sample-Chunk
    int pulsAnstiegSamples = 0;      // 250 ms Kosinus-Schwellen (auf+ab)
    int pulsRuheSamples = 0;         // 375 ms exakt 0 dB (Referenzphase, B2)
    int fadeSamples = 0;             // linearer Crossfade (B5): Solo 80 ms, Puls 30 ms
};

// Eingabe des Editors — direkt aus der Befundkarte (Diagnose::Befund).
struct MarkierungsWunsch
{
    MarkierungsModus modus = MarkierungsModus::aus;
    bool istResonanz = false;        // Resonanz ⇒ Bandpass-Solo; Zone ⇒ HP+LP
    double fVon = 0.0, fBis = 0.0, fSchwerpunkt = 0.0;
    double breiteOktaven = 0.0;      // nur Resonanz (Diagnose: ≥ 1/12 Okt)
    double pulsTiefeDb = 4.5;        // headroom-bewusst vom Editor (B3), 2,0–4,5
    double fs = 0.0;
};

// Baut den Auftrag. false = NaN-Riegel/Kanten (CS1-Muster) — dann kein Engage.
inline bool baueMarkierungsAuftrag (MarkierungsAuftrag& ziel, const MarkierungsWunsch& w)
{
    if (w.modus == MarkierungsModus::aus)
    {
        ziel = MarkierungsAuftrag {};
        return true;
    }
    if (! (std::isfinite (w.fs) && w.fs >= 8000.0 && std::isfinite (w.fVon)
           && std::isfinite (w.fBis) && std::isfinite (w.fSchwerpunkt)
           && std::isfinite (w.pulsTiefeDb)))
        return false;

    const double nyKappe = 0.475 * w.fs;         // 0,95 · Nyquist (LTAS-Lehre M1)
    const double von = w.fVon;
    const double bis = std::min (w.fBis, nyKappe);
    if (! (von > 10.0 && bis > von))
        return false;

    ziel = MarkierungsAuftrag {};
    ziel.modus = w.modus;
    ziel.fs = w.fs;

    if (w.modus == MarkierungsModus::solo)
    {
        if (w.istResonanz)
        {
            // Echter Bandpass, 2 identische Stufen; Kaskaden-Korrektur:
            // zwei 2.-Ordnung-BPs verengen die −3-dB-Breite um √(√2−1) ≈ 0,6436.
            const double zielBreite = std::max ({ w.breiteOktaven,
                                                  std::log2 (bis / von), 1.0 / 3.0 });
            const double stufenBreite = zielBreite / 0.6436;
            const double f0 = std::min (std::max (w.fSchwerpunkt, 20.0), nyKappe);
            ziel.statisch[0] = rbjBandpass0dB (w.fs, f0, stufenBreite);
            ziel.statisch[1] = ziel.statisch[0];
            ziel.sektionen = 2;
        }
        else
        {
            // Zone: Butterworth 4. Ordnung als HP+LP (Q-Paar der Kaskade).
            constexpr double q1 = 0.54119610, q2 = 1.30656296;
            ziel.statisch[0] = rbjHochpass (w.fs, von, q1);
            ziel.statisch[1] = rbjHochpass (w.fs, von, q2);
            ziel.statisch[2] = rbjTiefpass (w.fs, bis, q1);
            ziel.statisch[3] = rbjTiefpass (w.fs, bis, q2);
            ziel.sektionen = 4;
        }
        ziel.fadeSamples = (int) std::lround (0.080 * w.fs);
    }
    else // puls
    {
        const double tiefe = std::min (std::max (w.pulsTiefeDb, 2.0), 4.5);
        const double f0roh = w.istResonanz ? w.fSchwerpunkt : std::sqrt (von * bis);
        const double f0 = std::min (std::max (f0roh, 20.0), nyKappe);
        const double bwOkt = std::max (w.istResonanz ? w.breiteOktaven
                                                     : std::log2 (bis / von),
                                       1.0 / 6.0);
        for (int g = 0; g < kPulsStufen; ++g)
            ziel.puls[(size_t) g] = rbjPeaking (w.fs, f0, bwOkt,
                                                tiefe * (double) g / (double) (kPulsStufen - 1));

        ziel.pulsAnstiegSamples = (int) std::lround (0.250 * w.fs);
        ziel.pulsRuheSamples    = (int) std::lround (0.375 * w.fs);
        const int chunks = (ziel.pulsAnstiegSamples + kPulsChunk - 1) / kPulsChunk;
        if (chunks > kPulsMaxChunks)
            return false;                        // > 262 kHz — praktisch unerreichbar
        for (int c = 0; c < chunks; ++c)
        {
            // Chunk-Mitte auf der raised-cosine-Hüllkurve (B2), als Stufenindex.
            const double pos = ((double) c + 0.5) * (double) kPulsChunk
                               / (double) ziel.pulsAnstiegSamples;
            const double env = pos >= 1.0 ? 0.0
                             : 0.5 - 0.5 * std::cos (2.0 * juce::MathConstants<double>::pi * pos);
            ziel.stufenFolge[(size_t) c] =
                (std::uint8_t) std::min (kPulsStufen - 1,
                                         (int) std::lround (env * (kPulsStufen - 1)));
        }
        ziel.sektionen = 1;
        ziel.fadeSamples = (int) std::lround (0.030 * w.fs);
    }

    for (int s = 0; s < ziel.sektionen && ziel.modus == MarkierungsModus::solo; ++s)
        if (! biquadStabil (ziel.statisch[(size_t) s]))
            return false;
    if (ziel.modus == MarkierungsModus::puls)
        for (int g = 0; g < kPulsStufen; ++g)
            if (! biquadStabil (ziel.puls[(size_t) g]))
                return false;
    return true;
}

// ── Audiothread-Seite ──────────────────────────────────────────────────────
class HoerMarkierungDsp
{
public:
    // prepareToPlay-Kontext (nie gleichzeitig mit verarbeite).
    void vorbereiten (int maxBlock)
    {
        wetKapazitaet = std::max (maxBlock, 16);
        wet.calloc ((size_t) wetKapazitaet * 2);
        hartAus();
    }

    // Message-Thread: neuen Auftrag publizieren (Ring aus 4, s. Kopfkommentar).
    void reicheEin (const MarkierungsAuftrag& a)
    {
        const std::uint32_t nr = veroeffentlicht.load (std::memory_order_relaxed) + 1;
        ring[nr % ring.size()] = a;
        veroeffentlicht.store (nr, std::memory_order_release);
        zielGesetztAtomic.store (a.modus != MarkierungsModus::aus,
                                 std::memory_order_relaxed);
    }
    void reicheAus()
    {
        reicheEin (MarkierungsAuftrag {});
    }

    // Audiothread. erlaubt = Echtzeit bewiesen ∧ Transport spielt (falls
    // vorhanden) ∧ kein Offline-Render ∧ Editor offen. Bei !erlaubt bleibt der
    // Puffer GARANTIERT unangetastet (harter Schnitt statt Fade-Rest, B6).
    void verarbeite (juce::AudioBuffer<float>& puffer, int kanaele, bool erlaubt)
    {
        const int n = puffer.getNumSamples();
        if (n <= 0 || kanaele <= 0 || n > wetKapazitaet)
        {
            if (n > wetKapazitaet) hartAus();     // Host größer als prepareToPlay: neutral
            return;
        }
        const auto nr = veroeffentlicht.load (std::memory_order_acquire);
        if (nr != gelesenNr)
        {
            // Aus-Wunsch überschreibt lokal NICHT: die Filterkonfiguration
            // bleibt für den weichen Fade-out stehen (sonst spränge das
            // Wet-Signal sofort auf trocken = harter Schnitt statt Blende).
            if (ring[nr % ring.size()].modus == MarkierungsModus::aus)
                ausGewuenscht = true;
            else
            {
                lokal = ring[nr % ring.size()];
                ausGewuenscht = false;
                pulsPos = 0;                      // frischer Auftrag beginnt am Hüllkurven-Anfang
            }
            gelesenNr = nr;
        }
        const bool zielAn = ! ausGewuenscht
                         && gelesenNr != 0
                         && lokal.modus != MarkierungsModus::aus
                         && lokal.sektionen > 0
                         && lokal.fs == fsAktuell;

        if (! erlaubt)
        {
            if (fade > 0.0 || pulsPos != 0)
                hartAus();
            hoerbarAtomic.store (false, std::memory_order_relaxed);
            phaseAtomic.store (0.0f, std::memory_order_relaxed);
            return;                               // Puffer unangetastet ⇒ bit-transparent
        }
        if (! zielAn && fade <= 0.0)
        {
            hoerbarAtomic.store (false, std::memory_order_relaxed);
            phaseAtomic.store (0.0f, std::memory_order_relaxed);
            return;                               // Ruhe = echter Originalpfad
        }

        // Wet rechnen (Kopie des Eingangs; Abgriff/Messung liegen davor).
        const int ch = std::min (kanaele, 2);
        float* wetK[2] = { wet.getData(), wet.getData() + wetKapazitaet };
        for (int k = 0; k < ch; ++k)
            std::memcpy (wetK[k], puffer.getReadPointer (k), (size_t) n * sizeof (float));

        float huell = 1.0f;
        if (lokal.modus == MarkierungsModus::solo)
        {
            for (int k = 0; k < ch; ++k)
                for (int s = 0; s < lokal.sektionen; ++s)
                    tdf2Lauf (wetK[k], n, lokal.statisch[(size_t) s], zust[k][s]);
        }
        else
        {
            // Puls: Bell-Koeffizienten je 32er-Chunk aus der Stufenfolge —
            // keine Transzendente, Zustände laufen über Stufenwechsel weiter.
            const int periode = lokal.pulsAnstiegSamples + lokal.pulsRuheSamples;
            int i = 0;
            while (i < n)
            {
                const int bisRaster = kPulsChunk - (pulsPos % kPulsChunk);
                const int stueck = std::min (n - i, bisRaster);
                int stufe = 0;
                if (periode > 0 && pulsPos < lokal.pulsAnstiegSamples)
                    stufe = lokal.stufenFolge[(size_t) std::min (pulsPos / kPulsChunk,
                                                                 kPulsMaxChunks - 1)];
                for (int k = 0; k < ch; ++k)
                    tdf2Lauf (wetK[k] + i, stueck, lokal.puls[(size_t) stufe], pulsZust[k]);
                huell = (float) stufe / (float) (kPulsStufen - 1);
                pulsPos += stueck;
                if (periode > 0 && pulsPos >= periode)
                    pulsPos -= periode;
                i += stueck;
            }
        }

        // Linearer Crossfade Richtung Ziel (korrelierte Signale, B5).
        const double schritt = lokal.fadeSamples > 0 ? 1.0 / (double) lokal.fadeSamples : 1.0;
        const double zielWert = zielAn ? 1.0 : 0.0;
        double fEnde = fade;
        for (int k = 0; k < ch; ++k)
        {
            float* aus = puffer.getWritePointer (k);
            const float* w = wetK[k];
            double f = fade;
            for (int i = 0; i < n; ++i)
            {
                if (f < zielWert)      f = std::min (zielWert, f + schritt);
                else if (f > zielWert) f = std::max (zielWert, f - schritt);
                if (f <= 0.0 && zielWert == 0.0)
                    break;             // Rest des Blocks bleibt wörtlich der Eingang
                aus[i] = (float) ((double) aus[i] + f * ((double) w[i] - (double) aus[i]));
            }
            fEnde = f;
        }
        fade = fEnde;
        if (fade <= 0.0 && ! zielAn)
        {
            resetZustaende();
            pulsPos = 0;
        }
        hoerbarAtomic.store (fade > 0.001, std::memory_order_relaxed);
        phaseAtomic.store (lokal.modus == MarkierungsModus::puls ? huell * (float) fade
                                                                 : (float) fade,
                           std::memory_order_relaxed);
    }

    void setzeSamplerate (double fs) { fsAktuell = fs; }   // prepareToPlay-Kontext

    bool  hoerbar() const     { return hoerbarAtomic.load (std::memory_order_relaxed); }
    float phase() const       { return phaseAtomic.load (std::memory_order_relaxed); }
    bool  zielGesetzt() const { return zielGesetztAtomic.load (std::memory_order_relaxed); }

private:
    struct Zust { double s1 = 0.0, s2 = 0.0; };

    static void tdf2Lauf (float* d, int n, const BiquadKoeff& c, Zust& z)
    {
        double s1 = z.s1, s2 = z.s2;
        for (int i = 0; i < n; ++i)
        {
            const double x = (double) d[i];
            const double y = c.b0 * x + s1;
            s1 = c.b1 * x - c.a1 * y + s2;
            s2 = c.b2 * x - c.a2 * y;
            d[i] = (float) y;
        }
        z.s1 = s1; z.s2 = s2;
    }

    void resetZustaende()
    {
        for (auto& kanal : zust)
            for (auto& s : kanal)
                s = Zust {};
        pulsZust[0] = pulsZust[1] = Zust {};
    }
    void hartAus()
    {
        fade = 0.0;
        pulsPos = 0;
        resetZustaende();
    }

    // Publikation (Message-Thread → Audiothread)
    std::array<MarkierungsAuftrag, 4> ring {};
    std::atomic<std::uint32_t> veroeffentlicht { 0 };
    std::atomic<bool> zielGesetztAtomic { false };

    // Nur Audiothread
    MarkierungsAuftrag lokal {};
    bool ausGewuenscht = false;      // Aus-Auftrag: Ziel 0, Konfiguration bleibt für den Fade
    std::uint32_t gelesenNr = 0;
    Zust zust[2][kMarkierungMaxSektionen] {};
    Zust pulsZust[2] {};
    double fade = 0.0;
    int pulsPos = 0;
    double fsAktuell = 0.0;

    juce::HeapBlock<float> wet;
    int wetKapazitaet = 0;

    // Audiothread → UI/Heartbeat
    std::atomic<bool>  hoerbarAtomic { false };
    std::atomic<float> phaseAtomic { 0.0f };
};

} // namespace eqcop
