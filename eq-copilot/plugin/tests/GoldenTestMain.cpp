// M1-Golden-Test: Kreuzvalidierung der AnalyseEngine gegen die eingefrorene
// Referenz des Offline-Analyzers (fixtures/golden-referenz.json, erzeugt aus
// tools/analyze-track.py). Plan §11 M1-Abnahme.
//
// Aufruf:  EqCopGoldenTest <pfad-zu-eq-copilot/fixtures>
// Die WAVs sind nicht eingecheckt — vorher einmal
//   py -3.13 tools/eq-copilot/erzeuge_fixtures.py --nur-wav
// laufen lassen. SHA-256 wird gegen das JSON geprüft (Determinismus-Riegel).
//
// Toleranz-Grundsätze (kMetricsVersion, dokumentiert je Prüfung):
//  · Skalare + 8192er-Referenz-LTAS laufen auf EXAKT der analyze-track-Achse
//    → enge Toleranzen.
//  · Die Komposit-Kurve (§5.10.1 Mehrfachauflösung) wird nur bei
//    BREITBAND-Inhalt kurvenverglichen: Schmalband-Bandpegel sind physikalisch
//    auflösungsabhängig (ENBW/Hauptkeule) — beim Sinus zählt stattdessen der
//    strukturelle Peak-Check, beim 116-Hz-Fall wird ±1/6 Okt um den Ton
//    ausgenommen. Alles andere wäre Scheingenauigkeit.
//  · Flatness ist bewusst NUR eine Größenordnungs-Prüfung: die Referenz misst
//    auf librosas 22050-Hz-Achse mit deren amin-Boden, die Engine auf der
//    eigenen 48-kHz-Achse.
//  · stille-mit-tick beweist die Produktregel: die Referenz LISTET dort sechs
//    Kleinstpegel-"Resonanzen" — die Engine muss schweigen (§5.3/§5.10.2).
#include <juce_core/juce_core.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_cryptography/juce_cryptography.h>
#include "AnalyseEngine.h"
#include "Diagnose.h"
#include "Nak380Pruefsignale.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

using namespace eqcop;

namespace eqcop
{
/** Rein lesender Testzugang fuer T-380-4. Die Produktklasse benennt ihn als
    Freund; kein Test schreibt einen Analysezustand durch diesen Zugang. */
struct AnalyseEngineTestzugang
{
    static std::uint64_t segmentHistogrammSumme (const AnalyseEngine& e, int band)
    {
        std::uint64_t summe = 0;
        const auto ab = (std::size_t) band * AnalyseEngine::kHistStufen;
        for (int i = 0; i < AnalyseEngine::kHistStufen; ++i)
            summe += e.pegelHistogramm[ab + (std::size_t) i];
        return summe;
    }

    static std::uint64_t teilblockHistogrammSumme (const AnalyseEngine& e, int band)
    {
        std::uint64_t summe = 0;
        const auto ab = (std::size_t) band * AnalyseEngine::kHistStufen;
        for (int i = 0; i < AnalyseEngine::kHistStufen; ++i)
            summe += e.teilblockHistogramm[ab + (std::size_t) i];
        return summe;
    }

    static std::uint64_t zustaendigeSegmente (const AnalyseEngine& e, int band)
    {
        const auto& akku = band < e.idxZust200 ? e.bassAkku
                         : band < e.idxZust2000 ? e.mittenAkku : e.hoehenAkku;
        return akku.segmente[(std::size_t) band];
    }

    static std::uint32_t teilblockRest (const AnalyseEngine& e, int band)
    {
        return e.teilblockSegmente[(std::size_t) band];
    }

    static double teilblockRestsumme (const AnalyseEngine& e, int band)
    {
        return e.teilblockSumme[(std::size_t) band];
    }

    static std::size_t zusaetzlicherSpeicherBytes (const AnalyseEngine& e)
    {
        return e.teilblockHistogramm.capacity() * sizeof (juce::uint32)
             + e.teilblockSumme.capacity() * sizeof (double)
             + e.teilblockSegmente.capacity() * sizeof (juce::uint32);
    }

    static std::vector<double> abdeckungAusSegmenthistogramm (const AnalyseEngine& e)
    {
        std::vector<double> aus;
        for (int start = 0; start < kLtasBaender; start += 8)
        {
            const int ende = std::min (start + 8, kLtasBaender);
            double teppich = -90.0; // kBodenDb, kalibrierte Produktkonstante
            double minInaktiv = 1e9;
            for (int b = start; b < ende; ++b)
                minInaktiv = std::min (minInaktiv, e.teppichInaktivDb[(std::size_t) b]);
            if (minInaktiv < 1e8)
                teppich = std::max (-90.0, minInaktiv);
            const int schwellBin = juce::jlimit (0, AnalyseEngine::kHistStufen - 1,
                (int) std::lround (teppich + 6.0) - AnalyseEngine::kHistMinDb);
            std::uint64_t gesamt = 0, ueber = 0;
            for (int b = start; b < ende; ++b)
                for (int bin = 0; bin < AnalyseEngine::kHistStufen; ++bin)
                {
                    const auto h = e.pegelHistogramm[(std::size_t) b
                                     * AnalyseEngine::kHistStufen + (std::size_t) bin];
                    gesamt += h;
                    if (bin > schwellBin) ueber += h;
                }
            aus.push_back (gesamt > 0u ? (double) ueber / (double) gesamt : 0.0);
        }
        return aus;
    }

    // ── NAK-380 Etappe 6 (T-380-11), nur lesend ─────────────────────────────
    // M-99: Punktzahl und Hop der vier Stufen (0 Bass, 1 Referenz, 2 Mitten,
    // 3 Hoehen).
    static const AnalyseEngine::WelchStufe& stufe (const AnalyseEngine& e, int s)
    {
        return s == 0 ? e.bass : s == 1 ? e.referenz : s == 2 ? e.mitten : e.hoehen;
    }
    static int punkte (const AnalyseEngine& e, int s) { return stufe (e, s).n; }
    static int hop (const AnalyseEngine& e, int s)    { return stufe (e, s).hop; }

    // M-101: die eigenen Vektoren einer Stufe, Kapazitaet mal Elementgroesse
    // (ringL, ringR, fftDaten, summePsd, fenster); die inneren Tafeln von
    // juce::dsp::FFT zaehlen nicht (Matrix §6.5).
    static std::uint64_t stufeHeapBytes (const AnalyseEngine& e, int s)
    {
        const auto& w = stufe (e, s);
        return (std::uint64_t) (w.ringL.capacity() * sizeof (float) + w.ringR.capacity() * sizeof (float)
                                + w.fftDaten.capacity() * sizeof (float) + w.summePsd.capacity() * sizeof (double)
                                + w.fenster.capacity() * sizeof (float));
    }
    static std::uint64_t histogrammBytes (const AnalyseEngine& e)
    {
        return (std::uint64_t) (e.pegelHistogramm.capacity() + e.teilblockHistogramm.capacity())
             * sizeof (juce::uint32);
    }
};
}

namespace
{
/** NAK-380 Etappe 6 (M-106 bis M-109): die neuen Traeger entstehen erst mit dem
    Bau. Damit derselbe Test gegen den unveraenderten Basisstand uebersetzt
    (Gegenprobe §8.1), liest er sie ueber Anfragen, die ohne den Traeger
    „fehlt“ melden - fail-closed: fehlt der Traeger, ist die Pruefung rot. */
template <typename M>
double nak380Suchgrenze (const M& m)
{
    if constexpr (requires { m.resonanzSucheAbHz; })
        return m.resonanzSucheAbHz;
    else
        return std::numeric_limits<double>::quiet_NaN();
}

template <typename M>
juce::String nak380Satz (const M& m)
{
    if constexpr (requires { suchgrenzeSatz (m); })
        return suchgrenzeSatz (m);
    else
        return {};
}
} // namespace

struct Pruefer
{
    int fehler = 0, ok = 0;
    juce::String signal;

    // NAK-380 Rotbeweise (§8.1): mit gesetzter Umgebungsvariable
    // NAK380_ALLE_PRUEFUNGEN nennt der Lauf auch jede bestandene Prüfung.
    static bool alleZeigen()
    {
        static const bool an = juce::SystemStats::getEnvironmentVariable (
                                   "NAK380_ALLE_PRUEFUNGEN", {}).isNotEmpty();
        return an;
    }

    void melde (bool bestanden, const juce::String& was, const juce::String& detail)
    {
        if (bestanden)
        {
            ++ok;
            if (alleZeigen())
                std::printf ("  ok [%s] %s\n", signal.toRawUTF8(), was.toRawUTF8());
            return;
        }
        ++fehler;
        std::printf ("  FEHLER [%s] %s — %s\n", signal.toRawUTF8(), was.toRawUTF8(), detail.toRawUTF8());
    }
    void nahe (double ist, double soll, double tol, const juce::String& was)
    {
        melde (std::isfinite (ist) && std::abs (ist - soll) <= tol, was,
               "ist " + juce::String (ist, 4) + ", soll " + juce::String (soll, 4)
               + " (Toleranz " + juce::String (tol, 3) + ")");
    }
    void relativ (double ist, double soll, double relTol, const juce::String& was)
    {
        melde (std::isfinite (ist) && std::abs (ist - soll) <= relTol * std::abs (soll), was,
               "ist " + juce::String (ist, 4) + ", soll " + juce::String (soll, 4)
               + " (rel. Toleranz " + juce::String (relTol * 100.0, 1) + " %)");
    }
    void wahr (bool cond, const juce::String& was, const juce::String& detail = "Bedingung verletzt")
    {
        melde (cond, was, detail);
    }
};

// Kurvenvergleich über Bänder, deren Referenzwert über dem Boden liegt.
static void vergleicheKurve (Pruefer& p, const juce::String& name,
                             const std::array<double, kLtasBaender>& ist,
                             const std::vector<double>& soll,
                             double meanTol, double maxTol,
                             double bodenRelDb = 70.0,
                             double ausnahmeZentrumHz = 0.0,
                             double ausnahmeOktaven = 0.0,
                             const std::array<double, kLtasBaender>* zentren = nullptr)
{
    double refMax = -1e18;
    for (const double v : soll)
        if (std::isfinite (v))
            refMax = std::max (refMax, v);

    double summe = 0.0, groesste = 0.0;
    int nVergleich = 0, groesstesBand = -1;
    for (size_t b = 0; b < soll.size() && b < (size_t) kLtasBaender; ++b)
    {
        if (! std::isfinite (soll[b]) || soll[b] <= refMax - bodenRelDb)
            continue;
        if (ausnahmeZentrumHz > 0.0 && zentren != nullptr
            && std::abs (std::log2 ((*zentren)[b] / ausnahmeZentrumHz)) <= ausnahmeOktaven)
            continue;
        const double istWert = ist[b];
        if (std::isnan (istWert))
        {
            p.melde (false, name, "Band " + juce::String ((int) b) + " ist NaN, Referenz "
                                  + juce::String (soll[b], 2));
            return;
        }
        const double delta = std::abs (istWert - soll[b]);
        summe += delta;
        if (delta > groesste)
        {
            groesste = delta;
            groesstesBand = (int) b;
        }
        ++nVergleich;
    }
    // Sicherung gegen Leerlauf-Vergleich; ein reiner Sinus hat nur ~5 Bänder
    // über dem Boden — die Mindestzahl muss vom Signal abhängen dürfen.
    p.wahr (nVergleich >= 3, name + " Abdeckung",
            "nur " + juce::String (nVergleich) + " vergleichbare Baender");
    if (nVergleich > 0)
    {
        juce::String wo;
        if (groesstesBand >= 0 && zentren != nullptr)
            wo = " bei " + juce::String ((*zentren)[(size_t) groesstesBand], 1) + " Hz";
        else if (groesstesBand >= 0)
            wo = " (Band " + juce::String (groesstesBand) + ")";
        p.melde (summe / nVergleich <= meanTol, name + " mittl. Abw.",
                 juce::String (summe / nVergleich, 3) + " dB > " + juce::String (meanTol, 2) + " dB");
        p.melde (groesste <= maxTol, name + " max. Abw.",
                 juce::String (groesste, 3) + " dB > " + juce::String (maxTol, 2) + " dB" + wo
                 + " (ist " + juce::String (groesstesBand >= 0 ? ist[(size_t) groesstesBand] : 0.0, 2)
                 + ", soll " + juce::String (groesstesBand >= 0 ? soll[(size_t) groesstesBand] : 0.0, 2) + ")");
    }
}

// ── M3-Diagnose: strukturelle Prüfungen auf konstruierten Fehlern ──────────
// Beweisidee: die diag-*-WAVs tragen GENAU EINEN eingebauten Fehler (Gauß-
// Glocke in log-f auf Pink) — die Diagnose muss genau seine Befundklasse
// liefern; das unveränderte Pink und die Stille bleiben karten-still
// (Falsch-Positiv-Riegel). Jede Karte muss die §7.3/§7.4-Struktur füllen.
static bool hatKlasse (const std::vector<Befund>& bs, BefundKlasse k)
{
    for (const auto& b : bs)
        if (b.klasse == k)
            return true;
    return false;
}

static void pruefeKartenStruktur (Pruefer& p, const std::vector<Befund>& bs)
{
    for (const auto& b : bs)
    {
        p.wahr (b.titel.isNotEmpty() && b.gemessen.isNotEmpty() && b.wirkung.isNotEmpty()
                    && b.tu.isNotEmpty() && b.warum.isNotEmpty() && b.hoeren.isNotEmpty(),
                "Karte vollstaendig (§7.3)", "leeres Feld in: " + b.titel);
        p.wahr (b.gemessen.containsChar ('d') && b.gemessen.contains ("dB"),
                "Gemessen traegt Zahlen", b.gemessen);
        p.wahr (b.hoeren.contains ("Stopp"), "Hoeren hat Stop-Bedingung (§7.4)", b.hoeren);
        p.wahr (b.konfidenzGrund.isNotEmpty(), "Konfidenz benannt", b.titel);
    }
}

