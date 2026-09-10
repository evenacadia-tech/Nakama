#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1 - keine JucePlugin_*-Konstante im Kern
#include "DspKern.h"

#include <algorithm>
#include <cstring>
#include <utility>

namespace nakama::dsp
{

namespace param = nakama::parameter;

// §5.9: die Auslenkungen sind LOCKFREI lesbar. Zoege eine Plattform hier
// still eine Sperre ein, waere die Zusage aus R14 gebrochen, ohne dass ein
// Test es merkte - deshalb faellt der Bau statt der Laufzeit.
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

/** Lineare Mischung zweier Biquads (B-4). Stabil ist sie, weil die Menge
    stabiler Nennerpaare (|a2| < 1, |a1| < 1 + a2) ein Dreieck und damit
    konvex ist: jeder Punkt auf der Strecke zwischen zwei stabilen
    Entwuerfen ist selbst stabil (Entscheid E-19). */
Biquad mische (const Biquad& a, const Biquad& b, double t) noexcept
{
    Biquad c;
    c.b0 = a.b0 + (b.b0 - a.b0) * t;
    c.b1 = a.b1 + (b.b1 - a.b1) * t;
    c.b2 = a.b2 + (b.b2 - a.b2) * t;
    c.a1 = a.a1 + (b.a1 - a.a1) * t;
    c.a2 = a.a2 + (b.a2 - a.a2) * t;
    return c;
}

/** Lineare Mischung zweier Huellkurvensaetze (W-2, Entscheid E-28): beide
    Pole linear, die Haltezeit als gerundete lineare Mischung ihrer
    Samplezahl. Ein Pol zwischen zwei Polen aus [0, 1) liegt selbst in
    [0, 1) - die gemischte Huellkurve bleibt stabil. */
HuellkurveKoeffizienten mische (const HuellkurveKoeffizienten& a, const HuellkurveKoeffizienten& b, double t) noexcept
{
    HuellkurveKoeffizienten c;
    c.attackPol   = a.attackPol  + (b.attackPol  - a.attackPol)  * t;
    c.releasePol  = a.releasePol + (b.releasePol - a.releasePol) * t;
    c.holdSamples = (std::int64_t) ((double) a.holdSamples
                                    + ((double) b.holdSamples - (double) a.holdSamples) * t + 0.5);
    return c;
}

inline double lerp (double a, double b, double t) noexcept { return a + (b - a) * t; }

const DspProgramm& ruheProgramm() noexcept
{
    static const DspProgramm p {};   // eqEngagiert = false: der Passthrough
    return p;
}

} // namespace

//==============================================================================
void DspKern::PfadZustand::ruhe() noexcept
{
    aktiv     = -1;
    quelle    = -1;
    uebergang = Uebergang::keiner;
    rest      = 0;
    rampen.setzeSofort (1.0);
    for (auto& a : auslenkungen) a.store (0.0, std::memory_order_relaxed);
}

DspKern::DspKern() = default;

void DspKern::bereiteVor (double samplerate, int maxBlock)
{
    abtastrate      = samplerate;
    maxBlockGroesse = maxBlock > 0 ? maxBlock : 0;

    const size_t n = (size_t) maxBlockGroesse;
    for (auto* v : { &dryL, &dryR, &eingL, &eingR,
                     &cVonL, &cVonR, &cNachL, &cNachR, &cAusL, &cAusR, &cGewicht,
                     &kVonL, &kVonR, &kNachL, &kNachR, &kAusL, &kAusR, &kGewicht })
        v->assign (n, 0.0);
    tapPuffer.assign ((size_t) kTaps * 2u * n, 0.0);

    baenke.zuruecksetzen();
    for (auto& z : pfade) z.ruhe();

    hoerwunsch .store (Hoermatrix::processed, std::memory_order_relaxed);
    hoerwirksam.store (Hoermatrix::processed, std::memory_order_relaxed);
    hoerLaufend  = Hoermatrix::processed;
    hoerVorher   = Hoermatrix::processed;
    hoerFadeRest = 0;
    candidateAktiv.store (false, std::memory_order_relaxed);
    dynamikAktiv  .store (false, std::memory_order_relaxed);
    for (auto& a : autoGainBericht) a.store (0.0, std::memory_order_relaxed);

    tapGueltig    = 0;
    letzteKanaele = 0;
    zaehlerZuruecksetzen();
}

void DspKern::freigeben()
{
    for (auto* v : { &dryL, &dryR, &eingL, &eingR,
                     &cVonL, &cVonR, &cNachL, &cNachR, &cAusL, &cAusR, &cGewicht,
                     &kVonL, &kVonR, &kNachL, &kNachR, &kAusL, &kAusR, &kGewicht })
        v->clear();
    tapPuffer.clear();
    maxBlockGroesse = 0;
    abtastrate      = 0.0;
    // B-12: der Pool setzt seine Ressourcen zurueck, nicht seinen
    // Generationszaehler.
    baenke.zuruecksetzen();
    for (auto& z : pfade) z.ruhe();
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
    zaehlerEingaenge  .store (0, std::memory_order_relaxed);
    zaehlerZustaende  .store (0, std::memory_order_relaxed);
    zaehlerVerworfen  .store (0, std::memory_order_relaxed);
    zaehlerAuslenkung .store (0, std::memory_order_relaxed);
    zaehlerDenormale  .store (0, std::memory_order_relaxed);
    zaehlerMsStufe    .store (0, std::memory_order_relaxed);
    zaehlerUebernahmen.store (0, std::memory_order_relaxed);
}

