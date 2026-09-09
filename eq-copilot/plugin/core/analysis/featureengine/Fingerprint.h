// FeatureEngine — Fingerprint: der Wiedererkennungswert einer Passage.
//
// NAK-225/S25d (09.09.2026), Definitionen herausgeloest aus der Klasse in
// core/analysis/FeatureEngine.h. Inhalt:
//
//   fingerprintSchritt   Ein Welch-Frame in den Akkumulator: Bandenergie,
//                        Chroma, Onset.
//   fingerprintJetzt     Der quantisierte Abzug, wie ihn der Vertrag fuehrt.
//   fingerprintLeeren    Der Rueckweg.
//
// Der Vergleich zweier Abzuege steht NICHT hier, sondern als
// fingerprintAehnlichkeit im Vertrag (featureengine/Vertrag.h): er gehoert den
// Verbrauchern, nicht der Engine.

#ifndef NAKAMA_FEATUREENGINE_TEIL
#error "Teilkopf von FeatureEngine.h - nur ueber FeatureEngine.h einbinden."
#endif

#pragma once

namespace nakama::analyse
{

//== Fingerprint (SONDE-013 M-26, M-27, M-31) =============================

/** Ein Welch-Frame der Hauptstufe in den Fingerprintakkumulator.

    Reihenfolge und Herkunft der drei Verlaeufe:

    - **Bandenergie**: die 221 Evidenzbaender werden auf 32 Gruppen
      gemittelt. 32 statt 221, weil ein Fingerprint GROB sein soll — er
      soll anderes Material erkennen, nicht dasselbe Material auf zwei
      Anlagen unterscheiden.
    - **Chroma**: jedes Band traegt zu genau einer Halbtonklasse bei,
      bestimmt aus seiner Mittenfrequenz gegen A4 = 440 Hz. Ueber alle
      Oktaven summiert — das ist der Punkt: derselbe Akkord in einer
      anderen Lage soll gleich aussehen.
    - **Onset**: der spektrale Fluss dieses Frames, an seiner Stelle im
      Zeitraster. Er kommt aus derselben Rechnung wie der
      Ereignisdetektor, aber ohne dessen Schwelle — hier zaehlt der
      Verlauf, nicht das Ereignis.

    ⚠️ Der Onsetverlauf braucht ein ZEITRASTER, und das ist die einzige
    Stelle, an der dieser Erzeuger etwas ueber die Passagenlaenge annimmt.
    Er verteilt die Frames gleichmaessig auf 32 Stuetzstellen und faengt
    von vorne an, sobald sie voll sind — jede Stuetzstelle traegt dann das
    Maximum ihrer Frames. Das ist kein Fenster, das ueberlaeuft, sondern
    eine Aufloesung, die mit der Laenge sinkt; ein Fingerprint ueber zehn
    Sekunden hat dieselben 32 Punkte wie einer ueber zwei. */
inline void FeatureEngine::fingerprintSchritt (const Stufe& s, double fluss) noexcept
{
    if ((int) fpBandSumme.size() < Fingerprint::kBaender)
        return;

    // Bandenergie: 221 Baender auf 32 Gruppen.
    constexpr int kProGruppe = (Gitter::evidenzBaender + Fingerprint::kBaender - 1)
                             / Fingerprint::kBaender;
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        const int von = s.bandVon[(std::size_t) b];
        const int bis = s.bandBis[(std::size_t) b];
        if (bis <= von)
            continue;
        double energie = 0.0;
        for (int k = von; k < bis; ++k)
            energie += s.psd[(std::size_t) k];
        energie /= (double) (bis - von);
        if (! std::isfinite (energie) || energie <= 0.0)
            continue;

        const int gruppe = std::min (b / kProGruppe, Fingerprint::kBaender - 1);
        fpBandSumme[(std::size_t) gruppe] += energie;
        ++fpBandAnzahl[(std::size_t) gruppe];

        // Chroma: die Halbtonklasse der Bandmitte gegen A4 = 440 Hz.
        const double hz = Gitter::evidenzMitte (b);
        if (hz > 0.0)
        {
            const double halbtoene = 12.0 * std::log2 (hz / 440.0);
            int klasse = (int) std::llround (halbtoene) % Fingerprint::kChroma;
            if (klasse < 0) klasse += Fingerprint::kChroma;
            fpChromaSumme[(std::size_t) klasse] += energie;
        }
    }

