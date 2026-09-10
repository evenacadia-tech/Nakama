#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1 - keine JucePlugin_*-Konstante im Kern
#include "DspKern.h"

#include <algorithm>
#include <cstring>

namespace nakama::dsp
{

namespace param = nakama::parameter;

// §5.9: die acht Auslenkungen sind LOCKFREI lesbar. Zoege eine Plattform
// hier still eine Sperre ein, waere die Zusage aus R14 gebrochen, ohne dass
// ein Test es merkte - deshalb faellt der Bau statt der Laufzeit.
static_assert (std::atomic<double>::is_always_lock_free,
               "R14/R9: die Auslenkungen muessen lockfrei lesbar sein");

namespace
{

/** Lineare Mischung zweier SVF-Koeffizientensaetze. Zwischen zwei
    Steuerschritten laeuft der dynamische Gain damit sample-genau weiter
    statt in Stufen zu springen. */
SvfKoeffizienten mische (const SvfKoeffizienten& a, const SvfKoeffizienten& b, double t) noexcept
{
    SvfKoeffizienten c;
    c.a1 = a.a1 + (b.a1 - a.a1) * t;
    c.a2 = a.a2 + (b.a2 - a.a2) * t;
    c.a3 = a.a3 + (b.a3 - a.a3) * t;
    c.m0 = a.m0 + (b.m0 - a.m0) * t;
    c.m1 = a.m1 + (b.m1 - a.m1) * t;
    c.m2 = a.m2 + (b.m2 - a.m2) * t;
    c.g  = b.g;
    c.k  = b.k;
    return c;
}

const DspProgramm& ruheProgramm() noexcept
{
    static const DspProgramm p {};   // eqEngagiert = false: der Passthrough
    return p;
}

} // namespace

//==============================================================================
DspKern::DspKern() = default;

void DspKern::bereiteVor (double samplerate, int maxBlock)
{
    abtastrate      = samplerate;
    maxBlockGroesse = maxBlock > 0 ? maxBlock : 0;

    const size_t n = (size_t) maxBlockGroesse;
    arbeitL.assign (n, 0.0); arbeitR.assign (n, 0.0);
    mischL .assign (n, 0.0); mischR .assign (n, 0.0);
    dryL   .assign (n, 0.0); dryR   .assign (n, 0.0);
    candL  .assign (n, 0.0); candR  .assign (n, 0.0);
    tapPuffer.assign ((size_t) kTaps * 2u * n, 0.0);

    baenke.zuruecksetzen();
    lageCommitted = {};
    lageCandidate = {};

    rInputTrim .setzeSofort (1.0);
    rOutputTrim.setzeSofort (1.0);
    rMix       .setzeSofort (1.0);
    rWidth     .setzeSofort (1.0);
    rAutoGain  .setzeSofort (1.0);

    hoerwunsch .store (Hoermatrix::processed, std::memory_order_relaxed);
    hoerwirksam.store (Hoermatrix::processed, std::memory_order_relaxed);
    hoerLaufend  = Hoermatrix::processed;
    hoerVorher   = Hoermatrix::processed;
    hoerFadeRest = 0;
    candidateAktiv.store (false, std::memory_order_relaxed);
    dynamikAktiv  .store (false, std::memory_order_relaxed);
    autoGainBericht.store (0.0, std::memory_order_relaxed);
    for (auto& a : auslenkungen) a.store (0.0, std::memory_order_relaxed);

    tapGueltig    = 0;
    letzteKanaele = 0;
    zaehlerZuruecksetzen();
}

void DspKern::freigeben()
{
    arbeitL.clear(); arbeitR.clear();
    mischL .clear(); mischR .clear();
    dryL   .clear(); dryR   .clear();
    candL  .clear(); candR  .clear();
    tapPuffer.clear();
    maxBlockGroesse = 0;
    abtastrate      = 0.0;
    baenke.zuruecksetzen();
    lageCommitted = {};
    lageCandidate = {};
    hoerwunsch .store (Hoermatrix::processed, std::memory_order_relaxed);
    hoerwirksam.store (Hoermatrix::processed, std::memory_order_relaxed);
    hoerLaufend  = Hoermatrix::processed;
    hoerVorher   = Hoermatrix::processed;
    hoerFadeRest = 0;
    candidateAktiv.store (false, std::memory_order_relaxed);
    dynamikAktiv  .store (false, std::memory_order_relaxed);
    tapGueltig    = 0;
    letzteKanaele = 0;
}

void DspKern::zaehlerZuruecksetzen() noexcept
{
    zaehlerEingaenge .store (0, std::memory_order_relaxed);
    zaehlerZustaende .store (0, std::memory_order_relaxed);
    zaehlerVerworfen .store (0, std::memory_order_relaxed);
    zaehlerAuslenkung.store (0, std::memory_order_relaxed);
    zaehlerDenormale .store (0, std::memory_order_relaxed);
}

//==============================================================================
bool DspKern::uebernehmeZustand (const param::DspSatz& satz, Pfad p)
{
    const int slot = baenke.reserviere();
    if (slot < 0) return false;   // busy_retry (M-44)

    auto& bank = baenke.bank (slot);
    baueProgramm (satz, abtastrate, baenke.naechsteGeneration(), bank.programm);

    // M-07: die neue Bank startet KALT. Alle Filter- und Huellkurvenzustaende
    // stehen auf 0; die alte Bank klingt waehrend des Crossfades aus, ihr
    // Zustand wandert nicht mit.
    bank.zustaendeNullen();
    for (int i = 0; i < kSlots; ++i)
    {
        auto& z = bank.baender[(size_t) i];
        z.svfVon = z.svfNach = bank.programm.baender[(size_t) i].svfRuhe;
    }

    baenke.publiziere (p, slot);

    if (p == Pfad::committed)
    {
        autoGainBericht.store (bank.programm.autoGainDb, std::memory_order_relaxed);
        dynamikAktiv.store (bank.programm.eqEngagiert && ! bank.programm.hardBypass
                            && bank.programm.irgendeinBandDynamisch(),
                            std::memory_order_release);
    }
    else
    {
        candidateAktiv.store (true, std::memory_order_release);
    }
    return true;
}

void DspKern::beendeCandidate()
{
    candidateAktiv.store (false, std::memory_order_release);
}

int DspKern::pflege() noexcept
{
    return baenke.ernteAcks();
}

const DspProgramm& DspKern::programmVon (int slot) const noexcept
{
    if (slot < 0 || slot >= DspBankPool::kBaenke) return ruheProgramm();
    return baenke.bank (slot).programm;
}

void DspKern::auslenkungenDb (double* achtWerte) const noexcept
{
    for (int i = 0; i < kSlots; ++i)
        achtWerte[i] = auslenkungen[(size_t) i].load (std::memory_order_relaxed);
}

const double* DspKern::tap (Tap t, int kanal) const noexcept
{
    if (tapGueltig <= 0 || kanal < 0 || kanal > 1) return nullptr;
    const size_t versatz = (((size_t) t * 2u) + (size_t) kanal) * (size_t) maxBlockGroesse;
    if (versatz + (size_t) tapGueltig > tapPuffer.size()) return nullptr;
    return tapPuffer.data() + versatz;
}

//==============================================================================
void DspKern::blockrand (Pfad p, PfadLage& lage) noexcept
{
    const int neu = baenke.uebernehmeBereiten (p);
    if (neu < 0) return;

    // Ein zweiter Wechsel waehrend eines laufenden Fades: die bisher
    // verblassende Bank dient sofort aus. Drei gleichzeitig fadende Baenke
    // sind nicht vorgesehen (§44.2 nennt vier im schlechtesten Fall - zwei
    // je Pfad), und ein Stapel waere unbeschraenkt.
    if (lage.verblassend >= 0)
    {
        baenke.meldeAusgedient (lage.verblassend);
        lage.verblassend = -1;
    }
    if (lage.aktiv >= 0 && baenke.beginneVerblassen (lage.aktiv))
        lage.verblassend = lage.aktiv;

    lage.aktiv    = neu;
    lage.fadeRest = kFadeSamples;

    if (p == Pfad::committed)
        setzeRampenziele (baenke.bank (neu).programm);
}

void DspKern::setzeRampenziele (const DspProgramm& p) noexcept
{
    rInputTrim .setzeZiel (p.inputTrimLin);
    rOutputTrim.setzeZiel (p.outputTrimLin);
    rMix       .setzeZiel (p.mix);
    rWidth     .setzeZiel (p.width);
    // M-35: angewandt nur bei eingeschaltetem Schalter - gerechnet und
    // lesbar ist der Wert immer.
    rAutoGain  .setzeZiel (p.autoGainAn ? p.autoGainLin : 1.0);
}

void DspKern::heileZustaende (int slot) noexcept
{
    if (slot < 0 || slot >= DspBankPool::kBaenke) return;
    auto& bank = baenke.bank (slot);

    // §5.9 Feinheit 3: die Zustandspruefung laeuft am BLOCKRAND, nicht je
    // Sample. Alle Zustaende je Sample zu pruefen kostete bei acht Baendern
    // mal zwei Komponenten ein Vielfaches der Filterarbeit selbst; die
    // zweistufige Fassung haelt die Zusage ("Filterzustaende bleiben
    // endlich") und deckt zusaetzlich den Fall, in dem ein Zustand OHNE
    // nicht-endlichen Eingang entgleist - extreme Q nahe Nyquist.
    for (int i = 0; i < kSlots; ++i)
    {
        auto& z = bank.baender[(size_t) i];
        if (! z.istEndlich())
        {
            z.nullen();
            z.svfVon = z.svfNach = bank.programm.baender[(size_t) i].svfRuhe;
            zaehlerZustaende.fetch_add (1, std::memory_order_relaxed);
        }
    }
    if (! bank.monoBassZustand.istEndlich())
    {
        bank.monoBassZustand.nullen();
        zaehlerZustaende.fetch_add (1, std::memory_order_relaxed);
    }

    // Denormals am selben Blockrand. Ein abklingender Filterzustand laeuft
    // nach genuegend Stille in den denormalen Bereich; auf x86 kostet jede
    // Rechnung damit ein Vielfaches, und ein Kern, der im Leerlauf langsamer
    // wird als unter Last, verletzt die Echtzeitfestigkeit aus §44.5. Der
    // Riegel braucht keine Intrinsics und keine Plattformannahme.
    const int getroffen = bank.riegleDenormale();
    if (getroffen > 0)
        zaehlerDenormale.fetch_add ((std::uint64_t) getroffen, std::memory_order_relaxed);
}

//==============================================================================
void DspKern::verarbeiteBand (DspBank& bank, int slotIndex, double* L, double* R, int numSamples) noexcept
{
    const BandProgramm& b = bank.programm.baender[(size_t) slotIndex];
    if (! b.aktiv) return;

    BandZustand& z = bank.baender[(size_t) slotIndex];

    // Ein statisches Band mit bitgenau neutralem Biquad kostet nichts und
    // aendert nichts - es wird uebersprungen. Ein dynamisches Band laeuft
    // immer, weil seine Auslenkung sich bewegen kann.
    if (! b.nutztSvf && b.statischIstEinheit) return;

    const bool zweiKomponenten = (b.modus == Kanalmodus::stereo);

    for (int i = 0; i < numSamples; ++i)
    {
        // --- die Kanalkomponente(n) dieses Bandes herausloesen ------------
        double x0 = 0.0, x1 = 0.0, m = 0.0, s = 0.0;
        switch (b.modus)
        {
            case Kanalmodus::stereo: x0 = L[i]; x1 = R[i]; break;
            case Kanalmodus::left:   x0 = L[i]; break;
            case Kanalmodus::right:  x0 = R[i]; break;
            case Kanalmodus::mid:    m = (L[i] + R[i]) * 0.5; s = (L[i] - R[i]) * 0.5; x0 = m; break;
            case Kanalmodus::side:   m = (L[i] + R[i]) * 0.5; s = (L[i] - R[i]) * 0.5; x0 = s; break;
        }

        double y0 = x0, y1 = x1;

        if (b.nutztSvf)
        {
            // --- Detektor: hoert das bandgefilterte Signal VOR dem Band ---
            if (b.detektorLaeuft)
            {
                double leistungEin;
                if (zweiKomponenten)
                {
                    const double d0 = z.detektor[0].tick (b.detektor, x0);
                    const double d1 = z.detektor[1].tick (b.detektor, x1);
                    // Ein gemeinsamer Pegel fuer beide Komponenten: zwei
                    // getrennte Auslenkungen zoegen das Stereobild
                    // auseinander, sobald sich L und R im Pegel trennen.
                    leistungEin = (d0 * d0 + d1 * d1) * 0.5;
                }
                else
                {
                    const double d0 = z.detektor[0].tick (b.detektor, x0);
                    leistungEin = d0 * d0;
                }
                z.huelle.tick (b.huelle, leistungEin);
            }

            // --- Steuerrate: Kennlinie und Neuentwurf ---------------------
            if (z.schrittRest <= 0)
            {
                double gDyn = 0.0;
                if (b.detektorLaeuft)
                {
                    const double pegelDb = leistungInDb (z.huelle.leistung);
                    gDyn = dynamischeKennlinie (pegelDb, b.thresholdDb, b.rangeDb);
                }
                z.auslenkungDb = gDyn;
                z.svfVon       = z.svfNach;
                z.svfNach      = svfEntwurf (b.typ, b.grundG, b.q, b.gainDb + gDyn);
                z.schrittRest  = kDynamikSchritt;
            }

            const double t = 1.0 - (double) z.schrittRest / (double) kDynamikSchritt;
            const auto k = mische (z.svfVon, z.svfNach, t);
            --z.schrittRest;

            y0 = z.svf[0].tick (k, x0);
            if (zweiKomponenten) y1 = z.svf[1].tick (k, x1);
        }
        else
        {
            y0 = z.statisch[0].tick (b.statisch, x0);
            if (zweiKomponenten) y1 = z.statisch[1].tick (b.statisch, x1);
        }

        // --- zurueck in L/R ----------------------------------------------
        switch (b.modus)
        {
            case Kanalmodus::stereo: L[i] = y0; R[i] = y1; break;
            case Kanalmodus::left:   L[i] = y0; break;
            case Kanalmodus::right:  R[i] = y0; break;
            case Kanalmodus::mid:    L[i] = y0 + s; R[i] = y0 - s; break;
            case Kanalmodus::side:   L[i] = m + y0; R[i] = m - y0; break;
        }
    }

    // M-27: der zuletzt gerechnete Wert am Ende des Fensters - kein Mittel,
    // nie aus den Einstellwerten hergeleitet. Ein nicht-endlicher Wert wird
    // 0 und gezaehlt (M-113); ein NaN reist nie ueber den Draht.
    double gemeldet = z.auslenkungDb;
    if (! std::isfinite (gemeldet))
    {
        gemeldet = 0.0;
        zaehlerAuslenkung.fetch_add (1, std::memory_order_relaxed);
    }
    auslenkungen[(size_t) slotIndex].store (gemeldet, std::memory_order_relaxed);
}

//==============================================================================
void DspKern::verarbeiteBank (int slot, double* L, double* R, int numSamples) noexcept
{
    const DspProgramm& p = programmVon (slot);

    // Passthrough: NICHTS rechnen, nichts sanitisieren. Die Rampen laufen
    // trotzdem weiter, damit ein spaeteres Einschalten sie nicht nachholen
    // muss - und damit beide Durchlaeufe eines Crossfades gleich viele
    // Ticks machen.
    if (! p.eqEngagiert || p.hardBypass)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            rInputTrim.tick(); rWidth.tick(); rAutoGain.tick(); rMix.tick(); rOutputTrim.tick();
        }
        return;
    }

    auto& bank = baenke.bank (slot);
    const double* dL = dryL.data();
    const double* dR = dryR.data();

    // --- der Nicht-Endlich-Riegel am EINGANG des aktiven Pfads -----------
    // R9: verriegelt und gezaehlt, VOR jedem Filterzustand. Der Dry-Zweig
    // (Mix, Hoermatrix) bleibt roh - bei Mix 0 ist der Weg ein Passthrough,
    // und ein Passthrough sanitisiert nichts (M-50).
    for (int i = 0; i < numSamples; ++i)
    {
        double l = L[i], r = R[i];
        if (! std::isfinite (l)) { l = 0.0; zaehlerEingaenge.fetch_add (1, std::memory_order_relaxed); }
        if (! std::isfinite (r)) { r = 0.0; zaehlerEingaenge.fetch_add (1, std::memory_order_relaxed); }
        L[i] = l; R[i] = r;
    }

    // --- Input-Trim ------------------------------------------------------
    // M-31: 0 dB ist ein Unity-KURZSCHLUSS. Die Ruhepruefung fragt den
    // Rampenzustand, nicht nur den Zielwert - eine laufende Rampe muss
    // rechnen, auch wenn ihr Ziel 1,0 ist.
    if (rInputTrim.ruhtBei (1.0))
    {
        for (int i = 0; i < numSamples; ++i) rInputTrim.tick();
    }
    else
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const double g = rInputTrim.tick();
            L[i] *= g; R[i] *= g;
        }
    }

    // --- M/S-Stufe: Width und Mono-Bass ----------------------------------
    // M-30: bei width == 1,0 und mono_bass_hz == 0 wird die Matrix GAR NICHT
    // gerechnet. `(L+R)/2 + (L-R)/2` ist nicht bitgenau L - genau daran
    // faellt der Rotbeweis, wenn die Stufe immer laeuft.
    const bool monoBass = p.monoBassHz > 0.0;
    if (rWidth.ruhtBei (1.0) && ! monoBass)
    {
        for (int i = 0; i < numSamples; ++i) rWidth.tick();
    }
    else
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const double w = rWidth.tick();
            const double m = (L[i] + R[i]) * 0.5;
            double s       = (L[i] - R[i]) * 0.5;
            if (monoBass) s = bank.monoBassZustand.tick (p.monoBassHochpass, s);
            s *= w;
            L[i] = m + s; R[i] = m - s;
        }
    }

    // M-121: ein Slot, der KEINE Auslenkung erzeugt, meldet exakt 0,0 - und
    // zwar HIER, nicht erst wenn er wieder laeuft. `verarbeiteBand` kehrt
    // bei einem freien oder statischen Band frueh zurueck und schriebe sonst
    // nie; die alte Auslenkung eines entfernten dynamischen Bandes bliebe
    // stehen und reiste als endlicher, formal gueltiger Wert weiter.
    for (int i = 0; i < kSlots; ++i)
    {
        const auto& bp = bank.programm.baender[(size_t) i];
        if (! (bp.aktiv && bp.nutztSvf))
            auslenkungen[(size_t) i].store (0.0, std::memory_order_relaxed);
    }

    // --- die acht Baender, Slot 0 zuerst ---------------------------------
    // Die Richtung ist Teil der Zusage (M-15, §5.3.1): bei nicht
    // kommutierenden Kombinationen klingt 7 -> 0 messbar anders.
    for (int slotIndex = 0; slotIndex < kSlots; ++slotIndex)
        verarbeiteBand (bank, slotIndex, L, R, numSamples);

    // --- Auto-Gain, Mix, Output-Trim -------------------------------------
    // Reihenfolge eingefroren (§3.0, §5.3): Auto-Gain im Wet-Zweig VOR Mix,
    // Output-Trim als Ausgangsregler des Users HINTER Mix.
    const bool autoGainRuht = rAutoGain.ruhtBei (1.0);
    const bool mixRuht      = rMix.ruhtBei (1.0);
    const bool outRuht      = rOutputTrim.ruhtBei (1.0);

    for (int i = 0; i < numSamples; ++i)
    {
        const double ag  = rAutoGain.tick();
        const double mix = rMix.tick();
        const double out = rOutputTrim.tick();

        double l = L[i], r = R[i];
        if (! autoGainRuht) { l *= ag; r *= ag; }
        if (! mixRuht)
        {
            // Linear, nicht equal-power (§5.3 Feinheit 4). Bei mix == 1,0
            // wird der Dry-Zweig nicht gerechnet, bei 0,0 nicht der
            // Wet-Zweig: `0 * wet` truege sonst ein NaN oder -0,0 aus dem
            // Wet-Zweig in den Ausgang, und M-33 waere keine Bitidentitaet.
            if (mix == 0.0)      { l = dL[i]; r = dR[i]; }
            else if (mix != 1.0) { l = mix * l + (1.0 - mix) * dL[i];
                                   r = mix * r + (1.0 - mix) * dR[i]; }
        }
        if (! outRuht) { l *= out; r *= out; }
        L[i] = l; R[i] = r;
    }
}

