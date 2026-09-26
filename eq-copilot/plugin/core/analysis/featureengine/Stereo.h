// FeatureEngine — Stereo: Kreuzspektren, Kohaerenz, Breite.
//
// NAK-225/S25d (09.09.2026), Definitionen herausgeloest aus der Klasse in
// core/analysis/FeatureEngine.h. Inhalt:
//
//   stereoSchritt        Ein Welch-Frame einer Stufe in die bandweisen
//                        Kreuzspektren und in den Ring je Bin. Ohne zweite
//                        FFT: die Engine transformiert MID und SIDE, und L/R
//                        folgen daraus linear.
//   stereoSample         Die Energien, die kein Frame braucht (Korrelation).
//   stereoAuswerten      Aus den Akkumulatoren werden Bandwerte und Skalare.
//   stereoFensterLeeren, stereoRingVorschub, stereoLeeren
//                        Die Rueckwege: jeder Akkumulator hat einen.
//
// aktivieren und abklingen gehoeren zusammen (CLAUDE.md): stereoSchritt und
// die drei Rueckwege stehen deshalb in derselben Datei.
//
// NAK-380 Etappe 5 (R-380-3, T-380-6, DSP-23): die Magnitude-Squared
// Coherence entsteht je BIN aus Auto- und Kreuzspektren, die ueber die Frames
// der letzten W Evidenzfenster summiert sind (Ring, W_H = kStereoRingHaupt,
// W_B = kStereoRingBass), und wird erst danach im Band gemittelt; die Phase
// wird am Bin der geometrischen Bandmitte gelesen, die Laufzeit als
// Phasensteigung ueber die Bins des Bandes (R-380-13 (ii): kleinste Quadrate
// ueber die sequenziell entwickelten Binphasen, nie aus dem Lag-1-Produkt).
// Korrelation, Mid/Side, Seitenanteil, Zeitperzentile und Folddown bleiben je
// Evidenzfenster.

#ifndef NAKAMA_FEATUREENGINE_TEIL
#error "Teilkopf von FeatureEngine.h - nur ueber FeatureEngine.h einbinden."
#endif

#pragma once

