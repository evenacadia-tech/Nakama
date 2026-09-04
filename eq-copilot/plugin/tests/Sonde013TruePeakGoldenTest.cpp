/*  EqCopSonde013TruePeakGoldenTest — der True-Peak-Pfad (SONDE-013 M-02).

    Matrixzeile M-02 verlangt eine MESSBARE Entscheidung, keine Behauptung:

        "Der vorhandene 8-fach-Pfad bleibt NUR, wenn er das offizielle
         EBU-Testset bei 48 kHz und generierte Mehrsampleraten-Goldens
         besteht; sonst wird der BS.1770-Polyphase-Referenzpfad verwendet.
         Toleranz +/-0,1 dB (§49.3). Das Bein muss die Entscheidung 8-fach
         gegen Polyphase MESSBAR treffen, nicht behaupten."

    ── WOGEGEN HIER GEMESSEN WIRD, UND WARUM DAS BESSER IST ALS EIN TESTSET ──

    Die True-Peak-Faelle aus EBU Tech 3341 (Testfaelle 15 bis 19) sind KEINE
    aufgenommenen Dateien, sondern analytisch definierte Sinus: eine Frequenz
    als Bruchteil der Samplerate, eine Amplitude in FFS und eine Phase. Der
    wahre Scheitel eines Sinus IST seine Amplitude — die Referenz ist damit
    exakt, nicht selbst eine Messung.

    Das hat drei Folgen, die dieses Bein tragen:

    1. Der Sampleraten-Sweep aus M-02 entsteht von selbst. Die Frequenzen sind
       fs/4, fs/6 und fs/8; bei jeder Samplerate ist die NORMIERTE Frequenz
       dieselbe, und dieselbe Erwartung gilt. Ein heruntergeladenes 48-kHz-Set
       koennte diese Frage gar nicht stellen.
    2. Es gibt keine zweite Implementierung, gegen die gerechnet wird. Ein
       Golden gegen einen zweiten eigenen Detektor waere eine Tautologie —
       genau der Fehler, den `docs/beweise/SONDE-008.md` bei der Loudness
       ausdruecklich vermieden hat.
    3. Die 8-fach-Entscheidung wird nachgerechnet statt geglaubt. Der
       Restfehler der Ueberabtastung hat eine geschlossene Form, und
       `TruePeakDetektor::interpolationsfehlerDb` traegt sie. Dieses Bein
       misst, dass die WIRKLICHE Abweichung diese Schranke einhaelt — und
       dass 4-fach sie bei fs/4 reissen wuerde. Das ist die Entscheidung
       zwischen 8-fach und Polyphase, als Zahl.

    Die EBU-Toleranz ist +0,2/-0,4 dB. Nakama misst gegen die schaerfere aus
    §49.3: +/-0,1 dB. Wer die weichere braeuchte, haette den Pfad nicht.
*/

#include <juce_core/juce_core.h>

#include "../core/analysis/TruePeak.h"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

using nakama::analyse::TruePeakDetektor;

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

/** Misst den groessten True Peak eines Sinus, linear.

    `teiler` ist der Nenner von fs: 4 heisst fs/4. Die Phase kommt in Grad,
    weil EBU Tech 3341 sie so angibt.

    Der Vorlauf von `kEinschwingen` Samples wird verworfen: der Interpolator
    ist ein FIR mit 24 Taps je Phase, und solange sein Verlauf noch Nullen
    enthaelt, misst er einen Ausschnitt, nicht den Sinus. Das ist keine
    Nachsicht gegenueber dem Detektor — im Betrieb steht der Verlauf nach
    24 Samples, also nach einer halben Millisekunde. */
double sinusTruePeak (double amplitude, int teiler, double phaseGrad,
                      int samples = 20000)
{
    constexpr int kEinschwingen = TruePeakDetektor::kTapsJePhase + 8;
    TruePeakDetektor d;
    d.vorbereiten (48000.0);
    const double phase = phaseGrad * kZweiPi / 360.0;
    double groesster = 0.0;
    for (int i = 0; i < samples; ++i)
    {
        const double x = amplitude * std::sin (kZweiPi * (double) i / (double) teiler + phase);
        const double tp = d.tick (x, x);
        if (i >= kEinschwingen)
            groesster = std::max (groesster, tp);
    }
    return groesster;
}

double db (double linear) { return linear > 0.0 ? 20.0 * std::log10 (linear) : -400.0; }