    // Onset: der Fluss an seiner Stelle im Zeitraster.
    if (fpOnsetStand >= Fingerprint::kOnsets)
    {
        // Raster voll: verdichten. Je zwei Stuetzstellen werden zu einer,
        // und die Frames je Stelle verdoppeln sich. Die Aufloesung sinkt
        // mit der Laenge, statt dass ein Fenster ueberlaeuft.
        for (int i = 0; i < Fingerprint::kOnsets / 2; ++i)
            fpOnset[(std::size_t) i] = std::max (fpOnset[(std::size_t) (2 * i)],
                                                 fpOnset[(std::size_t) (2 * i + 1)]);
        for (int i = Fingerprint::kOnsets / 2; i < Fingerprint::kOnsets; ++i)
            fpOnset[(std::size_t) i] = 0.0;
        fpOnsetStand = Fingerprint::kOnsets / 2;
        fpOnsetProStelle *= 2;
        fpOnsetInStelle = 0;
    }
    if (std::isfinite (fluss) && fluss > 0.0)
        fpOnset[(std::size_t) fpOnsetStand] =
            std::max (fpOnset[(std::size_t) fpOnsetStand], fluss);
    if (++fpOnsetInStelle >= fpOnsetProStelle)
    {
        fpOnsetInStelle = 0;
        ++fpOnsetStand;
    }
    ++fpFenster;
}

/** Baut den Fingerprint aus den Akkumulatoren.

    Quantisierung: jeder Verlauf wird auf sein eigenes Maximum normiert
    und dann auf 0..255 abgebildet. Die Normierung JE VERLAUF ist tragend
    — sie macht den Fingerprint pegelunabhaengig, und genau das soll er
    sein: dieselbe Passage lauter gespielt ist dasselbe Material.

    Die Bandenergie geht dabei ueber dB, nicht ueber Leistung. Linear
    quantisiert waeren 60 dB Dynamik in den unteren zwei Bytewerten
    zusammengedrueckt, und der ganze Verlauf saehe aus wie eine Spitze. */
inline Fingerprint FeatureEngine::fingerprintJetzt() const noexcept
{
    Fingerprint f;
    if (fpFenster < (std::uint32_t) kFingerprintMindestFenster
        || (int) fpBandSumme.size() < Fingerprint::kBaender)
        return f;                             // kein Bit, kein Wert

    // ── Bandenergie in dB, dann auf die Spanne normiert ──────────────
    double db[Fingerprint::kBaender] {};
    bool   hat[Fingerprint::kBaender] {};
    double maxDb = -1e300, minDb = 1e300;
    for (int i = 0; i < Fingerprint::kBaender; ++i)
    {
        if (fpBandAnzahl[(std::size_t) i] == 0)
            continue;
        const double mittel = fpBandSumme[(std::size_t) i]
                            / (double) fpBandAnzahl[(std::size_t) i];
        if (! (mittel > 0.0))
            continue;
        db[i] = 10.0 * std::log10 (mittel);
        if (! std::isfinite (db[i]))
            continue;
        hat[i] = true;
        maxDb = std::max (maxDb, db[i]);
        minDb = std::min (minDb, db[i]);
    }
    if (maxDb <= -1e299)
        return f;                             // nichts Messbares
    // Spanne auf hoechstens 96 dB deckeln: darunter ist alles Rauschen,
    // und eine unbegrenzte Spanne machte einen einzelnen stillen Bin zum
    // Massstab des ganzen Fingerprints.
    const double unten = std::max (minDb, maxDb - 96.0);
    const double spanne = std::max (maxDb - unten, 1e-9);
    for (int i = 0; i < Fingerprint::kBaender; ++i)
        f.bandEnergie[(std::size_t) i] = hat[i]
            ? (std::uint8_t) std::clamp (
                  std::llround (255.0 * (db[i] - unten) / spanne), 0LL, 255LL)
            : (std::uint8_t) 0;

    // ── Chroma und Onset: linear auf ihr eigenes Maximum ─────────────
    auto normiere = [] (const double* quelle, std::uint8_t* ziel, int n)
    {
        double gross = 0.0;
        for (int i = 0; i < n; ++i)
            if (std::isfinite (quelle[i]))
                gross = std::max (gross, quelle[i]);
        if (! (gross > 0.0))
            return;
        for (int i = 0; i < n; ++i)
            ziel[(std::size_t) i] = std::isfinite (quelle[i])
                ? (std::uint8_t) std::clamp (
                      std::llround (255.0 * quelle[i] / gross), 0LL, 255LL)
                : (std::uint8_t) 0;
    };
    normiere (fpChromaSumme.data(), f.chroma, Fingerprint::kChroma);
    normiere (fpOnset.data(), f.onset, Fingerprint::kOnsets);

    f.gesetzt = true;
    f.fenster = fpFenster;
    return f;
}

/** Leert den Fingerprintakkumulator. §32.3 nennt das Fingerprintfenster
    ausdruecklich neben FFT, Loudness und Korrelation — es ueberbrueckt
    keine Grenze (M-27). */
inline void FeatureEngine::fingerprintLeeren() noexcept
{
    for (auto& v : fpBandSumme)   v = 0.0;
    for (auto& v : fpBandAnzahl)  v = 0;
    for (auto& v : fpChromaSumme) v = 0.0;
    for (auto& v : fpOnset)       v = 0.0;
    fpOnsetStand = 0;
    fpOnsetProStelle = 1;
    fpOnsetInStelle = 0;
    fpFenster = 0;
}

} // namespace nakama::analyse