//==============================================================================
void DspKern::meldeProgramm (Pfad p, const DspProgramm& prog) noexcept
{
    autoGainBericht[(size_t) p].store (prog.autoGainDb, std::memory_order_relaxed);
    if (p == Pfad::committed)
        dynamikAktiv.store (prog.eqEngagiert && ! prog.hardBypass && prog.irgendeinBandDynamisch(),
                            std::memory_order_release);
}

bool DspKern::uebernehmeZustand (const param::DspSatz& satz, Pfad p)
{
    // Etappe 4a: EIN Publikationsweg. Die Transaktion ruft beide Haelften
    // getrennt - S5 vor, S8 hinter ihrem Commit-Punkt (Manifest §5.11.4).
    if (! baueVor (satz, p)) return false;   // busy_retry (M-44)
    publiziereVorbau (p);
    return true;
}

bool DspKern::baueVor (const param::DspSatz& satz, Pfad p)
{
    // B-10 / M-07: ein ausgeschalteter Zustand belegt KEINE Bank - er ist
    // bankfrei und scheitert hier nie. Gerechnet wird trotzdem, damit der
    // abgeleitete Wert lesbar bleibt (M-35).
    auto& prog = vorbau[(size_t) p];
    baueProgramm (satz, abtastrate, 0, prog);
    const bool pflicht = satz.werte[(size_t) param::kIndexEqEnabled].b;
    vorbauBankpflichtig[(size_t) p] = pflicht;
    return ! pflicht || baenke.freieSlots() > 0;
}

void DspKern::publiziereVorbau (Pfad p) noexcept
{
    const auto& prog = vorbau[(size_t) p];

    if (! vorbauBankpflichtig[(size_t) p])
    {
        // Bankfrei (E-18): die ENDE-Marke. Der Pfad blendet am Blockrand in
        // die Ruhe und dient seine Bank ueber den ACK aus.
        meldeProgramm (p, prog);
        baenke.publiziereEnde (p);
        if (p == Pfad::candidate) candidateAktiv.store (false, std::memory_order_release);
        return;
    }

    // Bankpflichtig: `baueVor` hat eine freie Bank gesehen, und zwischen
    // beiden Aufrufen belegt kein anderer Schreiber eine (DspKern.h). Der
    // Rueckweg darunter ist deshalb unerreichbar; er steht, damit ein
    // Vertragsbruch des Aufrufers nie eine fremde Bank beschreibt.
    const int slot = baenke.reserviere();
    if (slot < 0) { jassertfalse; return; }

    auto& bank = baenke.bank (slot);
    bank.programm = prog;
    bank.programm.generation = baenke.naechsteGeneration();

    // M-07/E-8: die neue Bank startet KALT. Ist der Wechsel am Blockrand ein
    // reiner Rampenwechsel, uebernimmt der Audiothread dort den Zustand der
    // laufenden Bank (B-4) - der Worker kann das nicht, weil ihm diese Bank
    // nicht gehoert.
    bank.zustaendeNullen();
    for (int i = 0; i < kSlots; ++i)
    {
        auto& z = bank.baender[(size_t) i];
        z.svfVon = z.svfNach = bank.programm.baender[(size_t) i].svfRuhe;
    }

    baenke.publiziere (p, slot);
    meldeProgramm (p, bank.programm);
    if (p == Pfad::candidate) candidateAktiv.store (true, std::memory_order_release);
}