//==============================================================================
void DspKern::mischeFade (const double* vonL, const double* vonR,
                          double* nachL, double* nachR,
                          int numSamples, PfadLage& lage) noexcept
{
    for (int i = 0; i < numSamples; ++i)
    {
        if (lage.fadeRest <= 0) break;   // ab hier steht das Ziel allein - BITGENAU
        const double t = 1.0 - (double) lage.fadeRest / (double) kFadeSamples;
        nachL[i] = vonL[i] * (1.0 - t) + nachL[i] * t;
        nachR[i] = vonR[i] * (1.0 - t) + nachR[i] * t;
        --lage.fadeRest;
    }
}

//==============================================================================
void DspKern::verarbeite (float* const* kanaele, int numKanaele, int numSamples) noexcept
{
    RtWache::Bereich wache;

    if (kanaele == nullptr || numKanaele <= 0 || numSamples <= 0) return;

    // M-48: verworfen wird die ANALYSE, nie Audio. Ein Block groesser als
    // `maxBlock` laeuft deshalb in Stuecken durch statt zurueckzuweisen -
    // ein Rueckweg hier hiesse, dass ein Host mit groesserem Puffer als
    // angekuendigt STILLE bekaeme, und das waere ein Audioausfall.
    if (maxBlockGroesse > 0 && numSamples > maxBlockGroesse)
    {
        zaehlerVerworfen.fetch_add (1, std::memory_order_relaxed);
        int versatz = 0;
        while (versatz < numSamples)
        {
            const int n = std::min (maxBlockGroesse, numSamples - versatz);
            float* teil[2] = { kanaele[0] + versatz,
                               numKanaele > 1 ? kanaele[1] + versatz : nullptr };
            verarbeiteStueck (teil, numKanaele, n);
            versatz += n;
        }
        // Der Tap dieses Blocks ist keine kohaerente Messung mehr - er wird
        // ausdruecklich fuer ungueltig erklaert statt ein Teilstueck zu
        // liefern, das wie ein voller Block aussieht.
        tapGueltig = 0;
        return;
    }

    verarbeiteStueck (kanaele, numKanaele, numSamples);
}

