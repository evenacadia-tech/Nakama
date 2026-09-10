#pragma once

/*  SONDE-015 Etappe 3 - das unveraenderliche `DspProgram` und sein Bau.

    ZWEI SEITEN, EINE WAHRHEIT. Der Kern hat keinen eigenen Zustandstyp:
    `nakama::parameter::DspSatz` aus Etappe 2 IST der DspState (120 Werte
    plus Zonenliste, Grenzen, Defaults und Reihenfolge aus
    `nakama-parameter-v2.json`). Was hier entsteht, ist ausschliesslich
    ABGELEITET - Koeffizienten, Rampenziele, der Auto-Gain-Betrag, die
    Klemmliste. Ein zweiter Zustandstyp waere die zweite Wahrheit, die R1
    und §33.5 ausschliessen.

    Ein `DspProgramm` ist fuer den Audiothread UNVERAENDERLICH. Es traegt
    keinen Filterzustand: §44.2 verlangt, dass der Worker Koeffizienten
    UND Filterzustand erst nach dem Audio-ACK ueberschreiben darf, und was
    im selben Objekt liegt, ist dabei nicht auseinanderzuhalten. Der
    Zustand gehoert deshalb zur BANK (`DspBankPool.h`).

    Gebaut wird AUSSERHALB des Audiothreads (R9). Der Bau rechnet `sin`,
    `cos`, `pow`, `tan` und die 121 Gitterstellen des Auto-Gains; nichts
    davon laeuft je im Callback.
*/

#include "DspFilter.h"
#include "DspSvf.h"
#include "NakamaParameter.h"

#include <array>
#include <cstdint>

namespace nakama::dsp
{

using nakama::parameter::kSlots;

/** Die fuenf Vertragswerte von `channel_mode` in Enumreihenfolge. */
enum class Kanalmodus { stereo = 0, left, right, mid, side };

/** Die drei Vertragswerte von `sidechain_source` in Enumreihenfolge. */
enum class Sidechain { none = 0, internal, prioritySidechain };

/** Die vier Zustaende der Hoermatrix (R10). Transient - kein Parameter,
    kein Feld im DTO, nichts im `state_hash`. */
enum class Hoermatrix { processed = 0, dry, delta, candidate };

//==============================================================================
// Technikkonstanten mit Test. Der Entwurf nennt keine Zahl fuer sie; sie
// stehen hier EINMAL und werden von B6 gemessen (§5.2, §5.7, §5.10).

/** Laenge des Crossfades bei topologischen Wechseln, in SAMPLES gezaehlt
    (§5.2 Feinheit 1). In Bloecken gezaehlt dauerte derselbe Fade bei
    Blockgroesse 1 und 2048 verschieden lang. */
inline constexpr int kFadeSamples = 256;

/** Laenge der Rampe kontinuierlicher Werte, in Samples. "Genau eine
    definierte Rampe" (§53.8, M-17) - es gibt keine zweite Glaettung. */
inline constexpr int kRampeSamples = 256;

/** Fester Lautheitsabgleich des Delta-Hoerzustands (§5.10 Feinheit 1).
    Fest heisst materialunabhaengig; ein aus dem laufenden Pegel gerechneter
    Abgleich waere ein Kompressor im Hoerweg (§49.2 Gate 5). */
inline constexpr double kDeltaMakeupDb = 12.0;

/** Steuerrate des dynamischen Bandes in Samples: alle so viele Samples
    werden Detektorpegel, Kennlinie und SVF-Koeffizienten neu gerechnet,
    dazwischen laeuft der Mischfaktor linear weiter. Bei 48 kHz sind das
    0,167 ms - feiner als die kuerzeste Attack (0,1 ms). Der DETEKTOR
    selbst laeuft mit voller Audiorate; nur `log10` und `pow` haengen an
    diesem Schritt. */
inline constexpr int kDynamikSchritt = 8;

/** Das Auto-Gain-Gitter: 20 Hz bis 20 kHz in 1/12-Oktav-Schritten (§5.4
    Feinheit 1). log2(1000) = 9,966 Oktaven, aufgerundet auf 120
    Intervalle, also 121 Stellen. */
inline constexpr int    kAutoGainStellen = 121;
inline constexpr double kAutoGainVonHz   = 20.0;
inline constexpr double kAutoGainBisHz   = 20000.0;

//==============================================================================
/** Ein Band-Slot im gebauten Programm. */
struct BandProgramm
{
    bool        aktiv        { false };   ///< occupied UND enabled
    Kanalmodus  modus        { Kanalmodus::stereo };
    Filtertyp   typ          { Filtertyp::bell };