namespace nakama::analyse
{

//== Der Ring je Bin: Aufbau (T-380-6) ====================================
//
// EIN flacher Vektor `stereoRing` aus `StereoBinAkku` (vier double: Sxx,
// Syy, Re Sxy, Im Sxy), angelegt in `vorbereiten`:
//
//   [Zaehlerkopf][Band 0][Band 1] ... [Band 220]
//
// Band b traegt je Bin seiner Stufe W + 1 Elemente hintereinander: die W
// Ringslots (einer je Evidenzfenster, W = kStereoRingBass unter der
// Trennung, sonst kStereoRingHaupt) und am Index W das Kurzfenster der
// Persistenz (A-4). Der Zaehlerkopf haelt je Band und Slot die Zahl der
// gueltigen Frames (Freiheitsgrade) als double, vier Zaehler je Element in
// der Reihenfolge sxx, syy, sxyRe, sxyIm; er faengt jedes Band, auch eines
// ohne Bin, damit sein Aufbau nicht an der Abtastrate haengt. Bytes je
// Engine: 32 je Bin und Slot plus 8 je Band und Slot (M-85; bei 44,1 kHz
// 236 224 B, bei 48 kHz 217 792 B).

inline int FeatureEngine::stereoRingSlots (int b, int trenn) noexcept
{
    return (b < trenn ? kStereoRingBass : kStereoRingHaupt) + 1;
}

inline int FeatureEngine::stereoBins (int b) const noexcept
{
    const auto i = (std::size_t) b;
    return std::max (0, bass.bandBis[i] - bass.bandVon[i])
         + std::max (0, haupt.bandBis[i] - haupt.bandVon[i]);
}

inline std::size_t FeatureEngine::stereoZaehlerElemente (int trenn) noexcept
{
    const std::size_t zaehler = (std::size_t) trenn * ((std::size_t) kStereoRingBass + 1u)
        + ((std::size_t) Gitter::evidenzBaender - (std::size_t) trenn) * ((std::size_t) kStereoRingHaupt + 1u);
    return (zaehler + 3u) / 4u;
}

inline std::size_t FeatureEngine::stereoZaehlerBasis (int b, int trenn) noexcept
{
    return b < trenn
        ? (std::size_t) b * ((std::size_t) kStereoRingBass + 1u)
        : (std::size_t) trenn * ((std::size_t) kStereoRingBass + 1u)
              + ((std::size_t) b - (std::size_t) trenn) * ((std::size_t) kStereoRingHaupt + 1u);
}

inline double& FeatureEngine::stereoZaehler (std::size_t i) noexcept
{
    auto& e = stereoRing[i / 4u];
    switch (i % 4u)
    {
        case 0:  return e.sxx;
        case 1:  return e.syy;
        case 2:  return e.sxyRe;
        default: return e.sxyIm;
    }
}

inline double FeatureEngine::stereoZaehlerWert (std::size_t i) const noexcept
{
    const auto& e = stereoRing[i / 4u];
    switch (i % 4u)
    {
        case 0:  return e.sxx;
        case 1:  return e.syy;
        case 2:  return e.sxyRe;
        default: return e.sxyIm;
    }
}

/** Gueltige Frames des Bandes in den belegten Ringslots seiner Stufe - das
    Feld `freiheitsgrade`. Nie aelter als W Fenster: der Ring hat W Slots,
    und der Vorschub leert den Slot, den er neu belegt. */
inline std::uint32_t FeatureEngine::stereoRingFrames (int b, int trenn) const noexcept
{
    if (stereoRing.empty())
        return 0u;
    const bool bassBand = b < trenn;
    const auto w = (std::uint32_t) (bassBand ? kStereoRingBass : kStereoRingHaupt);
    const auto stand = bassBand ? stereoRingStandBass : stereoRingStandHaupt;
    const auto belegt = std::min (bassBand ? stereoRingBelegtBass : stereoRingBelegtHaupt, w);
    const std::size_t z0 = stereoZaehlerBasis (b, trenn);
    double frames = 0.0;
    for (std::uint32_t j = 0; j < belegt; ++j)
    {
        const std::uint32_t s = (stand + w - j) % w;     // j Fenster zurueck
        frames += stereoZaehlerWert (z0 + (std::size_t) s);
    }
    return (std::uint32_t) frames;
}

//== Stereoevidenz (SONDE-013 M-08, M-10 bis M-12) ========================

/** Ein Welch-Frame einer Stufe in die bandweisen Kreuzspektren und den Ring.

    🔑 WARUM KEINE ZWEITE FFT. Die Engine transformiert MID und SIDE, nicht
    L und R. Die Fouriertransformation ist linear, und M = (L+R)/2,
    S = (L-R)/2 sind Linearkombinationen — also gilt im Spektrum
    EXAKT dasselbe:

        L(f) = M(f) + S(f),      R(f) = M(f) - S(f).

    Zwei zusaetzliche 4096-Punkt-FFTs je Fenster waeren also nicht
    genauer, sondern nur teurer. Der Umweg ueber M/S ist hier kein
    Kompromiss, sondern die identische Rechnung.

    ⚠️ Was NICHT gilt: das energienormierte M/S aus §40.3
    (M = (L+R)/√2) ist eine ANDERE Rechnung als der physische
    Mono-Check (L+R)/2. Diese Engine fuehrt durchgehend die Halbierung,
    und der Mono-Folddown wird deshalb am wirklich gefalteten Puffer
    gemessen (siehe `verarbeiteSamples`), nicht aus diesen Spektren
    geschaetzt.

    Die Nyquist-Kappe aus M-10 wirkt hier ueber die Bandzuordnung: ein
    Band ueber `kappeBand` hat `bandBis <= bandVon`, bekommt also keinen
    einzigen Bin und bleibt bei null Freiheitsgraden. Es entsteht keine
    zweite Kappenregel.

    NAK-380 Etappe 5: je Bin gehen Sxx = |L|^2, Syy = |R|^2 und Sxy =
    L·conj(R) in den laufenden Ringslot der Stufe und in das Kurzfenster, und
    der Zaehler des Bandes steigt je Slot um eins - vier Additionen je Bin
    und Ziel, keine Allokation. Der NaN-Riegel bleibt je Band und Frame: ist
    eine Bandsumme nicht endlich, zaehlt der Frame fuer DIESES Band nirgends
    (M-95), die uebrigen Baender zaehlen weiter. `nanKreuzBin` setzt nur der
    Testzugang (M-95): er macht das Kreuzspektrum genau eines Bins in genau
    diesem Frame nicht endlich; das Produkt ruft mit -1. */
inline void FeatureEngine::stereoSchritt (const Stufe& s, bool zaehlKurzfenster,
                                          int nanKreuzBin) noexcept
{
    if ((int) stereoAkku.size() < Gitter::evidenzBaender || stereoRing.empty())
        return;

    const int trenn = trennIndex();
    const bool bassStufe = &s == &bass;
    const auto w = (std::size_t) (bassStufe ? kStereoRingBass : kStereoRingHaupt);
    const auto slot = (std::size_t) (bassStufe ? stereoRingStandBass : stereoRingStandHaupt);
    const std::size_t slots = w + 1u;               // W Ringslots und das Kurzfenster
    constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();
    std::size_t basis = stereoZaehlerElemente (trenn);
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        const std::size_t bandBasis = basis;
        basis += (std::size_t) stereoBins (b) * (std::size_t) stereoRingSlots (b, trenn);
        const int von = s.bandVon[(std::size_t) b];
        const int bis = s.bandBis[(std::size_t) b];
        if (bis <= von)
            continue;                       // nicht messbar (Kappe, Aufloesung)

        double smm = 0.0, sss = 0.0, sll = 0.0, srr = 0.0;
        double sxyRe = 0.0, sxyIm = 0.0;
        for (int k = von; k < bis; ++k)
        {
            const double mr = s.fftM.realTeil (k), mi = s.fftM.imagTeil (k);
            const double sr = s.fftS.realTeil (k), si = s.fftS.imagTeil (k);
            const double lr = mr + sr, li = mi + si;      // L = M + S
            const double rr = mr - sr, ri = mi - si;      // R = M - S
            smm += mr * mr + mi * mi;
            sss += sr * sr + si * si;
            sll += lr * lr + li * li;
            srr += rr * rr + ri * ri;
            // L · conj(R)
            sxyRe += k == nanKreuzBin ? kNaN : lr * rr + li * ri;
            sxyIm += k == nanKreuzBin ? kNaN : li * rr - lr * ri;
        }
        if (! (std::isfinite (smm) && std::isfinite (sss)
               && std::isfinite (sll) && std::isfinite (srr)
               && std::isfinite (sxyRe) && std::isfinite (sxyIm)))
            continue;                       // NaN-Riegel beim ERZEUGEN, je Band und Frame

        auto& a = stereoAkku[(std::size_t) b];
        a.smm += smm; a.sss += sss;
        a.sll += sll; a.srr += srr;
        a.sxyRe += sxyRe; a.sxyIm += sxyIm;
        ++a.frames;

        auto& kz = stereoKurz[(std::size_t) b];
        kz.sll += sll; kz.srr += srr;
        kz.sxyRe += sxyRe; kz.sxyIm += sxyIm;
        ++kz.frames;

        // Der Ring je Bin (T-380-6): die Bandsummen oben sind endlich, also
        // ist es jeder ihrer Summanden - dieselben Werte je Bin noch einmal
        // (samt dem Setzer des Testzugangs, damit ein fehlender Riegel das
        // Nichtendliche auch hier truege), in den laufenden Slot und das
        // Kurzfenster (A-4).
        for (int k = von; k < bis; ++k)
        {
            const double mr = s.fftM.realTeil (k), mi = s.fftM.imagTeil (k);
            const double sre = s.fftS.realTeil (k), sim = s.fftS.imagTeil (k);
            const double lr = mr + sre, li = mi + sim;
            const double rr = mr - sre, ri = mi - sim;
            const double xx = lr * lr + li * li, yy = rr * rr + ri * ri;
            const double re = k == nanKreuzBin ? kNaN : lr * rr + li * ri;
            const double im = k == nanKreuzBin ? kNaN : li * rr - lr * ri;
            auto* bin = &stereoRing[bandBasis + ((std::size_t) k - (std::size_t) von) * slots];
            bin[slot].sxx += xx; bin[slot].syy += yy;
            bin[slot].sxyRe += re; bin[slot].sxyIm += im;
            bin[w].sxx += xx; bin[w].syy += yy;
            bin[w].sxyRe += re; bin[w].sxyIm += im;
        }
        const std::size_t z0 = stereoZaehlerBasis (b, trenn);
        stereoZaehler (z0 + slot) += 1.0;
        stereoZaehler (z0 + w) += 1.0;

        // Der Zeitverlauf des SEITENANTEILS, ein Wert je Frame. Er ist
        // die Grundlage der Zeitperzentile aus §40.1 - und die einzige
        // Groesse hier, die ueber die Zeit und nicht ueber die Baender
        // geht.
        const double gesamt = smm + sss;
        if (gesamt > 0.0 && (int) stereoVerlauf.size() > b)
        {
            const double anteil = sss / gesamt;
            stereoVerlauf[(std::size_t) b].schiebe (
                (float) (10.0 * std::log10 (std::max (anteil, 1e-12))));
        }
    }