void DspKern::verarbeiteStueck (float* const* kanaele, int numKanaele, int numSamples) noexcept
{
    blockrand (Pfad::committed, lageCommitted);
    blockrand (Pfad::candidate, lageCandidate);

    // --- die WIRKSAME Hoermatrix (M-56) ----------------------------------
    const bool candDa  = candidateAktiv.load (std::memory_order_acquire) && lageCandidate.aktiv >= 0;
    Hoermatrix wirksam = hoerwunsch.load (std::memory_order_acquire);
    if (wirksam == Hoermatrix::candidate && ! candDa) wirksam = Hoermatrix::processed;
    hoerwirksam.store (wirksam, std::memory_order_release);

    const DspProgramm& pAktiv = programmVon (lageCommitted.aktiv);
    const bool aktivPassthrough = ! pAktiv.eqEngagiert || pAktiv.hardBypass;

    // Der Tap-Platz ist hier immer gegeben - `verarbeite` hat groessere
    // Bloecke bereits gestueckelt und den Tap fuer ungueltig erklaert.
    const bool tapPlatz = numSamples <= maxBlockGroesse && ! tapPuffer.empty();

    // --- der reine Passthrough: KEIN Sample wird geschrieben --------------
    // M-01/M-05: nicht "mit 1,0 multipliziert", nicht "durch neutrale
    // Biquads gerechnet" - der Puffer wird nicht angefasst.
    const bool nurEingang = lageCommitted.fadeRest <= 0
                            && aktivPassthrough
                            && hoerFadeRest <= 0
                            && wirksam == hoerLaufend
                            && (wirksam == Hoermatrix::processed || wirksam == Hoermatrix::dry);

    if (nurEingang)
    {
        if (tapPlatz)
        {
            // Die Taps sind Messpunkte, kein Audioweg: sie werden auch im
            // ausgeschalteten Zustand gefuellt, damit die Sonde weiter misst.
            // `pre_nakama` und `post_committed` sind dann derselbe Block, und
            // `post_candidate` bleibt leer.
            const size_t n = (size_t) numSamples;
            for (int k = 0; k < 2; ++k)
            {
                const float* q = kanaele[(numKanaele > 1 && k == 1) ? 1 : 0];
                double* pre   = tapZeiger (Tap::preNakama,     k);
                double* post  = tapZeiger (Tap::postCommitted, k);
                double* postC = tapZeiger (Tap::postCandidate, k);
                for (size_t i = 0; i < n; ++i) { pre[i] = (double) q[i]; post[i] = pre[i]; }
                std::memset (postC, 0, n * sizeof (double));
            }
            tapGueltig = numSamples;
        }
        letzteKanaele = numKanaele;

        for (int i = 0; i < numSamples; ++i)
        {
            rInputTrim.tick(); rWidth.tick(); rAutoGain.tick(); rMix.tick(); rOutputTrim.tick();
        }
        // Ein nicht rechnender Pfad meldet exakt 0,0 Auslenkung (M-27).
        for (auto& a : auslenkungen) a.store (0.0, std::memory_order_relaxed);
        hoerLaufend = wirksam;
        return;
    }

    // --- Eingang nach double, Dry-Kopie ----------------------------------
    if ((size_t) numSamples > dryL.size()) { letzteKanaele = numKanaele; return; }   // nur bei freigegebenem Kern
    const size_t n = (size_t) numSamples;

    for (size_t i = 0; i < n; ++i)
    {
        dryL[i] = (double) kanaele[0][i];
        dryR[i] = (numKanaele > 1) ? (double) kanaele[1][i] : dryL[i];
    }

    // Beide Durchlaeufe eines Crossfades muessen dieselben Rampenwerte
    // sehen: die globalen Stufen sind EINE Stufe, nicht zwei. Der
    // Rampenstand wird deshalb gesichert und zurueckgesetzt - billiger und
    // exakter als fuenf Vorabpuffer ueber die Blockgroesse.
    const Rampe sInput = rInputTrim, sOut = rOutputTrim, sMix = rMix,
                sWidth = rWidth,     sAuto = rAutoGain;

    const bool fadeMitBank = lageCommitted.fadeRest > 0 && lageCommitted.verblassend >= 0;

    if (fadeMitBank)
    {
        std::memcpy (mischL.data(), dryL.data(), n * sizeof (double));
        std::memcpy (mischR.data(), dryR.data(), n * sizeof (double));
        verarbeiteBank (lageCommitted.verblassend, mischL.data(), mischR.data(), numSamples);
        rInputTrim = sInput; rOutputTrim = sOut; rMix = sMix; rWidth = sWidth; rAutoGain = sAuto;
    }

    std::memcpy (arbeitL.data(), dryL.data(), n * sizeof (double));
    std::memcpy (arbeitR.data(), dryR.data(), n * sizeof (double));
    verarbeiteBank (lageCommitted.aktiv, arbeitL.data(), arbeitR.data(), numSamples);

    const Rampe nInput = rInputTrim, nOut = rOutputTrim, nMix = rMix,
                nWidth = rWidth,     nAuto = rAutoGain;

    if (lageCommitted.fadeRest > 0)
    {
        // Von der verblassenden Bank, oder - wenn es keine gibt - von der
        // Ruhe her, also vom unveraenderten Eingang.
        const double* vonL = fadeMitBank ? mischL.data() : dryL.data();
        const double* vonR = fadeMitBank ? mischR.data() : dryR.data();
        mischeFade (vonL, vonR, arbeitL.data(), arbeitR.data(), numSamples, lageCommitted);

        if (lageCommitted.fadeRest == 0 && lageCommitted.verblassend >= 0)
        {
            baenke.meldeAusgedient (lageCommitted.verblassend);
            lageCommitted.verblassend = -1;
        }
    }

    // --- Candidate-Pfad --------------------------------------------------
    // Eigene Bank, eigener Puffer, eigene Filterzustaende: Committed und
    // Candidate teilen nie eine Bank und nie einen Zustand (§44.2).
    bool candGerechnet = false;
    if (candDa)
    {
        rInputTrim = sInput; rOutputTrim = sOut; rMix = sMix; rWidth = sWidth; rAutoGain = sAuto;
        std::memcpy (candL.data(), dryL.data(), n * sizeof (double));
        std::memcpy (candR.data(), dryR.data(), n * sizeof (double));
        verarbeiteBank (lageCandidate.aktiv, candL.data(), candR.data(), numSamples);
        candGerechnet = true;

        if (lageCandidate.fadeRest > 0)
        {
            // Der Candidate blendet vom unveraenderten Eingang her ein. Eine
            // verblassende Candidate-Bank wird NICHT zusaetzlich gerechnet:
            // sie kostete eine fuenfte Bank, und §44.2 nennt vier als
            // schlechtesten Fall. Sie dient sofort aus.
            mischeFade (dryL.data(), dryR.data(), candL.data(), candR.data(),
                        numSamples, lageCandidate);
            if (lageCandidate.fadeRest == 0 && lageCandidate.verblassend >= 0)
            {
                baenke.meldeAusgedient (lageCandidate.verblassend);
                lageCandidate.verblassend = -1;
            }
        }

        // Der Committed-Stand der Rampen gilt weiter; der Candidate-Lauf war
        // ein Nebenweg und darf ihn nicht verschieben.
        rInputTrim = nInput; rOutputTrim = nOut; rMix = nMix; rWidth = nWidth; rAutoGain = nAuto;
    }

    // --- Taps ------------------------------------------------------------
    if (tapPlatz)
    {
        for (int k = 0; k < 2; ++k)
        {
            const double* q = (k == 0) ? dryL.data() : dryR.data();
            std::memcpy (tapZeiger (Tap::preNakama, k), q, n * sizeof (double));

            const double* w = (k == 0) ? arbeitL.data() : arbeitR.data();
            std::memcpy (tapZeiger (Tap::postCommitted, k), w, n * sizeof (double));

            double* postC = tapZeiger (Tap::postCandidate, k);
            if (candGerechnet)
            {
                const double* c = (k == 0) ? candL.data() : candR.data();
                std::memcpy (postC, c, n * sizeof (double));
            }
            else
            {
                std::memset (postC, 0, n * sizeof (double));
            }
        }
        tapGueltig = numSamples;
    }

    // --- Hoermatrix, HINTER allen drei Taps (§44.2 letzter Absatz) --------
    //
    // M-55: der Wechsel ist KLICKFREI. Die Hoermatrix liegt hinter den Taps
    // und hat deshalb keinen Bankwechsel, an dem sie mitfahren koennte - sie
    // braucht einen EIGENEN Uebergang. Ohne ihn springt der Ausgang beim
    // Wechsel Processed -> Dry um die volle Differenz der beiden Wege, und
    // genau das ist der Klick, den die Zeile ausschliesst.
    const double makeup = dbInLinear (kDeltaMakeupDb);

    if (wirksam != hoerLaufend && hoerFadeRest <= 0)
    {
        hoerVorher    = hoerLaufend;
        hoerLaufend   = wirksam;
        hoerFadeRest  = kFadeSamples;
    }
    else if (wirksam != hoerLaufend)
    {
        // Ein zweiter Wechsel waehrend eines laufenden Fades: das Ziel
        // wandert, der Zaehler laeuft weiter. Ein Neustart machte den
        // Uebergang laenger, je haeufiger der User klickt.
        hoerLaufend = wirksam;
    }

    auto hoerWert = [&] (Hoermatrix h, int i, double& l, double& r)
    {
        switch (h)
        {
            case Hoermatrix::processed: l = arbeitL[(size_t) i]; r = arbeitR[(size_t) i]; break;
            case Hoermatrix::dry:       l = dryL[(size_t) i];    r = dryR[(size_t) i];    break;
            case Hoermatrix::delta:
                // Fester, MATERIALUNABHAENGIGER Abgleich (§5.10 Feinheit 1).
                // Bei Gleichheit ist die Differenz bitgenau 0, und 0 * k ist
                // es auch (M-54).
                l = (arbeitL[(size_t) i] - dryL[(size_t) i]) * makeup;
                r = (arbeitR[(size_t) i] - dryR[(size_t) i]) * makeup;
                break;
            case Hoermatrix::candidate:
                l = candL[(size_t) i]; r = candR[(size_t) i];
                break;
        }
    };

    for (int i = 0; i < numSamples; ++i)
    {
        double l = 0.0, r = 0.0;
        hoerWert (hoerLaufend, i, l, r);

        if (hoerFadeRest > 0)
        {
            // Linear, wie jeder andere Uebergang des Kerns (§5.2 Feinheit 2).
            double lv = 0.0, rv = 0.0;
            hoerWert (hoerVorher, i, lv, rv);
            const double t = 1.0 - (double) hoerFadeRest / (double) kFadeSamples;
            l = lv * (1.0 - t) + l * t;
            r = rv * (1.0 - t) + r * t;
            --hoerFadeRest;
        }

        kanaele[0][i] = (float) l;
        if (numKanaele > 1) kanaele[1][i] = (float) r;
    }

    letzteKanaele = numKanaele;

    // --- Blockrand: Filterzustaende heilen -------------------------------
    heileZustaende (lageCommitted.aktiv);
    heileZustaende (lageCommitted.verblassend);
    heileZustaende (lageCandidate.aktiv);
}

double* DspKern::tapZeiger (Tap t, int kanal) noexcept
{
    return tapPuffer.data() + (((size_t) t * 2u) + (size_t) kanal) * (size_t) maxBlockGroesse;
}

} // namespace nakama::dsp

#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1-Endpruefung nach allen Kern-Headern