    /*  Der statische Weg: ein RBJ-Biquad, fest entworfen. `istEinheit`
        erlaubt das Ueberspringen ohne Rechnung - bei `gain_db` = 0 ist ein
        Bell BITGENAU 1 (b0 = a0, b1 = a1, b2 = a2), und ein uebersprungener
        Filter kostet auch keinen Zustand. */
    Biquad      statisch     {};
    bool        statischIstEinheit { false };

    /*  Der dynamische Weg: TPT-SVF, dessen Gain je `kDynamikSchritt`
        Samples neu gemischt wird. `grundG` und `q` bleiben dabei fest. */
    bool        dynamisch    { false };   ///< dynamic_enabled UND aktiv
    bool        nutztSvf     { false };   ///< dynamisch UND der Typ hat einen Gain
    double      grundG       { 0.0 };     ///< tan(pi*fc/fs), fuer den SVF-Neuentwurf
    double      q            { 1.0 };
    double      gainDb       { 0.0 };     ///< statischer Anteil
    SvfKoeffizienten svfRuhe {};          ///< SVF bei Auslenkung 0 (Startwert)

    /*  Der Detektor. `laeuft` ist false bei `sidechain_source` = none und
        bei `dynamic_range_db` = 0 - dann wird gar nichts gerechnet (M-20,
        M-22), nicht nur multipliziert. `detektor` und `huelle` sind trotzdem
        entworfen, sobald das Band einen Detektor hat (Sidechain nicht
        `none`): eine Rampe der Range ueber 0 braucht beide Seiten (W-3,
        E-29). */
    bool        detektorLaeuft { false };
    Sidechain   quelle       { Sidechain::none };   ///< der persistente Vertragswert (topologisch)
    Biquad      detektor     {};
    HuellkurveKoeffizienten huelle {};
    double      thresholdDb  { 0.0 };
    double      rangeDb      { 0.0 };

    /*  Die Klemmliste dieses Slots (M-12, M-23). Der PERSISTENTE Wert
        bleibt in beiden Faellen unberuehrt; geklemmt wird beim Bauen. */
    bool        geklemmtSidechain { false };   ///< priority_sidechain -> internal
    bool        gekapptFreq       { false };   ///< freq_hz > 0,45*fs
    double      freqHzWirksam     { 0.0 };     ///< nach der Kappung
};

//==============================================================================
/** Das vollstaendige, unveraenderliche Programm eines Pfades. */
struct DspProgramm
{
    double        samplerate  { 0.0 };
    std::uint64_t generation  { 0 };

    /*  Die beiden Schalter, die den Pfad ueberhaupt oeffnen. Beide sind im
        Vertrag `blockrand`, gehoeren also ins Programm und nicht in eine
        Rampe. */
    bool   eqEngagiert { false };   ///< v2.global.eq_enabled
    bool   hardBypass  { false };   ///< v1.global.bypass

    /*  Rampenziele. Sie stehen als dB beziehungsweise Verhaeltnis DANEBEN,
        weil der Unity-Kurzschluss am dB-Wert haengt und nicht am linearen
        Faktor: `pow(10, 0/20)` ist zwar exakt 1,0, aber eine Rampe, die auf
        1,0 zulaeuft, kann durch 0.99999994 gehen (M-02). */
    double inputTrimDb  { 0.0 }, inputTrimLin  { 1.0 };
    double outputTrimDb { 0.0 }, outputTrimLin { 1.0 };
    double mix          { 1.0 };
    double width        { 1.0 };
    double monoBassHz   { 0.0 };

    /*  Auto-Gain. `autoGainDb` wird IMMER gerechnet und ist immer lesbar
        (M-35); angewandt wird es nur bei `autoGainAn`. */
    bool   autoGainAn   { false };
    double autoGainDb   { 0.0 };
    double autoGainLin  { 1.0 };