void DspKern::beendeCandidate()
{
    // B-8: gekoppelt - Flag UND regulaerer Weg. Das Flag laesst die Auswahl
    // sofort auf Processed zurueckfallen; die ENDE-Marke fuehrt die Baenke
    // ueber Fade und ACK zurueck in den Pool.
    candidateAktiv.store (false, std::memory_order_release);
    baenke.publiziereEnde (Pfad::candidate);
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

bool DspKern::istPassthrough (int slot) const noexcept
{
    const auto& p = programmVon (slot);
    return ! p.eqEngagiert || p.hardBypass;
}

bool DspKern::pfadRechnet (Pfad p) const noexcept
{
    const auto& z = pfade[(size_t) p];
    return z.aktiv >= 0 || z.quelle >= 0 || z.uebergang != Uebergang::keiner;
}

void DspKern::auslenkungenDb (double* achtWerte) const noexcept
{
    for (int i = 0; i < kSlots; ++i)
        achtWerte[i] = pfade[0].auslenkungen[(size_t) i].load (std::memory_order_relaxed);
}

void DspKern::auslenkungenCandidateDb (double* achtWerte) const noexcept
{
    for (int i = 0; i < kSlots; ++i)
        achtWerte[i] = pfade[1].auslenkungen[(size_t) i].load (std::memory_order_relaxed);
}

const double* DspKern::tap (Tap t, int kanal) const noexcept
{
    if (tapGueltig <= 0 || kanal < 0 || kanal > 1) return nullptr;
    const size_t versatz = (((size_t) t * 2u) + (size_t) kanal) * (size_t) maxBlockGroesse;
    if (versatz + (size_t) tapGueltig > tapPuffer.size()) return nullptr;
    return tapPuffer.data() + versatz;
}

double* DspKern::tapZeiger (Tap t, int kanal) noexcept
{
    return tapPuffer.data() + (((size_t) t * 2u) + (size_t) kanal) * (size_t) maxBlockGroesse;
}

//==============================================================================
bool DspKern::hoermatrixMischtCandidate() const noexcept
{
    return hoerLaufend == Hoermatrix::candidate
        || (hoerFadeRest > 0 && hoerVorher == Hoermatrix::candidate);
}

void DspKern::beendeHoerHalt() noexcept
{
    auto& z = pfade[(size_t) Pfad::candidate];
    if (z.uebergang != Uebergang::hoerHalt || hoermatrixMischtCandidate()) return;

    // X-1: die Rueckblende ist zu Ende, keine Auswahl hoert die Bank mehr. Sie
    // dient jetzt ueber den regulaeren Weg aus - ACK, danach Reclaim durch
    // den Worker (M-43, M-46).
    baenke.meldeAusgedient (z.quelle);
    z.quelle    = -1;
    z.uebergang = Uebergang::keiner;
    z.rest      = 0;
}

//==============================================================================
void DspKern::blockrand (Pfad p) noexcept
{
    auto& z = pfade[(size_t) p];

    // B-5, Entscheid E-17: ein laufender Uebergang wird ZU ENDE gefuehrt. Ein
    // Wechsel, der jetzt eintrifft, bleibt publiziert (der Worker darf ihn
    // durch einen neueren verdraengen) und wird am ersten Blockrand nach dem
    // Ende genommen. Jeder Uebergang beginnt damit beim zuletzt ausgegebenen
    // Signal, und kein Pfad haelt je mehr als zwei Baenke.
    if (z.uebergang != Uebergang::keiner) return;

    const int neu = baenke.uebernehmeBereiten (p);
    if (neu == -1) return;
    zaehlerUebernahmen.fetch_add (1, std::memory_order_relaxed);

    const int alt = z.aktiv;

    if (neu == DspBankPool::kEnde)
    {
        // B-8 / B-10: in die Ruhe, ueber denselben Crossfade wie jeder
        // topologische Wechsel. Ist der Pfad schon in Ruhe, gibt es nichts
        // auszublenden.
        if (alt < 0) return;
        baenke.beginneVerblassen (alt);
        z.quelle = alt;
        z.aktiv  = -1;
        // Die Rampen behalten ihre Ziele: die ausblendende Bank klingt mit
        // IHREN Gains aus, genau wie beim Hard-Bypass-Wechsel (M-06).

        // X-1 (Entscheid E-33): mischt die Hoermatrix noch Candidate-Anteile,
        // ist IHRE Rueckblende die einzige Blende. Die Bank fadet dann nicht
        // parallel nach Dry, sondern klingt unveraendert weiter und dient
        // erst aus, wenn die Hoermatrix sie loslaesst (`beendeHoerHalt`) -
        // sonst nullten zwei identische A/B-Zustaende nicht (M-55).
        const bool hoerHalt = p == Pfad::candidate && hoermatrixMischtCandidate();
        z.uebergang = hoerHalt ? Uebergang::hoerHalt : Uebergang::crossfade;
        z.rest      = hoerHalt ? 0 : kFadeSamples;
        return;
    }

    auto& bankNeu = baenke.bank (neu);
    const bool nurRampen = alt >= 0 && rampenKompatibel (baenke.bank (alt).programm, bankNeu.programm);

    if (nurRampen)
    {
        // B-4: der Filter- und Huellkurvenzustand WANDERT mit. Beide Baenke
        // gehoeren in diesem Moment dem Audiothread (die alte ist aktiv, die
        // neue gerade genommen); die Kopie ist ein flacher Wertetransfer ohne
        // Allokation.
        bankNeu.baender         = baenke.bank (alt).baender;
        bankNeu.monoBassZustand = baenke.bank (alt).monoBassZustand;
    }

    if (alt >= 0) baenke.beginneVerblassen (alt);
    z.quelle    = alt;
    z.aktiv     = neu;
    z.uebergang = nurRampen ? Uebergang::rampe : Uebergang::crossfade;
    z.rest      = nurRampen ? kRampeSamples : kFadeSamples;

    // B-6: die Rampen DIESES Pfades laufen auf die Ziele DIESES Programms.
    const auto& pn = bankNeu.programm;
    z.rampen.input.setzeZiel (pn.inputTrimLin);
    z.rampen.output.setzeZiel (pn.outputTrimLin);
    z.rampen.mix.setzeZiel (pn.mix);
    z.rampen.width.setzeZiel (pn.width);
    // M-35: angewandt nur bei eingeschaltetem Schalter - gerechnet und
    // lesbar ist der Wert immer.
    z.rampen.autoGain.setzeZiel (pn.autoGainAn ? pn.autoGainLin : 1.0);
}

void DspKern::heileZustaende (int slot) noexcept
{
    if (slot < 0 || slot >= DspBankPool::kBaenke) return;
    auto& bank = baenke.bank (slot);

    // §5.9 Feinheit 3, Entscheid E-22: die Zustandspruefung laeuft am
    // BLOCKRAND VOR dem ersten Sample. Ein Zustand, der zwischen zwei Bloecken
    // entgleist ist, rechnet damit keinen einzigen Sample mehr - der Ausgang
    // bleibt endlich (B-20).
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

    // Denormals am selben Blockrand (E-14).
    const int getroffen = bank.riegleDenormale();
    if (getroffen > 0)
        zaehlerDenormale.fetch_add ((std::uint64_t) getroffen, std::memory_order_relaxed);
}

//==============================================================================
void DspKern::verarbeiteBand (PfadZustand& pz, DspBank& bank, const DspBank* quelle, int rampeRest,
                              int slotIndex, double* L, double* R, int numSamples) noexcept
{
    const BandProgramm& b = bank.programm.baender[(size_t) slotIndex];
    if (! b.aktiv) return;

    BandZustand& z = bank.baender[(size_t) slotIndex];

    // B-4: waehrend eines Rampenuebergangs laufen die Koeffizienten von der
    // Quellbank zur neuen - `rampenKompatibel` hat gleiche Topologie
    // zugesagt, also sind Belegung, Typ und Modus beider Seiten dieselben.
    const BandProgramm* qb = (quelle != nullptr && rampeRest > 0)
                                 ? &quelle->programm.baender[(size_t) slotIndex] : nullptr;
    if (qb != nullptr && ! qb->aktiv) qb = nullptr;

    // Ein statisches Band mit bitgenau neutralem Biquad kostet nichts und
    // aendert nichts - es wird uebersprungen, solange auch seine Rampe nicht
    // von einem nicht neutralen Entwurf herkommt UND sein Zustand leer ist.
    // Nach einer Rampe auf 0 dB traegt der Zustand noch zwei Samples Rest;
    // der neutrale Biquad spuelt ihn in genau zwei Samples auf exakt 0, und
    // erst danach ist Ueberspringen dasselbe wie Rechnen. Ein dynamisches
    // Band laeuft immer, weil seine Auslenkung sich bewegen kann.
    const bool zustandLeer = z.statisch[0].z1 == 0.0 && z.statisch[0].z2 == 0.0
                          && z.statisch[1].z1 == 0.0 && z.statisch[1].z2 == 0.0;
    if (! b.nutztSvf && b.statischIstEinheit && (qb == nullptr || qb->statischIstEinheit) && zustandLeer) return;

    const bool zweiKomponenten = (b.modus == Kanalmodus::stereo);
    bool detektorZuletzt = false;   ///< lief der Detektor am letzten Sample dieses Fensters?

    for (int i = 0; i < numSamples; ++i)
    {
        // Rampenposition dieses Samples: das erste Sample des Uebergangs
        // traegt 1/kRampeSamples, das letzte 1,0 - dieselbe Zaehlung wie
        // `Rampe::tick`.
        double t = 1.0;
        if (qb != nullptr && i < rampeRest)
            t = 1.0 - (double) (rampeRest - i - 1) / (double) kRampeSamples;
        const bool mitte = t < 1.0;

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
            // W-3 (Entscheid E-29): im Rampenuebergang laeuft der Detektor,
            // solange EINES der beiden Programme ihn laufen laesst - die
            // interpolierte Range ist dann ungleich 0. Das Flag des
            // Zielprogramms gilt erst nach dem Ende der Rampe; M-20 bleibt die
            // Ruhezusage.
            const bool detektorAktiv = b.detektorLaeuft || (mitte && qb->detektorLaeuft);
            detektorZuletzt = detektorAktiv;

            // --- Detektor: hoert das bandgefilterte Signal VOR dem Band ---
            if (detektorAktiv)
            {
                // Beide Seiten tragen entworfene Koeffizienten, sobald das Band
                // einen Detektor hat (E-29) - auch die Seite mit Range 0.
                const Biquad d = mitte ? mische (qb->detektor, b.detektor, t) : b.detektor;
                double leistungEin;
                if (zweiKomponenten)
                {
                    const double d0 = z.detektor[0].tick (d, x0);
                    const double d1 = z.detektor[1].tick (d, x1);
                    // Ein gemeinsamer Pegel fuer beide Komponenten (E-5).
                    leistungEin = (d0 * d0 + d1 * d1) * 0.5;
                }
                else
                {
                    const double d0 = z.detektor[0].tick (d, x0);
                    leistungEin = d0 * d0;
                }
                // W-2 (E-28): attack_ms, hold_ms und release_ms tragen im
                // Vertrag `wechsel = rampe` - ihre Koeffizienten laufen mit den
                // uebrigen Rampenwerten, der Huellkurvenzustand wandert mit.
                const HuellkurveKoeffizienten h = mitte ? mische (qb->huelle, b.huelle, t) : b.huelle;
                z.huelle.tick (h, leistungEin);
            }

            // --- Steuerrate: Kennlinie und Neuentwurf ---------------------
            if (z.schrittRest <= 0)
            {
                double grundG = b.grundG, q = b.q, gainDb = b.gainDb;
                double thresholdDb = b.thresholdDb, rangeDb = b.rangeDb;
                if (mitte)
                {
                    grundG      = lerp (qb->grundG,      grundG,      t);
                    q           = lerp (qb->q,           q,           t);
                    gainDb      = lerp (qb->gainDb,      gainDb,      t);
                    thresholdDb = lerp (qb->thresholdDb, thresholdDb, t);
                    rangeDb     = lerp (qb->rangeDb,     rangeDb,     t);
                }

                double gDyn = 0.0;
                if (detektorAktiv)
                    gDyn = dynamischeKennlinie (leistungInDb (z.huelle.leistung), thresholdDb, rangeDb);
                z.auslenkungDb = gDyn;
                z.svfVon       = z.svfNach;
                z.svfNach      = svfEntwurf (b.typ, grundG, q, gainDb + gDyn);
                z.schrittRest  = kDynamikSchritt;
            }

            const double tSchritt = 1.0 - (double) z.schrittRest / (double) kDynamikSchritt;
            const auto k = mische (z.svfVon, z.svfNach, tSchritt);
            --z.schrittRest;

            y0 = z.svf[0].tick (k, x0);
            if (zweiKomponenten) y1 = z.svf[1].tick (k, x1);
        }
        else
        {
            const Biquad f = mitte ? mische (qb->statisch, b.statisch, t) : b.statisch;
            y0 = z.statisch[0].tick (f, x0);
            if (zweiKomponenten) y1 = z.statisch[1].tick (f, x1);
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

    // W-3 (E-29): ein Detektor, der nicht mehr laeuft, haelt keinen Zustand.
    // Nach einer Rampe der Range auf 0 ist er damit wirklich aus, und eine
    // spaetere Rampe von 0 weg beginnt wie ein frischer Detektor bei 0 statt
    // bei einem Pegel von damals.
    if (b.nutztSvf && ! detektorZuletzt)
    {
        z.huelle.nullen();
        for (auto& d : z.detektor) d.nullen();
    }

    // M-27: der zuletzt gerechnete Wert am Ende des Fensters - kein Mittel,
    // nie aus den Einstellwerten hergeleitet. Ein nicht-endlicher Wert wird
    // 0 und gezaehlt (M-113); ein NaN reist nie ueber den Draht. B-9: in die
    // Auslenkungen DIESES Pfades.
    double gemeldet = z.auslenkungDb;
    if (! std::isfinite (gemeldet))
    {
        gemeldet = 0.0;
        zaehlerAuslenkung.fetch_add (1, std::memory_order_relaxed);
    }
    pz.auslenkungen[(size_t) slotIndex].store (gemeldet, std::memory_order_relaxed);
}

//==============================================================================
void DspKern::verarbeiteBank (PfadZustand& z, int slot, int rampeQuelle, int rampeRest,
                              double* L, double* R, int numSamples) noexcept
{
    const DspProgramm& p = programmVon (slot);
    auto& r = z.rampen;

    // Passthrough: NICHTS rechnen. Die Rampen laufen weiter, damit beide
    // Durchlaeufe eines Crossfades gleich viele Ticks machen.
    if (! p.eqEngagiert || p.hardBypass)
    {
        for (int i = 0; i < numSamples; ++i) r.tick();
        for (auto& a : z.auslenkungen) a.store (0.0, std::memory_order_relaxed);
        return;
    }

    auto& bank = baenke.bank (slot);
    const DspBank* quelle = (rampeQuelle >= 0 && rampeRest > 0) ? &baenke.bank (rampeQuelle) : nullptr;
    const double* dL = dryL.data();
    const double* dR = dryR.data();

    // --- Input-Trim ------------------------------------------------------
    // M-31: 0 dB ist ein Unity-KURZSCHLUSS. Die Ruhepruefung fragt den
    // Rampenzustand, nicht nur den Zielwert.
    if (r.input.ruhtBei (1.0))
    {
        for (int i = 0; i < numSamples; ++i) r.input.tick();
    }
    else
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const double g = r.input.tick();
            L[i] *= g; R[i] *= g;
        }
    }

    // --- M/S-Stufe: Width und Mono-Bass ----------------------------------
    // M-30: bei width == 1,0 und mono_bass_hz == 0 wird die Matrix GAR NICHT
    // gerechnet; `msStufenLaeufe` macht das zaehlbar (B-19).
    const bool monoBass = p.monoBassHz > 0.0;
    if (r.width.ruhtBei (1.0) && ! monoBass)
    {
        for (int i = 0; i < numSamples; ++i) r.width.tick();
    }
    else
    {
        zaehlerMsStufe.fetch_add (1, std::memory_order_relaxed);
        for (int i = 0; i < numSamples; ++i)
        {
            const double w = r.width.tick();
            const double m = (L[i] + R[i]) * 0.5;
            double s       = (L[i] - R[i]) * 0.5;
            if (monoBass)
            {
                Biquad hp = p.monoBassHochpass;
                if (quelle != nullptr && i < rampeRest)
                    hp = mische (quelle->programm.monoBassHochpass, hp,
                                 1.0 - (double) (rampeRest - i - 1) / (double) kRampeSamples);
                s = bank.monoBassZustand.tick (hp, s);
            }
            s *= w;
            L[i] = m + s; R[i] = m - s;
        }
    }

    // M-121: ein Slot, der KEINE Auslenkung erzeugt, meldet exakt 0,0 - und
    // zwar HIER, nicht erst wenn er wieder laeuft.
    for (int i = 0; i < kSlots; ++i)
    {
        const auto& bp = bank.programm.baender[(size_t) i];
        if (! (bp.aktiv && bp.nutztSvf))
            z.auslenkungen[(size_t) i].store (0.0, std::memory_order_relaxed);
    }

    // --- die acht Baender, Slot 0 zuerst (M-15) --------------------------
    for (int slotIndex = 0; slotIndex < kSlots; ++slotIndex)
        verarbeiteBand (z, bank, quelle, rampeRest, slotIndex, L, R, numSamples);

    // --- Auto-Gain, Mix, Output-Trim -------------------------------------
    // Reihenfolge eingefroren (§3.0, §5.3): Auto-Gain im Wet-Zweig VOR Mix,
    // Output-Trim als Ausgangsregler des Users HINTER Mix.
    const bool autoGainRuht = r.autoGain.ruhtBei (1.0);
    const bool mixRuht      = r.mix.ruhtBei (1.0);
    const bool outRuht      = r.output.ruhtBei (1.0);

    for (int i = 0; i < numSamples; ++i)
    {
        const double ag  = r.autoGain.tick();
        const double mix = r.mix.tick();
        const double out = r.output.tick();

        double l = L[i], rr = R[i];
        if (! autoGainRuht) { l *= ag; rr *= ag; }
        if (! mixRuht)
        {
            // Linear, nicht equal-power (§5.3 Feinheit 4). Bei 0,0 wird der
            // Wet-Zweig nicht gerechnet: `0 * wet` truege sonst ein NaN aus
            // dem Wet-Zweig in den Ausgang (M-33).
            if (mix == 0.0)      { l = dL[i]; rr = dR[i]; }
            else if (mix != 1.0) { l  = mix * l  + (1.0 - mix) * dL[i];
                                   rr = mix * rr + (1.0 - mix) * dR[i]; }
        }
        if (! outRuht) { l *= out; rr *= out; }
        L[i] = l; R[i] = rr;
    }
}

