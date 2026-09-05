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
        // §7.1 E-01: der Oversize-Riegel gilt "bis zum naechsten
        // prepareToPlay". Der ruft beide Vorbereiter, also loesen ihn auch
        // beide — sonst haengte die Verriegelung daran, welchen von zweien
        // der Prozessor zuerst ruft.
        oversizeRiegel = false;
        warHoerbar = false;
        hoerbareSamples = 0;
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

    /** Was ein Block an der Hoerbarkeit geaendert hat (SONDE-013 M-37/M-38).

        Die Markierung kennt den Interventionsring nicht — sie MELDET nur,
        wann sie zu klingen begann und wann sie still wurde, und der
        Prozessor macht daraus die zwei P0-Ereignisse. Eine Markierung, die
        selbst sendete, waere im Audiothread nicht mehr pruefbar (§48.1). */
    struct Schritt
    {
        bool begann { false };   ///< in diesem Block wurde sie hoerbar
        bool endete { false };   ///< in diesem Block wurde sie still
        /// Wie viele Samples sie im laufenden Eingriff insgesamt klang.
        /// Nur bei `endete` gefuellt; die Grundlage des konservativen
        /// `tail_samples` (§34.2).
        std::uint64_t dauerSamples { 0 };
        /// SONDE-013 M-38/M-52: der Offset des LETZTEN gefaerbten Samples
        /// innerhalb dieses Blocks. Nur bei `endete` gefuellt.
        ///
        /// ⚠️ Ohne ihn stempelte der Prozessor `project_sample_end` auf den
        /// BLOCKANFANG, und die Invalidierung liesse bis zu einen ganzen
        /// Hostblock gefaerbtes Audio ausserhalb der Quarantaene — genau der
        /// Bereich, in dem der Ausfade lief. Der Offset ist relativ zum
        /// Blockanfang, weil nur der Prozessor die Projektzeit kennt.
        int endeOffsetSamples { 0 };
    };

    // Audiothread. erlaubt = Echtzeit bewiesen ∧ Transport spielt (falls
    // vorhanden) ∧ Aufnahme nachweislich AUS ∧ kein Offline-Render ∧ Editor
    // offen.
    //
    // 🔑 NAK-47 (SONDE-013 M-34): bei `!erlaubt` wird NICHT mehr geschnitten.
    // `hartAus()` setzte `fade = 0` sofort, und das Signal sprang im naechsten
    // Block abrupt vom gefilterten auf den Originalpfad — ein hoerbarer Klick
    // und ein Bruch des Startbudgets §49.3 ("A/B-Zustandswechsel: kein
    // Klick"). Statt dessen laeuft die vorhandene Rampe zu Ende; ERST danach
    // ist der Pfad wieder bit-transparent (M-35).
    //
    // ⚠️ Das ist kein Aufweichen von Gate 1. Gate 1 sagt: AUSGESCHALTET ist
    // der Pfad bitidentisch. Waehrend des Ausfades ist er nicht
    // ausgeschaltet, sondern wird es gerade — und danach faesst ihn niemand
    // mehr an (`return` vor jedem Schreibzugriff, wie vorher).
    Schritt verarbeite (juce::AudioBuffer<float>& puffer, int kanaele, bool erlaubt)
    {
        Schritt uebergang;
        const int n = puffer.getNumSamples();
        if (n <= 0 || kanaele <= 0)
            return uebergang;

        // ── Oversize (§7.1, E-01) ────────────────────────────────────────
        //
        // Ein Hostblock ueber `wetKapazitaet` kann der Wet-Pfad nicht
        // vollstaendig rechnen. Frueher hiess das `hartAus()` — derselbe
        // Klick wie oben, nur an einer zweiten Stelle. Der Entscheid E-01
        // lautet: erzwungener Ausfade INNERHALB der Kapazitaet, danach
        // Riegel bis `prepareToPlay`.
        //
        // Der Riegel ist der Teil, der leicht zu vergessen waere: ohne ihn
        // blendete die Markierung nach jedem Oversizeblock wieder ein und
        // beim naechsten wieder aus — ein Flattern, das schlimmer ist als
        // der Schnitt.
        const bool oversize = n > wetKapazitaet;
        if (oversize)
        {
            oversizeRiegel = true;
            if (fade <= 0.0)
            {
                // Nichts klingt: der Block bleibt vollstaendig unangetastet.
                hoerbarAtomic.store (false, std::memory_order_relaxed);
                phaseAtomic.store (0.0f, std::memory_order_relaxed);
                return uebergang;
            }
        }
        // Ab hier ist `nutzbar` die Zahl der Samples, die der Wet-Pfad
        // wirklich rechnen kann. Bei einem Oversizeblock sind das die ersten
        // `wetKapazitaet`; der Rest bleibt woertlich der Eingang.
        const int nutzbar = oversize ? wetKapazitaet : n;
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
                         && ! oversizeRiegel        // E-01: Ziel 0 und kein Wiedereinblenden
                         && erlaubt                 // NAK-47: Erlaubnisverlust = Ziel 0
                         && lokal.modus != MarkierungsModus::aus
                         && lokal.sektionen > 0
                         && lokal.fs == fsAktuell;

        if (! zielAn && fade <= 0.0)
        {
            // Wirklich still. Der Puffer bleibt unangetastet ⇒
            // bit-transparent (Gate 1, M-35). Das gilt fuer beide Wege
            // hierher: nie eingeschaltet ODER Ausfade abgeschlossen.
            if (pulsPos != 0)
                hartAus();
            // Auch hier kann ein Ende faellig sein: der letzte Block hat den
            // Fade auf 0 gebracht, und DIESER stellt fest, dass nichts mehr
            // klingt. Ohne die zwei Zeilen bliebe das Taintintervall offen.
            if (warHoerbar)
            {
                uebergang.endete = true;
                uebergang.dauerSamples = hoerbareSamples;
                hoerbareSamples = 0;
                warHoerbar = false;
            }
            hoerbarAtomic.store (false, std::memory_order_relaxed);
            phaseAtomic.store (0.0f, std::memory_order_relaxed);
            return uebergang;
        }

        // Wet rechnen (Kopie des Eingangs; Abgriff/Messung liegen davor).
        const int ch = std::min (kanaele, 2);
        float* wetK[2] = { wet.getData(), wet.getData() + wetKapazitaet };
        for (int k = 0; k < ch; ++k)
            std::memcpy (wetK[k], puffer.getReadPointer (k), (size_t) nutzbar * sizeof (float));

        // NAK-180 R4: je Blocklauf EINE Zahl, EINE Addition aufs Atomic.
        std::uint64_t nichtEndlichImBlock = 0;
        float huell = 1.0f;
        if (lokal.modus == MarkierungsModus::solo)
        {
            for (int k = 0; k < ch; ++k)
                for (int s = 0; s < lokal.sektionen; ++s)
                    tdf2Lauf (wetK[k], nutzbar, lokal.statisch[(size_t) s], zust[k][s],
                              nichtEndlichImBlock);
        }
        else
        {
            // Puls: Bell-Koeffizienten je 32er-Chunk aus der Stufenfolge —
            // keine Transzendente, Zustände laufen über Stufenwechsel weiter.
            const int periode = lokal.pulsAnstiegSamples + lokal.pulsRuheSamples;
            int i = 0;
            while (i < nutzbar)
            {
                const int bisRaster = kPulsChunk - (pulsPos % kPulsChunk);
                const int stueck = std::min (nutzbar - i, bisRaster);
                int stufe = 0;
                if (periode > 0 && pulsPos < lokal.pulsAnstiegSamples)
                    stufe = lokal.stufenFolge[(size_t) std::min (pulsPos / kPulsChunk,
                                                                 kPulsMaxChunks - 1)];
                for (int k = 0; k < ch; ++k)
                    tdf2Lauf (wetK[k] + i, stueck, lokal.puls[(size_t) stufe], pulsZust[k],
                              nichtEndlichImBlock);
                huell = (float) stufe / (float) (kPulsStufen - 1);
                pulsPos += stueck;
                if (periode > 0 && pulsPos >= periode)
                    pulsPos -= periode;
                i += stueck;
            }
        }

        // Linearer Crossfade Richtung Ziel (korrelierte Signale, B5).
        //
        // 🔑 SONDE-013 M-36 (§7.1 E-01): auf dem Oversize-Pfad ist die
        // Rampenlaenge `min(fadeSamples, wetKapazitaet)` — NICHT die volle
        // Fadelaenge. Mit der vollen Laenge reichten die `wetKapazitaet`
        // rechenbaren Samples nicht aus, um `fade` auf 0 zu bringen: bei
        // 48 kHz, 80 ms Fade (3840 Samples) und 512 Samples Kapazitaet blieb
        // ein Wet-Anteil von rund 0,867 stehen, und die FOLGEbloecke waren
        // weiterhin gefaerbt. E-01 verlangt aber "Rest des Blocks und jeder
        // Folgeblock sind bitidentischer Neutralpfad". Die Kuerzung macht die
        // Rampe steiler, nicht laenger — und genau das ist der Entscheid.
        const int rampe = oversize ? std::min (lokal.fadeSamples, wetKapazitaet)
                                   : lokal.fadeSamples;
        const double schritt = rampe > 0 ? 1.0 / (double) rampe : 1.0;
        const double zielWert = zielAn ? 1.0 : 0.0;
        double fEnde = fade;
        // Index des letzten Samples MIT Wet-Anteil; -1 heisst "keines".
        int letztesGemischt = -1;
        for (int k = 0; k < ch; ++k)
        {
            float* aus = puffer.getWritePointer (k);
            const float* w = wetK[k];
            double f = fade;
            int letztes = -1;
            for (int i = 0; i < nutzbar; ++i)
            {
                if (f < zielWert)      f = std::min (zielWert, f + schritt);
                else if (f > zielWert) f = std::max (zielWert, f - schritt);
                if (f <= 0.0 && zielWert == 0.0)
                    break;             // Rest des Blocks bleibt wörtlich der Eingang
                aus[i] = (float) ((double) aus[i] + f * ((double) w[i] - (double) aus[i]));
                letztes = i;
            }
            fEnde = f;
            if (letztes > letztesGemischt)
                letztesGemischt = letztes;
        }
        fade = fEnde;

        // ── SONDE-013 M-37/M-38: die zwei Uebergaenge ────────────────────
        //
        // Hoerbar heisst hier: der Wet-Anteil ist von null verschieden —
        // EXAKT null, nicht "unter 0,001".
        //
        // ⚠️ Die frueher hier stehende Schwelle 0,001 meldete das Ende, waehrend
        // die Folgebloecke noch Wet mischten: die Rampe laeuft linear bis 0,
        // und zwischen 0,001 und 0 liegen bei 80 ms Fade noch knapp vier
        // Samples plus jeder weitere Block, den die Rampe braucht. Ein Ende zu
        // frueh gemeldet heisst den Taint freigeben, waehrend noch gefaerbtes
        // Audio laeuft (M-38). Die Rampe erreicht die 0 exakt, weil
        // `std::max (zielWert, f - schritt)` auf `zielWert` klemmt — es gibt
        // also keinen Restwert, an dem ein Vergleich haengenbliebe.
        // Das `hoerbar()`-Bit fuehrt dieselbe Schwelle, damit Ereignis und
        // Anzeige nicht auseinanderlaufen koennen.
        const bool istHoerbar = fade > 0.0;
        if (istHoerbar && ! warHoerbar)
        {
            uebergang.begann = true;
            hoerbareSamples = 0;
        }
        if (istHoerbar)
            hoerbareSamples += (std::uint64_t) nutzbar;
        if (! istHoerbar && warHoerbar)
        {
            // ⚠️ Das Ende faellt HIER und nicht beim Erlaubnisverlust: M-38
            // verlangt "der Marker endet erst nach abgeschlossenem Ausfade
            // (M-34), nicht bei Verlust der Erlaubnis". Ein Ende zu frueh
            // gemeldet, hiesse einen Bereich freizugeben, in dem noch
            // gefaerbtes Audio lief.
            uebergang.endete = true;
            uebergang.dauerSamples = hoerbareSamples;
            // M-38/M-52: der gemeldete Bereich endet am letzten gefaerbten
            // Sample dieses Blocks. `letztesGemischt` ist -1, wenn der Block
            // gar nichts mischte (die Rampe war schon beim ersten Sample auf
            // 0) — dann ist der Blockanfang die richtige Grenze.
            uebergang.endeOffsetSamples = letztesGemischt >= 0 ? letztesGemischt : 0;
            hoerbareSamples = 0;
        }
        warHoerbar = istHoerbar;

        if (fade <= 0.0 && ! zielAn)
        {
            resetZustaende();
            pulsPos = 0;
        }
        hoerbarAtomic.store (istHoerbar, std::memory_order_relaxed);
        phaseAtomic.store (lokal.modus == MarkierungsModus::puls ? huell * (float) fade
                                                                 : (float) fade,
                           std::memory_order_relaxed);
        // NAK-180 R4: EINE Addition je Blocklauf, ohne Sperre und ohne
        // Allokation. Vorbild ist `nanSeen` im Prozessor; anders als dort
        // wird nicht nur ein Bit gesetzt, sondern gezaehlt — die Zahl
        // unterscheidet einen einzelnen Ausrutscher von einem Dauerzustand.
        if (nichtEndlichImBlock != 0)
            wetNichtEndlich.fetch_add (nichtEndlichImBlock, std::memory_order_relaxed);
        return uebergang;
    }

    /** prepareToPlay-Kontext. Loest zugleich den Oversize-Riegel (§7.1 E-01):
        eine neue Blockgroesse ist genau der Moment, in dem der Grund fuer die
        Verriegelung entfaellt. */
    void setzeSamplerate (double fs)
    {
        fsAktuell = fs;
        oversizeRiegel = false;
        warHoerbar = false;
        hoerbareSamples = 0;
    }

    /** NAK-180 R4: wie viele nicht-endliche Zwischenwerte der Wet-Pfad
        verriegelt hat. Kumulativ ueber die Lebenszeit des Prozessors; ein
        Reset waere ein Leser, der seine eigene Vergangenheit loescht. */
    std::uint64_t nichtEndlicheWetSamples() const
    {
        return wetNichtEndlich.load (std::memory_order_relaxed);
    }

    bool  hoerbar() const     { return hoerbarAtomic.load (std::memory_order_relaxed); }
    float phase() const       { return phaseAtomic.load (std::memory_order_relaxed); }
    bool  zielGesetzt() const { return zielGesetztAtomic.load (std::memory_order_relaxed); }