    /*  Die M/S-Stufe (Width und Mono-Bass). `msStufeAktiv` ist false, wenn
        width == 1,0 UND monoBassHz == 0 - dann wird die Matrix gar nicht
        gerechnet und der Weg ist bitidentisch (M-30). */
    bool   msStufeAktiv { false };
    Biquad monoBassHochpass {};

    std::array<BandProgramm, (size_t) kSlots> baender {};

    /** Traegt ueberhaupt ein Band Arbeit? Wird fuer die Feldanwesenheit von
        `band_dynamic_gain_db` gebraucht (M-114) und fuer den Kurzschluss
        eines vollstaendig neutralen Programms. */
    bool irgendeinBandAktiv() const noexcept
    {
        for (const auto& b : baender) if (b.aktiv) return true;
        return false;
    }

    bool irgendeinBandDynamisch() const noexcept
    {
        for (const auto& b : baender) if (b.aktiv && b.dynamisch) return true;
        return false;
    }
};

//==============================================================================
/** Baut ein Programm aus dem DTO. AUSSERHALB des Audiothreads (R9).

    `samplerate` muss > 0 sein. Der `DspSatz` gilt als bereits validiert
    (`nakama::parameter::validiere`); dieser Bau prueft ihn nicht erneut,
    sondern KAPPT nur, was samplerateabhaengig ist (Nyquist), und KLEMMT,
    was der Vertrag heute nicht liefern kann (priority_sidechain). */
void baueProgramm (const nakama::parameter::DspSatz& satz, double samplerate,
                   std::uint64_t generation, DspProgramm& aus);

/** Die Auto-Gain-Ableitung, einzeln aufrufbar (§5.4).

    Pink-gewichteter ENERGIEmittelwert der statischen Betragsantwort ueber
    die 121 Gitterstellen: `g_db = -10 * log10( (1/N) * SUM |H(f)|^2 )` mit
    `E(f) = 0,5 * (|H_L|^2 + |H_R|^2)`, `H_L = H_stereo * H_left * (H_M +
    H_S)/2` und `H_R = H_stereo * H_right * (H_M + H_S)/2`.

    Der Diagonalterm `(H_M + H_S)/2` ist die materialfreie Faltung der
    M/S-Matrix: ein IDENTISCHES Mid/Side-Paar faellt damit exakt auf den
    Stereo-Fall zusammen (M-37). Der Kreuzterm `(H_M - H_S)/2` geht nicht
    ein - ihn exakt zu behandeln hiesse, zwischen korreliertem und
    dekorreliertem Material zu waehlen, und genau diese Materialannahme
    schliesst R4 aus.

    Traegt das Programm KEIN aktives Band, ist das Ergebnis exakt 0,0
    (Kurzschluss statt Gitterlauf, M-36). Dynamische Anteile, Trims, Width,
    Mono-Bass und Mix gehen nicht ein (§5.4 Feinheit 5). */
double leiteAutoGainAb (const DspProgramm& p);

/** Die Gitterfrequenz einer Stelle 0..120. Oeffentlich, damit der Golden
    dieselben Stellen prueft, ohne sie abzuschreiben. */
double autoGainGitterHz (int stelle) noexcept;

/** Unterscheiden sich zwei Programme NUR in Werten, deren Vertragsspalte
    `wechsel = rampe` sagt (R8, Nacharbeit 1 B-4)?

    Dann rampt der Audiothread die laufende Bank auf die neuen Koeffizienten
    und uebernimmt ihren Filter- und Huellkurvenzustand; sonst blendet er
    ueber einen Crossfade zwischen zwei vollstaendigen Programmen. Topologisch
    sind `eq_enabled`, `bypass` (beide `blockrand`), je Slot `occupied` und
    `enabled` (blockrand), `type`, `channel_mode`, `dynamic_enabled` und
    `sidechain_source`, dazu das Ein- und Ausschalten der Mono-Bass-Stufe
    (Entscheid E-20). Laeuft im Audiothread: keine Allokation, keine
    Ausnahme. */
bool rampenKompatibel (const DspProgramm& alt, const DspProgramm& neu) noexcept;

} // namespace nakama::dsp
