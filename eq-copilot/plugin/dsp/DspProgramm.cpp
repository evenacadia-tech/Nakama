#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1 - keine JucePlugin_*-Konstante im Kern
#include "DspProgramm.h"
#include "DspRtWache.h"

#include <complex>

namespace nakama::dsp
{

namespace param = nakama::parameter;

namespace
{

/** |H(e^{jw})|^2 eines Biquads - analytisch. Steht hier und NICHT in
    `DspFilter.h`, weil sie ausschliesslich die Auto-Gain-Ableitung bedient:
    ein Golden, der sie benutzte, vergliche das Produkt mit sich selbst. */
double biquadLeistung (const Biquad& f, double w) noexcept
{
    const std::complex<double> z (std::cos (-w), std::sin (-w));
    const std::complex<double> z2 = z * z;
    const auto h = (f.b0 + f.b1 * z + f.b2 * z2) / (1.0 + f.a1 * z + f.a2 * z2);
    return std::norm (h);
}

std::complex<double> biquadAntwort (const Biquad& f, double w) noexcept
{
    const std::complex<double> z (std::cos (-w), std::sin (-w));
    const std::complex<double> z2 = z * z;
    return (f.b0 + f.b1 * z + f.b2 * z2) / (1.0 + f.a1 * z + f.a2 * z2);
}

/** Antwort des TPT-SVF an der Kreisfrequenz `w`.

    Der TPT-SVF realisiert die analogen Prototypen unter der bilinearen
    Abbildung mit Prewarping bei seiner Eckfrequenz: die normierte analoge
    Frequenz ist `Omega = tan(w/2) / g`. Damit gilt

        D      = 1 - Omega^2 + j*k*Omega
        BP_roh = j*Omega / D          (Spitze 1/k bei Omega = 1)
        LP     = 1 / D
        H      = m0 + m1*BP_roh + m2*LP

    Das ist dieselbe Mischung, die `SvfZustand::tick` im Zeitbereich
    rechnet - hier nur geschlossen statt Sample fuer Sample. */
std::complex<double> svfAntwort (const SvfKoeffizienten& c, double w) noexcept
{
    if (c.g <= 0.0) return { c.m0, 0.0 };
    const double omega = std::tan (w * 0.5) / c.g;
    const std::complex<double> d (1.0 - omega * omega, c.k * omega);
    const std::complex<double> bp (0.0, omega);
    return c.m0 + c.m1 * (bp / d) + c.m2 * (1.0 / d);
}

/** Die Ruheantwort EINES Bandes: der Weg, den es ohne dynamische
    Auslenkung nimmt. Ein dynamisches Band laeuft ueber den SVF, ein
    statisches ueber den RBJ-Biquad - der Auto-Gain rechnet den Gang, der
    tatsaechlich anliegt, nicht den einer Familie, die dieses Band gar nicht
    benutzt. */
std::complex<double> bandRuheAntwort (const BandProgramm& b, double w) noexcept
{
    if (! b.aktiv) return { 1.0, 0.0 };
    if (b.nutztSvf) return svfAntwort (b.svfRuhe, w);
    if (b.statischIstEinheit) return { 1.0, 0.0 };
    return biquadAntwort (b.statisch, w);
}

param::Zelle bandZelle (const param::Satz& s, int slot, int feld) noexcept
{
    return s[(size_t) param::indexBandV1 (slot, feld)];
}

} // namespace

//==============================================================================
double autoGainGitterHz (int stelle) noexcept
{
    // 20 Hz .. 20 kHz, log-gleichverteilt ueber 120 Intervalle. Bei
    // stelle == 120 ist der Exponent exakt 1,0 und das Ergebnis exakt
    // 20 * 1000; die Stelle 0 ist exakt 20,0.
    const double anteil = (double) stelle / (double) (kAutoGainStellen - 1);
    return kAutoGainVonHz * std::pow (kAutoGainBisHz / kAutoGainVonHz, anteil);
}

double leiteAutoGainAb (const DspProgramm& p)
{
    // M-39, B-18: der Rechenort wird GEZAEHLT, nicht behauptet. Laeuft diese
    // Ableitung je im Audiopfad, steigt dort ein eigener Zaehler.
    RtWache::meldeAbleitung();

    // M-36: flache Kurve ergibt EXAKT 0,0 dB. Traegt das Programm kein
    // aktives Band, ist der Gitterlauf ueberfluessig - und ein Lauf ueber
    // 121 Stellen, der 1,0 aufsummiert und dann durch 121 teilt, laendete
    // bei -0,0000000 statt bei 0,0. Der Kurzschluss ist deshalb keine
    // Optimierung, sondern die Zusage selbst.
    if (! p.irgendeinBandAktiv() || p.samplerate <= 0.0) return 0.0;

    double summe = 0.0;
    for (int i = 0; i < kAutoGainStellen; ++i)
    {
        const double f = autoGainGitterHz (i);
        const double w = 2.0 * kPi * f / p.samplerate;

        std::complex<double> hStereo { 1.0, 0.0 }, hLinks { 1.0, 0.0 }, hRechts { 1.0, 0.0 };
        std::complex<double> hMid    { 1.0, 0.0 }, hSide  { 1.0, 0.0 };

        for (const auto& b : p.baender)
        {
            if (! b.aktiv) continue;
            const auto h = bandRuheAntwort (b, w);
            switch (b.modus)
            {
                case Kanalmodus::stereo: hStereo *= h; break;
                case Kanalmodus::left:   hLinks  *= h; break;
                case Kanalmodus::right:  hRechts *= h; break;
                case Kanalmodus::mid:    hMid    *= h; break;
                case Kanalmodus::side:   hSide   *= h; break;
            }
        }

        // §5.4 Feinheit 3: der Diagonalterm der M/S-Matrix. Ein identisches
        // Mid/Side-Paar faellt damit exakt auf den Stereo-Fall zusammen; der
        // Kreuzterm (H_M - H_S)/2 braeuchte eine Annahme ueber die
        // Korrelation des Materials, und genau die schliesst R4 aus.
        const auto diagonal = (hMid + hSide) * 0.5;
        const auto hL = hStereo * hLinks  * diagonal;
        const auto hR = hStereo * hRechts * diagonal;

        summe += 0.5 * (std::norm (hL) + std::norm (hR));
    }

    const double mittel = summe / (double) kAutoGainStellen;
    if (! (mittel > 0.0) || ! std::isfinite (mittel)) return 0.0;
    if (mittel == 1.0) return 0.0;   // bitgenau flach: log10(1,0) ist 0, aber -0,0 ist es nicht
    return -10.0 * std::log10 (mittel);
}

//==============================================================================
bool rampenKompatibel (const DspProgramm& alt, const DspProgramm& neu) noexcept
{
    // Beide Programme muessen den Pfad OEFFNEN: ein Passthrough hat keine
    // Filter, deren Zustand wandern koennte, und der Weg in ihn hinein oder
    // aus ihm heraus ist `blockrand` mit Crossfade (R2, M-03, M-04, M-06).
    if (! alt.eqEngagiert || ! neu.eqEngagiert || alt.hardBypass || neu.hardBypass) return false;
    if (alt.samplerate != neu.samplerate) return false;
    if ((alt.monoBassHz > 0.0) != (neu.monoBassHz > 0.0)) return false;   // E-20

    for (int i = 0; i < kSlots; ++i)
    {
        const auto& a = alt.baender[(size_t) i];
        const auto& n = neu.baender[(size_t) i];
        if (a.aktiv != n.aktiv) return false;
        if (! a.aktiv) continue;
        if (a.typ != n.typ || a.modus != n.modus || a.dynamisch != n.dynamisch
            || a.nutztSvf != n.nutztSvf || a.quelle != n.quelle)
            return false;
    }
    return true;
}

//==============================================================================
void baueProgramm (const param::DspSatz& satz, double samplerate,
                   std::uint64_t generation, DspProgramm& aus)
{
    aus = DspProgramm {};
    aus.samplerate = samplerate;
    aus.generation = generation;

    const auto& w = satz.werte;

    aus.hardBypass   = w[0].b;                                          // v1.global.bypass
    aus.inputTrimDb  = w[1].zahl;                                       // v1.global.input_trim_db
    aus.outputTrimDb = w[2].zahl;                                       // v1.global.output_trim_db
    aus.width        = w[3].zahl;                                       // v1.global.width
    aus.monoBassHz   = w[4].zahl;                                       // v1.global.mono_bass_hz
    aus.eqEngagiert  = w[(size_t) param::kIndexEqEnabled].b;
    aus.mix          = w[(size_t) param::kIndexMix].zahl;
    aus.autoGainAn   = w[(size_t) param::kIndexAutoGain].b;

    // M-02/M-31: 0 dB ist ein Unity-KURZSCHLUSS, keine Multiplikation. Der
    // lineare Faktor wird trotzdem gerechnet - der Kurzschluss haengt am
    // dB-Wert, damit eine Rampe auf dem Weg zu 1,0 nicht durch 0,99999994
    // laeuft und dort haengenbleibt.
    aus.inputTrimLin  = aus.inputTrimDb  == 0.0 ? 1.0 : dbInLinear (aus.inputTrimDb);
    aus.outputTrimLin = aus.outputTrimDb == 0.0 ? 1.0 : dbInLinear (aus.outputTrimDb);

    // M-30: die M/S-Stufe wird bei width == 1,0 und mono_bass_hz == 0 gar
    // nicht gerechnet. `(L+R)/2 + (L-R)/2` ist NICHT bitgenau L, und genau
    // daran faellt der Rotbeweis, wenn die Matrix immer laeuft.
    aus.msStufeAktiv = (aus.width != 1.0) || (aus.monoBassHz > 0.0);
    if (aus.monoBassHz > 0.0)
        aus.monoBassHochpass = entwurfLowCut (samplerate, aus.monoBassHz, 1.0 / std::sqrt (2.0));

    for (int slot = 0; slot < kSlots; ++slot)
    {
        auto& b = aus.baender[(size_t) slot];

        const bool occupied = w[(size_t) param::indexOccupied (slot)].b;
        const bool enabled  = bandZelle (w, slot, param::kEnabled).b;

        // R5: ein freier Slot verarbeitet nichts, gleichgueltig was seine
        // Werte sagen. Deshalb wird hier NICHT weitergebaut - ein Programm,
        // das die Koeffizienten trotzdem entwirft, haette einen Zustand, den
        // niemand eingeschaltet hat.
        b.aktiv = occupied && enabled;
        if (! b.aktiv) continue;

        b.typ   = (Filtertyp)  bandZelle (w, slot, param::kType).enumIndex;
        b.modus = (Kanalmodus) bandZelle (w, slot, param::kChannelMode).enumIndex;
        b.q     = bandZelle (w, slot, param::kQ).zahl;
        b.gainDb = bandZelle (w, slot, param::kGainDb).zahl;

        // M-12: gekappt wird beim BAUEN, der persistente Wert bleibt stehen.
        const double freqRoh = bandZelle (w, slot, param::kFreqHz).zahl;
        b.freqHzWirksam = kappeNyquist (freqRoh, samplerate, b.gekapptFreq);

        b.statisch = entwurf (b.typ, samplerate, b.freqHzWirksam, b.q, b.gainDb);
        b.statischIstEinheit = b.statisch.istEinheit();

        const bool dynAn = bandZelle (w, slot, param::kDynamicEnabled).b;
        b.dynamisch = dynAn;
        b.nutztSvf  = dynAn && typHatGain (b.typ);
        if (b.nutztSvf)
        {
            b.grundG  = svfGrundG (b.freqHzWirksam, samplerate);
            b.svfRuhe = svfEntwurf (b.typ, b.grundG, b.q, b.gainDb);
        }

        b.rangeDb     = bandZelle (w, slot, param::kDynamicRangeDb).zahl;
        b.thresholdDb = bandZelle (w, slot, param::kThresholdDb).zahl;

        // R7: `priority_sidechain` wird auf `internal` GEKLEMMT und als
        // geklemmt gemeldet. Der persistente Wert bleibt `priority_sidechain`
        // - waere er beim Speichern umgeschrieben, ginge der Userwunsch
        // unwiederbringlich verloren, und P8 faende einen stillschweigend
        // umgedeuteten Bestand vor.
        const auto quelle = (Sidechain) bandZelle (w, slot, param::kSidechainSource).enumIndex;
        b.quelle            = quelle;
        b.geklemmtSidechain = (quelle == Sidechain::prioritySidechain);
        const bool detektorGewuenscht = (quelle != Sidechain::none);

        // M-20/M-22: bei Range 0 oder `none` wird der Detektor GAR NICHT
        // gerechnet - nicht gerechnet und mit 0 multipliziert. Ein
        // ausdruecklich abgewaehlter Sidechain darf keine Rechenzeit kosten.
        b.detektorLaeuft = b.nutztSvf && detektorGewuenscht && b.rangeDb != 0.0;

        // W-2, W-3 (E-29): ENTWORFEN werden Detektor und Huellkurve, sobald
        // das Band einen Detektor HAT - auch bei Range 0. Eine Rampe der Range
        // auf 0 oder von 0 weg interpoliert zwischen beiden Programmen und
        // braucht auf beiden Seiten gueltige Koeffizienten. Gerechnet wird im
        // Audiothread weiter nur, was `detektorLaeuft` oder die laufende Rampe
        // verlangt; der Entwurf hier laeuft im Worker.
        if (b.nutztSvf && detektorGewuenscht)
        {
            b.detektor = entwurfBandpass (samplerate, b.freqHzWirksam, b.q);
            b.huelle   = huellkurveEntwurf (bandZelle (w, slot, param::kAttackMs).zahl,
                                            bandZelle (w, slot, param::kHoldMs).zahl,
                                            bandZelle (w, slot, param::kReleaseMs).zahl,
                                            samplerate);
        }
    }

    // R4: immer gerechnet, angewandt nur bei eingeschaltetem Schalter
    // (M-35). Der Wert steht ausserhalb des Audiothreads fest und wird dort
    // ueber denselben Rampenweg wie der Output-Trim angewandt (M-39).
    aus.autoGainDb  = leiteAutoGainAb (aus);
    aus.autoGainLin = aus.autoGainDb == 0.0 ? 1.0 : dbInLinear (aus.autoGainDb);
}

} // namespace nakama::dsp

#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1-Endpruefung nach allen Kern-Headern