private:
    struct Zust { double s1 = 0.0, s2 = 0.0; };

    /** NAK-180 R4: der Wet-Pfad latcht NIE einen nicht-endlichen Zustand.

        Vorher genuegte ein einziges +-Inf im Hostpuffer, um den Filter
        dauerhaft zu vergiften: `y = b0*x + s1` wurde Inf, und in DERSELBEN
        Iteration ergab `s1 = b1*x - a1*y + s2` genau `Inf - Inf = NaN`. Ab da
        lieferte jede weitere Iteration NaN, unabhaengig vom Eingang, und die
        Mischzeile schrieb das in den Hostpuffer, solange der Marker engagiert
        blieb — `resetZustaende()` laeuft erst bei `fade <= 0 && ! zielAn`.

        DREI Riegel, nicht einer (Matrix E5):
          - EINGANG: ein nicht-endliches Sample geht als 0.0 ins Filter.
          - AUSGANG: ein nicht-endliches `y` wird als 0.0f geschrieben, statt
            in die Wet-Kopie zu gelangen.
          - ZUSTAND: nach dem Block faellt ein nicht-endliches s1/s2 auf 0.
        Der Eingangsriegel allein genuegt nicht: `y` kann auch bei endlichem
        `x` nicht endlich werden, wenn `s1` es aus einem frueheren Block schon
        war — und genau diese Verkettung IST der Latch.

        Gezaehlt wird in `n`, einer lokalen Variable des Aufrufers; das Atomic
        bekommt EINE Addition je Blocklauf, nie eine je Sample. Bei endlichem
        Material aendert der Riegel kein Bit: `isfinite` ist dann wahr und die
        Konvertierung dieselbe wie zuvor (Nulltest und Goldens unberuehrt). */
    static void tdf2Lauf (float* d, int n, const BiquadKoeff& c, Zust& z,
                          std::uint64_t& nichtEndlich)
    {
        double s1 = z.s1, s2 = z.s2;
        for (int i = 0; i < n; ++i)
        {
            double x = (double) d[i];
            if (! std::isfinite (x)) { x = 0.0; ++nichtEndlich; }
            const double y = c.b0 * x + s1;
            s1 = c.b1 * x - c.a1 * y + s2;
            s2 = c.b2 * x - c.a2 * y;
            if (std::isfinite (y))
                d[i] = (float) y;
            else
                { d[i] = 0.0f; ++nichtEndlich; }
        }
        if (! std::isfinite (s1)) { s1 = 0.0; ++nichtEndlich; }
        if (! std::isfinite (s2)) { s2 = 0.0; ++nichtEndlich; }
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
    /// SONDE-013 M-37/M-38: der Zustand, aus dem die zwei Uebergaenge
    /// entstehen, und die Dauer des laufenden Eingriffs in Samples.
    bool          warHoerbar = false;
    std::uint64_t hoerbareSamples = 0;
    /// §7.1 E-01: nach einem Oversizeblock bleibt die Markierung bis zum
    /// naechsten `prepareToPlay` verriegelt. Ein erneutes `erlaubt` blendet
    /// in dieser Laufzeit nicht wieder ein — sonst flatterte sie bei
    /// wiederholten Oversizebloecken.
    bool oversizeRiegel = false;

    juce::HeapBlock<float> wet;
    int wetKapazitaet = 0;

    // Audiothread → UI/Heartbeat
    std::atomic<bool>  hoerbarAtomic { false };
    // NAK-180 R4: Riegelzaehler des Wet-Pfads (Eingang, Ausgang, Zustand).
    std::atomic<std::uint64_t> wetNichtEndlich { 0 };
    std::atomic<float> phaseAtomic { 0.0f };
};

} // namespace eqcop