static void diagnosePruefungen (Pruefer& p, const juce::String& name, const MessSnapshot& m)
{
    const auto bs = diagnose (m, "hub");

    if (name == "stille-mit-tick")
    {
        p.wahr (bs.empty(), "Diagnose still bei Stille",
                juce::String ((int) bs.size()) + " Karten");
        return;
    }
    pruefeKartenStruktur (p, bs);

    if (name == "pink-minus20")
    {
        // DER Falsch-Positiv-Riegel: neutrales Breitband ⇒ null Karten.
        p.wahr (bs.empty(), "Falsch-Positiv-Riegel: Pink ohne Karten",
                juce::String ((int) bs.size()) + " Karten (erste: "
                + (bs.empty() ? juce::String() : bs.front().titel) + ")");
    }
    else if (name == "sinus-1k-minus12")
    {
        // Sinus: genau die Resonanz-Karte; die Zonen-Gates (Abdeckung §5.10.2)
        // müssen alle Balance-Regeln schlucken.
        p.wahr (bs.size() == 1 && bs.front().klasse == BefundKlasse::resonanz,
                "Sinus: genau 1 Resonanz-Karte",
                juce::String ((int) bs.size()) + " Karten");
    }
    else if (name == "resonanz-116hz")
    {
        p.wahr (hatKlasse (bs, BefundKlasse::resonanz), "116-Hz-Karte vorhanden", "fehlt");
        p.wahr (! hatKlasse (bs, BefundKlasse::mittenLoch)
                    && ! hatKlasse (bs, BefundKlasse::haerte),
                "keine Fremdklassen am 116er", "Scoop/Haerte gemeldet");
        for (const auto& b : bs)
            if (b.klasse == BefundKlasse::resonanz)
            {
                // Frequenz = Band-Center, nie feiner als das Fenster (§5.10.3)
                // — deshalb ±1/6 Okt um 116, nicht wörtlich „116".
                p.wahr (std::abs (std::log2 (b.fSchwerpunkt / 116.0)) <= 1.0 / 6.0,
                        "Karte am 116er (Bassfenster)", juce::String (b.fSchwerpunkt, 1) + " Hz");
                const juce::String fWort = juce::String ((int) std::lround (b.fSchwerpunkt)) + " Hz";
                p.wahr (b.titel.contains (fWort), "Titel nennt die Frequenz", b.titel);
                // M3a: musikalische Verortung in FL-Zählung (116 Hz = A#3;
                // MIDI 60 = C5 — Landmine tools/midi-ideen).
                p.wahr (b.titel.contains ("A#3"), "Titel nennt die Note (FL-Zaehlung)", b.titel);
                p.wahr (b.titel.contains (juce::CharPointer_UTF8 ("dauerhaft")),
                        "als dauerhaft erkannt", b.titel);
                p.wahr (b.tu.contains (fWort) && b.tu.containsChar ((juce::juce_wchar) 8722 /* − */),
                        "Tu nennt Frequenz + konkreten Absenk-Startwert", b.tu);
                p.wahr (b.konfidenz == Konfidenz::hoch,
                        "messfeste Dauer-Resonanz = Konfidenz hoch", b.konfidenzGrund);
                p.wahr (b.topRang, "Resonanz ist Top-Rang", b.titel);
                p.wahr (b.gemessen.contains ("steht ruhig"),
                        "380/M-36 karte_116hz_steht_ruhig", b.gemessen);
                p.wahr (! b.tu.contains ("Smooth Operator"),
                        "380/M-36 116-Hz-Karte behaelt den festen Absenker", b.tu);
            }
    }
    else if (name == "diag-scoop-mitte")
    {
        p.wahr (hatKlasse (bs, BefundKlasse::mittenLoch), "Mitten-Loch gefunden",
                juce::String ((int) bs.size()) + " Karten, keine mittenLoch");
        p.wahr (! hatKlasse (bs, BefundKlasse::mulm) && ! hatKlasse (bs, BefundKlasse::haerte),
                "keine Fremdklassen am Scoop", "Mulm/Haerte faelschlich gemeldet");
        for (const auto& b : bs)
            if (b.klasse == BefundKlasse::mittenLoch)
            {
                p.wahr (b.staerkeDb <= -3.0, "Fehlbetrag >= Schwelle",
                        juce::String (b.staerkeDb, 2) + " dB");
                p.wahr (b.tu.contains ("Fader") || b.tu.contains ("Balance")
                            || b.tu.contains (juce::CharPointer_UTF8 ("nach vorn")),
                        "Erstidee ist Balance, nicht EQ (§5.10.6 breit+dauerhaft)", b.tu);
                p.wahr (b.konfidenz != Konfidenz::niedrig && b.topRang,
                        "Scoop erreicht Top-Rang", b.konfidenzGrund);
            }
    }
    else if (name == "diag-mulm")
    {
        p.wahr (hatKlasse (bs, BefundKlasse::mulm), "Mulm gefunden",
                juce::String ((int) bs.size()) + " Karten, kein mulm");
        p.wahr (! hatKlasse (bs, BefundKlasse::mittenLoch) && ! hatKlasse (bs, BefundKlasse::haerte),
                "keine Fremdklassen am Mulm", "Scoop/Haerte faelschlich gemeldet");
        for (const auto& b : bs)
            if (b.klasse == BefundKlasse::mulm)
            {
                p.wahr (b.staerkeDb >= 4.0, "Ueberschuss >= Schwelle",
                        juce::String (b.staerkeDb, 2) + " dB");
                p.wahr (b.fSchwerpunkt > 110.0 && b.fSchwerpunkt < 320.0,
                        "Schwerpunkt in der Zone", juce::String (b.fSchwerpunkt, 1) + " Hz");
                p.wahr (b.warum.contains ("Sustain") || b.warum.contains ("Fundament"),
                        "Warum schuetzt das Klavier-Fundament", b.warum);
            }
    }
    else if (name == "diag-haerte")
    {
        p.wahr (hatKlasse (bs, BefundKlasse::haerte), "Haerte gefunden",
                juce::String ((int) bs.size()) + " Karten, keine haerte");
        p.wahr (! hatKlasse (bs, BefundKlasse::mulm), "kein Mulm an der Haerte", "faelschlich gemeldet");
        for (const auto& b : bs)
            if (b.klasse == BefundKlasse::haerte)
                p.wahr (b.tu.contains ("Smooth Operator"),
                        "dynamische Erstidee nennt sein Werkzeug", b.tu);
    }
    else if (name == "diag-mulm-halb")
    {
        // M3a: das Zeitmittel triggert die Karte, der Zeitverlauf macht sie
        // ehrlich — „Das war in NN % der Musikzeit so." muss auf der Karte
        // stehen (Lernsprache §7.4).
        p.wahr (hatKlasse (bs, BefundKlasse::mulm), "Halbzeit-Mulm gefunden",
                juce::String ((int) bs.size()) + " Karten, kein mulm");
        p.wahr (! hatKlasse (bs, BefundKlasse::mittenLoch) && ! hatKlasse (bs, BefundKlasse::haerte),
                "keine Fremdklassen am Halbzeit-Mulm", "Scoop/Haerte faelschlich gemeldet");
        for (const auto& b : bs)
            if (b.klasse == BefundKlasse::mulm)
                p.wahr (b.gemessen.contains (juce::CharPointer_UTF8 ("% der Musikzeit")),
                        "Karte traegt den Zeitverlauf", b.gemessen);
    }
    else if (name == "diag-wander-ton")
    {
        p.wahr (hatKlasse (bs, BefundKlasse::resonanz), "Resonanz-Karte vorhanden", "fehlt");
        for (const auto& b : bs)
            if (b.klasse == BefundKlasse::resonanz)
            {
                p.wahr (! b.dauerhaft, "Karte sagt zeitweise (wandernder Ton)", b.titel);
                p.wahr (b.titel.contains (juce::CharPointer_UTF8 ("zeitweise")),
                        "Titel nennt zeitweise", b.titel);
                p.wahr (b.tu.contains ("Smooth Operator"),
                        "zeitweise ⇒ dynamische Erstidee", b.tu);
            }
    }
}