    // Ein abgeschlossenes KURZFENSTER: Korrelation einfrieren und die
    // Persistenz zaehlen. Sie wird HIER gezaehlt und nicht am Ende
    // gerechnet, weil sie eine Aussage ueber den VERLAUF ist - am Ende
    // stuende nur noch das Gesamtmittel zur Verfuegung.
    // Das Kurzfenster zaehlt NUR die Hauptstufe. Zwei Stufen mit
    // verschiedenem Hop wuerden es doppelt und ungleichmaessig
    // weiterschieben, und die Persistenz waere ein Anteil ueber zwei
    // Zeitachsen — derselbe Fehler, den die Abdeckung nebenan vermeidet.
    if (! zaehlKurzfenster)
        return;
    if (++stereoKurzFrames >= kKorrelationKurzFrames)
    {
        stereoKurzFrames = 0;
        ++stereoKurzfenster;
        std::size_t basisKurz = stereoZaehlerElemente (trenn);
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            const auto slotsB = (std::size_t) stereoRingSlots (b, trenn);
            const auto binsB = (std::size_t) stereoBins (b);
            const std::size_t bandBasis = basisKurz;
            basisKurz += binsB * slotsB;
            const std::size_t kurz = slotsB - 1u;          // Index des Kurzfensters
            const std::size_t zKurz = stereoZaehlerBasis (b, trenn) + kurz;
            auto& kz = stereoKurz[(std::size_t) b];
            if (kz.frames > 0)
            {
                const double nenner = std::sqrt (kz.sll * kz.srr);
                if (nenner > 0.0)
                {
                    // NAK-181 R5a: DIESES Band hatte in DIESEM Kurzfenster
                    // einen gueltigen Nenner — der Messnachweis, an dem
                    // `persistenzGesetzt` haengt.
                    ++stereoKurzfensterBand[(std::size_t) b];
                    const double r = kz.sxyRe / nenner;
                    if (std::isfinite (r))
                    {
                        stereoKorrKurz[(std::size_t) b] =
                            (float) std::clamp (r, -1.0, 1.0);
                        stereoKorrKurzGesetzt[(std::size_t) b] = 1u;
                    }
                    // NAK-380 A-4: die Kohaerenz des Kurzfensters ist dieselbe
                    // Groesse wie die des Evidenzfensters - Bandmittel der MSC
                    // je Bin ueber die Bins mit Energie, nicht die Bandsumme.
                    double summe = 0.0;
                    int mitEnergie = 0;
                    for (std::size_t i = 0; i < binsB; ++i)
                    {
                        const auto& x = stereoRing[bandBasis + i * slotsB + kurz];
                        const double n2 = x.sxx * x.syy;
                        if (n2 > 0.0)
                        {
                            summe += (x.sxyRe * x.sxyRe + x.sxyIm * x.sxyIm) / n2;
                            ++mitEnergie;
                        }
                    }
                    const double koh = mitEnergie > 0 ? summe / (double) mitEnergie : 0.0;
                    if (mitEnergie > 0 && std::isfinite (koh) && koh >= kKohaerenzSchwellePhase
                        && stereoZaehlerWert (zKurz) >= (double) kWelchMindestFrames)
                        ++stereoPersistenzZaehler[(std::size_t) b];
                }
            }
            kz = StereoAkku {};
            for (std::size_t i = 0; i < binsB; ++i)
                stereoRing[bandBasis + i * slotsB + kurz] = StereoBinAkku {};
            stereoZaehler (zKurz) = 0.0;
        }
    }
}

