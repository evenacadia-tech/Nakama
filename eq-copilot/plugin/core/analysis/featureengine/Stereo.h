// FeatureEngine — Stereo: Kreuzspektren, Kohaerenz, Breite.
//
// NAK-225/S25d (09.09.2026), Definitionen herausgeloest aus der Klasse in
// core/analysis/FeatureEngine.h. Inhalt:
//
//   stereoSchritt        Ein Welch-Frame der Hauptstufe in die bandweisen
//                        Kreuzspektren. Ohne zweite FFT: die Engine
//                        transformiert MID und SIDE, und L/R folgen daraus
//                        linear.
//   stereoSample         Die Energien, die kein Frame braucht (Korrelation).
//   stereoAuswerten      Aus den Akkumulatoren werden Bandwerte und Skalare.
//   stereoLeeren         Der Rueckweg: jeder Akkumulator hat einen.
//
// aktivieren und abklingen gehoeren zusammen (CLAUDE.md): stereoSchritt und
// stereoLeeren stehen deshalb in derselben Datei.

#ifndef NAKAMA_FEATUREENGINE_TEIL
#error "Teilkopf von FeatureEngine.h - nur ueber FeatureEngine.h einbinden."
#endif

#pragma once

namespace nakama::analyse
{

//== Stereoevidenz (SONDE-013 M-08, M-10 bis M-12) ========================

/** Ein Welch-Frame der Hauptstufe in die bandweisen Kreuzspektren.

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
    zweite Kappenregel. */
inline void FeatureEngine::stereoSchritt (const Stufe& s, bool zaehlKurzfenster) noexcept
{
    if ((int) stereoAkku.size() < Gitter::evidenzBaender)
        return;

    const double hopMs = 1000.0 * (double) s.hop / s.fs;
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
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
            sxyRe += lr * rr + li * ri;
            sxyIm += li * rr - lr * ri;
        }
        if (! (std::isfinite (smm) && std::isfinite (sss)
               && std::isfinite (sll) && std::isfinite (srr)
               && std::isfinite (sxyRe) && std::isfinite (sxyIm)))
            continue;                       // NaN-Riegel beim ERZEUGEN

        auto& a = stereoAkku[(std::size_t) b];
        a.smm += smm; a.sss += sss;
        a.sll += sll; a.srr += srr;
        a.sxyRe += sxyRe; a.sxyIm += sxyIm;
        ++a.frames;
        a.dauerMs += hopMs;

        auto& kz = stereoKurz[(std::size_t) b];
        kz.sll += sll; kz.srr += srr;
        kz.sxyRe += sxyRe; kz.sxyIm += sxyIm;
        ++kz.frames;

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
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
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
                    const double koh = (kz.sxyRe * kz.sxyRe + kz.sxyIm * kz.sxyIm)
                                     / (kz.sll * kz.srr);
                    if (std::isfinite (koh) && koh >= kKohaerenzSchwellePhase
                        && kz.frames >= kWelchMindestFrames)
                        ++stereoPersistenzZaehler[(std::size_t) b];
                }
            }
            kz = StereoAkku {};
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
    Evidenzfensters, zusammen mit den Baendern und der Verteilung. */
inline void FeatureEngine::stereoAuswerten() noexcept
{
    if ((int) stereoAkku.size() < Gitter::evidenzBaender
        || (int) stereoErgebnis.size() < Gitter::evidenzBaender)
        return;

    float folge[kVerteilungPlaetze];
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        const auto& a = stereoAkku[(std::size_t) b];
        auto& e = stereoErgebnis[(std::size_t) b];
        e = StereoBandwert {};
        e.freiheitsgrade = a.frames;
        e.fensterDauerMs = (float) a.dauerMs;
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

            // Stufe 1: Kohaerenz nur mit genug Frames UND Energie.
            if (a.frames >= (std::uint32_t) kWelchMindestFrames
                && a.sll > 0.0 && a.srr > 0.0)
            {
                const double koh = (a.sxyRe * a.sxyRe + a.sxyIm * a.sxyIm)
                                 / (a.sll * a.srr);
                if (std::isfinite (koh))
                {
                    e.kohaerenzGesetzt = true;
                    e.kohaerenz = (float) std::clamp (koh, 0.0, 1.0);
                    // Stufe 2: Phase nur ueber der benannten Schwelle.
                    if (koh > kKohaerenzSchwellePhase)
                    {
                        const double phi = std::atan2 (a.sxyIm, a.sxyRe);
                        if (std::isfinite (phi))
                        {
                            e.phaseGesetzt = true;
                            e.phaseRad = (float) phi;
                        }
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

/** Leert alles, was zu GENAU DIESEM Evidenzfenster gehoert. Wird von
    `evidenzLeeren()` und von `grenzeZiehen()` gerufen — die
    Stereoevidenz ist ein Fenster wie jedes andere (§32.3). */
inline void FeatureEngine::stereoLeeren() noexcept
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
}

} // namespace nakama::analyse