//==============================================================================
void DspKern::verarbeitePfad (Pfad p, const double* eingangL, const double* eingangR, int numSamples,
                              double* vonL, double* vonR, double* nachL, double* nachR,
                              double* ausL, double* ausR, double* gewicht) noexcept
{
    auto& z = pfade[(size_t) p];
    const size_t n     = (size_t) numSamples;
    const size_t bytes = n * sizeof (double);

    const bool crossfade = z.uebergang == Uebergang::crossfade && z.rest > 0;
    const bool rampe     = z.uebergang == Uebergang::rampe     && z.rest > 0;
    const bool hoerHalt  = z.uebergang == Uebergang::hoerHalt;

    // --- die Quelle eines Crossfades -------------------------------------
    // Beide Durchlaeufe sehen dieselben Rampenwerte: die globalen Stufen des
    // Pfades sind EINE Stufe (E-7). Der Rampenstand wird deshalb gesichert
    // und nach dem Quelllauf zurueckgesetzt.
    if (crossfade)
    {
        if (z.quelle >= 0 && ! istPassthrough (z.quelle))
        {
            const Rampen gesichert = z.rampen;
            std::memcpy (vonL, eingangL, bytes);
            std::memcpy (vonR, eingangR, bytes);
            verarbeiteBank (z, z.quelle, -1, 0, vonL, vonR, numSamples);
            z.rampen = gesichert;
        }
        else
        {
            // Aus der Ruhe oder aus einem Passthrough: vom unveraenderten,
            // NICHT verriegelten Eingang - der Passthrough sanitisiert nichts.
            std::memcpy (vonL, dryL.data(), bytes);
            std::memcpy (vonR, dryR.data(), bytes);
        }
    }

    // --- die aktive Bank -------------------------------------------------
    // X-1: im Hoerhalt ist die endende Bank die EINZIGE, die klingt - sie
    // rechnet wie eine aktive Bank weiter, mit ihren Rampen und ihren
    // Auslenkungen, bis die Hoermatrix sie loslaesst.
    const int klingend = hoerHalt ? z.quelle : z.aktiv;
    if (klingend >= 0 && ! istPassthrough (klingend))
    {
        std::memcpy (nachL, eingangL, bytes);
        std::memcpy (nachR, eingangR, bytes);
        verarbeiteBank (z, klingend, rampe ? z.quelle : -1, rampe ? z.rest : 0, nachL, nachR, numSamples);
    }
    else
    {
        std::memcpy (nachL, dryL.data(), bytes);
        std::memcpy (nachR, dryR.data(), bytes);
        for (size_t i = 0; i < n; ++i) z.rampen.tick();
        // Ein nicht rechnender Pfad meldet exakt 0,0 Auslenkung (M-27).
        for (auto& a : z.auslenkungen) a.store (0.0, std::memory_order_relaxed);
    }

    // --- Mischung und Uebergangsrest -------------------------------------
    // Crossfade linear (§5.2 Feinheit 2): das erste Fadesample traegt die
    // Quelle allein, ab dem Fadeende steht die aktive Bank BITGENAU allein
    // (M-04). Eine Rampe mischt nicht - sie laeuft in den Koeffizienten.
    int rest = z.rest;
    for (size_t i = 0; i < n; ++i)
    {
        if (crossfade && rest > 0)
        {
            const double t = 1.0 - (double) rest / (double) kFadeSamples;
            ausL[i] = vonL[i] * (1.0 - t) + nachL[i] * t;
            ausR[i] = vonR[i] * (1.0 - t) + nachR[i] * t;
            gewicht[i] = t;
            --rest;
        }
        else
        {
            ausL[i] = nachL[i];
            ausR[i] = nachR[i];
            gewicht[i] = 1.0;
            if (rampe && rest > 0) --rest;
        }
    }
    z.rest = rest;

    // --- Uebergangsende: die Quelle dient ueber den ACK aus ---------------
    // Den Hoerhalt beendet nicht der Rest, sondern die Hoermatrix (X-1).
    if (z.uebergang != Uebergang::keiner && ! hoerHalt && z.rest <= 0)
    {
        if (z.quelle >= 0) baenke.meldeAusgedient (z.quelle);
        z.quelle    = -1;
        z.uebergang = Uebergang::keiner;
        z.rest      = 0;
    }
}