/** Der laufende Mono-Folddown und die L/R-Balance des Evidenzfensters.

    Beide entstehen im SAMPLEPFAD und nicht aus den Spektren: §40.3
    verlangt den Folddown "am wirklich gefalteten Puffer", und genau das
    ist `(l + r) / 2`. Eine Schaetzung aus der Korrelation liegt bei
    dekorrelierten Kanaelen um Dezibel daneben - der Golden misst die
    Uebereinstimmung auf 0,25 dB. */
inline void FeatureEngine::stereoSample (double l, double r) noexcept
{
    const double mono = 0.5 * (l + r);
    stereoMonoEnergie += mono * mono;
    stereoStereoEnergie += 0.5 * (l * l + r * r);
    stereoLEnergie += l * l;
    stereoREnergie += r * r;
}

/** Wertet die Akkumulatoren zu `StereoBandwert`n aus. Am Ende eines
    Evidenzfensters, zusammen mit den Baendern und der Verteilung.

    NAK-380 Etappe 5 (T-380-6): Freiheitsgrade und Fensterdauer kommen aus
    dem Ring - die gueltigen Frames der letzten W Evidenzfenster und die
    Summe ihrer Hopdauern. Die Kohaerenz (Stufe 1) ist das Mittel der MSC je
    Bin |Sxy|^2/(Sxx·Syy) ueber die Bins mit Sxx·Syy > 0, jeder Bin ueber die
    belegten Slots summiert; ohne Bin mit Energie oder unter
    `kWelchMindestFrames` gibt es kein Bit. Die Phase (Stufe 2) steht am Bin
    der geometrischen Bandmitte. Die Laufzeit nach R-380-13 (ii): je Band mit
    Kohaerenzbit werden die Binphasen phi_k = arg Sxy[k] der Bins mit
    Sxx·Syy > 0 und endlichen Summen gelesen (Ringsumme, dieselben Akkus wie
    die MSC), in aufsteigender Binfolge sequenziell entwickelt (Sprung ueber
    pi um 2 pi berichtigt; ein uebersprungener Bin verdoppelt den Schritt und
    ist erlaubt), und die Gruppenlaufzeit ist die Steigung der kleinsten
    Quadrate s = Summe (k - k̄)(phi_k - phī)/Summe (k - k̄)^2 ueber die K_eff
    genutzten Bins, tau = s/(2 pi Δf) mit Δf = fs/N der Stufe, Vorzeichen so,
    dass ein verzoegertes R (Sxy = L·conj(R) ~ e^{+i 2 pi k d/N}) eine
    positive Laufzeit ergibt. K_eff >= 2, sonst kein Laufzeitbit (nie 0 als
    Wert). Eindeutig bis |tau| < 1/(2 Δf). Zweiter Durchlauf ueber die Bins
    mit laufenden Summen (Summe k, Summe phi, Summe k^2, Summe k·phi; k relativ
    zum ersten Bin, die Steigung ist verschiebungsfrei), kein Speicher. Phase
    und Laufzeit nur ueber `kKohaerenzSchwellePhase`, weil die Laufzeit eine
    Deutung der Interchannel-Phase ist (§40.1). Kohaerenz, Phase und Laufzeit
    gibt es nur fuer ein Band, das in DIESEM Evidenzfenster gemessen wurde (wie
    bisher unter `basisGesetzt`). */