namespace
{
struct Nak380M1Lauf
{
    MessSnapshot snapshot;
    std::uint64_t samples {};
};

template <typename Generator>
std::uint64_t nak380M1SpeisenSamples (AnalyseEngine& engine, std::uint64_t samplesSoll,
                                      Generator&& naechstes)
{
    constexpr int block = 512;
    std::vector<float> inter ((std::size_t) block * 2u);
    std::uint64_t sample = 0;
    while (sample < samplesSoll)
    {
        const int n = (int) std::min<std::uint64_t> ((std::uint64_t) block,
                                                      samplesSoll - sample);
        for (int i = 0; i < n; ++i)
        {
            const float v = naechstes (sample + (std::uint64_t) i);
            inter[(std::size_t) i * 2u] = v;
            inter[(std::size_t) i * 2u + 1u] = v;
        }
        engine.verarbeite (inter.data(), n, 2);
        sample += (std::uint64_t) n;
    }
    return sample;
}

template <typename Generator>
std::uint64_t nak380M1Speisen (AnalyseEngine& engine, double dauerS,
                              Generator&& naechstes)
{
    constexpr double fs = 48000.0;
    return nak380M1SpeisenSamples (engine, (std::uint64_t) std::llround (dauerS * fs),
                                   std::forward<Generator> (naechstes));
}

template <typename Generator>
Nak380M1Lauf nak380M1Fahren (double dauerS, Generator&& naechstes)
{
    auto engine = std::make_unique<AnalyseEngine>();
    engine->vorbereiten (48000.0);
    const auto sample = nak380M1Speisen (*engine, dauerS,
                                        std::forward<Generator> (naechstes));
    engine->auswerten();
    return { engine->snapshot(), sample };
}

// Länge der Welch-Stufe, die ein Band führt (AnalyseEngine.cpp:195-196,
// :214-217): Zentrum unter 200 Hz Bass 16384, unter 2000 Hz Mitten 4096,
// sonst Höhen 2048.
int nak380StufenPunkte (int band)
{
    const double lo = 30.0 * std::pow (2.0, (double) band / 24.0);
    const double hi = 30.0 * std::pow (2.0, (double) (band + 1) / 24.0);
    const double zentrum = std::sqrt (lo * hi);
    return zentrum < 200.0 ? 16384 : (zentrum < 2000.0 ? 4096 : 2048);
}

// Segmente einer Welch-Stufe der Länge n, `samples` Samples nach Start oder
// Reset: das erste schließt nach n Samples, jedes weitere einen Hop n/2
// später (AnalyseEngine.cpp:70-71, :106-109, :157-159), also
// S = floor((samples - n)/(n/2)) + 1 und vor n Samples 0.
std::uint64_t nak380Segmente (std::uint64_t samples, int n)
{
    const auto laenge = (std::uint64_t) n;
    return samples < laenge ? 0u : (samples - laenge) / (laenge / 2u) + 1u;
}

int nak380BinsImBand (int band)
{
    constexpr double fs = 48000.0;
    const double lo = 30.0 * std::pow (2.0, (double) band / 24.0);
    const double hi = 30.0 * std::pow (2.0, (double) (band + 1) / 24.0);
    const int punkte = nak380StufenPunkte (band);
    const int von = (int) std::ceil (lo * (double) punkte / fs);
    const int bis = std::min ((int) std::ceil (hi * (double) punkte / fs), punkte / 2 + 1);
    return std::max (0, bis - von);
}

void nak380M32 (Pruefer& p)
{
    namespace sig = nakama::test::nak380;
    sig::GaussRauschen rauschen { sig::kW4Saat };
    const auto lauf = nak380M1Fahren (60.0, [&] (std::uint64_t)
    { return (float) (0.1 * rauschen.naechstes()); });
    const auto& m = lauf.snapshot;
    int belegt = 0, einBin = 0, zuBreit = 0, einBinZuBreit = 0;
    double groesste = 0.0, groessteEinBin = 0.0;
    for (int b = 0; b < kLtasBaender; ++b)
    {
        const int bins = nak380BinsImBand (b);
        if (bins <= 0)
            continue;
        ++belegt;
        const double p50 = m.perzentilP50[(std::size_t) b];
        const double p95 = m.perzentilP95[(std::size_t) b];
        const double spanne = p95 - p50;
        const bool endlich = std::isfinite (p50) && std::isfinite (p95);
        if (! endlich || spanne > 6.0) ++zuBreit;
        if (endlich) groesste = std::max (groesste, spanne);
        if (bins == 1)
        {
            ++einBin;
            if (! endlich || spanne > 4.0) ++einBinZuBreit;
            if (endlich) groessteEinBin = std::max (groessteEinBin, spanne);
        }
    }
    // Gitter/FFT-Formel aus §7.4: 196 Bänder mit Bin, davon 69 Einbinbänder.
    p.wahr (lauf.samples == 2'880'000u && belegt == 196 && einBin == 69,
            "380/M-32 Vorbedingung W4 vollstaendig",
            juce::String ((juce::int64) lauf.samples) + " Samples, "
                + juce::String (belegt) + "/" + juce::String (einBin) + " Baender");
    // ν=2304/151 für acht Hann-Segmente ergibt 2,39 dB; plus höchstens
    // 1 dB Quantisierung => Einbinbänder <=4 dB, alle belegten <=6 dB.
    p.wahr (zuBreit == 0, "380/M-32 m1_rauschen_steht_ruhig: jedes belegte Band <=6 dB",
            juce::String (zuBreit) + " von 196; max " + juce::String (groesste, 2));
    p.wahr (einBinZuBreit == 0,
            "380/M-32 m1_rauschen_steht_ruhig: jedes Einbinband <=4 dB",
            juce::String (einBinZuBreit) + " von 69; max " + juce::String (groessteEinBin, 2));
    std::printf ("380/M-32 Messung: max alle %.2f dB, max Einbin %.2f dB\n",
                 groesste, groessteEinBin);
}

void nak380Bewegung (Pruefer& p, int matrix, double breiteDb,
                     std::uint64_t rauschSaat, std::uint64_t pegelSaat,
                     double minDb, double maxDb)
{
    namespace sig = nakama::test::nak380;
    sig::GaussRauschen rauschen { rauschSaat };
    sig::GleichRauschen pegel { pegelSaat };
    double stufeDb = breiteDb * pegel.offen01();
    constexpr std::uint64_t schrittSamples = 2u * 48000u;
    const auto lauf = nak380M1Fahren (300.0, [&] (std::uint64_t n)
    {
        if (n > 0u && n % schrittSamples == 0u)
            stufeDb = breiteDb * pegel.offen01();
        return (float) (0.1 * std::pow (10.0, stufeDb / 20.0) * rauschen.naechstes());
    });
    std::vector<double> spannen;
    for (int b = 0; b < kLtasBaender; ++b)
    {
        const double zentrum = 30.0 * std::pow (2.0, ((double) b + 0.5) / 24.0);
        if (zentrum >= 200.0 && zentrum < 400.0 && nak380BinsImBand (b) == 1)
        {
            const double spanne = lauf.snapshot.perzentilP95[(std::size_t) b]
                                - lauf.snapshot.perzentilP50[(std::size_t) b];
            if (std::isfinite (spanne)) spannen.push_back (spanne);
        }
    }
    std::sort (spannen.begin(), spannen.end());
    const double median = spannen.empty() ? std::numeric_limits<double>::quiet_NaN()
                                          : spannen[spannen.size() / 2u];
    // Gleichverteilung [0,X]: wahr P95-P50 = 0,45*X. Das Intervall ist die
    // vor dem Bau festgelegte Faltung mit χ²_(2304/151) plus 1-dB-Rundung:
    // wahr-0,5 bis wahr+2,0 dB (Matrix M-33 bis M-35).
    p.wahr (lauf.samples == 14'400'000u && spannen.size() == 17u,
            "380/M-" + juce::String (matrix) + " Vorbedingung 300 s und 17 Einbinbaender",
            juce::String ((juce::int64) lauf.samples) + " Samples, "
                + juce::String ((int) spannen.size()) + " Baender");
    const char* fallname = matrix == 33 ? "m1_bewegung_1p8db"
                         : matrix == 34 ? "m1_bewegung_3p6db"
                                        : "m1_bewegung_5p4db";
    p.wahr (std::isfinite (median) && median >= minDb && median <= maxDb,
            "380/M-" + juce::String (matrix) + " " + fallname,
            "Median " + juce::String (median, 2) + " dB, Referenz "
                + juce::String (0.45 * breiteDb, 1) + " dB, Intervall ["
                + juce::String (minDb, 1) + ", " + juce::String (maxDb, 1) + "]");
    std::printf ("380/M-%d Messung: Median %.2f dB\n", matrix, median);
}

// NAK-380 M-36 (Fassung §31): drei Modulationseingänge beiderseits der
// Textschwellen 6 dB (Charakter, `Diagnose.cpp:212`) und 10 dB (Werkzeug,
// `:224`). Herleitung der gelesenen Spanne (Signal `sig::M36Pegelrechteck`):
//  · Band 135 wird von der Mittenstufe geführt (n 4096, Hop 2048). Der Ton
//    auf Bin 128 füllt mit periodischem Hann genau die Bins 127 bis 129 mit
//    |X|² = A²n²/64, A²n²/16, A²n²/64; mit dichteFaktor 2/(fs·3n/8), vier
//    Bins im Band und der Skala 1,5·fs/8192 ist der Bandpegel
//    10·log10(1,5·A²·n/65 536) = 10·log10(0,09375·0,1²) = −30,28 dB im
//    Grundpegel und −30,28 dB + Tiefe im lauten Pegel. Das Rauschen
//    (σ = 0,01, Banddichte 2σ²/fs) liegt 10·log10(A²n/(16σ²)) = 44,08 dB
//    darunter und bewegt einen Teilblock um weniger als 0,01 dB; beide
//    Stufen runden deshalb auf −30 und −30 + Tiefe (0,22 dB Abstand zur
//    Rundungsgrenze).
//  · Teilblock j der Mittenstufe deckt die Samples [16 384·j, 16 384·j
//    + 18 432). Von den floor(1405/8) = 175 Teilblöcken in 60 s liegen 116
//    ganz im Grundpegel, 47 ganz im lauten Pegel und 12 über einem Wechsel;
//    ein Segment über einem Wechsel trägt eine Bandleistung zwischen beiden
//    Stufen. P50 (Rang ceil(0,5·175) = 88 <= 116) fällt daher auf −30 dB,
//    P95 (Rang ceil(0,95·175) = 167 > 175 − 47 = 128) auf −30 dB + Tiefe:
//    die gelesene Spanne ist genau die Tiefe.
//  · Tiefen: 5 dB steht ruhig (5 <= 6) und kippt, sobald die Schwelle unter
//    5 liegt; 8 dB kommt in Wellen (8 > 6), behält den festen Absenker
//    (8 <= 10) und kippt bei einer Werkzeugschwelle unter 8; 13 dB kommt in
//    Wellen mit dem dynamischen Werkzeug (13 > 10). Die Eingänge halten die
//    Schwellen damit in 5 <= T6 < 8 und 8 <= T10 < 13.
//  · Karte: nur Band 135 trägt den Ton, rund 44 dB über der Rauschdichte
//    (Breite 1/24 Okt <= 1/6 → „sehr schmal“; dbOver weit über 9,17 →
//    cutBis 6), jedes Mittensegment zeigt ihn (Persistenz 1 → dauerhaft),
//    das Bandzentrum 30·2^(135,5/24) = 1502,06 Hz schreibt sich „1,5 kHz“.
void nak380M36Eingang (Pruefer& p, const char* kennung, double tiefeDb, std::uint64_t saat,
                       const juce::String& charakterSoll, const char* charakterWort,
                       const juce::String& werkzeugSoll, const char* werkzeugWort)
{
    namespace sig = nakama::test::nak380;
    const juce::String fall = juce::String ("380/M-36 ") + kennung + " "
                            + juce::String ((int) tiefeDb) + " dB: ";
    sig::M36Pegelrechteck signal { tiefeDb, saat };
    auto engine = std::make_unique<AnalyseEngine>();
    engine->vorbereiten (48000.0);
    const auto samples = nak380M1Speisen (*engine, 60.0, [&] (std::uint64_t n)
    { return signal.naechstes (n); });
    engine->auswerten();
    const auto m = engine->snapshot();

    constexpr int tonBand = 135;
    const double p50 = m.perzentilP50[(std::size_t) tonBand];
    const double p95 = m.perzentilP95[(std::size_t) tonBand];
    p.wahr (samples == 2'880'000u && p50 == -30.0 && p95 == -30.0 + tiefeDb,
            fall + "Vorbedingung gelesene Spanne = Tiefe (P50 -30 dB, P95 -30 dB + Tiefe)",
            juce::String ((juce::int64) samples) + " Samples, P50 " + juce::String (p50, 1)
                + ", P95 " + juce::String (p95, 1));

    std::vector<Befund> resonanz;
    for (const auto& b : diagnose (m, "hub"))
        if (b.klasse == BefundKlasse::resonanz)
            resonanz.push_back (b);
    const double zentrumHz = std::sqrt (30.0 * std::pow (2.0, (double) tonBand / 24.0)
                                        * 30.0 * std::pow (2.0, (double) (tonBand + 1) / 24.0));
    const bool karte = resonanz.size() == 1u && resonanz.front().dauerhaft
                    && std::abs (resonanz.front().fSchwerpunkt - zentrumHz) < 1e-6;
    p.wahr (karte, fall + "Vorbedingung genau eine dauerhafte Resonanzkarte bei 1502,06 Hz",
            juce::String ((int) resonanz.size()) + " Resonanzkarten");
    if (! karte)
        return;
    const auto& b = resonanz.front();
    // Seit NAK-380 Etappe 6 (M-106) folgt dem Charaktertext der Satz zur
    // Suchgrenze (48 kHz: 17,30·48 000/16 384 = 50,68 Hz, gerundet 51).
    const auto suchgrenzeSoll = juce::String (juce::CharPointer_UTF8 (
        "Unter 51 Hz wurde nicht nach Tönen gesucht – das Messfenster ist dort zu grob."));
    p.wahr (b.gemessen.endsWith (charakterSoll + " " + suchgrenzeSoll),
            fall + "Charaktertext " + charakterWort + " woertlich (Schwelle 6 dB)", b.gemessen);
    p.wahr (b.tu == werkzeugSoll,
            fall + "Werkzeugtext " + werkzeugWort + " woertlich (Schwelle 10 dB)", b.tu);
}

void nak380M36Eingaenge (Pruefer& p)
{
    namespace sig = nakama::test::nak380;
    const auto u8 = [] (const char* text) { return juce::String (juce::CharPointer_UTF8 (text)); };
    // Diagnose.cpp:227-230 und :236-237 mit schmalheit „sehr schmal“,
    // f „1,5 kHz“ und cutBis 6 (Herleitung oben).
    const auto fest = u8 ("Senk genau dort sehr schmal ab: im Fruity PEQ2 eine sehr schmale Glocke auf 1,5 kHz (erst −3 dB, bei Bedarf bis −6 dB; die Breite regelt dort Q — hoch heißt schmal).");
    const auto dynamisch = u8 ("Der Ton ist fast immer da, aber sein Pegel wogt — nimm das Werkzeug, das nur die Spitzen abfängt: Smooth Operator Pro mit sehr schmalem Fokus um 1,5 kHz und sanfter Stärke.");
    nak380M36Eingang (p, "M36a", 5.0, sig::kM36aSaat,
                      u8 (" Der Pegel dort steht ruhig (Schwankung ~5 dB)."), "steht ruhig",
                      fest, "fester Absenker");
    nak380M36Eingang (p, "M36b", 8.0, sig::kM36bSaat,
                      u8 (" Der Pegel dort kommt in Wellen (~8 dB Schwankung)."), "kommt in Wellen",
                      fest, "fester Absenker");
    nak380M36Eingang (p, "M36c", 13.0, sig::kM36cSaat,
                      u8 (" Der Pegel dort kommt in Wellen (~13 dB Schwankung)."), "kommt in Wellen",
                      dynamisch, "dynamisches Werkzeug");
}

void nak380M37 (Pruefer& p)
{
    namespace sig = nakama::test::nak380;
    sig::GaussRauschen rauschen { sig::kW4Saat };
    auto engine = std::make_unique<AnalyseEngine>();
    engine->vorbereiten (48000.0);
    const auto samples = nak380M1Speisen (*engine, 60.0, [&] (std::uint64_t)
    { return (float) (0.1 * rauschen.naechstes()); });

    int abweichend = 0, belegt = 0, segmentFalsch = 0;
    for (int b = 0; b < kLtasBaender; ++b)
    {
        const bool mitBin = nak380BinsImBand (b) > 0;
        if (mitBin)
            ++belegt;
        const auto segmente = AnalyseEngineTestzugang::zustaendigeSegmente (*engine, b);
        const auto sollSegmente = mitBin ? nak380Segmente (samples, nak380StufenPunkte (b)) : 0u;
        if (segmente != sollSegmente || (mitBin && segmente == 0u))
            ++segmentFalsch;
        if (AnalyseEngineTestzugang::segmentHistogrammSumme (*engine, b) != segmente)
            ++abweichend;
    }
    // 60*48000 Samples; §7.4 ergibt bei 48 kHz genau 196 belegte Bänder.
    // Die Gleichheit gilt trotzdem für alle 221 Bänder; unbelegte tragen 0=0.
    p.wahr (samples == 2'880'000u && belegt == 196,
            "380/M-37 Vorbedingung W4 und 196 belegte Baender");
    // Z2 (Fassung §31): die Segmentzahl steht fest, bevor die Gleichheit
    // gilt, sonst gälte sie auch bei null Segmenten. Jedes W4-Segment ist
    // aktiv (σ = 0,1 liegt 40 dB über dem Gate −60 dBFS), also zählt jedes
    // Band mit Bin S = floor((2 880 000 - n)/(n/2)) + 1 Segmente seiner Stufe:
    // Bass (n 16384) 350, Mitten (4096) 1405, Höhen (2048) 2811; ohne Bin 0.
    p.wahr (segmentFalsch == 0 && nak380Segmente (samples, 16384) == 350u
                && nak380Segmente (samples, 4096) == 1405u
                && nak380Segmente (samples, 2048) == 2811u,
            "380/M-37 Segmentzahl je Band = Formel der Stufe (Bass 350, Mitten 1405, Hoehen 2811), jede > 0",
            juce::String (segmentFalsch) + " von 221 abweichend");
    p.wahr (abweichend == 0,
            "380/M-37 abdeckung_zaehlt_segmente: jedes Band",
            juce::String (abweichend) + " von 221 abweichend");

    // Kontrastlauf auf derselben W4-Rauschquelle: 4 s unter dem Aktivgate
    // setzen den Teppich, danach wechseln 60 s in 0,25-s-Stufen zwischen
    // RMS 0,0011 (knapp aktiv, unter Teppich+6 dB) und 0,01. So unterscheiden
    // sich Segment- und Achtsegmentverteilung, ohne eine Produktzahl zu setzen.
    sig::GaussRauschen kontrast { sig::kW4Saat };
    auto abdeckungEngine = std::make_unique<AnalyseEngine>();
    abdeckungEngine->vorbereiten (48000.0);
    nak380M1Speisen (*abdeckungEngine, 4.0, [&] (std::uint64_t)
    { return (float) (0.0006 * kontrast.naechstes()); });
    nak380M1Speisen (*abdeckungEngine, 60.0, [&] (std::uint64_t n)
    {
        const bool hoch = ((n / 12'000u) & 1u) != 0u;
        return (float) ((hoch ? 0.01 : 0.0011) * kontrast.naechstes());
    });
    abdeckungEngine->auswerten();
    // Auch der Nachbau braucht gezählte Segmente. Segmente ganz in den ersten
    // 4 s bleiben inaktiv (σ = 0,0006: −64,4 dBFS), Segmente ganz im 60-s-Teil
    // sind aktiv (schwächste Stufe σ = 0,0011: −59,2 dBFS, 0,83 dB über dem
    // Gate, bei 2048 Samples über sechs Standardabweichungen der
    // Leistungsschätzung); offen sind nur die zwei Segmente je Stufe über der
    // 4-s-Grenze. Je Band mit Bin liegen die aktiven Segmente daher in
    // [S(3 072 000) - ceil(192 000/(n/2)), S(3 072 000) - S(192 000)]:
    // Bass [350, 352], Mitten [1405, 1407], Höhen [2811, 2813]; ohne Bin 0.
    int kontrastFalsch = 0;
    for (int b = 0; b < kLtasBaender; ++b)
    {
        const auto segmente = AnalyseEngineTestzugang::zustaendigeSegmente (*abdeckungEngine, b);
        if (nak380BinsImBand (b) <= 0)
        {
            if (segmente != 0u)
                ++kontrastFalsch;
            continue;
        }
        const int n = nak380StufenPunkte (b);
        const auto hop = (std::uint64_t) n / 2u;
        const auto gesamt = nak380Segmente (3'072'000u, n);
        const auto unten = gesamt - (192'000u + hop - 1u) / hop;
        const auto oben = gesamt - nak380Segmente (192'000u, n);
        if (segmente == 0u || segmente < unten || segmente > oben)
            ++kontrastFalsch;
    }
    p.wahr (kontrastFalsch == 0,
            "380/M-37 Kontrastlauf Segmentzahl je Band in [innen, gesamt - still] der Stufe, jede > 0",
            juce::String (kontrastFalsch) + " von 221 ausserhalb");
    const auto ist = abdeckungEngine->snapshot().abdeckung;
    const auto soll = AnalyseEngineTestzugang::abdeckungAusSegmenthistogramm (*abdeckungEngine);
    int anteilAbweichend = 0;
    for (std::size_t i = 0; i < soll.size() && i < ist.size(); ++i)
        if (std::abs (ist[i].anteil - soll[i]) > 1e-12)
            ++anteilAbweichend;
    p.wahr (ist.size() == soll.size() && anteilAbweichend == 0,
            "380/M-37 berechneAbdeckung liest das Einzelsegmenthistogramm",
            juce::String (anteilAbweichend) + " von " + juce::String ((int) soll.size())
                + " Dritteloktavgruppen abweichend");
}

// Soll-Zählgrößen eines Bandes unter W4, `samples` Samples nach Start oder
// Reset. Jedes W4-Segment ist aktiv (σ = 0,1 liegt 40 dB über dem Gate
// −60 dBFS, AnalyseEngine.cpp:13, :122-123); ein Band ohne Bin in seiner
// Stufe zählt nichts. T-380-4: je 8 Segmente ein Teilblock, der Rest wartet.
struct Nak380TeilblockSoll
{
    std::uint64_t segmente {}, bloecke {}, rest {};
};

Nak380TeilblockSoll nak380TeilblockSoll (int band, std::uint64_t samples)
{
    if (nak380BinsImBand (band) <= 0)
        return {};
    const auto segmente = nak380Segmente (samples, nak380StufenPunkte (band));
    return { segmente, segmente / 8u, segmente % 8u };
}

// M-38 Satz 1 und 2 an einem Auswertepunkt: Segment-, Teilblock- und
// Restzahl je Band gegen die Formel, dazu die Restsumme, die genau dann leer
// ist, wenn kein Segment wartet.
void nak380M38Zaehler (Pruefer& p, const AnalyseEngine& e, std::uint64_t samples,
                       const juce::String& punkt)
{
    int segmentFalsch = 0, blockFalsch = 0, restFalsch = 0;
    for (int b = 0; b < kLtasBaender; ++b)
    {
        const auto soll = nak380TeilblockSoll (b, samples);
        if (AnalyseEngineTestzugang::segmentHistogrammSumme (e, b) != soll.segmente)
            ++segmentFalsch;
        if (AnalyseEngineTestzugang::teilblockHistogrammSumme (e, b) != soll.bloecke)
            ++blockFalsch;
        const bool summeLeer = AnalyseEngineTestzugang::teilblockRestsumme (e, b) == 0.0;
        if (AnalyseEngineTestzugang::teilblockRest (e, b) != soll.rest
            || summeLeer != (soll.rest == 0u))
            ++restFalsch;
    }
    const juce::String stufen = juce::String ((juce::int64) nak380Segmente (samples, 16384)) + "/"
                              + juce::String ((juce::int64) nak380Segmente (samples, 4096)) + "/"
                              + juce::String ((juce::int64) nak380Segmente (samples, 2048));
    p.wahr (segmentFalsch == 0,
            "380/M-38 " + punkt + ": Segmente je Band = Formel der Stufe",
            juce::String (segmentFalsch) + " von 221 abweichend, Soll Bass/Mitten/Hoehen " + stufen);
    p.wahr (blockFalsch == 0,
            "380/M-38 " + punkt + ": Teilbloecke je Band = floor(Segmente/8)",
            juce::String (blockFalsch) + " von 221 abweichend");
    p.wahr (restFalsch == 0,
            "380/M-38 " + punkt + ": Rest je Band = Segmente mod 8, Restsumme leer genau ohne Rest",
            juce::String (restFalsch) + " von 221 abweichend");
}

// M-38 Satz 3 und 4 an einem Auswertepunkt: je Feld NaN (nie 0) in jedem
// Band ohne vollständigen Teilblock und endlich in jedem Band mit einem,
// beides aus der Formel; dazu das Flag.
void nak380M38Perzentile (Pruefer& p, const MessSnapshot& m, std::uint64_t samples,
                          bool gueltigSoll, const juce::String& punkt)
{
    const std::array<const std::array<double, kLtasBaender>*, 3> felder {
        &m.perzentilP10, &m.perzentilP50, &m.perzentilP95 };
    const std::array<const char*, 3> namen { "P10", "P50", "P95" };
    for (std::size_t f = 0; f < felder.size(); ++f)
    {
        int ohneBlockNichtNaN = 0, mitBlockNichtEndlich = 0;
        for (int b = 0; b < kLtasBaender; ++b)
        {
            const double wert = (*felder[f])[(std::size_t) b];
            if (nak380TeilblockSoll (b, samples).bloecke == 0u)
            {
                if (! std::isnan (wert))
                    ++ohneBlockNichtNaN;
            }
            else if (! std::isfinite (wert))
                ++mitBlockNichtEndlich;
        }
        p.wahr (ohneBlockNichtNaN == 0 && mitBlockNichtEndlich == 0,
                "380/M-38 " + punkt + ": " + namen[f] + " je Band NaN ohne, endlich mit Teilblock",
                juce::String (ohneBlockNichtNaN) + " ohne Teilblock nicht NaN, "
                    + juce::String (mitBlockNichtEndlich) + " mit Teilblock nicht endlich");
    }
    p.wahr (m.perzentileGueltig == gueltigSoll,
            "380/M-38 " + punkt + ": perzentileGueltig " + (gueltigSoll ? "wahr" : "falsch"),
            juce::String ("Flag ") + (m.perzentileGueltig ? "wahr" : "falsch"));
}

void nak380M38 (Pruefer& p)
{
    namespace sig = nakama::test::nak380;
    // §7.4-Gitter bei 48 kHz: 196 Bänder mit Bin, 25 ohne; die 25 tragen nie
    // einen Teilblock.
    int belegt = 0;
    for (int b = 0; b < kLtasBaender; ++b)
        if (nak380BinsImBand (b) > 0)
            ++belegt;
    p.wahr (belegt == 196, "380/M-38 Vorbedingung 196 Baender mit Bin, 25 ohne",
            juce::String (belegt) + " mit Bin");

    // Lauf 1 (Matrix M-38, Fassung §31): W4 über 60 s ohne Reset,
    // N = 2 880 000. Segmente Bass 350, Mitten 1405, Höhen 2811; Teilblöcke
    // 43/175/351, Rest 6/5/3; jedes Band mit Bin trägt Teilblöcke, das Flag
    // ist wahr.
    {
        sig::GaussRauschen rauschen { sig::kW4Saat };
        const auto w4 = [&] (std::uint64_t) { return (float) (0.1 * rauschen.naechstes()); };
        auto engine = std::make_unique<AnalyseEngine>();
        engine->vorbereiten (48000.0);
        const auto samples = nak380M1Speisen (*engine, 60.0, w4);
        engine->auswerten();
        p.wahr (samples == 2'880'000u, "380/M-38 Lauf 1 Vorbedingung W4 60 s",
                juce::String ((juce::int64) samples) + " Samples");
        nak380M38Zaehler (p, *engine, samples, "Lauf 1 60 s");
        nak380M38Perzentile (p, engine->snapshot(), samples, true, "Lauf 1 60 s");
    }

    // Lauf 2: W4, zuruecksetzen() bei 30 s, Weiterfahrt bis 60 s. Nach dem
    // Reset zählen die Auswertepunkte die Samples seit dem Reset:
    //  · 30 s vor dem Reset (1 440 000): Segmente 174/702/1405, Teilblöcke
    //    21/87/175, Rest 6/6/5 — der Stand, den der Reset leeren muss;
    //  · unmittelbar nach dem Reset (0): alle Zähler und Restsummen 0, nach
    //    auswerten() jedes Feld NaN und das Flag falsch. Der Snapshot, den
    //    zuruecksetzen() selbst bis zum nächsten auswerten() stehen lässt,
    //    trägt die Vorgabe 0,0 (Randbefund R1, Härtung NAK-403) und ist nicht
    //    Gegenstand dieser Zeile;
    //  · 9215 Samples: der erste Teilblock entsteht in der Stufe mit dem
    //    kleinsten Hop (Höhen, n 2048, Hop 1024) nach 2048 + 7·1024 = 9216
    //    Samples, in den Mitten nach 4096 + 7·2048 = 18 432, im Bass nach
    //    16 384 + 7·8192 = 73 728. Bei 9215 trägt keine Stufe einen (Höhen 7,
    //    Mitten 3, Bass 0 Segmente): jedes Feld NaN, Flag falsch;
    //  · 9216 Samples: genau die Höhenbänder mit Bin tragen einen Teilblock,
    //    nur sie sind endlich, das Flag ist wahr;
    //  · 60 s (1 440 000 seit dem Reset): wieder 174/702/1405, 21/87/175,
    //    Rest 6/6/5, Flag wahr.
    {
        sig::GaussRauschen rauschen { sig::kW4Saat };
        const auto w4 = [&] (std::uint64_t) { return (float) (0.1 * rauschen.naechstes()); };
        auto engine = std::make_unique<AnalyseEngine>();
        engine->vorbereiten (48000.0);
        const auto vorReset = nak380M1Speisen (*engine, 30.0, w4);
        p.wahr (vorReset == 1'440'000u, "380/M-38 Lauf 2 Vorbedingung W4 30 s bis zum Reset",
                juce::String ((juce::int64) vorReset) + " Samples");
        nak380M38Zaehler (p, *engine, vorReset, "Lauf 2 30 s vor dem Reset");

        engine->zuruecksetzen();
        nak380M38Zaehler (p, *engine, 0u, "Lauf 2 unmittelbar nach dem Reset");
        engine->auswerten();
        nak380M38Perzentile (p, engine->snapshot(), 0u, false, "Lauf 2 unmittelbar nach dem Reset");

        std::uint64_t seitReset = nak380M1SpeisenSamples (*engine, 9215u, w4);
        engine->auswerten();
        nak380M38Zaehler (p, *engine, seitReset, "Lauf 2 9215 Samples nach dem Reset");
        nak380M38Perzentile (p, engine->snapshot(), seitReset, false,
                             "Lauf 2 9215 Samples nach dem Reset");

        seitReset += nak380M1SpeisenSamples (*engine, 1u, w4);
        engine->auswerten();
        nak380M38Zaehler (p, *engine, seitReset, "Lauf 2 9216 Samples nach dem Reset");
        nak380M38Perzentile (p, engine->snapshot(), seitReset, true,
                             "Lauf 2 9216 Samples nach dem Reset");

        seitReset += nak380M1SpeisenSamples (*engine, 1'440'000u - seitReset, w4);
        engine->auswerten();
        p.wahr (seitReset == 1'440'000u, "380/M-38 Lauf 2 Vorbedingung Weiterfahrt bis 60 s",
                juce::String ((juce::int64) seitReset) + " Samples seit dem Reset");
        nak380M38Zaehler (p, *engine, seitReset, "Lauf 2 60 s");
        nak380M38Perzentile (p, engine->snapshot(), seitReset, true, "Lauf 2 60 s");

        p.wahr (AnalyseEngineTestzugang::zusaetzlicherSpeicherBytes (*engine) == 127'296u,
                "380/M-38 Heap-Speicherzahl je AnalyseEngine",
                juce::String ((juce::int64) AnalyseEngineTestzugang::zusaetzlicherSpeicherBytes (*engine))
                    + " Bytes, Formel 221*(141*4+8+4)=127296");
    }
}

//==============================================================================
// NAK-380 Etappe 6 (DSP-20; R-380-5, T-380-7, T-380-8; Matrix §6.5). Jede
// Referenz ist im Test nachgerechnet (`Nak380Pruefsignale.h`), nie aus der
// Engine gelesen (R-380-8); Engines im Heap (NAK-175).

constexpr double kNak380E6Raten[] = { 44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0 };

juce::String nak380KHz (double fs) { return juce::String (fs / 1000.0, 1) + " kHz"; }

/** M-99: Laengentafel der M1-Analyse je Rate und Stufe gegen die Formel
    T-380-7 (`referenzPunkte`, Basen 16 384/8 192/4 096/2 048) und die Tafel
    der Matrix §6.5; Hop = N/2. Dauern bei 48 kHz 341,3/170,7/85,3/42,7 ms,
    in der 48-kHz-Familie gleich, in der 44,1-kHz-Familie 371,5/185,8/92,9/
    46,4 ms (Rechnung 371,52; 185,76; 92,88; 46,44); Toleranz 0,05 ms = halbe
    letzte Stelle. */
void nak380M99 (Pruefer& p)
{
    namespace sig = nakama::test::nak380;
    const int basis[4] = { 16384, 8192, 4096, 2048 };
    const char* namen[4] = { "Bass", "Referenz", "Mitten", "Hoehen" };
    for (const double fs : kNak380E6Raten)
    {
        auto e = std::make_unique<AnalyseEngine>();
        e->vorbereiten (fs);
        const bool familie441 = std::fmod (fs, 44100.0) == 0.0;
        const double dauer441[4] = { 371.5, 185.8, 92.9, 46.4 };
        const double dauer48[4]  = { 341.3, 170.7, 85.3, 42.7 };
        const int faktor = fs < 80000.0 ? 1 : fs < 160000.0 ? 2 : 4;
        for (int s = 0; s < 4; ++s)
        {
            const int n = AnalyseEngineTestzugang::punkte (*e, s);
            const int formel = sig::referenzPunkte (basis[s], fs);
            const int matrix = basis[s] * faktor;
            const double dauer = 1000.0 * (double) n / fs;
            const double soll = familie441 ? dauer441[s] : dauer48[s];
            std::printf ("  mess [380/M-99] %s %s: N %d (Formel %d, Matrix %d), Hop %d, Dauer %.2f ms\n",
                         nak380KHz (fs).toRawUTF8(), namen[s], n, formel, matrix,
                         AnalyseEngineTestzugang::hop (*e, s), dauer);
            p.wahr (n == formel && formel == matrix && AnalyseEngineTestzugang::hop (*e, s) == n / 2
                        && std::abs (dauer - soll) <= 0.05,
                    "380/M-99 laengentafel_m1 (" + nak380KHz (fs) + ", " + namen[s] + "): N = 2^round(log2(T*fs)) "
                        "gekappt = Matrix, Hop N/2, Dauer der Familie",
                    "N " + juce::String (n) + " (Formel " + juce::String (formel) + ", Matrix " + juce::String (matrix)
                        + "), Dauer " + juce::String (dauer, 2) + " ms (Soll " + juce::String (soll, 1) + ")");
        }
    }
}

/** M-101: Speicher der M1-Analyse je Rate. Eigene Vektoren je Stufe (Kapazitaet
    mal Elementgroesse): ringL, ringR, fenster je 4 B, fftDaten 2*n float = 8 B,
    summePsd (n/2 + 1) double = 4 B je Punkt + 8 B, also 24 B je Punkt + 8 B
    (`AnalyseEngine.cpp` WelchStufe::init); die inneren Tafeln von
    juce::dsp::FFT zaehlen nicht. Summe der vier Stufen 24*Summe N + 32 B; der
    Punktanteil ist die Matrixzahl 737 280 / 1 474 560 / 2 949 120 B. Dazu die
    zwei Histogramme 2 * 221 * 141 * 4 = 2 * 124 644 B, unabhaengig von der
    Rate. */
void nak380M101 (Pruefer& p)
{
    namespace sig = nakama::test::nak380;
    const int basis[4] = { 16384, 8192, 4096, 2048 };
    for (const double fs : kNak380E6Raten)
    {
        auto e = std::make_unique<AnalyseEngine>();
        e->vorbereiten (fs);
        std::uint64_t summe = 0, sollSumme = 0, punkte = 0;
        bool jeStufe = true;
        for (int s = 0; s < 4; ++s)
        {
            const int n = sig::referenzPunkte (basis[s], fs);
            const auto ist = AnalyseEngineTestzugang::stufeHeapBytes (*e, s);
            const std::uint64_t soll = 24u * (std::uint64_t) n + 8u;
            jeStufe = jeStufe && ist == soll && AnalyseEngineTestzugang::punkte (*e, s) == n;
            summe += ist;
            sollSumme += soll;
            punkte += (std::uint64_t) n;
        }
        const std::uint64_t matrix = fs < 80000.0 ? 737280u : fs < 160000.0 ? 1474560u : 2949120u;
        const auto hist = AnalyseEngineTestzugang::histogrammBytes (*e);
        std::printf ("  mess [380/M-101] %s: Stufen %llu B (Soll %llu, Punktanteil %llu, Matrix %llu), Histogramme %llu B\n",
                     nak380KHz (fs).toRawUTF8(), (unsigned long long) summe, (unsigned long long) sollSumme,
                     (unsigned long long) (24u * punkte), (unsigned long long) matrix, (unsigned long long) hist);
        p.wahr (jeStufe && summe == sollSumme && 24u * punkte == matrix && hist == 2u * 124644u,
                "380/M-101 speicher_m1 (" + nak380KHz (fs) + "): 24 B je Punkt + 8 B je Stufe, Punktanteil = Matrix, "
                    "Histogramme 2 * 124 644 B",
                "Stufen " + juce::String ((juce::int64) summe) + " B (Soll " + juce::String ((juce::int64) sollSumme)
                    + "), Punktanteil " + juce::String ((juce::int64) (24u * punkte)) + " (Matrix "
                    + juce::String ((juce::int64) matrix) + "), Histogramme " + juce::String ((juce::int64) hist));
    }

    // Obergrenze (Matrixmutation: Kappe aufgehoben, 384 kHz vorbereitet): die
    // Regel ergaebe bei 384 kHz 131 072/65 536/32 768/16 384; gekappt traegt
    // keine Stufe mehr als N_max = 65 536 Punkte, also 65 536/65 536/32 768/
    // 16 384 und 24 * 180 224 + 32 = 4 325 408 B.
    {
        auto e = std::make_unique<AnalyseEngine>();
        e->vorbereiten (384000.0);
        const int soll[4] = { 65536, 65536, 32768, 16384 };
        std::uint64_t summe = 0;
        bool gleich = true;
        juce::String ist;
        for (int s = 0; s < 4; ++s)
        {
            const int n = AnalyseEngineTestzugang::punkte (*e, s);
            gleich = gleich && n == soll[s];
            summe += AnalyseEngineTestzugang::stufeHeapBytes (*e, s);
            ist << (s ? "/" : "") << n;
        }
        p.wahr (gleich && summe == 24u * 180224u + 32u,
                "380/M-101 speicher_m1 (384 kHz): Obergrenze N_max = 65 536 haelt - Stufen 65 536/65 536/32 768/16 384, "
                "4 325 408 B",
                "Stufen " + ist + ", " + juce::String ((juce::int64) summe) + " B");
    }
}

/** Ein K-Lauf (§7.2 K1 bis K5, M-103 bis M-107): frische Engine bei fs,
    Block 512 (Rest im letzten Block), `auswerten` an jeder vollen
    Viertelsekunde (Worker-Takt 250 ms) und am Ende. Festgehalten wird jeder
    Snapshot ab der Vorbedingung `messbereit` (15 s aktive Zeit) und die
    Karten der Diagnose dazu. Der Lauf verriegelt gespeiste = verarbeitete
    Samples = Signallaenge. */
struct Nak380KLauf
{
    std::uint64_t gespeist {}, verarbeitet {};
    int snapshots {}, messbereit {};
    std::vector<std::vector<Befund>> karten;   // je Snapshot ab messbereit
    MessSnapshot letzter;
    // Nur Messausgabe (§54 der Etappe 6): Breite der Resonanzkandidaten im
    // Snapshot ab messbereit, kleinste und groesste, in Oktaven.
    int kandidaten {};
    double breiteMin { 1.0e9 }, breiteMax {};
};

std::unique_ptr<Nak380KLauf> nak380KFahren (const std::vector<float>& x, double fs)
{
    auto aus = std::make_unique<Nak380KLauf>();
    auto engine = std::make_unique<AnalyseEngine>();
    engine->vorbereiten (fs);
    std::vector<float> inter (1024u);
    const auto takt = (std::uint64_t) std::llround (0.25 * fs);
    std::uint64_t naechster = takt, bei = 0;
    std::size_t i = 0;
    const auto auswerten = [&]
    {
        bei = aus->gespeist;
        engine->auswerten();
        ++aus->snapshots;
        const auto m = engine->snapshot();
        if (m.zustand == MessZustand::messbereit)
        {
            ++aus->messbereit;
            aus->karten.push_back (diagnose (m, "hub"));
            for (const auto& r : m.resonanzen)
            {
                ++aus->kandidaten;
                aus->breiteMin = std::min (aus->breiteMin, r.breiteOktaven);
                aus->breiteMax = std::max (aus->breiteMax, r.breiteOktaven);
            }
        }
    };
    while (i < x.size())
    {
        const int n = (int) std::min<std::size_t> (512u, x.size() - i);
        for (int k = 0; k < n; ++k)
            inter[(std::size_t) k * 2u] = inter[(std::size_t) k * 2u + 1u] = x[i + (std::size_t) k];
        engine->verarbeite (inter.data(), n, 2);
        i += (std::size_t) n;
        aus->gespeist += (std::uint64_t) n;
        if (aus->gespeist >= naechster)
        {
            auswerten();
            naechster += takt;
        }
    }
    if (bei != aus->gespeist)   // das Ende, falls es nicht auf einen Takt fiel
        auswerten();
    aus->letzter = engine->snapshot();
    aus->verarbeitet = aus->letzter.verarbeiteteSamples;
    return aus;
}

/** Die Karten eines K-Laufs: je Snapshot ab `messbereit` die Resonanzkarten
    der Diagnose. Zusage M-103 bis M-105: an JEDEM dieser Snapshots genau eine
    Resonanzkarte, ihr Schwerpunkt innerhalb +-1/6 Oktave um den Ton (die
    Karte nennt die Bandmitte des Spitzenbandes; `kBreiteMaxOktaven` = 1/6
    Oktave ist die groesste Breite einer Resonanz, die Spitze liegt also
    hoechstens so weit neben dem Ton), und sie ist dauerhaft (Persistenz
    >= 0,5). */
struct Nak380Karten
{
    int snapshots {}, genauEine {}, amTon {}, dauerhaft {}, ohneKarte {};
    double fErste {}, fLetzte {};
};

Nak380Karten nak380Karten (const Nak380KLauf& l, double fTon)
{
    Nak380Karten k;
    for (const auto& bs : l.karten)
    {
        ++k.snapshots;
        int n = 0;
        const Befund* r = nullptr;
        for (const auto& b : bs)
            if (b.klasse == BefundKlasse::resonanz) { ++n; r = &b; }
        if (n == 0) ++k.ohneKarte;
        if (n == 1)
        {
            ++k.genauEine;
            if (std::abs (std::log2 (r->fSchwerpunkt / fTon)) <= 1.0 / 6.0) ++k.amTon;
            if (r->dauerhaft) ++k.dauerhaft;
            if (k.fErste == 0.0) k.fErste = r->fSchwerpunkt;
            k.fLetzte = r->fSchwerpunkt;
        }
    }
    return k;
}

/** Gemeinsamer Teil von M-103 bis M-105: Signal erzeugen, Kopffunktion des
    Erzeugers vor dem Nutzer, Lauf, Vorbedingungen, Kartenzusage. */
void nak380Resonanzfall (Pruefer& p, const char* id, const char* name, double fTon, double fs)
{
    namespace sig = nakama::test::nak380;
    const auto samples = (std::uint64_t) std::llround (30.0 * fs);
    const auto k = sig::kSignal (fTon, fs, samples);
    const auto selbst = sig::kSelbstpruefung (k, fTon, fs, samples);
    const juce::String kopf = juce::String ("380/") + id + " " + name;
    p.wahr (selbst.ok, kopf + ": Vorbedingung k_selbstpruefung (E-380-13 Rosa, Tonfrequenz und -pegel gemessen)",
            juce::String (selbst.meldung));
    const auto l = nak380KFahren (k.x, fs);
    p.wahr (l->gespeist == samples && l->verarbeitet == samples,
            kopf + ": Vorbedingung gespeiste Samplezahl = 30 s * fs (Laeufer und Engine)",
            "gespeist " + juce::String ((juce::int64) l->gespeist) + ", verarbeitet "
                + juce::String ((juce::int64) l->verarbeitet));
    // messbereit ab 150 aktiven Zellen (15 s); bis 30 s liegen die Takte
    // 15,00 s bis 29,75 s und das Ende: 60 + 1 Snapshots.
    p.wahr (l->messbereit == 61, kopf + ": Vorbedingung 61 Snapshots ab messbereit (15 s bis 30 s im 250-ms-Takt)",
            juce::String (l->messbereit) + " von " + juce::String (l->snapshots));
    const auto kk = nak380Karten (*l, fTon);
    std::printf ("  mess [%s] %s: %d Snapshots ab messbereit, genau eine Karte %d, am Ton %d, dauerhaft %d, "
                 "ohne Karte %d, Schwerpunkt %.1f bis %.1f Hz, Breite %.4f bis %.4f Oktaven (%d Kandidaten)\n",
                 kopf.toRawUTF8(), nak380KHz (fs).toRawUTF8(), kk.snapshots, kk.genauEine, kk.amTon, kk.dauerhaft,
                 kk.ohneKarte, kk.fErste, kk.fLetzte, l->kandidaten > 0 ? l->breiteMin : 0.0, l->breiteMax,
                 l->kandidaten);
    p.wahr (kk.snapshots > 0 && kk.genauEine == kk.snapshots,
            kopf + ": genau eine Resonanzkarte an jedem Snapshot ab messbereit",
            juce::String (kk.genauEine) + " von " + juce::String (kk.snapshots) + " (ohne Karte "
                + juce::String (kk.ohneKarte) + ")");
    p.wahr (kk.snapshots > 0 && kk.amTon == kk.snapshots,
            kopf + ": die Karte liegt bei " + juce::String (fTon, 0) + " Hz +-1/6 Oktave an jedem Snapshot",
            juce::String (kk.amTon) + " von " + juce::String (kk.snapshots) + ", Schwerpunkt "
                + juce::String (kk.fErste, 1) + " bis " + juce::String (kk.fLetzte, 1) + " Hz");
    p.wahr (kk.snapshots > 0 && kk.dauerhaft == kk.snapshots,
            kopf + ": und sie ist dauerhaft an jedem Snapshot",
            juce::String (kk.dauerhaft) + " von " + juce::String (kk.snapshots));
}

/** Der Wortlaut aus §8.6 mit der gerundeten Suchgrenze. */
juce::String nak380SatzSoll (int hz)
{
    return juce::String (juce::CharPointer_UTF8 ("Unter ")) + juce::String (hz)
         + juce::String (juce::CharPointer_UTF8 (" Hz wurde nicht nach T\xc3\xb6nen gesucht \xe2\x80\x93 das Messfenster ist dort zu grob."));
}

/** Suchgrenze nach T-380-8 im Test: 17,30 * fs/N_Bass(fs). Herleitung des
    Faktors (§6.5): die -6-dB-Breite des Hann-Hauptlappens ist 2 Bins; sie
    ist 1/6 Oktave (`kBreiteMaxOktaven`), wenn 2*Δf = f*(2^(1/12) - 2^(-1/12)),
    also f = 2/(2^(1/12) - 2^(-1/12))*Δf = 17,30*Δf. */
double nak380SuchgrenzeSoll (double fs)
{
    return 17.30 * fs / (double) nakama::test::nak380::referenzPunkte (16384, fs);
}

/** M-106: „nicht gesucht“ im vorhandenen Textweg. (a) Die Satzfunktion nennt
    bei 48 kHz 51 Hz und bei 44,1 kHz 47 Hz (gerundetes resonanzSucheAbHz,
    50,684 und 46,566 Hz). (b) K4 (Ton 45 Hz bei 48 kHz, unter der Grenze):
    keine Resonanzkarte an irgendeinem Snapshot ab messbereit. (c) Jede
    Resonanzkarte traegt den Satz am Ende von `gemessen`: gefahren am Ton
    60 Hz bei 48 kHz (derselbe Erzeuger, ueber der Grenze), jede Karte jedes
    Snapshots. Der Leertext der Befundliste am echten Editor ist B15. */
void nak380M106 (Pruefer& p)
{
    namespace sig = nakama::test::nak380;
    for (const double fs : { 48000.0, 44100.0 })
    {
        auto e = std::make_unique<AnalyseEngine>();
        e->vorbereiten (fs);
        const auto m = e->snapshot();
        const int hz = (int) std::lround (nak380SuchgrenzeSoll (fs));
        const auto satz = nak380Satz (m);
        std::printf ("  mess [380/M-106] %s: Satz \"%s\"\n", nak380KHz (fs).toRawUTF8(), satz.toRawUTF8());
        p.wahr (satz == nak380SatzSoll (hz) && hz == (fs == 48000.0 ? 51 : 47),
                "380/M-106 nicht_gesucht_satz (" + nak380KHz (fs) + "): die Satzfunktion nennt die gerundete Suchgrenze "
                    + juce::String (hz) + " Hz im Wortlaut aus §8.6",
                satz.isEmpty() ? juce::String ("kein Satz") : satz);
    }
    {
        const auto samples = (std::uint64_t) 1440000u;
        const auto k = sig::kSignal (45.0, 48000.0, samples);
        const auto selbst = sig::kSelbstpruefung (k, 45.0, 48000.0, samples);
        p.wahr (selbst.ok, "380/M-106 K4: Vorbedingung k_selbstpruefung", juce::String (selbst.meldung));
        const auto l = nak380KFahren (k.x, 48000.0);
        const auto kk = nak380Karten (*l, 45.0);
        std::printf ("  mess [380/M-106] K4 45 Hz: %d Snapshots ab messbereit, ohne Karte %d\n", kk.snapshots, kk.ohneKarte);
        // Nacharbeit 1 der Etappe 6 (D6): der Satz sagt "gespeist und
        // verarbeitet", die Pruefung misst beides (Muster M-103 bis M-105, M-107).
        p.wahr (l->gespeist == samples && l->verarbeitet == samples && l->messbereit == 61,
                "380/M-106 K4: Vorbedingung 30 s gespeist und verarbeitet, 61 Snapshots ab messbereit",
                "gespeist " + juce::String ((juce::int64) l->gespeist) + ", verarbeitet "
                    + juce::String ((juce::int64) l->verarbeitet) + " Samples, " + juce::String (l->messbereit)
                    + " Snapshots");
        p.wahr (kk.snapshots == 61 && kk.ohneKarte == kk.snapshots,
                "380/M-106 nicht_gesucht_satz K4 (45 Hz bei 48 kHz): keine Resonanzkarte an irgendeinem Snapshot",
                juce::String (kk.ohneKarte) + " von " + juce::String (kk.snapshots) + " ohne Karte");
        p.wahr (nak380Satz (l->letzter) == nak380SatzSoll (51),
                "380/M-106 nicht_gesucht_satz K4: der Satz nennt 51 Hz", nak380Satz (l->letzter));
    }
    {
        const auto samples = (std::uint64_t) 1440000u;
        const auto k = sig::kSignal (60.0, 48000.0, samples);
        const auto selbst = sig::kSelbstpruefung (k, 60.0, 48000.0, samples);
        p.wahr (selbst.ok, "380/M-106 Karte 60 Hz: Vorbedingung k_selbstpruefung", juce::String (selbst.meldung));
        const auto l = nak380KFahren (k.x, 48000.0);
        // Nacharbeit 1 der Etappe 6 (D7): derselbe Vorbedingungssatz wie die
        // M-107-Gegenprobe vor der Kartenpruefung.
        p.wahr (l->gespeist == samples && l->verarbeitet == samples && l->messbereit == 61,
                "380/M-106 Karte 60 Hz: Vorbedingung 30 s gespeist und verarbeitet, 61 Snapshots ab messbereit",
                "gespeist " + juce::String ((juce::int64) l->gespeist) + ", verarbeitet "
                    + juce::String ((juce::int64) l->verarbeitet) + " Samples, " + juce::String (l->messbereit)
                    + " Snapshots");
        const juce::String ende = " " + nak380SatzSoll (51);
        int karten = 0, mitSatz = 0;
        juce::String beispiel;
        for (const auto& bs : l->karten)
            for (const auto& b : bs)
                if (b.klasse == BefundKlasse::resonanz)
                {
                    ++karten;
                    if (b.gemessen.endsWith (ende)) ++mitSatz;
                    else if (beispiel.isEmpty()) beispiel = b.gemessen;
                }
        p.wahr (karten > 0 && mitSatz == karten,
                "380/M-106 nicht_gesucht_satz: jede Resonanzkarte traegt den Satz am Ende von gemessen",
                juce::String (mitSatz) + " von " + juce::String (karten) + " Karten"
                    + (beispiel.isNotEmpty() ? "; ohne Satz: " + beispiel : juce::String()));
    }
}

/** M-107: ein Kandidat unter der Suchgrenze wird nicht gemeldet. K5 (Ton 48 Hz
    bei 48 kHz, 48 < 50,69 Hz): keine Resonanzkarte an irgendeinem Snapshot ab
    messbereit, der Satz nennt 51 Hz. Gegenprobe ueber der Grenze mit demselben
    Erzeuger (Trennschaerfe, Lehre Z1): Ton 60 Hz bei 48 kHz ergibt an jedem
    Snapshot genau eine Karte bei 60 Hz. Zwei Riegel tragen die Zusage
    „keine Karte darunter“: die Unterdrueckung in `findeResonanzen` (Bandmitte
    unter `resonanzSucheAbHz`) und das Breitenkriterium (`kBreiteMaxOktaven`),
    das bei 2 Bins Hauptlappen unter 50,69 Hz breiter als 1/6 Oktave misst. */
void nak380M107 (Pruefer& p)
{
    namespace sig = nakama::test::nak380;
    const auto samples = (std::uint64_t) 1440000u;
    for (const double fTon : { 48.0, 60.0 })
    {
        const auto k = sig::kSignal (fTon, 48000.0, samples);
        const auto selbst = sig::kSelbstpruefung (k, fTon, 48000.0, samples);
        const juce::String kopf = fTon < 50.0 ? "380/M-107 kandidat_unter_suchgrenze K5 (48 Hz bei 48 kHz)"
                                              : "380/M-107 kandidat_unter_suchgrenze Gegenprobe (60 Hz bei 48 kHz)";
        p.wahr (selbst.ok, kopf + ": Vorbedingung k_selbstpruefung", juce::String (selbst.meldung));
        const auto l = nak380KFahren (k.x, 48000.0);
        p.wahr (l->gespeist == samples && l->verarbeitet == samples && l->messbereit == 61,
                kopf + ": Vorbedingung 30 s gespeist und verarbeitet, 61 Snapshots ab messbereit",
                juce::String ((juce::int64) l->gespeist) + " Samples, " + juce::String (l->messbereit) + " Snapshots");
        const auto kk = nak380Karten (*l, fTon);
        std::printf ("  mess [380/M-107] %.0f Hz: %d Snapshots, genau eine %d, am Ton %d, ohne Karte %d, Schwerpunkt %.1f Hz, "
                     "Breite %.4f bis %.4f Oktaven (%d Kandidaten)\n",
                     fTon, kk.snapshots, kk.genauEine, kk.amTon, kk.ohneKarte, kk.fLetzte,
                     l->kandidaten > 0 ? l->breiteMin : 0.0, l->breiteMax, l->kandidaten);
        if (fTon < 50.0)
        {
            p.wahr (kk.snapshots == 61 && kk.ohneKarte == kk.snapshots,
                    kopf + ": keine Resonanzkarte an irgendeinem Snapshot (48 Hz liegt unter 50,69 Hz)",
                    juce::String (kk.ohneKarte) + " von " + juce::String (kk.snapshots) + " ohne Karte");
            p.wahr (nak380Satz (l->letzter) == nak380SatzSoll (51), kopf + ": der Satz nennt 51 Hz",
                    nak380Satz (l->letzter));
        }
        else
            p.wahr (kk.snapshots == 61 && kk.genauEine == kk.snapshots && kk.amTon == kk.snapshots,
                    kopf + ": genau eine Karte bei 60 Hz +-1/6 Oktave an jedem Snapshot (Trennschaerfe)",
                    juce::String (kk.genauEine) + " von " + juce::String (kk.snapshots) + ", am Ton "
                        + juce::String (kk.amTon));
    }
}

/** M-108: die Suchgrenze ist ein Zustand der Engine (T-380-8): nach
    `vorbereiten` bei jeder der sechs Raten endlich, > 0 und gleich 17,30*fs/
    N_Bass(fs) auf 0,01 Hz (46,57 Hz in der 44,1-kHz-, 50,69 Hz in der
    48-kHz-Familie; Rechnung 46,566 und 50,684). Die Toleranz 0,01 Hz ist die
    Rundung der Matrixangabe; die Engine rechnet dieselbe Formel in double. */
void nak380M108 (Pruefer& p)
{
    for (const double fs : kNak380E6Raten)
    {
        auto e = std::make_unique<AnalyseEngine>();
        e->vorbereiten (fs);
        const double ist = nak380Suchgrenze (e->snapshot());
        const double soll = nak380SuchgrenzeSoll (fs);
        const double matrix = std::fmod (fs, 44100.0) == 0.0 ? 46.57 : 50.69;
        std::printf ("  mess [380/M-108] %s: resonanzSucheAbHz %.4f Hz (Soll %.4f, Matrix %.2f)\n",
                     nak380KHz (fs).toRawUTF8(), ist, soll, matrix);
        p.wahr (std::isfinite (ist) && ist > 0.0 && std::abs (ist - soll) <= 0.01 && std::abs (soll - matrix) <= 0.01,
                "380/M-108 suchgrenze_je_rate (" + nak380KHz (fs) + "): resonanzSucheAbHz = 17,30*fs/N_Bass +-0,01 Hz",
                std::isfinite (ist) ? juce::String (ist, 4) + " Hz (Soll " + juce::String (soll, 4) + ")"
                                    : juce::String ("Feld fehlt oder nicht endlich"));
    }
}

/** M-112: Zonen-Ticks deterministisch (Zeit ist Musikzeit, E-380-16). P2
    (-20 dBFS, selbstgeprueft) genau 30 s bei 48 und 96 kHz, jede Zelle aktiv:
    300 Zellen zu lround(0,1*fs) Samples, ein Tick je 10 aktiven Zellen, also
    30 Ticks je belegter Zone. Vor der Tickzahl die Voraussetzungen jeder Zone
    (`AnalyseEngine.cpp` zonenTick): Zone und Schultern mit mindestens 3
    belegten Baendern und 60 % Belegung, jedes Regionsmittel der Live-Kurve
    >= -80 dB (kBodenDb + 10), gelesen an der Live-Kurve des letzten
    Snapshots (dieselbe Skala wie der Tick). Die Tickzahlen sind zusaetzlich
    ratengleich. */
void nak380M112 (Pruefer& p)
{
    namespace sig = nakama::test::nak380;
    std::array<juce::uint32, kZonenAnzahl> ticks48 {};
    for (const double fs : { 48000.0, 96000.0 })
    {
        const auto samples = (std::uint64_t) std::llround (30.0 * fs);
        const auto x = sig::rosaMono (sig::kP2Saat, 0.1, samples);
        const auto r = sig::rosaSelbstpruefung (x, 0.1, fs);
        const juce::String kopf = "380/M-112 zonenticks_30_bei_48_und_96k (" + nak380KHz (fs) + ")";
        p.wahr (r.ok, kopf + ": Vorbedingung rosa_selbstpruefung_E-380-13", juce::String (r.meldung));
        auto e = std::make_unique<AnalyseEngine>();
        e->vorbereiten (fs);
        std::vector<float> inter (1024u);
        for (std::size_t i = 0; i < x.size();)
        {
            const int n = (int) std::min<std::size_t> (512u, x.size() - i);
            for (int k = 0; k < n; ++k)
                inter[(std::size_t) k * 2u] = inter[(std::size_t) k * 2u + 1u] = x[i + (std::size_t) k];
            e->verarbeite (inter.data(), n, 2);
            i += (std::size_t) n;
        }
        e->auswerten();
        const auto m = e->snapshot();
        p.wahr (m.verarbeiteteSamples == samples && std::llround (m.aktivSekunden * 10.0) == 300,
                kopf + ": Vorbedingung 30 s gespeist, 300 aktive Zellen",
                juce::String ((juce::int64) m.verarbeiteteSamples) + " Samples, aktiv " + juce::String (m.aktivSekunden, 2) + " s");
        for (int z = 0; z < kZonenAnzahl; ++z)
        {
            const auto& g = kZonen[z];
            struct Reg { int belegt = 0, gesamt = 0; double summe = 0.0; };
            const auto region = [&] (double lo, double hi)
            {
                Reg rg;
                for (int b = 0; b < kLtasBaender; ++b)
                {
                    const double f = m.ltasZentrenHz[(std::size_t) b];
                    if (f < lo || f >= hi) continue;
                    ++rg.gesamt;
                    if (std::isfinite (m.ltasLiveDb[(std::size_t) b])) { ++rg.belegt; rg.summe += m.ltasLiveDb[(std::size_t) b]; }
                }
                return rg;
            };
            const auto genug = [] (const Reg& rg)
            {
                return rg.gesamt > 0 && rg.belegt >= 3 && (double) rg.belegt / (double) rg.gesamt >= 0.6
                    && rg.summe / rg.belegt >= -80.0;
            };
            const bool mitLinie = g.schulterHiVon > 0.0;
            const bool voraus = genug (region (g.zoneVon, g.zoneBis)) && genug (region (g.schulterLoVon, g.schulterLoBis))
                             && (! mitLinie || genug (region (g.schulterHiVon, g.schulterHiBis)));
            const auto t = m.zonenZeit[(std::size_t) z].ticks;
            p.wahr (voraus, kopf + ": Zone " + juce::String (z) + " Vorbedingung Belegung und Pegel der Zone und ihrer Schultern",
                    "Regionen belegt und >= -80 dB: " + juce::String (voraus ? "ja" : "NEIN"));
            p.wahr (voraus && t == 30u, kopf + ": Zone " + juce::String (z) + " genau 30 Ticks (300 aktive Zellen / 10)",
                    juce::String ((int) t) + " Ticks");
            if (fs == 48000.0)
                ticks48[(std::size_t) z] = t;
            else
                p.wahr (t == ticks48[(std::size_t) z], kopf + ": Zone " + juce::String (z) + " ratengleich mit 48 kHz",
                        juce::String ((int) t) + " / " + juce::String ((int) ticks48[(std::size_t) z]));
            std::printf ("  mess [380/M-112] %s Zone %d: %u Ticks, Vorbedingung %s\n", nak380KHz (fs).toRawUTF8(), z,
                         (unsigned) t, voraus ? "ja" : "NEIN");
        }
    }
}

void nak380M1Faelle (const char* nur, int& ok, int& fehler)
{
    namespace sig = nakama::test::nak380;
    const auto waehlt = [nur] (const char* id)
    { return nur == nullptr || std::strcmp (nur, id) == 0; };
    if (waehlt ("M-32"))
    {
        Pruefer p; p.signal = "380/M-32 W4"; nak380M32 (p);
        ok += p.ok; fehler += p.fehler;
    }
    if (waehlt ("M-33"))
    {
        Pruefer p; p.signal = "380/M-33 M1";
        nak380Bewegung (p, 33, 4.0, sig::kM1RauschSaat, sig::kM1PegelSaat, 1.3, 3.8);
        ok += p.ok; fehler += p.fehler;
    }
    if (waehlt ("M-34"))
    {
        Pruefer p; p.signal = "380/M-34 M2";
        nak380Bewegung (p, 34, 8.0, sig::kM2RauschSaat, sig::kM2PegelSaat, 3.1, 5.6);
        ok += p.ok; fehler += p.fehler;
    }
    if (waehlt ("M-35"))
    {
        Pruefer p; p.signal = "380/M-35 M3";
        nak380Bewegung (p, 35, 12.0, sig::kM3RauschSaat, sig::kM3PegelSaat, 4.9, 7.4);
        ok += p.ok; fehler += p.fehler;
    }
    if (waehlt ("M-36"))
    {
        Pruefer p; p.signal = "380/M-36 M36a bis M36c"; nak380M36Eingaenge (p);
        ok += p.ok; fehler += p.fehler;
    }
    if (waehlt ("M-37"))
    {
        Pruefer p; p.signal = "380/M-37 abdeckung_zaehlt_segmente"; nak380M37 (p);
        ok += p.ok; fehler += p.fehler;
    }
    if (waehlt ("M-38"))
    {
        Pruefer p; p.signal = "380/M-38 teilblock_raender"; nak380M38 (p);
        ok += p.ok; fehler += p.fehler;
    }
    // NAK-380 Etappe 6 (DSP-20).
    const struct { const char* id; const char* signal; void (*fall) (Pruefer&); } etappe6[] = {
        { "M-99",  "380/M-99 laengentafel_m1",           nak380M99 },
        { "M-101", "380/M-101 speicher_m1",              nak380M101 },
        { "M-103", "380/M-103 resonanz_300hz_96k",       [] (Pruefer& p) { nak380Resonanzfall (p, "M-103", "resonanz_300hz_96k K1", 300.0, 96000.0); } },
        { "M-104", "380/M-104 resonanz_500hz_192k",      [] (Pruefer& p) { nak380Resonanzfall (p, "M-104", "resonanz_500hz_192k K2", 500.0, 192000.0); } },
        { "M-105", "380/M-105 resonanz_60hz_192k",       [] (Pruefer& p) { nak380Resonanzfall (p, "M-105", "resonanz_60hz_192k K3", 60.0, 192000.0); } },
        { "M-106", "380/M-106 nicht_gesucht_satz",       nak380M106 },
        { "M-107", "380/M-107 kandidat_unter_suchgrenze", nak380M107 },
        { "M-108", "380/M-108 suchgrenze_je_rate",       nak380M108 },
        { "M-112", "380/M-112 zonenticks_30_bei_48_und_96k", nak380M112 },
    };
    for (const auto& f : etappe6)
        if (waehlt (f.id))
        {
            Pruefer p; p.signal = f.signal; f.fall (p);
            std::printf ("%-44s %3d ok, %d Fehler\n", f.signal, p.ok, p.fehler);
            ok += p.ok; fehler += p.fehler;
        }
}
} // namespace

int main (int argc, char* argv[])
{
    if (argc < 2)
    {
        std::printf ("Aufruf: EqCopGoldenTest <pfad-zu-eq-copilot/fixtures>\n");
        return 2;
    }
    const juce::File fixDir { juce::String { juce::CharPointer_UTF8 { argv[1] } } };
    const auto refDatei = fixDir.getChildFile ("golden-referenz.json");
    if (! refDatei.existsAsFile())
    {
        std::printf ("golden-referenz.json fehlt unter %s\n", fixDir.getFullPathName().toRawUTF8());
        return 2;
    }
    const auto wurzel = juce::JSON::parse (refDatei.loadFileAsString());
    if (! wurzel.isObject())
    {
        std::printf ("golden-referenz.json nicht parsebar\n");
        return 2;
    }
    const auto rate = (double) wurzel["rate_hz"];
    const auto shaObjekt = wurzel["wav_sha256"];
    const auto signale = wurzel["signale"];

    juce::AudioFormatManager formate;
    formate.registerBasicFormats();

    int fehlerGesamt = 0, okGesamt = 0;
    const bool nurNak380 = argc == 4 && std::strcmp (argv[2], "--nak380") == 0;
    if (nurNak380 && std::strcmp (argv[3], "M-36") != 0)
    {
        nak380M1Faelle (argv[3], okGesamt, fehlerGesamt);
        std::printf ("\n%s — %d Pruefungen ok, %d Fehler (metrics %s · diagnose %s)\n",
                     fehlerGesamt == 0 ? "GOLDEN OK" : "GOLDEN FEHLGESCHLAGEN",
                     okGesamt, fehlerGesamt, kMetricsVersion, kDiagnoseVersion);
        return fehlerGesamt == 0 && okGesamt > 0 ? 0 : 1;
    }
    // Voller Lauf: alle NAK-380-Fälle. `--nak380 M-36`: die drei
    // Modulationseingänge, danach nur die Fixture resonanz-116hz.
    nak380M1Faelle (nurNak380 ? argv[3] : nullptr, okGesamt, fehlerGesamt);
    // Die vier Kern-Signale tragen die M1-Kreuzvalidierung (analyze-track-
    // Referenz); die diag-*-Signale tragen NUR SHA-Riegel + M3-Diagnose.
    // M3a: diag-mulm-halb + diag-wander-ton beweisen den ZEITVERLAUF
    // (Zonen-Persistenz ≈ 50 % · wandernder Ton = zeitweise, nie dauerhaft).
    const char* namen[] = { "sinus-1k-minus12", "pink-minus20", "resonanz-116hz", "stille-mit-tick",
                            "diag-scoop-mitte", "diag-mulm", "diag-haerte",
                            "diag-mulm-halb", "diag-wander-ton" };
    for (const char* name : namen)
    {
        if (nurNak380 && juce::String (name) != "resonanz-116hz")
            continue;
        Pruefer p;
        p.signal = name;
        const auto wav = fixDir.getChildFile (juce::String (name) + ".wav");
        if (! wav.existsAsFile())
        {
            std::printf ("WAV fehlt: %s\n  Erst erzeugen:  py -3.13 tools/eq-copilot/erzeuge_fixtures.py --nur-wav\n",
                         wav.getFullPathName().toRawUTF8());
            return 2;
        }
        // Determinismus-Riegel: die WAVs müssen exakt der eingefrorene Stand sein.
        {
            juce::FileInputStream strom (wav);
            const auto sha = juce::SHA256 (strom).toHexString();
            const auto soll = shaObjekt[juce::Identifier (name)].toString();
            if (! sha.equalsIgnoreCase (soll))
            {
                std::printf ("SHA-256-Abweichung bei %s\n  ist  %s\n  soll %s\n  Fixtures neu erzeugen (erzeuge_fixtures.py --nur-wav).\n",
                             name, sha.toRawUTF8(), soll.toRawUTF8());
                return 2;
            }
        }

        std::unique_ptr<juce::AudioFormatReader> leser (formate.createReaderFor (wav));
        if (leser == nullptr || leser->numChannels != 2 || std::abs (leser->sampleRate - rate) > 0.5)
        {
            std::printf ("WAV unlesbar/falsches Format: %s\n", wav.getFullPathName().toRawUTF8());
            return 2;
        }

        AnalyseEngine engine;
        engine.vorbereiten (rate);
        const int blockFrames = 512;
        juce::AudioBuffer<float> puffer (2, blockFrames);
        std::vector<float> interleaved ((size_t) blockFrames * 2);
        juce::int64 gelesen = 0;
        while (gelesen < (juce::int64) leser->lengthInSamples)
        {
            const int n = (int) std::min<juce::int64> (blockFrames, (juce::int64) leser->lengthInSamples - gelesen);
            leser->read (&puffer, 0, n, gelesen, true, true);
            for (int i = 0; i < n; ++i)
            {
                interleaved[(size_t) i * 2]     = puffer.getSample (0, i);
                interleaved[(size_t) i * 2 + 1] = puffer.getSample (1, i);
            }
            engine.verarbeite (interleaved.data(), n, 2);
            gelesen += n;
        }
        engine.auswerten();
        const auto m = engine.snapshot();
        const auto ref = signale[juce::Identifier (name)];

        const bool istStille = juce::String (name) == "stille-mit-tick";
        const bool istDiag = juce::String (name).startsWith ("diag-");

        // ── Zustand + Befund-Sperre (§5.3/§5.10.2) ──
        if (istDiag)
        {
            p.wahr (m.zustand == MessZustand::messbereit, "Zustand messbereit",
                    "20 s aktives Signal muss messbereit sein");

            // ── M3a: Zeitverlauf-Beweise auf Engine-Ebene ──
            if (juce::String (name) == "diag-mulm")
            {
                // Stationärer Mulm: der Zeitverlauf muss „praktisch durchgehend"
                // melden (die EMA braucht ~3 s zum Einschwingen — 0,8 genügt).
                const auto& zz = m.zonenZeit[(size_t) kZonenSlotMulm];
                p.wahr (zz.gueltig && zz.ticks >= 15, "Mulm-Zeitverlauf belegt",
                        "ticks " + juce::String ((int) zz.ticks));
                p.wahr (zz.anteil >= 0.8, "stationaerer Mulm ~durchgehend",
                        "anteil " + juce::String (zz.anteil, 2));
            }
            if (juce::String (name) == "diag-mulm-halb")
            {
                // Halbzeit-Mulm: Anteil MUSS im Fenster um 0,5 liegen — genau
                // das trennt „durchgehend" von „zeitweise" (M3a-Kernbeweis).
                const auto& zz = m.zonenZeit[(size_t) kZonenSlotMulm];
                p.wahr (zz.gueltig && zz.ticks >= 15, "Halbzeit-Zeitverlauf belegt",
                        "ticks " + juce::String ((int) zz.ticks));
                p.wahr (zz.anteil >= 0.30 && zz.anteil <= 0.70, "Halbzeit-Anteil ~0,5",
                        "anteil " + juce::String (zz.anteil, 2));
            }
            if (juce::String (name) == "diag-wander-ton")
            {
                // Wandernder Ton (4 × 5 s): Kandidaten JA, aber keiner darf
                // als dauerhaft gelten — jedes Band trägt den Ton nur ~25 %.
                p.wahr (m.resonanzen.size() >= 2, "wandernder Ton liefert Kandidaten",
                        juce::String ((int) m.resonanzen.size()) + " Kandidaten");
                for (const auto& k : m.resonanzen)
                {
                    p.wahr (k.klasse == ResonanzKlasse::zeitweise,
                            "Kandidat zeitweise (nie dauerhaft)",
                            juce::String (k.freqHz, 1) + " Hz, Persistenz "
                            + juce::String (k.persistenzAnteil, 2));
                    p.wahr (k.persistenzAnteil >= 0.10 && k.persistenzAnteil <= 0.45,
                            "Persistenz ~25 %",
                            juce::String (k.freqHz, 1) + " Hz: "
                            + juce::String (k.persistenzAnteil, 2));
                }
            }
        }
        else if (istStille)
        {
            p.wahr (m.zustand != MessZustand::messbereit, "Zustand",
                    "Stille darf nie messbereit werden (0 s aktiv)");
            p.wahr (m.resonanzen.empty(), "Befund-Sperre",
                    "Referenz listet 6 Kleinstpegel-Funde — die Engine muss schweigen, hat aber "
                    + juce::String ((int) m.resonanzen.size()) + " Kandidaten");
            p.wahr (! m.spektralGueltig, "Spektral-Sperre",
                    "ohne aktive Zeit keine Spektralaussage");
            p.wahr (! m.lufsGueltig, "LUFS null", "Referenz: null (alles unter dem absoluten Gate)");
            p.nahe (m.truePeakDb, (double) ref["tp_dbtp"], 0.4, "True Peak dBTP");
            p.nahe (m.crestDb, (double) ref["crest"], 0.3, "Crest dB");
        }
        else
        {
            p.wahr (m.zustand == MessZustand::messbereit, "Zustand messbereit",
                    "20 s aktives Signal muss messbereit sein");
            p.nahe (m.aktivSekunden, 20.0, 0.5, "aktive Sekunden");

            // ── Skalare (identische 8192er-Achse ⇒ eng) ──
            p.nahe (m.lufsIntegriert, (double) ref["lufs"], 0.3, "LUFS integriert");
            p.nahe (m.truePeakDb, (double) ref["tp_dbtp"], 0.3, "True Peak dBTP");
            p.nahe (m.crestDb, (double) ref["crest"], 0.3, "Crest dB");
            p.relativ (m.centroidMagHz, (double) ref["centroid_mag"], 0.03, "Centroid (magnitude)");
            {
                const double soll = (double) ref["rolloff"];
                const double faktor = m.rolloffHz > 0 ? m.rolloffHz / soll : 0.0;
                p.wahr (faktor > std::pow (2.0, -1.0 / 6.0) && faktor < std::pow (2.0, 1.0 / 6.0),
                        "Rolloff", "ist " + juce::String (m.rolloffHz, 1) + ", soll "
                        + juce::String (soll, 1) + " (Toleranz 1/6 Okt)");
            }
            p.nahe (m.lowFrac, (double) ref["low_frac"], 0.02, "low_frac (<250 Hz)");
            p.nahe (m.width, (double) ref["width"], 0.02, "Stereo-Width");
            p.nahe (m.corr, (double) ref["corr"], 0.02, "Korrelation");
            {
                const auto band = ref["band_pct"];
                for (int b = 0; b < 8 && b < band.size(); ++b)
                    p.nahe (m.band8Prozent[(size_t) b], (double) band[b][1], 1.0,
                            "Band-% " + band[b][0].toString());
            }

            // ── LTAS-Referenzachse: exakt die analyze-track-Achse ──
            const auto ltas = ref["ltas"];
            std::vector<double> sollDb;
            for (int i = 0; i < ltas["db"].size(); ++i)
                sollDb.push_back (ltas["db"][i].isDouble() || ltas["db"][i].isInt()
                                      ? (double) ltas["db"][i]
                                      : std::numeric_limits<double>::quiet_NaN());
            vergleicheKurve (p, "LTAS Referenzachse", m.ltasReferenzDb, sollDb, 0.5, 2.0);

            // ── Komposit §5.10.1: Kurvenvergleich nur für Breitband-Anteile ──
            if (juce::String (name) == "pink-minus20")
                vergleicheKurve (p, "LTAS Komposit", m.ltasKompositDb, sollDb, 1.2, 3.5);
            else if (juce::String (name) == "resonanz-116hz")
                vergleicheKurve (p, "LTAS Komposit", m.ltasKompositDb, sollDb, 1.2, 3.5,
                                 70.0, 116.0, 1.0 / 6.0, &m.ltasZentrenHz);
            else   // Sinus: Bandpegel sind auflösungsabhängig — struktureller Check
            {
                int peakBand = 0; double peakWert = -1e18;
                for (int b = 0; b < kLtasBaender; ++b)
                    if (! std::isnan (m.ltasKompositDb[(size_t) b])
                        && m.ltasKompositDb[(size_t) b] > peakWert)
                    {
                        peakWert = m.ltasKompositDb[(size_t) b];
                        peakBand = b;
                    }
                p.wahr (std::abs (std::log2 (m.ltasZentrenHz[(size_t) peakBand] / 1000.0)) <= 1.0 / 12.0,
                        "Komposit-Peak bei 1 kHz",
                        "Peak liegt bei " + juce::String (m.ltasZentrenHz[(size_t) peakBand], 1) + " Hz");
            }

            // ── Flatness: nur Größenordnung (Achsen-Differenz dokumentiert) ──
            if (juce::String (name) == "sinus-1k-minus12")
                p.wahr (m.flatness < 1e-2, "Flatness-Ordnung Sinus",
                        "ist " + juce::String (m.flatness, 8));
            if (juce::String (name) == "pink-minus20")
                p.wahr (m.flatness > 0.01 && m.flatness < 0.6, "Flatness-Ordnung Pink",
                        "ist " + juce::String (m.flatness, 5));

            // ── Resonanzkandidaten §5.10.3 ──
            if (juce::String (name) == "sinus-1k-minus12")
            {
                p.wahr (m.resonanzen.size() == 1, "genau 1 Kandidat (1 kHz)",
                        juce::String ((int) m.resonanzen.size())
                        + " Kandidaten — Leakage-Funde in unbespielten Baendern muss das "
                          "Abdeckungs-Gate schlucken (Referenz listet 6!)");
                if (! m.resonanzen.empty())
                {
                    p.wahr (std::abs (std::log2 (m.resonanzen[0].freqHz / 1000.0)) <= 1.0 / 12.0,
                            "Kandidat bei 1 kHz", juce::String (m.resonanzen[0].freqHz, 1) + " Hz");
                    p.wahr (m.resonanzen[0].klasse == ResonanzKlasse::dauerhaft,
                            "Kandidat dauerhaft", "Dauerton muss dauerhaft sein");
                }
            }
            if (juce::String (name) == "pink-minus20")
            {
                p.wahr (m.resonanzen.empty(), "keine Kandidaten in Pink",
                        juce::String ((int) m.resonanzen.size()) + " Kandidaten gefunden");
                // M3a: neutrales Pink darf in KEINER Zone Zeitverlauf-Alarm
                // sammeln (Falsch-Positiv-Riegel des Zeitverlaufs) …
                for (int z = 0; z < kZonenAnzahl; ++z)
                {
                    const auto& zz = m.zonenZeit[(size_t) z];
                    p.wahr (zz.gueltig && zz.ticks >= 15, "Zonen-Ticks belegt",
                            "Zone " + juce::String (z) + ": " + juce::String ((int) zz.ticks));
                    p.wahr (zz.anteil <= 0.10, "Pink ohne Zonen-Alarm",
                            "Zone " + juce::String (z) + ": anteil " + juce::String (zz.anteil, 2));
                }
                // … und die Perzentile müssen belegt und geordnet sein.
                p.wahr (m.perzentileGueltig, "Perzentile belegt", "perzentileGueltig=false");
                int geordnet = 0, belegt = 0;
                for (int b = 0; b < kLtasBaender; ++b)
                {
                    const double p10 = m.perzentilP10[(size_t) b];
                    const double p50 = m.perzentilP50[(size_t) b];
                    const double p95 = m.perzentilP95[(size_t) b];
                    if (! std::isfinite (p10) || ! std::isfinite (p50) || ! std::isfinite (p95))
                        continue;
                    ++belegt;
                    if (p10 <= p50 && p50 <= p95)
                        ++geordnet;
                }
                p.wahr (belegt >= 150, "Perzentile weitgehend belegt",
                        juce::String (belegt) + " Baender");
                p.wahr (geordnet == belegt, "P10 <= P50 <= P95 ueberall",
                        juce::String (belegt - geordnet) + " ungeordnete Baender");
                std::printf ("NAK-380 pink-minus20: %d Baender mit Teilblock-Perzentilen\n",
                             belegt);
            }
            if (juce::String (name) == "resonanz-116hz")
            {
                p.wahr (m.resonanzen.size() == 1, "genau 1 Kandidat (116 Hz)",
                        juce::String ((int) m.resonanzen.size()) + " Kandidaten");
                if (! m.resonanzen.empty())
                {
                    const auto& k = m.resonanzen[0];
                    p.wahr (std::abs (std::log2 (k.freqHz / 116.0)) <= 1.0 / 6.0,
                            "Kandidat bei 116 Hz (Bassfenster §5.10.1)",
                            juce::String (k.freqHz, 1) + " Hz");
                    p.wahr (k.dbOver >= 15.0 && k.dbOver <= 32.0, "db_over plausibel",
                            juce::String (k.dbOver, 1) + " dB (Referenz 21.7 auf groeberem Raster)");
                    p.wahr (k.klasse == ResonanzKlasse::dauerhaft && k.persistenzAnteil >= 0.5,
                            "dauerhaft + Persistenz >= 50 %",
                            "Persistenz " + juce::String (k.persistenzAnteil, 2));
                }
            }

            // ── Abdeckung §5.10.2 ──
            if (juce::String (name) == "pink-minus20")
            {
                int belastbar = 0;
                for (const auto& a : m.abdeckung)
                    if (a.klasse == AbdeckungsKlasse::belastbar)
                        ++belastbar;
                p.wahr (belastbar >= 24, "Pink weitgehend belastbar",
                        juce::String (belastbar) + " von " + juce::String ((int) m.abdeckung.size()));
            }
            if (juce::String (name) == "sinus-1k-minus12")
            {
                int belastbar = 0;
                for (const auto& a : m.abdeckung)
                    if (a.klasse == AbdeckungsKlasse::belastbar)
                        ++belastbar;
                p.wahr (belastbar <= 4, "Sinus nur um 1 kHz belastbar",
                        juce::String (belastbar) + " belastbare 1/3-Okt-Baender — Leakage-Baender "
                        "duerfen nicht belastbar wirken");
            }
        }

        // ── M3: Graph-Diagnose (der Produktkern) auf JEDEM Signal ──
        diagnosePruefungen (p, name, m);

        std::printf ("%-18s  %3d ok, %d Fehler\n", name, p.ok, p.fehler);
        fehlerGesamt += p.fehler;
        okGesamt += p.ok;
    }

    // ── Adversariale Stereo-/Robustheitsprüfungen (Übergabe Paket B/C/D) ────
    // Die Referenzkurve stammt aus analyze-track auf dem L==R-Pink (Mid ≡
    // Kanalenergie); jede Variante unten hat dazu eine mathematisch zwingende
    // Erwartung. Vor der Kanalenergie-Umstellung lieferte Antiphase „keine
    // Daten" — genau dieser Ausfall wird hier dauerhaft verriegelt.
    // `--nak380 M-36` läuft ohne diesen Block (nur der betroffene Fall).
    if (! nurNak380)
    {
        std::vector<float> pinkL;
        {
            const auto wav = fixDir.getChildFile ("pink-minus20.wav");
            std::unique_ptr<juce::AudioFormatReader> leser (formate.createReaderFor (wav));
            if (leser != nullptr)
            {
                juce::AudioBuffer<float> alles (2, (int) leser->lengthInSamples);
                leser->read (&alles, 0, (int) leser->lengthInSamples, 0, true, true);
                pinkL.resize ((size_t) alles.getNumSamples());
                for (int i = 0; i < alles.getNumSamples(); ++i)
                    pinkL[(size_t) i] = alles.getSample (0, i);
            }
        }
        const auto ref = signale[juce::Identifier ("pink-minus20")];
        std::vector<double> sollDb;
        for (int i = 0; i < ref["ltas"]["db"].size(); ++i)
            sollDb.push_back (ref["ltas"]["db"][i].isDouble() || ref["ltas"]["db"][i].isInt()
                                  ? (double) ref["ltas"]["db"][i]
                                  : std::numeric_limits<double>::quiet_NaN());
        const double refLufs  = (double) ref["lufs"];
        const double refCrest = (double) ref["crest"];
        const double refTp    = (double) ref["tp_dbtp"];
        constexpr double kHalb = 3.0102999566;   // 10·log10(2): ein Kanal = halbe Leistung

        auto fuettere = [] (AnalyseEngine& e, const std::vector<float>& l, const std::vector<float>& r)
        {
            const int blockFrames = 512;
            std::vector<float> inter ((size_t) blockFrames * 2);
            size_t pos = 0;
            while (pos < l.size())
            {
                const int n = (int) std::min<size_t> ((size_t) blockFrames, l.size() - pos);
                for (int i = 0; i < n; ++i)
                {
                    inter[(size_t) i * 2]     = l[pos + (size_t) i];
                    inter[(size_t) i * 2 + 1] = r[pos + (size_t) i];
                }
                e.verarbeite (inter.data(), n, 2);
                pos += (size_t) n;
            }
            e.auswerten();
        };
        auto lauf = [&] (const char* name, const std::vector<float>& l,
                         const std::vector<float>& r, double engineRate,
                         auto&& pruefe)
        {
            Pruefer p;
            p.signal = name;
            AnalyseEngine engine;
            engine.vorbereiten (engineRate);
            fuettere (engine, l, r);
            pruefe (p, engine.snapshot());
            std::printf ("%-18s  %3d ok, %d Fehler\n", name, p.ok, p.fehler);
            fehlerGesamt += p.fehler;
            okGesamt += p.ok;
        };

        if (pinkL.empty())
        {
            std::printf ("pink-minus20.wav nicht ladbar — Stereo-Pruefungen uebersprungen\n");
            ++fehlerGesamt;
        }
        else
        {
            // a) Antiphase (R = −L): reine Side-Energie. Kanalenergie, LUFS,
            //    Crest und True Peak sind zur L==R-Referenz IDENTISCH — nur
            //    corr/width kippen. Der Mid-Mix hätte hier „keine Daten".
            std::vector<float> neg (pinkL.size());
            for (size_t i = 0; i < pinkL.size(); ++i)
                neg[i] = -pinkL[i];
            lauf ("stereo-antiphase", pinkL, neg, rate, [&] (Pruefer& p, const MessSnapshot& m)
            {
                p.wahr (m.zustand == MessZustand::messbereit, "Antiphase ist messbereit",
                        "Side-only darf nie als still gelten (alter Mid-Mix-Ausfall)");
                p.nahe (m.aktivSekunden, 20.0, 0.5, "aktive Sekunden");
                vergleicheKurve (p, "LTAS Referenzachse", m.ltasReferenzDb, sollDb, 0.5, 2.0);
                p.nahe (m.lufsIntegriert, refLufs, 0.3, "LUFS wie Referenz");
                p.nahe (m.crestDb, refCrest, 0.3, "Crest wie Referenz");
                p.nahe (m.truePeakDb, refTp, 0.3, "True Peak wie Referenz");
                p.nahe (m.corr, -1.0, 0.001, "Korrelation exakt −1");
                p.wahr (m.width > 1000.0, "Width side-dominant",
                        "ist " + juce::String (m.width, 1));
            });

            // b) Nur links: halbe Kanalenergie ⇒ Kurve und LUFS −3,01 dB,
            //    Crest +3,01 dB (Peak bleibt), True Peak unverändert.
            std::vector<float> stille (pinkL.size(), 0.0f);
            std::vector<double> sollMinus3 (sollDb);
            for (auto& v : sollMinus3)
                v -= kHalb;
            lauf ("stereo-nur-links", pinkL, stille, rate, [&] (Pruefer& p, const MessSnapshot& m)
            {
                p.wahr (m.zustand == MessZustand::messbereit, "messbereit", "20 s aktiv");
                vergleicheKurve (p, "LTAS = Referenz − 3,01 dB", m.ltasReferenzDb, sollMinus3, 0.5, 2.0);
                p.nahe (m.lufsIntegriert, refLufs - kHalb, 0.3, "LUFS − 3,01");
                p.nahe (m.crestDb, refCrest + kHalb, 0.3, "Crest + 3,01");
                p.nahe (m.truePeakDb, refTp, 0.3, "True Peak unveraendert");
                p.nahe (m.width, 1.0, 0.02, "Width = 1 (einseitig)");
            });

            // c) Nur rechts: exakt symmetrisch — beweist, dass der R-Kanal
            //    wirklich eigenständig gerechnet wird (vorher nur im Mid-Mix).
            lauf ("stereo-nur-rechts", stille, pinkL, rate, [&] (Pruefer& p, const MessSnapshot& m)
            {
                p.wahr (m.zustand == MessZustand::messbereit, "messbereit", "20 s aktiv");
                vergleicheKurve (p, "LTAS = Referenz − 3,01 dB", m.ltasReferenzDb, sollMinus3, 0.5, 2.0);
                p.nahe (m.lufsIntegriert, refLufs - kHalb, 0.3, "LUFS − 3,01");
            });

            // d) Dekorreliert (R = 5 s rotiertes L): gleiche PSD je Kanal ⇒
            //    Kurve bleibt die Referenz, aber corr ≈ 0 und width ≈ 1 —
            //    der Hall-/breite-Chor-Fall des Users.
            std::vector<float> rot (pinkL.size());
            {
                const size_t versatz = (size_t) (5.0 * rate);
                for (size_t i = 0; i < pinkL.size(); ++i)
                    rot[i] = pinkL[(i + versatz) % pinkL.size()];
            }
            lauf ("stereo-dekorr", pinkL, rot, rate, [&] (Pruefer& p, const MessSnapshot& m)
            {
                p.wahr (m.zustand == MessZustand::messbereit, "messbereit", "20 s aktiv");
                vergleicheKurve (p, "LTAS wie Referenz", m.ltasReferenzDb, sollDb, 1.0, 3.0);
                p.nahe (m.lufsIntegriert, refLufs, 0.4, "LUFS wie Referenz");
                p.melde (std::abs (m.corr) < 0.25, "Korrelation nahe 0",
                         "ist " + juce::String (m.corr, 3));
                p.nahe (m.width, 1.0, 0.15, "Width nahe 1");
            });

            // e) NaN-Recovery (Paket C §8.2): Sekunde 5–6 komplett NaN. Die
            //    Analyse ersetzt exakt 2·48000 Samples durch Stille, bleibt
            //    messbereit und liefert ausschließlich endliche Werte.
            std::vector<float> mitNaN (pinkL);
            {
                const size_t von = (size_t) (5.0 * rate), bis = (size_t) (6.0 * rate);
                for (size_t i = von; i < bis && i < mitNaN.size(); ++i)
                    mitNaN[i] = std::numeric_limits<float>::quiet_NaN();
            }
            lauf ("nan-recovery", mitNaN, mitNaN, rate, [&] (Pruefer& p, const MessSnapshot& m)
            {
                p.wahr (m.zustand == MessZustand::messbereit, "trotz NaN messbereit",
                        "1 s NaN darf 19 s Musik nicht entwerten");
                p.wahr (m.nanErsetzt == (juce::uint64) (2.0 * rate), "Ersetzt-Zaehler exakt",
                        "ist " + juce::String ((juce::int64) m.nanErsetzt)
                        + ", soll " + juce::String ((juce::int64) (2.0 * rate)));
                p.wahr (std::isfinite (m.lufsIntegriert) && m.lufsGueltig, "LUFS endlich+gueltig",
                        "NaN hat das Gating vergiftet");
                p.nahe (m.lufsIntegriert, refLufs, 0.5, "LUFS nahe Referenz");
                p.wahr (std::isfinite (m.truePeakDb), "True Peak endlich", "vergiftet");
                p.wahr (std::isfinite (m.crestDb), "Crest endlich", "vergiftet");
                vergleicheKurve (p, "LTAS erholt", m.ltasReferenzDb, sollDb, 1.0, 3.5);
            });

            // f) Nyquist-Kappe (Paket D): bei fs=32 kHz endet der messbare
            //    Bereich bei 0,95·16 kHz — darüber MUSS die Kurve NaN sein
            //    (vorher wurde der letzte Randwert fortgeschrieben).
            lauf ("nyquist-32k", pinkL, pinkL, 32000.0, [&] (Pruefer& p, const MessSnapshot& m)
            {
                const double grenzeHz = 0.95 * 16000.0;
                int erstesNaNBand = kLtasBaender;
                for (int b = 0; b < kLtasBaender; ++b)
                    if (m.ltasZentrenHz[(size_t) b] > grenzeHz)
                    {
                        erstesNaNBand = b;
                        break;
                    }
                p.wahr (erstesNaNBand < kLtasBaender, "Kappe liegt im Raster",
                        "0,95·Nyquist muss unter 18 kHz liegen");
                bool untenOk = true, obenOk = true;
                for (int b = 0; b < kLtasBaender; ++b)
                {
                    const bool nan = std::isnan (m.ltasKompositDb[(size_t) b]);
                    if (b < erstesNaNBand && nan)  untenOk = false;
                    if (b >= erstesNaNBand && ! nan) obenOk = false;
                }
                p.wahr (m.ltasGueltig, "LTAS gueltig", "messbarer Bereich muss gelten");
                p.wahr (untenOk, "unterhalb der Kappe keine Luecken",
                        "NaN im messbaren Bereich");
                p.wahr (obenOk, "oberhalb der Kappe nur NaN",
                        "fortgeschriebener Randwert = erfundene Evidenz");
                for (const auto& k : m.resonanzen)
                    p.wahr (k.freqHz <= grenzeHz, "kein Kandidat oberhalb der Kappe",
                            juce::String (k.freqHz, 1) + " Hz");
            });

            // g) M3a auswertenLeicht (FPS-Fix): der Leichtpfad publiziert
            //    Live-Kurve/Meter/Zustand OHNE Schwerlauf — und im Leerlauf
            //    publiziert er NICHTS (Revision steht).
            {
                Pruefer p;
                p.signal = "leicht-publikation";
                AnalyseEngine engine;
                engine.vorbereiten (rate);
                const int blockFrames = 512;
                std::vector<float> inter ((size_t) blockFrames * 2);
                auto fuettereBereich = [&] (size_t von, size_t bis)
                {
                    size_t pos = von;
                    while (pos < bis)
                    {
                        const int n = (int) std::min<size_t> ((size_t) blockFrames, bis - pos);
                        for (int i = 0; i < n; ++i)
                        {
                            inter[(size_t) i * 2]     = pinkL[pos + (size_t) i];
                            inter[(size_t) i * 2 + 1] = pinkL[pos + (size_t) i];
                        }
                        engine.verarbeite (inter.data(), n, 2);
                        pos += (size_t) n;
                    }
                };
                fuettereBereich (0, (size_t) (5.0 * rate));
                engine.auswertenLeicht();
                const auto m1 = engine.snapshot();
                p.wahr (m1.revision > 0, "Leichtpfad publiziert", "Revision 0");
                p.wahr (m1.zustand == MessZustand::sammelt, "Zustand sammelt (5 s)",
                        "zustand falsch");
                p.wahr (m1.lufsShortGueltig && std::isfinite (m1.lufsShort),
                        "Kurz-LUFS ohne Schwerlauf", "fehlt");
                p.wahr (std::isfinite (m1.truePeakDb) && m1.truePeakDb > -100.0,
                        "True Peak ohne Schwerlauf", juce::String (m1.truePeakDb, 1));
                int liveBaender = 0;
                for (const double v : m1.ltasLiveDb)
                    if (std::isfinite (v))
                        ++liveBaender;
                p.wahr (liveBaender > 100, "Live-Kurve belegt",
                        juce::String (liveBaender) + " Baender");
                p.wahr (! m1.ltasGueltig, "Komposit bleibt dem Schwerlauf vorbehalten",
                        "ltasGueltig ohne auswerten()");
                engine.auswertenLeicht();   // keine neuen Samples
                p.wahr (engine.snapshot().revision == m1.revision,
                        "Leerlauf publiziert nicht", "Revision lief weiter");
                fuettereBereich ((size_t) (5.0 * rate), (size_t) (6.0 * rate));
                engine.auswertenLeicht();
                p.wahr (engine.snapshot().revision > m1.revision,
                        "neue Daten => neue Revision", "Revision steht");
                std::printf ("%-18s  %3d ok, %d Fehler\n", p.signal.toRawUTF8(), p.ok, p.fehler);
                fehlerGesamt += p.fehler;
                okGesamt += p.ok;
            }

            // h) M3a Konvergenz („Kurve steht"): zwei Schwerläufe im Abstand
            //    von 10 s Aktivzeit — stationäres Pink muss als „steht" lesen.
            {
                Pruefer p;
                p.signal = "konvergenz";
                AnalyseEngine engine;
                engine.vorbereiten (rate);
                const int blockFrames = 512;
                std::vector<float> inter ((size_t) blockFrames * 2);
                auto fuettereBereich = [&] (size_t von, size_t bis)
                {
                    size_t pos = von;
                    while (pos < bis)
                    {
                        const int n = (int) std::min<size_t> ((size_t) blockFrames, bis - pos);
                        for (int i = 0; i < n; ++i)
                        {
                            inter[(size_t) i * 2]     = pinkL[pos + (size_t) i];
                            inter[(size_t) i * 2 + 1] = pinkL[pos + (size_t) i];
                        }
                        engine.verarbeite (inter.data(), n, 2);
                        pos += (size_t) n;
                    }
                };
                fuettereBereich (0, pinkL.size() / 2);
                engine.auswerten();
                p.wahr (! engine.snapshot().konvergenzGueltig,
                        "erstes Fenster: noch kein Konvergenzwert", "zu frueh gueltig");
                fuettereBereich (pinkL.size() / 2, pinkL.size());
                engine.auswerten();
                const auto m2 = engine.snapshot();
                p.wahr (m2.konvergenzGueltig, "Konvergenz nach zweitem Fenster", "fehlt");
                p.wahr (std::isfinite (m2.konvergenzMeanDb)
                            && m2.konvergenzMeanDb >= 0.0 && m2.konvergenzMeanDb < 1.0,
                        "mittlere Bewegung klein (stationaeres Pink)",
                        juce::String (m2.konvergenzMeanDb, 3) + " dB");
                p.wahr (std::isfinite (m2.konvergenzMaxDb) && m2.konvergenzMaxDb < 6.0,
                        "maximale Bewegung plausibel",
                        juce::String (m2.konvergenzMaxDb, 2) + " dB");
                std::printf ("%-18s  %3d ok, %d Fehler\n", p.signal.toRawUTF8(), p.ok, p.fehler);
                fehlerGesamt += p.fehler;
                okGesamt += p.ok;
            }
        }
    }

    std::printf ("\n%s — %d Pruefungen ok, %d Fehler (metrics %s · diagnose %s)\n",
                 fehlerGesamt == 0 ? "GOLDEN OK" : "GOLDEN FEHLGESCHLAGEN",
                 okGesamt, fehlerGesamt, kMetricsVersion, kDiagnoseVersion);
    return fehlerGesamt == 0 ? 0 : 1;
}