//==============================================================================
void DspKern::verarbeite (float* const* kanaele, int numKanaele, int numSamples) noexcept
{
    RtWache::Bereich wache;

    if (kanaele == nullptr || numKanaele <= 0 || numSamples <= 0) return;

    // B-11: die Uebernahme beider Pfade laeuft GENAU EINMAL je aeusserem
    // Aufruf, vor dem ersten Sample (M-25, R9). Die Stueckelung unten
    // begrenzt nur den Puffer.
    blockrand (Pfad::committed);
    blockrand (Pfad::candidate);

    // E-22, B-20: Filterzustaende heilen, BEVOR ein Sample sie liest.
    for (const auto& z : pfade)
    {
        heileZustaende (z.aktiv);
        heileZustaende (z.quelle);
    }

    // M-48: verworfen wird die ANALYSE, nie Audio. Ein Block groesser als
    // `maxBlock` laeuft deshalb in Stuecken durch statt zurueckzuweisen.
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
            if (versatz < numSamples && teilstueckHaken != nullptr)
                teilstueckHaken (teilstueckKontext);
        }
        // Der Tap dieses Blocks ist keine kohaerente Messung mehr.
        tapGueltig = 0;
        return;
    }

    verarbeiteStueck (kanaele, numKanaele, numSamples);
}