inline void FeatureEngine::stereoAuswerten() noexcept
{
    if ((int) stereoAkku.size() < Gitter::evidenzBaender
        || (int) stereoErgebnis.size() < Gitter::evidenzBaender)
        return;

    const int trenn = trennIndex();
    std::size_t basis = stereoZaehlerElemente (trenn);
    float folge[kVerteilungPlaetze];
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        const auto slotsB = (std::size_t) stereoRingSlots (b, trenn);
        const int binsB = stereoBins (b);
        const std::size_t bandBasis = basis;
        basis += (std::size_t) binsB * slotsB;
        const bool bassBand = b < trenn;
        const Stufe& st = bassBand ? bass : haupt;

        const auto& a = stereoAkku[(std::size_t) b];
        auto& e = stereoErgebnis[(std::size_t) b];
        e = StereoBandwert {};
        const std::uint32_t ringFrames = stereoRingFrames (b, trenn);
        e.freiheitsgrade = ringFrames;
        e.fensterDauerMs = st.fs > 0.0
            ? (float) ((double) ringFrames * 1000.0 * (double) st.hop / st.fs) : 0.0f;
        if (a.frames == 0u)
            continue;                       // kein Bit, kein Wert

        // ⚠️ NUR die Gesamtenergie muss positiv sein, nicht die Mid-Energie.
        // Bei perfekter Polaritaetsinvertierung (R = -L) ist die
        // Mid-Energie EXAKT null - und genau dieser Fall ist der
        // interessanteste, den Stereoanalyse kennt. Eine Bedingung
        // `smm > 0` sperrte ihn aus, und das Band schwiege ausgerechnet
        // dort, wo es am meisten zu sagen haette (gemessen beim Bau:
        // Korrelation 0.000 statt -1). `midDb` laeuft dann ueber das
        // Epsilon unten an die Untergrenze - eine Aussage, kein
        // Schweigen.
        const double gesamt = a.smm + a.sss;
        if (gesamt > 0.0)
        {
            e.basisGesetzt = true;
            e.midDb  = (float) (10.0 * std::log10 (std::max (a.smm / (double) a.frames, 1e-30)));
            e.sideDb = (float) (10.0 * std::log10 (std::max (a.sss / (double) a.frames, 1e-30)));
            e.seitenanteilDb =
                (float) (10.0 * std::log10 (std::max (a.sss / gesamt, 1e-12)));

            const double nenner = std::sqrt (a.sll * a.srr);
            if (nenner > 0.0)
            {
                const double rMittel = a.sxyRe / nenner;
                if (std::isfinite (rMittel))
                {
                    e.korrelationMittel = (float) std::clamp (rMittel, -1.0, 1.0);
                    e.korrelationMittelGesetzt = true;   // NAK-181 R5
                }
            }
            if (stereoKorrKurzGesetzt[(std::size_t) b] != 0u)
            {
                e.korrelationKurz = stereoKorrKurz[(std::size_t) b];
                e.korrelationKurzGesetzt = true;         // NAK-181 R5
            }

            // Stufe 1: Kohaerenz nur mit genug Frames UND Energie, je Bin
            // aus dem Ring, danach im Band gemittelt (R-380-3).
            if (ringFrames >= (std::uint32_t) kWelchMindestFrames && binsB > 0)
            {
                const auto w = (std::uint32_t) (bassBand ? kStereoRingBass : kStereoRingHaupt);
                const auto stand = bassBand ? stereoRingStandBass : stereoRingStandHaupt;
                const auto belegt = std::min (bassBand ? stereoRingBelegtBass : stereoRingBelegtHaupt, w);
                const int von = st.bandVon[(std::size_t) b];
                const double df = st.fs / (double) st.punkte;
                // Der Bin, der der geometrischen Bandmitte am naechsten liegt.
                const int kMitte = std::clamp ((int) std::llround (Gitter::evidenzMitte (b) / df),
                                               von, von + binsB - 1);
                double summe = 0.0, mitteRe = 0.0, mitteIm = 0.0;
                int mitEnergie = 0;
                bool mitteEnergie = false;
                for (int i = 0; i < binsB; ++i)
                {
                    const std::size_t binBasis = bandBasis + (std::size_t) i * slotsB;
                    double xx = 0.0, yy = 0.0, re = 0.0, im = 0.0;
                    for (std::uint32_t j = 0; j < belegt; ++j)
                    {
                        const std::uint32_t s = (stand + w - j) % w;     // j Fenster zurueck
                        const auto& x = stereoRing[binBasis + (std::size_t) s];
                        xx += x.sxx; yy += x.syy;
                        re += x.sxyRe; im += x.sxyIm;
                    }
                    const double n2 = xx * yy;
                    if (n2 > 0.0)
                    {
                        summe += (re * re + im * im) / n2;
                        ++mitEnergie;
                    }
                    if (von + i == kMitte)
                    {
                        mitteRe = re;
                        mitteIm = im;
                        mitteEnergie = n2 > 0.0;
                    }
                }
                const double koh = mitEnergie > 0 ? summe / (double) mitEnergie : 0.0;
                if (mitEnergie > 0 && std::isfinite (koh))
                {
                    e.kohaerenzGesetzt = true;
                    e.kohaerenz = (float) std::clamp (koh, 0.0, 1.0);
                    // Stufe 2: Phase und Laufzeit nur ueber der benannten Schwelle.
                    if (koh > kKohaerenzSchwellePhase)
                    {
                        const double phi = std::atan2 (mitteIm, mitteRe);
                        if (mitteEnergie && std::isfinite (phi))
                        {
                            e.phaseGesetzt = true;
                            e.phaseRad = (float) phi;
                        }
                        // Laufzeit (R-380-13 (ii)), zweiter Durchlauf: Steigung der kleinsten
                        // Quadrate der entwickelten Binphasen; R verzoegert: s > 0, tau > 0.
                        double sk = 0.0, sp = 0.0, skk = 0.0, skp = 0.0, phiVor = 0.0, phiEntw = 0.0;
                        int kEff = 0;
                        for (int i = 0; i < binsB; ++i)
                        {
                            double xx = 0.0, yy = 0.0, re = 0.0, im = 0.0;
                            for (std::uint32_t j = 0; j < belegt; ++j)
                            {
                                const auto& x = stereoRing[bandBasis + (std::size_t) i * slotsB + (stand + w - j) % w];
                                xx += x.sxx; yy += x.syy; re += x.sxyRe; im += x.sxyIm;
                            }
                            const double n2 = xx * yy;
                            if (! (n2 > 0.0 && std::isfinite (n2) && std::isfinite (re) && std::isfinite (im)))
                                continue;                       // kein genutzter Bin
                            const double phiK = std::atan2 (im, re);
                            double sprung = phiK - phiVor;      // Sprung ueber pi: um 2 pi berichtigt
                            sprung += sprung > kPi ? -2.0 * kPi : (sprung < -kPi ? 2.0 * kPi : 0.0);
                            phiEntw = kEff == 0 ? phiK : phiEntw + sprung;
                            phiVor = phiK;
                            sk += (double) i; sp += phiEntw; skk += (double) i * (double) i; skp += (double) i * phiEntw;
                            ++kEff;
                        }
                        const double nennerS = (double) kEff * skk - sk * sk;
                        const double tauMs = kEff >= 2 && nennerS > 0.0
                            ? 1000.0 * ((double) kEff * skp - sk * sp) / nennerS / (2.0 * kPi * df) : 0.0;
                        e.laufzeitGesetzt = kEff >= 2 && nennerS > 0.0 && std::isfinite (tauMs);
                        e.laufzeitMs = e.laufzeitGesetzt ? (float) tauMs : 0.0f;
                    }
                }
            }

            // 🔑 NAK-181 R5a: das Bit haengt am BANDWEISEN Nachweis, der
            // Nenner am Evidenzfenster. `stereoKurzfenster > 0` allein
            // waere fuer ein stilles Band ein Bit ohne Messung.
            if (stereoKurzfensterBand[(std::size_t) b] > 0u && stereoKurzfenster > 0)
            {
                e.persistenz = (float) std::clamp (
                    (double) stereoPersistenzZaehler[(std::size_t) b]
                    / (double) stereoKurzfenster, 0.0, 1.0);
                e.persistenzGesetzt = true;
            }

            if ((int) stereoVerlauf.size() > b
                && stereoVerlauf[(std::size_t) b].gefuellt >= 4)
            {
                const int n = ringInZeitfolge (stereoVerlauf[(std::size_t) b], folge);
                std::sort (folge, folge + n);
                e.perzentileGesetzt = true;
                e.p10Db = (float) perzentil (folge, n, 0.10);
                e.p50Db = (float) perzentil (folge, n, 0.50);
                e.p95Db = (float) perzentil (folge, n, 0.95);
            }
        }
    }

    // ⚠️ Die zwei Grenzfaelle sind AUSSAGEN, kein Schweigen — und beim
    // ersten Bau war es umgekehrt. Bei Polaritaetsinvertierung ist die
    // Monosumme exakt null, bei einem stillen Kanal eine Kanalenergie;
    // ein `log10(0)` ist -unendlich, und ohne Bit haette der Empfaenger
    // an der wichtigsten Stelle nichts erfahren: „die Monosumme loescht
    // sich vollstaendig aus" ist die schaerfste Stereoaussage, die es
    // gibt. Beide Werte laufen deshalb ueber ein Epsilon und werden auf
    // die Vertragsgrenze +/-400 dB geklemmt, die das Schema ohnehin
    // zieht. Ein Bit fehlt nur, wenn gar nichts gemessen wurde.
    stereoSkalareErgebnis = StereoSkalare {};
    constexpr double kEps = 1e-40;
    if (stereoStereoEnergie > 0.0)
    {
        stereoSkalareErgebnis.folddownGesetzt = true;
        stereoSkalareErgebnis.monoFolddownDb = (float) std::clamp (
            10.0 * std::log10 (std::max (stereoMonoEnergie, kEps) / stereoStereoEnergie),
            -400.0, 400.0);
    }
    if (stereoLEnergie > 0.0 || stereoREnergie > 0.0)
    {
        stereoSkalareErgebnis.balanceGesetzt = true;
        stereoSkalareErgebnis.lrBalanceDb = (float) std::clamp (
            10.0 * std::log10 (std::max (stereoLEnergie, kEps)
                             / std::max (stereoREnergie, kEps)),
            -400.0, 400.0);
    }
}