/** Der groesste Abtastwert desselben Signals — die Zahl, die ein Detektor
    OHNE Ueberabtastung melden wuerde. Sie ist der Gegenbeleg: ohne sie
    saehe man nicht, wie viel die Interpolation ueberhaupt findet. */
double sinusSamplePeak (double amplitude, int teiler, double phaseGrad,
                        int samples = 20000)
{
    const double phase = phaseGrad * kZweiPi / 360.0;
    double groesster = 0.0;
    for (int i = 0; i < samples; ++i)
        groesster = std::max (groesster,
            std::abs (amplitude * std::sin (kZweiPi * (double) i / (double) teiler + phase)));
    return groesster;
}
} // namespace

int main()
{
    std::cout << "== Nakama SONDE-013 M-02 - True Peak nach ITU-R BS.1770-5 ==" << std::endl;

    // ── 1. Die fuenf True-Peak-Faelle aus EBU Tech 3341 ───────────────────
    //
    // Sie stehen hier mit ihrer Nummer, damit ein Leser sie in der Norm
    // wiederfindet. Die Erwartung ist NICHT die Tabellenzahl der Norm,
    // sondern 20*log10(Amplitude) — dieselbe Zahl, aber ausgerechnet statt
    // abgelesen, und damit gegen einen Tippfehler in dieser Datei gesichert.
    abschnitt ("EBU Tech 3341, Testfaelle 15 bis 19 (Toleranz aus §49.3: +/-0,1 dB)");
    {
        const struct { const char* name; double amplitude; int teiler; double phase; } faelle[] = {
            { "15  fs/4, 0,50 FFS, 0 Grad",    0.50, 4, 0.0 },
            { "16  fs/4, 0,50 FFS, 45 Grad",   0.50, 4, 45.0 },
            { "17  fs/6, 0,50 FFS, 60 Grad",   0.50, 6, 60.0 },
            { "18  fs/8, 0,50 FFS, 67,5 Grad", 0.50, 8, 67.5 },
            { "19  fs/4, 1,41 FFS, 45 Grad",   1.41, 4, 45.0 },
        };
        for (const auto& f : faelle)
        {
            const double gemessen = db (sinusTruePeak (f.amplitude, f.teiler, f.phase));
            const double erwartet = db (f.amplitude);
            const double abw = std::abs (gemessen - erwartet);
            pruefe (abw <= 0.1,
                    juce::String ("Testfall ") + f.name + ": True Peak trifft die Amplitude",
                    "gemessen " + juce::String (gemessen, 3) + " dBTP, erwartet "
                    + juce::String (erwartet, 3) + ", Abweichung "
                    + juce::String (abw, 4) + " dB");
        }
    }

    // ── 2. Der Gegenbeleg: OHNE Ueberabtastung waeren es 3 dB weniger ─────
    //
    // Ohne diesen Abschnitt koennte der ganze Test gruen sein, weil der
    // Detektor einfach den Samplemaximalwert zurueckgibt. Testfall 16 ist
    // genau dafuer gebaut: seine Abtastpunkte liegen bei 45, 135, 225 und
    // 315 Grad, also alle bei +/-A/Wurzel(2). Der Sample-Peak liegt 3,01 dB
    // unter dem wahren Scheitel — und wer ihn meldet, ist um drei Dezibel zu
    // optimistisch.
    abschnitt ("Gegenbeleg: der Sample-Peak liegt bei Testfall 16 um 3 dB darunter");
    {
        const double samplePeak = db (sinusSamplePeak (0.50, 4, 45.0));
        const double truePeak   = db (sinusTruePeak (0.50, 4, 45.0));
        pruefe (std::abs (samplePeak - (-9.03)) < 0.05,
                "Sample-Peak von Testfall 16 liegt bei -9,03 dBFS",
                juce::String (samplePeak, 3) + " dBFS");
        pruefe (truePeak - samplePeak > 2.9,
                "der True Peak findet die drei Dezibel, die zwischen den Samples liegen",
                "Unterschied " + juce::String (truePeak - samplePeak, 3) + " dB");
    }

    // ── 3. Der Sampleraten-Sweep aus M-02 ────────────────────────────────
    //
    // Die EBU-Frequenzen sind an fs gebunden, also ist die normierte
    // Frequenz bei jeder Samplerate dieselbe und dieselbe Erwartung gilt.
    // Der Sweep prueft damit genau das, was M-02 "aequivalente generierte
    // Mehrsampleraten-Goldens" nennt: dass der Pfad nicht zufaellig bei
    // 48 kHz stimmt.
    abschnitt ("Sampleraten-Sweep: dieselbe Zusage bei 44,1 / 48 / 88,2 / 96 / 192 kHz");
    {
        const double raten[] = { 44100.0, 48000.0, 88200.0, 96000.0, 192000.0 };
        for (const double fs : raten)
        {
            double schlimmste = 0.0;
            for (const int teiler : { 4, 6, 8 })
            {
                TruePeakDetektor d;
                d.vorbereiten (fs);
                double groesster = 0.0;
                for (int i = 0; i < 20000; ++i)
                {
                    const double x = 0.5 * std::sin (kZweiPi * (double) i / (double) teiler + 0.7);
                    const double tp = d.tick (x, x);
                    if (i >= TruePeakDetektor::kTapsJePhase + 8)
                        groesster = std::max (groesster, tp);
                }
                schlimmste = std::max (schlimmste, std::abs (db (groesster) - db (0.5)));
            }
            pruefe (schlimmste <= 0.1,
                    "bei " + juce::String (fs / 1000.0, 1) + " kHz halten fs/4, fs/6 und fs/8 "
                    "die Toleranz",
                    "groesste Abweichung " + juce::String (schlimmste, 4) + " dB");
        }
    }

    // ── 4. DIE ENTSCHEIDUNG 8-FACH GEGEN 4-FACH, ALS ZAHL ────────────────
    //
    // M-02 verlangt, dass dieses Bein die Entscheidung MESSBAR trifft. Der
    // Restfehler der Ueberabtastung hat eine geschlossene Form: der wahre
    // Scheitel liegt hoechstens eine HALBE Stuetzstelle neben der naechsten,
    // also
    //
    //     Fehler(f) = 20*log10( cos( 2*pi * f/fs * 1/(2*Faktor) ) ).
    //
    // Bei 8-fach und fs/4 sind das -0,042 dB, bei 4-fach -0,17 dB. Die
    // erste Zahl haelt die +/-0,1 dB aus §49.3, die zweite nicht. GENAU DAS
    // ist die Begruendung fuer 8-fach, und sie steht als `constexpr` im
    // Produktpfad, nicht als Kommentar.
    abschnitt ("Die 8-fach-Entscheidung: die Schranke, die 4-fach reissen wuerde");
    {
        const double schranke8 = TruePeakDetektor::interpolationsfehlerDb (0.25);
        pruefe (std::abs (schranke8) <= 0.1,
                "8-fach: die ausgerechnete Schranke bei fs/4 haelt die Toleranz aus §49.3",
                juce::String (schranke8, 4) + " dB");

        // Dieselbe Formel mit Faktor 4 — von Hand, weil der Produktpfad den
        // Faktor 4 gar nicht kennt. Sie ist der Gegenbeleg: ohne sie waere
        // "8-fach ist genug" eine Behauptung ueber eine Alternative, die nie
        // gerechnet wurde.
        const double schranke4 = 20.0 * std::log10 (std::cos (kZweiPi * 0.25 / 8.0));
        pruefe (std::abs (schranke4) > 0.1,
                "4-fach wuerde sie bei derselben Frequenz REISSEN - deshalb 8-fach",
                juce::String (schranke4, 4) + " dB");

        // Und die wirkliche Messung bleibt innerhalb der ausgerechneten
        // Schranke. Liefe sie darueber, waere entweder die Formel falsch
        // oder der Filter — und beides waere ein Befund.
        const double wirklich = db (sinusTruePeak (0.5, 4, 45.0)) - db (0.5);
        pruefe (wirklich >= schranke8 - 0.02,
                "die WIRKLICHE Abweichung bleibt innerhalb der ausgerechneten Schranke",
                "gemessen " + juce::String (wirklich, 4) + " dB, Schranke "
                + juce::String (schranke8, 4) + " dB");
    }

    // ── 5. Fail-closed und Zahlenraender ─────────────────────────────────
    abschnitt ("Raender: Stille, NaN, Gleichanteil und der ungeruestete Detektor");
    {
        TruePeakDetektor d;
        d.vorbereiten (48000.0);
        double groesster = 0.0;
        for (int i = 0; i < 1000; ++i)
            groesster = std::max (groesster, d.tick (0.0, 0.0));
        pruefe (groesster == 0.0, "Stille ergibt exakt 0, nicht einen Filterrest",
                juce::String (groesster, 12));

        // Ein NaN im Eingang darf weder durchkommen noch im Filterzustand
        // bleiben: der Riegel liegt VOR der Rechnung, wie in der Engine.
        d.zuruecksetzen();
        for (int i = 0; i < 40; ++i)
            (void) d.tick (0.5, 0.5);
        const double mitNan = d.tick (std::numeric_limits<double>::quiet_NaN(), 0.5);
        pruefe (std::isfinite (mitNan), "ein NaN im Eingang erzeugt keinen NaN im Ausgang",
                juce::String (mitNan, 6));
        double nachNan = 0.0;
        for (int i = 0; i < 200; ++i)
            nachNan = std::max (nachNan, d.tick (0.5, 0.5));
        pruefe (std::isfinite (nachNan) && nachNan > 0.4,
                "und er bleibt nicht im Filterzustand haengen - danach misst er wieder",
                juce::String (nachNan, 6));

        // Ein Gleichanteil ist der Test der PHASENWEISEN Normierung. Bei
        // Normierung nur der Gesamtsumme wuerde jede Phase leicht anders
        // verstaerken, aus der Konstanten wuerde eine mit 8*fs modulierte
        // Welle, und ihr Maximum laege ueber dem Gleichanteil.
        d.zuruecksetzen();
        double dc = 0.0;
        for (int i = 0; i < 500; ++i)
        {
            const double tp = d.tick (0.5, 0.5);
            if (i >= TruePeakDetektor::kTapsJePhase + 8)
                dc = std::max (dc, tp);
        }
        pruefe (std::abs (dc - 0.5) < 1e-9,
                "ein Gleichanteil bleibt exakt er selbst - jede Phase hat DC-Verstaerkung 1",
                "gemessen " + juce::String (dc, 12) + ", Abweichung "
                + juce::String (std::abs (dc - 0.5), 12));

        // Fail-closed: ohne `vorbereiten()` gibt es keinen Verlauf, und der
        // Detektor liefert 0 statt in einen leeren Vektor zu greifen.
        TruePeakDetektor roh;
        pruefe (! roh.bereit() && roh.tick (0.9, 0.9) == 0.0,
                "ohne vorbereiten() misst er nicht, statt zu raten oder zu stuerzen");
    }

    // ── 6. Der Vertrag der Ueberabtastung selbst ─────────────────────────
    abschnitt ("Der Interpolator: Faktor, Taps und die Zusage ueber den Nulldurchgang");
    {
        pruefe (TruePeakDetektor::kFaktor == 8,
                "Ueberabtastfaktor ist 8 (BS.1770-4 Annex 2 verlangt mindestens 4)");
        pruefe (TruePeakDetektor::kTaps == TruePeakDetektor::kFaktor
                                         * TruePeakDetektor::kTapsJePhase + 1,
                "die Filterlaenge ist ungerade und zerlegt sich glatt in die Phasen",
                juce::String (TruePeakDetektor::kTaps) + " Taps, "
                + juce::String (TruePeakDetektor::kTapsJePhase) + " je Phase");

        // Jede Phase summiert sich auf 1 — die Zusage, die den Gleichanteil
        // oben traegt. Sie wird hier an den Koeffizienten selbst gemessen,
        // damit ein Fehler in der Normierung nicht erst am Signal auffaellt.
        const double* h = TruePeakDetektor::koeffizienten();
        double groessteAbweichung = 0.0;
        for (int phase = 0; phase < TruePeakDetektor::kFaktor; ++phase)
        {
            double summe = 0.0;
            for (int k = 0; phase + TruePeakDetektor::kFaktor * k < TruePeakDetektor::kTaps; ++k)
                summe += h[phase + TruePeakDetektor::kFaktor * k];
            groessteAbweichung = std::max (groessteAbweichung, std::abs (summe - 1.0));
        }
        pruefe (groessteAbweichung < 1e-12,
                "JEDE der acht Phasen summiert sich auf 1 - nicht nur die Gesamtsumme auf 8",
                "groesste Abweichung " + juce::String (groessteAbweichung, 15));
    }

    std::cout << "\n-----------------------------------------" << std::endl;
    std::cout << bestanden << " bestanden, " << fehler << " gescheitert" << std::endl;
    return fehler == 0 ? 0 : 1;
}