void DspKern::verarbeiteStueck (float* const* kanaele, int numKanaele, int numSamples) noexcept
{
    if ((size_t) numSamples > dryL.size()) { letzteKanaele = numKanaele; return; }   // nur bei freigegebenem Kern
    const size_t n     = (size_t) numSamples;
    const size_t bytes = n * sizeof (double);

    // --- die WIRKSAME Hoermatrix (M-56) ----------------------------------
    // E-34: ein Candidate-Pfad, der gerade AUS DER RUHE einblendet, ist fuer
    // die Hoermatrix noch nicht da. Sie blendet erst nach seinem Einblenden
    // auf Candidate; sonst liefe ihr Fade ueber seinen Fade von Dry her, und
    // zwei identische A/B-Zustaende nullten nicht (M-55, Gegenstueck zu X-1).
    const auto& zk = pfade[1];
    const bool candEinblendend = zk.uebergang == Uebergang::crossfade && zk.quelle < 0;
    const bool candDa  = candidateAktiv.load (std::memory_order_acquire) && zk.aktiv >= 0 && ! candEinblendend;
    Hoermatrix wirksam = hoerwunsch.load (std::memory_order_acquire);
    if (wirksam == Hoermatrix::candidate && ! candDa) wirksam = Hoermatrix::processed;
    hoerwirksam.store (wirksam, std::memory_order_release);

    const bool tapPlatz = numSamples <= maxBlockGroesse && ! tapPuffer.empty();

    for (size_t i = 0; i < n; ++i)
    {
        dryL[i] = (double) kanaele[0][i];
        dryR[i] = (numKanaele > 1) ? (double) kanaele[1][i] : dryL[i];
    }

    // --- der Committed-Zustand am Stueckbeginn ---------------------------
    const auto& zc = pfade[0];
    const bool cCrossfade = zc.uebergang == Uebergang::crossfade && zc.rest > 0;
    const bool vonPass    = zc.quelle < 0 || istPassthrough (zc.quelle);
    const bool nachPass   = zc.aktiv  < 0 || istPassthrough (zc.aktiv);

    // B-3: ruht der Committed-Pfad in einem Passthrough - oder blendet er
    // zwischen zwei Passthroughs -, wird KEIN Sample geschrieben, gleich was
    // die Hoermatrix waehlt. Die Hoermatrix liegt hinter dem engagierten
    // Pfad; ohne ihn gibt es nichts, das sie abhoeren koennte.
    const bool committedRuht = nachPass && (zc.uebergang == Uebergang::keiner || vonPass);
    const bool candRechnet   = pfadRechnet (Pfad::candidate);

    // --- der Nicht-Endlich-Riegel, EINMAL je Stueck ----------------------
    // R9: verriegelt und gezaehlt, VOR jedem Filterzustand - und nur, wenn
    // ueberhaupt eine engagierte Bank rechnet. Der Dry-Zweig bleibt roh.
    const auto engagiert = [this] (const PfadZustand& z) noexcept
    {
        return (z.aktiv >= 0 && ! istPassthrough (z.aktiv)) || (z.quelle >= 0 && ! istPassthrough (z.quelle));
    };
    if (engagiert (pfade[0]) || (candRechnet && engagiert (pfade[1])))
    {
        for (size_t i = 0; i < n; ++i)
        {
            double l = dryL[i], r = dryR[i];
            if (! std::isfinite (l)) { l = 0.0; zaehlerEingaenge.fetch_add (1, std::memory_order_relaxed); }
            if (! std::isfinite (r)) { r = 0.0; zaehlerEingaenge.fetch_add (1, std::memory_order_relaxed); }
            eingL[i] = l; eingR[i] = r;
        }
    }

    // --- beide Pfade -----------------------------------------------------
    verarbeitePfad (Pfad::committed, eingL.data(), eingR.data(), numSamples,
                    cVonL.data(), cVonR.data(), cNachL.data(), cNachR.data(),
                    cAusL.data(), cAusR.data(), cGewicht.data());

    if (candRechnet)
    {
        verarbeitePfad (Pfad::candidate, eingL.data(), eingR.data(), numSamples,
                        kVonL.data(), kVonR.data(), kNachL.data(), kNachR.data(),
                        kAusL.data(), kAusR.data(), kGewicht.data());
    }
    else
    {
        // B-8: ohne Candidate-Bank traegt der Puffer den AKTUELLEN Eingang -
        // nie Daten eines Vorblocks.
        std::memcpy (kAusL.data(), dryL.data(), bytes);
        std::memcpy (kAusR.data(), dryR.data(), bytes);
        for (auto& a : pfade[1].auslenkungen) a.store (0.0, std::memory_order_relaxed);
    }

    // --- Taps ------------------------------------------------------------
    if (tapPlatz)
    {
        for (int k = 0; k < 2; ++k)
        {
            std::memcpy (tapZeiger (Tap::preNakama, k),     (k == 0) ? dryL.data()  : dryR.data(),  bytes);
            std::memcpy (tapZeiger (Tap::postCommitted, k), (k == 0) ? cAusL.data() : cAusR.data(), bytes);
            double* postC = tapZeiger (Tap::postCandidate, k);
            if (candRechnet) std::memcpy (postC, (k == 0) ? kAusL.data() : kAusR.data(), bytes);
            else             std::memset (postC, 0, bytes);
        }
        tapGueltig = numSamples;
    }
    letzteKanaele = numKanaele;

    if (committedRuht)
    {
        // Der Uebergang der Hoermatrix hat hier nichts zu blenden - beide
        // Seiten waeren der unveraenderte Eingang.
        hoerLaufend  = wirksam;
        hoerVorher   = wirksam;
        hoerFadeRest = 0;
        beendeHoerHalt();   // X-1: die Hoermatrix mischt jetzt keinen Candidate mehr
        return;
    }

    // --- Hoermatrix, HINTER allen drei Taps (§44.2 letzter Absatz) --------
    // M-55: der Wechsel ist KLICKFREI - ein eigener Uebergang.
    if (wirksam != hoerLaufend)
    {
        if (hoerFadeRest <= 0)
        {
            hoerVorher   = hoerLaufend;
            hoerLaufend  = wirksam;
            hoerFadeRest = kFadeSamples;
        }
        else if (wirksam == hoerVorher)
        {
            // W-5 (Entscheid E-30): zurueck zur QUELLE des laufenden Fades -
            // etwa ein Candidate-Abbruch mitten im Einblenden. Der Fade kehrt
            // seine Richtung um und laeuft vom AKTUELLEN Mischstand zurueck:
            // das Gewicht beider Seiten bleibt am Umschaltsample stehen und
            // wandert von dort in Fadeschritten zurueck.
            std::swap (hoerVorher, hoerLaufend);
            hoerFadeRest = std::max (0, kFadeSamples - hoerFadeRest - 1);
        }
        // Ein Wechsel zu einem DRITTEN Zustand wartet, bis der laufende Fade
        // endet (E-30, wie E-17 fuer die Baenke): ein Mischstand aus zwei
        // Seiten laesst sich ohne dritten Puffer nur zu einer seiner beiden
        // Seiten stetig fortsetzen.
    }

    const double makeup = dbInLinear (kDeltaMakeupDb);

    const auto hoerAus = [&] (Hoermatrix h, double pl, double pr, size_t i, double& l, double& r) noexcept
    {
        switch (h)
        {
            case Hoermatrix::processed: l = pl; r = pr; break;
            case Hoermatrix::dry:       l = dryL[i]; r = dryR[i]; break;
            case Hoermatrix::delta:
                // Fester, MATERIALUNABHAENGIGER Abgleich (§5.10 Feinheit 1).
                l = (pl - dryL[i]) * makeup;
                r = (pr - dryR[i]) * makeup;
                break;
            case Hoermatrix::candidate: l = kAusL[i]; r = kAusR[i]; break;
        }
    };

    // B-3: blendet der Committed-Pfad in einen Passthrough hinein oder aus
    // ihm heraus, blendet JEDE Auswahl zwischen ihrem Wert auf der
    // engagierten Seite und dem unveraenderten Eingang auf der
    // Passthrough-Seite - auch Delta und Candidate. Sonst liefe ein Delta
    // gegen Stille statt gegen den Eingang.
    const bool passthroughUebergang = cCrossfade && (vonPass || nachPass);

    const auto hoerWert = [&] (Hoermatrix h, size_t i, double& l, double& r) noexcept
    {
        if (passthroughUebergang)
        {
            const double w = cGewicht[i];
            if (w < 1.0)
            {
                double lv = dryL[i], rv = dryR[i], ln = dryL[i], rn = dryR[i];
                if (! vonPass)  hoerAus (h, cVonL[i],  cVonR[i],  i, lv, rv);
                if (! nachPass) hoerAus (h, cNachL[i], cNachR[i], i, ln, rn);
                l = lv * (1.0 - w) + ln * w;
                r = rv * (1.0 - w) + rn * w;
                return;
            }
            if (nachPass) { l = dryL[i]; r = dryR[i]; return; }
        }
        hoerAus (h, cAusL[i], cAusR[i], i, l, r);
    };

    // W-1 (Entscheid E-32): blendet der Committed-Pfad in einen Passthrough,
    // endet der Schreibzugriff EXAKT am Fade-Ende - auch mitten in diesem
    // Stueck. Ab dem ersten Sample mit Gewicht 1 ist die Kette verlassen, und
    // der Puffer bleibt unberuehrt: kein float->double->float-Ruecklauf, keine
    // Kopie des Eingangs auf sich selbst (M-05, REGEL B-3).
    size_t schreibBis = n;
    if (passthroughUebergang && nachPass)
        while (schreibBis > 0 && cGewicht[schreibBis - 1] >= 1.0) --schreibBis;

    for (size_t i = 0; i < schreibBis; ++i)
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

    if (schreibBis < n)
    {
        // Ab hier ruht der Pfad (W-1): die Hoermatrix hat nichts mehr zu
        // blenden - dieselbe Ruhe wie im naechsten Stueck mit `committedRuht`.
        hoerLaufend  = wirksam;
        hoerVorher   = wirksam;
        hoerFadeRest = 0;
    }

    // X-1: der Hoermatrix-Zustand dieses Stuecks steht fest. Endete in ihm die
    // Rueckblende, dient die gehaltene Candidate-Bank jetzt aus.
    beendeHoerHalt();
}

} // namespace nakama::dsp

#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1-Endpruefung nach allen Kern-Headern