/** Leert alles, was zu GENAU DIESEM Evidenzfenster gehoert: die
    Bandsummen, das Kurzfenster (Bandsumme und je Bin), Verlauf,
    Persistenz und die Skalare. Der Ring bleibt - ihn schiebt
    `stereoRingVorschub`, ihn leert `stereoLeeren`. */
inline void FeatureEngine::stereoFensterLeeren() noexcept
{
    for (auto& a : stereoAkku)  a = StereoAkku {};
    for (auto& a : stereoKurz)  a = StereoAkku {};
    for (auto& r : stereoVerlauf) r.leeren();
    for (auto& v : stereoKorrKurz) v = 0.0f;
    for (auto& v : stereoKorrKurzGesetzt) v = 0u;
    for (auto& v : stereoPersistenzZaehler) v = 0u;
    for (auto& v : stereoKurzfensterBand) v = 0u;   // NAK-181 R5a
    stereoKurzFrames = 0;
    stereoKurzfenster = 0;
    stereoMonoEnergie = stereoStereoEnergie = 0.0;
    stereoLEnergie = stereoREnergie = 0.0;
    if (stereoRing.empty())
        return;
    // Das Kurzfenster je Bin (A-4) teilt den Lebenszyklus von `stereoKurz`.
    const int trenn = trennIndex();
    std::size_t basis = stereoZaehlerElemente (trenn);
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        const auto slotsB = (std::size_t) stereoRingSlots (b, trenn);
        const auto binsB = (std::size_t) stereoBins (b);
        for (std::size_t i = 0; i < binsB; ++i)
            stereoRing[basis + i * slotsB + slotsB - 1u] = StereoBinAkku {};
        stereoZaehler (stereoZaehlerBasis (b, trenn) + slotsB - 1u) = 0.0;
        basis += binsB * slotsB;
    }
}

/** Der Rueckweg eines Evidenzfensters (`evidenzLeeren`): das Fenster
    faellt, der Ring schiebt je Stufe genau einen Slot weiter und leert
    den Slot, den das naechste Fenster belegt - er traegt sonst das Fenster
    von vor W Fenstern. Die belegten Fenster saettigen bei W. */
inline void FeatureEngine::stereoRingVorschub() noexcept
{
    stereoFensterLeeren();
    if (stereoRing.empty())
        return;
    stereoRingStandHaupt = (stereoRingStandHaupt + 1u) % (std::uint32_t) kStereoRingHaupt;
    stereoRingStandBass  = (stereoRingStandBass  + 1u) % (std::uint32_t) kStereoRingBass;
    if (stereoRingBelegtHaupt < (std::uint32_t) kStereoRingHaupt) ++stereoRingBelegtHaupt;
    if (stereoRingBelegtBass  < (std::uint32_t) kStereoRingBass)  ++stereoRingBelegtBass;
    const int trenn = trennIndex();
    std::size_t basis = stereoZaehlerElemente (trenn);
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        const auto slotsB = (std::size_t) stereoRingSlots (b, trenn);
        const auto binsB = (std::size_t) stereoBins (b);
        const auto slot = (std::size_t) (b < trenn ? stereoRingStandBass : stereoRingStandHaupt);
        for (std::size_t i = 0; i < binsB; ++i)
            stereoRing[basis + i * slotsB + slot] = StereoBinAkku {};
        stereoZaehler (stereoZaehlerBasis (b, trenn) + slot) = 0.0;
        basis += binsB * slotsB;
    }
}

/** Leert die GANZE Stereoevidenz einschliesslich des Rings. Wird von
    `grenzeZiehen()` und `zuruecksetzen()` (also auch von `vorbereiten()`)
    gerufen — kein Kreuzspektrum ueberbrueckt eine Grenze (§32.3). */
inline void FeatureEngine::stereoLeeren() noexcept
{
    stereoFensterLeeren();
    for (auto& r : stereoRing) r = StereoBinAkku {};
    stereoRingStandHaupt = stereoRingStandBass = 0u;
    stereoRingBelegtHaupt = stereoRingBelegtBass = 1u;
}

} // namespace nakama::analyse
